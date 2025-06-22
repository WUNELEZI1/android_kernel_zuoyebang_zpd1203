/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Carveout heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __DMABUF_CARVEOUT_HEAP_H__
#define __DMABUF_CARVEOUT_HEAP_H__

#include "xring_heaps.h"

int xr_carveout_heap_create(struct xring_heap_platform_data *data);

#endif
