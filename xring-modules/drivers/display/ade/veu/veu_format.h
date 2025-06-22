/* SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _VEU_FORMAT_H_
#define _VEU_FORMAT_H_

#include <linux/types.h>
#include <drm/drm_fourcc.h>

#include "veu_defs.h"
#include "veu_enum.h"

// format for rdma config
enum VEU_RDMA_PIXEL_FORMAT {
	// 10 bit
	VEU_PIXEL_FORMAT_ARGB2101010 = 0,
	VEU_PIXEL_FORMAT_ABGR2101010,
	VEU_PIXEL_FORMAT_RGBA1010102,
	VEU_PIXEL_FORMAT_BGRA1010102,

	VEU_PIXEL_FORMAT_ARGB8888 = 4,
	VEU_PIXEL_FORMAT_ABGR8888,
	VEU_PIXEL_FORMAT_RGBA8888,
	VEU_PIXEL_FORMAT_BGRA8888,
	VEU_PIXEL_FORMAT_XRGB8888,
	VEU_PIXEL_FORMAT_XBGR8888,
	VEU_PIXEL_FORMAT_RGBX8888,
	VEU_PIXEL_FORMAT_BGRX8888,
	VEU_PIXEL_FORMAT_RGB888,
	VEU_PIXEL_FORMAT_BGR888,
	VEU_PIXEL_FORMAT_RGBA5551,
	VEU_PIXEL_FORMAT_BGRA5551,
	VEU_PIXEL_FORMAT_ABGR1555,
	VEU_PIXEL_FORMAT_ARGB1555,
	VEU_PIXEL_FORMAT_RGBX5551,
	VEU_PIXEL_FORMAT_BGRX5551,
	VEU_PIXEL_FORMAT_XBGR1555,
	VEU_PIXEL_FORMAT_XRGB1555,
	VEU_PIXEL_FORMAT_RGB565,
	VEU_PIXEL_FORMAT_BGR565,
	VEU_PIXEL_FORMAT_ARGB_16161616, // unsupported
	VEU_PIXEL_FORMAT_ABGR_16161616, // unsupported
	VEU_PIXEL_FORMAT_BGRA_16161616, // unsupported
	VEU_PIXEL_FORMAT_RGBA_16161616, // unsupported

	// YUV
	VEU_PIXEL_FORMAT_XYUV_444_8 = 32, // unsupported
	VEU_PIXEL_FORMAT_XYUV_444_10, // unsupported
	VEU_PIXEL_FORMAT_VYUY_422_8,
	VEU_PIXEL_FORMAT_YVYU_422_8,
	VEU_PIXEL_FORMAT_RSVD, // NOT USED
	VEU_PIXEL_FORMAT_YUV420_P2_8 = 37, // NV12 NV21
	VEU_PIXEL_FORMAT_YUV420_P3_8,
	VEU_PIXEL_FORMAT_RSVD1, // NOT USED
	VEU_PIXEL_FORMAT_YUV420_P2_10 = 40,
	VEU_PIXEL_FORMAT_YUV420_P3_10,
};

// format for wdma config
enum VEU_WB_PIXEL_FORMAT {
	WB_PIXEL_FORMAT_ARGB2101010 = 0,
	WB_PIXEL_FORMAT_ABGR2101010,
	WB_PIXEL_FORMAT_RGBA1010102,
	WB_PIXEL_FORMAT_BGRA1010102,
	WB_PIXEL_FORMAT_ARGB8888,
	WB_PIXEL_FORMAT_ABGR8888,
	WB_PIXEL_FORMAT_RGBA8888,
	WB_PIXEL_FORMAT_BGRA8888,
	WB_PIXEL_FORMAT_XRGB8888,
	WB_PIXEL_FORMAT_XBGR8888,
	WB_PIXEL_FORMAT_RGBX8888,
	WB_PIXEL_FORMAT_BGRX8888,
	WB_PIXEL_FORMAT_RGB888,
	WB_PIXEL_FORMAT_BGR888,
	WB_PIXEL_FORMAT_RGBA5551,
	WB_PIXEL_FORMAT_BGRA5551,
	WB_PIXEL_FORMAT_ABGR1555,
	WB_PIXEL_FORMAT_ARGB1555,
	WB_PIXEL_FORMAT_RGBX5551,
	WB_PIXEL_FORMAT_BGRX5551,
	WB_PIXEL_FORMAT_XBGR1555,
	WB_PIXEL_FORMAT_XRGB1555,
	WB_PIXEL_FORMAT_RGB565,
	WB_PIXEL_FORMAT_BGR565,

	WB_PIXEL_FORMAT_NV21_8 = 24,
	WB_PIXEL_FORMAT_NV12_8,
	WB_PIXEL_FORMAT_NV21_10,
	WB_PIXEL_FORMAT_NV12_10,
};

/**
 * veu_format_map - format info for veu
 * @drm_format: format released by hal
 * @veu_format: format for rdma/wdma
 * @is_afbc_support: is format support afbc by hardware
 * @uv_swap: NV21 need swap uv plane
 * @is_yuv: yuv format
 * @plane_count: currently not used
 * @bpp:currently not used
 */
struct veu_format_map {
	uint32_t drm_format;
	uint32_t veu_format;
	bool is_afbc_support;
	bool uv_swap;
	bool is_yuv;
	uint32_t plane_count;
	uint32_t bpp;
};


const struct veu_format_map *veu_format_get(
	enum VEU_MODULE_TYPE module_type, uint32_t drm_format, bool is_afbc_format);

bool is_format_support_tile32x4(uint32_t format);

#endif // _VEU_FORMAT_H_
