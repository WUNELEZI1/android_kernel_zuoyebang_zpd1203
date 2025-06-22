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

#ifndef _DPU_HW_PRE_3DLUT_H_
#define _DPU_HW_PRE_3DLUT_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_pre_3dlut_ops - the external interface of post 3dlut
 */
struct dpu_hw_pre_3dlut_ops {
	/**
	 * enable - enable or disable the post 3dlut module
	 * @hw: the pre_3dlut hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * set_3dlut - set 3dlut table
	 * @hw: the pre_3dlut hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_3dlut)(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the pre_3dlut hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw, bool use_cpu);

	/**
	 * get_3dlut - get 3dlut data for debug
	 * @hw: the pre_3dlut hardware pointer
	 * @tbl_data: pointer to 3dlut data
	 */
	void (*get_3dlut)(struct dpu_hw_blk *hw, void *tbl_data);
};

/**
 * dpu_hw_pre_3dlut - post 3dlut hardware object
 * @hw: the post 3dlut hardware info
 * @blk_cap: the post 3dlut hardware resource and capability
 * @ops: the post 3dlut function table
 */
struct dpu_hw_pre_3dlut {
	struct dpu_hw_blk hw;
	const struct dpu_pre_lut3d_cap *blk_cap;

	struct dpu_hw_pre_3dlut_ops ops;
};

static inline struct dpu_hw_pre_3dlut *
	to_dpu_hw_pre_3dlut(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_pre_3dlut, hw);
}

/**
 * dpu_hw_pre_3dlut_init - init post 3dlut hw block with its capability
 * @cap: the post 3dlut capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_pre_3dlut_init(struct dpu_pre_lut3d_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_pre_3dlut_deinit - deinit post 3dlut hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_pre_3dlut_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_PRE_3DLUT_H_ */
