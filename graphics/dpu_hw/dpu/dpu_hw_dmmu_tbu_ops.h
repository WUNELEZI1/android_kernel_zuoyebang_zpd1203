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

#ifndef _DPU_HW_DMMU_TBU_H_
#define _DPU_HW_DMMU_TBU_H_

#include "dpu_hw_common.h"

/* module_type - rdma/wdma channel to operate */
enum module_type {
	MODULE_LEFT_RDMA = 0,
	MODULE_RIGHT_RDMA,
	MODULE_WDMA,
	MODULE_DSC_WDMA,
	MODULE_DSC_RDMA,
};

/**
 * framebuffer_info - framebuffer info for dmmu to use
 * @plane_num: plane num
 * @iova: iova address for hardware
 * @tlb_pa: tlb buffer phy address for hw
 * @iova_offsets: offset of plane iova base addr
 * @tlb_offsets: offset of plane tlb phy base addr
 * @is_afbc: is afbc mode or not
 * @qos: qos value
 */
struct framebuffer_info {
	u8 plane_num;
	u64 iova;
	dma_addr_t tlb_pa;
	unsigned int iova_offsets[3];
	unsigned int tlb_offsets[3];

	bool is_afbc;
	u32 qos;
};

/**
 * dpu_mmu_tbu_config - configure tbu.
 * @hw: dpu_hw_blk struct pointer
 * @fb_info: pointer of framebuffer_info
 * @type: rdma/wdma channel to operate
 */
void dpu_mmu_tbu_config(struct dpu_hw_blk *hw,
		struct framebuffer_info *fb_info, enum module_type type);

#endif
