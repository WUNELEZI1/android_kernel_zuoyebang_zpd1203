/*
 *    VSI v4l2 message pipe manager.
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
#include <linux/delay.h>
#include "vsi-v4l2-priv.h"
#include "vsi-v4l2.h"
#include "x-v4l2.h"

#define PIPE_DEVICE_NAME      "vsiv4l2daemon"
#define TIMER_TIMEOUT_MS      (10000)

struct daemon_data {
	spinlock_t list_spinlock;
	struct list_head daemon_list;
	struct vsi_v4l2_device *vpu;
};

static bool invoke_vsidaemon;
module_param(invoke_vsidaemon, bool, 0644);

static int loglevel;
module_param(loglevel, int, 0644);

static u32 format_bufinfo_enc(struct vsi_v4l2_ctx *ctx, struct vsi_v4l2_msg *pmsg, struct vb2_buffer *buf);
static void format_bufinfo_dec(struct vsi_v4l2_ctx *ctx, struct vsi_v4l2_msg *pmsg, struct vb2_buffer *buf);

static void _daemon_timer_cb(struct timer_list *daemon_timer)
{
	struct daemon_timer_node *daemon_node;

	if (!daemon_timer) {
		v4l2_klog(LOGLVL_ERROR, "daemon_timer is null.\n");
		return;
	}
	daemon_node = container_of(daemon_timer, struct daemon_timer_node, timer);
	v4l2_klog(LOGLVL_WARNING, "inst_id(%#llx) cmd_id(%d) timer expires, so send signal to work_thread.\n",
			daemon_node->inst_id, daemon_node->cmd_id);

	send_sig(SIGUSR2, daemon_node->task, 0);
}

static int enable_daemon_timer(struct daemon_data *priv_data, unsigned long arg)
{
	int ret;
	bool find = false;
	struct timer_cfg cfg;
	struct daemon_timer_node *daemon_node;

	/* Disable driver from monitoring worker_thread time consumption */
	return 0;

	if (!priv_data) {
		v4l2_klog(LOGLVL_ERROR, "priv_data is null.\n");
		return -EINVAL;
	}
	ret = copy_from_user(&cfg, (void __user *)arg,
					sizeof(struct timer_cfg));
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "copy_from_user failed, returned %d\n", ret);
		return -EFAULT;
	}

	spin_lock(&priv_data->list_spinlock);
	list_for_each_entry(daemon_node, &priv_data->daemon_list, list) {
		if (daemon_node->inst_id == cfg.inst_id) {
			find = true;
			daemon_node->cmd_id  = cfg.cmd_id;
			daemon_node->timeout_ms = TIMER_TIMEOUT_MS;
			/* another cmd in the same worker_thread */
			v4l2_klog(LOGLVL_DEBUG, "inst_id(%#llx) cmd_id(%d) timeout_ms(%d)\n",
				cfg.inst_id, cfg.cmd_id, TIMER_TIMEOUT_MS);
			mod_timer(&daemon_node->timer,
				jiffies + msecs_to_jiffies(daemon_node->timeout_ms));
			break;
		}
	}
	spin_unlock(&priv_data->list_spinlock);

	if (!find) {
		/* the first cmd in worker_thread */
		daemon_node = kzalloc(sizeof(struct daemon_timer_node), GFP_KERNEL);
		if (!daemon_node)
			return -ENOMEM;
		daemon_node->cmd_id  = cfg.cmd_id;
		daemon_node->inst_id = cfg.inst_id;
		daemon_node->timeout_ms = TIMER_TIMEOUT_MS;
		daemon_node->task = current;
		spin_lock(&priv_data->list_spinlock);
		list_add(&daemon_node->list, &priv_data->daemon_list);
		timer_setup(&daemon_node->timer, _daemon_timer_cb, 0);
		daemon_node->timer.expires = jiffies +
				       msecs_to_jiffies(daemon_node->timeout_ms);
		v4l2_klog(LOGLVL_DEBUG, "inst_id(%#llx) cmd_id(%d) timeout_ms(%d)\n",
				cfg.inst_id, cfg.cmd_id, TIMER_TIMEOUT_MS);
		add_timer(&daemon_node->timer);
		spin_unlock(&priv_data->list_spinlock);
	}
	return 0;
}

static int del_daemon_timer(struct daemon_data *priv_data, unsigned long arg)
{
	int ret;
	bool find = false;
	struct timer_cfg cfg;
	struct daemon_timer_node *daemon_node;

	/* Disable driver from monitoring worker_thread time consumption */
	return 0;

	if (!priv_data) {
		v4l2_klog(LOGLVL_ERROR, "priv_data is null.\n");
		return -EINVAL;
	}
	ret = copy_from_user(&cfg, (void __user *)arg,
					sizeof(struct timer_cfg));
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "copy_from_user failed, returned %d\n", ret);
		return -EFAULT;
	}

	spin_lock(&priv_data->list_spinlock);
	list_for_each_entry(daemon_node, &priv_data->daemon_list, list) {
		if (daemon_node->inst_id == cfg.inst_id) {
			find = true;
			v4l2_klog(LOGLVL_DEBUG, "inst_id(%#llx) cmd_id(%d) timeout_ms(%d)\n",
				cfg.inst_id, cfg.cmd_id, cfg.timeout_ms);
			del_timer_sync(&daemon_node->timer);
			break;
		}
	}
	spin_unlock(&priv_data->list_spinlock);

	if (!find) {
		v4l2_klog(LOGLVL_ERROR, "not find inst_id(%#llx) node.\n", cfg.inst_id);
		return -EFAULT;
	}

	return 0;
}

