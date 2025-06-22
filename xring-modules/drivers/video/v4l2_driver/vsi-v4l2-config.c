/*
 *    VSI v4l2 media config manager.
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
#include "vsi-v4l2-priv.h"

#define MAX_PLANE_SIZE        (2 * 16384 * 2 * 16384UL)

static struct vsi_v4l2_dev_info vsi_v4l2_hwconfig = {0};
static int vsiv4l2_verifyfmt(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt);
static s32 vsiv4l2_enc_getalign(u32 srcfmt, u32 dstfmt, s32 bytesperline, s32 planeidx);
static u32 vsi_find_local_dec_format(s32 fmt, int braw);
static const struct v4l2_format_info *vsi_v4l2_fmtinfo(u32 fourcc);

/* must be in order of enum vsi_v4l2enc_pixel_fmt */
static s32 raw_fmt_tbl[][2] = {
	/* vsi-pixel-fmt		bytes/pix/plane	*/
	{VSI_V4L2ENC_PIX_FMT_NV12,		1   /*V4L2_PIX_FMT_NV12*/},
	{VSI_V4L2ENC_PIX_FMT_NV12M,		1   /*V4L2_PIX_FMT_NV12M*/},
	{VSI_V4L2ENC_PIX_FMT_GREY,		1   /*V4L2_PIX_FMT_GREY*/},
	{VSI_V4L2ENC_PIX_FMT_Y10L,		2   /*V4L2_PIX_FMT_Y10*/},
	{VSI_V4L2ENC_PIX_FMT_Y10H,		2   /*V4L2_PIX_FMT_Y010*/},
	{VSI_V4L2ENC_PIX_FMT_411SP,		1   /*V4L2_PIX_FMT_411SP*/},
	{VSI_V4L2ENC_PIX_FMT_NV16,		1   /*V4L2_PIX_FMT_NV16*/},
	{VSI_V4L2ENC_PIX_FMT_NV16M,		1   /*V4L2_PIX_FMT_NV16M*/},
	{VSI_V4L2ENC_PIX_FMT_NV61,		1   /*V4L2_PIX_FMT_NV61*/},
	{VSI_V4L2ENC_PIX_FMT_NV61M,		1   /*V4L2_PIX_FMT_NV61M*/},
	{VSI_V4L2ENC_PIX_FMT_NV24,		1   /*V4L2_PIX_FMT_NV24*/},
	{VSI_V4L2ENC_PIX_FMT_YUV420,	1   /*V4L2_PIX_FMT_YUV420*/},
	{VSI_V4L2ENC_PIX_FMT_YUV420M,	1   /*V4L2_PIX_FMT_YUV420M*/},
	{VSI_V4L2ENC_PIX_FMT_YVU420,	1   /*V4L2_PIX_FMT_YVU420*/},
	{VSI_V4L2ENC_PIX_FMT_YVU420M,	1   /*V4L2_PIX_FMT_YVU420M*/},
	{VSI_V4L2ENC_PIX_FMT_YUV422M,	1   /*V4L2_PIX_FMT_YUV422M*/},
	{VSI_V4L2ENC_PIX_FMT_YVU422M,	1   /*V4L2_PIX_FMT_YVU422M*/},
	{VSI_V4L2ENC_PIX_FMT_YUV444M,	1   /*V4L2_PIX_FMT_YUV444M*/},
	{VSI_V4L2ENC_PIX_FMT_YVU444M,	1   /*V4L2_PIX_FMT_YVU444M*/},
	{VSI_V4L2ENC_PIX_FMT_NV21,		1   /*V4L2_PIX_FMT_NV21*/},
	{VSI_V4L2ENC_PIX_FMT_NV21M,		1   /*V4L2_PIX_FMT_NV21M*/},
	{VSI_V4L2ENC_PIX_FMT_YUYV,		2   /*V4L2_PIX_FMT_YUYV*/},
	{VSI_V4L2ENC_PIX_FMT_UYVY,		2   /*V4L2_PIX_FMT_UYVY*/},
	{VSI_V4L2ENC_PIX_FMT_RGB565,	2   /*V4L2_PIX_FMT_RGB565*/},
	{VSI_V4L2ENC_PIX_FMT_BGR565,	2   /*V4L2_PIX_FMT_BGR565*/},
	{VSI_V4L2ENC_PIX_FMT_RGB555,	2   /*V4L2_PIX_FMT_RGB555*/},
	{VSI_V4L2ENC_PIX_FMT_ABGR555,	2   /*V4L2_PIX_FMT_ABGR555*/},
	{VSI_V4L2ENC_PIX_FMT_RGB444,	2   /*V4L2_PIX_FMT_RGB444*/},
	{VSI_V4L2ENC_PIX_FMT_BGR444,	2   /*V4L2_PIX_FMT_BGR444*/},
	{VSI_V4L2ENC_PIX_FMT_RGB32,		4   /*V4L2_PIX_FMT_RGB32*/},
	{VSI_V4L2ENC_PIX_FMT_RGBA32,	4   /*V4L2_PIX_FMT_RGBA32*/},
	{VSI_V4L2ENC_PIX_FMT_ARGB32,	4   /*V4L2_PIX_FMT_ARGB32*/},
	{VSI_V4L2ENC_PIX_FMT_RGBX32,	4   /*V4L2_PIX_FMT_RGBX32*/},
	{VSI_V4L2ENC_PIX_FMT_XRGB32,	4   /*V4L2_PIX_FMT_XRGB32*/},
	{VSI_V4L2ENC_PIX_FMT_BGR32,		4   /*V4L2_PIX_FMT_BGR32*/},
	{VSI_V4L2ENC_PIX_FMT_BGRA32,	4   /*V4L2_PIX_FMT_BGRA32*/},
	{VSI_V4L2ENC_PIX_FMT_ABGR32,	4   /*V4L2_PIX_FMT_ABGR32*/},
	{VSI_V4L2ENC_PIX_FMT_XBGR32,	4   /*V4L2_PIX_FMT_XBGR32*/},
	{VSI_V4L2ENC_PIX_FMT_BGRX32,	4   /*V4L2_PIX_FMT_BGRX32*/},
	{VSI_V4L2ENC_PIX_FMT_RGBA1010102, 4  /*V4L2_PIX_FMT_RGBA1010102*/},
	{VSI_V4L2ENC_PIX_FMT_ARGB2101010, 4  /*V4L2_PIX_FMT_ARGB2101010*/},
	{VSI_V4L2ENC_PIX_FMT_ABGR2101010, 4  /*V4L2_PIX_FMT_ABGR2101010*/},
	{VSI_V4L2ENC_PIX_FMT_BGRA1010102, 4  /*V4L2_PIX_FMT_BGRA1010102*/},
	{VSI_V4L2ENC_PIX_FMT_P010,		2   /*V4L2_PIX_FMT_P010*/},
	{VSI_V4L2ENC_PIX_FMT_P010M,		2   /*V4L2_PIX_FMT_P010M*/},
	{VSI_V4L2ENC_PIX_FMT_I010,		2   /*V4L2_PIX_FMT_I010*/},
	{VSI_V4L2ENC_PIX_FMT_NV12_4L4,	1   /*V4L2_PIX_FMT_NV12_4L4*/},
	{VSI_V4L2ENC_PIX_FMT_P010_4L4,	2   /*V4L2_PIX_FMT_P010_4L4*/},
	{VSI_V4L2ENC_PIX_FMT_DTRC,		1   /*V4L2_PIX_FMT_DTRC*/},
	{VSI_V4L2ENC_PIX_FMT_NV12X,		1   /*V4L2_PIX_FMT_NV12X*/},
	{VSI_V4L2ENC_PIX_FMT_TILEX,		1   /*V4L2_PIX_FMT_TILEX*/},
	{VSI_V4L2ENC_PIX_FMT_RFC,		1   /*V4L2_PIX_FMT_RFC*/},
	{VSI_V4L2ENC_PIX_FMT_RFCX,		1   /*V4L2_PIX_FMT_RFCX*/},
};

static const u8 colorprimaries[] = {
	0,
	V4L2_COLORSPACE_REC709,        /*Rec. ITU-R BT.709-6*/
	0,
	0,
	V4L2_COLORSPACE_470_SYSTEM_M, /*Rec. ITU-R BT.470-6 System M*/
	V4L2_COLORSPACE_470_SYSTEM_BG,/*Rec. ITU-R BT.470-6 System B, G*/
	V4L2_COLORSPACE_SMPTE170M,    /*SMPTE170M*/
	V4L2_COLORSPACE_SMPTE240M,    /*SMPTE240M*/
	V4L2_COLORSPACE_GENERIC_FILM, /*Generic film*/
	V4L2_COLORSPACE_BT2020,       /*Rec. ITU-R BT.2020-2*/
	V4L2_COLORSPACE_ST428,        /*SMPTE ST 428-1*/
	V4L2_COLORSPACE_DCI_P3,
	V4L2_COLORSPACE_DCI_P3,
};

static const u8 colortransfers[] = {
	0,
	V4L2_XFER_FUNC_709,      /*Rec. ITU-R BT.709-6*/
	0,
	0,
	V4L2_XFER_FUNC_GAMMA22,  /*Rec. ITU-R BT.470-6 System M*/
	V4L2_XFER_FUNC_GAMMA28,  /*Rec. ITU-R BT.470-6 System B, G*/
	V4L2_XFER_FUNC_709,      /*SMPTE170M*/
	V4L2_XFER_FUNC_SMPTE240M,/*SMPTE240M*/
	V4L2_XFER_FUNC_LINEAR,   /*Linear transfer characteristics*/
	0,
	0,
	V4L2_XFER_FUNC_XVYCC,    /*IEC 61966-2-4*/
	V4L2_XFER_FUNC_BT1361,   /*Rec. ITU-R BT.1361-0 extended colour gamut*/
	V4L2_XFER_FUNC_SRGB,     /*IEC 61966-2-1 sRGB or sYCC*/
	V4L2_XFER_FUNC_709,      /*Rec. ITU-R BT.2020-2 (10 bit system)*/
	V4L2_XFER_FUNC_709,      /*Rec. ITU-R BT.2020-2 (12 bit system)*/
	V4L2_XFER_FUNC_SMPTE2084,/*SMPTE ST 2084*/
	V4L2_XFER_FUNC_ST428,    /*SMPTE ST 428-1*/
	V4L2_XFER_FUNC_HLG       /*Rec. ITU-R BT.2100-0 hybrid log-gamma (HLG)*/
};

static const u8 colormatrixcoefs[] = {
	0,
	V4L2_YCBCR_ENC_709,             /*Rec. ITU-R BT.709-6*/
	0,
	0,
	V4L2_YCBCR_ENC_BT470_6M,        /*Title 47 Code of Federal Regulations*/
	V4L2_YCBCR_ENC_601,             /*Rec. ITU-R BT.601-7 625*/
	V4L2_YCBCR_ENC_601,             /*Rec. ITU-R BT.601-7 525*/
	V4L2_YCBCR_ENC_SMPTE240M,       /*SMPTE240M*/
	0,
	V4L2_YCBCR_ENC_BT2020,          /*Rec. ITU-R BT.2020-2*/
	V4L2_YCBCR_ENC_BT2020_CONST_LUM /*Rec. ITU-R BT.2020-2 constant*/
};

static int enc_isRGBformat(u32 fourcc)
{
	switch (fourcc) {
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_BGR565:
	case V4L2_PIX_FMT_RGB555:
	case V4L2_PIX_FMT_ABGR555:
	case V4L2_PIX_FMT_RGB444:
	case V4L2_PIX_FMT_ABGR444:
	case V4L2_PIX_FMT_XBGR444:
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_RGBA32:
	case V4L2_PIX_FMT_ARGB32:
	case V4L2_PIX_FMT_RGBX32:
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_BGR32:
	case V4L2_PIX_FMT_BGRA32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_BGRX32:
	case V4L2_PIX_FMT_RGBA1010102:
	case V4L2_PIX_FMT_ARGB2101010:
	case V4L2_PIX_FMT_ABGR2101010:
	case V4L2_PIX_FMT_BGRA1010102:
		return 1;
	default:
		return 0;
	}
}

static int enc_setvui(struct v4l2_format *v4l2fmt, struct vsi_v4l2_ctx *ctx)
{
	u32 colorspace, quantization, transfer, matrixcoeff;
	struct v4l2_enccfg_vuiinfo *pvui = &ctx->mediacfg.m_encparams.m_vuiinfo;
	int i;

	quantization = v4l2fmt->fmt.pix_mp.quantization;
	colorspace = v4l2fmt->fmt.pix_mp.colorspace;
	transfer = v4l2fmt->fmt.pix_mp.xfer_func;
	matrixcoeff = v4l2fmt->fmt.pix_mp.ycbcr_enc;

	pvui->vuiColorDescripPresentFlag = 1;
	pvui->vuiVideoSignalTypePresentFlag = 1;

	if (quantization == V4L2_QUANTIZATION_FULL_RANGE)
		pvui->videoRange = VSI_V4L2ENC_CSC_FULL_RANGE;
	else if (quantization == V4L2_QUANTIZATION_LIM_RANGE)
		pvui->videoRange = VSI_V4L2ENC_CSC_LIM_RANGE;
	else
		pvui->videoRange = VSI_V4L2ENC_CSC_DEFAULT;

	pvui->vuiColorPrimaries = 0;
	if (colorspace == V4L2_COLORSPACE_SRGB)	//SRGB is duplicated with REC709
		pvui->vuiColorPrimaries = 1;
	else {
		for (i = 0; i < ARRAY_SIZE(colorprimaries); i++) {
			if (colorprimaries[i] == colorspace) {
				pvui->vuiColorPrimaries = i;
				break;
			}
		}
	}
	pvui->vuiTransferCharacteristics = 0;
	for (i = 0; i < ARRAY_SIZE(colortransfers); i++) {
		if (colortransfers[i] == transfer) {
			pvui->vuiTransferCharacteristics = i;
			break;
		}
	}
	pvui->vuiMatrixCoefficients = 0;
	for (i = 0; i < ARRAY_SIZE(colormatrixcoefs); i++) {
		if (colormatrixcoefs[i] == matrixcoeff) {
			pvui->vuiMatrixCoefficients = i;
			break;
		}
	}
	v4l2_klog(LOGLVL_DEBUG, "%x from %d:%d:%d:%d to %d:%d:%d:%d",
		v4l2fmt->fmt.pix_mp.pixelformat, colorspace, transfer, matrixcoeff, quantization,
		pvui->vuiColorPrimaries,
		pvui->vuiTransferCharacteristics,
		pvui->vuiMatrixCoefficients,
		pvui->videoRange);
	if (binputqueue(v4l2fmt->type) &&  enc_isRGBformat(v4l2fmt->fmt.pix_mp.pixelformat)) {
		switch (colorspace) {
		case V4L2_COLORSPACE_REC709:
			pvui->colorSpace = VSI_V4L2_COLORSPACE_REC709;
			break;
		case V4L2_COLORSPACE_BT2020:
			pvui->colorSpace = VSI_V4L2_COLORSPACE_BT2020;
			break;
		case V4L2_COLORSPACE_DCI_P3:
			pvui->colorSpace = VSI_V4L2_COLORSPACE_P3;
			break;
		case V4L2_COLORSPACE_JPEG:
		default:
			pvui->colorSpace = VSI_V4L2_COLORSPACE_REC601;
			break;
		}
	}
	flag_updateparam(m_vuiinfo)
	return 0;
}

void vsi_dec_getvui(struct v4l2_format *v4l2fmt, struct v4l2_daemon_dec_info *decinfo)
{
	u32 colorspace, quantization, transfer, matrixcoeff;

	colorspace = quantization = transfer = matrixcoeff = 0;
	quantization = (decinfo->video_range == 0 ?
				V4L2_QUANTIZATION_LIM_RANGE :
				V4L2_QUANTIZATION_FULL_RANGE);
	if (decinfo->colour_description_present_flag) {
		if (decinfo->colour_primaries < ARRAY_SIZE(colorprimaries))
			colorspace = colorprimaries[decinfo->colour_primaries];
		if (decinfo->transfer_characteristics < ARRAY_SIZE(colortransfers))
			transfer = colortransfers[decinfo->transfer_characteristics];
		if (decinfo->matrix_coefficients < ARRAY_SIZE(colormatrixcoefs))
			matrixcoeff = colormatrixcoefs[decinfo->matrix_coefficients];
	}
	v4l2fmt->fmt.pix_mp.quantization = quantization;
	v4l2fmt->fmt.pix_mp.colorspace = colorspace;
	v4l2fmt->fmt.pix_mp.xfer_func = transfer;
	v4l2fmt->fmt.pix_mp.ycbcr_enc = matrixcoeff;
	v4l2_klog(LOGLVL_DEBUG, "%x:%d:%d:%d:%d",
		v4l2fmt->fmt.pix_mp.pixelformat, colorspace, transfer, matrixcoeff, quantization);
}

void vsi_dec_updatevui(struct v4l2_daemon_dec_info *src, struct v4l2_daemon_dec_info *dst)
{
	v4l2_klog(LOGLVL_DEBUG, "%d:%d:%d:%d:%d",
		src->colour_primaries, src->transfer_characteristics, src->matrix_coefficients,
		src->colour_description_present_flag, src->video_range);
	dst->video_range = src->video_range;
	if (src->colour_description_present_flag) {
		dst->colour_description_present_flag = 1;
		dst->colour_primaries = src->colour_primaries;
		dst->transfer_characteristics = src->transfer_characteristics;
		dst->matrix_coefficients = src->matrix_coefficients;
	}
}

