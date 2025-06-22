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

#ifndef __XR_CLK_PMU_H__
#define __XR_CLK_PMU_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

struct xr_clk_pmu {
	struct clk_hw hw;
	unsigned int offset;
	unsigned int bit_mask; /* bit mask */
	unsigned int always_on;
	spinlock_t *lock;
};

struct clk_hw *devm_xr_clk_hw_pmu_clock(struct device *dev, const char *name,
				const char *parent_name, unsigned int flags,
				unsigned int offset, unsigned int bit_shift,
				unsigned int always_on, spinlock_t *lock);

#endif /* __XR_CLK_PMU_H__ */
