/*
 *    VSI V4L2 encoder entry.
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
#include <linux/version.h>
#include "vsi-v4l2-priv.h"

#define THRESHOLD_MS                        (400ULL)

static int vsi_enc_querycap(
	struct file *filp,
	void *priv,
	struct v4l2_capability *cap)
{
	struct vsi_v4l2_dev_info *hwinfo;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	u32 high;
	u32 low;

	v4l2_klog(LOGLVL_DEBUG, "in\n");
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, querycap error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	high = (u32)((ctx->ctxid >> 32) & 0xFFFFFFFF);
	low = (u32)(ctx->ctxid & 0xFFFFFFFF);
	hwinfo = vsiv4l2_get_hwinfo();
	if (hwinfo->enc_corenum == 0)
		return -ENODEV;

	strlcpy(cap->driver, "vsi_v4l2", sizeof(cap->driver));
	strlcpy(cap->card, "vsi_v4l2enc", sizeof(cap->card));
	strlcpy(cap->bus_info, "platform:vsi_v4l2enc", sizeof(cap->bus_info));
	cap->reserved[0] = high;
	cap->reserved[1] = low;

	cap->device_caps = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_META_OUTPUT;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

static int vsi_enc_reqbufs(
	struct file *filp,
	void *priv,
	struct v4l2_requestbuffers *p)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	int ret;
	struct vb2_queue *q;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctxid=%d:%#lx, type=%u daemon is die.\n",
				ctx->dev->deviceid, ctx->ctxid, p->type);
		return -ENODEV;
	};
	if (!isvalidtype(p->type, ctx->flag))
		return -EINVAL;

	if (ismetabuf(p->type)) {
		q = &ctx->meta_que;
		if (p->memory == V4L2_MEMORY_MMAP)
			return -EPERM;
		if (p->count != 0)
			p->count = 1;
	} else if (binputqueue(p->type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;

	ret = vb2_reqbufs(q, p);
	if (!binputqueue(p->type) && p->count == 0)
		set_bit(CTX_FLAG_ENC_FLUSHBUF, &ctx->flag);
	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%d ask for %d buffer, got %d:%d:%d",
		ctx->dev->deviceid, ctx->ctxid, p->type, p->count, q->num_buffers, ret, ctx->status);
	return ret;
}

static int vsi_enc_create_bufs(struct file *filp, void *priv,
				struct v4l2_create_buffers *create)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	int ret;
	struct vb2_queue *q;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx, enc_create_bufs error, daemon is die\n",
			ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(create->format.type, ctx->flag))
		return -EINVAL;

	if (ismetabuf(create->format.type))
		q = &ctx->meta_que;
	else if (binputqueue(create->format.type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;
	ret = vb2_create_bufs(q, create);

	if (!binputqueue(create->format.type) && create->count == 0)
		set_bit(CTX_FLAG_ENC_FLUSHBUF, &ctx->flag);
	v4l2_klog(LOGLVL_INFO, "%d:%lx:%d create for %d buffer, got %d:%d:%d\n",
		ctx->dev->deviceid, ctx->ctxid, create->format.type, create->count,
		q->num_buffers, ret, ctx->status);
	return ret;
}

static int vsi_enc_s_parm(struct file *filp, void *priv, struct v4l2_streamparm *parm)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);

	v4l2_klog(LOGLVL_DEBUG, "in\n");
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_s_parm error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(parm->type, ctx->flag))
		return -EINVAL;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%lx: mutex_lock_killable return from signal\n",
			ctx->ctxid);
		return -EBUSY;
	}
	if (binputqueue(parm->type)) {
		memset(parm->parm.output.reserved, 0, sizeof(parm->parm.output.reserved));
		if (!parm->parm.output.timeperframe.denominator)
			parm->parm.output.timeperframe.denominator = 25;		//default val
		if (!parm->parm.output.timeperframe.numerator)
			parm->parm.output.timeperframe.numerator = 1;			//default val
		if (ctx->mediacfg.m_encparams.m_framerate.inputRateDenom != parm->parm.output.timeperframe.numerator ||
			ctx->mediacfg.m_encparams.m_framerate.inputRateNumer != parm->parm.output.timeperframe.denominator) {
			ctx->mediacfg.m_encparams.m_framerate.inputRateDenom = parm->parm.output.timeperframe.numerator;
			ctx->mediacfg.m_encparams.m_framerate.inputRateNumer = parm->parm.output.timeperframe.denominator;
			flag_updateparam(m_framerate)
		}
		//according to spec, capture can use output's rate set if not set
		if (ctx->mediacfg.bcapturerateset == 0) {
			if (ctx->mediacfg.m_encparams.m_framerate.outputRateDenom != ctx->mediacfg.m_encparams.m_framerate.inputRateDenom ||
				ctx->mediacfg.m_encparams.m_framerate.outputRateNumer != ctx->mediacfg.m_encparams.m_framerate.inputRateNumer) {
				ctx->mediacfg.m_encparams.m_framerate.outputRateDenom = ctx->mediacfg.m_encparams.m_framerate.inputRateDenom;
				ctx->mediacfg.m_encparams.m_framerate.outputRateNumer = ctx->mediacfg.m_encparams.m_framerate.inputRateNumer;
				flag_updateparam(m_framerate)
			}
		}
		parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
	} else {
		memset(parm->parm.capture.reserved, 0, sizeof(parm->parm.capture.reserved));
		if (!parm->parm.capture.timeperframe.denominator)
			parm->parm.capture.timeperframe.denominator = 25;
		if (!parm->parm.capture.timeperframe.numerator)
			parm->parm.capture.timeperframe.numerator = 1;
		if (ctx->mediacfg.m_encparams.m_framerate.outputRateDenom != parm->parm.capture.timeperframe.numerator ||
			ctx->mediacfg.m_encparams.m_framerate.outputRateNumer != parm->parm.capture.timeperframe.denominator) {
			ctx->mediacfg.m_encparams.m_framerate.outputRateDenom = parm->parm.capture.timeperframe.numerator;
			ctx->mediacfg.m_encparams.m_framerate.outputRateNumer = parm->parm.capture.timeperframe.denominator;
			flag_updateparam(m_framerate)
		}
		ctx->mediacfg.bcapturerateset = 1;
		parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	}
	mutex_unlock(&ctx->ctxlock);
	return 0;
}

static int vsi_enc_g_parm(struct file *filp, void *priv, struct v4l2_streamparm *parm)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct v4l2_enccfg_framerate *pfr = &ctx->mediacfg.m_encparams.m_framerate;

	v4l2_klog(LOGLVL_DEBUG, "in\n");
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_g_parm error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}

	if (!isvalidtype(parm->type, ctx->flag))
		return -EINVAL;
	if (binputqueue(parm->type)) {
		memset(parm->parm.output.reserved, 0, sizeof(parm->parm.output.reserved));
		parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.output.timeperframe.denominator = pfr->inputRateNumer;
		parm->parm.output.timeperframe.numerator = pfr->inputRateDenom;
	} else {
		memset(parm->parm.capture.reserved, 0, sizeof(parm->parm.capture.reserved));
		parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.capture.timeperframe.denominator = pfr->outputRateNumer;
		parm->parm.capture.timeperframe.numerator = pfr->inputRateDenom;
	}
	return 0;
}

static int vsi_enc_g_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	v4l2_klog(LOGLVL_DEBUG, "%d", f->type);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	v4l2_klog(LOGLVL_DEBUG, "%lx:%d", ctx->ctxid, f->type);
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_g_fmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}

	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(f->type))
		return -EINVAL;
	return vsiv4l2_getfmt(ctx, f);
}

static int vsi_enc_s_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	int ret;

	v4l2_klog(LOGLVL_INFO, "%d:%d:%x",
		f->fmt.pix_mp.width, f->fmt.pix_mp.height, f->fmt.pix_mp.pixelformat);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_s_fmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(f->type))
		return -EINVAL;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	ret = vsiv4l2_setfmt(ctx, f);
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_enc_querybuf(
	struct file *filp,
	void *priv,
	struct v4l2_buffer *buf)
{
	u32 i;
	int ret;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct vb2_queue *q;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_querybuf, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(buf->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(buf->type))
		q = &ctx->meta_que;
	else if (binputqueue(buf->type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;
	v4l2_klog(LOGLVL_DEBUG, "%lx:%d:%d", ctx->flag, buf->type, buf->index);
	ret = vb2_querybuf(q, buf);
	if (ret == 0 && buf->memory == V4L2_MEMORY_MMAP) {
		if (binputqueue(buf->type)) {
			for (i = 0; i < min(ctx->mediacfg.srcplanes, buf->length); i++) {
				ctx->inputbufoffset[buf->index][i] = buf->m.planes[i].m.mem_offset;
				buf->m.planes[i].m.mem_offset = i | (buf->index << 2) | (0 << 7);
				buf->m.planes[i].m.mem_offset <<= PAGE_SHIFT;
			}
		} else if (ismetabuf(buf->type)) {
			ctx->metabufoffset[0] = buf->m.offset;
			buf->m.offset = (2 << 7);
			buf->m.offset <<= PAGE_SHIFT;
		} else {
			for (i = 0; i < min(ctx->mediacfg.dstplanes, buf->length); i++) {
				ctx->outputbufoffset[buf->index][i] = buf->m.planes[i].m.mem_offset;
				buf->m.planes[i].m.mem_offset = i | (buf->index << 2) | (1 << 7);
				buf->m.planes[i].m.mem_offset <<= PAGE_SHIFT;
			}
		}
	}

	return ret;
}

static int vsi_enc_trystartenc(struct vsi_v4l2_ctx *ctx)
{
	int ret = 0;

	v4l2_klog(LOGLVL_DEBUG, "streaming:%d:%d:%d, queued buf:%d:%d",
		ctx->status, ctx->input_que.streaming, ctx->output_que.streaming,
		ctx->input_que.queued_count, ctx->output_que.queued_count);
	if (vb2_is_streaming(&ctx->input_que) && vb2_is_streaming(&ctx->output_que)) {
		vb2_clear_last_buffer_dequeued(&ctx->output_que);
		if ((ctx->status == VSI_STATUS_INIT ||
			ctx->status == ENC_STATUS_STOPPED ||
			ctx->status == ENC_STATUS_EOS) &&
			ctx->input_que.queued_count >= ctx->input_que.min_buffers_needed &&
			ctx->output_que.queued_count >= ctx->output_que.min_buffers_needed) {
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMON, NULL);
			if (ret == 0) {
				ctx->status = ENC_STATUS_ENCODING;
				if (test_and_clear_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag)) {
					ret |= vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_CMD_STOP, NULL);
					ctx->status = ENC_STATUS_DRAINING;
				}
			}
		}
	}
	return ret;
}

static int vsi_enc_qbuf(struct file *filp, void *priv, struct v4l2_buffer *buf)
{
	int ret;
	int i;
	//struct vb2_queue *vq = vb->vb2_queue;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct video_device *vdev;
	struct v4l2_plane *plane;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	/* fuzz test report */
	if (!buf->length) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx, buf->length=%d\n", ctx->ctxid, buf->length);
		return -EINVAL;
	}

	vdev = ctx->dev->venc;

	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%d:%d:%d", ctx->dev->deviceid, ctx->ctxid, buf->type, buf->index, buf->length);
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
			v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx, enc_qbuf error, buf->type=%d, index=%d daemon is die\n",
			ctx->ctxid, buf->type, buf->index);
		return -ENODEV;
	}

	if (!isvalidtype(buf->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(buf->type)) {
		if (vb2_is_streaming(&ctx->input_que))
			return -EBUSY;
		//ctx->mediacfg.metabufinfo.busMetaBuf is left to bufqueue
		ctx->mediacfg.metabufinfo.metabufsize = buf->length;
		ret = vb2_qbuf(&ctx->meta_que, vdev->v4l2_dev->mdev, buf);
		if (ret == 0)
			ret = vb2_streamon(&ctx->meta_que, buf->type);
		else
			v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx vb2_qbuf error, ret=%d. type=%d index=%d byteuse=%d\n",
				ctx->ctxid, ret, buf->type, buf->index, buf->bytesused);

		return ret;
	}
	//ignore input buf in spec's STOP state
	if (binputqueue(buf->type) &&
		(ctx->status == ENC_STATUS_STOPPED) &&
		!vb2_is_streaming(&ctx->input_que))
		return 0;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}

	if (!binputqueue(buf->type))
		ret = vb2_qbuf(&ctx->output_que, vdev->v4l2_dev->mdev, buf);
	else {
		if (test_and_clear_bit(CTX_FLAG_FORCEIDR_BIT, &ctx->flag))
			ctx->srcvbufflag[buf->index] |= FORCE_IDR;

		ret = vb2_qbuf(&ctx->input_que, vdev->v4l2_dev->mdev, buf);
	}

	if (ret != 0) {
		if (V4L2_TYPE_IS_MULTIPLANAR(buf->type) && buf->m.planes) {
			for (i = 0; i < buf->length; i++) {
				plane = &buf->m.planes[i];
				v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx vb2_qbuf error, ret=%d. type=%d index=%d plane[%d]:fd=%d byteused=%d\n",
					ctx->ctxid, ret, buf->type, buf->index, i, plane->m.fd, plane->bytesused);
			}
		}  else
			v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx vb2_qbuf error, ret=%d. type=%d index=%d fd=%d byteused=%d\n",
				ctx->ctxid, ret, buf->type, buf->index, buf->m.fd, buf->bytesused);
		goto qbuf_exit;
	}

	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%d:%d:%d:%d, %d:%d, %d:%d",
		ctx->dev->deviceid, ctx->ctxid, ctx->status, buf->type, buf->index, buf->bytesused,
		buf->m.planes[0].bytesused, buf->m.planes[0].length,
		buf->m.planes[1].bytesused, buf->m.planes[1].length);

	if (ret == 0 && ctx->status != ENC_STATUS_ENCODING && ctx->status != ENC_STATUS_EOS)
		ret = vsi_enc_trystartenc(ctx);
