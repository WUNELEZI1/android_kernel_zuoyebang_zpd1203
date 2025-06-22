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
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/log2.h>

#include "clk.h"
#include "dt-bindings/xring/platform-specific/common/clk/include/clk_resource.h"
#include "xr-clk-common.h"
#include "xr-clk-divider.h"
#include "xsp_ffa.h"

#define to_xr_clk_divider(_hw) container_of(_hw, struct xr_clk_divider, hw)


static inline u32 clk_div_readl(struct xr_clk_divider *divider)
{
	return readl(divider->reg);
}

static inline void clk_div_writel(struct xr_clk_divider *divider, u32 val)
{
	writel(val, divider->reg);
}

static unsigned int get_maxdiv(u8 width, unsigned long flags)
{
	return clk_div_mask(width) + 1;
}

static int div_round_up(unsigned long prate, unsigned long rate,
				unsigned long flags)
{
	int div = DIV_ROUND_UP_ULL((u64)prate, rate);
	return div;
}

static bool is_best_div(unsigned long rate, unsigned long now,
			 unsigned long best, unsigned long flags)
{
	if (flags & CLK_DIVIDER_ROUND_CLOSEST)
		return abs(rate - now) < abs(rate - best);

	return now <= rate && now > best;
}

unsigned long xr_divider_recalc_rate(struct clk_hw *hw, unsigned long prate,
				unsigned int val, unsigned long flags, unsigned long width)
{
	unsigned int div;

	div = val + 1;
	if (!div) {
		WARN(1, "%s recalc a zero div value!\n", clk_hw_get_name(hw));
		return prate;
	}

	return DIV_ROUND_UP_ULL((u64)prate, div);
}

static unsigned long xr_clk_divider_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	struct xsp_ffa_msg msg = { 0 };
	struct xr_clk_divider *divider = to_xr_clk_divider(hw);
	unsigned int val = 0;
	int ret;

	/* get div val through xsp if div is security */
	if (divider->flags & CLK_DIVIDER_SAFE_SET) {
		msg.fid = FFA_MSG_CLK_SECDIV;
		msg.data0 = divider->safe_id;
		msg.data1 = FFA_CLK_OPS_READ;
		ret = xrsp_ffa_direct_message(&msg);
		if (ret != 0)
			clkerr("get secdiv failed,ret=%d, msg.ret=%lu\n", ret, msg.ret);
		val = (unsigned int)msg.ret;
	} else {
		val = clk_div_readl(divider) >> divider->shift;
		val &= clk_div_mask(divider->width);
	}

	return xr_divider_recalc_rate(hw, parent_rate, val, divider->flags, divider->width);
}

static int div_round_closest(unsigned long prate, unsigned long rate,
			      unsigned long flags)
{
	int up, down;
	unsigned long up_rate, down_rate;

	up = DIV_ROUND_UP_ULL((u64)prate, rate);
	down = prate / rate;
	if (!down)
		down = 1;

	up_rate = DIV_ROUND_UP_ULL((u64)prate, up);
	down_rate = DIV_ROUND_UP_ULL((u64)prate, down);

	return (rate - up_rate) <= (down_rate - rate) ? up : down;
}

static int div_round(unsigned long prate, unsigned long rate,
				unsigned long flags)
{
	if (flags & CLK_DIVIDER_ROUND_CLOSEST)
		return div_round_closest(prate, rate, flags);

	return div_round_up(prate, rate, flags);
}

static int get_next_div(int div)
{
	div++;

	return div;
}

