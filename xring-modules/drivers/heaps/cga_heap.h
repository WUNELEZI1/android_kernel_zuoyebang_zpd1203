/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Xring cga heap
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef _XRING_CGA_HEAP_H
#define _XRING_CGA_HEAP_H

#include <linux/err.h>
#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/dma-map-ops.h>
#include <linux/spinlock_types.h>
#include <linux/types.h>

#include "xring_heaps.h"
#include "xr_heap_tee_ops.h"
#include "soc/xring/xr_dmabuf_helper.h"

#define XRING_GP_SHIFT 12

/**
 * struct cga_heap - represents a cga heap in system
 * @dma_heap			point to the parent class of dma heap
 * @dev					point to the xring_heaps device
 * @cma					point to the related cma of this heap
 * @cma_base			cma base address
 * @cma_size			size of cga heap's cma
 * @pool				gen pool to manage memory
 * @mutex				heap mutex
 * @genpool_shift		gen pool manage unit
 * @heap_size			total heap size
 * @alloc_size			heap allocated size
 * @cma_alloc_size		heap alloc cma size
 * @pre_alloc_work		worker to asynchronously do cma alloc fill pool
 * @heap_attr			heap attr, load image or runtime memory
 * @pre_alloc_attr		do pre alloc or not
 * @pre_alloc_mutex		pre alloc mutex
 * @append_size			alloc sizefrom cma each time
 * @subregion_size		subregion align size, 2MB/4MB
 * @allocate_head		record alloc cma memory
 *
 * Represents a secure heap which alloc non-contiguous physic memory.
 */
struct cga_heap {
	struct dma_heap *heap;
	struct device *dev;
	struct cma *cma;
	phys_addr_t cma_base;
	size_t cma_size;
	struct gen_pool *pool;
	struct mutex mutex;
	u32 genpool_shift;
	size_t heap_size;
	size_t alloc_size;
	size_t cma_alloc_size;
	struct work_struct pre_alloc_work;
	u32 heap_attr;
	u32 pre_alloc_attr;
	struct mutex pre_alloc_mutex;
	u64 append_size;
	u64 subregion_size;
	struct list_head allocate_head;
};

/**
 * struct cga_heap_buffer - represents cga heap shared buffer
 * @heap			point to secsg heap
 * @mutex			buffer mutex
 * @sfd				secure fd
 * @len				buffer alloc length
 * @sg_table		all memory store in sg table
 * @attachments		list of dma_buf_attachment that denotes all devices attached
 * @vaddr			kernel address
 * @vmap_cnt		vmap times
 */
struct cga_heap_buffer {
	struct xr_heap_buffer_header header;
	struct cga_heap *heap;
	struct mutex mutex;
	u64 sfd;
	size_t len;
	struct sg_table *sg_table;
	struct list_head attachments;
	void *vaddr;
	int vmap_cnt;
};

/**
 * struct alloc_list - represents cga heap alloc cma memory
 * @id			identify a table in tee
 * @addr		alloc cma base
 * @size		alloc cma size
 * @list	cma region list
 */
struct alloc_list {
	u64 id;
	u64 addr;
	u32 size;
	struct list_head list;
};

int xr_cga_heap_create(struct xring_heap_platform_data *data);
#endif

