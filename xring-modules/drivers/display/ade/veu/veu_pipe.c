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
#include "veu_utils.h"
#include "veu_defs.h"
#include "veu_format.h"
#include "veu_enum.h"
#include "include/veu_hw_pipe_reg.h"
#include "include/veu_base_addr.h"

static int r2y_matrix_8b[COLOR_SPACE_NUM][3][4] = {
	/* YUVCSC_BT601_WIDE */
	{
		{1224,  2403,  469,	0},
		{-691, -1357, 2048,  128},
		{2048, -1714, -334,  128},
	},
	/* YUVCSC_BT_601_NARROW */
	{
		{1051,  2064,  403,   16},
		{-607, -1192, 1799,  128},
		{1799, -1505, -294,  128},
	},
	/* YUVCSC_BT709_WIDE */
	{
		{871,  2929,  296,	0},
		{-469, -1579, 2048,  128},
		{2048, -1860, -188,  128},
	},
	/* YUVCSC_BT709_NARROW */
	{
		{748, 2516,  254,   16},
		{-412, -1387, 1799,  128},
		{1799, -1634, -165,  128},
	},
	/* YUVCSC_BT2020_WIDE */
	{
		{1076,  2777,  243,	0},
		{-572, -1476, 2048,  128},
		{2048, -1883, -165,  128},
	},
	/* YUVCSC_BT_2020_NARROW */
	{
		{924,  2385,  209,   16},
		{-502, -1297, 1799,  128},
		{1799, -1654, -145,  128},
	},
	/* YUVCSC_P3_WIDE */
	{
		{4787,	 0,  6901, -3750},
		{4787,  -770, -2674,  1423},
		{4787,  8805,	 0, -4702},
	},
	/* YUVCSC_P3_NARROW */
	{
		{4787,	 0,  6901, -3750},
		{4787,  -770, -2674,  1423},
		{4787,  8805,	 0, -4702},
	},
};

static int r2y_matrix_10b[COLOR_SPACE_NUM][3][4] = {
	/* YUVCSC_BT601_WIDE */
	{
		{1224,  2403,  469,	0},
		{-691, -1357, 2048,  512},
		{2048, -1714, -334,  512},
	},
	/* YUVCSC_BT_601_NARROW */
	{
		{1048,  2058,  402,   64},
		{-605, -1188, 1794,  512},
		{1794, -1501, -293,  512},
	},
	/* YUVCSC_BT709_WIDE */
	{
		{871,  2929,  296,	0},
		{-469, -1579, 2048,  512},
		{2048, -1860, -188,  512},
	},
	/* YUVCSC_BT709_NARROW */
	{
		{746, 2508,  253,   64},
		{-411, -1383, 1794,  512},
		{1794, -1629, -164,  512},
	},
	/* YUVCSC_BT2020_WIDE */
	{
		{1076,  2777,  243,	0},
		{-572, -1476, 2048,  512},
		{2048, -1883, -165,  512},
	},
	/* YUVCSC_BT_2020_NARROW */
	{
		{921,  2378,  208,   64},
		{-501, -1293, 1794,  512},
		{1794, -1650, -144,  512},
	},
	/* YUVCSC_P3_WIDE */
	{
		{4787,	 0,  6901, -3750},
		{4787,  -770, -2674,  1423},
		{4787,  8805,	 0, -4702},
	},
	/* YUVCSC_P3_NARROW */
	{
		{4787,	 0,  6901, -3750},
		{4787,  -770, -2674,  1423},
		{4787,  8805,	 0, -4702},
	},
};

