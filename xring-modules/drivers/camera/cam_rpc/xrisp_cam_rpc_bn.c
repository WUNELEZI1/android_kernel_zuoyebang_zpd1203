// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "xrisp_rpc", __func__, __LINE__

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/bitops.h>

#include "xrisp_cam_rpc_service.h"
#include "xrisp_cam_rpc_bn.h"
#include "xrisp_log.h"

#define FUNC_ISP_POWER_DOWN_HASH  0
#define FUNC_ISP_POWER_UP_HASH    1

int xrisp_cam_rpc_run_func(u8 *data, size_t size)
{
	struct rpc_msg_head *recv_msg = (struct rpc_msg_head *)data;
	u8 *param = (u8 *)(recv_msg + 1);

	XRISP_PR_INFO("entry %s, rev msg msg_id %d isack %d fun_hash %d head_index %u", __func__,
			 recv_msg->msg_id, recv_msg->ack, recv_msg->func_hash, recv_msg->head_index);

	switch (recv_msg->func_hash) {
	case FUNC_ISP_POWER_UP_HASH: {
		struct xrisp_power_up *powerup_set = (struct xrisp_power_up *)param;
		uint8_t ret_buffer[IPCMAXLEN]      = { 0 };
		struct  rpc_msg_head *ret_msg      = (struct rpc_msg_head *)ret_buffer;
		unsigned int    buffer_length      = sizeof(struct rpc_msg_head);

		int rpc_ret = xrisp_pipe_power_up(powerup_set);

		memcpy(ret_msg, recv_msg, sizeof(struct rpc_msg_head));

		if (rpc_ret != 0)
			ret_msg->ack = RPC_FAIL;
		else
			ret_msg->ack = RPC_SUCCESS;

		xrisp_cam_rpc_send_ack((void *)ret_buffer, buffer_length);

		XRISP_PR_INFO("isp pipeline power up status %d", ret_msg->ack);
		break;
	}
	case FUNC_ISP_POWER_DOWN_HASH: {
		struct xrisp_power_down *powerup_set = (struct xrisp_power_down *)param;
		uint8_t ret_buffer[IPCMAXLEN]        = { 0 };
		struct  rpc_msg_head *ret_msg        = (struct rpc_msg_head *)ret_buffer;
		unsigned int    buffer_length        = sizeof(struct rpc_msg_head);

		int rpc_ret = xrisp_pipe_power_down(powerup_set);

		memcpy(ret_msg, recv_msg, sizeof(struct rpc_msg_head));

		if (rpc_ret != 0)
			ret_msg->ack = RPC_FAIL;
		else
			ret_msg->ack = RPC_SUCCESS;

		xrisp_cam_rpc_send_ack((void *)ret_buffer, buffer_length);

		XRISP_PR_INFO("isp pipeline power down status %d", ret_msg->ack);
		break;
	}
	default:
		XRISP_PR_ERROR("rpc service error id %d", recv_msg->msg_id);
		break;
	}

	return 0;
}