static int des_daemon_timer(struct daemon_data *priv_data, unsigned long arg)
{
	int ret;
	bool find = false;
	struct timer_cfg cfg;
	struct daemon_timer_node *daemon_node, *tmp = NULL;

	/* Disable driver from monitoring worker_thread time consumption */
	return 0;

	if (!priv_data) {
		v4l2_klog(LOGLVL_ERROR, "priv_data is null.\n");
		return -EINVAL;
	}
	ret = copy_from_user(&cfg, (void __user *)arg,
					sizeof(struct timer_cfg));
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "copy_from_user failed, returned %d\n", ret);
		return -EFAULT;
	}

	spin_lock(&priv_data->list_spinlock);
	list_for_each_entry_safe(daemon_node, tmp, &priv_data->daemon_list, list) {
		if (daemon_node->inst_id == cfg.inst_id) {
			find = true;
			list_del_init(&daemon_node->list);
			v4l2_klog(LOGLVL_DEBUG, "inst_id(%#llx) cmd_id(%d) timeout_ms(%d)\n",
				cfg.inst_id, cfg.cmd_id, cfg.timeout_ms);
			kfree(daemon_node);
			break;
		}
	}
	spin_unlock(&priv_data->list_spinlock);

	if (!find) {
		v4l2_klog(LOGLVL_ERROR, "not find inst_id(%#llx) node.\n", cfg.inst_id);
		return -EFAULT;
	}
	return 0;
}

int vsi_v4l2_daemonalive(struct vsi_v4l2_device *vpu)
{
	return (vpu && atomic_read(&vpu->daemon_fn) > 0);
}

void vsiv4l2_cleanupdaemonmsg(struct vsi_v4l2_device *vpu)
{
	struct list_head *head = &vpu->listhead;

	v4l2_klog(LOGLVL_DEBUG, "%d\n", vpu->deviceid);
	mutex_lock(&vpu->cmd_lock);
	while (!list_empty(head))
		vsi_v4l2daemon_freemsg(head->next);
	mutex_unlock(&vpu->cmd_lock);
}

void vsiv4l2_cleanupdaemon(struct vsi_v4l2_device *vpu)
{
	char daemondevname[sizeof(PIPE_DEVICE_NAME) + 3] = {0};

	vsiv4l2_cleanupdaemonmsg(vpu);
	memcpy(daemondevname, PIPE_DEVICE_NAME, sizeof(PIPE_DEVICE_NAME));
	if (vpu->deviceid < 10) {
		daemondevname[sizeof(PIPE_DEVICE_NAME) - 1] = vpu->deviceid + 0x30;
		daemondevname[sizeof(PIPE_DEVICE_NAME)] = 0;
	} else {
		daemondevname[sizeof(PIPE_DEVICE_NAME) - 1] = vpu->deviceid / 10 + 0x30;
		daemondevname[sizeof(PIPE_DEVICE_NAME)] = vpu->deviceid % 10 + 0x30;
		daemondevname[sizeof(PIPE_DEVICE_NAME) + 1] = 0;
	}
	unregister_chrdev(VSI_DAEMON_DEVMAJOR + vpu->deviceid, daemondevname);
}

int vsi_clear_daemonmsg(struct vsi_v4l2_ctx *ctx)
{
	ulong instid = ctx->ctxid;
	struct vsi_v4l2_device *vpu = ctx->dev;
	struct list_head *head = &vpu->listhead;
	struct list_head *node, *next;
	struct vsi_msghdr_node *msghdr;
	s32 count;

	mutex_lock(&vpu->cmd_lock);

	count = atomic_read(&ctx->buffed_msgnum);
	v4l2_klog(LOGLVL_DEBUG, "%lx %s:%d", ctx->ctxid, __func__, count);
	if (count <= 0)
		goto tail;

	node = head->next;
	while (node != head) {
		msghdr = container_of(node, struct vsi_msghdr_node, list);
		if (msghdr->msghdr.inst_id == instid) {
			v4l2_klog(LOGLVL_DEBUG, "clear unused cmd %lx:%lx:%d", instid, msghdr->msghdr.seq_id, msghdr->msghdr.cmd_id);
			next = node->next;
			vsi_v4l2daemon_freemsg(node);
			node = next;
		} else
			node = node->next;
	}
tail:
	atomic_set(&ctx->buffed_msgnum, 0);
	mutex_unlock(&vpu->cmd_lock);

	return 0;
}

static int hasMsg(struct vsi_v4l2_device *vpu)
{
	int empty = 0;

	if (mutex_lock_killable(&vpu->cmd_lock)) {
		v4l2_klog(LOGLVL_ERROR, "mutex_lock_killable return from signal\n");
		return -EBUSY;
	}
	empty = list_empty(&vpu->listhead);
	mutex_unlock(&vpu->cmd_lock);
	return !empty;
}

static int getMsg(struct vsi_v4l2_device *vpu, char __user *buf, size_t size)
{
	size_t offset = 0;
	struct list_head *head = &vpu->listhead, *node;
	struct vsi_msghdr_node *msghdr;

	if (mutex_lock_killable(&vpu->cmd_lock)) {
		v4l2_klog(LOGLVL_ERROR, "mutex_lock_killable return from signal\n");
		return -EBUSY;
	}
	node = head->next;
	if (node == head)	//double protect
		goto tail;
	msghdr = container_of(node, struct vsi_msghdr_node, list);
	if ((offset + sizeof(struct vsi_v4l2_msg_hdr) + msghdr->msghdr.size) > size) {
		v4l2_klog(LOGLVL_WARNING, "%lx read msg buffer size %ld too small", msghdr->msghdr.inst_id, size);
		goto tail;
	}
	if (copy_to_user((void __user *)buf + offset,
		(void *)&msghdr->msghdr, sizeof(struct vsi_v4l2_msg_hdr) + msghdr->msghdr.size) != 0) {
		v4l2_klog(LOGLVL_ERROR, "%lx send msg id %d:%ld fail", msghdr->msghdr.inst_id, msghdr->msghdr.cmd_id, size);
		goto tail;
	} else {
		v4l2_klog(LOGLVL_VERBOSE, "%lx send msg  id = %d", msghdr->msghdr.inst_id, msghdr->msghdr.cmd_id);
	}
	offset += sizeof(struct vsi_v4l2_msg_hdr) + msghdr->msghdr.size;
	vsi_v4l2daemon_freemsg(node);

tail:
	mutex_unlock(&vpu->cmd_lock);
	return offset;
}

