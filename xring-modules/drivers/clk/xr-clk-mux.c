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

#include "clk.h"
#include "xr-clk-mux.h"

#define to_xr_clk_mux(_hw) container_of(_hw, struct xr_clk_mux, hw)

#define SW_ACK_MASK(width, shift) ((bitmask(width)) << (shift))

static inline u32 clk_mux_readl(struct xr_clk_mux *mux)
{
	return readl(mux->reg);
}

static inline void clk_mux_writel(struct xr_clk_mux *mux, u32 val)
{
	writel(val, mux->reg);
}

static bool clk_mux_waitack(struct xr_clk_mux *mux, u32 w_val)
{
	u32 val;
	u8 time_us;

	/* only switch should wait, mux no need */
	if (mux->ack_reg == NULL)
		return true;

	if (is_fpga())
		return true;

	for (time_us = 0; time_us < MUX_WAIT_ACK_TIME_OUT_US; time_us++) {
		val = readl(mux->ack_reg) >> mux->ack_shift;
		val &= bitmask(mux->ack_width);
		if ((ffs(val)-1) == w_val)
			return true;
		udelay(1);
	}

	return false;
}

static int xr_clk_mux_val_to_index(struct clk_hw *hw, unsigned int flags,
			 unsigned int val)
{
	int num_parents = clk_hw_get_num_parents(hw);

	if (val >= num_parents)
		return -EINVAL;

	return val;
}

static unsigned int xr_clk_mux_index_to_val(u8 index)
{
	return index;
}

static u8 clk_mux_get_parent(struct clk_hw *hw)
{
	struct xr_clk_mux *mux = to_xr_clk_mux(hw);
	u32 val;

	val = clk_mux_readl(mux) >> mux->shift;
	val &= mux->mask;

	return xr_clk_mux_val_to_index(hw, mux->flags, val);
}

static int clk_mux_determine_rate(struct clk_hw *hw,
				  struct clk_rate_request *req)
{
	struct xr_clk_mux *mux = to_xr_clk_mux(hw);

	return clk_mux_determine_rate_flags(hw, req, mux->flags);
}

static int clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct xr_clk_mux *mux = to_xr_clk_mux(hw);
	unsigned long flags = 0;
	u32 val = xr_clk_mux_index_to_val(index);
	u32 reg;
	u8 ret = 0;

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);

	if (mux->flags & CLK_MUX_HIWORD_MASK) {
		reg = mux->mask << (mux->shift + 16);
	} else {
		reg = clk_mux_readl(mux);
		reg &= ~(mux->mask << mux->shift);
	}
	val = val << mux->shift;
	reg |= val;
	clk_mux_writel(mux, reg);

	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);

	if (!clk_mux_waitack(mux, index)) {
		ret = -EFAULT;
		clkerr("sw:[%s] wait ack failed!\n", clk_hw_get_name(&mux->hw));
	}

	return ret;
}

static int clk_mux_save_context(struct clk_hw *hw)
{
	struct xr_clk_mux *mux = to_xr_clk_mux(hw);

	if (mux->flags & CLK_DYNAMIC_POFF_AREA)
		return 0;

	mux->saved_parent = clk_mux_get_parent(hw);
	return 0;
}

static void clk_mux_restore_context(struct clk_hw *hw)
{
	struct xr_clk_mux *mux = to_xr_clk_mux(hw);

	if (mux->flags & CLK_DYNAMIC_POFF_AREA)
		return;

	clk_mux_set_parent(hw, mux->saved_parent);
}

const struct clk_ops clk_mux_ops = {
	.get_parent = clk_mux_get_parent,
	.set_parent = clk_mux_set_parent,
	.determine_rate = clk_mux_determine_rate,
	.save_context = clk_mux_save_context,
	.restore_context = clk_mux_restore_context,
};

struct clk_hw *xr_clk_hw_register_mux(struct device *dev,
		const char *name, u8 num_parents,
		const char * const *parent_names,
		unsigned long flags, void __iomem *reg, u8 shift, u32 mask,
		u16 clk_mux_flags, void __iomem *ack_reg,
		u8 ack_shift, u8 ack_width, spinlock_t *lock)
{
	struct xr_clk_mux *mux;
	struct clk_hw *hw;
	struct clk_init_data init = {};
	u8 width;
	int ret;

	if (clk_mux_flags & CLK_MUX_HIWORD_MASK) {
		width = fls(mask) - ffs(mask) + 1;
		if (width + shift > 16) {
			pr_err("mux value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	mux = kzalloc(sizeof(*mux), GFP_KERNEL);
	if (!mux) {
		clkerr("failed to allocate memory for mux:%s\n", name);
		return ERR_PTR(-ENOMEM);
	}

	init.flags = flags;
	init.name = name;
	init.parent_names = parent_names;
	init.num_parents = num_parents;
	init.ops = &clk_mux_ops;

	mux->reg = reg;
	mux->shift = shift;
	mux->mask = mask;
	mux->flags = clk_mux_flags;
	mux->lock = lock;
	mux->ack_reg = ack_reg;
	mux->ack_shift = ack_shift;
	mux->ack_width = ack_width;
	mux->hw.init = &init;
	hw = &mux->hw;

	ret = clk_hw_register(dev, hw);
	if (ret) {
		kfree(mux);
		hw = ERR_PTR(ret);
	}

	return hw;
}

static void xr_clk_hw_unregister_mux(struct clk_hw *hw)
{
	struct xr_clk_mux *mux;

	mux = to_xr_clk_mux(hw);

	clk_hw_unregister(hw);
	kfree(mux);
}

static void devm_clk_hw_release_mux(struct device *dev, void *res)
{
	xr_clk_hw_unregister_mux(*(struct clk_hw **)res);
}

struct clk_hw *devm_xr_clk_hw_register_mux(struct device *dev,
		const char *name, u8 num_parents,
		const char * const *parent_names,
		unsigned long flags, void __iomem *reg, u8 shift, u32 mask,
		u16 clk_mux_flags, void __iomem *ack_reg,
		u8 ack_shift, u8 ack_width, spinlock_t *lock)
{
	struct clk_hw **ptr, *hw;

	ptr = devres_alloc(devm_clk_hw_release_mux, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	hw = xr_clk_hw_register_mux(dev, name, num_parents, parent_names,
					flags, reg, shift, mask, clk_mux_flags,
					ack_reg, ack_shift, ack_width, lock);

	if (!IS_ERR(hw)) {
		*ptr = hw;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_register_mux);
