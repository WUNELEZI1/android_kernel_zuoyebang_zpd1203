/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright 2018-2020 NXP
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#ifndef _UAPI__LINUX_IMX_VPU_H
#define _UAPI__LINUX_IMX_VPU_H

#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>

#define MAX_VIDEODEV_NO	16	//multi video device number ceil
/*imx v4l2 controls & extension controls*/

//ctrls & extension ctrls definitions
#define VSI_V4L2_CID_NORMAL_BASE			(V4L2_CID_USER_BASE + 0x1080)
#define V4L2_CID_NON_FRAME			(VSI_V4L2_CID_NORMAL_BASE + 0)
#define V4L2_CID_DIS_REORDER		(VSI_V4L2_CID_NORMAL_BASE + 1)
#define V4L2_CID_ROI_COUNT			(VSI_V4L2_CID_NORMAL_BASE + 2)
#define V4L2_CID_ROI				(VSI_V4L2_CID_NORMAL_BASE + 3)
#define V4L2_CID_IPCM_COUNT			(VSI_V4L2_CID_NORMAL_BASE + 4)
#define V4L2_CID_IPCM				(VSI_V4L2_CID_NORMAL_BASE + 5)
#define V4L2_CID_HDR10META			(VSI_V4L2_CID_NORMAL_BASE + 6)
#define V4L2_CID_SECUREMODE			(VSI_V4L2_CID_NORMAL_BASE + 7)
#define V4L2_CID_COMPRESSOR_CAPS	(VSI_V4L2_CID_NORMAL_BASE + 8)
#define V4L2_CID_COMPRESSOR_MODE	(VSI_V4L2_CID_NORMAL_BASE + 9)
#define V4L2_CID_DIVX_VERSION		(VSI_V4L2_CID_NORMAL_BASE + 10)
#define V4L2_CID_ADDR_OFFSET		(VSI_V4L2_CID_NORMAL_BASE + 11)
#define V4L2_CID_ENC_SCALE_INFO	(VSI_V4L2_CID_NORMAL_BASE + 12)
#define V4L2_CID_EXT_SLICE_ENCODE	(VSI_V4L2_CID_NORMAL_BASE + 13)

//extension to V4L2_CID_MPEG_VIDEO_BITRATE_MODE based on ctrl SW
#define V4L2_MPEG_VIDEO_BITRATE_MODE_CVBR		3
#define V4L2_MPEG_VIDEO_BITRATE_MODE_ABR		4
#define V4L2_MPEG_VIDEO_BITRATE_MODE_CRF		5
//endof extension to V4L2_CID_MPEG_VIDEO_BITRATE_MODE

/**  missing symbols for old linux version ***/
//symbols newin 5.10
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_1_0
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_1_0			0
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_1_1
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_1_1			1
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_2_0
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_2_0			2
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_2_1
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_2_1			3
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_3_0
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_3_0			4
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_3_1
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_3_1			5
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_4_0
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_4_0			6
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_4_1
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_4_1			7
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_5_0
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_5_0			8
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_5_1
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_5_1			9
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_5_2
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_5_2			10
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_6_0
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_6_0			11
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_6_1
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_6_1			12
#endif
#ifndef V4L2_MPEG_VIDEO_VP9_LEVEL_6_2
#define  V4L2_MPEG_VIDEO_VP9_LEVEL_6_2			13
#endif

/*  missing symbols for old linux version */
#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_5_2
#define V4L2_MPEG_VIDEO_H264_LEVEL_5_2	16
#endif
#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_6_0
#define V4L2_MPEG_VIDEO_H264_LEVEL_6_0	17
#endif
#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_6_1
#define V4L2_MPEG_VIDEO_H264_LEVEL_6_1	18
#endif
#ifndef V4L2_MPEG_VIDEO_H264_LEVEL_6_2
#define V4L2_MPEG_VIDEO_H264_LEVEL_6_2	19
#endif

#define VSI_V4L2_CID_MISSING_BASE			(V4L2_CID_USER_BASE + 0x1000)
#ifndef V4L2_CID_MPEG_VIDEO_VP9_LEVEL
#define V4L2_CID_MPEG_VIDEO_VP9_LEVEL			(VSI_V4L2_CID_MISSING_BASE + 11)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY
#define V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY		(VSI_V4L2_CID_MISSING_BASE + 12)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE
#define V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE		(VSI_V4L2_CID_MISSING_BASE + 13)
#endif
//symbols new in v5.11
#ifndef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP	(VSI_V4L2_CID_MISSING_BASE + 14)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP        (VSI_V4L2_CID_MISSING_BASE + 15)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP        (VSI_V4L2_CID_MISSING_BASE + 16)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP        (VSI_V4L2_CID_MISSING_BASE + 17)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP        (VSI_V4L2_CID_MISSING_BASE + 18)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP        (VSI_V4L2_CID_MISSING_BASE + 19)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP
#define V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP        (VSI_V4L2_CID_MISSING_BASE + 20)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_AU_DELIMITER
#define V4L2_CID_MPEG_VIDEO_AU_DELIMITER				(VSI_V4L2_CID_MISSING_BASE + 21)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP
#define V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP	(VSI_V4L2_CID_MISSING_BASE + 22)
#endif
//symbols new in v5.15
#ifndef V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD
#define V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD	(VSI_V4L2_CID_MISSING_BASE + 23)
#endif

//symbols new in v6.5
#ifndef V4L2_CID_MPEG_VIDEO_AV1_PROFILE
#define V4L2_CID_MPEG_VIDEO_AV1_PROFILE (V4L2_CID_CODEC_BASE + 655)
#endif
#ifndef V4L2_CID_MPEG_VIDEO_AV1_LEVEL
#define V4L2_CID_MPEG_VIDEO_AV1_LEVEL (V4L2_CID_CODEC_BASE + 656)
#endif

#ifndef V4L2_MPEG_VIDEO_AV1_PROFILE_MAIN
#define V4L2_MPEG_VIDEO_AV1_PROFILE_MAIN                   0
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_PROFILE_HIGH
#define V4L2_MPEG_VIDEO_AV1_PROFILE_HIGH                   1
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_PROFILE_PROFESSIONAL
#define V4L2_MPEG_VIDEO_AV1_PROFILE_PROFESSIONAL           2
#endif

