/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef __XR_CLK_PLL_H__
#define __XR_CLK_PLL_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

#define PLL_REG_NUM 2 /* offset, bit */

#define PLL_FLAGS_FIXED_RATE    BIT(0)    /* pll fixed rate */
#define PLL_FLAGS_DYNAMIC_RATE  BIT(1)    /* pll support set rate by calculate */
#define PLL_FLAGS_SOFT_CTRL     BIT(2)    /* pll ccontroled by software only */

enum {
	XR_PPLL0 = 0,
	XR_PPLL1,
	XR_PPLL2,
	XR_PPLL2_DPU,
	XR_PPLL3,
	XR_PCIE_PLL,
	XR_DP_PLL,
	XR_DPU_PLL,
	XR_PPLLMAX,
};

struct clk_pll_rate_table {
	unsigned long	output_rate; /* Hz */
	unsigned int	refdiv;
	unsigned int	fbdiv;
	unsigned int	frac;
	unsigned int	postdiv1;
	unsigned int	postdiv2;
};

struct clk_pll_cfg_table {
	unsigned int pll_id;
	unsigned int lock_ctrl[PLL_REG_NUM];
	unsigned int vote_en[PLL_REG_NUM];
	void __iomem *base;
};

struct xr_clk_pll {
	struct clk_hw hw;
	const struct clk_pll_cfg_table *cfg;
	const struct clk_pll_rate_table *rate_table;
	unsigned int num_entry;
	unsigned char safe_id;
	unsigned char flags;
};

int wait_ap_pll_lock(const void __iomem *lock_base,
		     int pll, unsigned int lock_bit);
struct clk_hw *devm_xr_clk_hw_pll(struct device *dev, const char *name,
					const char *parent_name,
					const struct clk_pll_cfg_table *cfg,
					const struct clk_pll_rate_table *rate_table,
					unsigned int num_entry,
					unsigned char safe_id,
					unsigned char pll_flags);

#endif /* __XR_CLK_PLL_H__ */
