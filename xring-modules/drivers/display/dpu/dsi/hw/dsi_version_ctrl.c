// SPDX-License-Identifier: GPL-2.0-only
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

#include "dsi_version_ctrl.h"
#include "dsi_ctrl_hw.h"
#include "dsi_phy_hw.h"
#include "dpu_log.h"

/**
 * dsi_ctrl_hw_2_0_ops_init() - init dsi controller hw callback func
 */
static void dsi_ctrl_hw_2_0_ops_init(struct dsi_ctrl_hw *ctrl,
		enum dsi_ctrl_version version)
{
	switch (version) {
	case DSI_CTRL_VERSION_2_0:
		/* TODO: add ctrl hw ops */
		ctrl->ops.deinit = dsi_hw_ctrl_deinit;
		ctrl->ops.clk_config = dsi_hw_ctrl_clk_cfg;
		ctrl->ops.ulps_enter = dsi_hw_ctrl_ulps_enter;
		ctrl->ops.ulps_exit = dsi_hw_ctrl_ulps_exit;
		ctrl->ops.enable = dsi_hw_ctrl_enable;
		ctrl->ops.disable = dsi_hw_ctrl_disable;
		ctrl->ops.reset = dsi_hw_ctrl_reset;
		ctrl->ops.phy_init = dsi_hw_ctrl_init_phy;
		ctrl->ops.send_pkt = dsi_hw_ctrl_send_pkt;
		ctrl->ops.send_pkt_cmdlist = dsi_hw_ctrl_send_pkt_by_cmdlist;
		ctrl->ops.read_pkt = dsi_hw_ctrl_read_pkt;
		ctrl->ops.read_pkt_pre_tx = dsi_hw_ctrl_read_pkt_pre_tx;
		ctrl->ops.hw_init = dsi_hw_init;
		ctrl->ops.wait_ready = dsi_hw_wait_ready;
		ctrl->ops.mipi_freq_update = dsi_hw_ctrl_mipi_freq_update;
		ctrl->ops.dsi_debug = dsi_hw_debug;
		ctrl->ops.cri_is_busy = dsi_hw_ctrl_cri_is_busy;
		ctrl->ops.cri_tx_ctrl = dsi_hw_ctrl_tx_ctrl;
		DSI_DEBUG("regist v2.0 ops success\n");
		break;
	default:
		DSI_ERROR("invalid dsi ctrl version\n");
		break;
	}
}

/**
 * dsi_phy_hw_1_2_ops_init() - init dsi phy hw callback func
 */
static void dsi_phy_hw_1_2_ops_init(struct dsi_phy_hw *phy,
		enum dsi_phy_version version)
{
	switch (version) {
	case DSI_PHY_VERSION_1_2:
		/* TODO: add phy hw ops */
		phy->ops.deinit = dsi_hw_phy_deinit;
		phy->ops.wait_ready = dsi_hw_phy_wait_ready;
		DSI_DEBUG("regist v1.2 ops success\n");
		break;
	default:
		break;
	}
}

int dsi_ctrl_version_ctrl(struct dsi_ctrl_hw *ctrl)
{
	enum dsi_ctrl_version version = ctrl->version;

	switch (version) {
	case DSI_CTRL_VERSION_2_0:
		dsi_ctrl_hw_2_0_ops_init(ctrl, version);
		break;
	default:
		DSI_ERROR("unsupported controller version: %d\n", version);
		return -EOPNOTSUPP;
	}

	return 0;
}

int dsi_phy_version_ctrl(struct dsi_phy_hw *phy)
{
	enum dsi_phy_version version = phy->version;

	switch (version) {
	case DSI_PHY_VERSION_1_2:
		dsi_phy_hw_1_2_ops_init(phy, version);
		break;
	default:
		DSI_ERROR("unsupported phy version: %d\n", version);
		return -EOPNOTSUPP;
	}

	return 0;
}

int dsi_tmg_version_ctrl(struct dsi_tmg_hw *tmg)
{
	tmg->ops.init = dsi_hw_tmg_init;
	tmg->ops.enable = dsi_hw_tmg_enable;
	tmg->ops.disable = dsi_hw_tmg_disable;
	tmg->ops.ulps_enter = dsi_hw_tmg_ulps_entry;
	tmg->ops.ulps_exit = dsi_hw_tmg_ulps_exit;
	tmg->ops.partial_update = dsi_hw_tmg_partial_update;
	tmg->ops.frame_update = dsi_hw_tmg_frame_update;
	tmg->ops.timestamp_get = dsi_hw_tmg_timestamp_get;
	tmg->ops.dyn_freq_update = dsi_hw_tmg_dyn_freq_update;
	tmg->ops.doze_enable = dsi_hw_tmg_doze_enable;
	tmg->ops.status_dump = dsi_hw_tmg_status_dump;

	return 0;
}
