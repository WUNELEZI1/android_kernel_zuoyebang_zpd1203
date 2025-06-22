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

#include "dpu_hw_intr.h"
#include "dpu_hw_cap.h"
#include "dpu_log.h"

static int32_t dpu_hw_intr_res_init(struct dpu_hw_intr *hw_intr,
		struct dpu_intr_cap *intr_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_intr_cap *blk_cap = intr_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong intr hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_intr->blk_cap = blk_cap;

	hw_intr->hw.iomem_base = base_mem->base;
	hw_intr->hw.base_addr = base_mem->addr;
	hw_intr->hw.blk_id = blk_cap->base.id;
	hw_intr->hw.blk_offset = blk_cap->base.addr;
	hw_intr->hw.blk_len = blk_cap->base.len;
	hw_intr->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_intr_ops_init(unsigned long features,
		struct dpu_hw_intr_ops *ops)
{
	ops->enable = dpu_hw_intr_enable;
	ops->status_get = dpu_hw_intr_status_get;
	ops->status_clear = dpu_hw_intr_status_clear;
}

struct dpu_hw_blk *dpu_hw_intr_init(
		struct dpu_intr_cap *intr_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_intr *hw_intr;
	int32_t ret;

	hw_intr = dpu_mem_alloc(sizeof(*hw_intr));
	if (!hw_intr)
		return NULL;

	ret = dpu_hw_intr_res_init(hw_intr, intr_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init intr hw blk %d\n", intr_cap->base.id);
		dpu_mem_free(hw_intr);
		return NULL;
	}

	dpu_intr_ops_init(intr_cap->base.features, &hw_intr->ops);

	return &hw_intr->hw;
}

void dpu_hw_intr_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_intr(hw));
}
