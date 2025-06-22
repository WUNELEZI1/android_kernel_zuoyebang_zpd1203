// SPDX-License-Identifier: GPL-2.0
/*
 * ipc.c - IPC Driver for XRing USB ADSP.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "core.h"
#include "hcd.h"
#include "ipc.h"
#include "xr-usb-adsp.h"

#include <common/fk-audio-ipc.h>
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <linux/atomic.h>
#include <linux/compaction.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/kfifo.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/overflow.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <soc/xring/ipc/xr_ipc.h>
#include <soc/xring/securelib/securec.h>

struct usb_adsp_ipc_wait {
	unsigned waiting : 1;
	struct completion comp;
	int result;
};

struct xr_usb_adsp_ipc {
	struct xring_usb_adsp *usb_adsp;
	struct work_struct msg_event_work;
	spinlock_t event_lock;
#define XR_USB_IPC_MSG_FIFO_SIZE 8
	DECLARE_KFIFO(msg_event_fifo, struct xr_usb_adsp_msg,
		      XR_USB_IPC_MSG_FIFO_SIZE);
	struct mutex wait_mutex;
	struct usb_adsp_ipc_wait ipcs_wait[XR_USB_ADSP_AP_MSG_ID_MAX];
};

/* Message handler table */
int usb_adsp_handle_start(struct xring_usb_adsp *usb_adsp,
			  struct xr_usb_adsp_msg *msg)
{
	if (msg->header.len != sizeof(msg->header))
		pr_warn("len in start message is invalid\n");

	return xr_usb_adsp_handle_start(usb_adsp);
}

int usb_adsp_handle_first_hid_in(struct xring_usb_adsp *usb_adsp,
				 struct xr_usb_adsp_msg *msg)
{
	if (msg->header.len != sizeof(msg->header) + sizeof(msg->hid_in_data))
		pr_err("len in first hid in message is invalid\n");

	return xr_usb_adsp_first_hid_in(usb_adsp, msg->hid_in_data.ep_index,
					msg->hid_in_data.actual_len,
					msg->hid_in_data.status);
}

int usb_adsp_handle_common_ack(struct xring_usb_adsp *usb_adsp,
			       struct xr_usb_adsp_msg *msg)
{
	struct usb_adsp_ipc_wait *ipc_wait = NULL;
	int ret = -EINVAL;

	if (msg->header.len != sizeof(msg->header) + sizeof(msg->ack_data)) {
		pr_err("len in ack message is invalid\n");
		return -EINVAL;
	}

	switch (msg->ack_data.id) {
	case XR_USB_ADSP_AP_MSG_ID_STARTINFO:
	case XR_USB_ADSP_AP_MSG_ID_STOP:
	case XR_USB_ADSP_AP_MSG_ID_RESET_DEVICE:
	case XR_USB_ADSP_AP_MSG_ID_GET_SR_STATUS:
		pr_info("ipc %u receive ack message\n", msg->ack_data.id);
		ret = 0;
		break;
	default:
		pr_err("invalid id %u in ack message\n", msg->ack_data.id);
		return ret;
	}

	mutex_lock(&usb_adsp->ipc->wait_mutex);
	ipc_wait = &usb_adsp->ipc->ipcs_wait[msg->ack_data.id];
	if (ipc_wait->waiting) {
		ipc_wait->result = msg->ack_data.result;
		complete(&ipc_wait->comp);
	}
	mutex_unlock(&usb_adsp->ipc->wait_mutex);
	return ret;
}

int usb_adsp_handle_port_change(struct xring_usb_adsp *usb_adsp,
				struct xr_usb_adsp_msg *msg)
{
	if (msg->header.len != sizeof(msg->header) + sizeof(msg->port_data)) {
		pr_err("len in portchange message is invalid\n");
		return -EINVAL;
	}

	return xr_usb_adsp_port_change(usb_adsp, msg->port_data.portsc);
}

int usb_adsp_handle_urb_giveback(struct xring_usb_adsp *usb_adsp,
				 struct xr_usb_adsp_msg *msg)
{
	if (msg->header.len != sizeof(msg->header) + sizeof(msg->buf_data)) {
		pr_err("len in urb giveback message is invalid\n");
		return -EINVAL;
	}

	return xr_usb_adsp_urb_giveback(usb_adsp, msg->buf_data.base,
					msg->buf_data.len);
}

typedef int (*xr_usb_adsp_msg_handler_t)(struct xring_usb_adsp *usb_adsp,
					 struct xr_usb_adsp_msg *msg);
static xr_usb_adsp_msg_handler_t xr_usb_adsp_msg_handler_table[] = {
	[XR_USB_ADSP_MSG_ID_START] = usb_adsp_handle_start,
	[XR_USB_ADSP_MSG_ID_FIRST_HID_IN] = usb_adsp_handle_first_hid_in,
	[XR_USB_ADSP_MSG_ID_COMMON_ACK] = usb_adsp_handle_common_ack,
	[XR_USB_ADSP_MSG_ID_PORT_CHANGE] = usb_adsp_handle_port_change,
	[XR_USB_ADSP_MSG_ID_URB_GIVEBACK] = usb_adsp_handle_urb_giveback,
};

