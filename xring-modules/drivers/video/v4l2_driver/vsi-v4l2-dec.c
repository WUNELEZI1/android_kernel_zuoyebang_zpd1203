/*
 *    VSI V4L2 decoder entry.
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

#define CREATE_TRACE_POINTS
#include "vpu_v4l2_trace.h"
#undef CREATE_TRACE_POINTS

#define THRESHOLD_MS                        (400ULL)

static int vsi_dec_querycap(
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
	if (hwinfo->dec_corenum == 0)
		return -ENODEV;
	strscpy(cap->driver, "vsi_v4l2", sizeof(cap->driver));
	strscpy(cap->card, "vsi_v4l2dec", sizeof(cap->card));
	strscpy(cap->bus_info, "platform:vsi_v4l2dec", sizeof(cap->bus_info));
	cap->reserved[0] = high;
	cap->reserved[1] = low;

	cap->device_caps = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_META_CAPTURE;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

static void vsi_dec_check_ctx_plane_size(struct vsi_v4l2_ctx *ctx, u32 type)
{
	struct vsi_v4l2_mediacfg *mediacfg;
	u32 planes_num;
	u32 *plane_size;
	char *direction;
	int i;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=NULL\n");
		return;
	}

	mediacfg = &(ctx->mediacfg);

	direction =  binputqueue(type) ? "[input]" : "[output]";
	planes_num = binputqueue(type) ? mediacfg->srcplanes : mediacfg->dstplanes;
	plane_size = binputqueue(type) ? &(mediacfg->sizeimagesrc[0]) : &(mediacfg->sizeimagedst[0]);

	if (planes_num == 0) {
		v4l2_klog(LOGLVL_ERROR, "ctx_id=%d:%#lx %s planes_num = 0\n", ctx->dev->deviceid, ctx->ctxid, direction);
		return;
	}

	for (i = 0; i < planes_num; i++) {
		v4l2_klog(LOGLVL_DEBUG, "ctx_id=%d:%#lx %s planes=%d plane[%d]=%u\n",
					ctx->dev->deviceid, ctx->ctxid, direction, planes_num, i, plane_size[i]);
	}

	for (i = 0; i < planes_num; i++) {
		if (plane_size[i] == 0)
			break;
	}

	/* if exception, dump all plans size */
	if (i != planes_num) {
		for (i = 0; i < planes_num; i++) {
			v4l2_klog(LOGLVL_ERROR, "ctx_id=%d:%#lx %s planes=%d plane[%d]=%u\n",
					ctx->dev->deviceid, ctx->ctxid, direction, planes_num, i, plane_size[i]);
		}
	}

}

static int vsi_dec_reqbufs(
	struct file *filp,
	void *priv,
	struct v4l2_requestbuffers *p)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	int ret = 0;
	struct vb2_queue *q;

	struct v4l2_fh *fh = filp->private_data;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	trace_vpu_reqbuf(ctx->ctxid, p->type);

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctxid=%d:%#lx, type=%u daemon is die.\n",
				ctx->dev->deviceid, ctx->ctxid, p->type);
		return -ENODEV;
	}

	if (!isvalidtype(p->type, ctx->flag)) {
		v4l2_klog(LOGLVL_ERROR, "ctxid=%#lx, type=%u invalid type.\n",
				ctx->ctxid, p->type);
		return -EINVAL;
	}

	if (!ismetabuf(p->type) && p->count != 0)
		vsi_dec_check_ctx_plane_size(ctx, p->type);

	if (ismetabuf(p->type)) {
		if (p->memory == V4L2_MEMORY_MMAP)
			return -EPERM;
		q = &ctx->meta_que;
	} else if (binputqueue(p->type))
		q = &ctx->input_que;
	else
		q = &ctx->output_que;

	ret = vb2_reqbufs(q, p);
	v4l2_klog(LOGLVL_DEBUG, "===>fh=%#llx %lx: %d ask for %d buffer, got %d:%d",
		(unsigned long long)fh, ctx->ctxid, p->type, p->count, q->num_buffers, ret);
	if (ret == 0) {
		print_queinfo(q);
		if (p->count == 0 && binputqueue(p->type)) {
			p->capabilities = V4L2_BUF_CAP_SUPPORTS_MMAP | V4L2_BUF_CAP_SUPPORTS_USERPTR | V4L2_BUF_CAP_SUPPORTS_DMABUF;
			//ctx->status = VSI_STATUS_INIT;
		}
	}
	return ret;
}

static int vsi_dec_create_bufs(struct file *filp, void *priv,
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
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_create_bufs error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
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

	v4l2_klog(LOGLVL_INFO, "%lx: %d create for %d buffer, got %d:%d",
		ctx->ctxid, create->format.type, create->count, q->num_buffers, ret);

	return ret;
}

static int vsi_dec_s_parm(struct file *filp, void *priv, struct v4l2_streamparm *parm)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);

	v4l2_klog(LOGLVL_DEBUG, "in\n");
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_s_parm error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(parm->type, ctx->flag))
		return -EINVAL;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}

	if (binputqueue(parm->type)) {
		memset(parm->parm.output.reserved, 0, sizeof(parm->parm.output.reserved));
		if (!parm->parm.output.timeperframe.denominator)
			parm->parm.output.timeperframe.denominator = 25;		//default val
		if (!parm->parm.output.timeperframe.numerator)
			parm->parm.output.timeperframe.numerator = 1;			//default val
		if (ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_denom !=
				parm->parm.output.timeperframe.numerator ||
			ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_numer !=
				parm->parm.output.timeperframe.denominator) {
			ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_denom =
				parm->parm.output.timeperframe.numerator;
			ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_numer =
				parm->parm.output.timeperframe.denominator;
			set_bit(PARAM_UPDATE_BIT, &ctx->mediacfg.decparams.io_buffer.framerate_info.head);
		}
		//according to spec, capture can use output's rate set if not set
		if (ctx->mediacfg.bcapturerateset == 0) {
			if (ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_denom !=
					ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_denom ||
				ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_numer !=
					ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_numer) {
				ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_denom =
					ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_denom;
				ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_numer =
					ctx->mediacfg.decparams.io_buffer.framerate_info.input_rate_numer;
				set_bit(PARAM_UPDATE_BIT, &ctx->mediacfg.decparams.io_buffer.framerate_info.head);
			}
		}
		parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
	} else {
		memset(parm->parm.capture.reserved, 0, sizeof(parm->parm.capture.reserved));
		if (!parm->parm.capture.timeperframe.denominator)
			parm->parm.capture.timeperframe.denominator = 25;
		if (!parm->parm.capture.timeperframe.numerator)
			parm->parm.capture.timeperframe.numerator = 1;
		if (ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_denom !=
				parm->parm.capture.timeperframe.numerator ||
			ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_numer !=
				parm->parm.capture.timeperframe.denominator) {
			ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_denom =
				parm->parm.capture.timeperframe.numerator;
			ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_numer =
				parm->parm.capture.timeperframe.denominator;
			set_bit(PARAM_UPDATE_BIT, &ctx->mediacfg.decparams.io_buffer.framerate_info.head);
		}
		ctx->mediacfg.bcapturerateset = 1;
		parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	}

	v4l2_klog(LOGLVL_DEBUG, "%d:%lx: type %d dec output frame rate %d",
		ctx->dev->deviceid, ctx->ctxid, parm->type, ctx->mediacfg.decparams.io_buffer.framerate_info.output_rate_denom);
	mutex_unlock(&ctx->ctxlock);
	return 0;
}

