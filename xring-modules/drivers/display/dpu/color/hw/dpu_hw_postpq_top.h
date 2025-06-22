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

#ifndef _DPU_HW_POSTPQ_TOP_H_
#define _DPU_HW_POSTPQ_TOP_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_postpq_top_ops - the external interface of postpq top
 */
struct dpu_hw_postpq_top_ops {
	/**
	 * init - init pipe configurations
	 * @hw: the postpq_top hardware pointer
	 */
	void (*init)(struct dpu_hw_blk *hw, struct postpq_hw_init_cfg *cfg);

	/**
	 * feature_enable - enable or disable the postpq top
	 * postpq contain: r2y, 2dscler, y2r, dither,  dither lite, csc_matrix
	 * @hw: the postpq_top hardware pointer
	 * @ctrl: enable flag for features of postpq top
	 */
	void (*feature_enable)(struct dpu_hw_blk *hw,
			struct postpq_feature_ctrl *ctrl);

	/**
	 * set_scaler_output_size - set postpq top scaler output size
	 * @hw: the postpq_top hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_scaler_output_size)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * set_dither - set dither cfg
	 * @hw: the postpq_top hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_dither)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * set_r2y - set r2y cfg
	 * @hw: the postpq_top hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_r2y)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * set_y2r - set y2r cfg
	 * @hw: the postpq_top hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_y2r)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * set_csc_matrix - set csc matrix cfg
	 * @hw: the postpq_top hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_csc_matrix)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * get_dither_loc - get the dither processing location for debug
	 * @hw: the postpq_top hardware pointer
	 * @loc: pointer to location info
	 */
	void (*get_dither_loc)(struct dpu_hw_blk *hw, void *loc);

	/**
	 * get_loc - get the processing location for debug
	 * @hw: the postpq_top hardware pointer
	 * @loc: pointer to location info
	 */
	void (*get_loc)(struct dpu_hw_blk *hw, void *loc);

	/**
	 * flush - flush module configurations
	 * @hw: the postpq_top hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_postpq_top - post pq top hardware object
 * @hw: the post postpq top hardware info
 * @blk_cap: the postpq top hardware resource and capability
 * @ops: the post postpq top function table
 */
struct dpu_hw_postpq_top {
	struct dpu_hw_blk hw;
	const struct dpu_post_pq_top_cap *blk_cap;
	struct dpu_hw_postpq_top_ops ops;
};

static inline struct dpu_hw_postpq_top *
	to_dpu_hw_postpq_top(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_postpq_top, hw);
}

/**
 * dpu_hw_postpq_top_init - init postpq_top hw block with its capability
 * @cap: the post capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_postpq_top_init(struct dpu_post_pq_top_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_postpq_top_deinit - deinit postpq top hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_postpq_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_POSTPQ_TOP_H_ */
