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

#ifndef _DPU_HW_DSC_H_
#define _DPU_HW_DSC_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsc_ops.h"

/**
 * dpu_hw_dsc_ops - the virtual function table of dsc class
 */
struct dpu_hw_dsc_ops {
	/**
	 * enable - enable dsc module
	 * @hw: the pointer of dsc hardware block
	 * @dsc_config: parameters of dsc
	 */
	void (*enable)(struct dpu_hw_blk *hw, struct dpu_dsc_config *dsc_config);

	/**
	 * disable - disable dsc module
	 * @hw: the pointer of dsc hardware block
	 */
	void (*disable)(struct dpu_hw_blk *hw);

	/**
	 * get_caps - get dsc module capabilities
	 * @hw: the pointer of dsc hardware block
	 * @caps_src: capabilities of DSC hardware module
	 */
	void (*get_caps)(struct dpu_hw_blk *hw, struct dsc_caps_src *caps_src);

	/**
	 * height_set - change dsc height
	 * @hw: the pointer of dsc hardware block
	 * @height: pic height of DSC hardware module
	 * @dual_port: Dual port dsi panel set true
	 */
	void (*height_set)(struct dpu_hw_blk *hw, u16 height, bool dual_port);
};

/**
 * dpu_hw_dsc - dsc hardware object
 * @hw: the dsc hardware info
 * @blk_cap: the dsc hardware resource and capability
 * @ops: the dsc function table
 */
struct dpu_hw_dsc {
	struct dpu_hw_blk hw;
	const struct dpu_dsc_cap *blk_cap;

	struct dpu_hw_dsc_ops ops;
};

static inline struct dpu_hw_dsc *to_dpu_hw_dsc(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_dsc, hw);
}

/**
 * dpu_hw_dsc_init - init one dsc hw block with its capability
 * @dsc_cap: the dsc capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_dsc_init(struct dpu_dsc_cap *dsc_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_dsc_deinit - deinit one dsc hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_dsc_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_DSC_H_ */
