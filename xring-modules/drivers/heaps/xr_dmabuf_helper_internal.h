/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Carveout heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on dma heap code
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef __XRING_DMABUF_HELPER_INTERNAL_H__
#define __XRING_DMABUF_HELPER_INTERNAL_H__

#include <linux/dma-heap.h>
#include <linux/dma-buf.h>
#include <linux/ktime.h>

#define TOTAL_RAM_PAGES_1G	(1 << 18)

#define KTIME_DEFINE(start, end, duration_ms) \
	ktime_t start, end, duration_ms

#define KTIME_DEFINE_GET_START(start, end, duration_ms) \
	ktime_t start, end, duration_ms; \
	start = ktime_get()


#define KTIME_GET_END_DURATION(start, end, duration_ms) do { \
		end = ktime_get(); \
		duration_ms = ktime_ms_delta(end, start); \
	} while (0)

#define KTIME_GET_END_DURATION_US(start, end, duration_us) do { \
		end = ktime_get(); \
		duration_us = ktime_us_delta(end, start); \
	} while (0)

struct xr_dmabuf_helper_ops {
	const char *name;
	void *priv; // which heap
	u32 scene;
	u32 (*get_sfd)(struct dma_buf *data);
	int (*change_mem_prot)(struct dma_buf *data);
	int (*pre_alloc)(void *heap, size_t size, int mode);
	u64 (*get_alloc_size)(void *data);
	u64 (*get_buddy_consumed)(void *data);
	u64 (*get_cma_consumed)(void *data);
	u64 (*get_pool_cached)(void *data);
	int (*dio_alloc)(int file_fd, int dmabuffd, loff_t offset, loff_t ppos, size_t len);
	struct list_head list_node;
};

int xr_dmabuf_helper_register(struct xr_dmabuf_helper_ops *ops);
int xr_dmabuf_helper_init(void);
void xr_dmabuf_helper_exit(void);
void show_dmabuf_detail(bool verbose);

#endif


