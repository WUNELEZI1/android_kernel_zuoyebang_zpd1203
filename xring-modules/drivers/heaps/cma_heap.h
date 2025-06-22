/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF CMA heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __DMABUF_CMA_HEAP_H__
#define __DMABUF_CMA_HEAP_H__

#include "xring_heaps.h"

int xr_cma_heap_create(struct xring_heap_platform_data *data);

#endif
