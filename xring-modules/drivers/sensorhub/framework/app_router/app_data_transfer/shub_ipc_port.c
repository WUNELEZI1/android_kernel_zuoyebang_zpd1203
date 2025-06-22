// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "inc/shub_port.h"
#include "inc/shub_protocol.h"
#include "soc/xring/sensorhub/ipc/shub_ipc_drv.h"

#define GET_VC_ID(id) ((id) >> 16)
#define GET_TAG(id) ((id) & 0x00ff)
#define IPC_ALIGN (4)

static LIST_HEAD(channels);

struct recv_warpper {
	rsp_cb cb;
	void *arg;
	uint8_t vc_id;
	uint16_t tag;
	struct list_head list;
};

static int ipc_warpper(struct ipc_pkt *pkt, u16 tag, u8 cmd, void *arg)
{
	struct recv_warpper *recv = (struct recv_warpper *)arg;
	struct ipc_shm *shm_pkt;
	uint8_t *data = NULL;
	uint16_t data_len = 0;

	if (!recv->cb)
		return 0;
	if (pkt->header.control.type == IPC_SHORT_PKT ||
	    pkt->header.control.type == IPC_LONG_PKT) {
		data = pkt->data;
		data_len = pkt->header.data_len;
	} else if (pkt->header.control.type == IPC_SHM_PKT) {
		shm_pkt = sh_ipc_get_shm((struct ipc_shm_pkt *)pkt);
		data = shm_pkt->data;
		data_len = shm_pkt->data_len;
	} else
		return 0;

	pr_debug("ipc recv [l:%u][pading:%u]\n", data_len, cmd);
	recv->cb(&data[cmd], data_len - cmd, recv->arg);
	return 0;
}

static int ipc_lookup(const int id)
{
	return sh_ipc_send_msg_sync(GET_VC_ID(id), GET_TAG(id), 0, NULL, 0);
}

static int ipc_open(const int id, rsp_cb cb, void *arg)
{
	struct ipc_notifier_info info;
	int err;
	struct recv_warpper *recv =
		kmalloc(sizeof(struct recv_warpper), GFP_KERNEL);
	if (!recv)
		return -ENOMEM;
	recv->arg = arg;
	recv->cb = cb;
	recv->vc_id = GET_VC_ID(id);
	recv->tag = GET_TAG(id);
	info.callback = ipc_warpper;
	info.arg = recv;
	err = sh_register_recv_notifier(recv->vc_id, recv->tag, &info);
	pr_debug("ipc open:%d %d %d [recv:%pK]\n", recv->vc_id, recv->tag, err,
		 recv);
	if (err)
		kfree(recv);
	else
		list_add(&recv->list, &channels);
	return err;
}

static int ipc_close(const int id, void **arg)
{
	struct ipc_notifier_info info;
	struct recv_warpper *pos;
	struct list_head *cur, *bak;
	int err;

	info.callback = ipc_warpper;
	err = sh_unregister_recv_notifier(GET_VC_ID(id), GET_TAG(id), &info);
	pr_info("ipc close:%d %d err:%d\n", GET_VC_ID(id), GET_TAG(id), err);
	if (err)
		return err;

	list_for_each_safe(cur, bak, &channels) {
		pos = list_entry(cur, struct recv_warpper, list);
		if (pos->vc_id == GET_VC_ID(id) && pos->tag == GET_TAG(id)) {
			list_del(cur);
			if (arg)
				*arg = pos->arg;
			pr_info("ipc close:[warpper:%pK]\n", pos);
			kfree(pos);
			return 0;
		}
	}
	return 0;
}

static int ipc_send(const int id, void *msg, uint16_t msg_len)
{
	uint64_t msg_align = ALIGN_DOWN((uint64_t)msg, IPC_ALIGN);
	uint8_t padding = (uint64_t)msg - msg_align;

	pr_debug("ipc send [VC_ID:%d][TAG:%d] [align:%u][len:%u]\n",
		 GET_VC_ID(id), GET_TAG(id), padding, msg_len);
	return sh_ipc_send_msg_sync(GET_VC_ID(id), GET_TAG(id), padding,
				    ((uint8_t *)msg - padding),
				    msg_len + padding);
}

static struct port_ops ipc_port = {
	.lookup = ipc_lookup,
	.open = ipc_open,
	.close = ipc_close,
	.send = ipc_send,
	.align = IPC_ALIGN,
	.max_len = (0xffff >> 2),
};

struct port_ops *get_ipc_port(void)
{
	return &ipc_port;
}
