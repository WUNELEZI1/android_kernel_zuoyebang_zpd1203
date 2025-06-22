/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#ifndef _DKSM_IOMMU_H_
#define _DKSM_IOMMU_H_

#include <linux/rbtree.h>
#include <linux/spinlock.h>
#include <linux/dma-buf.h>
#include <linux/scatterlist.h>
#include <linux/atomic.h>

#include "dksm_chrdev.h"

#define SIZE_16_BYTES 16
#define EXTRA_ENTRY 120
#define EXTRA_TLB_ALIGN_ENTRY 12
#define BUS_INTERLEAVE_FLAG_BIT (1 << 26)
#define GET_TLB_SIZE(buf_size) ((((buf_size) >> PAGE_SHIFT) + EXTRA_TLB_ALIGN_ENTRY + EXTRA_ENTRY) * 4)
#define IOVE_ALIGN      (4 * PAGE_SIZE)
#define IOMMU_PLANE_MAX 3

#define IOMMU_MAGIC 'I'
#define IOMMU_MAP_IOVA      _IOWR(IOMMU_MAGIC, 0x1, struct map_dma_buf)
#define IOMMU_UNMAP_IOVA    _IOWR(IOMMU_MAGIC, 0x2, struct map_dma_buf)

/**
 * struct iova_domain - iova info
 *
 * @node: node of rb_node, store all iova domain
 * @plane_num: cnt of plane, used in dpu hw
 * @iova: iova used in dpu hw
 * @size: buffer size in byte
 * @iova_offsets: offset of plane iova base addr, from iova
 * @tlb_pa: physical addr of tlb
 * @tlb_vaddr: virtual addr of tlb
 * @tlb_offsets: offset of plane tbu base addr, from tlb_pa or tlb_vaddr
 * @ref: refcount of map
 * @key: key of rb_node, such as dma buf
 * @dev: point of struct device, such as drm_device
 * @pid: process id (unused briefly)
 */
struct iova_domain {
	struct rb_node node;

	u8 plane_num;
	unsigned long iova;
	u32 size;
	unsigned int iova_offsets[IOMMU_PLANE_MAX];

	dma_addr_t tlb_pa;
	void *tlb_vaddr;
	unsigned int tlb_offsets[IOMMU_PLANE_MAX];

	atomic_long_t ref;
	u64 key;

	struct device *dev;
	pid_t pid;
};

/**
 * struct dksm_iommu_domain - iommu context
 *
 * @iova_lock: spin lock used to protect rb tree
 * @iova_root: root node of rb tree
 * @iova_start: start addr of iova
 * @iova_size: total size of iova pool
 * @iova_align: PAGE_SIZE
 * @iova_pool: pointer of iova pool
 * @tlb_mem_pool: pointer of tlb mem pool
 * @dev: point of struct device, such as drm_device
 * @inited: bool
 */
struct dksm_iommu_domain {
	spinlock_t iova_lock;
	struct rb_root iova_root;

	unsigned long iova_start;
	unsigned long iova_size;
	unsigned long iova_align;

	struct gen_pool *iova_pool;
	struct dksm_mem_pool *tlb_mem_pool;

	struct device *dev;
	struct dksm_chrdev chrdev;
	atomic_t refcnt;
	bool inited;
};

struct map_dma_buf {
	/* input info */
	u32 size;
	int share_fd;

	/* output info */
	u64 tlb_pa;
	u64 iova;
};

/**
 * struct iommu_format_info - plane fromat info used iommu
 *
 * @offsets: Offset from buffer start in bytes
 * @is_afbc: is afbc mode or not
 */
struct iommu_format_info {
	unsigned int offsets[IOMMU_PLANE_MAX];
	bool is_afbc;
};

/**
 * dksm_iommu_fill_pt_entry - assemble tlb page table (only used in wfd)
 *
 * @dma_buf: pointer of struct dma_buf. if not afbc buffer, dma buf can be NULL.
 * @sgt: pointer of struct sg_table.
 * @format_info: pointer of struct iommu_format_info, can't be NULL
 * @iova_dom: pointer of struct iova_domain, can't be NULL.
 */
void dksm_iommu_fill_pt_entry(struct dma_buf *dma_buf, struct sg_table *sgt,
		struct iommu_format_info *format_info, struct iova_domain *iova_dom);

/**
 * dksm_iommu_map_dmabuf - map iova by dmabuf
 *
 * @dmabuf: pointer of struct dma_buf
 * @sgt: pointer of struct sg_table. sgt is optional.
 * if sgt is null, this func will get sgt from dma buf
 * Return struct iova_domain*
 * @format_info: pointer of struct iommu_format_info, can't be NULL
 */
struct iova_domain *dksm_iommu_map_dmabuf(struct dma_buf *dmabuf, struct sg_table *sgt,
		struct iommu_format_info *format_info);

/**
 * dksm_iommu_unmap_dmabuf - unmap iova by dmabuf
 *
 * @dmabuf: pointer of struct dma_buf
 */
void dksm_iommu_unmap_dmabuf(struct dma_buf *dmabuf);

/**
 * dksm_iommu_context_init - init iommu ctx
 *
 * @dev: device of drm_dev
 * @mem_pool: pointer of mem_pool
 * Return int
 */
int dksm_iommu_context_init(struct device *dev, struct dksm_mem_pool *mem_pool);

/**
 * dksm_iommu_context_deinit - deinit iommu ctx
 */
void dksm_iommu_context_deinit(void);

/**
 * dksm_iommu_create_iova_domain - create iova domain
 *
 * @size: buffer size of layer
 * @iova_dom: pointer of struct iova_domain
 * Return int
 */
int dksm_iommu_create_iova_domain(u32 size, struct iova_domain *iova_dom);

/**
 * dksm_iommu_destory_iova_domain - destory iova domain
 *
 * @iova_dom: pointer of struct iova_domain
 */
void dksm_iommu_destory_iova_domain(struct iova_domain *iova_dom);

#endif
