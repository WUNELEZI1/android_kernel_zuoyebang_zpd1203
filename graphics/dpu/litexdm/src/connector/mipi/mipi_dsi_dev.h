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

#ifndef _MIPI_DSI_DEV_H_
#define _MIPI_DSI_DEV_H_

#include "dpu_hw_common.h"
#include "dpu_common_info.h"

struct connector;
struct dsi_ctrl_hw_blk;
struct platform_device;

 /**
 * struct mipi_dsi_msg - standard msg object used to mipi tx/rx
 *
 * @channel: cmd channel
 * @type: cmd type
 * @flags: cmd flag
 * @tx_len: tx cmd size
 * @tx_buf: tx cmds
 * @rx_len: rx cmd size
 * @rx_buf: rx cmds
 */
struct mipi_dsi_msg {
	uint8_t channel;
	uint8_t type;
	uint16_t flags;

	size_t tx_len;
	void *tx_buf;

	size_t rx_len;
	void *rx_buf;
};

/**
 * dsi_cmd_desc - description of a dsi command
 * @msg: dsi mipi msg packet
 * @post_wait_us: post wait duration
 */
struct dsi_cmd_desc {
	struct mipi_dsi_msg msg;
	u32 post_wait_us;
};

/**
 * mipi_ops_set_up - set up mipi ops and hw res
 *
 * @connector: connector object
 */
void mipi_ops_set_up(struct connector *connector);

void mipi_ops_remove(struct connector *connector);

/**
 * mipi_set_backlight - set backlight will call panel ops
 *
 * @pdev: platform_device
 * @id: scene id
 * @value: abstract transfer parameters
 * @return 0 on success, -1 on failure
 */
int32_t mipi_set_backlight(struct platform_device *pdev, uint32_t id, void *value);

/**
 * get_dsi_ctrl_hw - Get the dsi ctrl hw object
 *
 * @connector: connector object
 * @hw: dsi ctrl hw blk used in lowlevel
 */
void get_dsi_ctrl_hw(struct connector *connector, struct dsi_ctrl_hw_blk *hw);

/**
 * dsi_ctrl_cmd_desc_transfer - api for mipi tx/rx
 *
 * @hw: dsi ctrl hw blk used in lowlevel
 * @cmd_desc: cmd info need to be send
 * @return 0 on success, -1 on failure
 */
int dsi_ctrl_cmd_desc_transfer(struct dsi_ctrl_hw_blk *hw,
		struct dsi_cmd_desc *cmd_desc);

/**
 * dsi_ipi_clk_mode_get - get ipi clk mode
 *
 * @pinfo: struct dpu_panel_info
 * @return uint32_t. ipi clk mode
 */
uint32_t dsi_ipi_clk_mode_get(struct dpu_panel_info *pinfo);

/**
 * dsi_clk_div_cal - calculate clk div
 *
 * @rate: input rate
 * @out_div: out div
 * @out_rate: out rate
 * @return 0 on success, else on failure
 */
int dsi_clk_div_cal(unsigned long rate, u32 *out_div, u64 *out_rate);

void display_to_dsi(struct connector *connector, struct dsi_ctrl_cfg *ctrl);
void mipi_dsi_init(struct platform_device *pdev, struct connector *connector, struct dsi_ctrl_cfg *ctrl_cfg);

#endif
