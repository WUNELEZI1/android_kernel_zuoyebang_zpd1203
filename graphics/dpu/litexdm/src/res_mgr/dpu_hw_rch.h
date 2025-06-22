// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _DPU_HW_RCH_H_
#define _DPU_HW_RCH_H_

#include "osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_cap.h"
#include "dpu_hw_rdma_ops.h"

/**
 * dpu_hw_rch_ops - the virtual function table of rch class
 */
struct dpu_hw_rch_ops {
	/**
	 * channel_default_refine - refine rdma channel default reg param
	 * @channel_param: the rdma channel common param for extender mode.
	 */
	void (*channel_default_refine)(struct rch_channel_param *channel_param);

	/**
	 * channel_config - config the rch common param
	 * @hw: the rdma hardware block pointer
	 * @rch_common: the rdma common param for extender mode
	 * @layer_cfg: single plane layer param
	 * @pipe_cfg: single plane pipe param.
	 */
	void (*channel_config)(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg,
		struct rdma_pipe_cfg *pipe_cfg);

	/**
	 * layer_config - config the rch layer data
	 * @hw: the rch hardware pointer
	 * @rch_common: the rdma common param for extender mode
	 * @layer_cfg: the rch layer config pointer
	 */
	void (*layer_config)(struct dpu_hw_blk *hw,
			struct rch_channel_param *rch_common,
			struct rdma_layer_cfg *layer_cfg);

	/**
	 * layer_line_buf_level_config - config the rch layer data
	 * @hw: the rch hardware pointer
	 * @rdma_lbuf_level: the rdma lbuf and obuf level
	 */
	void (*layer_line_buf_level_config)(struct dpu_hw_blk *hw,
		struct rdma_line_buf_level rdma_lbuf_level);

	/**
	 * reset - reset the rch registers
	 * @hw: the rch hardware pointer
	 */
	void (*reset)(struct dpu_hw_blk *hw);
};

/**
 * dpu_hw_rch - rch hardware object
 * @hw: the rch hardware info
 * @blk_cap: the rch hardware resource and capability
 * @ops: the rch function table
 */
struct dpu_hw_rch {
	struct dpu_hw_blk hw;
	const struct dpu_rch_cap *blk_cap;

	struct dpu_hw_rch_ops ops;

	struct rch_channel_param channel_param;
};

static inline struct dpu_hw_rch *to_dpu_hw_rch(struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_rch, hw);
}

/**
 * dpu_hw_rch_init - init one rch hw block with its capability
 * @rch_cap: the rch capability pointer to indicate the
 *            capability of this rch
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_rch_init(struct dpu_rch_cap *rch_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_rch_deinit - deinit one rch hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_rch_deinit(struct dpu_hw_blk *hw);

#endif /* DPU_HW_RCH_H */
