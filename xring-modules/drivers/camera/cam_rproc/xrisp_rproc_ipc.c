// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rproc_ipc", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rproc_ipc", __func__, __LINE__

#include <linux/slab.h>
#include <soc/xring/ipc/xr_rproc.h>
#include "xrisp_rproc.h"
#include "xrisp_rproc_api.h"
#include "xrisp_log.h"

static struct rproc_mbox_msg_cb rproc_msg_cb[XRISP_RPROC_MSG_MAX];

void xrisp_rproc_msg_register(enum xrisp_rproc_msg_type msg_type,
			     callback_func_t callback, void *priv)
{
	if (!callback || !priv)
		return;

	rproc_msg_cb[msg_type].callback = callback;
	rproc_msg_cb[msg_type].priv = priv;
}

void xrisp_rproc_msg_unregister(enum xrisp_rproc_msg_type msg_type)
{

	rproc_msg_cb[msg_type].callback = NULL;
	rproc_msg_cb[msg_type].priv = NULL;
}

static void xrisp_rproc_mbox_rxwork(struct work_struct *work)
{
	struct rproc_mbox_work *rproc_work =
		container_of(work, struct rproc_mbox_work, work);
	if (rproc_work->callback)
		rproc_work->callback(rproc_work->priv, rproc_work->data);

	kfree(rproc_work);
}

static void xrisp_rproc_mbox_recv_callback(void *rx_buff, void *arg)
{
	struct rproc_mbox_work *rproc_work;
	enum xrisp_rproc_msg_type msg_type;

	msg_type = *(uint32_t *)rx_buff;

	// XRISP_PR_DEBUG("rproc mbox recv %d", msg_type);

	if (!g_xrproc)
		return;

	if (msg_type >= XRISP_RPROC_MSG_MAX ||
		msg_type <= XRISP_RPROC_MSG_INVALED) {
		XRISP_PR_WARN("unkown rproc msg %d", msg_type);
		return;
	}

	if (msg_type == XRISP_RPROC_MSG_KICK) {
		ktime_t now = ktime_get();

		if (queue_work(g_xrproc->workqueue, &g_xrproc->kick_work))
			g_xrproc->kick_time = now;
		return;
	}

	if (!rproc_msg_cb[msg_type].callback || !rproc_msg_cb[msg_type].priv)
		XRISP_PR_WARN("no work to do for %d", msg_type);

	rproc_work = kzalloc(sizeof(struct rproc_mbox_work), GFP_ATOMIC);
	memcpy(rproc_work->data, rx_buff, MBOX_MAX_DATA_REG * sizeof(uint32_t));
	rproc_work->priv = rproc_msg_cb[msg_type].priv;
	rproc_work->callback = rproc_msg_cb[msg_type].callback;
	INIT_WORK(&rproc_work->work, xrisp_rproc_mbox_rxwork);

	queue_work(g_xrproc->workqueue, &rproc_work->work);
}

int xrisp_rproc_mbox_init(void)
{
	int ret = 0;

	ret = xr_rproc_fast_ack_recv_register(XRISP_RPROC_IPC_ID,
					      xrisp_rproc_mbox_recv_callback, NULL);
	if (ret)
		XRISP_PR_ERROR("rproc mbox init fail");
	memset(rproc_msg_cb, 0, sizeof(rproc_msg_cb));
	XRISP_PR_INFO("rproc mbox init success");
	return ret;
}

void xrisp_rproc_mbox_exit(void)
{
	int ret = 0;

	memset(rproc_msg_cb, 0, sizeof(rproc_msg_cb));
	ret = xr_rproc_fast_ack_recv_unregister(XRISP_RPROC_IPC_ID);
	if (ret)
		XRISP_PR_ERROR("rproc mbox exit fail");
}

int xrisp_rproc_mbox_send_sync(unsigned int *tx_buffer, unsigned int len)
{
	return xr_rproc_send_sync(XRISP_RPROC_IPC_ID, tx_buffer, len);
}