static int vsi_dec_g_parm(struct file *filp, void *priv, struct v4l2_streamparm *parm)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct v4l2_daemon_dec_framerate_params *pfr = &ctx->mediacfg.decparams.io_buffer.framerate_info;

	v4l2_klog(LOGLVL_DEBUG, "in\n");

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_g_parm error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(parm->type, ctx->flag))
		return -EINVAL;
	if (binputqueue(parm->type)) {
		memset(parm->parm.output.reserved, 0, sizeof(parm->parm.output.reserved));
		parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.output.timeperframe.denominator = pfr->input_rate_numer;
		parm->parm.output.timeperframe.numerator = pfr->input_rate_denom;
	} else {
		memset(parm->parm.capture.reserved, 0, sizeof(parm->parm.capture.reserved));
		parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		parm->parm.capture.timeperframe.denominator = pfr->output_rate_numer;
		parm->parm.capture.timeperframe.numerator = pfr->input_rate_denom;
	}
	return 0;
}

static int vsi_dec_g_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	v4l2_klog(LOGLVL_DEBUG, "%lx:%d", ctx->ctxid, f->type);
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_g_fmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(f->type))
		return -EINVAL;

	return vsiv4l2_getfmt(ctx, f);
}

static int vsi_dec_s_fmt(struct file *file, void *priv, struct v4l2_format *f)
{
	int ret;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}
	v4l2_klog(LOGLVL_DEBUG, "%lx:%d", ctx->ctxid, f->type);

	trace_vpu_dec_setfmt(
				ctx->ctxid,
				f->type,
				f->fmt.pix_mp.num_planes,
				f->fmt.pix_mp.plane_fmt[0].sizeimage,
				f->fmt.pix_mp.plane_fmt[1].sizeimage,
				f->fmt.pix_mp.plane_fmt[2].sizeimage);

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_s_fmt error, daemon is die\n",
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

	if (V4L2_TYPE_IS_OUTPUT(f->type) && !test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag)) {
		struct v4l2_format fc;

		memset(&fc, 0, sizeof(fc));
		fc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fc.fmt.pix_mp.pixelformat = ctx->mediacfg.outfmt_fourcc;
		fc.fmt.pix_mp.width = f->fmt.pix_mp.width;
		fc.fmt.pix_mp.height = f->fmt.pix_mp.height;
		ret = vsiv4l2_setfmt(ctx, &fc);
	}

	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_dec_querybuf(
	struct file *filp,
	void *priv,
	struct v4l2_buffer *buf)
{
	int ret;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct vb2_queue *q;
	u32 planeidx = 0;
	u32 i;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_querybuf, daemon is die\n",
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
			ctx->inputbufoffset[buf->index][0] = buf->m.planes[0].m.mem_offset;
			buf->m.planes[0].m.mem_offset = planeidx | (buf->index << 2) | (0 << 7);
			buf->m.planes[0].m.mem_offset <<= PAGE_SHIFT;
		} else if (ismetabuf(buf->type)) {
			ctx->metabufoffset[0] = buf->m.offset;
			buf->m.offset = (2 << 7);
			buf->m.offset <<= PAGE_SHIFT;
		} else {
			ctx->outputbufoffset[buf->index][0] = buf->m.planes[0].m.mem_offset;
			buf->m.planes[0].m.mem_offset = planeidx | (buf->index << 2) | (1 << 7);
			buf->m.planes[0].m.mem_offset <<= PAGE_SHIFT;
			for (i = 1; i < min(ctx->mediacfg.dstplanes, buf->length); i++) {
				ctx->outputbufoffset[buf->index][i] = buf->m.planes[i].m.mem_offset;
				buf->m.planes[i].m.mem_offset = i | (buf->index << 2) | (1 << 7);
				buf->m.planes[i].m.mem_offset <<= PAGE_SHIFT;
			}
		}
	}
	return ret;
}

static int vsi_dec_qbuf(struct file *filp, void *priv, struct v4l2_buffer *buf)
{
	int ret, i;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct video_device *vdev = ctx->dev->vdec;
	struct v4l2_plane tmp_planes[MAX_PLANENO];
	struct vb2_buffer *vb = NULL;
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

	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%d:%d:%d", ctx->dev->deviceid, ctx->ctxid, buf->type, buf->index, buf->length);
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_qbuf error, buf->type=%d, index=%d daemon is die\n",
		ctx->dev->deviceid, ctx->ctxid, buf->type, buf->index);
		return -ENODEV;
	}
	if (!isvalidtype(buf->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(buf->type)) {
		if (vb2_is_streaming(&ctx->output_que))
			return -EBUSY;
		//ctx->mediacfg.metabufinfo.busMetaBuf is left to bufqueue
		ctx->mediacfg.metabufinfo.metabufsize = buf->length;
		ret = vb2_qbuf(&ctx->meta_que, vdev->v4l2_dev->mdev, buf);
		if (ret == 0)
			ret = vb2_streamon(&ctx->meta_que, buf->type);
		else
			v4l2_klog(LOGLVL_ERROR, "ctx->id=%#lx vb2_qbuf error, ret=%d. type=%d index=%d byteused=%d\n",
				ctx->ctxid, ret, buf->type, buf->index, buf->bytesused);
		return ret;
	}

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	if (binputqueue(buf->type))
		set_bit(CTX_FLAG_SRCBUF_BIT, &ctx->flag);
	if (binputqueue(buf->type) && buf->m.planes[0].bytesused == 0 &&
		ctx->status == DEC_STATUS_DECODING) {
		if (test_and_clear_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag)) {
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_CMD_STOP, NULL);
			ctx->status = DEC_STATUS_DRAINING;
		} else
			ret = 0;
		mutex_unlock(&ctx->ctxlock);
		return ret;
	}

	memcpy(tmp_planes, buf->m.planes, sizeof(struct v4l2_plane) * buf->length);
	if (!binputqueue(buf->type)) {
		ret = vb2_qbuf(&ctx->output_que, vdev->v4l2_dev->mdev, buf);
		/* fuzz report ubsan out of bounds */
		if (ret == 0) {
			// Refill offset of each plane, otherwise offset is 0
			vb = ctx->output_que.bufs[buf->index];
			for (i = 0; i < vb->num_planes; i++)
				vb->planes[i].data_offset =  tmp_planes[i].data_offset;
		}
	} else {
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
	}

	if (ret == 0 && (ctx->status == VSI_STATUS_INIT || ctx->status == DEC_STATUS_SEEK)
		&& ctx->input_que.queued_count >= ctx->input_que.min_buffers_needed
		&& vb2_is_streaming(&ctx->input_que)) {
		v4l2_klog(LOGLVL_DEBUG, "%lx: start streaming", ctx->ctxid);
		ctx->status = DEC_STATUS_DECODING;
		ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMON_OUTPUT, NULL);
	}
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_dec_dec2drain(struct vsi_v4l2_ctx *ctx)
{
	int ret = 0;

	if (ctx->status == DEC_STATUS_DECODING &&
		test_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag)) {
		ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_CMD_STOP, NULL);
		ctx->status = DEC_STATUS_DRAINING;
	}
	return ret;
}

