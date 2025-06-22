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

#ifndef _DPU_HW_WB_CORE_H_
#define _DPU_HW_WB_CORE_H_

#include "dpu_hw_common.h"
#include "dpu_hw_format.h"
#include "dpu_color_common.h"

#define WB_LAYER_PLANAR_NUM_MAX    2

/**
 * wb_planar_type - the wb planar type;
 * @WB_PLANAR_Y_RGB: the first componet-Y and RGB format.
 * @WB_PLANAR_UV: the second componet-UV of YUV_P2.
 * @WB_PLANAR_MAX: indicate stride sum number.
 */
enum wb_planar_type {
	WB_PLANAR_Y_RGB = 0,
	WB_PLANAR_UV,
	WB_PLANAR_MAX,
};

struct wb_frame_cfg {
	u16 input_width;
	u16 input_height;

	u8 output_format;
	u8 plane_count;
	bool is_yuv;
	bool is_uv_swap;

	bool is_afbc;
	bool is_afbc_32x8;
	bool is_afbc_split;
	bool is_afbc_copy_mode;
	bool is_afbc_default_color;
	bool is_afbc_yuv_transform;
	bool is_afbc_tile_header_mode;

	enum protected_status drm_enable_status;

	/**
	 * @output_original_width: width of output original image
	 * @output_original_height: height of output original image
	 * @output_subframe_rect: rect of output subframe rect
	 */
	u16 output_original_width;
	u16 output_original_height;
	struct dpu_rect_v2 output_subframe_rect;

	u64 wdma_base_addrs[WB_LAYER_PLANAR_NUM_MAX];
	struct dpu_csc_matrix_cfg csc_cfg;
	bool dither_en;
	struct dpu_dither_cfg dither_cfg;
	/**
	 * @raw_format_stride: the stride of RAW image
	 */
	u16 raw_format_stride;

	enum rot_mode frame_rot_mode;
	enum flip_mode frame_flip_mode;

	bool crop0_en;
	bool crop1_en;
	bool crop2_en;
	struct dpu_rect crop0_area;
	struct dpu_rect crop1_area;
	struct dpu_rect crop2_area;

	bool is_offline_mode;
	bool scl_en;

	/**
	 * wb sliceinfor
	 */
	bool slice_header_wren;
	u16 slice_num;
	u16 slice_size;
	u32 sliceinfor_addr_low;
	u16 sliceinfor_addr_high;

	/**
	 * qos value
	 */
	u32 qos;
};

/*
 * rdma_buffer_level - rdma line buffer level infomation
 * @buf_lvl_down0: wb line buffer down level0
 * @buf_lvl_up0: wb line buffer up level0
 * @buf_urgent_down：wb line buffer urgent down level
 * @buf_urgent_up：wb line buffer urgent up level
 */
struct wb_line_buf_level {
	u32 buf_lvl_down0;
	u32 buf_lvl_up0;
	u32 buf_lvl_down1;
	u32 buf_lvl_up1;
	u32 buf_lvl_down2;
	u32 buf_lvl_up2;

	u32 buf_urgent_down;
	u32 buf_urgent_up;
};

/**
 * dpu_hw_wb_cfg_setup - set input and output frame configurations
 * @hw: the wb hardware block pointer
 * @wb_frame_cfg: configurations of input and output frame
 */
void dpu_hw_wb_cfg_setup(struct dpu_hw_blk *hw, struct wb_frame_cfg *cfg);

/**
 * dpu_hw_wb_reset - reset the wb registers
 * @hw: the wb hardware block pointer
 */
void dpu_hw_wb_reset(struct dpu_hw_blk *hw);

/**
 * dpu_hw_wb_core_status_dump - dump wb_core status for debug
 * @hw: the wb_core hardware block pointer
 */
void dpu_hw_wb_core_status_dump(struct dpu_hw_blk *hw);

/**
 * dpu_hw_wb_top_status_dump - dump wb_top status for debug
 * @hw: the wb_top hardware block pointer
 */
void dpu_hw_wb_top_status_dump(struct dpu_hw_blk *hw);

/**
 * dpu_hw_wb_top_status_clear - clear wb_top irq raw
 * @hw: the wb_top hardware block pointer
 */
void dpu_hw_wb_top_status_clear(struct dpu_hw_blk *hw);

/**
 * dpu_hw_wb_top_ro_trig_enable - trigger the ro regs
 * @hw: the wb_top hardware block pointer
 * @enable: true for enabling or false for disabling
 */
void dpu_hw_wb_top_ro_trig_enable(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_wb_core_ro_trig_enable - trigger the ro regs
 * @hw: the wb_core hardware block pointer
 * @enable: true for enabling or false for disabling
 */
void dpu_hw_wb_core_ro_trig_enable(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_wb_qos_config - wb qos connfig
 * @hw: the wb_core hardware block pointer
 * @qos: qos value
 */
void dpu_hw_wb_qos_config(struct dpu_hw_blk *hw, u32 qos);

/**
 * dpu_hw_wb_line_buf_level_config - line buf level config
 * @hw: the wb_core hardware block pointer
 * @wb_lbuf_level: wb line buf level infomation
 */
void dpu_hw_wb_line_buf_level_config(struct dpu_hw_blk *hw, struct wb_line_buf_level wb_lbuf_level);

/**
 * dpu_hw_wb_top_dbg_irq_enable - unmask wdma dbg irq
 * @hw: the wb_top hardware block pointer
 * @enable: mask or unmask the irq
 */
void dpu_hw_wb_top_dbg_irq_enable(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_wb_top_dbg_irq_clear - clear wdma dbg irq
 * @hw: the wb_top hardware block pointer
 */
void dpu_hw_wb_top_dbg_irq_clear(struct dpu_hw_blk *hw);

/**
 * dpu_hw_wb_top_dbg_irq_dump - dump wdma dbg irq raw status
 * @hw: the wb_top hardware block pointer
 *
 * Returns: the value of dbg irq status
 */
u32 dpu_hw_wb_top_dbg_irq_dump(struct dpu_hw_blk *hw);

#endif
