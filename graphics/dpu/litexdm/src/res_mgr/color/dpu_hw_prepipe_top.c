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

#include "dpu_hw_prepipe_top.h"
#include "dpu_hw_prepipe_top_ops.h"
#include "dpu_hw_cap.h"
#include "dpu_log.h"

static inline void dpu_prepipe_top_ops_init(struct dpu_hw_prepipe_top_ops *ops)
{
	ops->feature_enable = dpu_hw_prepipe_top_feature_enable;
	ops->set_r2y = dpu_hw_prepipe_rgb2yuv_set;
	ops->set_y2r = dpu_hw_prepipe_yuv2rgb_set;
	ops->set_nonpre_alpha = dpu_hw_prepipe_non_prealpha_set;
	ops->flush = dpu_hw_prepipe_top_flush;
}

static int dpu_prepipe_top_cap_init(struct dpu_hw_prepipe_top *prepipe_top,
		struct dpu_prepipe_top_cap *cap, struct dpu_iomem *base_mem)
{
	if ((cap->base.addr + cap->base.len) > base_mem->len) {
		dpu_pr_err("wrong prepipe_top hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	prepipe_top->blk_cap = cap;

	prepipe_top->hw.iomem_base = base_mem->base;
	prepipe_top->hw.base_addr = base_mem->addr;
	prepipe_top->hw.blk_id = cap->base.id;
	prepipe_top->hw.blk_offset = cap->base.addr;
	prepipe_top->hw.blk_len = cap->base.len;
	prepipe_top->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_prepipe_top_init(struct dpu_prepipe_top_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_prepipe_top *prepipe_top;
	int ret;

	prepipe_top = dpu_mem_alloc(sizeof(struct dpu_hw_prepipe_top));
	dpu_check_and_return(!prepipe_top, NULL, "prepipe_top is null\n");

	ret = dpu_prepipe_top_cap_init(prepipe_top, cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init prepipe_top hw blk %d\n",
				cap->base.id);
		dpu_mem_free(prepipe_top);
		return NULL;
	}

	dpu_prepipe_top_ops_init(&prepipe_top->ops);

	return &prepipe_top->hw;
}

void dpu_hw_prepipe_top_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_prepipe_top(hw));
}