static int vsi_dec_streamon(struct file *filp, void *priv, enum v4l2_buf_type type)
{
	int ret = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_stream_on error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(type))
		return 0;

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	v4l2_klog(LOGLVL_DEBUG, "%d:%lx %d in status %d", ctx->dev->deviceid, ctx->ctxid, type, ctx->status);
	if (!binputqueue(type)) {
		ret = vb2_streamon(&ctx->output_que, type);
		if (ret == 0) {
			if (ctx->status != DEC_STATUS_SEEK && ctx->status != DEC_STATUS_ENDSTREAM)
				ctx->status = DEC_STATUS_DECODING;
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMON_CAPTURE, NULL);
			if (ret == 0)
				vsi_dec_dec2drain(ctx);
			if (test_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag)) {
				if (list_empty(&ctx->output_que.done_list)) {
					struct vb2_buffer *vb = ctx->output_que.bufs[0];

					vb->planes[0].bytesused = 0;
					ctx->lastcapbuffer_idx = 0;
					vb->state = VB2_BUF_STATE_ACTIVE;
					vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
				}
			}
		}
		printbufinfo(&ctx->output_que);
	} else {
		ret = vb2_streamon(&ctx->input_que, type);
		if (ret == 0 && ctx->input_que.queued_count >= ctx->input_que.min_buffers_needed) {
			ctx->status = DEC_STATUS_DECODING;
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMON_OUTPUT, NULL);
			if (ret == 0)
				vsi_dec_dec2drain(ctx);
		}
		printbufinfo(&ctx->input_que);
	}
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_dec_checkctx_srcbuf(struct vsi_v4l2_ctx *ctx)
{
	int ret = 0;

	if (ctx->queued_srcnum == 0 || ctx->error < 0)
		ret = 1;
	return ret;
}

static void vsi_dec_update_reso(struct vsi_v4l2_ctx *ctx)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	s32 i;

	pcfg->decparams.dec_info.dec_info = pcfg->decparams_bkup.dec_info.dec_info;
	pcfg->decparams.dec_info.io_buffer.srcwidth = pcfg->decparams_bkup.io_buffer.srcwidth;
	pcfg->decparams.dec_info.io_buffer.srcheight = pcfg->decparams_bkup.io_buffer.srcheight;
	pcfg->decparams.dec_info.io_buffer.output_width = pcfg->decparams_bkup.io_buffer.output_width;
	pcfg->decparams.dec_info.io_buffer.output_height = pcfg->decparams_bkup.io_buffer.output_height;
	pcfg->decparams.dec_info.io_buffer.output_wstride = pcfg->decparams_bkup.io_buffer.output_wstride;
	pcfg->bytesperline = pcfg->decparams_bkup.io_buffer.output_wstride;
	//pcfg->orig_dpbsize = pcfg->sizeimagedst_bkup;
	pcfg->src_pixeldepth = pcfg->decparams_bkup.dec_info.dec_info.bit_depth;
	pcfg->minbuf_4output = pcfg->minbuf_4capture = pcfg->minbuf_4output_bkup;
	for (i = 0; i < MAX_PLANENO; i++)
		pcfg->sizeimagedst[i] = pcfg->sizeimagedst_bkup[i];
	set_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag);
}

static int vsi_dec_streamoff(
	struct file *file,
	void *priv,
	enum v4l2_buf_type type)
{
	int ret;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(priv);
	struct vb2_queue *q;
	struct x_pp_buf_node *pos_node = NULL;
	struct x_pp_buf_node *next_node = NULL;
	ktime_t start, end, input_start, output_start;
	ktime_t input_end, output_end;
	u64 ms, input_ms, output_ms;

	start = ktime_get();
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_stream_off error, type=%d, daemon is die\n",
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

	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	v4l2_klog(LOGLVL_DEBUG, "%lx %d:%d:%d", ctx->ctxid, type, ctx->status, ctx->queued_srcnum);
	if (!vb2_is_streaming(q) && ctx->status == VSI_STATUS_INIT) {
		mutex_unlock(&ctx->ctxlock);
		return 0;
	}

	if (binputqueue(type)) {
		ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMOFF_OUTPUT, NULL);
		ctx->status = DEC_STATUS_SEEK;
	} else {
		ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_STREAMOFF_CAPTURE, NULL);
		if (ctx->status != DEC_STATUS_SEEK && ctx->status != DEC_STATUS_ENDSTREAM)
			ctx->status = DEC_STATUS_STOPPED;
	}
	if (ret < 0) {
		mutex_unlock(&ctx->ctxlock);
		return -EFAULT;
	}
	if (binputqueue(type)) {
		if (!(test_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag))) {
			//here we need to wait all OUTPUT buffer returned
			mutex_unlock(&ctx->ctxlock);
			input_start = ktime_get();
			ret = wait_event_interruptible_timeout(ctx->retbuf_queue,
				vsi_dec_checkctx_srcbuf(ctx) != 0, msecs_to_jiffies(XRING_STREAM_OFF_TIMEOUT));
			if (!ret) {
				v4l2_klog(LOGLVL_WARNING, "%lx wait input buffer return by timeout.\n", ctx->ctxid);
			} else if (ret == -ERESTARTSYS)
				v4l2_klog(LOGLVL_WARNING, "%lx wait input buffer return by signal.\n", ctx->ctxid);
			input_end = ktime_get();
			input_ms = ktime_to_ms(ktime_sub(input_end, input_start));
			if (input_ms > THRESHOLD_MS)
				v4l2_klog(LOGLVL_WARNING, "===> %lx input_ms=%llums\n", ctx->ctxid, input_ms);

			if (mutex_lock_killable(&ctx->ctxlock)) {
				v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
					ctx->dev->deviceid, ctx->ctxid);
				return -EBUSY;
			}
		}
		clear_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag);
		clear_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag);
		clear_bit(CTX_FLAG_SRCBUF_BIT, &ctx->flag);
		if (test_and_clear_bit(CTX_FLAG_DELAY_SRCCHANGED_BIT, &ctx->flag)) {
			vsi_dec_update_reso(ctx);
			vsi_v4l2_send_reschange(ctx);
			v4l2_klog(LOGLVL_INFO, "%lx send delayed src change in streamoff", ctx->ctxid);
		}
	} else {
		mutex_unlock(&ctx->ctxlock);
		output_start = ktime_get();
		ret = wait_event_interruptible_timeout(ctx->capoffdone_queue,
			vsi_checkctx_capoffdone(ctx) != 0, msecs_to_jiffies(XRING_STREAM_OFF_TIMEOUT));
		if (!ret) {
			v4l2_klog(LOGLVL_WARNING, "%lx wait output streamoff return by timeout.\n", ctx->ctxid);
		} else if (ret == -ERESTARTSYS)
			v4l2_klog(LOGLVL_WARNING, "%lx wait output streamoff return by signal\n", ctx->ctxid);
		output_end = ktime_get();
		output_ms = ktime_to_ms(ktime_sub(output_end, output_start));
		if (output_ms > THRESHOLD_MS)
			v4l2_klog(LOGLVL_WARNING, "===> %lx output_ms=%llums\n", ctx->ctxid, output_ms);

		if (mutex_lock_killable(&ctx->ctxlock)) {
			v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
				ctx->dev->deviceid, ctx->ctxid);
			return -EBUSY;
		}
		ctx->buffed_capnum = 0;
		ctx->buffed_cropcapnum = 0;

		if (!list_empty(&ctx->x_pp_list)) {
			list_for_each_entry_safe(pos_node, next_node, &ctx->x_pp_list, node) {
				list_del(&pos_node->node);
				dma_buf_put(pos_node->dbuf);
				v4l2_klog(LOGLVL_DEBUG, "delelte node(%p) ,dbuf(%p), index(%u) from list\n",
								pos_node, pos_node->dbuf, pos_node->index);
				vfree(pos_node);
			}
		}
	}
	return_all_buffers(q, VB2_BUF_STATE_DONE, 1);
	ret = vb2_streamoff(q, type);
	mutex_unlock(&ctx->ctxlock);

	end = ktime_get();
	ms = ktime_to_ms(ktime_sub(end, start));
	if (ms > THRESHOLD_MS)
		v4l2_klog(LOGLVL_WARNING, "===>%s %lx total=%llums\n",
			binputqueue(type)?"input":"output", ctx->ctxid, ms);

	return ret;
}

