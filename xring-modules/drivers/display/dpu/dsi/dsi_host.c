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

#include <drm/drm_mipi_dsi.h>

#include "dsi_display.h"
#include "dsi_host.h"
#include "dpu_idle_helper.h"
#include "dpu_hw_ulps.h"

static int dsi_host_attach(struct mipi_dsi_host *host,
			   struct mipi_dsi_device *dsi)
{
	return 0;
}

static int dsi_host_detach(struct mipi_dsi_host *host,
		struct mipi_dsi_device *dsi)
{
	return 0;
}

static void dsi_cmd_dump(struct dsi_cmd_desc *cmds)
{
	bool is_read_pkt;
	u8 *buf;
	size_t buf_len;
	int i;

	if (!cmds) {
		DSI_ERROR("invalid parameter, %pK\n", cmds);
		return;
	}

	is_read_pkt = cmds->msg.rx_len > 0 ? true : false;
	DSI_INFO("%s, port_index:%d, tx_len: %zu\n",
			is_read_pkt > 0 ? "read" : "send",
			cmds->port_index, cmds->msg.tx_len);

	buf = (u8 *)cmds->msg.tx_buf;
	buf_len = cmds->msg.tx_len;
	DSI_INFO("tx_buf:");
	for (i = 0; i < buf_len; i++)
		DSI_INFO("0x%x ", buf[i]);
}

int dsi_host_cmd_transfer(struct mipi_dsi_host *host,
		struct dsi_cmd_desc *cmd_desc, u8 transfer_type,
		u32 transfer_flag)
{
	bool is_dual_port, is_broadcast_mode;
	struct dsi_display *display;
	struct dsi_ctrl *ctrl;
	int master_ctrl_index;
	int i, ctrl_index;
	u32 wait_timeout;
	int ret = 0;
	bool is_read_pkt, is_already_dumpd = false;
	static bool need_clear_mipi_cmd_state;

	if (unlikely(!host || !cmd_desc || transfer_type > USE_CPU)) {
		DSI_ERROR("invalid parameters, %pK, %pK, %u\n", host, cmd_desc, transfer_type);
		return -EINVAL;
	}

	is_broadcast_mode = false;
	display = container_of(host, struct dsi_display, host);

	if (unlikely(display->panel->panel_info.is_fake_panel)) {
		DSI_INFO("bypass the send cmd for fake panel\n");
		return 0;
	}

	is_read_pkt = dsi_cmd_type_is_read(cmd_desc->msg.type);
	display_frame_timeout_get(display, &wait_timeout);
	is_dual_port = display->display_info.is_dual_port;
	if (is_dual_port && !(cmd_desc->msg.flags & MIPI_DSI_MSG_UNICAST_COMMAND))
		is_broadcast_mode = true;
	else if (is_dual_port && cmd_desc->port_index != DSI_PORT_MAX)
		master_ctrl_index = cmd_desc->port_index;
	else
		master_ctrl_index = display->port[DSI_PORT_0].enabled ?
				DSI_PORT_0 : DSI_PORT_1;

	if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_EXIT_ULPS)) {
		dpu_idle_ulps_exit();
		dpu_hw_ulps_ctrl_dyn(false);
		dsi_display_cmd_flush(display);
	}

	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ctrl_index = dsi_ctrl_hw_blk_id_get(ctrl);
		if (!is_broadcast_mode && (master_ctrl_index != ctrl_index))
			continue;

		/* wait all dsi ctrl cri is not busy */
		if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_WAIT_CRI_AVAILABLE)) {
			if (dsi_ctrl_cri_is_busy(ctrl, wait_timeout)) {
				DSI_WARN("check dsi-%d cri avail timeout!, timeout %d\n",
						dsi_ctrl_hw_blk_id_get(ctrl), wait_timeout);
				dsi_cmd_dump(cmd_desc);
				is_already_dumpd = true;
			}
			DPU_DEBUG("wait cri available\n");
		}
	}

	/* get cmd state, make sure that the consistency of cmd in dual port */
	if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_GET_MIPI_STATE)) {
		ret = dsi_display_mipi_cmd_state_get(display);
		if (ret) {
			DSI_WARN("failed to get mipi cmd state\n");
			if (!is_already_dumpd) {
				dsi_cmd_dump(cmd_desc);
				is_already_dumpd = true;
			}
			ret = 0;
			need_clear_mipi_cmd_state = false;
		} else {
			need_clear_mipi_cmd_state = true;
		}
		DPU_DEBUG("get mipi status\n");
	}

	if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_CRI_HOLD)) {
		display_for_each_active_port(i, display) {
			ctrl = display->port[i].ctrl;
			ctrl_index = dsi_ctrl_hw_blk_id_get(ctrl);
			if (!is_broadcast_mode && (master_ctrl_index != ctrl_index))
				continue;

			dsi_ctrl_cri_tx_ctrl(ctrl, CRI_MODE_PKT_STACK, 1);
		}
	}

	/* Send pre tx pkt for this read ops */
	if (is_read_pkt) {
		display_for_each_active_port(i, display) {
			ctrl = display->port[i].ctrl;
			ctrl_index = dsi_ctrl_hw_blk_id_get(ctrl);
			if (!is_broadcast_mode && (master_ctrl_index != ctrl_index))
				continue;

			ret = dsi_ctrl_cmd_read_pre_tx(ctrl, cmd_desc);
			if (ret) {
				DSI_ERROR("failed to send pre tx cmd in dsi-%d\n",
						dsi_ctrl_hw_blk_id_get(ctrl));
				if (!is_already_dumpd) {
					dsi_cmd_dump(cmd_desc);
					is_already_dumpd = true;
				}
			}
		}

	}

	/**
	 * In the hw_dvfs scenario, it is necessary to force refresh one frame
	 * to exit the ULPS state. However, for the read operation,
	 * the ULPS state needs to be exited before sending the read ops.
	 * Therefore, the MIPI State needs to be released in advance
	 * so that the dsip_commit thread can properly obtain the MIPI State
	 * for successfuly exit ULPS.
	 */
	if (transfer_flag & DSI_CMD_FLAG_EARLY_CLEAR_MIPI_STATE) {
		if (!need_clear_mipi_cmd_state) {
			DSI_WARN("skip to early clear mipi cmd state\n");
		} else {
			ret = dsi_display_mipi_cmd_state_clear(display);
			if (ret) {
				DSI_WARN("failed to early clear mipi cmd state");
				if (!is_already_dumpd) {
					dsi_cmd_dump(cmd_desc);
					is_already_dumpd = true;
				}
				ret = 0;
			}
			need_clear_mipi_cmd_state = false;
			DPU_DEBUG("early clear mipi status\n");
		}
	}

	display_for_each_active_port(i, display) {
		ctrl = display->port[i].ctrl;
		ctrl_index = dsi_ctrl_hw_blk_id_get(ctrl);
		if (!is_broadcast_mode && (master_ctrl_index != ctrl_index))
			continue;

		ret = dsi_ctrl_cmd_transfer(ctrl, cmd_desc, transfer_type, wait_timeout);
		if (unlikely(ret)) {
			DSI_ERROR("failed to transfer cmd desc in dsi-%d\n",
					dsi_ctrl_hw_blk_id_get(ctrl));
			if (!is_already_dumpd) {
				dsi_cmd_dump(cmd_desc);
				is_already_dumpd = true;
			}
			break;
		}
	}

	if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_CRI_UNHOLD)) {
		display_for_each_active_port(i, display) {
			ctrl = display->port[i].ctrl;
			ctrl_index = dsi_ctrl_hw_blk_id_get(ctrl);
			if (!is_broadcast_mode && (master_ctrl_index != ctrl_index))
				continue;

			dsi_ctrl_cri_tx_ctrl(ctrl, CRI_MODE_PKT_STACK, 0);
		}
	}

	if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_CLEAR_MIPI_STATE)) {
		if (!need_clear_mipi_cmd_state) {
			DSI_WARN("skip to clear mipi cmd state\n");
		} else {
			ret = dsi_display_mipi_cmd_state_clear(display);
			if (ret) {
				DSI_WARN("failed to clear mipi cmd state");
				if (!is_already_dumpd) {
					dsi_cmd_dump(cmd_desc);
					is_already_dumpd = true;
				}
				ret = 0;
			}
			need_clear_mipi_cmd_state = false;
			DPU_DEBUG("clear mipi status\n");
		}
	}

	if (transfer_type == USE_CPU && (transfer_flag & DSI_CMD_FLAG_ENTER_ULPS)) {
		dpu_idle_ulps_enter();
		dpu_hw_ulps_ctrl_dyn(true);
	}

	return ret;
}

