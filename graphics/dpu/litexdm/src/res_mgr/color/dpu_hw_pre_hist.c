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

#include "dpu_hw_cap.h"
#include "dpu_log.h"
#include "dpu_hw_pre_hist.h"
#include "dpu_hw_pre_hist_ops.h"

static void dpu_pre_hist_ops_init(struct dpu_hw_pre_hist_ops *ops)
{
	ops->enable = dpu_hw_pre_hist_enable;
	ops->set_hist = dpu_hw_pre_hist_set;
	ops->flush = dpu_hw_pre_hist_flush;
	ops->get_hist = dpu_hw_pre_hist_get;
	ops->enable_update = dpu_hw_pre_hist_enable_update;
}

static int dpu_hw_pre_hist_cap_init(struct dpu_hw_pre_hist *hw_hist,
		struct dpu_pre_hist_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		DPU_COLOR_ERROR("wrong hist hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_hist->blk_cap = cap;

	hw_hist->hw.iomem_base = base_mem->base;
	hw_hist->hw.base_addr = base_mem->addr;
	hw_hist->hw.blk_id = cap->base.id;
	hw_hist->hw.blk_offset = cap->base.addr;
	hw_hist->hw.blk_len = cap->base.len;
	hw_hist->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_pre_hist_init(struct dpu_pre_hist_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_pre_hist *hw_hist;
	int ret;

	hw_hist = dpu_mem_alloc(sizeof(*hw_hist));
	dpu_check_and_return(!hw_hist, NULL, "hw_hist is null\n");

	ret = dpu_hw_pre_hist_cap_init(hw_hist, cap, base_mem);
	if (ret) {
		DPU_COLOR_ERROR("failed to init pre hist hw blk %d\n",
				cap->base.id);
		dpu_mem_free(hw_hist);
		return NULL;
	}

	dpu_pre_hist_ops_init(&hw_hist->ops);

	return &hw_hist->hw;
}

void dpu_hw_pre_hist_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_pre_hist(hw));
}