static int vsi_dec_dqbuf(struct file *file, void *priv, struct v4l2_buffer *p)
{
	int ret = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vb2_queue *q;
	struct vb2_buffer *vb;
	struct vsi_vpu_buf *vsibuf;
	struct x_pp_buf_node *pos_node = NULL;
	struct x_pp_buf_node *next_node = NULL;
	struct v4l2_vsi_stream_meta_info_dec *pmeta_dec;
	struct v4l2_vsi_dec_meta_info_CAPTURE *pmeta_capture;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_dqbuf error, type=%d, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid, p->type);
		return -ENODEV;
	}
	if (!isvalidtype(p->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(p->type)) {
		if (vb2_is_streaming(&ctx->output_que))
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

	if (!vb2_is_streaming(q))
		return -EPIPE;

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
		if (!binputqueue(p->type)) {
			pmeta_dec = (struct v4l2_vsi_stream_meta_info_dec *)ctx->pmeta;
			pmeta_capture = (struct v4l2_vsi_dec_meta_info_CAPTURE *)((u8 *)ctx->pmeta +
			sizeof(struct v4l2_vsi_stream_meta_info_dec) + sizeof(struct v4l2_vsi_dec_meta_info_OUTPUT)
			* pmeta_dec->OUTPUT_num + sizeof(struct v4l2_vsi_dec_meta_info_CAPTURE) * p->index);

			if (pmeta_capture->valid & META_VALID_MASK_PPOUT_LAYOUT) {
				if (pmeta_capture->pp_layout.num > 1 && (!list_empty(&ctx->x_pp_list))) {
					list_for_each_entry_safe(pos_node, next_node, &ctx->x_pp_list, node) {
						if (pos_node->index == p->index) {
							list_del(&pos_node->node);
							dma_buf_put(pos_node->dbuf);
							v4l2_klog(LOGLVL_DEBUG, "free pos_node(%p), dbuf(%p), index(%u)\n",
							pos_node, pos_node->dbuf, pos_node->index);
							vfree(pos_node);
							break;
						}
					}
				}
			}
			pmeta_dec = NULL;
			pmeta_capture = NULL;
			clear_bit(BUF_FLAG_DONE, &ctx->vbufflag[p->index]);
			ctx->buffed_capnum--;
			ctx->buffed_cropcapnum--;
		} else
			clear_bit(BUF_FLAG_DONE, &ctx->srcvbufflag[p->index]);
		if (ctx->status != DEC_STATUS_ENDSTREAM &&
			!(test_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag)) &&
			 p->m.planes[0].bytesused == 0) {
			mutex_unlock(&ctx->ctxlock);
			return -EAGAIN;
		}
	}
	if (!binputqueue(p->type)) {
		p->reserved = ctx->rfc_luma_offset[p->index];
		p->reserved2 = ctx->rfc_chroma_offset[p->index];
		v4l2_klog(LOGLVL_DEBUG, "rfc offest update=%x:%x", p->reserved, p->reserved2);

		if (p->m.planes[0].bytesused == 0 && (ctx->status == DEC_STATUS_ENDSTREAM || test_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag))) {
			ctx->status = DEC_STATUS_ENDSTREAM;
			clear_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag);
			v4l2_klog(LOGLVL_INFO, "send eos flag");
		} else if (test_bit(CTX_FLAG_DELAY_SRCCHANGED_BIT, &ctx->flag) && ctx->buffed_capnum == 0) {
			vsi_dec_update_reso(ctx);
			vsi_v4l2_send_reschange(ctx);
			clear_bit(CTX_FLAG_DELAY_SRCCHANGED_BIT, &ctx->flag);
			v4l2_klog(LOGLVL_INFO, "%lx send delayed src change", ctx->ctxid);
		} else if (test_and_clear_bit(BUF_FLAG_CROPCHANGE, &ctx->vbufflag[p->index])) {
			struct v4l2_event event;

			if (update_and_removecropinfo(ctx)) {
				struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
				struct v4l2_daemon_extra_info *extra_info = (struct v4l2_daemon_extra_info *)event.u.data;

				memset((void *)&event, 0, sizeof(struct v4l2_event));
				event.type = V4L2_EVENT_CROPCHANGE;
				memcpy(extra_info,
						&pcfg->decparams.pic_info.pic_info.extra_info,
						sizeof(struct v4l2_daemon_extra_info));

				v4l2_event_queue_fh(&ctx->fh, &event);
			}
			v4l2_klog(LOGLVL_INFO, "send delayed crop change at buf %d", p->index);
		}
		p->field = V4L2_FIELD_NONE;
	}
	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%d:%d:%x:%d", ctx->dev->deviceid, ctx->ctxid, p->type, p->index, p->flags, p->m.planes[0].bytesused);
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_dec_prepare_buf(
	struct file *file,
	void *priv,
	struct v4l2_buffer *p)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vb2_queue *q;
	struct video_device *vdev = ctx->dev->vdec;

	v4l2_klog(LOGLVL_DEBUG, "in\n");

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_prepare_buf error, daemon is die\n",
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

static int vsi_dec_expbuf(
	struct file *file,
	void *priv,
	struct v4l2_exportbuffer *p)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vb2_queue *q;

	v4l2_klog(LOGLVL_DEBUG, "in\n");
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_exbuf error, daemon is die\n",
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

static int vsi_dec_try_fmt(struct file *file, void *prv, struct v4l2_format *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	v4l2_klog(LOGLVL_DEBUG, "in\n");

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_tryfmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(f->type))
		return -EINVAL;

	if (vsi_find_format(ctx, f) == NULL)
		return -EINVAL;
	vsi_dec_getvui(f, &ctx->mediacfg.decparams.dec_info.dec_info);
	return 0;
}

static int vsi_dec_enum_fmt(struct file *file, void *prv, struct v4l2_fmtdesc *f)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vsi_video_fmt *pfmt;
	int braw = brawfmt(ctx->flag, f->type);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_enum_fmt error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (!isvalidtype(f->type, ctx->flag))
		return -EINVAL;
	if (ismetabuf(f->type))
		return -EINVAL;

	pfmt = vsi_enum_dec_format(f->index, braw, ctx);
	if (pfmt == NULL)
		return -EINVAL;

	if (pfmt->name && strlen(pfmt->name))
		strscpy(f->description, pfmt->name, sizeof(f->description));
	f->pixelformat = pfmt->fourcc;
	f->flags = pfmt->flag;
	v4l2_klog(LOGLVL_DEBUG, "%d:%d:%x", f->index, f->type, pfmt->fourcc);
	return 0;
}

static int vsi_dec_tryfmt_meta(struct file *file, void *fh, struct v4l2_format *f)
{
	if (f->type != V4L2_BUF_TYPE_META_CAPTURE ||
		f->fmt.meta.dataformat != V4L2_META_FMT_VSIDEC)
		return -EINVAL;
	return 0;
}

