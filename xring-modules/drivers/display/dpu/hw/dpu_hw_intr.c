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
#include "dpu_hw_intr.h"

static int dpu_hw_intr_res_init(struct dpu_hw_intr *hw_intr,
		struct dpu_intr_cap *intr_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_intr_cap *blk_cap = intr_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong intr hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_intr->blk_cap = blk_cap;

	hw_intr->hw.iomem_base = base_mem->base;
	hw_intr->hw.base_addr = base_mem->addr;
	hw_intr->hw.blk_id = blk_cap->base.id;
	hw_intr->hw.blk_offset = blk_cap->base.addr;
	hw_intr->hw.blk_len = blk_cap->base.len;
	hw_intr->hw.features = blk_cap->base.features;

	hw_intr->intr_reg_num = DPU_INTR_REG_NUM;
	hw_intr->intr_reg_val =
			kcalloc(hw_intr->intr_reg_num, sizeof(u32), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_intr->intr_reg_val))
		return -ENOMEM;

	return 0;
}

static void dpu_intr_ops_init(unsigned long features,
		struct dpu_hw_intr_ops *ops)
{
	ops->enable = dpu_hw_intr_enable;
	ops->status_get = dpu_hw_intr_status_get;
	ops->status_clear = dpu_hw_intr_status_clear;
	ops->status_dump = dpu_hw_intr_irq_status_dump;
	ops->pslverr_dump = dpu_hw_intr_pslverr_dump;
	ops->status_get_all = dpu_hw_intr_irq_status_get_all;
	ops->status_get_from_cache = dpu_hw_intr_irq_status_get_from_cache;
}

struct dpu_hw_blk *dpu_hw_intr_init(
		struct dpu_intr_cap *intr_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_intr *hw_intr;
	int ret;

	hw_intr = kzalloc(sizeof(*hw_intr), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_intr))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_intr_res_init(hw_intr, intr_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init intr hw blk %d\n",
				intr_cap->base.id);
		kfree(hw_intr);
		return ERR_PTR(ret);
	}

	dpu_intr_ops_init(intr_cap->base.features, &hw_intr->ops);

	return &hw_intr->hw;
}

void dpu_hw_intr_deinit(struct dpu_hw_blk *hw)
{
	struct dpu_hw_intr *hw_intr;

	if (hw) {
		hw_intr = to_dpu_hw_intr(hw);
		kfree(hw_intr->intr_reg_val);
		kfree(hw_intr);
	}
}
