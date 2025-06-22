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

#include "dpu_hw_rch.h"
#include "dpu_log.h"

static int32_t dpu_hw_rch_res_init(struct dpu_hw_rch *hw_rch,
		struct dpu_rch_cap *rch_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_rch_cap *blk_cap = rch_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong rch hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_rch->blk_cap = blk_cap;

	hw_rch->hw.iomem_base = base_mem->base;
	hw_rch->hw.base_addr = base_mem->addr;
	hw_rch->hw.blk_id = blk_cap->base.id;
	hw_rch->hw.blk_offset = blk_cap->base.addr;
	hw_rch->hw.blk_len = blk_cap->base.len;
	hw_rch->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_rch_ops_init(unsigned long features,
		struct dpu_hw_rch_ops *ops)
{
	ops->channel_default_refine = dpu_hw_rdma_channel_default_refine;
	ops->layer_config = dpu_hw_rdma_layer_config;
	ops->channel_config = dpu_hw_rdma_channel_config;
	ops->reset = dpu_hw_rdma_reset;
	ops->layer_line_buf_level_config = dpu_hw_rch_line_buf_level_config;
}

struct dpu_hw_blk *dpu_hw_rch_init(struct dpu_rch_cap *rch_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_rch *hw_rch;
	int32_t ret;

	hw_rch = dpu_mem_alloc(sizeof(*hw_rch));
	if (!hw_rch)
		return NULL;

	ret = dpu_hw_rch_res_init(hw_rch, rch_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init rch hw blk %d\n", rch_cap->base.id);
		dpu_mem_free(hw_rch);
		return NULL;
	}

	dpu_rch_ops_init(rch_cap->base.features, &hw_rch->ops);

	hw_rch->ops.channel_default_refine(&hw_rch->channel_param);

	return &hw_rch->hw;
}

void dpu_hw_rch_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_rch(hw));
}