static int vsi_dec_setfmt_meta(struct file *file, void *fh, struct v4l2_format *f)
{
	int ret;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	ret = vsi_dec_tryfmt_meta(file, fh, f);
	if (ret)
		return ret;
	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_setfmt_meta error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}

	f->fmt.meta.dataformat = V4L2_META_FMT_VSIDEC;
	f->fmt.meta.buffersize = ctx->output_que.num_buffers * sizeof(struct v4l2_vsi_dec_meta_info_CAPTURE) +
									ctx->input_que.num_buffers * sizeof(struct v4l2_vsi_dec_meta_info_OUTPUT) +
									sizeof(struct v4l2_vsi_stream_meta_info_dec);
	return 0;
}

static int vsi_dec_enumfmt_meta(struct file *file, void *fh, struct v4l2_fmtdesc *f)
{
	static const char *decmeta_name = "dec_meta";

	if (f->index > 0 || f->type != V4L2_BUF_TYPE_META_CAPTURE)
		return -EINVAL;
	f->flags = 0;
	f->pixelformat = V4L2_META_FMT_VSIDEC;
	strscpy(f->description, decmeta_name, sizeof(f->description));
	return 0;
}

static int vsi_dec_get_selection(struct file *file, void *prv, struct v4l2_selection *s)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_get_selection error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	switch (s->target) {
	case V4L2_SEL_TGT_COMPOSE:
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_COMPOSE_PADDED:
		s->r.left = pcfg->decparams.dec_info.dec_info.visible_rect.left;
		s->r.top = pcfg->decparams.dec_info.dec_info.visible_rect.top;
		s->r.width = pcfg->decparams.dec_info.dec_info.visible_rect.width;
		s->r.height = pcfg->decparams.dec_info.dec_info.visible_rect.height;
		break;
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
		s->r.left = 0;
		s->r.top = 0;
		s->r.width = pcfg->decparams.dec_info.io_buffer.output_width;
		s->r.height = pcfg->decparams.dec_info.io_buffer.output_height;
		break;
	default:
		return -EINVAL;
	}
	v4l2_klog(LOGLVL_DEBUG, "%lx:%d,%d,%d,%d",
		ctx->ctxid,  s->r.left, s->r.top, s->r.width, s->r.height);

	return 0;
}

static int vsi_dec_subscribe_event(
	struct v4l2_fh *fh,
	const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_CTRL:
		return v4l2_ctrl_subscribe_event(fh, sub);
	case V4L2_EVENT_SKIP:
		return v4l2_event_subscribe(fh, sub, 16, NULL);
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subscribe(fh, sub);
	case V4L2_EVENT_EOS:
	case V4L2_EVENT_CODEC_ERROR:
	case V4L2_EVENT_CROPCHANGE:
	case V4L2_EVENT_INVALID_OPTION:
		return v4l2_event_subscribe(fh, sub, 0, NULL);
	default:
//by spec should return error, but current gst will fail, so leave it here
		return 0;//-EINVAL;
	}
}

static int vsi_dec_handlestop_unspec(struct vsi_v4l2_ctx *ctx)
{
	//some unexpected condition fro CTS, not quite conformant to spec
	if ((ctx->status == VSI_STATUS_INIT && !test_bit(CTX_FLAG_DAEMONLIVE_BIT, &ctx->flag)) ||
		ctx->status == DEC_STATUS_STOPPED) {
		clear_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag);
		vsi_v4l2_sendeos(ctx);
		return 0;
	}
	if (ctx->status == DEC_STATUS_SEEK && !test_bit(CTX_FLAG_SRCBUF_BIT, &ctx->flag)) {
		vsi_v4l2_sendeos(ctx);
		return 0;
	}
	return 0;
}

static int vsi_dec_try_decoder_cmd(struct file *file, void *fh, struct v4l2_decoder_cmd *cmd)
{
	switch (cmd->cmd) {
	case V4L2_DEC_CMD_STOP:
		cmd->stop.pts = 0;
		break;
	case V4L2_DEC_CMD_START:
		cmd->start.speed = 0;
		cmd->start.format = V4L2_DEC_START_FMT_NONE;
		break;
	case V4L2_DEC_CMD_RESET:
		break;
	case V4L2_DEC_CMD_PAUSE:
	case V4L2_DEC_CMD_RESUME:
	default:
		return -EINVAL;
	}
	cmd->flags = 0;

	return 0;
}

int vsi_dec_decoder_cmd(struct file *file, void *fh, struct v4l2_decoder_cmd *cmd)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	int ret = 0;

	v4l2_klog(LOGLVL_INFO, "%lx:%d in state %d:%d", ctx->ctxid,
		cmd->cmd, ctx->status, vb2_is_streaming(&ctx->output_que));

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_decoder_cmd error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (mutex_lock_killable(&ctx->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -EBUSY;
	}
	switch (cmd->cmd) {
	case V4L2_DEC_CMD_STOP:
		set_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag);
		if (ctx->status == DEC_STATUS_DECODING) {
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_CMD_STOP, NULL);
			ctx->status = DEC_STATUS_DRAINING;
		} else
			ret = vsi_dec_handlestop_unspec(ctx);
		break;
	case V4L2_DEC_CMD_START:
		if (ctx->status == DEC_STATUS_STOPPED) {
			ctx->status = DEC_STATUS_DECODING;
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_CMD_START, cmd);
		}
		break;
	case V4L2_DEC_CMD_RESET:
		ret = vsi_v4l2_reset_ctx(ctx);
		break;
	case V4L2_DEC_CMD_PAUSE:
	case V4L2_DEC_CMD_RESUME:
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&ctx->ctxlock);
	return ret;
}

static int vsi_dec_enum_framesizes(struct file *file, void *priv,
				  struct v4l2_frmsizeenum *fsize)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	struct v4l2_format fmt;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_enum_framesizes error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	if (fsize->index != 0)		//only stepwise
		return -EINVAL;

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	fmt.fmt.pix_mp.pixelformat = fsize->pixel_format;
	if (vsi_find_format(ctx,  &fmt) != NULL)
		vsi_enum_decfsize(fsize, ctx->mediacfg.infmt_fourcc);
	else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		fmt.fmt.pix_mp.pixelformat = fsize->pixel_format;
		if (vsi_find_format(ctx,  &fmt) == NULL)
			return -EINVAL;
		vsi_enum_decfsize(fsize, fsize->pixel_format);
	}
	v4l2_klog(LOGLVL_DEBUG, "%x->%d:%d:%d:%d:%d:%d", fsize->pixel_format,
		fsize->stepwise.min_width, fsize->stepwise.max_width, fsize->stepwise.min_height,
		fsize->stepwise.max_height, fsize->stepwise.step_width, fsize->stepwise.step_height);
	return 0;
}

