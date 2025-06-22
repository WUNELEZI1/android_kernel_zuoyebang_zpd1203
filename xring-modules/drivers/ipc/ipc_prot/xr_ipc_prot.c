// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/genalloc.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/timekeeping.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/spinlock.h>

#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <soc/xring/securelib/securectype.h>
#include <soc/xring/securelib/securec.h>
#include <soc/xring/ipc/xr_ipc_prot.h>
#include <soc/xring/ipc/xr_rproc.h>
#include "../xr_ipc_main.h"

#define IPC_MSG_BUFEER_TYPE			(2)

#define IPC_SHM_ADDR_OFFSET			(0)
#define IPC_SHM_LEN				(1)
#define IPC_SHM_SEND_MSG			(2)
#define IPC_SHM_DATA_SIZE			(8)

enum send_mode {
	IPC_SEND_SYNC = 0,
	IPC_SEND_ASYNC,
	IPC_SEND_RECV,
	IPC_SEND_RESP,
};

enum rx_sm {
	STATE_NORMAL = 0,
	STATE_WAIT,
	STATE_ERR,
	STATE_END,
};

struct ap_ipc_pkt {
	struct ipc_pkt_header header;
	uint8_t data[IPC_SHORT_PKT_DATA_SIZE];
};

/* used to private data for tx mbox */
struct ipc_mbox_tx_priv {
	bool inited;
	u8 vc_id;
	u8 seqid;
	struct ap_ipc_pkt *tx_pkt;
	struct mutex lock;
};

struct long_pkt_data_node {
	u8 *data;
	struct long_pkt_data_node *next;
};

/* used to private data for rx mbox */
struct ipc_mbox_rx_priv {
	bool inited;
	u8 vc_id;
	u8 seqid;
	u8 subid;
	u8 state;	/* state of state machine, 0: normal, 1: waitting for ending pkt */
	u32 msg_len;	/* the length of receiving packet */
	struct mutex lock;
	spinlock_t tag_lock;
	spinlock_t info_lock;
	struct list_head tag_head;	/* used to link tags */
	struct notifier_block nb;	/* notifier block */
	struct long_pkt_data_node *long_pkt_head;
	struct long_pkt_data_node *long_pkt_tail;
	u8 *rx_pkt;	/* received pkt, this packet will be used by the business layer */
};

struct ipc_mbox_priv {
	struct device *dev;
	struct rproc_list *rp_list;
	u8 *rx_pkts;
	struct ap_ipc_pkt *tx_pkts;
	u8 priv_num;
	struct ipc_mbox_tx_priv *tx_priv;
	struct ipc_mbox_rx_priv *rx_priv;
};

struct tag_notifier_node {
	u16 ipc_tag;
	u8 vc_id;
	struct list_head node;
	struct atomic_notifier_head nh;
	struct ap_ipc_pkt *pkt;
	struct list_head info_head;	/* used to link callbacks */
};

struct pkt_send_info {
	struct ipc_pkt_header header;
	u8 vc_id;
	u32 pkt_size;
	size_t msg_size;
	void *msg_buff;
	void *rx_buff;
	size_t rx_len;
	int send_mode;
	ack_callback_t cb;
	void *arg;
};

static struct ipc_mbox_priv g_ipc_mbox_priv;

#define ipc_prot_err(fmt, args...)	pr_err("[xr_ipc_prot]" fmt "\n", ##args)

static void end_of_recv_pkt(struct ipc_mbox_rx_priv *rx_priv)
{
	rx_priv->state = STATE_NORMAL;
	rx_priv->seqid++;
	rx_priv->subid = 0;
	rx_priv->msg_len = 0;
}

static struct ipc_mbox_tx_priv *get_ipc_mbox_tx_priv(u8 vc_id)
{
	int i;

	for (i = 0; i < g_ipc_mbox_priv.priv_num; i++) {
		if (g_ipc_mbox_priv.tx_priv[i].vc_id == vc_id)
			return &g_ipc_mbox_priv.tx_priv[i];
	}

	return NULL;
}

static struct ipc_mbox_rx_priv *get_ipc_mbox_rx_priv(u8 vc_id)
{
	int i;

	for (i = 0; i < g_ipc_mbox_priv.priv_num; i++) {
		if (g_ipc_mbox_priv.rx_priv[i].vc_id == vc_id)
			return &g_ipc_mbox_priv.rx_priv[i];
	}

	return NULL;
}

static int recv_short_pkt(struct ipc_mbox_rx_priv *rx_priv,
			struct ap_ipc_pkt *pkt)
{
	struct device *dev = g_ipc_mbox_priv.dev;

	if (rx_priv->state == STATE_WAIT)
		dev_warn(dev, "Long pkt loss occurred");

	/* copy data form receiving short pkt */
	memcpy_s(rx_priv->rx_pkt, pkt->header.data_len + IPC_PKT_HEADER_SIZE,
		pkt, pkt->header.data_len + IPC_PKT_HEADER_SIZE);
	end_of_recv_pkt(rx_priv);

	return STATE_NORMAL;
}

static int recv_long_pkt(struct ipc_mbox_rx_priv *rx_priv,
			struct ap_ipc_pkt *pkt)
{
	struct device *dev = NULL;
	u8 pkt_subid = pkt->header.control.subid;
	int dest_offst = 0;
	int msg_size = 0;

	dev = g_ipc_mbox_priv.dev;

	msg_size = IPC_SHORT_PKT_DATA_SIZE;
	dest_offst = pkt_subid * IPC_SHORT_PKT_DATA_SIZE + IPC_PKT_HEADER_SIZE;

	memcpy_s(rx_priv->rx_pkt + dest_offst, msg_size, pkt->data, msg_size);

