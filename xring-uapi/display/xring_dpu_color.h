/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __XRING_DPU_COLOR_H__
#define __XRING_DPU_COLOR_H__

#ifndef CONFIG_UEFI_DISPLAY
#include <linux/types.h>
#endif

/* borrowed from linux/include/linux/compiler-gcc3.h */
#ifndef __aligned
#define __aligned(x)			__attribute__((aligned(x)))
#endif

struct dpu_pixel_location {
	__u16 x;
	__u16 y;
} __aligned(8);

struct dpu_region {
	__u16 x;
	__u16 y;
	__u16 w;
	__u16 h;
} __aligned(8);

#define GAMMA_CURVE_SIZE 257

/**
 * dpu_gamma_cfg - the gamma data structure
 * @r: r component lut
 * @b: b component lut
 * @g: g component lut
 */
struct dpu_gamma_cfg {
	__u16 r[GAMMA_CURVE_SIZE];
	__u16 b[GAMMA_CURVE_SIZE];
	__u16 g[GAMMA_CURVE_SIZE];
} __aligned(8);

#define LUT_3D_SIZE 15606 // 18x17x17x3
/**
 * dpu_lut_3d_cfg - 3dlut register table
 * @lut_3d: 3dlut data
 */
struct dpu_lut_3d_cfg {
	__u16 lut_3d[LUT_3D_SIZE];
} __aligned(8);

#define DITHER_BAYER_MAP_SIZE 64
/**
 * dither_algo_mode - dither algorithm
 * 0: error diffusion, only dither_out_depth needed
 * 1: order pattern, all params needed
 */
enum dither_algo_mode {
	DITHER_MODE_ARM,
	DITHER_MODE_PATTERN,
};

/**
 * dpu_dither_cfg - dpu dither config data
 * @mode: dither handle algorithm
 * @dither_out_depthx: bit num after transfer
 * @bayermap: matrix used by order_pattern mode
 * @patternbits: max bit width in bayermap
 * @temporalvalue: pixel offset value
 * @rotatemode: rotate bayermap every 4 frame
 * @autotemp: auto calculate temporal value
 */
struct dpu_dither_cfg {
	enum dither_algo_mode mode;
	__u8 dither_out_depth0;
	__u8 dither_out_depth1;
	__u8 dither_out_depth2;
	__u8 bayermap[DITHER_BAYER_MAP_SIZE];
	__u8 patternbits;
	__u8 temporalvalue;
	__u8 rotatemode;
	__u8 autotemp;
} __aligned(8);

#define CSC_MATRIX_SIZE 12
/**
 * dpu_csc_matrix_cfg - r2y/y2r matrix
 * @matrix: matrix table data, size: 3x4
 */


struct dpu_csc_matrix_cfg{
	__s16 matrix[CSC_MATRIX_SIZE];
} __aligned(8);


#define POST_MATRIX_3X3_SIZE 9
/**
 * dpu_post_matrix_3x3_cfg - csc matrix in dpp
 * @matrix: matrix table data, size: 3x3
 */
struct dpu_post_matrix_3x3_cfg {
	__u16 matrix[POST_MATRIX_3X3_SIZE];
} __aligned(8);

enum dpu_tm_transfer_mode {
	TM_PQ = 0,
	TM_HLG,
	TM_SRGB,
	TM_BT709,
	TM_GAMMA22,
	TM_GAMMA26,
};

#define TM_TRANSFER_LUT_SIZE 71
/**
 * dpu_tm_transfer - tm gamma transfer mode
 * @eotf_mode:eotf mode
 * @oetf_mode:oetf mode
 * @oetf_max: value to clip the oetf curve
 * @eotf_tbl_size: to modify lut4 table size
 * @oetf_tbl_size: to modify lut5 table size
 * @eotf_user_tbl: tbl which set at eotf/oetf mode 4
 * @oetf_user_tbl: tbl which set at eotf/oetf mode 5
 */