static const struct v4l2_ioctl_ops vsi_dec_ioctl = {
	.vidioc_querycap = vsi_dec_querycap,
	.vidioc_reqbufs             = vsi_dec_reqbufs,
	.vidioc_create_bufs         = vsi_dec_create_bufs,
	.vidioc_prepare_buf         = vsi_dec_prepare_buf,
	//create_buf can be provided now since we don't know buf type in param
	.vidioc_querybuf            = vsi_dec_querybuf,
	.vidioc_qbuf                = vsi_dec_qbuf,
	.vidioc_dqbuf               = vsi_dec_dqbuf,
	.vidioc_streamon        = vsi_dec_streamon,
	.vidioc_streamoff       = vsi_dec_streamoff,
	//.vidioc_g_fmt_vid_cap = vsi_dec_g_fmt,
	.vidioc_g_fmt_vid_cap_mplane = vsi_dec_g_fmt,
	//.vidioc_s_fmt_vid_cap = vsi_dec_s_fmt,
	.vidioc_s_fmt_vid_cap_mplane = vsi_dec_s_fmt,
	.vidioc_expbuf = vsi_dec_expbuf,		//this is used to export MMAP ptr as prime fd to user space app

	//.vidioc_g_fmt_vid_out = vsi_dec_g_fmt,
	.vidioc_g_fmt_vid_out_mplane = vsi_dec_g_fmt,
	//.vidioc_s_fmt_vid_out = vsi_dec_s_fmt,
	.vidioc_s_fmt_vid_out_mplane = vsi_dec_s_fmt,
	//.vidioc_try_fmt_vid_cap = vsi_dec_try_fmt,
	.vidioc_try_fmt_vid_cap_mplane = vsi_dec_try_fmt,
	//.vidioc_try_fmt_vid_out = vsi_dec_try_fmt,
	.vidioc_try_fmt_vid_out_mplane = vsi_dec_try_fmt,

	.vidioc_enum_fmt_vid_cap = vsi_dec_enum_fmt,
	.vidioc_enum_fmt_vid_out = vsi_dec_enum_fmt,

	//.vidioc_g_fmt_vid_out = vsi_dec_g_fmt,
	.vidioc_g_fmt_vid_out_mplane = vsi_dec_g_fmt,

	.vidioc_g_selection = vsi_dec_get_selection,		//VIDIOC_G_SELECTION, VIDIOC_G_CROP

	.vidioc_subscribe_event = vsi_dec_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,

	.vidioc_try_decoder_cmd = vsi_dec_try_decoder_cmd,
	.vidioc_decoder_cmd = vsi_dec_decoder_cmd,
	.vidioc_enum_framesizes = vsi_dec_enum_framesizes,

	.vidioc_try_fmt_meta_cap = vsi_dec_tryfmt_meta,
	.vidioc_s_fmt_meta_cap = vsi_dec_setfmt_meta,
	.vidioc_g_fmt_meta_cap = vsi_dec_setfmt_meta,	//meta set/get are identical since it's unchangeable
	.vidioc_enum_fmt_meta_cap = vsi_dec_enumfmt_meta,

	.vidioc_s_parm		= vsi_dec_s_parm,
	.vidioc_g_parm		= vsi_dec_g_parm,
};

/*setup buffer information before real allocation*/
static int vsi_dec_queue_setup(
	struct vb2_queue *vq,
	unsigned int *nbuffers,
	unsigned int *nplanes,
	unsigned int sizes[],
	struct device *alloc_devs[])
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(vq->drv_priv);
	int i, ret;

	ret = vsiv4l2_buffer_config(ctx, vq->type, nbuffers, nplanes, sizes);
	v4l2_klog(LOGLVL_INFO, "%d:%lx:%d,%d,%d", ctx->dev->deviceid, ctx->ctxid, *nbuffers, *nplanes, sizes[0]);

	if (ret == 0) {
		for (i = 0; i < *nplanes; i++)
			alloc_devs[i] = ctx->dev->dev;
	}
	return ret;
}

static void vsi_dec_buf_queue(struct vb2_buffer *vb)
{
	struct vb2_queue *vq = vb->vb2_queue;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(vq->drv_priv);
	struct vsi_vpu_buf *vsibuf;
	struct v4l2_vsi_stream_meta_info_dec *pmeta_dec;
	struct v4l2_vsi_dec_meta_info_CAPTURE *pmeta_capture;
	struct x_pp_buf_node *ppnode;
	int ret;

	v4l2_klog(LOGLVL_DEBUG, "%d:%d", vq->type, vb->index);
	vsibuf = vb_to_vsibuf(vb);
	if (ismetabuf(vb->type)) {
		//so meta buf streamon must be before input and output queue stream on
		ctx->mediacfg.metabufinfo.busMetaBuf = getbusaddr_meta(ctx, vb);
		ctx->pmeta = getvaddr_meta(ctx, vb);
		return;
	} else if (!binputqueue(vq->type)) {
		set_bit(BUF_FLAG_QUEUED, &ctx->vbufflag[vb->index]);
		list_add_tail(&vsibuf->list, &ctx->output_list);
		pmeta_dec = (struct v4l2_vsi_stream_meta_info_dec *)ctx->pmeta;
		pmeta_capture = (struct v4l2_vsi_dec_meta_info_CAPTURE *)((u8 *)ctx->pmeta +
			sizeof(struct v4l2_vsi_stream_meta_info_dec) + sizeof(struct v4l2_vsi_dec_meta_info_OUTPUT)
			* pmeta_dec->OUTPUT_num + sizeof(struct v4l2_vsi_dec_meta_info_CAPTURE) * vb->index);
		if (pmeta_capture->valid & META_DEC_VALID_MASK_PP_BUF) {
			if ((pmeta_capture->pp_buf.num == 1) && (pmeta_capture->pp_buf.info[0].ppu_chn == 1)) {
				ppnode = (struct x_pp_buf_node *)vmalloc(sizeof(struct x_pp_buf_node));
				if (ppnode) {
					ppnode->index = vb->index;
					ppnode->dbuf = dma_buf_get(pmeta_capture->pp_buf.info[0].bus_addr[0]);
					ppnode->size = pmeta_capture->pp_buf.info[0].size[0];
					list_add_tail(&ppnode->node, &ctx->x_pp_list);
					v4l2_klog(LOGLVL_DEBUG, "insert pp1 node to list, index(%u), fd(%llu), dbuf(%p)",
						vb->index, (unsigned long long)pmeta_capture->pp_buf.info[0].bus_addr[0],
						ppnode->dbuf);
				}
			}
		}
	} else {
		set_bit(BUF_FLAG_QUEUED, &ctx->srcvbufflag[vb->index]);
		list_add_tail(&vsibuf->list, &ctx->input_list);
		ctx->queued_srcnum++;
	}
	ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_BUF_RDY, vb);
}

static int vsi_dec_buf_init(struct vb2_buffer *vb)
{
	return 0;
}

static int vsi_dec_buf_prepare(struct vb2_buffer *vb)
{
	return 0;
}

static int vsi_dec_start_streaming(struct vb2_queue *q, unsigned int count)
{
	return 0;
}
static void vsi_dec_stop_streaming(struct vb2_queue *vq)
{
}

static void vsi_dec_buf_finish(struct vb2_buffer *vb)
{
}

static void vsi_dec_buf_cleanup(struct vb2_buffer *vb)
{
}

static void vsi_dec_buf_wait_finish(struct vb2_queue *vq)
{
	vb2_ops_wait_finish(vq);
}

static void vsi_dec_buf_wait_prepare(struct vb2_queue *vq)
{
	vb2_ops_wait_prepare(vq);
}

static int vsi_dec_buf_out_validate(struct vb2_buffer *vb)
{
	return 0;
}

static void vsi_dec_buf_request_complete(struct vb2_buffer *vb)
{
}

