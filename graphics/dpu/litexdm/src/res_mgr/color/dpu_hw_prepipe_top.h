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

#ifndef _DPU_HW_PREPIPE_TOP_H_
#define _DPU_HW_PREPIPE_TOP_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_color_internal.h"

/**
 * dpu_hw_prepipe_top_ops - the external interface of prepipe top
 */
struct dpu_hw_prepipe_top_ops {
	/**
	 * feature_enable - enable or disable the prepipe process top sub modules
	 * currently contain: wb right sign, r2y, y2r, scale, sbs
	 * @hw: the prepipe top hardware pointer
	 * @feature: choose feature to enable
	 * @ctrl: prepq feature control info
	 */
	void (*feature_enable)(struct dpu_hw_blk *hw, struct prepq_feature_ctrl *ctrl);

	/**
	 * set_r2y - set r2y cfg
	 * @hw: the prepipe top hardware pointer
	 * @hw_cfg: pointer to prepq_hw_cfg
	 */
	void (*set_r2y)(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);

	/**
	 * set_y2r - set y2r cfg
	 * @hw: the prepipe top hardware pointer
	 * @hw_cfg: pointer to prepq_hw_cfg
	 */
	void (*set_y2r)(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);

	/**
	 * set_non pre alpha - set nonpre alpha cfg
	 * @hw: the prepipe top hardware pointer
	 * @hw_cfg: pointer to prepq_hw_cfg
	 */
	void (*set_nonpre_alpha)(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg);

	/**
	 * flush - flush module configurations
	 * @hw: the prepipe top hardware pointer
	 */
	void (*flush)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_prepipe_top - prepipe top hardware object
 * @hw: the prepipe top hardware info
 * @blk_cap: the prepipe top hardware resource and capability
 * @ops: the prepipe top function table
 */
struct dpu_hw_prepipe_top {
	struct dpu_hw_blk hw;
	const struct dpu_prepipe_top_cap *blk_cap;
	struct dpu_hw_prepipe_top_ops ops;
};

static inline struct dpu_hw_prepipe_top *
to_dpu_hw_prepipe_top(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_prepipe_top, hw);
}

/**
 * dpu_hw_prepipe_top_init - init prepipe top hw block with its capability
 * @cap: the prepipe top capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_prepipe_top_init(struct dpu_prepipe_top_cap *cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_prepipe_top_deinit - deinit prepie top hw block
 * @hw: base hw block pointer
 */
void dpu_hw_prepipe_top_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_PREPIPE_TOP_H_ */