static int y2r_matrix[COLOR_SPACE_NUM][3][4] = {
	/* BT601_FULL */
	{
		{4096, 0, 5743, -718},
		{4096, -1416, -2926, 543},
		{4096, 7254, 0, -907},
	},
	/* BT601_LIMITED */
	{
		{4783, 0, 6557, -894},
		{4783, -1616, -3341, 545},
		{4783, 8282, 0, -1110},
	},
	/* BT709_FULL */
	{
		{4096,    0,  6450, -806},
		{4096, -767, -1918,  336},
		{4096, 7601,     0, -950},
	},
	/* BT709_LIMITED */
	{
		{4783,    0,  7364,  -995},
		{4783, -876, -2190,   308},
		{4783, 8678,     0, -1159},
	},
	/* P3D65_FULL */
	{
		{4096, 0, 6316, -790},
		{4096, -864, -2091, 369},
		{4096, 7543, 0, -943},
	},
	/* P3D65_LIMITED */
	{
		{4783, 0, 7212, -976},
		{4783, -987, -2387, 347},
		{4783, 8612, 0, -1151},
	},
	/* BT2020_FULL */
	{
		{4096,    0,  6040, -755},
		{4096, -674, -2340,  377},
		{4096, 7706,     0, -963},
	},
	/* BT2020_LIMITED */
	{
		{4783, 0, 6896, -937},
		{4783, -769, -2672, 355},
		{4783, 8799, 0, -1175},
	},
};

static void veu_scaler_pipe_config(struct veu_data *veu_dev, struct veu_stripe_param *stripe_param)
{
	struct veu_pipe_param *pipe_param = &veu_dev->pipe_param;

	pipe_param->scaler_en = stripe_param->scaler.scaler_en;

	if (pipe_param->scaler_en && !pipe_param->is_yuv_in) {
		pipe_param->pipe_en |= R2Y_EN;
		pipe_param->pipe_en |= PREALPHA_EN;
		pipe_param->pipe_en |= NONPREALPHA_EN;
	}
	if (pipe_param->scaler_en || pipe_param->is_yuv_in)
		pipe_param->pipe_en |= Y2R_EN;
}

/* veu pipe csc + dfc module */
static void veu_dat_cvt_config(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pipe_param = &veu_dev->pipe_param;

	if (pipe_param->is_yuv_in)
		pipe_param->pipe_en |= Y2R_EN;

	if (pipe_param->format_out == WB_PIXEL_FORMAT_NV12_10 ||
		pipe_param->format_out == WB_PIXEL_FORMAT_NV21_10)
		pipe_param->is_yuv_10bit = true;
	else
		pipe_param->is_yuv_10bit = false;
}

static void veu_crop1_config(struct veu_data *veu_dev, struct veu_stripe_param *param)
{
	struct display_wb_stripe_info *wb_stripe = &param->wb_stripe_param;
	struct veu_pipe_param *pipe_param = &veu_dev->pipe_param;
	struct veu_rect_ltrb in_rect;
	struct veu_rect_ltrb out_rect;

	if (pipe_param->scaler_en)
		in_rect = stripe_rect_to_veu_rect_ltrb(wb_stripe->wb_scaler_param.scaler_out_rect);
	else
		in_rect = stripe_rect_to_veu_rect_ltrb(wb_stripe->wb_input_rect);

	veu_rect_clear(&out_rect);
	if (param->wb_stripe_param.wb_crop0_enable) {
		pipe_param->pipe_en |= CROP1_EN;
		out_rect = stripe_rect_to_veu_rect_ltrb(param->wb_stripe_param.wb_crop0_rect);
	} else {
		pipe_param->pipe_en &= ~CROP1_EN;
		out_rect.right = in_rect.right;
		out_rect.bottom = in_rect.bottom;
	}

	// rgb data dont need 2 align
	if (is_yuv420_wformat(pipe_param->format_out)) {
		int width = in_rect.right - in_rect.left + 1;
		int height = in_rect.bottom - in_rect.top + 1;

		if ((width % 2) != 0) {
			out_rect.right--;
			pipe_param->pipe_en |= CROP1_EN;
		}
		if ((height % 2) != 0) {
			out_rect.bottom--;
			pipe_param->pipe_en |= CROP1_EN;
		}
	}

	pipe_param->crop1_in = in_rect;
	pipe_param->crop1_out = out_rect;
}

static void veu_crop2_config(struct veu_data *veu_dev,
		struct veu_stripe_param *param)
{
	struct veu_pipe_param *pipe_param = &veu_dev->pipe_param;
	struct veu_rect_ltrb in_rect = pipe_param->crop1_out;
	struct display_wb_stripe_info *wb_stripe = NULL;
	struct veu_rect_ltrb out_rect;
	u32 height;
	u32 width;

	wb_stripe = &param->wb_stripe_param;

