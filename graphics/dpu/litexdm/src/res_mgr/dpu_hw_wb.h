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

#ifndef _DPU_HW_WB_H_
#define _DPU_HW_WB_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_wb_ops.h"

#include "dpu_hw_ctl_ops.h"

/**
 * dpu_hw_wb_ops - the virtual function table of wb class
 */
struct dpu_hw_wb_ops {
	/**
	 * frame_config - update the wb input and output config
	 * @hw: the wb hardware pointer
	 * @cfg: the pointer of wb frame related config
	 */
	void (*frame_config)(struct dpu_hw_blk *hw, struct wb_frame_cfg *cfg);

	/**
	 * reset - reset the wb register
	 * @hw: the wb hardware pointer
	 */
	void (*reset)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_wb - wb hardware object
 * @hw: the wb hardware info
 * @blk_cap: the wb hardware resource and capability
 * @ops: the wb function table
 */
struct dpu_hw_wb {
	struct dpu_hw_blk hw;
	const struct dpu_wb_core_cap *blk_cap;

	struct dpu_hw_wb_ops ops;
};

static inline struct dpu_hw_wb *to_dpu_hw_wb(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_wb, hw);
}

/**
 * dpu_hw_wb_init - init one wb hw block with its capability
 * @wb_cap: the wb capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_wb_init(struct dpu_wb_core_cap *wb_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_wb_deinit - deinit one wb hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_wb_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_WB_TOP_H_ */
