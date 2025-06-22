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

#include "veu_format.h"

// afbc, uvswap, isyuv, planecount
static const struct veu_format_map rdma_format_map[] = {
	{DRM_FORMAT_RGBA8888, VEU_PIXEL_FORMAT_RGBA8888, true, false, false, 1, 4},
	{DRM_FORMAT_BGRA8888, VEU_PIXEL_FORMAT_BGRA8888, true, false, false, 1, 4},
	{DRM_FORMAT_RGBX8888, VEU_PIXEL_FORMAT_RGBX8888, true, false, false, 1, 4},
	{DRM_FORMAT_BGRX8888, VEU_PIXEL_FORMAT_BGRX8888, true, false, false, 1, 4},
	{DRM_FORMAT_ABGR8888, VEU_PIXEL_FORMAT_ABGR8888, true, false, false, 1, 4},
	{DRM_FORMAT_ARGB8888, VEU_PIXEL_FORMAT_ARGB8888, true, false, false, 1, 4},
	{DRM_FORMAT_XBGR8888, VEU_PIXEL_FORMAT_XBGR8888, true, false, false, 1, 4},
	{DRM_FORMAT_XRGB8888, VEU_PIXEL_FORMAT_XRGB8888, true, false, false, 1, 4},
	{DRM_FORMAT_RGB888, VEU_PIXEL_FORMAT_RGB888, true, false, false, 1, 3},
	{DRM_FORMAT_BGR888, VEU_PIXEL_FORMAT_BGR888, true, false, false, 1, 3},
	{DRM_FORMAT_RGB565, VEU_PIXEL_FORMAT_RGB565, true, false, false, 1, 2},
	{DRM_FORMAT_BGR565, VEU_PIXEL_FORMAT_BGR565, true, false, false, 1, 2},
	{DRM_FORMAT_RGBA5551, VEU_PIXEL_FORMAT_RGBA5551, false, false, false, 1, 2},
	{DRM_FORMAT_BGRA5551, VEU_PIXEL_FORMAT_BGRA5551, false, false, false, 1, 2},
	{DRM_FORMAT_ARGB1555, VEU_PIXEL_FORMAT_ARGB1555, false, false, false, 1, 2},
	{DRM_FORMAT_ABGR1555, VEU_PIXEL_FORMAT_ABGR1555, false, false, false, 1, 2},
	{DRM_FORMAT_RGBX5551, VEU_PIXEL_FORMAT_RGBX5551, false, false, false, 1, 2},
	{DRM_FORMAT_BGRX5551, VEU_PIXEL_FORMAT_BGRX5551, false, false, false, 1, 2},
	{DRM_FORMAT_XRGB1555, VEU_PIXEL_FORMAT_XRGB1555, false, false, false, 1, 2},
	{DRM_FORMAT_XBGR1555, VEU_PIXEL_FORMAT_XBGR1555, false, false, false, 1, 2},
	{DRM_FORMAT_RGBA1010102, VEU_PIXEL_FORMAT_RGBA1010102, false, false, false, 1, 4},
	{DRM_FORMAT_BGRA1010102, VEU_PIXEL_FORMAT_BGRA1010102, false, false, false, 1, 4},
	{DRM_FORMAT_ARGB2101010, VEU_PIXEL_FORMAT_ARGB2101010, true, false, false, 1, 4},
	{DRM_FORMAT_ABGR2101010, VEU_PIXEL_FORMAT_ABGR2101010, true, false, false, 1, 4},
	{DRM_FORMAT_NV12, VEU_PIXEL_FORMAT_YUV420_P2_8, true, false, true, 2, 1},
	{DRM_FORMAT_NV21, VEU_PIXEL_FORMAT_YUV420_P2_8, true, true, true, 2, 1},
	{DRM_FORMAT_P010, VEU_PIXEL_FORMAT_YUV420_P2_10, true, false, true, 2, 2},
	{DRM_FORMAT_P016, VEU_PIXEL_FORMAT_YUV420_P2_10, true, true, true, 2, 2},
	{DRM_FORMAT_YUV420, VEU_PIXEL_FORMAT_YUV420_P3_8, false, false, true, 3, 1},
	{DRM_FORMAT_YVU420, VEU_PIXEL_FORMAT_YUV420_P3_8, false, true, true, 3, 1},
	{DRM_FORMAT_YUYV, VEU_PIXEL_FORMAT_VYUY_422_8, false, false, true, 1, 2},
	{DRM_FORMAT_YVYU, VEU_PIXEL_FORMAT_YVYU_422_8, false, false, true, 1, 2},
};

