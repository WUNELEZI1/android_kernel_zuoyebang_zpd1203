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

#ifndef __XR_CLK_MUX_H__
#define __XR_CLK_MUX_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

struct xr_clk_mux {
	struct clk_hw	hw;
	void __iomem	*reg;
	void __iomem	*ack_reg;
	u32		mask;
	u8		shift;
	u8		ack_shift;
	u8		ack_width;
	u8		saved_parent;
	u16		flags;
	spinlock_t	*lock;
};

struct clk_hw *devm_xr_clk_hw_register_mux(struct device *dev,
		const char *name, u8 num_parents,
		const char * const *parent_names,
		unsigned long flags, void __iomem *reg, u8 shift, u32 mask,
		u16 clk_mux_flags, void __iomem *ack_reg,
		u8 ack_shift, u8 ack_width, spinlock_t *lock);

#endif /* __XR_CLK_MUX_H__ */