struct dpu_tm_transfer {
	enum dpu_tm_transfer_mode eotf_mode;
	enum dpu_tm_transfer_mode oetf_mode;
	__u16 oetf_max;
	__u8 eotf_tbl_size;
	__u8 oetf_tbl_size;
	__u32 eotf_user_tbl[TM_TRANSFER_LUT_SIZE];
	__u32 oetf_user_tbl[TM_TRANSFER_LUT_SIZE];
} __aligned(8);

enum dpu_tm_ootf_rgb_mode {
	TM_OOTF_RGB_MODE_ONLY_Y = 0,
	TM_OOTF_RGB_MODE_MAX_XYZ,
	TM_OOTF_RGB_MODE_SEPRATE,
};

#define TM_MATRIX_SIZE 9
#define TM_MATRIX_OFFSET_SIZE 3
struct dpu_tm_matrix {
	__s16 matrix[TM_MATRIX_SIZE];
	__s32 offset_tbl[TM_MATRIX_OFFSET_SIZE];
} __aligned(8);

#define END_OOTF_GAIN_TABLE_SIZE_V1 257
#define END_OOTF_GAIN_TABLE_SIZE 65
#define END_OOTF_WEIGHT_Y_TABLE_SIZE 3
struct dpu_tm_end_ootf {
	__u16 gain_tbl[END_OOTF_GAIN_TABLE_SIZE_V1];
	__u16 tbl_length;
	__u8 shift_bits;
	enum dpu_tm_ootf_rgb_mode rgb_mode;
	__u16 y_weight[END_OOTF_WEIGHT_Y_TABLE_SIZE];
} __aligned(8);

/**
 * dpu_tm_cfg - prepq tm config
 * @transfer_en: enable/disable eotf and oetf
 * @transfer: set eotf and oetf mode, oetf_max and lut table
 * @gain_to_full_en: enable/disable gain_to_full
 * @gain_to_full: set gain_to_full value
 * @matrix_en: enable/disable matrix
 * @matrix: set matrix table, size: 3x4
 * @end_ootf_en: enable/disable end ootf
 * @end_ootf: set end ootf config
 */
struct dpu_tm_cfg {
	bool transfer_en;
	struct dpu_tm_transfer transfer;
	bool gain_to_full_en;
	__u16 gain_to_full;
	bool matrix_en;
	struct dpu_tm_matrix matrix;
	bool end_ootf_en;
	struct dpu_tm_end_ootf end_ootf;
} __aligned(8);

#define POST_MATRIX_3X4_SIZE 9
#define POST_MATRIX_3X4_OFFSET_SIZE 3
struct dpu_post_matrix_3x4_cfg {
	__s32 matrix[POST_MATRIX_3X4_SIZE];
	__s32 offset_tbl[POST_MATRIX_3X4_OFFSET_SIZE];
} __aligned(8);

/**
 * dpu_ltm_cfg - ltm gamma transfer mode
 * same with dpu_hw/color/dpu_hw_color_api.h
 * @eotf_mode:eotf mode
 * @oetf_mode:oetf mode
 * @oetf_max: value to clip the oetf curve
 * @eotf_tbl_size: to judge lut4 table size
 * @oetf_tbl_size: to judge lut5 table size
 * @eotf_user_tbl: tbl which set at eotf/oetf mode 4
 * @oetf_user_tbl: tbl which set at eotf/oetf mode 5
 */
struct dpu_ltm_cfg {
	enum dpu_tm_transfer_mode eotf_mode;
	enum dpu_tm_transfer_mode oetf_mode;
	__u16 oetf_max;
	__u8 eotf_tbl_size;
	__u8 oetf_tbl_size;
	__u32 eotf_user_tbl[TM_TRANSFER_LUT_SIZE];
	__u32 oetf_user_tbl[TM_TRANSFER_LUT_SIZE];
} __aligned(8);

enum dpu_acad_hist_mode {
	ACAD_HIST_MODE_SUBSAMPLE,
	ACAD_HIST_MODE_AVERAGE,
};

enum dpu_acad_curve_mode {
	ACAD_CURVE_MODE_LIMIT,
	ACAD_CURVE_MODE_NORM,
};

