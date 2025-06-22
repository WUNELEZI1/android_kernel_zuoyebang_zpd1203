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

#ifndef _DSI_HW_TMG_OPS_H_
#define _DSI_HW_TMG_OPS_H_

#include "dpu_osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsi.h"
#include "dpu_hw_obuffer.h"

#define TMG_VIDEO_MODE    0x1
#define TMG_COMMAND_MODE  0x0
#define TMG_BASE_TIMING_NUM 4

enum vrr_mode {
	CMD_MODE_AUTO_VRR,
	CMD_MODE_MANUAL_VRR,
	CMD_MODE_ACK_ATONCE_VRR,

	TMG_VRR_SKIP_FRAME_MODE,
	TMG_VRR_EM_PULSE_MODE,
	TMG_VRR_SKIP_AND_EM_MODE,
	TMG_VRR_LONG_V_MODE,
	TMG_VRR_LONG_H_MODE,
	TMG_VRR_MODE_MAX,
};

struct base_timing {
	struct dsi_sync_timing base0;
	struct dsi_sync_timing base1;
	struct dsi_sync_timing base2;
	struct dsi_sync_timing base3;
};

struct frame_params {
	u8 skip_num;
	u8 em_num;
};

struct vrr_cmd_parms {
	/* VRR next frame */
	u32 cmd_skip_te_next;
	/* 0 - mode 0, 1 - mode 1 not used*/
	u8  cmd_man_mode;
	u8  cmd_auto_en;
	/* 0- cmd mode vrr manual mode, 1- auto mode  */
};

struct vrr_vid_parms {
	u32 video_em_unit_num;
	u8  video_sync_opt;
	u32 video_em_next_num;
	u32 video_skip_frame_num;
	u32 video_frame_em_num;
	u32 video_det_te_en;
};

struct vg_parms {
	/* 0:R, 1:G, 2:B, 3:User */
	u8 pat_sel;
	/* user pattern */
	u32 user_pattern;
};

/* dsi_tmg_cfg - dsi timing generator config data structure */
struct dsi_tmg_cfg {
	/* 1: DIRECT_WRITE(true), 0: CMDLIST_WRITE(false) */
	u8 cmdlist_flag;
	/* dvfs enable: 1 enable, 0 disable */
	u8 dvfs_en;
	/* doze enter flag: 1 enable, 0 disable */
	u8 doze_enter;
	/* frame_power enable: 1 enable, 0 disable */
	u8 frame_power_en;
	/* frame power ctrl's max refresh rate */
	u8 frame_power_max_refresh_rate;
	/* need hw ulps : 1 need, 0 not need */
	u8 need_hw_ulps;
	/* hw ulps's max refresh rate */
	u8 hw_ulps_max_refresh_rate;
	/* vrr: only for skip_frame mode */
	u8 at_once;
	/* ulps enter or exit */
	u8 ulps_enter;
	u8 ulps_exit;
	/* tmg vrr function enable */
	u8 vrr_en;
	/* video or command mode */
	enum dsi_ctrl_mode tmg_mode;
	/* for dual port mipi dsi: 0 - disable, 1 - enable */
	u8 split_en;
	/* dsi vg test: 0 - disable, 1 - enable */
	u8 vg_en;
	/* side by side: 0 - disable, 1 - enable */
	u8 sbs_en;
	/* dsi vg pattern: 0 - Color-bar, 1 - R, 2 - G, 3 - B */
	u8 vg_dpmode;
	/* partial update mode: 0 - disable, 1 - enable */
	u8 partial_en;
	/* only command mode blurred screen: 0-Disable, 1-Enable */
	u32 cmd_wait_data_en;
	/* dsi ipi clock : 1/4 pixel clock */
	u32 ipi_clk;
	/* frame rate for next frame */
	u8 fps_update;
	/* base frame rate */
	u8 fps_base;
	/* select one base frame from 4 base fames: 0/1/2/3 */
	u8 current_base;
	/* TE select: 0 - from GIPO TE, 1 - from MIPI DSI */
	u8 te_sel;
	/* base frame timing */
	struct base_timing base;
	/* pixel format: RGB88, RGB666, RGB101010, DSC */
	enum dsi_pixel_format format;
	/* vrr mode */
	enum vrr_mode vrr_mode;
	/* command mode vrr parameters */
	struct vrr_cmd_parms vrr_cmd;
	/* video mode vrr parameters */
	struct vrr_vid_parms vrr_vid;
	/* tmg self test enable: 0 - disable, 1 - enable */
	struct vg_parms vg;
	/* partial update mode enable: only for command mode */
	struct partial_parms partial;
	/* obuffer parameters */
	struct obuffer_parms obuf;
};

void dsi_hw_tmg_reset(struct dpu_hw_blk *hw);
void dsi_hw_tmg_obufen_config(struct dpu_hw_blk *hw, u32 obufen);
void dsi_hw_tmg_init(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);
void dsi_hw_tmg_enable(struct dpu_hw_blk *hw);
void dsi_hw_tmg_disable(struct dpu_hw_blk *hw);
void dsi_hw_tmg_doze_enable(struct dpu_hw_blk *hw);
//void dsi_hw_tmg_vrr_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);
int dsi_hw_tmg_ulps_entry(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);
int dsi_hw_tmg_ulps_exit(struct dpu_hw_blk *hw, u8 flag);
void dsi_hw_tmg_obuffer_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);
void dsi_hw_tmg_partial_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);
u64 dsi_hw_tmg_timestamp_get(struct dpu_hw_blk *hw);
int dsi_hw_tmg_is_auto_selftest(struct dpu_hw_blk *hw);
void dsi_hw_tmg_frame_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);
void dsi_hw_tmg_status_dump(struct dpu_hw_blk *hw);
/**
 * dsi_hw_tmg_obuffer_level_config - config dsi obuffer level
 *
 * @hw: hardware resource of dsi
 * @obuffer: obuffer paramters to config
 */
void dsi_hw_tmg_obuffer_level_config(struct dpu_hw_blk *hw, struct obuffer_parms *obuffer);

/**
 * dsi_hw_tmg_dyn_freq_update - update tmg timing for dynamic frequency
 * @hw: hardware resource of dsi
 * @cfg: dsi tmg config
 */
void dsi_hw_tmg_dyn_freq_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

#endif /* _DSI_HW_TMG_OPS_H_ */
