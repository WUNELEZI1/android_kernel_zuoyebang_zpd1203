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

#include "dpu_conn_mgr.h"
#include "mipi_dsi_host.h"
#include "dsi_hw_ctrl_ops.h"
#include "dsi_hw_ctrl.h"
/* MIPI DSI Processor-to-Peripheral transaction types */

static inline bool mipi_dsi_packet_format_is_long(uint8_t type)
{
	switch (type) {
	case MIPI_DSI_NULL_PACKET:
	case MIPI_DSI_BLANKING_PACKET:
	case MIPI_DSI_GENERIC_LONG_WRITE:
	case MIPI_DSI_DCS_LONG_WRITE:
	case MIPI_DSI_PICTURE_PARAMETER_SET:
	case MIPI_DSI_COMPRESSED_PIXEL_STREAM:
	case MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20:
	case MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24:
	case MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16:
	case MIPI_DSI_PACKED_PIXEL_STREAM_30:
	case MIPI_DSI_PACKED_PIXEL_STREAM_36:
	case MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12:
	case MIPI_DSI_PACKED_PIXEL_STREAM_16:
	case MIPI_DSI_PACKED_PIXEL_STREAM_18:
	case MIPI_DSI_PIXEL_STREAM_3BYTE_18:
	case MIPI_DSI_PACKED_PIXEL_STREAM_24:
		return true;
	default:
		break;
	}
	return false;
}

static inline bool mipi_dsi_packet_format_is_short(uint8_t type)
{
	switch (type) {
	case MIPI_DSI_V_SYNC_START:
	case MIPI_DSI_V_SYNC_END:
	case MIPI_DSI_H_SYNC_START:
	case MIPI_DSI_H_SYNC_END:
	case MIPI_DSI_COMPRESSION_MODE:
	case MIPI_DSI_END_OF_TRANSMISSION:
	case MIPI_DSI_COLOR_MODE_OFF:
	case MIPI_DSI_COLOR_MODE_ON:
	case MIPI_DSI_SHUTDOWN_PERIPHERAL:
	case MIPI_DSI_TURN_ON_PERIPHERAL:
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM:
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_DCS_READ:
	case MIPI_DSI_EXECUTE_QUEUE:
	case MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE:
		return true;
	default:
		break;
	}
	return false;
}

static int32_t mipi_dsi_create_packet(struct mipi_dsi_packet *packet, const struct mipi_dsi_msg *msg)
{
	const uint8_t *tx = msg->tx_buf;

	if (!mipi_dsi_packet_format_is_short(msg->type) &&
		!mipi_dsi_packet_format_is_long(msg->type))
		return -1;
	if (msg->channel > 3)
		return -1;

	packet->header[0] = ((msg->channel & 0x3) << 6) | (msg->type & 0x3f);

	if (mipi_dsi_packet_format_is_long(msg->type)) {
		packet->header[1] = (msg->tx_len >> 0) & 0xff;
		packet->header[2] = (msg->tx_len >> 8) & 0xff;
		packet->payload_length = msg->tx_len;
		packet->payload = msg->tx_buf;
	} else {
		packet->header[1] = (msg->tx_len > 0) ? tx[0] : 0;
		packet->header[2] = (msg->tx_len > 1) ? tx[1] : 0;
	}
	packet->size = sizeof(packet->header) + packet->payload_length;
	return 0;
}

int32_t mipi_dsi_msg_sent(struct connector *connector, struct mipi_dsi_msg *msg)
{
	struct mipi_dsi_packet packet;
	struct cri_tx_hdr hdr;
	uint32_t pld_data_bytes;
	const uint8_t *tx_buf;
	uint32_t res, len;
	uint32_t word;

	pld_data_bytes = sizeof(uint32_t);
	dpu_mem_set(&packet,0,sizeof(packet));

	if (!msg->tx_buf && msg->tx_len != 0) {
		dpu_pr_debug("Null parameters to send!\n");
		return -1;
	}
	res = mipi_dsi_create_packet(&packet, msg);
	if (res) {
		dpu_pr_debug("failed to create packet!: %d\n", res);
		return res;
	}

	dpu_udelay(200);

	tx_buf = packet.payload;
	len = packet.payload_length;
	if (msg->tx_len > CRI_PLD_FIFO_MAX) {
		dpu_pr_debug("Params length too large!\n");
		return -1;
	}

	/* Step 2: Write CRI TX Payload */
	if (mipi_dsi_packet_format_is_long(msg->type)) {
		while (len) {
			if (len < pld_data_bytes) {
				word = 0;
				dpu_mem_cpy(&word, tx_buf, len);
				len = 0;
			} else {
				dpu_mem_cpy(&word, tx_buf, pld_data_bytes);
				tx_buf += pld_data_bytes;
				len -= pld_data_bytes;
			}
		}
	}

	/* Step 3: Write CRI TX Header */
	hdr.virtual_channel = msg->channel;
	hdr.data_type = msg->type;
	hdr.wc_msb = packet.header[2];
	hdr.wc_lsb = packet.header[1];
	hdr.cmd_hdr_rd = CRI_WRITE_CMD_HDR;
	if (mipi_dsi_packet_format_is_long(msg->type))
		hdr.cmd_hdr_long = CRI_SEND_LONG_PKT;
	else if (mipi_dsi_packet_format_is_short(msg->type))
		hdr.cmd_hdr_long = CRI_SEND_SHORT_PKT;
	if (msg->flags)
		hdr.cmd_tx_mode = CRI_SEND_BY_HS;
	else
		hdr.cmd_tx_mode = CRI_SEND_BY_LP;

	return 0;
}
