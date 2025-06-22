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

#include "dpu_hw_mixer.h"
#include "dpu_hw_cap.h"
#include "dpu_log.h"

static int32_t dpu_hw_mixer_cap_init(struct dpu_hw_mixer *hw_mixer,
		struct dpu_mixer_cap *mixer_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_mixer_cap *blk_cap = mixer_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong mixer hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
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
}

struct dpu_hw_blk *dpu_hw_mixer_init(struct dpu_mixer_cap *mixer_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_mixer *hw_mixer;
	int32_t ret;

	hw_mixer = dpu_mem_alloc(sizeof(*hw_mixer));
	if (!hw_mixer)
		return NULL;

	ret = dpu_hw_mixer_cap_init(hw_mixer, mixer_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init mixer hw blk %d\n", mixer_cap->base.id);
		dpu_mem_free(hw_mixer);
		return NULL;
	}

	dpu_mixer_ops_init(mixer_cap->base.features, &hw_mixer->ops);

	return &hw_mixer->hw;
}

void dpu_hw_mixer_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_mixer(hw));
}