static int clk_find_best_div(struct clk_hw *hw, struct clk_hw *parent,
					unsigned long rate,
					unsigned long *best_prate,
					u8 width, unsigned long flags)
{
	unsigned long prate_saved = *best_prate;
	unsigned long prate, best = 0, now, maxdiv;
	int i, bestdiv = 0;

	if (!rate)
		rate = 1;

	maxdiv = get_maxdiv(width, flags);

	if (!(clk_hw_get_flags(hw) & CLK_SET_RATE_PARENT)) {
		prate = *best_prate;
		bestdiv = div_round(prate, rate, flags);
		bestdiv = bestdiv == 0 ? 1 : bestdiv;
		bestdiv = bestdiv > maxdiv ? maxdiv : bestdiv;
		return bestdiv;
	}

	maxdiv = min(ULONG_MAX / rate, maxdiv);

	for (i = get_next_div(0); i <= maxdiv; i = get_next_div(i)) {
		if (rate * i == prate_saved) {
			*best_prate = prate_saved;
			return i;
		}
		prate = clk_hw_round_rate(parent, rate * i);
		now = DIV_ROUND_UP_ULL((u64)prate, i);
		if (is_best_div(rate, now, best, flags)) {
			bestdiv = i;
			best = now;
			*best_prate = prate;
		}
	}

	if (!bestdiv) {
		*best_prate = clk_hw_round_rate(parent, 1);
		bestdiv = get_maxdiv(width, flags);
	}

	return bestdiv;
}

static int xr_divider_determine_rate(struct clk_hw *hw,
				struct clk_rate_request *req,
				u8 width, unsigned long flags)
{
	int div;

	div = clk_find_best_div(hw, req->best_parent_hw, req->rate,
					&req->best_parent_rate, width, flags);
	if (div <= 0) {
		clkerr("get invalid div value: %d\n", div);
		return -EINVAL;
	}

	req->rate = DIV_ROUND_UP_ULL((u64)req->best_parent_rate, div);

	return 0;
}

static long xr_clk_divider_round_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long *prate)
{
	struct xr_clk_divider *divider = to_xr_clk_divider(hw);

	return divider_round_rate(hw, rate, prate, NULL,
				  divider->width, divider->flags);
}

static int xr_clk_divider_determine_rate(struct clk_hw *hw,
				struct clk_rate_request *req)
{
	struct xr_clk_divider *divider = to_xr_clk_divider(hw);

	return xr_divider_determine_rate(hw, req, divider->width,
				divider->flags);
}

int xr_divider_get_val(unsigned long rate, unsigned long prate,
				u8 width, unsigned long flags)
{
	unsigned int div, value;

	div = DIV_ROUND_UP_ULL((u64)prate, rate);
	value = div - 1;

	return min_t(unsigned int, value, clk_div_mask(width));
}

static int xr_clk_divider_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct xsp_ffa_msg msg = { 0 };
	struct xr_clk_divider *divider = to_xr_clk_divider(hw);
	int value;
	unsigned long flags = 0;
	u32 val;
	u8 ret = 0;

	value = xr_divider_get_val(rate, parent_rate,
					divider->width, divider->flags);
	if (value < 0)
		return value;

	if (divider->lock)
		spin_lock_irqsave(divider->lock, flags);
	else
		__acquire(divider->lock);

	/* security clk set rate through xsp to config */
	if (divider->flags == CLK_DIVIDER_SAFE_SET) {
		msg.fid = FFA_MSG_CLK_SECDIV;
		msg.data0 = divider->safe_id;
		msg.data1 = FFA_CLK_OPS_WRITE;
		msg.data2 = value;
		ret = xrsp_ffa_direct_message(&msg);
		if ((ret != 0) || (msg.ret != 0)) {
			clkerr("clk set safe div failed: ret=%d,msg.ret=0x%lx, safe_id=%u\n",
				ret, msg.ret, divider->safe_id);
			ret = -EINVAL;
		}
		goto out;
	}

	/* unsecurity clk set rate here */
	if (divider->flags & CLK_DIVIDER_HIWORD_MASK) {
		val = clk_div_mask(divider->width) << (divider->shift + 16);
	} else {
		val = clk_div_readl(divider);
		val &= ~(clk_div_mask(divider->width) << divider->shift);
	}
	val |= (u32)value << divider->shift;

	clk_div_writel(divider, val);

out:
	if (divider->lock)
		spin_unlock_irqrestore(divider->lock, flags);
	else
		__release(divider->lock);

	udelay(2);

	return ret;
}

