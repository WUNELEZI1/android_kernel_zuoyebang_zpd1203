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

#ifndef _DPU_HW_WB_TOP_H_
#define _DPU_HW_WB_TOP_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_wb_ops.h"

/**
 * dpu_hw_wb_top_ops - the virtual function table of wb_top class
 */
struct dpu_hw_wb_top_ops {
	/**
	 * init - initialize wb top block
	 * @hw: the wb_top hardware pointer
	 */
	void (*init)(struct dpu_hw_blk *hw);

	/**
	 * status_dump - dump hw status for debug
	 * @hw: the wb_top hardware pointer
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);

	/**
	 * status_clear - clear irq raw of wb_top
	 * @hw: the wb_top hardware pointer
	 */
	void (*status_clear)(struct dpu_hw_blk *hw);

	/**
	 * debug_enable - trigger the read only regs async transmission
	 * @hw: the wb_top hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*debug_enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * dbg_irq_enable - unmask wdma dbg irq
	 * @hw: the wb_top hardware pointer
	 * @enable: mask or unmask the irq
	 */
	void (*dbg_irq_enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * dbg_irq_clear - clear wdma dbg irq
	 * @hw: the wb_top hardware pointer
	 */
	void (*dbg_irq_clear)(struct dpu_hw_blk *hw);

	/**
	 * dbg_irq_dump - dump wdma dbg irq raw status
	 * @hw: the wb_top hardware pointer
	 *
	 * Returns: the value of wdma dbg irq raw status
	 */
	u32 (*dbg_irq_dump)(struct dpu_hw_blk *hw);
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
