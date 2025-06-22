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
 * GNU General Public License for more details.s_vote
 */

#include "xr-clk-fast-dfs.h"
#include "clk.h"
#include "xr-clk-common.h"
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/units.h>

#define to_xr_clk_fast_dfs(_hw) container_of(_hw, struct xr_clk_fast_dfs, hw)

static DEFINE_SPINLOCK(fast_dfs_lock);

static unsigned int get_rate_level(struct clk_hw *hw, unsigned long rate)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	int level;

	for (level = PROFILE_CNT - 1; level >= 0; level--) {
		if ((fastclk->profile_freq[level] != 0) &&
				(rate >= fastclk->profile_freq[level]))
			break;
	}

	if (level < 0)
		level = 0;

	return (unsigned int)level;
}

/* return the closest rate */
static int fast_dfs_clk_determine_rate(struct clk_hw *hw,
	struct clk_rate_request *req)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	unsigned int level;

	level = get_rate_level(hw, req->rate);

	/* if req rate smaller than all array value, set to profile[0] */
	req->rate = fastclk->profile_freq[level];

	return 0;
}

static unsigned long fast_dfs_clk_recalc_rate(struct clk_hw *hw,
	unsigned long parent_rate)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	struct fast_dfs_hw_cfg *hw_cfg = &(fastclk->hw_cfg);
	void __iomem *div_addr = NULL;
	void __iomem *sw_addr = NULL;
	unsigned int div_value, sw_value;

	div_addr = fastclk->base + hw_cfg->div_cfg[CFG_OFFSET];
	div_value = clkread(div_addr);
	div_value = div_value >> (hw_cfg->div_cfg[CFG_SHIFT]);
	div_value = div_value & bitmask(hw_cfg->div_cfg[CFG_LENGTH]);
	div_value++;

	sw_addr = fastclk->base + hw_cfg->sw_cfg[CFG_OFFSET];
	sw_value = clkread(sw_addr);
	sw_value = sw_value >> (hw_cfg->sw_cfg[CFG_SHIFT]);
	sw_value = sw_value & bitmask(hw_cfg->sw_cfg[CFG_LENGTH]);

	if (sw_value >= PLL_CNT) {
		clkerr("sw value is illegal, sw_value = 0x%x!\n", sw_value);
		return 0;
	}

	/* div_value already "++" and it will not be zero */
	fastclk->rate = (fastclk->sw_freq[sw_value]) / div_value;

	return fastclk->rate;
}

#define FAST_DFS_DEFAULT_MAX_DIV 64
static unsigned int get_best_transitional_div(struct xr_clk_fast_dfs *fastclk,
	unsigned long cur_rate, unsigned long tar_rate,
	unsigned int cur_level, unsigned int tar_level)
{
	unsigned int min_div, max_div, best_trans_div;
	unsigned long cur_pll_profile, tar_pll_profile;
	unsigned long max_rate;

	min_div = min(fastclk->profile_div[cur_level], fastclk->profile_div[tar_level]);
	max_div = max(fastclk->profile_div[cur_level], fastclk->profile_div[tar_level]);

	/* cornor case: if max_div is 0, we set best_trans_div to 64 */
	if (max_div == 0) {
		clkerr("max div is zero!\n");
		max_div = FAST_DFS_DEFAULT_MAX_DIV;
	}

	/* increse HZ_PER_MHZ: compensating for non-divisible scenarios */
	max_rate = max(cur_rate, tar_rate) + HZ_PER_MHZ;

	/* worst case scenario: best div is max_div */
	best_trans_div = max_div;

	if (min_div == 0)
		goto out;

	cur_pll_profile = fastclk->profile_pll[cur_level];
	tar_pll_profile = fastclk->profile_pll[tar_level];

	/*
	 * traverse max_div to min_div, find the best div so than
	 * the transitional frequency is smaller than cur_profile and tar_profile.
	 */
	for (; min_div < max_div; min_div++) {
		if ((cur_pll_profile / min_div) <= max_rate &&
			(tar_pll_profile / min_div) <= max_rate) {
			best_trans_div = min_div;
			break;
		}
	}

out:
	return best_trans_div;
}