#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_2_0
#define V4L2_MPEG_VIDEO_AV1_LEVEL_2_0                      0
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_2_1
#define V4L2_MPEG_VIDEO_AV1_LEVEL_2_1                      1
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_2_2
#define V4L2_MPEG_VIDEO_AV1_LEVEL_2_2                      2
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_2_3
#define V4L2_MPEG_VIDEO_AV1_LEVEL_2_3                      3
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_3_0
#define V4L2_MPEG_VIDEO_AV1_LEVEL_3_0                      4
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_3_1
#define V4L2_MPEG_VIDEO_AV1_LEVEL_3_1                      5
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_3_2
#define V4L2_MPEG_VIDEO_AV1_LEVEL_3_2                      6
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_3_3
#define V4L2_MPEG_VIDEO_AV1_LEVEL_3_3                      7
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_4_0
#define V4L2_MPEG_VIDEO_AV1_LEVEL_4_0                      8
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_4_1
#define V4L2_MPEG_VIDEO_AV1_LEVEL_4_1                      9
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_4_2
#define V4L2_MPEG_VIDEO_AV1_LEVEL_4_2                      10
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_4_3
#define V4L2_MPEG_VIDEO_AV1_LEVEL_4_3                      11
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_5_0
#define V4L2_MPEG_VIDEO_AV1_LEVEL_5_0                      12
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_5_1
#define V4L2_MPEG_VIDEO_AV1_LEVEL_5_1                      13
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_5_2
#define V4L2_MPEG_VIDEO_AV1_LEVEL_5_2                      14
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_5_3
#define V4L2_MPEG_VIDEO_AV1_LEVEL_5_3                      15
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_6_0
#define V4L2_MPEG_VIDEO_AV1_LEVEL_6_0                      16
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_6_1
#define V4L2_MPEG_VIDEO_AV1_LEVEL_6_1                      17
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_6_2
#define V4L2_MPEG_VIDEO_AV1_LEVEL_6_2                      18
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_6_3
#define V4L2_MPEG_VIDEO_AV1_LEVEL_6_3                      19
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_7_0
#define V4L2_MPEG_VIDEO_AV1_LEVEL_7_0                      20
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_7_1
#define V4L2_MPEG_VIDEO_AV1_LEVEL_7_1                      21
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_7_2
#define V4L2_MPEG_VIDEO_AV1_LEVEL_7_2                      22
#endif
#ifndef V4L2_MPEG_VIDEO_AV1_LEVEL_7_3
#define V4L2_MPEG_VIDEO_AV1_LEVEL_7_3                      23
#endif


/**  endof missing symbols for old linux version ***/

/* Extension of color macros copied from NXP ticket 464. In new kernel version they may be removed */
#define V4L2_COLORSPACE_GENERIC_FILM  (V4L2_COLORSPACE_DCI_P3+1)
#define V4L2_COLORSPACE_ST428			(V4L2_COLORSPACE_DCI_P3+2)

#define V4L2_XFER_FUNC_LINEAR			(V4L2_XFER_FUNC_SMPTE2084+1)
#define V4L2_XFER_FUNC_GAMMA22		(V4L2_XFER_FUNC_SMPTE2084+2)
#define V4L2_XFER_FUNC_GAMMA28		(V4L2_XFER_FUNC_SMPTE2084+3)
#define V4L2_XFER_FUNC_HLG				(V4L2_XFER_FUNC_SMPTE2084+4)
#define V4L2_XFER_FUNC_XVYCC			(V4L2_XFER_FUNC_SMPTE2084+5)
#define V4L2_XFER_FUNC_BT1361			(V4L2_XFER_FUNC_SMPTE2084+6)
#define V4L2_XFER_FUNC_ST428			(V4L2_XFER_FUNC_SMPTE2084+7)

#define V4L2_YCBCR_ENC_BT470_6M		(V4L2_YCBCR_ENC_SMPTE240M+1)


#define V4L2_MAX_ROI_REGIONS		(8)
struct v4l2_enc_roi_param {
	struct v4l2_rect rect;
	__u32 enable;
	__u32 qp_mode; /*0:obsolute QP; 1:delta QP*/
	__s32 qp;/*delta QP or absolute QP according to qp_mode*/
	__u32 reserved[2];
};

struct v4l2_enc_roi_params {
	__u32 num_roi_regions;
	struct v4l2_enc_roi_param roi_params[V4L2_MAX_ROI_REGIONS];
	__u32 config_store;
	__u32 reserved[2];
};

#define V4L2_MAX_IPCM_REGIONS		8
struct v4l2_enc_ipcm_param {
	struct v4l2_rect rect;
	__u32 enable;
	__u32 reserved[2];
};
struct v4l2_enc_ipcm_params {
	__u32 num_ipcm_regions;
	struct v4l2_enc_ipcm_param ipcm_params[V4L2_MAX_IPCM_REGIONS];
	__u32 config_store;
	__u32 reserved[2];
};

struct v4l2_hdr10_meta {
	__u32 hasHdr10Meta;
	__u32 redPrimary[2];
	__u32 greenPrimary[2];
	__u32 bluePrimary[2];
	__u32 whitePoint[2];
	__u32 maxMasteringLuminance;
	__u32 minMasteringLuminance;
	__u32 maxContentLightLevel;
	__u32 maxFrameAverageLightLevel;
};

//work with V4L2_CID_ENC_SCALE_INFO
struct v4l2_enc_scaleinfo {
	__u32 scaleWidth;
	__u32 scaleHeight;
	__u32 scaleOutput;			//if enable scale output
	__u32 scaleOutputFormat;		//0:yuv422, 1 YUV420SP
};

//for V4L2_CID_EXT_SET_PP_CFG
#define MAX_PP_CHANNELS	2
#define MAX_OUTPUT_PLANE 3
#define MAX_ENC_BATCH_TIMESTAMP 16

typedef enum {
	VSI_LINEAR = 0,
	LANCZOS,
	NEAREST,
	BILINEAR,
	BICUBIC,
	SPLINE,
	BOX,
	FAST_LINEAR,
	FAST_BICUBIC,
} v4l2_dec_pp_filter;

typedef enum {
	TILE4x4 = 1,
	TILE8x8,
	TILE16x16,
	TILE32x8,
	TILE128x2,
} v4l2_dec_tile_mode;

struct PpUnitConfig {
  __u32 enabled;    /**< PP unit enabled */
  /**< pixel format, only valid for non-first pp channel. */
  __u32 pix_fmt;
  __u32 compress_mode;/**< 0: no compress; 1: dec400 compress; 2: afbc compress. */
  /**< 0: raster scan; 1:TILE4x4; 2: TILE8x8; 3: TILE16x16; 4: TILE32x8; 5:TILE128x2.
   * When compress_mode is afbc compress, needs to set tile_mode to TILE16x16 or TILE32x8.*/
  __u32 tile_mode;
  __u32 ystride; /**< Stride for Y plane : If set to 0, software should use the stride calculated
                from software. When not 0, software should check the validation of the value. */
  __u64 chroma_offset; /**< chroma offset of current PPU to pp buffer start address, for single plane output format. */
  struct {
    __u32 enabled; /**< whether cropping is enabled */
    __u32 set_by_user; /**< cropping set by user, use this variable to record
                          whether user set crop.*/
    __u32 x;        /**< cropping start x */
    __u32 y;        /**< cropping start y */
    __u32 width;    /**< cropping width */
    __u32 height;   /**< cropping height */
  } crop;
  struct {
    __u32 enabled;
    __u32 x;        /**< cropping start x */
    __u32 y;        /**< cropping start y */
    __u32 width;    /**< cropping width */
    __u32 height;   /**< cropping height */
  } crop2;
  struct {
     __u32 enabled;  /**< whether scaling is enabled */
     __u32 scale_by_ratio;   /**< scaling output size will be calculated by ratio */
     __u32 ratio_x;  /**< 0 indicate flexiable mode, or 1/2/4/8 indicate ratio */
     __u32 ratio_y;  /**< 0 indicate flexiable mode, or 1/2/4/8 indicate ratio */
     __u32 width;    /**< scaled output width */
     __u32 height;   /**< scaled output height */
  } scale;
  /**< set the center point select algorithm for pp scale.
   * 0: DOWN_ROUND; 1: NO_ROUND; 2: UP_ROUND.*/
  __u32 src_sel_mode;
  __u32 monochrome; /**< PP output monochrome (luma only) for YUV output */
  enum v4l2_quantization quantization;/**< video range.*/
  __u32 rgb_stan;     /* color conversion standard applied to set coeffs */
  __u32 rgb_alpha;  /**< alpha channel value for RGB format */
  v4l2_dec_pp_filter pp_filter;/**< selector for pp scale algorithm. */
  __u32 x_filter_param; /**< when select LANCZOS scale, set the horizontal window param */
  __u32 y_filter_param; /**< when select LANCZOS scale, set the vertical window param */
  __u32 pad_sel; /**< whether pad value is used for pp scale for the picture boundary */
  __u32 pad_Y; /**< the luma pad value */
  __u32 pad_U; /**< the Cb pad value */
  __u32 pad_V; /**< the Cr pad value */
  __u32 lc_stripe;  /**< number of lines to trigger line counter interrupt */
  __u32 header_size;  /**< header size calculated by gralloc */
  __u32 payload_size;  /**< payload size calculated by gralloc */
};

