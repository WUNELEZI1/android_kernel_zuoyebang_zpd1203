/* SPDX-License-Identifier: GPL-2.0 */
/*
 * XRING DMA BUF PagePool implementation
 * Based on earlier ION code by Google
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_DMABUF_PAGEPOOL_H__
#define __XRING_DMABUF_PAGEPOOL_H__

#include <linux/version.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/sched/signal.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/scatterlist.h>

#if KERNEL_VERSION(6, 5, 0) < LINUX_VERSION_CODE
#include "page_pool.h"
#endif

int xr_dmabuf_pagepool_create(const unsigned int *orders, gfp_t *gfp_mask, unsigned int len);
int xr_dmabuf_pagepool_destroy(const unsigned int *orders, unsigned int len);
int xr_dmabuf_pagepool_alloc(struct sg_table *table, unsigned int size,  unsigned int align_order);
int xr_dmabuf_pagepool_alloc_for_buddy(struct sg_table *table, unsigned int size, unsigned int align_order);
void xr_dmabuf_pagepool_free(struct sg_table *table);
long xr_dmabuf_pagepool_get_size(void);
long xr_dmabuf_pagepool_get_size_per_heap(void);
void xr_dmabuf_pagepool_dumpinfo(void);
int xr_dmabuf_pagepool_init(void);
#endif