/* send msg from v4l2 driver to user space daemon */
static int vsi_v4l2_sendcmd(
	struct vsi_v4l2_ctx *ctx,
	enum v4l2_daemon_cmd_id cmdid,
	int codecformat,
	void *args,
	u32 param_type)
{
	u32 msgsize;
	struct vsi_v4l2_device *vpu;
	unsigned long instid;
	struct vsi_v4l2_msg *pmsg;
	struct vsi_v4l2_msg_hdr *msghdr;

	if (cmdid == V4L2_DAEMON_VIDIOC_EXIT) {
		vpu = (struct vsi_v4l2_device *)args;
		instid = 0;
	} else {
		vpu = ctx->dev;
		instid = ctx->ctxid;
	}

	if (atomic_read(&vpu->daemon_fn) <= 0)
		return DAEMON_ERR_DAEMON_MISSING;

	if (mutex_lock_killable(&vpu->cmd_lock)) {
		v4l2_klog(LOGLVL_ERROR, "mutex_lock_killable return from signal\n");
		return -EBUSY;
	}

	v4l2_klog(LOGLVL_VERBOSE, "%lx:%d:%x", instid, cmdid, param_type);
	if (cmdid == V4L2_DAEMON_VIDIOC_BUF_RDY) {
		pmsg = vsi_v4l2daemon_allocmsgmode(&vpu->listhead, ctx, sizeof(struct vsi_v4l2_msg));
		if (!pmsg) {
			mutex_unlock(&vpu->cmd_lock);
			return DAEMON_ERR_NO_MEM;
		}
		if (isencoder(ctx))
			msgsize = format_bufinfo_enc(ctx, pmsg, args);
		else {
			format_bufinfo_dec(ctx, pmsg, args);
			msgsize = sizeof(struct v4l2_daemon_dec_buffers);
		}
		pmsg->inst_id = instid;
		pmsg->cmd_id = cmdid;
		pmsg->codec_fmt = codecformat;
		pmsg->param_type = param_type;
		pmsg->seq_id = vpu->g_seqid;
		pmsg->size = msgsize;
		pmsg->error = 0;
	} else {
		//these two cmds may casue use_after_free if ctx is not null
		//V4L2_DAEMON_VIDIOC_DESTROY_ENC trigger by release_ctx(), and the function will free(ctx) following
		//in another thread, getMsg() --> vsi_v4l2daemon_freemsg() will use ctx
		//set ctx to null in the two cmd to avoid racing condition
		if (cmdid == V4L2_DAEMON_VIDIOC_DESTROY_ENC ||
			cmdid == V4L2_DAEMON_VIDIOC_DESTROY_DEC) {
			ctx = NULL;
		}
		msghdr = vsi_v4l2daemon_allocmsgmode(&vpu->listhead, ctx, sizeof(struct vsi_v4l2_msg_hdr));
		if (!msghdr) {
			mutex_unlock(&vpu->cmd_lock);
			return DAEMON_ERR_NO_MEM;
		}
		msghdr->inst_id = instid;
		msghdr->cmd_id = cmdid;
		msghdr->codec_fmt = codecformat;
		msghdr->param_type = param_type;
		msghdr->size = 0;
		msghdr->error = 0;
		msghdr->seq_id = vpu->g_seqid;
	}
	vpu->g_seqid++;
	if (vpu->g_seqid >= SEQID_UPLIMT)
		vpu->g_seqid = 1;
	mutex_unlock(&vpu->cmd_lock);
	wake_up_interruptible_all(&vpu->cmd_queue);

	v4l2_klog(LOGLVL_VERBOSE, "%lx:%d", instid, cmdid);
	return 0;
}

