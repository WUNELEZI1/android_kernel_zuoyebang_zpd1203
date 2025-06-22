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

#include <linux/slab.h>
#include "dpu_hw_post_scaler.h"
#include "dpu_hw_post_scaler_ops.h"

static inline void dpu_post_scaler_ops_init(struct dpu_hw_post_scaler_ops *ops)
{
	ops->enable = dpu_hw_post_scaler_enable;
	ops->dump_enable = dpu_hw_post_scaler_dump_enable;
	ops->set = dpu_hw_post_scaler_set;
	ops->flush = dpu_hw_post_scaler_flush;
	ops->get = dpu_hw_post_scaler_get;
	ops->status_dump = dpu_hw_post_scaler_status_dump;
}

static int dpu_post_scaler_cap_init(struct dpu_hw_post_scaler *hw_scaler,
		struct dpu_post_scaler_cap *cap, struct dpu_iomem *base_mem)
{
	if ((cap->base.addr + cap->base.len) > base_mem->len) {
		DPU_ERROR("wrong scaler hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_scaler->blk_cap = cap;

	hw_scaler->hw.iomem_base = base_mem->base;
	hw_scaler->hw.base_addr = base_mem->addr;
	hw_scaler->hw.blk_id = cap->base.id;
	hw_scaler->hw.blk_offset = cap->base.addr;
	hw_scaler->hw.blk_len = cap->base.len;
	hw_scaler->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_post_scaler_init(struct dpu_post_scaler_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_post_scaler *hw_scaler;
	int ret;

	hw_scaler = kzalloc(sizeof(struct dpu_hw_post_scaler), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_scaler))
		return ERR_PTR(-ENOMEM);

	ret = dpu_post_scaler_cap_init(hw_scaler, cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init post scaler hw blk %d\n",
				cap->base.id);
		kfree(hw_scaler);
		return ERR_PTR(ret);
	}

	dpu_post_scaler_ops_init(&hw_scaler->ops);

	return &hw_scaler->hw;
}

void dpu_hw_post_scaler_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_post_scaler(hw));
}
