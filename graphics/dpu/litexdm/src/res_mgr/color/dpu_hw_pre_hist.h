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

#ifndef _DPU_HW_PRE_HIST_H_
#define _DPU_HW_PRE_HIST_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_pre_hist_ops - the external interface of pre hist
 */
struct dpu_hw_pre_hist_ops {
	/**
	 * enable_update - init pre hist module's readdone config
	 * @hw: the pre_hist hardware pointer
	 */
	void (*enable_update)(struct dpu_hw_blk *hw);

	/**
	 * enable - enable or disable the pre hist module
	 * @hw: the pre_hist hardware pointer
	 * @enable: true for enabling or false for disabling
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * set_hist - set hist table
	 * @hw: the pre_hist hardware pointer
	 * @hw_cfg: pointer to prepq_hw_cfg
	 */
	void (*set_hist)(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the pre_hist hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);

	/**
	 * get_hist - get histogram collected data
	 * @hw: the pre_hist hardware pointer
	 * @base_addr: the start point in registor
	 * @end_addr: the end point
	 * @data: the collectd data from histogram
	 */
	void (*get_hist)(struct dpu_hw_blk *hw,
			int base_addr, int end_addr, void *data);
};

/**
 * dpu_hw_pre_hist - pre hist hardware object
 * @hw: the pre hist hardware info
 * @blk_cap: the pre hist hardware resource and capability
 * @ops: the pre hist function table
 */
struct dpu_hw_pre_hist {
	struct dpu_hw_blk hw;
	const struct dpu_pre_hist_cap *blk_cap;

	struct dpu_hw_pre_hist_ops ops;
};

static inline struct dpu_hw_pre_hist *
	to_dpu_hw_pre_hist(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_pre_hist, hw);
}

/**
 * dpu_hw_pre_hist_init - init histogram hw block with its capability
 * @cap: the pre hist capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_pre_hist_init(struct dpu_pre_hist_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_pre_hist_deinit - deinit histogram hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_pre_hist_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_PRE_HIST_H_ */
