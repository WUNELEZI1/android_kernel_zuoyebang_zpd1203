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

#ifndef _DPU_HW_COLOR_H_
#define _DPU_HW_COLOR_H_

#include "dpu_hw_common.h"

/**
 * dpu_crtc_postpq_features - list of crtc features in post pq
 */
enum dpu_crtc_postpq_features {
	/* Append new post pq features before DPU_CRTC_POSTPQ_MAX_FEATURES */
	DPU_CRTC_POSTPQ_LTM_EN,
	DPU_CRTC_POSTPQ_LTM,
	DPU_CRTC_POSTPQ_R2Y_EN,
	DPU_CRTC_POSTPQ_R2Y,
	DPU_CRTC_POSTPQ_SCALER_EN,
	DPU_CRTC_POSTPQ_SCALER,
	DPU_CRTC_POSTPQ_DITHER_LITE_EN,
	DPU_CRTC_POSTPQ_Y2R_EN,
	DPU_CRTC_POSTPQ_Y2R,
	DPU_CRTC_POSTPQ_MUL_MATRIX_EN,
	DPU_CRTC_POSTPQ_MUL_MATRIX,
	DPU_CRTC_POSTPQ_3DLUT_EN,
	DPU_CRTC_POSTPQ_3DLUT,
	DPU_CRTC_POSTPQ_CSC_MATRIX_EN,
	DPU_CRTC_POSTPQ_CSC_MATRIX,
	DPU_CRTC_POSTPQ_USR_GAMMA_EN,
	DPU_CRTC_POSTPQ_USR_GAMMA,
	DPU_CRTC_POSTPQ_DITHER_EN,
	DPU_CRTC_POSTPQ_DITHER,
	DPU_CRTC_POSTPQ_ACAD_EN,
	DPU_CRTC_POSTPQ_ACAD,
	DPU_CRTC_POSTPQ_HIST_READ_MODE,
	DPU_CRTC_POSTPQ_HIST,
	DPU_CRTC_POSTPQ_RC_REGION,
	DPU_CRTC_POSTPQ_RC,
	DPU_CRTC_POSTPQ_FEATURE_MAX,
};

enum dpu_plane_color_features {
	PLANE_COLOR_SCALER_EN = 0,
	PLANE_COLOR_SCALER,
	PLANE_COLOR_PREALPHA_EN,
	PLANE_COLOR_NON_PREALPHA_EN,
	PLANE_COLOR_NON_PREALPHA,
	PLANE_COLOR_R2Y_EN,
	PLANE_COLOR_R2Y,
	PLANE_COLOR_Y2R_EN,
	PLANE_COLOR_Y2R,
	PLANE_COLOR_ALPHA_EN,
	PLANE_COLOR_TM_EN,
	PLANE_COLOR_TM,
	PLANE_COLOR_LUT3D_EN,
	PLANE_COLOR_LUT3D,
	PLANE_COLOR_HIST_READ_MODE,
	PLANE_COLOR_HIST,
	PLANE_COLOR_DITHER_LITE_EN,
	PLANE_COLOR_PROP_MAX,
};

/**
 * postpq_hw_cfg - the property node data structure
 * @data: the payload of bolb property
 * @len: length of payload
 * @value: property value
 * @lm_width: width of layer mixer;
 * @lm_height: height of layer mixer;
 * @panel_width: panel's horizontal resolution
 * @panel_height: panel's vertical resolution
 */
struct postpq_hw_cfg {
	void *data;
	u32 len;
	u64 value;
	u32 lm_width;
	u32 lm_height;
	u32 panel_width;
	u32 panel_height;
};

struct postpq_feature_ctrl {
	u32 enabled_features;
	bool lp_enable;
};

struct postpq_hw_init_cfg {
	u32 hdisplay;
	u32 vdisplay;
};

/**
 * prepq_property_node - the property node data structure
 * @data: the payload of bolb property
 * @len: length of payload
 */
struct prepq_hw_cfg {
	void *data;
	u32 len;
};

struct prepq_feature_ctrl {
	u32 enabled_features;
	bool left_right_reuse;
};

#endif