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

#include <linux/slab.h>
#include "dp_hw_msgbus.h"

#define CR_APB_ADDRESS                                 0xE1800000
#define CR_APB_SIZE                                    0x40000

static struct dpu_hw_blk cr_hw = {0};

static void dp_hw_msgbus_set_vswing_preemp(
		struct dpu_hw_blk *msgbus_hw,
		u32 (*signal_table)[4][3],
		u8 swing_level[DP_LANE_COUNT_MAX],
		u8 preemp_level[DP_LANE_COUNT_MAX])
{
	_dp_hw_msgbus_set_vswing_preemp(
			msgbus_hw, &cr_hw, signal_table, swing_level, preemp_level);
}

void dp_hw_msgbus_set_vboost(u32 vboost)
{
	_dp_hw_msgbus_set_vboost(&cr_hw, vboost);
}

void dp_hw_msgbus_set_ssc_val(u32 ssc_step_size, u32 ssc_peak)
{
	_dp_hw_msgbus_set_ssc_val(&cr_hw, ssc_step_size, ssc_peak);
}

static struct dp_hw_msgbus_ops dp_hw_msgbus_ops = {
	.set_vswing_preemp        = dp_hw_msgbus_set_vswing_preemp,
	.reserve_usb_tx_lane      = dp_hw_msgbus_reserve_usb_tx_lane,
};

struct dp_hw_msgbus *dp_hw_msgbus_init(struct dp_blk_cap *cap)
{
	struct dp_hw_msgbus *hw_msgbus;

	hw_msgbus = kzalloc(sizeof(*hw_msgbus), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_msgbus))
		return ERR_PTR(-ENOMEM);

	hw_msgbus->hw.iomem_base = cap->vaddr;
	hw_msgbus->hw.base_addr = cap->paddr;
	hw_msgbus->hw.blk_len = cap->len;
	hw_msgbus->ops = &dp_hw_msgbus_ops;

	cr_hw.iomem_base = ioremap(CR_APB_ADDRESS, CR_APB_SIZE);
	cr_hw.base_addr = CR_APB_ADDRESS;
	cr_hw.blk_len = CR_APB_SIZE;

	return hw_msgbus;
}

void dp_hw_msgbus_deinit(struct dp_hw_msgbus *hw_msgbus)
{
	iounmap(cr_hw.iomem_base);
	kfree(hw_msgbus);
}