static int xr_clk_divider_save_context(struct clk_hw *hw)
{
	struct xr_clk_divider *divider = to_xr_clk_divider(hw);
	u32 val;

	if (divider->flags & CLK_DYNAMIC_POFF_AREA)
		return 0;

	if (divider->flags & CLK_DIVIDER_SAFE_SET)
		return 0;

	val = clk_div_readl(divider) >> divider->shift;
	divider->saved_div = val & clk_div_mask(divider->width);

	return 0;
}

static void xr_clk_divider_restore_context(struct clk_hw *hw)
{
	struct xr_clk_divider *divider = to_xr_clk_divider(hw);
	u32 val;

	if (divider->flags & CLK_DYNAMIC_POFF_AREA)
		return;

	if (divider->flags & CLK_DIVIDER_SAFE_SET)
		return;

	if (divider->flags & CLK_DIVIDER_HIWORD_MASK) {
		val = clk_div_mask(divider->width) << (divider->shift + 16);
	} else {
		val = clk_div_readl(divider);
		val &= ~(clk_div_mask(divider->width) << divider->shift);
	}
	val |= (u32)(divider->saved_div) << divider->shift;

	clk_div_writel(divider, val);
}

const struct clk_ops clk_divider_ops = {
	.determine_rate = xr_clk_divider_determine_rate,
	.round_rate = xr_clk_divider_round_rate,
	.recalc_rate = xr_clk_divider_recalc_rate,
	.set_rate = xr_clk_divider_set_rate,
	.save_context = xr_clk_divider_save_context,
	.restore_context = xr_clk_divider_restore_context,
};

struct clk_hw *xr_clk_hw_register_divider(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width, u32 clk_divider_flags,
		void __iomem *ack_reg, u8 ack_shift,  u8 safe_id,
		spinlock_t *lock)
{
	struct clk_hw *hw;
	struct clk_init_data init = { 0 };
	struct xr_clk_divider *div;
	int ret;

	if (clk_divider_flags & CLK_DIVIDER_HIWORD_MASK) {
		if (width + shift > 16) {
			clkwarn("divider reg width or shift error!\n");
			return ERR_PTR(-EINVAL);
		}
	}

	div = kzalloc(sizeof(*div), GFP_KERNEL);
	if (!div)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.flags = flags;
	init.parent_names = parent_name ? &parent_name : NULL;
	if (parent_name)
		init.num_parents = 1;
	else
		init.num_parents = 0;
	init.ops = &clk_divider_ops;

	div->flags = clk_divider_flags;
	div->shift = shift;
	div->width = width;
	div->safe_id = safe_id;
	div->hw.init = &init;
	div->ack_reg = ack_reg;
	div->ack_shift = ack_shift;
	div->reg = reg;
	div->lock = lock;

	/* register the clock */
	hw = &div->hw;
	ret = clk_hw_register(dev, hw);
	if (ret) {
		kfree(div);
		hw = ERR_PTR(ret);
	}

	return hw;
}

/**
 * xr_clk_hw_unregister_divider - unregister a clk divider
 * @hw: hardware-specific clock data to unregister
 */
static void xr_clk_hw_unregister_divider(struct clk_hw *hw)
{
	struct xr_clk_divider *div;

	div = to_xr_clk_divider(hw);

	clk_hw_unregister(hw);
	kfree(div);
}

static void devm_xr_clk_hw_release_divider(struct device *dev, void *res)
{
	xr_clk_hw_unregister_divider(*(struct clk_hw **)res);
}

struct clk_hw *devm_xr_clk_hw_register_divider(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		void __iomem *reg, u8 shift, u8 width, u32 clk_divider_flags,
		void __iomem *ack_reg, u8 ack_shift, u8 safe_id,
		spinlock_t *lock)
{
	struct clk_hw **ptr, *hw;

	ptr = devres_alloc(devm_xr_clk_hw_release_divider, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	hw = xr_clk_hw_register_divider(dev, name, parent_name,
				flags, reg, shift, width, clk_divider_flags,
				ack_reg, ack_shift, safe_id, lock);

	if (!IS_ERR(hw)) {
		*ptr = hw;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_register_divider);
