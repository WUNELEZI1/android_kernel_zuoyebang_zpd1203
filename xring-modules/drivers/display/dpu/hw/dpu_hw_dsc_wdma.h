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

#ifndef _DPU_HW_DSC_WDMA_H_
#define _DPU_HW_DSC_WDMA_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsc_wdma_ops.h"
#include "dpu_hw_dmmu_tbu_ops.h"

/**
 * dpu_hw_dsc_wdma_ops - the virtual function table of dsc_wdma class
 */
struct dpu_hw_dsc_wdma_ops {
	/**
	 * layer_config - dsc rdma layer config
	 * @hw: the pointer of dsc_wdma hardware block
	 * @layer_cfg: dsc rdma layer config info
	 */
	void (*layer_config)(struct dpu_hw_blk *hw, struct dsc_wr_layer_cfg *layer_cfg);

	/**
	 * axi_cfg - dsc wdma axi config
	 * @hw: the pointer of dsc_wdma hardware block
	 * @axi_cfg: axi config info
	 */
	void (*axi_cfg)(struct dpu_hw_blk *hw, struct dsc_wr_axi_cfg *axi_cfg);

	/**
	 * mem_lp_ctl - control memory lowpower
	 * @hw: the pointer of dsc_wdma hardware block
	 * @auto_en: auto_en
	 */
	void (*mem_lp_ctl)(struct dpu_hw_blk *hw, u8 auto_en);

	/**
	 * secure_mode - set secure_mode
	 * @hw: the pointer of dsc_wdma hardware block
	 * @secure_en: secure_en
	 */
	void (*secure_mode)(struct dpu_hw_blk *hw, u8 secure_en);

	/**
	 * dmmu_tbu_config - configure tbu.
	 * @hw: the rch hardware pointer
	 * @fb_info: pointer of framebuffer_info
	 * @type: rdma/wdma channel to operate
	 */
	void (*mmu_tbu_config)(struct dpu_hw_blk *hw,
			struct framebuffer_info *fb_info, enum module_type type);
};

/**
 * dpu_hw_dsc_wdma - dsc_wdma hardware object
 * @hw: the dsc_wdma hardware info
 * @blk_cap: the dsc_wdma hardware resource and capability
 * @ops: the dsc_wdma function table
 */
struct dpu_hw_dsc_wdma {
	struct dpu_hw_blk hw;
	const struct dpu_dsc_wdma_cap *blk_cap;

	struct dpu_hw_dsc_wdma_ops ops;
};

static inline struct dpu_hw_dsc_wdma *to_dpu_hw_dsc_wdma(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_dsc_wdma, hw);
}

/**
 * dpu_hw_dsc_wdma_init - init one dsc_wdma hw block with its capability
 * @dsc_wdma_cap: the dsc_wdma capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_dsc_wdma_init(struct dpu_dsc_wdma_cap *dsc_wdma_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_dsc_wdma_deinit - deinit one dsc_wdma hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_dsc_wdma_deinit(struct dpu_hw_blk *hw);

#endif