	rx_priv->msg_len = pkt->header.data_len;

	if (unlikely(pkt->header.control.end == IPC_PKT_END)) {
		memcpy_s(rx_priv->rx_pkt, IPC_PKT_HEADER_SIZE, &pkt->header, IPC_PKT_HEADER_SIZE);
		end_of_recv_pkt(rx_priv);

		return STATE_NORMAL;
	}

	rx_priv->state = STATE_WAIT;

	return STATE_WAIT;
}

static int recv_pkt_handler(struct ipc_mbox_rx_priv *rx_priv, struct ap_ipc_pkt *pkt)
{
	int ret;
	u8 pkt_type = pkt->header.control.type;

	if (pkt_type == IPC_SHORT_PKT)
		ret = recv_short_pkt(rx_priv, pkt);
	else
		ret = recv_long_pkt(rx_priv, pkt);

	return ret;
}

static struct tag_notifier_node *find_tag_node(struct ipc_mbox_rx_priv *rx_priv, u16 tag)
{
	struct tag_notifier_node *tag_node_pos = NULL;
	struct tag_notifier_node *tag_node_next = NULL;

	list_for_each_entry_safe(tag_node_pos, tag_node_next,
				&rx_priv->tag_head, node) {
		if (tag_node_pos->ipc_tag == tag)
			return tag_node_pos;
	}

	return NULL;
}

static struct tag_notifier_node *find_tag_node_spin(struct ipc_mbox_rx_priv *rx_priv, u16 tag)
{
	struct tag_notifier_node *tag_node_pos;
	unsigned long flags = 0;

	spin_lock_irqsave(&rx_priv->tag_lock, flags);
	tag_node_pos = find_tag_node(rx_priv, tag);
	spin_unlock_irqrestore(&rx_priv->tag_lock, flags);

	return tag_node_pos;
}

static int notifier_func(struct notifier_block *nb, unsigned long act, void *arg)
{
	struct notifier_info *info = container_of(nb, struct notifier_info, nb);
	struct ipc_pkt *pkt = (struct ipc_pkt *)arg;
	u16 pkt_tag = pkt->header.tag;

	return info->handler(pkt, pkt_tag, act, info->arg);
}

static void callback_tag_handler(struct ipc_mbox_rx_priv *rx_priv)
{
	struct ap_ipc_pkt *pkt = (struct ap_ipc_pkt *)rx_priv->rx_pkt;
	struct tag_notifier_node *tag_of_node = NULL;
	u16 pkt_tag = pkt->header.tag;
	u8 cmd = pkt->header.cmd;
	struct device *dev = NULL;

	dev = g_ipc_mbox_priv.dev;

	/* call callback of business registered */
	tag_of_node = find_tag_node_spin(rx_priv, pkt_tag);
	if (!tag_of_node) {
		dev_info(dev, "this tag has not been registered");
		return;
	}

	atomic_notifier_call_chain(&tag_of_node->nh, cmd, pkt);
}

static u8 get_packet_type(size_t msg_size)
{
	if (msg_size <= IPC_SHORT_PKT_DATA_SIZE)
		return IPC_SHORT_PKT;
	else if ((msg_size > IPC_SHORT_PKT_DATA_SIZE) &&
		(msg_size <= IPC_LONG_PKT_DATA_SIZE))
		return IPC_LONG_PKT;

	return IPC_PKT_TYPE_INVALID;
}

static int check_short_pkt(struct ipc_mbox_rx_priv *rx_priv, struct ap_ipc_pkt *pkt)
{
	uint8_t actu_pkt_type;
	uint16_t pkt_data_len;
	uint8_t pkt_end = pkt->header.control.end;
	uint8_t pkt_subid = pkt->header.control.subid;
	struct device *dev = NULL;

	dev = g_ipc_mbox_priv.dev;

	if (rx_priv->state == STATE_WAIT)
		dev_warn(dev,  "long pkt loss occurred\n");

	if (pkt_subid != 0 || pkt_end != IPC_PKT_END) {
		dev_err(dev, "short pkt subid is error");
		return -EINVAL;
	}

	/* check pkt type validity */
	pkt_data_len = pkt->header.data_len;
	actu_pkt_type = get_packet_type(pkt_data_len);
	if (actu_pkt_type != IPC_SHORT_PKT) {
		dev_err(dev, "invalid short pkt");
		return -EINVAL;
	}

	return 0;
}

static int check_shm_pkt(struct ipc_mbox_rx_priv *rx_priv, struct ap_ipc_pkt *pkt)
{
	uint8_t actu_pkt_type = pkt->header.control.type;
	uint8_t pkt_end = pkt->header.control.end;
	uint8_t pkt_subid = pkt->header.control.subid;
	struct device *dev = NULL;

	dev = g_ipc_mbox_priv.dev;

	if (rx_priv->state == STATE_WAIT)
		dev_warn(dev,  "long pkt loss occurred\n");

	if (pkt_subid != 0 || pkt_end != IPC_PKT_END) {
		dev_err(dev, "shm pkt subid is error");
		return -EINVAL;
	}

	if (actu_pkt_type != IPC_SHM_PKT) {
		dev_err(dev, "invalid shm pkt");
		return -EINVAL;
	}

	return 0;
}

