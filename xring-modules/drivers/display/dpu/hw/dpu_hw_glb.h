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

#ifndef _DPU_HW_GLB_H_
#define _DPU_HW_GLB_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_glb_ops.h"

/**
 * dpu_hw_glb_ops - the virtual function table of glb class
 */
struct dpu_hw_glb_ops {
	/**
	 * hdc_enable - enable hdc module
	 * @hw: the glb hardware pointer
	 */
	void (*hdc_enable)(struct dpu_hw_blk *hw);

	/**
	 * hdc_config - config hdc module
	 * @hw: the glb hardware pointer
	 * @profile_id: profile id
	 */
	void (*hdc_config)(struct dpu_hw_blk *hw, int profile_id);

	/**
	 * hdc_disable - disable the hdc module
	 * @hw: the glb hardware pointer
	 */
	void (*hdc_disable)(struct dpu_hw_blk *hw);

	/**
	 * hdc_get_hw_vote_val - get hdc current hw vote value
	 * @hw: the glb hardware pointer
	 *
	 * return current hw vote value
	 */
	u32 (*hdc_get_hw_vote_val)(struct dpu_hw_blk *hw);

	/**
	 * hdc_update_avs_codes - update dpu avs codes
	 * @avs_codes: the dpu avs codes array
	 */
	void (*hdc_update_avs_codes)(u8 *avs_codes);

	/**
	 * hdc_dump - dump hdc work status
	 * @hw: the glb hardwware pointer
	 * @status: the hdc work status
	 */
	void (*hdc_dump)(struct dpu_hw_blk *hw, struct hdc_status *status, int flag);
};

/**
 * dpu_hw_glb - glb hardware object
 * @hw: the glb hardware info
 * @blk_cap: the glb hardware resource and capability
 * @ops: the glb function table
 */
struct dpu_hw_glb {
	struct dpu_hw_blk hw;
	const struct dpu_glb_cap *blk_cap;

	struct dpu_hw_glb_ops ops;
};

static inline struct dpu_hw_glb *to_dpu_hw_glb(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_glb, hw);
}

/**
 * dpu_hw_glb_init - init glb hw block with its capability
 * @glb_cap: the glb capability pointer
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_glb_init(struct dpu_glb_cap *glb_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_glb_deinit - deinit glb hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_glb_deinit(struct dpu_hw_blk *hw);

#endif /* _DPU_HW_GLB_H_ */