struct v4l2_pp_channel_cfg {
	__u32 chn_id; /* ppu channel id for configuring */
	struct PpUnitConfig cfg; /* configuration */
};

struct v4l2_pp_cfg {
	__u32 cfg_num; /* number of ppu channels to configure */
	struct v4l2_pp_channel_cfg pp_cfg[MAX_PP_CHANNELS];
};

/*imx v4l2 command*/
#define V4L2_DEC_CMD_IMX_BASE		(0x08000000)
#define V4L2_DEC_CMD_RESET			(V4L2_DEC_CMD_IMX_BASE + 1)

/*imx v4l2 event*/
//error happened in dec/enc
#define V4L2_EVENT_CODEC_ERROR		(V4L2_EVENT_PRIVATE_START + 1)
//frame loss in dec/enc
#define V4L2_EVENT_SKIP				(V4L2_EVENT_PRIVATE_START + 2)
//crop area change in dec, not reso change
#define V4L2_EVENT_CROPCHANGE		(V4L2_EVENT_PRIVATE_START + 3)
//some options can't be handled by codec, so might be ignored or updated. But codec could go on.
#define V4L2_EVENT_INVALID_OPTION	(V4L2_EVENT_PRIVATE_START + 4)

/*imx v4l2 warning msg, attached with event V4L2_EVENT_INVALID_OPTION*/
enum {
	UNKONW_WARNING = -1,		//not known warning type
	ROIREGION_NOTALLOW,		//(part of)roi region can not work with media setting and be ignored by enc
	IPCMREGION_NOTALLOW,		//(part of)ipcm region can not work with media setting and be ignored by enc
	LEVEL_UPDATED,				//current level cant't work with media setting and be updated by enc

	INTRAREGION_NOTALLOW,		//intra region can not work with media setting and be ignored by enc
	GOPSIZE_NOTALLOW,			//wrong gop size (B frame numbers) setting and be updated by enc
};

/* used to explain struct v4l2_event.u.data for V4L2_EVENT_CROPCHANGE in app side */
struct v4l2_extra_info {
	__u32 offset[3]; //offset of each plane
	__u8 bit_depth; //pixel bit depth
	__u8 is_interlaced;/**< is sequence interlaced */
	__u8 fields_in_picture;/**< how many fields in picture:0-no field;1-one field;2-two fields */
	__u8 top_field_first;/**< 1-top field firs; 0-bottom field first */
	__u8 height_align_factor; //height align factor
	__u8 compressed; //whether the data is compressed (1) or not (0)
	__u8 dec400_tile_mode[3];//dec400 compressing tile mode of each plane
};

/* used to explain struct v4l2_event.u.data for V4L2_EVENT_SKIP in app side */
struct v4l2_pic_consumed_info {
  __u32 inbufidx; /* OUTPUT buffer index. */
  __u64 timestamp;
};

enum hw_compressor_format {
	COMPRESSOR_NONE = 0,
	COMPRESSOR_DEC400 = 1,
};

enum dec400_mode {
	DEC400_MODE_STREAM_BYPASS = (0 << 8),
	DEC400_MODE_WORKING = (1 << 8),
};

//definitions for frame meta info mask
#define META_VALID_MASK_HDR 0x00000001ULL /* New hdr info is valid (1) or not (0) */
#define META_VALID_MASK_PPOUT_LAYOUT 0x00000002ULL /* New multi-PP output layout info is valid or not */
#define META_VALID_MASK_CROP 0x00000004ULL /* New cropping info is valid or not */
#define META_VALID_MASK_SLICE_TEXT_INFO 0x00000008ULL /* New Slice Text Info*/
#define META_VALID_MASK_GRID_IDX 0x00000010ULL /* grid index */
#define META_DEC_VALID_MASK_PP_BUF 0x00000020ULL /* pp buffer*/
#define META_VALID_MASK_LAST_BUF 0x00000040ULL /* last CAPTURE buffer corresponding to OUTPUT buffer.*/
#define META_VALID_MASK_OPERATING_RATE 0x00000080ULL /* operating rate for codec.*/
#define META_VALID_MASK_BUF_LAYOUT_INFO 0x00000100ULL /* buffer layout info for dec CAPTURE meta */
#define META_VALID_MASK_Y_HISTOGRAM 0x00000200ULL
#define META_VALID_MASK_BUF_BATCH_TIMESTAMP 0x00001000ULL /* batch timestamp for high record */


//definitions for dec meta info mask
#define META_DEC_VALID_MASK_PPCFG 0x00000001ULL /* multipp cfg set */
#define META_DEC_VALID_MASK_BATCH_CFG 0x00000002ULL /* batch cfg set */
#define META_DEC_VALID_MASK_HEIF_CFG 0x00000004ULL /* heif cfg set */
#define META_DEC_VALID_MASK_SVCT_CFG 0x00000008ULL /* svct cfg set */
#define META_DEC_VALID_MASK_PP_BUF_INFO 0x00000010ULL /* pp buffer info set */
#define META_DEC_VALID_MASK_LEVEL_CFG 0x00000020ULL /* level set */
#define META_DEC_VALID_MASK_RES_CFG 0x00000040ULL /* resolution cfg */
#define META_DEC_VALID_MASK_HEIF_ENABLE 0x00000080ULL /* heif enable set */
#define META_DEC_VALID_MASK_SLICE_CFG 0x00000100ULL /* cfg for slice decoding */
#define META_DEC_VALID_MASK_LOW_LATENCY_SLICE_DISABLE 0x00000200ULL


//definitions for enc meta info mask
#define META_ENC_VALID_MASK_EOS_CFG 0x00000001ULL /* enc cfg for EOS data.*/
#define META_ENC_VALID_MASK_BATCH_CFG 0x00000002ULL /* batch cfg set */
#define META_ENC_VALID_MASK_HEIF_CFG 0x00000004ULL /* heif cfg set */
#define META_ENC_VALID_MASK_Q_TABLE 0x00000008ULL /* quantization table for jpeg.*/
#define META_ENC_VALID_MASK_UFBC_CFG 0x00000010ULL /* ufbc cfg set.*/
#define META_ENC_VALID_MASK_RDO_CFG 0x00000020ULL /* rdo cfg set.*/
#define META_ENC_VALID_MASK_THUMBNAIL_CFG 0x00000040ULL /* thumbnail cfg set.*/
#define META_ENC_VALID_MASK_BIT_DEPTH 0x00000080ULL /* bit depth cfg set.*/
#define META_ENC_VALID_MASK_CHROMA_FORMAT_IDC 0x00000100ULL /* chroma_format_idc set.*/
#define META_ENC_VALID_MASK_SLICE_IRQ 0x00000200ULL /* enc slice irq set.*/
#define META_ENC_VALID_MASK_PRIORITY 0x00000400ULL /* enc priority of current instance set.*/
#define META_ENC_VALID_MASK_FPS_ADJUST_CFG 0x00000800ULL /* enc cfg for FPS adjust set.*/
#define META_ENC_VALID_MASK_HIGHRECORD 0x00001000 /* enc high record set.*/
#define META_ENC_VALID_MASK_FRAME_RATE 0x00002000ULL /* frame rate for enc OUTPUT.*/