enum dpu_acad_tm_mode {
	ACAD_TM_MODE_MANUAL,
	ACAD_TM_MODE_AUTO,
};

/**
 * dpu_acad_tm - acad enhanced tone mapping mode
 * @tm_mode: mode for enahanced tone mapping mode
 * @alpha_step: steps need for update
 * @skip_frames: skip frames to update
 */
struct dpu_acad_tm {
	enum dpu_acad_tm_mode tm_mode;
	__u8 alpha_step;
	__u8 skip_frames;
} __aligned(8);

/**
 * dpu_acad_internel_blk -acad hist block config
 * @blk_width: acad hist block width
 * @blk_height: acad hist block height
 * @blk_num_hori: acad hist block horizontal number
 * @blk_num_vert: acad hist block vertical number
 * @weight_threshold_x: acad weight x threshold
 * @weight_threshold_y: acad weight y threshold
 */
struct dpu_acad_internel_blk {
	__u16 blk_width;
	__u16 blk_height;
	__u8 blk_num_hori;
	__u8 blk_num_vert;
	__u16 weight_threshold_x;
	__u16 weight_threshold_y;
} __aligned(8);

#define CONTRAST_CURVE_NUM 4
#define CONTRAST_CURVE_SIZE 65
/**
 * dpu_acad_frame_curve - acad contrast curve data
 * @contrast_curve: four acad contrast curves, size: 4x64
 */
struct dpu_acad_frame_curve {
	__u16 contrast_curve[CONTRAST_CURVE_NUM][CONTRAST_CURVE_SIZE];
} __aligned(8);

#define SUB_ROI_AREA_SUPPORT_MAX 4
/**
 * dpu_acad_cfg - acad config
 * @curve_alpha: blending coefficient of current and previous frame
 * @dst_alpha: destination intensity
 * @tp_alpha: blending ratio of rgb and y channel
 * @internel_blk: acad hist block config
 * @hist_mode: acad hist sampling mode
 * @curve_mode: acad contrast curve mode
 * @tm: acad enhanced tone mapping mode
 * @sub_roi: acad sub roi region
 * @roi: acad roi region
 * @contrast_curve: acad contrast curve
 * @segment_threshold: four sections' threshold of contrast curve
 */
struct dpu_acad_cfg {
	__u16 curve_alpha;
	__u8 dst_alpha;
	__u16 tp_alpha;
	struct dpu_acad_internel_blk internel_blk;
	enum dpu_acad_hist_mode hist_mode;
	enum dpu_acad_curve_mode curve_mode;
	struct dpu_acad_tm tm;
	struct dpu_region sub_roi[SUB_ROI_AREA_SUPPORT_MAX];
	struct dpu_region roi;
	struct dpu_acad_frame_curve contrast_curve;
	__u8 segment_threshold[CONTRAST_CURVE_NUM + 1];
} __aligned(8);

struct dpu_hist_region {
	__u16 x_begin;
	__u16 x_end;
	__u16 y_begin;
	__u16 y_end;
} __aligned(8);

enum dpu_hist_bit_type {
	HIST_BIT_TYPE_10,
	HIST_BIT_TYPE_8,
	HIST_BIT_TYPE_6,
};
enum dpu_hist_chnl_type {
	HIST_CHNL_TYPE_SINGLE,
	HIST_CHNL_TYPE_FOUR,
};

enum dpu_hist_calc_mode {
	HIST_CALC_MODE_MAXRGB,
	HIST_CALC_MODE_WEIGHT,
};

enum dpu_hist_event_read_mode {
	HIST_READ_DISABLE = 0,
	HIST_READ_ALWAYS,
	HIST_READ_ONCE,
	HIST_READ_MODE_MAX = HIST_READ_ONCE,
};

enum dpu_hist_update_mode {
	HIST_UPDATE_EACH_FRAME,
	HIST_UPDATE_WHEN_READ_DONE,
};

