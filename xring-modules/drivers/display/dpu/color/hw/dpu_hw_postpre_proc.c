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

#include "dpu_hw_postpre_proc.h"
#include "dpu_hw_postpre_proc_ops.h"

static void dpu_hw_postpre_proc_ops_init(struct dpu_hw_postpre_proc_ops *ops)
{
	ops->feature_enable = dpu_hw_post_preproc_enable;
	ops->set_eotf_oetf = dpu_hw_post_ltm_eotf_oetf_set;
	ops->set_mul_matrix = dpu_hw_post_mul_matrix_set;
	ops->flush = dpu_hw_post_preproc_flush;
}

static int dpu_postpre_proc_cap_init(struct dpu_hw_postpre_proc *hw_post_pre_proc,
		struct dpu_post_pre_proc_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong postpq pre proc hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_post_pre_proc->blk_cap = cap;

	hw_post_pre_proc->hw.iomem_base = base_mem->base;
	hw_post_pre_proc->hw.base_addr = base_mem->addr;
	hw_post_pre_proc->hw.blk_id = cap->base.id;
	hw_post_pre_proc->hw.blk_offset = cap->base.addr;
	hw_post_pre_proc->hw.blk_len = cap->base.len;
	hw_post_pre_proc->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_postpre_proc_init(struct dpu_post_pre_proc_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_postpre_proc *hw_post_pre_proc;
	int ret;

	hw_post_pre_proc = kzalloc(sizeof(*hw_post_pre_proc), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_post_pre_proc))
		return ERR_PTR(-ENOMEM);

	ret = dpu_postpre_proc_cap_init(hw_post_pre_proc, cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init post pre proc hw blk %d\n",
				cap->base.id);
		kfree(hw_post_pre_proc);
		return ERR_PTR(ret);
	}

	dpu_hw_postpre_proc_ops_init(&hw_post_pre_proc->ops);

	return &hw_post_pre_proc->hw;
}

void dpu_hw_postpre_proc_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_postpre_proc(hw));
}
