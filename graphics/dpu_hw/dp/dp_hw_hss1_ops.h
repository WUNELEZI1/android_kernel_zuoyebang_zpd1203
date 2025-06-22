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

#ifndef _DP_HW_HSS1_OPS_H_
#define _DP_HW_HSS1_OPS_H_

#include "dp_hw_common.h"

#define DP_IPI_CLOCK_HZ_MIN                            24000000
#define DP_IPI_CLOCK_HZ_MAX                            576000000

int dp_hw_hss1_power_on(struct dpu_hw_blk *hw);
void dp_hw_hss1_power_off(struct dpu_hw_blk *hw);
int dp_hw_hss1_ipi_clock_gate_on(struct dpu_hw_blk *hw);
void dp_hw_hss1_ipi_clock_gate_off(struct dpu_hw_blk *hw);
int dp_hw_hss1_set_ipi_clock_div_ratio(struct dpu_hw_blk *hw, u32 div_ratio);

#endif /* _DP_HW_HSS1_OPS_H_ */
