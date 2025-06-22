/* SPDX-License-Identifier: MIT
 * Copyright (C) 2018 Intel Corp.
 *
 * Authors:
 * Manasi Navare <manasi.d.navare@intel.com>
 */

#ifndef _DPU_HW_DSC_CONFIG_H_
#define _DPU_HW_DSC_CONFIG_H_

#include "dpu_osal.h"

#define DSC_PPS_TABLE_BYTES_SIZE            128

/* VESA Display Stream Compression DSC 1.2 constants */
#define DSC_NUM_BUF_RANGES                  15
#define DSC_MUX_WORD_SIZE_8_10_BPC          48
#define DSC_MUX_WORD_SIZE_12_BPC            64
#define DSC_RC_PIXELS_PER_GROUP             3
#define DSC_SCALE_DECREMENT_INTERVAL_MAX    4095
#define DSC_RANGE_BPG_OFFSET_MASK           0x3f

/* DSC Rate Control Constants */
#define DSC_RC_MODEL_SIZE_CONST             8192
#define DSC_RC_EDGE_FACTOR_CONST            6
#define DSC_RC_TGT_OFFSET_HI_CONST          3
#define DSC_RC_TGT_OFFSET_LO_CONST          3

/* DSC PPS constants and macros */
#define DSC_PPS_VERSION_MAJOR_SHIFT         4
#define DSC_PPS_BPC_SHIFT                   4
#define DSC_PPS_MSB_SHIFT                   8
#define DSC_PPS_LSB_MASK                    (0xFF << 0)
#define DSC_PPS_BPP_HIGH_MASK               (0x3 << 8)
#define DSC_PPS_VBR_EN_SHIFT                2
#define DSC_PPS_SIMPLE422_SHIFT             3
#define DSC_PPS_CONVERT_RGB_SHIFT           4
#define DSC_PPS_BLOCK_PRED_EN_SHIFT         5
#define DSC_PPS_INIT_XMIT_DELAY_HIGH_MASK   (0x3 << 8)
#define DSC_PPS_SCALE_DEC_INT_HIGH_MASK     (0xF << 8)
#define DSC_PPS_RC_TGT_OFFSET_HI_SHIFT      4
#define DSC_PPS_RC_RANGE_MINQP_LEN          5
#define DSC_PPS_RC_RANGE_MINQP_SHIFT        11
#define DSC_PPS_RC_RANGE_MAXQP_LEN          5
#define DSC_PPS_RC_RANGE_MAXQP_SHIFT        6
#define DSC_PPS_RC_RANGE_BPG_OFFSET_LEN     6
#define DSC_PPS_RC_RANGE_BPG_OFFSET_SHIFT   0
#define DSC_PPS_NATIVE_420_SHIFT            1
#define DSC_1_2_MAX_LINEBUF_DEPTH_BITS      16
#define DSC_1_2_MAX_LINEBUF_DEPTH_VAL       0
#define DSC_1_1_MAX_LINEBUF_DEPTH_BITS      13
#define DSC_0_PORT1_OFFSET                  0x80
/**
 * struct dpu_dsc_rc_range_parameters - DSC Rate Control range parameters
 *
 * This defines different rate control parameters used by the DSC engine
 * to compress the frame.
 */
struct dpu_dsc_rc_range_parameters {
	/**
	 * @range_min_qp: Min Quantization Parameters allowed for this range
	 */
	u8 range_min_qp;
	/**
	 * @range_max_qp: Max Quantization Parameters allowed for this range
	 */
	u8 range_max_qp;
	/**
	 * @range_bpg_offset:
	 * Bits/group offset to apply to target for this group
	 */
	u8 range_bpg_offset;
};

/**
 * struct dpu_dsc_config - Parameters required to configure DSC
 *
 * Driver populates this structure with all the parameters required
 * to configure the display stream compression on the source.
 */