qbuf_exit:
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_enc_streamon(struct file *filp, void *priv, enum v4l2_buf_type type)
{
	int ret = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);

	v4l2_klog(LOGLVL_INFO, "%d:%d", type, ctx->status);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_stream_on error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(type, ctx->flag))
		return -EINVAL;
	if (ctx->status == ENC_STATUS_ENCODING)
		return 0;
	if (ismetabuf(type))
		return 0;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	if (!binputqueue(type)) {
		ret = vb2_streamon(&ctx->output_que, type);
		printbufinfo(&ctx->output_que);
	} else {
		ret = vb2_streamon(&ctx->input_que, type);
		printbufinfo(&ctx->input_que);
	}

	if (ret == 0) {
		if (ctx->status == ENC_STATUS_EOS) {
			//to avoid no queued buf when streamon
			ctx->status = ENC_STATUS_STOPPED;
		}
		ret = vsi_enc_trystartenc(ctx);
	}

	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_enc_streamoff(
	struct file *file,
	void *priv,
	enum v4l2_buf_type type)
{
	int i, ret;
	int binput = binputqueue(type);
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(priv);
	struct vb2_queue *q;
	ktime_t start, end, input_start, output_start, input_end, output_end;
	u64 ms, input_ms, output_ms;

	start = ktime_get();
	v4l2_klog(LOGLVL_INFO, "%d:%d", type, ctx->status);
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_stream_off error, type=%d, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid, type);
		return -ENODEV;
	}
	if (!isvalidtype(type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(type))
		return 0;

	if (binputqueue(type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;

	if (!vb2_is_streaming(q))
		return 0;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	if (binput)
		vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMOFF_OUTPUT, &binput);
	else
		vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMOFF_CAPTURE, &binput);
	mutex_unlock(&ctx->ctxlock);
	if (binput) {
		input_start = ktime_get();
		ret = wait_event_interruptible_timeout(ctx->capoffdone_queue, vsi_checkctx_outputoffdone(ctx),
										msecs_to_jiffies(XRING_STREAM_OFF_TIMEOUT));
		if (!ret) {
			v4l2_klog(LOGLVL_WARNING, "%lx wait input buffer return by timeout.\n", ctx->ctxid);
		} else if (ret == -ERESTARTSYS)
			v4l2_klog(LOGLVL_WARNING, "%lx wait input buffer return by signal.\n", ctx->ctxid);

		input_end = ktime_get();
		input_ms = ktime_to_ms(ktime_sub(input_end, input_start));
		if (input_ms > THRESHOLD_MS)
			v4l2_klog(LOGLVL_WARNING, "===> %lx input_ms=%llu\n", ctx->ctxid, input_ms);
	} else {
		output_start = ktime_get();
		ret = wait_event_interruptible_timeout(ctx->capoffdone_queue, vsi_checkctx_capoffdone(ctx),
										msecs_to_jiffies(XRING_STREAM_OFF_TIMEOUT));
		if (!ret) {
			v4l2_klog(LOGLVL_WARNING, "%lx wait output buffer return by timeout.\n", ctx->ctxid);
		} else if (ret == -ERESTARTSYS)
			v4l2_klog(LOGLVL_WARNING, "%lx wait output buffer return by signal.\n", ctx->ctxid);

		output_end = ktime_get();
		output_ms = ktime_to_ms(ktime_sub(output_end, output_start));
		if (output_ms > THRESHOLD_MS)
			v4l2_klog(LOGLVL_WARNING, "===> %lx output_ms=%llu\n", ctx->ctxid, output_ms);
	}
	if (ret <= 0)
		v4l2_klog(LOGLVL_WARNING, "%d:%lx binput:%d, enc wait strmoff done fail\n",
			ctx->dev->deviceid, ctx->ctxid, binput);

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	if (ctx->status != VSI_STATUS_INIT)
		ctx->status = ENC_STATUS_STOPPED;

	if (binput) {
		clear_bit(CTX_FLAG_FORCEIDR_BIT, &ctx->flag);
		for (i = 0; i < VIDEO_MAX_FRAME; i++)
			ctx->srcvbufflag[i] = 0;
	}

	return_all_buffers(q, VB2_BUF_STATE_DONE, 1);
	ret = vb2_streamoff(q, type);
	mutex_unlock(&ctx->ctxlock);

	end = ktime_get();
	ms = ktime_to_ms(ktime_sub(end, start));
	if (ms > THRESHOLD_MS)
		v4l2_klog(LOGLVL_WARNING, "===>%s %lx ms=%llu\n",
			binputqueue(type)?"input":"output", ctx->ctxid, ms);

	return ret;
}

