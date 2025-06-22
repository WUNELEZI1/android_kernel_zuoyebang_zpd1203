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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#include <linux/slab.h>
#include "dpu_hw_wb.h"

static int dpu_hw_wb_res_init(struct dpu_hw_wb *hw_wb,
		struct dpu_wb_core_cap *wb_core_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_wb_core_cap *blk_cap = wb_core_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong wb hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_wb->blk_cap = blk_cap;

	hw_wb->hw.iomem_base = base_mem->base;
	hw_wb->hw.base_addr = base_mem->addr;
	hw_wb->hw.blk_id = blk_cap->base.id;
	hw_wb->hw.blk_offset = blk_cap->base.addr;
	hw_wb->hw.blk_len = blk_cap->base.len;
	hw_wb->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_wb_ops_init(unsigned long features,
		struct dpu_hw_wb_ops *ops)
{
	ops->frame_config = dpu_hw_wb_cfg_setup;
	ops->reset = dpu_hw_wb_reset;
	ops->mmu_tbu_config = dpu_mmu_tbu_config;
	ops->status_dump = dpu_hw_wb_core_status_dump;
	ops->debug_enable = dpu_hw_wb_core_ro_trig_enable;
}

struct dpu_hw_blk *dpu_hw_wb_init(
		struct dpu_wb_core_cap *wb_core_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_wb *hw_wb;
	int ret;

	hw_wb = kzalloc(sizeof(*hw_wb), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_wb))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_wb_res_init(hw_wb, wb_core_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init wb hw blk %d\n",
				wb_core_cap->base.id);
		kfree(hw_wb);
		return ERR_PTR(ret);
	}

	dpu_wb_ops_init(wb_core_cap->base.features, &hw_wb->ops);

	return &hw_wb->hw;
}

void dpu_hw_wb_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_wb(hw));
}