/* ioctl handler from daemon dev */
static long vsi_v4l2_daemon_ioctl(
	struct file *filp,
	unsigned int cmd,
	unsigned long arg)
{
	int error = 0;
	struct vsi_v4l2_dev_info *hwinfo;
	struct vsi_v4l2_dma_buffers_info info;
	struct daemon_data *priv_data;
	struct vsi_v4l2_device *vpu;
	int ret;

	priv_data = filp->private_data;
	if (unlikely(!priv_data))
		return -EFAULT;

	vpu = priv_data->vpu;
	switch (cmd) {
	case VSI_IOCTL_CMD_INITDEV:
		hwinfo = kzalloc(sizeof(struct vsi_v4l2_dev_info), GFP_KERNEL);
		if (!hwinfo)
			return -ENOMEM;
		if (copy_from_user((void *)hwinfo, (void __user *)arg, sizeof(*hwinfo)) != 0) {
			kfree(hwinfo);
			v4l2_klog(LOGLVL_ERROR, "fail to get data");
			return -EINVAL;
		}
		vsiv4l2_set_hwinfo(hwinfo);
		kfree(hwinfo);
		break;
	case VSI_IOCTL_CMD_DMA_BUFFER_INFO:
		if (!vpu) {
			v4l2_klog(LOGLVL_ERROR, "vpu is null\n");
			return -ENODEV;
		}
		if (copy_from_user((void *)&info, (void __user *)arg, sizeof(info)) != 0) {
			v4l2_klog(LOGLVL_ERROR, "fail to get data");
			return -EINVAL;
		}
		if (vsi_v4l2_get_buffer_info(vpu, &info) != 0) {
			v4l2_klog(LOGLVL_ERROR, "fail to get buffer info");
			return -EINVAL;
		}
		if (copy_to_user((void __user *)arg, &info, sizeof(info)) != 0) {
			v4l2_klog(LOGLVL_ERROR, "fail to copy to user");
			return -EINVAL;
		}
		break;
	case VSI_IOCTL_CMD_START_TIMER: {
		ret = enable_daemon_timer(priv_data, arg);
		return ret;
	}
	case VSI_IOCTL_CMD_DEL_TIMER: {
		ret = del_daemon_timer(priv_data, arg);
		return ret;
	}
	case VSI_IOCTL_CMD_DESTROY_TIMER: {
		ret = des_daemon_timer(priv_data, arg);
		return ret;
	}
	default:
		return -EINVAL;
	}
	return error;
}

static int getbusaddr(struct vsi_v4l2_ctx *ctx, dma_addr_t  *busaddr, struct vb2_buffer *buf)
{
	// void *baseaddr, *p;
	int planeno = buf->num_planes, i;

	for (i = 0; i < planeno; i++)
		busaddr[i] = 0;

	v4l2_klog(LOGLVL_VERBOSE, "%d:%d:%lx:%lx:%lx", buf->type, planeno,
		(unsigned long)busaddr[0], (unsigned long)busaddr[1], (unsigned long)busaddr[2]);
	return planeno;
}

void *getvaddr_meta(struct vsi_v4l2_ctx *ctx, struct vb2_buffer *buf)
{
	void *va;

	va = vb2_plane_vaddr(buf, 0);
	if (va == NULL) {
		unsigned long *p = vb2_plane_cookie(buf, 0);

		va = ioremap(*p, 0x1000);
		if (va == NULL) //in RAM
			va = phys_to_virt(*p);
		else	//inform for dqueue meta buf
			set_bit(CTX_FLAG_MAPMETA, &ctx->flag);
	}
	return va;
}

dma_addr_t  getbusaddr_meta(struct vsi_v4l2_ctx *ctx,  struct vb2_buffer *buf)
{
	void *baseaddr, *p;

	baseaddr = vb2_plane_vaddr(buf, 0);
	p = vb2_plane_cookie(buf, 0);
	if (p != NULL)
		return *(dma_addr_t *)p;
	else
		return virt_to_phys(baseaddr);
}

static u32 format_bufinfo_enc(struct vsi_v4l2_ctx *ctx, struct vsi_v4l2_msg *pmsg, struct vb2_buffer *buf)
{
	u32 planeno, size = 0;
	struct v4l2_daemon_enc_buffers *encbufinfo;
	dma_addr_t  busaddr[MAX_PLANENO];

	if (buf == NULL) {
		v4l2_klog(LOGLVL_ERROR, "error buf is null");
		return 0;
	}

	if (binputqueue(buf->type) && ctx->srcvbufflag[buf->index] & FORCE_IDR) {
		if (!isimage(ctx->mediacfg.outfmt_fourcc))
			flag_updateparam(m_forceidr)
		ctx->srcvbufflag[buf->index] &= ~FORCE_IDR;
	}

	memset((void *)&pmsg->params.enc_params, 0, sizeof(struct v4l2_daemon_enc_params));
	if (binputqueue(buf->type)) {
		pmsg->params.enc_params.param_num = writeout_encparam(ctx, pmsg->params.enc_params.paramlist, &size);
		pmsg->params.enc_params.io_buffer.timestamp = buf->timestamp;
	}
	pmsg->params.enc_params.io_buffer.addr_offset = ctx->addr_offset;
	size += (sizeof(struct v4l2_daemon_enc_buffers) + sizeof(u32));		//io_buffer and param_num
	planeno = getbusaddr(ctx, busaddr, buf);
	encbufinfo = &pmsg->params.enc_params.io_buffer;
	encbufinfo->metabufinfo.busMetaBuf = ctx->mediacfg.metabufinfo.busMetaBuf;
	encbufinfo->metabufinfo.metabufsize = ctx->mediacfg.metabufinfo.metabufsize;
	if (ctx->mediacfg.metabufinfo.metabufsize > 0 &&
			ctx->mediacfg.metabufinfo.metabufsize <
							ctx->input_que.num_buffers * sizeof(struct v4l2_vsi_enc_meta_info_OUTPUT) +
									ctx->output_que.num_buffers * sizeof(struct v4l2_vsi_enc_meta_info_CAPTURE) +
									sizeof(struct v4l2_vsi_stream_meta_info_enc))
		v4l2_klog(LOGLVL_WARNING, "queued meta buffer size %d is smaller than required",
							ctx->mediacfg.metabufinfo.metabufsize);
	if (binputqueue(buf->type)) {
		encbufinfo->busLuma = busaddr[0] + buf->planes[0].data_offset;
		encbufinfo->busLumaSize = ctx->mediacfg.sizeimagesrc[0];
		encbufinfo->busChromaU = 0;
		encbufinfo->busChromaUSize = 0;
		encbufinfo->busChromaV = 0;
		encbufinfo->busChromaVSize = 0;
		if (planeno == 2) {
			encbufinfo->busChromaU = busaddr[1] + buf->planes[1].data_offset;
			encbufinfo->busChromaUSize = ctx->mediacfg.sizeimagesrc[1];
		} else if (planeno == 3) {
			encbufinfo->busChromaU = busaddr[1] + buf->planes[1].data_offset;
			encbufinfo->busChromaUSize = ctx->mediacfg.sizeimagesrc[1];
			encbufinfo->busChromaV = busaddr[2] + buf->planes[2].data_offset;
			encbufinfo->busChromaVSize = ctx->mediacfg.sizeimagesrc[2];
		}
		encbufinfo->busOutBuf = 0;
		encbufinfo->outBufSize = 0;
		encbufinfo->inbufidx = buf->index;
		encbufinfo->outbufidx = -1;
	} else {
		encbufinfo->busLuma = 0;
		encbufinfo->busChromaU = 0;
		encbufinfo->busChromaV = 0;
		encbufinfo->busOutBuf = busaddr[0] + buf->planes[0].data_offset;
		encbufinfo->outBufSize = ctx->mediacfg.sizeimagedst[0];

		if (ctx->mediacfg.m_encparams.m_scaleoutput.scaleOutput) {
			encbufinfo->busScaleOutBuf = busaddr[1] + buf->planes[1].data_offset;
			encbufinfo->scaleoutBufSize = ctx->mediacfg.sizeimagedst[1];
		}
		encbufinfo->outbufidx = buf->index;
		encbufinfo->inbufidx = -1;
	}
	encbufinfo->bytesused = buf->planes[0].bytesused;
	return size;
}