static int check_long_pkt(struct ipc_mbox_rx_priv *rx_priv, struct ap_ipc_pkt *pkt)
{
	uint8_t pkt_end = pkt->header.control.end;
	uint16_t pkt_data_len = pkt->header.data_len;
	uint8_t pkt_subid = pkt->header.control.subid;
	struct device *dev = NULL;
	uint8_t actu_pkt_type;

	dev = g_ipc_mbox_priv.dev;

	/* if loss pkt, we will abandon this pkt. */
	if (pkt_subid != rx_priv->subid) {
		dev_err(dev, "long pkt loss occurred, pkt_subid: %d, rx_subid:%d\n",
			pkt_subid, rx_priv->subid);
		rx_priv->subid++;
		return -EINVAL;
	}
	rx_priv->subid++;

	/* check subid and data_len */
	if ((pkt_subid == 0 && pkt_end == IPC_PKT_END) ||
		(pkt_subid >= IPC_LONG_PKT_SPLIT_NUM) ||
		(pkt_data_len <= pkt_subid * IPC_SHORT_PKT_DATA_SIZE)) {
		dev_warn(dev, "check subid and data_len failed");
		return -EINVAL;
	}

	/* check pkt type validity */
	actu_pkt_type = get_packet_type(pkt_data_len);
	if (actu_pkt_type != IPC_LONG_PKT) {
		dev_err(dev, "invalid long pkt");
		return -EINVAL;
	}

	if (pkt_end == IPC_PKT_END) {
		/* check data_len and subid */
		if (pkt_data_len > (pkt_subid + 1) * IPC_SHORT_PKT_DATA_SIZE) {
			dev_err(dev, "long pkt data_len err");
			return -EINVAL;
		}

		return 0;
	}

	rx_priv->state = STATE_WAIT;

	return STATE_NORMAL;
}

static int check_recv_pkt(struct ipc_mbox_rx_priv *rx_priv, struct ap_ipc_pkt *pkt)
{
	uint8_t pkt_seqid = pkt->header.seqid;
	uint8_t pkt_type = pkt->header.control.type;
	struct device *dev = NULL;
	int ret;

	dev = g_ipc_mbox_priv.dev;

	/* check seqid to determine if packet loss is sent */
	if (pkt_seqid != rx_priv->seqid) {
		dev_warn(dev, "[%d] pkt loss occurred, pkt_seqid: %d, rx_seqid:%d\n",
			rx_priv->vc_id, pkt_seqid, rx_priv->seqid);
		rx_priv->seqid = pkt_seqid;
	}

	/* check current pkt */
	switch (pkt_type) {
	case IPC_SHORT_PKT:
		ret = check_short_pkt(rx_priv, pkt);
		break;

	case IPC_LONG_PKT:
		ret = check_long_pkt(rx_priv, pkt);
		break;

	case IPC_SHM_PKT:
		ret = check_shm_pkt(rx_priv, pkt);
		break;

	default:
		/* reset state of receiving state machine */
		rx_priv->state = STATE_NORMAL;
		dev_err(dev, "invalid pkt type");
		return -EINVAL;
	}

	return ret;
}

/* it will be registered to low_drv as fast_ack mode */
static void ipc_mbox_fast_ack_callback(void *data, void *arg)
{
	struct ipc_mbox_rx_priv *rx_priv = (struct ipc_mbox_rx_priv *)arg;
	struct ap_ipc_pkt *pkt = NULL;
	struct device *dev = NULL;
	uint32_t vc_id = rx_priv->vc_id;
	int ret;

	dev = g_ipc_mbox_priv.dev;
	if (IS_ERR_OR_NULL(data)) {
		dev_err(dev, "vc_id[%d] data is invalid", vc_id);
		return;
	}
	pkt = (struct ap_ipc_pkt *)data;

	ret = check_recv_pkt(rx_priv, pkt);
	if (ret) {
		dev_err(dev, "packet check failed");
		if (pkt->header.control.end == IPC_PKT_END)
			end_of_recv_pkt(rx_priv);

		return;
	}

	ret = recv_pkt_handler(rx_priv, pkt);
	if (ret == STATE_NORMAL)
		callback_tag_handler(rx_priv);
}

/* it will be registered to low_drv as normal mode */
static int ipc_mbox_recv_callback(struct notifier_block *nb,
				unsigned long action, void *data)
{
	struct ipc_mbox_rx_priv *rx_priv = NULL;
	struct ap_ipc_pkt *pkt = NULL;
	struct device *dev = NULL;
	int vc_id = action;
	int ret;

	dev = g_ipc_mbox_priv.dev;

	if (IS_ERR_OR_NULL(data)) {
		dev_err(dev, "vc_id[%d] data is invalid", vc_id);
		return -ENODEV;
	}
	pkt = (struct ap_ipc_pkt *)data;

	rx_priv = get_ipc_mbox_rx_priv(vc_id);
	if (IS_ERR_OR_NULL(rx_priv)) {
		dev_err(dev, "vc_id %d is invalid", vc_id);
		return -ENODEV;
	}

	ret = check_recv_pkt(rx_priv, pkt);
	if (ret) {
		dev_err(dev, "packet check failed");
		if (pkt->header.control.end == IPC_PKT_END)
			end_of_recv_pkt(rx_priv);

		return -EPERM;
	}

	ret = recv_pkt_handler(rx_priv, pkt);
	if (ret == STATE_NORMAL)
		callback_tag_handler(rx_priv);

	return ret;
}

static int register_mbox_recv_callback(struct ipc_mbox_rx_priv *rx_priv)
{
	struct notifier_block *nb = &rx_priv->nb;
	struct device *dev = NULL;
	u8 vc_id = rx_priv->vc_id;
	int ret;

	dev = g_ipc_mbox_priv.dev;

	if (xr_mbox_fastack_sta(vc_id) == IPC_FAST_ACK) {
		ret = xr_rproc_fast_ack_recv_register(vc_id,
			ipc_mbox_fast_ack_callback, (void *)rx_priv);
	} else {
		nb->notifier_call = ipc_mbox_recv_callback;
		nb->next = NULL;
		nb->priority = 0;

		/* register notifier block to lower driver of IPC */
		ret = xr_rproc_recv_register(vc_id, nb);
	}
	if (ret != 0) {
		dev_err(dev, "register receive callback failed");
		return -EINVAL;
	}

	return 0;
}

