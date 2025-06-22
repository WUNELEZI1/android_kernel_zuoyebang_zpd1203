/*
 *    VSI V4L2 kernel driver main entrance.
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
#include <linux/version.h>
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
#include "x-v4l2.h"

#define DRIVER_NAME	"xring_vpu_v4l2"

int vsi_kloglvl = LOGLVL_INFO;
module_param(vsi_kloglvl, int, 0644);

static int vsi_deviceno = 1;

#ifdef USE_X86_SYS
static struct platform_device *gvsidevz;
#endif

static void release_ctx(struct vsi_v4l2_ctx *ctx, int notifydaemon)
{
	int ret = 0;
	struct vsi_v4l2_device *vpu = ctx->dev;

	if (notifydaemon == 1 && test_bit(CTX_FLAG_DAEMONLIVE_BIT, &ctx->flag)) {
		if (isdecoder(ctx))
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_DESTROY_DEC, NULL);
		else
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_DESTROY_ENC, NULL);
	}
	/*vsi_vpu_buf obj is freed here, together with all buffer memory */
	mutex_lock(&vpu->vsi_ctx_array_lock);
	return_all_buffers(&ctx->input_que, VB2_BUF_STATE_DONE, 0);
	return_all_buffers(&ctx->output_que, VB2_BUF_STATE_DONE, 0);
	removeallcropinfo(ctx);
	idr_remove(&vpu->vsi_inst_array, CTX_ARRAY_ID(ctx->ctxid));
	vb2_queue_release(&ctx->input_que);
	vb2_queue_release(&ctx->output_que);

	if (ctx->meta_que.bufs[0] &&
		ctx->meta_que.bufs[0]->state == VB2_BUF_STATE_ACTIVE) {
		vb2_buffer_done(ctx->meta_que.bufs[0], VB2_BUF_STATE_DONE);
		if (test_and_clear_bit(CTX_FLAG_MAPMETA, &ctx->flag))
			iounmap(ctx->pmeta);
	}
	vb2_queue_release(&ctx->meta_que);
	v4l2_ctrl_handler_free(&ctx->ctrlhdl);
	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);
	kfree(ctx);
	ctx = NULL;
	mutex_unlock(&vpu->vsi_ctx_array_lock);
}

void vsi_remove_ctx(struct vsi_v4l2_ctx *ctx)
{
	struct vsi_v4l2_device *vpu;

	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "[%s]%d ctx invalid\n",
			__func__, __LINE__);
		return;
	}
	vpu = ctx->dev;

	mutex_lock(&vpu->vsi_ctx_array_lock);
	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);
	idr_remove(&vpu->vsi_inst_array, CTX_ARRAY_ID(ctx->ctxid));
	kfree(ctx);
	mutex_unlock(&vpu->vsi_ctx_array_lock);
}

struct vsi_v4l2_ctx *vsi_create_ctx(struct file *filp, struct vsi_v4l2_device *vpu)
{
	struct vsi_v4l2_ctx *ctx = NULL;

	if (!filp) {
		v4l2_klog(LOGLVL_ERROR, "filp is NULL.\n");
		return NULL;
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		v4l2_klog(LOGLVL_ERROR, "kmalloc ctx fail.\n");
		return NULL;
	}

	mutex_lock(&vpu->vsi_ctx_array_lock);

	ctx->ctxid = idr_alloc(&vpu->vsi_inst_array, (void *)ctx, 1, 0, GFP_KERNEL);
	if ((int)ctx->ctxid < 0) {
		v4l2_klog(LOGLVL_ERROR, "idr_alloc ctxid failed.\n");
		kfree(ctx);
		ctx = NULL;
		/* fuzz test report */
		goto exit;
	} else {
		vpu->ctx_seqid++;
		if (vpu->ctx_seqid >= CTX_SEQID_UPLIMT)
			vpu->ctx_seqid = 1;
		ctx->ctxid |= (vpu->ctx_seqid << 32);
		v4l2_klog(LOGLVL_INFO, "create ctx on %d with %lx", vpu->deviceid, ctx->ctxid);
	}

	v4l2_fh_init(&ctx->fh, video_devdata(filp));
	filp->private_data = &ctx->fh;
	v4l2_fh_add(&ctx->fh);

	init_waitqueue_head(&ctx->retbuf_queue);
	init_waitqueue_head(&ctx->capoffdone_queue);
	atomic_set(&ctx->vb2_queue_complete, 0);
exit:
	mutex_unlock(&vpu->vsi_ctx_array_lock);

	return ctx;
}

void vsi_set_ctx_error(struct vsi_v4l2_ctx *ctx, s32 error)
{
	struct v4l2_event event;

	ctx->error = error;
	if (error < 0) {
		v4l2_klog(LOGLVL_ERROR, "set ctx(%#lx) error %d", ctx->ctxid, error);
		memset(&event, 0, sizeof(struct v4l2_event));
		event.type = V4L2_EVENT_CODEC_ERROR;
		v4l2_event_queue_fh(&ctx->fh, &event);
	}
}
void wakeup_ctxqueues(struct vsi_v4l2_device *vpu)
{
	struct vsi_v4l2_ctx *ctx = NULL;
	int id = 0;

	if (mutex_lock_killable(&vpu->vsi_ctx_array_lock)) {
		v4l2_klog(LOGLVL_ERROR, "mutex_lock_killable return from signal\n");
		return;
	}

	idr_for_each_entry(&vpu->vsi_inst_array, ctx, id) {
		if (ctx) {
			vsi_set_ctx_error(ctx, DAEMON_ERR_DAEMON_MISSING);

			/* If vb2_queue_init is not completed,
			 * executing wake_up_interruptible_all will cause a coredump.
			 */
			if (atomic_read(&ctx->vb2_queue_complete)) {
				wake_up_interruptible_all(&ctx->input_que.done_wq);
				wake_up_interruptible_all(&ctx->output_que.done_wq);
			}
			wake_up_interruptible_all(&ctx->retbuf_queue);
			wake_up_interruptible_all(&ctx->capoffdone_queue);
			wake_up_interruptible_all(&ctx->fh.wait);
		}
	}

	mutex_unlock(&vpu->vsi_ctx_array_lock);
}

