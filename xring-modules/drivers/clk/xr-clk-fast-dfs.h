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

#ifndef __XR_CLK_FAST_DFS_H__
#define __XR_CLK_FAST_DFS_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

/* fast dfs clk define */
#define PROFILE_CNT		5
#define PLL_CNT                 4
#define GATE_CFG_CNT		2
#define SW_DIV_CFG_CNT		3

#define FAST_DFS_SCGT_GATE	0
#define FAST_DFS_SCGT_NO_GATE	1

struct fast_dfs_hw_cfg {
	/* offset shift length */
	unsigned int sw_cfg[SW_DIV_CFG_CNT];
	/* offset shift length */
	unsigned int sw_state_cfg[SW_DIV_CFG_CNT];
	/* offset shift */
	unsigned int scgt_cfg[GATE_CFG_CNT];
	/* offset shift length */
	unsigned int div_cfg[SW_DIV_CFG_CNT];
	/* offset shift */
	unsigned int div_state_cfg[SW_DIV_CFG_CNT];
};

struct fast_dfs_clock {
	unsigned long profile_freq[PROFILE_CNT];
	unsigned long profile_pll[PROFILE_CNT];
	unsigned int profile_div[PROFILE_CNT];
	unsigned int profile_sw[PROFILE_CNT];
	struct fast_dfs_hw_cfg hw_cfg;
	const char * const *sw_sels;
	unsigned long sw_freq[PLL_CNT];
	unsigned int sw_ids[PLL_CNT];
	unsigned int sw_sel_num;
	unsigned int gate_flag;
};

struct xr_clk_fast_dfs {
	struct clk_hw hw;
	void __iomem *base;
	unsigned long profile_freq[PROFILE_CNT];
	unsigned long profile_pll[PROFILE_CNT];
	unsigned int profile_div[PROFILE_CNT];
	unsigned int profile_sw[PROFILE_CNT];
	struct fast_dfs_hw_cfg hw_cfg;
	const char * const *sw_sels;
	unsigned int sw_sel_num;
	unsigned int sw_ids[PLL_CNT];
	struct clk *sw_pll[PLL_CNT];
	unsigned long sw_freq[PLL_CNT];
	unsigned int gate_flag;
	unsigned int en_count;
	unsigned long rate;
	unsigned int always_on;
	spinlock_t *lock;
};

struct clk_hw *devm_xr_hw_fast_dfs_clock(struct device *dev, void __iomem *base,
		const char *name, struct fast_dfs_clock *fast_dfs_cfg,
		unsigned int always_on, spinlock_t *lock);
#endif /* __XR_CLK_FAST_DFS_H__ */
