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

#ifndef _DPU_HW_COMMON_H_
#define _DPU_HW_COMMON_H_

#include "dpu_osal.h"

/* indicate the bit length of one register filed */
#define REG_BIT_LEN(len) (len)

/* get lower 32bit or higher 12bit from 64bit input data */
#define LOWER_32_BITS(n) ((u32)((n) & 0xffffffff))
#define HIGHER_12_BITS(n) (((u32)((n) >> 32)) & 0xfff)

/* reset all registers of one module */
#define REG_BANK_RESET_VAL 1

#define MINUS_ONE(value)	((value) - 1)
/* BIT related operations for operating register */
#define BITS_MASK(bit_start, bit_num) \
	((BIT((bit_num)) - 1) << (bit_start))
#define CLEAR_MASK_BITS(val, bit_mask) \
	((val) & (~(uint32_t)(bit_mask)))
#define MERGE_MASK_BITS(reg_val, bits_val, bit_start, bit_mask) \
	(CLEAR_MASK_BITS(reg_val, bit_mask) | (uint32_t)(bits_val) << (bit_start))
#define MERGE_BITS(reg_val, bits_val, bit_start, bit_num) \
	MERGE_MASK_BITS(reg_val, bits_val, bit_start, \
		BITS_MASK(bit_start, bit_num))

#define GET_LOW32_ADDR(addr) ((addr) & 0xFFFFFFFF)
#define GET_HIGH32_ADDR(addr) ((addr) >> 32)
#define ALIGN_UP(x, a) ((((x) + ((a) - 1)) / (a)) * (a))

#define GET_BIT_VAL(val, bit) (((val) >> bit) & 0x1)
#define GET_BITS_VAL(value, shift, len) (((value) >> shift) & (BIT(len) - 1))

#define MAX_VIDEO_SLICE_CNT 4
#define MAX_VIDEO_TLB_COUNT 20
#define MAX_VIDEO_SIZE 8192 * 8192
#define WFD_RESERVE_RCH_ID RCH_V_1

/* the index of dpu wb */
enum dpu_wb_id {
	WB_INVALID_ID = -1,
	WB0 = 0,
	WB1,
	WB_MAX,
};

/* the hardware index of dpu timing engine */
enum dpu_tmg_id {
	TMG_DSI0 = BIT(0),
	TMG_DSI1 = BIT(1),
	TMG_DP0 = BIT(2),
};

/* the supported rotation mode */
enum rot_mode {
	ROT_0,
	ROT_90,
	ROT_180,
	ROT_270,
};

/* the supported flip mode */
enum flip_mode {
	NO_FLIP,
	H_FLIP,
	V_FLIP,
	HV_FLIP,
};

/* layer_plane_num - for YUV_P2/P3, layer have different plane number */
enum layer_plane_num {
	PLANE_ONE = 1,
	PLANE_TWO,
	PLANE_THREE,
	PLANE_MAX = PLANE_THREE,
};

enum protected_status {
	SWITCH_OFF,
	SWITCH_ON,
	HOLD,
};

/**
 * dpu_rect - the dpu rectangle structure
 * @x1: the top left x coordinate
 * @y1: the top left y coordinate
 * @x2: the bottom right x coordinate
 * @y2: the bottom right y coordinate
 */
struct dpu_rect {
	u16 x1;
	u16 y1;
	u16 x2;
	u16 y2;
};

/**
 * dpu_rect_v2 - the dpu rectangle structure version 2
 * @x: the x coordinate of rectangle
 * @y: the y coordinate of rectangle
 * @w: the width of rectangle
 * @h: the height of rectangle
 */
struct dpu_rect_v2 {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
};

enum {
	DPU_PROFILE_0,
	DPU_PROFILE_1,
	DPU_PROFILE_2,
	DPU_PROFILE_MAX_NUM,
};

#endif /* _DPU_HW_COMMON_H_ */
