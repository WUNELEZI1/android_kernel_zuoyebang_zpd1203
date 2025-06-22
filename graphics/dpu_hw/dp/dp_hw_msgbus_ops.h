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

#ifndef _DP_HW_MSGBUS_OPS_H_
#define _DP_HW_MSGBUS_OPS_H_

#include "dp_hw_common.h"

void _dp_hw_msgbus_set_vswing_preemp(
		struct dpu_hw_blk *msgbus_hw,
		struct dpu_hw_blk *cr_hw,
		u32 (*signal_table)[4][3],
		u8 swing_level[DP_LANE_COUNT_MAX],
		u8 preemp_level[DP_LANE_COUNT_MAX]);

void dp_hw_msgbus_reserve_usb_tx_lane(struct dpu_hw_blk *hw,
		enum dp_lane_count lane_count);

void _dp_hw_msgbus_set_vboost(struct dpu_hw_blk *cr_hw, u32 vboost);
void _dp_hw_msgbus_set_ssc_val(struct dpu_hw_blk *cr_hw,
		u32 ssc_step_size, u32 ssc_peak);

#endif /* _DP_HW_MSGBUS_OPS_H_ */
