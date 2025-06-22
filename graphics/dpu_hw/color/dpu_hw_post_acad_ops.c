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

#include "dpu_reg_ops.h"
#include "dpu_color_common.h"
#include "dpu_hw_post_acad_reg.h"
#include "dpu_hw_post_acad_ops.h"

void dpu_hw_acad_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, ACAD_EN_OFFSET, enable, CMDLIST_WRITE);
}

static void dpu_hw_acad_roi_set(struct dpu_hw_blk *hw, struct dpu_region cfg)
{
	u32 val = 0;

	if (!hw) {
		DPU_ERROR("invalid acad roi parameter!\n");
		return;
	}

	val = MERGE_BITS(val, cfg.x, M_N_ROI_X_SHIFT, M_N_ROI_X_LENGTH);
	val = MERGE_BITS(val, cfg.y, M_N_ROI_Y_SHIFT, M_N_ROI_Y_LENGTH);
	DPU_REG_WRITE(hw, M_N_ROI_X_OFFSET, val, CMDLIST_WRITE);

	val = 0;
	val = MERGE_BITS(val, cfg.w, M_N_ROI_W_SHIFT, M_N_ROI_W_LENGTH);
	val = MERGE_BITS(val, cfg.h, M_N_ROI_H_SHIFT, M_N_ROI_H_LENGTH);
	DPU_REG_WRITE(hw, M_N_ROI_W_OFFSET, val, CMDLIST_WRITE);
}

static void dpu_hw_acad_sub_roi_set(struct dpu_hw_blk *hw, struct dpu_region *cfg)
{
	u32 val;
	int i;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	for (i = 0; i < SUB_ROI_AREA_SUPPORT_MAX; i++) {
		val = 0;
		val = MERGE_BITS(val, cfg[i].x,
				M_N_SUB_ROI0_X_SHIFT, M_N_SUB_ROI0_X_LENGTH);
		val = MERGE_BITS(val, cfg[i].y,
				M_N_SUB_ROI0_Y_SHIFT, M_N_SUB_ROI0_Y_LENGTH);
		DPU_REG_WRITE(hw, M_N_SUB_ROI0_X_OFFSET + (i << 3), val, CMDLIST_WRITE);

		val = MERGE_BITS(val, cfg[i].w,
				M_N_SUB_ROI0_W_SHIFT, M_N_SUB_ROI0_W_LENGTH);
		val = MERGE_BITS(val, cfg[i].h,
				M_N_SUB_ROI0_H_SHIFT, M_N_SUB_ROI0_H_LENGTH);
		DPU_REG_WRITE(hw, M_N_SUB_ROI0_W_OFFSET + (i << 3), val, CMDLIST_WRITE);
	}
}

static void dpu_hw_acad_blk_set(struct dpu_hw_blk *hw, struct dpu_acad_internel_blk cfg)
{
	u32 val = 0;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	val = MERGE_BITS(val, cfg.blk_width,
			M_N_BLK_WIDTH_SHIFT, M_N_BLK_WIDTH_LENGTH);
	val = MERGE_BITS(val, cfg.blk_num_hori,
			M_N_BLK_NUM_HORI_SHIFT, M_N_BLK_NUM_HORI_LENGTH);
	val = MERGE_BITS(val, cfg.blk_height,
			M_N_BLK_HEIGHT_SHIFT, M_N_BLK_HEIGHT_LENGTH);
	val = MERGE_BITS(val, cfg.blk_num_vert,
			M_N_BLK_NUM_VERT_SHIFT, M_N_BLK_NUM_VERT_LENGTH);
	DPU_REG_WRITE(hw, M_N_BLK_WIDTH_OFFSET, val, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg.weight_threshold_x,
			M_N_WEIGHT_SCALER_X_SHIFT, M_N_WEIGHT_SCALER_X_LENGTH);
	val = MERGE_BITS(val, cfg.weight_threshold_y,
			M_N_WEIGHT_SCALER_Y_SHIFT, M_N_WEIGHT_SCALER_Y_LENGTH);
	DPU_REG_WRITE(hw, M_N_WEIGHT_SCALER_X_OFFSET, val, CMDLIST_WRITE);
}

