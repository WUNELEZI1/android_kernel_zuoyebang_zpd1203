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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#ifndef _DPU_HW_RCH_H_
#define _DPU_HW_RCH_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_rdma_ops.h"
#include "dpu_hw_dmmu_tbu_ops.h"

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
	 * layer_config - config the rch layer data
	 * @hw: the rdma hardware block pointer
	 * @rch_common: the rdma common param for extender mode
	 * @layer_cfg: single plane layer param.
	 */
	void (*layer_config)(struct dpu_hw_blk *hw,
			struct rch_channel_param *channel_param,
			struct rdma_layer_cfg *layer_cfg);

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
	 * reset - reset the rch registers
	 * @hw: the rch hardware pointer
	 */
	void (*reset)(struct dpu_hw_blk *hw);

	/**
	 * dmmu_tbu_config - configure tbu.
	 * @hw: the rch hardware pointer
	 * @fb_info: pointer of drm_framebuffer
	 * @type: rdma/wdma channel to operate
	 */
	void (*mmu_tbu_config)(struct dpu_hw_blk *hw,
			struct framebuffer_info *fb_info, enum module_type type);

	/**
	 * status_dump - dump hw status for debug
	 * @hw: the rch hardware pointer
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);

	/**
	 * status_clear - clear hw intr status
	 * @hw: the rch hardware pointer
	 */
	void (*status_clear)(struct dpu_hw_blk *hw);

#if defined(__KERNEL__)
	/**
	 * dump_to_file - dump regs to file
	 * @hw: the rch hardware pointer
	 * @f: the file pointer
	 */
	void (*dump_to_file)(struct dpu_hw_blk *hw, struct file *f);
#else
	void (*dump_to_file)(void);
#endif

	/**
	 * dbg_irq_enable - unmask rdma dbg irq
	 * @hw: the rch hardware pointer
	 * @enable: mask or unmask the irq
	 */
	void (*dbg_irq_enable)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * dbg_irq_clear - clear rdma dbg irq
	 * @hw: the rch hardware pointer
	 */
	void (*dbg_irq_clear)(struct dpu_hw_blk *hw);

	/**
	 * dbg_irq_dump - dump rdma dbg irq raw status
	 * @hw: the rch hardware pointer
	 *
	 * Returns: the value of rdma dbg irq raw status
	 */
	u32 (*dbg_irq_dump)(struct dpu_hw_blk *hw);

	/**
	 * dbg_irq_status - read rdma dbg irq status
	 * @hw: the rch hardware pointer
	 *
	 * Returns: the value of rdma dbg irq status
	 */
	u32 (*dbg_irq_status)(struct dpu_hw_blk *hw);
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

#endif /* _DPU_HW_RCH_H_ */
