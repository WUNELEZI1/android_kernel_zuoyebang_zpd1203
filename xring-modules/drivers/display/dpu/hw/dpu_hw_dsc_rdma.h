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

#ifndef _DPU_HW_DSC_RDMA_H_
#define _DPU_HW_DSC_RDMA_H_

#include "dpu_hw_parser.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsc_rdma_ops.h"
#include "dpu_hw_dmmu_tbu_ops.h"

/**
 * dpu_hw_dsc_rdma_ops - the virtual function table of dsc_rdma class
 */
struct dpu_hw_dsc_rdma_ops {
	/**
	 * layer_config - dsc rdma layer config
	 * @hw: the pointer of dsc_rdma hardware block
	 * @layer_cfg: dsc rdma layer config info
	 */
	void (*layer_config)(struct dpu_hw_blk *hw, struct dsc_rd_layer_cfg *layer_cfg);

	/**
	 * axi_cfg - dsc wdma axi config
	 * @hw: the pointer of dsc_rdma hardware block
	 * @axi_cfg: axi config info
	 */
	void (*axi_cfg)(struct dpu_hw_blk *hw, struct dsc_rd_axi_cfg *axi_cfg);

	/**
	 * secure_mode - set secure_mode
	 * @hw: the pointer of dsc_rdma hardware block
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
 * dpu_hw_dsc_rdma - dsc_rdma hardware object
 * @hw: the dsc_rdma hardware info
 * @blk_cap: the dsc_rdma hardware resource and capability
 * @ops: the dsc_rdma function table
 */
struct dpu_hw_dsc_rdma {
	struct dpu_hw_blk hw;
	const struct dpu_dsc_rdma_cap *blk_cap;

	struct dpu_hw_dsc_rdma_ops ops;
};

static inline struct dpu_hw_dsc_rdma *to_dpu_hw_dsc_rdma(
		struct dpu_hw_blk *hw)
{
	return container_of(hw, struct dpu_hw_dsc_rdma, hw);
}

/**
 * dpu_hw_dsc_rdma_init - init one dsc_rdma hw block with its capability
 * @dsc_rdma_cap: the dsc_rdma capability pointer to indicate the capability
 * @base_mem: dpu base memory info
 *
 * Return: valid hw block pointer on success, error pointer on failure
 */
struct dpu_hw_blk *dpu_hw_dsc_rdma_init(struct dpu_dsc_rdma_cap *dsc_rdma_cap,
		struct dpu_iomem *base_mem);

/**
 * dpu_hw_dsc_rdma_deinit - deinit one dsc_rdma hw block with base hw block pointer
 * @hw: base hw block pointer
 */
void dpu_hw_dsc_rdma_deinit(struct dpu_hw_blk *hw);

#endif
