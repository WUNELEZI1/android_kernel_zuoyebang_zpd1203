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

#include <linux/kernel.h>
#include <linux/slab.h>

#include "dpu_hw_dsc_wdma.h"

static int dpu_hw_dsc_wdma_res_init(struct dpu_hw_dsc_wdma *hw_dsc_wdma,
		struct dpu_dsc_wdma_cap *dsc_wdma_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_dsc_wdma_cap *blk_cap = dsc_wdma_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong dsc wdma hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_dsc_wdma->blk_cap = blk_cap;

	hw_dsc_wdma->hw.iomem_base = base_mem->base;
	hw_dsc_wdma->hw.base_addr = base_mem->addr;
	hw_dsc_wdma->hw.blk_id = blk_cap->base.id;
	hw_dsc_wdma->hw.blk_offset = blk_cap->base.addr;
	hw_dsc_wdma->hw.blk_len = blk_cap->base.len;
	hw_dsc_wdma->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_dsc_wdma_ops_init(unsigned long features,
		struct dpu_hw_dsc_wdma_ops *ops)
{
	ops->layer_config = dsc_wdma_layer_config;
	ops->axi_cfg = dsc_wdma_axi_cfg;
	ops->mem_lp_ctl = dsc_wdma_mem_lp_ctl;
	ops->secure_mode = dsc_wdma_secure_mode;
	ops->mmu_tbu_config = dpu_mmu_tbu_config;
}

struct dpu_hw_blk *dpu_hw_dsc_wdma_init(
		struct dpu_dsc_wdma_cap *dsc_wdma_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_dsc_wdma *hw_dsc_wdma;
	int ret;

	hw_dsc_wdma = kzalloc(sizeof(*hw_dsc_wdma), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_dsc_wdma))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_dsc_wdma_res_init(hw_dsc_wdma, dsc_wdma_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init dsc wdma hw blk %d\n",
				dsc_wdma_cap->base.id);
		kfree(hw_dsc_wdma);
		return ERR_PTR(ret);
	}

	dpu_dsc_wdma_ops_init(dsc_wdma_cap->base.features, &hw_dsc_wdma->ops);

	return &hw_dsc_wdma->hw;
}

void dpu_hw_dsc_wdma_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_dsc_wdma(hw));
}
