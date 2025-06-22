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

#ifndef _DPU_HW_FORMAT_H
#define _DPU_HW_FORMAT_H

enum dpu_rch_format {
	/* RGB format definition */
	RCH_FORMAT_ARGB_2101010 = 0,
	RCH_FORMAT_ABGR_2101010,
	RCH_FORMAT_RGBA_1010102,
	RCH_FORMAT_BGRA_1010102,
	RCH_FORMAT_ARGB_8888,
	RCH_FORMAT_ABGR_8888,
	RCH_FORMAT_RGBA_8888,
	RCH_FORMAT_BGRA_8888,
	RCH_FORMAT_XRGB_8888,
	RCH_FORMAT_XBGR_8888,
	RCH_FORMAT_RGBX_8888,
	RCH_FORMAT_BGRX_8888,
	RCH_FORMAT_RGB_888,
	RCH_FORMAT_BGR_888,
	RCH_FORMAT_RGBA_5551,
	RCH_FORMAT_BGRA_5551,
	RCH_FORMAT_ABGR_1555,
	RCH_FORMAT_ARGB_1555,
	RCH_FORMAT_RGBX_5551,
	RCH_FORMAT_BGRX_5551,
	RCH_FORMAT_XBGR_5551,
	RCH_FORMAT_XRGB_5551,
	RCH_FORMAT_RGB_565,
	RCH_FORMAT_BGR_565,
	/* YUV format definition */
	RCH_FORMAT_XYUV_444_P1_8 = 32,
	RCH_FORMAT_XYUV_444_P1_10,
	RCH_FORMAT_VYUY_422_P1_8,
	RCH_FORMAT_YVYU_422_P1_8,
	RCH_FORMAT_RESERVE_0,
	RCH_FORMAT_YUV_420_P2_8,
	RCH_FORMAT_YUV_420_P3_8,
	RCH_FORMAT_RESERVE_1,
	RCH_FORMAT_YUV_420_P2_10,
	RCH_FORMAT_YUV_420_P3_10,
	RCH_FORMAT_INVALID = 0xff,
};

enum dpu_wb_format {
	/* RGB format definition */
	WB_FORMAT_ARGB_2101010 = 0,
	WB_FORMAT_ABGR_2101010,
	WB_FORMAT_RGBA_1010102,
	WB_FORMAT_BGRA_1010102,
	WB_FORMAT_ARGB_8888,
	WB_FORMAT_ABGR_8888,
	WB_FORMAT_RGBA_8888,
	WB_FORMAT_BGRA_8888,
	WB_FORMAT_XRGB_8888,
	WB_FORMAT_XBGR_8888,
	WB_FORMAT_RGBX_8888,
	WB_FORMAT_BGRX_8888,
	WB_FORMAT_RGB_888,
	WB_FORMAT_BGR_888,
	WB_FORMAT_RGBA_5551,
	WB_FORMAT_BGRA_5551,
	WB_FORMAT_ABGR_1555,
	WB_FORMAT_ARGB_1555,
	WB_FORMAT_RGBX_5551,
	WB_FORMAT_BGRX_5551,
	WB_FORMAT_XBGR_5551,
	WB_FORMAT_XRGB_5551,
	WB_FORMAT_RGB_565,
	WB_FORMAT_BGR_565,
	/* YUV format definition */
	WB_FORMAT_NV21_8,
	WB_FORMAT_NV12_8,
	WB_FORMAT_NV21_10,
	WB_FORMAT_NV12_10,
	WB_FORMAT_INVALID = 0xff,
};

/* pack data format of post pipe top */
enum pack_data_format {
	POSTPIPE_PACK_FMT_RGB666 = 0,
	POSTPIPE_PACK_FMT_RGB888,
	POSTPIPE_PACK_FMT_RGB101010,
	POSTPIPE_PACK_FMT_DSC_BYTE,
	POSTPIPE_PACK_FMT_MAX,
};

static inline u32 get_bit_per_pixel(u32 format)
{
	u32 bit_per_pixel= 24;

	switch (format) {
	case POSTPIPE_PACK_FMT_RGB888:
		bit_per_pixel= 24;
		break;
	case POSTPIPE_PACK_FMT_RGB101010:
		bit_per_pixel= 30;
		break;
	case POSTPIPE_PACK_FMT_DSC_BYTE:
		bit_per_pixel= 8;
		break;
	default:
		break;
	}
	return bit_per_pixel;
}

#endif