int vsi_set_Level(struct vsi_v4l2_ctx *ctx, int type, int level)
{
	u32 level_map = 0;
	s32 *p_level = NULL;

	switch (type) {
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
		level_map = vsi_v4l2_hwconfig.h264info.common.map_level;
		p_level = &ctx->mediacfg.avclevel;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
		level_map = vsi_v4l2_hwconfig.hevcinfo.common.map_level;
		p_level = &ctx->mediacfg.hevclevel;
		break;
	case V4L2_CID_MPEG_VIDEO_VP9_LEVEL:
		level_map = vsi_v4l2_hwconfig.vp9info.common.map_level;
		p_level = &ctx->mediacfg.vp9level;
		break;
	default:
		//fixme: other formats not ready yet
		return -EINVAL;
	}

	if (level_map & (1 << level)) {
		*p_level = level;
		return 0;
	}

	return -EINVAL;
}

int vsi_get_level(struct vsi_v4l2_ctx *ctx)
{
	v4l2_klog(LOGLVL_DEBUG, "in\n");
	switch (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_H264:
		return ctx->mediacfg.avclevel;
	case V4L2_DAEMON_CODEC_ENC_HEVC:
		return ctx->mediacfg.hevclevel;
	case V4L2_DAEMON_CODEC_ENC_VP9:
		return ctx->mediacfg.vp9level;
	case V4L2_DAEMON_CODEC_ENC_AV1:
		return ctx->mediacfg.av1level;
	default:
		return -EINVAL;
	}
}

static struct vsi_video_fmt vsi_raw_fmt[] = {
	{
		.fourcc = V4L2_PIX_FMT_NV12,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV12,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV12,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_GREY,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_GREY,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_400,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_Y10,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_Y10L,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_Y10L,
		.flag = 0,
	},
	{
		.name = "10-bit Greyscale(bits 15-6)",
		.fourcc = V4L2_PIX_FMT_Y010,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_Y10H,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_Y10H,
		.flag = 0,
	},
	{
		.name = "411 semi planar",
		.fourcc = V4L2_PIX_FMT_411SP,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_411SP,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_411SP,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV24,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV24,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_444SP,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUV420,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YUV420,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YUV420,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUV420M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YUV420M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YUV420M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUV422M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YUV422M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YUV422M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YVU422M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YVU422M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YVU422M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUV444M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YUV444M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YUV444M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YVU444M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YVU444M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YVU444M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV21,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV21,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV21,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YUYV,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YUYV,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YUYV,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB565,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGB565,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGB565,
		.flag = 0,
	},
	{
		.name = "BGR16",
		.fourcc = V4L2_PIX_FMT_BGR565,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGR565,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_BGR565,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB555,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGB555,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGB555,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGBA32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGBA32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGBA32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_BGR32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGR32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_BGR32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_ABGR32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_ABGR32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_ABGR32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGBX32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGBX32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGBX32,
		.flag = 0,
	},
	{
		.name = "VSI DTRC",
		.fourcc = V4L2_PIX_FMT_DTRC,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_DTRC,
		.dec_fmt = VSI_V4L2_DECOUT_DTRC,
		.flag = 0,
	},
	{
		.name = "P010",
		.fourcc = V4L2_PIX_FMT_P010,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_P010,
		.dec_fmt = VSI_V4L2_DECOUT_P010,
		.flag = 0,
	},
	{
		.name = "P010M",
		.fourcc = V4L2_PIX_FMT_P010M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_P010M,
		.dec_fmt = VSI_V4L2_DECOUT_P010M,
		.flag = 0,
	},
	{
		.name = "NV12 10Bit",
		.fourcc = V4L2_PIX_FMT_NV12X,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV12X,
		.dec_fmt = VSI_V4L2_DECOUT_NV12_10BIT,
		.flag = 0,
	},
	{
		.name = "DTRC 10Bit",
		.fourcc = V4L2_PIX_FMT_TILEX,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_TILEX,
		.dec_fmt = VSI_V4L2_DECOUT_DTRC_10BIT,
		.flag = 0,
	},
	{
		.name = "VSI DTRC compressed",
		.fourcc = V4L2_PIX_FMT_RFC,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RFC,
		.dec_fmt = VSI_V4L2_DECOUT_RFC,
		.flag = 0,
	},
	{
		.name = "VSI DTRC 10 bit compressed",
		.fourcc = V4L2_PIX_FMT_RFCX,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RFCX,
		.dec_fmt = VSI_V4L2_DECOUT_RFC_10BIT,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV16,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV16,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_422SP,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV16M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV16M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV16M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV61,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV61,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV61,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV61M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV61M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV61M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YVU420,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YVU420,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YVU420,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_YVU420M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_YVU420M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_YVU420M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV21M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV21M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV21M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_NV12M,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV12M,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV12M,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_UYVY,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_UYVY,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_UYVY,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_ABGR555,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_ABGR555,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_ABGR555,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB444,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGB444,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGB444,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_ABGR444,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGR444,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_ABGR444,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_XBGR444,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGR444,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_XBGR444,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_RGB32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGB32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGB32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_ARGB32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_ARGB32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_ARGB32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_XRGB32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_XRGB32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_XRGB32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_BGRA32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGRA32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_BGRA32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_XBGR32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_XBGR32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_XBGR32,
		.flag = 0,
	},
	{
		.fourcc = V4L2_PIX_FMT_BGRX32,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGRX32,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_BGRX32,
		.flag = 0,
	},
	{
		.name = "420 4x4 tiles",
		.fourcc = V4L2_PIX_FMT_NV12_4L4,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_NV12_4L4,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_NV12_4L4,
		.flag = 0,
	},
	{
		.name = "420 10bits 4x4 macro",
		.fourcc = V4L2_PIX_FMT_P010_4L4,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_P010_4L4,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_P010_4L4,
		.flag = 0,
	},
	{
		.name = "RGBA 10-10-10-2",
		.fourcc = V4L2_PIX_FMT_RGBA1010102,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_RGBA1010102,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_RGBA1010102,
		.flag = 0,
	},
	{
		.name = "ARGB 2-10-10-10",
		.fourcc = V4L2_PIX_FMT_ARGB2101010,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_ARGB2101010,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_ARGB2101010,
		.flag = 0,
	},
	{
		.name = "ABGR 2-10-10-10",
		.fourcc = V4L2_PIX_FMT_ABGR2101010,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_ABGR2101010,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_ABGR2101010,
		.flag = 0,
	},
	{
		.name = "BGRA 10-10-10-2",
		.fourcc = V4L2_PIX_FMT_BGRA1010102,
		.enc_fmt = VSI_V4L2ENC_PIX_FMT_BGRA1010102,
		.dec_fmt = VSI_V4L2_DEC_PIX_FMT_BGRA1010102,
		.flag = 0,
	},
};

static struct vsi_video_fmt vsi_coded_fmt[] = {
	{
		.fourcc = V4L2_PIX_FMT_HEVC,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_HEVC,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_HEVC,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_H264,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_H264,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_H264,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_H264_MVC,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_H264_MVC,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_JPEG,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_JPEG,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_JPEG,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "VP6",
		.fourcc = V4L2_PIX_FMT_VP6,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_VP6,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "VP7",
		.fourcc = V4L2_PIX_FMT_VP7,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_VP7,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "WEBP",
		.fourcc = V4L2_PIX_FMT_WEBP,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_WEBP,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_WEBP,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_VP8,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_VP8,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_VP8,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_VP9,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_VP9,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_VP9,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "av1",
		.fourcc = V4L2_PIX_FMT_AV1,
		.enc_fmt = V4L2_DAEMON_CODEC_ENC_AV1,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_AV1,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_MPEG2,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_MPEG2,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "sorenson",
		.fourcc = V4L2_PIX_FMT_SORENSON,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_SORENSON,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "DIVX",
		.fourcc = V4L2_PIX_FMT_DIVX,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_DIVX,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_XVID,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_XVID,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_MPEG4,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_MPEG4,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_H263,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_H263,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_VC1_ANNEX_G,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_VC1_G,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.fourcc = V4L2_PIX_FMT_VC1_ANNEX_L,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_VC1_L,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "rv",
		.fourcc = V4L2_PIX_FMT_RV,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_RV,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "avs",
		.fourcc = V4L2_PIX_FMT_AVS,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_AVS,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
	{
		.name = "avs2",
		.fourcc = V4L2_PIX_FMT_AVS2,
		.enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE,
		.dec_fmt = V4L2_DAEMON_CODEC_DEC_AVS2,
		.flag = (V4L2_FMT_FLAG_DYN_RESOLUTION | V4L2_FMT_FLAG_COMPRESSED),
	},
};

static int istiledfmt(int pixelformat)
{
	switch (pixelformat) {
	case VSI_V4L2_DECOUT_DTRC:
	case VSI_V4L2_DECOUT_DTRC_10BIT:
	case VSI_V4L2_DECOUT_RFC:
	case VSI_V4L2_DECOUT_RFC_10BIT:
		return 1;
	default:
		return 0;
	}
}

static int isJpegOnlyFmt(int outfmt)
{
	switch (outfmt) {
	case VSI_V4L2_DEC_PIX_FMT_400:
	case VSI_V4L2_DEC_PIX_FMT_411SP:
	case VSI_V4L2_DEC_PIX_FMT_NV16M:
	case VSI_V4L2_DEC_PIX_FMT_422SP:
	case VSI_V4L2_DEC_PIX_FMT_444SP:
		return 1;
	default:
		return 0;
	}
}

void vsi_enum_decfsize(struct v4l2_frmsizeenum *f, u32 pixel_format)
{
	struct vsi_v4l2_decfmt_common *preso;

	f->type = V4L2_FRMSIZE_TYPE_STEPWISE;
	switch (pixel_format) {
	case V4L2_PIX_FMT_HEVC:
		preso = &vsi_v4l2_hwconfig.decinfo_HEVC.common;
		break;
	case V4L2_PIX_FMT_H264:
		preso = &vsi_v4l2_hwconfig.decinfo_H264.common;
		break;
	case V4L2_PIX_FMT_H264_MVC:
		preso = &vsi_v4l2_hwconfig.decinfo_H264_MVC.common;
		break;
	case V4L2_PIX_FMT_JPEG:
		preso = &vsi_v4l2_hwconfig.decinfo_JPEG.common;
		break;
	case V4L2_PIX_FMT_VP6:
		preso = &vsi_v4l2_hwconfig.decinfo_VP6.common;
		break;
	case V4L2_PIX_FMT_VP7:
		preso = &vsi_v4l2_hwconfig.decinfo_VP7.common;
		break;
	case V4L2_PIX_FMT_WEBP:
		preso = &vsi_v4l2_hwconfig.decinfo_WEBP.common;
		break;
	case V4L2_PIX_FMT_VP8:
		preso = &vsi_v4l2_hwconfig.decinfo_VP8.common;
		break;
	case V4L2_PIX_FMT_VP9:
		preso = &vsi_v4l2_hwconfig.decinfo_VP9.common;
		break;
	case V4L2_PIX_FMT_AV1:
		preso = &vsi_v4l2_hwconfig.decinfo_AV1.common;
		break;
	case V4L2_PIX_FMT_MPEG2:
		preso = &vsi_v4l2_hwconfig.decinfo_MPEG2.common;
		break;
	case V4L2_PIX_FMT_SORENSON:
		preso = &vsi_v4l2_hwconfig.decinfo_SORENSON.common;
		break;
	case V4L2_PIX_FMT_DIVX:
		preso = &vsi_v4l2_hwconfig.decinfo_DIVX.common;
		break;
	case V4L2_PIX_FMT_XVID:
		preso = &vsi_v4l2_hwconfig.decinfo_XVID.common;
		break;
	case V4L2_PIX_FMT_MPEG4:
		preso = &vsi_v4l2_hwconfig.decinfo_MPEG4.common;
		break;
	case V4L2_PIX_FMT_H263:
		preso = &vsi_v4l2_hwconfig.decinfo_H263.common;
		break;
	case V4L2_PIX_FMT_VC1_ANNEX_G:
		preso = &vsi_v4l2_hwconfig.decinfo_VC1_G.common;
		break;
	case V4L2_PIX_FMT_VC1_ANNEX_L:
		preso = &vsi_v4l2_hwconfig.decinfo_VC1_L.common;
		break;
	case V4L2_PIX_FMT_RV:
		preso = &vsi_v4l2_hwconfig.decinfo_RV.common;
		break;
	case V4L2_PIX_FMT_AVS:
		preso = &vsi_v4l2_hwconfig.decinfo_AVS.common;
		break;
	case V4L2_PIX_FMT_AVS2:
		preso = &vsi_v4l2_hwconfig.decinfo_AVS2.common;
		break;
	default:
		f->stepwise.min_width = 48;
		f->stepwise.max_width = 1920;
		f->stepwise.min_height = 48;
		f->stepwise.max_height = 1920;
		f->stepwise.step_height = 2;
		f->stepwise.step_width = 2;
		return;
	}
	f->stepwise.min_width = preso->min_width;
	f->stepwise.max_width = preso->max_width;
	f->stepwise.step_width = preso->width_align;
	f->stepwise.min_height = preso->min_height;
	f->stepwise.max_height = preso->max_height;
	f->stepwise.step_height = preso->height_align;
	v4l2_klog(LOGLVL_DEBUG, "%x->%d:%d:%d:%d:%d:%d",
		pixel_format,
		f->stepwise.min_width, f->stepwise.max_width, f->stepwise.step_width,
		f->stepwise.min_height, f->stepwise.max_height, f->stepwise.step_height);

}

void vsi_enum_encfsize(struct v4l2_frmsizeenum *f, u32 pixel_format)
{
	struct vsi_v4l2_codedfmt_common *preso;

	switch (pixel_format) {
	case V4L2_PIX_FMT_HEVC:
		preso = &vsi_v4l2_hwconfig.hevcinfo.common;
		break;
	case V4L2_PIX_FMT_H264:
		preso = &vsi_v4l2_hwconfig.h264info.common;
		break;
	case V4L2_PIX_FMT_VP8:
		preso = &vsi_v4l2_hwconfig.vp8info.common;
		break;
	case V4L2_PIX_FMT_VP9:
		preso = &vsi_v4l2_hwconfig.vp9info.common;
		break;
	case V4L2_PIX_FMT_AV1:
		preso = &vsi_v4l2_hwconfig.av1info.common;
		break;
	case V4L2_PIX_FMT_JPEG:
		preso = &vsi_v4l2_hwconfig.jpeginfo.common;
		break;
	case V4L2_PIX_FMT_WEBP:
		preso = &vsi_v4l2_hwconfig.webpinfo.common;
		break;
	default:
		f->stepwise.min_width =
		f->stepwise.max_width =
		f->stepwise.min_height =
		f->stepwise.max_height = 0;
		f->stepwise.step_width =
		f->stepwise.step_height = 16;
		return;
	}
	f->stepwise.min_width = preso->min_width;
	f->stepwise.max_width = preso->max_width;
	f->stepwise.step_width = preso->width_align;
	f->stepwise.min_height = preso->min_height;
	f->stepwise.max_height = preso->max_height;
	f->stepwise.step_height = preso->height_align;
	f->type = V4L2_FRMSIZE_TYPE_STEPWISE;
	v4l2_klog(LOGLVL_DEBUG, "%x->%d:%d:%d:%d:%d:%d",
		pixel_format,
		f->stepwise.min_width, f->stepwise.max_width, f->stepwise.step_width,
		f->stepwise.min_height, f->stepwise.max_height, f->stepwise.step_height);
}

int vsi_set_profile(struct vsi_v4l2_ctx *ctx, int type, int profile)
{
	u32 profile_map = 0;
	s32 *p_profile = NULL;

	v4l2_klog(LOGLVL_DEBUG, "%d:%d", type, profile);

	switch (type) {
	case V4L2_CID_MPEG_VIDEO_H264_PROFILE:
		profile_map = vsi_v4l2_hwconfig.h264info.common.map_profile;
		p_profile = &ctx->mediacfg.profile_h264;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE:
		profile_map = vsi_v4l2_hwconfig.hevcinfo.common.map_profile;
		p_profile = &ctx->mediacfg.profile_hevc;
		break;
	case V4L2_CID_MPEG_VIDEO_VP8_PROFILE:
		profile_map = vsi_v4l2_hwconfig.vp8info.common.map_profile;
		p_profile = &ctx->mediacfg.profile_vp8;
		break;
	case V4L2_CID_MPEG_VIDEO_VP9_PROFILE:
		profile_map = vsi_v4l2_hwconfig.vp9info.common.map_profile;
		p_profile = &ctx->mediacfg.profile_vp9;
		break;
	default:
		return -EINVAL;
	}

	if (profile_map & (1 << profile)) {
		*p_profile = profile;
		return 0;
	}
	return -EINVAL;
}

static int check_enc_raw_format(struct vsi_v4l2_ctx *ctx, u32 rawfmt)
{
	u8 mask = 1 << PIX_FMT_BIT_VIDEO;

	if (!ctx)
		mask |= 1 << PIX_FMT_BIT_IMAGE;
	else if (isimage(ctx->mediacfg.outfmt_fourcc))
		mask = 1 << PIX_FMT_BIT_IMAGE;

	if (rawfmt < VSI_V4L2ENC_PIX_FMT_MAX)
		return (mask & vsi_v4l2_hwconfig.has_pix_fmt[rawfmt]);
	// not supported.
	return 0;
}

static int check_dec_raw_format(struct vsi_v4l2_ctx *ctx, u32 rawfmt)
{
	if (rawfmt < VSI_V4L2_DEC_PIX_FMT_MAX)
		return (vsi_v4l2_hwconfig.dec_has_pix_fmt[rawfmt]);
	// not supported.
	return 0;
}

struct vsi_video_fmt *vsi_find_format(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	u32 fourcc;
	int i;
	int braw = brawfmt(ctx->flag, fmt->type);
	struct vsi_video_fmt *retfmt = NULL;

	fourcc = fmt->fmt.pix_mp.pixelformat;
	if (braw) {
		for (i = 0; i < ARRAY_SIZE(vsi_raw_fmt); i++) {
			if (vsi_raw_fmt[i].fourcc == fourcc) {
				if (isencoder(ctx) && check_enc_raw_format(ctx, vsi_raw_fmt[i].enc_fmt))
					retfmt = &vsi_raw_fmt[i];
				if (isdecoder(ctx) && check_dec_raw_format(ctx, vsi_raw_fmt[i].dec_fmt))
					retfmt = &vsi_raw_fmt[i];
				break;
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(vsi_coded_fmt); i++) {
			if (vsi_coded_fmt[i].fourcc == fourcc) {
				if (isencoder(ctx) && vsi_coded_fmt[i].enc_fmt != V4L2_DAEMON_CODEC_UNKNOW_TYPE)
					retfmt = &vsi_coded_fmt[i];
				if (isdecoder(ctx) && vsi_coded_fmt[i].dec_fmt != V4L2_DAEMON_CODEC_UNKNOW_TYPE)
					retfmt = &vsi_coded_fmt[i];
				break;
			}
		}
	}
	return retfmt;
}

struct vsi_video_fmt *vsi_enum_dec_format(int idx, int braw, struct vsi_v4l2_ctx *ctx)
{
	u32 inputformat = ctx->mediacfg.decparams.dec_info.io_buffer.inputFormat;
	int i = 0, k =  -1, outfmt;

	if (braw == 1) {
		for (; i < ARRAY_SIZE(vsi_raw_fmt); i++) {
			outfmt = vsi_raw_fmt[i].dec_fmt;
			if (!check_dec_raw_format(NULL, outfmt))
				continue;
			if (istiledfmt(outfmt)) {
				if (inputformat != V4L2_DAEMON_CODEC_DEC_HEVC &&
					inputformat != V4L2_DAEMON_CODEC_DEC_VP9)
					continue;
				if ((outfmt == VSI_V4L2_DECOUT_DTRC ||
					outfmt == VSI_V4L2_DECOUT_DTRC_10BIT) && !vsi_v4l2_hwconfig.has_DTRC)
					continue;
				if ((outfmt == VSI_V4L2_DECOUT_RFC ||
					outfmt == VSI_V4L2_DECOUT_RFC_10BIT) && !vsi_v4l2_hwconfig.has_RFC)
					continue;
			}
			if (test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag)) {
				if (inputformat == V4L2_DAEMON_CODEC_DEC_JPEG &&
					outfmt != ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt)
					continue;
				if ((outfmt == VSI_V4L2_DECOUT_NV12_10BIT ||
					outfmt == VSI_V4L2_DECOUT_P010 ||
					outfmt == VSI_V4L2_DECOUT_P010M ||
					outfmt == VSI_V4L2_DEC_PIX_FMT_RGBA1010102 ||
					outfmt == VSI_V4L2_DEC_PIX_FMT_ARGB2101010 ||
					outfmt == VSI_V4L2_DEC_PIX_FMT_ABGR2101010 ||
					outfmt == VSI_V4L2_DEC_PIX_FMT_BGRA1010102) &&
					ctx->mediacfg.decparams.dec_info.dec_info.bit_depth < 10)
					continue;
				if ((outfmt == VSI_V4L2_DECOUT_DTRC_10BIT ||
					outfmt == VSI_V4L2_DECOUT_RFC_10BIT) &&
					ctx->mediacfg.decparams.dec_info.dec_info.bit_depth != 10)
					continue;
				if ((outfmt == VSI_V4L2_DECOUT_DTRC ||
					outfmt == VSI_V4L2_DECOUT_RFC) &&
					ctx->mediacfg.decparams.dec_info.dec_info.bit_depth != 8)
					continue;
			}
			k++;

			if (k == idx) {
				v4l2_klog(LOGLVL_DEBUG, "%d:%d=%x", idx, braw, vsi_raw_fmt[i].fourcc);
				return &vsi_raw_fmt[i];
			}
		}
	} else {
		for (; i < ARRAY_SIZE(vsi_coded_fmt); i++) {
			if (vsi_coded_fmt[i].dec_fmt != V4L2_DAEMON_CODEC_UNKNOW_TYPE)
				k++;
			if (k == idx) {
				v4l2_klog(LOGLVL_DEBUG, "%d:%d=%x", idx, braw, vsi_coded_fmt[i].fourcc);
				return &vsi_coded_fmt[i];
			}
		}
	}
	return NULL;
}

struct vsi_video_fmt *vsi_enum_encformat(int idx, int braw)
{
	int i = 0, k =  -1;

