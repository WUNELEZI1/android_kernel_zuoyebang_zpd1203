/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF sga heap
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __DMABUF_SGA_HEAP_H__
#define __DMABUF_SGA_HEAP_H__

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/dma-map-ops.h>

#include "xring_heaps.h"
#include "xr_heap_tee_ops.h"
#include "soc/xring/xr_dmabuf_helper.h"

int xr_perpare_change_mem_prot(struct dma_buf *dmabuf);
int xr_sga_heap_create(struct xring_heap_platform_data *data);

#endif