static void format_bufinfo_dec(struct vsi_v4l2_ctx *ctx, struct vsi_v4l2_msg *pmsg, struct vb2_buffer *buf)
{
	struct v4l2_daemon_dec_buffers *decbufinfo;
	dma_addr_t  busaddr[MAX_PLANENO];
	struct vb2_buffer *vb;
	struct vb2_v4l2_buffer *vbuf;
	s32 i, planeno = 0;

	memcpy((void *)&pmsg->params.dec_params.io_buffer, (void *)&ctx->mediacfg.decparams.io_buffer, sizeof(struct v4l2_daemon_dec_buffers));

	test_and_clear_bit(PARAM_UPDATE_BIT, &ctx->mediacfg.decparams.io_buffer.framerate_info.head);
	if (binputqueue(buf->type))
		pmsg->params.dec_params.dec_info.io_buffer.timestamp = buf->timestamp;
	planeno = getbusaddr(ctx, busaddr, buf);
	decbufinfo = &pmsg->params.dec_params.io_buffer;
	decbufinfo->addr_offset = ctx->addr_offset;
	decbufinfo->metabufinfo.busMetaBuf = ctx->mediacfg.metabufinfo.busMetaBuf;
	decbufinfo->metabufinfo.metabufsize = ctx->mediacfg.metabufinfo.metabufsize;
	if (ctx->mediacfg.metabufinfo.metabufsize > 0 &&
			ctx->mediacfg.metabufinfo.metabufsize <
							ctx->output_que.num_buffers * sizeof(struct v4l2_vsi_dec_meta_info_CAPTURE) +
									ctx->input_que.num_buffers * sizeof(struct v4l2_vsi_dec_meta_info_OUTPUT) +
									sizeof(struct v4l2_vsi_stream_meta_info_dec))
		v4l2_klog(LOGLVL_WARNING, "queued meta buffer size %d is smaller than required",
							ctx->mediacfg.metabufinfo.metabufsize);
	for (i = 0; i < MAX_PLANENO; i++) {
		decbufinfo->busOutBuf[i] = 0;
		decbufinfo->OutBufSize[i] = 0;
		decbufinfo->bytesused[i] = 0;
	}
	if (!binputqueue(buf->type)) {
		decbufinfo->inbufidx = -1;
		decbufinfo->outbufidx = buf->index;
		decbufinfo->busInBuf = 0;
		decbufinfo->inBufSize = 0;
		vb = ctx->output_que.bufs[buf->index];
		vbuf = to_vb2_v4l2_buffer(vb);
		for (i = 0; i < planeno; i++) {
			decbufinfo->busOutBuf[i] = busaddr[i] + buf->planes[i].data_offset;
			decbufinfo->OutBufSize[i] = vbuf->planes[i].length;
			decbufinfo->bytesused[i] = vbuf->planes[i].bytesused;
		}
		if (((ctx->mediacfg.src_pixeldepth == ctx->mediacfg.decparams.dec_info.io_buffer.outputPixelDepth)
			&& ctx->mediacfg.src_pixeldepth != 16)	//p010 can only set by user, not from ctrl sw
			|| !test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag))
			pmsg->params.dec_params.io_buffer.outputPixelDepth = DEFAULT_PIXELDEPTH;
	} else {
		decbufinfo->inbufidx = buf->index;
		decbufinfo->outbufidx = -1;
		decbufinfo->busInBuf = busaddr[0] + buf->planes[0].data_offset;
		vb = ctx->input_que.bufs[buf->index];
		vbuf = to_vb2_v4l2_buffer(vb);
		decbufinfo->inBufSize = vbuf->planes[0].length;
		decbufinfo->bytesused[0] = vbuf->planes[0].bytesused;
	}
}