	if (braw == 1) {
		for (; i < ARRAY_SIZE(vsi_raw_fmt); i++) {
			if (check_enc_raw_format(NULL, vsi_raw_fmt[i].enc_fmt))
				k++;
			if (k == idx) {
				v4l2_klog(LOGLVL_DEBUG, "%d:%d=%x", idx, braw, vsi_raw_fmt[i].fourcc);
				return &vsi_raw_fmt[i];
			}
		}
	} else {
		for (; i < ARRAY_SIZE(vsi_coded_fmt); i++) {
			if (vsi_coded_fmt[i].enc_fmt != V4L2_DAEMON_CODEC_UNKNOW_TYPE)
				k++;
			if (k == idx) {
				v4l2_klog(LOGLVL_DEBUG, "%d:%d=%x", idx, braw, vsi_coded_fmt[i].fourcc);
				return &vsi_coded_fmt[i];
			}
		}
	}
	return NULL;
}

static void vsi_set_default_parameter_enc(
	struct vsi_v4l2_encparams  *enccfg)
{
	init_encparams(enccfg);
	//these 4 are beyond ctrl value. Let's keep 25/1 currently.
	enccfg->m_framerate.outputRateNumer = vsi_v4l2_hwconfig.outputRateNumer;
	enccfg->m_framerate.outputRateDenom = vsi_v4l2_hwconfig.outputRateDenom;
	enccfg->m_framerate.inputRateNumer = vsi_v4l2_hwconfig.inputRateNumer;
	enccfg->m_framerate.inputRateDenom = vsi_v4l2_hwconfig.inputRateDenom;
	enccfg->m_srcinfo.inputFormat = VSI_V4L2ENC_PIX_FMT_NV12M;
	enccfg->m_codecfmt.codecFormat = V4L2_DAEMON_CODEC_ENC_HEVC;
	enccfg->m_ipcminfo.pcm_loop_filter_disabled_flag = 1;

