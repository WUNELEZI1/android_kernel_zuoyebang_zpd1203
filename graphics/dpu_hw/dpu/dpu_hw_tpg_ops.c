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
#include "dpu_hw_tpg_ops.h"
#include "dpu_hw_tpg_reg.h"
#include "dpu_hw_common.h"

#define TPG_REG_WRITE(iomem_base, id, reg_offset, val) \
	DPU_REG_WRITE_BARE((iomem_base) + offset_table[(id)].offset + (reg_offset), (val))

#define BIT_ENABLE 1
#define B_START 0
#define G_START 10
#define R_START 20
#define CHANNEL_WIDTH 10

struct tpg_offset_table {
	enum tpg_position tpg_pos;
	uint32_t offset;
};

struct tpg_offset_table offset_table[TPG_POS_MAX] = {
	{TPG_POS_G0_PREPQ, 0x001780},
	{TPG_POS_V0_PREPQ, 0x002780},
	{TPG_POS_G1_PREPQ, 0x00B780},
	{TPG_POS_V1_PREPQ, 0x00C780},
	{TPG_POS_G2_PREPQ, 0x00D780},
	{TPG_POS_G3_PREPQ, 0x00E780},
	{TPG_POS_G4_PREPQ, 0x010780},
	{TPG_POS_V2_PREPQ, 0x011780},
	{TPG_POS_V3_PREPQ, 0x012780},
	{TPG_POS_G5_PREPQ, 0x013780},
	{TPG_POS_MIXER0, 0x018280},
	{TPG_POS_MIXER1, 0x030280},
	{TPG_POS_MIXER2, 0x048280},
	{TPG_POS_POSTPQ0, 0x021000},
	{TPG_POS_POSTPQ1, 0x039000},
};

static void __tpg_pattern_mode_0_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos,
		struct horizontal_vertical_param *param)
{
	uint32_t h_v_repeat_num = 0;
	uint32_t h_v_step_size = 0;

	TPG_REG_WRITE(iomem_base, tpg_pos,
		PATTERN_MODE_OFFSET, TPG_MODE_0);

	h_v_step_size = MERGE_BITS(h_v_step_size, param->v_step_size,
		VERTICAL_STEP_SIZE_SHIFT, VERTICAL_STEP_SIZE_LENGTH);
	h_v_step_size = MERGE_BITS(h_v_step_size, param->h_step_size,
		HORIZONTAL_STEP_SIZE_SHIFT, HORIZONTAL_STEP_SIZE_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_STEP_SIZE_OFFSET, h_v_step_size);

	h_v_repeat_num = MERGE_BITS(h_v_repeat_num, param->h_repeat_num,
		HORIZONTAL_REPEAT_NUM_SHIFT, HORIZONTAL_REPEAT_NUM_LENGTH);
	h_v_repeat_num = MERGE_BITS(h_v_repeat_num, param->v_repeat_num,
		VERTICAL_REPEAT_NUM_SHIFT, VERTICAL_REPEAT_NUM_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_REPEAT_NUM_OFFSET, h_v_repeat_num);
}

static void __tpg_pattern_mode_1_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, uint16_t width, struct gray_range *value,
		struct horizontal_vertical_param *param)
{
	uint32_t start_end_value = 0;
	uint32_t h_v_step_size = 0;
	uint32_t h_v_repeat_num = 0;

	TPG_REG_WRITE(iomem_base, tpg_pos,
		PATTERN_MODE_OFFSET, TPG_MODE_1);

	start_end_value = MERGE_BITS(start_end_value, value->start_value,
		START_VALUE_SHIFT, START_VALUE_LENGTH);
	start_end_value = MERGE_BITS(start_end_value, value->end_value,
		END_VALUE_SHIFT, END_VALUE_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		START_VALUE_OFFSET, start_end_value);

	h_v_step_size = MERGE_BITS(h_v_step_size, param->h_step_size,
		HORIZONTAL_STEP_SIZE_SHIFT, HORIZONTAL_STEP_SIZE_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_STEP_SIZE_OFFSET, h_v_step_size);

	h_v_repeat_num = MERGE_BITS(h_v_repeat_num, param->h_repeat_num,
		HORIZONTAL_REPEAT_NUM_SHIFT, HORIZONTAL_REPEAT_NUM_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_REPEAT_NUM_OFFSET, h_v_repeat_num);
}

static void __tpg_pattern_mode_2_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, struct rgba_value *rgba)
{
	uint32_t rgba_value = 0;
	uint32_t a_value_type = 0;

	TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_MODE_OFFSET, TPG_MODE_2);

	rgba_value = MERGE_BITS(rgba_value, rgba->b_value, B_START, CHANNEL_WIDTH);
	rgba_value = MERGE_BITS(rgba_value, rgba->g_value, G_START, CHANNEL_WIDTH);
	rgba_value = MERGE_BITS(rgba_value, rgba->r_value, R_START, CHANNEL_WIDTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, COLOR_OFFSET, rgba_value);

	a_value_type = MERGE_BITS(a_value_type, rgba->a_value,
		ALPHA_VALUE_SHIFT, ALPHA_VALUE_LENGTH);
	a_value_type = MERGE_BITS(a_value_type, rgba->a_type,
		ALPHA_TYPE_SHIFT, ALPHA_TYPE_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, ALPHA_VALUE_OFFSET, a_value_type);
}

