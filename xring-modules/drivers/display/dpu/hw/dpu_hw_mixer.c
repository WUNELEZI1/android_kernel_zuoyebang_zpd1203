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
#include "dpu_hw_mixer.h"

static int dpu_hw_mixer_cap_init(struct dpu_hw_mixer *hw_mixer,
		struct dpu_mixer_cap *mixer_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_mixer_cap *blk_cap = mixer_cap;
	u32 base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		DPU_ERROR("wrong mixer hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -EINVAL;
	}

	hw_mixer->blk_cap = blk_cap;

	hw_mixer->hw.iomem_base = base_mem->base;
	hw_mixer->hw.base_addr = base_mem->addr;
	hw_mixer->hw.blk_id = blk_cap->base.id;
	hw_mixer->hw.blk_offset = blk_cap->base.addr;
	hw_mixer->hw.blk_len = blk_cap->base.len;
	hw_mixer->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_mixer_ops_init(unsigned long features,
		struct dpu_hw_mixer_ops *ops)
{
	ops->enable = dpu_hw_mixer_module_enable;
	ops->output_size_config = dpu_hw_mixer_dst_w_h_config;
	ops->bg_color_config = dpu_hw_mixer_bg_color_config;
	ops->rch_layer_config = dpu_hw_mixer_dma_layer_config;
	ops->solid_layer_config = dpu_hw_mixer_solid_layer_config;
	ops->reset = dpu_hw_mixer_reset;
	ops->status_dump = dpu_hw_mixer_status_dump;
}

struct dpu_hw_blk *dpu_hw_mixer_init(struct dpu_mixer_cap *mixer_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_mixer *hw_mixer;
	int ret;

	hw_mixer = kzalloc(sizeof(*hw_mixer), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_mixer))
		return ERR_PTR(-ENOMEM);

	ret = dpu_hw_mixer_cap_init(hw_mixer, mixer_cap, base_mem);
	if (ret) {
		DPU_ERROR("failed to init mixer hw blk %d\n",
				mixer_cap->base.id);
		kfree(hw_mixer);
		return ERR_PTR(ret);
	}

	dpu_mixer_ops_init(mixer_cap->base.features, &hw_mixer->ops);

	return &hw_mixer->hw;
}

void dpu_hw_mixer_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		kfree(to_dpu_hw_mixer(hw));
}
