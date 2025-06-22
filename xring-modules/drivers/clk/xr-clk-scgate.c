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
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "clk.h"
#include "xr-clk-scgate.h"

#define to_clk_scgate(_hw) container_of(_hw, struct xr_clk_scgate, hw)

static inline u32 clk_scgt_readl(struct xr_clk_scgate *gate)
{
	return readl(gate->reg);
}

static inline void clk_scgt_writel(struct xr_clk_scgate *gate, u32 val)
{
	writel(val, gate->reg);
}

static void clk_scgate_endisable(struct clk_hw *hw, int enable)
{
	struct xr_clk_scgate *gate = to_clk_scgate(hw);
	int set = (gate->flags & CLK_GATE_SET_TO_DISABLE) ? 1 : 0;
	unsigned long flags;
	u32 reg;

	set ^= enable;

	if ((!set) && (gate->flags & CLK_SCGATE_ALWAYS_ON_MASK))
		return;

	if (gate->lock)
		spin_lock_irqsave(gate->lock, flags);
	else
		__acquire(gate->lock);

	if (gate->flags & CLK_GATE_HIWORD_MASK) {
		reg = BIT(gate->bit_idx + 16);
		if (set)
			reg |= BIT(gate->bit_idx);
	} else {
		reg = clk_scgt_readl(gate);

		if (set)
			reg |= BIT(gate->bit_idx);
		else
			reg &= ~BIT(gate->bit_idx);
	}

	clk_scgt_writel(gate, reg);

	if (gate->lock)
		spin_unlock_irqrestore(gate->lock, flags);
	else
		__release(gate->lock);
}

static int clk_scgate_enable(struct clk_hw *hw)
{
	clk_scgate_endisable(hw, 1);

	return 0;
}

static void clk_scgate_disable(struct clk_hw *hw)
{
#ifndef CONFIG_XRING_CLK_ALWAYS_ON
	clk_scgate_endisable(hw, 0);
#endif
}

const struct clk_ops clk_scgate_ops = {
	.enable = clk_scgate_enable,
	.disable = clk_scgate_disable,
};

struct clk_hw *devm_xr_clk_hw_scgate(struct device *dev, struct device_node *np,
					const char *name, const char *parent_name,
					const struct clk_hw *parent_hw,
					const struct clk_parent_data *parent_data,
					unsigned long flags, void __iomem *reg,
					u8 bit_idx,	u8 clk_gate_flags,
					spinlock_t *lock)
{
	struct xr_clk_scgate *scgt;
	struct clk_hw *hw;
	struct clk_init_data init = {};
	int ret;

	if (clk_gate_flags & CLK_GATE_HIWORD_MASK) {
		if (bit_idx > 15) {
			clkerr("bit_idx error, must not greater than 15!\n");
			return ERR_PTR(-EINVAL);
		}
	}

	scgt = devm_kzalloc(dev, sizeof(*scgt), GFP_KERNEL);
	if (!scgt) {
		clkerr("failed to allocate memory for scgt:%s\n", name);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.parent_hws = parent_hw ? &parent_hw : NULL;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.parent_data = parent_data;
	init.num_parents = (parent_name || parent_hw || parent_data) ? 1 : 0;
	init.ops = &clk_scgate_ops;
	init.flags = flags;

	scgt->bit_idx = bit_idx;
	scgt->flags = clk_gate_flags;
	scgt->reg = reg;
	scgt->lock = lock;
	scgt->hw.init = &init;

	hw = &scgt->hw;
	ret = clk_hw_register(NULL, hw);
	if (ret)
		hw = ERR_PTR(ret);

	scgt->hw.init = NULL;
	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_scgate);