//these two must be used together when get_ctx return success
static struct vsi_v4l2_ctx *get_ctx(struct vsi_v4l2_device *vpu, unsigned long ctxid)
{
	unsigned long id = CTX_ARRAY_ID(ctxid);
	unsigned long seq = CTX_SEQ_ID(ctxid);
	struct vsi_v4l2_ctx *ctx;

	if (mutex_lock_killable(&vpu->vsi_ctx_array_lock)) {
		v4l2_klog(LOGLVL_ERROR, "%lx: mutex_lock_killable return from signal\n",
			ctxid);
		return NULL;
	}
	ctx  = (struct vsi_v4l2_ctx *)idr_find(&vpu->vsi_inst_array, id);
	if (ctx && (CTX_SEQ_ID(ctx->ctxid)  == seq))
		return ctx;
	else {
		mutex_unlock(&vpu->vsi_ctx_array_lock);
		return NULL;
	}
}

static void put_ctx(struct vsi_v4l2_device *vpu)
{
	mutex_unlock(&vpu->vsi_ctx_array_lock);
}

static void vsi_v4l2_clear_event(struct vsi_v4l2_ctx *ctx)
{
	struct v4l2_event event;
	int ret;

	if (v4l2_event_pending(&ctx->fh)) {
		while (v4l2_event_pending(&ctx->fh)) {
			ret = v4l2_event_dequeue(&ctx->fh, &event, 1);
			if (ret)
				return;
		};
	}
}

int vsi_v4l2_reset_ctx(struct vsi_v4l2_ctx *ctx)
{
	int ret = 0;

	if (ctx->status != VSI_STATUS_INIT) {
		v4l2_klog(LOGLVL_INFO, "reset ctx on %d:%lx", ctx->dev->deviceid, ctx->ctxid);
		ctx->queued_srcnum = ctx->buffed_capnum = ctx->buffed_cropcapnum = 0;
		vsi_v4l2_clear_event(ctx);
		if (isdecoder(ctx)) {
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_DESTROY_DEC, NULL);
			ctx->flag = CTX_FLAG_DEC;
		} else {
			ret = vsiv4l2_execcmd(ctx, V4L2_DAEMON_VIDIOC_DESTROY_ENC, NULL);
			ctx->flag = CTX_FLAG_ENC;
			set_bit(CTX_FLAG_ENC_FLUSHBUF, &ctx->flag);
		}
		return_all_buffers(&ctx->input_que, VB2_BUF_STATE_DONE, 0);
		return_all_buffers(&ctx->output_que, VB2_BUF_STATE_DONE, 0);
		removeallcropinfo(ctx);
		ctx->status = VSI_STATUS_INIT;
		vsi_set_ctx_error(ctx, 0);
		if (isdecoder(ctx)) {
			wake_up_interruptible_all(&ctx->retbuf_queue);
			wake_up_interruptible_all(&ctx->capoffdone_queue);
		}
	}
	return ret;
}

int vsi_v4l2_release(struct file *filp)
{
	struct vsi_v4l2_ctx *ctx = fh_to_ctx(filp->private_data);
	struct vsi_v4l2_device *vpu = ctx->dev;

	/*normal streaming end should fall here*/
	v4l2_klog(LOGLVL_INFO, "%s ctx on %d:%lx", __func__, ctx->dev->deviceid, ctx->ctxid);
	vsi_clear_daemonmsg(ctx);
	release_ctx(ctx, 1);
	vsi_v4l2_quitinstance(vpu);
	return 0;
}

/*orphan error msg from daemon write, should not call daemon back*/
int vsi_v4l2_handle_picconsumed(struct vsi_v4l2_device *vpu, struct vsi_v4l2_msg *pmsg)
{
	unsigned long ctxid = pmsg->inst_id;
	struct vsi_v4l2_ctx *ctx;
	struct v4l2_event event;

	v4l2_klog(LOGLVL_DEBUG, "%d:%lx got picconsumed event", vpu->deviceid, ctxid);
	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -1;

	memset((void *)&event, 0, sizeof(struct v4l2_event));
	event.type = V4L2_EVENT_SKIP;
	if (isdecoder(ctx)) {
		struct v4l2_pic_consumed_info *info =
			(struct v4l2_pic_consumed_info *)event.u.data;
		info->inbufidx = pmsg->params.dec_params.io_buffer.inbufidx;
		info->timestamp = pmsg->params.dec_params.io_buffer.timestamp;
	}

	v4l2_event_queue_fh(&ctx->fh, &event);
	put_ctx(vpu);
	return 0;
}

void vsi_v4l2_sendeos(struct vsi_v4l2_ctx *ctx)
{
	struct v4l2_event event;

	memset((void *)&event, 0, sizeof(struct v4l2_event));
	event.type = V4L2_EVENT_EOS;
	v4l2_event_queue_fh(&ctx->fh, &event);
}

