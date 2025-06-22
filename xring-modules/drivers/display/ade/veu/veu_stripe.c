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
#include "veu_stripe.h"
#include "veu_utils.h"
#include "include/veu_base_addr.h"
#include "include/veu_hw_rdma_path_reg.h"
#include "include/veu_hw_wb_core_reg.h"
#include "include/veu_hw_2dscaler_reg.h"
#include "include/veu_hw_pipe_reg.h"

static int veu_stripe_rdma_config(struct veu_data *veu_dev,
		struct veu_stripe_param *stripe_param, int stripe_id)
{
	struct veu_rect_ltrb layer_src_rect;

	layer_src_rect = stripe_rect_to_veu_rect_ltrb(
			stripe_param->layer_stripe_param.layer_crop_rect);
	veu_dev->set_reg(veu_dev, stripe_id, RDMA_PATH + LEFT_BBOX_START_X_OFFSET,
		(layer_src_rect.left | (layer_src_rect.right << LEFT_BBOX_END_X_SHIFT)));
	veu_dev->set_reg(veu_dev, stripe_id, RDMA_PATH + LEFT_BBOX_START_Y_OFFSET,
		(layer_src_rect.top | (layer_src_rect.bottom << LEFT_BBOX_END_Y_SHIFT)));

	return 0;
}

static int veu_stripe_2d_scaler_config(struct veu_data *veu_dev,
		struct veu_stripe_param *stripe_param, int stripe_id)
{
	struct veu_2dscaler_param *scaler_param = NULL;
	uint32_t scaler_out_h, scaler_out_w;
	struct dpu_2d_scaler_cfg scaler;
	uint32_t value;

	if (!(veu_dev->pipe_param.scaler_en))
		return 0;
	scaler_param = &stripe_param->scaler;
	if (scaler_param == NULL) {
		VEU_ERR("invalid scaler param for stripe %d", stripe_id);
		return -1;
	}
	scaler = scaler_param->scaler_cfg;
	scaler_out_h = scaler.stripe_height_out;
	scaler_out_w = scaler.stripe_width_out;
	value = (scaler_out_h << M_SCALE_HEIGHT_SHIFT) + (scaler_out_w);
	veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + M_SCALE_WIDTH_OFFSET,
			value);
	value = (scaler.stripe_height_in << STRIPE_HEIGHT_IN_SHIFT)
			+ (scaler.stripe_width_in << STRIPE_WIDTH_IN_SHIFT);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + STRIPE_HEIGHT_IN_OFFSET, value);
	VEU_DBG("2d scaler input stripe height: %d, width: %d\n",
			scaler.stripe_height_in, scaler.stripe_width_in);
	value = (scaler.stripe_height_out << STRIPE_HEIGHT_OUT_SHIFT)
			+ (scaler.stripe_width_out << STRIPE_WIDTH_OUT_SHIFT);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + STRIPE_HEIGHT_OUT_OFFSET, value);
	VEU_DBG("2d scaler output stripe height: %d, width: %d\n",
			scaler.stripe_height_out, scaler.stripe_width_out);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + Y_STEP_OFFSET, scaler.y_step);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + X_STEP_OFFSET, scaler.x_step);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + STRIPE_INIT_PHASE_OFFSET,
			scaler.stripe_init_phase);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + CROP_WIDTH_OUT_PHASE_OFFSET,
			scaler.crop_width_out_phase);
	veu_dev->set_reg(veu_dev, stripe_id,
			VEU_2DSCL_BASE + CROP_HEIGHT_OUT_PHASE_OFFSET,
			scaler.crop_height_out_phase);

	return 0;
}

