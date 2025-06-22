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
#include "dpu_hw_pre_scaler_ops.h"
#include "dpu_hw_scaler_reg.h"

void dpu_hw_pre_scaler_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter!\n");
		return;
	}

	DPU_REG_WRITE(hw, SCL_EN_OFFSET, enable, CMDLIST_WRITE);
}


static void dpu_hw_pre_scaler_stirpe_size_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_cfg *scaler)
{
	u32 val = 0;

	val = MERGE_BITS(val, scaler->stripe_height_in, STRIPE_HEIGHT_IN_SHIFT,
			STRIPE_HEIGHT_IN_LENGTH);
	val = MERGE_BITS(val, scaler->stripe_width_in, STRIPE_WIDTH_IN_SHIFT,
			STRIPE_WIDTH_IN_LENGTH);
	DPU_REG_WRITE(hw, STRIPE_HEIGHT_IN_OFFSET, val, CMDLIST_WRITE);

	val = 0;
	val = MERGE_BITS(val, scaler->stripe_height_out, STRIPE_HEIGHT_OUT_SHIFT,
			STRIPE_HEIGHT_OUT_LENGTH);
	val = MERGE_BITS(val, scaler->stripe_width_out, STRIPE_WIDTH_OUT_SHIFT,
			STRIPE_WIDTH_OUT_LENGTH);
	DPU_REG_WRITE(hw, STRIPE_HEIGHT_OUT_OFFSET, val, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, Y_STEP_OFFSET, scaler->y_step, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, X_STEP_OFFSET, scaler->x_step, CMDLIST_WRITE);
}

static void dpu_hw_pre_scaler_gauss_kernel_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_gauss_kernel *gauss_kernel)
{
	int i;

	for (i = 0; i < GAUSS_KERNEL_NUM_5X5; i++) {
		DPU_REG_WRITE(hw, ARRAY_GAUSS_KERNEL_0_OFFSET + (i << 2),
				gauss_kernel->gauss_kernel_5x5[i], CMDLIST_WRITE);
	}

	for (i = 0; i < GAUSS_KERNEL_NUM_9X9; i++) {
		DPU_REG_WRITE(hw, ARRAY_GAUSS_KERNEL_1_OFFSET + (i << 2),
				gauss_kernel->gauss_kernel_9x9[i], CMDLIST_WRITE);
	}
}

static void dpu_hw_pre_scaler_detail_enhance_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_cfg *cfg)
{
	u32 val = 0;

	val = MERGE_BITS(val, cfg->sigmoid_val0, SIGMOID_VAL0_SHIFT,
			SIGMOID_VAL0_LENGTH);
	val = MERGE_BITS(val, cfg->sigmoid_val1, SIGMOID_VAL1_SHIFT,
			SIGMOID_VAL1_LENGTH);
	val = MERGE_BITS(val, cfg->sigmoid_val2, SIGMOID_VAL2_SHIFT,
			SIGMOID_VAL2_LENGTH);
	DPU_REG_WRITE(hw, SIGMOID_VAL0_OFFSET, val, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, BASE_THRESHOLD_OFFSET, cfg->base_threshold, CMDLIST_WRITE);

	dpu_hw_pre_scaler_gauss_kernel_set(hw, &(cfg->gauss_kernel));
}

static void dpu_hw_pre_scaler_outlier_remove_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_cfg *cfg)
{
	u32 val = 0;

	val = MERGE_BITS(val, cfg->outlier_threshold1,
			OUTLIER_THRESHOLD1_SHIFT,
			OUTLIER_THRESHOLD1_LENGTH);
	val = MERGE_BITS(val, cfg->outlier_threshold2,
			OUTLIER_THRESHOLD2_SHIFT,
			OUTLIER_THRESHOLD2_LENGTH);
	val = MERGE_BITS(val, cfg->threshold_number,
			THRESHOLD_NUMBER_SHIFT,
			THRESHOLD_NUMBER_LENGTH);
	DPU_REG_WRITE(hw, OUTLIER_THRESHOLD1_OFFSET, val, CMDLIST_WRITE);
}

static void dpu_hw_pre_scaler_bicubic_kernel_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_bicubic_kernel *data)
{
	u32 val, tmp;
	int i;

	for (i = 0; i < SCALER_2D_KERNEL_LUT_NUM; i++) {
		val = 0;
		tmp = 0;
		/**
		 * bicubic_kernel_0_3: [11 : 0] is signed in complement form,
		 * bicubic_kernel_1_2: [11 : 0] is signed in complement form,
		 * combine them according to bit field: 0_3: [11 : 00],
		 * 1_2: [23 : 12],
		 */
		tmp |= (data->bicubic_kernel_0_3[i] & 0x7FF) |
				(data->bicubic_kernel_0_3[i] >> 15 & 0x800);
		val = MERGE_BITS(val, tmp, 0, 12);
		tmp = 0;
		tmp |= (data->bicubic_kernel_1_2[i] & 0x7FF) |
				(data->bicubic_kernel_1_2[i] >> 15 & 0x800);
		val = MERGE_BITS(val, tmp, 12, 12);

		DPU_REG_WRITE(hw, (ARRAY_BICUBIC_KERNEL_COMBINE_0_OFFSET + (i << 2)), val,
				CMDLIST_WRITE);
	}
}