static void unregister_mbox_recv_callback(u8 vc_id, struct notifier_block *nb)
{
	struct device *dev = NULL;
	int ret;

	dev = g_ipc_mbox_priv.dev;

	/* unregister notifier block to lower driver of IPC */
	ret = xr_rproc_recv_unregister(vc_id, nb);
	if (ret != 0)
		dev_err(dev, "unregister receive callback failed");
}

static struct tag_notifier_node *register_tag_node(struct ipc_mbox_rx_priv *rx_priv,
				u16 ipc_tag)
{
	struct tag_notifier_node *tag_node = NULL;
	struct device *dev = NULL;
	unsigned long flags = 0;

	dev = g_ipc_mbox_priv.dev;

	tag_node = devm_kcalloc(dev, 1, sizeof(struct tag_notifier_node), GFP_KERNEL);
	if (tag_node == NULL)
		return NULL;

	tag_node->ipc_tag = ipc_tag;
	ATOMIC_INIT_NOTIFIER_HEAD(&tag_node->nh);
	INIT_LIST_HEAD(&tag_node->info_head);

	spin_lock_irqsave(&rx_priv->tag_lock, flags);
	list_add_tail(&tag_node->node, &rx_priv->tag_head);
	spin_unlock_irqrestore(&rx_priv->tag_lock, flags);

	return tag_node;
}

static void unregister_tag(struct ipc_mbox_rx_priv *rx_priv)
{
	struct tag_notifier_node *tag_node_pos = NULL;
	struct tag_notifier_node *tag_node_next = NULL;
	unsigned long flags = 0;

	list_for_each_entry_safe(tag_node_pos, tag_node_next, &rx_priv->tag_head, node) {
		spin_lock_irqsave(&rx_priv->tag_lock, flags);
		list_del(&tag_node_pos->node);
		spin_unlock_irqrestore(&rx_priv->tag_lock, flags);
	}
}

static struct notifier_info *find_info_node(struct tag_notifier_node *tag_node,
			notifier_handler_t nb)
{
	struct notifier_info *info_node_pos = NULL;
	struct notifier_info *info_node_next = NULL;

	list_for_each_entry_safe(info_node_pos, info_node_next,
				&tag_node->info_head, node) {
		if (info_node_pos->handler == nb)
			return info_node_pos;
	}

	return NULL;
}

static void pkt_send_info_init(struct pkt_send_info *send_info, u8 vc_id,
	u16 tag, void *msg_buff, size_t msg_size, u8 resp, u8 mode, u8 cmd)
{
	send_info->vc_id = vc_id;
	send_info->msg_size = msg_size;
	send_info->msg_buff = msg_buff;
	send_info->header.tag = tag;
	send_info->header.control.resp = resp;
	send_info->send_mode = mode;
	send_info->header.cmd = cmd;
	send_info->header.rsvd = 0;
	send_info->rx_buff = NULL;
	send_info->rx_len = 0;
}

static void fill_short_long_pkt_body(struct pkt_send_info *pkt_send_info,
					struct ap_ipc_pkt *pkt)
{
	pkt->header.data_len = pkt_send_info->msg_size;

	memcpy_s(pkt->data, pkt_send_info->pkt_size - IPC_PKT_HEADER_SIZE,
		pkt_send_info->msg_buff, pkt_send_info->pkt_size - IPC_PKT_HEADER_SIZE);
}

static void fill_shm_pkt_body(struct pkt_send_info *pkt_send_info,
					struct ap_ipc_pkt *pkt)
{
	pkt->header.data_len = pkt_send_info->msg_size;

	memcpy_s(pkt->data, IPC_SHM_DATA_SIZE,
		pkt_send_info->msg_buff, IPC_SHM_DATA_SIZE);
}

static int fill_packet_body(struct pkt_send_info *pkt_send_info, struct ap_ipc_pkt *pkt)
{
	u8 pkt_type = pkt_send_info->header.control.type;

	/* if short or long packet, fill size and data */
	if (pkt_type == IPC_SHORT_PKT || pkt_type == IPC_LONG_PKT) {
		fill_short_long_pkt_body(pkt_send_info, pkt);
		return 0;
	}

	if (pkt_type == IPC_SHM_PKT) {
		fill_shm_pkt_body(pkt_send_info, pkt);
		return 0;
	}

	return -EINVAL;
}

static int send_packet_by_ipc(struct pkt_send_info *pkt_send_info, struct ap_ipc_pkt *pkt)
{
	struct device *dev = NULL;
	int ret = 0;

	dev = g_ipc_mbox_priv.dev;

	/* send message using low driver */
	if (pkt_send_info->header.control.resp == IPC_SEND_NO_RESP) {
		switch (pkt_send_info->send_mode) {
		case IPC_SEND_SYNC:
			ret = xr_rproc_send_sync(pkt_send_info->vc_id,
				(unsigned int *)pkt, pkt_send_info->pkt_size);
			break;

		case IPC_SEND_ASYNC:
			ret = xr_rproc_send_async(pkt_send_info->vc_id,
				(unsigned int *)pkt, pkt_send_info->pkt_size,
				pkt_send_info->cb, pkt_send_info->arg);
			break;

		case IPC_SEND_RESP:
			ret = xr_rproc_send_resp(pkt_send_info->vc_id,
				(unsigned int *)pkt, pkt_send_info->pkt_size);
			break;

		default:
			ret = -EINVAL;
			dev_err(dev, "invalid send mode failed\n");
			break;
		}
	} else {
		ret = xr_rproc_send_recv(pkt_send_info->vc_id,
					(unsigned int *)pkt, pkt_send_info->pkt_size,
					pkt_send_info->rx_buff, pkt_send_info->rx_len);
	}

	return ret;
}

