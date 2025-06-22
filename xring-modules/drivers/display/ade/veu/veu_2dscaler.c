// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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
#include "veu_drv.h"
#include "veu_uapi.h"
#include "veu_defs.h"
#include "veu_utils.h"
#include "include/veu_hw_2dscaler_reg.h"
#include "include/veu_base_addr.h"

static void veu_hw_2dscaler_enable(struct veu_data *veu_dev, int enable)
{
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + SCL_EN_OFFSET, enable);
}

static void veu_hw_2dscaler_stripe_size_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_cfg *scaler)
{
	int val = 0;

	val = (scaler->stripe_height_in << STRIPE_HEIGHT_IN_SHIFT) +
			(scaler->stripe_width_in << STRIPE_WIDTH_IN_SHIFT);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + STRIPE_HEIGHT_IN_OFFSET, val);
	VEU_DBG("2d scaler input stripe height: %d, width: %d\n",
			scaler->stripe_height_in, scaler->stripe_width_in);

	val = (scaler->stripe_height_out << STRIPE_HEIGHT_OUT_SHIFT) +
			(scaler->stripe_width_out << STRIPE_WIDTH_OUT_SHIFT);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + STRIPE_HEIGHT_OUT_OFFSET, val);
	VEU_DBG("2d scaler output stripe height: %d, width: %d\n",
			scaler->stripe_height_out, scaler->stripe_width_out);

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + Y_STEP_OFFSET, scaler->y_step);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + X_STEP_OFFSET, scaler->x_step);
}

static void veu_hw_2dscaler_gauss_kernel_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_gauss_kernel *gauss_kernel)
{
	int i;

	for (i = 0; i < GAUSS_KERNEL_NUM_5X5; i++) {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				VEU_2DSCL_BASE + ARRAY_GAUSS_KERNEL_0_OFFSET + (i << 2),
				gauss_kernel->gauss_kernel_5x5[i]);
	}

	for (i = 0; i < GAUSS_KERNEL_NUM_9X9; i++) {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				VEU_2DSCL_BASE + ARRAY_GAUSS_KERNEL_1_OFFSET + (i << 2),
				gauss_kernel->gauss_kernel_9x9[i]);
	}
}

static void veu_hw_2dscaler_detail_enhance_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_cfg *cfg)
{
	int val = 0;

	val = (cfg->sigmoid_val0 << SIGMOID_VAL0_SHIFT) +
			(cfg->sigmoid_val1 << SIGMOID_VAL1_SHIFT) +
			(cfg->sigmoid_val2 << SIGMOID_VAL2_SHIFT);

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + SIGMOID_VAL0_OFFSET, val);

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + BASE_THRESHOLD_OFFSET, cfg->base_threshold);

	veu_hw_2dscaler_gauss_kernel_set(veu_dev, &(cfg->gauss_kernel));
}

static void veu_hw_2dscaler_outlier_remove_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_cfg *cfg)
{
	int val = 0;

	val = (cfg->outlier_threshold1 << OUTLIER_THRESHOLD1_SHIFT) +
			(cfg->outlier_threshold2 << OUTLIER_THRESHOLD2_SHIFT) +
			(cfg->threshold_number << THRESHOLD_NUMBER_SHIFT);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + OUTLIER_THRESHOLD1_OFFSET, val);
}

static void veu_hw_2dscaler_bicubic_kernel_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_bicubic_kernel *data)
{
	int val, tmp;
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
		val += tmp << 0;

		tmp = 0;
		tmp |= (data->bicubic_kernel_1_2[i] & 0x7FF) |
				(data->bicubic_kernel_1_2[i] >> 15 & 0x800);
		val += tmp << 12;

		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				VEU_2DSCL_BASE + ARRAY_BICUBIC_KERNEL_COMBINE_0_OFFSET + (i << 2),
				val);
	}
}

