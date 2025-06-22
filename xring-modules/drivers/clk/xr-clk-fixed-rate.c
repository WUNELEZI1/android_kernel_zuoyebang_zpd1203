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
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "xr-clk-common.h"

#define to_clk_fixrate(_hw) container_of(_hw, struct clk_fixed_rate, hw)

/*
 * The fixed_clock return the rate that registered in probe flow.
 * in adddition, the fixed_clock may be auto-registered by
 * a given dts node who has following properties:
 *  compatible = "fixed-clock";
 *  #clock-cells = <0x00>;
 *  clock-frequency = <100000>;
 */
static unsigned long xr_clk_fixrate_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	return to_clk_fixrate(hw)->fixed_rate;
}

const struct clk_ops clk_fixrate_ops = {
	.recalc_rate = xr_clk_fixrate_recalc_rate,
};

struct clk_hw *devm_xr_clk_hw_fixed_rate(struct device *dev, struct device_node *np,
		const char *name, const char *parent_name, const struct clk_hw *parent_hw,
		unsigned long flags, unsigned long fixed_rate, unsigned long clk_fixed_flags)
{
	struct clk_init_data init = {};
	struct clk_hw *hw;
	struct clk_fixed_rate *fixed;
	int ret;

	fixed = devm_kzalloc(dev, sizeof(*fixed), GFP_KERNEL);
	if (!fixed) {
		clkerr("failed to allocate memory for fixed-clk:%s\n", name);
		return ERR_PTR(-ENOMEM);
	}

	init.flags = flags;
	init.name = name;
	init.parent_hws = parent_hw ? &parent_hw : NULL;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = (parent_hw || parent_name) ? 1 : 0;
	init.ops = &clk_fixrate_ops;

	fixed->hw.init = &init;
	fixed->fixed_rate = fixed_rate;
	fixed->flags = clk_fixed_flags;

	hw = &fixed->hw;
	ret = clk_hw_register(NULL, hw);
	if (ret)
		hw = ERR_PTR(ret);

	fixed->hw.init = NULL;
	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_fixed_rate);
