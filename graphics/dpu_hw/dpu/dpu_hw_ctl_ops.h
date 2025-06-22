/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _DPU_HW_DPU_CTL_OPS_H_
#define _DPU_HW_DPU_CTL_OPS_H_

#include "dpu_hw_common.h"

enum dpu_hw_wb_position {
	WB_POS_BEFORE_PREPQ0 = 0,
	WB_POS_AFTER_PREPQ0,
	WB_POS_BEFORE_PREPQ1,
	WB_POS_AFTER_PREPQ1,
	WB_POS_BEFORE_PREPQ2,
	WB_POS_AFTER_PREPQ2,
	WB_POS_BEFORE_PREPQ3,
	WB_POS_AFTER_PREPQ3,
	WB_POS_BEFORE_PREPQ4,
	WB_POS_AFTER_PREPQ4,
	WB_POS_BEFORE_PREPQ5,
	WB_POS_AFTER_PREPQ5,
	WB_POS_BEFORE_PREPQ6,
	WB_POS_AFTER_PREPQ6,
	WB_POS_BEFORE_PREPQ7,
	WB_POS_AFTER_PREPQ7,
	WB_POS_BEFORE_PREPQ8,
	WB_POS_AFTER_PREPQ8,
	WB_POS_BEFORE_PREPQ9,
	WB_POS_AFTER_PREPQ9,
	WB_POS_AFTER_MIXER0,
	WB_POS_AFTER_MIXER1,
	WB_POS_AFTER_MIXER2,
	WB_POS_AFTER_POSTPQ0,
	WB_POS_AFTER_POSTPQ1,
	WB_POS_AFTER_RC,
	WB_POS_INVALID,
	WB_POS_MAX = WB_POS_INVALID,
};

#define GET_WB_POS_AFTER_MIXER(scene_id) ((scene_id) + WB_POS_AFTER_MIXER0)
#define GET_WB_POS_AFTER_POSTPQ(scene_id) ((scene_id) + WB_POS_AFTER_POSTPQ0)
#define GET_WB_POS_BEFORE_PREPQ(prepq_id) ((prepq_id) * 2)
#define GET_WB_POS_AFTER_PREPQ(prepq_id) ((prepq_id) * 2 + 1)

/**********************dpu ctl top*********************/

/**
 * dpu_hw_wb_input_position_setup - wb(index) input position setup
 * @hw: module addr
 * @wb_id: wb index
 * @position: wb input position
 */
void dpu_hw_wb_input_position_setup(struct dpu_hw_blk *hw,
		enum dpu_wb_id wb_id, enum dpu_hw_wb_position position);

/**********************dpu scene ctl*********************/

/**
 * dpu_hw_rch_mount - rchX mount to ctl
 * @hw: module addr
 * @rch_mask: the rch bitmask for mounting rchs to scene control
 */
void dpu_hw_rch_mount(struct dpu_hw_blk *hw, u32 rch_mask);

/**
 * dpu_hw_rch_vrt_enable - config rch v-extender function
 * @hw: module addr
 * @rch_id: the rch id for v-extender
 * @enable: true or false
 */
void dpu_hw_rch_vrt_config(struct dpu_hw_blk *hw, u32 rch_id, bool enable);

/**
 * dpu_hw_wb_mount - wbX mount to ctl
 * @hw: module addr
 * @wb_mask: the bitmask for mounting write back to scene control
 */
void dpu_hw_wb_mount(struct dpu_hw_blk *hw, u32 wb_mask);

/**
 * dpu_hw_dsc_rdma_mount - dsc rdma mount to ctl
 * @hw: module addr
 * @mask: the dsc rdma bitmask for mounting rchs to scene control
 */
void dpu_hw_dsc_rdma_mount(struct dpu_hw_blk *hw, u32 mask);

/**
 * dpu_hw_dsc_wdma_mount - dsc wdma mount to ctl
 * @hw: module addr
 * @mask: the dsc wdma bitmask for mounting write back to scene control
 */
void dpu_hw_dsc_wdma_mount(struct dpu_hw_blk *hw, u32 mask);

/**
 * dpu_hw_timing_engine_mode_set - set tmg mode
 * @hw: the scene_ctl hardware pointer
 * @is_cmd_mode: true for cmd mode or false for video mode
 * @auto_refresh_enable: enable auto refresh for cmd mode
 */
void dpu_hw_timing_engine_mode_set(struct dpu_hw_blk *hw,
		bool is_cmd_mode, bool auto_refresh_enable);

/**
 * dpu_hw_timing_engine_mount - mount timing engine to this ctl
 * @hw: pointer of scene control base
 * @tmg_mask: the bitmask for mounting timing engine to scene control
 */
void dpu_hw_timing_engine_mount(struct dpu_hw_blk *hw, u32 tmg_mask);

/**
 * dpu_hw_outctrl_mount - mount outctrl in online scene
 *
 * @hw: the scene_ctl hardware pointer
 * @enable: enable or disable outctrl
 */
void dpu_hw_outctrl_mount(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_scene_cfg_ready_update - used when reg cfg ready
 * @hw: module addr
 * @update_en: cmd update enable
 * @tui_en: secure tui enable
 */
void dpu_hw_scene_cfg_ready_update(struct dpu_hw_blk *hw,
		bool update_en, bool tui_en);

/**
 * dpu_hw_scene_init_en - enable init cfg for dpu power on init
 * @hw: the scene_ctl hardware pointer
 * @init_en: normal init enable
 */
void dpu_hw_scene_init_en(struct dpu_hw_blk *hw, bool init_en);

/**
 * dpu_hw_first_frame_start - sw control the first frame begin
 * @hw: module addr
 */
void dpu_hw_first_frame_start(struct dpu_hw_blk *hw);

/**
 * dpu_hw_scene_reset - reset the scene registers
 * @hw: the scene hardware pointer
 */
void dpu_hw_scene_reset(struct dpu_hw_blk *hw);

/**
 * dpu_hw_scene_ctrl_sw_clear - clear abnormal scene ctl registers
 * @hw: the scene_ctl hardware pointer
 */
int dpu_hw_scene_ctrl_sw_clear(struct dpu_hw_blk *hw);

/**
 * dpu_hw_ctrl_top_status_dump - dump debug registers of ctrl top
 * @hw: the ctrl_top hardware pointer
 */
void dpu_hw_ctrl_top_status_dump(struct dpu_hw_blk *hw);

/**
 * dpu_hw_ctrl_top_status_clear - clear the irq raw of ctrl_top
 * @hw: the ctrl_top hardware pointer
 */
void dpu_hw_ctrl_top_status_clear(struct dpu_hw_blk *hw);

/**
 * dpu_hw_scene_ctrl_status_dump - dump debug registers of scene ctrl
 * @hw: the scene_ctrl hardware pointer
 */
void dpu_hw_scene_ctrl_status_dump(struct dpu_hw_blk *hw);

/**
 * is_dpu_hw_sense_ctrl_idle - check dpu hw sense ctrl is idle or not
 * @hw: the scene_ctrl hardware pointer
 * @hw_status: get current sense ctrl status
 *
 * Return: true for idle, false for not
 */
bool is_dpu_hw_sense_ctrl_idle(struct dpu_hw_blk *hw, u32 *hw_status);

#endif
