// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include "xrisp_ipc.h"
#include "xrisp_common.h"

#include "xrisp_cam_ctrl.h"
#include "xrisp_camctrl_ipcm.h"

//edr ipc recv function
static int edr_ipc_rcv_cb(void *data, unsigned int len)
{
	uint16_t type;
	uint16_t subtype;
	uint32_t log_info;
	uint8_t  reset;
	uint64_t frame_id;
	struct edr_event *msg = (struct edr_event *)(data);

	if (!msg) {
		edr_err("rcv data is null");
		return -1;
	}
#if EDR_DEBUG
	edr_info("recv edr msg: %d bytes", len);
	edr_info("msg->type: %d", msg->type);
	edr_info("msg->subtype: %d", msg->sub_type);
	edr_info("msg->log_info: %d", msg->log_info);
	edr_info("msg->timestamp: %llu", msg->timestamp);
	edr_info("msg->reset: %d", msg->reset);
	edr_info("msg->frame_id: %llu", msg->frame_id);
#endif
	type = msg->type;
	subtype = msg->sub_type;
	log_info = msg->log_info;
	reset = msg->reset;
	frame_id = msg->frame_id;
	edr_drv_submit_api(type, subtype, log_info, reset, frame_id);
	return 0;
}

void edr_ipc_rcv(void)
{
	edr_info("ipc register ok");
	xrisp_topic_handle_register(TOPIC_EDR, edr_ipc_rcv_cb, FAST_ORDERED_HANDLE);
}

void edr_ipc_exit(void)
{
	edr_info("ipc unregister ok");
	xrisp_topic_handle_unregister(TOPIC_EDR);
}
EXPORT_SYMBOL_GPL(edr_ipc_rcv);

//edr ipc send function
int edr_ipc_send(void *data, unsigned int len)
{
	//msg id: send count
	//static uint64_t msg_id;
	int ret;
	int i;
#if EDR_DEBUG
	u8 *pr_data = data;
	int rowcount = 0;
	struct EdrIpcAp2Isp *val = (struct EdrIpcAp2Isp *)data;
#endif
	if (!data) {
		edr_err("send data is null");
		return -1;
	}
	//val->msgid = msg_id++;
	edr_info("buffer: len:%d;msgid:%llu;opcode:%d;size:%d;hwaddr:0x%llx",
		len, val->msgid, val->opcode, val->buf_info.size, val->buf_info.hwaddr);
#if EDR_DEBUG
	rowcount = (len / 8) * 8;
	for (i = 0; i < rowcount; i += 8) {
		edr_info("send data: %02x %02x %02x %02x %02x %02x %02x %02x",
			*(pr_data + i), *(pr_data + i + 1), *(pr_data + i + 2),
			*(pr_data + i + 3), *(pr_data + i + 4), *(pr_data + i + 5),
			*(pr_data + i + 6), *(pr_data + i + 7));
	}
#endif
	ret = xrisp_msg_send(data, len, TOPIC_EDR);
	if (ret != 0) {
		edr_err("send data failed, ret = %d", ret);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(edr_ipc_send);
