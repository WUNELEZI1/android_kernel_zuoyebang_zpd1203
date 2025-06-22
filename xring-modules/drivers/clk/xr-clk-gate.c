// SPDX-License-Identifier: GPL-2.0 or later
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "soc/xring/doze.h"
#include "clk.h"
#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"
#include "dt-bindings/xring/platform-specific/common/pmu/include/dvs_resource.h"
#include "xr-clk-gate.h"
#include "xsp_ffa.h"

#define to_xr_clk_gate(_hw) container_of(_hw, struct xr_clk_gate, hw)

static int dvs_channel_bypass(struct xr_clk_gate *gate, unsigned int flag)
{
	struct xsp_ffa_msg msg = { 0 };
	unsigned int dev_id = gate->dvs_flag;
	int ret;

	if (!is_peri_dvfs_ft_on() && !is_media_dvfs_ft_on())
		return 0;

	msg.fid = FFA_MSG_CLK_DVFS;
	msg.data0 = dev_id;
	msg.data1 = flag;
	ret = xrsp_ffa_direct_message(&msg);
	if ((ret != 0) || (msg.ret != 0)) {
		clkerr("dvs channel bypass fail,ret=%d,msg.ret=0x%lx, devid=%u, flag=%u\n",
			ret, msg.ret, dev_id, flag);
		ret = -EINVAL;
	}

	return ret;
}

static int clk_safe_gate_en_dis(unsigned int safe_id, unsigned int en)
{
	struct xsp_ffa_msg msg = { 0 };
	int ret = 0;

	msg.fid = FFA_MSG_CLK_SECGATE;
	msg.data0 = safe_id;
	msg.data1 = FFA_CLK_OPS_WRITE;
	msg.data2 = en;
	ret = xrsp_ffa_direct_message(&msg);
	if ((ret != 0) || (msg.ret != 0)) {
		clkerr("safe gate set failed,safe_id:%d, ret=%lx\n", safe_id, msg.ret);
		return -EINVAL;
	}

	return msg.ret;
}

static int clk_gate_prepare(struct clk_hw *hw)
{
	struct xr_clk_gate *gate = to_xr_clk_gate(hw);
	struct clk *friend_clk = get_friend_clk(gate->friend_hw);
	int ret = 0;

	/* un-bypass dvs soft channel before clock open */
	if (gate->flags & CLK_DVS_BYPASS_SET) {
		ret = dvs_channel_bypass(gate, FFA_DVS_CH_UNBYPASS);
		if (ret)
			return ret;
	}

	/* if friend clk exist,enable it */
	if (friend_clk != NULL) {
		ret = clk_prepare(friend_clk);
		if (ret) {
			clkerr("friend clock:%s prepare failed!\n",
				__clk_get_name(friend_clk));
			goto excpt_out;
		}
	}

	return ret;
excpt_out:
	if (gate->flags & CLK_DVS_BYPASS_SET)
		(void)dvs_channel_bypass(gate, FFA_DVS_CH_BYPASS);
	return ret;
}

static int clk_gate_enable(struct clk_hw *hw)
{
	struct xr_clk_gate *gate = to_xr_clk_gate(hw);
	struct clk *friend_clk = get_friend_clk(gate->friend_hw);
	int ret = 0;

	/* enable clock */
	if (gate->enable != NULL)
		writel(gate->ebits, gate->enable);

	if (gate->flags & CLK_GATE_SAFE_SET) {
		ret = clk_safe_gate_en_dis(gate->safe_id, BL31_GATE_ENABLE);
		if (ret) {
			clkerr("safe clk:%s enable failed!\n", clk_hw_get_name(&gate->hw));
			return ret;
		}
	}

	/* if friend clk exist,enable it */
	if (friend_clk != NULL) {
		ret = clk_enable(friend_clk);
		if (ret) {
			clkerr("friend clock:%s enable failed!\n",
				__clk_get_name(friend_clk));
			return ret;
		}
	}

	if (gate->flags & CLK_GATE_DOZE_VOTE) {
		ret = sys_state_doz2nor_vote(gate->vote_id);
		if (ret) {
			clkerr("clk:%s doz2nor vote failed!\n", clk_hw_get_name(&gate->hw));
			return ret;
		}
	}

	if (gate->sync_time > 0)
		udelay(gate->sync_time);

	return ret;
}

