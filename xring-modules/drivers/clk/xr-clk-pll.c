// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"

#include "clk.h"
#include "xr-clk-common.h"
#include "xr-clk-pll.h"
#include "xsp_ffa.h"

#define PLL_MASK_OFFSET         16
#define AP_PLL_STABLE_TIME      1000

#define PLL_VOTE_BIT_WIDTH      8
#define FPGA_LOCK_STATUS        1

#define RACC_PLL_RATE_MAX       576000000        /* FVCO_max(576M) */
#define RACC_PLL_RATE_MIN       24000000         /* FVCO_min/postdiv_max = 192M/8 */

#define to_xr_clk_pll(_hw) container_of(_hw, struct xr_clk_pll, hw)


int wait_ap_pll_lock(const void __iomem *lock_base, int pll, unsigned int lock_bit)
{
	int ret = 0;
	unsigned int val;
	unsigned int timeout = 0;

	if (is_fpga())
		return ret;

	udelay(10);
	do {
		val = readl(lock_base);
		val &= BIT(lock_bit);
		timeout++;
		udelay(1);
		if (timeout > AP_PLL_STABLE_TIME) {
			clkerr("ppll-%d enable is timeout\n", pll);
			ret = -1;
			break;
		}
	} while (!val);

	return ret;
}
EXPORT_SYMBOL_GPL(wait_ap_pll_lock);

static void pll_enable_open(struct xr_clk_pll *pll)
{
	unsigned int val = 0;
	const struct clk_pll_cfg_table *cfg = pll->cfg;

	/* vote_en[x]=1 */
	val |= BIT(cfg->vote_en[1]);
	val |= BIT(cfg->vote_en[1] + PLL_MASK_OFFSET);
	writel(val, cfg->base + cfg->vote_en[0]);
}

static int clk_pll_is_enabled(struct clk_hw *hw)
{
	int en, lock;
	struct xr_clk_pll *pll = to_xr_clk_pll(hw);
	const struct clk_pll_cfg_table *cfg = pll->cfg;

	/* if pll controled by soft mode,vote_en not used */
	if (pll->flags & PLL_FLAGS_SOFT_CTRL)
		en = 1;
	else
		en = readl(cfg->base + cfg->vote_en[0]) & BIT(cfg->vote_en[1]);

	if (is_fpga())
		lock = FPGA_LOCK_STATUS;
	else
		lock = readl(cfg->base + cfg->lock_ctrl[0]) & BIT(cfg->lock_ctrl[1]);

	if (en && lock)
		return 1;

	return 0;
}

static int pll_enable_ready(struct xr_clk_pll *pll)
{
	void __iomem *lock_base = NULL;
	int ret_value;
	const struct clk_pll_cfg_table *cfg = pll->cfg;

	lock_base = cfg->base + cfg->lock_ctrl[0];
	ret_value = wait_ap_pll_lock(lock_base, cfg->pll_id, cfg->lock_ctrl[1]);
	if (ret_value)
		return ret_value;

	return 0;
}

static int clk_pll_enable(struct clk_hw *hw)
{
	int ret = 0;
	int vco_rate_flag = VCO_RATE_NO_CARE;
	struct xr_clk_pll *pll = to_xr_clk_pll(hw);
	struct xsp_ffa_msg msg = { 0 };

	ret = clk_pll_is_enabled(hw);
	if (ret)
		return 0;

	if (pll->flags & PLL_FLAGS_SOFT_CTRL) {
		if (pll->safe_id == SAFE_PLL_PCIE_PLL)
			vco_rate_flag = get_pciepll_vco_rate();
		msg.fid = FFA_MSG_CLK_PLL;
		msg.data0 = pll->safe_id;
		msg.data1 = FFA_CLK_OPS_WRITE;
		msg.data2 = FFA_PLL_ENABLE;
		msg.data3 = vco_rate_flag;
		ret = xrsp_ffa_direct_message(&msg);
		if ((ret != 0) || (msg.ret != 0)) {
			clkerr("%s enabled by soft ctrl failed! ret=%d,msg.ret=0x%lx\n",
						clk_hw_get_name(hw), ret, msg.ret);
			ret = -EINVAL;
			goto en_out;
		}
	} else {
		pll_enable_open(pll);
		ret = pll_enable_ready(pll);
		if (ret)
			goto en_out;
	}

en_out:
	return ret;
}

#ifndef CONFIG_XRING_CLK_ALWAYS_ON
static void pll_disable(struct xr_clk_pll *pll)
{
	unsigned int val = 0;
	const struct clk_pll_cfg_table *cfg = pll->cfg;

	/* vote_en[x]=0 */
	val |= BIT(cfg->vote_en[1] + PLL_MASK_OFFSET);
	writel(val, cfg->base + cfg->vote_en[0]);
	udelay(2);
}
#endif

