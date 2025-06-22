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

#ifndef __XR_CLK_GATE_H__
#define __XR_CLK_GATE_H__

#include <linux/clk-provider.h>
#include <linux/spinlock.h>

struct xr_clk_gate {
	struct clk_hw hw;
	void __iomem *enable;
	unsigned int ebits; /* bit mask */
	struct clk_hw **friend_hw;
	unsigned int always_on;
	unsigned int sync_time;
	unsigned int flags;
	unsigned int dvs_flag;
	unsigned int safe_id;
	unsigned int vote_id;
	spinlock_t *lock;
};

struct clk_hw *devm_xr_clk_hw_gate(struct device *dev, const char *name,
				   const char *parent_name,
				   struct clk_hw **friend_hw,
				   unsigned long flags, void __iomem *reg,
				   unsigned int offset, unsigned int bit_mask,
				   unsigned int always_on,
				   unsigned int sync_time, spinlock_t *lock,
				   unsigned int gate_flags, unsigned int safe_id,
				   unsigned int vote_id, unsigned int dvs_flag);

#endif /* __XR_CLK_GATE_H__ */
