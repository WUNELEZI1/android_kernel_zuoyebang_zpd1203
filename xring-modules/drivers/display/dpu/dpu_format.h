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

#ifndef _DPU_FORMAT_H_
#define _DPU_FORMAT_H_

#include <linux/types.h>
#include <linux/bits.h>
#include <drm/drm_fourcc.h>
#include "dpu_hw_format.h"

#define AFBC_MODE_MASK      0x000fffffffffffffULL
#define AFBC_16x16          AFBC_FORMAT_MOD_BLOCK_SIZE_16x16
#define AFBC_32x8           AFBC_FORMAT_MOD_BLOCK_SIZE_32x8
#define AFBC_TILED          AFBC_FORMAT_MOD_TILED
#define AFBC_YTR            AFBC_FORMAT_MOD_YTR
#define AFBC_SPLIT          AFBC_FORMAT_MOD_SPLIT
#define AFBC_COPY           AFBC_FORMAT_MOD_CBR
#define AFBC_COLOR          AFBC_FORMAT_MOD_SC
#define AFBC_NONE           (0)

#define DPU_FORMAT_NAME_LEN       64
/* macro for afbc format modifier support */
#define RCH_AFBC_FORMAT(sb, tile, ytr, split) \
	DRM_FORMAT_MOD_ARM_AFBC((sb) | (tile) | (ytr) | (split))
#define WB_AFBC_FORMAT(sb, tile, ytr, split, color, copy) \
	DRM_FORMAT_MOD_ARM_AFBC((sb) | (tile) | (ytr) | (split) \
		| (color) | (copy))

/* macro for afbc format checking */
#define DPU_FMT_IS_AFBC(modifier) \
	(((modifier) & GENMASK_ULL(63, 52)) == DRM_FORMAT_MOD_ARM_AFBC(0))

/* macro for super block size checking */
#define DPU_FMT_IS_AFBC_16x16(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_16x16)
#define DPU_FMT_IS_AFBC_32x8(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_32x8)

/* macro for mode support checking */
#define DPU_FMT_HAS_AFBC_TILED(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_TILED)
#define DPU_FMT_HAS_AFBC_Y2R(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_YTR)
#define DPU_FMT_HAS_AFBC_SPLIT(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_SPLIT)
#define DPU_FMT_HAS_AFBC_COPY(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_COPY)
#define DPU_FMT_HAS_AFBC_COLOR(modifier) \
	(AFBC_MODE_MASK & (modifier) & AFBC_COLOR)

/* the supported modules for this format util */
enum dpu_fmt_module_type {
	FMT_MODULE_RCH = 0,
	FMT_MODULE_WB,
};

/**
 * dpu_format_map - the dpu format mapping structure
 * @drm_format: the DRM fourcc format
 * @dpu_format: the mapping for converting drm format to dpu format
 * @is_fbc_support: the flag is used to indicate that is afbc supported for
 *                  this drm_format or not
 * @uv_swap: the flag is used to swap uv for yuv format
 * @has_alpha: the format has alpha component or not
 * @is_yuv: the format is yuv format or rgb format
 * @plane_count: the count of format plane
 */
struct dpu_format_map {
	u32 drm_format;
	u8 dpu_format;
	u8 dpu_format_name[DPU_FORMAT_NAME_LEN];
	bool is_afbc_support;
	bool uv_swap;
	bool has_alpha;
	bool is_yuv;
	u8 plane_count;
};

/**
 * dpu_format_get - get a dpu format map item
 * @module_type: wb or rch module for getting
 * @drm_format: drm format value
 * @modifier: drm modifier value
 *
 * Return: valid pointer on success, error pointer on failure
 */
const struct dpu_format_map *dpu_format_get(
		enum dpu_fmt_module_type module_type,
		u32 drm_format, u64 modifier);

/**
 * dpu_format_is_afbc_supported - check if format is afbc supported
 * @drm_format: drm format value
 * @modifier: drm modifier value
 *
 * Return: true if format is afbc supported
 */
bool dpu_format_is_afbc_supported(
		enum dpu_fmt_module_type module_type,
		u32 drm_format, u64 modifier);

/**
 * dpu_format_all_supported_get - get all supported formats
 * @module_type: wb or rch module for getting
 * @drm_formats: output data, array of drm format value
 *
 * Return: the count of all returned formats
 */
int dpu_format_all_supported_get(
		enum dpu_fmt_module_type module_type,
		u32 **drm_formats);

/**
 * dpu_format_info_get - override specific dpu format info
 * @mode_cmd: pointer of drm_mode_fb_cmd2 data
 *
 * Return: valid pointer on success or NULL on failure
 */
const struct drm_format_info *dpu_format_info_get(
		const struct drm_mode_fb_cmd2 *mode_cmd);

/**
 * dpu_get_drm_format_bpp - get bit per pixel from drm format
 * @dpu_drm_format: drm format
 *
 * Return: bit per pixel of drm format
 */
u32 dpu_get_drm_format_bpp(u32 dpu_drm_format);

#endif /* _DPU_FORMAT_H_ */
