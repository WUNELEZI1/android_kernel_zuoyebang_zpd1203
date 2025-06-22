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

#ifndef _VEU_DMMU_OPS_H_
#define _VEU_DMMU_OPS_H_

#include "veu_drv.h"
#include "veu_defs.h"
#include "veu_enum.h"

enum layer_plane_num {
	PLANE_ONE = 0,
	PLANE_TWO = 1,
	PLANE_THREE = 2,
	PLANE_MAX = 3,
};

/**
 * framebuffer_info - framebuffer info for dmmu to use
 * @iova_offsets: iova offset for each plane
 * @tlb_offsets: tlb offset for each plane
 * @is_afbc: is afbc mode or not
 * @size: size of the drm_gem_object, in bytes
 * @tlb_pa: tlb buffer phy address for hw
 * @tlb_vaddr: tlb virtual address pointer
 * @iova: iova address for hardware
 * @plane_num: plane num for current framebuffer
 */
struct buffer_info {
	unsigned int iova_offsets[PLANE_MAX];
	unsigned int tlb_offsets[PLANE_MAX];
	bool is_afbc;
	size_t size;
	dma_addr_t tlb_pa;
	void *tlb_vaddr;
	u64 iova;
	u8 plane_num;
};

/**
 * dpu_mmu_tbu_config - configure tbu.
 * @hw: dpu_hw_blk struct pointer
 * @fb_info: pointer of framebuffer_info
 * @type: rdma/wdma channel to operate
 */
void veu_mmu_tbu_config(struct veu_data *veu_dev,
		struct buffer_info *info, enum VEU_MODULE_TYPE type);

#endif // _VEU_DMMU_OPS_H_
