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
#include "dpu_hw_top.h"

static void dpu_hw_enable_vsync_update(struct dpu_hw_blk *hw,
		bool enable)
{
	DPU_ERROR("vsync update enable: %u\n", enable);
}

static int dpu_hw_top_res_init(struct dpu_hw_top *hw_dpu_top,
		struct dpu_top_cap *dpu_top_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_top_cap *blk_cap = dpu_top_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong dpu_top hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_dpu_top->blk_cap = blk_cap;

	hw_dpu_top->hw.iomem_base = base_mem->base;
	hw_dpu_top->hw.base_addr = base_mem->addr;
	hw_dpu_top->hw.blk_id = blk_cap->base.id;
	hw_dpu_top->hw.blk_offset = blk_cap->base.addr;
	hw_dpu_top->hw.blk_len = blk_cap->base.len;
	hw_dpu_top->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_top_ops_init(unsigned long features,
		struct dpu_hw_top_ops *ops)
{
	ops->enable_vsync_update = dpu_hw_enable_vsync_update;
}

struct dpu_hw_blk *dpu_hw_dpu_top_init(
		struct dpu_top_cap *dpu_top_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_top *hw_dpu_top;
	int ret;

	hw_dpu_top = kzalloc(sizeof(*hw_dpu_top), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_dpu_top))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_top_res_init(hw_dpu_top, dpu_top_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init dpu_top hw blk %d\n",
				dpu_top_cap->base.id);
		kfree(hw_dpu_top);
		return ERR_PTR(ret);
	}

	dpu_top_ops_init(dpu_top_cap->base.features, &hw_dpu_top->ops);

	return &hw_dpu_top->hw;
}

void dpu_hw_dpu_top_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_top(hw));
}