static void veu_stripe_pipe_crop1_config(struct veu_data *veu_dev,
		struct veu_pipe_param *pipe_param, struct veu_stripe_param *stripe_param,
		int stripe_id, struct veu_rect_ltrb *crop1_in)
{
	struct display_wb_stripe_info *wb_info = &stripe_param->wb_stripe_param;
	struct veu_rect_ltrb crop1_in_rect;
	struct veu_rect_ltrb crop1_out_rect;
	uint32_t crop1_outx, crop1_outy;
	uint32_t crop1_insize;

	if (pipe_param->scaler_en)
		crop1_in_rect =
			stripe_rect_to_veu_rect_ltrb(wb_info->wb_scaler_param.scaler_out_rect);
	else
		crop1_in_rect = stripe_rect_to_veu_rect_ltrb(wb_info->wb_input_rect);
	veu_rect_clear(&crop1_out_rect);
	if (wb_info->wb_crop0_enable) {
		crop1_out_rect = stripe_rect_to_veu_rect_ltrb(wb_info->wb_crop0_rect);
		pipe_param->pipe_en |= CROP1_EN;
	} else {
		crop1_out_rect.right = crop1_in_rect.right;
		crop1_out_rect.bottom = crop1_in_rect.bottom;
		pipe_param->pipe_en &= ~CROP1_EN;
	}
	if (wb_info->wb_crop0_enable) {
		crop1_insize =
			((crop1_in_rect.bottom - crop1_in_rect.top + 1) <<
			MVSIZE_CROP1_SHIFT) +
			(crop1_in_rect.right - crop1_in_rect.left + 1);
		veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + MHSIZE_CROP1_OFFSET,
			crop1_insize);
		crop1_outx = (crop1_out_rect.right << CROP1_X_RIGHT_SHIFT) +
			crop1_out_rect.left;
		crop1_outy = (crop1_out_rect.bottom << CROP1_Y_BOT_SHIFT) +
			crop1_out_rect.top;
		veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + CROP1_X_LEFT_OFFSET,
			crop1_outx);
		veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + CROP1_Y_TOP_OFFSET,
			crop1_outy);
	}
	crop1_insize = ((crop1_in_rect.bottom - crop1_in_rect.top + 1) <<
		MVSIZE_CROP1_SHIFT) + (crop1_in_rect.right - crop1_in_rect.left + 1);
	veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + MHSIZE_CROP1_OFFSET,
		crop1_insize);
	/* when crop1 disable, crop2 needs to config minsize = crop1_in_rect */
	*crop1_in = crop1_in_rect;
}

static void veu_stripe_pipe_crop2_config(struct veu_data *veu_dev,
		struct veu_pipe_param *pipe_param, struct veu_stripe_param *stripe_param,
		int stripe_id, struct veu_rect_ltrb crop1_in)
{
	struct display_wb_stripe_info *wb_stripe = NULL;
	struct veu_rect_ltrb crop2_out_rect;
	struct veu_rect_ltrb crop2_in_rect;
	int crop2_outx, crop2_outy;
	int crop2_insize;

	wb_stripe = &stripe_param->wb_stripe_param;
	if (wb_stripe->wb_crop0_enable)
		crop2_in_rect = stripe_rect_to_veu_rect_ltrb(wb_stripe->wb_crop0_rect);
	else
		crop2_in_rect = crop1_in;
	veu_rect_clear(&crop2_out_rect);
	if (wb_stripe->wb_crop2_enable) {
		crop2_out_rect = stripe_rect_to_veu_rect_ltrb(wb_stripe->wb_crop2_rect);
		pipe_param->pipe_en |= CROP2_EN;
	} else {
		crop2_out_rect.right = crop2_in_rect.right;
		crop2_out_rect.bottom = crop2_in_rect.bottom;
		pipe_param->pipe_en &= ~CROP2_EN;
	}
	if (wb_stripe->wb_crop2_enable) {
		crop2_outx = (crop2_out_rect.right << CROP2_X_RIGHT_SHIFT) +
			crop2_out_rect.left;
		crop2_outy = (crop2_out_rect.bottom << CROP2_Y_BOT_SHIFT) +
			crop2_out_rect.top;
		veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + CROP2_X_LEFT_OFFSET,
			crop2_outx);
		veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + CROP2_Y_TOP_OFFSET,
			crop2_outy);
	}
	crop2_insize = ((crop2_in_rect.bottom - crop2_in_rect.top + 1) <<
		MVSIZE_CROP2_SHIFT) + (crop2_in_rect.right - crop2_in_rect.left + 1);
	veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP + MHSIZE_CROP2_OFFSET,
		crop2_insize);
}