	//these def are set to invalid value so they'll be updated in writeout_encparam
	enccfg->m_profile.profile = -255;
	enccfg->m_level.level = -255;
	enccfg->m_qphdrip.qpHdrI = -255;
	enccfg->m_qphdrip.qpHdrP = -255;
	enccfg->m_qphdrip.qpHdrB = -255;
	enccfg->m_refno.refFrameAmount = -255;
	enccfg->m_qprange.qpMin = -255;
	enccfg->m_qprange.qpMax = -255;
	enccfg->m_loopfilter.disableDeblockingFilter = -255;
	enccfg->m_refno.refFrameAmount = -255;
	//other default values are set in vsi_enc_ctx_defctrl
}

void vsiv4l2_initcfg(struct vsi_v4l2_ctx *ctx)
{
	struct vsi_v4l2_mediacfg *cfg = &ctx->mediacfg;

	if (isencoder(ctx)) {
		cfg->srcplanes = 2;		//default src format is NV12 now
		vsi_set_default_parameter_enc(&cfg->m_encparams);
		cfg->outfmt_fourcc = -1;
	} else {
		cfg->srcplanes = 1;
		cfg->decparams.dec_info.io_buffer.inputFormat = V4L2_DAEMON_CODEC_DEC_HEVC;
		cfg->decparams.dec_info.io_buffer.outBufFormat = VSI_V4L2_DEC_PIX_FMT_NV12;
		cfg->decparams.dec_info.io_buffer.outputPixelDepth = DEFAULT_PIXELDEPTH;
		cfg->decparams.dec_info.dec_info.bit_depth = DEFAULT_PIXELDEPTH;
		cfg->outfmt_fourcc = V4L2_PIX_FMT_NV12;
	}

	cfg->src_pixeldepth = DEFAULT_PIXELDEPTH;
	cfg->infmt_fourcc = -1;

	cfg->dstplanes = 1;
	cfg->field = V4L2_FIELD_NONE;
	cfg->colorspace = V4L2_COLORSPACE_REC709;
	cfg->quantization = V4L2_QUANTIZATION_LIM_RANGE;
	cfg->minbuf_4capture = 1;
	cfg->minbuf_4output = 1;
}

static int get_fmtprofile(struct vsi_v4l2_mediacfg *pcfg)
{
	int codecFormat = pcfg->m_encparams.m_codecfmt.codecFormat;

	switch (codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_HEVC:
		return pcfg->profile_hevc;
	case V4L2_DAEMON_CODEC_ENC_H264:
		return pcfg->profile_h264;
	case V4L2_DAEMON_CODEC_ENC_VP8:
	case V4L2_DAEMON_CODEC_ENC_WEBP:
		return pcfg->profile_vp8;
	case V4L2_DAEMON_CODEC_ENC_VP9:
		return pcfg->profile_vp9;
	case V4L2_DAEMON_CODEC_ENC_AV1:
	case V4L2_DAEMON_CODEC_ENC_JPEG:
		return 0;
	default:
		return -EINVAL;
	}
}

static int tailzero(u32 num)
{
	int exp = 0, i;

	for (i = 0; i < 32; i++) {
		if (num & 0x1)
			break;
		exp++;
		num >>= 1;
	}
	return exp;
}

static void vsi_getShifter(u32 div, u32 *shifter, u32 *tail)
{
	if (div == 2) {
		*shifter = 1;
		*tail = 1;
	} else if (div == 4) {
		*shifter = 2;
		*tail = 3;
	} else {
		*shifter = 0;
		*tail = 0;
	}
}

/*
 * JIRA amazon 467, chroma's bytesperline is half of luma's for YUV420 alike formats according to v4l2 spec, while
 * VSI ctrlsw supports only identical alignment for luma and chroma, so multiple chroma align to make luma
 * align satisfy ctrlsw requirement in some config
 */
static int adjustChromaStride4MultiPFormats(u32 pixelformat, u32 codecformat, u32 *lumastride)
{
	const struct v4l2_format_info *fmtinfo = vsi_v4l2_fmtinfo(pixelformat);
	u32 shifter, tail;
	s32 chromexp;
	int daemonexp;
	u32 chromAlign;

	if (fmtinfo && fmtinfo->mem_planes == 1 && fmtinfo->comp_planes == 3 && fmtinfo->hdiv > 1) {
		chromAlign = (codecformat == V4L2_DAEMON_CODEC_ENC_JPEG ?
			vsi_v4l2_hwconfig.enc_image_stride_align : vsi_v4l2_hwconfig.enc_video_stride_align);
		daemonexp = tailzero(chromAlign);

		vsi_getShifter(fmtinfo->hdiv, &shifter, &tail);
		chromexp = tailzero((*lumastride + tail) >> shifter);
		if (chromexp < daemonexp) {
			int lumexp = tailzero(*lumastride);

			lumexp += (daemonexp - chromexp);
			*lumastride = ((*lumastride + ((1 << lumexp) - 1)) >> lumexp) << lumexp;
			return 1;
		}
	}
	return 0;
}

static void calcPlanesize(struct vsi_v4l2_ctx *ctx, u32 pixelformat, int braw, int bytesperline, int height, u32 size[], int planeno, int bnewcfg)
{
	u32 basesize = bytesperline * height;
	u32 compbytesperline, compheight;
	struct vsi_v4l2_encparams *param = &ctx->mediacfg.m_encparams;
	const struct v4l2_format_info *pfinfo;
	s32 i;
	struct v4l2_frmsizeenum fenum;
	u32 compsize[MAX_PLANENO] = {0}, accsize = 0;
	u32 hshifter, htail, vshifter, vtail;

	if (bnewcfg) {
		for (i = 0; i < planeno; i++)
			size[i] = 0;
	}
	if (!braw) {
		if (bnewcfg)
			size[0] = ALIGN(basesize + ENC_EXTRA_HEADER_SIZE, PAGE_SIZE);
		else
			size[0] = max_t(int, (basesize), size[0]);
		return;
	}
	pfinfo = vsi_v4l2_fmtinfo(pixelformat);
	if (!pfinfo) {
		v4l2_klog(LOGLVL_ERROR, "wrong format %x", pixelformat);
		return;
	}
	vsi_getShifter(pfinfo->hdiv, &hshifter, &htail);
	vsi_getShifter(pfinfo->vdiv, &vshifter, &vtail);
	compbytesperline = (bytesperline + htail) >> hshifter;
	compheight = (height + vtail) >> vshifter;
	if (!isdecoder(ctx)) {
		vsi_enum_encfsize(&fenum, ctx->mediacfg.outfmt_fourcc);
		if (pfinfo->mem_planes == pfinfo->comp_planes) {
			compbytesperline = vsiv4l2_enc_getalign(param->m_srcinfo.inputFormat,
							param->m_codecfmt.codecFormat, compbytesperline, 1);
		} else {
			compbytesperline = ALIGN(compbytesperline, fenum.stepwise.step_width);
		}
	} else {
		vsi_enum_decfsize(&fenum, ctx->mediacfg.infmt_fourcc);
		compbytesperline = ALIGN(compbytesperline, fenum.stepwise.step_width);
	}
	accsize = compsize[0] = basesize;
	for (i = 1; i < pfinfo->comp_planes; i++) {
		if (pfinfo->bpp[0] != 0) {
			compsize[i] = compbytesperline * compheight * pfinfo->bpp[i] / pfinfo->bpp[0];
		} else //to avoid panic, not really exist
			compsize[i] = compbytesperline * compheight;

		accsize += compsize[i];
	}
	if (planeno == 1 || pfinfo->mem_planes == 1)
		size[0] = max_t(int, (accsize), size[0]);
	else {
		//pfinfo->mem_planes should equal comp_planes, and planeno > 1
		planeno = (planeno <= pfinfo->mem_planes ? planeno : pfinfo->mem_planes);
		for (i = 0; i < planeno; i++)
			size[i] = max_t(int, (compsize[i]), size[i]);
	}

	v4l2_klog(LOGLVL_DEBUG, "%d:%d:%d:%d", planeno, size[0], size[1], size[2]);
}

static const struct v4l2_format_info *vsi_v4l2_fmtinfo(u32 fourcc)
{
#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
	static const struct v4l2_format_info vsi_fmtinfo[] = {
		{.format = V4L2_PIX_FMT_Y10, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 1, .bpp = {2, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_Y010, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 1, .bpp = {2, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_411SP, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 3, .bpp = {1, 1, 1, 0}, .hdiv = 4, .vdiv = 1},
		{.format = V4L2_PIX_FMT_NV12X, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_NV12_4L4, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_P010, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {2, 2, 0, 0}, .hdiv = 2, .vdiv = 1},
		{.format = V4L2_PIX_FMT_P010M, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 2, .comp_planes = 2, .bpp = {2, 2, 0, 0}, .hdiv = 2, .vdiv = 1},
		{.format = V4L2_PIX_FMT_P010_4L4, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {2, 4, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_DTRC, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_TILEX, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_RFC, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_RFCX, .pixel_enc = V4L2_PIXEL_ENC_YUV, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_BGR565, .pixel_enc = V4L2_PIXEL_ENC_RGB, .mem_planes = 1, .comp_planes = 1, .bpp = {2, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_RGBA1010102, .pixel_enc = V4L2_PIXEL_ENC_RGB, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_ARGB2101010, .pixel_enc = V4L2_PIXEL_ENC_RGB, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_ABGR2101010, .pixel_enc = V4L2_PIXEL_ENC_RGB, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_BGRA1010102, .pixel_enc = V4L2_PIXEL_ENC_RGB, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
	};
#else
	static const struct v4l2_format_info vsi_fmtinfo[] = {
		{.format = V4L2_PIX_FMT_Y10, .mem_planes = 1, .comp_planes = 1, .bpp = {2, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_Y010, .mem_planes = 1, .comp_planes = 1, .bpp = {2, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_411SP, .mem_planes = 1, .comp_planes = 3, .bpp = {1, 1, 1, 0}, .hdiv = 4, .vdiv = 1},
		{.format = V4L2_PIX_FMT_NV12X, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_NV12_4L4, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_P010, .mem_planes = 1, .comp_planes = 2, .bpp = {2, 2, 0, 0}, .hdiv = 2, .vdiv = 1},
		{.format = V4L2_PIX_FMT_P010M, .mem_planes = 2, .comp_planes = 2, .bpp = { 2, 2, 0, 0}, .hdiv = 2, .vdiv = 1},
		{.format = V4L2_PIX_FMT_P010_4L4, .mem_planes = 1, .comp_planes = 2, .bpp = {2, 4, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_DTRC, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_TILEX, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_RFC, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_RFCX, .mem_planes = 1, .comp_planes = 2, .bpp = {1, 2, 0, 0}, .hdiv = 2, .vdiv = 2},
		{.format = V4L2_PIX_FMT_BGR565, .mem_planes = 1, .comp_planes = 1, .bpp = {2, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_RGBA1010102, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_ARGB2101010, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_ABGR2101010, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
		{.format = V4L2_PIX_FMT_BGRA1010102, .mem_planes = 1, .comp_planes = 1, .bpp = {4, 0, 0, 0}, .hdiv = 1, .vdiv = 1},
	};
#endif

	s32 i;
	const struct v4l2_format_info *pv4l2_format_info = v4l2_format_info(fourcc);

	if (pv4l2_format_info == NULL) {
		for (i = 0; i < ARRAY_SIZE(vsi_fmtinfo); i++) {
			if (fourcc == vsi_fmtinfo[i].format) {
				pv4l2_format_info = &vsi_fmtinfo[i];
				break;
			}
		}
	}
	return pv4l2_format_info;
}

static s32 vsiv4l2_enc_getalign(u32 srcfmt, u32 dstfmt, s32 bytesperline, s32 planeidx)
{
	s32 align, alignfactor;

	switch (dstfmt) {
	case V4L2_DAEMON_CODEC_ENC_HEVC:
	case V4L2_DAEMON_CODEC_ENC_VP9:
	case V4L2_DAEMON_CODEC_ENC_AV1:
	case V4L2_DAEMON_CODEC_ENC_H264:
	case V4L2_DAEMON_CODEC_ENC_VP8:
	case V4L2_DAEMON_CODEC_ENC_WEBP:
		align = vsi_v4l2_hwconfig.enc_video_stride_align & 0xffff;
		alignfactor = ((u32)vsi_v4l2_hwconfig.enc_video_stride_align >> (16 + planeidx * 4)) & 0xf;
		break;
	case V4L2_DAEMON_CODEC_ENC_JPEG:
		align = vsi_v4l2_hwconfig.enc_image_stride_align & 0xffff;
		alignfactor = ((u32)vsi_v4l2_hwconfig.enc_image_stride_align >> (16 + planeidx * 4)) & 0xf;
		break;
	default:
		align = vsi_v4l2_hwconfig.enc_video_stride_align & 0xffff;
		alignfactor = ((u32)vsi_v4l2_hwconfig.enc_video_stride_align >> (16 + planeidx * 4)) & 0xf;
		break;
	}
	if (alignfactor)
		align /= alignfactor;
	else
		v4l2_klog(LOGLVL_INFO, "plane %d factor is 0", planeidx);
	if (vsi_v4l2_hwconfig.b_pixelAlign)
		align *= raw_fmt_tbl[srcfmt][1];
	bytesperline = ALIGN(bytesperline, align);
	v4l2_klog(LOGLVL_DEBUG, "plane %d's stride=%d", planeidx, bytesperline);
	return bytesperline;
}

static s32 vsiv4l2_decide_encpixeldepth(u32 srcfcc, u32 dstfcc)
{
	s32 mode, depth;
	u32 depthmap = 0xffffffff;

	switch (srcfcc) {
	case V4L2_PIX_FMT_NV12X:
	case V4L2_PIX_FMT_TILEX:
	case V4L2_PIX_FMT_RFCX:
		mode = BITDEPTH_10;
		depth = 10;
		break;
	case V4L2_PIX_FMT_P010:
	case V4L2_PIX_FMT_P010M:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_BGR565:
	case V4L2_PIX_FMT_RGB555:
	case V4L2_PIX_FMT_ABGR555:
	case V4L2_PIX_FMT_RGB444:
	case V4L2_PIX_FMT_ABGR444:
	case V4L2_PIX_FMT_XBGR444:
	case V4L2_PIX_FMT_Y10:
	case V4L2_PIX_FMT_Y010:
		mode = BITDEPTH_16;
		depth = 16;
		break;
	case V4L2_PIX_FMT_RGB32:
	case V4L2_PIX_FMT_RGBA32:
	case V4L2_PIX_FMT_ARGB32:
	case V4L2_PIX_FMT_RGBX32:
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_BGR32:
	case V4L2_PIX_FMT_BGRA32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_BGRX32:
	case V4L2_PIX_FMT_RGBA1010102:
	case V4L2_PIX_FMT_ARGB2101010:
	case V4L2_PIX_FMT_ABGR2101010:
	case V4L2_PIX_FMT_BGRA1010102:
		mode = BITDEPTH_32;
		depth = 32;
		break;
	default:
		mode = BITDEPTH_8;
		depth = 8;
		break;
	}
	switch (dstfcc) {
	case V4L2_PIX_FMT_HEVC:
		depthmap = vsi_v4l2_hwconfig.hevcinfo.common.map_bitdepth;
		break;
	case V4L2_PIX_FMT_H264:
		depthmap = vsi_v4l2_hwconfig.h264info.common.map_bitdepth;
		break;
	case V4L2_PIX_FMT_VP8:
		depthmap = vsi_v4l2_hwconfig.vp8info.common.map_bitdepth;
		break;
	case V4L2_PIX_FMT_VP9:
		depthmap = vsi_v4l2_hwconfig.vp9info.common.map_bitdepth;
		break;
	case V4L2_PIX_FMT_AV1:
		depthmap = vsi_v4l2_hwconfig.av1info.common.map_bitdepth;
		break;
	case V4L2_PIX_FMT_JPEG:
		depthmap = vsi_v4l2_hwconfig.jpeginfo.common.map_bitdepth;
		break;
	case V4L2_PIX_FMT_WEBP:
		depthmap = vsi_v4l2_hwconfig.webpinfo.common.map_bitdepth;
		break;
	default:
		break;
	}
	if (depthmap & (1 << mode))
		return depth;
	return -1;
}

static u32 vsiv4l2_getenc_scaleoutput(struct vsi_v4l2_ctx *ctx, u32 *pstride)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	struct v4l2_enccfg_scaleoutput *pscale = &pcfg->m_encparams.m_scaleoutput;
	u32 size;
	u32 sfmt = (pscale->scaleOutputFormat == 0 ? VSI_V4L2ENC_PIX_FMT_YUYV : VSI_V4L2ENC_PIX_FMT_YUV420);
	u32 fmtcc = (pscale->scaleOutputFormat == 0 ? V4L2_PIX_FMT_YUYV : V4L2_PIX_FMT_YUV420);
	s32 bytesperline = pscale->scaleWidth * raw_fmt_tbl[sfmt][1];

	bytesperline = vsiv4l2_enc_getalign(sfmt, pcfg->m_encparams.m_codecfmt.codecFormat, bytesperline, 0);
	//here's also affected by Amazon ticket 467 for YUV420
	adjustChromaStride4MultiPFormats(fmtcc, pcfg->m_encparams.m_codecfmt.codecFormat, &bytesperline);
	calcPlanesize(ctx, fmtcc, 1, bytesperline, pscale->scaleHeight, &size, 1, 1);
	*pstride = bytesperline;
	return size;
}

static int vsiv4l2_enc_set3plane_bytesperline(struct v4l2_format *fmt, struct vsi_v4l2_mediacfg *pcfg)
{
	const struct v4l2_format_info *pv4l2_format_info = vsi_v4l2_fmtinfo(fmt->fmt.pix_mp.pixelformat);
	u8 div;

	if (!pv4l2_format_info)
		return 0;
	div = pv4l2_format_info->hdiv;
	if (pv4l2_format_info && pv4l2_format_info->mem_planes == 3 && div != 0) {
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline = pcfg->bytesperline;
		if (div == 1) {
			fmt->fmt.pix_mp.plane_fmt[1].bytesperline = pcfg->bytesperline;
			fmt->fmt.pix_mp.plane_fmt[2].bytesperline = pcfg->bytesperline;
		} else if (div == 2) {
			fmt->fmt.pix_mp.plane_fmt[1].bytesperline =
				vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat, pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline/2, 1);
			fmt->fmt.pix_mp.plane_fmt[2].bytesperline =
				vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat, pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline/2, 2);
		} else if (div == 4) {
			fmt->fmt.pix_mp.plane_fmt[1].bytesperline =
				vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat, pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline/4, 1);
			fmt->fmt.pix_mp.plane_fmt[2].bytesperline =
				vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat, pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline/4, 2);
		} else {	// not exist, just in case
			fmt->fmt.pix_mp.plane_fmt[1].bytesperline =
				vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat, pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline/div, 1);
			fmt->fmt.pix_mp.plane_fmt[2].bytesperline =
				vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat, pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline/div, 2);
		}
		return 1;
	}
	return 0;
}

static int vsiv4l2_calc_ufbc_size(struct vsi_v4l2_ctx *ctx,
						struct v4l2_format *fmt, s32 inputFormat)
{
	struct v4l2_vsi_stream_meta_info_enc *pmeta_enc;
	struct v4l2_enc_ufbc_cfg *ufbc_cfg;
	const struct v4l2_format_info *pfinfo;
	u32 w_align = 0, h_align = 0;
	u32 stride = 0, height = 0;
	u32 header_size = 0, payload_size = 0;
	u32 hshifter = 0, htail = 0, vshifter = 0, vtail = 0;
	u32 compbytesperline = 0, compheight = 0;
	u32 bytesperline = 0;
	s32 i = 0;
	u32 compsize[MAX_PLANENO] = {0};

	if (ctx->pmeta == NULL)
		return 0;
	if (fmt->fmt.pix_mp.num_planes > 1)
		return 0;
	pmeta_enc = (struct v4l2_vsi_stream_meta_info_enc *)ctx->pmeta;
	if (!(pmeta_enc->validmap & META_ENC_VALID_MASK_UFBC_CFG))
		return 0;
	ufbc_cfg = &pmeta_enc->ufbc_cfg;
	if (!ufbc_cfg->enable)
		return 0;
	pfinfo = vsi_v4l2_fmtinfo(fmt->fmt.pix_mp.pixelformat);
	bytesperline = fmt->fmt.pix_mp.width * raw_fmt_tbl[inputFormat][1];
	w_align = (ufbc_cfg->block_type == 0) ? 32 : 16;
	h_align = (ufbc_cfg->block_type == 0) ? 8 : 16;
	bytesperline = ALIGN(bytesperline, w_align * raw_fmt_tbl[inputFormat][1]);
	height = ALIGN(fmt->fmt.pix_mp.height, h_align);
	fmt->fmt.pix_mp.plane_fmt[0].bytesperline = bytesperline;
	vsi_getShifter(pfinfo->hdiv, &hshifter, &htail);
	vsi_getShifter(pfinfo->vdiv, &vshifter, &vtail);
	compsize[0] = bytesperline * height;
	compbytesperline = (bytesperline + htail) >> hshifter;
	compheight = (height + vtail) >> vshifter;

	payload_size += compsize[0];
	for (i = 1; i < pfinfo->comp_planes; i++) {
		if (pfinfo->bpp[0] != 0)
			compsize[i] = compbytesperline * compheight
						* pfinfo->bpp[i] / pfinfo->bpp[0];
		else	//to avoid panic, not really exist
			compsize[i] = compbytesperline * compheight;
		payload_size += compsize[i];
	}

	stride = bytesperline / raw_fmt_tbl[inputFormat][1];//in pixel
	header_size = ALIGN(stride * height / 256 * 16, 4096);
	if (fmt->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_P010)
		payload_size = stride * height / 256 * 512;

	fmt->fmt.pix_mp.plane_fmt[0].sizeimage = header_size + payload_size;
	v4l2_klog(LOGLVL_INFO, "%d:%d:%d:%d\n", bytesperline, height, header_size, payload_size);

	return 1;
}

static int vsiv4l2_setfmt_enc(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	s32 i;
	struct vsi_video_fmt *targetfmt;
	s32 userset_planeno;
	u32 *psize;
	s32 braw = brawfmt(ctx->flag, fmt->type);
	s32 rv;
	s32 pfl;
	const struct v4l2_format_info *fmtinfo;

	targetfmt = vsi_find_format(ctx, fmt);
	if (targetfmt == NULL)
		return -EINVAL;

	vsiv4l2_verifyfmt(ctx, fmt);
	v4l2_klog(LOGLVL_INFO, "%d:%x:%d:%d:%d",
		fmt->type, fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.width,
		fmt->fmt.pix_mp.height, fmt->fmt.pix_mp.plane_fmt[0].bytesperline);
	if (binputqueue(fmt->type))
		psize = pcfg->sizeimagesrc;
	else
		psize = pcfg->sizeimagedst;

	vsiv4l2_verifyfmt(ctx, fmt);
	if (binputqueue(fmt->type)) {
		pcfg->m_encparams.m_srcinfo.lumWidthSrc = fmt->fmt.pix_mp.width;
		pcfg->m_encparams.m_srcinfo.lumHeightSrc = fmt->fmt.pix_mp.height;
		pcfg->m_encparams.m_cropinfo.width = fmt->fmt.pix_mp.width;
		pcfg->m_encparams.m_cropinfo.height = fmt->fmt.pix_mp.height;
		pcfg->m_encparams.m_srcinfo.inputFormat = targetfmt->enc_fmt;
		pcfg->infmt_fourcc = fmt->fmt.pix_mp.pixelformat;
		flag_updateparam(m_cropinfo)
	} else {
		pcfg->m_encparams.m_codecfmt.codecFormat = targetfmt->enc_fmt;
		pcfg->outfmt_fourcc = fmt->fmt.pix_mp.pixelformat;
		if (!isimage(fmt->fmt.pix_mp.pixelformat)) {
			pfl = pcfg->m_encparams.m_profile.profile;
			pcfg->m_encparams.m_profile.profile = get_fmtprofile(pcfg);
			if (pcfg->m_encparams.m_profile.profile != pfl)
				flag_updateparam(m_profile)
		}
	}
	rv = vsiv4l2_decide_encpixeldepth(pcfg->infmt_fourcc, pcfg->outfmt_fourcc);
	if (rv == -1)
		return -EINVAL;
	userset_planeno = fmt->fmt.pix_mp.num_planes;
	//force it
	if (braw) {
		fmtinfo = vsi_v4l2_fmtinfo(fmt->fmt.pix_mp.pixelformat);
		if (!fmtinfo)
			return -EINVAL;
		fmt->fmt.pix_mp.num_planes = fmtinfo->mem_planes;
	} else
		fmt->fmt.pix_mp.num_planes = 1;
	pcfg->bytesperline = vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat,
			pcfg->m_encparams.m_codecfmt.codecFormat, fmt->fmt.pix_mp.plane_fmt[0].bytesperline, 0);
	if (pcfg->bytesperline == 0) {
		pcfg->bytesperline = fmt->fmt.pix_mp.width * rv / 8;
		pcfg->bytesperline = vsiv4l2_enc_getalign(pcfg->m_encparams.m_srcinfo.inputFormat,
				pcfg->m_encparams.m_codecfmt.codecFormat, pcfg->bytesperline, 0);
	}
	adjustChromaStride4MultiPFormats(fmt->fmt.pix_mp.pixelformat,
			pcfg->m_encparams.m_codecfmt.codecFormat, &pcfg->bytesperline);

	if (!vsiv4l2_enc_set3plane_bytesperline(fmt, pcfg)) {
		for (i = 0; i < fmt->fmt.pix_mp.num_planes; i++)
			fmt->fmt.pix_mp.plane_fmt[i].bytesperline = pcfg->bytesperline;
	}
	pcfg->m_encparams.m_srcinfo.lumWidthSrc = pcfg->bytesperline / raw_fmt_tbl[targetfmt->enc_fmt][1];
	if (!braw) {
		struct v4l2_frmsizeenum f;

		vsi_enum_encfsize(&f, fmt->fmt.pix_mp.pixelformat);
		psize[0] = fmt->fmt.pix_mp.plane_fmt[0].sizeimage;
		calcPlanesize(ctx, fmt->fmt.pix_mp.pixelformat, braw, f.stepwise.min_width,
				f.stepwise.min_height, psize, fmt->fmt.pix_mp.num_planes, 0);
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = psize[0];
	} else {
		if (fmt->fmt.pix_mp.num_planes == userset_planeno) {
			for (i = 0; i < fmt->fmt.pix_mp.num_planes; i++)
				psize[i] = fmt->fmt.pix_mp.plane_fmt[i].sizeimage;
			calcPlanesize(ctx, fmt->fmt.pix_mp.pixelformat, braw, pcfg->bytesperline, fmt->fmt.pix_mp.height,
						psize, userset_planeno, 0);
		} else
			calcPlanesize(ctx, fmt->fmt.pix_mp.pixelformat, braw, pcfg->bytesperline, fmt->fmt.pix_mp.height,
						psize, fmt->fmt.pix_mp.num_planes, 1);
		for (i = 0; i < fmt->fmt.pix_mp.num_planes; i++)
			fmt->fmt.pix_mp.plane_fmt[i].sizeimage = psize[i];
	}

	if (!binputqueue(fmt->type) &&
		userset_planeno > 1){
		if (pcfg->m_encparams.m_scaleoutput.scaleOutput != 0) {
			fmt->fmt.pix_mp.plane_fmt[1].sizeimage = psize[1] =
				vsiv4l2_getenc_scaleoutput(ctx, &fmt->fmt.pix_mp.plane_fmt[1].bytesperline);
			fmt->fmt.pix_mp.num_planes = 2;
		}
	}

	if (binputqueue(fmt->type) && ctx->pmeta != NULL &&
		vsiv4l2_calc_ufbc_size(ctx, fmt, targetfmt->enc_fmt)) {
		pcfg->bytesperline = fmt->fmt.pix_mp.plane_fmt[0].bytesperline;
		psize[0] = fmt->fmt.pix_mp.plane_fmt[0].sizeimage;
		pcfg->m_encparams.m_srcinfo.lumWidthSrc =
			pcfg->bytesperline / raw_fmt_tbl[targetfmt->enc_fmt][1];
	}

	if (binputqueue(fmt->type))
		pcfg->srcplanes = fmt->fmt.pix_mp.num_planes;
	else
		pcfg->dstplanes = fmt->fmt.pix_mp.num_planes;
	pcfg->field = fmt->fmt.pix_mp.field;
	pcfg->colorspace = fmt->fmt.pix_mp.colorspace;
	pcfg->flags = fmt->fmt.pix_mp.flags;
	pcfg->quantization = fmt->fmt.pix_mp.quantization;
	pcfg->ycbcr_enc = fmt->fmt.pix_mp.ycbcr_enc;
	pcfg->xfer_func = fmt->fmt.pix_mp.xfer_func;
	if (!isimage(fmt->fmt.pix_mp.pixelformat))
		enc_setvui(fmt, ctx);

	if (binputqueue(fmt->type)) {
		v4l2_klog(LOGLVL_INFO, "%d:%d:%d:%d",
			fmt->fmt.pix_mp.num_planes, fmt->fmt.pix_mp.plane_fmt[0].bytesperline,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage, fmt->fmt.pix_mp.plane_fmt[1].sizeimage);
	} else {
		v4l2_klog(LOGLVL_INFO, "%d:%d:%d",
			fmt->fmt.pix_mp.num_planes, fmt->fmt.pix_mp.plane_fmt[0].bytesperline, fmt->fmt.pix_mp.plane_fmt[0].sizeimage);
	}
	flag_updateparam(m_srcinfo)
	return 0;
}

static void vsiv4l2_convertpixel2MB(
	struct v4l2_rect *src,
	int fmt,
	s32 *left,
	s32 *top,
	s32 *right,
	s32 *bottom)
{
	int align = (fmt == V4L2_DAEMON_CODEC_ENC_HEVC ? 64:16);
	u32 r, b;

	*left = src->left / align;
	*top = src->top / align;
	r = src->left + src->width;
	b = src->top + src->height;
	*right = (r + align - 1) / align;
	*bottom = (b + align - 1) / align;
}

s32 vsi_convertROI(struct vsi_v4l2_ctx *ctx)
{
	struct v4l2_enc_roi_params *proi = &ctx->mediacfg.roiinfo;
	int i, fmt = ctx->mediacfg.m_encparams.m_codecfmt.codecFormat;
	s32 num;
	struct v4l2_enccfg_roiinfo *daemonroi = &ctx->mediacfg.m_encparams.m_roiinfo;

	if (isimage(ctx->mediacfg.outfmt_fourcc))
		return 0;
	if (fmt != V4L2_DAEMON_CODEC_ENC_HEVC &&
		fmt != V4L2_DAEMON_CODEC_ENC_H264)
		return 0;
	num = proi->num_roi_regions;
	for (i = 0; i < num; i++) {
		daemonroi->roiAreaEnable[i] = proi->roi_params[i].enable;
		vsiv4l2_convertpixel2MB(&proi->roi_params[i].rect, fmt, &daemonroi->roiAreaLeft[i],
			&daemonroi->roiAreaTop[i], &daemonroi->roiAreaRight[i], &daemonroi->roiAreaBottom[i]);
		if (proi->roi_params[i].qp_mode) {
			daemonroi->roiDeltaQp[i] = proi->roi_params[i].qp;
			daemonroi->roiQp[i] = -1;
		} else {
			daemonroi->roiQp[i] = proi->roi_params[i].qp;
			daemonroi->roiDeltaQp[i] = 0;
		}
	}
	/*disable left ones*/
	for (; i < VSI_V4L2_MAX_ROI_REGIONS; i++) {
		daemonroi->roiAreaEnable[i] = daemonroi->roiAreaTop[i] = daemonroi->roiAreaLeft[i] =
			daemonroi->roiAreaBottom[i] = daemonroi->roiAreaRight[i] = 0;
	}
	return num;
}

s32 vsi_convertIPCM(struct vsi_v4l2_ctx *ctx)
{
	struct v4l2_enc_ipcm_params *ipcm = &ctx->mediacfg.ipcminfo;
	int i, fmt = ctx->mediacfg.m_encparams.m_codecfmt.codecFormat;
	s32 num;
	struct v4l2_enccfg_ipcminfo *daemonipcm = &ctx->mediacfg.m_encparams.m_ipcminfo;

	if (isimage(ctx->mediacfg.outfmt_fourcc))
		return 0;
	if (fmt != V4L2_DAEMON_CODEC_ENC_HEVC &&
		fmt != V4L2_DAEMON_CODEC_ENC_H264)
		return 0;
	num = ipcm->num_ipcm_regions;
	daemonipcm->pcm_loop_filter_disabled_flag = (num > 0);
	for (i = 0; i < num; i++) {
		daemonipcm->ipcmAreaEnable[i] = ipcm->ipcm_params[i].enable;
		vsiv4l2_convertpixel2MB(&ipcm->ipcm_params[i].rect, fmt, &daemonipcm->ipcmAreaLeft[i],
			&daemonipcm->ipcmAreaTop[i], &daemonipcm->ipcmAreaRight[i], &daemonipcm->ipcmAreaBottom[i]);
	}
	/*disable left ones*/
	for (; i < VSI_V4L2_MAX_IPCM_REGIONS; i++) {
		daemonipcm->ipcmAreaEnable[i] = daemonipcm->ipcmAreaTop[i] = daemonipcm->ipcmAreaLeft[i] =
			daemonipcm->ipcmAreaBottom[i] = daemonipcm->ipcmAreaRight[i] = 0;
	}
	return num;
}

int vsiv4l2_setROI(struct vsi_v4l2_ctx *ctx, void *params)
{
	int i;
	struct v4l2_enc_roi_params *proi = (struct v4l2_enc_roi_params *)params;

	if (vsi_v4l2_hwconfig.max_ROIregion <= 0)
		return 0;
	ctx->mediacfg.roiinfo = *proi;
	ctx->mediacfg.roiinfo.num_roi_regions =
		min(ctx->mediacfg.roiinfo.num_roi_regions, (u32)vsi_v4l2_hwconfig.max_ROIregion);
	v4l2_klog(LOGLVL_DEBUG, "%d", proi->num_roi_regions);
	for (i = 0; i < proi->num_roi_regions; i++) {
		v4l2_klog(LOGLVL_DEBUG, "%d:%d:%d:%d:%d:%d:%d", proi->roi_params[i].enable,
			proi->roi_params[i].qp_mode, proi->roi_params[i].qp, proi->roi_params[i].rect.left,
			proi->roi_params[i].rect.top, proi->roi_params[i].rect.width, proi->roi_params[i].rect.height);
	}
	flag_updateparam(m_roiinfo)
	return 0;
}

static int vsiv4l2_checkScaleoutFmt(int scalefmt)
{
	if (scalefmt == 0)
		return 0;

	if (scalefmt == 1 &&
			(vsi_v4l2_hwconfig.map_scaledOutput & (1 << ENC_SCALEDOUT_NV12)))
		return 0;

	return -1;
}

int vsiv4l2_configScaleOutput(struct vsi_v4l2_ctx *ctx, void *params)
{
	struct v4l2_enc_scaleinfo *pscaleinfo = (struct v4l2_enc_scaleinfo *)params;
	struct v4l2_enccfg_scaleoutput *mscale = &ctx->mediacfg.m_encparams.m_scaleoutput;

	if (pscaleinfo->scaleOutput == 0) {
		if (mscale->scaleOutput == 1) {
			mscale->scaleOutput = 0;
			flag_updateparam(m_scaleoutput)
		}
		return 0;
	}
	if (vsiv4l2_checkScaleoutFmt(pscaleinfo->scaleOutputFormat) < 0)
		return -1;
	if (mscale->scaleOutput != pscaleinfo->scaleOutput ||
		mscale->scaleHeight != pscaleinfo->scaleHeight ||
		mscale->scaleWidth != pscaleinfo->scaleWidth ||
		mscale->scaleOutputFormat != pscaleinfo->scaleOutputFormat) {
		mscale->scaleOutput = 1;
		mscale->scaleHeight = pscaleinfo->scaleHeight;
		mscale->scaleWidth = pscaleinfo->scaleWidth;
		mscale->scaleOutputFormat = pscaleinfo->scaleOutputFormat;
		flag_updateparam(m_scaleoutput)
	}
	v4l2_klog(LOGLVL_INFO, "%d:%d:%d:%d",
		mscale->scaleOutput, mscale->scaleWidth, mscale->scaleHeight,  mscale->scaleOutputFormat);
	return 0;
}

int vsiv4l2_getROIcount(void)
{
	return vsi_v4l2_hwconfig.max_ROIregion;
}

int vsiv4l2_setIPCM(struct vsi_v4l2_ctx *ctx, void *params)
{
	int i;
	struct v4l2_enc_ipcm_params *ipcm = (struct v4l2_enc_ipcm_params *)params;

	if (vsi_v4l2_hwconfig.max_IPCMregion <= 0)
		return 0;
	ctx->mediacfg.ipcminfo = *ipcm;
	ctx->mediacfg.ipcminfo.num_ipcm_regions =
		min(ctx->mediacfg.ipcminfo.num_ipcm_regions, (u32)vsi_v4l2_hwconfig.max_IPCMregion);
	v4l2_klog(LOGLVL_DEBUG, "%d", ipcm->num_ipcm_regions);
	for (i = 0; i < ipcm->num_ipcm_regions; i++) {
		v4l2_klog(LOGLVL_DEBUG, "ipcm %d:%d:%d:%d:%d", ipcm->ipcm_params[i].enable,
			ipcm->ipcm_params[i].rect.left, ipcm->ipcm_params[i].rect.top,
			ipcm->ipcm_params[i].rect.width, ipcm->ipcm_params[i].rect.height);
	}
	flag_updateparam(m_ipcminfo)
	return 0;
}

int vsiv4l2_getIPCMcount(void)
{
	return vsi_v4l2_hwconfig.max_IPCMregion;
}

static s32 vsiv4l2_decide_decpixeldepth(u32 pixelformat, s32 origdepth)
{
	s32 pixdepth;

	switch (pixelformat) {
	case VSI_V4L2_DECOUT_P010:
	case VSI_V4L2_DECOUT_P010M:
	case VSI_V4L2_DEC_PIX_FMT_YUYV:
	case VSI_V4L2_DEC_PIX_FMT_UYVY:
	case VSI_V4L2_DEC_PIX_FMT_Y10L:
	case VSI_V4L2_DEC_PIX_FMT_Y10H:
		pixdepth = 16;
		break;
	case VSI_V4L2_DEC_PIX_FMT_NV12:
	case VSI_V4L2_DEC_PIX_FMT_NV12M:
	case VSI_V4L2_DEC_PIX_FMT_NV21:
	case VSI_V4L2_DEC_PIX_FMT_NV21M:
	case VSI_V4L2_DEC_PIX_FMT_400:
	case VSI_V4L2_DEC_PIX_FMT_411SP:
	case VSI_V4L2_DEC_PIX_FMT_444SP:
	case VSI_V4L2_DEC_PIX_FMT_YUV444M:
	case VSI_V4L2_DEC_PIX_FMT_YUV420:
	case VSI_V4L2_DEC_PIX_FMT_422SP:
	case VSI_V4L2_DEC_PIX_FMT_NV61:
	case VSI_V4L2_DEC_PIX_FMT_NV61M:
	case VSI_V4L2_DEC_PIX_FMT_YUV422M:
	case VSI_V4L2_DEC_PIX_FMT_YVU420:
	case VSI_V4L2_DEC_PIX_FMT_YVU420M:
	case VSI_V4L2_DEC_PIX_FMT_NV12_4L4:
		pixdepth = 8;
		break;
	case VSI_V4L2_DECOUT_NV12_10BIT:
	case VSI_V4L2_DEC_PIX_FMT_P010_4L4:
		pixdepth = 10;
		break;
	case VSI_V4L2_DEC_PIX_FMT_RGBA32:
	case VSI_V4L2_DEC_PIX_FMT_BGR32:
	case VSI_V4L2_DEC_PIX_FMT_ABGR32:
	case VSI_V4L2_DEC_PIX_FMT_RGBX32:
	case VSI_V4L2_DEC_PIX_FMT_RGB32:
	case VSI_V4L2_DEC_PIX_FMT_ARGB32:
	case VSI_V4L2_DEC_PIX_FMT_XRGB32:
	case VSI_V4L2_DEC_PIX_FMT_BGRA32:
	case VSI_V4L2_DEC_PIX_FMT_XBGR32:
	case VSI_V4L2_DEC_PIX_FMT_BGRX32:
	case VSI_V4L2_DEC_PIX_FMT_RGBA1010102:
	case VSI_V4L2_DEC_PIX_FMT_ARGB2101010:
	case VSI_V4L2_DEC_PIX_FMT_ABGR2101010:
	case VSI_V4L2_DEC_PIX_FMT_BGRA1010102:
		pixdepth = 32;
		break;
	case VSI_V4L2_DECOUT_DTRC_10BIT:
	case VSI_V4L2_DECOUT_RFC_10BIT:
	case VSI_V4L2_DECOUT_DTRC:
	case VSI_V4L2_DECOUT_RFC:
	default:
		pixdepth = origdepth;
		break;
	}

	return pixdepth;
}

static u32 vsiv4l2_get_ppu_width(struct PpUnitConfig *ppu_cfg,
								 struct v4l2_daemon_dec_resochange_params *dec_info,
								 struct v4l2_dec_heif_cfg *heif_cfg)
{
	u32 scale_width = 0;
	u32 crop_width = 0;
	u32 pp_width = 0;

	if (!ppu_cfg->crop.width || !ppu_cfg->crop.height)
		crop_width = dec_info->dec_info.frame_width;
	else
		crop_width = ppu_cfg->crop.width;

	if (ppu_cfg->scale.ratio_x && ppu_cfg->scale.ratio_y &&
		!ppu_cfg->scale.width && !ppu_cfg->scale.height) {
		scale_width = (crop_width / ppu_cfg->scale.ratio_x) & ~0x1;
	} else if (ppu_cfg->scale.width && ppu_cfg->scale.height) {
		scale_width = ppu_cfg->scale.width;
	} else {
		scale_width = crop_width;
	}
	pp_width = ppu_cfg->crop2.enabled ? ppu_cfg->crop2.width : scale_width;

	if (heif_cfg && heif_cfg->has_grid) {
		pp_width = heif_cfg->grid_columns * pp_width;
	} else if (dec_info->io_buffer.inputFormat == V4L2_DAEMON_CODEC_DEC_VP9 ||
		dec_info->io_buffer.inputFormat == V4L2_DAEMON_CODEC_DEC_AV1) {
		if (pp_width < dec_info->dec_info.frame_width)
			pp_width = dec_info->dec_info.frame_width;
	}

	return pp_width;
}

static u32 vsiv4l2_get_ppu_height(struct PpUnitConfig *ppu_cfg,
								  struct v4l2_daemon_dec_resochange_params *dec_info,
								  struct v4l2_dec_heif_cfg *heif_cfg)
{
	u32 pp_height = 0;
	u32 scale_height = 0;
	u32 crop_height = 0;

	if (!ppu_cfg->crop.width || !ppu_cfg->crop.height)
		crop_height = dec_info->dec_info.frame_height;
	else
		crop_height = ppu_cfg->crop.height;

	if (ppu_cfg->scale.ratio_x && ppu_cfg->scale.ratio_y &&
		!ppu_cfg->scale.width && !ppu_cfg->scale.height) {
		scale_height = (crop_height / ppu_cfg->scale.ratio_y) & ~0x1;
	} else if (ppu_cfg->scale.width && ppu_cfg->scale.height) {
		scale_height = ppu_cfg->scale.height;
	} else {
		scale_height = crop_height;
	}

	pp_height = ppu_cfg->crop2.enabled ?
				ppu_cfg->crop2.height : scale_height;

	if (heif_cfg && heif_cfg->has_grid) {
		pp_height = heif_cfg->grid_rows * pp_height;
	} else if (dec_info->io_buffer.inputFormat == V4L2_DAEMON_CODEC_DEC_VP9 ||
		dec_info->io_buffer.inputFormat == V4L2_DAEMON_CODEC_DEC_AV1) {
		if (pp_height < dec_info->dec_info.frame_height)
			pp_height = dec_info->dec_info.frame_height;
	}

	return pp_height;
}

static u32 vsiv4l2_calc_ppu_afbc_size(struct PpUnitConfig *ppu_cfg,
									  u32 pp_width, u32 pp_height,
									  u32 pix_fmt, u32 header)
{
	u32 width_in_block = 0, height_in_block = 0;
	u32 block_payload_size = 0;
	u32 header_size = 0, payload_size = 0;

	if (ppu_cfg->tile_mode == TILE16x16) {
		width_in_block = ALIGN(pp_width, 16) >> 4;
		height_in_block = ALIGN(pp_height, 16) >> 4;
	} else if (ppu_cfg->tile_mode == TILE32x8) {
		width_in_block = ALIGN(pp_width, 32) >> 5;
		height_in_block = ALIGN(pp_height, 8) >> 3;
	}

	if (pix_fmt == V4L2_PIX_FMT_P010)
		block_payload_size = 512;
	else
		block_payload_size = 384;

	header_size = ALIGN(width_in_block * 16 * height_in_block, 4096);
	payload_size = ALIGN(block_payload_size * width_in_block * height_in_block, 128);
	if (header)
		return header_size;
	else
		return payload_size;
}


static s32 vsiv4l2_calc_ppbufsize(struct vsi_v4l2_ctx *ctx, int *ppsize, u32 pix_fmt,
									struct vsi_v4l2_mediacfg *pcfg)
{
	struct v4l2_vsi_stream_meta_info_dec *pmetadec;
	struct v4l2_pp_cfg *ppcfg;
	int i, j, chn_id;
	struct PpUnitConfig *ppu_cfg;
	struct v4l2_pp_buf_info *pp_buf;
	u32 header_size = 0, payload_size = 0;
	u32 pp_comp = 0;
	u32 width = 0, height = 0;
	u32 bytesperline = 0;
	u32 pp_sz[VB2_MAX_PLANES] = {0};
	struct v4l2_dec_heif_cfg *heif_cfg = NULL;
	const struct v4l2_format_info *fmtinfo = NULL;
	struct vsi_video_fmt *targetfmt = NULL;
	u32 pp_pix_fmt = 0;
	s32 pixel_depth = 8;
	struct v4l2_daemon_dec_resochange_params *dec_info = NULL;
	struct v4l2_format fmt;

	if (ctx->pmeta == NULL)
		return 0;

	//stream_meta_info_dec is at first place
	pmetadec = (struct v4l2_vsi_stream_meta_info_dec *)ctx->pmeta;
	if (!(pmetadec->validmap & META_DEC_VALID_MASK_PPCFG))
		return 0;

	if (pmetadec->validmap & META_DEC_VALID_MASK_HEIF_CFG)
		heif_cfg = &pmetadec->heif_cfg;
	dec_info = &pcfg->decparams.dec_info;
	ppcfg = &pmetadec->mutippcfg;
	pp_buf = &pmetadec->pp_buf_info;
	memset(pp_buf, 0, sizeof(struct v4l2_pp_buf_info));
	for (i = 0; i < ppcfg->cfg_num; i++) {
		chn_id = ppcfg->pp_cfg[i].chn_id;
		ppu_cfg = &ppcfg->pp_cfg[i].cfg;

		if (chn_id == 0 && ppu_cfg->compress_mode == 2 &&
			(ppu_cfg->tile_mode == TILE16x16 ||
			 ppu_cfg->tile_mode == TILE32x8)) {
			pp_comp = 1;
		} else {
			pp_comp = 0;
		}
		if (i == 0)
			pp_pix_fmt = pix_fmt;
		else
			pp_pix_fmt = ppu_cfg->pix_fmt;

		fmtinfo = vsi_v4l2_fmtinfo(pp_pix_fmt);
		if (!fmtinfo) {
			v4l2_klog(LOGLVL_ERROR, "wrong format %x in pp%d", pp_pix_fmt, i);
			return -1;
		}
		fmt.fmt.pix_mp.pixelformat = pp_pix_fmt;
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		targetfmt = vsi_find_format(ctx, &fmt);
		if (targetfmt == NULL) {
			v4l2_klog(LOGLVL_ERROR, "vsi_find_format(%x) failed in pp%d", pp_pix_fmt, i);
			return -1;
		}
		pixel_depth = vsiv4l2_decide_decpixeldepth(targetfmt->dec_fmt, pcfg->src_pixeldepth);
		width = vsiv4l2_get_ppu_width(ppu_cfg, dec_info, heif_cfg);
		height = vsiv4l2_get_ppu_height(ppu_cfg, dec_info, heif_cfg);
		if (pp_comp) {
			header_size = vsiv4l2_calc_ppu_afbc_size(ppu_cfg, width, height, pix_fmt, 1);
			payload_size = vsiv4l2_calc_ppu_afbc_size(ppu_cfg, width, height, pix_fmt, 0);
		} else {
			bytesperline = ppu_cfg->ystride;
			if (!bytesperline)
				bytesperline = ALIGN(width * pixel_depth / 8, 2);
			calcPlanesize(ctx, pp_pix_fmt, 1, bytesperline, height, pp_sz, fmtinfo->mem_planes, 0);
		}

		if (i == 0) {
			/* first pp channel work with v4l2 standard CAPTURE buffer. */
			if (pp_comp) {
				ppsize[0] = header_size + payload_size;
				ppsize[1] = 0;
				ppsize[2] = 0;
			} else {
				for (j = 0; j < fmtinfo->mem_planes; j++)
					ppsize[j] = pp_sz[j];

				if (fmtinfo->mem_planes == 1 &&
					fmtinfo->comp_planes > 1 &&
					ppu_cfg->chroma_offset) {
					ppsize[0] += ppu_cfg->chroma_offset - bytesperline * height;
				}
				pcfg->bytesperline = bytesperline;
			}
		} else {
			pp_buf->info[pp_buf->num].ppu_chn = chn_id;
			if (pp_comp) {
				pp_buf->info[pp_buf->num].size[0] = header_size + payload_size;
				pp_buf->info[pp_buf->num].size[1] = 0;
				pp_buf->info[pp_buf->num].size[2] = 0;
				pp_buf->info[pp_buf->num].num_planes = 1;
			} else {
				for (j = 0; j < fmtinfo->mem_planes; j++)
					pp_buf->info[pp_buf->num].size[j] = pp_sz[j];

				pp_buf->info[pp_buf->num].num_planes = fmtinfo->mem_planes;
				if (fmtinfo->mem_planes == 1 &&
					fmtinfo->comp_planes > 1 &&
					ppu_cfg->chroma_offset) {
					ppsize[0] += ppu_cfg->chroma_offset - bytesperline * height;
				}
			}
			pp_buf->num++;
			pmetadec->validmap |= META_DEC_VALID_MASK_PP_BUF_INFO;
		}

		v4l2_klog(LOGLVL_DEBUG, "%d:%d:%d:%d:%d", header_size,
					payload_size, pp_sz[0], pp_sz[1], pp_sz[2]);
	}

	return 1;
}

static int vsiv4l2_setfmt_dec(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	struct vsi_video_fmt *targetfmt;
	u32 *psize;
	s32 braw = brawfmt(ctx->flag, fmt->type);
	s32 planeno, i;
	u32 deffmtcc = fmt->fmt.pix_mp.pixelformat;
	const struct v4l2_format_info *fmtinfo = NULL;
	u32 shifter = 0, tail = 0;
	s32 ret = 0;
	u32 ppsize[VB2_MAX_PLANES] = {0};
	struct v4l2_daemon_dec_info *decinfo = &pcfg->decparams.dec_info.dec_info;
	struct v4l2_pix_format_mplane *pix_mp;

	if (braw) {
		fmtinfo = vsi_v4l2_fmtinfo(fmt->fmt.pix_mp.pixelformat);
		if (!fmtinfo)
			return -EINVAL;
		vsi_getShifter(fmtinfo->hdiv, &shifter, &tail);
	}
	targetfmt = vsi_find_format(ctx, fmt);
	if (targetfmt == NULL)
		return -EINVAL;

	pix_mp = &(fmt->fmt.pix_mp);
	v4l2_klog(LOGLVL_DEBUG, "ctx_id=%lx pixel_info=%d:%x:%d:%d:%d plane_info=%u[%u-%u-%u]",
			ctx->ctxid,
			fmt->type,
			pix_mp->pixelformat,
			pix_mp->width,
			pix_mp->height,
			pix_mp->plane_fmt[0].bytesperline,
			pix_mp->num_planes,
			pix_mp->plane_fmt[0].sizeimage,
			pix_mp->plane_fmt[1].sizeimage,
			pix_mp->plane_fmt[2].sizeimage);

	if (!binputqueue(fmt->type)) {
		pcfg->decparams.dec_info.io_buffer.outputPixelDepth =
			vsiv4l2_decide_decpixeldepth(targetfmt->dec_fmt, pcfg->src_pixeldepth);
		if (pcfg->decparams.dec_info.io_buffer.outputPixelDepth == -1)
			return -EINVAL;	//pixdepth from set CAPTURE fmt can't be satisfied
		if (!test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag)) {
			struct v4l2_frmsizeenum fmsize;

			pcfg->decparams.dec_info.io_buffer.output_width = fmt->fmt.pix_mp.width;
			pcfg->decparams.dec_info.io_buffer.output_height = fmt->fmt.pix_mp.height;
			pcfg->decparams.dec_info.io_buffer.outBufFormat = targetfmt->dec_fmt;
			pcfg->bytesperline = fmt->fmt.pix_mp.plane_fmt[0].bytesperline;

			vsi_enum_decfsize(&fmsize, pcfg->infmt_fourcc);
			pcfg->decparams.dec_info.dec_info.visible_rect.left = 0;
			pcfg->decparams.dec_info.dec_info.visible_rect.top = 0;
			pcfg->decparams.dec_info.dec_info.visible_rect.width =
				clamp(fmt->fmt.pix_mp.width, fmsize.stepwise.min_width, fmsize.stepwise.max_width);
			pcfg->decparams.dec_info.dec_info.visible_rect.height =
				clamp(fmt->fmt.pix_mp.height, fmsize.stepwise.min_height, fmsize.stepwise.max_height);

		} else {
			//dtrc is only for HEVC and VP9
			if (istiledfmt(targetfmt->dec_fmt)
				&& pcfg->decparams.dec_info.io_buffer.inputFormat != V4L2_DAEMON_CODEC_DEC_VP9
				&& pcfg->decparams.dec_info.io_buffer.inputFormat != V4L2_DAEMON_CODEC_DEC_HEVC)
				return -EINVAL;
			fmt->fmt.pix_mp.width = pcfg->decparams.dec_info.io_buffer.output_width;
			fmt->fmt.pix_mp.height = pcfg->decparams.dec_info.io_buffer.output_height;
			pcfg->decparams.dec_info.io_buffer.outBufFormat = targetfmt->dec_fmt;
		}
		pcfg->outfmt_fourcc = fmt->fmt.pix_mp.pixelformat;
		psize = pcfg->sizeimagedst;
		if (!fmtinfo)
			return -EINVAL;
		planeno = fmtinfo->mem_planes;
		if (test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag))
			deffmtcc = vsi_find_local_dec_format(decinfo->src_pix_fmt, 1);

		v4l2_klog(LOGLVL_DEBUG, "ctx_id=%lx Pixeldepth %d src %d bytes %d\n",
			ctx->ctxid,
			pcfg->decparams.dec_info.io_buffer.outputPixelDepth,
			pcfg->src_pixeldepth,
			pcfg->bytesperline);

		if (test_bit(CTX_FLAG_SRCCHANGED_BIT, &ctx->flag) && deffmtcc == fmt->fmt.pix_mp.pixelformat) {
			for (i = 0; i < planeno; i++) {
				fmt->fmt.pix_mp.plane_fmt[i].sizeimage = max(psize[i], fmt->fmt.pix_mp.plane_fmt[i].sizeimage);
				if (i == 0)
					fmt->fmt.pix_mp.plane_fmt[0].bytesperline = pcfg->bytesperline;
				else
					fmt->fmt.pix_mp.plane_fmt[i].bytesperline = ALIGN(((pcfg->bytesperline + tail) >> shifter), 2);
			}
		} else {
			if (pcfg->decparams.dec_info.io_buffer.outputPixelDepth != pcfg->src_pixeldepth) {
				pcfg->bytesperline = fmt->fmt.pix_mp.width * pcfg->decparams.dec_info.io_buffer.outputPixelDepth / 8;
				pcfg->bytesperline = ALIGN(pcfg->bytesperline, 2);
			}
			for (i = 0; i < planeno; i++)
				psize[i] = fmt->fmt.pix_mp.plane_fmt[i].sizeimage;

			calcPlanesize(ctx, fmt->fmt.pix_mp.pixelformat, braw, pcfg->bytesperline, fmt->fmt.pix_mp.height, psize, planeno, 0);
			for (i = 0; i < planeno; i++) {
				fmt->fmt.pix_mp.plane_fmt[i].sizeimage = psize[i];
				if (i == 0)
					fmt->fmt.pix_mp.plane_fmt[0].bytesperline = pcfg->bytesperline;
				else
					fmt->fmt.pix_mp.plane_fmt[i].bytesperline = ALIGN(((pcfg->bytesperline + tail) >> shifter), 2);
			}
		}
		fmt->fmt.pix_mp.num_planes = planeno;

		ret = vsiv4l2_calc_ppbufsize(ctx, ppsize, fmt->fmt.pix_mp.pixelformat, pcfg);
		if (ret == -1)
			return -EINVAL;
		if (ret == 1) {
			v4l2_klog(LOGLVL_DEBUG, "ctx_id=%lx pp size:[%d][%d][%d]", ctx->ctxid, ppsize[0], ppsize[1], ppsize[2]);
			for (i = 0; i < planeno; i++) {
				if (ppsize[i] > 0) {
					psize[i] = ppsize[i];
					fmt->fmt.pix_mp.plane_fmt[i].sizeimage = ppsize[i];
				}
				if (i == 0)
					fmt->fmt.pix_mp.plane_fmt[0].bytesperline = pcfg->bytesperline;
				else
					fmt->fmt.pix_mp.plane_fmt[i].bytesperline = ALIGN(((pcfg->bytesperline + tail) >> shifter), 2);
			}
		}
		pcfg->dstplanes = fmt->fmt.pix_mp.num_planes;
		v4l2_klog(LOGLVL_INFO, "ctx_id=%lx [output]  planes_nums=%u plane_size=%u-%u-%u\n",
			ctx->ctxid, pcfg->dstplanes, pcfg->sizeimagedst[0], pcfg->sizeimagedst[1], pcfg->sizeimagedst[2]);
	} else {
		pcfg->decparams.dec_info.io_buffer.srcwidth = fmt->fmt.pix_mp.width;
		pcfg->decparams.dec_info.io_buffer.srcheight = fmt->fmt.pix_mp.height;
		pcfg->decparams.dec_info.io_buffer.inputFormat = targetfmt->dec_fmt;
		pcfg->infmt_fourcc = fmt->fmt.pix_mp.pixelformat;
		pcfg->sizeimagesrc[0] = fmt->fmt.pix_mp.plane_fmt[0].sizeimage;
		pcfg->srcplanes = 1;
		v4l2_klog(LOGLVL_DEBUG, "ctx_id=%lx [input] planes_nums=%u plane_size=%u\n",
			ctx->ctxid, pcfg->srcplanes, pcfg->sizeimagesrc[0]);
	}
	pcfg->field = fmt->fmt.pix_mp.field;
	pcfg->colorspace = fmt->fmt.pix_mp.colorspace;
	pcfg->flags = fmt->fmt.pix_mp.flags;
	pcfg->quantization = fmt->fmt.pix_mp.quantization;
	pcfg->xfer_func = fmt->fmt.pix_mp.xfer_func;
	pcfg->ycbcr_enc = fmt->fmt.pix_mp.ycbcr_enc;
	v4l2_klog(LOGLVL_DEBUG, "ctx_id=%lx %d:%d",
		ctx->ctxid,
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline,
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage);

	return 0;
}


int vsiv4l2_setfmt(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	if (isencoder(ctx))
		return vsiv4l2_setfmt_enc(ctx, fmt);
	else
		return vsiv4l2_setfmt_dec(ctx, fmt);
}

static int vsiv4l2_verify_grid_enc(struct vsi_v4l2_ctx *ctx,
									struct v4l2_format *fmt,
									struct v4l2_frmsizeenum *fmsize)
{
	struct v4l2_vsi_stream_meta_info_enc *pmeta_enc;
	struct v4l2_enc_heif_cfg *heif_cfg;

