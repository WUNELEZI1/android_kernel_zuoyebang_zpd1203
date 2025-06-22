/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __XRING_MEM_ADAPTER_H__
#define __XRING_MEM_ADAPTER_H__
#include <linux/dma-direction.h>
#include <linux/dma-buf.h>

#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#endif

enum mm_event_code {
	MM_CMA_HEAP_ERR,
	MM_CMA_HEAP_TIMEOUT,
	MM_CPA_HEAP_ERR,
	MM_CPA_HEAP_TIMEOUT,
	MM_SYSTEM_HEAP_ERR,
	MM_SYSTEM_HEAP_TIMEOUT,
	MM_LEAK_SLAB,
	MM_LEAK_VMALLOC,
	NR_MM_EVENT
};

/* xring_killer.c */
unsigned long xr_direct_kill(int model, unsigned long reclaim_by_kill);

/* xring_reclaim.c */
void xring_drop_slabcache(void);
void xring_drop_pagecache(void);
unsigned long xring_reclaim(unsigned long toreclaim_page);
void xring_compact(int model, unsigned int order,
				unsigned long nrpages);
bool xring_zone_watermark_ok(enum zone_type classzone_idx, unsigned int order);
enum zone_type xring_gfp_zone(gfp_t flags);

/* xring_pgtable.c */
void xring_change_secpage_range(phys_addr_t phys, unsigned long addr, unsigned long size, pgprot_t prot);

unsigned long xr_get_file_page(void);
unsigned long xr_get_anon_page(void);
unsigned long xr_get_free_page(void);
unsigned long xr_get_zram_page(void);
unsigned long xr_cpa_get_zram_watermark_page(void);

unsigned long xring_direct_kill(int model, unsigned long reclaim_by_kill);
int xring_heap_dma_map(struct device *dev, struct dma_buf *dmabuf,
	struct sg_table *sgt, enum dma_data_direction dir, unsigned long attrs);
void xring_heap_dma_unmap(struct device *dev, struct dma_buf *dmabuf,
	struct sg_table *sgt, enum dma_data_direction dir, unsigned long attrs);
void xr_change_page_flag(struct sg_table *table, bool set);
struct task_struct *find_task_mm(struct task_struct *p);

/* mm_event/mm_event.c */
void xring_memory_event_report(unsigned int code);

#endif