static int vsi_enc_dqbuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	int ret = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vb2_queue *q;
	struct vb2_buffer *vb;
	struct vsi_vpu_buf *vsibuf;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_dqbuf error, type=%d, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid, p->type);
		return -ENODEV;
	}
	if (!isvalidtype(p->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(p->type)) {
		if (vb2_is_streaming(&ctx->input_que))
			return -EBUSY;
		q = &ctx->meta_que;
		/* fuzz test report */
		if (!q->bufs[0])
			return -EINVAL;
		vb2_buffer_done(q->bufs[0], VB2_BUF_STATE_DONE);
		ret = vb2_dqbuf(q, p, file->f_flags & O_NONBLOCK);
		if (ret == 0)
			vb2_streamoff(q, p->type);
		ctx->mediacfg.metabufinfo.busMetaBuf = 0;
		if (test_and_clear_bit(CTX_FLAG_MAPMETA, &ctx->flag))
			iounmap(ctx->pmeta);
		ctx->pmeta = NULL;
		return ret;
	}
	if (binputqueue(p->type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;

	if (ctx->status == ENC_STATUS_STOPPED ||
		ctx->status == ENC_STATUS_EOS) {
		p->bytesused = 0;
		return -EPIPE;
	}

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	ret = vb2_dqbuf(q, p, file->f_flags & O_NONBLOCK);
	if (ret == 0) {
		vb = q->bufs[p->index];
		vsibuf = vb_to_vsibuf(vb);
		list_del(&vsibuf->list);
		p->flags &= ~(V4L2_BUF_FLAG_KEYFRAME | V4L2_BUF_FLAG_PFRAME | V4L2_BUF_FLAG_BFRAME | V4L2_BUF_FLAG_ERROR);
		if (!binputqueue(p->type)) {
			if (ctx->vbufflag[p->index] & FRAME_ERROR)
				p->flags |= V4L2_BUF_FLAG_ERROR;
			else {
				if (ctx->vbufflag[p->index] & FRAMETYPE_I)
					p->flags |= V4L2_BUF_FLAG_KEYFRAME;
				else  if (ctx->vbufflag[p->index] & FRAMETYPE_P)
					p->flags |= V4L2_BUF_FLAG_PFRAME;
				else  if (ctx->vbufflag[p->index] & FRAMETYPE_B)
					p->flags |= V4L2_BUF_FLAG_BFRAME;
			}
		}
	}
	if (!binputqueue(p->type)) {
		if (ret == 0) {
			if (ctx->vbufflag[p->index] & LAST_BUFFER_FLAG) {
				vsi_v4l2_sendeos(ctx);
				if (ctx->status == ENC_STATUS_DRAINING)
					ctx->status = ENC_STATUS_EOS;
				v4l2_klog(LOGLVL_INFO, "dqbuf get eos flag");
			}
		}
	}
	mutex_unlock(&ctx->ctxlock);
	v4l2_klog(LOGLVL_DEBUG, "%d:%d:%d:%x:%d", p->type, p->index, ret, p->flags, ctx->status);
	return ret;
}

static int vsi_enc_prepare_buf(
	struct file *file,
	void *priv,
	struct v4l2_buffer *p)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vb2_queue *q;
	struct video_device *vdev;

	v4l2_klog(LOGLVL_DEBUG, "%d", p->type);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	vdev = ctx->dev->venc;

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_prepare_buf error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(p->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(p->type))
		q = &ctx->meta_que;
	else if (binputqueue(p->type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;
	return vb2_prepare_buf(q, vdev->v4l2_dev->mdev, p);
}

static int vsi_enc_expbuf(
	struct file *file,
	void *priv,
	struct v4l2_exportbuffer *p)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vb2_queue *q;

	v4l2_klog(LOGLVL_DEBUG, "%d", p->type);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_exbuf error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(p->type, ctx->flag))
		return -EINVAL;

	if (ismetabuf(p->type))
		q = &ctx->meta_que;
	else if (binputqueue(p->type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;
	return vb2_expbuf(q, p);
}

static int vsi_enc_try_fmt(struct file *file, void *prv, struct v4l2_format *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	v4l2_klog(LOGLVL_INFO, "%d", f->type);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_tryfmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (ismetabuf(f->type))
		return -EINVAL;
	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;
	if (vsi_find_format(ctx, f) == NULL)
		return -EINVAL;

	return 0;
}

static int vsi_enc_enum_fmt(struct file *file, void *prv, struct v4l2_fmtdesc *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vsi_video_fmt *pfmt;
	int braw;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	braw = brawfmt(ctx->flag, f->type);

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_enum_fmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (ismetabuf(f->type))
		return -EINVAL;
	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;

	pfmt = vsi_enum_encformat(f->index, braw);
	if (pfmt == NULL)
		return -EINVAL;

	if (pfmt->name && strlen(pfmt->name))
		strlcpy(f->description, pfmt->name, sizeof(f->description));
	f->pixelformat = pfmt->fourcc;
	f->flags = pfmt->flag;
	v4l2_klog(LOGLVL_DEBUG, "%d:%d:%x", f->index, f->type, pfmt->fourcc);
	return 0;
}

static int vsi_enc_tryfmt_meta(struct file *file, void *fh, struct v4l2_format *f)
{
	if (f->type != V4L2_BUF_TYPE_META_OUTPUT ||
		f->fmt.meta.dataformat != V4L2_META_FMT_VSIENC)
		return -EINVAL;
	return 0;
}

static int vsi_enc_setfmt_meta(struct file *file, void *fh, struct v4l2_format *f)
{
	int ret;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	ret = vsi_enc_tryfmt_meta(file, fh, f);
	if (ret)
		return ret;
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_setfmt_meta error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}

	f->fmt.meta.dataformat = V4L2_META_FMT_VSIENC;
	f->fmt.meta.buffersize = ctx->input_que.num_buffers * sizeof(struct v4l2_vsi_enc_meta_info_OUTPUT) +
									ctx->output_que.num_buffers * sizeof(struct v4l2_vsi_enc_meta_info_CAPTURE) +
									sizeof(struct v4l2_vsi_stream_meta_info_enc);
	return 0;
}

static int vsi_enc_enumfmt_meta(struct file *file, void *fh, struct v4l2_fmtdesc *f)
{
	static const char *encmeta_name = "enc_meta";
	if (f->index > 0 || f->type != V4L2_BUF_TYPE_META_OUTPUT)
		return -EINVAL;
	f->flags = 0;
	f->pixelformat = V4L2_META_FMT_VSIENC;
	strlcpy(f->description, encmeta_name, sizeof(f->description));
	return 0;
}

static int vsi_enc_set_selection(struct file *file, void *prv, struct v4l2_selection *s)
{
	int ret = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vsi_v4l2_encparams *pcfg = &ctx->mediacfg.m_encparams;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_set_selection error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (s->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;
	if (s->target != V4L2_SEL_TGT_CROP)
		return -EINVAL;
	ret = vsiv4l2_verifycrop(ctx, s);
	if (!ret) {
		if (mutex_lock_killable(&ctx->ctxlock)) {
			v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
				ctx->dev->deviceid, ctx->ctxid);
			return -EBUSY;
		}
		pcfg->m_cropinfo.horOffsetSrc = s->r.left;
		pcfg->m_cropinfo.verOffsetSrc = s->r.top;
		pcfg->m_cropinfo.width = s->r.width;
		pcfg->m_cropinfo.height = s->r.height;
		flag_updateparam(m_cropinfo)
		mutex_unlock(&ctx->ctxlock);
	}
	v4l2_klog(LOGLVL_DEBUG, "%lx:%d,%d,%d,%d",
		ctx->ctxid, s->r.left, s->r.top, s->r.width, s->r.height);

	return ret;
}

static int vsi_enc_get_selection(struct file *file, void *prv, struct v4l2_selection *s)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vsi_v4l2_encparams *pcfg = &ctx->mediacfg.m_encparams;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_get_selection error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (s->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;
	switch (s->target) {
	case V4L2_SEL_TGT_CROP:
		s->r.left = pcfg->m_cropinfo.horOffsetSrc;
		s->r.top = pcfg->m_cropinfo.verOffsetSrc;
		s->r.width = pcfg->m_cropinfo.width;
		s->r.height = pcfg->m_cropinfo.height;
		break;
	case V4L2_SEL_TGT_CROP_DEFAULT:
	case V4L2_SEL_TGT_CROP_BOUNDS:
		s->r.left = 0;
		s->r.top = 0;
		s->r.width = pcfg->m_srcinfo.lumWidthSrc;
		s->r.height = pcfg->m_srcinfo.lumHeightSrc;
		break;
	default:
		return -EINVAL;
	}
	v4l2_klog(LOGLVL_INFO, "%lx:%d,%d,%d,%d",
		ctx->ctxid, s->r.left, s->r.top, s->r.width, s->r.height);

	return 0;
}

static int vsi_enc_subscribe_event(
	struct v4l2_fh *fh,
	const struct v4l2_event_subscription *sub)
{
	v4l2_klog(LOGLVL_DEBUG, "%d", sub->type);
	switch (sub->type) {
	case V4L2_EVENT_CTRL:
		return v4l2_ctrl_subscribe_event(fh, sub);
	case V4L2_EVENT_SKIP:
		return v4l2_event_subscribe(fh, sub, 16, NULL);
	case V4L2_EVENT_EOS:
	case V4L2_EVENT_CODEC_ERROR:
	case V4L2_EVENT_INVALID_OPTION:
		return v4l2_event_subscribe(fh, sub, 0, NULL);
	default:
//by spec should return error, but current gst will fail, so leave it here
		return 0;//-EINVAL;
	}
}

static int vsi_enc_try_encoder_cmd(struct file *file, void *fh, struct v4l2_encoder_cmd *cmd)
{
	switch (cmd->cmd) {
	case V4L2_ENC_CMD_STOP:
	case V4L2_ENC_CMD_START:
	case V4L2_ENC_CMD_PAUSE:
	case V4L2_ENC_CMD_RESUME:
		cmd->flags = 0;
		return 0;
	default:
		return -EINVAL;
	}
}

static int vsi_enc_encoder_cmd(struct file *file, void *fh, struct v4l2_encoder_cmd *cmd)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	int ret = 0;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_encoder_cmd error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	v4l2_klog(LOGLVL_INFO, "%d:%d", ctx->status, cmd->cmd);
	switch (cmd->cmd) {
	case V4L2_ENC_CMD_STOP:
		set_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag);
		if (ctx->status == ENC_STATUS_ENCODING) {
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_CMD_STOP, cmd);
			if (ret == 0) {
				ctx->status = ENC_STATUS_DRAINING;
				clear_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag);
			}
		}
		break;
	case V4L2_ENC_CMD_START:
		if (ctx->status == ENC_STATUS_STOPPED ||
			ctx->status == ENC_STATUS_EOS) {
			vb2_streamon(&ctx->input_que, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
			vb2_streamon(&ctx->output_que, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
			ret = vsi_enc_trystartenc(ctx);
		}
		break;
	case V4L2_ENC_CMD_PAUSE:
	case V4L2_ENC_CMD_RESUME:
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_enc_encoder_enum_framesizes(struct file *file, void *priv,
				  struct v4l2_frmsizeenum *fsize)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct v4l2_format fmt;

	v4l2_klog(LOGLVL_DEBUG, "%x", fsize->pixel_format);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_encoder_enum_framesizes error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (fsize->index != 0)		//only stepwise
		return -EINVAL;

	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	fmt.fmt.pix_mp.pixelformat = fsize->pixel_format;
	if (vsi_find_format(ctx,  &fmt) != NULL)
		vsi_enum_encfsize(fsize, ctx->mediacfg.outfmt_fourcc);
	else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix_mp.pixelformat = fsize->pixel_format;
		if (vsi_find_format(ctx,  &fmt) == NULL)
			return -EINVAL;
		vsi_enum_encfsize(fsize, fsize->pixel_format);
	}

	return 0;
}

/* ioctl handler */
/* take VIDIOC_S_INPUT for example, ioctl goes to V4l2-ioctl.c.: v4l_s_input() -> V4l2-dev.c: v4l2_ioctl_ops.vidioc_s_input() */
/* ioctl cmd could be disabled by v4l2_disable_ioctl() */
static const struct v4l2_ioctl_ops vsi_enc_ioctl = {
	.vidioc_querycap = vsi_enc_querycap,
	.vidioc_reqbufs             = vsi_enc_reqbufs,
	.vidioc_create_bufs         = vsi_enc_create_bufs,
	.vidioc_prepare_buf         = vsi_enc_prepare_buf,
	//create_buf can be provided now since we don't know buf type in param
	.vidioc_querybuf            = vsi_enc_querybuf,
	.vidioc_qbuf                = vsi_enc_qbuf,
	.vidioc_dqbuf               = vsi_enc_dqbuf,
	.vidioc_streamon        = vsi_enc_streamon,
	.vidioc_streamoff       = vsi_enc_streamoff,
	.vidioc_s_parm		= vsi_enc_s_parm,
	.vidioc_g_parm		= vsi_enc_g_parm,
	//.vidioc_g_fmt_vid_cap = vsi_enc_g_fmt,
	.vidioc_g_fmt_vid_cap_mplane = vsi_enc_g_fmt,
	//.vidioc_s_fmt_vid_cap = vsi_enc_s_fmt,
	.vidioc_s_fmt_vid_cap_mplane = vsi_enc_s_fmt,
	.vidioc_expbuf = vsi_enc_expbuf,		//this is used to export MMAP ptr as prime fd to user space app

	//.vidioc_g_fmt_vid_out = vsi_enc_g_fmt,
	.vidioc_g_fmt_vid_out_mplane = vsi_enc_g_fmt,
	//.vidioc_s_fmt_vid_out = vsi_enc_s_fmt,
	.vidioc_s_fmt_vid_out_mplane = vsi_enc_s_fmt,
	//.vidioc_try_fmt_vid_cap = vsi_enc_try_fmt,
	.vidioc_try_fmt_vid_cap_mplane = vsi_enc_try_fmt,
	//.vidioc_try_fmt_vid_out = vsi_enc_try_fmt,
	.vidioc_try_fmt_vid_out_mplane = vsi_enc_try_fmt,
	.vidioc_enum_fmt_vid_cap = vsi_enc_enum_fmt,
	.vidioc_enum_fmt_vid_out = vsi_enc_enum_fmt,

	//.vidioc_g_fmt_vid_out = vsi_enc_g_fmt,
	.vidioc_g_fmt_vid_out_mplane = vsi_enc_g_fmt,

	.vidioc_s_selection = vsi_enc_set_selection,		//VIDIOC_S_SELECTION, VIDIOC_S_CROP
	.vidioc_g_selection = vsi_enc_get_selection,		//VIDIOC_G_SELECTION, VIDIOC_G_CROP

	.vidioc_subscribe_event = vsi_enc_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
	.vidioc_try_encoder_cmd = vsi_enc_try_encoder_cmd,

	//fixme: encoder cmd stop will make streamoff not coming from ffmpeg. Maybe this is the right way to get finished, check later
	.vidioc_encoder_cmd = vsi_enc_encoder_cmd,
	.vidioc_enum_framesizes = vsi_enc_encoder_enum_framesizes,

	.vidioc_try_fmt_meta_out = vsi_enc_tryfmt_meta,
	.vidioc_s_fmt_meta_out = vsi_enc_setfmt_meta,
	.vidioc_g_fmt_meta_out = vsi_enc_setfmt_meta,	//meta set/get are identical since it's unchangeable
	.vidioc_enum_fmt_meta_out = vsi_enc_enumfmt_meta,
};