	if (wb_stripe->wb_crop0_enable)
		in_rect = stripe_rect_to_veu_rect_ltrb(wb_stripe->wb_crop0_rect);
	else
		in_rect = pipe_param->crop1_in;

	veu_rect_clear(&out_rect);
	if (wb_stripe->wb_crop2_enable) {
		pipe_param->pipe_en |= CROP2_EN;
		out_rect = stripe_rect_to_veu_rect_ltrb(wb_stripe->wb_crop2_rect);
	} else {
		pipe_param->pipe_en &= ~CROP2_EN;
		out_rect.right = in_rect.right;
		out_rect.bottom = in_rect.bottom;
	}

	width = out_rect.right - out_rect.left + 1;
	height = out_rect.bottom - out_rect.top + 1;

	if (is_yuv420_wformat(pipe_param->format_out)) {
		if ((width % 2) != 0) {
			out_rect.right--;
			pipe_param->pipe_en |= CROP2_EN;
		}
		if ((height % 2) != 0) {
			out_rect.bottom--;
			pipe_param->pipe_en |= CROP2_EN;
		}
		if ((out_rect.left % 2) != 0) {
			out_rect.left++;
			out_rect.right--;
			pipe_param->pipe_en |= CROP2_EN;
		}
		if ((out_rect.top % 2) != 0) {
			out_rect.top++;
			out_rect.bottom--;
			pipe_param->pipe_en |= CROP2_EN;
		}
	}

	pipe_param->crop2_in = in_rect;
	pipe_param->crop2_out = out_rect;
}

static void veu_dither_config(struct veu_data *veu_dev,
		struct veu_dither_param *dither)
{
	struct veu_pipe_param *pipe_param = NULL;

	pipe_param = &veu_dev->pipe_param;

	if (dither->dither_en) {
		pipe_param->pipe_en |= DITHER_EN;
		pipe_param->dither_cfg = dither->dither_cfg;
	}
};

int veu_pipe_config(struct veu_data *veu_dev, struct veu_layer *inputlayer,
		struct veu_layer *outputlayer)
{
	const struct veu_format_map *format_map = NULL;
	struct veu_stripe_param *stripe_param = NULL;
	struct veu_pipe_param *pipe_param = NULL;

	VEU_DBG("enter");

	veu_check_and_return(!veu_dev, -1, "veu_dev is null");
	veu_check_and_return(!inputlayer, -1, "inputlayer is null");
	veu_check_and_return(!outputlayer, -1, "outputlayer is null");
	veu_check_and_return(!veu_dev->veu_stripe, -1, "veu stripe is null");

	pipe_param = &veu_dev->pipe_param;

	stripe_param = &veu_dev->veu_stripe->stripe_param[0];

	pipe_param->pipe_en = 0;

	format_map = veu_format_get(MODULE_RDMA, inputlayer->format, inputlayer->afbc_used);
	pipe_param->format_in = format_map->veu_format;
	pipe_param->is_yuv_in = format_map->is_yuv;

	if (stripe_param->scaler.scaler_en && !pipe_param->is_yuv_in)
		pipe_param->scaler_en = true;

	format_map = veu_format_get(MODULE_WDMA, outputlayer->format, outputlayer->afbc_used);
	pipe_param->format_out = format_map->veu_format;
	pipe_param->is_yuv_out = format_map->is_yuv;
	pipe_param->color_space = outputlayer->color_space;

	veu_scaler_pipe_config(veu_dev, stripe_param);

	veu_dither_config(veu_dev, &stripe_param->dither);

	veu_crop1_config(veu_dev, stripe_param);

	veu_dat_cvt_config(veu_dev);

	veu_crop2_config(veu_dev, stripe_param);

	return 0;
}