	if (!binputqueue(fmt->type) || !ctx->pmeta)
		return 0;
	if (fmsize->pixel_format != V4L2_PIX_FMT_HEVC &&
		fmsize->pixel_format != V4L2_PIX_FMT_AV1)
		return 0;
	pmeta_enc = (struct v4l2_vsi_stream_meta_info_enc *)ctx->pmeta;
	if (!(pmeta_enc->validmap & META_ENC_VALID_MASK_HEIF_CFG))
		return 0;

	heif_cfg = &pmeta_enc->heif_cfg;
	if (!heif_cfg->input_mode)
		return 0;// not whole-frame mode.

	heif_cfg->grid_width =
		clamp(heif_cfg->grid_width, fmsize->stepwise.min_width, fmsize->stepwise.max_width);
	heif_cfg->grid_height =
		clamp(heif_cfg->grid_height, fmsize->stepwise.min_height, fmsize->stepwise.max_height);
	heif_cfg->grid_width =
		ALIGN(heif_cfg->grid_width, fmsize->stepwise.step_width);
	heif_cfg->grid_height =
		ALIGN(heif_cfg->grid_height, fmsize->stepwise.step_height);

	v4l2_klog(LOGLVL_INFO, "%x:%d",
		heif_cfg->grid_width, heif_cfg->grid_height);