int vsiv4l2_execcmd(struct vsi_v4l2_ctx *ctx, enum v4l2_daemon_cmd_id id, void *args)
{
	int ret = 0;
	u32 param = 0;
	struct vsi_v4l2_device *vpu = NULL;

	if (!ctx || !ctx->dev) {
		v4l2_klog(LOGLVL_ERROR, "ctx %p or ctx->dev is null\n", ctx);
		return -ENODEV;
	}

	vpu = ctx->dev;
	if (atomic_read(&vpu->daemon_fn) <= 0) {
		ret = -DAEMON_ERR_DAEMON_MISSING;
		goto tail;
	}

	switch (id) {
	case V4L2_DAEMON_VIDIOC_DESTROY_ENC:
	case V4L2_DAEMON_VIDIOC_ENC_RESET:
		ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.m_encparams.m_codecfmt.codecFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_DESTROY_DEC:
		ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.decparams.dec_info.io_buffer.outBufFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_CMD_STOP:
		ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.m_encparams.m_codecfmt.codecFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_STREAMON:
		if (test_and_clear_bit(CTX_FLAG_ENC_FLUSHBUF, &ctx->flag))
			param = 1;
		ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.m_encparams.m_codecfmt.codecFormat, NULL, param);
		break;
	case V4L2_DAEMON_VIDIOC_STREAMOFF_OUTPUT:
		if (isencoder(ctx))
			ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.m_encparams.m_srcinfo.inputFormat, NULL, 0);
		else
			ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.decparams.dec_info.io_buffer.inputFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_STREAMOFF_CAPTURE:
		if (isencoder(ctx))
			ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.m_encparams.m_codecfmt.codecFormat, NULL, 0);
		else
			ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.decparams.dec_info.io_buffer.outBufFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_STREAMON_OUTPUT:
		ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.decparams.dec_info.io_buffer.inputFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_STREAMON_CAPTURE:
		ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.decparams.dec_info.io_buffer.outBufFormat, NULL, 0);
		break;
	case V4L2_DAEMON_VIDIOC_BUF_RDY:
		if (isencoder(ctx))
			ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.m_encparams.m_codecfmt.codecFormat, args, 0);
		else
			ret = vsi_v4l2_sendcmd(ctx, id, ctx->mediacfg.decparams.dec_info.io_buffer.inputFormat, args, 0);

		break;
	default:
		v4l2_klog(LOGLVL_WARNING, "unexpected cmd id %d", id);
		return -1;
	}
tail:
	if (ret < 0) {
		vsi_set_ctx_error(ctx, ret);
		v4l2_klog(LOGLVL_ERROR, "%d:%lx fail to communicate with daemon, error=%d, cmd=%d", ctx->dev->deviceid, ctx->ctxid, ret, id);
	} else
		set_bit(CTX_FLAG_DAEMONLIVE_BIT, &ctx->flag);
	return ret;
}

static int invoke_daemonapp(u32 devid)
{
#define DAEMON_LVL "HANTRO_LOG_LEVEL="
	int ret;
	char loglvl[sizeof(DAEMON_LVL) + 2] = {0};
	char idstr[3];	//suppose MAX_VIDEODEV_NO is less than 100
#if defined(CONFIG_ANDROID)
	char *argv[] = {"/system/bin/sh", "-c", "/vendor/bin/vsidaemon", idstr, NULL};
	char *env[] = {"LD_LIBRARY_PATH=/vendor/lib64",
		"DAEMON_LOGPATH=/data/vendor/vsi/daemon.log",
		loglvl,
		NULL};
#else
	char *argv[] = {VSI_DAEMON_PATH, idstr, NULL};
	char *env[] = {"LD_LIBRARY_PATH=/usr/lib",
		"DAEMON_LOGPATH=/home/vsi/daemon.log",
		loglvl,
		NULL};
#endif

	memcpy(loglvl, DAEMON_LVL, sizeof(DAEMON_LVL));
	loglvl[sizeof(DAEMON_LVL) - 1] = loglevel/10 + 0x30;
	loglvl[sizeof(DAEMON_LVL)] = loglevel%10 + 0x30;
	loglvl[sizeof(DAEMON_LVL) + 1] = 0;
	if (devid < 10) {
		idstr[0] = devid + 0x30;
		idstr[1] = 0;
	} else {
		idstr[0] = devid / 10 + 0x30;
		idstr[1] = devid % 10 + 0x30;
		idstr[2] = 0;
	}
	ret = call_usermodehelper(argv[0], argv, env, UMH_WAIT_EXEC);

	return ret;
}

int vsi_v4l2_addinstance(struct vsi_v4l2_device *vpu, pid_t *ppid)
{
	int ret = 0;

	v4l2_klog(LOGLVL_DEBUG, "from inst num %d", vpu->v4l2_fn);
	if (!invoke_vsidaemon && atomic_read(&vpu->daemon_fn) <= 0)
		return -ENODEV;
	if (mutex_lock_killable(&vpu->instance_lock)) {
		v4l2_klog(LOGLVL_ERROR, "mutex_lock_killable return from signal\n");
		return -EBUSY;
	}

	if (vpu->v4l2_fn >= MAX_STREAMS)
		ret = -EBUSY;
	else {
		vpu->v4l2_fn++;
		if (vpu->v4l2_fn == 1 && invoke_vsidaemon) {
			ret = invoke_daemonapp(vpu->deviceid);
			if (ret < 0)
				vpu->v4l2_fn--;
			else {
				ret = wait_event_interruptible_timeout(vpu->instance_queue,
						atomic_read(&vpu->daemon_fn) > 0, msecs_to_jiffies(10000));
				if (ret == -ERESTARTSYS || ret == 0) {
					ret = -ERESTARTSYS;
					vpu->v4l2_fn--;
					goto tail;
				}
			}
			v4l2_klog(LOGLVL_DEBUG, "invoke daemon on %d=%d:%d", vpu->deviceid, ret, vpu->v4l2_fn);
		}
	}
tail:
	mutex_unlock(&vpu->instance_lock);

	return ret;
}