#define RGB_CHNL_NUM 3
/**
 * dpu_hist_mode_cfg - histogram calculate mode config
 * @hist_chnl: histogram calculate with one/four channels
 * @hist_cal_mode: the first/forth channel calculat way
 * @hist_cal_weight: r/g/b weight used at weight mode
 */
struct dpu_hist_mode {
	enum dpu_hist_chnl_type hist_chnl;
	enum dpu_hist_calc_mode hist_cal_mode;
	__u16 hist_cal_weight[RGB_CHNL_NUM];
} __aligned(8);

#define HIST_DATA_SIZE 256
/**
 * dpu_hist_data - histogram collected data
 * @data: histogram collected data, size: 256
 */
struct dpu_hist_data {
	__u32 data[HIST_DATA_SIZE];
} __aligned(8);

/**
 * dpu_hist_cfg - histogram config
 * @hist_pos: postpq 0: after mixer, 1: after dpp, prepq 0: after r2y, prepq 1: after tm
 * @read_mode: 0:update each frame, 1: update when read_done 1
 * @read_done: update the collected data to read_bin when mode 1
 * @hist_input_bit: input bit choice, currently only use after dpp
 * @roi_enable: enable/disable roi
 * @roi_region: roi region config
 * @blk_enable: enable/disable blk
 * @blk_region: blk region config
 * @roi_se_enable: enable/disable roi_se
 * @roi_se_region: roi_se region config
 * @mode_cfg: calculate mode config
 * @hist_left: choose the left/right channel layer, should be 1 in postpq
 */
struct dpu_hist_cfg {
	__u8 hist_pos;
	enum dpu_hist_update_mode read_mode;
	enum dpu_hist_bit_type hist_input_bit;
	bool roi_enable;
	struct dpu_hist_region roi_region;
	bool blk_enable;
	struct dpu_hist_region blk_region;
	bool roi_se_enable;
	struct dpu_hist_region roi_se_region;
	struct dpu_hist_mode mode_cfg;
	bool hist_left; // only prepq use
} __aligned(8);

struct dpu_none_prealpha_offset {
	__u16 offset0;
	__u16 offset1;
	__u16 offset2;
} __aligned(8);

enum dpu_scaler_interp_mode {
	SCALER_2D_LANCZOS3 = 0,
	SCALER_2D_EDGE_DIRECTED = 1,
};

#define SCALER_2D_KERNEL_LUT_NUM 129
struct dpu_2d_scaler_lanczos3_kernel {
	__s16 lanczos3_kernel_0_5[SCALER_2D_KERNEL_LUT_NUM];
	__s16 lanczos3_kernel_1_4[SCALER_2D_KERNEL_LUT_NUM];
	__u16 lanczos3_kernel_2_3[SCALER_2D_KERNEL_LUT_NUM];
} __aligned(8);

struct dpu_2d_scaler_bicubic_kernel {
	__s16 bicubic_kernel_0_3[SCALER_2D_KERNEL_LUT_NUM];
	__s16 bicubic_kernel_1_2[SCALER_2D_KERNEL_LUT_NUM];
} __aligned(8);

#define GAUSS_KERNEL_NUM_5X5 6
#define GAUSS_KERNEL_NUM_9X9 15
/* dpu_2d_scaler_gauss_kernel - 2d scaler gauss kernel for detail enhance feature
 * @gauss_kernel_5x5: 5x5 gauss kernel
 * @gauss_kernel_9x9: 9x9 gauss kernel
 */
struct dpu_2d_scaler_gauss_kernel {
	__u16 gauss_kernel_5x5[GAUSS_KERNEL_NUM_5X5];
	__u16 gauss_kernel_9x9[GAUSS_KERNEL_NUM_9X9];
} __aligned(8);

