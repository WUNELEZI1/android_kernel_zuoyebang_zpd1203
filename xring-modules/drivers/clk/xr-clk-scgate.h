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

#ifndef __XR_CLK_SCGATE_H__
#define __XR_CLK_SCGATE_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

struct xr_clk_scgate {
	struct clk_hw hw;
	void __iomem	*reg;
	u8		bit_idx;
	u8		flags;
	spinlock_t	*lock;
};

struct clk_hw *devm_xr_clk_hw_scgate(struct device *dev, struct device_node *np,
					const char *name, const char *parent_name,
					const struct clk_hw *parent_hw,
					const struct clk_parent_data *parent_data,
					unsigned long flags, void __iomem *reg,
					u8 bit_idx, u8 clk_gate_flags,
					spinlock_t *lock);

#endif /* __XR_CLK_SCGATE_H__ */
