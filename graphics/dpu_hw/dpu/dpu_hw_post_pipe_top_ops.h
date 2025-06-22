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

#ifndef _DPU_HW_POST_PIPE_TOP_OPS_H_
#define _DPU_HW_POST_PIPE_TOP_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_hw_format.h"

/**
 * tmg_type select of post_pipe_top
 * @TMG0: select tmg0
 * @TMG1: select tmg1
 * @TMG2: select tmg2
 * @NONE: select none
 */
enum tmg_type {
	TMG0_DSI = 0,
	TMG1_DSI,
	TMG2_DP,
	NONE,
};

enum postpipe_debug {
	CROP_EN = BIT(0),
	INSPECT_EN = BIT(1),
	CRC_EN = BIT(2),
	SPLIT_EN = BIT(3),
	POSTPIPE_DEBUG_MAX,
};

/**
 * post_pipe_debug_ctrl - crop/inspec/crc/split config in post pipe top
 * @crop_en: crop enable
 * @inspect_en: inspect enable
 * @crc_en: crc enable
 * @split_en: split enable
 */
struct post_pipe_debug_ctrl {
	bool crop_en;
	bool inspect_en;
	bool crc_en;
	bool split_en;
};

/**
 * post_pipe_top_type
 * @POST_PIPE_FULL_TOP: post pipe full top
 * @POST_PIPE_LITE_TOP: post pipe lite top
 */
enum post_pipe_top_type {
	POST_PIPE_FULL_TOP = 0,
	POST_PIPE_LITE_TOP,
};

/**
 * post_pipe_dsc_cfg - dsc config in post pipe top
 * @dsc_port0_en: dsc_port0 enable status
 * @dsc_port1_en: dsc_port1 enable status
 * @dsc_clr_per_frm_en: dsc_clr_per_frm_en enable status
 */
struct post_pipe_dsc_cfg {
	bool dsc_port0_en;
	bool dsc_port1_en;
	bool dsc_clr_per_frm_en;
};

/**
 * post_pipe_cfg - post pipe top all config info
 * @port0_sel: post pipe port0 select status
 * @port1_sel: post pipe port1 select status
 * @pack_format: pack data format
 * @dsc_cfg: all dsc config info in post pipe top
 */
struct post_pipe_cfg {
	struct post_pipe_debug_ctrl debug_ctrl;
	enum tmg_type port0_sel;
	enum tmg_type port1_sel;
	enum pack_data_format pack_format;
	struct post_pipe_dsc_cfg dsc_cfg;
};

#define ACRP_SPR_POS 1
#define ACRP_WORK_MODE_BYPASS 1
#define ACRP_SETVAL_BYPASS 0
#define ACRP_PIXEL_FORMAT_REAL_RGB 0

/**
 * dpu_post_pipe_port_config - init the post_pipe_top registers
 * @hw: the post_pipe_top hardware pointer
 * @cfg: the post_pipe_top config info
 */
void dpu_post_pipe_port_config(struct dpu_hw_blk *hw, struct post_pipe_cfg *cfg);
/**
 * dpu_post_pipe_rc_enable - enable the RC module
 * @enable true: enable, false: disable
 * @hw: the post_pipe_top hardware pointer
 */
void dpu_post_pipe_rc_enable(struct dpu_hw_blk *hw,
		u16 width, u16 height, bool enable);
/**
 * dpu_post_pipe_rc_stream_config - config the RC stream registers
 * @hw: the post_pipe_top hardware pointer
 * @cfg: RC stream
 */
void dpu_post_pipe_rc_stream_config(struct dpu_hw_blk *hw, void *cfg);
/**
 * dpu_post_pipe_rc_config - config the RC registers
 * @hw: the post_pipe_top hardware pointer
 * @cfg: RC config info
 */
void dpu_post_pipe_rc_config(struct dpu_hw_blk *hw, void *cfg);

/**
 * dpu_post_pipe_top_status_dump - dump post_pipe_top status for debug
 * @hw: the post_pipe_top hardware pointer
 */
void dpu_post_pipe_top_status_dump(struct dpu_hw_blk *hw);

/**
 * dpu_post_pipe_top_status_clear - clear the post_pipe_top irq raw
 * @hw: the post_pipe_top hardware pointer
 */
void dpu_post_pipe_top_status_clear(struct dpu_hw_blk *hw);

/**
 * dpu_post_pipe_crc_dump - dump postpipe crc register
 * @hw: the post_pipe_top hardware pointer
 * @dsc_crc_selected: if true to choose dsc crc, false to choose postpipe crc
 *
 * @return: crc value on success, -1 on failure
 */
int32_t dpu_post_pipe_crc_dump(struct dpu_hw_blk *hw, bool dsc_crc_selected);
#endif
