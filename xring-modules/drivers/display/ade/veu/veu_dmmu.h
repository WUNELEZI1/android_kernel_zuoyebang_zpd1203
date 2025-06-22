/* SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _VEU_DMMU_H_
#define _VEU_DMMU_H_

#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include "veu_drv.h"
#include "veu_dmmu_ops.h"
#include "veu_enum.h"

#define IOVA_BASE                  0x10000000ULL
#define IOVA_ONE_BLOCK_SIZE        (4 * 1024 * 1024)
#define IOVA_MAX_BLOCK_NUM         (1000)
#define IOVA_RESERVED_BASE         0x8000000ULL

#define EXTRA_ENTRY 120
#define EXTRA_TLB_ALIGN_ENTRY 12
#define GET_TLB_SIZE(buf_size) ((((buf_size) >> PAGE_SHIFT) + EXTRA_TLB_ALIGN_ENTRY + EXTRA_ENTRY) * 4)
#define IOMMU_PLANE_MAX 3

/**
 * veu_mmu_object - the veu mmu object structure
 * @sgt: the sg table for veu mmu object
 * @buf: dma buf convert from sharefd given by user space
 * @paddr: the physical address pointer
 * @vaddr: the virtual address pointer
 * @tlb_pa: tlb buffer phy address for hw
 * @tlb_vaddr: tlb virtual address pointer
 * @iova: iova address for hardware
 * @vmap_use_count: the reference count of vmap
 * @vmap_lock: the lock of vmap accessing
 * @plane_num: plane_num for current layer of mmu_object
 * @iova_offsets: iova offset for each plane
 * @tlb_offsets: tlb offset for each plane
 */
struct veu_mmu_object {
	struct sg_table *sgt;

	struct dma_buf *buf;
	struct dma_buf_attachment *attach;

	dma_addr_t paddr;
	void *vaddr;

	dma_addr_t tlb_pa;
	void *tlb_vaddr;
	u64 iova;

	u32 vmap_use_count;
	struct mutex vmap_lock;

	u32 buf_size;
	u32 width;
	u32 height;

	bool is_afbc;
	u32 afbc_layout;

	u8 plane_num;
	unsigned int iova_offsets[IOMMU_PLANE_MAX];
	unsigned int tlb_offsets[IOMMU_PLANE_MAX];
};

/**
 * veu_mmu_mgr_ctx - the veu mmu manager context
 * @iova_pool_bitmap: use bitmap to manage iova space
 *                  iova = IOVA_BASE + bit_start * IOVA_ONE_BLOCK_SIZE;
 *                  bitmap val 1: busy
 *                             0: idle
 *                  if no available space in bitmap:
 *                  iova = IOVA_RESERVED_BASE;
 * @veu_dev: the veu device pointer
 * @lock: spin lock from mmu_mgr_ctx
 * @pt_pool: page table gen pool
 * @pt_pa: cpu pa,dma handle of page table memory pool
 * @pt_va: cpu virtual address of page table memory pool
 * @pt_size: size of page table memory pool
 */
struct veu_mmu_mgr_ctx {
	DECLARE_BITMAP(iova_pool_bitmap, IOVA_MAX_BLOCK_NUM);
	struct device *dev;
	struct mutex lock;

	struct gen_pool *pt_pool;
	dma_addr_t pt_pa;
	void *pt_va;
	u64 pt_size;
};

/**
 * veu_mmu_mgr_ctx_init - init veu mmu manager context
 * @veu_dev: the veu device pointer
 *
 * Return: 0 on success, error code on failure
 */
int veu_mmu_mgr_ctx_init(struct device *dev);
/**
 * veu_mmu_mgr_ctx_cleanup - clean veu mmu manager context
 */
void veu_mmu_mgr_ctx_cleanup(void);

/**
 * veu_mmu_map_layer_buffer - map layer buffer
 * @obj: pointer of mmu object of veu device
 *
 * Return: error code on failure, 0 on success
 */
int veu_mmu_map_layer_buffer(struct veu_mmu_object *obj);

/**
 * veu_mmu_unmap_layer_buffer - unmap layer buffer
 * @obj: pointer of mmu object of veu device
 */
void veu_mmu_unmap_layer_buffer(struct veu_mmu_object *obj);

/*
 * veu_mmu_get_framebuffer_info - get buffer info for mmu
 * @veu_dev: pointer of veu device
 * @info: info created by veu device
 * @module_type: identify rdma and wdma
 */
void veu_mmu_get_framebuffer_info(struct veu_data *veu_dev,
		struct buffer_info *info, enum VEU_MODULE_TYPE module_type);

#endif // _VEU_DMMU_H_
