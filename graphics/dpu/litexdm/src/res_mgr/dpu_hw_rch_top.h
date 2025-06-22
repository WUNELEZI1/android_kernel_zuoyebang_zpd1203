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

#ifndef _DPU_HW_RCH_TOP_H_
#define _DPU_HW_RCH_TOP_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"

/**
 * dpu_hw_rch_top_ops - the virtual function table of rch_top class
 */
struct dpu_hw_rch_top_ops {
	/**
	 * enable_icg - enable clock
	 * @hw: the rch_top hardware pointer
	 * @enable: enable or disable
	 */
	void (*enable_icg)(struct dpu_hw_blk *hw, bool enable);
};

/**
 * dpu_hw_rch_top - rch_top hardware object
 * @hw: the rch_top hardware info
 * @blk_cap: the rch_top hardware resource and capability
 * @ops: the rch_top function table
 */
struct dpu_hw_rch_top {
	struct dpu_hw_blk hw;
	const struct dpu_rch_top_cap *blk_cap;

	struct dpu_hw_rch_top_ops ops;
};

static inline struct dpu_hw_rch_top *to_dpu_hw_rch_top(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_rch_top, hw);
}

/**
 * dpu_hw_rch_top_init - init one rch_top hw block with its capability
 * @rch_top_cap: the rch_top capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_rch_top_init(
		struct dpu_rch_top_cap *rch_top_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_rch_top_deinit - deinit one rch_top hw block with
 *                           base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_rch_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_RCH_TOP_H_ */

