// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _DPU_HW_INTR_H_
#define _DPU_HW_INTR_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_intr_ops.h"

/**
 * dpu_hw_intr_ops - the virtual function table of interrupt class
 */
struct dpu_hw_intr_ops {
	/**
	 * enable - enable an interrupt
	 * @hw: the intr hardware block pointer
	 * @intr_id: the interrupt index
	 * @enable: enable or disable
	 */
	void (*enable)(struct dpu_hw_blk *hw,
		enum dpu_intr_id intr_id, bool enable);

	/**
	 * status_get - get an interrupt status
	 * @hw: the intr hardware block pointer
	 * @intr_id: the interrupt index
	 *
	 * Return: 1 on triggered, 0 on not-triggered
	 */
	bool (*status_get)(struct dpu_hw_blk *hw,
		enum dpu_intr_id intr_id);

	/**
	 * enable - clear an interrupt status
	 * @hw: the intr hardware block pointer
	 * @intr_id: the interrupt index
	 */
	void (*status_clear)(struct dpu_hw_blk *hw,
		enum dpu_intr_id intr_id);
};

/**
 * dpu_hw_intr - intr hardware object
 * @hw: the intr hardware info
 * @blk_cap: the intr hardware resource and capability
 * @ops: the intr function table
 */
struct dpu_hw_intr {
	struct dpu_hw_blk hw;
	const struct dpu_intr_cap *blk_cap;

	struct dpu_hw_intr_ops ops;
};

static inline struct dpu_hw_intr *to_dpu_hw_intr(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_intr, hw);
}

/**
 * dpu_hw_intr_init - init one intr hw block with its capability
 * @intr_cap: the intr capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_intr_init(
		struct dpu_intr_cap *intr_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_intr_deinit - deinit one intr hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_intr_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_INTR_H_ */
