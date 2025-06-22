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

#ifndef _DPU_HW_POST_HIST_H_
#define _DPU_HW_POST_HIST_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_post_hist_ops - the external interface of post hist
 */
struct dpu_hw_post_hist_ops {
	/**
	 * enable_update - init post hist module's readdone config
	 * @hw: the post_hist hardware pointer
	 */
	void (*enable_update)(struct dpu_hw_blk *hw);

	/**
	 * enable - enable or disable the post hist module
	 * @hw: the post_hist hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * set_hist - set hist table
	 * @hw: the post_hist hardware pointer
	 * @hw_cfg: pointer to postpq_hw_cfg
	 */
	void (*set_hist)(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the post_hist hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);

	/**
	 * get_hist - get histogram collected data
	 * @hw: the post_hist hardware pointer
	 * @base_addr: the start point in registor
	 * @end_addr: the end point
	 * @data: the collectd data from histogram
	 */
	void (*get_hist)(struct dpu_hw_blk *hw,
			int base_addr, int end_addr, void *data);
};

/**
 * dpu_hw_post_hist - post hist hardware object
 * @hw: the post hist hardware info
 * @blk_cap: the post hist hardware resource and capability
 * @ops: the post hist function table
 */
struct dpu_hw_post_hist {
	struct dpu_hw_blk hw;
	const struct dpu_post_hist_cap *blk_cap;

	struct dpu_hw_post_hist_ops ops;
};

static inline struct dpu_hw_post_hist *
	to_dpu_hw_post_hist(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_post_hist, hw);
}

/**
 * dpu_hw_post_hist_init - init histogram hw block with its capability
 * @cap: the post hist capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_post_hist_init(struct dpu_post_hist_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_post_hist_deinit - deinit histogram hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_post_hist_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_POST_HIST_H_ */
