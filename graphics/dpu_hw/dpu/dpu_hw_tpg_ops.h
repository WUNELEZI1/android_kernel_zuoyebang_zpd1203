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

#ifndef _DPU_HW_TPG_OPS_H_
#define _DPU_HW_TPG_OPS_H_

#include "dpu_hw_common.h"

enum tpg_position {
	TPG_POS_G0_PREPQ = 0,
	TPG_POS_V0_PREPQ,
	TPG_POS_G1_PREPQ,
	TPG_POS_V1_PREPQ,
	TPG_POS_G2_PREPQ,
	TPG_POS_G3_PREPQ,
	TPG_POS_G4_PREPQ,
	TPG_POS_V2_PREPQ,
	TPG_POS_V3_PREPQ,
	TPG_POS_G5_PREPQ,
	TPG_POS_MIXER0,
	TPG_POS_MIXER1,
	TPG_POS_MIXER2,
	TPG_POS_POSTPQ0,
	TPG_POS_POSTPQ1,
	TPG_POS_MAX,
};

/**
 * tpg_mode_index - different TPG image
 * @TPG_MODE_0: horizental/veritical/oblique gray level
 * @TPG_MODE_1: 32/16 step gray level
 * @TPG_MODE_2: solid color
 * @TPG_MODE_3: checkboard
 * @TPG_MODE_4: 100% color bar
 * @TPG_MODE_5: 75% color bar
 * @TPG_MODE_6: pattermode switch
 * @TPG_MODE_7: static image
 */
enum tpg_mode_index {
	TPG_MODE_0 = 0,
	TPG_MODE_1,
	TPG_MODE_2,
	TPG_MODE_3,
	TPG_MODE_4,
	TPG_MODE_5,
	TPG_MODE_6,
	TPG_MODE_7,
};

enum bpc_mode {
	BPC_10BIT = 0,
	BPC_8BIT,
	BPC_6BIT,
};

enum switch_mode {
	SWITCH_MODE_0 = 0,
	SWITCH_MODE_1,
};

/**
 * horizontal_vertical_param - contain the horizontal and
 * vertical parameters (step size and repeat num)
 * used in TPG_PATTERN_MODE(0,1,4,5,7)
 * @h_step_size: horizontal step size
 * @v_step_size: vertical step size
 * @h_repeat_num: horizontal repeat num
 * @v_repeat_num: vertical repeat num
 */
struct horizontal_vertical_param {
	u16 h_step_size;
	u16 v_step_size;
	u16 h_repeat_num;
	u16 v_repeat_num;
};


/**
 * rgba_value - contain rgb value ,alpha value and alpha type
 * used in TPG_PATTERN_MODE 2 and 7
 * @r_value: r channel value, from 20~29 bit in register
 * @g_value: g channel value, from 10~19 bit in register
 * @b_value: b channel value, from 0~9 bit in register
 * @a_value: alpha channel value
 * @a_type: 0:alpha value equals b_value 1:alpha value decide by a_value
 */
struct rgba_value {
	u16 r_value;
	u16 g_value;
	u16 b_value;
	u8 a_value;
	u8 a_type;
};

/**
 * switch_param - the image switch parameters
 * used in TPG_PATTERN_MODE(6,7)
 * @frame_num: image flash frequency
 * @switch_mode: 0:move the gray image 1:rise up the soid color image
 * @switch_channel: choose channel of the RGB to rise color
 */
struct switch_param {
	u8 frame_num;
	enum switch_mode switch_mode;
	u8 switch_channel;
};

/**
 * start_end_value - the start and end gray value
 * used in TPG_PATTERN_MODE1
 */
struct gray_range {
	u16 start_value;
	u16 end_value;
};


/**
 * tpg_config - contain the parameters of different pattern mode
 * @tpg_enable: enable the tpg module  0:bypass tpg module, 1:enable tpg module
 * @width: the width of the output image
 * @height: the height of the output image
 * @bpc: the bit size (6/8/10) of the tpg on postpq position
 * @h_blank: the cycle number between the two horizontal lines
 * @pattern_index: the pattern index of the TPG module, to output which image
 */
struct tpg_config {
	bool tpg_enable;
	u16 width;
	u16 height;
	enum bpc_mode bpc;
	u16 h_blank;
	enum tpg_mode_index mode_index;
	struct horizontal_vertical_param h_v_param;
	struct rgba_value rgba;
	struct switch_param switch_param;
	struct gray_range range_value;
};

/**
 * dpu_hw_tpg_config - config the different pattern mode
 * @iomem_base: module_addr
 * @tpg_pos: the position of the tpg module
 * @mode: set the format of the output image; set the specific parameters of
 * the different pattern mode to output corresponding images
 * @enable: enable or disable tpg
 */
void dpu_hw_tpg_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, enum tpg_mode_index mode, bool enable);

/**
 * dpu_hw_tpg_enable - config tpg regs
 * @iomem_base: module_addr
 * @tpg_pos: the position of the tpg module
 * @config: tpg configs
 */
void dpu_hw_tpg_enable(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, struct tpg_config *config);

/**
 * dpu_tpg_param_set - update TPG configs with parameters
 * @tpg_cfg: tpg configs
 * @param: defined as the internal implementation
 */
void dpu_tpg_param_set(struct tpg_config *tpg_cfg,
	u32 param);

#endif
