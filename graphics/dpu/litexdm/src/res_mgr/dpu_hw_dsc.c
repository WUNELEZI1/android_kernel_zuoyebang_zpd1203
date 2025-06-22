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

#include "dpu_hw_dsc.h"
#include "dpu_log.h"

static int dpu_hw_dsc_res_init(struct dpu_hw_dsc *hw_dsc,
		struct dpu_dsc_cap *dsc_cap, struct dpu_iomem *base_mem)
{
	const struct dpu_dsc_cap *blk_cap = dsc_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong dsc hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
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

static void dpu_dsc_ops_init(unsigned long features,
		struct dpu_hw_dsc_ops *ops)
{
	ops->enable = dpu_hw_dsc_enable;
	ops->disable = dpu_hw_dsc_disable;
	ops->get_caps = dpu_hw_dsc_get_caps;
}

struct dpu_hw_blk *dpu_hw_dsc_init(
		struct dpu_dsc_cap *dsc_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_dsc *hw_dsc;
	int ret;

	hw_dsc = dpu_mem_alloc(sizeof(*hw_dsc));
	if (!hw_dsc)
		return NULL;

	ret = dpu_hw_dsc_res_init(hw_dsc, dsc_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init dsc hw blk %d\n",
				dsc_cap->base.id);
		dpu_mem_free(hw_dsc);
		return NULL;
	}

	dpu_dsc_ops_init(dsc_cap->base.features, &hw_dsc->ops);

	return &hw_dsc->hw;
}

void dpu_hw_dsc_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_dsc(hw));
}

