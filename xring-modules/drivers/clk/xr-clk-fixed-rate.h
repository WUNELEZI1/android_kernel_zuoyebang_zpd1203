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

#ifndef __XR_CLK_FIXED_RATE_H__
#define __XR_CLK_FIXED_RATE_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

struct clk_hw *devm_xr_clk_hw_fixed_rate(struct device *dev,
		struct device_node *np, const char *name,
		const char *parent_name, const struct clk_hw *parent_hw,
		unsigned long flags, unsigned long fixed_rate,
		unsigned long clk_fixed_flags);

#endif /* __XR_CLK_FIXED_RATE_H__ */
