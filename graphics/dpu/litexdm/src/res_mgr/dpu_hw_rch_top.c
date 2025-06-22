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

#include "dpu_hw_rch_top.h"
#include "dpu_log.h"

static void dpu_hw_enable_icg(struct dpu_hw_blk *hw,
		bool enable)
{
	dpu_pr_err("icg enable: %u\n", enable);
}

static int dpu_hw_rch_top_res_init(struct dpu_hw_rch_top *hw_rch_top,
		struct dpu_rch_top_cap *rch_top_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_rch_top_cap *blk_cap = rch_top_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong rch_top hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_rch_top->blk_cap = blk_cap;

	hw_rch_top->hw.iomem_base = base_mem->base;
	hw_rch_top->hw.base_addr = base_mem->addr;
	hw_rch_top->hw.blk_id = blk_cap->base.id;
	hw_rch_top->hw.blk_offset = blk_cap->base.addr;
	hw_rch_top->hw.blk_len = blk_cap->base.len;
	hw_rch_top->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_rch_top_ops_init(unsigned long features,
		struct dpu_hw_rch_top_ops *ops)
{
	ops->enable_icg = dpu_hw_enable_icg;
}

struct dpu_hw_blk *dpu_hw_rch_top_init(
		struct dpu_rch_top_cap *rch_top_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_rch_top *hw_rch_top;
	int ret;

	hw_rch_top = dpu_mem_alloc(sizeof(*hw_rch_top));
	if (!hw_rch_top)
		return NULL;

	ret = dpu_hw_rch_top_res_init(hw_rch_top, rch_top_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init rch_top hw blk %d\n",
				rch_top_cap->base.id);
		dpu_mem_free(hw_rch_top);
		return NULL;
	}

	dpu_rch_top_ops_init(rch_top_cap->base.features, &hw_rch_top->ops);

	return &hw_rch_top->hw;
}

void dpu_hw_rch_top_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_rch_top(hw));
}
