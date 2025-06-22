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
#include "dpu_hw_post_gamma.h"
#include "dpu_hw_post_gamma_ops.h"

static void dpu_post_gamma_ops_init(struct dpu_hw_post_gamma_ops *ops)
{
	ops->enable = dpu_hw_post_gamma_enable;
	ops->set_gamma = dpu_hw_post_gamma_set;
	ops->flush = dpu_hw_post_gamma_flush;
}

static int dpu_hw_post_gamma_cap_init(struct dpu_hw_post_gamma *hw_gamma,
		struct dpu_post_gamma_cap *cap,
		struct dpu_iomem *base_mem)
{
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + cap->base.addr + cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong post gma hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_gamma->blk_cap = cap;

	hw_gamma->hw.iomem_base = base_mem->base;
	hw_gamma->hw.base_addr = base_mem->addr;
	hw_gamma->hw.blk_id = cap->base.id;
	hw_gamma->hw.blk_offset = cap->base.addr;
	hw_gamma->hw.blk_len = cap->base.len;
	hw_gamma->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_post_gamma_init(struct dpu_post_gamma_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_post_gamma *hw_gamma;
	int ret;

	hw_gamma = dpu_mem_alloc(sizeof(*hw_gamma));
	dpu_check_and_return(!hw_gamma, NULL, "hw_gamma is null\n");

	ret = dpu_hw_post_gamma_cap_init(hw_gamma, cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init post gamma hw blk %d\n",
				cap->base.id);
		dpu_mem_free(hw_gamma);
		return NULL;
	}

	dpu_post_gamma_ops_init(&hw_gamma->ops);

	return &hw_gamma->hw;
}

void dpu_hw_post_gamma_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_post_gamma(hw));
}