int vsiv4l2_execexitcmd(struct vsi_v4l2_device *vpu)
{
	int ret;

	if (atomic_read(&vpu->daemon_fn) <= 0)
		return -DAEMON_ERR_DAEMON_MISSING;

	ret = vsi_v4l2_sendcmd(NULL, V4L2_DAEMON_VIDIOC_EXIT, 0, vpu, 0);
	return ret;
}

int vsi_v4l2_quitinstance(struct vsi_v4l2_device *vpu)
{
	int ret = 0;

	v4l2_klog(LOGLVL_DEBUG, "%d from instnum %d", vpu->deviceid, vpu->v4l2_fn);
	if (mutex_lock_killable(&vpu->instance_lock)) {
		v4l2_klog(LOGLVL_ERROR, "mutex_lock_killable return from signal\n");
		return -EBUSY;
	}
	vpu->v4l2_fn--;
	if (invoke_vsidaemon && vpu->v4l2_fn == 0) {
		ret = vsiv4l2_execexitcmd(vpu);
		if (wait_event_interruptible(vpu->instance_queue, atomic_read(&vpu->daemon_fn) <= 0)) {
			v4l2_klog(LOGLVL_WARNING, "wait_event_interruptible return from signal\n");
			ret = -ERESTARTSYS;
		}
	}
	mutex_unlock(&vpu->instance_lock);
	return 0;
}

static ssize_t v4l2_msg_read(struct file *filp, char __user *buf, size_t size, loff_t *offest)
{
	int ret;
	struct vsi_v4l2_device *vpu;
	struct daemon_data *priv_data;

	priv_data = filp->private_data;
	if (unlikely(!priv_data))
		return -EFAULT;
	vpu = priv_data->vpu;

	ret = wait_event_interruptible_timeout(vpu->cmd_queue, (hasMsg(vpu) != 0),
			msecs_to_jiffies(600000));
	if (ret == -ERESTARTSYS)
		return -EIO;
	else if (ret == 0)
		return 0;
	return getMsg(vpu, buf, size);
}

static int vsi_handle_daemonmsg(struct vsi_v4l2_device *vpu, struct vsi_v4l2_msg *pmsg)
{
	if (pmsg->error < 0)
		return vsi_v4l2_handleerror(vpu, pmsg->inst_id, pmsg->error);

	switch (pmsg->cmd_id) {
	case V4L2_DAEMON_VIDIOC_BUF_RDY:
		return vsi_v4l2_bufferdone(vpu, pmsg);
	case V4L2_DAEMON_VIDIOC_CHANGE_RES:
		return vsi_v4l2_notify_reschange(vpu, pmsg);
	case V4L2_DAEMON_VIDIOC_PICCONSUMED:
		return vsi_v4l2_handle_picconsumed(vpu, pmsg);
	case V4L2_DAEMON_VIDIOC_CROPCHANGE:
		return vsi_v4l2_handle_cropchange(vpu, pmsg);
	case V4L2_DAEMON_VIDIOC_WARNONOPTION:
		return vsi_v4l2_handle_warningmsg(vpu, pmsg);
	case V4L2_DAEMON_VIDIOC_STREAMOFF_CAPTURE_DONE:
	case V4L2_DAEMON_VIDIOC_STREAMOFF_OUTPUT_DONE:
		return vsi_v4l2_handle_streamoffdone(vpu, pmsg);
	default:
		return -EINVAL;
	}
}

static ssize_t v4l2_msg_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
	size_t msgsize = 0, ret = 0;
	struct vsi_v4l2_msg *pmsg;
	struct vsi_v4l2_device *vpu;
	struct vsi_v4l2_msg_hdr msghdr;
	struct daemon_data *priv_data;

	priv_data = filp->private_data;
	if (unlikely(!priv_data))
		return -EFAULT;
	vpu = priv_data->vpu;

	if (vpu->v4l2_fn == 0)
		goto error;
	if (size < sizeof(struct vsi_v4l2_msg_hdr))
		goto error;
	if (!access_ok((void *) buf, size)) {
		v4l2_klog(LOGLVL_ERROR, "input data unaccessable");
		goto error;
	}
	if (copy_from_user((void *)&msghdr,
		(void __user *)buf, sizeof(struct vsi_v4l2_msg_hdr)) != 0) {
		goto error;
	}
	msgsize = msghdr.size;
	if (msgsize + sizeof(struct vsi_v4l2_msg_hdr) > size ||
		msgsize + sizeof(struct vsi_v4l2_msg_hdr) > sizeof(struct vsi_v4l2_msg))
		goto error;
	if (msgsize > 0) {
		pmsg = kzalloc(sizeof(struct vsi_v4l2_msg), GFP_KERNEL);
		if (!pmsg)
			goto error;

		pmsg->size = msghdr.size;
		pmsg->error = msghdr.error;
		pmsg->seq_id = msghdr.seq_id;
		pmsg->inst_id = msghdr.inst_id;
		pmsg->cmd_id = msghdr.cmd_id;
		pmsg->codec_fmt = msghdr.codec_fmt;
		pmsg->param_type = msghdr.param_type;
		if (copy_from_user((void *)pmsg + sizeof(struct vsi_v4l2_msg_hdr),
			(void __user *)buf + sizeof(struct vsi_v4l2_msg_hdr), msgsize) != 0) {
			kfree(pmsg);
			goto error;
		}
	} else {
		pmsg = (struct vsi_v4l2_msg *)&msghdr;
	}
	v4l2_klog(LOGLVL_VERBOSE, "get msg  id = %d, flag = %x, seqid = %lx, err = %d",
		pmsg->cmd_id, pmsg->param_type, pmsg->seq_id, pmsg->error);

	//there's competence between daemon thread here, but actual operation is in ctx, so
	//no lock is used here
	if (pmsg->seq_id == NO_RESPONSE_SEQID)
		vsi_handle_daemonmsg(vpu, pmsg);
	if (msgsize > 0)
		kfree(pmsg);

	ret = size;

