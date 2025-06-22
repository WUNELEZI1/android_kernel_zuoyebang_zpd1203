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

#include <linux/slab.h>
#include "dp_hw_tmg.h"

static struct dp_hw_tmg_ops hw_tmg_ops = {
	.enable                        = dp_hw_tmg_enable,
	.config                        = dp_hw_tmg_config,
	.get_state                     = dp_hw_tmg_get_state,
	.enable_self_test              = dp_hw_tmg_enable_self_test,
};

struct dp_hw_tmg *dp_hw_tmg_init(struct dp_blk_cap *cap)
{
	struct dp_hw_tmg *hw_tmg;

	hw_tmg = kzalloc(sizeof(*hw_tmg), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_tmg))
		return ERR_PTR(-ENOMEM);

	hw_tmg->hw.iomem_base = cap->vaddr;
	hw_tmg->hw.base_addr = cap->paddr;
	hw_tmg->hw.blk_len = cap->len;
	hw_tmg->ops = &hw_tmg_ops;

	return hw_tmg;
}

void dp_hw_tmg_deinit(struct dp_hw_tmg *hw_tmg)
{
	kfree(hw_tmg);
}
