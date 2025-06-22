// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring Mem adapter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/gfp_types.h>
#include <linux/mmzone.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/swap.h>
#include <linux/gfp.h>
#include <linux/fs.h>
#include <linux/compaction.h>

#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/page-def.h>
#include <asm-generic/getorder.h>
#include <soc/xring/xring_mem_adapter.h>
#include <../../common/mm/internal.h>
#include <linux/android_debug_symbols.h>

#include "xring_mem_adapter.h"

#if KERNEL_VERSION(5, 15, 0) >= LINUX_VERSION_CODE
extern unsigned long shrink_slab(gfp_t gfp_mask, int nid,
				struct mem_cgroup *memcg,
				int priority);

void drop_slab_node(int nid)
{
	struct mem_cgroup *memcg = NULL;
	unsigned long freed = 0;
	int i = 0;

	for (i = 0; i < (1 << MEM_CGROUP_ID_SHIFT); i++) {
		rcu_read_lock();
		memcg = mem_cgroup_from_id(i);
		if (!memcg) {
			rcu_read_unlock();
			continue;
		}
		rcu_read_unlock();

		freed += shrink_slab(GFP_KERNEL, nid, memcg, 0);
		if (freed < 10)
			break;
	}
}

void xring_drop_slabcache(void)
{
	int nid = 0;

	for_each_online_node(nid)
		drop_slab_node(nid);
}
EXPORT_SYMBOL(xring_drop_slabcache);
#else
typedef void (*drop_slab_func)(void);
void xring_drop_slabcache(void)
{
	drop_slab_func drop_slab = (drop_slab_func)android_debug_symbol(ADS_DROP_SLAB);

	drop_slab();
}
EXPORT_SYMBOL(xring_drop_slabcache);
#endif

static void drop_pagecache_sb(struct super_block *sb, void *unused)
{
	struct inode *inode = NULL;
	struct inode *toput_inode = NULL;

	spin_lock(&sb->s_inode_list_lock);
	list_for_each_entry(inode, &sb->s_inodes, i_sb_list) {
		spin_lock(&inode->i_lock);

		if ((inode->i_state & (I_FREEING | I_WILL_FREE | I_NEW)) ||
			(mapping_empty(inode->i_mapping) && !need_resched())) {
			spin_unlock(&inode->i_lock);
			continue;
		}
		atomic_inc(&inode->i_count);
		spin_unlock(&inode->i_lock);
		spin_unlock(&sb->s_inode_list_lock);

		invalidate_mapping_pages(inode->i_mapping, 0, -1);
		iput(toput_inode);
		toput_inode = inode;

		cond_resched();
		spin_lock(&sb->s_inode_list_lock);
	}

	spin_unlock(&sb->s_inode_list_lock);
	iput(toput_inode);
}

typedef void (*iterate_supers_func)(void (*)(struct super_block *, void *), void *);
void xring_drop_pagecache(void)
{
	iterate_supers_func iterate_supers = (iterate_supers_func)android_debug_symbol(ADS_ITERATE_SUPERS);

	iterate_supers(drop_pagecache_sb, NULL);
}
EXPORT_SYMBOL(xring_drop_pagecache);

#if KERNEL_VERSION(5, 15, 0) >= LINUX_VERSION_CODE
unsigned long xring_reclaim(unsigned long toreclaim_page)
{
	unsigned long reclaimed_page;
	struct mem_cgroup *memcg = NULL;
	gfp_t gfp_mask = GFP_KERNEL;
	bool may_swap = false;

	reclaimed_page = try_to_free_mem_cgroup_pages(memcg, toreclaim_page, gfp_mask, may_swap);
	return reclaimed_page;
}
EXPORT_SYMBOL(xring_reclaim);
#else
typedef unsigned long (*try_to_free_pages_func)(struct zonelist *zonelist, int order,
				gfp_t gfp_mask, nodemask_t *nodemask);
unsigned long xring_reclaim(unsigned long toreclaim_page)
{
	unsigned long progress;
	struct zonelist *zonelist = node_zonelist(numa_node_id(), GFP_HIGHUSER_MOVABLE);
	int order = get_order(toreclaim_page);
	gfp_t gfp_mask = GFP_HIGHUSER_MOVABLE;
	nodemask_t *nodemask = NULL;

	current->flags |= PF_MEMALLOC;
	try_to_free_pages_func try_to_free_pages = (try_to_free_pages_func)android_debug_symbol(ADS_FREE_PAGES);

	progress = try_to_free_pages(zonelist, order, gfp_mask, nodemask);
	current->flags &= ~PF_MEMALLOC;

	return progress;
}
EXPORT_SYMBOL(xring_reclaim);
#endif