int vsi_v4l2_handleerror(struct vsi_v4l2_device *vpu, unsigned long ctxid, int error)
{
	struct vsi_v4l2_ctx *ctx;

	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -1;

	if (error == DAEMON_ERR_DEC_METADATA_ONLY) {
		v4l2_klog(LOGLVL_INFO, "%d:%lx got DEC_METADATA_ONLY, send eos", vpu->deviceid, ctxid);
		vsi_v4l2_sendeos(ctx);
	} else {
		v4l2_klog(LOGLVL_ERROR, "%d:%lx got error %d", vpu->deviceid, ctxid, error);
		vsi_set_ctx_error(ctx, error > 0 ? -error:error);
		wake_up_interruptible_all(&ctx->retbuf_queue);
		wake_up_interruptible_all(&ctx->capoffdone_queue);
		wake_up_interruptible_all(&ctx->input_que.done_wq);
		wake_up_interruptible_all(&ctx->output_que.done_wq);
		wake_up_interruptible_all(&ctx->fh.wait);
	}
	put_ctx(vpu);
	return 0;
}

int vsi_v4l2_send_reschange(struct vsi_v4l2_ctx *ctx)
{
	struct v4l2_event event;

	vsi_v4l2_update_decfmt(ctx);

	memset((void *)&event, 0, sizeof(struct v4l2_event));
	event.type = V4L2_EVENT_SOURCE_CHANGE,
	event.u.src_change.changes = V4L2_EVENT_SRC_CH_RESOLUTION,
	v4l2_event_queue_fh(&ctx->fh, &event);
	return 0;
}

int vsi_v4l2_notify_reschange(struct vsi_v4l2_device *vpu, struct vsi_v4l2_msg *pmsg)
{
	unsigned long ctxid = pmsg->inst_id;
	struct vsi_v4l2_ctx *ctx;

	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -ESRCH;

	if (isdecoder(ctx)) {
		struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
		struct v4l2_daemon_dec_info *decinfo = &pmsg->params.dec_params.dec_info.dec_info;
		s32 i;

		if (mutex_lock_killable(&ctx->ctxlock)) {
			v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
				vpu->deviceid, ctx->ctxid);
			put_ctx(vpu);
			return -EBUSY;
		}
		v4l2_klog(LOGLVL_INFO, "%d:%lx sending event res change:%d, delay=%d", vpu->deviceid, ctx->ctxid, ctx->status,
			(ctx->status == DEC_STATUS_DECODING || ctx->status == DEC_STATUS_DRAINING) && !list_empty(&ctx->output_que.done_list));
		v4l2_klog(LOGLVL_INFO, "reso=%d:%d,bitdepth=%d,stride=%d,dpb=%d,orig yuvfmt=%d",
			decinfo->frame_width, decinfo->frame_height, decinfo->bit_depth, pmsg->params.dec_params.io_buffer.output_wstride,
			decinfo->needed_dpb_nums, decinfo->src_pix_fmt);
		if ((ctx->status == DEC_STATUS_DECODING || ctx->status == DEC_STATUS_DRAINING)
			&& !list_empty(&ctx->output_que.done_list)) {
			pcfg->decparams_bkup.dec_info = pmsg->params.dec_params.dec_info;
			pcfg->decparams_bkup.io_buffer.srcwidth = pmsg->params.dec_params.io_buffer.srcwidth;
			pcfg->decparams_bkup.io_buffer.srcheight = pmsg->params.dec_params.io_buffer.srcheight;
			pcfg->decparams_bkup.io_buffer.output_width = pmsg->params.dec_params.io_buffer.output_width;
			pcfg->decparams_bkup.io_buffer.output_height = pmsg->params.dec_params.io_buffer.output_height;
			pcfg->decparams_bkup.io_buffer.output_wstride = pmsg->params.dec_params.io_buffer.output_wstride;
			pcfg->minbuf_4output_bkup = pmsg->params.dec_params.dec_info.dec_info.needed_dpb_nums;
			for (i = 0; i < MAX_PLANENO; i++)
				pcfg->sizeimagedst_bkup[i] = pmsg->params.dec_params.io_buffer.OutBufSize[i];

			set_bit(CTX_FLAG_DELAY_SRCCHANGED_BIT, &ctx->flag);
			set_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag);
			v4l2_klog(LOGLVL_INFO, "[reso] planes_num:%u planes_size(bkup): %u-%u-%u\n",
				pcfg->dstplanes, pcfg->sizeimagedst_bkup[0], pcfg->sizeimagedst_bkup[1], pcfg->sizeimagedst_bkup[2]);
		} else {
			pcfg->decparams.dec_info.dec_info = pmsg->params.dec_params.dec_info.dec_info;
			pcfg->decparams.dec_info.io_buffer.srcwidth = pmsg->params.dec_params.dec_info.io_buffer.srcwidth;
			pcfg->decparams.dec_info.io_buffer.srcheight = pmsg->params.dec_params.dec_info.io_buffer.srcheight;
			pcfg->decparams.dec_info.io_buffer.output_width = pmsg->params.dec_params.dec_info.io_buffer.output_width;
			pcfg->decparams.dec_info.io_buffer.output_height = pmsg->params.dec_params.dec_info.io_buffer.output_height;
			pcfg->decparams.dec_info.io_buffer.output_wstride = pmsg->params.dec_params.dec_info.io_buffer.output_wstride;
			pcfg->bytesperline = pmsg->params.dec_params.dec_info.io_buffer.output_wstride;
			//pcfg->orig_dpbsize = decinfo->dpb_buffer_size;
			pcfg->src_pixeldepth = decinfo->bit_depth;
			pcfg->minbuf_4output =
				pcfg->minbuf_4capture = pmsg->params.dec_params.dec_info.dec_info.needed_dpb_nums;
			for (i = 0; i < MAX_PLANENO; i++)
				pcfg->sizeimagedst[i] = pmsg->params.dec_params.io_buffer.OutBufSize[i];

			set_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag);

			v4l2_klog(LOGLVL_INFO, "[reso] planes_num:%u planes_size: %u-%u-%u\n",
				pcfg->dstplanes, pcfg->sizeimagedst[0], pcfg->sizeimagedst[1], pcfg->sizeimagedst[2]);

			vsi_v4l2_send_reschange(ctx);
		}
		vsi_dec_updatevui(&pmsg->params.dec_params.dec_info.dec_info, &pcfg->decparams.dec_info.dec_info);
		mutex_unlock(&ctx->ctxlock);
	}
	put_ctx(vpu);
	return 0;
}

