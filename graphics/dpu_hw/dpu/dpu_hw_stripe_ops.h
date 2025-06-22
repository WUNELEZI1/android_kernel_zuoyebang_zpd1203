/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _DPU_HW_STRIPE_OPS_H_
#define _DPU_HW_STRIPE_OPS_H_

#include "dpu_hw_common.h"
#include "dpu_hw_rdma_ops.h"
#include "dpu_hw_wb_scaler_ops.h"
#include "../color/dpu_hw_pre_scaler_ops.h"
#include "../color/dpu_hw_color.h"

struct rdma_layer_stripe_cfg {
	/**
	 * @crop_area: layer crop area(x1,y1,x2,y2)
	 * @img_size: layer source image size.
	 * @roi_area: layer roi area(x1,y1,x2,y2)
	 */
	struct dpu_rect crop_area;
	struct dpu_rect_v2 img_size;

	/**
	 * @roi0_area: layer roi0 area(x1,y1,x2,y2)
	 * @roi1_area: layer roi1 area(x1,y1,x2,y2)
	 */
	struct dpu_rect roi0_area;
	struct dpu_rect roi1_area;
	bool roi0_en;
	bool roi1_en;

	/**
	 * @layer_mixer_y_offset: indicate layer y offset at mixer
	 */
	u16 layer_mixer_y_offset;
};

struct wb_stripe_frame_cfg {
	u16 input_width;
	u16 input_height;
	bool is_offline_mode;
	enum rot_mode frame_rot_mode;
	enum flip_mode frame_flip_mode;
	u8 output_format;
	bool is_afbc;

	/**
	 * @output_original_width: width of output original image
	 * @output_original_height: height of output original image
	 * @output_subframe_rect: rect of output subframe rect
	 */
	u16 output_original_width;
	u16 output_original_height;
	struct dpu_rect_v2 output_subframe_rect;

	bool crop0_en;
	bool crop1_en;
	bool crop2_en;
	struct dpu_rect crop0_area;
	struct dpu_rect crop1_area;
	struct dpu_rect crop2_area;
	bool dither_en;
	bool scl_en;
};

/**
 * dpu_hw_stripe_rdma_layer_config - setup stripe rdma_path input layer configure.
 * @hw: the rdma hardware block pointer
 * @node_id: cmdlist node index
 * @layer_cfg: rdma_path input layer configure.
 * @extender_mode: left or right image flag
 */
void dpu_hw_stripe_rdma_layer_config(struct dpu_hw_blk *hw, s64 node_id,
		struct rdma_layer_stripe_cfg *layer_cfg, enum rdma_extender_mode mode);

/**
 * dpu_hw_stripe_pre_scaler_set - setup stripe rch 2d-scaler param configure.
 * @hw: the rch 2d-scaler hardware block pointer
 * @node_id: cmdlist node index
 * @hw_cfg: rch 2d-scaler param configure.
 */
void dpu_hw_stripe_pre_scaler_set(struct dpu_hw_blk *hw,
		s64 node_id, struct dpu_2d_scaler_cfg *hw_cfg);

/**
 * dpu_hw_stripe_mixer_rch_layer_config - set mixer blend config for a layer
 * @hw: the mixer hardware pointer
 * @node_id: cmdlist node id
 * @blend_cfg: pointer of mixer blend config
 * @area_cfg: rch layer rect info in mixer
 * @zorder: layer z order
 * @dma_id: dma id
 */
void dpu_hw_stripe_mixer_rch_layer_config(struct dpu_hw_blk *hw, s64 node_id,
		struct mixer_blend_cfg *blend_cfg, struct dpu_rect_v2 *area_cfg, 
		u32 zorder, u32 dma_id);

/**
 * dpu_hw_stripe_mixer_wb_input_config - set width and height config for mixer output to wb
 * @hw: the mixer hardware info
 * @node_id: cmdlist node index
 * @w: width of wb input
 * @h: height of wb input
 */
void dpu_hw_stripe_mixer_wb_input_config(struct dpu_hw_blk *hw, s64 node_id,
		u16 w, u16 h);

/**
 * dpu_stripe_hw_wb_cfg_setup - set input and output frame configurations
 * @hw: the wb hardware block pointer
 * @node_id: cmdlist node index
 * @cfg: configurations of wb stirpe input and output frame
 */
void dpu_stripe_hw_wb_cfg_setup(struct dpu_hw_blk *hw, s64 node_id,
		struct wb_stripe_frame_cfg *cfg);

/**
 * dpu_hw_stripe_wb_scaler_set - set wb scaler parameters
 * @hw: the wb scaler hardware pointer
 * @node_id: cmdlist node id
 * @scaler: the pointer of wb scaler
 */
void dpu_hw_stripe_wb_scaler_set(struct dpu_hw_blk *hw, s64 node_id,
	struct dpu_1d_scaler_cfg *scaler);


#endif