//x-ring definitions for enc meta info mask
#define META_ENC_VALID_MASK_SVCT_CFG 0x80000000ULL /* enc cfg for svct.*/
#define META_ENC_VALID_MASK_LTR_CFG 0x40000000ULL /* enc cfg for ltr.*/
#define META_ENC_VALID_MASK_INPUTSURFACE 0x20000000ULL /* enc cfg for mInputSurface.*/
#define META_ENC_VALID_MASK_BANKLOAN_ENABLE 0x10000000ULL /*bankloan enable for cvbr*/

enum v4l2_hdr_info_mask {
	META_HDR_VALID_MASK_VUI = 0x00000001, /* color description in the vui.*/
	META_HDR_VALID_MASK_MASTER = 0x00000002, /* mastering display color volume SEI message.*/
	META_HDR_VALID_MASK_LIGHT = 0x00000004, /* content light level info SEI message.*/
	META_HDR_VALID_MASK_T35 = 0x00000008, /* ITU-T-T35 data */
};

//vsi v4l2 meta type defs
#define V4L2_META_FMT_VSIENC		v4l2_fourcc('V', 'S', 'M', 'E')
#define V4L2_META_FMT_VSIDEC		v4l2_fourcc('V', 'S', 'M', 'D')
#define MAX_BLOCK_LEVEL 254
#define MAX_T35_PAYLOAD_NUM 64
#define MAX_T35_PAYLOAD_SIZE (9 * 1024)

/**
 * \brief Contains the information in Bezier curve packets defined by HDR10+.
 */
typedef struct {
  /** \brief <tt>tone_mapping_flag[w]</tt> indicates whether the metadata of the tone mapping
   *  function in the w-th processing window is present. */
  __u32 tone_mapping_flag[3];
  /** \brief <tt>knee_point_x[w]</tt> indicates the X coordinate of the separation point
   *  between the linear part and the curved part of the tone mapping function in the w-th
   *  processing window. */
  __u32 knee_point_x[3];
  /** \brief <tt>knee_point_y[w]</tt> indicates the Y coordinate of the separation point
   *  between the linear part and the curved part of the tone mapping function in the w-th
   *  processing window. */
  __u32 knee_point_y[3];
  /** \brief <tt>num_bezier_curve_anchors[w]</tt> indicates the number of the intermediate
   *  anchor parameters of the tone mapping function in the w-th processing window. */
  __u32 num_bezier_curve_anchors[3];
  /** \brief <tt>bezier_curve_anchors[w*15+i]</tt> indicates the i-th intermediate anchor
   *  parameter of the tone mapping function in the w-th processing window in the scene. */
  __u32 bezier_curve_anchors[3 * 15];
  /** \brief <tt>color_saturation_mapping_flag[w]</tt> indicates the color saturation mapping
   *  flag for the w-th processing window.
   *  \n <tt>0</tt>: compliant with SMPTE ST 2094-40.
   *  \n <tt>1</tt>: reserved for future use. */
  __u32 color_saturation_mapping_flag[3];
  /** \brief <tt>color_saturation_weight[w]</tt> indicates the color saturation gain in the
   *  w-th processing window in the scene. */
  __u32 color_saturation_weight[3];
} bezier_dict;

/**
 * \brief Contains the information in Mastering packets defined by HDR10+.
 */
typedef struct {
  /** \brief The actual peak luminance flag of the mastering display.
   *  \n <tt>0</tt>: compliant with SMPTE ST 2094-40.
   *  \n <tt>1</tt>: reserved for future use. */
  __u32 mastering_display_actual_peak_luminance_flag;
  /** \brief The number of rows in the <tt>mastering_dict.mastering_display_actual_peak_luminance</tt>
   *  array. */
  __u32 num_rows_mastering_display_actual_peak_luminance;
  /** \brief The number of columns in the <tt>mastering_dict.mastering_display_actual_peak_luminance</tt>
   *  array. */
  __u32 num_cols_mastering_display_actual_peak_luminance;
  /** \brief The normalized actual peak luminance of the mastering display. */
  __u32 mastering_display_actual_peak_luminance[31 * 31];
} mastering_dict;

/**
 * \brief Contains the information in DistDict packets defined by HDR10+.
 */
typedef struct {
  /** \brief The maximum linearized RGB values in each processing window in the scene.
   *  \n <tt>maxscl[w*3+i]</tt> indicates the maximum value for the i-th color component of
   *  R, G, and B in the w-th processing window. */
  __u32 maxscl[3 * 3];
  /** \brief <tt>average_maxrgb[w]</tt> indicates the average of maximum linearized RGB values
   *  in the w-th processing window in the scene. */
  __u32 average_maxrgb[3];
  /** \brief <tt>num_distribution_maxrgb_percentiles[w]</tt> indicates the number of maximum
   *  linearized RGB values at given percentiles in the w-th processing window in the scene. */
  __u32 num_distribution_maxrgb_percentiles[3];
  /** \brief <tt>distribution_maxrgb_percentages[w*15+i]</tt> indicates the integer percentage
   *  of maximum linearized RGB values at the i-th percentile in the w-th processing window in
   *  the scene. */
  __u32 distribution_maxrgb_percentages[3 * 15];
  /** \brief <tt>distribution_maxrgb_percentiles[w*15+i]</tt> indicates the number of maximum
   *  linearized RGB values at the i-th percentile in the w-th processing window in the scene. */
  __u32 distribution_maxrgb_percentiles[3 * 15];
  /** \brief <tt>fraction_bright_pixels[w]</tt> indicates the fraction of selected pixels in
   *  the picture that contains the brightest pixel in the w-th processing window in the scene. */
  __u32 fraction_bright_pixels[3];
} dist_dict;

/**
 * \brief Contains the information in peak_lum packets defined by HDR10+.
 */
typedef struct {
  /** \brief The number of rows in the <tt>peak_lum.targeted_system_display_actual_peak_luminance</tt>
   *  array. */
  __u32 num_rows_targeted_system_display_actual_peak_luminance;
  /** \brief The number of columns in the <tt>peak_lum.targeted_system_display_actual_peak_luminance</tt>
   *  array. */
  __u32 num_cols_targeted_system_display_actual_peak_luminance;
  /** \brief The normalized actual peak luminance of targeted system display. */
  __u32 targeted_system_display_actual_peak_luminance[31 * 31];
} peak_lum;

/**
 * \brief Contains the information of a processing window in proc_windows packets defined by HDR10+.
 */
