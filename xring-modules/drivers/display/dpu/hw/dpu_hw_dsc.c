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

#include <linux/kernel.h>
#include <linux/slab.h>
#include "dpu_hw_dsc.h"

static int dpu_hw_dsc_res_init(struct dpu_hw_dsc *hw_dsc,
		struct dpu_dsc_cap *dsc_cap, struct dpu_iomem *base_mem)
{
	const struct dpu_dsc_cap *blk_cap = dsc_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong dsc hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_dsc->blk_cap = blk_cap;

	hw_dsc->hw.iomem_base = base_mem->base;
	hw_dsc->hw.base_addr = base_mem->addr;
	hw_dsc->hw.blk_id = blk_cap->base.id;
	hw_dsc->hw.blk_offset = blk_cap->base.addr;
	hw_dsc->hw.blk_len = blk_cap->base.len;
	hw_dsc->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_dsc_ops_init(unsigned long features, struct dpu_hw_dsc_ops *ops)
{
	ops->enable = dpu_hw_dsc_enable;
	ops->disable = dpu_hw_dsc_disable;
	ops->get_caps = dpu_hw_dsc_get_caps;
	ops->height_set = dpu_hw_dsc_height_set;
}

struct dpu_hw_blk *dpu_hw_dsc_init(struct dpu_dsc_cap *dsc_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_dsc *hw_dsc;
	int ret;

	hw_dsc = kzalloc(sizeof(*hw_dsc), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_dsc))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_dsc_res_init(hw_dsc, dsc_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init dsc hw blk %d\n",
				dsc_cap->base.id);
		kfree(hw_dsc);
		return ERR_PTR(ret);
	}

	dpu_dsc_ops_init(dsc_cap->base.features, &hw_dsc->ops);

	return &hw_dsc->hw;
}

void dpu_hw_dsc_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_dsc(hw));
}
