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

#ifndef _DPU_HW_DVS_OPS_H_
#define _DPU_HW_DVS_OPS_H_

#include "dpu_hw_common.h"

/**
 * dpu_hw_dvs_sw_to_hw_vote - change dpu sw dvfs to hw dvfs
 * @hw: dvs hw block
 */
void dpu_hw_dvs_sw_to_hw_vote(struct dpu_hw_blk *hw);

/**
 * dpu_hw_dvs_hw_to_sw_vote - change dpu hw dvfs to sw dvfs
 * @hw: dvs hw block
 * @hw_volt: hw dvfs configure value
 */
void dpu_hw_dvs_hw_to_sw_vote(struct dpu_hw_blk *hw, u32 hw_volt);

/**
 * dpu_hw_dvs_state_dump - dump dvs hw state
 * @hw: dvs hw block
 * @cnt: counter for polling dvs arbit reg
 */
void dpu_hw_dvs_state_dump(struct dpu_hw_blk *hw, int cnt);
#endif
