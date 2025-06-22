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

#ifndef _DPU_HW_DPU_TOP_H_
#define _DPU_HW_DPU_TOP_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"

/**
 * dpu_hw_top_ops - the virtual function table of dpu_top class
 */
struct dpu_hw_top_ops {
	/**
	 * enable_wb_done_dpu_top - enable vynsc update
	 * @hw: the dpu_top hardware pointer
	 * @enable: enable or disable
	 */
	void (*enable_vsync_update)(struct dpu_hw_blk *hw, bool enable);
};

/**
 * dpu_hw_top - dpu_top hardware object
 * @hw: the dpu_top hardware info
 * @blk_cap: the dpu_top hardware resource and capability
 * @ops: the dpu_top function table
 */
struct dpu_hw_top {
	struct dpu_hw_blk hw;
	const struct dpu_top_cap *blk_cap;

	struct dpu_hw_top_ops ops;
};

static inline struct dpu_hw_top *to_dpu_hw_top(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_top, hw);
}

/**
 * dpu_hw_dpu_top_init - init one dpu_top hw block with its capability
 * @dpu_top_cap: the dpu_top capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_dpu_top_init(
		struct dpu_top_cap *dpu_top_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_dpu_top_deinit - deinit one dpu_top hw block with
 *                           base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_dpu_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_DPU_TOP_H_ */
