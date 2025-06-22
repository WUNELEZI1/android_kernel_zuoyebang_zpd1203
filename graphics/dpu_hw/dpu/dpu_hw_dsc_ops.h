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

#ifndef _DPU_HW_DSC_OPS_H_
#define _DPU_HW_DSC_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_hw_dsc_cfg.h"

#define DSC_VERSION_V11                0x11
#define DSC_VERSION_V12A               0x12
#define DSC_VERSION_MAJOR_V1           0x1
#define DSC_VERSION_MINOR_V1           0x1
#define DSC_VERSION_MINOR_V2A          0x2

/* bits per component */
enum dsc_bpc {
	DSC_INPUT_8BPC  = 8,
	DSC_INPUT_10BPC = 10,
	DSC_INPUT_12BPC = 12,
	DSC_INPUT_14BPC = 14, /* Valid MNR_VRE = 2 */
	DSC_INPUT_16BPC = 16, /* Valid MNR_VRE = 2 */
	DSC_INPUT_MAX
};

/**
 * dsc_parms - dsc parameters
 * @dsc_en: is dsc enabl
 * @dsc_version_major: the dsc version info
 * @dsc_version_minor: the dsc version info
 * @pic_height: picture height
 * @pic_width: picture width
 * @slice_height: slice height
 * @slice_width: slice width
 * @bits_per_component: bits per component
 * @bits_per_pixel: bpp after dsc compression
 * @customized_pps_table: the customized pps table. pps table calculation will be by passed
 * if this customized pps table is setted.
 */
struct dsc_parms {
	bool dsc_en;

	u8 dsc_version_major;
	u8 dsc_version_minor;

	u32 pic_height;
	u32 pic_width;

	u32 slice_height;
	u32 slice_width;

	u8 bits_per_component;
	u8 bits_per_pixel;
	u8 *customized_pps_table;
};

#define SINK_DSC_CAP_BPCS_SIZE                     3
#define SINK_DSC_CAP_SLICE_COUNT_SIZE              10

/* the dsc parameters of sink device */
struct dsc_caps_sink {
	bool dsc_supported;
	u16 rate_buffer_size_kbytes;

	u8 dsc_version_minor;
	u8 dsc_version_major;

	u32 max_slice_width;
	/* supported slice_count is stored in @slice_counts in descending order */
	u8 slice_counts[SINK_DSC_CAP_SLICE_COUNT_SIZE];
	u8 slice_counts_size;

	u8 max_line_buffer_depth;
	bool block_predication_supported;

	u8 rgb_supported;

	/* supported bpc is stored in @bpcs in descending order */
	u8 bpcs[SINK_DSC_CAP_BPCS_SIZE];
	u8 bpcs_size;

	/* max decoding pixel rate per second for 4:4:4, in unit of MP/s */
	u32 throughput_0_MP;
};

#define SRC_DSC_CAP_BPCS_SIZE                     5

/**
 * source dsc parameters provide DSC module
 * it is the hardware restrictions and required to compared with dsc_params
 */
struct dsc_caps_src {
	u8 dsc_version_minor;
	u8 dsc_version_major;

	u32 max_line_buffer_depth;
	u32 max_pic_height;
	u32 max_pic_width;
	u32 max_slice_height;
	u32 max_slice_width;
	u8 max_slice_cnt;

	u8 native_422_supported;
	u8 native_420_supported;
	/* supported bpc is stored in @bpcs in descending order */
	u8 bpcs[SRC_DSC_CAP_BPCS_SIZE];
	u8 bpcs_size;
	u8 var_bit_rate_supported;
	u8 block_predication_supported;

	u16 rate_buffer_size_bytes;
};

void dpu_hw_dsc_enable(struct dpu_hw_blk *hw, struct dpu_dsc_config *dsc_config);
void dpu_hw_dsc_disable(struct dpu_hw_blk *hw);
void dpu_hw_dsc_get_caps(struct dpu_hw_blk *hw, struct dsc_caps_src *caps_src);
void dpu_hw_dsc_height_set(struct dpu_hw_blk *hw, u16 height, bool dual_port);

void dpu_hw_dsc_convert_to_pps(struct dpu_dsc_config *dsc_cfg,
		u8 pps[DSC_PPS_TABLE_BYTES_SIZE]);

#endif /* _DPU_HW_DSC_OPS_H_ */
