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

#ifndef _DP_HW_TMG_OPS_H_
#define _DP_HW_TMG_OPS_H_

#include "dp_hw_common.h"
#include "dpu_hw_obuffer.h"

#define DP_TMG_MIN_H_FRONT_PORCH                     9
#define DP_TMG_MIN_H_SYNC_WIDTH                      4
#define DP_TMG_MIN_H_BACK_PORCH                      4
#define DP_TMG_MIN_H_BLANK                           17

void dp_hw_tmg_enable(struct dpu_hw_blk *hw, bool enable);
void dp_hw_tmg_config(struct dpu_hw_blk *hw, struct dp_display_timing *timing,
		enum dp_bpc bpc, bool dsc_en);
void dp_hw_tmg_get_state(struct dpu_hw_blk *hw, u32 *count, u32 *state);
void dp_hw_tmg_enable_self_test(struct dpu_hw_blk *hw, u32 mode_idx, bool enable);
void dp_hw_tmg_send_black_frame(struct dpu_hw_blk *hw);
void dp_hw_tmg_obufen_config(struct dpu_hw_blk *hw, u32 obufen);

/**
 * dp_hw_tmg_obuffer_level_config - config dp obuffer level
 *
 * @hw: hw res of dp
 * @obuffer: obuffer paramters to config
 */
void dp_hw_tmg_obuffer_level_config(struct dpu_hw_blk *hw, struct obuffer_parms *obuffer);

#endif /* _DP_HW_TMG_OPS_H_ */