static int convert_daemonwarning_to_appwarning(int daemon_warnmsg)
{
	switch (daemon_warnmsg) {
	case WARN_ROIREGION:
		return ROIREGION_NOTALLOW;
	case WARN_IPCMREGION:
		return IPCMREGION_NOTALLOW;
	case WARN_LEVEL:
		return LEVEL_UPDATED;

	case WARN_INTRA_AREA:
		return INTRAREGION_NOTALLOW;
	case WARN_GOP_SIZE:
		return GOPSIZE_NOTALLOW;
	default:
		return UNKONW_WARNING;
	}
}

int vsi_v4l2_handle_warningmsg(struct vsi_v4l2_device *vpu, struct vsi_v4l2_msg *pmsg)
{
	unsigned long ctxid = pmsg->inst_id;
	struct vsi_v4l2_ctx *ctx;
	struct v4l2_event event;

	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -ESRCH;
	memset((void *)&event, 0, sizeof(struct v4l2_event));
	event.type = V4L2_EVENT_INVALID_OPTION;
	event.id = convert_daemonwarning_to_appwarning(pmsg->error);
	v4l2_klog(LOGLVL_WARNING, "%d:%lx got warning msg %d", vpu->deviceid, ctxid, pmsg->error);
	v4l2_event_queue_fh(&ctx->fh, &event);
	put_ctx(vpu);
	return 0;
}

int vsi_v4l2_handle_streamoffdone(struct vsi_v4l2_device *vpu, struct vsi_v4l2_msg *pmsg)
{
	unsigned long ctxid = pmsg->inst_id;
	struct vsi_v4l2_ctx *ctx;

	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -ESRCH;
	if (pmsg->cmd_id == V4L2_DAEMON_VIDIOC_STREAMOFF_CAPTURE_DONE)
		set_bit(CTX_FLAG_CAPTUREOFFDONE, &ctx->flag);
	else
		set_bit(CTX_FLAG_OUTPUTOFFDONE, &ctx->flag);
	wake_up_interruptible_all(&ctx->capoffdone_queue);
	v4l2_klog(LOGLVL_DEBUG, "%d:%lx got cap streamoff done", vpu->deviceid, ctxid);
	put_ctx(vpu);
	return 0;
}

int vsi_v4l2_handle_cropchange(struct vsi_v4l2_device *vpu,  struct vsi_v4l2_msg *pmsg)
{
	unsigned long ctxid = pmsg->inst_id;
	struct vsi_v4l2_ctx *ctx;
	struct v4l2_daemon_extra_info *extra_info;

	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -ESRCH;

	if (isdecoder(ctx)) {
		struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
		struct v4l2_event event;

		if (mutex_lock_killable(&ctx->ctxlock)) {
			v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
				vpu->deviceid, ctx->ctxid);
			put_ctx(vpu);
			return -EBUSY;
		}
		v4l2_klog(LOGLVL_DEBUG, "%d:%lx sending crop change:%d:%d:%d", vpu->deviceid, ctx->ctxid, ctx->status, ctx->buffed_cropcapnum, ctx->lastcapbuffer_idx);
		v4l2_klog(LOGLVL_DEBUG, "crop info:%d:%d:%d:%d:%d:%d:%d",
			pmsg->params.dec_params.pic_info.pic_info.width,
			pmsg->params.dec_params.pic_info.pic_info.height,
			pmsg->params.dec_params.pic_info.pic_info.pic_wstride,
			pmsg->params.dec_params.pic_info.pic_info.crop_left,
			pmsg->params.dec_params.pic_info.pic_info.crop_top,
			pmsg->params.dec_params.pic_info.pic_info.crop_width,
			pmsg->params.dec_params.pic_info.pic_info.crop_height);
		if ((ctx->status == DEC_STATUS_DECODING || ctx->status == DEC_STATUS_DRAINING)
			&& ctx->buffed_cropcapnum > 0) {
			if (addcropmsg(ctx, pmsg) != 0) {
				vsi_set_ctx_error(ctx, DAEMON_ERR_NO_MEM);
				v4l2_klog(LOGLVL_ERROR, "driver out of mem");
			} else
				set_bit(BUF_FLAG_CROPCHANGE, &ctx->vbufflag[ctx->lastcapbuffer_idx]);
		} else {
			extra_info = (struct v4l2_daemon_extra_info *)event.u.data;
			pcfg->decparams.dec_info.io_buffer.output_width = pmsg->params.dec_params.pic_info.pic_info.width;
			pcfg->decparams.dec_info.io_buffer.output_height = pmsg->params.dec_params.pic_info.pic_info.height;
			pcfg->decparams.dec_info.io_buffer.output_wstride = pmsg->params.dec_params.pic_info.pic_info.pic_wstride;
			pcfg->decparams.dec_info.dec_info.frame_width = pmsg->params.dec_params.pic_info.pic_info.width;
			pcfg->bytesperline = pmsg->params.dec_params.pic_info.pic_info.pic_wstride;
			pcfg->decparams.dec_info.dec_info.frame_height = pmsg->params.dec_params.pic_info.pic_info.height;
			pcfg->decparams.dec_info.dec_info.visible_rect.left = pmsg->params.dec_params.pic_info.pic_info.crop_left;
			pcfg->decparams.dec_info.dec_info.visible_rect.top = pmsg->params.dec_params.pic_info.pic_info.crop_top;
			pcfg->decparams.dec_info.dec_info.visible_rect.width = pmsg->params.dec_params.pic_info.pic_info.crop_width;
			pcfg->decparams.dec_info.dec_info.visible_rect.height = pmsg->params.dec_params.pic_info.pic_info.crop_height;
			memset((void *)&event, 0, sizeof(struct v4l2_event));
			event.type = V4L2_EVENT_CROPCHANGE;
			memcpy(extra_info, &pmsg->params.dec_params.pic_info.pic_info.extra_info,
						sizeof(struct v4l2_daemon_extra_info));
			v4l2_event_queue_fh(&ctx->fh, &event);
		}
		mutex_unlock(&ctx->ctxlock);
	}
	put_ctx(vpu);
	return 0;
}