typedef struct {
  /** \brief The X coordinate of the top-left pixel of the processing window. */
  __u32 window_upper_left_corner_x;
  /** \brief The Y coordinate of the top-left pixel of the processing window. */
  __u32 window_upper_left_corner_y;
  /** \brief The X coordinate of the bottom-left pixel of the processing window. */
  __u32 window_lower_left_corner_x;
  /** \brief The Y coordinate of the bottom-left pixel of the processing window. */
  __u32 window_lower_left_corner_y;
  /** \brief The X coordinate of the center position of concentric internal and external ellipses
   *  of the elliptical pixel selector in the processing window. */
  __u32 center_of_elipse_x;
  /** \brief The Y coordinate of the center position of concentric internal and external ellipses
   *  of the elliptical pixel selector in the processing window. */
  __u32 center_of_elipse_y;
  /** \brief The clockwise rotation angle in degrees, relative to the positive direction of the
   *  X-axis of the concentric internal and external ellipses of the elliptical pixel selector in
   *  the processing window. */
  __u32 rotation_angle;
  /** \brief The semi-major axis value, in pixels, of the internal ellipse of the elliptical pixel
   *  selector in the processing window. */
  __u32 semimajor_axis_internal_elipse;
  /** \brief The semi-major axis value, in pixels, of the external ellipse of the elliptical pixel
   *  selector in the processing window. */
  __u32 semimajor_axis_external_elipse;
  /** \brief The semi-minor axis value, in pixels, of the external ellipse of the elliptical pixel
   *  selector in the processing window. */
  __u32 semiminor_axis_external_elipse;
  /** \brief The method to combine rendered pixels in the processing window with at least one
   *  elliptical pixel selector.
   *  \n <tt>0</tt>: weighted averaging.
   *  \n <tt>1</tt>: layering. */
  __u32 overlap_process_option;
} proc_windows;

/**
 * \brief Contains the information of an extended display mapping metadata block in ext_block
 * packets defined by Dolby Vision.
 */
typedef struct {
  /** \brief A length used to derive the size of the extended display mapping metadata block payload
   *  in bytes.
   *  \n The value range is from <tt>0</tt> to <tt>1023</tt>, inclusive. */
  __u32 ext_block_length;
  /** \brief The level of payload contained in the extended display mapping metadata block.
   *  \n The value range is from <tt>0</tt> to <tt>255</tt>, inclusive. */
  __u32 ext_block_level;
  /** \brief The minimum luminance value of the current picture in 12-bit PQ encoding.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive. */
  __u32 min_PQ;
  /** \brief The maximum luminance value of the current picture in 12-bit PQ encoding.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive. */
  __u32 max_PQ;
  /** \brief The midpoint luminance value of the current picture in 12-bit PQ encoding.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive. */
  __u32 avg_PQ;
  /** \brief The maximum luminance value of a target display in 12-bit PQ encoding.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive. */
  __u32 target_max_PQ;
  /** \brief The slope metadata.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive.
   *  \n If trim_slope is not present, the value is <tt>2048</tt>. */
  __u32 trim_slope;
  /** \brief The offset metadata.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive.
   *  \n If trim_offset is not present, the value is <tt>2048</tt>. */
  __u32 trim_offset;
  /** \brief The power metadata.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive.
   *  \n If trim_power is not present, the value is <tt>2048</tt>. */
  __u32 trim_power;
  /** \brief The chroma weight metadata.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive.
   *  \n If trim_chroma_weight is not present, the value is <tt>2048</tt>. */
  __u32 trim_chroma_weight;
  /** \brief The saturation gain metadata.
   *  \n The value range is from <tt>0</tt> to <tt>4095</tt>, inclusive.
   *  \n If trim_saturation_gain is not present, the value is <tt>2048</tt>. */
  __u32 trim_saturation_gain;
  /** \brief Reserved. */
  __u32 ms_weight;
  /** \brief The X offset between the left border of the active area and the leftmost
   *  selected pixel of the current picture.
   *  \n The value range is from <tt>0</tt> to <tt>8191</tt>, inclusive. For details,
   *  see ProcessingWindow definitions in SMPTE ST 2094-10. */
  __u32 active_area_left_offset;
  /** \brief The X offset between the right border of the active area and the rightmost
   *  selected pixel of the current picture.
   *  \n The value range is from <tt>0</tt> to <tt>8191</tt>, inclusive. For details,
   *  see ProcessingWindow definitions in SMPTE ST 2094-10. */
  __u32 active_area_right_offset;
  /** \brief The Y offset between the top border of the active area and the topmost
   *  selected pixel of the current picture.
   *  \n The value range is from <tt>0</tt> to <tt>8191</tt>, inclusive. For details,
   *  see ProcessingWindow definitions in SMPTE ST 2094-10. */
  __u32 active_area_top_offset;
  /** \brief The Y offset between the bottom border of the active area and the bottommost
   *  selected pixel of the current picture.
   *  \n The value range is from <tt>0</tt> to <tt>8191</tt>, inclusive. For details,
   *  see ProcessingWindow definitions in SMPTE ST 2094-10. */
  __u32 active_area_bottom_offset;
} ext_block;

/**
 * \brief Contains the information in meta_data packets defined by Dolby Vision.
 */
typedef struct {
  /** \brief The number of extended display mapping metadata blocks. */
  __u32 num_ext_blocks;
  /** \brief The information of each extended display mapping metadata block. */
  ext_block ext_dm_data_block[100];
} meta_data;

struct t35_hdr10_plus {
  /** \brief The country code defined in ITU-T Rec. T.35. */
  __u32 itu_t_t35_country_code;
  /** \brief The terminal provider code defined in ITU-T Rec. T.35. */
  __u32 itu_t_t35_terminal_provider_code;
  /** \brief The terminal provider oriented code defined in ITU-T Rec. T.35. */
  __u32 itu_t_t35_terminal_provider_oriented_code;
  /** \brief The application identifier. */
  __u32 application_identifier;
  /** \brief The application version. */
  __u32 application_version;
  /** \brief The number of windows.
   *  \n A maximum of three windows are supported. */
  __u32 num_windows;
  /** \brief The information in processing window packets of HDR10+, with
   *  <tt>processing_windows[w]</tt> for the w-th processing window. */
  proc_windows processing_windows[3];
  /** \brief The maximum luminance for display, in the unit of 0.0001 candelas
   *  per square meter. */
  __u32 targeted_system_display_maximum_luminance;
  /** \brief The actual peak luminance flag of the targeted system display.
   *  \n <tt>0</tt>: compliant with SMPTE ST 2094-40.
   *  \n <tt>1</tt>: reserved for future use. */
  __u32 targeted_system_display_actual_peak_luminance_flag;
  /** \brief The information in actual peak luminance packets of HDR10+. */
  peak_lum targeted_system_display_actual_peak_luminance;
  /** \brief The information in distribution packets of HDR10+. */
  dist_dict distribution;
  /** \brief The information in mastering packets of HDR10+. */
  mastering_dict mastering;
  /** \brief The information in bezier curve packets of HDR10+. */
  bezier_dict bezier_curve;
};

struct t35_dolby_vision {
  /** \brief The country code defined in ITU-T Rec. T.35. */
  __u32 itu_t_t35_country_code;
  /** \brief The terminal provider code defined in ITU-T Rec. T.35. */
  __u32 itu_t_t35_terminal_provider_code;
  /** \brief The terminal provider oriented code defined in ITU-T Rec. T.35. */
  __u32 itu_t_t35_terminal_provider_oriented_code;
  /** \brief The application identifier. */
  __u32 application_identifier;
  /** \brief The application version. */
  __u32 application_version;
  /** \brief The data type code defined by Dolby Vision. */
  __u32 data_type_code;
  /** \brief The metadata refresh flag defined by Dolby Vision.
   *  \n This field is used to identify whether the data to be parsed is compatible
   *  with HDR10+ or Dolby Vision.
   *  \n <tt>0</tt>: HDR10+.
   *  \n <tt>1</tt>: Dolby Vision. */
  __u32 metadata_refrensh_flag;
  /** \brief The metadata packets of Dolby Vision. */
  meta_data metadata;
};

