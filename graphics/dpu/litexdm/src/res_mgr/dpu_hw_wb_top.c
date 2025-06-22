// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#include "dpu_hw_wb_top.h"
#include "dpu_log.h"

static void dpu_hw_init(struct dpu_hw_blk *hw)
{
	dpu_pr_err("init wb top\n");
}

static int dpu_hw_wb_top_res_init(struct dpu_hw_wb_top *hw_wb_top,
		struct dpu_wb_top_cap *wb_top_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_wb_top_cap *blk_cap = wb_top_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong wb_top hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_wb_top->blk_cap = blk_cap;

	hw_wb_top->hw.iomem_base = base_mem->base;
	hw_wb_top->hw.base_addr = base_mem->addr;
	hw_wb_top->hw.blk_id = blk_cap->base.id;
	hw_wb_top->hw.blk_offset = blk_cap->base.addr;
	hw_wb_top->hw.blk_len = blk_cap->base.len;
	hw_wb_top->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_wb_top_ops_init(unsigned long features,
		struct dpu_hw_wb_top_ops *ops)
{
	ops->init = dpu_hw_init;
}

struct dpu_hw_blk *dpu_hw_wb_top_init(
		struct dpu_wb_top_cap *wb_top_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_wb_top *hw_wb_top;
	int ret;

	hw_wb_top = dpu_mem_alloc(sizeof(*hw_wb_top));
	if (!hw_wb_top)
		return NULL;

	ret = dpu_hw_wb_top_res_init(hw_wb_top, wb_top_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init wb_top hw blk %d\n",
				wb_top_cap->base.id);
		dpu_mem_free(hw_wb_top);
		return NULL;
	}

	dpu_wb_top_ops_init(wb_top_cap->base.features, &hw_wb_top->ops);

	return &hw_wb_top->hw;
}

void dpu_hw_wb_top_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_wb_top(hw));
}