static int set_rate_before(struct xr_clk_fast_dfs *fastclk,
	int cur_level, int tar_level)
{
	struct clk *tar_pll = NULL;
	struct clk *cur_pll = NULL;
	const char *tar_pll_name = NULL;
	const char *cur_pll_name = NULL;
	unsigned int cur_sw_val, tar_sw_val;
	int ret;

	tar_sw_val = fastclk->profile_sw[tar_level];
	cur_sw_val = fastclk->profile_sw[cur_level];
	if (tar_sw_val >= fastclk->sw_sel_num || cur_sw_val >= fastclk->sw_sel_num) {
		clkerr("tar sw val %u or cur sw val %u >= sw_sel_num %u fail!\n",
			tar_sw_val, cur_sw_val, fastclk->sw_sel_num);
		return -EINVAL;
	}

	tar_pll_name = fastclk->sw_sels[tar_sw_val];
	tar_pll = fastclk->sw_pll[tar_sw_val];

	cur_pll_name = fastclk->sw_sels[cur_sw_val];
	cur_pll = fastclk->sw_pll[cur_sw_val];

	ret = clk_prepare_enable(tar_pll);
	if (ret) {
		clkerr("tar_pll %s clk_prepare_enable fail, ret = %d!\n", tar_pll_name, ret);
		return ret;
	}

	ret = clk_prepare_enable(cur_pll);
	if (ret) {
		clkerr("cur_pll %s clk_prepare_enable fail, ret = %d!\n", cur_pll_name, ret);
		clk_disable_unprepare(tar_pll);
	}

	return ret;
}

static int set_rate_after(struct xr_clk_fast_dfs *fastclk,
	int cur_level, int tar_level)
{
	struct clk *tar_pll = NULL;
	struct clk *cur_pll = NULL;
	unsigned int tar_sw_val, cur_sw_val;

	tar_sw_val = fastclk->profile_sw[tar_level];
	cur_sw_val = fastclk->profile_sw[cur_level];
	if (tar_sw_val >= fastclk->sw_sel_num || cur_sw_val >= fastclk->sw_sel_num) {
		clkerr("tar sw val %u or cur sw val %u >= sw_sel_num %u fail!\n",
			tar_sw_val, cur_sw_val, fastclk->sw_sel_num);
		return -EINVAL;
	}

	tar_pll = fastclk->sw_pll[tar_sw_val];

	cur_pll = fastclk->sw_pll[cur_sw_val];

	clk_disable_unprepare(cur_pll);

	if (fastclk->en_count)
		clk_disable_unprepare(cur_pll);
	else
		clk_disable_unprepare(tar_pll);

	return 0;
}

static int clk_mux_waitack(struct clk_hw *hw)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	struct fast_dfs_hw_cfg *hw_cfg = &(fastclk->hw_cfg);
	void __iomem *sw_addr = NULL;
	unsigned int sw_value, sw_state_value;
	unsigned int time_us;

	if (is_fpga())
		return 0;

	sw_addr = fastclk->base + hw_cfg->sw_cfg[CFG_OFFSET];
	sw_value = clkread(sw_addr);
	sw_value = sw_value >> (hw_cfg->sw_cfg[CFG_SHIFT]);
	sw_value = sw_value & bitmask(hw_cfg->sw_cfg[CFG_LENGTH]);

	for (time_us = 0; time_us < MUX_WAIT_ACK_TIME_OUT_US; time_us++) {
		sw_addr = fastclk->base + hw_cfg->sw_state_cfg[CFG_OFFSET];
		sw_state_value = clkread(sw_addr);
		sw_state_value = sw_state_value >> (hw_cfg->sw_state_cfg[CFG_SHIFT]);
		sw_state_value = sw_state_value & bitmask(hw_cfg->sw_state_cfg[CFG_LENGTH]);

		if ((ffs(sw_state_value) - 1) == sw_value)
			return 0;
		udelay(1);
	}

	clkerr("%s fast dfs wait mux done timeout, sw_value %u, sw_state %u!\n",
		clk_hw_get_name(hw), sw_value, sw_state_value);
	return -ETIMEDOUT;
}

static int fast_dfs_waitack(struct clk_hw *hw)
{
	return clk_mux_waitack(hw);
}

