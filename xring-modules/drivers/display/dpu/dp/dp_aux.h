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

#ifndef _DP_AUX_H_
#define _DP_AUX_H_

#include <linux/completion.h>
#include <linux/version.h>
#include <drm/display/drm_dp_helper.h>
#include "dp_hw_ctrl.h"

#define AUX_MAX_BYTES_SIZE                             16

/**
 * dp_aux_request_info - the structure to record aux request info
 * @is_i2c: whether the aux request is i2c
 * @is_mot: whether the aux request is mot
 * @rw: read or write
 * @request: aux request flag
 * @address: aus request address
 * @size: aux request size
 * @buffer: buffer to save aux write data or aux read data
 */
struct dp_aux_request_info {
	bool is_i2c;
	bool is_mot;
	u8 rw;
	u8 request;
	u32 address;
	size_t size;
	void *buffer;
};

/**
 * dp_aux - the structure to provide aux transaction
 * @request: used to record aux request
 * @reply_status: aux reply status
 * @reply_completed: synchronizing signal of axu reply ready
 * @aux_intr_ops: aux reply interrupt operations
 * @base: drm aux
 * @hw_ctrl: the handle of dp controller hardware operation
 * @display: callback pointer of display
 */
struct dp_aux {
	struct dp_aux_request_info request_info;
	struct dp_aux_reply_status reply_status;
	struct completion reply_completed;
	struct dp_intr_ops *aux_intr_ops;

	struct drm_dp_aux base;
	struct dp_hw_ctrl *hw_ctrl;
	struct dp_display *display;
};

/**
 * dp_aux_transfer - make an aux transmission, including native and i2c
 * @drm_aux: handle of drm_dp_aux
 * @msg: sended message, contains of transmission type and data
 *
 * this function implemented the aux transfer and should be registered to the
 * drm_dp_aux.transfer()
 */
ssize_t dp_aux_transfer(struct drm_dp_aux *drm_aux, struct drm_dp_aux_msg *msg);

int dp_aux_init(struct dp_display *display, struct dp_aux **aux);
void dp_aux_deinit(struct dp_aux *aux);

#endif /* _DP_AUX_H_ */