static int send_packet(struct ipc_mbox_tx_priv *tx_priv,
			struct pkt_send_info *pkt_send_info)
{
	struct ap_ipc_pkt *pkt = tx_priv->tx_pkt;
	struct device *dev = NULL;
	int ret;

	dev = g_ipc_mbox_priv.dev;
	pkt_send_info->header.seqid = tx_priv->seqid;
	pkt_send_info->pkt_size = IPC_PKT_HEADER_SIZE + pkt_send_info->msg_size;

	memcpy_s(&pkt->header, sizeof(struct ipc_pkt_header),
		&pkt_send_info->header, sizeof(struct ipc_pkt_header));

	ret = fill_packet_body(pkt_send_info, pkt);
	if (unlikely(ret != 0)) {
		dev_err(dev, "fill_packet_body failed, ret = %d\n", ret);
		return ret;
	}

	ret = send_packet_by_ipc(pkt_send_info, pkt);
	if (ret != 0)
		dev_err(dev, "send packet failed, ret = %d\n", ret);
	tx_priv->seqid++; /* increase seqid */

	memset(tx_priv->tx_pkt, 0, IPC_PKT_MAX_SIZE);

	return ret;
}

static int send_short_packet(struct ipc_mbox_tx_priv *tx_priv,
			struct pkt_send_info *pkt_send_info)
{
	int ret;

	pkt_send_info->header.control.subid = 0;
	pkt_send_info->header.control.end = IPC_PKT_END;
	pkt_send_info->header.data_len = pkt_send_info->msg_size;
	ret = send_packet(tx_priv, pkt_send_info);

	return ret;
}

static int assemble_packet(struct ipc_mbox_tx_priv *tx_priv,
			struct pkt_send_info *pkt_send_info, int id)
{
	struct ap_ipc_pkt *pkt;
	struct device *dev = NULL;
	int ret;

	dev = g_ipc_mbox_priv.dev;
	pkt = tx_priv->tx_pkt + id;
	pkt_send_info->header.seqid = tx_priv->seqid;

	memcpy_s(&pkt->header, sizeof(struct ipc_pkt_header),
		&pkt_send_info->header, sizeof(struct ipc_pkt_header));

	ret = fill_packet_body(pkt_send_info, pkt);
	if (unlikely(ret != 0)) {
		dev_err(dev, "fill_packet_body failed, ret = %d\n", ret);
		return ret;
	}

	return ret;
}

static int send_long_packet(struct ipc_mbox_tx_priv *tx_priv,
			struct pkt_send_info *pkt_send_info)
{
	struct device *dev = NULL;
	u8 send_times;
	u32 msg_size;
	u8 i;
	int ret;

	dev = g_ipc_mbox_priv.dev;
	send_times = (pkt_send_info->msg_size + IPC_SHORT_PKT_DATA_SIZE - 1)
			/ IPC_SHORT_PKT_DATA_SIZE;
	msg_size = pkt_send_info->msg_size;

	dev_dbg(dev, "send_times: %d\n", send_times);
	pkt_send_info->header.control.end = IPC_PKT_NOTEND;
	pkt_send_info->header.data_len =  pkt_send_info->msg_size;
	pkt_send_info->pkt_size = IPC_PKT_MAX_SIZE;

	for (i = 0; i < send_times - 1; i++) {
		pkt_send_info->header.control.subid = i;

		ret = assemble_packet(tx_priv, pkt_send_info, i);
		if (ret != 0) {
			dev_err(dev, "long packet assemble failed, ret = %d\n", ret);
			return ret;
		}

		pkt_send_info->msg_buff += IPC_SHORT_PKT_DATA_SIZE;
		msg_size -= IPC_SHORT_PKT_DATA_SIZE;
	}

	/* send last packet */
	pkt_send_info->header.control.subid = i;
	pkt_send_info->header.control.end = IPC_PKT_END;
	pkt_send_info->pkt_size = IPC_PKT_HEADER_SIZE + msg_size;
	ret = assemble_packet(tx_priv, pkt_send_info, i);
	if (ret != 0) {
		dev_err(dev, "long packet sending failed, ret = %d\n", ret);
		return ret;
	}

	pkt_send_info->pkt_size = pkt_send_info->msg_size +
			send_times * IPC_PKT_HEADER_SIZE;

	ret = send_packet_by_ipc(pkt_send_info, tx_priv->tx_pkt);
	if (ret != 0)
		dev_err(dev, "send_packet failed, ret = %d\n", ret);

	tx_priv->seqid++; /* increase seqid */
	memset(tx_priv->tx_pkt, 0, IPC_LONG_PKT_SPLIT_NUM * IPC_PKT_MAX_SIZE);

	return 0;
}

static int ipc_send_packet(struct ipc_mbox_tx_priv *tx_priv,
			struct pkt_send_info *pkt_send_info)
{
	struct device *dev = NULL;
	u8 pkt_type;
	int ret;

	dev = g_ipc_mbox_priv.dev;

	pkt_type = get_packet_type(pkt_send_info->msg_size);
	if (unlikely(pkt_type == IPC_PKT_TYPE_INVALID)) {
		dev_err(dev, "invalied packet type\n");
		return -EMSGSIZE;
	}
	pkt_send_info->header.control.type = pkt_type;

	switch (pkt_type) {
	case IPC_SHORT_PKT:
		ret = send_short_packet(tx_priv, pkt_send_info);
		break;

	case IPC_LONG_PKT:
		ret = send_long_packet(tx_priv, pkt_send_info);
		break;

	default:
		dev_err(dev, "failed peckets type");
		return -EMSGSIZE;
	}

