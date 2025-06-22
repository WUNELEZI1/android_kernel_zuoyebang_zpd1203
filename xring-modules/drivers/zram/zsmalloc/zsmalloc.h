/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */
#ifndef _XR_ZS_MALLOC_H_
#define _XR_ZS_MALLOC_H_

#include <linux/types.h>

/*
 * zsmalloc mapping modes
 *
 * NOTE: These only make a difference when a mapped object spans pages.
 */
enum zs_mapmode {
	ZS_MM_RW, /* normal read-write mapping */
	ZS_MM_RO, /* read-only (no copy-out at unmap time) */
	ZS_MM_WO /* write-only (no copy-in at map time) */
	/*
	 * NOTE: ZS_MM_WO should only be used for initializing new
	 * (uninitialized) allocations.  Partial writes to already
	 * initialized allocations should use ZS_MM_RW to preserve the
	 * existing data.
	 */
};

enum zsmalloc_type {
	ZSMALLOC_TYPE_BASEPAGE,
#ifdef CONFIG_XRING_ZSMALLOC_MULTI_PAGES
	ZSMALLOC_TYPE_MULTI_PAGES,
#endif
	ZSMALLOC_TYPE_MAX,
};

struct zs_pool_stats {
	/* How many pages were migrated (freed) */
	atomic_long_t pages_compacted;
};

struct zs_pool;

struct zs_pool *zs_create_pool(const char *name);
void zs_destroy_pool(struct zs_pool *pool);

unsigned long zs_malloc(struct zs_pool *pool, size_t size, gfp_t flags);
void zs_free(struct zs_pool *pool, unsigned long obj);

size_t zs_huge_class_size(struct zs_pool *pool, enum zsmalloc_type type);

void *zs_map_object(struct zs_pool *pool, unsigned long handle,
			enum zs_mapmode mm);
void zs_unmap_object(struct zs_pool *pool, unsigned long handle);

unsigned long zs_get_total_pages(struct zs_pool *pool);
unsigned long zs_compact(struct zs_pool *pool);

unsigned int zs_lookup_class_index(struct zs_pool *pool, unsigned int size);

void zs_pool_stats(struct zs_pool *pool, struct zs_pool_stats *stats);

int __init zs_init(void);
void __exit zs_exit(void);

#endif