/**
 * dpu_2d_scaler_cfg - 2d scaler config
 * @alpha_disable: enable/disable alpha component
 * @detail_enhance_enable: enable/disable detail enhance feature
 * @outlier_remove_enable: enable/disable outlier point remove feature
 * @interp_method: interpoloation mode
 * @stripe_height_in: height of the input stripe image
 * @stripe_width_in: width of the input stripe image
 * @stripe_height_out: height of the output stripe image
 * @stripe_width_out: width of the output stripe image
 * @x_step: height direction's mapping step
 * @y_step: width direction's mapping step
 * @base_threshold: base layer's luma threshold
 * @sigmoid_val0: diff0 layer's sigmoid mapping coefficient
 * @sigmoid_val1: diff1 layer's sigmoid mapping coefficient
 * @sigmoid_val2: base layer's sigmoid mapping coefficient
 * @outlier_threshold1: outlier remove's threshold 1
 * @outlier_threshold2: outlier remove's threshold 2
 * @threshold_number: outlier point remove's threshold number
 * @gradient_threshold: gradient check's threshold
 * @stripe_init_phase: stripe image's initial phase value
 * @crop_width_out_phase: output width direction's crop pix phase value
 * @crop_height_out_phase: output height direction's crop pix phase value
 * @gauss_kernel: gauss filter's coefficient for detail enhance
 * @lanczos3_kernel: SCALER_2D_MODE_LANCZOS3 interpolation kernel
 * @bicubic_kernel: BICUBIC interpolation kernel
 */
struct dpu_2d_scaler_cfg {
	bool alpha_disable;
	bool detail_enhance_enable;
	bool outlier_remove_enable;
	enum dpu_scaler_interp_mode interp_method;
	__u16 stripe_height_in;
	__u16 stripe_width_in;
	__u16 stripe_height_out;
	__u16 stripe_width_out;
	__u32 y_step;
	__u32 x_step;
	__u32 base_threshold;
	__u8 sigmoid_val0;
	__u8 sigmoid_val1;
	__u8 sigmoid_val2;
	__u8 outlier_threshold1;
	__u8 outlier_threshold2;
	__u8 threshold_number;
	__u16 gradient_threshold;
	__u32 stripe_init_phase;
	__u32 crop_width_out_phase;
	__u32 crop_height_out_phase;
	struct dpu_2d_scaler_gauss_kernel gauss_kernel;
	struct dpu_2d_scaler_lanczos3_kernel lanczos3_kernel;
	struct dpu_2d_scaler_bicubic_kernel bicubic_kernel;
} __aligned(8);

#define SR_8_TAP_COEF_SIZE 112
#define SR_6_TAP_COEF_SIZE 80
/**
 * dpu_1d_scaler_cfg - 1d scaler config
 * @input_height: the input image height of 1d scaler
 * @input_width: the input image width of 1d scaler
 * @output_height: the output image height of 1d scaler
 * @output_width: the output image widht of 1d scaler
 * @hor_init_phase_l32b: the horizontal init phase of lowe 32 bit
 * @hor_init_phase_h1b: the horizontal init phase of sign bit
 * @ver_init_phase_l32b: the vertical init phase of lowe 32 bit
 * @ver_init_phase_h1b: the vertical init phase of sign bit
 * @hor_delta_phase: the horizontal initial phase
 * @ver_delta_phase: the vertical initial phase
 * @hor_coef: horizontal direction's interpolation coefficient
 * @ver_coef: vertical direction's interpolation coefficient
 */
struct dpu_1d_scaler_cfg {
	__u32 input_height;
	__u32 input_width;
	__u32 output_height;
	__u32 output_width;
	__u32 hor_init_phase_l32b;
	__u32 hor_init_phase_h1b;
	__u32 ver_init_phase_l32b;
	__u32 ver_init_phase_h1b;
	__u32 hor_delta_phase;
	__u32 ver_delta_phase;
	__s32 hor_coef[SR_8_TAP_COEF_SIZE];
	__s32 ver_coef[SR_6_TAP_COEF_SIZE];
} __aligned(8);

#define RC_BS_LEN 2500
/* dpu_rc_cfg - rc configs and stream data
 * @white_mode_en: enable use alpha seperately
 * @work_mode: enable fill color in punchout area
 * @set_val_r: color r in work mode
 * @set_val_g: color g in work mode
 * @set_val_b: color b in work mode
 * @acrp_bs_len: rc stream length
 * @rc_bs: rc stream, max length = 8192
 */
