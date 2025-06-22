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
#ifndef _DPU_HW_POST_SCALER_H_
#define _DPU_HW_POST_SCALER_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_post_scaler_ops - the external interface of post scaler
 */
struct dpu_hw_post_scaler_ops {
	/**
	 * enable - enable or disable the post scaler
	 * @hw: the post scaler hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * set - set post scaler parameter
	 * @hw: the post scaler hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set)(struct dpu_hw_blk *hw, struct postpq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the post scaler hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);

	/**
	 * get - get post scaler register parameter
	 * @hw: the post scaler hardware pointer
	 */
	void (*get)(struct dpu_hw_blk *hw,
		u16 base_idx, u16 end_idx, void *data);

	/**
	 * status_dump - hw status dump
	 * @hw: the post scaler hardware pointer
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_post_scaler - post scaler hardware object
 * @hw: the post scaler hardware info
 * @blk_cap: the post scaler hardware resource and capability
 * @ops: the post scaler function table
 */
struct dpu_hw_post_scaler {
	struct dpu_hw_blk hw;
	const struct dpu_post_scaler_cap *blk_cap;
	struct dpu_hw_post_scaler_ops ops;
};

static inline struct dpu_hw_post_scaler *to_dpu_hw_post_scaler(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_post_scaler, hw);
}

/**
 * dpu_hw_post_scaler_init - init post scaler hw block with its capability
 * @cap: the post scaler capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_post_scaler_init(struct dpu_post_scaler_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_post_scaler_deinit - deinit post scaler hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_post_scaler_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_POST_SCALER_H_ */