static int fast_dfs_clk_set_rate(struct clk_hw *hw,
	unsigned long rate, unsigned long parent_rate)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	struct fast_dfs_hw_cfg *hw_cfg = &(fastclk->hw_cfg);
	unsigned long cur_rate, tar_rate;
	unsigned int cur_level, tar_level, best_trans_div;
	void __iomem *div_addr = NULL;
	void __iomem *sw_addr = NULL;
	unsigned long flags;
	int ret;

	cur_rate = clk_get_rate(hw->clk);
	tar_rate = rate;

	cur_level = get_rate_level(hw, cur_rate);
	tar_level = get_rate_level(hw, tar_rate);

	/* best transitional div to prevent middle high and low transitional frequency */
	best_trans_div = get_best_transitional_div(fastclk, cur_rate, tar_rate,
						cur_level, tar_level);
	best_trans_div = best_trans_div - 1;

	div_addr = hw_cfg->div_cfg[CFG_OFFSET] + fastclk->base;
	sw_addr = hw_cfg->sw_cfg[CFG_OFFSET] + fastclk->base;

	ret = set_rate_before(fastclk, cur_level, tar_level);
	if (ret) {
		clkerr("set rate before fail, tar_level = %u, cur_level = %u, ret %d!\n",
			tar_level, cur_level, ret);
		return ret;
	}

	spin_lock_irqsave(&fast_dfs_lock, flags);
	/* set transitional frequency div */
	clkwrite(himask_set(hw_cfg->div_cfg[CFG_SHIFT],
			    hw_cfg->div_cfg[CFG_LENGTH],
			    best_trans_div),
			    div_addr);
	udelay(1);
	/* set target sw */
	clkwrite(himask_set(hw_cfg->sw_cfg[CFG_SHIFT],
			    hw_cfg->sw_cfg[CFG_LENGTH],
			    fastclk->profile_sw[tar_level]),
			    sw_addr);

	udelay(1);
	/* set target div */
	clkwrite(himask_set(hw_cfg->div_cfg[CFG_SHIFT],
			    hw_cfg->div_cfg[CFG_LENGTH],
			    fastclk->profile_div[tar_level] - 1),
			    div_addr);
	udelay(2);

	spin_unlock_irqrestore(&fast_dfs_lock, flags);

	ret = fast_dfs_waitack(hw);
	if (ret)
		clkerr("%s fast dfs waitack timeout, ret %d\n", clk_hw_get_name(hw), ret);

	ret = set_rate_after(fastclk, cur_level, tar_level);
	if (ret) {
		clkerr("set rate after fail, tar_level = %u, cur_level = %u, ret %d!\n",
			tar_level, cur_level, ret);
		return ret;
	}

	fastclk->rate = rate;
	return ret;
}

static int fast_dfs_clk_prepare(struct clk_hw *hw)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	const char *cur_pll_name = NULL;
	struct clk *cur_pll = NULL;
	unsigned int cur_level, sw_val;
	int ret;

	cur_level = get_rate_level(hw, fastclk->rate);
	sw_val = fastclk->profile_sw[cur_level];
	if (sw_val >= fastclk->sw_sel_num) {
		clkerr("sw val %u >= sw_sel_num %u fail!\n",
			sw_val, fastclk->sw_sel_num);
		return -EINVAL;
	}

	cur_pll_name = fastclk->sw_sels[sw_val];
	cur_pll = fastclk->sw_pll[sw_val];

	ret = clk_prepare(cur_pll);
	if (ret)
		clkerr("%s clk prepare fail, ret = %d!\n",
			cur_pll_name, ret);

	return ret;
}

static void fast_dfs_clk_unprepare(struct clk_hw *hw)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	struct clk *cur_pll = NULL;
	unsigned int cur_level, sw_val;

	cur_level = get_rate_level(hw, fastclk->rate);
	sw_val = fastclk->profile_sw[cur_level];
	cur_pll = fastclk->sw_pll[sw_val];

	clk_unprepare(cur_pll);
}

static int fast_dfs_clk_enable(struct clk_hw *hw)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	struct fast_dfs_hw_cfg *hw_cfg = &(fastclk->hw_cfg);
	const char *cur_pll_name = NULL;
	struct clk *cur_pll = NULL;
	unsigned int cur_level, sw_val;
	int ret;

	cur_level = get_rate_level(hw, fastclk->rate);
	sw_val = fastclk->profile_sw[cur_level];
	if (sw_val >= fastclk->sw_sel_num) {
		clkerr("sw_val %u >= sw_sel_num %u fail!\n",
			sw_val, fastclk->sw_sel_num);
		return -EINVAL;
	}

	cur_pll_name = fastclk->sw_sels[sw_val];
	cur_pll = fastclk->sw_pll[sw_val];

	ret = clk_enable(cur_pll);
	if (ret) {
		clkerr("%s enable fail, ret %d!\n", cur_pll_name, ret);
		return ret;
	}

	if (fastclk->gate_flag == FAST_DFS_SCGT_NO_GATE)
		goto out;

	/* enable gate in front of div */
	clkwrite(himask_enable(hw_cfg->scgt_cfg[CFG_SHIFT]),
		hw_cfg->scgt_cfg[CFG_OFFSET] + fastclk->base);
out:
	fastclk->en_count++;

	return 0;
}