static void clk_pll_disable(struct clk_hw *hw)
{
#ifndef CONFIG_XRING_CLK_ALWAYS_ON
	struct xr_clk_pll *pll = to_xr_clk_pll(hw);
	struct xsp_ffa_msg msg = { 0 };
	int ret = 0;

	if (pll->flags & PLL_FLAGS_SOFT_CTRL) {
		msg.fid = FFA_MSG_CLK_PLL;
		msg.data0 = pll->safe_id;
		msg.data1 = FFA_CLK_OPS_WRITE;
		msg.data2 = FFA_PLL_DISABLE;
		ret = xrsp_ffa_direct_message(&msg);
		if ((ret != 0) || (msg.ret != 0)) {
			clkerr("%s disable by soft ctrl failed! ret=%d,msg.ret=0x%lx\n",
						clk_hw_get_name(hw), ret, msg.ret);
			return;
		}
	} else {
		pll_disable(pll);
	}
#endif
}

static int clk_pll_determine_rate(struct clk_hw *hw,
				struct clk_rate_request *req)
{
	struct xr_clk_pll *pll = to_xr_clk_pll(hw);
	const struct clk_pll_rate_table *rate_table = pll->rate_table;

	if (pll->flags & PLL_FLAGS_DYNAMIC_RATE) {
		if ((req->rate > RACC_PLL_RATE_MAX) || (req->rate < RACC_PLL_RATE_MIN)) {
			clkerr("rate request out of range! req_rate=%lu\n ", req->rate);
			return -EINVAL;
		}
		return 0;
	}

	if (IS_ERR_OR_NULL(rate_table))
		return -EINVAL;

	req->rate = rate_table[0].output_rate;
	return 0;
}

static unsigned long clk_pll_recalc_rate(struct clk_hw *hw,
				unsigned long parent_rate)
{
	struct xr_clk_pll *pll = to_xr_clk_pll(hw);
	const struct clk_pll_rate_table *rate_table = pll->rate_table;
	struct xsp_ffa_msg msg = { 0 };
	int ret = 0;

	/* get pll rate from re-calculate */
	if (pll->flags & PLL_FLAGS_DYNAMIC_RATE) {
		msg.fid = FFA_MSG_CLK_PLL;
		msg.data0 = pll->safe_id;
		msg.data1 = FFA_CLK_OPS_READ;
		ret = xrsp_ffa_direct_message(&msg);
		if (ret != 0)
			clkerr("%s get pll rate failed! ret=%d\n",
						clk_hw_get_name(hw), ret);
		return msg.ret;
	}

	return rate_table[0].output_rate;
}

static int clk_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct xsp_ffa_msg msg = { 0 };
	struct xr_clk_pll *pll = to_xr_clk_pll(hw);
	int ret;

	if (pll->flags & PLL_FLAGS_DYNAMIC_RATE) {
		msg.fid = FFA_MSG_CLK_PLL;
		msg.data0 = pll->safe_id;
		msg.data1 = FFA_CLK_OPS_WRITE;
		msg.data2 = FFA_PLL_SET_RATE;
		msg.data3 = rate;
		ret = xrsp_ffa_direct_message(&msg);
		if ((ret != 0) || (msg.ret != 0)) {
			clkerr("%s set pll rate failed! ret=%d,msg.ret=0x%lx\n",
					clk_hw_get_name(hw), ret, msg.ret);
			return -EINVAL;
		}
	}

	return 0;
}

const struct clk_ops pll_ops = {
	.enable			= clk_pll_enable,
	.disable		= clk_pll_disable,
	.is_enabled		= clk_pll_is_enabled,
	.recalc_rate		= clk_pll_recalc_rate,
	.determine_rate		= clk_pll_determine_rate,
	.set_rate		= clk_pll_set_rate,
};

struct clk_hw *devm_xr_clk_hw_pll(struct device *dev, const char *name,
				const char *parent_name,
				const struct clk_pll_cfg_table *cfg,
				const struct clk_pll_rate_table *rate_table,
				unsigned int num_entry,
				unsigned char safe_id,
				unsigned char pll_flags)
{
	struct xr_clk_pll *pll;
	struct clk_hw *hw;
	struct clk_init_data init;
	int ret;

	pll = devm_kzalloc(dev, sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.flags = 0;
	init.parent_names = &parent_name;
	init.num_parents = 1;
	init.ops = &pll_ops;

	pll->hw.init = &init;
	pll->cfg = cfg;
	pll->rate_table = rate_table;
	pll->num_entry = num_entry;
	pll->safe_id = safe_id;
	pll->flags = pll_flags;

	hw = &pll->hw;
	ret = clk_hw_register(NULL, hw);
	if (ret) {
		clkerr("failed to register pll %s %d\n",
			name, ret);
		return ERR_PTR(ret);
	}
	/* init is local variable, need set NULL before func */
	pll->hw.init = NULL;
	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_pll);
