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
#include "dpu_hw_dvs.h"
#include "dpu_hw_dvs_ops.h"

#define DVS_REG_ADDRESS                                 0xEB016000
#define DVS_REG_SIZE                                    0x1000

static struct dpu_hw_dvs_ops dpu_hw_dvs_ops = {
	.sw_to_hw_vote = dpu_hw_dvs_sw_to_hw_vote,
	.hw_to_sw_vote = dpu_hw_dvs_hw_to_sw_vote,
	.dvs_state_dump = dpu_hw_dvs_state_dump,
};

struct dpu_hw_dvs *dpu_hw_dvs_init(void)
{
	struct dpu_hw_dvs *hw_dvs;

	hw_dvs = kzalloc(sizeof(*hw_dvs), GFP_KERNEL);
	if (!hw_dvs)
		return ERR_PTR(-ENOMEM);

	hw_dvs->hw.iomem_base = ioremap(DVS_REG_ADDRESS, DVS_REG_SIZE);
	hw_dvs->hw.base_addr = DVS_REG_ADDRESS;
	hw_dvs->ops = &dpu_hw_dvs_ops;

	return hw_dvs;
}

void dpu_hw_dvs_deinit(struct dpu_hw_dvs *hw_dvs)
{
	if (!hw_dvs) {
		PERF_ERROR("invalid parameter\n");
		return;
	}
	iounmap(hw_dvs->hw.iomem_base);
	kfree(hw_dvs);
}
