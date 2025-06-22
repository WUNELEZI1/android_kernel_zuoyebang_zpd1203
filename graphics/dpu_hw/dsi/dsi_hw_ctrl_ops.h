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

#ifndef _DSI_HW_CTRL_OPS_H_
#define _DSI_HW_CTRL_OPS_H_

#include "dpu_hw_dsi.h"

#define DEFAULT_WAIT_TIMEOUT_MS 25

void dsi_hw_sctrl_phy_reset(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
void dsi_hw_sctrl_deinit(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_sctrl_init(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_sctrl_ipi_clk_config(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg);
void dsi_hw_sctrl_vg_en(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
void dsi_hw_sctrl_frame_update(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_hw_sctrl_phy_pll_config - config the phy pll of sctrl
 * @hw: the pointer of sctrl dpu hw blk
 * @cfg: the pointer of dsi controller config
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_sctrl_phy_pll_config(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg);

/**
 * dsi_hw_sctrl_dyn_freq_wait_pll_lock - wait sctrl pll lock
 * @hw: the pointer of sctrl dpu hw blk
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_sctrl_dyn_freq_wait_pll_lock(struct dpu_hw_blk *hw);

/**
 * dsi_hw_sctrl_dyn_freq_wait_phy_ready - wait sctrl phy ready
 * @hw: the pointer of sctrl dpu hw blk
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_sctrl_dyn_freq_wait_phy_ready(struct dpu_hw_blk *hw);

void dsi_hw_ctrl_deinit(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_ctrl_clk_cfg(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_ctrl_ulps_enter(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_ctrl_ulps_exit(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

int dsi_hw_ctrl_enable(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_ctrl_disable(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
void dsi_hw_ctrl_reset(struct dsi_ctrl_hw_blk *hw);

int dsi_hw_ctrl_init_phy(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

int dsi_hw_init(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
int dsi_hw_wait_ready(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

int dsi_hw_ctrl_send_pkt(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg);
int dsi_hw_ctrl_read_pkt_pre_tx(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg);
int dsi_hw_ctrl_read_pkt(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg, u32 wait_timeout);
int dsi_hw_ctrl_cmdlist_send(struct dsi_ctrl_hw_blk *hw, struct dsi_cmds *cmds);
int dsi_hw_ctrl_send_pkt_by_cmdlist(struct dsi_ctrl_hw_blk *hw,
		struct dsi_msg *msg);
int dsi_hw_ctrl_tx_ctrl(struct dsi_ctrl_hw_blk *hw, u8 cri_mode, u8 cri_hold);

/**
 * dsi_hw_ctrl_mipi_freq_update - update mipi frequency
 * @hw: the pointer of ctrl hw blk
 * @cfg: the pointer of dsi controller config
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_ctrl_mipi_freq_update(struct dsi_ctrl_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg);

/**
 * dsi_hw_debug - for dsi debug interface
 * @hw: the pointer of ctrl hw blk
 * @type: debug funcion select
 * @parm_cnt: parameters count
 * @parm: debug parmeters
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_debug(struct dsi_ctrl_hw_blk *hw,
		enum dsi_debug type, u8 parm_cnt, u8 *parm);

/**
 * dsi_hw_check_ulps_state - check ulps state
 *
 * @ctrl_hw_blk：the pointer of ctrl hw blk
 * @enter: enter or exit
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_check_ulps_state(struct dsi_ctrl_hw_blk *ctrl_hw_blk, u8 enter);

/**
 * dsi_hw_ctrl_cri_is_busy
 *
 * @hw: the pointer of ctrl hw blk
 * @timeout_ms: threshold for timeout
 * Return: 0 on success or a negative number on failure.
 */
int dsi_hw_ctrl_cri_is_busy(struct dsi_ctrl_hw_blk *hw, u32 timeout_ms);
#endif /* _DSI_HW_CTRL_OPS_H_ */
