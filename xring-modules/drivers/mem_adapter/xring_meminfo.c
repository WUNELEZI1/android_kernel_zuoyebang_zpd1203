// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring Mem adapter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/module.h>
#include <linux/mmzone.h>
#include <linux/mm.h>
#include <linux/vmstat.h>
#include <linux/swap.h>

#include "xring_mem_adapter.h"

static unsigned int zram_percentage = 100;

struct pglist_data *first_online_pgdat(void)
{
	return NODE_DATA(first_online_node);
}

struct pglist_data *next_online_pgdat(struct pglist_data *pgdat)
{
#if MAX_NUMNODES > 1
	int nid = next_online_node(pgdat->node_id);

	return NODE_DATA(nid);
#else
	return NULL;
#endif
}

unsigned long xr_get_totalreserve_pages(void)
{
	struct pglist_data *pgdat = NULL;
	unsigned long totalreserve_pages = 0;
	enum zone_type i, j;

	for_each_online_pgdat(pgdat) {

		pgdat->totalreserve_pages = 0;

		for (i = 0; i < MAX_NR_ZONES; i++) {
			struct zone *zone = pgdat->node_zones + i;
			long max = 0;
			unsigned long managed_pages = zone_managed_pages(zone);

			/* Find valid and maximum lowmem_reserve in the zone */
			for (j = i; j < MAX_NR_ZONES; j++) {
				if (zone->lowmem_reserve[j] > max)
					max = zone->lowmem_reserve[j];
			}

			/* we treat the high watermark as reserved pages. */
			max += high_wmark_pages(zone);

			if (max > managed_pages)
				max = managed_pages;

			pgdat->totalreserve_pages += max;

			totalreserve_pages += max;
		}
	}
	return totalreserve_pages;
}
EXPORT_SYMBOL(xr_get_totalreserve_pages);

unsigned long xr_get_free_page(void)
{
	unsigned long free_pages = 0;
	unsigned long totalreserve_pages = 0;

	free_pages = global_zone_page_state(NR_FREE_PAGES);
	totalreserve_pages = xr_get_totalreserve_pages();
	if (free_pages > totalreserve_pages)
		free_pages -= totalreserve_pages;

	xrmem_info("free_pages:%ld, totalreserve_pages:%ld\n", free_pages, totalreserve_pages);

	return free_pages;
}
EXPORT_SYMBOL(xr_get_free_page);

unsigned long xr_get_file_page(void)
{
	return global_node_page_state(NR_FILE_PAGES) -
		    global_node_page_state(NR_SHMEM) -
		    global_node_page_state(NR_UNEVICTABLE) -
		    total_swapcache_pages();
}
EXPORT_SYMBOL(xr_get_file_page);

unsigned long xr_get_anon_page(void)
{
	return global_node_page_state(NR_ACTIVE_ANON) +
			global_node_page_state(NR_INACTIVE_ANON);
}
EXPORT_SYMBOL(xr_get_anon_page);

unsigned long xr_get_zram_page(void)
{
	struct sysinfo i;

	/* get used swap pages */
	si_swapinfo(&i);

	return (unsigned long)(i.totalswap - i.freeswap);
}
EXPORT_SYMBOL(xr_get_zram_page);

unsigned long xr_cpa_get_zram_watermark_page(void)
{
	struct sysinfo i;
	unsigned int cpa_zram_ratio;
	long total_swap_pages;

	/* get used swap pages */
	si_swapinfo(&i);

	total_swap_pages = i.totalswap - i.freeswap + get_nr_swap_pages();
	if (total_swap_pages > (totalram_pages() * zram_percentage / 100))
		cpa_zram_ratio = 20;
	else
		cpa_zram_ratio = 75;

	xrmem_info("cpa_zram_ratio: %u\n", cpa_zram_ratio);

	return (total_swap_pages * cpa_zram_ratio / 100);
}
EXPORT_SYMBOL(xr_cpa_get_zram_watermark_page);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring OS_ADAPTER Driver");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(MINIDUMP);
