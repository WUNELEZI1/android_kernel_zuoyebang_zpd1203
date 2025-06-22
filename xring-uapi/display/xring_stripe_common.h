/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef __XRING_STRIPE_COMMON_H__
#define __XRING_STRIPE_COMMON_H__

#include <linux/types.h>

/* when use stripe, the stripe count is limited to [1, 36] */
#define MAX_STRIPE_COUNT 36
#define MIN_STRIPE_COUNT 1

struct stripe_rect {
	__u16 x;
	__u16 y;
	__u16 w;
	__u16 h;
};

/**
 * display_scaler_stripe - dpu 2d-scaler stripe config param
 * @scaler_in_rect: input rect of 2d-scaler
 * @scaler_out_rect: output rect of 2d-scaler
 * @scaler_hor_delta: delta of horizontal direction
 * @scaler_ver_delta: delta of vertical direction
 * @scaler_hor_init_phase: horizontal direction init phase of stripe start
 * @scaler_ver_init_phase: vertical direction init phase of stripe start
 * @scaler_enable: scaler module enable
 */
struct display_scaler_stripe {
	struct stripe_rect scaler_in_rect;
	struct stripe_rect scaler_out_rect;

	__u32 scaler_hor_delta;
	__u32 scaler_ver_delta;
	__u64 scaler_hor_init_phase;
	__u64 scaler_ver_init_phase;

	bool scaler_enable;
};

/**
 * display_rch_stripe_info - stirpe param of rdma input layer
 * @rdma_id: the rdma id of layer corresponding
 * @roi0_enable: first exclusive roi enable flag of layer
 * @roi1_enable: second exclusive roi enable flag of layer
 * @roi0_rect: first exclusive roi area coordinates of layer
 * @roi1_rect: secongd exclusive roi area coordinates of layer
 * @layer_crop_rect: the crop area coordinates of input layer stripe block
 * @layer_output_rect: rch output size information of input layer stripe block
 * @layer_scaler_enable: the scaler enable flag for input layer, only use in DPU
 * @layer_scaler_stripe: rch scaler param of input layer stripe block
 */
struct display_rch_stripe_info {
	int rdma_id;

	bool roi0_enable;
	bool roi1_enable;
	struct stripe_rect roi0_rect;
	struct stripe_rect roi1_rect;

	struct stripe_rect layer_crop_rect;
	struct stripe_rect layer_output_rect;

	bool layer_scaler_enable;
	struct display_scaler_stripe layer_scaler_stripe;
};

/**
 * display_wb_stripe_info - stirpe param of wb output layer
 * @stripe_id: current stirpe block id
 * @stripe_width: current stripe block width
 * @scaler_enable: the scaler enable flag of output layer, for DPU is 1d-scaler,
 *                 VEU is 2d-scaler
 * @wb_crop0_enable: crop0 module enable flag of wb
 * @wb_crop1_enable: crop1 module enable flag of wb
 * @wb_crop2_enable: crop1 module enable flag of wb
 * @wb_input_rect: current stripe block intput rect of wb output layer
 * @wb_output_rect: current stripe block output rect of wb output layer
 * @wb_crop0_rect: current stripe block crop0 module output rect
 * @wb_crop1_rect: current stripe block crop1 module output rect
 * @wb_crop2_rect: current stripe block crop2 module output rect
 * @wb_scaler_param: current stripe block sclaer module param
 */
struct display_wb_stripe_info {
	__u32 stripe_id;

	__u32 stripe_width;

	bool scaler_enable;
	bool wb_crop0_enable;
	bool wb_crop1_enable;
	bool wb_crop2_enable;

	struct stripe_rect wb_input_rect;
	struct stripe_rect wb_output_rect;
	struct stripe_rect wb_crop0_rect;
	struct stripe_rect wb_crop1_rect;
	struct stripe_rect wb_crop2_rect;

	struct display_scaler_stripe wb_scaler_param;
};

/**
 * display_rch_stripe - plane stripe info
 * @count: stripe block number for input layer
 * @stripe: each stripe block param
 */
struct display_rch_stripe {
	__u32 count;
	__u32 reserve;
	struct display_rch_stripe_info stripe[MAX_STRIPE_COUNT];
};

/**
 * display_wb_stripe - wb stripe info
 * @count: stripe block number for output layer
 * @stripe: each stripe block param
 */
struct display_wb_stripe {
	__u32 count;
	__u32 reserve;
	struct display_wb_stripe_info stripe[MAX_STRIPE_COUNT];
};

#endif /* __XRING_DPU_STRIPE_H__ */