static int veu_dither_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pparam = NULL;
	int dither_mode_set = 0;
	int map_val;
	int i;

	pparam = &veu_dev->pipe_param;

	if (pparam->pipe_en & DITHER_EN) {
		dither_mode_set = (pparam->dither_cfg.mode <<
				DITHER_MODE_SHIFT) +
				(pparam->dither_cfg.dither_out_depth0 <<
				DITHER_OUT_DPTH0_SHIFT) +
				(pparam->dither_cfg.dither_out_depth1 <<
				DITHER_OUT_DPTH1_SHIFT) +
				(pparam->dither_cfg.dither_out_depth2 <<
				DITHER_OUT_DPTH2_SHIFT);

		if (pparam->dither_cfg.mode == DITHER_MODE_PATTERN) {
			dither_mode_set = dither_mode_set +
					(pparam->dither_cfg.autotemp <<
					DITHER_AUTO_TEMP_SHIFT) +
					(pparam->dither_cfg.rotatemode <<
					DITHER_ROTATE_MODE_SHIFT) +
					(pparam->dither_cfg.temporalvalue <<
					DITHER_TEMP_VALUE_SHIFT) +
					(pparam->dither_cfg.patternbits <<
					DITHER_PATTERN_BIT_SHIFT);

			for (i = 0; i < DITHER_BAYER_MAP_SIZE; i = i + 4) {
				map_val = (pparam->dither_cfg.bayermap[i] <<
						DITHER_BAYER_MAP00_SHIFT) +
						(pparam->dither_cfg.bayermap[i + 1] <<
						DITHER_BAYER_MAP01_SHIFT) +
						(pparam->dither_cfg.bayermap[i + 2] <<
						DITHER_BAYER_MAP02_SHIFT) +
						(pparam->dither_cfg.bayermap[i + 3] <<
						DITHER_BAYER_MAP03_SHIFT);

				veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
						PIPE_TOP + DITHER_BAYER_MAP00_OFFSET + i, map_val);
			}
		}

		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + DITHER_MODE_OFFSET, dither_mode_set);
	}

	return 0;
}

static void veu_scaler_pipe_set_reg(struct veu_data *veu_dev)
{
	struct veu_layer *layer = &(veu_dev->wdma_param.layer);
	int scaler_out_h, scaler_out_w;
	int value;

	if (!veu_dev->pipe_param.scaler_en)
		return;

	scaler_out_h = layer->dst_rect.bottom - layer->dst_rect.top + 1;
	scaler_out_w = layer->dst_rect.right - layer->dst_rect.left + 1;
	value = (scaler_out_h << M_SCALE_HEIGHT_SHIFT) + scaler_out_w;
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + M_SCALE_WIDTH_OFFSET,
			value);
}

static void config_cvt_r2y_matrix(struct veu_data *veu_dev, int *matrix)
{
	int i;

	for (i = 0; i < (CSC_MATRIX_TABLE_SIZE >> 1); i++) {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + FMT_CVT_RGB2YUV_MATRIX00_OFFSET + (i << 2),
				*(matrix + (i << 1)) +
				(*(matrix + (i << 1) + 1) << FMT_CVT_RGB2YUV_MATRIX01_SHIFT));
	}
}

static int veu_crop1_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pparam = &veu_dev->pipe_param;
	int crop1_outx, crop1_outy;
	int crop1_insize;

	if (pparam->pipe_en & CROP1_EN) {
		crop1_insize =
			((pparam->crop1_in.bottom - pparam->crop1_in.top + 1) <<
			MVSIZE_CROP1_SHIFT) +
			(pparam->crop1_in.right - pparam->crop1_in.left + 1);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + MHSIZE_CROP1_OFFSET, crop1_insize);

		crop1_outx = (pparam->crop1_out.right << CROP1_X_RIGHT_SHIFT) +
			pparam->crop1_out.left;
		crop1_outy = (pparam->crop1_out.bottom << CROP1_Y_BOT_SHIFT) +
			pparam->crop1_out.top;
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + CROP1_X_LEFT_OFFSET,
			crop1_outx);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + CROP1_Y_TOP_OFFSET,
			crop1_outy);
	}

	crop1_insize = ((pparam->crop1_in.bottom - pparam->crop1_in.top + 1) <<
		MVSIZE_CROP1_SHIFT) + (pparam->crop1_in.right - pparam->crop1_in.left + 1);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + MHSIZE_CROP1_OFFSET,
		crop1_insize);
	return 0;
}

static void veu_dat_cvt_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pipe_param = &veu_dev->pipe_param;

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + YUV_CVT_VER_COEF_OFFSET,
			(pipe_param->format_out << DAT_CVT_OUT_FORMAT_SHIFT));

	if (pipe_param->is_yuv_out) {
		if (pipe_param->is_yuv_10bit)
			config_cvt_r2y_matrix(veu_dev, &r2y_matrix_10b[pipe_param->color_space][0][0]);
		else
			config_cvt_r2y_matrix(veu_dev, &r2y_matrix_8b[pipe_param->color_space][0][0]);
	}
}

