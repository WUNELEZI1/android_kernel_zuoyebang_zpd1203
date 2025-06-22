// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/wait.h>

#include <soc/xring/ipc/xr_rproc.h>

#include "xr_mbox.h"

#define xr_rproc_pr_err(fmt, args...)	pr_err("[xr_ipc]" fmt "\n", ##args)
#define xr_rproc_pr_dbg(fmt, args...)	pr_debug("[xr_ipc]" fmt "\n", ##args)

#define IPC_ALIGN4_MASK			(0x03ul)

static int check_send_info(unsigned int *msg, unsigned int len)
{
	/* There is allow to send a blank packet */
	if (len > IPC_MBOX_B2B_DATA_SIZE || (msg == NULL && len > 0)) {
		xr_rproc_pr_err("send message is invalid\n");
		return -EINVAL;
	}

	if (((uintptr_t)msg & IPC_ALIGN4_MASK) != 0) {
		xr_rproc_pr_err("please ensure that the seNding data is 4-byte aligned\n");
		return -EINVAL;
	}

	return 0;
}

int xr_rproc_send_async(unsigned int ipc_id, unsigned int *msg, unsigned int len,
			ack_callback_t cb, void *arg)
{
	int ret = 0;

	ret = check_send_info(msg, len);
	if (ret) {
		xr_rproc_pr_err("%s: invalid notifier block", __func__);
		return ret;
	}

	ret = xr_ipc_send_async(ipc_id, msg, len, cb, arg);
	if (ret)
		xr_rproc_pr_err("ipc-%d async send failed, errno: %d", ipc_id, ret);

	return ret;
}
EXPORT_SYMBOL(xr_rproc_send_async);

int xr_rproc_async_fifo_reset(unsigned int ipc_id)
{
	xr_ipc_reset_async_cnt(ipc_id);

	return xr_ipc_async_fifo_reset(ipc_id);
}
EXPORT_SYMBOL(xr_rproc_async_fifo_reset);

int xr_rproc_send_sync(unsigned int ipc_id, unsigned int *msg, unsigned int len)
{
	int ret = 0;

	ret = check_send_info(msg, len);
	if (ret) {
		xr_rproc_pr_err("%s: invalid notifier block", __func__);
		return ret;
	}

	ret = xr_ipc_send_sync(ipc_id, msg, len);
	if (ret)
		xr_rproc_pr_err("fail to sync send");

	return ret;
}
EXPORT_SYMBOL(xr_rproc_send_sync);

int xr_rproc_send_resp(unsigned int ipc_id, unsigned int *msg, unsigned int len)
{
	int ret = 0;

	ret = check_send_info(msg, len);
	if (ret) {
		xr_rproc_pr_err("%s: invalid notifier block", __func__);
		return ret;
	}

	ret = xr_ipc_send_resp(ipc_id, msg, len);
	if (ret)
		xr_rproc_pr_err("fail to sync send");

	return ret;
}
EXPORT_SYMBOL(xr_rproc_send_resp);

int xr_rproc_send_recv(unsigned int ipc_id, unsigned int *msg, unsigned int len,
			unsigned int *rx_buffer, unsigned int rx_len)
{
	int ret = 0;

	ret = check_send_info(msg, len);
	if (ret) {
		xr_rproc_pr_err("%s: invalid notifier block", __func__);
		return ret;
	}

	if (!rx_buffer) {
		xr_rproc_pr_err("%s: invalid notifier block", __func__);
		return -EINVAL;
	}

	if (rx_len > MBOX_CHAN_DATA_SIZE) {
		xr_rproc_pr_err("msg length exceeds");
		return -EINVAL;
	}

	ret = xr_ipc_send_recv(ipc_id, msg, len, rx_buffer, rx_len);
	if (ret)
		xr_rproc_pr_err("fail to sync send");

	return ret;
}
EXPORT_SYMBOL(xr_rproc_send_recv);

int xr_rproc_recv_register(unsigned int ipc_id, struct notifier_block *nb)
{
	if (!nb) {
		xr_rproc_pr_err("invalid notifier block");
		return -EINVAL;
	}

	return xr_ipc_recv_register(ipc_id, nb);
}
EXPORT_SYMBOL(xr_rproc_recv_register);

int xr_rproc_recv_unregister(unsigned int ipc_id, struct notifier_block *nb)
{
	if (!nb) {
		xr_rproc_pr_err("invalid notifier block");
		return -EINVAL;
	}

	return xr_ipc_recv_unregister(ipc_id, nb);
}
EXPORT_SYMBOL(xr_rproc_recv_unregister);

struct rproc_list *xr_rproc_list_get(void)
{
	return vc_id_get();
}
EXPORT_SYMBOL(xr_rproc_list_get);

int xr_rproc_list_free(struct rproc_list *list)
{
	if (!list) {
		xr_rproc_pr_err("invalid notifier block");
		return -EINVAL;
	}
	vc_id_free(list);

	return 0;
}
EXPORT_SYMBOL(xr_rproc_list_free);

int xr_rproc_fast_ack_recv_register(unsigned int ipc_id,
			fast_ack_callback_t cb, void *arg)
{
	if (!cb) {
		xr_rproc_pr_err("invalid callback block");
		return -EINVAL;
	}

	return xr_ipc_fast_ack_recv_register(ipc_id, cb, arg);
}
EXPORT_SYMBOL(xr_rproc_fast_ack_recv_register);

int xr_rproc_fast_ack_recv_unregister(unsigned int ipc_id)
{
	return xr_ipc_fast_ack_recv_unregister(ipc_id);
}
EXPORT_SYMBOL(xr_rproc_fast_ack_recv_unregister);

void xr_rproc_dump_async_cnt(unsigned int ipc_id)
{
	xr_ipc_dump_async_cnt(ipc_id);
}
EXPORT_SYMBOL(xr_rproc_dump_async_cnt);
