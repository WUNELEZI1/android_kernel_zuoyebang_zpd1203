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

#ifndef _DPU_HW_WB_TOP_H_
#define _DPU_HW_WB_TOP_H_

#include "osal.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_common.h"

/**
 * dpu_hw_wb_top_ops - the virtual function table of wb_top class
 */
struct dpu_hw_wb_top_ops {
	/**
	 * init - initialize wb top block
	 * @hw: the wb_top hardware pointer
	 */
	void (*init)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_wb_top - wb_top hardware object
 * @hw: the wb_top hardware info
 * @blk_cap: the wb_top hardware resource and capability
 * @ops: the wb_top function table
 */
struct dpu_hw_wb_top {
	struct dpu_hw_blk hw;
	const struct dpu_wb_top_cap *blk_cap;

	struct dpu_hw_wb_top_ops ops;
};

static inline struct dpu_hw_wb_top *to_dpu_hw_wb_top(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_wb_top, hw);
}

/**
 * dpu_hw_wb_top_init - init one wb_top hw block with its capability
 * @wb_top_cap: the wb_top capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_wb_top_init(struct dpu_wb_top_cap *wb_top_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_wb_top_deinit - deinit one wb_top hw block with
 *                          base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_wb_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_WB_TOP_H_ */