static int veu_crop2_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pparam = &veu_dev->pipe_param;
	int crop2_outx, crop2_outy;
	int crop2_insize;

	if (pparam->pipe_en & CROP2_EN) {
		crop2_outx = (pparam->crop2_out.right << CROP2_X_RIGHT_SHIFT) +
			pparam->crop2_out.left;
		crop2_outy = (pparam->crop2_out.bottom << CROP2_Y_BOT_SHIFT) +
			pparam->crop2_out.top;
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + CROP2_X_LEFT_OFFSET, crop2_outx);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + CROP2_Y_TOP_OFFSET, crop2_outy);
	}

	crop2_insize = ((pparam->crop2_in.bottom - pparam->crop2_in.top + 1) <<
		MVSIZE_CROP2_SHIFT) + (pparam->crop2_in.right - pparam->crop2_in.left + 1);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + MHSIZE_CROP2_OFFSET,
		crop2_insize);

	return 0;
}

static void veu_r2y_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pparam = &veu_dev->pipe_param;
	int i;

	if (pparam->pipe_en & R2Y_EN) {
		int *matrix = &r2y_matrix_10b[pparam->color_space][0][0];

		for (i = 0; i < (CSC_MATRIX_TABLE_SIZE >> 1); i++) {
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
					PIPE_TOP + RGB2YUV_MATRIX00_OFFSET + (i << 2),
					*(matrix + (i << 1)) +
					(*(matrix + (i << 1) + 1) << RGB2YUV_MATRIX01_SHIFT));
		}
	}
}

static void veu_y2r_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pparam = &veu_dev->pipe_param;
	int i;

	if (pparam->pipe_en & Y2R_EN) {
		int *matrix = &y2r_matrix[pparam->color_space][0][0];

		for (i = 0; i < (CSC_MATRIX_TABLE_SIZE >> 1); i++) {
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
					PIPE_TOP + YUV2RGB_MATRIX00_OFFSET + (i << 2),
					*(matrix + (i << 1)) +
					(*(matrix + (i << 1) + 1) << YUV2RGB_MATRIX01_SHIFT));
		}
	}
}

static void veu_non_prealpha_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pparam = &veu_dev->pipe_param;
	int offset1, offset2, offset3;

	if (pparam->pipe_en & NONPREALPHA_EN) {
		offset1 = r2y_matrix_10b[pparam->color_space][0][3];
		offset2 = r2y_matrix_10b[pparam->color_space][1][3];
		offset3 = r2y_matrix_10b[pparam->color_space][2][3];

		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + NONPREALPHA_OFFSET1_OFFSET,
				offset1 + (offset2 << NONPREALPHA_OFFSET2_SHIFT));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + NONPREALPHA_OFFSET3_OFFSET, offset3);
	}
}

void veu_pipe_set_reg(struct veu_data *veu_dev)
{
	struct veu_pipe_param *pipe_param = &veu_dev->pipe_param;

	VEU_DBG("enter");

	outp32(veu_dev->base + PIPE_TOP + VEU_PIPE_REG_TRIGGER2_OFFSET, 0x1);

	veu_r2y_set_reg(veu_dev);
	veu_scaler_pipe_set_reg(veu_dev);
	veu_non_prealpha_set_reg(veu_dev);
	veu_y2r_set_reg(veu_dev);
	veu_dither_set_reg(veu_dev);

	veu_crop1_set_reg(veu_dev);
	veu_dat_cvt_set_reg(veu_dev);
	veu_crop2_set_reg(veu_dev);

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP + 0x10, 0);

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, PIPE_TOP, pipe_param->pipe_en);

	if (pipe_param->is_yuv_in)
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + DATA_FORMAT_OFFSET, BIT(1));
	else
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				PIPE_TOP + DATA_FORMAT_OFFSET, BIT(0));

	outp32(veu_dev->base + PIPE_TOP + VEU_PIPE_REG_FORCE_UPDATE_EN_OFFSET,
		BIT(1));

	VEU_DBG("exit");
}