struct dpu_dsc_config {
	/**
	 * @valid:
	 * if the dsc config is valid
	 */
	bool valid;
	/**
	 * @dual_port:
	 * Dual port dsi panel set 1
	 */
	bool dual_port;
	/**
	 * @line_buf_depth:
	 * Bits per component for previous reconstructed line buffer
	 */
	u8 line_buf_depth;
	/**
	 * @pps_identifier: Application specific identifier that can be
	 * used to differentiate between different PPS tables.
	 */
	u8 pps_identifier;
	/**
	 * @bits_per_component: Bits per component to code (8/10/12)
	 */
	u8 bits_per_component;
	/**
	 * @convert_rgb:
	 * Flag to indicate if RGB - YCoCg conversion is needed
	 * True if RGB input, False if YCoCg input
	 */
	bool convert_rgb;
	/**
	 * @slice_count: Number fo slices per line used by the DSC encoder
	 */
	u8 slice_count;
	/**
	 *  @slice_width: Width of each slice in pixels
	 */
	u16 slice_width;
	/**
	 * @slice_height: Slice height in pixels
	 */
	u16 slice_height;
	/**
	 * @simple_422: True if simple 4_2_2 mode is enabled else False
	 */
	bool simple_422;
	/**
	 * @pic_width: Width of the input display frame in pixels
	 */
	u16 pic_width;
	/**
	 * @pic_height: Vertical height of the input display frame
	 */
	u16 pic_height;
	/**
	 * @rc_tgt_offset_high:
	 * Offset to bits/group used by RC to determine QP adjustment
	 */
	u8 rc_tgt_offset_high;
	/**
	 * @rc_tgt_offset_low:
	 * Offset to bits/group used by RC to determine QP adjustment
	 */
	u8 rc_tgt_offset_low;
	/**
	 * @bits_per_pixel:
	 * Target bits per pixel with 4 fractional bits, bits_per_pixel << 4
	 */
	u16 bits_per_pixel;
	/**
	 * @rc_edge_factor:
	 * Factor to determine if an edge is present based on the bits produced
	 */
	u8 rc_edge_factor;
	/**
	 * @rc_quant_incr_limit1:
	 * Slow down incrementing once the range reaches this value
	 */
	u8 rc_quant_incr_limit1;
	/**
	 * @rc_quant_incr_limit0:
	 * Slow down incrementing once the range reaches this value
	 */
	u8 rc_quant_incr_limit0;
	/**
	 * @initial_xmit_delay:
	 * Number of pixels to delay the initial transmission
	 */
	u16 initial_xmit_delay;
	/**
	 * @initial_dec_delay:
	 * Initial decoder delay, number of pixel times that the decoder
	 * accumulates data in its rate buffer before starting to decode
	 * and output pixels.
	 */
	u16 initial_dec_delay;
	/**
	 * @block_pred_enable:
	 * True if block prediction is used to code any groups within the
	 * picture. False if BP not used
	 */
	bool block_pred_enable;
	/**
	 * @first_line_bpg_offset:
	 * Number of additional bits allocated for each group on the first
	 * line of slice.
	 */
	u8 first_line_bpg_offset;
	/**
	 * @initial_offset: Value to use for RC model offset at slice start
	 */
	u16 initial_offset;
	/**
	 * @rc_buf_thresh: Thresholds defining each of the buffer ranges
	 */
	u16 rc_buf_thresh[DSC_NUM_BUF_RANGES - 1];
	/**
	 * @rc_range_params:
	 * Parameters for each of the RC ranges defined in
	 * &struct dpu_dsc_rc_range_parameters
	 */
	struct dpu_dsc_rc_range_parameters rc_range_params[DSC_NUM_BUF_RANGES];
	/**
	 * @rc_model_size: Total size of RC model
	 */
	u16 rc_model_size;
	/**
	 * @flatness_min_qp: Minimum QP where flatness information is sent
	 */
	u8 flatness_min_qp;
	/**
	 * @flatness_max_qp: Maximum QP where flatness information is sent
	 */
	u8 flatness_max_qp;
	/**
	 * @initial_scale_value: Initial value for the scale factor
	 */
	u8 initial_scale_value;
	/**
	 * @scale_decrement_interval:
	 * Specifies number of group times between decrementing the scale factor
	 * at beginning of a slice.
	 */
	u16 scale_decrement_interval;
	/**
	 * @scale_increment_interval:
	 * Number of group times between incrementing the scale factor value
	 * used at the beginning of a slice.
	 */
	u16 scale_increment_interval;
	/**
	 * @nfl_bpg_offset: Non first line BPG offset to be used
	 */
	u16 nfl_bpg_offset;
	/**
	 * @slice_bpg_offset: BPG offset used to enforce slice bit
	 */
	u16 slice_bpg_offset;
	/**
	 * @final_offset: Final RC linear transformation offset value
	 */
	u16 final_offset;
	/**
	 * @vbr_enable: True if VBR mode is enabled, false if disabled
	 */
	bool vbr_enable;
	/**
	 * @mux_word_size: Mux word size (in bits) for SSM mode
	 */
	u8 mux_word_size;
	/**
	 * @slice_chunk_size:
	 * The (max) size in bytes of the "chunks" that are used in slice
	 * multiplexing.
	 */
	u16 slice_chunk_size;
	/**
	 * @rc_bits: Rate control buffer size in bits
	 */
	u16 rc_bits;
	/**
	 * @dsc_version_minor: DSC minor version
	 */
	u8 dsc_version_minor;
	/**
	 * @dsc_version_major: DSC major version
	 */
	u8 dsc_version_major;
	/**
	 * @native_422: True if Native 4:2:2 supported, else false
	 */
	bool native_422;
	/**
	 * @native_420: True if Native 4:2:0 supported else false.
	 */
	bool native_420;
	/**
	 * @second_line_bpg_offset:
	 * Additional bits/grp for seconnd line of slice for native 4:2:0
	 */
	u8 second_line_bpg_offset;
	/**
	 * @nsl_bpg_offset:
	 * Num of bits deallocated for each grp that is not in second line of
	 * slice
	 */
	u16 nsl_bpg_offset;
	/**
	 * @second_line_offset_adj:
	 * Offset adjustment for second line in Native 4:2:0 mode
	 */
	u16 second_line_offset_adj;
	/**
	 * @customized_pps_table: the customized pps table. pps table
	 * calculation will be by passed if this customized pps table is setted.
	 */
	u8 *customized_pps_table;
};

#endif
