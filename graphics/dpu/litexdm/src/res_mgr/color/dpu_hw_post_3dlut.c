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
#include "dpu_hw_post_3dlut.h"
#include "dpu_hw_3dlut_ops.h"

static void dpu_post_3dlut_ops_init(struct dpu_hw_post_3dlut_ops *ops)
{
	ops->enable = dpu_hw_3dlut_enable;
	ops->set_3dlut = dpu_hw_post_3dlut_set;
	ops->flush = dpu_hw_3dlut_flush;
	ops->get_3dlut = dpu_hw_3dlut_get;
}

static int dpu_hw_post_3dlut_cap_init(struct dpu_hw_post_3dlut *hw_3dlut,
		struct dpu_post_3dlut_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong 3dlut hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_3dlut->blk_cap = cap;

	hw_3dlut->hw.iomem_base = base_mem->base;
	hw_3dlut->hw.base_addr = base_mem->addr;
	hw_3dlut->hw.blk_id = cap->base.id;
	hw_3dlut->hw.blk_offset = cap->base.addr;
	hw_3dlut->hw.blk_len = cap->base.len;
	hw_3dlut->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_post_3dlut_init(struct dpu_post_3dlut_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_post_3dlut *hw_3dlut;
	int ret;

	hw_3dlut = dpu_mem_alloc(sizeof(*hw_3dlut));
	dpu_check_and_return(!hw_3dlut, NULL, "hw_scaler is null\n");

	ret = dpu_hw_post_3dlut_cap_init(hw_3dlut, cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init post 3dlut hw blk %d\n",
				cap->base.id);
		dpu_mem_free(hw_3dlut);
		return NULL;
	}

	dpu_post_3dlut_ops_init(&hw_3dlut->ops);

	return &hw_3dlut->hw;
}

void dpu_hw_post_3dlut_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_post_3dlut(hw));
}