	return 1;
}

static int vsiv4l2_verifyfmt(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	struct v4l2_frmsizeenum fmsize;

	/* verify and change format member to valid range */
	if (isencoder(ctx)) {
		if (!binputqueue(fmt->type))
			fmsize.pixel_format = fmt->fmt.pix_mp.pixelformat;
		else
			fmsize.pixel_format = pcfg->outfmt_fourcc;
		vsi_enum_encfsize(&fmsize, fmsize.pixel_format);

		fmt->fmt.pix_mp.width =
			max(fmt->fmt.pix_mp.width, fmsize.stepwise.min_width);
		fmt->fmt.pix_mp.height =
			max(fmt->fmt.pix_mp.height, fmsize.stepwise.min_height);
		if (!vsiv4l2_verify_grid_enc(ctx, fmt, &fmsize)) {
			fmt->fmt.pix_mp.width =
				min(fmt->fmt.pix_mp.width, fmsize.stepwise.max_width);
			fmt->fmt.pix_mp.height =
				min(fmt->fmt.pix_mp.height, fmsize.stepwise.max_height);
		}
	} else {
		if (binputqueue(fmt->type))
			fmsize.pixel_format = fmt->fmt.pix.pixelformat;
		else
			fmsize.pixel_format = pcfg->infmt_fourcc;
		vsi_enum_decfsize(&fmsize, fmsize.pixel_format);
		fmt->fmt.pix_mp.width =
			clamp(fmt->fmt.pix_mp.width, fmsize.stepwise.min_width, fmsize.stepwise.max_width);
		fmt->fmt.pix_mp.height =
			clamp(fmt->fmt.pix_mp.height, fmsize.stepwise.min_height, fmsize.stepwise.max_height);
	}

	fmt->fmt.pix_mp.width = ALIGN(fmt->fmt.pix_mp.width, fmsize.stepwise.step_width);
	fmt->fmt.pix_mp.height = ALIGN(fmt->fmt.pix_mp.height, fmsize.stepwise.step_height);
	v4l2_klog(LOGLVL_DEBUG, "%x:%d:%d",
		fmsize.pixel_format, fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height);

	return 0;
}

int vsiv4l2_verifycrop(struct vsi_v4l2_ctx *ctx, struct v4l2_selection *s)
{
	int update = 0, align, mask;
	struct v4l2_rect rect = s->r;
	int right = rect.left + rect.width;
	int bottom = rect.top + rect.height;

	switch (ctx->mediacfg.outfmt_fourcc) {
	case V4L2_PIX_FMT_HEVC:
		align = vsi_v4l2_hwconfig.hevcinfo.common.crop_align;
		break;
	case V4L2_PIX_FMT_H264:
		align = vsi_v4l2_hwconfig.h264info.common.crop_align;
		break;
	case V4L2_PIX_FMT_VP8:
		align = vsi_v4l2_hwconfig.vp8info.common.crop_align;
		break;
	case V4L2_PIX_FMT_VP9:
		align = vsi_v4l2_hwconfig.vp9info.common.crop_align;
		break;
	case V4L2_PIX_FMT_AV1:
		align = vsi_v4l2_hwconfig.av1info.common.crop_align;
		break;
	case V4L2_PIX_FMT_JPEG:
		align = vsi_v4l2_hwconfig.jpeginfo.common.crop_align;
		break;
	case V4L2_PIX_FMT_WEBP:
		align = vsi_v4l2_hwconfig.webpinfo.common.crop_align;
		break;
	default:
		align = 1;
	}
	mask = align - 1;
	if ((rect.left & mask) || (right & mask) || (rect.top & 0x1) || (bottom & 0x1))
		update = 1;
	if (!update)
		return 0;
	if (s->flags == (V4L2_SEL_FLAG_GE | V4L2_SEL_FLAG_LE))
		return -ERANGE;
	if (s->flags & V4L2_SEL_FLAG_LE) {
		rect.left = ALIGN(rect.left, align);
		if (right & mask)
			right = ALIGN(right, align) - align;
		rect.top = ALIGN(rect.top, 2);
		if (bottom & 0x1)
			bottom = ALIGN(bottom, 2) - 2;
		rect.width = right - rect.left;
		rect.height = bottom - rect.top;
		s->r = rect;
		return 0;
	}
	/*enlarge is previleged*/
	if (rect.left & mask)
		rect.left = ALIGN(rect.left, align) - align;
	right = ALIGN(right, align);
	if (rect.top & 2)
		rect.top = ALIGN(rect.top, 2) - 2;
	bottom = ALIGN(bottom, 2);
	rect.width = right - rect.left;
	rect.height = bottom - rect.top;
	s->r = rect;
	v4l2_klog(LOGLVL_INFO, "%d:%d:%d:%d",
		rect.top, rect.left, rect.width, rect.height);
	return 0;
}

static u32 vsi_find_local_dec_format(s32 fmt, int braw)
{
	int i;

	if (braw) {
		for (i = 0; i < ARRAY_SIZE(vsi_raw_fmt); i++) {
			if (vsi_raw_fmt[i].dec_fmt == fmt)
				return vsi_raw_fmt[i].fourcc;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(vsi_coded_fmt); i++) {
			if (vsi_coded_fmt[i].dec_fmt == fmt)
				return vsi_coded_fmt[i].fourcc;
		}
	}
	return -1;
}

static int vsiv4l2_getfmt_enc(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	int i;
	int *psize = (binputqueue(fmt->type) ? pcfg->sizeimagesrc : pcfg->sizeimagedst);

	if (binputqueue(fmt->type)) {
		fmt->fmt.pix_mp.width = pcfg->m_encparams.m_srcinfo.lumWidthSrc;
		fmt->fmt.pix_mp.height = pcfg->m_encparams.m_srcinfo.lumHeightSrc;
		fmt->fmt.pix_mp.pixelformat = pcfg->infmt_fourcc;
	} else {
		fmt->fmt.pix_mp.width = pcfg->m_encparams.m_cropinfo.width;
		fmt->fmt.pix_mp.height = pcfg->m_encparams.m_cropinfo.height;
		fmt->fmt.pix_mp.pixelformat = pcfg->outfmt_fourcc;
	}
	fmt->fmt.pix_mp.field = pcfg->field;
	if (binputqueue(fmt->type))
		fmt->fmt.pix_mp.num_planes = pcfg->srcplanes;
	else
		fmt->fmt.pix_mp.num_planes = pcfg->dstplanes;
	if (fmt->fmt.pix_mp.num_planes == 0)
		fmt->fmt.pix_mp.num_planes = 1;
	for (i = 0; i < fmt->fmt.pix_mp.num_planes; i++)
		fmt->fmt.pix_mp.plane_fmt[i].sizeimage = psize[i];
	if (vsiv4l2_enc_set3plane_bytesperline(fmt, pcfg))
		i = 3;
	else {
		for (i = 0; i < fmt->fmt.pix_mp.num_planes; i++)
			fmt->fmt.pix_mp.plane_fmt[i].bytesperline = pcfg->bytesperline;
		if (!binputqueue(fmt->type) && fmt->fmt.pix_mp.num_planes == 2)
			vsiv4l2_getenc_scaleoutput(ctx, &fmt->fmt.pix_mp.plane_fmt[1].bytesperline);
	}

	for (; i < VIDEO_MAX_PLANES; i++) {
		fmt->fmt.pix_mp.plane_fmt[i].bytesperline = 0;
		fmt->fmt.pix_mp.plane_fmt[i].sizeimage = 0;
	}
	v4l2_klog(LOGLVL_DEBUG, "%x:%d:%d:%d:%d",
		fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.num_planes,
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage, fmt->fmt.pix_mp.plane_fmt[1].sizeimage,
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline);
	fmt->fmt.pix_mp.colorspace = pcfg->colorspace;
	fmt->fmt.pix_mp.flags = pcfg->flags;
	fmt->fmt.pix_mp.quantization = pcfg->quantization;
	fmt->fmt.pix_mp.xfer_func = pcfg->xfer_func;
	fmt->fmt.pix_mp.ycbcr_enc = pcfg->ycbcr_enc;
	return 0;
}

static int vsiv4l2_getfmt_dec(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	struct vsi_v4l2_mediacfg *pcfg = &ctx->mediacfg;
	s32 braw = brawfmt(ctx->flag, fmt->type);
	s32 i;
	u32 shifter = 0, tail = 0;
	const struct v4l2_format_info *fmtinfo;

	if (binputqueue(fmt->type)) {
		fmt->fmt.pix_mp.width = pcfg->decparams.dec_info.io_buffer.srcwidth;
		fmt->fmt.pix_mp.height = pcfg->decparams.dec_info.io_buffer.srcheight;
		fmt->fmt.pix_mp.pixelformat = vsi_find_local_dec_format(pcfg->decparams.dec_info.io_buffer.inputFormat, braw);
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = pcfg->sizeimagesrc[0];
		fmt->fmt.pix_mp.num_planes = pcfg->srcplanes;
	} else {
		fmt->fmt.pix_mp.pixelformat = vsi_find_local_dec_format(pcfg->decparams.dec_info.io_buffer.outBufFormat, braw);
		fmtinfo = vsi_v4l2_fmtinfo(fmt->fmt.pix_mp.pixelformat);
		if (fmtinfo)
			vsi_getShifter(fmtinfo->hdiv, &shifter, &tail);
		fmt->fmt.pix_mp.num_planes = pcfg->dstplanes;
		fmt->fmt.pix_mp.width = pcfg->decparams.dec_info.io_buffer.output_width;
		fmt->fmt.pix_mp.height = pcfg->decparams.dec_info.io_buffer.output_height;
		for (i = 0; i < pcfg->dstplanes; i++) {
			if (i == 0)
				fmt->fmt.pix_mp.plane_fmt[i].bytesperline = pcfg->bytesperline;
			else
				fmt->fmt.pix_mp.plane_fmt[i].bytesperline = ALIGN(((pcfg->bytesperline + tail) >> shifter), 2);
			fmt->fmt.pix_mp.plane_fmt[i].sizeimage = pcfg->sizeimagedst[i];
		}
	}
	vsiv4l2_verifyfmt(ctx, fmt);
	fmt->fmt.pix_mp.field = pcfg->field;
	fmt->fmt.pix_mp.colorspace = pcfg->colorspace;
	fmt->fmt.pix_mp.flags = pcfg->flags;
	fmt->fmt.pix_mp.quantization = pcfg->quantization;
	fmt->fmt.pix_mp.xfer_func = pcfg->xfer_func;
	fmt->fmt.pix_mp.ycbcr_enc = pcfg->ycbcr_enc;
	vsi_dec_getvui(fmt, &pcfg->decparams.dec_info.dec_info);
	v4l2_klog(LOGLVL_DEBUG, "%x:%d:%d",
		fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.plane_fmt[0].sizeimage,  fmt->fmt.pix_mp.plane_fmt[0].bytesperline);
	return 0;
}



int vsiv4l2_getfmt(struct vsi_v4l2_ctx *ctx, struct v4l2_format *fmt)
{
	if (isencoder(ctx))
		return vsiv4l2_getfmt_enc(ctx, fmt);
	else
		return vsiv4l2_getfmt_dec(ctx, fmt);
}

void vsi_v4l2_update_decfmt(struct vsi_v4l2_ctx *ctx)
{
	struct v4l2_format fmt;

	memset(&fmt, 0, sizeof(fmt));

	if (ctx->mediacfg.decparams.dec_info.dec_info.bit_depth == 10 &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DECOUT_NV12_10BIT &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DECOUT_P010 &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DECOUT_P010M &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DECOUT_DTRC_10BIT &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DECOUT_RFC_10BIT &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DEC_PIX_FMT_Y10L &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DEC_PIX_FMT_Y10H &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DEC_PIX_FMT_RGBA1010102 &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DEC_PIX_FMT_ARGB2101010 &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DEC_PIX_FMT_ABGR2101010 &&
		ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt != VSI_V4L2_DEC_PIX_FMT_BGRA1010102) {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		vsiv4l2_getfmt(ctx, &fmt);
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12X;
		vsiv4l2_setfmt(ctx, &fmt);
	} else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		vsiv4l2_getfmt(ctx, &fmt);
		fmt.fmt.pix_mp.pixelformat = vsi_find_local_dec_format(ctx->mediacfg.decparams.dec_info.dec_info.src_pix_fmt, 1);
		vsiv4l2_setfmt(ctx, &fmt);
	}
}