	return ret;
}

static int xr_ipc_send(u8 vc_id, u16 ipc_tag, void *msg_buff, size_t msg_size,
			u8 cmd, int mode, ack_callback_t cb, void *arg)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct pkt_send_info pkt_send_info;
	struct device *dev = NULL;
	int ret;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		dev_err(dev, "xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	if (IS_ERR_OR_NULL(msg_buff) && msg_size > 0) {
		dev_err(dev, "msgbuff is null");
		return -ENOMEM;
	}

	pkt_send_info_init(&pkt_send_info, vc_id, ipc_tag, msg_buff,
			msg_size, IPC_SEND_NO_RESP, mode, cmd);
	pkt_send_info.cb = cb;
	pkt_send_info.arg = arg;

	tx_priv = get_ipc_mbox_tx_priv(vc_id);
	if (tx_priv == NULL) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	mutex_lock(&tx_priv->lock);
	ret = ipc_send_packet(tx_priv, &pkt_send_info);
	mutex_unlock(&tx_priv->lock);

	return ret;
}

int ap_ipc_send_sync(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len)
{
	return xr_ipc_send(vc_id, ipc_tag, tx_buff, tx_len,
			cmd, IPC_SEND_SYNC, NULL, NULL);
}
EXPORT_SYMBOL(ap_ipc_send_sync);

int ap_ipc_send_async(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len, ack_callback_t cb, void *arg)
{
	return xr_ipc_send(vc_id, ipc_tag, tx_buff, tx_len,
			cmd, IPC_SEND_ASYNC, cb, arg);
}
EXPORT_SYMBOL(ap_ipc_send_async);

int ap_ipc_send_recv(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len, void *rx_buff, size_t rx_len)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct pkt_send_info pkt_send_info;
	struct device *dev = NULL;
	u8 pkt_type;
	int ret;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	if (IS_ERR_OR_NULL(tx_buff) || IS_ERR_OR_NULL(rx_buff)
				|| rx_len > IPC_PKT_MAX_SIZE) {
		dev_err(dev, "parameters is invalid\n");
		return -ENOMEM;
	}

	pkt_type = get_packet_type(tx_len);
	if (unlikely(pkt_type == IPC_PKT_TYPE_INVALID)) {
		dev_err(dev, "invalied packet type\n");
		return -EMSGSIZE;
	}

	pkt_send_info_init(&pkt_send_info, vc_id, ipc_tag, tx_buff,
			tx_len, IPC_SEND_NEED_RESP, IPC_SEND_RECV, cmd);
	pkt_send_info.rx_buff = rx_buff;
	pkt_send_info.rx_len = rx_len;
	pkt_send_info.header.control.type = pkt_type;

	tx_priv = get_ipc_mbox_tx_priv(vc_id);
	if (tx_priv == NULL) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	mutex_lock(&tx_priv->lock);
	ret = ipc_send_packet(tx_priv, &pkt_send_info);
	mutex_unlock(&tx_priv->lock);

	return ret;
}
EXPORT_SYMBOL(ap_ipc_send_recv);

int ap_ipc_send_resp(u8 vc_id, u16 ipc_tag, u8 cmd, void *tx_buff, size_t tx_len)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct pkt_send_info pkt_send_info;
	struct device *dev = NULL;
	u8 pkt_type;
	int ret;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	if (IS_ERR_OR_NULL(tx_buff)) {
		dev_err(dev, "parameters is invalid\n");
		return -ENOMEM;
	}

	pkt_type = get_packet_type(tx_len);
	if (unlikely(pkt_type == IPC_PKT_TYPE_INVALID)) {
		dev_err(dev, "invalied packet type\n");
		return -EMSGSIZE;
	}

	pkt_send_info_init(&pkt_send_info, vc_id, ipc_tag, tx_buff,
			tx_len, IPC_SEND_NO_RESP, IPC_SEND_RESP, cmd);
	pkt_send_info.header.control.type = pkt_type;

	tx_priv = get_ipc_mbox_tx_priv(vc_id);
	if (tx_priv == NULL) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	mutex_lock(&tx_priv->lock);
	ret = ipc_send_packet(tx_priv, &pkt_send_info);
	mutex_unlock(&tx_priv->lock);

	return 0;
}
EXPORT_SYMBOL(ap_ipc_send_resp);

int ipc_send_shm_pkt(u8 vc_id, u16 ipc_tag, u8 cmd,
			u32 shm_addr_offset, u16 shm_len)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct pkt_send_info pkt_send_info;
	struct device *dev = NULL;
	u32 tx_msg[IPC_SHM_SEND_MSG] = {0};
	u8 pkt_type;
	int ret;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	pkt_type = IPC_SHM_PKT;
	tx_msg[IPC_SHM_ADDR_OFFSET] = shm_addr_offset;
	tx_msg[IPC_SHM_LEN] = shm_len;

	pkt_send_info_init(&pkt_send_info, vc_id, ipc_tag, tx_msg,
			IPC_SHM_DATA_SIZE, IPC_SEND_NO_RESP, IPC_SEND_SYNC, cmd);
	pkt_send_info.header.control.type = pkt_type;

	tx_priv = get_ipc_mbox_tx_priv(vc_id);
	if (tx_priv == NULL) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	mutex_lock(&tx_priv->lock);
	ret = send_short_packet(tx_priv, &pkt_send_info);
	mutex_unlock(&tx_priv->lock);

	return 0;
}
EXPORT_SYMBOL(ipc_send_shm_pkt);