static int veu_stripe_pipe_config(struct veu_data *veu_dev,
		struct veu_stripe_param *stripe_param, int stripe_id)
{
	struct veu_pipe_param *pipe_param;
	struct veu_rect_ltrb crop1_in_rect;
	int ret;

	pipe_param = &veu_dev->pipe_param;
	ret = veu_stripe_2d_scaler_config(veu_dev, stripe_param, stripe_id);
	veu_check_and_return(ret, -1, "veu 2d-scaler stripe config error");
	veu_stripe_pipe_crop1_config(veu_dev, pipe_param, stripe_param, stripe_id,
			&crop1_in_rect);
	veu_stripe_pipe_crop2_config(veu_dev, pipe_param, stripe_param, stripe_id,
			crop1_in_rect);
	veu_dev->set_reg(veu_dev, stripe_id, PIPE_TOP, (BIT(11) | pipe_param->pipe_en));

	return 0;
}

static int veu_stripe_wdma_config(struct veu_data *veu_dev,
		struct veu_stripe_param *stripe_param, int stripe_id)
{
	uint32_t out_subframe_width, out_subframe_height;
	uint32_t out_subframe_ltopx, out_subframe_ltopy;
	struct veu_layer *layer;
	struct veu_rect_ltrb layer_src_rect;

	layer = &veu_dev->wdma_param.layer;
	layer_src_rect = stripe_rect_to_veu_rect_ltrb(
			stripe_param->wb_stripe_param.wb_output_rect);
	out_subframe_width = layer_src_rect.right - layer_src_rect.left + 1;
	out_subframe_height = layer_src_rect.bottom - layer_src_rect.top + 1;
	out_subframe_ltopx = layer_src_rect.left;
	out_subframe_ltopy = layer_src_rect.top;
	veu_dev->set_reg(veu_dev, stripe_id, WB_CORE + WB_OUT_SUBFRAME_WIDTH_OFFSET,
		((out_subframe_width << WB_OUT_SUBFRAME_WIDTH_SHIFT) +
		(out_subframe_height << WB_OUT_SUBFRAME_HEIGHT_SHIFT)));
	veu_dev->set_reg(veu_dev, stripe_id, WB_CORE + WB_OUT_SUBFRAME_LTOPX_OFFSET,
		((out_subframe_ltopx << WB_OUT_SUBFRAME_LTOPX_SHIFT) +
		(out_subframe_ltopy << WB_OUT_SUBFRAME_LTOPY_SHIFT)));

	return 0;
}

int veu_stripe_config(struct veu_data *veu_dev)
{
	struct veu_stripe_param *stripe_param = NULL;
	int ret;
	int i;

	veu_check_and_return(!veu_dev, -1, "veu_dev null");
	veu_check_and_return(!veu_dev->veu_stripe, -1, "veu stripe null");
	for (i = 1; i < veu_dev->veu_stripe->count; i++) {
		stripe_param = &(veu_dev->veu_stripe->stripe_param[i]);
		ret = veu_stripe_rdma_config(veu_dev, stripe_param, i);
		if (ret) {
			VEU_ERR("veu rdma stripe %d config error", i);
			return -1;
		}
		ret = veu_stripe_pipe_config(veu_dev, stripe_param, i);
		if (ret) {
			VEU_ERR("veu pipe stripe %d config error", i);
			return -1;
		}
		ret = veu_stripe_wdma_config(veu_dev, stripe_param, i);
		if (ret) {
			VEU_ERR("veu wdma stripe %d config error", i);
			return -1;
		}
	}

	return 0;
}