struct dpu_rc_cfg {
	bool white_mode_en;
	bool work_mode;
	__u32 set_val_r;
	__u32 set_val_g;
	__u32 set_val_b;
	__u32 acrp_bs_len;
	__u32 rc_bs[RC_BS_LEN];
} __aligned(8);

enum dp_eotf {
	DP_EOTF_TRADITIONAL_GAMMA_SDR = 1 << 0,
	DP_EOTF_TRADITIONAL_GAMMA_HDR = 1 << 1,
	DP_EOTF_PQ = 1 << 2,
	DP_EOTF_HLG = 1 << 3,
};

enum static_metadata_type {
	STATIC_METADATA_TYPE_1 = 1 << 0,
};

enum dp_colorimetry_space {
	DP_COLORIMETRY_BT601 = 1 << 0,
	DP_COLORIMETRY_BT709 = 1 << 1,
	DP_COLORIMETRY_BT2020 = 1 << 2,
	DP_COLORIMETRY_ADOBE_RGB = 1 << 3,
	DP_COLORIMETRY_DCIP3 = 1 << 4,
};

/**
 * struct dp_hdr_caps_sink - the hdr capabilities of sink
 *
 * @caps_valid: if sink have cea extension edid block and medadata data block
 * @eotf_supported: eotf supported by sink
 * @static_metadata_type_supported: static metadata type supported by sink,
 *     according to CTA-861-G Table 86 Supported Static Metadata Descriptor.
 *     currently the value only can be 1
 * @max_cll: sink desired max content luminance code value,
 *       actual value = 50 * 2^(code_value / 32)
 * @max_fall: sink desired max frame-average luminance code value,
 *       actual value = 50 * 2^(code_value / 32)
 * @min_cll: sink desired min content luminance code value,
 *       actual value = max_cll * (code_value / 255)^2 / 100
 * @hdr_dolby_supported: if sink supports hdr dolby
 * @hdr_dolby_metadata_version: hdr dolby metadata version supported by sink
 * @hdr_dolby_supported: if sink supports hdr10 plus
 * @hdr_dolby_metadata_version: hdr10 plus metadata version supported by sink
 * @colorimetry_supported: sink supported colorimetry
 * @max_metadata_size: supported maximum metadata size, in units of byte
 */
struct dp_hdr_caps_sink {
	bool caps_valid;
	__u8 eotf_supported;
	__u8 static_metadata_type_supported;

	__u8 max_cll;
	__u8 max_fall;
	__u8 min_cll;

	bool hdr_dolby_supported;
	__u8 hdr_dolby_metadata_version;
	bool hdr10_plus_supported;
	__u8 hdr10_plus_metadata_version;

	__u8 colorimetry_supported;
	__u32 max_metadata_size;
} __aligned(8);

/**
 * struct dp_hdr_static_metadata - hdr static metadata
 * @eotf: eotf
 * @static_metadata_type: static metadata type
 * @display_primaries_red_x: x-axis coordinate of red in color gamut,
 *     in units of 0.00002
 * @display_primaries_red_y: y-axis coordinate of red in color gamut,
 *     in units of 0.00002
 * @display_primaries_green_x: x-axis coordinate of green in color gamut,
 *     in units of 0.00002
 * @display_primaries_green_y: y-axis coordinate of green in color gamut,
 *     in units of 0.00002
 * @display_primaries_blue_x: x-axis coordinate of blue in color gamut,
 *     in units of 0.00002
 * @display_primaries_blue_y: y-axis coordinate of blue in color gamut,
 *     in units of 0.00002
 * @white_point_x: x-axis coordinate of white point in color gamut,
 *     in units of 0.00002
 * @white_point_y: y-axis coordinate of white point in color gamut,
 *     in units of 0.00002
 * @max_display_luminance: max display mastering luminance, in units of 1 cd/㎡
 * @min_display_luminance: min display mastering luminance,
 *     in units of 0.0001 cd/㎡
 * @max_cll: max content light level, in units of 1 cd/㎡
 * @max_fall: max frame-average light level, in units of 1 cd/㎡
 */