static void veu_hw_2dscaler_lanczos3_kernel_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_lanczos3_kernel *data)
{
	int val, tmp;
	int i;

	for (i = 0; i < SCALER_2D_KERNEL_LUT_NUM; i++) {
		val = 0;
		/**
		 * lanczos3_kernel_0_5: [9 : 0] is signed in complement form,
		 * lanczos3_kernel_1_4: [10 : 0] is signed in complement form,
		 * lanczos3_kernel_2_3: [10 : 0] is unsigned;
		 * combine them according to bit field: 0_5: [09 : 00],
		 * 1_4: [21 : 10], 2_3: [31 : 22]
		 */
		tmp = 0;
		tmp = (data->lanczos3_kernel_0_5[i] & 0x1FF) |
				(data->lanczos3_kernel_0_5[i] >> 15 & 0x200);
		val += tmp << 0;
		tmp = 0;
		tmp = (data->lanczos3_kernel_1_4[i] & 0x3FF) |
				(data->lanczos3_kernel_1_4[i] >> 15 & 0x400);
		val += tmp << 10;
		tmp = 0;
		tmp = (data->lanczos3_kernel_2_3[i] & 0x7FF);
		val += tmp << 21;

		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				VEU_2DSCL_BASE + ARRAY_LANCZOS3_KERNEL_COMBINE_0_OFFSET + (i << 2),
				val);
	}
}

static void veu_hw_2dscaler_stripe_set(struct veu_data *veu_dev,
		struct dpu_2d_scaler_cfg *scaler)
{
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + STRIPE_INIT_PHASE_OFFSET,
			scaler->stripe_init_phase);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + CROP_WIDTH_OUT_PHASE_OFFSET,
			scaler->crop_width_out_phase);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + CROP_HEIGHT_OUT_PHASE_OFFSET,
			scaler->crop_height_out_phase);
}

static void veu_hw_2dscaler_set(struct veu_data *veu_dev)
{
	struct veu_2dscaler_param *scaler_param = NULL;
	struct dpu_2d_scaler_cfg scaler;

	if (veu_dev->veu_stripe == NULL) {
		VEU_ERR("invalid input param\n");
		return;
	}

	scaler_param = &veu_dev->veu_stripe->stripe_param[0].scaler;
	if (scaler_param == NULL) {
		VEU_ERR("invalid scaler param\n");
		return;
	}

	scaler = scaler_param->scaler_cfg;

	veu_hw_2dscaler_stripe_size_set(veu_dev, &scaler);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + ALPHA_DISABLE_OFFSET,
			scaler.alpha_disable);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + GRADIENT_THRESHOLD_OFFSET,
			scaler.gradient_threshold);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + INTERP_METHOD_OFFSET,
			scaler.interp_method);

	if (scaler.interp_method == SCALER_2D_LANCZOS3)
		veu_hw_2dscaler_lanczos3_kernel_set(veu_dev, &scaler.lanczos3_kernel);
	else
		veu_hw_2dscaler_bicubic_kernel_set(veu_dev, &scaler.bicubic_kernel);

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			VEU_2DSCL_BASE + DETAIL_ENHANCE_ENABLE_OFFSET,
			scaler.detail_enhance_enable);
	if (scaler.detail_enhance_enable) {
		veu_hw_2dscaler_detail_enhance_set(veu_dev, &scaler);

		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				VEU_2DSCL_BASE + OUTLIER_REMOVE_ENABLE_OFFSET,
				scaler.outlier_remove_enable);
		if (scaler.outlier_remove_enable)
			veu_hw_2dscaler_outlier_remove_set(veu_dev, &scaler);
	}

	veu_hw_2dscaler_stripe_set(veu_dev, &scaler);
}

static void veu_hw_2dscaler_flush(struct veu_data *veu_dev)
{
	outp32(veu_dev->base + VEU_2DSCL_BASE + DPU_2DSCL_FORCE_UPDATE_EN_OFFSET,
			2);
}

void veu_2dscaler_set_reg(struct veu_data *veu_dev)
{
	if (veu_dev == NULL) {
		VEU_ERR("invalid input param\n");
		return;
	}

	if (veu_dev->veu_stripe == NULL) {
		VEU_ERR("invalid input param\n");
		return;
	}

	veu_hw_2dscaler_enable(veu_dev,
			veu_dev->veu_stripe->stripe_param[VEU_BASE_NODE_INDEX].scaler.scaler_en);

	if (veu_dev->veu_stripe->stripe_param[VEU_BASE_NODE_INDEX].scaler.scaler_en)
		veu_hw_2dscaler_set(veu_dev);

	veu_hw_2dscaler_flush(veu_dev);
}
