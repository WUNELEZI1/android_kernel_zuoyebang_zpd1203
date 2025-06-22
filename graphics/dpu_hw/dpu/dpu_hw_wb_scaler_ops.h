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

#ifndef _DPU_HW_WB_SCALER_OPS_H_
#define _DPU_HW_WB_SCALER_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_color_common.h"

/**
 * dpu_hw_wb_scaler_enable - enable or disable the wb scaler
 * @hw: the wb scaler hardware pointer
 * @enable: true for enabling or false for disabling
 */
void dpu_hw_wb_scaler_enable(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_wb_scaler_set - set wb scaler parameters
 * @hw: the wb scaler hardware pointer
 * @scaler: the pointer of wb scaler
 */
void dpu_hw_wb_scaler_set(struct dpu_hw_blk *hw,
		struct dpu_1d_scaler_cfg  *scaler);

#endif /* _DPU_HW_WB_SCALER_OPS_H_ */
