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
#include "dpu_hw_glb.h"
#include "dpu_hw_glb_ops.h"

static int dpu_hw_glb_res_init(struct dpu_hw_glb *hw_glb,
		struct dpu_glb_cap *glb_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_glb_cap *blk_cap = glb_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong glb hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_glb->blk_cap = blk_cap;

	hw_glb->hw.iomem_base = base_mem->base;
	hw_glb->hw.base_addr = base_mem->addr;
	hw_glb->hw.blk_id = blk_cap->base.id;
	hw_glb->hw.blk_offset = blk_cap->base.addr;
	hw_glb->hw.blk_len = blk_cap->base.len;
	hw_glb->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_glb_ops_init(unsigned long features,
		struct dpu_hw_glb_ops *ops)
{
	ops->hdc_config = dpu_hw_glb_hdc_config;
	ops->hdc_enable = dpu_hw_glb_hdc_enable;
	ops->hdc_disable = dpu_hw_glb_hdc_disable;
	ops->hdc_get_hw_vote_val = dpu_hw_glb_hdc_get_hw_vote_val;
	ops->hdc_update_avs_codes = dpu_hw_glb_hdc_update_avs_codes;
	ops->hdc_dump = dpu_hw_glb_hdc_dump;
}

struct dpu_hw_blk *dpu_hw_glb_init(struct dpu_glb_cap *glb_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_glb *hw_glb;
	int ret;

	hw_glb = kzalloc(sizeof(*hw_glb), GFP_KERNEL);
	if (!hw_glb)
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_glb_res_init(hw_glb, glb_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init glb hw blk %d\n",
				glb_cap->base.id);
		kfree(hw_glb);
		return ERR_PTR(ret);
	}

	dpu_glb_ops_init(glb_cap->base.features, &hw_glb->ops);

	return &hw_glb->hw;
}

void dpu_hw_glb_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_glb(hw));
}