int ap_ipc_recv_register(u8 vc_id, u16 ipc_tag,
			notifier_handler_t nb, void *arg)
{
	struct tag_notifier_node *tag_node = NULL;
	struct ipc_mbox_rx_priv *rx_priv = NULL;
	struct notifier_info *info = NULL;
	struct device *dev = NULL;
	unsigned long flags = 0;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	rx_priv = get_ipc_mbox_rx_priv(vc_id);
	if (IS_ERR_OR_NULL(rx_priv)) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	tag_node = find_tag_node_spin(rx_priv, ipc_tag);
	if (IS_ERR_OR_NULL(tag_node))
		tag_node = register_tag_node(rx_priv, ipc_tag);

	spin_lock_irqsave(&rx_priv->info_lock, flags);
	info = find_info_node(tag_node, nb);
	spin_unlock_irqrestore(&rx_priv->info_lock, flags);
	if (info) {
		dev_err(dev, "this callback have been register");
		return -EPERM;
	}

	tag_node->vc_id = vc_id;
	info = devm_kcalloc(dev, 1, sizeof(struct notifier_info), GFP_KERNEL);
	if (IS_ERR_OR_NULL(info)) {
		dev_err(dev, "info alloc failed");
		return -ENOMEM;
	}
	info->nb.notifier_call = notifier_func;
	info->nb.priority = 0;
	info->nb.next = NULL;
	info->handler = nb;
	info->arg = arg;

	spin_lock_irqsave(&rx_priv->info_lock, flags);
	list_add_tail(&info->node, &tag_node->info_head);
	spin_unlock_irqrestore(&rx_priv->info_lock, flags);

	atomic_notifier_chain_register(&tag_node->nh, &info->nb);

	return 0;
}
EXPORT_SYMBOL(ap_ipc_recv_register);

int ap_ipc_recv_unregister(u8 vc_id, u16 ipc_tag, notifier_handler_t nb)
{
	struct tag_notifier_node *tag_node = NULL;
	struct ipc_mbox_rx_priv *rx_priv = NULL;
	struct notifier_info *info = NULL;
	struct device *dev = NULL;
	unsigned long flags = 0;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	rx_priv = get_ipc_mbox_rx_priv(vc_id);
	if (IS_ERR_OR_NULL(rx_priv)) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	tag_node = find_tag_node_spin(rx_priv, ipc_tag);
	if (IS_ERR_OR_NULL(tag_node)) {
		dev_warn(dev, "this tag has not been registred, %u\n", ipc_tag);
		return -ENODEV;
	}

	spin_lock_irqsave(&rx_priv->info_lock, flags);
	info = find_info_node(tag_node, nb);
	spin_unlock_irqrestore(&rx_priv->info_lock, flags);
	if (!info) {
		dev_err(dev, "this callback haven't been register");
		return -EPERM;
	}

	atomic_notifier_chain_unregister(&tag_node->nh, &info->nb);

	spin_lock_irqsave(&rx_priv->info_lock, flags);
	list_del(&info->node);
	spin_unlock_irqrestore(&rx_priv->info_lock, flags);

	devm_kfree(dev, info);

	return 0;
}
EXPORT_SYMBOL(ap_ipc_recv_unregister);

#ifdef IPC_PROT_TEST
int ap_ipc_send_lose_seq_test(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct device *dev = NULL;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	tx_priv = get_ipc_mbox_tx_priv(vc_id);
	if (tx_priv == NULL) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	mutex_lock(&tx_priv->lock);
	tx_priv->seqid++;
	mutex_unlock(&tx_priv->lock);

	return xr_ipc_send(vc_id, ipc_tag, tx_buff, tx_len,
			cmd, IPC_SEND_SYNC, NULL, NULL);
}
EXPORT_SYMBOL(ap_ipc_send_lose_seq_test);

int ap_ipc_send_repe_pkt_test(u8 vc_id, u16 ipc_tag, u8 cmd,
		void *tx_buff, size_t tx_len)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct device *dev = NULL;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	tx_priv = get_ipc_mbox_tx_priv(vc_id);
	if (tx_priv == NULL) {
		dev_err(dev, "vc_id %d is invalid\n", vc_id);
		return -ENODEV;
	}

	mutex_lock(&tx_priv->lock);
	tx_priv->seqid--;
	mutex_unlock(&tx_priv->lock);

	return xr_ipc_send(vc_id, ipc_tag, tx_buff, tx_len,
			cmd, IPC_SEND_SYNC, NULL, NULL);
}
EXPORT_SYMBOL(ap_ipc_send_repe_pkt_test);
#endif

static int xr_ipc_prot_priv_init(struct ipc_mbox_priv *privs, struct rproc_list *mbox_list)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct ipc_mbox_rx_priv *rx_priv = NULL;
	struct device *dev = NULL;
	int ret;
	int i;

	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		return -ENODEV;
	}
	dev = g_ipc_mbox_priv.dev;

	for (i = 0; i < mbox_list->vc_num; i++) {
		/* init tx_priv */
		tx_priv = &privs->tx_priv[i];
		tx_priv->vc_id = mbox_list->vc_list[i];
		mutex_init(&tx_priv->lock);
		tx_priv->tx_pkt = privs->tx_pkts + (i * IPC_LONG_PKT_SPLIT_NUM);
		tx_priv->inited = true;

		/* init rx_priv */
		rx_priv = &privs->rx_priv[i];
		rx_priv->seqid = 0;
		rx_priv->subid = 0;
		rx_priv->state = STATE_NORMAL;
		rx_priv->vc_id = mbox_list->vc_list[i];

		rx_priv->rx_pkt = privs->rx_pkts + (i *
			(IPC_LONG_PKT_DATA_SIZE + IPC_PKT_HEADER_SIZE));

		INIT_LIST_HEAD(&rx_priv->tag_head);

		if ((rx_priv->vc_id == IPC_VC_PERI_NS_AP_ISP_CAMERA) ||
			(rx_priv->vc_id == IPC_VC_PERI_NS_AP_ISP0) ||
			(rx_priv->vc_id == IPC_VC_PERI_NS_AP_ISP1) ||
			(rx_priv->vc_id == IPC_VC_PERI_NS_AP_ADSP_AUDIO) ||
			(rx_priv->vc_id == IPC_VC_AO_NS_ADSP_AP_WACKUP))
			continue;

		ret = register_mbox_recv_callback(rx_priv);
		if (ret != 0) {
			dev_err(dev, "register_mbox_recv_callback failed, ret:%d", ret);
			goto unreg_cb;
		}

		mutex_init(&rx_priv->lock);
		spin_lock_init(&rx_priv->tag_lock);
		spin_lock_init(&rx_priv->info_lock);
		rx_priv->inited = true;

	}
	return 0;