static void xr_usb_adsp_handle_msg(struct work_struct *work)
{
	struct xr_usb_adsp_msg msg;
	struct xr_usb_adsp_ipc *adsp_ipc =
		container_of(work, struct xr_usb_adsp_ipc, msg_event_work);
	int ret = -EINVAL;
	xr_usb_adsp_msg_handler_t handler = NULL;
	struct xring_usb_adsp *usb_adsp = adsp_ipc->usb_adsp;

	mutex_lock(&usb_adsp->mutex);
	if (usb_adsp->suspended) {
		pr_info("delay ipc message handle\n");
		mutex_unlock(&usb_adsp->mutex);
		return;
	}
	mutex_unlock(&usb_adsp->mutex);

	while (kfifo_out_spinlocked(&adsp_ipc->msg_event_fifo, &msg, 1,
				    &adsp_ipc->event_lock)) {
		if (msg.header.id >=
		    ARRAY_SIZE(xr_usb_adsp_msg_handler_table)) {
			pr_err("invalid msg id %u\n", msg.header.id);
			continue;
		}
		handler = xr_usb_adsp_msg_handler_table[msg.header.id];
		if (handler)
			ret = handler(usb_adsp, &msg);

		if (ret)
			pr_err("failed to handle msg %u\n", msg.header.id);
	}
}

static void xr_usb_adsp_ipc_recv(void *msg, int len, void *priv)
{
	struct xr_usb_adsp_msg *usb_msg = (struct xr_usb_adsp_msg *)msg;
	struct xr_usb_adsp_msg msg_queued = { 0 };
	struct xr_usb_adsp_ipc *adsp_ipc = (struct xr_usb_adsp_ipc *)priv;

	if (!usb_msg || !adsp_ipc ||
	    len < sizeof(struct xr_usb_adsp_msg_header)) {
		pr_err("invalid message\n");
		return;
	}

	if (usb_msg->header.msg_source != AP_USB_MSG)
		return;

	if ((int)usb_msg->header.len != len) {
		pr_err("mseeage length %u does not match ipc len %d\n",
		       usb_msg->header.len, len);
		return;
	}

	if (memcpy_s(&msg_queued, sizeof(msg_queued), msg, len) != EOK) {
		pr_err("falied to copy message\n");
		return;
	}
	if (!kfifo_in_spinlocked(&adsp_ipc->msg_event_fifo, &msg_queued, 1,
				 &adsp_ipc->event_lock)) {
		pr_err("falied to queue message event\n");
		return;
	}

	if (!queue_work(system_wq, &adsp_ipc->msg_event_work))
		pr_info("message work busy\n");
}

static void xr_usb_adsp_ipc_recv_register(struct xr_usb_adsp_ipc *adsp_ipc)
{
	fk_audio_ipc_recv_register(AP_USB_MSG, xr_usb_adsp_ipc_recv, adsp_ipc);
}

static void xr_usb_adsp_ipc_recv_unregister(void)
{
	fk_audio_ipc_recv_unregister(AP_USB_MSG);
}

static int xr_usb_adsp_ipc_send(struct xr_usb_adsp_msg *msg)
{
	if (msg->header.len >= XR_USB_ADSP_MSG_LEN_MAX)
		return -EINVAL;

	return fk_audio_ipc_send_async(AP_USB_MSG, (unsigned int *)msg,
				       msg->header.len, NULL, NULL);
}

static int xr_usb_adsp_ipc_send_wait_ack(struct xr_usb_adsp_ipc *ipc,
					 struct xr_usb_adsp_msg *msg,
					 unsigned long timeout)
{
	struct usb_adsp_ipc_wait *ipc_wait = NULL;
	int ret;

	if (!ipc)
		return -EINVAL;

	if (msg->header.id >= XR_USB_ADSP_AP_MSG_ID_MAX)
		return -EINVAL;

	mutex_lock(&ipc->wait_mutex);
	ipc_wait = &ipc->ipcs_wait[msg->header.id];
	if (ipc_wait->waiting) {
		pr_err("message %u wait busy\n", msg->header.id);
		mutex_unlock(&ipc->wait_mutex);
		return -EBUSY;
	}

	reinit_completion(&ipc_wait->comp);
	ipc_wait->result = -EINPROGRESS;
	ipc_wait->waiting = true;
	mutex_unlock(&ipc->wait_mutex);

	ret = xr_usb_adsp_ipc_send(msg);
	if (ret) {
		pr_err("falied to send message\n");
		goto err_release_wait;
	}

	if (!wait_for_completion_timeout(&ipc_wait->comp,
					 msecs_to_jiffies(timeout))) {
		pr_err("timed out waiting for ack of ipc %u\n", msg->header.id);
		ret = -ETIMEDOUT;
		goto err_release_wait;
	}

	mutex_lock(&ipc->wait_mutex);
	ret = ipc_wait->result;
	ipc_wait->waiting = false;
	mutex_unlock(&ipc->wait_mutex);

	return ret;

err_release_wait:
	mutex_lock(&ipc->wait_mutex);
	complete(&ipc_wait->comp);
	ipc_wait->waiting = false;
	mutex_unlock(&ipc->wait_mutex);

	return ret;
}