int vsi_v4l2_bufferdone(struct vsi_v4l2_device *vpu, struct vsi_v4l2_msg *pmsg)
{
	unsigned long ctxid = pmsg->inst_id;
	s32 inbufidx, outbufidx;
	struct vsi_v4l2_ctx *ctx;
	struct vb2_queue *vq = NULL;
	struct vb2_v4l2_buffer *vbuf;
	struct vb2_buffer	*vb;
	s32 i;

	ctx = get_ctx(vpu, ctxid);
	if (ctx == NULL)
		return -1;

	if (isencoder(ctx)) {
		inbufidx = pmsg->params.enc_params.io_buffer.inbufidx;
		outbufidx = pmsg->params.enc_params.io_buffer.outbufidx;
	} else {
		inbufidx = pmsg->params.dec_params.io_buffer.inbufidx;
		outbufidx = pmsg->params.dec_params.io_buffer.outbufidx;
	}
	v4l2_klog(LOGLVL_DEBUG, "%d:%lx:%lx:%d:%d",
		vpu->deviceid, ctx->ctxid, ctx->flag, inbufidx, outbufidx);
	//write comes over once, so avoid this problem.
	if (inbufidx >= 0 && inbufidx < ctx->input_que.num_buffers) {
		vq = &ctx->input_que;
		vb = vq->bufs[inbufidx];
		if (mutex_lock_killable(&ctx->ctxlock)) {
			v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
				vpu->deviceid, ctx->ctxid);
			put_ctx(vpu);
			return -EBUSY;
		}
		atomic_inc(&ctx->srcframen);
		if (ctx->input_que.streaming && vb->state == VB2_BUF_STATE_ACTIVE)
			vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
		if (isdecoder(ctx)) {
			ctx->queued_srcnum--;
			if (!test_bit(BUF_FLAG_QUEUED, &ctx->srcvbufflag[inbufidx])) {
				v4l2_klog(LOGLVL_WARNING, "got unqueued srcbuf %d", inbufidx);
			} else {
				clear_bit(BUF_FLAG_QUEUED, &ctx->srcvbufflag[inbufidx]);
				set_bit(BUF_FLAG_DONE, &ctx->srcvbufflag[inbufidx]);
			}
		}
		mutex_unlock(&ctx->ctxlock);
		if (ctx->queued_srcnum == 0)
			wake_up_interruptible_all(&ctx->retbuf_queue);
		put_ctx(vpu);
		return 0;
	}
	if (outbufidx >= 0 && outbufidx < ctx->output_que.num_buffers) {
		if (mutex_lock_killable(&ctx->ctxlock)) {
			v4l2_klog(LOGLVL_ERROR, "%d:%lx: mutex_lock_killable return from signal\n",
				vpu->deviceid, ctx->ctxid);
			put_ctx(vpu);
			return -EBUSY;
		}
		if (!inst_isactive(ctx)) {
			if (!vb2_is_streaming(&ctx->output_que))
				v4l2_klog(LOGLVL_ERROR, "%d:%lx ignore dst buffer %d in state %d", vpu->deviceid, ctx->ctxid, outbufidx, ctx->status);
			mutex_unlock(&ctx->ctxlock);
			put_ctx(vpu);
			return 0;
		}
		vq = &ctx->output_que;
		vb = vq->bufs[outbufidx];
		vbuf = to_vb2_v4l2_buffer(vb);
		if (pmsg->params.dec_params.io_buffer.bytesused[0] > 0)
			ctx->frameidx++;
		atomic_inc(&ctx->dstframen);
		if (vb->state == VB2_BUF_STATE_ACTIVE) {
			for (i = 0; i < vb->num_planes; i++)
				vb->planes[i].bytesused = pmsg->params.dec_params.io_buffer.bytesused[i];
			if (isencoder(ctx)) {
				vb->planes[0].bytesused = pmsg->params.enc_params.io_buffer.bytesused;
				if (ctx->mediacfg.m_encparams.m_scaleoutput.scaleOutput)
					vb->planes[1].bytesused = pmsg->params.enc_params.io_buffer.scale_bytesout;
				vb->timestamp = pmsg->params.enc_params.io_buffer.timestamp;
				ctx->vbufflag[outbufidx] = pmsg->param_type;
				v4l2_klog(LOGLVL_DEBUG,  "enc output framed %d size = %d,flag=%lx, timestamp=%lld",
						outbufidx, vb->planes[0].bytesused, ctx->vbufflag[outbufidx], vb->timestamp);
				if (vb->planes[0].bytesused == 0 || (pmsg->param_type & LAST_BUFFER_FLAG)) {
					vbuf->flags |= V4L2_BUF_FLAG_LAST;
					ctx->vbufflag[outbufidx] |= LAST_BUFFER_FLAG;
					v4l2_klog(LOGLVL_INFO, "%d:%lx encoder got eos buffer", vpu->deviceid, ctx->ctxid);
				}
			} else {
				ctx->lastcapbuffer_idx = outbufidx;
				if (!test_bit(BUF_FLAG_QUEUED, &ctx->vbufflag[outbufidx])) {
					v4l2_klog(LOGLVL_WARNING, "got unqueued dstbuf %d", outbufidx);
				} else {
					clear_bit(BUF_FLAG_QUEUED, &ctx->vbufflag[outbufidx]);
					set_bit(BUF_FLAG_DONE, &ctx->vbufflag[outbufidx]);
				}
				ctx->rfc_luma_offset[outbufidx] = pmsg->params.dec_params.io_buffer.rfc_luma_offset;
				ctx->rfc_chroma_offset[outbufidx] = pmsg->params.dec_params.io_buffer.rfc_chroma_offset;
				if (pmsg->params.dec_params.io_buffer.bytesused[0] == 0) {
					v4l2_klog(LOGLVL_INFO, "%d:%lx decoder got zero buffer in state %d", vpu->deviceid, ctx->ctxid, ctx->status);
					vbuf->flags |= V4L2_BUF_FLAG_LAST;
					if ((ctx->status == DEC_STATUS_DRAINING) || test_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag)) {
						ctx->status = DEC_STATUS_ENDSTREAM;
						set_bit(CTX_FLAG_ENDOFSTRM_BIT, &ctx->flag);
						clear_bit(CTX_FLAG_PRE_DRAINING_BIT, &ctx->flag);
					}
				} else
					vb->timestamp = pmsg->params.dec_params.io_buffer.timestamp;
				ctx->buffed_capnum++;
				ctx->buffed_cropcapnum++;
				v4l2_klog(LOGLVL_DEBUG, "dec output framed %d size = %d", outbufidx, vb->planes[0].bytesused);
			}
			if (vb->state == VB2_BUF_STATE_ACTIVE)
				vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
			else
				v4l2_klog(LOGLVL_WARNING, "dstbuf %d is not active", outbufidx);
		}
		mutex_unlock(&ctx->ctxlock);
		put_ctx(vpu);
		return 0;
	}
	put_ctx(vpu);
	return 0;
}

