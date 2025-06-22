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

#include "dpu_color_common.h"
#include "dpu_reg_ops.h"
#include "dpu_hw_postpq_top_reg.h"
#include "dpu_hw_postpq_top_ops.h"

void dpu_hw_postpq_top_pipe_init(struct dpu_hw_blk *hw,
		struct postpq_hw_init_cfg *cfg)
{
	u32 display_wh = 0;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	display_wh = MERGE_BITS(display_wh, cfg->hdisplay,
			POSTPQ_FULL_TOP_M_INWIDTH_SHIFT,
			POSTPQ_FULL_TOP_M_INWIDTH_LENGTH);
	display_wh = MERGE_BITS(display_wh, cfg->vdisplay,
			POSTPQ_FULL_TOP_M_INHEIGHT_SHIFT,
			POSTPQ_FULL_TOP_M_INHEIGHT_LENGTH);
	DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_M_INWIDTH_OFFSET,
			display_wh, CMDLIST_WRITE);
}

void dpu_hw_postpq_top_enable(struct dpu_hw_blk *hw,
		struct postpq_feature_ctrl *ctrl)
{
	u32 enable = 0;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (ctrl->lp_enable)
		enable |= BIT(POSTPQ_FULL_TOP_DITHER_MEM_LP_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_R2Y_EN))
		enable |= BIT(POSTPQ_FULL_TOP_R2Y_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_SCALER_EN))
		enable |= BIT(POSTPQ_FULL_TOP_SCALE_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_DITHER_EN))
		enable |= BIT(POSTPQ_FULL_TOP_DITHER_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_DITHER_LITE_EN))
		enable |= BIT(DITHER_LITE_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_Y2R_EN))
		enable |= BIT(POSTPQ_FULL_TOP_Y2R_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_CSC_MATRIX_EN))
		enable |= BIT(MATRIX_EN_SHIFT);

	if (ctrl->enabled_features > 0)
		enable |= BIT(POSTPQ_FULL_TOP_POSTPQ_EN_SHIFT);

	DPU_REG_WRITE(hw, POSTPQ_TOP_EN_OFFSET, enable, CMDLIST_WRITE);
}

void dpu_hw_post_dither_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_dither_cfg *dither_cfg;
	u32 val = 0;
	u32 map_val;
	int i;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*dither_cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*dither_cfg));
	}

	dither_cfg = hw_cfg->data;

	val = MERGE_BITS(val, dither_cfg->mode,
			POSTPQ_FULL_TOP_DITHER_MODE_SHIFT,
			POSTPQ_FULL_TOP_DITHER_MODE_LENGTH);

	val = MERGE_BITS(val, dither_cfg->dither_out_depth0,
			POSTPQ_FULL_TOP_DITHER_OUT_DPTH0_SHIFT,
			POSTPQ_FULL_TOP_DITHER_OUT_DPTH0_LENGTH);
	val = MERGE_BITS(val, dither_cfg->dither_out_depth1,
			POSTPQ_FULL_TOP_DITHER_OUT_DPTH1_SHIFT,
			POSTPQ_FULL_TOP_DITHER_OUT_DPTH1_LENGTH);
	val = MERGE_BITS(val, dither_cfg->dither_out_depth2,
			POSTPQ_FULL_TOP_DITHER_OUT_DPTH2_SHIFT,
			POSTPQ_FULL_TOP_DITHER_OUT_DPTH2_LENGTH);

	if (dither_cfg->mode == DITHER_MODE_PATTERN) {
		val = MERGE_BITS(val, dither_cfg->autotemp,
				POSTPQ_FULL_TOP_DITHER_AUTO_TEMP_SHIFT,
				POSTPQ_FULL_TOP_DITHER_AUTO_TEMP_LENGTH);
		val = MERGE_BITS(val, dither_cfg->rotatemode,
				POSTPQ_FULL_TOP_DITHER_ROTATE_MODE_SHIFT,
				POSTPQ_FULL_TOP_DITHER_ROTATE_MODE_LENGTH);
		val = MERGE_BITS(val, dither_cfg->temporalvalue,
				POSTPQ_FULL_TOP_DITHER_TEMP_VALUE_SHIFT,
				POSTPQ_FULL_TOP_DITHER_TEMP_VALUE_LENGTH);
		val = MERGE_BITS(val, dither_cfg->patternbits,
				POSTPQ_FULL_TOP_DITHER_PATTERN_BIT_SHIFT,
				POSTPQ_FULL_TOP_DITHER_PATTERN_BIT_LENGTH);

		for (i = 0; i < DITHER_BAYER_MAP_SIZE; i = i + 4) {
			map_val = 0;
			map_val = MERGE_BITS(map_val, dither_cfg->bayermap[i],
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP00_SHIFT,
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP00_LENGTH);
			map_val = MERGE_BITS(map_val, dither_cfg->bayermap[i + 1],
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP01_SHIFT,
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP01_LENGTH);
			map_val = MERGE_BITS(map_val, dither_cfg->bayermap[i + 2],
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP02_SHIFT,
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP02_LENGTH);
			map_val = MERGE_BITS(map_val, dither_cfg->bayermap[i + 3],
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP03_SHIFT,
					POSTPQ_FULL_TOP_DITHER_BAYER_MAP03_LENGTH);
			DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_DITHER_BAYER_MAP00_OFFSET + i, map_val,
					CMDLIST_WRITE);
		}
	}

	DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_DITHER_MODE_OFFSET, val, CMDLIST_WRITE);
}

