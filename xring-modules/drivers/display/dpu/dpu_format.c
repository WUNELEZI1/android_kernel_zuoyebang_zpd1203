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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include "dpu_log.h"
#include "dpu_format.h"

static const struct dpu_format_map rch_format_map[] = {
	{ DRM_FORMAT_ARGB2101010, RCH_FORMAT_ARGB_2101010, "ARGB2101010",
		true, false, true, false, 1 },
	{ DRM_FORMAT_ABGR2101010, RCH_FORMAT_ABGR_2101010, "ABGR2101010",
		true, false, true, false, 1 },
	{ DRM_FORMAT_RGBA1010102, RCH_FORMAT_RGBA_1010102, "RGBA1010102",
		false, false, true, false, 1 },
	{ DRM_FORMAT_BGRA1010102, RCH_FORMAT_BGRA_1010102, "BGRA1010102",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ARGB8888, RCH_FORMAT_ARGB_8888, "ARGB8888",
		true, false, true, false, 1 },
	{ DRM_FORMAT_ABGR8888, RCH_FORMAT_ABGR_8888, "ABGR8888",
		true, false, true, false, 1 },
	{ DRM_FORMAT_RGBA8888, RCH_FORMAT_RGBA_8888, "RGBA8888",
		true, false, true, false, 1 },
	{ DRM_FORMAT_BGRA8888, RCH_FORMAT_BGRA_8888, "BGRA8888",
		true, false, true, false, 1 },
	{ DRM_FORMAT_XRGB8888, RCH_FORMAT_XRGB_8888, "XRGB8888",
		true, false, false, false, 1 },
	{ DRM_FORMAT_XBGR8888, RCH_FORMAT_XBGR_8888, "XBGR8888",
		true, false, false, false, 1 },
	{ DRM_FORMAT_RGBX8888, RCH_FORMAT_RGBX_8888, "RGBX8888",
		true, false, false, false, 1 },
	{ DRM_FORMAT_BGRX8888, RCH_FORMAT_BGRX_8888, "BGRX8888",
		true, false, false, false, 1 },
	{ DRM_FORMAT_RGB888, RCH_FORMAT_RGB_888, "RGB888",
		true,  false, false, false, 1 },
	{ DRM_FORMAT_BGR888, RCH_FORMAT_BGR_888, "BGR888",
		true,  false, false, false, 1 },
	{ DRM_FORMAT_RGB565, RCH_FORMAT_RGB_565, "RGB565",
		true,  false, false, false, 1 },
	{ DRM_FORMAT_BGR565, RCH_FORMAT_BGR_565, "BGR565",
		true,  false, false, false, 1 },
	{ DRM_FORMAT_RGBA5551, RCH_FORMAT_RGBA_5551, "RGBA5551",
		false, false, true, false, 1 },
	{ DRM_FORMAT_BGRA5551, RCH_FORMAT_BGRA_5551, "BGRA5551",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ARGB1555, RCH_FORMAT_ARGB_1555, "ARGB1555",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ABGR1555, RCH_FORMAT_ABGR_1555, "ABGR1555",
		false, false, true, false, 1 },
	{ DRM_FORMAT_RGBX5551, RCH_FORMAT_RGBX_5551, "RGBX5551",
		false, false, false, false, 1 },
	{ DRM_FORMAT_BGRX5551, RCH_FORMAT_BGRX_5551, "BGRX5551",
		false, false, false, false, 1 },
	{ DRM_FORMAT_XRGB1555, RCH_FORMAT_XRGB_5551, "XRGB1555",
		false, false, false, false, 1 },
	{ DRM_FORMAT_XBGR1555, RCH_FORMAT_XBGR_5551, "XBGR1555",
		false, false, false, false, 1 },
	{ DRM_FORMAT_XYUV8888, RCH_FORMAT_XYUV_444_P1_8, "XYUV8888",
		false, false, false, true, 1 },
	{ DRM_FORMAT_XVYU2101010, RCH_FORMAT_XYUV_444_P1_10, "XVYU2101010",
		false, false, false, true, 1 },
	{ DRM_FORMAT_YUYV, RCH_FORMAT_VYUY_422_P1_8, "YUYV",
		false, false, false, true, 1 },
	{ DRM_FORMAT_UYVY, RCH_FORMAT_YVYU_422_P1_8, "UYVY",
		false, false, false, true, 1 },
	{ DRM_FORMAT_NV12, RCH_FORMAT_YUV_420_P2_8, "NV12",
		true, false, false, true, 2 },
	{ DRM_FORMAT_NV21, RCH_FORMAT_YUV_420_P2_8, "NV21",
		true, true, false, true, 2 },
	{ DRM_FORMAT_YUV420, RCH_FORMAT_YUV_420_P3_8, "YUV420",
		false, false, false, true, 3 },
	{ DRM_FORMAT_YVU420, RCH_FORMAT_YUV_420_P3_8, "YVU420",
		true, true, false, true, 3 },
	/**
	 * DRM_FORMAT_P016 have same memory request with P010, and there will be
	 * no actual requirements in subsequent scenarios for YUV420_16bit.
	 * Therefore, use P016 to satisfy the requirements of YUV420_NV21_10bit.
	 */
	{ DRM_FORMAT_P010, RCH_FORMAT_YUV_420_P2_10, "P010",
		true, false, false, true, 2 },
	{ DRM_FORMAT_P016, RCH_FORMAT_YUV_420_P2_10, "P016",
		true, true, false, true, 2 },
};

