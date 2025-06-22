/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_HW_SCENE_CTL_H_
#define _DPU_HW_SCENE_CTL_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_ctl_ops.h"

enum {
	NO_NEED_UPDT_EN = 0,
	NEED_UPDT_EN = 1,
};

/**
 * dpu_hw_scene_ctl_ops - the virtual function table of scene_ctl class
 */
struct dpu_hw_scene_ctl_ops {
	/**
	 * rch_mount - mount one rch to this scene control path
	 * @hw: the scene_ctl hardware pointer
	 * @rch_mask: the rch bitmask for mounting rchs to scene control
	 */
	void (*rch_mount)(struct dpu_hw_blk *hw, u32 rch_mask);


	/**
	 * rch_vrt_config - config rch v-extender function
	 * @hw: module addr
	 * @rch_id: the rch id for v-extender
	 * @enable: true or false
	 */
	void (*rch_vrt_config)(struct dpu_hw_blk *hw, u32 rch_id, bool enable);

	/**
	 * wb_mount - mount one wb to this scene control path
	 * @hw: the scene_ctl hardware pointer
	 * @wb_mask: the bitmask for mounting wb to scene control
	 */
	void (*wb_mount)(struct dpu_hw_blk *hw, u32 wb_mask);

	/**
	 * dsc_rdma_mount - dsc rdma mount to ctl
	 * @hw: module addr
	 * @mask: the dsc rdma bitmask for mounting rchs to scene control
	 */
	void (*dsc_rdma_mount)(struct dpu_hw_blk *hw, u32 mask);

	/**
	 * dsc_wdma_mount - dsc wdma mount to ctl
	 * @hw: module addr
	 * @mask: the dsc wdma bitmask for mounting write back to scene control
	 */
	void (*dsc_wdma_mount)(struct dpu_hw_blk *hw, u32 mask);

	/**
	 * timing_engine_mode_set - set tmg mode
	 * @hw: the scene_ctl hardware pointer
	 * @is_cmd_mode: true for cmd mode or false for video mode
	 * @auto_refresh_enable: enable auto refresh for cmd mode
	 */
	void (*timing_engine_mode_set)(struct dpu_hw_blk *hw,
			bool is_cmd_mode, bool auto_refresh_enable);

	/**
	 * timing_engine_mount - mount one tmg to this scene control path
	 * @hw: the scene_ctl hardware pointer
	 * @tmg_mask: the bitmask for mounting timing engine to scene control
	 */
	void (*timing_engine_mount)(struct dpu_hw_blk *hw, u32 tmg_mask);

	/**
	 * outctrl_mount - mount outctrl in online scene
	 *
	 * @hw: the scene_ctl hardware pointer
	 * @enable: enable or disable outctrl
	 */
	void (*outctrl_mount)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * cfg_ready_update - update cfg ready bit to true for flushing
	 * @hw: the scene_ctl hardware pointer
	 * @update_en: cmd update enable
	 * @tui_en: secure tui enable
	 */
	void (*cfg_ready_update)(struct dpu_hw_blk *hw, bool update_en, bool tui_en);

	/**
	 * init_en - enable init cfg for dpu power on init
	 * @hw: the scene_ctl hardware pointer
	 * @init_en: normal init enable
	 */
	void (*init_en)(struct dpu_hw_blk *hw, bool init_en);

	/**
	 * first_frame_start - set the first frame register to true when
	 *                     start first frame
	 * @hw: the scene_ctl hardware pointer
	 */
	void (*first_frame_start)(struct dpu_hw_blk *hw);

	/**
	 * reset - reset scene ctl registers
	 * @hw: the scene_ctl hardware pointer
	 */
	void (*reset)(struct dpu_hw_blk *hw);

	/**
	 * sw_clear - clean up scene ctl
	 * @hw: the scene_ctl hardware pointer
	 */
	int (*sw_clear)(struct dpu_hw_blk *hw);

	/**
	 * status_dump - hw status dump
	 * @hw: the scene_ctl hardware pointer
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);

	/**
	 * is_idle - is sense ctrl idle or not
	 * @hw: the scene_ctl hardware pointer
	 * @hw_status: get current sense ctrl status
	 *
	 * Return: true for idle, false for not
	 */
	bool (*is_idle)(struct dpu_hw_blk *hw, u32 *hw_status);
};

/**
 * dpu_hw_scene_ctl - scene_ctl hardware object
 * @hw: the scene_ctl hardware info
 * @blk_cap: the scene_ctl hardware resource and capability
 * @ops: the scene_ctl function table
 */
struct dpu_hw_scene_ctl {
	struct dpu_hw_blk hw;
	const struct dpu_scene_ctl_cap *blk_cap;

	struct dpu_hw_scene_ctl_ops ops;
};

static inline struct dpu_hw_scene_ctl *to_dpu_hw_scene_ctl(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_scene_ctl, hw);
}

/**
 * dpu_hw_scene_ctl_init - init one scene_ctl hw block with its capability
 * @scene_ctl_cap: the scene_ctl capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_scene_ctl_init(
		struct dpu_scene_ctl_cap *scene_ctl_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_scene_ctl_deinit - deinit one scene_ctl hw block with
 *                             base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_scene_ctl_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_SCENE_CTL_H_ */