static void dpu_hw_pre_scaler_lanczos3_kernel_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_lanczos3_kernel *data)
{
	u32 val, tmp;
	int i;

	for (i = 0; i < SCALER_2D_KERNEL_LUT_NUM; i++) {
		val = 0;
		/**
		 * lanczos3_kernel_0_5: [9 : 0] is signed in complement form,
		 * lanczos3_kernel_1_44: [10 : 0] is signed in complement form,
		 * lanczos3_kernel_2_3: [10 : 0] is unsigned;
		 * combine them according to bit field: 0_5: [09 : 00],
		 * 1_4: [21 : 10], 2_3: [31 : 22]
		 */
		tmp = 0;
		tmp = (data->lanczos3_kernel_0_5[i] & 0x1FF) |
				(data->lanczos3_kernel_0_5[i] >> 15 & 0x200);
		val = MERGE_BITS(val, tmp, 0, 10);
		tmp = 0;
		tmp = (data->lanczos3_kernel_1_4[i] & 0x3FF) |
				(data->lanczos3_kernel_1_4[i] >> 15 & 0x400);
		val = MERGE_BITS(val, tmp, 10, 11);
		tmp = 0;
		tmp = (data->lanczos3_kernel_2_3[i] & 0x7FF);
		val = MERGE_BITS(val, tmp, 21, 11);
		DPU_REG_WRITE(hw, ARRAY_LANCZOS3_KERNEL_COMBINE_0_OFFSET + (i << 2),
				val, CMDLIST_WRITE);
	}
}

static void dpu_hw_pre_scaler_stripe_set(struct dpu_hw_blk *hw,
		struct dpu_2d_scaler_cfg *scaler)
{
	DPU_REG_WRITE(hw, STRIPE_INIT_PHASE_OFFSET, scaler->stripe_init_phase, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, CROP_WIDTH_OUT_PHASE_OFFSET, scaler->crop_width_out_phase, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, CROP_HEIGHT_OUT_PHASE_OFFSET, scaler->crop_height_out_phase, CMDLIST_WRITE);
}

void dpu_hw_pre_scaler_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_2d_scaler_cfg *scaler = NULL;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*scaler)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*scaler));
		return;
	}
	scaler = hw_cfg->data;

	dpu_hw_pre_scaler_stirpe_size_set(hw, scaler);
	DPU_REG_WRITE(hw, ALPHA_DISABLE_OFFSET, scaler->alpha_disable, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, GRADIENT_THRESHOLD_OFFSET, scaler->gradient_threshold, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, INTERP_METHOD_OFFSET, scaler->interp_method, CMDLIST_WRITE);
	if (scaler->interp_method == SCALER_2D_LANCZOS3)
		dpu_hw_pre_scaler_lanczos3_kernel_set(hw, &(scaler->lanczos3_kernel));
	else
		dpu_hw_pre_scaler_bicubic_kernel_set(hw, &(scaler->bicubic_kernel));

	DPU_REG_WRITE(hw, DETAIL_ENHANCE_ENABLE_OFFSET,
			scaler->detail_enhance_enable, CMDLIST_WRITE);
	if (scaler->detail_enhance_enable) {
		dpu_hw_pre_scaler_detail_enhance_set(hw, scaler);
		DPU_REG_WRITE(hw, OUTLIER_REMOVE_ENABLE_OFFSET,
				scaler->outlier_remove_enable, CMDLIST_WRITE);
		if (scaler->outlier_remove_enable)
			dpu_hw_pre_scaler_outlier_remove_set(hw, scaler);
	}

	dpu_hw_pre_scaler_stripe_set(hw, scaler);
}

void dpu_hw_pre_scaler_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter!\n");
		return;
	}

	DPU_REG_WRITE(hw, DPU_2DSCL_FORCE_UPDATE_EN_OFFSET, 0x02, CMDLIST_WRITE);
	DPU_COLOR_DEBUG("Flush done!\n");
}

void dpu_hw_pre_scaler_get(struct dpu_hw_blk *hw,
		u16 base_idx, u16 end_idx, void *data)
{
	DPU_ERROR("No implementation!\n");
}