enum zone_type xring_gfp_zone(gfp_t flags)
{
	enum zone_type z;
	gfp_t local_flags = flags;
	int bit;

	bit = (__force int) ((local_flags) & GFP_ZONEMASK);

	z = (GFP_ZONE_TABLE >> (bit * GFP_ZONES_SHIFT)) &
					 ((1 << GFP_ZONES_SHIFT) - 1);
	VM_BUG_ON((GFP_ZONE_BAD >> bit) & 1);
	return z;
}
EXPORT_SYMBOL(xring_gfp_zone);

typedef enum compact_result (*try_to_compact_pages_func)(gfp_t gfp_mask, unsigned int order,
		unsigned int alloc_flags, const struct alloc_context *ac,
		enum compact_priority prio, struct page **capture);
void xring_compact(int model, unsigned int order,
				unsigned long nrpages)
{
	enum compact_result compact_result;
	gfp_t gfp_mask = GFP_KERNEL;
	unsigned int alloc_flags = ALLOC_WMARK_MIN;
	struct alloc_context ac = {};
	enum compact_priority prio = COMPACT_PRIO_SYNC_FULL;
	struct page *page = NULL;
	try_to_compact_pages_func try_to_compact_pages = (try_to_compact_pages_func)android_debug_symbol(
		ADS_COMPACT_PAGES);

	/* call from userspace, order = -1 */
	order = -1;
	ac.highest_zoneidx = xring_gfp_zone(gfp_mask);
	ac.zonelist = node_zonelist(0, gfp_mask);
	compact_result = try_to_compact_pages(gfp_mask, order, alloc_flags, &ac, prio, &page);
}
EXPORT_SYMBOL(xring_compact);

static bool __xring_zone_watermark_ok_safe(struct zone *z, unsigned int order, unsigned long mark,
					     int highest_zoneidx, long free_pages)
{
	long min = mark;
	long unusable_free;
	int o;

	/*
	 * Access to high atomic reserves is not required, and CMA should not be
	 * used, since these allocations are non-movable.
	 */
	unusable_free = ((1 << order) - 1) + z->nr_reserved_highatomic;
#ifdef CONFIG_CMA
	unusable_free += zone_page_state(z, NR_FREE_CMA_PAGES);
#endif

	/* free_pages may go negative - that's OK */
	free_pages -= unusable_free;

	/*
	 * Check watermarks for an order-0 allocation request. If these
	 * are not met, then a high-order request also cannot go ahead
	 * even if a suitable page happened to be free.
	 *
	 * 'min' can be taken as 'mark' since we do not expect these allocations
	 * to require disruptive actions (such as running the OOM killer) or
	 * a lot of effort.
	 */
	if (free_pages <= min + z->lowmem_reserve[highest_zoneidx])
		return false;

	/* If this is an order-0 request then the watermark is fine */
	if (!order)
		return true;

	/* For a high-order request, check at least one suitable page is free */
	for (o = order; o < MAX_ORDER; o++) {
		struct free_area *area = &z->free_area[o];
		int mt;

		if (!area->nr_free)
			continue;

		for (mt = 0; mt < MIGRATE_PCPTYPES; mt++) {
#ifdef CONFIG_CMA
			/*
			 * Note that this check is needed only
			 * when MIGRATE_CMA < MIGRATE_PCPTYPES.
			 */
			if (mt == MIGRATE_CMA)
				continue;
#endif
			if (!list_empty(&area->free_list[mt]))
				return true;
		}
	}

	return false;
}

/* Based on zone_watermark_ok_safe from mm/page_alloc.c since it is not exported. */
static bool xring_zone_watermark_ok_safe(struct zone *z, unsigned int order,
						unsigned long mark, int highest_zoneidx)
{
	long free_pages = zone_page_state(z, NR_FREE_PAGES);

	if (z->percpu_drift_mark && free_pages < z->percpu_drift_mark)
		free_pages = zone_page_state_snapshot(z, NR_FREE_PAGES);

	return __xring_zone_watermark_ok_safe(z, order, mark, highest_zoneidx, free_pages);
}

bool xring_zone_watermark_ok(enum zone_type classzone_idx, unsigned int order)
{
	struct zone *zone;
	int i, mark;

	for (i = classzone_idx; i >= 0; i--) {
		zone = &NODE_DATA(numa_node_id())->node_zones[i];

		if (!strcmp(zone->name, "DMA32"))
			continue;

		mark = high_wmark_pages(zone);
		mark += 1 << order;
		if (!xring_zone_watermark_ok_safe(zone, order, mark, classzone_idx))
			return false;
	}

	return true;
}
EXPORT_SYMBOL(xring_zone_watermark_ok);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring MEM ADAPTER Driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(MINIDUMP);
