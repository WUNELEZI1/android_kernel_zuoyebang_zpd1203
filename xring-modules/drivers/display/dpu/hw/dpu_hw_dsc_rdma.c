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

#include "dpu_hw_dsc_rdma.h"

static int dpu_hw_dsc_rdma_res_init(struct dpu_hw_dsc_rdma *hw_dsc_rdma,
		struct dpu_dsc_rdma_cap *dsc_rdma_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_dsc_rdma_cap *blk_cap = dsc_rdma_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong dsc rdma hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_dsc_rdma->blk_cap = blk_cap;

	hw_dsc_rdma->hw.iomem_base = base_mem->base;
	hw_dsc_rdma->hw.base_addr = base_mem->addr;
	hw_dsc_rdma->hw.blk_id = blk_cap->base.id;
	hw_dsc_rdma->hw.blk_offset = blk_cap->base.addr;
	hw_dsc_rdma->hw.blk_len = blk_cap->base.len;
	hw_dsc_rdma->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_dsc_rdma_ops_init(unsigned long features,
		struct dpu_hw_dsc_rdma_ops *ops)
{
	ops->layer_config = dsc_rdma_layer_config;
	ops->axi_cfg = dsc_rdma_axi_cfg;
	ops->secure_mode = dsc_rdma_secure_mode;
	ops->mmu_tbu_config = dpu_mmu_tbu_config;
}

struct dpu_hw_blk *dpu_hw_dsc_rdma_init(
		struct dpu_dsc_rdma_cap *dsc_rdma_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_dsc_rdma *hw_dsc_rdma;
	int ret;

	hw_dsc_rdma = kzalloc(sizeof(*hw_dsc_rdma), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_dsc_rdma))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_dsc_rdma_res_init(hw_dsc_rdma, dsc_rdma_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init dsc rdma hw blk %d\n",
				dsc_rdma_cap->base.id);
		kfree(hw_dsc_rdma);
		return ERR_PTR(ret);
	}

	dpu_dsc_rdma_ops_init(dsc_rdma_cap->base.features, &hw_dsc_rdma->ops);

	return &hw_dsc_rdma->hw;
}

void dpu_hw_dsc_rdma_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_dsc_rdma(hw));
}
