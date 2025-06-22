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
#include "dpu_hw_prepipe_top_ops.h"
#include "dpu_hw_prepipe_top_reg.h"

void dpu_hw_prepipe_top_feature_enable(struct dpu_hw_blk *hw,
		struct prepq_feature_ctrl *ctrl)
{
	u32 enable = 0;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (ctrl->enabled_features & BIT(PLANE_COLOR_NON_PREALPHA_EN))
		enable |= BIT(V_PREPIPE_TOP_NON_PREALPHA_EN_SHIFT);

	DPU_REG_WRITE(hw, V_PREPIPE_TOP_NON_PREALPHA_EN_OFFSET, enable, CMDLIST_WRITE);

	enable = 0;
	if (ctrl->enabled_features & BIT(PLANE_COLOR_ALPHA_EN))
		enable |= BIT(V_PREPIPE_TOP_ALPHA_DISABLE_SHIFT);

	if (ctrl->enabled_features & BIT(PLANE_COLOR_R2Y_EN))
		enable |= BIT(V_PREPIPE_TOP_R2Y_EN_SHIFT);

	if (ctrl->enabled_features & BIT(PLANE_COLOR_Y2R_EN))
		enable |= BIT(V_PREPIPE_TOP_Y2R_EN_SHIFT);

	if (ctrl->left_right_reuse)
		enable |= BIT(V_PREPIPE_TOP_SBS_EN_SHIFT);

	DPU_REG_WRITE(hw, V_PREPIPE_TOP_WB_RIGHT_SIGN_EN_OFFSET, enable, CMDLIST_WRITE);

	enable = 0;
	if (ctrl->enabled_features & BIT(PLANE_COLOR_PREALPHA_EN))
		enable |= BIT(PREALPHA_EN_SHIFT);

	DPU_REG_WRITE(hw, PREALPHA_EN_OFFSET, enable, CMDLIST_WRITE);
}

void dpu_hw_prepipe_top_dither_lite_enable(struct dpu_hw_blk *hw,
		bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid hw %pK\n", hw);
		return;
	}

	DPU_REG_WRITE(hw, V_PREPIPE_TOP_DITHER_EN_OFFSET,
			enable, CMDLIST_WRITE);
}

void dpu_hw_prepipe_rgb2yuv_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_csc_matrix_cfg *matrix_cfg;
	u32 val;
	int i;

	if (!hw_cfg) {
		DPU_ERROR("invalid hw_cfg %pK\n", hw_cfg);
		return;
	}

	if (!hw_cfg->data || hw_cfg->len != sizeof(*matrix_cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*matrix_cfg));
		return;
	}

	matrix_cfg = hw_cfg->data;

	for (i = 0; i < (CSC_MATRIX_SIZE >> 1); i++) {
		val = 0;
		val = MERGE_BITS(val, matrix_cfg->matrix[i << 1],
				V_PREPIPE_TOP_RGB2YUV_MATRIX00_SHIFT,
				V_PREPIPE_TOP_RGB2YUV_MATRIX00_LENGTH);
		val = MERGE_BITS(val, matrix_cfg->matrix[(i << 1) + 1],
				V_PREPIPE_TOP_RGB2YUV_MATRIX01_SHIFT,
				V_PREPIPE_TOP_RGB2YUV_MATRIX01_LENGTH);
		DPU_REG_WRITE(hw, V_PREPIPE_TOP_RGB2YUV_MATRIX00_OFFSET + (i << 2),
				val, CMDLIST_WRITE);
	}
}

void dpu_hw_prepipe_yuv2rgb_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_csc_matrix_cfg *matrix_cfg;
	u32 val;
	int i;

	if (!hw_cfg) {
		DPU_ERROR("invalid hw_cfg %pK\n", hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*matrix_cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*matrix_cfg));
		return;
	}

	matrix_cfg = hw_cfg->data;

	for (i = 0; i < (CSC_MATRIX_SIZE >> 1); i++) {
		val = 0;
		val = MERGE_BITS(val, matrix_cfg->matrix[i << 1],
				V_PREPIPE_TOP_YUV2RGB_MATRIX00_SHIFT,
				V_PREPIPE_TOP_YUV2RGB_MATRIX00_LENGTH);
		val = MERGE_BITS(val, matrix_cfg->matrix[(i << 1) + 1],
				V_PREPIPE_TOP_YUV2RGB_MATRIX01_SHIFT,
				V_PREPIPE_TOP_YUV2RGB_MATRIX01_LENGTH);
		DPU_REG_WRITE(hw, V_PREPIPE_TOP_YUV2RGB_MATRIX00_OFFSET + (i << 2),
				val, CMDLIST_WRITE);
	}
}

void dpu_hw_prepipe_non_prealpha_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_none_prealpha_offset *offsets;

	if (!hw) {
		DPU_ERROR("Invalid parameter!\n");
		return;
	}

	if (!hw_cfg->data || hw_cfg->len != sizeof(struct dpu_none_prealpha_offset)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(struct dpu_none_prealpha_offset));
		return;
	}

	offsets = hw_cfg->data;

	DPU_REG_WRITE(hw, V_PREPIPE_TOP_OFFSET_OFFSET,
			offsets->offset0, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, V_PREPIPE_TOP_OFFSET_OFFSET + (1 << 2),
			offsets->offset1, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, V_PREPIPE_TOP_OFFSET_OFFSET + (1 << 3),
			offsets->offset2, CMDLIST_WRITE);
}

void dpu_hw_prepipe_top_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter!\n");
		return;
	}

	DPU_REG_WRITE(hw, V_PREPIPE_TOP_FORCE_UPDATE_EN_OFFSET, 0x02, CMDLIST_WRITE);
	DPU_COLOR_DEBUG("Flush done!\n");
}
