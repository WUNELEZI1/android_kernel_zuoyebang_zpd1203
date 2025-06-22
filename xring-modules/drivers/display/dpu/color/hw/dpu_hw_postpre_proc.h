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

#ifndef _DPU_HW_POSTPRE_PROC_H_
#define _DPU_HW_POSTPRE_PROC_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_postpre_proc_ops - the external interface of post
 */
struct dpu_hw_postpre_proc_ops {
	/**
	 * feature_enable - enable or disable the post pre process module
	 * currently contain: eotf, mul_matrix, oetf
	 * @hw: the post_pre_proc hardware pointer
	 * @ctrl: true for enabling or false for disabling
	 */
	void (*feature_enable)(struct dpu_hw_blk *hw,
			struct postpq_feature_ctrl *ctrl);

	/**
	 * set_eotf - set eotf cfg
	 * @hw: the post pre process hardware pointer
	 * @hw_cfg: pointer to post_pre_proc_hw_cfg
	 */
	void (*set_eotf_oetf)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * set_mul_matrix - set mul matrix cfg
	 * @hw: the post pre process hardware pointer
	 * @hw_cfg: pointer to post_pre_proc_hw_cfg
	 */
	void (*set_mul_matrix)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the postpq_top hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_postpre_proc - post pre proc hardware object
 * @hw: the post pre proc hardware info
 * @blk_cap: the post pre proc hardware resource and capability
 * @ops: the post pre proc function table
 */
struct dpu_hw_postpre_proc {
	struct dpu_hw_blk hw;
	const struct dpu_post_pre_proc_cap *blk_cap;
	struct dpu_hw_postpre_proc_ops ops;
};

static inline struct dpu_hw_postpre_proc *
	to_dpu_hw_postpre_proc(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_postpre_proc, hw);
}

/**
 * dpu_hw_postpre_proc_init - init postpre_proc hw block with its capability
 * @cap: the post capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_postpre_proc_init(struct dpu_post_pre_proc_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_postpre_proc_deinit - deinit postpre_proc hw block
 * @hw: base hw block pointer
 */
void dpu_hw_postpre_proc_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_POSTPRE_PROC_H_ */
