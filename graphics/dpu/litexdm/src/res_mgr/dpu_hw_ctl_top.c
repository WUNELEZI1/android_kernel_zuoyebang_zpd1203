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

#include "dpu_hw_ctl_top.h"
#include "dpu_hw_cap.h"
#include "dpu_log.h"

static int dpu_hw_ctl_top_res_init(struct dpu_hw_ctl_top *hw_ctl_top,
		struct dpu_ctl_top_cap *ctl_top_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_ctl_top_cap *blk_cap = ctl_top_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong ctl_top hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_ctl_top->blk_cap = blk_cap;

	hw_ctl_top->hw.iomem_base = base_mem->base;
	hw_ctl_top->hw.base_addr = base_mem->addr;
	hw_ctl_top->hw.blk_id = blk_cap->base.id;
	hw_ctl_top->hw.blk_offset = blk_cap->base.addr;
	hw_ctl_top->hw.blk_len = blk_cap->base.len;
	hw_ctl_top->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_ctl_top_ops_init(unsigned long features,
		struct dpu_hw_ctl_top_ops *ops)
{
	ops->wb_input_position_config = dpu_hw_wb_input_position_setup;
}

struct dpu_hw_blk *dpu_hw_ctl_top_init(
		struct dpu_ctl_top_cap *ctl_top_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_ctl_top *hw_ctl_top;
	int ret;

	hw_ctl_top = dpu_mem_alloc(sizeof(*hw_ctl_top));
	if (!hw_ctl_top)
		return NULL;

	ret = dpu_hw_ctl_top_res_init(hw_ctl_top, ctl_top_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init ctl_top hw blk %d\n",
				ctl_top_cap->base.id);
		dpu_mem_free(hw_ctl_top);
		return NULL;
	}

	dpu_ctl_top_ops_init(ctl_top_cap->base.features, &hw_ctl_top->ops);

	return &hw_ctl_top->hw;
}

void dpu_hw_ctl_top_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_ctl_top(hw));
}