static struct vb2_ops vsi_dec_qops = {
	.queue_setup = vsi_dec_queue_setup,
	.wait_prepare = vsi_dec_buf_wait_prepare,	/*these two are just mutex protection for done_que*/
	.wait_finish = vsi_dec_buf_wait_finish,
	.buf_init = vsi_dec_buf_init,
	.buf_prepare = vsi_dec_buf_prepare,
	.buf_finish = vsi_dec_buf_finish,
	.buf_cleanup = vsi_dec_buf_cleanup,
	.start_streaming = vsi_dec_start_streaming,
	.stop_streaming = vsi_dec_stop_streaming,
	.buf_queue = vsi_dec_buf_queue,
	.buf_out_validate = vsi_dec_buf_out_validate,
	.buf_request_complete = vsi_dec_buf_request_complete,
};

static int vsi_v4l2_dec_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct vsi_v4l2_ctx *ctx = ctrl_to_ctx(ctrl);

	v4l2_klog(LOGLVL_DEBUG, "%x=%d", ctrl->id, ctrl->val);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_s_ctrl error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
		return -ENODEV;
	}
	switch (ctrl->id) {
	case V4L2_CID_DIS_REORDER:
		ctx->mediacfg.decparams.io_buffer.no_reordering_decoding = ctrl->val;
		break;
	case V4L2_CID_SECUREMODE:
		ctx->mediacfg.decparams.io_buffer.securemode_on = ctrl->val;
		break;
	case V4L2_CID_COMPRESSOR_MODE:
		ctx->mediacfg.decparams.io_buffer.compressor_mode = ctrl->val;
		break;
	case V4L2_CID_DIVX_VERSION:
		ctx->mediacfg.decparams.io_buffer.divx_version = ctrl->val;
		break;
	case V4L2_CID_ADDR_OFFSET:
		if (ctrl->p_new.p_s64)
			ctx->addr_offset = *ctrl->p_new.p_s64;
		break;
	default:
		return 0;
	}
	return 0;
}

static int vsi_v4l2_dec_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct vsi_v4l2_ctx *ctx = ctrl_to_ctx(ctrl);
	struct vsi_v4l2_dev_info *hwinfo;

	v4l2_klog(LOGLVL_DEBUG, "%x", ctrl->id);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		return -ENODEV;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, dec_g_volatile_ctrl error, daemon is die\n",
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
	case V4L2_CID_HDR10META:
		if (ctrl->p_new.p) {
			if (!test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag))
				memset(ctrl->p_new.p, 0, sizeof(struct v4l2_hdr10_meta));
			else
				memcpy(ctrl->p_new.p,
					&ctx->mediacfg.decparams.dec_info.dec_info.vpu_hdr10_meta,
					sizeof(struct v4l2_hdr10_meta));
		}
		break;
	case V4L2_CID_COMPRESSOR_CAPS:
		hwinfo = vsiv4l2_get_hwinfo();
		ctrl->val = hwinfo->compressor_fmt;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/********* for ext ctrl *************/
#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
static bool vsi_dec_ctrl_equal(const struct v4l2_ctrl *ctrl,
		      union v4l2_ctrl_ptr ptr1,
		      union v4l2_ctrl_ptr ptr2)
#else
static bool vsi_dec_ctrl_equal(const struct v4l2_ctrl *ctrl, u32 idx,
		      union v4l2_ctrl_ptr ptr1,
		      union v4l2_ctrl_ptr ptr2)
#endif
{
	//always update now, fix it later
	return 0;
}

static void vsi_dec_ctrl_init(const struct v4l2_ctrl *ctrl, u32 idx,
		     union v4l2_ctrl_ptr ptr)
{
	void *p = ptr.p + idx * ctrl->elem_size;

	memset(p, 0, ctrl->elem_size);
}

static void vsi_dec_ctrl_log(const struct v4l2_ctrl *ctrl)
{
	//do nothing now
}

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
static int vsi_dec_ctrl_validate(const struct v4l2_ctrl *ctrl,
			union v4l2_ctrl_ptr ptr)
#else
static int vsi_dec_ctrl_validate(const struct v4l2_ctrl *ctrl, u32 idx,
			union v4l2_ctrl_ptr ptr)
#endif
{
	//always true
	return 0;
}

static const struct v4l2_ctrl_type_ops vsi_dec_type_ops = {
	.equal = vsi_dec_ctrl_equal,
	.init = vsi_dec_ctrl_init,
	.log = vsi_dec_ctrl_log,
	.validate = vsi_dec_ctrl_validate,
};
/********* for ext ctrl *************/

static const struct v4l2_ctrl_ops vsi_dec_ctrl_ops = {
	.s_ctrl = vsi_v4l2_dec_s_ctrl,
	.g_volatile_ctrl = vsi_v4l2_dec_g_volatile_ctrl,
};

static struct v4l2_ctrl_config vsi_v4l2_dec_ctrl_defs[] = {
	{
		.ops = &vsi_dec_ctrl_ops,
		.id = V4L2_CID_DIS_REORDER,
		.name = "frame disable reorder ctrl",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_dec_ctrl_ops,
		.type_ops = &vsi_dec_type_ops,
		.id = V4L2_CID_HDR10META,
		.name = "vsi get 10bit meta",
		.type = VSI_V4L2_CMPTYPE_HDR10META,
		.flags = V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_READ_ONLY,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
		.elem_size = sizeof(struct v4l2_hdr10_meta),
	},
	/* kernel defined controls */
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
		.ops = &vsi_dec_ctrl_ops,
		.id = V4L2_CID_SECUREMODE,
		.name = "en/disable secure mode",
		.type = V4L2_CTRL_TYPE_BOOLEAN,
		.min = 0,
		.max = 1,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_dec_ctrl_ops,
		.type_ops = &vsi_dec_type_ops,
		.id = V4L2_CID_COMPRESSOR_CAPS,
		.name = "vsi get compressor ability",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.flags = V4L2_CTRL_FLAG_VOLATILE,
		.min = 0,
		.max = 0xffff,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_dec_ctrl_ops,
		.id = V4L2_CID_COMPRESSOR_MODE,
		.name = "vsi set specify compressor mode",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 0xffff,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_dec_ctrl_ops,
		.id = V4L2_CID_DIVX_VERSION,
		.name = "specify divx version",
		.type = V4L2_CTRL_TYPE_INTEGER,
		.min = 0,
		.max = 0xffff,
		.step = 1,
		.def = 0,
	},
	{
		.ops = &vsi_dec_ctrl_ops,
		.id = V4L2_CID_ADDR_OFFSET,
		.name = "set addr offset between VPU and CPU",
		.type = V4L2_CTRL_TYPE_INTEGER64,
		.min = 0x8000000000000000,
		.max = 0x7fffffffffffffff,
		.step = 1,
		.def = 0,
		.elem_size = sizeof(s64),
	},
};