int vsi_v4l2_get_buffer_info(struct vsi_v4l2_device *vpu, struct vsi_v4l2_dma_buffers_info *info)
{
	unsigned long instance_id = info->instance_id;
	int index = -1;
	int i = 0;
	struct vb2_queue *buffer_queue = NULL;
	struct vb2_buffer *buffer = NULL;
	int fd = -1;
	struct vsi_v4l2_ctx *context = get_ctx(vpu, instance_id);
	struct x_pp_buf_node *pp_node;
	int ret = 0;
	bool is_pp1_request = false;
	struct vb2_plane *plane;

	if (!context) {
		v4l2_klog(LOGLVL_ERROR, "can not get context for id %lx\n", instance_id);
		return -1;
	}
	if (mutex_lock_killable(&context->ctxlock)) {
		v4l2_klog(LOGLVL_ERROR, "%lx: mutex_lock_killable return from signal", context->ctxid);
		put_ctx(vpu);
		return -1;
	}
	index = info->index;
	switch (info->type) {
	case VSI_V4L2_BUFFER_TYPE_INPUT:
		buffer_queue = &context->input_que;
		break;
	case VSI_V4L2_BUFFER_TYPE_OUTPUT:
		buffer_queue = &context->output_que;
		break;
	case VSI_V4L2_BUFFER_TYPE_META:
		buffer_queue = &context->meta_que;
		break;
	case VSI_V4L2_BUFFER_TYPE_PP:
		is_pp1_request = true;
		break;
	default:
		break;
	}

	if (is_pp1_request && (!list_empty(&context->x_pp_list))) {
		list_for_each_entry(pp_node, &context->x_pp_list, node) {
			if (pp_node->index == index) {
				v4l2_klog(LOGLVL_DEBUG, "%lx: find index(%u) from pp1 list",
					context->ctxid, index);
				fd = dma_buf_fd(pp_node->dbuf, O_CLOEXEC);
				if (fd < 0) {
					v4l2_klog(LOGLVL_ERROR, "get pp fd failed");
					ret = fd;
					goto exit;
				}
				dma_buf_get(fd);
				info->dma_buf = (unsigned long)pp_node->dbuf;
				info->plane_count = 1;
				info->plane_infos[0].fd = fd;
				info->plane_infos[0].offset = 0;
				info->plane_infos[i].length = pp_node->size;
				info->plane_infos[i].bytes_used = pp_node->size;
				goto exit;
			}
		}

		if (fd < 0) {
			v4l2_klog(LOGLVL_ERROR, "find pp fd failed");
			ret = fd;
			goto exit;
		}
	}

	if (!buffer_queue || index >= buffer_queue->num_buffers) {
		v4l2_klog(LOGLVL_ERROR, "invalid index, index %d, type %d\n", index, info->type);
		ret = -1;
		goto exit;
	}

	buffer = buffer_queue->bufs[index];
	info->plane_count = buffer->num_planes;
	for (i = 0; i < info->plane_count; i++) {
		plane = &buffer->planes[i];
		if (i == 0) {
			fd = dma_buf_fd(plane->dbuf, O_CLOEXEC);
			if (fd < 0) {
				v4l2_klog(LOGLVL_ERROR, "get fd %d failed. dbuf info %p %p\n", fd, plane->dbuf, plane->dbuf ? plane->dbuf->file : NULL);
				ret = fd;
				goto exit;
			}
			dma_buf_get(fd);
			info->dma_buf = (unsigned long)plane->dbuf;
		}
		info->plane_infos[i].fd = fd;
		info->plane_infos[i].offset = plane->data_offset;
		info->plane_infos[i].length = plane->length - plane->data_offset;
		info->plane_infos[i].bytes_used = plane->bytesused;
	}
exit:
	mutex_unlock(&context->ctxlock);
	put_ctx(vpu);
	return ret;
}