int vsiv4l2_buffer_config(
	struct vsi_v4l2_ctx *ctx,
	int type,
	unsigned int *nbuffers,
	unsigned int *nplanes,
	unsigned int sizes[]
)
{
	s32 i;
	bool plane_size_overflow = false;
	s32 *psize = (binputqueue(type) ? ctx->mediacfg.sizeimagesrc : ctx->mediacfg.sizeimagedst);
	s32 planeno = binputqueue(type) ? ctx->mediacfg.srcplanes : ctx->mediacfg.dstplanes;

	for (i = 0; i < VB2_MAX_PLANES; i++)
		sizes[i] = 0;
	if (ismetabuf(type)) {
		*nbuffers = 1;
		*nplanes = 1;
		if (isencoder(ctx))
			sizes[0] = ctx->input_que.num_buffers * sizeof(struct v4l2_vsi_enc_meta_info_OUTPUT) +
									ctx->output_que.num_buffers * sizeof(struct v4l2_vsi_enc_meta_info_CAPTURE) +
									sizeof(struct v4l2_vsi_stream_meta_info_enc);
		else
			sizes[0] = ctx->output_que.num_buffers * sizeof(struct v4l2_vsi_dec_meta_info_CAPTURE) +
									ctx->input_que.num_buffers * sizeof(struct v4l2_vsi_dec_meta_info_OUTPUT) +
									sizeof(struct v4l2_vsi_stream_meta_info_dec);
		return 0;
	}

	if (isdecoder(ctx) && !binputqueue(type)) {
		if (*nbuffers < ctx->mediacfg.minbuf_4capture)
			*nbuffers = ctx->mediacfg.minbuf_4capture;
	}
	if (isencoder(ctx)) {
		/*the upper limit is done in videobuf2-core*/
		if (!isimage(ctx->mediacfg.outfmt_fourcc)) {
			if (*nbuffers < ctx->mediacfg.m_encparams.m_gopsize.gopSize)
				*nbuffers = ctx->mediacfg.m_encparams.m_gopsize.gopSize;
		}
	}
	*nplanes = planeno;
	for (i = 0; i < planeno; i++) {
		sizes[i] = psize[i];
		/*
		 * In fuzz scenarios, the size of the plane may be very large,
		 * causing memory leaks, so limit its size.
		 */
		if (psize[i] > MAX_PLANE_SIZE) {
			plane_size_overflow = true;
			v4l2_klog(LOGLVL_ERROR, "ctx(%lu) nbuffers(%d) plane[%d] size %d is too large.\n",
				ctx->ctxid, *nbuffers, i, psize[i]);
			sizes[i] = MAX_PLANE_SIZE;
		}
	}
	/* if plane size overflow, so limit the number of buffers to avoid memory leak. */
	if (plane_size_overflow)
		*nbuffers = 4;

	//will this happen?
	if (isdecoder(ctx) && binputqueue(type) &&
		sizes[0] <= 0) {
		sizes[0] = PAGE_SIZE;
	}
	v4l2_klog(LOGLVL_INFO, "%d:%lx:%d:%d:%d:%d:%d:%d", ctx->dev->deviceid, ctx->ctxid, type,
		*nbuffers, *nplanes, sizes[0], sizes[1], sizes[2]);
	return 0;
}

int vsiv4l2_has_jpgcodingmode(s32 codingmode)
{
	if ((1 << codingmode) & vsi_v4l2_hwconfig.jpeginfo.map_jpgcodingmode)
		return 1;
	else
		return 0;
}

int vsiv4l2_has_loopfilter(struct vsi_v4l2_ctx *ctx)
{
	int ret = 0, val = -1;

	if (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_H264) {
		ret = vsi_v4l2_hwconfig.h264info.map_loopfilter & (1 << ctx->mediacfg.disableDeblockingFilter_h264);
		val = ctx->mediacfg.disableDeblockingFilter_h264;
	} else if (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_HEVC) {
		ret = vsi_v4l2_hwconfig.hevcinfo.map_loopfilter & (1 << ctx->mediacfg.disableDeblockingFilter_hevc);
		val = ctx->mediacfg.disableDeblockingFilter_hevc;
	}
	if (ret == 0)
		val = -1;
	return val;
}

int vsiv4l2_has_meMode(struct vsi_v4l2_ctx *ctx)
{
	u32 ret = 0, mode;

	switch (ctx->mediacfg.m_encparams.m_mvrange.meVertSearchRange) {
	case 24:
		mode = MESEARCH_24;
		break;
	case 40:
		mode = MESEARCH_40;
		break;
	case 48:
		mode = MESEARCH_48;
		break;
	case 64:
		mode = MESEARCH_64;
		break;
	default:
		return 0;
	}
	if (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_H264)
		ret = vsi_v4l2_hwconfig.h264info.map_meVertSearchRange & (1 << mode);
	else if (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_HEVC)
		ret = vsi_v4l2_hwconfig.hevcinfo.map_meVertSearchRange & (1 << mode);
	return (ret != 0);
}

int vsiv4l2_verify_slicemode(struct vsi_v4l2_ctx *ctx)
{
	u32 modemap = 0;

	if (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_H264)
		modemap = vsi_v4l2_hwconfig.h264info.map_multislicemode;
	else if (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_HEVC)
		modemap = vsi_v4l2_hwconfig.hevcinfo.map_multislicemode;
	else
		return 0;
	if (!(modemap & (1 << ctx->mediacfg.m_encparams.m_sliceinfo.multislice_mode)))
		ctx->mediacfg.m_encparams.m_sliceinfo.multislice_mode = V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE;
	if (ctx->mediacfg.m_encparams.m_sliceinfo.multislice_mode == V4L2_MPEG_VIDEO_MULTI_SLICE_MODE_SINGLE)
		ctx->mediacfg.m_encparams.m_sliceinfo.sliceSize = 0;
	return 1;
}

void vsiv4l2_verify_gopsize(struct vsi_v4l2_ctx *ctx)
{
	int hasBframe = 0;

	switch (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_H264:
		hasBframe = vsi_v4l2_hwconfig.h264info.has_Bframe;
		break;
	case V4L2_DAEMON_CODEC_ENC_HEVC:
		hasBframe = vsi_v4l2_hwconfig.hevcinfo.has_Bframe;
		break;
	case V4L2_DAEMON_CODEC_ENC_VP8:
		hasBframe = vsi_v4l2_hwconfig.vp8info.has_Bframe;
		break;
	case V4L2_DAEMON_CODEC_ENC_VP9:
		hasBframe = vsi_v4l2_hwconfig.vp9info.has_Bframe;
		break;
	case V4L2_DAEMON_CODEC_ENC_AV1:
		hasBframe = vsi_v4l2_hwconfig.av1info.has_Bframe;
		break;
	default:
		return;
	}
	if (!hasBframe)
		ctx->mediacfg.m_encparams.m_gopsize.gopSize = 1;
}

s32 vsiv4l2_verify_h264_codinglayerqp(s32 ctrlval)
{
	s32 layern = (u32)ctrlval >> 16;
	s32 qp = ctrlval & 0xffff;

	if (layern > vsi_v4l2_hwconfig.h264info.max_codinglayer)
		return -1;
	return  min(qp, vsi_v4l2_hwconfig.h264info.max_codinglayerQP);
}

int vsiv4l2_verify_vpxfiltersharp(struct vsi_v4l2_ctx *ctx)
{
	s32 maxsharp = 0;

	switch (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_VP8:
		maxsharp = vsi_v4l2_hwconfig.vp8info.max_vpxfilterSharpness;
		break;
	case V4L2_DAEMON_CODEC_ENC_WEBP:
		maxsharp = vsi_v4l2_hwconfig.webpinfo.max_vpxfilterSharpness;
		break;
	case V4L2_DAEMON_CODEC_ENC_VP9:
		maxsharp = vsi_v4l2_hwconfig.vp9info.max_vpxfilterSharpness;
		break;
	default:
		return 0;
	}
	ctx->mediacfg.m_encparams.m_vpxfiltersharp.filterSharpness =
		min(ctx->mediacfg.m_encparams.m_vpxfiltersharp.filterSharpness, maxsharp);
	return 1;
}

int vsiv4l2_verify_vpxgoldenrate(struct vsi_v4l2_ctx *ctx)
{
	s32 maxrate = 0;

	switch (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_VP8:
		maxrate = vsi_v4l2_hwconfig.vp8info.max_vpxgoldenPictureRate;
		break;
	case V4L2_DAEMON_CODEC_ENC_VP9:
		maxrate = vsi_v4l2_hwconfig.vp9info.max_vpxgoldenPictureRate;
		break;
	default:
		return 0;
	}
	ctx->mediacfg.m_encparams.m_goldenperiod.goldenPictureRate =
		min(ctx->mediacfg.m_encparams.m_goldenperiod.goldenPictureRate, maxrate);
	return 1;
}