static void __tpg_pattern_mode_4_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, uint16_t width)
{
	uint32_t h_repeat_num = 0;

	TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_MODE_OFFSET,
		TPG_MODE_4);

	h_repeat_num = MERGE_BITS(h_repeat_num, width >> 3,
		HORIZONTAL_REPEAT_NUM_SHIFT, HORIZONTAL_REPEAT_NUM_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_REPEAT_NUM_OFFSET, h_repeat_num);
}

static void __tpg_pattern_mode_5_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, uint16_t width)
{
	uint32_t h_repeat_num = 0;

	TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_MODE_OFFSET,
		TPG_MODE_5);

	h_repeat_num = MERGE_BITS(h_repeat_num, width >> 3,
		HORIZONTAL_REPEAT_NUM_SHIFT, HORIZONTAL_REPEAT_NUM_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_REPEAT_NUM_OFFSET, h_repeat_num);
}

static void __tpg_pattern_mode_6_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, struct switch_param *switch_param)
{
	TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_MODE_OFFSET,
			TPG_MODE_6);
	TPG_REG_WRITE(iomem_base, tpg_pos, FRM_NUM_OFFSET,
			switch_param->frame_num);
}


static void __tpg_pattern_mode_7_config(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos,
		struct horizontal_vertical_param *param,
		struct switch_param *switch_param,
		struct rgba_value *rgba)
{
	uint32_t rgba_value = 0;
	uint32_t a_value_type = 0;
	uint32_t switch_mode_channel = 0;
	uint32_t h_v_repeat_num = 0;
	uint32_t h_v_step_size = 0;

	TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_MODE_OFFSET,
			TPG_MODE_7);
	TPG_REG_WRITE(iomem_base, tpg_pos, FRM_NUM_OFFSET,
			switch_param->frame_num);

	rgba_value = MERGE_BITS(rgba_value, rgba->b_value, B_START, CHANNEL_WIDTH);
	rgba_value = MERGE_BITS(rgba_value, rgba->g_value, G_START, CHANNEL_WIDTH);
	rgba_value = MERGE_BITS(rgba_value, rgba->r_value, R_START, CHANNEL_WIDTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, COLOR_OFFSET, rgba_value);

	a_value_type = MERGE_BITS(a_value_type, rgba->a_value,
		ALPHA_VALUE_SHIFT, ALPHA_VALUE_LENGTH);
	a_value_type = MERGE_BITS(a_value_type, rgba->a_type,
		ALPHA_TYPE_SHIFT, ALPHA_TYPE_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, ALPHA_VALUE_OFFSET, a_value_type);

	switch_mode_channel = MERGE_BITS(switch_mode_channel, switch_param->switch_mode,
		SWITCH_MODE_SHIFT, SWITCH_MODE_LENGTH);
	switch_mode_channel = MERGE_BITS(switch_mode_channel, switch_param->switch_channel,
		SWITCH_CHANNEL_SHIFT, SWITCH_CHANNEL_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, SWITCH_MODE_OFFSET,
		switch_mode_channel);

	h_v_step_size = MERGE_BITS(h_v_step_size, param->v_step_size,
		VERTICAL_STEP_SIZE_SHIFT, VERTICAL_STEP_SIZE_LENGTH);
	h_v_step_size = MERGE_BITS(h_v_step_size, param->h_step_size,
		HORIZONTAL_STEP_SIZE_SHIFT, HORIZONTAL_STEP_SIZE_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_STEP_SIZE_OFFSET, h_v_step_size);

	h_v_repeat_num = MERGE_BITS(h_v_repeat_num, param->h_repeat_num,
		HORIZONTAL_REPEAT_NUM_SHIFT, HORIZONTAL_REPEAT_NUM_LENGTH);
	h_v_repeat_num = MERGE_BITS(h_v_repeat_num, param->v_repeat_num,
		VERTICAL_REPEAT_NUM_SHIFT, VERTICAL_REPEAT_NUM_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos,
		HORIZONTAL_REPEAT_NUM_OFFSET, h_v_repeat_num);
}

void dpu_hw_tpg_enable(DPU_IOMEM iomem_base,
		enum tpg_position tpg_pos, struct tpg_config *config)
{
	uint32_t clk_cg12 = 0;
	u32 image_width_height = 0;