struct t35_hdr_info {
  __u8 type; /* 0: hdr10+; 1: dolby.*/
  union {
    struct t35_hdr10_plus hdr10_plus;
    struct t35_dolby_vision dolby;
  } data;
};

struct master_display_params {
	__u32 mastering_display_present_flag;
	__u16 display_primaries_x[3];
	__u16 display_primaries_y[3];
	__u16 white_point_x;
	__u16 white_point_y;
	__u32 max_display_mastering_luminance;
	__u32 min_display_mastering_luminance;
};


struct light_level_params {
	__u32 lightlevel_present_flag;
	__u16 max_content_light_level;
	__u16 max_pic_average_light_level;
};

struct vui_color_description {
	__u32 colour_description_present_flag;
	__u32 colour_primaries;
	__u32 transfer_characteristics;
	__u32 matrix_coefficients;
};

struct vui_params {
	struct vui_color_description vui_color_param;
	/* video signal sample range in the stream.
	   0: limited range, sample range [16, 235];
	   1: full range, sample range [0, 255]. */
	__u32 video_full_range_flag;
};

struct t35_params {
	__u8 type; /* 0: itu35 data structure; 1: itu35 data packed data */
	__u32 num; /* valid t35 number */
	union {
		struct t35_hdr_info info; /* for payload type 0 */
		struct {
			__u32 size; /* size of packed data, in byte */
			__u8 data[MAX_T35_PAYLOAD_SIZE];
		} pack; /* for payload type 1 */
	} payload[MAX_T35_PAYLOAD_NUM];
};

struct v4l2_hdr_payload_info {
	__u32 mask;/* bit-mask of valid hdr info, refer to enum v4l2_hdr_info_mask.*/
	struct t35_params t35_param;
	struct vui_params vui_param;
	struct master_display_params master_display_param;
	struct light_level_params light_level_param;
	__u8 t35_country_code;
};

struct v4l2_hdr_info {
	/* offset of hdr payload(struct v4l2_hdr_payload_info)
	   from the start of metadata buffer, in byte.*/
	__u32 offset;
};

struct v4l2_pp_chn_layout{
  __u32 ppu_chn; /* ppu channel id*/
  __u32 offset[MAX_OUTPUT_PLANE]; /* offset of each component plane to frame buffer start, in
  byte */
  __u32 stride[MAX_OUTPUT_PLANE]; /* stride of each component plane in byte */
  __u32 bytesused[MAX_OUTPUT_PLANE]; /* bytesused of each memory plane in byte */
  __u32 width; /* width in pixel of current ppu channel */
  __u32 height; /* height in pixel of current ppu channel */
};

struct v4l2_pp_layout {
	__u32 num; /* number of valid ppu output channels */
	struct v4l2_pp_chn_layout info[MAX_PP_CHANNELS];
};

struct v4l2_pp_chn_buf{
  __u32 ppu_chn; /* ppu channel id */
  __u32 size[MAX_OUTPUT_PLANE];
  __u64 bus_addr[MAX_OUTPUT_PLANE];
};

struct v4l2_pp_buf {
	__u32 num; /* number of valid ppu output channels */
	struct v4l2_pp_chn_buf info[MAX_PP_CHANNELS];
};

struct v4l2_buf_layout_info {
	__u32 num; /* number of valid buffer layout info */
	struct {
		__u32 ppu_chn; /* ppu channel id */
		__u32 stride; /* stride for y plane */
		/* chroma offset to buffer start address, only
		valid for single memory plane output format. */
		__u64 chroma_offset;
	} info[MAX_PP_CHANNELS];
};

struct v4l2_pp_buf_info {
	__u32 num; /* number of valid ppu output channels */
	struct {
		__u32 ppu_chn; /* ppu channel id */
		__u8 num_planes; /* number of memory planes */
		__u8 is_pp_buffer_recycle; /* need confirm whether support */
		/* decoder needed pp buffer size of each plane */
		__u32 size[MAX_OUTPUT_PLANE];
	} info[MAX_PP_CHANNELS];
};

struct v4l2_cropping {
	__u32 pic_width;
	__u32 pic_height;
	__u32 crop_left;
	__u32 crop_top;
	__u32 crop_width;
	__u32 crop_height;
};

struct v4l2_dec_slice_text_info {
  __u32 slice_txt_cnt; /**< count of texts to be output when a slice is decoded */
  struct {
    __u32 lines;  /**< line number to output of this slice */
    __u64 text_base; /**< base bus address of slice text buffer */
    __u32 buf_size; /* size of slice text buffer, in bytes. */
    __u32 text[4];  /**< values of 4 texts to be written to slice text buffer */
    __u32 offset[4]; /**< offsets of 4 texts to the slice text buffer for each text */
  } slice_params[4]; /**< struct of slice parameters of slice texts */
};

/**
 * \brief configure for slice decoding.
 * \ingroup meta_group
 */
struct v4l2_dec_slice_cfg {
  /** \brief If power off and on happend in slice decoding, to make sure
   * the output picture are correct after resume, daemon will hold all the
   * slice input buffers of one frame and won't return them to applications until
   * the frame is decoded. But the input buffer number allocated by applications
   * may be smaller than the slice number of one frame, so to avoid hang issue,
   * applications can set the max input buffer number(default is 8) that daemon can hold. */
  __u32 max_inbuf_num;
};

struct v4l2_enc_slice_text_info {
	/* 0 = Disable DDR low-latency mode.*/
	/* 1 = Enabel. HW will poll sliceinfo, which in a 64 byte ddr space.*/
	__u32 slice_info_en;
	/**< \brief The base bus address of the slice information for low-latency encoding.*/
	__u64 slice_info_base;
	__u32 buf_size; /* size of slice text buffer, in bytes. */
};

struct v4l2_frame_rate {
	/* The numerator for calculating the target frame rate.*/
	__u32 numerator;
	/* The denominator for calculating the target frame rate.*/
	__u32 denominator;
};

struct v4l2_enc_fps_adjust_cfg {
	/* send new SPS on FPS change or not
	* 0 = not send SPS
	* 1 = send SPS */
	__u8 send_sps;
	/* insert IDR on FPS change or not
	 * 0 = not insert IDR
	 * 1 = insert IDR */
	__u8 insert_idr;
};

struct v4l2_enc_slice_irq {
	/* Whether to enable the hardware to issue an
		interrupt each time it finishes encoding a slice.
		0: disable; 1: enable. */
	__u8 enable;
};

struct v4l2_enc_eos_cfg {
	/* Whether to enable writing EOS data in last
	   CAPTURE buffer(for video), default is enabled.
	   1: enable; 0: disable. */
	__u8 enable;
};

struct v4l2_enc_priority {
	/* the priority of current instance. */
	__u32 value;
};

struct v4l2_enc_high_record_cfg {
	/* config high record mode*/
	__u8 is_high_record_mode;
	__u32 frame_size;
};

