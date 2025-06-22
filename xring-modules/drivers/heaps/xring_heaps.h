/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Xring heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef __XRING_HEAPS_H__
#define __XRING_HEAPS_H__

#include <dt-bindings/xring/xr-heaps-dts.h>

#include <linux/types.h>
#include <linux/cma.h>
#include <linux/of.h>

#include "../mm/cma.h"
#include "xring_meminfo/xring_meminfo.h"
#include "xr_dmabuf_helper_internal.h"

#define xrheap_err(fmt, ...) \
	pr_err("[HEAPS]%s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrheap_info(fmt, ...) \
	pr_info("[HEAPS]%s:%d "fmt, __func__,  __LINE__, ##__VA_ARGS__)

#define xrheap_debug(fmt, ...) \
	pr_debug("[HEAPS]%s:%d "fmt, __func__,  __LINE__, ##__VA_ARGS__)

#define xrheap_warn(fmt, ...) \
	pr_warn("[HEAPS]%s:%d "fmt, __func__,  __LINE__, ##__VA_ARGS__)

/**
 * struct xring_heap_platform_data - represents the common data in all xring heap
 * @heap_type		heap type
 * @heap_name		heap name
 * @cma_name		every heap has a related cma memory
 * @base			heap memory base
 * @size			heap memory size
 * @priv			heap root device
 */
struct xring_heap_platform_data {
	u32 heap_type;
	u32 heap_attr;
	u32 heap_case;
	const char *heap_name;
	struct cma *cma;
	phys_addr_t cma_base;
	size_t cma_size;
	struct device_node *node;
	struct device *priv;
};

struct xring_secure_heap_data {
	u32 heap_case;
	phys_addr_t heap_base;
	size_t heap_size;
};

struct cma *xr_cma_get_by_name(const char *name);

#endif
