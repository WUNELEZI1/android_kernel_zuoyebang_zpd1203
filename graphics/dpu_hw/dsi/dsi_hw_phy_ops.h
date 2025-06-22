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

#include "dpu_hw_common.h"
#include "dpu_hw_dsi.h"

#ifndef _DSI_HW_PHY_OPS_H_
#define _DSI_HW_PHY_OPS_H_

int dsi_hw_phy_init(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);
void dsi_hw_phy_deinit(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
int dsi_hw_phy_pll_cfg(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
int dsi_hw_phy_wait_ready(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);

/**
 * dsi_hw_phy_dyn_freq_pll_disable - disable phy pll for dynamic frequency
 * @hw: the pointer of phy hw blk
 */
void dsi_hw_phy_dyn_freq_pll_disable(struct dpu_hw_blk *hw);

/**
 * dsi_hw_phy_dyn_freq_pll_enable - enable phy pll for dynamic frequency
 * @hw: the pointer of phy hw blk
 */
void dsi_hw_phy_dyn_freq_pll_enable(struct dpu_hw_blk *hw);

/**
 * dsi_hw_phy_mipi_freq_update - update mipi frequency
 * @hw: the pointer of phy hw blk
 * @cfg: the pointer of dsi controller config
 */
int dsi_hw_phy_mipi_freq_update(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_hw_phy_set_eq_debug - set phy eq parmaters
 * @hw: the pointer of phy hw blk
 * @setr: eq range select
 * @eqa: eq paramter a
 * @eqb: eq paramter b
 */
void dsi_hw_phy_set_eq_debug(struct dpu_hw_blk *hw, u8 setr, u8 eqa, u8 eqb);

/**
 * dsi_hw_phy_set_amplitude - set dphy amplitude
 * @hw: the pointer of phy hw blk
 * @amplitude: phy amplitude parameter
 */
void dsi_hw_phy_set_amplitude(struct dpu_hw_blk *hw, u32 amplitude);

/**
 * dsi_hw_phy_get_eq_parms - get phy eq params
 * @hw: the pointer of phy hw blk
 */
u32 dsi_hw_phy_get_eq_parms(struct dpu_hw_blk *hw);


#endif /* _DSI_HW_PHY_OPS_H_ */