struct v4l2_batch_timestamp {
	__u32 count;
	__u64 timestamp[MAX_ENC_BATCH_TIMESTAMP];
};

struct v4l2_enc_batch_cfg {
	/* 1: aggregation, for decoder and encoder;
	   2: high-rate-encoding, for encoder only.*/
	__u32 mode;
	/* for aggregation, it's means "maximum batch count+1";
	   for high-rate-encoding,it means the maximun number of tasks
	   in VCMD and suggests set it to "duration_without_int/resolution+1" */
	__u32 parallel_core_num;
	__u32 init_batch_count; /* batch count for instance start stage, for aggregation mode only.*/
	__u32 batch_count; /* batch count for instance normal stage, for aggregation mode only.*/
};

struct v4l2_dec_svct_cfg {
	__s8 max_temporal_layer; /*The maximum temporal layer(only hevc/h264/vvc).*/
};

struct v4l2_dec_batch_cfg {
	/* The frame count at init batch stage(batch size is 1), to support batch
	   size from 1->N. for example, when init_batch_count is set to 3, it means
	   batch size of first 3 input frames is 1.*/
	__u32 init_batch_count;
	__u32 batch_enable; /* The batch enable flag.*/
};

struct v4l2_enc_chroma_format_idc {
	/*chroma sampling modes relative to luma sampling.
		0-4:0:0; 1-4:2:0; 2-4:2:2 (under development) */
	__u8 value;
};

struct v4l2_enc_heif_cfg {
	__u32 input_mode; /* 0:grid mode; 1:whole-frame mode */
	/* blow fields are available for whole frame input mode only */
	__u32 grid_width; /* width of grid, in pixel */
	__u32 grid_height; /* height of grid, in pixel */
};

struct v4l2_enc_q_table {
	__u8 luma[64];/* quantization table for luminance.*/
  __u8 chroma[64];/* quantization table for chrominance.*/
};

struct v4l2_enc_ufbc_cfg {
	__u8 enable;/*0: UFBC enable; 1: UFBC disable.*/
	__u8 yuv_trans;/*0: enable interal YUV Transform; 1: disable interal YUV Transform.*/
	__u8 block_type;/*0: superblock type is 32x8; 1: superblock type is 16x16. */
	__u8 block_split;/*0: superblock split mode off; 1: superblock split mode on.*/
};

struct v4l2_enc_rdo_cfg {
	/* RDO level, valid values:0,1,2.*/
	__u8 rdo_level;
	/* 0: disable dynamic rdo; 1: enable dynamic rdo.*/
	__u8 dynamic_rdo_enable;
	/* programable hardware dynamic rdo cu16 bias, valid values: 0..255.*/
	__u8 dynamic_rdo_cu16_bias;
	/* programable hardware dynamic rdo cu16 factor, valid values: 0..255.*/
	__u8 dynamic_rdo_cu16_factor;
	/* programable hardware dynamic rdo cu32 bias, valid values: 0..255.*/
	__u8 dynamic_rdo_cu32_bias;
	/* programable hardware dynamic rdo cu32 factor, valid values: 0..255.*/
	__u8 dynamic_rdo_cu32_factor;
	/* 0: disable rdo Quantization; 1: enable rdo Quantization.*/
	__u8 rdoq_enable;
	/* 0: Disable rdoq map Mode; 1: Enable rdoq map Mode.*/
	__u8 rdoq_map_enable;
};

struct v4l2_enc_thumbnail_cfg {
	__u8 format; /* Thumbnail format,1:JPEG;2:RGB8;3:RGB24.*/
	__u16 width; /* Thumbnail width in pixels.*/
	__u16 height;/* Thumbnail height in pixels.*/
	__u64 bus_addr; /* Bus address of thumbnail data.*/
	__u32 size; /* Thumbnail data amount in bytes.*/
};

struct v4l2_enc_bit_depth {
	/* The bit depth of luma samples in the encoded stream.
		8:8-bit luma samples.10:10-bit luma samples. */
	__u8 luma;
	/* The bit depth of chroma samples in the encoded stream.
		8:8-bit chroma samples.10:10-bit chroma samples. */
	__u8 chroma;
};

struct v4l2_dec_heif_cfg {
	/* heif image type, 0: master sequnce, 1:thumbnail sequence, 2:master image, 3:thumbnail image */
	__u32 heif_img_type;
	/* it's for master image, identify it has grid, 0: doesn't have grid, 1: has grid */
	__u32 has_grid;
	/* blow fields are available when has_grid=1 */
	__u32 grid_rows; /* grid rows of whole frame */
	__u32 grid_columns; /* grid columns of whole frame */
	__u32 grid_width; /* width of grid, in pixel */
	__u32 grid_height; /* height of grid, in pixel */
};

struct v4l2_dec_heif_idx {
	__u32 grid_idx; /* grid index */
};

struct v4l2_dec_level_cfg {
	/* level in stream syntax(for vp9, enum v4l2_mpeg_video_vp9_level). */
	__u32 level;
};

struct v4l2_enc_yhistogram {
	int y_his[1024];
};

struct v4l2_enc_svct_cfg {
	__u32 layerCount;
};

struct v4l2_enc_ltr_cfg {
	__u32 ltr_interval;
  __u32 ltr_offset;
	__u32 ltr_gap;
};

struct v4l2_dec_res_cfg {
	/* max width, for vp9. */
	__u32 max_width;
	/* max height, for vp9. */
	__u32 max_height;
};

/* so the meta layout of dec is:
v4l2_vsi_stream_meta_info_dec:[v4l2_vsi_dec_meta_info_OUTPUT 0:... v4l2_vsi_dec_meta_info_OUTPUT m]:[v4l2_vsi_dec_meta_info_CAPTURE 0..v4l2_vsi_dec_meta_info_CAPTURE n][v4l2_hdr_payload_info 0:... v4l2_hdr_payload_info n]
meta layout of enc is:
v4l2_vsi_stream_meta_info_enc:[v4l2_vsi_enc_meta_info_OUTPUT 0:... v4l2_vsi_enc_meta_info_OUTPUT m]:[v4l2_vsi_enc_meta_info_CAPTURE 0..v4l2_vsi_enc_meta_info_CAPTURE n][v4l2_hdr_payload_info 0:... v4l2_hdr_payload_info m]
 */
//global meta info for dec stream
struct v4l2_vsi_stream_meta_info_dec {
	__u64 validmap;	//v4l2_dec_meta_info_mask
	__u32 OUTPUT_num;		//OUTPUT meta buffer number
	__u32 CAPTURE_num;	//CAPTURE meta buffer number
	__u8  is_output_buffer_recycle;
	__u8  is_capture_buffer_recycle;
	struct v4l2_pp_cfg mutippcfg;
	struct v4l2_dec_batch_cfg batch_cfg;
	struct v4l2_dec_heif_cfg heif_cfg;
	struct v4l2_dec_svct_cfg svct_cfg;
	struct v4l2_pp_buf_info pp_buf_info;/* pp buffer info for multi-plane.*/
	struct v4l2_dec_level_cfg level;
	struct v4l2_dec_res_cfg res_cfg;
	struct v4l2_dec_slice_cfg slice_cfg; /* configure for slice decoding. */
};

