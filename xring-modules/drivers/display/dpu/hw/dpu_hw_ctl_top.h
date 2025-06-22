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

#ifndef _DPU_HW_CTL_TOP_H_
#define _DPU_HW_CTL_TOP_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_ctl_ops.h"

/**
 * dpu_hw_ctl_top_ops - the virtual function table of ctl_top class
 */
struct dpu_hw_ctl_top_ops {
	/**
	 * wb_input_position_config - config wb input position
	 * @hw: the ctl_top hardware pointer
	 * @wb_id: the wb index
	 * @postion: the input source position for wb
	 */
	void (*wb_input_position_config)(struct dpu_hw_blk *hw,
			enum dpu_wb_id wb_id,
			enum dpu_hw_wb_position position);

	/**
	 * status_dump - hw status dump
	 * @hw: the ctl_top hardware pointer
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);

	/**
	 * status_clear - hw status clear
	 * @hw: the ctl_top hardware pointer
	 */
	void (*status_clear)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_ctl_top - ctl_top hardware object
 * @hw: the ctl_top hardware info
 * @blk_cap: the ctl_top hardware resource and capability
 * @ops: the ctl_top function table
 */
struct dpu_hw_ctl_top {
	struct dpu_hw_blk hw;
	const struct dpu_ctl_top_cap *blk_cap;

	struct dpu_hw_ctl_top_ops ops;
};

static inline struct dpu_hw_ctl_top *to_dpu_hw_ctl_top(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_ctl_top, hw);
}

/**
 * dpu_hw_ctl_top_init - init one ctl_top hw block with its capability
 * @ctl_top_cap: the ctl_top capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_ctl_top_init(
		struct dpu_ctl_top_cap *ctl_top_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_ctl_top_deinit - deinit one ctl_top hw block with
 *                           base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_ctl_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_CTL_TOP_H_ */