static int vsi_dec_setup_ctrls(struct v4l2_ctrl_handler *handler)
{
	int i, ctrl_num = ARRAY_SIZE(vsi_v4l2_dec_ctrl_defs);
	struct v4l2_ctrl *ctrl = NULL;

	v4l2_ctrl_handler_init(handler, ctrl_num);

	if (handler->error)
		return handler->error;

	for (i = 0; i < ctrl_num; i++) {
		vsi_v4l2_update_ctrlcfg(&vsi_v4l2_dec_ctrl_defs[i]);
		if (is_vsi_ctrl(vsi_v4l2_dec_ctrl_defs[i].id))
			ctrl = v4l2_ctrl_new_custom(handler, &vsi_v4l2_dec_ctrl_defs[i], NULL);
		else {
			if (vsi_v4l2_dec_ctrl_defs[i].type == V4L2_CTRL_TYPE_MENU) {
				ctrl = v4l2_ctrl_new_std_menu(handler, &vsi_dec_ctrl_ops,
					vsi_v4l2_dec_ctrl_defs[i].id,
					vsi_v4l2_dec_ctrl_defs[i].max,
					0,
					vsi_v4l2_dec_ctrl_defs[i].def);
			} else {
				ctrl = v4l2_ctrl_new_std(handler,
					&vsi_dec_ctrl_ops,
					vsi_v4l2_dec_ctrl_defs[i].id,
					vsi_v4l2_dec_ctrl_defs[i].min,
					vsi_v4l2_dec_ctrl_defs[i].max,
					vsi_v4l2_dec_ctrl_defs[i].step,
					vsi_v4l2_dec_ctrl_defs[i].def);
			}
		}
		if (ctrl && (vsi_v4l2_dec_ctrl_defs[i].flags & V4L2_CTRL_FLAG_VOLATILE))
			ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;

		if (handler->error) {
			v4l2_klog(LOGLVL_ERROR, "fail to set ctrl %d:%d", i, handler->error);
			handler->error = 0;
		}
	}

	v4l2_ctrl_handler_setup(handler);
	return 0;
}

static int v4l2_dec_open(struct file *filp)
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
		v4l2_klog(LOGLVL_ERROR, "create dec ctx fail.\n");
		return -ENOMEM;
	}

	ctx->dev = dev;
	mutex_init(&ctx->ctxlock);
	init_waitqueue_head(&ctx->retbuf_queue);
	init_waitqueue_head(&ctx->capoffdone_queue);
	ctx->flag = CTX_FLAG_DEC;
	ctx->frameidx = 0;
	vsiv4l2_initcfg(ctx);
	vsi_dec_setup_ctrls(&ctx->ctrlhdl);
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

	q = &ctx->input_que;
	q->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->min_buffers_needed = 1;
	q->drv_priv = &ctx->fh;
	q->lock = &ctx->ctxlock;
	q->buf_struct_size = sizeof(struct vsi_vpu_buf);		//used to alloc mem control structures in reqbuf
	q->ops = &vsi_dec_qops;		/*it might be used to identify input and output */
	q->mem_ops = get_vsi_mmop();
	q->memory = VB2_MEMORY_UNKNOWN;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	INIT_LIST_HEAD(&ctx->input_list);
	ret = vb2_queue_init(q);
	/*q->buf_ops = &v4l2_buf_ops is set here*/
	if (ret) {
		v4l2_klog(LOGLVL_ERROR, "[%s]%d ctx->input_que vb2_queue_init() fail.\n",
			__func__, __LINE__);
		goto err_enc_dec_exit;
	}

	q = &ctx->output_que;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->drv_priv = &ctx->fh;
	q->lock = &ctx->ctxlock;
	q->buf_struct_size = sizeof(struct vsi_vpu_buf);
	q->ops = &vsi_dec_qops;
	q->mem_ops = get_vsi_mmop();
	q->memory = VB2_MEMORY_UNKNOWN;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	q->min_buffers_needed = 1;
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
	q->type = V4L2_BUF_TYPE_META_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->drv_priv = &ctx->fh;
	q->lock = &ctx->ctxlock;
	q->buf_struct_size = sizeof(struct vsi_vpu_buf);
	q->ops = &vsi_dec_qops;
	q->mem_ops = get_vsi_mmop();
	q->memory = VB2_MEMORY_UNKNOWN;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	q->min_buffers_needed = 1;
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

	//dev->vdev->queue = q;
	//single queue is used for v4l2 default ops such as ioctl, read, write and poll
	//If we wanna manage queue by ourselves, leave it null and don't use default v4l2 ioctl/read/write/poll interfaces.

	return 0;

err_enc_dec_exit:
	vsi_remove_ctx(ctx);
	return ret;
}

static int v4l2_dec_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	unsigned long offset = vma->vm_pgoff;
	int ret;
	u32 bufidx, planeidx;

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

static __poll_t vsi_dec_poll(struct file *file, poll_table *wait)
{
	__poll_t ret = 0;
	__poll_t ret_vb2 = 0;
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(file->private_data);
	int dstn = atomic_read(&ctx->dstframen);
	int srcn = atomic_read(&ctx->srcframen);

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "ctx=%p\n", ctx);
		ret |= POLLERR;
		return ret;
	}

	if (!vsi_v4l2_daemonalive(ctx->dev)) {
		ret |= POLLERR;
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx, poll error, daemon is die\n",
			ctx->dev->deviceid, ctx->ctxid);
	}

	if (v4l2_event_pending(&ctx->fh))
		ret |= POLLPRI;

	if (vb2_is_streaming(&ctx->output_que)) {
		ret_vb2 = vb2_poll(&ctx->output_que, file, wait);
		if (ret_vb2 & POLLERR)
			v4l2_klog(LOGLVL_ERROR, "vb2_poll output_que error, ret_vb2 %x is_streaming %d, error %d, num_buffers %d\n",
					ret_vb2, vb2_is_streaming(&ctx->output_que),
					ctx->output_que.error, ctx->output_que.num_buffers);
		ret |= ret_vb2;
	}

	if (vb2_is_streaming(&ctx->input_que)) {
		ret_vb2 = vb2_poll(&ctx->input_que, file, wait);
		if (ret_vb2 & POLLERR)
			v4l2_klog(LOGLVL_ERROR, "vb2_poll input_que error, ret_vb2 %x is_streaming %d, error %d, num_buffers %d\n",
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
		v4l2_klog(LOGLVL_ERROR, "ctx->id=%d:%#lx poll error, ret %x, ctx error: %d\n",
			ctx->dev->deviceid, ctx->ctxid, ret, ctx->error);
	}
	v4l2_klog(LOGLVL_VERBOSE, "%x", ret);
	return ret;
}

static const struct v4l2_file_operations v4l2_dec_fops = {
	.owner = THIS_MODULE,
	.open = v4l2_dec_open,
	.release = vsi_v4l2_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = v4l2_dec_mmap,
	.poll = vsi_dec_poll,
};

struct video_device *vsi_v4l2_probe_dec(struct platform_device *pdev, struct vsi_v4l2_device *vpu)
{
	struct video_device *vdec;
	int ret = 0;

	v4l2_init_klog(LOGLVL_INFO, "module start\n");
	vdec = video_device_alloc();
	if (!vdec) {
		v4l2_err(&vpu->v4l2_dev, "Failed to allocate dec device\n");
		ret = -ENOMEM;
		goto err;
	}

	vdec->fops = &v4l2_dec_fops;
	vdec->ioctl_ops = &vsi_dec_ioctl;
	vdec->device_caps = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING | V4L2_CAP_META_CAPTURE;
	vdec->release = video_device_release;
	vdec->lock = &vpu->lock;
	vdec->v4l2_dev = &vpu->v4l2_dev;
	vdec->vfl_dir = VFL_DIR_M2M;
	vdec->vfl_type = VSI_DEVTYPE;
	vdec->queue = NULL;

	video_set_drvdata(vdec, vpu);

	ret = video_register_device(vdec, VSI_DEVTYPE, 0);
	if (ret) {
		v4l2_err(&vpu->v4l2_dev, "Failed to register dec device\n");
		video_device_release(vdec);
		goto err;
	}
	return vdec;

err:
	return NULL;
}

void vsi_v4l2_release_dec(struct video_device *vdec)
{
	video_unregister_device(vdec);
}

