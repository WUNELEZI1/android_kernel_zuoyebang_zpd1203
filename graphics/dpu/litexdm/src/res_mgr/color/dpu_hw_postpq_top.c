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

#include "dpu_hw_postpq_top.h"
#include "dpu_hw_postpq_top_ops.h"
#include "dpu_hw_cap.h"
#include "dpu_log.h"

static void dpu_hw_postpq_top_ops_init(struct dpu_hw_postpq_top_ops *ops)
{
	ops->init = dpu_hw_postpq_top_pipe_init;
	ops->feature_enable = dpu_hw_postpq_top_enable;
	ops->set_scaler_output_size = dpu_hw_post_scaler_output_size_set;
	ops->set_dither = dpu_hw_post_dither_set;
	ops->set_r2y = dpu_hw_post_rgb2yuv_set;
	ops->set_y2r = dpu_hw_post_yuv2rgb_set;
	ops->set_csc_matrix = dpu_hw_csc_matrix_set;
	ops->get_dither_loc = dpu_hw_post_dither_proc_loc_get;
	ops->get_loc = dpu_hw_post_proc_loc_get;
	ops->flush = dpu_hw_postpq_top_flush;
}

static int dpu_post_pq_top_cap_init(struct dpu_hw_postpq_top *hw_pospq_top,
		struct dpu_post_pq_top_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong postpq top hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_pospq_top->blk_cap = cap;

	hw_pospq_top->hw.iomem_base = base_mem->base;
	hw_pospq_top->hw.base_addr = base_mem->addr;
	hw_pospq_top->hw.blk_id = cap->base.id;
	hw_pospq_top->hw.blk_offset = cap->base.addr;
	hw_pospq_top->hw.blk_len = cap->base.len;
	hw_pospq_top->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_postpq_top_init(struct dpu_post_pq_top_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_postpq_top *hw_pospq_top;
	int ret;

	hw_pospq_top = dpu_mem_alloc(sizeof(*hw_pospq_top));
	dpu_check_and_return(!hw_pospq_top, NULL, "hw_pospq_top is null\n");

	ret = dpu_post_pq_top_cap_init(hw_pospq_top, cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init post pq top hw blk %d\n",
				cap->base.id);
		dpu_mem_free(hw_pospq_top);
		return NULL;
	}

	dpu_hw_postpq_top_ops_init(&hw_pospq_top->ops);

	return &hw_pospq_top->hw;
}

void dpu_hw_postpq_top_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_postpq_top(hw));
}
