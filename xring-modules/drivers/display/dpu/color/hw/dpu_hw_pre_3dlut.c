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
#include <linux/kernel.h>
#include "dpu_hw_pre_3dlut.h"
#include "dpu_hw_3dlut_ops.h"

static void dpu_pre_3dlut_ops_init(struct dpu_hw_pre_3dlut_ops *ops)
{
	ops->enable = dpu_hw_3dlut_enable;
	ops->set_3dlut = dpu_hw_pre_3dlut_set;
	ops->flush = dpu_hw_3dlut_flush;
	ops->get_3dlut = dpu_hw_3dlut_get;
}

static int dpu_hw_pre_3dlut_cap_init(struct dpu_hw_pre_3dlut *hw_3dlut,
		struct dpu_pre_lut3d_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong 3dlut hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_3dlut->blk_cap = cap;

	hw_3dlut->hw.iomem_base = base_mem->base;
	hw_3dlut->hw.base_addr = base_mem->addr;
	hw_3dlut->hw.blk_id = cap->base.id;
	hw_3dlut->hw.blk_offset = cap->base.addr;
	hw_3dlut->hw.blk_len = cap->base.len;
	hw_3dlut->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_pre_3dlut_init(struct dpu_pre_lut3d_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_pre_3dlut *hw_3dlut;
	int ret;

	hw_3dlut = kzalloc(sizeof(*hw_3dlut), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_3dlut))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_pre_3dlut_cap_init(hw_3dlut, cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init pre 3dlut hw blk %d\n",
				cap->base.id);
		kfree(hw_3dlut);
		return ERR_PTR(ret);
	}

	dpu_pre_3dlut_ops_init(&hw_3dlut->ops);

	return &hw_3dlut->hw;
}

void dpu_hw_pre_3dlut_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_pre_3dlut(hw));
}
