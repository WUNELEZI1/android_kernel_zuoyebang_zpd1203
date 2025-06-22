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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#ifndef _DPU_HW_WB_SCALER_H_
#define _DPU_HW_WB_SCALER_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_wb_scaler_ops.h"

/**
 * dpu_hw_wb_scaler_ops - the virtual function table of wb scaler class
 */
struct dpu_hw_wb_scaler_ops {
	/**
	 * enable - enable or disable the wb scaler
	 * @hw: the wb scaler hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * set - set wb scaler parameters
	 * @hw: the wb scaler hardware pointer
	 * @scaler: the pointer of wb scaler config
	 */
	void (*set)(struct dpu_hw_blk *hw, struct dpu_1d_scaler_cfg *scaler);
};

/**
 * dpu_hw_wb_scaler - wb scaler hardware object
 * @hw: the wb scaler hardware info
 * @blk_cap: the wb scaler hardware resource and capability
 * @ops: the wb scaler function table
 */
struct dpu_hw_wb_scaler {
	struct dpu_hw_blk hw;
	const struct dpu_wb_scaler_cap *blk_cap;
	struct dpu_hw_wb_scaler_ops ops;
};

static inline struct dpu_hw_wb_scaler *to_dpu_hw_wb_scaler(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_wb_scaler, hw);
}

/**
 * dpu_hw_wb_scaler_init - init wb scaler hw block with its capability
 * @wb_scaler_cap: the wb scaler capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_wb_scaler_init(struct dpu_wb_scaler_cap *wb_scaler_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_wb_scaler_deinit - deinit wb scaler hw block
 * @hw: base hw block pointer
 */
void dpu_hw_wb_scaler_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_WB_SCALER_H_ */