static const struct dpu_format_map wb_format_map[] = {
	{ DRM_FORMAT_ARGB2101010, WB_FORMAT_ARGB_2101010, "ARGB2101010",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ABGR2101010, WB_FORMAT_ABGR_2101010, "ABGR2101010",
		true, false, true, false, 1 },
	{ DRM_FORMAT_RGBA1010102, WB_FORMAT_RGBA_1010102, "RGBA1010102",
		false, false, true, false, 1 },
	{ DRM_FORMAT_BGRA1010102, WB_FORMAT_BGRA_1010102, "BGRA1010102",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ARGB8888, WB_FORMAT_ARGB_8888, "ARGB8888",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ABGR8888, WB_FORMAT_ABGR_8888, "ABGR8888",
		true, false, true, false, 1 },
	{ DRM_FORMAT_RGBA8888, WB_FORMAT_RGBA_8888, "RGBA8888",
		false, false, true, false, 1 },
	{ DRM_FORMAT_BGRA8888, WB_FORMAT_BGRA_8888, "BGRA8888",
		false, false, true, false, 1 },
	{ DRM_FORMAT_XRGB8888, WB_FORMAT_XRGB_8888, "XRGB8888",
		false, false, false, false, 1 },
	{ DRM_FORMAT_XBGR8888, WB_FORMAT_XBGR_8888, "XBGR8888",
		false, false, false, false, 1 },
	{ DRM_FORMAT_RGBX8888, WB_FORMAT_RGBX_8888, "RGBX8888",
		false, false, false, false, 1 },
	{ DRM_FORMAT_BGRX8888, WB_FORMAT_BGRX_8888, "BGRX8888",
		false, false, false, false, 1 },
	{ DRM_FORMAT_RGB888, WB_FORMAT_RGB_888, "RGB888",
		false, false, false, false, 1 },
	{ DRM_FORMAT_BGR888, WB_FORMAT_BGR_888, "BGR888",
		true,  false, false, false, 1 },
	{ DRM_FORMAT_RGB565, WB_FORMAT_RGB_565, "RGB565",
		false, false, false, false, 1 },
	{ DRM_FORMAT_BGR565, WB_FORMAT_BGR_565, "BGR565",
		true, false, false, false, 1 },
	{ DRM_FORMAT_RGBA5551, WB_FORMAT_RGBA_5551, "RGBA5551",
		false, false, true, false, 1 },
	{ DRM_FORMAT_BGRA5551, WB_FORMAT_BGRA_5551, "BGRA5551",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ARGB1555, WB_FORMAT_ARGB_1555, "ARGB1555",
		false, false, true, false, 1 },
	{ DRM_FORMAT_ABGR1555, WB_FORMAT_ABGR_1555, "ABGR1555",
		false, false, true, false, 1 },
	{ DRM_FORMAT_RGBX5551, WB_FORMAT_RGBX_5551, "RGBX5551",
		false, false, false, false, 1 },
	{ DRM_FORMAT_BGRX5551, WB_FORMAT_BGRX_5551, "BGRX5551",
		false, false, false, false, 1 },
	{ DRM_FORMAT_XRGB1555, WB_FORMAT_XRGB_5551, "XRGB1555",
		false, false, false, false, 1 },
	{ DRM_FORMAT_XBGR1555, WB_FORMAT_XBGR_5551, "XBGR1555",
		false, false, false, false, 1 },
	{ DRM_FORMAT_NV21, WB_FORMAT_NV21_8, "NV21",
		false, false, false, true, 2 },
	{ DRM_FORMAT_NV12, WB_FORMAT_NV12_8, "NV12",
		true, false, false, true, 2 },
	{ DRM_FORMAT_P010, WB_FORMAT_NV12_10, "P010",
		true, false, false, true, 2 },
	{ DRM_FORMAT_P016, WB_FORMAT_NV21_10, "P016",
		true, true, false, true, 2 },
};

