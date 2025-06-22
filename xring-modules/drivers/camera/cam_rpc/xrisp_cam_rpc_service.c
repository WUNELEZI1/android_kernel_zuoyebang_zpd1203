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

#include "../cam_ctrl/xrisp_camctrl_ipcm.h"
#include "../cam_ctrl/xrisp_cam_ctrl.h"

#include "xrisp_cam_rpc_service.h"
#include "xrisp_log.h"

#define RPC_SENT_MSG_ID(id) ((id) % 0xFFFFFFFF)

#define RPC_MSG_HEAD_NUM (512)
#define RPC_BIT_MAP_LEN  (RPC_MSG_HEAD_NUM / (sizeof(unsigned long) * 8))

struct rpc_service_head_ctrl {
	struct mutex lock;
	struct rpc_head rpc_head_list[RPC_MSG_HEAD_NUM];
	unsigned long bit_map[RPC_BIT_MAP_LEN];
};

static struct rpc_service_head_ctrl g_rpc_head_ctrl;
static uint64_t g_rpc_msg_id;

static int rpc_get_head_index(void)
{
	int ret;

	mutex_lock(&g_rpc_head_ctrl.lock);

	ret = find_first_zero_bit(g_rpc_head_ctrl.bit_map, RPC_MSG_HEAD_NUM);

	if (ret < 0 || ret >= RPC_MSG_HEAD_NUM)
		XRISP_PR_ERROR("no free rpc head, idx = %d", ret);

	mutex_unlock(&g_rpc_head_ctrl.lock);

	return ret;
}

static struct rpc_head *rpc_get_head(int32_t index)
{
	if (index >= RPC_MSG_HEAD_NUM)
		return NULL;

	return g_rpc_head_ctrl.rpc_head_list + index;
}

static void rpc_release_head(int32_t index)
{
	if (index >= RPC_MSG_HEAD_NUM) {
		XRISP_PR_ERROR("release invalid head index %d", index);
		return;
	}
	mutex_lock(&g_rpc_head_ctrl.lock);

	clear_bit(index, g_rpc_head_ctrl.bit_map);
	memset(g_rpc_head_ctrl.rpc_head_list + index, 0, sizeof(struct rpc_head));

	mutex_unlock(&g_rpc_head_ctrl.lock);
	XRISP_PR_INFO("release index %d", index);
}

int xrisp_cam_rpc_process(u8 *data, size_t size)
{
	struct rpc_msg_head *head = (struct rpc_msg_head *)data;

	XRISP_PR_INFO("rev msg: ack %d msg_id %u func_hash %d head_index %u", head->ack, head->msg_id, head->func_hash, head->head_index);

	switch (head->ack) {
	case RPC_SERVICE:
		xrisp_cam_rpc_run_func(data, size);
		break;
	case RPC_FAIL:
	case RPC_SUCCESS: {
		int32_t head_index = head->head_index;
		struct rpc_head *rpc_head = rpc_get_head(head_index);

		if (rpc_head == NULL) {
			XRISP_PR_ERROR("invalid head index %u", head_index);
			return 0;
		}

		if (rpc_head->result_len != 0 && head->ack == RPC_SUCCESS)
			memcpy(rpc_head->result_data, head + 1, rpc_head->result_len);

		// for synchronous
		if (rpc_head->sem != NULL) {
			*(rpc_head->rpc_ret) = head->ack;
			complete(rpc_head->sem);
		}
		// for asynchronous
		if (rpc_head->cb != NULL) {
			rpc_head->cb((void *)(head + 1), head->ack);
			rpc_release_head(head_index);
		}
		break;
	}
	default:
		XRISP_PR_ERROR("invalid ack %u", head->ack);
		break;
	}

	return 0;
}
EXPORT_SYMBOL(xrisp_cam_rpc_process);

int rpc_service_init(void)
{
	g_rpc_msg_id = 0;

	mutex_init(&g_rpc_head_ctrl.lock);

	bitmap_zero(g_rpc_head_ctrl.bit_map, RPC_MSG_HEAD_NUM);

	return 0;
}

int rpc_service_uninit(void)
{
	mutex_destroy(&g_rpc_head_ctrl.lock);

	bitmap_zero(g_rpc_head_ctrl.bit_map, RPC_MSG_HEAD_NUM);

	return 0;
}


int xrisp_cam_rpc_send_msg(struct rpc_head *rpc_head)
{
	int32_t index;
	int ret;
	struct rpc_msg_head *head = (struct rpc_msg_head *)rpc_head->msg_data;

	head->msg_id = RPC_SENT_MSG_ID(g_rpc_msg_id++);

	index = rpc_get_head_index();
	if (index == -1) {
		XRISP_PR_ERROR("no free rpc head buffer msg_id %u func_hash %u",
				head->msg_id, head->func_hash);
		return -EFAULT;
	}

	head->head_index = index;

	memcpy(g_rpc_head_ctrl.rpc_head_list + index, rpc_head, sizeof(struct rpc_head));

	ret = xrisp_cam_rpc_send_kernel_msg((void *)rpc_head->msg_data, rpc_head->msg_size);
	if (ret)
		rpc_release_head(index);

	return ret;
}

int xrisp_cam_rpc_send_ack(void *buffer, size_t size)
{
	return xrisp_cam_rpc_send_kernel_msg(buffer, size);
}
