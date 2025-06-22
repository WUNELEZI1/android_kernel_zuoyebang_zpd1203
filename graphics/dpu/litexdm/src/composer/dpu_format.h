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

#ifndef DPU_FORMAT_H
#define DPU_FORMAT_H

#include "dpu_hw_format.h"

static inline bool is_yuv(uint32_t format)
{
	bool is_yuv;

	switch (format) {
	case RCH_FORMAT_XYUV_444_P1_8:
	case RCH_FORMAT_XYUV_444_P1_10:
	case RCH_FORMAT_VYUY_422_P1_8:
	case RCH_FORMAT_YVYU_422_P1_8:
	case RCH_FORMAT_YUV_420_P2_8:
	case RCH_FORMAT_YUV_420_P3_8:
	case RCH_FORMAT_YUV_420_P2_10:
	case RCH_FORMAT_YUV_420_P3_10:
		is_yuv = true;
		break;
	default:
		is_yuv = false;
		break;
	}
	return is_yuv;
}

static inline uint32_t get_plane_count(uint32_t format)
{
	uint32_t plane_count;

	switch (format) {
	case RCH_FORMAT_YUV_420_P2_8:
	case RCH_FORMAT_YUV_420_P2_10:
		plane_count = 2;
		break;
	case RCH_FORMAT_YUV_420_P3_8:
	case RCH_FORMAT_YUV_420_P3_10:
		plane_count = 3;
		break;
	default:
		plane_count = 1;
		break;
	}

	return plane_count;
}

#endif