/*setup buffer information before real allocation*/
static int vsi_enc_queue_setup(
	struct vb2_queue *vq,
	unsigned int *nbuffers,
	unsigned int *nplanes,
	unsigned int sizes[],
	struct device *alloc_devs[])
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(vq->drv_priv);
	int i, ret;

	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%d,%d,%d", ctx->dev->deviceid, ctx->ctxid, *nbuffers, *nplanes, sizes[0]);
	ret = vsiv4l2_buffer_config(ctx, vq->type, nbuffers, nplanes, sizes);
	if (ret == 0) {
		for (i = 0; i < *nplanes; i++)
			alloc_devs[i] = ctx->dev->dev;
	}
	return ret;
}

static void vsi_enc_buf_queue(struct vb2_buffer *vb)
{
	struct vb2_queue *vq = vb->vb2_queue;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(vq->drv_priv);
	struct vsi_vpu_buf *vsibuf;
	int ret;

	v4l2_klog(LOGLVL_DEBUG, "%d:%d", vb->type, vb->index);
	vsibuf = vb_to_vsibuf(vb);
	if (ismetabuf(vb->type)) {
		//so meta buf streamon must be before input and output queue stream on
		ctx->mediacfg.metabufinfo.busMetaBuf = getbusaddr_meta(ctx, vb);
		ctx->pmeta = getvaddr_meta(ctx, vb);
		return;
	} else if (!binputqueue(vq->type))
		list_add_tail(&vsibuf->list, &ctx->output_list);
	else
		list_add_tail(&vsibuf->list, &ctx->input_list);
	ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_BUF_RDY, vb);
}

static int vsi_enc_buf_init(struct vb2_buffer *vb)
{
	return 0;
}

static int vsi_enc_buf_prepare(struct vb2_buffer *vb)
{
	/*any valid init operation on buffer vb*/
	/*gspca and rockchip both check buffer size here*/
	//like vb2_set_plane_payload(vb, 0, 1920*1080);
	return 0;
}

static int vsi_enc_start_streaming(struct vb2_queue *q, unsigned int count)
{
	return 0;
}
static void vsi_enc_stop_streaming(struct vb2_queue *vq)
{
}

static void vsi_enc_buf_finish(struct vb2_buffer *vb)
{
}

static void vsi_enc_buf_cleanup(struct vb2_buffer *vb)
{
}

static void vsi_enc_buf_wait_finish(struct vb2_queue *vq)
{
	vb2_ops_wait_finish(vq);
}

static void vsi_enc_buf_wait_prepare(struct vb2_queue *vq)
{
	vb2_ops_wait_prepare(vq);
}

static int vsi_enc_buf_out_validate(struct vb2_buffer *vb)
{
	return 0;
}

static void vsi_enc_buf_request_complete(struct vb2_buffer *vb)
{
}

static struct vb2_ops vsi_enc_qops = {
	.queue_setup = vsi_enc_queue_setup,
	.wait_prepare = vsi_enc_buf_wait_prepare,	/*these two are just mutex protection for done_que*/
	.wait_finish = vsi_enc_buf_wait_finish,
	.buf_init = vsi_enc_buf_init,
	.buf_prepare = vsi_enc_buf_prepare,
	.buf_finish = vsi_enc_buf_finish,
	.buf_cleanup = vsi_enc_buf_cleanup,
	.start_streaming = vsi_enc_start_streaming,
	.stop_streaming = vsi_enc_stop_streaming,
	.buf_queue = vsi_enc_buf_queue,
	//fill_user_buffer
	.buf_out_validate = vsi_enc_buf_out_validate,
	.buf_request_complete = vsi_enc_buf_request_complete,
};