int usb_adsp_send_start_msg_wait_ack(struct xr_usb_adsp_ipc *ipc,
				     u32 addr, u32 len, unsigned long timeout)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_STARTINFO,
				 sizeof(struct xr_usb_adsp_shared_buf_data));
	msg.buf_data.base = addr;
	msg.buf_data.len = len;

	return xr_usb_adsp_ipc_send_wait_ack(ipc, &msg, timeout);
}

int usb_adsp_send_stop_msg_wait_ack(struct xr_usb_adsp_ipc *ipc,
				    u32 addr, u32 len, unsigned long timeout)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_STOP,
				 sizeof(struct xr_usb_adsp_shared_buf_data));
	msg.buf_data.base = addr;
	msg.buf_data.len = len;

	return xr_usb_adsp_ipc_send_wait_ack(ipc, &msg, timeout);
}

int usb_adsp_send_get_sr_msg_wait_ack(struct xr_usb_adsp_ipc *ipc,
				      unsigned long timeout)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_GET_SR_STATUS, 0);

	return xr_usb_adsp_ipc_send_wait_ack(ipc, &msg, timeout);
}

int usb_adsp_send_set_sr_msg(u32 sr_status)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_SET_SR_STATUS,
				 sizeof(struct xr_usb_adsp_sr_status_data));
	msg.sr_data.status = sr_status;

	return xr_usb_adsp_ipc_send(&msg);
}

int usb_adsp_send_urb_msg(u32 addr, u32 len)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_URB_ENQUEUE,
				 sizeof(struct xr_usb_adsp_shared_buf_data));

	msg.buf_data.base = addr;
	msg.buf_data.len = len;

	return xr_usb_adsp_ipc_send(&msg);
}

int usb_adsp_send_deq_urb_msg(u32 addr, u32 len, bool first_hid)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_URB_DEQUEUE,
				 sizeof(struct xr_usb_adsp_deq_urb_data));

	msg.deq_urb_data.base = addr;
	msg.deq_urb_data.len = len;
	msg.deq_urb_data.first_hid = first_hid;

	return xr_usb_adsp_ipc_send(&msg);
}

int usb_adsp_send_debug_msg(u16 mode, u16 point)
{
	struct xr_usb_adsp_msg msg = { 0 };

	usb_adsp_init_msg_header(&msg, XR_USB_ADSP_AP_MSG_ID_DEBUG,
				 sizeof(struct xr_usb_adsp_debug_data));
	msg.debug_data.mode = mode;
	msg.debug_data.point = point;

	return xr_usb_adsp_ipc_send(&msg);
}

void xr_usb_adsp_ipc_suspend(struct xr_usb_adsp_ipc *ipc)
{
	flush_work(&ipc->msg_event_work);
}

void xr_usb_adsp_ipc_resume(struct xr_usb_adsp_ipc *ipc)
{
	spin_lock(&ipc->event_lock);
	if (!kfifo_is_empty(&ipc->msg_event_fifo))
		if (!queue_work(system_wq, &ipc->msg_event_work))
			pr_info("message work busy\n");
	spin_unlock(&ipc->event_lock);
}

struct xr_usb_adsp_ipc *xr_usb_adsp_ipc_create(struct device *dev,
					       struct xring_usb_adsp *usb_adsp)
{
	struct xr_usb_adsp_ipc *adsp_ipc = NULL;
	int i;

	BUILD_BUG_ON(sizeof(struct xr_usb_adsp_msg) != XR_USB_ADSP_MSG_LEN_MAX);

	adsp_ipc = devm_kzalloc(dev, sizeof(*adsp_ipc), GFP_KERNEL);
	if (!adsp_ipc)
		return ERR_PTR(-ENOMEM);

	xr_usb_adsp_ipc_recv_register(adsp_ipc);

	spin_lock_init(&adsp_ipc->event_lock);
	INIT_WORK(&adsp_ipc->msg_event_work, xr_usb_adsp_handle_msg);
	INIT_KFIFO(adsp_ipc->msg_event_fifo);
	mutex_init(&adsp_ipc->wait_mutex);
	adsp_ipc->usb_adsp = usb_adsp;

	/* init ipc wait completion */
	for (i = 0; i < XR_USB_ADSP_AP_MSG_ID_MAX; i++)
		init_completion(&adsp_ipc->ipcs_wait[i].comp);

	return adsp_ipc;
}

void xr_usb_adsp_ipc_destroy(struct device *dev,
			     struct xr_usb_adsp_ipc *adsp_ipc)
{
	if (!adsp_ipc)
		return;

	cancel_work_sync(&adsp_ipc->msg_event_work);
	xr_usb_adsp_ipc_recv_unregister();
	devm_kfree(dev, adsp_ipc);
}
