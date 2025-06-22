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

#include <dt-bindings/xring/xr-o1-clock.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/spmi.h>
#include <linux/spinlock.h>
#include <soc/xring/xr-pmic-spmi.h>

#include "xr-clk-pmu.h"
#include "clk.h"

#define to_clk_pmu(_hw) container_of(_hw, struct xr_clk_pmu, hw)

static int clk_pmu_enable(struct clk_hw *hw)
{
	struct xr_clk_pmu *clk_pmu = to_clk_pmu(hw);
	unsigned int buf;
	int ret = 0;

	if (is_fpga())
		return ret;

	ret = xr_pmic_reg_read(clk_pmu->offset, &buf);
	if (ret) {
		clkerr("pmu clk enable failed cause read register fail,ret=%d\n", ret);
		return ret;
	}

	buf |= clk_pmu->bit_mask;
	ret = xr_pmic_reg_write(clk_pmu->offset, buf);
	if (ret) {
		clkerr("pmu clk enable failed cause write register fail,ret=%d\n", ret);
		return ret;
	}

	return ret;
}

static void clk_pmu_disable(struct clk_hw *hw)
{
#ifndef CONFIG_XRING_CLK_ALWAYS_ON
	struct xr_clk_pmu *clk_pmu = to_clk_pmu(hw);
	unsigned int buf;
	unsigned long flags = 0;
	int ret;

	if (is_fpga())
		return;

	if (clk_pmu->always_on)
		return;

	if (clk_pmu->lock)
		spin_lock_irqsave(clk_pmu->lock, flags);
	else
		__acquire(clk_pmu->lock);

	ret = xr_pmic_reg_read(clk_pmu->offset, &buf);
	if (ret) {
		clkerr("pmu clk:%s disable failed cause read register fail,ret=%d\n",
				clk_hw_get_name(hw), ret);
		goto err;
	}

	buf &= ~clk_pmu->bit_mask;
	ret = xr_pmic_reg_write(clk_pmu->offset, buf);
	if (ret) {
		clkerr("pmu clk:%s disable failed cause write register fail,ret=%d\n",
				clk_hw_get_name(hw), ret);
		goto err;
	}

err:
	if (clk_pmu->lock)
		spin_unlock_irqrestore(clk_pmu->lock, flags);
	else
		__release(clk_pmu->lock);

#endif
}

static int clk_pmu_is_enabled(struct clk_hw *hw)
{
	struct xr_clk_pmu *clk_pmu = to_clk_pmu(hw);
	unsigned int buf;
	int ret;

	if (is_fpga())
		return 1;

	ret = xr_pmic_reg_read(clk_pmu->offset, &buf);
	if (ret) {
		clkerr("get pmu clk: %s enable status failed,ret=%d\n",
				clk_hw_get_name(hw), ret);
		return 0;
	}

	if (buf & clk_pmu->bit_mask)
		return 1;

	return 0;
}

const struct clk_ops clk_pmu_ops = {
	.enable			= clk_pmu_enable,
	.disable		= clk_pmu_disable,
	.is_enabled		= clk_pmu_is_enabled,
};

struct clk_hw *devm_xr_clk_hw_pmu_clock(struct device *dev, const char *name,
				const char *parent_name, unsigned int flags,
				unsigned int offset, unsigned int bit_shift,
				unsigned int always_on, spinlock_t *lock)
{
	struct xr_clk_pmu *clk_pmu;
	struct clk_hw *hw;
	struct clk_init_data init;
	int ret;

	clk_pmu = devm_kzalloc(dev, sizeof(*clk_pmu), GFP_KERNEL);
	if (!clk_pmu)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.flags = flags;
	init.parent_names = &parent_name;
	init.num_parents = 1;
	init.ops = &clk_pmu_ops;

	clk_pmu->always_on = always_on;
	clk_pmu->offset = offset;
	clk_pmu->bit_mask = BIT(bit_shift);
	clk_pmu->lock = lock;

	clk_pmu->hw.init = &init;
	hw = &clk_pmu->hw;

	ret = clk_hw_register(NULL, hw);
	if (ret) {
		clkerr("failed to register pmu clock %s %d\n",
			name, ret);
		return ERR_PTR(ret);
	}
	/* init is local variable, need set NULL before func */
	clk_pmu->hw.init = NULL;
	return hw;
}
EXPORT_SYMBOL_GPL(devm_xr_clk_hw_pmu_clock);
