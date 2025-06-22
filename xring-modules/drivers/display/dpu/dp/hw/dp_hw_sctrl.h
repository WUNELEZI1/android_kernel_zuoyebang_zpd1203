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

#ifndef _DP_HW_SCTRL_H_
#define _DP_HW_SCTRL_H_

#include "dp_hw_sctrl_ops.h"
#include "dp_parser.h"

struct dp_hw_sctrl_ops {
	void (*reset_state)(struct dpu_hw_blk *hw);
	void (*clear_reset)(struct dpu_hw_blk *hw);
	void (*hpd_trigger)(struct dpu_hw_blk *hw, bool high);
	void (*enable_self_test)(struct dpu_hw_blk *hw,
			struct dp_hw_sctrl_vg_config *config, u32 mode_idx, bool enable);
	void (*power_manage_ext_sdp)(struct dpu_hw_blk *hw, bool enable);
	void (*send_ext_sdp)(struct dpu_hw_blk *hw, struct dp_sdp_packet *sdp, u8 count);
	void (*suspend_ext_sdp)(struct dpu_hw_blk *hw);
	void (*powerdown_rate_sync_bypass)(struct dpu_hw_blk *hw);
	void (*aux_enable)(struct dpu_hw_blk *hw, bool enable);
	int (*pll_request)(struct dpu_hw_blk *hw, u8 req);
};

struct dp_hw_sctrl {
	struct dpu_hw_blk hw;

	struct dp_hw_sctrl_ops *ops;
};

struct dp_hw_sctrl *dp_hw_sctrl_init(struct dp_blk_cap *cap);
void dp_hw_sctrl_deinit(struct dp_hw_sctrl *sctrl);

#endif