static void fast_dfs_clk_disable(struct clk_hw *hw)
{
	struct xr_clk_fast_dfs *fastclk = to_xr_clk_fast_dfs(hw);
	struct fast_dfs_hw_cfg *hw_cfg = &(fastclk->hw_cfg);
	struct clk *cur_pll = NULL;
	unsigned int cur_level, sw_val;

	cur_level = get_rate_level(hw, fastclk->rate);
	sw_val = fastclk->profile_sw[cur_level];
	cur_pll = fastclk->sw_pll[sw_val];

	if (fastclk->always_on == ALWAYS_ON)
		goto out;

	if (fastclk->gate_flag == FAST_DFS_SCGT_NO_GATE)
		goto out;
#ifndef CONFIG_XRING_CLK_ALWAYS_ON
	/* disable gate in front of div */
	clkwrite(himask_disable(hw_cfg->scgt_cfg[CFG_SHIFT]),
		hw_cfg->scgt_cfg[CFG_OFFSET] + fastclk->base);
#endif
out:
	clk_disable(cur_pll);
	if (fastclk->en_count != 0)
		fastclk->en_count--;
}

static const struct clk_ops clk_fast_dfs_ops = {
	.recalc_rate = fast_dfs_clk_recalc_rate,
	.set_rate = fast_dfs_clk_set_rate,
	.determine_rate = fast_dfs_clk_determine_rate,
	.prepare = fast_dfs_clk_prepare,
	.unprepare = fast_dfs_clk_unprepare,
	.enable = fast_dfs_clk_enable,
	.disable = fast_dfs_clk_disable,
};

static int fast_dfs_clk_init(struct device *dev,
			      const struct fast_dfs_clock *fast_dfs_cfg,
			      struct xr_clk_fast_dfs *fastclk)
{
	struct clk_hw *hw = NULL;
	unsigned int i;

	fastclk->gate_flag = fast_dfs_cfg->gate_flag;
	fastclk->sw_sel_num = fast_dfs_cfg->sw_sel_num;

	for (i = 0; i < PROFILE_CNT; i++) {
		fastclk->profile_freq[i] = fast_dfs_cfg->profile_freq[i];
		fastclk->profile_pll[i] = fast_dfs_cfg->profile_pll[i];
		fastclk->profile_div[i] = fast_dfs_cfg->profile_div[i];
		fastclk->profile_sw[i] = fast_dfs_cfg->profile_sw[i];
	}

	for (i = 0; i < GATE_CFG_CNT; i++)
		fastclk->hw_cfg.scgt_cfg[i] = fast_dfs_cfg->hw_cfg.scgt_cfg[i];

	for (i = 0; i < SW_DIV_CFG_CNT; i++) {
		fastclk->hw_cfg.sw_cfg[i] = fast_dfs_cfg->hw_cfg.sw_cfg[i];
		fastclk->hw_cfg.sw_state_cfg[i] = fast_dfs_cfg->hw_cfg.sw_state_cfg[i];
		fastclk->hw_cfg.div_cfg[i] = fast_dfs_cfg->hw_cfg.div_cfg[i];
		fastclk->hw_cfg.div_state_cfg[i] = fast_dfs_cfg->hw_cfg.div_state_cfg[i];
	}

	fastclk->sw_sels = fast_dfs_cfg->sw_sels;
	for (i = 0; i < PLL_CNT; i++) {
		fastclk->sw_freq[i] = fast_dfs_cfg->sw_freq[i];
		fastclk->sw_ids[i] = fast_dfs_cfg->sw_ids[i];
		hw = xr_clk_hw_get(fast_dfs_cfg->sw_ids[i]);
		if (IS_ERR_OR_NULL(hw))
			return -ENODEV;

		fastclk->sw_pll[i] = hw->clk;
	}
	return 0;
}

struct clk_hw *devm_xr_hw_fast_dfs_clock(struct device *dev, void __iomem *base,
		const char *name, struct fast_dfs_clock *fast_dfs_cfg,
		unsigned int always_on, spinlock_t *lock)
{
	struct xr_clk_fast_dfs *fastclk = NULL;
	struct clk_hw *hw = NULL;
	struct clk_init_data init;
	int ret;

	fastclk = devm_kzalloc(dev, sizeof(struct xr_clk_fast_dfs), GFP_KERNEL);
	if (!fastclk)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &clk_fast_dfs_ops;
	init.flags = 0;
	init.parent_names = NULL;
	init.num_parents = 0;

	fastclk->base = base;
	fastclk->en_count = 0;
	fastclk->rate = 0;
	fastclk->lock = lock;
	fastclk->always_on = always_on;

	ret = fast_dfs_clk_init(dev, fast_dfs_cfg, fastclk);
	if (ret) {
		clkerr("fast clock %s init error %d!\n", name, ret);
		return ERR_PTR(ret);
	}

	fastclk->hw.init = &init;
	hw = &fastclk->hw;

	ret = clk_hw_register(NULL, hw);
	if (ret)
		return ERR_PTR(ret);

	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_hw_fast_dfs_clock);