void dpu_hw_post_scaler_output_size_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	u32 val = 0;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	val = MERGE_BITS(val, hw_cfg->lm_width,
			POSTPQ_FULL_TOP_M_INWIDTH_SHIFT,
			POSTPQ_FULL_TOP_M_INWIDTH_LENGTH);
	val = MERGE_BITS(val, hw_cfg->lm_height,
			POSTPQ_FULL_TOP_M_INHEIGHT_SHIFT,
			POSTPQ_FULL_TOP_M_INHEIGHT_LENGTH);
	DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_M_INWIDTH_OFFSET, val, CMDLIST_WRITE);

	val = 0;
	val = MERGE_BITS(val, hw_cfg->panel_width,
			M_SCALE_WIDTH_SHIFT, M_SCALE_WIDTH_LENGTH);
	val = MERGE_BITS(val, hw_cfg->panel_height,
			M_SCALE_HEIGHT_SHIFT, M_SCALE_HEIGHT_LENGTH);
	DPU_REG_WRITE(hw, M_SCALE_WIDTH_OFFSET, val, CMDLIST_WRITE);
}

void dpu_hw_post_rgb2yuv_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	int i;
	u32 val;
	struct dpu_csc_matrix_cfg *matrix_cfg;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*matrix_cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*matrix_cfg));
	}

	matrix_cfg = hw_cfg->data;

	for (i = 0; i < (CSC_MATRIX_SIZE >> 1); i++) {
		val = 0;
		val = MERGE_BITS(val, matrix_cfg->matrix[i << 1],
				POSTPQ_FULL_TOP_RGB2YUV_MATRIX00_SHIFT,
				POSTPQ_FULL_TOP_RGB2YUV_MATRIX00_LENGTH);
		val = MERGE_BITS(val, matrix_cfg->matrix[(i << 1) + 1],
				POSTPQ_FULL_TOP_RGB2YUV_MATRIX01_SHIFT,
				POSTPQ_FULL_TOP_RGB2YUV_MATRIX01_LENGTH);
		DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_RGB2YUV_MATRIX00_OFFSET + (i << 2),
				val, CMDLIST_WRITE);
	}
}

void dpu_hw_post_yuv2rgb_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	int i;
	u32 val;
	struct dpu_csc_matrix_cfg *matrix_cfg;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*matrix_cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*matrix_cfg));
	}

	matrix_cfg = hw_cfg->data;

	for (i = 0; i < (CSC_MATRIX_SIZE >> 1); i++) {
		val = 0;
		val = MERGE_BITS(val, matrix_cfg->matrix[i << 1],
				POSTPQ_FULL_TOP_YUV2RGB_MATRIX00_SHIFT,
				POSTPQ_FULL_TOP_YUV2RGB_MATRIX00_LENGTH);
		val = MERGE_BITS(val, matrix_cfg->matrix[(i << 1) + 1],
				POSTPQ_FULL_TOP_YUV2RGB_MATRIX01_SHIFT,
				POSTPQ_FULL_TOP_YUV2RGB_MATRIX01_LENGTH);
		DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_YUV2RGB_MATRIX00_OFFSET + (i << 2),
				val, CMDLIST_WRITE);
	}
}

void dpu_hw_csc_matrix_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	int i;
	u32 val;
	struct dpu_post_matrix_3x3_cfg *matrix_cfg;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*matrix_cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*matrix_cfg));
	}

	matrix_cfg = hw_cfg->data;

	for (i = 0; i < (POST_MATRIX_3X3_SIZE - 1); i = i + 2) {
		val = 0;
		val = MERGE_BITS(val, matrix_cfg->matrix[i],
				CSC_MATRIX00_SHIFT, CSC_MATRIX00_LENGTH);
		val = MERGE_BITS(val, matrix_cfg->matrix[i + 1],
				CSC_MATRIX01_SHIFT, CSC_MATRIX01_LENGTH);
		DPU_REG_WRITE(hw, CSC_MATRIX00_OFFSET + (i << 1), val, CMDLIST_WRITE);
	}

	DPU_REG_WRITE(hw, CSC_MATRIX22_OFFSET,
			matrix_cfg->matrix[POST_MATRIX_3X3_SIZE - 1], CMDLIST_WRITE);
}

void dpu_hw_post_dither_proc_loc_get(struct dpu_hw_blk *hw, void *loc)
{
	u32 val;
	struct dpu_pixel_location *location;

	if (!hw || !loc) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	location = (struct dpu_pixel_location *)loc;
	val = DPU_REG_READ(hw, POSTPQ_FULL_TOP_DITHER_PROC_LOC_OFFSET);
	location->x = val & 0xFFF;
	location->y = (val >> 16) & 0xFFF;
}

void dpu_hw_post_proc_loc_get(struct dpu_hw_blk *hw, void *loc)
{
	u32 val;
	struct dpu_pixel_location *location;

	if (!hw || !loc) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	location = (struct dpu_pixel_location *)loc;
	val = DPU_REG_READ(hw, PREPROC_LOC_OFFSET);
	location->x = val & 0xFFF;
	location->y = (val >> 16) & 0xFFF;
}


void dpu_hw_postpq_top_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, POSTPQ_FULL_TOP_FORCE_UPDATE_EN_OFFSET, 0x02,
			CMDLIST_WRITE);
	DPU_COLOR_DEBUG("done\n");
}
