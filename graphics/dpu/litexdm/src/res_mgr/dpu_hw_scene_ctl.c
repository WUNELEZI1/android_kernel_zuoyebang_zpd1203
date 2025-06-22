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

#include "dpu_log.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_scene_ctl.h"

static int32_t dpu_hw_scene_ctl_res_init(
		struct dpu_hw_scene_ctl *hw_scene_ctl,
		struct dpu_scene_ctl_cap *scene_ctl_cap,
		struct dpu_iomem *base_mem)
{
	const struct dpu_scene_ctl_cap *blk_cap = scene_ctl_cap;
	uint32_t base_end, blk_end;

	base_end = base_mem->addr + base_mem->len;
	blk_end = base_mem->addr + blk_cap->base.addr + blk_cap->base.len;

	if (blk_end > base_end) {
		dpu_pr_err("wrong scene_ctl hw info 0x%x, %d, 0x%x, %d\n",
				blk_cap->base.addr, blk_cap->base.len,
				base_mem->addr, base_mem->len);
		return -1;
	}

	hw_scene_ctl->blk_cap = blk_cap;

	hw_scene_ctl->hw.iomem_base = base_mem->base;
	hw_scene_ctl->hw.base_addr = base_mem->addr;
	hw_scene_ctl->hw.blk_id = blk_cap->base.id;
	hw_scene_ctl->hw.blk_offset = blk_cap->base.addr;
	hw_scene_ctl->hw.blk_len = blk_cap->base.len;
	hw_scene_ctl->hw.features = blk_cap->base.features;

	return 0;
}

static void dpu_scene_ctl_ops_init(unsigned long features,
		struct dpu_hw_scene_ctl_ops *ops)
{
	ops->rch_mount = dpu_hw_rch_mount;
	ops->wb_mount = dpu_hw_wb_mount;
	ops->timing_engine_mode_set = dpu_hw_timing_engine_mode_set;
	ops->timing_engine_mount = dpu_hw_timing_engine_mount;
	ops->cfg_ready_update = dpu_hw_scene_cfg_ready_update;
	ops->first_frame_start = dpu_hw_first_frame_start;
	ops->reset = dpu_hw_scene_reset;
	ops->sw_clear = dpu_hw_scene_ctrl_sw_clear;
}

struct dpu_hw_blk *dpu_hw_scene_ctl_init(
		struct dpu_scene_ctl_cap *scene_ctl_cap,
		struct dpu_iomem *base_mem)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	int32_t ret;

	hw_scene_ctl = dpu_mem_alloc(sizeof(*hw_scene_ctl));
	if (!hw_scene_ctl)
		return NULL;

	ret = dpu_hw_scene_ctl_res_init(hw_scene_ctl, scene_ctl_cap, base_mem);
	if (ret) {
		dpu_pr_err("failed to init scene_ctl hw blk %d\n", scene_ctl_cap->base.id);
		dpu_mem_free(hw_scene_ctl);
		return NULL;
	}

	dpu_scene_ctl_ops_init(scene_ctl_cap->base.features, &hw_scene_ctl->ops);
	return &hw_scene_ctl->hw;
}

void dpu_hw_scene_ctl_deinit(struct dpu_hw_blk *hw)
{
	if (hw)
		dpu_mem_free(to_dpu_hw_scene_ctl(hw));
}