	if (!iomem_base || !config) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", iomem_base, config);
		return;
	}

	TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_EN_OFFSET, config->tpg_enable);

	if (!config->tpg_enable)
		return;

	TPG_REG_WRITE(iomem_base, tpg_pos, BIT_POS_OFFSET, config->bpc);
	TPG_REG_WRITE(iomem_base, tpg_pos, H_BLANK_OFFSET, config->h_blank);
	TPG_REG_WRITE(iomem_base, tpg_pos, VSYNC_UPDATE_EN_OFFSET, BIT_ENABLE);

	image_width_height = MERGE_BITS(image_width_height,
		config->width, IMAGE_WIDTH_SHIFT, IMAGE_WIDTH_LENGTH);
	image_width_height = MERGE_BITS(image_width_height,
		config->height, IMAGE_HEIGHT_SHIFT, IMAGE_HEIGHT_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, IMAGE_WIDTH_OFFSET,
		image_width_height);

	clk_cg12 = MERGE_BITS(clk_cg12, BIT_ENABLE, CLK_CG1_AUTO_EN_SHIFT,
		CLK_CG1_AUTO_EN_LENGTH);
	clk_cg12 = MERGE_BITS(clk_cg12, BIT_ENABLE, CLK_CG2_AUTO_EN_SHIFT,
		CLK_CG2_AUTO_EN_LENGTH);
	TPG_REG_WRITE(iomem_base, tpg_pos, CLK_CG1_AUTO_EN_OFFSET, clk_cg12);

	switch (config->mode_index) {
	case TPG_MODE_0:
		__tpg_pattern_mode_0_config(iomem_base, tpg_pos, &config->h_v_param);
		break;

	case TPG_MODE_1:
		__tpg_pattern_mode_1_config(iomem_base, tpg_pos,
			config->width, &config->range_value, &config->h_v_param);
		break;

	case TPG_MODE_2:
		__tpg_pattern_mode_2_config(iomem_base, tpg_pos, &config->rgba);
		break;

	case TPG_MODE_3:
		TPG_REG_WRITE(iomem_base, tpg_pos, PATTERN_MODE_OFFSET,
			TPG_MODE_3);
		break;

	case TPG_MODE_4:
		__tpg_pattern_mode_4_config(iomem_base, tpg_pos, config->width);
		break;

	case TPG_MODE_5:
		__tpg_pattern_mode_5_config(iomem_base, tpg_pos, config->width);
		break;

	case TPG_MODE_6:
		__tpg_pattern_mode_6_config(iomem_base, tpg_pos, &config->switch_param);
		break;

	case TPG_MODE_7:
		__tpg_pattern_mode_7_config(iomem_base, tpg_pos,
			&config->h_v_param, &config->switch_param, &config->rgba);
		break;
	default:
		DPU_ERROR("invalid mode_index: %d\n", config->mode_index);
		break;
	}
}

void dpu_tpg_param_set(struct tpg_config *tpg_cfg,
	u32 param)
{
	tpg_cfg->bpc = BPC_10BIT;
	tpg_cfg->h_blank = 0;
	tpg_cfg->rgba.a_value = 255;
	tpg_cfg->rgba.a_type = 1;

	switch (tpg_cfg->mode_index) {
	case TPG_MODE_0:
		tpg_cfg->h_v_param.h_step_size = 1;
		tpg_cfg->h_v_param.v_step_size = 0;
		tpg_cfg->h_v_param.h_repeat_num = 1;
		tpg_cfg->h_v_param.v_repeat_num = 15;
		break;
	case TPG_MODE_1:
		tpg_cfg->h_v_param.h_step_size = 32;
		tpg_cfg->h_v_param.h_repeat_num = 23;
		tpg_cfg->range_value.start_value = 0;
		tpg_cfg->range_value.end_value = 1023;
		break;
	case TPG_MODE_2:
		tpg_cfg->rgba.r_value = (param >> 20) & 0x3FF;
		tpg_cfg->rgba.g_value = (param >> 10) & 0x3FF;
		tpg_cfg->rgba.b_value = param & 0x3FF;
		break;
	case TPG_MODE_3:
		break;
	case TPG_MODE_4:
		tpg_cfg->h_v_param.h_repeat_num = 90;
		break;
	case TPG_MODE_5:
		tpg_cfg->h_v_param.h_repeat_num = 90;
		break;
	case TPG_MODE_6:
		tpg_cfg->switch_param.frame_num = 1;
		break;
	case TPG_MODE_7:
		tpg_cfg->h_v_param.h_step_size = 1;
		tpg_cfg->h_v_param.v_step_size = 0;
		tpg_cfg->h_v_param.h_repeat_num = 1;
		tpg_cfg->h_v_param.v_repeat_num = 15;
		tpg_cfg->rgba.r_value = 1023;
		tpg_cfg->rgba.g_value = 0;
		tpg_cfg->rgba.b_value = 0;
		tpg_cfg->switch_param.frame_num = 1;
		tpg_cfg->switch_param.switch_mode = 0;
		tpg_cfg->switch_param.switch_channel = 0;
		break;
	}
}