static const struct veu_format_map wdma_format_map[] = {
	{DRM_FORMAT_RGBA8888, WB_PIXEL_FORMAT_RGBA8888, false, false, false, 1, 4},
	{DRM_FORMAT_BGRA8888, WB_PIXEL_FORMAT_BGRA8888, false, false, false, 1, 4},
	{DRM_FORMAT_ABGR8888, WB_PIXEL_FORMAT_ABGR8888, true, false, false, 1, 4},
	{DRM_FORMAT_ARGB8888, WB_PIXEL_FORMAT_ARGB8888, false, false, false, 1, 4},
	{DRM_FORMAT_XBGR8888, WB_PIXEL_FORMAT_XBGR8888, false, false, false, 1, 4},
	{DRM_FORMAT_XRGB8888, WB_PIXEL_FORMAT_XRGB8888, false, false, false, 1, 4},
	{DRM_FORMAT_BGRA8888, WB_PIXEL_FORMAT_BGRA8888, false, false, false, 1, 4},
	{DRM_FORMAT_BGRX8888, WB_PIXEL_FORMAT_BGRX8888, false, false, false, 1, 4},
	{DRM_FORMAT_ARGB8888, WB_PIXEL_FORMAT_ARGB8888, false, false, false, 1, 4},
	{DRM_FORMAT_XRGB8888, WB_PIXEL_FORMAT_XRGB8888, false, false, false, 1, 4},
	{DRM_FORMAT_XBGR8888, WB_PIXEL_FORMAT_XBGR8888, false, false, false, 1, 4},
	{DRM_FORMAT_RGB888, WB_PIXEL_FORMAT_RGB888, false, false, false, 1, 3},
	{DRM_FORMAT_BGR888, WB_PIXEL_FORMAT_BGR888, true, false, false, 1, 3},
	{DRM_FORMAT_RGB565, WB_PIXEL_FORMAT_RGB565, false, false, false, 1, 2},
	{DRM_FORMAT_BGR565, WB_PIXEL_FORMAT_BGR565, true, false, false, 1, 2},
	{DRM_FORMAT_RGBA5551, WB_PIXEL_FORMAT_RGBX5551, false, false, false, 1, 2},
	{DRM_FORMAT_RGBA1010102, WB_PIXEL_FORMAT_RGBA1010102, false, false, false, 1, 4},
	{DRM_FORMAT_BGRA1010102, WB_PIXEL_FORMAT_BGRA1010102, false, false, false, 1, 4},
	{DRM_FORMAT_ARGB2101010, WB_PIXEL_FORMAT_ARGB2101010, false, false, false, 1, 4},
	{DRM_FORMAT_ABGR2101010, WB_PIXEL_FORMAT_ABGR2101010, true, false, false, 1, 4},
	{DRM_FORMAT_NV12, WB_PIXEL_FORMAT_NV12_8, true, false, true, 2, 1},
	{DRM_FORMAT_NV21, WB_PIXEL_FORMAT_NV21_8, false, true, true, 2, 1},
	{DRM_FORMAT_P010, WB_PIXEL_FORMAT_NV12_10, true, false, true, 2, 2},
	{DRM_FORMAT_P016, WB_PIXEL_FORMAT_NV21_10, false, false, true, 2, 2},
};

const struct veu_format_map *veu_format_get(
	enum VEU_MODULE_TYPE module_type, uint32_t drm_format, bool is_afbc_format)
{
	const struct veu_format_map *format_map;
	bool format_found = false;
	int format_num;
	int i;

	switch (module_type) {
	case MODULE_RDMA:
		format_num = ARRAY_SIZE(rdma_format_map);
		VEU_DBG("rdma format support num %d", format_num);
		format_map = rdma_format_map;
		break;
	case MODULE_WDMA:
		format_num = ARRAY_SIZE(wdma_format_map);
		VEU_DBG("wdma format support num %d", format_num);
		format_map = wdma_format_map;
		break;
	default:
		VEU_ERR("module %d unsupported", module_type);
		return NULL;
	}

	for (i = 0; i < format_num; i++) {
		if (format_map[i].drm_format != drm_format)
			continue;
		if ((!is_afbc_format) || (is_afbc_format && format_map[i].is_afbc_support)) {
			VEU_DBG("format found");
			format_found = true;
		}
		break;
	}

	if (format_found)
		return &format_map[i];

	VEU_ERR("unsupported format: %d", drm_format);

	return NULL;
}

bool is_format_support_tile32x4(uint32_t format)
{
	switch (format) {
	case WB_PIXEL_FORMAT_ABGR2101010:
	case WB_PIXEL_FORMAT_ABGR8888:
	case WB_PIXEL_FORMAT_BGR888:
		return true;
	case WB_PIXEL_FORMAT_BGR565:
	case WB_PIXEL_FORMAT_NV12_8:
	case WB_PIXEL_FORMAT_NV12_10:
		return false;
	default:
		return false;
	}
}
