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
#include "dpu_hw_post_acad.h"
#include "dpu_hw_post_acad_ops.h"

static void dpu_post_acad_ops_init(struct dpu_hw_post_acad_ops *ops)
{
	ops->enable = dpu_hw_acad_enable;
	ops->set_acad = dpu_hw_acad_set;
	ops->flush = dpu_hw_acad_flush;
}

static int dpu_hw_post_acad_cap_init(struct dpu_hw_post_acad *hw_acad,
		struct dpu_acad_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong ACAD hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_acad->blk_cap = cap;

	hw_acad->hw.iomem_base = base_mem->base;
	hw_acad->hw.base_addr = base_mem->addr;
	hw_acad->hw.blk_id = cap->base.id;
	hw_acad->hw.blk_offset = cap->base.addr;
	hw_acad->hw.blk_len = cap->base.len;
	hw_acad->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_post_acad_init(struct dpu_acad_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_post_acad *hw_acad;
	int ret;

	hw_acad = kzalloc(sizeof(*hw_acad), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_acad))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_post_acad_cap_init(hw_acad, cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init post acad hw blk %d\n",
				cap->base.id);
		kfree(hw_acad);
		return ERR_PTR(ret);
	}

	dpu_post_acad_ops_init(&hw_acad->ops);

	return &hw_acad->hw;
}

void dpu_hw_post_acad_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_post_acad(hw));
}