error:
	return ret;
}

static int v4l2_daemon_open(struct inode *inode, struct file *filp)
{
	/*we need single daemon. Each deamon uses 2 handles for ioctl and mmap*/
	struct vsi_v4l2_device *vpu;
	struct daemon_data *priv_data;

	vpu = container_of(filp->f_op, struct vsi_v4l2_device, fop);
	if (vpu->magic != VSI_VDEV_MAGIC)
		return -ENODEV;			//simple verification

	priv_data = kzalloc(sizeof(*priv_data), GFP_KERNEL);
	if (!priv_data)
		return -ENOMEM;

	spin_lock_init(&priv_data->list_spinlock);
	INIT_LIST_HEAD(&priv_data->daemon_list);
	priv_data->vpu = vpu;

	filp->private_data = priv_data;
	v4l2_klog(LOGLVL_INFO, "%d:%d", vpu->deviceid, atomic_read(&vpu->daemon_fn));
	if (atomic_read(&vpu->daemon_fn) >= 1)
		return -EBUSY;
	atomic_inc(&vpu->daemon_fn);
	wake_up_interruptible_all(&vpu->instance_queue);
	return 0;
}

static int v4l2_daemon_release(struct inode *inode, struct file *filp)
{
	struct daemon_data *priv_data;
	struct vsi_v4l2_device *vpu;
	struct daemon_timer_node *daemon_node, *tmp = NULL;

	priv_data = filp->private_data;
	if (!priv_data)
		return -EINVAL;

	vpu = priv_data->vpu;
	spin_lock(&priv_data->list_spinlock);
	list_for_each_entry_safe(daemon_node, tmp, &priv_data->daemon_list, list) {
		v4l2_klog(LOGLVL_WARNING, "free daemon_node inst_id(%#llx)\n", daemon_node->inst_id);
		list_del_init(&daemon_node->list);
		del_timer_sync(&daemon_node->timer);
		kfree(daemon_node);
	}
	spin_unlock(&priv_data->list_spinlock);

	kfree(priv_data);
	filp->private_data = NULL;
	atomic_dec(&vpu->daemon_fn);
	v4l2_klog(LOGLVL_INFO, "%d:%d", vpu->deviceid, atomic_read(&vpu->daemon_fn));
	if (atomic_read(&vpu->daemon_fn) <= 0) {
		vsiv4l2_cleanupdaemonmsg(vpu);
		wakeup_ctxqueues(vpu);
		wake_up_interruptible_all(&vpu->instance_queue);
	}
	return 0;
}

static int vsi_v4l2_mmap(
	struct file *filp,
	struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	phys_addr_t offset = (phys_addr_t)vma->vm_pgoff << PAGE_SHIFT;

	/* Does it even fit in phys_addr_t? */
	if (offset >> PAGE_SHIFT != vma->vm_pgoff)
		return -EINVAL;

	/* It's illegal to wrap around the end of the physical address space. */
	if (offset + (phys_addr_t)size - 1 < offset)
		return -EINVAL;

	//if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
	//	return -EINVAL;

	if (!(vma->vm_flags & VM_MAYSHARE))
		return -EPERM;

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	return remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot) ? -EAGAIN : 0;
}

int vsiv4l2_initdaemon(struct vsi_v4l2_device *vpu)
{
	int result;
	u32 devid = vpu->deviceid;
	char daemondevname[sizeof(PIPE_DEVICE_NAME) + 3] = {0};
	atomic_t daemon_fn = ATOMIC_INIT(0);

	init_waitqueue_head(&vpu->cmd_queue);
	init_waitqueue_head(&vpu->instance_queue);

	vpu->fop.owner = THIS_MODULE;
	vpu->fop.open = v4l2_daemon_open;
	vpu->fop.release = v4l2_daemon_release;
	vpu->fop.unlocked_ioctl = vsi_v4l2_daemon_ioctl;
	vpu->fop.read = v4l2_msg_read;
	vpu->fop.write = v4l2_msg_write;
	vpu->fop.mmap = vsi_v4l2_mmap;
	memcpy(daemondevname, PIPE_DEVICE_NAME, sizeof(PIPE_DEVICE_NAME));
	if (devid < 10) {
		daemondevname[sizeof(PIPE_DEVICE_NAME) - 1] = devid + 0x30;
		daemondevname[sizeof(PIPE_DEVICE_NAME)] = 0;
	} else {
		daemondevname[sizeof(PIPE_DEVICE_NAME) - 1] = devid / 10 + 0x30;
		daemondevname[sizeof(PIPE_DEVICE_NAME)] = devid % 10 + 0x30;
		daemondevname[sizeof(PIPE_DEVICE_NAME) + 1] = 0;
	}

	result = register_chrdev(VSI_DAEMON_DEVMAJOR + devid, daemondevname, (const struct file_operations  *)&vpu->fop);
	if (result < 0) {
		v4l2_klog(LOGLVL_ERROR, "fail to create daemon handle %d:%s", vpu->deviceid, daemondevname);
		return result;
	}
	mutex_init(&vpu->cmd_lock);
	mutex_init(&vpu->instance_lock);
	INIT_LIST_HEAD(&vpu->listhead);
	vpu->daemon_fn = daemon_fn;
	vpu->g_seqid = 1;
	vpu->magic = VSI_VDEV_MAGIC;
	loglevel = clamp(loglevel, 0, 10);

	return 0;
}

