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

#ifndef _DPU_HW_TM_H_
#define _DPU_HW_TM_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_tm_ops - the external interface of tm
 */
struct dpu_hw_tm_ops {
	/**
	 * enable - enable or disable scaler
	 * @hw: the scaler hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * set_tm - set tm cfg
	 * @hw: the tm hardware pointer
	 * @hw_cfg: pointer to tm_hw_cfg
	 */
	void (*set_tm)(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the tm hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_tm - tm hardware object
 * @hw: the tm hardware info
 * @blk_cap: the tm hardware resource and capability
 * @ops: the tm function table
 */
struct dpu_hw_tm {
	struct dpu_hw_blk hw;
	const struct dpu_tm_cap *blk_cap;
	struct dpu_hw_tm_ops ops;
};

static inline struct dpu_hw_tm *to_dpu_hw_tm(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_tm, hw);
}

/**
 * dpu_hw_tm_init - init tm hw block with its capability
 * @cap: the tm capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_tm_init(struct dpu_tm_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_tm_deinit - deinit tm hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_tm_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_TM_H_ */