struct dp_hdr_static_metadata {
	enum dp_eotf eotf;
	enum static_metadata_type static_metadata_type;

	__u16 display_primaries_red_x;
	__u16 display_primaries_red_y;
	__u16 display_primaries_green_x;
	__u16 display_primaries_green_y;
	__u16 display_primaries_blue_x;
	__u16 display_primaries_blue_y;
	__u16 white_point_x;
	__u16 white_point_y;
	__u16 max_display_luminance;
	__u16 min_display_luminance;
	__u16 max_cll;
	__u16 max_fall;
} __aligned(8);

#define DP_MAX_METADATA_SIZE 128

/**
 * struct dp_hdr_dynamic_metadta - hdr dynamic metadata
 *
 * @data: payload of dynamic metadata
 * @size: size of payload
 */
struct dp_hdr_dynamic_metadata {
	__u8 data[DP_MAX_METADATA_SIZE];
	__u32 size;
} __aligned(8);

enum dp_metadata_type {
	DP_STATIC_METADATA = 0,
	DP_DYNAMIC_METADATA_HDR10_PLUS = 1,
};

/**
 * struct dp_hdr_metadata - dp hdr metadata
 *
 * @type: indicate static or dynamic metadata
 * @buffer: static or dynamic metadata pointer of userspace
 * @size: static or dynamic metadata size
 */
struct dp_hdr_metadata {
	enum dp_colorimetry_space colorimetry;
	enum dp_metadata_type type;
	void *buffer;
	__u32 size;
} __aligned(8);

#define COLOR_PRIMATIES_COORDS 8
/**
 * dsi_panel_hdr_caps -hdr info of the screen
 * @hdr_enabled: indicate HDR support in panel
 * @color_primaties: w/r/g/b coordinates
 * @peak_brightness: the panel peak brightness
 * @blackness_level: the panel blackness level
 */
struct dsi_panel_hdr_caps {
	bool hdr_enabled;
	__u32 color_primaries[COLOR_PRIMATIES_COORDS];
	__u32 peak_brightness;
	__u32 blackness_level;
} __aligned(8);

#define MAX_DSI_PANEL_NAME_LENGTH 128
/**
 * dsi_vrr_type - Variable Refresh Rate support type
 * @DSI_VRR_NONE: VRR is not supported.
 * @DSI_VRR_LONG_H:
 * @DSI_VRR_LONG_V:
 * @DSI_VRR_SKIP_FRAME:
 */
enum dsi_vrr_type {
	DSI_VRR_NONE = 0,
	DSI_VRR_LONG_H,
	DSI_VRR_LONG_V,
	DSI_VRR_SKIP_FRAME,
} __aligned(8);

struct dsi_panel_vrr_caps {
	enum dsi_vrr_type vrr_type;
	__u16 frame_rate;
	__u16 h_skew;
	__u16 te_rate;
	__u16 base_rate;
} __aligned(8);

#define DSI_MODE_MAX 32
/**
 * dsi_panel_caps - capabilities of the panel
 * @hdr_caps: hdr capabilities of the screen
 * @name: the panel name
 * @min_brightness: the panel min brightness
 * @init_brightness: the init brightness before first frame
 * @aod_mode: the panel support always on display mode
 */
struct dsi_panel_caps {
	struct dsi_panel_hdr_caps hdr_caps;
	char name[MAX_DSI_PANEL_NAME_LENGTH];
	__u32 min_brightness;
	__u32 init_brightness;
	__u32 aod_mode;
	struct dsi_panel_vrr_caps vrr_caps[DSI_MODE_MAX];
	__u32 num_modes;
} __aligned(8);

#define AOD_MODE_DOZE_MASK (1 << 0)
#define AOD_MODE_SUSPEND_MASK (1 << 1)

#endif /* __XRING_DPU_COLOR_H__ */