static void vsi_daemonsdevice_release(struct device *dev)
{
}

static int v4l2_probe(struct platform_device *pdev)
{
	struct vsi_v4l2_device *vpu = NULL;
	struct video_device *venc, *vdec;
	int i, ret = 0, validnum = 0, stage;
	int deviceno = clamp(vsi_deviceno, 0, MAX_VIDEODEV_NO);
	struct vsi_device_node *vsidev = platform_get_drvdata(pdev);
	char daemonname[sizeof(VSI_DAEMON_FNAME) + 3] = {0};

	v4l2_init_klog(LOGLVL_DEBUG, "");
	if (vsidev != NULL) {
		v4l2_klog(LOGLVL_ERROR, "dev already registered\n");
		return 0;
	}
	vsidev = kzalloc(sizeof(struct vsi_device_node), GFP_KERNEL);
	if (!vsidev) {
		v4l2_klog(LOGLVL_ERROR, "no mem for device\n");
		return -ENOMEM;
	}

#if KERNEL_VERSION(6, 4, 0) > LINUX_VERSION_CODE
	vsidev->mclass = class_create(THIS_MODULE, "vsi_class");
#else
	vsidev->mclass = class_create("vsi_class");
#endif
	if (IS_ERR(vsidev->mclass)) {
		v4l2_klog(LOGLVL_ERROR, "unable to register class\n");
		kfree(vsidev);
		return -ENODEV;
	}
	v4l2_klog(LOGLVL_DEBUG, "%d devnode, %p", deviceno, vsidev);
	for (i = 0; i < deviceno; i++) {
		vpu = kzalloc(sizeof(struct vsi_v4l2_device), GFP_KERNEL);
		if (!vpu) {
			v4l2_klog(LOGLVL_ERROR, "no mem for vpu node %d\n", i);
			continue;
		}
		v4l2_klog(LOGLVL_DEBUG, "handle vpu %d:%p", i, vpu);
		vpu->deviceid = i;
		vpu->dev = &pdev->dev;
		vpu->pdev = pdev;
		mutex_init(&vpu->lock);
		mutex_init(&vpu->vsi_ctx_array_lock);
		vpu->venc = NULL;
		vpu->vdec = NULL;
		idr_init(&vpu->vsi_inst_array);
		vpu->ctx_seqid = 0;
		vpu->vsidaemondev.parent = NULL;
		stage = 0;

		ret = v4l2_device_register(&pdev->dev, &vpu->v4l2_dev);
		if (ret) {
			v4l2_klog(LOGLVL_ERROR, "Failed to register %d v4l2 node\n", i);
			goto relerr;
		}

		if (i == 0) {
			ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
			v4l2_init_klog(LOGLVL_DEBUG, "set dma mask %d", ret);
			if (!pdev->dev.dma_parms)
				pdev->dev.dma_parms = devm_kzalloc(&pdev->dev,
					sizeof(*pdev->dev.dma_parms), GFP_KERNEL);
			if (pdev->dev.dma_parms) {
				ret = dma_set_max_seg_size(&pdev->dev, (unsigned int)DMA_BIT_MASK(64));
				v4l2_init_klog(LOGLVL_DEBUG, "set dma max seg size %d", ret);
			}
		}

		stage = 1;
		ret = vsiv4l2_initdaemon(vpu);
		if (ret < 0) {
			v4l2_klog(LOGLVL_ERROR, "Failed to create %d daemon\n", i);
			goto relerr;
		}
		stage = 2;
		vpu->vsidaemondev.devt = MKDEV(VSI_DAEMON_DEVMAJOR + vpu->deviceid, 0);
		memcpy(daemonname, VSI_DAEMON_FNAME, sizeof(VSI_DAEMON_FNAME));
		if (i == 0) {
			daemonname[sizeof(VSI_DAEMON_FNAME) - 1] = 0;
		} else if (i < 10) {
			daemonname[sizeof(VSI_DAEMON_FNAME) - 1] = i + 0x30;
			daemonname[sizeof(VSI_DAEMON_FNAME)] = 0;
		} else {
			daemonname[sizeof(VSI_DAEMON_FNAME) - 1] = i/10 + 0x30;
			daemonname[sizeof(VSI_DAEMON_FNAME)] = i%10 + 0x30;
			daemonname[sizeof(VSI_DAEMON_FNAME) + 1] = 0;
		}
		dev_set_name(&vpu->vsidaemondev, "%s", daemonname);
		vpu->vsidaemondev.release = vsi_daemonsdevice_release;
		vpu->vsidaemondev.class = vsidev->mclass;
		ret = device_register(&vpu->vsidaemondev);
		if (ret < 0) {
			v4l2_klog(LOGLVL_ERROR, "Failed to create %d daemon device\n", i);
			goto relerr;
		}

		stage = 3;
		venc = vsi_v4l2_probe_enc(pdev, vpu);
		if (venc == NULL) {
			v4l2_klog(LOGLVL_ERROR, "Failed to probe %d encoder\n", i);
			goto relerr;
		}
		vpu->venc = venc;

		vdec = vsi_v4l2_probe_dec(pdev, vpu);
		if (vdec == NULL) {
			v4l2_klog(LOGLVL_ERROR, "Failed to probe %d decoder\n", i);
			goto relerr;
		}
		vpu->vdec = vdec;
		vsidev->node[i] = vpu;
		validnum++;
		v4l2_klog(LOGLVL_DEBUG, "vpu v4l2: module inserted %d\n", i);
		continue;
relerr:
		if (vpu->venc) {
			vsi_v4l2_release_enc(vpu->venc);
			video_device_release(vpu->venc);
		}
		if (vpu->vdec) {
			vsi_v4l2_release_dec(vpu->vdec);
			video_device_release(vpu->vdec);
		}
		if (stage >= 3)
			device_unregister(&vpu->vsidaemondev);
		if (stage >= 2)
			vsiv4l2_cleanupdaemon(vpu);
		if (stage >= 1)
			v4l2_device_unregister(&vpu->v4l2_dev);
		kfree(vpu);
	} //for i < deviceno

	if (validnum > 0) {
		platform_set_drvdata(pdev, vsidev);
		return 0;
	} else {
		class_destroy(vsidev->mclass);
		kfree(vsidev);
		return -ENXIO;
	}
}

