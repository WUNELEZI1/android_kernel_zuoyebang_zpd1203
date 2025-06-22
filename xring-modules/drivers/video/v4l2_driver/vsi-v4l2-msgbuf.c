/*
 *    VSI v4l2 message buffer manager.
 *
 *    Copyright (c) 2019, VeriSilicon Inc.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/videodev2.h>
#include <linux/v4l2-dv-timings.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/atomic.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-dv-timings.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h>
#include <media/videobuf2-vmalloc.h>
#include "vsi-v4l2-priv.h"

void *vsi_v4l2daemon_allocmsgmode(struct list_head *head, struct vsi_v4l2_ctx *ctx, u32 msgsize)
{
	struct vsi_msg_node *pmsg;
	struct vsi_msghdr_node *pmsghdr;

	v4l2_klog(LOGLVL_VERBOSE, "%s:%d", __func__, msgsize);
	if (msgsize > sizeof(struct vsi_v4l2_msg_hdr)) {
		pmsg = (struct vsi_msg_node *)kzalloc(sizeof(struct vsi_msg_node), GFP_KERNEL);
		if (!pmsg) {
			v4l2_klog(LOGLVL_ERROR, "%s:%d outof mem", __func__, msgsize);
			return NULL;
		}
		pmsg->ctx = ctx;
		if (ctx) {
			atomic_inc(&ctx->buffed_msgnum);
			v4l2_klog(LOGLVL_VERBOSE, "%lx:%s msgnum=%d", ctx->ctxid, __func__, atomic_read(&ctx->buffed_msgnum));
		}
		list_add_tail(&pmsg->list, head);
		return &pmsg->msg;
	}

	pmsghdr = (struct vsi_msghdr_node *)kzalloc(sizeof(struct vsi_msghdr_node), GFP_KERNEL);
	if (!pmsghdr) {
		v4l2_klog(LOGLVL_ERROR, "%s:%d outof mem", __func__, msgsize);
		return NULL;
	}
	pmsghdr->ctx = ctx;
	if (ctx) {
		atomic_inc(&ctx->buffed_msgnum);
		v4l2_klog(LOGLVL_VERBOSE, "%lx:%s msgnum=%d", ctx->ctxid, __func__, atomic_read(&ctx->buffed_msgnum));
	}
	list_add_tail(&pmsghdr->list, head);
	return &pmsghdr->msghdr;
}

void vsi_v4l2daemon_freemsg(struct list_head *node)
{
	struct vsi_msghdr_node *msghdr = container_of(node, struct vsi_msghdr_node, list);
	list_del(node);
	if (msghdr->ctx) {
		atomic_dec(&msghdr->ctx->buffed_msgnum);
		v4l2_klog(LOGLVL_VERBOSE, "%lx:%s msgnum=%d", msghdr->ctx->ctxid, __func__, atomic_read(&msghdr->ctx->buffed_msgnum));
		msghdr->ctx = NULL;
	}
	kfree(msghdr);
	msghdr = NULL;
}