/* TODO: remove DRM_FORMAT_P016, it does not satisfy the FOURCC format */
static const struct drm_format_info dpu_specific_formats[] = {
	{ .format = DRM_FORMAT_P010, .depth = 0, .num_planes = 2,
		.cpp = {0, 0, 0}, .hsub = 2, .vsub = 2, .is_yuv = true},
	{ .format = DRM_FORMAT_P016, .depth = 0, .num_planes = 2,
		.cpp = {0, 0, 0}, .hsub = 2, .vsub = 2, .is_yuv = true},
};

const struct dpu_format_map *dpu_format_get(
		enum dpu_fmt_module_type module_type,
		u32 drm_format, u64 modifier)
{
	const struct dpu_format_map *format_map;
	bool is_afbc_format;
	bool format_found;
	u32 count;
	int i;

	is_afbc_format = false;
	format_found = false;

	if (modifier && DPU_FMT_IS_AFBC(modifier))
		is_afbc_format = true;

	switch (module_type) {
	case FMT_MODULE_RCH:
		count = ARRAY_SIZE(rch_format_map);
		format_map = rch_format_map;
		break;
	case FMT_MODULE_WB:
		count = ARRAY_SIZE(wb_format_map);
		format_map = wb_format_map;
		break;
	default:
		DPU_ERROR("wrong module type %d\n", module_type);
		count = 0;
		break;
	}

	for (i = 0; i < count; i++) {
		if (format_map[i].drm_format != drm_format)
			continue;

		if ((is_afbc_format && format_map[i].is_afbc_support)
				|| (!is_afbc_format)) {
			format_found = true;
			break;
		}
	}

	if (!format_found)
		return NULL;

	return &format_map[i];
}

bool dpu_format_is_afbc_supported(enum dpu_fmt_module_type module_type,
		u32 drm_format, u64 modifier)
{
	const struct dpu_format_map *format_map;

	format_map = dpu_format_get(module_type, drm_format, modifier);
	if (IS_ERR_OR_NULL(format_map))
		return false;

	if (DPU_FMT_HAS_AFBC_Y2R(modifier) && format_map->is_yuv)
		return false;

	return true;
}

int dpu_format_all_supported_get(enum dpu_fmt_module_type module_type,
	u32 **drm_formats)
{
	const struct dpu_format_map *format_map;
	u32 *formats;
	u32 format_count;
	int i;

	switch (module_type) {
	case FMT_MODULE_RCH:
		format_count = ARRAY_SIZE(rch_format_map);
		format_map = rch_format_map;
		break;
	case FMT_MODULE_WB:
		format_count = ARRAY_SIZE(wb_format_map);
		format_map = wb_format_map;
		break;
	default:
		DPU_ERROR("wrong module type %d\n", module_type);
		return 0;
	}

	/**
	 * Each format is encoded as a bit and the current code
	 * only supports a u64.
	 */
	if (format_count > 64) {
		DPU_ERROR("drm only supports 64 formats\n");
		return -EINVAL;
	}

	formats = kzalloc(format_count * sizeof(u32), GFP_KERNEL);
	if (!formats)
		return -ENOMEM;

	for (i = 0; i < format_count; i++)
		formats[i] = format_map[i].drm_format;

	*drm_formats = formats;

	return format_count;
}

const struct drm_format_info *dpu_format_info_get(
		const struct drm_mode_fb_cmd2 *mode_cmd)
{
	int i;

	if (!mode_cmd || !mode_cmd->modifier[0])
		return NULL;

	for (i = 0; i < ARRAY_SIZE(dpu_specific_formats); i++)
		if (mode_cmd->pixel_format == dpu_specific_formats[i].format)
			return &dpu_specific_formats[i];

	return NULL;
}

u32 dpu_get_drm_format_bpp(u32 dpu_drm_format)
{
	u32 bpp;

	switch (dpu_drm_format) {
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_XYUV8888:
	case DRM_FORMAT_XVYU2101010:
		bpp = 32;
		break;
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_P010:
	case DRM_FORMAT_P016:
		bpp = 24;
		break;
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_UYVY:
		bpp = 16;
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		bpp = 12;
		break;
	default:
		DPU_WARN("unsupported drm format %u\n", dpu_drm_format);
		bpp = 0;
	}

	return bpp;
}

