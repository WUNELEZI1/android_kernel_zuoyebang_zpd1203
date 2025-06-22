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
#include "dpu_hw_tm.h"
#include "dpu_hw_tm_ops.h"

static inline void dpu_tm_ops_init(struct dpu_hw_tm_ops *ops)
{
	ops->enable = dpu_hw_tm_enable;
	ops->set_tm = dpu_hw_tm_set;
	ops->flush = dpu_hw_tm_flush;
}

static int dpu_tm_cap_init(struct dpu_hw_tm *tm,
		struct dpu_tm_cap *cap, struct dpu_iomem *base_mem)
{
	if ((cap->base.addr + cap->base.len) > base_mem->len) {
		dpu_pr_err("wrong tm hw info 0x%x, %d, 0x%x, %d\n",
				cap->base.addr, cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	tm->blk_cap = cap;

	tm->hw.iomem_base = base_mem->base;
	tm->hw.base_addr = base_mem->addr;
	tm->hw.blk_id = cap->base.id;
	tm->hw.blk_offset = cap->base.addr;
	tm->hw.blk_len = cap->base.len;
	tm->hw.features = cap->base.features;

	return 0;
}

struct dpu_hw_blk *dpu_hw_tm_init(struct dpu_tm_cap *cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_tm *tm;
	int ret;

	tm = dpu_mem_alloc(sizeof(struct dpu_hw_tm));
	dpu_check_and_return(!tm, NULL, "tm is null\n");

	ret = dpu_tm_cap_init(tm, cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init tm hw blk %d\n",
				cap->base.id);
		dpu_mem_free(tm);
		return NULL;
	}

	dpu_tm_ops_init(&tm->ops);

	return &tm->hw;
}

void dpu_hw_tm_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_tm(hw));
}
