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

#ifndef _DP_HW_TMG_H_
#define _DP_HW_TMG_H_

#include "dp_hw_tmg_ops.h"
#include "dp_parser.h"

struct dp_hw_tmg_ops {
	/**
	 * config - config timing engine for display
	 * @hw: hardware memory address
	 * @timing: vertical and horizontal blank, active, front/back porch and so on
	 * @bpc: bits per component
	 * @dsc_en: if enable dsc
	 */
	void (*config)(struct dpu_hw_blk *hw, struct dp_display_timing *timing,
			enum dp_bpc bpc, bool dsc_en);

	/**
	 * enable - enable timing engine
	 * @hw: hardware memory address
	 * @enable: true or false
	 */
	void (*enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * get_state - get timing engine state
	 * @hw: hardware memory address
	 * @count: timing engine underflow frame count
	 * @state: timing state, including underflow, buf_empty, afifo_full and so on
	 */
	void (*get_state)(struct dpu_hw_blk *hw, u32 *count, u32 *state);

	/**
	 * enable_self_test - enable tmg dp self test mode
	 * @hw: hardware memory address
	 * @mode_idx: self test mode idx, 0 indicates turning off self test
	 * @enable: turn on/off tmg self test
	 */
	void (*enable_self_test)(struct dpu_hw_blk *hw, u32 mode_idx, bool enable);
};


/**
 * dp_hw_tmg - handle of timing engine hardware module
 * @hw: hardware memory address
 * @ops: timing engine supported operations
 */
struct dp_hw_tmg {
	struct dpu_hw_blk hw;

	struct dp_hw_tmg_ops *ops;
};

struct dp_hw_tmg *dp_hw_tmg_init(struct dp_blk_cap *cap);
void dp_hw_tmg_deinit(struct dp_hw_tmg *hw_tmg);

#endif
