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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#ifndef _DPU_HW_STRIPE_H_
#define _DPU_HW_STRIPE_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"

#include "color/dpu_hw_pre_scaler_ops.h"
#include "dpu_hw_wb_scaler_ops.h"
#include "dpu_hw_stripe_ops.h"

struct dpu_hw_stripe_ops {
	/**
	 * layer_stripe_config - config the stripe rch layer data
	 * @hw: the rch hardware pointer
	 * @node_id: cmdlist node id
	 * @layer_cfg: the stripe rch layer config pointer
	 * @mode: rdma_extender_mode
	 */
	void (*layer_stripe_config)(struct dpu_hw_blk *hw, s64 node_id,
			struct rdma_layer_stripe_cfg *layer_cfg, enum rdma_extender_mode mode);

	/**
	 * layer_scaler_stripe_set - stripe set pre scaler parameter
	 * @hw: the pre scaler hardware pointer
	 * @node_id: comdlist node id
	 * @hw_cfg: pointer to prepq_hw_cfg
	 */
	void (*layer_scaler_stripe_set)(struct dpu_hw_blk *hw, s64 node_id, struct dpu_2d_scaler_cfg *hw_cfg);

	/**
	 * cmps_rch_layer_config - config the cmps rch layer enaable data
	 * @hw: the mixer hardware pointer
	 * @node_id: cmdlist node id
	 * @blend_cfg: pointer of mixer blend config
	 * @zorder: layer z order
	 * @dma_id: dma id
	 */
	void (*cmps_rch_layer_config)(struct dpu_hw_blk *hw, s64 node_id,
		struct mixer_blend_cfg *blend_cfg, struct dpu_rect_v2 *area_cfg,
		u32 zorder, u32 dma_id);

	/**
	 * cmps_wb_stripe_dst_config - config the cmps wb input data
	 * @hw: the mixer hardware pointer
	 * @node_id: cmdlist node id
	 * @w:cmps output w
	 * @h:cmps output h
	 */
	void (*cmps_wb_stripe_dst_config)(struct dpu_hw_blk *hw, s64 node_id,
			u16 w, u16 h);

	/**
	 * wb_frame_stripe_config - update the wb stripe input and output config
	 * @hw: the wb hardware pointer
	 * @node_id: cmdlist node id
	 * @cfg: the pointer of wb stripe frame related config
	 */
	void (*wb_frame_stripe_config)(struct dpu_hw_blk *hw, s64 node_id, struct wb_stripe_frame_cfg *cfg);

	/**
	 * wb_scaler_stripe_set - stripe set wb scaler parameters
	 * @hw: the wb scaler hardware pointer
	 * @node_id: cmdlist node id
	 * @scaler: the pointer of wb scaler config
	 */
	void (*wb_scaler_stripe_set)(struct dpu_hw_blk *hw, s64 node_id, struct dpu_1d_scaler_cfg *scaler);
};

struct dpu_hw_stripe {
	struct dpu_hw_stripe_ops ops;
};

struct dpu_hw_stripe *dpu_stripe_res_init(void);

void dpu_stripe_res_deinit(struct dpu_hw_stripe *hw_dvs);

#endif /* _DPU_HW_STRIPE_H_ */
