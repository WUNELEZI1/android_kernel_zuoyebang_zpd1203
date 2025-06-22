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

#ifndef _DP_HW_SCTRL_OPS_H_
#define _DP_HW_SCTRL_OPS_H_

#include "dp_hw_common.h"
#include "dp_hw_sctrl_reg.h"

#define DP_MAXPCLKREQ_PLL_ON_REF_CLK_ON            0x3
#define DP_MAXPCLKREQ_PLL_ON_REF_CLK_OFF           0x1
#define DP_MAXPCLKREQ_PLL_OFF_REF_CLK_ON           0x2
#define DP_MAXPCLKREQ_PLL_OFF_REF_CLK_OFF          0x0

#define DP_COMBO_MODE_ACK_DISABLE                  1
#define DP_COMBO_MODE_ACK_ENABLE                   0

struct dp_hw_sctrl_vg_config {
	struct dp_display_timing *timing;
	enum dp_bpc bpc;
};

void dp_hw_sctrl_set_g_combo_mode(enum dp_combo_mode combo_mode);
void dp_hw_sctrl_set_g_orientation(enum dp_orientation orientation);

int _dp_hw_sctrl_wait_phy_status(struct dpu_hw_blk *hw);
void _dp_hw_sctrl_combo_mode_ack(struct dpu_hw_blk *hw, bool ack);
int dp_hw_sctrl_pll_request(struct dpu_hw_blk *hw, u8 req);
int _dp_hw_sctrl_restart_combo_phy_pll(struct dpu_hw_blk *hw);
void dp_hw_sctrl_powerdown_rate_sync_bypass(struct dpu_hw_blk *hw);
void dp_hw_sctrl_clear_reset(struct dpu_hw_blk *hw);
void dp_hw_sctrl_reset_state(struct dpu_hw_blk *hw);
void dp_hw_sctrl_hpd_trigger(struct dpu_hw_blk *hw, bool high);
void _dp_hw_sctrl_set_aux_orientation(struct dpu_hw_blk *hw);
void dp_hw_sctrl_enable_self_test(struct dpu_hw_blk *hw,
		struct dp_hw_sctrl_vg_config *config, u32 mode_idx, bool enable);
void dp_hw_sctrl_power_manage_ext_sdp(struct dpu_hw_blk *hw, bool enable);
void dp_hw_sctrl_send_ext_sdp(struct dpu_hw_blk *hw, struct dp_sdp_packet *sdp,
		u8 count);
void dp_hw_sctrl_suspend_ext_sdp(struct dpu_hw_blk *hw);
void dp_hw_sctrl_aux_enable(struct dpu_hw_blk *hw, bool enable);

#endif /* _DP_HW_SCTRL_OPS_H_ */