//global meta info for enc stream
struct v4l2_vsi_stream_meta_info_enc {
	__u64 validmap;		//v4l2_enc_meta_info_mask
	__u32 OUTPUT_num;		//as in dec
	__u32 CAPTURE_num;
	__u8  is_output_buffer_recycle;
	__u8  is_capture_buffer_recycle;
	struct v4l2_enc_batch_cfg batch_cfg;
	struct v4l2_enc_heif_cfg heif_cfg;
	struct v4l2_enc_q_table q_table;
	struct v4l2_enc_ufbc_cfg ufbc_cfg;
	struct v4l2_enc_rdo_cfg rdo_cfg;
	struct v4l2_enc_thumbnail_cfg thumbnail_cfg;
	struct v4l2_enc_bit_depth bit_depth;
	struct v4l2_enc_chroma_format_idc chroma_format_idc;
	struct v4l2_enc_slice_irq slice_irq;
	struct v4l2_enc_priority priority;
	struct v4l2_enc_high_record_cfg hr_cfg;
	struct v4l2_enc_fps_adjust_cfg fps_adjust_cfg;
	struct v4l2_enc_svct_cfg svct_cfg;
	struct v4l2_enc_ltr_cfg ltr_cfg;
        struct v4l2_enc_eos_cfg eos_cfg;
};

/* meta info on each frame in two arrays:output and capture */
struct v4l2_vsi_dec_meta_info_OUTPUT {
	__u64 valid; /* bit-mask of valid meta info, refer to v4l2_frame_meta_info_mask */
	struct v4l2_dec_slice_text_info slice_text; /* slice text info, for input */
	struct v4l2_dec_heif_idx grid_idx; /* grid index */
	struct v4l2_frame_rate operating_rate;/* operating rate info for input*/
};

struct v4l2_vsi_dec_meta_info_CAPTURE {
	__u64 valid; /* bit-mask of valid meta info, refer to v4l2_frame_meta_info_mask */
	struct v4l2_hdr_info hdr_info; /* hdr info for output*/
	struct v4l2_pp_layout pp_layout; /* multi-PP output layout info for output*/
	struct v4l2_cropping cropping; /* cropping info for output.*/
	struct v4l2_pp_buf pp_buf; /* pp buffer for multi-plane.*/
	struct v4l2_buf_layout_info buf_layout; /* buffer layout info.*/
};

struct v4l2_vsi_enc_meta_info_OUTPUT {
	__u64 valid; /* bit-mask of valid meta info, refer to v4l2_frame_meta_info_mask */
	struct v4l2_hdr_info hdr_info; /* hdr info for input*/
	struct v4l2_enc_slice_text_info slice_text; /* slice text info for input*/
	struct v4l2_batch_timestamp batch_ts;
	struct v4l2_frame_rate frame_rate;/* frame rate info for input*/
	struct v4l2_frame_rate operating_rate;/* operating rate info for input*/
	struct v4l2_enc_yhistogram yhistogram;/* luma historgram info for input*/
	struct vui_params vui_color_aspect; /* vui color aspect info*/
};

struct v4l2_vsi_enc_meta_info_CAPTURE {
	__u64 valid; /* bit-mask of valid meta info, refer to v4l2_frame_meta_info_mask */
	//no catual member yet
};

/* imx v4l2 formats */
/*raw formats*/
/* 400 10 bit accupies 16 bit, data in the 10 high bits[15:6],
 with padding zeros in the 4 low bits[5:0]. */
#define V4L2_PIX_FMT_Y010		v4l2_fourcc('Y', '0', '1', '0')
#define V4L2_PIX_FMT_BGR565		v4l2_fourcc('B', 'G', 'R', 'P') /* 16  BGR-5-6-5     */
#define V4L2_PIX_FMT_NV12X			v4l2_fourcc('N', 'V', 'X', '2') /* Y/CbCr 4:2:0 for 10bit  */
#define V4L2_PIX_FMT_DTRC			v4l2_fourcc('D', 'T', 'R', 'C') /* 8bit tile output, uncompressed */
#define V4L2_PIX_FMT_P010			v4l2_fourcc('P', '0', '1', '0')	/*ms p010, data stored in upper 10 bits of 16 */
#define V4L2_PIX_FMT_TILEX			v4l2_fourcc('D', 'T', 'R', 'X') /* 10 bit tile output, uncompressed */
#define V4L2_PIX_FMT_RFC			v4l2_fourcc('R', 'F', 'C', '0') /* 8bit tile output, with rfc*/
#define V4L2_PIX_FMT_RFCX			v4l2_fourcc('R', 'F', 'C', 'X') /* 10 bit tile output, with rfc */
#define V4L2_PIX_FMT_411SP			v4l2_fourcc('4', '1', 'S', 'P') /* YUV 411 Semi planar */
#define V4L2_PIX_FMT_P010M			v4l2_fourcc('P', 'M', '1', '0') /* p010, 2 mem planes. */
#ifndef	V4L2_PIX_FMT_NV12_4L4
#define V4L2_PIX_FMT_NV12_4L4		v4l2_fourcc('V', 'T', '1', '2') /* 12 Y/CbCr 4:2:0 4x4 tiles */
#endif
#ifndef	V4L2_PIX_FMT_P010_4L4
#define V4L2_PIX_FMT_P010_4L4		v4l2_fourcc('T', '0', '1', '0') /* 12 Y/CbCr 4:2:0 10-bit 4x4 macroblocks */
#endif
#ifndef V4L2_PIX_FMT_RGBA1010102
#define V4L2_PIX_FMT_RGBA1010102 v4l2_fourcc('R', 'A', '3', '0') /* 32  RGBA-10-10-10-2 */
#endif
#ifndef V4L2_PIX_FMT_ARGB2101010
#define V4L2_PIX_FMT_ARGB2101010 v4l2_fourcc('A', 'R', '3', '0') /* 32  ARGB-2-10-10-10 */
#endif
#ifndef V4L2_PIX_FMT_ABGR2101010
#define V4L2_PIX_FMT_ABGR2101010 v4l2_fourcc('A', 'B', '3', '0') /* 32  ABGR-2-10-10-10 */
#endif
#ifndef V4L2_PIX_FMT_BGRA1010102
#define V4L2_PIX_FMT_BGRA1010102 v4l2_fourcc('B', 'A', '3', '0') /* 32  BGRA-10-10-10-2 */
#endif
/*codec format*/
#define V4L2_PIX_FMT_AV1			v4l2_fourcc('A', 'V', '1', '0')	/* av1 */
#define V4L2_PIX_FMT_RV				v4l2_fourcc('R', 'V', '0', '0')	/* rv */
#define V4L2_PIX_FMT_AVS			v4l2_fourcc('A', 'V', 'S', '0')	/* avs */
#define V4L2_PIX_FMT_AVS2			v4l2_fourcc('A', 'V', 'S', '2')	/* avs2 */
#define V4L2_PIX_FMT_VP6			v4l2_fourcc('V', 'P', '6', '0') /* vp6 */
#define V4L2_PIX_FMT_VP7			v4l2_fourcc('V', 'P', '7', '0') /* vp7 */
#define V4L2_PIX_FMT_WEBP			v4l2_fourcc('W', 'E', 'B', 'P') /* webp */
#define V4L2_PIX_FMT_DIVX			v4l2_fourcc('D', 'I', 'V', 'X') /* DIVX */
#define V4L2_PIX_FMT_SORENSON		v4l2_fourcc('S', 'R', 'S', 'N') /* sorenson */
/*codec formats*/
#endif	//#ifndef _UAPI__LINUX_IMX_VPU_H

