/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _DPU_HW_DVS_H_
#define _DPU_HW_DVS_H_

#include "dpu_hw_dvs_ops.h"

struct dpu_hw_dvs_ops {
	/**
	 * sw_to_hw_vote - change sw dvfs to hw dvfs
	 * @hw: hardware memory address
	 */
	void (*sw_to_hw_vote)(struct dpu_hw_blk *hw);

	/**
	 * hw_to_sw_vote - change hw dvfs to sw dvfs
	 * @hw: hardware memory address
	 */
	void (*hw_to_sw_vote)(struct dpu_hw_blk *hw, u32 hw_volt);

	/**
	 * dvs_state_dump - dump current dvs state
	 * @hw: hardware memory address
	 * @cnt: counter for polling dvs state reg
	 */
	void (*dvs_state_dump)(struct dpu_hw_blk *hw, int cnt);
};

/**
 * dpu_hw_dvs - handle of dvs module
 * @hw: hardware memory address
 * @ops: dvs supported operations
 */
struct dpu_hw_dvs {
	struct dpu_hw_blk hw;

	struct dpu_hw_dvs_ops *ops;
};

struct dpu_hw_dvs *dpu_hw_dvs_init(void);
void dpu_hw_dvs_deinit(struct dpu_hw_dvs *hw_dvs);

#endif