static void vsi_enc_updateV4l2Aspect(struct vsi_v4l2_ctx *ctx, int id, int param)
{
	struct v4l2_enccfg_aspect *pasp = &ctx->mediacfg.m_encparams.m_aspect;
	u32 width = pasp->sarWidth;
	u32 height = pasp->sarHeight;

	switch (id) {
	case V4L2_CID_MPEG_VIDEO_ASPECT:
		if (param == V4L2_MPEG_VIDEO_ASPECT_1x1) {
			width = 1; height = 1;
		} else if (param == V4L2_MPEG_VIDEO_ASPECT_4x3) {
			width = 4; height = 3;
		} else if (param == V4L2_MPEG_VIDEO_ASPECT_16x9) {
			width = 16; height = 9;
		} else if (param == V4L2_MPEG_VIDEO_ASPECT_221x100) {
			width = 221; height = 100;
		};
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH:
		width = param;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT:
		height = param;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC:
		{
			switch (param) {
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_1x1:
				width = 1; height = 1;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_12x11:
				width = 12; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_10x11:
				width = 10; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_16x11:
				width = 1; height = 1;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_40x33:
				width = 40; height = 33;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_24x11:
				width = 24; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_20x11:
				width = 20; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_32x11:
				width = 30; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_80x33:
				width = 80; height = 33;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_18x11:
				width = 18; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_15x11:
				width = 15; height = 11;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_64x33:
				width = 64; height = 33;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_160x99:
				width = 160; height = 99;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_4x3:
				width = 4; height = 3;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_3x2:
				width = 3; height = 2;
				break;
			case V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_2x1:
				width = 2; height = 1;
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	if (width != pasp->sarWidth || height != pasp->sarHeight) {
		pasp->sarHeight = height;
		pasp->sarWidth = width;
		flag_updateparam(m_aspect)
	}
}

static s32 vsi_enc_convertV4l2Quality(s32 v4l2val)
{
	if (v4l2val < 0)
		return v4l2val;
	if (v4l2val >= 100)
		return 0;
	else
		return (101 - v4l2val) / 2 + 1;	//ours is from 0-51
}

static int vsi_v4l2_enc_s_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret;
	s32 val;
	u64 temp64;
	struct vsi_v4l2_ctx *ctx = ctrl_to_ctx(ctrl);
	struct vsi_v4l2_encparams *pencparm = &ctx->mediacfg.m_encparams;

	v4l2_klog(LOGLVL_DEBUG, "%x=%d", ctrl->id, ctrl->val);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_s_ctrl error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	switch (ctrl->id) {
	case V4L2_CID_MPEG_VIDEO_B_FRAMES:
		pencparm->m_gopsize.gopSize = ctrl->val + 1;
		flag_checkparam(m_gopsize)
		break;
	case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
		if (pencparm->m_intrapicrate.intraPicRate != ctrl->val) {
			pencparm->m_intrapicrate.intraPicRate = ctrl->val;
			flag_updateparam(m_intrapicrate)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
	case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
	case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
	case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
		ret = vsi_set_profile(ctx, ctrl->id, ctrl->val);
		//flag_updateparam(m_profile) is left to get_fmtprofile
		return ret;
	case V4L2_CID_MPEG_VIDEO_BITRATE:
		if (pencparm->m_bitrate.bitPerSecond != ctrl->val) {
			pencparm->m_bitrate.bitPerSecond = ctrl->val;
			flag_updateparam(m_bitrate)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
	case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
	case V4L2_CID_MPEG_VIDEO_VP9_LEVEL:
		ret = vsi_set_Level(ctx, ctrl->id, ctrl->val);
		if (ret == 0) {
			flag_checkparam(m_level)
		}
		return ret;
	case V4L2_CID_MPEG_VIDEO_VPX_MAX_QP:
		ctx->mediacfg.qpMax_vpx = ctrl->val;
		flag_checkparam(m_qprange)
		break;
	case V4L2_CID_MPEG_VIDEO_H264_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP:
		ctx->mediacfg.qpMax_h26x = ctrl->val;
		flag_checkparam(m_qprange)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_MIN_QP:
		ctx->mediacfg.qpMin_vpx = ctrl->val;
		flag_checkparam(m_qprange)
		break;
	case V4L2_CID_MPEG_VIDEO_H264_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP:
		ctx->mediacfg.qpMin_h26x = ctrl->val;
		flag_checkparam(m_qprange)
		break;
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		if (pencparm->m_bitratemode.bitrateMode != ctrl->val) {
			pencparm->m_bitratemode.bitrateMode = ctrl->val;
			flag_updateparam(m_bitratemode)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME:
		set_bit(CTX_FLAG_FORCEIDR_BIT, &ctx->flag);
		break;
	case V4L2_CID_MPEG_VIDEO_HEADER_MODE:
		pencparm->m_headermode.headermode = ctrl->val;
		flag_updateparam(m_headermode)
		break;
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE:
		pencparm->m_sliceinfo.multislice_mode = ctrl->val;
		flag_checkparam(m_sliceinfo)
		break;
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
		pencparm->m_sliceinfo.sliceSize = ctrl->val;
		flag_checkparam(m_sliceinfo)
		break;
	case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE:
		if (pencparm->m_rcmode.picRc != ctrl->val) {
			pencparm->m_rcmode.picRc = ctrl->val;
			flag_updateparam(m_rcmode)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE:
		if (pencparm->m_rcmode.ctbRc != ctrl->val) {
			pencparm->m_rcmode.ctbRc = ctrl->val;
			flag_updateparam(m_rcmode)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
		ctx->mediacfg.qpHdrI_h26x = ctrl->val;
		flag_checkparam(m_qphdrip)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP:
		ctx->mediacfg.qpHdrI_vpx = ctrl->val;
		flag_checkparam(m_qphdrip)
		break;
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP:
		ctx->mediacfg.qpHdrP_h26x = ctrl->val;
		flag_checkparam(m_qphdrip)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP:
		ctx->mediacfg.qpHdrP_vpx = ctrl->val;
		flag_checkparam(m_qphdrip)
		break;
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP:
		if (pencparm->m_qphdrip.qpHdrB != ctrl->val) {
			pencparm->m_qphdrip.qpHdrB = ctrl->val;
			flag_updateparam(m_qphdrip)
		}
		break;
	case V4L2_CID_ROTATE:
		if (ctrl->val != pencparm->m_rotation.rotation) {
			pencparm->m_rotation.rotation = ctrl->val;
			flag_updateparam(m_rotation)
		}
		break;
	case V4L2_CID_ROI:
		if (pencparm->m_gdrduration.gdrDuration > 0)
			return -EINVAL;
		if (ctrl->p_new.p)
			vsiv4l2_setROI(ctx, ctrl->p_new.p);
		break;
	case V4L2_CID_IPCM:
		if (pencparm->m_gdrduration.gdrDuration > 0)
			return -EINVAL;
		if (ctrl->p_new.p)
			vsiv4l2_setIPCM(ctx, ctrl->p_new.p);
		break;
	case V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER:
		if (pencparm->m_idrhdr.idrHdr != ctrl->val) {
			pencparm->m_idrhdr.idrHdr = ctrl->val;
			flag_updateparam(m_idrhdr)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
		if (pencparm->m_bitrate.bitPerSecond <= 0 || pencparm->m_bitrate.bitPerSecond > ctrl->val)
			return -EINVAL;
		temp64 = (u64)(ctrl->val - pencparm->m_bitrate.bitPerSecond);
		temp64 = (temp64 * 100ul)/pencparm->m_bitrate.bitPerSecond;
		if (pencparm->m_bitratemode.tolMovingBitRate != (s32)temp64) {
			pencparm->m_bitratemode.tolMovingBitRate = (s32)temp64;
			flag_updateparam(m_bitratemode)
		}
		break;
	case V4L2_CID_JPEG_COMPRESSION_QUALITY:
		if (pencparm->m_jpgquality.quality != ctrl->val) {
			pencparm->m_jpgquality.quality = ctrl->val;
			flag_updateparam(m_jpgquality)
		}
		break;
	case V4L2_CID_JPEG_RESTART_INTERVAL:
		if (pencparm->m_restartinterval.restartInterval != ctrl->val) {
			pencparm->m_restartinterval.restartInterval = ctrl->val;
			flag_updateparam(m_restartinterval)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD:
		if (pencparm->m_gdrduration.gdrDuration != ctrl->val) {
			pencparm->m_gdrduration.gdrDuration = ctrl->val;
			flag_updateparam(m_gdrduration)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_PERIOD:	//wait for ctrlSW to realize the API
		if (pencparm->m_refresh.idr_interval != ctrl->val) {
			pencparm->m_refresh.idr_interval = ctrl->val;
			flag_updateparam(m_refresh)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
		if (pencparm->m_entropymode.enableCabac != ctrl->val) {
			pencparm->m_entropymode.enableCabac = ctrl->val;
			flag_updateparam(m_entropymode)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE:
		if (pencparm->m_cpbsize.cpbSize != ctrl->val) {
			pencparm->m_cpbsize.cpbSize = ctrl->val;
			flag_updateparam(m_cpbsize)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM:
		if (pencparm->m_trans8x8.transform8x8Enable != ctrl->val) {
			pencparm->m_trans8x8.transform8x8Enable = ctrl->val;
			flag_updateparam(m_trans8x8)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY:
		pencparm->m_rcmode.picRc =
			pencparm->m_rcmode.ctbRc = -1;
		val = vsi_enc_convertV4l2Quality(ctrl->val);
		if (pencparm->m_qphdr.qpHdr != val) {
			pencparm->m_qphdr.qpHdr = val;
			flag_updateparam(m_qphdr)
		}
		flag_updateparam(m_rcmode)
		break;
	case V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION:
		if (pencparm->m_ctrintrapred.constrained_intra_pred_flag != ctrl->val) {
			pencparm->m_ctrintrapred.constrained_intra_pred_flag = ctrl->val;
			flag_updateparam(m_ctrintrapred)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE:
		if (pencparm->m_aspect.enable != ctrl->val) {
			pencparm->m_aspect.enable = ctrl->val;
			flag_updateparam(m_aspect)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_ASPECT:
	case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC:
	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT:
	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH:
		vsi_enc_updateV4l2Aspect(ctx, ctrl->id, ctrl->val);
		break;
	case V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE:
		pencparm->m_skipmode.pictureSkip = ctrl->val - 1;
		flag_updateparam(m_skipmode)
		break;
	case V4L2_CID_MPEG_VIDEO_AU_DELIMITER:
		if (pencparm->m_aud.sendAud != ctrl->val) {
			pencparm->m_aud.sendAud = ctrl->val;
			flag_updateparam(m_aud)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
		ctx->mediacfg.disableDeblockingFilter_h264 = ctrl->val;
		flag_checkparam(m_loopfilter)
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE:
		ctx->mediacfg.disableDeblockingFilter_hevc = ctrl->val ? 0 : 1;
		flag_checkparam(m_loopfilter)
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA:
	case V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2:
		if (pencparm->m_loopfilter.tc_Offset != ctrl->val) {
			pencparm->m_loopfilter.tc_Offset = ctrl->val;
			flag_updateparam(m_loopfilter)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA:
	case V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2:
		if (pencparm->m_loopfilter.beta_Offset != ctrl->val) {
			pencparm->m_loopfilter.beta_Offset = ctrl->val;
			flag_updateparam(m_loopfilter)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET:
		if (pencparm->m_chromaqpoffset.chroma_qp_offset != ctrl->val) {
			pencparm->m_chromaqpoffset.chroma_qp_offset = ctrl->val;
			flag_updateparam(m_chromaqpoffset)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP:
		if (pencparm->m_iqprange.qpMinI != ctrl->val) {
			pencparm->m_iqprange.qpMinI = ctrl->val;
			flag_updateparam(m_iqprange)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP:
		if (pencparm->m_iqprange.qpMaxI != ctrl->val) {
			pencparm->m_iqprange.qpMaxI = ctrl->val;
			flag_updateparam(m_iqprange)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP:
		if (pencparm->m_pqprange.qpMinPB != ctrl->val) {
			pencparm->m_pqprange.qpMinPB = ctrl->val;
			flag_updateparam(m_pqprange)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP:
		if (pencparm->m_pqprange.qpMaxPB != ctrl->val) {
			pencparm->m_pqprange.qpMaxPB = ctrl->val;
			flag_updateparam(m_pqprange)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE:
		pencparm->m_mvrange.meVertSearchRange = ctrl->val;
		flag_checkparam(m_mvrange)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS:
		if (pencparm->m_vpxpartitions.dctPartitions != ctrl->val) {
			pencparm->m_vpxpartitions.dctPartitions = ctrl->val;
			flag_updateparam(m_vpxpartitions)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES:
		ctx->mediacfg.vpx_PRefN = ctrl->val + 1;
		flag_checkparam(m_refno)
		break;
	case V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES:
		ctx->mediacfg.hevc_PRefN = ctrl->val;
		flag_checkparam(m_refno)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL:
		pencparm->m_vpxfilterlvl.filterLevel = ctrl->val;
		flag_checkparam(m_vpxfilterlvl)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS:
		pencparm->m_vpxfiltersharp.filterSharpness = ctrl->val;
		flag_checkparam(m_vpxfiltersharp)
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD:
		pencparm->m_goldenperiod.goldenPictureRate = ctrl->val;
		flag_checkparam(m_goldenperiod)
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
		if (pencparm->m_tier.tier != ctrl->val) {
			pencparm->m_tier.tier = ctrl->val;
			flag_updateparam(m_tier)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE:
		if (pencparm->m_refresh.refreshtype != ctrl->val) {
			pencparm->m_refresh.refreshtype = ctrl->val;
			flag_updateparam(m_refresh)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD:
		if (pencparm->m_intrapicrate.intraPicRate != ctrl->val) {
			pencparm->m_intrapicrate.intraPicRate = ctrl->val;
			flag_updateparam(m_intrapicrate)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID:
		if (pencparm->m_temporalid.temporalId != ctrl->val) {
			pencparm->m_temporalid.temporalId = ctrl->val;
			flag_updateparam(m_temporalid)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING:
		if (pencparm->m_strongsmooth.strong_intra_smoothing_enabled_flag != ctrl->val) {
			pencparm->m_strongsmooth.strong_intra_smoothing_enabled_flag = ctrl->val;
			flag_updateparam(m_strongsmooth)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION:
		if (pencparm->m_tmvp.enableTMVP != ctrl->val) {
			pencparm->m_tmvp.enableTMVP = ctrl->val;
			flag_updateparam(m_tmvp)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE:
		if (pencparm->m_startcode.streamType != ctrl->val) {
			pencparm->m_startcode.streamType = ctrl->val;
			flag_updateparam(m_startcode)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR:
		if (pencparm->m_resendSPSPPS.resendSPSPPS != ctrl->val) {
			pencparm->m_resendSPSPPS.resendSPSPPS = ctrl->val;
			flag_updateparam(m_resendSPSPPS)
		}
		break;
	case V4L2_CID_JPEG_CHROMA_SUBSAMPLING:
		if (!vsiv4l2_has_jpgcodingmode(ctrl->val))
			break;
		if (pencparm->m_jpgcodingmode.codingMode != ctrl->val) {
			pencparm->m_jpgcodingmode.codingMode = ctrl->val;
			flag_updateparam(m_jpgcodingmode)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING:
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP:
		if (pencparm->m_gopcfg.hierachy_enable != ctrl->val) {
			pencparm->m_gopcfg.hierachy_enable = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE:
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
		if (pencparm->m_gopcfg.hierachy_codingtype != ctrl->val) {
			pencparm->m_gopcfg.hierachy_codingtype = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER:
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
		if (pencparm->m_gopcfg.codinglayer != ctrl->val) {
			pencparm->m_gopcfg.codinglayer = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP:
		val = vsiv4l2_verify_h264_codinglayerqp(ctrl->val);
		if (val != -1) {
			u32 layern = (u32)ctrl->val >> 16;
			if (val != pencparm->m_gopcfg.codinglayerqp[layern]) {
				pencparm->m_gopcfg.codinglayerqp[layern] = val;
				flag_updateparam(m_gopcfg)
			}
			break;
		} else
			return -EINVAL;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP:
		if (vsiv4l2_get_maxhevclayern() >= 0) {
			pencparm->m_gopcfg.codinglayerqp[0] = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP:
		if (vsiv4l2_get_maxhevclayern() >= 1) {
			pencparm->m_gopcfg.codinglayerqp[1] = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP:
		if (vsiv4l2_get_maxhevclayern() >= 2) {
			pencparm->m_gopcfg.codinglayerqp[2] = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP:
		if (vsiv4l2_get_maxhevclayern() >= 3) {
			pencparm->m_gopcfg.codinglayerqp[3] = ctrl->val;
			flag_updateparam(m_gopcfg)
		}
		break;
	case V4L2_CID_SECUREMODE:
		if (pencparm->m_securemode.secureModeOn != ctrl->val) {
			pencparm->m_securemode.secureModeOn = ctrl->val;
			flag_updateparam(m_securemode)
		}
		break;
	case V4L2_CID_ADDR_OFFSET:
		if (ctrl->p_new.p_s64)
			ctx->addr_offset = *ctrl->p_new.p_s64;
		break;
	case V4L2_CID_ENC_SCALE_INFO:
		if (ctrl->p_new.p) {
			if (vsiv4l2_configScaleOutput(ctx, ctrl->p_new.p) < 0)
				return -EINVAL;
		}
		break;
	case V4L2_CID_EXT_SLICE_ENCODE:
		{
			pencparm->m_sliceencoding.ctu_rows = ctrl->val;
			flag_updateparam(m_sliceencoding)
		}
		break;
	default:
		return 0;
	}
	return 0;
}

static int vsi_v4l2_enc_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct vsi_v4l2_ctx *ctx = ctrl_to_ctx(ctrl);

	v4l2_klog(LOGLVL_DEBUG, "%x", ctrl->id);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, enc_g_volatile_ctrl error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = ctx->mediacfg.minbuf_4capture;	//these two may come from resoultion change
		break;
	case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
		ctrl->val = ctx->mediacfg.minbuf_4output;
		break;
	case V4L2_CID_ROI_COUNT:
		ctrl->val = vsiv4l2_getROIcount();
		break;
	case V4L2_CID_IPCM_COUNT:
		ctrl->val = vsiv4l2_getIPCMcount();
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/********* for ext ctrl *************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
static bool vsi_enc_ctrl_equal(const struct v4l2_ctrl *ctrl,
		      union v4l2_ctrl_ptr ptr1,
		      union v4l2_ctrl_ptr ptr2)
#else
static bool vsi_enc_ctrl_equal(const struct v4l2_ctrl *ctrl, u32 idx,
		      union v4l2_ctrl_ptr ptr1,
		      union v4l2_ctrl_ptr ptr2)
#endif
{
	//always update now, fix it later
	return 0;
}

static void vsi_enc_ctrl_init(const struct v4l2_ctrl *ctrl, u32 idx,
		     union v4l2_ctrl_ptr ptr)
{
	void *p = ptr.p + idx * ctrl->elem_size;

	memset(p, 0, ctrl->elem_size);
}

static void vsi_enc_ctrl_log(const struct v4l2_ctrl *ctrl)
{
	//do nothing now
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
static int vsi_enc_ctrl_validate(const struct v4l2_ctrl *ctrl,
			union v4l2_ctrl_ptr ptr)
#else
static int vsi_enc_ctrl_validate(const struct v4l2_ctrl *ctrl, u32 idx,
			union v4l2_ctrl_ptr ptr)
#endif
{
	//always true
	return 0;
}

static const struct v4l2_ctrl_type_ops vsi_enc_type_ops = {
	.equal = vsi_enc_ctrl_equal,
	.init = vsi_enc_ctrl_init,
	.log = vsi_enc_ctrl_log,
	.validate = vsi_enc_ctrl_validate,
};
/********* for ext ctrl *************/

static const struct v4l2_ctrl_ops vsi_encctrl_ops = {
	.s_ctrl = vsi_v4l2_enc_s_ctrl,
	.g_volatile_ctrl = vsi_v4l2_enc_g_volatile_ctrl,
};

static struct v4l2_ctrl_config vsi_v4l2_encctrl_defs[] = {
	{
		.ops = &vsi_encctrl_ops,
		.id = V4L2_CID_ROI_COUNT,
		.name = "get max ROI region number",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_READ_ONLY,
		.min = 0,
		.max = V4L2_MAX_ROI_REGIONS,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_encctrl_ops,
		.type_ops = &vsi_enc_type_ops,
		.id = V4L2_CID_ROI,
		.name = "vsi priv v4l2 roi params set",
		.type = VSI_V4L2_CMPTYPE_ROI,
		.min = 0,
		.max = V4L2_MAX_ROI_REGIONS,
		.step = 1,
		.def = 0,
		.elem_size = sizeof(struct v4l2_enc_roi_params),
	},
	{
		.ops = &vsi_encctrl_ops,
		.id = V4L2_CID_IPCM_COUNT,
		.name = "get max IPCM region number",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_READ_ONLY,
		.min = 0,
		.max = V4L2_MAX_IPCM_REGIONS,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_encctrl_ops,
		.type_ops = &vsi_enc_type_ops,
		.id = V4L2_CID_IPCM,
		.name = "vsi priv v4l2 ipcm params set",
		.type = VSI_V4L2_CMPTYPE_IPCM,
		.min = 0,
		.max = V4L2_MAX_IPCM_REGIONS,
		.step = 1,
		.def = 0,
		.elem_size = sizeof(struct v4l2_enc_ipcm_params),
	},
	/* kernel defined controls */
	{
		.id = V4L2_CID_MPEG_VIDEO_GOP_SIZE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = MAX_INTRA_PIC_RATE,
		.step = 1,
		.def = DEFAULT_INTRA_PIC_RATE,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_BITRATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 10000,
		.max = 288000000,
		.step = 1,
		.def = 2097152,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_PROFILE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
		.max = V4L2_MPEG_VIDEO_H264_PROFILE_MULTIVIEW_HIGH,
		.def = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VP8_PROFILE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_VP8_PROFILE_0,
		.max = V4L2_MPEG_VIDEO_VP8_PROFILE_3,
		.def = V4L2_MPEG_VIDEO_VP8_PROFILE_0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VP9_PROFILE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_VP9_PROFILE_0,
		.max = V4L2_MPEG_VIDEO_VP9_PROFILE_3,
		.def = V4L2_MPEG_VIDEO_VP9_PROFILE_0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_PROFILE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min =  V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN,
		.max = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN_10,
		.def = V4L2_MPEG_VIDEO_HEVC_PROFILE_MAIN,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_H264_LEVEL_1_0,
		.max = V4L2_MPEG_VIDEO_H264_LEVEL_6_2,
		.def = V4L2_MPEG_VIDEO_H264_LEVEL_5_0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_LEVEL,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_HEVC_LEVEL_1,
		.max = V4L2_MPEG_VIDEO_HEVC_LEVEL_6_2,
		.def = V4L2_MPEG_VIDEO_HEVC_LEVEL_5,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VP9_LEVEL,
		.name = "set vp9 encoding level",
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_VP9_LEVEL_1_0,
		.max = V4L2_MPEG_VIDEO_VP9_LEVEL_6_2,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_VP9_LEVEL_1_0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 51,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 51,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_MIN_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEADER_MODE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE,
		.max = V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME,
		.def = V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_B_FRAMES,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = MAX_GOP_SIZE - 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 51,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 51,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_BITRATE_MODE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_BITRATE_MODE_VBR,
		.max = V4L2_MPEG_VIDEO_BITRATE_MODE_CRF,
		.def = V4L2_MPEG_VIDEO_BITRATE_MODE_VBR,
	},
	{
		.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE,	//volatile contains read
		.min = 1,
		.max = MAX_MIN_BUFFERS_FOR_CAPTURE,
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MIN_BUFFERS_FOR_OUTPUT,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE,
		.min = 1,
		.max = MAX_MIN_BUFFERS_FOR_OUTPUT,
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME,
		.type = V4L2_CTRL_TYPE_BUTTON,
		.min = 0,
		.max = 0,
		.step = 0,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 51,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 51,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 51,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 51,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE,
		.max = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_MAX_MB,
		.def = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 1,
		.max = 120,		//1920 div 16
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 127,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 127,
		.step = 1,
		.def = DEFAULT_QP,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_MIN_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 127,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_MAX_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 127,
		.step = 1,
		.def = 127,
	},
	{
		.id = V4L2_CID_ROTATE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 270,
		.step = 90,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_BITRATE_PEAK,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 10000,
		.max = 288000000,
		.step = 1,
		.def = 2097152,
	},
	{
		.id = V4L2_CID_JPEG_COMPRESSION_QUALITY,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 100,
		.step = 1,
		.def = 40,
	},
	{
		.id = V4L2_CID_JPEG_RESTART_INTERVAL,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 7,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD,
		.name = "set refresh rate",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = MAX_INTRA_PIC_RATE,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
		.max = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CABAC,
		.def = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 175000,
		.max = 240000000,
		.step = 1,
		.def = 135000000,				//sync to default h264 level = L5
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY,
		.name = "video encoding constant quality",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -1,
		.max = 100,
		.step = 1,
		.def = 40,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE,
		.name = "frame skip mode select",
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE_DISABLED,
		.max = V4L2_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE_BUF_LIMIT,
		.step = 1,
		.def = V4L2_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE_DISABLED,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_AU_DELIMITER,
		.name = "set au delimiter",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_ASPECT,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_ASPECT_1x1,
		.max = V4L2_MPEG_VIDEO_ASPECT_221x100,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_ASPECT_4x3,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_UNSPECIFIED,
		.max = V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_EXTENDED,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_UNSPECIFIED,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 65535,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 65535,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_ENABLED,
		.max = V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_H264_LOOP_FILTER_MODE_DISABLED,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_DISABLED,
		.max = V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_ENABLED,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE_DISABLED,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -6,
		.max = 6,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -6,
		.max = 6,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -6,
		.max = 6,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -6,
		.max = 6,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = -12,
		.max = 12,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP,
		.name = "set hevc Iframe min qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP,
		.name = "set hevc Iframe max qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP,
		.name = "set 264 Bframe min qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP,
		.name = "set 264 Bframe max qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP,
		.name = "set hevc Pframe min qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP,
		.name = "set hevc Bframe min qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP,
		.name = "set hevc pframe max qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP,
		.name = "set hevc Bframe max qp",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 64,
		.step = 8,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS,
		.type = V4L2_CTRL_TYPE_INTEGER_MENU,
		.min = V4L2_CID_MPEG_VIDEO_VPX_1_PARTITION,
		.max = V4L2_CID_MPEG_VIDEO_VPX_8_PARTITIONS,
		.step = 1,
		.def = V4L2_CID_MPEG_VIDEO_VPX_1_PARTITION,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES,
		.type = V4L2_CTRL_TYPE_INTEGER_MENU,
		.min = V4L2_CID_MPEG_VIDEO_VPX_1_REF_FRAME,
		.max = V4L2_CID_MPEG_VIDEO_VPX_3_REF_FRAME,
		.step = 1,
		.def = V4L2_CID_MPEG_VIDEO_VPX_1_REF_FRAME,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 1,
		.max = 2,
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 64,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 8,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 1000,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_TIER,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_HEVC_TIER_MAIN,
		.max = V4L2_MPEG_VIDEO_HEVC_TIER_HIGH,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_HEVC_TIER_MAIN,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_HEVC_REFRESH_NONE,
		.max = V4L2_MPEG_VIDEO_HEVC_REFRESH_IDR,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_HEVC_REFRESH_NONE,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 1000,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_JPEG_CHROMA_SUBSAMPLING,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_JPEG_CHROMA_SUBSAMPLING_444,
		.max = V4L2_JPEG_CHROMA_SUBSAMPLING_GRAY,
		.step = 1,
		.def = V4L2_JPEG_CHROMA_SUBSAMPLING_420,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 1,
		.max = 4,
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 0x30033,		//layer 3, max qp 51
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B,
		.max = V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_P,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_HEVC_HIERARCHICAL_CODING_B,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 1,
		.max = 4,
		.step = 1,
		.def = 1,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 51,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE,
		.type = V4L2_CTRL_TYPE_MENU,
		.min = V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_B,
		.max = V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_P,
		.step = 1,
		.def = V4L2_MPEG_VIDEO_H264_HIERARCHICAL_CODING_B,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.id = V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION,
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_encctrl_ops,
		.id = V4L2_CID_SECUREMODE,
		.name = "en/disable enc secure mode",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_encctrl_ops,
		.id = V4L2_CID_ADDR_OFFSET,
		.name = "set addr offset between VPU and CPU",
		.type = V4L2_CTRL_TYPE_INTEGER64,
		.min = 0x8000000000000000,
		.max = 0x7fffffffffffffff,
		.step = 1,
		.def = 0,
		.elem_size = sizeof(s64),
	},
	{
		.ops = &vsi_encctrl_ops,
		.type_ops = &vsi_enc_type_ops,
		.id = V4L2_CID_ENC_SCALE_INFO,
		.name = "vsi priv v4l2 enable enc scale output channel",
		.type = VSI_V4L2_CMPTYPE_ENCSCALEOUT,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
		.elem_size = sizeof(struct v4l2_enc_scaleinfo),
	},
	{
		.ops = &vsi_encctrl_ops,
		.id = V4L2_CID_EXT_SLICE_ENCODE,
		.name = "ctu rows per slice",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = INT_MAX,
		.step = 1,
		.def = 0,
	},

};

static const char *const vsi_bitrate_mode[] = {
	"variable bitrate",
	"constant bitrate",
	"constant quality",
	"constrained variable bitrate mode",
	"average bitrate mode",
	"constant rate factor mode",
	NULL,
};

void vsi_enc_update_ctrltbl(int ctrlid, int index, s32 val)
{
	int i, ctrl_num = ARRAY_SIZE(vsi_v4l2_encctrl_defs);
	for (i = 0; i < ctrl_num; i++) {
		if (vsi_v4l2_encctrl_defs[i].id == ctrlid) {
			if (index == CTRL_IDX_MIN) {
				vsi_v4l2_encctrl_defs[i].min = val;
				if (vsi_v4l2_encctrl_defs[i].def < val)
					vsi_v4l2_encctrl_defs[i].min = val;
			}
			if (index == CTRL_IDX_MAX) {
				// for these two integer_menu items, max range can't be expanded
				if (ctrlid == V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS)
					val = min(V4L2_CID_MPEG_VIDEO_VPX_8_PARTITIONS, val);
				if (ctrlid == V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES)
					val = min(V4L2_CID_MPEG_VIDEO_VPX_3_REF_FRAME, val);
				vsi_v4l2_encctrl_defs[i].max = val;
				if (vsi_v4l2_encctrl_defs[i].def > val)
					vsi_v4l2_encctrl_defs[i].def = val;
			}
			if (index == CTRL_IDX_DEF) {
				vsi_v4l2_encctrl_defs[i].def = val;
				if (vsi_v4l2_encctrl_defs[i].type == V4L2_CTRL_TYPE_INTEGER &&
					val < vsi_v4l2_encctrl_defs[i].min &&
					val == -1)
				vsi_v4l2_encctrl_defs[i].min = val;
			}
		}
	}
}

static void vsi_enc_ctx_defctrl(struct vsi_v4l2_ctx *ctx, int ctrlid, s32 defval)
{
	struct vsi_v4l2_encparams *pencparm = &ctx->mediacfg.m_encparams;

	switch (ctrlid) {
	case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
		pencparm->m_intrapicrate.intraPicRate = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_BITRATE:
		pencparm->m_bitrate.bitPerSecond = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
		ctx->mediacfg.profile_h264 = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
		ctx->mediacfg.profile_vp8 = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
		ctx->mediacfg.profile_vp9 = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
		ctx->mediacfg.profile_hevc = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
		ctx->mediacfg.avclevel = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
		ctx->mediacfg.hevclevel = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VP9_LEVEL:
		ctx->mediacfg.vp9level = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP:
		ctx->mediacfg.qpMax_h26x = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP:
		ctx->mediacfg.qpMin_h26x = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_B_FRAMES:
		pencparm->m_gopsize.gopSize = defval + 1;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP:
		pencparm->m_qphdrip.qpHdrB = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		pencparm->m_bitratemode.bitrateMode = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
		ctx->mediacfg.qpHdrI_h26x = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP:
		ctx->mediacfg.qpHdrP_h26x = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE:
		pencparm->m_sliceinfo.multislice_mode = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB:
		pencparm->m_sliceinfo.sliceSize = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE:
		pencparm->m_rcmode.picRc = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE:
		pencparm->m_rcmode.ctbRc = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP:
		ctx->mediacfg.qpHdrI_vpx = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP:
		ctx->mediacfg.qpHdrP_vpx = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_MIN_QP:
		ctx->mediacfg.qpMin_vpx = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_MAX_QP:
		ctx->mediacfg.qpMax_vpx = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER:
		pencparm->m_idrhdr.idrHdr = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
		pencparm->m_bitratemode.tolMovingBitRate = defval;
		break;
	case V4L2_CID_JPEG_COMPRESSION_QUALITY:
		pencparm->m_jpgquality.quality = defval;
		break;
	case V4L2_CID_JPEG_RESTART_INTERVAL:
		pencparm->m_restartinterval.restartInterval = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD:
		pencparm->m_gdrduration.gdrDuration = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE:
		pencparm->m_entropymode.enableCabac = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE:
		pencparm->m_cpbsize.cpbSize = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM:
		pencparm->m_trans8x8.transform8x8Enable = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY:
		pencparm->m_qphdr.qpHdr = vsi_enc_convertV4l2Quality(defval);
		break;
	case V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE:
		pencparm->m_skipmode.pictureSkip = defval - 1;
		break;
	case V4L2_CID_MPEG_VIDEO_AU_DELIMITER:
		pencparm->m_aud.sendAud = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE:
		pencparm->m_aspect.enable = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH:
		pencparm->m_aspect.sarWidth = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT:
		pencparm->m_aspect.sarHeight = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE:
		ctx->mediacfg.disableDeblockingFilter_h264 = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE:
		ctx->mediacfg.disableDeblockingFilter_hevc = (defval ? 0:1);
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA:
	case V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2:
		pencparm->m_loopfilter.tc_Offset = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA:
	case V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2:
		pencparm->m_loopfilter.beta_Offset = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET:
		pencparm->m_chromaqpoffset.chroma_qp_offset = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP:
		pencparm->m_iqprange.qpMinI = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP:
		pencparm->m_iqprange.qpMaxI = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP:
		pencparm->m_pqprange.qpMinPB = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP:
	case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP:
		pencparm->m_pqprange.qpMaxPB = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE:
		pencparm->m_mvrange.meVertSearchRange = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS:
		pencparm->m_vpxpartitions.dctPartitions = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES:
		ctx->mediacfg.vpx_PRefN = defval + 1;
		break;
	case V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES:
		ctx->mediacfg.hevc_PRefN = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL:
		pencparm->m_vpxfilterlvl.filterLevel = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS:
		pencparm->m_vpxfiltersharp.filterSharpness = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD:
		pencparm->m_goldenperiod.goldenPictureRate = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_TIER:
		pencparm->m_tier.tier = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE:
		pencparm->m_refresh.refreshtype = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD:
		pencparm->m_intrapicrate.intraPicRate = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID:
		pencparm->m_temporalid.temporalId = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION:
		pencparm->m_tmvp.enableTMVP = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE:
		pencparm->m_startcode.streamType = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR:
		pencparm->m_resendSPSPPS.resendSPSPPS = defval;
		break;
	case V4L2_CID_JPEG_CHROMA_SUBSAMPLING:
		pencparm->m_jpgcodingmode.codingMode = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING:
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP:
		pencparm->m_gopcfg.hierachy_enable = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER:
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
		pencparm->m_gopcfg.codinglayer = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP:
		pencparm->m_gopcfg.codinglayerqp[0] = defval;
		pencparm->m_gopcfg.codinglayerqp[1] = defval | (1 << 16);
		pencparm->m_gopcfg.codinglayerqp[2] = defval | (2 << 16);
		pencparm->m_gopcfg.codinglayerqp[3] = defval | (3 << 16);
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE:
	case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE:
		pencparm->m_gopcfg.hierachy_codingtype = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP:
		pencparm->m_gopcfg.codinglayerqp[0] = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP:
		pencparm->m_gopcfg.codinglayerqp[1] = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP:
		pencparm->m_gopcfg.codinglayerqp[2] = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP:
		pencparm->m_gopcfg.codinglayerqp[3] = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING:
		pencparm->m_strongsmooth.strong_intra_smoothing_enabled_flag = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION:
		pencparm->m_ctrintrapred.constrained_intra_pred_flag = defval;
		break;
	case V4L2_CID_MPEG_VIDEO_HEADER_MODE:
		pencparm->m_headermode.headermode = defval;
		break;
	default:
		break;
	}
}

static int vsi_v4l2_enc_verify_ctrl_typeNrange(struct v4l2_ctrl_config *pctrl)
{
	if (pctrl->type == V4L2_CTRL_TYPE_MENU ||
		pctrl->type == V4L2_CTRL_TYPE_BOOLEAN)
		if (pctrl->max <= pctrl->min)
			return 0;
	return 1;
}

static void vsi_handle_memu_ctrls(struct v4l2_ctrl *ctrl)
{
	if (ctrl) {
		if (ctrl->id == V4L2_CID_MPEG_VIDEO_BITRATE_MODE)
			ctrl->qmenu = vsi_bitrate_mode;
	}
}

static int vsi_setup_enc_ctrls(struct vsi_v4l2_ctx *ctx)
{
	int i, ctrl_num = ARRAY_SIZE(vsi_v4l2_encctrl_defs);
	struct v4l2_ctrl_handler *handler = &ctx->ctrlhdl;
	struct v4l2_ctrl *ctrl = NULL;

	v4l2_ctrl_handler_init(handler, ctrl_num);

	if (handler->error)
		return handler->error;

	for (i = 0; i < ctrl_num; i++) {
		vsi_enc_ctx_defctrl(ctx, vsi_v4l2_encctrl_defs[i].id, vsi_v4l2_encctrl_defs[i].def);
		if (!vsi_v4l2_enc_verify_ctrl_typeNrange(&vsi_v4l2_encctrl_defs[i]))
			continue;
		if (is_vsi_ctrl(vsi_v4l2_encctrl_defs[i].id)) {
			vsi_v4l2_encctrl_defs[i].ops = &vsi_encctrl_ops;
			if (vsi_v4l2_encctrl_defs[i].type == V4L2_CTRL_TYPE_MENU) {
				vsi_v4l2_encctrl_defs[i].type = V4L2_CTRL_TYPE_INTEGER;
				vsi_v4l2_encctrl_defs[i].step = 1;
			}
			ctrl = v4l2_ctrl_new_custom(handler, &vsi_v4l2_encctrl_defs[i], NULL);
		} else {
			if (vsi_v4l2_encctrl_defs[i].type == V4L2_CTRL_TYPE_MENU ||
				vsi_v4l2_encctrl_defs[i].type == V4L2_CTRL_TYPE_INTEGER_MENU) {
				ctrl = v4l2_ctrl_new_std_menu(handler, &vsi_encctrl_ops,
					vsi_v4l2_encctrl_defs[i].id,
					vsi_v4l2_encctrl_defs[i].max,
					0,
					vsi_v4l2_encctrl_defs[i].def);
				vsi_handle_memu_ctrls(ctrl);
			} else {
				ctrl = v4l2_ctrl_new_std(handler,
					&vsi_encctrl_ops,
					vsi_v4l2_encctrl_defs[i].id,
					vsi_v4l2_encctrl_defs[i].min,
					vsi_v4l2_encctrl_defs[i].max,
					vsi_v4l2_encctrl_defs[i].step,
					vsi_v4l2_encctrl_defs[i].def);
			}
		}
		if (ctrl && (vsi_v4l2_encctrl_defs[i].flags & V4L2_CTRL_FLAG_VOLATILE))
			ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;

		if (handler->error) {
			v4l2_klog(LOGLVL_ERROR, "fail to set ctrl %d 0x%x:%d",
								i, vsi_v4l2_encctrl_defs[i].id, handler->error);
			handler->error = 0;
		}
	}

	v4l2_ctrl_handler_setup(handler);
	return handler->error;
}

static int v4l2_enc_open(struct file *filp)
{
	//struct video_device *vdev = video_devdata(filp);
	struct vsi_v4l2_device *dev = video_drvdata(filp);
	struct vsi_v4l2_ctx *ctx = NULL;
	struct vb2_queue *q;
	int ret = 0;
	struct v4l2_fh *vfh;
	pid_t  pid;

	/* Allocate memory for context */
	//fh->video_devdata = struct video_device, struct video_device->video_drvdata = struct vsi_v4l2_device
	if (!dev)
		return -ENOENT;

	ctx = vsi_create_ctx(filp, dev);
	if (ctx == NULL) {
		v4l2_klog(LOGLVL_ERROR, "create enc ctx fail.\n");
		return -ENOMEM;
	}

	ctx->dev = dev;
	mutex_init(&ctx->ctxlock);
	init_waitqueue_head(&ctx->retbuf_queue);
	init_waitqueue_head(&ctx->capoffdone_queue);
	ctx->flag = CTX_FLAG_ENC;
	set_bit(CTX_FLAG_ENC_FLUSHBUF, &ctx->flag);
	vsiv4l2_initcfg(ctx);
	vsi_setup_enc_ctrls(ctx);
	vfh = (struct v4l2_fh *)filp->private_data;
	vfh->ctrl_handler = &ctx->ctrlhdl;
#ifdef USE_FAKE_M2M
	mutex_init(&ctx->ioctl_lock);
	ctx->m2mfake.q_lock = &ctx->ioctl_lock;
	vfh->m2m_ctx = &ctx->m2mfake;
#endif
	atomic_set(&ctx->srcframen, 0);
	atomic_set(&ctx->dstframen, 0);
	ctx->status = VSI_STATUS_INIT;

	ctx->frameidx = 0;
	q = &ctx->input_que;
	q->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->min_buffers_needed = MIN_FRAME_4ENC;
	q->drv_priv = &ctx->fh;
	q->lock = &ctx->ctxlock;
	q->buf_struct_size = sizeof(struct vsi_vpu_buf);		//used to alloc mem control structures in reqbuf
	q->ops = &vsi_enc_qops;		/*it might be used to identify input and output */
	q->mem_ops = get_vsi_mmop();
	q->memory = VB2_MEMORY_UNKNOWN;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	INIT_LIST_HEAD(&ctx->input_list);
	ret = vb2_queue_init(q);
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "[%s]%d ctx->input_que vb2_queue_init() fail.\n",
			__func__, __LINE__);
		goto err_enc_dec_exit;
	}

	q = &ctx->output_que;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->min_buffers_needed = 1;
	q->drv_priv = &ctx->fh;
	q->lock = &ctx->ctxlock;
	q->buf_struct_size = sizeof(struct vsi_vpu_buf);
	q->ops = &vsi_enc_qops;
	q->mem_ops = get_vsi_mmop();
	q->memory = VB2_MEMORY_UNKNOWN;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	INIT_LIST_HEAD(&ctx->output_list);
	ret = vb2_queue_init(q);
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "[%s]%d ctx->output_que vb2_queue_init() fail.\n",
			__func__, __LINE__);
		vb2_queue_release(&ctx->input_que);
		goto err_enc_dec_exit;
	}
	INIT_LIST_HEAD(&ctx->x_pp_list);

	q = &ctx->meta_que;
	q->type = V4L2_BUF_TYPE_META_OUTPUT;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->min_buffers_needed = 1;
	q->drv_priv = &ctx->fh;
	q->lock = &ctx->ctxlock;
	q->buf_struct_size = sizeof(struct vsi_vpu_buf);		//used to alloc mem control structures in reqbuf
	q->ops = &vsi_enc_qops;		/*it might be used to identify input and output */
	q->mem_ops = get_vsi_mmop();
	q->memory = VB2_MEMORY_UNKNOWN;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	ret = vb2_queue_init(q);
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "[%s]%d ctx->meta_que vb2_queue_init() fail.\n",
			__func__, __LINE__);
		vb2_queue_release(&ctx->input_que);
		vb2_queue_release(&ctx->output_que);
		goto err_enc_dec_exit;
	}

	ret = vsi_v4l2_addinstance(dev, &pid);
	if (ret < 0) {
		vb2_queue_release(&ctx->input_que);
		vb2_queue_release(&ctx->output_que);
		vb2_queue_release(&ctx->meta_que);
		goto err_enc_dec_exit;
	}

	return 0;

err_enc_dec_exit:
	vsi_remove_ctx(ctx);
	return ret;
}

static int v4l2_enc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	unsigned long offset = vma->vm_pgoff;
	u32 bufidx, planeidx;
	int ret;

	v4l2_klog(LOGLVL_DEBUG, "in\n");
	switch ((offset >> 7) & 0x3) {
	case 0:
		bufidx = (offset >> 2) & 0x1f;
		planeidx = offset & 0x3;
		vma->vm_pgoff = ctx->inputbufoffset[bufidx][planeidx] >> PAGE_SHIFT;
		ret = vb2_mmap(&ctx->input_que, vma);
		break;
	case 1:
		bufidx = (offset >> 2) & 0x1f;
		planeidx = offset & 0x3;
		vma->vm_pgoff = ctx->outputbufoffset[bufidx][planeidx] >> PAGE_SHIFT;
		ret = vb2_mmap(&ctx->output_que, vma);
		break;
	case 2:
		vma->vm_pgoff = ctx->metabufoffset[0] >> PAGE_SHIFT;
		ret = vb2_mmap(&ctx->meta_que, vma);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

static __poll_t vsi_enc_poll(struct file *file, poll_table *wait)
{
	__poll_t ret = 0;
	__poll_t ret_vb2 = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	int dstn;
	int srcn;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		ret |= POLLERR;
		return ret;
	}

	dstn = atomic_read(&ctx->dstframen);
	srcn = atomic_read(&ctx->srcframen);

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		ret |= POLLERR;
		v4l2_klog(LOGLVL_ERROR, "poll error, daemon is die\n");
	}

	if (v4l2_event_pending(&ctx->fh)) {
		v4l2_klog(LOGLVL_INFO, "event");
		ret |= POLLPRI;
	}

	if (vb2_is_streaming(&ctx->output_que)) {
		ret_vb2 = vb2_poll(&ctx->output_que, file, wait);
		if (ret_vb2 & POLLERR)
			v4l2_klog(LOGLVL_ERROR, "vb2_poll output_que error, ret_vb2 %x, "
					"is_streaming %d, error %d, num_buffers %d\n",
					ret_vb2, vb2_is_streaming(&ctx->output_que),
					ctx->output_que.error, ctx->output_que.num_buffers);
		ret |= ret_vb2;
	}

	if (vb2_is_streaming(&ctx->input_que)) {
		ret_vb2 = vb2_poll(&ctx->input_que, file, wait);
		if (ret_vb2 & POLLERR)
			v4l2_klog(LOGLVL_ERROR, "vb2_poll input_que error, ret_vb2 %x "
					"is_streaming %d, error %d, num_buffers %d\n",
					ret_vb2, vb2_is_streaming(&ctx->input_que),
					ctx->input_que.error, ctx->input_que.num_buffers);
		ret |= ret_vb2;
	}

	/*recheck for poll hang*/
	if (ret == 0) {
		if (dstn != atomic_read(&ctx->dstframen)) {
			ret_vb2 = vb2_poll(&ctx->output_que, file, wait);
			if (ret_vb2 & POLLERR)
				v4l2_klog(LOGLVL_ERROR, "recheck: vb2_poll output_que error, ret_vb2 %x\n", ret_vb2);
			ret |= ret_vb2;
		}
		if (srcn != atomic_read(&ctx->srcframen)) {
			ret_vb2 = vb2_poll(&ctx->input_que, file, wait);
			if (ret_vb2 & POLLERR)
				v4l2_klog(LOGLVL_ERROR, "recheck: vb2_poll input_que error, ret_vb2 %x\n", ret_vb2);
			ret |= ret_vb2;
		}
	}
	if (ctx->error < 0) {
		ret |= POLLERR;
		v4l2_klog(LOGLVL_ERROR, "poll error, ret %x, ctx error: %d\n", ret, ctx->error);
	}

	v4l2_klog(LOGLVL_VERBOSE, "%x", ret);
	return ret;
}

static const struct v4l2_file_operations v4l2_enc_fops = {
	.owner = THIS_MODULE,
	.open = v4l2_enc_open,
	.release = vsi_v4l2_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = v4l2_enc_mmap,
	.poll = vsi_enc_poll,
};

struct video_device *vsi_v4l2_probe_enc(struct platform_device *pdev, struct vsi_v4l2_device *vpu)
{
	struct video_device *venc;
	int ret = 0;

	v4l2_init_klog(LOGLVL_INFO, "module %d start\n", vpu->deviceid);

	/*init video device0, encoder */
	venc = video_device_alloc();
	if (!venc) {
		v4l2_err(&vpu->v4l2_dev, "Failed to allocate enc device\n");
		ret = -ENOMEM;
		goto err;
	}

	venc->fops = &v4l2_enc_fops;
	venc->ioctl_ops = &vsi_enc_ioctl;
	venc->device_caps = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_META_OUTPUT;
	venc->release = video_device_release;
	venc->lock = &vpu->lock;
	venc->v4l2_dev = &vpu->v4l2_dev;
	venc->vfl_dir = VFL_DIR_M2M;
	venc->vfl_type = VSI_DEVTYPE;
	venc->queue = NULL;

	video_set_drvdata(venc, vpu);

	ret = video_register_device(venc, VSI_DEVTYPE, 0);
	if (ret) {
		v4l2_err(&vpu->v4l2_dev, "Failed to register enc device\n");
		video_device_release(venc);
		goto err;
	}

	return venc;
err:
	return NULL;
}

void vsi_v4l2_release_enc(struct video_device *venc)
{
	video_unregister_device(venc);
}

