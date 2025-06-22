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

#include "dpu_hw_wb.h"
#include "dpu_log.h"

static int dpu_hw_wb_res_init(struct dpu_hw_wb *hw_wb,
		struct dpu_wb_core_cap *wb_core_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_wb_core_cap *blk_cap = wb_core_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong wb hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_wb->blk_cap = blk_cap;

	hw_wb->hw.iomem_base = base_mem->base;
	hw_wb->hw.base_addr = base_mem->addr;
	hw_wb->hw.blk_id = blk_cap->base.id;
	hw_wb->hw.blk_offset = blk_cap->base.addr;
	hw_wb->hw.blk_len = blk_cap->base.len;
	hw_wb->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_wb_ops_init(unsigned long features,
		struct dpu_hw_wb_ops *ops)
{
	ops->frame_config = dpu_hw_wb_cfg_setup;
	ops->reset = dpu_hw_wb_reset;
}

struct dpu_hw_blk *dpu_hw_wb_init(
		struct dpu_wb_core_cap *wb_core_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_wb *hw_wb;
	int ret;

	hw_wb = dpu_mem_alloc(sizeof(*hw_wb));
	if (!hw_wb)
		return NULL;

	ret = dpu_hw_wb_res_init(hw_wb, wb_core_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init wb hw blk %d\n",
				wb_core_cap->base.id);
		dpu_mem_free(hw_wb);
		return NULL;
	}

	dpu_wb_ops_init(wb_core_cap->base.features, &hw_wb->ops);

	return &hw_wb->hw;
}

void dpu_hw_wb_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_wb(hw));
}