static void clk_gate_disable(struct clk_hw *hw)
{
#ifndef CONFIG_XRING_CLK_ALWAYS_ON
#define CLK_GATE_DISABLE_OFFSET		0x4
	int ret;
	struct xr_clk_gate *gate = to_xr_clk_gate(hw);
	struct clk *friend_clk = get_friend_clk(gate->friend_hw);

	if (gate->enable != NULL && !gate->always_on)
		writel(gate->ebits, gate->enable + CLK_GATE_DISABLE_OFFSET);

	if ((gate->flags & CLK_GATE_SAFE_SET) && !gate->always_on) {
		ret = clk_safe_gate_en_dis(gate->safe_id, BL31_GATE_DISABLE);
		if (ret) {
			clkerr("safe clk:%s disable failed!\n", clk_hw_get_name(&gate->hw));
			return;
		}
	}

	if (gate->flags & CLK_GATE_DOZE_VOTE) {
		ret = sys_state_doz2nor_unvote(gate->vote_id);
		if (ret) {
			clkerr("clk:%s doz2nor unvote failed!ret=%d\n", clk_hw_get_name(&gate->hw), ret);
			return;
		}
	}

	/* if friend clk exist, disable it . */
	if (friend_clk != NULL)
		clk_disable(friend_clk);
#endif
}

static void clk_gate_unprepare(struct clk_hw *hw)
{
#ifndef CONFIG_XRING_CLK_ALWAYS_ON
	struct xr_clk_gate *gate = to_xr_clk_gate(hw);
	struct clk *friend_clk = get_friend_clk(gate->friend_hw);

	if (friend_clk != NULL)
		clk_unprepare(friend_clk);

	/* bypass dvs soft channel after clock close */
	if (gate->flags & CLK_DVS_BYPASS_SET)
		(void)dvs_channel_bypass(gate, FFA_DVS_CH_BYPASS);
#endif
}

const struct clk_ops clk_gate_ops = {
	.prepare        = clk_gate_prepare,
	.unprepare      = clk_gate_unprepare,
	.enable         = clk_gate_enable,
	.disable        = clk_gate_disable,
};

struct clk_hw *devm_xr_clk_hw_gate(struct device *dev, const char *name,
				   const char *parent_name,
				   struct clk_hw **friend_hw,
				   unsigned long flags, void __iomem *reg,
				   unsigned int offset, unsigned int bit_mask,
				   unsigned int always_on,
				   unsigned int sync_time, spinlock_t *lock,
				   unsigned int gate_flags, unsigned int safe_id,
				    unsigned int vote_id, unsigned int dvs_flag)
{
	struct xr_clk_gate *gate = NULL;
	struct clk_hw *hw = NULL;
	struct clk_init_data init;
	int ret;

	gate = devm_kzalloc(dev, sizeof(struct xr_clk_gate), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	/* if bit_mask is 0, represents the enable reg is fake */
	gate->enable = NULL;
	if (bit_mask)
		gate->enable = reg + offset;

	gate->ebits = bit_mask;
	gate->lock = lock;
	gate->always_on = always_on;
	gate->sync_time = sync_time;
	gate->friend_hw = friend_hw;
	gate->flags = gate_flags;
	gate->dvs_flag = dvs_flag;
	gate->safe_id = safe_id;
	gate->vote_id = vote_id;

	init.name = name;
	init.ops = &clk_gate_ops;
	init.flags = flags;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = parent_name ? 1 : 0;

	gate->hw.init = &init;
	hw = &gate->hw;

	ret = clk_hw_register(NULL, hw);
	if (ret)
		return ERR_PTR(ret);
	/* init is local variable, need set NULL before func */
	gate->hw.init = NULL;
	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_gate);