unreg_cb:
	for (i = 0; i < mbox_list->vc_num; i++) {
		rx_priv = &privs->rx_priv[i];
		unregister_mbox_recv_callback(rx_priv->vc_id, &rx_priv->nb);
	}

	return ret;
}

static int xr_ipc_priv_init(struct device *dev)
{
	struct rproc_list *rp_list = xr_rproc_list_get();
	u8 priv_num = rp_list->vc_num;
	int pkts_size;
	int ret;

	g_ipc_mbox_priv.dev = dev;
	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.dev)) {
		ipc_prot_err("xr_ipc_prot have not register");
		ret = -ENODEV;
		goto free;
	}

	g_ipc_mbox_priv.rp_list = rp_list;
	g_ipc_mbox_priv.priv_num = priv_num;

	/* allocate memory for tx_priv */
	g_ipc_mbox_priv.tx_priv = devm_kcalloc(dev, priv_num,
			sizeof(struct ipc_mbox_tx_priv), GFP_KERNEL);
	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.tx_priv)) {
		dev_err(dev, "tx_priv alloc failed");
		ret = -ENOMEM;
		goto free;
	}

	/* allocate memory for rx_priv */
	g_ipc_mbox_priv.rx_priv = devm_kcalloc(dev, priv_num,
			sizeof(struct ipc_mbox_rx_priv), GFP_KERNEL);
	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.rx_priv)) {
		dev_err(dev, "rx_priv alloc failed");
		ret = -ENOMEM;
		goto free;
	}

	/* alloc memory pkts for sending msg */
	g_ipc_mbox_priv.tx_pkts = devm_kcalloc(dev, priv_num * IPC_LONG_PKT_SPLIT_NUM,
				sizeof(struct ap_ipc_pkt), GFP_KERNEL);
	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.tx_pkts)) {
		dev_err(dev, "tx_pkts alloc failed");
		ret = -ENOMEM;
		goto free;
	}

	/* alloc memory pkts for receiving msg */
	pkts_size = priv_num * (IPC_LONG_PKT_DATA_SIZE + IPC_PKT_HEADER_SIZE);
	g_ipc_mbox_priv.rx_pkts = devm_kcalloc(dev, pkts_size,
			sizeof(u8), GFP_KERNEL);
	if (IS_ERR_OR_NULL(g_ipc_mbox_priv.rx_pkts)) {
		dev_err(dev, "rx_pkts alloc failed");
		ret = -ENOMEM;
		goto free;
	}

	ret = xr_ipc_prot_priv_init(&g_ipc_mbox_priv, rp_list);
	if (ret != 0) {
		dev_err(dev, "priv_init failed, ret:%d", ret);
		goto free;
	}

free:
	if (ret)
		xr_rproc_list_free(rp_list);
	return 0;
}

static void xr_ipc_priv_uninit(void)
{
	struct ipc_mbox_tx_priv *tx_priv = NULL;
	struct ipc_mbox_rx_priv *rx_priv = NULL;
	u8 priv_num = g_ipc_mbox_priv.priv_num;
	int i;

	for (i = 0; i < priv_num; i++) {
		tx_priv = &g_ipc_mbox_priv.tx_priv[i];
		tx_priv->inited = false;
		rx_priv = &g_ipc_mbox_priv.rx_priv[i];
		unregister_tag(rx_priv);
		unregister_mbox_recv_callback(rx_priv->vc_id, &rx_priv->nb);

		mutex_destroy(&tx_priv->lock);
		mutex_destroy(&rx_priv->lock);
	}

	if (g_ipc_mbox_priv.rp_list)
		xr_rproc_list_free(g_ipc_mbox_priv.rp_list);
}

static void xr_ipc_prot_realease(struct device *dev)
{
	kfree(dev);
}

int xr_ipc_prot_init(void)
{
	struct device *dev = NULL;
	int ret;

	dev = kcalloc(1, sizeof(struct device), GFP_KERNEL);
	if (!dev)
		return -ENODEV;
	dev_set_name(dev, "ipc_prot_device");
	dev->release = xr_ipc_prot_realease;
	ret = device_register(dev);
	if (ret) {
		ipc_prot_err("Failed to register ipc protocol device\n");
		kfree(dev);
		return -ENODEV;
	}

	ret = xr_ipc_priv_init(dev);
	if (ret != 0) {
		dev_err(dev, "priv_init failed, ret: %d", ret);
		kfree(dev);
		return ret;
	}

	dev_info(dev, "xr ipc protocol device init succeed!");

	return 0;
}

void xr_ipc_prot_exit(void)
{
	xr_ipc_priv_uninit();

	if (g_ipc_mbox_priv.dev)
		device_unregister(g_ipc_mbox_priv.dev);

	pr_info("xr ipc protocol device exit succeed!");
}