static ssize_t dsi_host_transfer_msg(struct mipi_dsi_host *host,
		const struct mipi_dsi_msg *msg)
{
	struct dsi_cmd_desc cmd_desc;
	int ret = 0;
	u32 transfer_flag = 0;

	if (!host || !msg) {
		DSI_ERROR("get invalid params\n");
		return 0;
	}

	memcpy(&cmd_desc.msg, msg, sizeof(*msg));
	cmd_desc.post_wait_us = 0;

	transfer_flag = dsi_basic_cmd_flag_get(&cmd_desc, USE_CPU, 0, 1);
	ret = dsi_host_cmd_transfer(host, &cmd_desc, USE_CPU, transfer_flag);
	if (ret)
		DSI_ERROR("failed to transfer cmd, ret = %d\n", ret);

	return ret;
}

static struct mipi_dsi_host_ops dsi_host_ops = {
	.attach = dsi_host_attach,
	.detach = dsi_host_detach,
	.transfer = dsi_host_transfer_msg,
};

int dsi_host_init(struct mipi_dsi_host *host, struct device *dev)
{
	host->dev = dev;
	host->ops = &dsi_host_ops;

	return mipi_dsi_host_register(host);
}

void dsi_host_deinit(struct mipi_dsi_host *host)
{

	mipi_dsi_host_unregister(host);

	host->dev = NULL;
	host->ops = NULL;
}