static int v4l2_remove(struct platform_device *pdev)
{
	void *obj;
	int id, i;
	struct vsi_v4l2_device *vpu;
	struct vsi_device_node *vsidev = platform_get_drvdata(pdev);

	if (vsidev) {
		for (i = 0; i < MAX_VIDEODEV_NO; i++) {
			if (vsidev->node[i]) {
				vpu = vsidev->node[i];
				v4l2_klog(LOGLVL_DEBUG, "%d:%p", i, vpu);
				vsi_v4l2_release_dec(vpu->vdec);
				vsi_v4l2_release_enc(vpu->venc);
				v4l2_device_unregister(&vpu->v4l2_dev);
				idr_for_each_entry(&vpu->vsi_inst_array, obj, id) {
					if (obj) {
						release_ctx(obj, 0);
						vsi_v4l2_quitinstance(vpu);
					}
				}
				device_unregister(&vpu->vsidaemondev);
				vsiv4l2_cleanupdaemon(vpu);
				kfree(vpu);
			}
		}
		class_destroy(vsidev->mclass);
		kfree(vsidev);
		platform_set_drvdata(pdev, NULL);
	}
	return 0;
}

static const struct platform_device_id v4l2_platform_ids[] = {
	{
		.name            = DRIVER_NAME,
	},
	{ },
};

static const struct of_device_id v4l2_of_match[] = {
	{ .compatible = "xring,vpu-v4l2", },
	{/* sentinel */}
};

static struct platform_driver v4l2_drm_platform_driver = {
	.probe      = v4l2_probe,
	.remove      = v4l2_remove,
	.driver      = {
		.name      = DRIVER_NAME,
		.owner		= THIS_MODULE,
		.of_match_table = v4l2_of_match,
	},
	.id_table = v4l2_platform_ids,
};

static const struct platform_device_info v4l2_platform_info = {
	.name		= DRIVER_NAME,
	.id		= -1,
	.dma_mask	= DMA_BIT_MASK(64),
};
#ifndef USE_X86_SYS
module_platform_driver(v4l2_drm_platform_driver);
#else

void __exit vsi_v4l2_cleanup(void)
{
	platform_driver_unregister(&v4l2_drm_platform_driver);
	platform_device_unregister(gvsidevz);
	gvsidevz = NULL;
}

int __init vsi_v4l2_init(void)
{
	int result;

	result = platform_driver_register(&v4l2_drm_platform_driver);
	if (result < 0) {
		platform_device_unregister(gvsidevz);
		gvsidevz = NULL;
	}

	if (gvsidevz == NULL) {
		gvsidevz = platform_device_register_full(&v4l2_platform_info);
		if (gvsidevz == NULL) {
			v4l2_klog(LOGLVL_ERROR, "v4l2 create platform device fail");
			platform_driver_unregister(&v4l2_drm_platform_driver);
			return -1;
		}
	}
	return result;
}
module_init(vsi_v4l2_init);
module_exit(vsi_v4l2_cleanup);
#endif
/* module description */
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Verisilicon");
MODULE_DESCRIPTION("VSI v4l2 manager");

