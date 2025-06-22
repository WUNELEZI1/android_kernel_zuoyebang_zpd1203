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

#ifndef _DPU_FlOW_CTRL_H_
#define _DPU_FlOW_CTRL_H_

#include <linux/types.h>
#include <linux/bits.h>
#include <drm/drm_atomic.h>
#include <drm/drm_fourcc.h>

#include "dpu_hw_format.h"
#include "dpu_hw_obuffer.h"
#include "dpu_hw_rdma_ops.h"
#include "dpu_hw_wb_ops.h"
#include "dpu_hw_dmmu_tbu_ops.h"

enum dpu_flow_line_buf_level_type {
	FLOW_OBUF_LEVEL_DSI = 0,
	FLOW_OBUF_LEVEL_DP,
	FLOW_LBUF_LEVEL_RDMA,
	FLOW_LBUF_LEVEL_WB,
};

enum dpu_flow_qos_type {
	FLOW_QOS_CMDLIST = 0,
	FLOW_QOS_MMU,
	FLOW_QOS_DSC_RDMA,
	FLOW_QOS_DSC_WDMA,
	FLOW_QOS_WB_WDMA,
	FLOW_QOS_RCH_RDMA,
};

/**
 * obuf_depth_info - dpu obuf depth information used for line buf level calculate
 * @obufen: obuffer mem used information
 * @fps: frame rate of panel
 * @vtotal: vtotal
 * @width: width of panel
 * @bpp: input format bpp of obuf
 */
struct obuf_depth_info {
	u32 obufen;
	u32 fps;
	u32 vtotal;
	u32 width;
	u32 bpp;
};

struct dpu_hw_qos_ops {
	/**
	 * cmdlist_qos_config - config cmdlist qos
	 * @ch_id: channel id of cmdlist
	 * @qos: qos value
	 */
	void (*cmdlist_qos_config)(u8 ch_id, u32 qos);

	/**
	 * mmu_qos_config - config mmu qos
	 * @hw: the tbu hardware pointer
	 * @type: the type of module used tbu
	 * @qos: qos value
	 */
	void (*mmu_qos_config)(struct dpu_hw_blk *hw, enum module_type type, u32 qos);

	/**
	 * dsc_wdma_qos_config - config dsc wdma qos
	 * @hw: the dsc wdma hardware pointer
	 * @qos: qos value
	 */
	void (*dsc_wdma_qos_config)(struct dpu_hw_blk *hw, u32 qos);

	/**
	 * dsc_rdma_qos_config - config dsc rdma qos
	 * @hw: the dsc rdma hardware pointer
	 * @qos: qos value
	 */
	void (*dsc_rdma_qos_config)(struct dpu_hw_blk *hw, u32 qos);

	/**
	 * wb_wdma_qos_config - config wb wdma qos
	 * @hw: the rch hardware pointer
	 * @qos: qos value
	 */
	void (*wb_wdma_qos_config)(struct dpu_hw_blk *hw, u32 qos);

	/**
	 * rch_rdma_qos_config - config rdma qos
	 * @hw: the rch hardware pointer
	 * @rdma_qos: rdma qos level
	 */
	void (*rch_rdma_qos_config)(struct dpu_hw_blk *hw,
			struct rdma_qos_level rdma_qos);
};

struct dpu_hw_line_buf_ops {
	/**
	 * dsi_obuf_line_buf_config - config dsi obuffer level
	 * @hw: the dsi tmg hardware pointer
	 * @obuffer: the obuffer parameter to config
	 */
	void (*dsi_obuf_level_config)(struct dpu_hw_blk *hw,
			struct obuffer_parms *obuffer);
	/**
	 * dp_obuf_line_buf_config - config dp obuffer level
	 * @hw: the dp tmg hardware pointer
	 * @obuffer: the obuffer parameter to config
	 */
	void (*dp_obuf_level_config)(struct dpu_hw_blk *hw,
			struct obuffer_parms *obuffer);

	/**
	 * rch_line_buf_config - config rch buffer level
	 * @hw: the rch rdma hardware pointer
	 * @buffer_level: rdma buffer level
	 */
	void (*rch_line_buf_level_config)(struct dpu_hw_blk *hw,
		struct rdma_line_buf_level rdma_lbuf_level);

	/**
	 * wb_line_buf_config - config wb buffer level
	 * @hw: the wb hardware pointer
	 * @wb_level: wb buffer level
	 */
	void (*wb_line_buf_level_config)(struct dpu_hw_blk *hw,
			struct wb_line_buf_level wb_lbuf_level);
};

/**
 * dpu_flow_ctrl - dpu flow control structure
 * @qos_ops: qos config options
 * @buffer_level_ops: buffer level config options
 * @dev: drm device
 */
struct dpu_flow_ctrl {
	struct dpu_hw_qos_ops qos_ops;
	struct dpu_hw_line_buf_ops line_buf_ops;
	struct drm_device *dev;
};

/**
 * dpu_flow_qos_get - dpu qos get interface for other dpu module
 * @qos_master_type: source module type
 * @input_data: input data for qos get
 * @output_data: output qos data
 */
void dpu_flow_qos_get(u32 qos_master_type, void *input_data, void *output_data);

/**
 * dpu_flow_line_buf_level_cfg - dpu buffer level config interface for other dpu module
 * @flow_ctrl: flow control structure
 * @hw: hardware pointer of source module
 * @buffer_level_type: source module type
 * @data: paramters from source module to calculate buffer level
 */
void dpu_flow_line_buf_level_cfg(struct dpu_flow_ctrl *flow_ctrl, struct dpu_hw_blk *hw,
		u32 buffer_level_type, void *data);

/**
 * dpu_flow_ctrl_init - initlize flow control structure
 * @flow_ctrl: pointer of flow control pointer
 * @dev: drm device
 */
int dpu_flow_ctrl_init(struct dpu_flow_ctrl **flow_ctrl, struct drm_device *dev);

/**
 * dpu_flow_ctrl_deinit - deinit flow control
 * @flow_ctrl: pointer of flow control pointer
 */
int dpu_flow_ctrl_deinit(struct dpu_flow_ctrl *flow_ctrl);

/**
 * dpu_flow_ctrl_get - get flow ctrl pointer
 * @dev: drm device
 */
struct dpu_flow_ctrl *dpu_flow_ctrl_get(struct drm_device *drm_dev);

#endif /* _DPU_FlOW_CTRL_H_ */
