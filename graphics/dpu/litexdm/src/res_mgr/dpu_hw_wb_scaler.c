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

#include "dpu_hw_wb_scaler.h"
#include "dpu_log.h"

static int dpu_hw_wb_scaler_res_init(struct dpu_hw_wb_scaler *hw_wb_scaler,
		struct dpu_wb_scaler_cap *wb_scaler_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_wb_scaler_cap *blk_cap = wb_scaler_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong wb scaler hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_wb_scaler->blk_cap = blk_cap;

	hw_wb_scaler->hw.iomem_base = base_mem->base;
	hw_wb_scaler->hw.base_addr = base_mem->addr;
	hw_wb_scaler->hw.blk_id = blk_cap->base.id;
	hw_wb_scaler->hw.blk_offset = blk_cap->base.addr;
	hw_wb_scaler->hw.blk_len = blk_cap->base.len;
	hw_wb_scaler->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_wb_scaler_ops_init(unsigned long features,
		struct dpu_hw_wb_scaler_ops *ops)
{
	ops->enable = dpu_hw_wb_scaler_enable;
	ops->set = dpu_hw_wb_scaler_set;
}

struct dpu_hw_blk *dpu_hw_wb_scaler_init(
		struct dpu_wb_scaler_cap *wb_scaler_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_wb_scaler *wb_scaler;
	int ret;

	wb_scaler = dpu_mem_alloc(sizeof(*wb_scaler));
	if (!wb_scaler)
		return NULL;

	ret = dpu_hw_wb_scaler_res_init(wb_scaler, wb_scaler_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init wb scaler hw blk %d\n",
				wb_scaler_cap->base.id);
		dpu_mem_free(wb_scaler);
		return NULL;
	}

	dpu_wb_scaler_ops_init(wb_scaler_cap->base.features, &wb_scaler->ops);

	return &wb_scaler->hw;
}

void dpu_hw_wb_scaler_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_wb_scaler(hw));
}