int vsiv4l2_verify_vpxfilterlvl(struct vsi_v4l2_ctx *ctx)
{
	s32 maxlvl = 0;

	switch (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_VP8:
		maxlvl = vsi_v4l2_hwconfig.vp8info.max_vpxfilterLevel;
		break;
	case V4L2_DAEMON_CODEC_ENC_WEBP:
		maxlvl = vsi_v4l2_hwconfig.webpinfo.max_vpxfilterLevel;
		break;
	case V4L2_DAEMON_CODEC_ENC_VP9:
		maxlvl = vsi_v4l2_hwconfig.vp9info.max_vpxfilterLevel;
		break;
	default:
		return 0;
	}
	ctx->mediacfg.m_encparams.m_vpxfilterlvl.filterLevel =
		min(ctx->mediacfg.m_encparams.m_vpxfilterlvl.filterLevel, maxlvl);
	return 1;
}

int vsiv4l2_verify_refno(struct vsi_v4l2_ctx *ctx)
{
	s32 maxrefno = 0;
	s32 refn = 0;

	switch (ctx->mediacfg.m_encparams.m_codecfmt.codecFormat) {
	case V4L2_DAEMON_CODEC_ENC_HEVC:
		maxrefno = vsi_v4l2_hwconfig.hevcinfo.max_refno;
		refn = ctx->mediacfg.hevc_PRefN;
		break;
	case V4L2_DAEMON_CODEC_ENC_VP8:
		maxrefno = vsi_v4l2_hwconfig.vp8info.max_refno;
		refn = ctx->mediacfg.vpx_PRefN;
		break;
	default:
		return 0;
	}
	refn = min(refn, maxrefno);
	if (ctx->mediacfg.m_encparams.m_refno.refFrameAmount != refn) {
		ctx->mediacfg.m_encparams.m_refno.refFrameAmount = refn;
		return 1;
	}
	return 0;
}

void vsiv4l2_verify_vpxqphdr(struct vsi_v4l2_ctx *ctx)
{
	struct vsi_v4l2_encparams *param = &ctx->mediacfg.m_encparams;

	if (param->m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_VP8) {
		param->m_qphdrip.qpHdrI = min(ctx->mediacfg.qpHdrI_vpx, vsi_v4l2_hwconfig.vp8info.common.max_IQP);
		param->m_qphdrip.qpHdrI = max(param->m_qphdrip.qpHdrI, vsi_v4l2_hwconfig.vp8info.common.min_IQP);
		param->m_qphdrip.qpHdrP = min(ctx->mediacfg.qpHdrP_vpx, vsi_v4l2_hwconfig.vp8info.max_PBQP);
		param->m_qphdrip.qpHdrP = max(param->m_qphdrip.qpHdrP, vsi_v4l2_hwconfig.vp8info.min_PBQP);
	} else if (param->m_codecfmt.codecFormat == V4L2_DAEMON_CODEC_ENC_VP9) {
		param->m_qphdrip.qpHdrI = min(ctx->mediacfg.qpHdrI_vpx, vsi_v4l2_hwconfig.vp9info.common.max_IQP);
		param->m_qphdrip.qpHdrI = max(param->m_qphdrip.qpHdrI, vsi_v4l2_hwconfig.vp9info.common.min_IQP);
		param->m_qphdrip.qpHdrP = min(ctx->mediacfg.qpHdrP_vpx, vsi_v4l2_hwconfig.vp9info.max_PBQP);
		param->m_qphdrip.qpHdrP = max(param->m_qphdrip.qpHdrP, vsi_v4l2_hwconfig.vp9info.min_PBQP);
	}
}

s32 vsiv4l2_get_maxhevclayern(void)
{
	return vsi_v4l2_hwconfig.hevcinfo.max_codinglayer;
}

static int vsiv4l2_getVuiAspect(int aspectype, s32 *width, s32 *height)
{
	switch (aspectype) {
	case VSI_V4L2_VUI_ASPECT_1x1:
		*width = 1; *height = 1;
		break;
	case VSI_V4L2_VUI_ASPECT_4x3:
		*width = 4; *height = 3;
		break;
	case VSI_V4L2_VUI_ASPECT_16x9:
		*width = 16; *height = 9;
		break;
	case VSI_V4L2_VUI_ASPECT_221x100:
		*width = 221; *height = 100;
		break;
	default:
		return -1;
	}
	return 0;
}

//inversion of vsi_enc_convertV4l2Quality
static s32 vsi_v4l2_convertdaemonQuality(s32 daemonVal)
{
	if (daemonVal < 0)
		return daemonVal;
	if (daemonVal == 0)
		return 100;
	if (daemonVal > 51)
		return 0;
	return (51 - daemonVal) * 2;
}

void vsiv4l2_set_hwinfo(struct vsi_v4l2_dev_info *hwinfo)
{
	int i, j;
	s32 minqp, maxqp;
	s32 width, height;
	u64 temp64 = 0;

	vsi_v4l2_hwconfig = *hwinfo;

	//update encoder part
	if (vsi_v4l2_hwconfig.hevcinfo.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_HEVC);
	if (vsi_v4l2_hwconfig.h264info.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_H264);
	if (vsi_v4l2_hwconfig.jpeginfo.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_JPEG);
	if (vsi_v4l2_hwconfig.vp8info.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_VP8);
	if (vsi_v4l2_hwconfig.webpinfo.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_WEBP);
	if (vsi_v4l2_hwconfig.vp9info.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_VP9);
	if (vsi_v4l2_hwconfig.av1info.common.valid)
		temp64 |= ENC_FMT_BIT(V4L2_DAEMON_CODEC_ENC_AV1);

	v4l2_klog(LOGLVL_INFO, "hw support enc=%lx", (ulong)temp64);
	//update vsi_coded_fmt table
	for (i = 0; i < ARRAY_SIZE(vsi_coded_fmt); i++) {
		s32 enc = vsi_coded_fmt[i].enc_fmt;

		if (enc != V4L2_DAEMON_CODEC_UNKNOW_TYPE) {
			if ((ENC_FMT_BIT(enc) & temp64) == 0)
				vsi_coded_fmt[i].enc_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE;
		}
	}

	//update decoder part
	temp64 = 0;
	if (vsi_v4l2_hwconfig.decinfo_HEVC.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_HEVC);
	if (vsi_v4l2_hwconfig.decinfo_H264.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_H264);
	if (vsi_v4l2_hwconfig.decinfo_H264_MVC.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_H264_MVC);
	if (vsi_v4l2_hwconfig.decinfo_JPEG.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_JPEG);
	if (vsi_v4l2_hwconfig.decinfo_VP6.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_VP6);
	if (vsi_v4l2_hwconfig.decinfo_VP7.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_VP7);
	if (vsi_v4l2_hwconfig.decinfo_WEBP.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_WEBP);
	if (vsi_v4l2_hwconfig.decinfo_VP8.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_VP8);
	if (vsi_v4l2_hwconfig.decinfo_VP9.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_VP9);
	if (vsi_v4l2_hwconfig.decinfo_AV1.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_AV1);
	if (vsi_v4l2_hwconfig.decinfo_MPEG2.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_MPEG2);
	if (vsi_v4l2_hwconfig.decinfo_SORENSON.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_SORENSON);
	if (vsi_v4l2_hwconfig.decinfo_DIVX.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_DIVX);
	if (vsi_v4l2_hwconfig.decinfo_XVID.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_XVID);
	if (vsi_v4l2_hwconfig.decinfo_MPEG4.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_MPEG4);
	if (vsi_v4l2_hwconfig.decinfo_H263.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_H263);
	if (vsi_v4l2_hwconfig.decinfo_VC1_G.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_VC1_G);
	if (vsi_v4l2_hwconfig.decinfo_VC1_L.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_VC1_L);
	if (vsi_v4l2_hwconfig.decinfo_RV.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_RV);
	if (vsi_v4l2_hwconfig.decinfo_AVS.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_AVS);
	if (vsi_v4l2_hwconfig.decinfo_AVS2.common.valid)
		temp64 |= DEC_FMT_BIT(V4L2_DAEMON_CODEC_DEC_AVS2);

	v4l2_klog(LOGLVL_INFO, "hw support dec=%lx", (ulong)temp64);
	//update vsi_coded_fmt table
	for (i = 0; i < ARRAY_SIZE(vsi_coded_fmt); i++) {
		s32 dec = vsi_coded_fmt[i].dec_fmt;

		if ((DEC_FMT_BIT(dec) & temp64) == 0)
			vsi_coded_fmt[i].dec_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE;
	}

	//disable all jpg only output fmt
	if (vsi_v4l2_hwconfig.decinfo_JPEG.common.valid == 0) {
		for (j = 0; j < ARRAY_SIZE(vsi_raw_fmt); j++) {
			if (isJpegOnlyFmt(vsi_raw_fmt[j].dec_fmt))
				vsi_raw_fmt[j].dec_fmt = V4L2_DAEMON_CODEC_UNKNOW_TYPE;
		}
	}

	if (vsi_v4l2_hwconfig.has_framerc == 0)
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE, CTRL_IDX_MAX, 0);
	if (vsi_v4l2_hwconfig.has_ctbrc == 0)
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE, CTRL_IDX_MAX, 0);
	if (vsi_v4l2_hwconfig.has_rotation == 0)
		vsi_enc_update_ctrltbl(V4L2_CID_ROTATE, CTRL_IDX_MAX, 0);
	if (vsi_v4l2_hwconfig.has_mirror == 0)	//no impl yet
		;//vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE, CTRL_IDX_MAX, 0);
	if (vsi_v4l2_hwconfig.has_enc_securemode == 0)
		vsi_enc_update_ctrltbl(V4L2_CID_SECUREMODE, CTRL_IDX_MAX, 0);
	if (!(vsi_v4l2_hwconfig.map_headerMode & (1 << HEADERMODE_JOINED)))
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEADER_MODE, CTRL_IDX_MAX, V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE);
	if (!(vsi_v4l2_hwconfig.map_headerMode & (1 << HEADERMODE_SEPARATE)))
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEADER_MODE, CTRL_IDX_MIN, V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME);

	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_BITRATE_PEAK, CTRL_IDX_MAX, vsi_v4l2_hwconfig.max_bitrate);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_BITRATE, CTRL_IDX_MAX, vsi_v4l2_hwconfig.max_bitrate);
	vsi_enc_update_ctrltbl(V4L2_CID_ROI_COUNT, CTRL_IDX_MAX, vsi_v4l2_hwconfig.max_ROIregion);
	vsi_enc_update_ctrltbl(V4L2_CID_IPCM_COUNT, CTRL_IDX_MAX, vsi_v4l2_hwconfig.max_IPCMregion);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.max_gdrDuration);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET,
							CTRL_IDX_MIN, vsi_v4l2_hwconfig.h264info.min_chromaqpoffset);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.h264info.max_chromaqpoffset);
	vsi_enc_update_ctrltbl(V4L2_CID_JPEG_RESTART_INTERVAL,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.jpeginfo.max_restartinterval);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.h264info.has_transform8x8);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.h264info.has_constrained_intra_pred);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.hevcinfo.has_strong_intra_smoothing);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_TIER,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.hevcinfo.max_tier);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VP8_PROFILE,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.vp8info.max_profile);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VP9_PROFILE,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.vp9info.max_profile);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_PROFILE,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.hevcinfo.max_profile);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_PROFILE,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.h264info.max_profile);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_LEVEL,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.h264info.max_level);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_LEVEL,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.hevcinfo.max_level);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VP9_LEVEL,
							CTRL_IDX_MAX, vsi_v4l2_hwconfig.vp9info.max_level);

	minqp = min(vsi_v4l2_hwconfig.h264info.common.min_IQP, vsi_v4l2_hwconfig.h264info.min_PBQP);
	maxqp = max(vsi_v4l2_hwconfig.h264info.common.max_IQP, vsi_v4l2_hwconfig.h264info.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_MAX_QP, CTRL_IDX_MIN, minqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_MAX_QP, CTRL_IDX_MAX, maxqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_MIN_QP, CTRL_IDX_MIN, minqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_MIN_QP, CTRL_IDX_MAX, maxqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.common.min_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.common.max_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.common.min_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.common.max_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.common.min_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.common.max_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.h264info.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_PBQP);

	minqp = min(vsi_v4l2_hwconfig.hevcinfo.common.min_IQP, vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	maxqp = max(vsi_v4l2_hwconfig.hevcinfo.common.max_IQP, vsi_v4l2_hwconfig.hevcinfo.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP, CTRL_IDX_MIN, minqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP, CTRL_IDX_MAX, maxqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP, CTRL_IDX_MIN, minqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP, CTRL_IDX_MAX, maxqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.common.min_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.common.max_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.common.min_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.common.max_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.common.min_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.common.max_IQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP, CTRL_IDX_MIN,
						vsi_v4l2_hwconfig.hevcinfo.min_PBQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_PBQP);

	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.h264info.max_codinglayer);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayer);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP, CTRL_IDX_MAX,
				((u32)vsi_v4l2_hwconfig.h264info.max_codinglayer << 16) | vsi_v4l2_hwconfig.h264info.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_QP, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.hevcinfo.max_codinglayerQP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS, CTRL_IDX_MAX,
						vsi_v4l2_hwconfig.vp8info.max_vpxdctPartitions);

	//min/max set should always be ahead of def val
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_GOP_SIZE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.intraPicRate);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.intraPicRate);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_BITRATE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.bitPerSecond);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_PROFILE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.profile_h264);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VP8_PROFILE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.profile_vp8);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VP9_PROFILE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.profile_vp9);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_PROFILE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.profile_hevc);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_LEVEL, CTRL_IDX_DEF,
							vsi_v4l2_hwconfig.def_ctrlval.avclevel);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_LEVEL, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevclevel);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VP9_LEVEL, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.vp9level);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMax_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMax_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMin_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMin_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.multislice_mode);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_B_FRAMES, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.gopSize - 1);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_BITRATE_MODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.bitratemode);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrI_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrI_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrP_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrP_h26x);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.multislice_maxsliceMB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_picRc);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_ctbRc);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrI_vpx);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpHdrP_vpx);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMin_vpx);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMax_vpx);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_idrHdr);
	vsi_enc_update_ctrltbl(V4L2_CID_JPEG_COMPRESSION_QUALITY, CTRL_IDX_DEF,
					  vsi_v4l2_hwconfig.def_ctrlval.jpeg_quality);
	vsi_enc_update_ctrltbl(V4L2_CID_JPEG_RESTART_INTERVAL, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.jpg_restartInterval);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.gdrDuration);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_enableCabac);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_cpbSize);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_8x8transform);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY, CTRL_IDX_DEF,
						vsi_v4l2_convertdaemonQuality(vsi_v4l2_hwconfig.def_ctrlval.qpHdr));
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_pictureSkip + 1);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_AU_DELIMITER, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_sendAud);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_vui_sar_enable);
	if (vsiv4l2_getVuiAspect(vsi_v4l2_hwconfig.def_ctrlval.vui_aspect, &width, &height) == 0) {
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_ASPECT, CTRL_IDX_DEF,
							vsi_v4l2_hwconfig.def_ctrlval.vui_aspect);
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH, CTRL_IDX_DEF,
							width);
		vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT, CTRL_IDX_DEF,
							height);
		if (vsi_v4l2_hwconfig.def_ctrlval.vui_aspect == VSI_V4L2_VUI_ASPECT_1x1)
			vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC, CTRL_IDX_DEF,
							V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_1x1);
		else
			vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC, CTRL_IDX_DEF,
							V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_4x3);
	}
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_disableDeblockingFilter);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_disableDeblockingFilter ? 0 : 1);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.tc_Offset);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.tc_Offset);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.beta_Offset);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.beta_Offset);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_chroma_qp_offset);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMinI);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMinI);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMaxI);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMaxI);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMinPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMinPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMinPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMinPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMaxPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMaxPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMaxPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.qpMaxPB);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.meVertSearchRange);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.vpx_dctPartitions);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.vpx_PRefN > 0 ?
						(vsi_v4l2_hwconfig.def_ctrlval.vpx_PRefN - 1) : 0);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_PRefN);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.vpx_filterLevel);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.vpx_filterSharpness);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.vpx_goldenPictureRate);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_TIER, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_tier);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_refreshtype);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_temporalId);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_enableTMVP);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_withoutStartcode);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.resendSPSPPS);
	vsi_enc_update_ctrltbl(V4L2_CID_JPEG_CHROMA_SUBSAMPLING, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.jpg_codingMode);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_hierachy);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.enable_hierachy);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerNumber);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerNumber);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.codinglayerqp);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.h264_enableConstrainedIntraPred);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.hevc_enableStrongsmoothing);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_BITRATE, CTRL_IDX_DEF,
						vsi_v4l2_hwconfig.def_ctrlval.bitPerSecond);
	temp64 = (u64)vsi_v4l2_hwconfig.def_ctrlval.bitPerSecond *
					vsi_v4l2_hwconfig.def_ctrlval.tolMovingBitRate / 100;
	temp64 += vsi_v4l2_hwconfig.def_ctrlval.bitPerSecond;
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_BITRATE_PEAK, CTRL_IDX_DEF, (s32)temp64);
	vsi_enc_update_ctrltbl(V4L2_CID_MPEG_VIDEO_HEADER_MODE, CTRL_IDX_DEF, vsi_v4l2_hwconfig.def_ctrlval.headermode);
	//fixme: may lack these two:
	//V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE
	//V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE
}

struct vsi_v4l2_dev_info *vsiv4l2_get_hwinfo(void)
{
	return &vsi_v4l2_hwconfig;
}

void vsi_v4l2_update_ctrlcfg(struct v4l2_ctrl_config *cfg)
{
	switch (cfg->id) {
	case V4L2_CID_MPEG_VIDEO_H264_LEVEL:
		cfg->max = vsi_v4l2_hwconfig.decinfo_H264.max_level;
		break;
	case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL:
		cfg->max = vsi_v4l2_hwconfig.decinfo_HEVC.max_level;
		break;
	default:
		break;
	}
}