static void dpu_hw_acad_frame_curve_set(struct dpu_hw_blk *hw,
		struct dpu_acad_frame_curve *cfg)
{
	u32 val;
	int i;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	for (i = 0; i < CONTRAST_CURVE_SIZE; i++) {
		val = 0;
		val = MERGE_BITS(val, cfg->contrast_curve[0][i],
				M_P_CONTRAST_CURVE0_0_SHIFT, M_P_CONTRAST_CURVE0_0_LENGTH);
		val = MERGE_BITS(val, cfg->contrast_curve[1][i],
				M_P_CONTRAST_CURVE1_0_SHIFT, M_P_CONTRAST_CURVE1_0_LENGTH);
		DPU_REG_WRITE(hw, M_P_CONTRAST_CURVE0_0_OFFSET + (i << 2), val, CMDLIST_WRITE);

		val = MERGE_BITS(val, cfg->contrast_curve[2][i],
				M_P_CONTRAST_CURVE0_0_SHIFT, M_P_CONTRAST_CURVE0_0_LENGTH);
		val = MERGE_BITS(val, cfg->contrast_curve[3][i],
				M_P_CONTRAST_CURVE1_0_SHIFT, M_P_CONTRAST_CURVE1_0_LENGTH);
		DPU_REG_WRITE(hw, M_P_CONTRAST_CURVE2_0_OFFSET + (i << 2), val, CMDLIST_WRITE);
	}
}

void dpu_hw_acad_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_acad_cfg *cfg;
	u32 val = 0;

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*cfg));
	}

	cfg = hw_cfg->data;

	DPU_REG_WRITE(hw, M_N_CURVE_ALPHA_OFFSET, cfg->curve_alpha, CMDLIST_WRITE);

	dpu_hw_acad_blk_set(hw, cfg->internel_blk);

	val = MERGE_BITS(val, 1, ACAD_EN_SHIFT, ACAD_EN_LENGTH);
	val = MERGE_BITS(val, cfg->hist_mode,
			M_N_HIST_MODE_SHIFT, M_N_HIST_MODE_LENGTH);
	val = MERGE_BITS(val, cfg->curve_mode,
			M_N_CURVE_MODE_SHIFT, M_N_CURVE_MODE_LENGTH);
	val = MERGE_BITS(val, cfg->tp_alpha,
			TP_ALPHA_SHIFT, TP_ALPHA_LENGTH);
	val = MERGE_BITS(val, cfg->tm.tm_mode,
			M_N_ETM_MODE_SHIFT, M_N_ETM_MODE_LENGTH);
	DPU_REG_WRITE(hw, ACAD_EN_OFFSET, val, CMDLIST_WRITE);

	dpu_hw_acad_roi_set(hw, cfg->roi);
	dpu_hw_acad_sub_roi_set(hw, cfg->sub_roi);
	dpu_hw_acad_frame_curve_set(hw, &cfg->contrast_curve);
	val = 0;

	if (cfg->tm.tm_mode == ACAD_TM_MODE_AUTO) {
		val = MERGE_BITS(val, cfg->tm.alpha_step,
				M_N_ALPHA_STEP_SHIFT, M_N_ALPHA_STEP_LENGTH);
		val = MERGE_BITS(val, cfg->tm.skip_frames,
				M_N_SKIP_FRAMES_SHIFT, M_N_SKIP_FRAMES_LENGTH);
	}
	val = MERGE_BITS(val, cfg->dst_alpha,
			M_N_DST_ALPHA_SHIFT, M_N_DST_ALPHA_LENGTH);
	val = MERGE_BITS(val, cfg->segment_threshold[0],
			M_P_SEGMENT_THR0_SHIFT, M_P_SEGMENT_THR0_LENGTH);
	DPU_REG_WRITE(hw, M_N_DST_ALPHA_OFFSET, val, CMDLIST_WRITE);

	val = MERGE_BITS(0, cfg->segment_threshold[1],
			M_P_SEGMENT_THR1_SHIFT, M_P_SEGMENT_THR1_LENGTH);
	val = MERGE_BITS(val, cfg->segment_threshold[2],
			M_P_SEGMENT_THR2_SHIFT, M_P_SEGMENT_THR2_LENGTH);
	val = MERGE_BITS(val, cfg->segment_threshold[3],
			M_P_SEGMENT_THR3_SHIFT, M_P_SEGMENT_THR3_LENGTH);
	val = MERGE_BITS(val, cfg->segment_threshold[4],
			M_P_SEGMENT_THR4_SHIFT, M_P_SEGMENT_THR4_LENGTH);
	DPU_REG_WRITE(hw, M_P_SEGMENT_THR1_OFFSET, val, CMDLIST_WRITE);
}

void dpu_hw_acad_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, ACAD_FORCE_UPDATE_EN_OFFSET, 0x02, CMDLIST_WRITE);
	DPU_COLOR_DEBUG("done\n");
}
