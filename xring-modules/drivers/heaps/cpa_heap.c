// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF CPA heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION carveout heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/dma-heap.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/genalloc.h>
#include <linux/mmzone.h>
#include <linux/vmstat.h>
#include <linux/swap.h>
#include <linux/freezer.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/cma.h>

#include <asm/ptrace.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/pgtable-types.h>
#include <asm/pgtable-prot.h>
#include <linux/highmem.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/xhee_hvc/hvc_id.h>

#include "linux/dccp.h"
#include "xr_dmabuf_pagepool.h"
#include "xring_heaps.h"
#include "xr_heap_tee_ops.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "soc/xring/xring_mem_adapter.h"

#define CPA_SIZE_320M_PAGE		81920
#define CPA_WATER_MARK_RATIO		50
#define CPA_KP_ZRAM_MODEL		1
#define NUM_ORDERS ARRAY_SIZE(orders)
#define HIGH_ORDER_GFP ((GFP_KERNEL | __GFP_ZERO | __GFP_NOWARN | __GFP_NORETRY) &	\
						~__GFP_DIRECT_RECLAIM)
#define LOW_ORDER_GFP (GFP_KERNEL | __GFP_NOWARN)

static unsigned int orders[] = {9, 0};
static gfp_t order_flags[] = {LOW_ORDER_GFP, LOW_ORDER_GFP};

#if IS_ENABLED(CONFIG_XRING_HEAP_ALLOC_MAX_TIMEOUT)
static unsigned int cpa_alloc_timeout = INT_MAX; // ms
#else
static unsigned int cpa_alloc_timeout = 3000; // ms
#endif

module_param(cpa_alloc_timeout, uint, 0644);

struct dma_heap {
	const char *name;
	const struct dma_heap_ops *ops;
	void *priv;
	dev_t heap_devt;
	struct list_head list;
	struct cdev heap_cdev;
};

struct cpa_cma_info {
	struct list_head pages;
	unsigned long num;
	unsigned long orders_count[NUM_ORDERS];
};

struct dmabuf_cma_pool {
	struct list_head pages;
	struct mutex mutex;
	int count;
	unsigned int order;
};

struct cpa_heap_buffer {
	struct xr_heap_buffer_header header;
	struct cpa_heap *cpa_heap;
	struct sg_table *sg_table;
	struct list_head attachments;
	struct mutex lock;
	size_t size;
	u32 sfd;
	atomic_t TEE_set;
	void (*free_callback)(struct cpa_heap_buffer *buffer);
};

struct cpa_heap {
	struct dma_heap *heap;
	struct cma *cma;
	struct mutex mutex;
	size_t heap_size;
	size_t heap_scene;
	atomic64_t alloc_size;
	atomic64_t cma_alloc_size;
	struct device *dev;
	struct dmabuf_cma_pool *pools[NUM_ORDERS];
	u32 heap_attr;
};

struct dma_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
	bool mapped;
};

struct cpa_work_data {
	struct work_struct work;
	size_t size;
	struct cpa_heap *cpa_heap;
};

static struct cpa_work_data *cpa_mem_work_data;

static void xr_prepare_memory_for_alloc(struct cpa_heap *cpa_heap,
					unsigned long size,
					unsigned long *kill_rem_page,
					unsigned long *reclaimed_page);

static int order_to_index(unsigned int order)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (order == orders[i])
			return i;
	}
	return -1;
}

static size_t order_to_size(unsigned int order)
{
	return (size_t)PAGE_SIZE << order;
}

static unsigned int size_to_order(size_t size)
{
	unsigned int order = 0;
	size_t page_size = PAGE_SIZE;

	while (size > page_size) {
		page_size <<= 1;
		order++;
	}
	return order;
}

u32 xr_cpa_get_sfd(struct dma_buf *dmabuf)
{
	struct cpa_heap_buffer *buffer = NULL;
	struct cpa_heap *heap = NULL;

	buffer = dmabuf->priv;
	if (!buffer) {
		xrheap_err("dmabuf has no dma buffer!\n");
		return 0;
	}

	heap = buffer->cpa_heap;
	if (!heap) {
		xrheap_err("invalid dma buffer's heap!\n");
		return 0;
	}

	return buffer->sfd;
}
EXPORT_SYMBOL(xr_cpa_get_sfd);

static void xr_prepare_fill_pool(struct cpa_heap *cpa_heap, size_t size)
{
	struct sg_table *table = NULL;
	size_t size_remaining = ALIGN(size, order_to_size(orders[1]));
	int ret = 0;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return;

	ret = xr_dmabuf_pagepool_alloc_for_buddy(table, size_remaining, orders[1]);
	if (ret < 0) {
		xrheap_debug("fill less than size_remaining\n");
		goto alloc_fail;
	}

	xr_dmabuf_pagepool_free(table);

alloc_fail:
	kfree(table);
}

static void cpa_prepare_memory_for_work(struct work_struct *work)
{
	unsigned long kill_rem_page = 0;
	unsigned long reclaimed_page = 0;
	struct cpa_work_data *work_data = container_of(work, struct cpa_work_data, work);
	size_t size = work_data->size;

	xr_prepare_memory_for_alloc(work_data->cpa_heap, size, &kill_rem_page, &reclaimed_page);
	xrheap_info("kill process free mempage: %lu, reclaimed mempage: %lu\n",
			kill_rem_page, reclaimed_page);

	xr_prepare_fill_pool(work_data->cpa_heap, size);
	xrheap_info("fill size_remaining, pool size: %ld\n", xr_dmabuf_pagepool_get_size_per_heap());
}

int xr_cpa_pre_alloc(void *heap, size_t size, int action)
{
	struct cpa_heap *cpa_heap = (struct cpa_heap *)heap;
	long pool_size = xr_dmabuf_pagepool_get_size_per_heap();
	size_t heap_size = cpa_heap->heap_size;
	size_t alloc_size = 0;
	int ret = 0;

	alloc_size = atomic64_read(&cpa_heap->alloc_size);
	if (pool_size + alloc_size >= cpa_heap->heap_size || size >= cpa_heap->heap_size) {
		xrheap_info("already have enough memory --- info:\n"
				"\t\t\t\t size : %lu\n"
				"\t\t\t\t heap size : %lu\n"
				"\t\t\t\t alloc size : %lu\n"
				"\t\t\t\t pool size : %ld\n",
				size,
				heap_size,
				alloc_size,
				pool_size);
		return 0;
	}

	switch (action) {
	case XR_ACTION_PREALLOC_START:
		cpa_mem_work_data->size = ALIGN(size, order_to_size(orders[1]));
		queue_work(system_highpri_wq, &cpa_mem_work_data->work);
		break;
	default:
		xrheap_err("Bad Actions:%u!\n", action);
		ret = -EINVAL;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(xr_cpa_pre_alloc);

static u64 get_alloc_size_cpa_heap(void *data)
{
	struct cpa_heap *cpa_heap = (struct cpa_heap *)data;

	return atomic64_read(&cpa_heap->alloc_size);
}

static u64 get_cma_consumed_cpa_heap(void *data)
{
	struct cpa_heap *cpa_heap = (struct cpa_heap *)data;

	return atomic64_read(&cpa_heap->cma_alloc_size);
}

static u64 get_pool_cached_cpa_heap(void *data)
{
	return xr_dmabuf_pagepool_get_size_per_heap();
}

static u64 get_buddy_consumed_cpa_heap(void *data)
{
	return get_alloc_size_cpa_heap(data)
			+ get_pool_cached_cpa_heap(data)
			- get_cma_consumed_cpa_heap(data);
}

static struct page *dmabuf_cmapool_remove(struct dmabuf_cma_pool *pool)
{
	struct page *page = NULL;

	mutex_lock(&pool->mutex);
	if (pool->count) {
		page = list_first_entry(&pool->pages, struct page, lru);
		pool->count--;
		list_del(&page->lru);
	}
	mutex_unlock(&pool->mutex);

	return page;
}

static struct page *alloc_cma_page(struct cpa_heap *cpa_heap, unsigned int order)
{
	struct dmabuf_cma_pool *pool = NULL;
	struct page *page = NULL;
	int index = order_to_index(order);

	if (index < 0)
		return NULL;

	pool = cpa_heap->pools[index];

	page = dmabuf_cmapool_remove(pool);
	if (!page) {
		page = cma_alloc(cpa_heap->cma, 1 << order, orders[1], false);
		if (!page)
			return NULL;
		atomic64_add((1LL << order) << PAGE_SHIFT, &cpa_heap->cma_alloc_size);

		memset(page_address(page), 0, order_to_size(order));

		xrheap_debug("success --- cma_alloc info:\n"
				"\t\t\t\t alloc size: %ld\n"
				"\t\t\t\t number of pages:%d\n"
				"\t\t\t\t check size/order:%ld\n"
				"\t\t\t\t check pool info:pool[%d]->count is %d\n",
				order_to_size(order),
				1 << order,
				order_to_size(order)/(1 << order),
				index,
				pool->count);
		return page;
	}

	return page;
}

static struct page *alloc_largest_available(struct cpa_heap *cpa_heap,
					    unsigned long ac_sz,
					    unsigned int max_order,
					    unsigned int *current_order)
{
	unsigned int i = 0;
	struct page *cma_page = NULL;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (ac_sz < order_to_size(orders[i]))
			continue;

		if (max_order < orders[i])
			continue;

		cma_page = alloc_cma_page(cpa_heap, orders[i]);
		if (!cma_page)
			continue;

		*current_order = orders[i];

		return cma_page;
	}

	xrheap_err(" --- fail\n");

	return NULL;
}

static size_t cma_zalloc_large(struct cpa_heap *cpa_heap,
				     struct cpa_cma_info *info,
				     size_t size)
{
	struct page *cma_page = NULL;
	unsigned int max_order = orders[0];
	size_t size_remaining = size;
	unsigned int current_order = 0;
	int index = 0;

	while (size_remaining > 0) {
		if (fatal_signal_pending(current))
			goto fatal_signal;

		cma_page = alloc_largest_available(cpa_heap,
					       size_remaining,
					       max_order,
					       &current_order);
		if (!cma_page)
			break;
		max_order = current_order;
		index = order_to_index(max_order);
		if (index < 0) {
			xrheap_err("fail --- index is a negative number:%d\n", index);
			break;
		}
		list_add_tail(&cma_page->lru, &info->pages);
		info->num++;
		info->orders_count[index]++;
		size_remaining -= order_to_size(max_order);
	}

	xrheap_debug(" --- success info:\n"
			"\t\t\t\t size_remaining is %ld\n"
			"\t\t\t\t cpa_cma_info num is 0x%lx\n"
			"\t\t\t\t info.orders_count:%ld, %ld\n",
			size_remaining,
			info->num,
			info->orders_count[0],
			info->orders_count[1]);

fatal_signal:
	return size_remaining;
}

static int xr_dmabuf_cmapool_alloc(struct cpa_heap *cpa_heap, struct sg_table *table,
							size_t size_remaining)
{
	struct page *page = NULL;
	struct page *tmp_page = NULL;
	struct scatterlist *sg = NULL;
	struct cpa_cma_info info;
	int ret = 0;

	info.num = 0;
	memset(info.orders_count, 0, sizeof(info.orders_count));
	INIT_LIST_HEAD(&info.pages);

	size_remaining = cma_zalloc_large(cpa_heap,
					  &info, size_remaining);
	if (size_remaining) {
		xrheap_err("fail -- size_remaining size is %zu\n", size_remaining);
		ret = -ENOMEM;
		goto free_cma_pages;
	}

	ret = sg_alloc_table(table, info.num, GFP_KERNEL);
	if (ret < 0) {
		ret = -ENOMEM;
		goto free_cma_pages;
	}

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &info.pages, lru) {
		if (info.orders_count[0]) {
			sg_set_page(sg, page, order_to_size(orders[0]), 0);
			info.orders_count[0]--;
		} else
			sg_set_page(sg, page, order_to_size(orders[1]), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	xrheap_info(" --- success:\n"
			"\t\t\t\t info nums: %ld\n"
			"\t\t\t\t info orders count: %ld, %ld\n",
			info.num,
			info.orders_count[0],
			info.orders_count[1]);
	return ret;

free_cma_pages:
	list_for_each_entry_safe(page, tmp_page, &info.pages, lru) {
		if (info.orders_count[0]) {
			cma_release(cpa_heap->cma, page, 1 << orders[0]);
			info.orders_count[0]--;
			atomic64_sub((1LL << orders[0]) << PAGE_SHIFT, &cpa_heap->cma_alloc_size);
		} else {
			cma_release(cpa_heap->cma, page, 1 << orders[1]);
			atomic64_sub((1LL << orders[1]) << PAGE_SHIFT, &cpa_heap->cma_alloc_size);
		}
	}

	return ret;
}

static void xr_dmabuf_cmapool_destroy(struct dmabuf_cma_pool **pools)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++)
		kfree(pools[i]);
}

struct dmabuf_cma_pool *dmabuf_cmapool_create(unsigned int order)
{
	struct dmabuf_cma_pool *pool = kzalloc(sizeof(*pool), GFP_KERNEL);

	if (!pool)
		return NULL;
	pool->count = 0;
	INIT_LIST_HEAD(&pool->pages);
	pool->order = order;
	mutex_init(&pool->mutex);

	return pool;
}

static int xr_dmabuf_cmapool_create(struct cpa_heap *cpa_heap)
{
	unsigned int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		struct dmabuf_cma_pool *pool = NULL;

		pool = dmabuf_cmapool_create(orders[i]);
		if (!pool)
			goto err_create_pool;
		cpa_heap->pools[i] = pool;
	}

	return 0;

err_create_pool:
	xr_dmabuf_cmapool_destroy(cpa_heap->pools);

	return -ENOMEM;
}

static void free_cma_page(struct cpa_heap *cpa_heap,
			     struct cpa_heap_buffer *buffer, struct scatterlist *sg)
{
	struct dmabuf_cma_pool *pool = NULL;
	unsigned int order = size_to_order(sg->length);
	struct page *page = sg_page(sg);
	int index = 0;

	index = order_to_index(order);
	if (index < 0) {
		xrheap_err("fail --- index is a negative number:%d\n", index);
		return;
	}

	pool = cpa_heap->pools[index];

	/*
	 * if need to use cma pool;
	 * now using the free func as below:
	 * dmabuf_cmapool_add(pool, page);
	 */
	xrheap_info("success --- pool->count is %d, pool->order is %u, order is %u\n",
			pool->count, pool->order, order);
	cma_release(cpa_heap->cma, page, 1 << order);
	atomic64_sub((1LL << order) << PAGE_SHIFT, &cpa_heap->cma_alloc_size);
}

static void xr_dmabuf_cmapool_free(struct cpa_heap_buffer *buffer)
{
	struct cpa_heap *cpa_heap = buffer->cpa_heap;
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	int i = 0;

	for_each_sg(table->sgl, sg, table->nents, i) {
		free_cma_page(cpa_heap, buffer, sg);
	}

	sg_free_table(buffer->sg_table);
}

void xr_cmapool_cb(struct cpa_heap_buffer *buffer)
{

	xr_dmabuf_cmapool_free(buffer);
}

void xr_pagepool_cb(struct cpa_heap_buffer *buffer)
{
	struct sg_table *table = NULL;

	table = buffer->sg_table;

	xr_dmabuf_pagepool_free(table);
}

static int cpa_input_check(struct cpa_heap *cpa_heap,
				  unsigned long size)
{
	long alloc_size;

	alloc_size = atomic64_read(&cpa_heap->alloc_size);
	if (alloc_size + size <= alloc_size) {
		xrheap_err("fail --- size overflow! alloc_size=0x%llx, size=0x%lx,\n",
			atomic64_read(&cpa_heap->alloc_size), size);
		return -EINVAL;
	}

	if ((alloc_size + size) > cpa_heap->heap_size) {
		xrheap_err("fail --- alloc size = 0x%lx, size = 0x%lx, heap size = 0x%lx\n",
		       alloc_size, size,
		       cpa_heap->heap_size);
		return -EINVAL;
	}

	if (size > (SZ_64M + order_to_size(orders[1]))) {
		xrheap_err("fail --- size too large! size 0x%lx, per_alloc_sz 0x4000000(64M) + 0x%lx\n",
		       size, order_to_size(orders[1]));
		return -EINVAL;
	}

	return 0;
}

static inline unsigned long cpa_water_mark_page(struct cpa_heap *cpa_heap)
{
	unsigned long alloc_size = (unsigned long)atomic64_read(&cpa_heap->alloc_size);

	return (((cpa_heap->heap_size - alloc_size) * 100 / CPA_WATER_MARK_RATIO) >> PAGE_SHIFT);
}

static void xr_prepare_memory_for_alloc(struct cpa_heap *cpa_heap,
					unsigned long size,
					unsigned long *kill_rem_page,
					unsigned long *reclaimed_page)
{
	unsigned long water_mark_page = 0;
	unsigned long tokill_rem_page = 0;
	unsigned long toreclaim_page = 0;
	unsigned long free_page = 0;
	unsigned long file_page = 0;
	unsigned long file_water_mark_page = 0;
	unsigned long zram_page = 0;
	unsigned long zram_water_mark_page = 0;
	unsigned long nr_expect = 0;

	water_mark_page = cpa_water_mark_page(cpa_heap);
	tokill_rem_page = (size / PAGE_SIZE);
	toreclaim_page = tokill_rem_page * 100 / CPA_WATER_MARK_RATIO;
	file_water_mark_page = CPA_SIZE_320M_PAGE;
	free_page = xr_get_free_page();
	file_page = xr_get_file_page();

	xrheap_debug("step drop pagecache before, free_page: %lu\n"
			"water_mark_page: %lu; file_page: %lu, file_page_water_mark: %lu\n",
			free_page, water_mark_page,
			file_page, file_water_mark_page);

	if ((free_page < water_mark_page) &&
	    (file_page > file_water_mark_page)) {
		xring_drop_pagecache();
		xring_drop_slabcache();
	}

	zram_water_mark_page = xr_cpa_get_zram_watermark_page();
	zram_page = xr_get_zram_page();

	xrheap_debug("step kill proc before, zram_page: %lu, zram_water_mark_page: %lu\n",
			zram_page, zram_water_mark_page);

	if (zram_page > zram_water_mark_page) {
		*kill_rem_page = xring_direct_kill(CPA_KP_ZRAM_MODEL, tokill_rem_page);
		if (!*kill_rem_page)
			xrheap_err("prepare memory kill process fail\n");
	}

	if (free_page < water_mark_page) {
		*reclaimed_page = xring_reclaim(toreclaim_page);
		if (!*reclaimed_page)
			xrheap_err("prepare memory reclaim memory fail\n");
	}

	if (*kill_rem_page || *reclaimed_page) {
		nr_expect = toreclaim_page >> orders[1];
		xrheap_debug("compact nr_expect page: %lu", nr_expect);

		xring_compact(1, orders[1], nr_expect);
	}

	xrheap_debug("kill process free mempage:%lu, reclaimed mempage:%lu\n",
			*kill_rem_page, *reclaimed_page);
}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
static void cpa_change_pte_prot(struct sg_table *table,
					pgprot_t prot)
{
	struct scatterlist *sg = NULL;
	int i = 0;

	for_each_sgtable_sg(table, sg, i) {
		struct page *page = sg_page(sg);

		xring_change_secpage_range(page_to_phys(page),
				(unsigned long)page_address(page),
				sg->length, prot);
	}

	flush_tlb_all();
}
#endif

static int cpa_change_mem_prot(struct cpa_heap *cpa_heap,
				struct cpa_heap_buffer *buffer, u32 cmd)
{
	struct sg_table *table = buffer->sg_table;
	struct tz_info *info = NULL;
	struct tz_pageinfo *pageinfo = NULL;
	unsigned int nents = table->nents;
	struct scatterlist *sg = NULL;
	int ret = 0;
	int i = 0;
	unsigned long total = 0;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		ret = -ENOMEM;
		return ret;
	}

	total = roundup(nents * sizeof(*pageinfo), PAGE_SIZE);
	pageinfo = kzalloc(total, GFP_KERNEL);
	if (!pageinfo) {
		ret = -ENOMEM;
		goto free_info;
	}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	if (cmd == XRING_SEC_CMD_ALLOC) {
		for_each_sgtable_sg(table, sg, i) {
			dma_sync_single_for_device(cpa_heap->dev, sg_phys(sg),
						sg->length, DMA_BIDIRECTIONAL);
		}
		cpa_change_pte_prot(table,
				__pgprot(PROT_DEVICE_nGnRE));
	}
#endif

	for_each_sgtable_sg(table, sg, i) {
		struct page *page = sg_page(sg);

		pageinfo[i].addr = page_to_phys(page);
		pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
		xrheap_err("size: 0x%x\n", sg->length);
	}

	info->pageinfo = pageinfo;
	info->size = total;
	info->nents = nents;

	if (cmd == XRING_SEC_CMD_ALLOC) {
		ret = xr_xhee_action(info, FID_XHEE_MEM_LEND);
		if (ret < 0) {
			xrheap_err("XHEE excute failed\n");
			ret = -ENOMEM;
			goto tee_excute_failed;
		}
	}

	if (cmd == XRING_SEC_CMD_FREE)
		info->sfd = buffer->sfd;

	info->type = CPA_HEAP;
	ret = xr_change_mem_prot_tee(info, cmd);
	if (ret < 0) {
		xrheap_err("TEE excute failed\n");
		ret = -ENOMEM;
		goto tee_excute_failed;
	}

	if (cmd == XRING_SEC_CMD_FREE) {
		ret = xr_xhee_action(info, FID_XHEE_MEM_RECLAIM);
		if (ret < 0) {
			xrheap_err("XHEE excute failed\n");
			ret = -ENOMEM;
			goto tee_excute_failed;
		}
	}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	if (cmd == XRING_SEC_CMD_FREE)
		cpa_change_pte_prot(table, PAGE_KERNEL);
#endif

	if (cmd == XRING_SEC_CMD_ALLOC) {
		buffer->sfd = info->sfd;
		xrheap_info("sfd: 0x%x\n", buffer->sfd);
	}

	kfree(pageinfo);
	kfree(info);

#ifdef CONFIG_XRING_HEAP_NOC_DFX
	if (cmd & XRING_SET_PROTECT_OR_SAFE)
		xr_change_page_flag(table, true);
	else if (cmd & XRING_CLEAR_PROTECT_OR_SAFE)
		xr_change_page_flag(table, false);
#endif

	return 0;

tee_excute_failed:
#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	if (cmd == XRING_SEC_CMD_ALLOC) {
		for_each_sgtable_sg(table, sg, i) {
			dma_sync_single_for_device(cpa_heap->dev, sg_phys(sg),
				sg->length, DMA_BIDIRECTIONAL);
		}
		cpa_change_pte_prot(table, PAGE_KERNEL);
	}
#endif
	kfree(pageinfo);
free_info:
	kfree(info);
	return ret;
}

static int cpa_heap_allocate(struct dma_heap *heap, struct cpa_heap_buffer *buffer,
				struct cpa_heap *cpa_heap, unsigned long size)
{
	struct sg_table *table = NULL;

	unsigned long kill_rem_page = 0;
	unsigned long reclaimed_page = 0;
	size_t size_remaining = ALIGN(size, order_to_size(orders[1]));
	int ret = 0;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		ret = -ENOMEM;
		goto out;
	}

	mutex_lock(&cpa_heap->mutex);
	ret = cpa_input_check(cpa_heap, size);
	if (ret < 0) {
		mutex_unlock(&cpa_heap->mutex);
		xrheap_err("cpa alloc input check: params err!\n");
		ret = -ENODATA;
		goto free_table;
	}

	atomic64_add(size_remaining, &cpa_heap->alloc_size);

	mutex_unlock(&cpa_heap->mutex);

	xr_prepare_memory_for_alloc(cpa_heap, size, &kill_rem_page, &reclaimed_page);

	buffer->free_callback = xr_pagepool_cb;
	ret = xr_dmabuf_pagepool_alloc(table, size_remaining, orders[1]);
	if (ret != 0) {
		buffer->free_callback = xr_cmapool_cb;
		ret = xr_dmabuf_cmapool_alloc(cpa_heap, table, size_remaining);
		if (ret != 0) {
			xrheap_err("fail --- cma alloc\n");
			goto sub_size;
		}
	}

	buffer->sg_table = table;

	ret = cpa_change_mem_prot(cpa_heap, buffer, XRING_SEC_CMD_ALLOC);
	if (ret < 0)
		goto free_pages;

	xrheap_info("success --- info:\n"
			"\t\t\t\talloc size: 0x%lx\n"
			"\t\t\t\thave already alloc size: 0x%llx\n",
			size_remaining, atomic64_read(&cpa_heap->alloc_size));

	return ret;

free_pages:
	buffer->free_callback(buffer);
sub_size:
	atomic64_sub(size_remaining, &cpa_heap->alloc_size);
free_table:
	kfree(table);
out:
	return ret;
}

static void cpa_heap_release(struct cpa_heap_buffer *buffer)
{
	struct cpa_heap *cpa_heap = buffer->cpa_heap;
	int ret = 0;

	ret = cpa_change_mem_prot(cpa_heap, buffer, XRING_SEC_CMD_FREE);
	if (WARN_ON(ret < 0)) {
		xrheap_err("TEE release fail, verify that other components are in use\n");
		return;
	}
	buffer->free_callback(buffer);
	kfree(buffer->sg_table);
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table = NULL;
	struct scatterlist *sg = NULL;
	struct scatterlist *new_sg = NULL;

	int ret = 0;
	int i = 0;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->orig_nents, GFP_KERNEL);
	if (ret < 0) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sgtable_sg(table, sg, i) {
		sg_set_page(new_sg, sg_page(sg), sg->length, sg->offset);
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static int xr_cpa_heap_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct cpa_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;
	struct sg_table *table = NULL;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	table = dup_sg_table(buffer->sg_table);
	if (IS_ERR(table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->table = table;
	a->dev = attachment->dev;
	INIT_LIST_HEAD(&a->list);
	attachment->priv = a;

#ifdef CONFIG_XRING_NEED_CMO_OPTI
	a->mapped = false;
	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);
#endif
	return 0;
}

static void xr_cpa_heap_detach(struct dma_buf *dmabuf,
			       struct dma_buf_attachment *attachment)
{
	struct dma_heap_attachment *a = attachment->priv;

#ifdef CONFIG_XRING_NEED_CMO_OPTI
	struct cpa_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);
#endif
	sg_free_table(a->table);
	kfree(a->table);
	kfree(a);

}

static struct sg_table *xr_cpa_heap_map_dma_buf(struct dma_buf_attachment *attachment,
						enum dma_data_direction direction)
{
	struct dma_heap_attachment *a = attachment->priv;
	int attr = attachment->dma_map_attrs;
	struct sg_table *table = a->table;
	int ret;

	attr |= DMA_ATTR_SKIP_CPU_SYNC;
	ret = xring_heap_dma_map(attachment->dev, attachment->dmabuf, table, direction, attr);
	if (ret)
		return ERR_PTR(ret);

#ifdef CONFIG_XRING_NEED_CMO_OPTI
	a->mapped = true;
#endif
	return table;
}

static void xr_cpa_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
					struct sg_table *table,
					enum dma_data_direction direction)
{
	int attr = attachment->dma_map_attrs;
#ifdef CONFIG_XRING_NEED_CMO_OPTI
	struct dma_heap_attachment *a = attachment->priv;
#endif

	attr |= DMA_ATTR_SKIP_CPU_SYNC;
	xring_heap_dma_unmap(attachment->dev, attachment->dmabuf, table, direction, attr);

#ifdef CONFIG_XRING_NEED_CMO_OPTI
	a->mapped = false;
#endif
}

static void xr_cpa_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct cpa_heap_buffer *buffer = dmabuf->priv;
	struct cpa_heap *cpa_heap = buffer->cpa_heap;
	struct sg_table *table = buffer->sg_table;
	int ret;

	size_t size_remaining = ALIGN(buffer->size, order_to_size(orders[1]));

	ret = cpa_change_mem_prot(cpa_heap, buffer, XRING_SEC_CMD_FREE);
	if (ret < 0) {
		WARN_ON("TEE release fail, verify that other components are in use\n");
		return;
	}

	if (buffer->free_callback)
		buffer->free_callback(buffer);

	atomic64_sub(size_remaining, &cpa_heap->alloc_size);

	kfree(table);
	kfree(buffer);
}

static const struct dma_buf_ops cpa_heap_buf_ops = {
	.attach = xr_cpa_heap_attach,
	.detach = xr_cpa_heap_detach,
	.map_dma_buf = xr_cpa_heap_map_dma_buf,
	.unmap_dma_buf = xr_cpa_heap_unmap_dma_buf,
	.release = xr_cpa_heap_dma_buf_release,
};

static struct dma_buf *xr_cpa_heap_do_allocate(struct dma_heap *heap,
					    unsigned long len,
					    u32 fd_flags,
					    u64 heap_flags)
{
	struct cpa_heap *cpa_heap = heap->priv;
	struct cpa_heap_buffer *buffer = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf = NULL;
	struct sg_table *table = NULL;

	int ret = 0;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

#ifdef CONFIG_XRING_NEED_CMO_OPTI
	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
#endif

	buffer->cpa_heap = cpa_heap;

	ret = cpa_heap_allocate(heap, buffer, cpa_heap, len);
	if (unlikely(ret)) {
		ret = -EINVAL;
		goto free_buffer;
	}

	table = buffer->sg_table;
	buffer->size = len;

	/* create the dmabuf */
	exp_info.exp_name = dma_heap_get_name(heap);
	exp_info.ops = &cpa_heap_buf_ops;
	exp_info.size = buffer->size;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		xrheap_err("dma_buf_export fail\n");
		ret = PTR_ERR(dmabuf);
		goto free_pages;
	}

	return dmabuf;

free_pages:
	cpa_heap_release(buffer);
free_buffer:
	kfree(buffer);
	return ERR_PTR(ret);
}

static struct dma_buf *xr_cpa_heap_allocate(struct dma_heap *heap, unsigned long len,
					    u32 fd_flags, u64 heap_flags)
{
	struct dma_buf *dmabuf;
	bool alloc_timeout = false;
	bool alloc_failed = false;

	KTIME_DEFINE_GET_START(start, end, duration_ms);

	dmabuf = xr_cpa_heap_do_allocate(heap, len, fd_flags, heap_flags);
	KTIME_GET_END_DURATION(start, end, duration_ms);

	alloc_timeout = duration_ms > cpa_alloc_timeout ? true : false;
	alloc_failed = IS_ERR(dmabuf) ? true : false;

	if (alloc_timeout || alloc_failed) {
		xrheap_info("len: 0x%lx bytes, duration_ms: %lld, dmabuf ok? %c:%ld\n",
			len, duration_ms, alloc_failed ? 'N' : 'Y', alloc_failed ? PTR_ERR(dmabuf) : 0);
		xr_meminfo_show(true, FROM_DMAHEAP);

		if (alloc_timeout)
			xring_memory_event_report(MM_CPA_HEAP_TIMEOUT);
		if (alloc_failed)
			xring_memory_event_report(MM_CPA_HEAP_ERR);
	}
	return dmabuf;
}

static const struct dma_heap_ops xr_cpa_heap_ops = {
	.allocate = xr_cpa_heap_allocate,
};

static void cpa_init_order(void)
{
	unsigned long totalram_page = 0;
	int i = 0;

	totalram_page = totalram_pages();

	if (totalram_page > 16 * TOTAL_RAM_PAGES_1G)
		orders[1] = 7;
	else if (totalram_page > 8 * TOTAL_RAM_PAGES_1G)
		orders[1] = 6;
	else if (totalram_page > 4 * TOTAL_RAM_PAGES_1G)
		orders[1] = 5;
	else
		orders[1] = 4;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (orders[i] >= 8)
			order_flags[i] = (HIGH_ORDER_GFP) & (~__GFP_RECLAIMABLE);
	}

	xrheap_info("success --- info:\n"
			"\t\t\t\ttotalram_pages : %ldG\n"
			"\t\t\t\tupdate order[1] : %u\n",
			totalram_page/TOTAL_RAM_PAGES_1G,
			orders[1]);

}

static int cpa_parse_dt(struct xring_heap_platform_data *data, struct cpa_heap *cpa_heap)
{
	struct device *dev = data->priv;
	struct device_node *nd = data->node;
	u32 heap_size = 0;
	u32 heap_scene = 0;
	int ret;

	cpa_heap->dev = dev;

	ret = of_property_read_u32(nd, "heap-size", &heap_size);
	if (ret < 0)
		xrheap_err("fail --- find property: heap-size\n");

	cpa_heap->heap_size = heap_size;

	ret = of_property_read_u32(nd, "heap-scene", &heap_scene);
	if (ret < 0)
		xrheap_err("fail --- find property: heap-scene\n");

	cpa_heap->heap_scene = heap_scene;

	xrheap_debug("success --- info:\n"
			"\t\t\t\theap-name : %s\n"
			"\t\t\t\theap-size : %uM\n"
			"\t\t\t\theap-scene : %uM\n",
			data->heap_name,
			heap_size / SZ_1M,
			heap_scene);

	return 0;
}

int xr_cpa_heap_create(struct xring_heap_platform_data *data)
{
	struct cpa_heap *cpa_heap = NULL;
	struct dma_heap_export_info exp_info;
	struct xr_dmabuf_helper_ops *helper_ops;
	struct cpa_work_data *cpa_work_data = NULL;

	int ret = 0;

	cpa_heap = kzalloc(sizeof(*cpa_heap), GFP_KERNEL);
	if (!cpa_heap)
		return -ENOMEM;

	mutex_init(&cpa_heap->mutex);

	cpa_init_order();
	cpa_parse_dt(data, cpa_heap);

	cpa_work_data = kzalloc(sizeof(*cpa_work_data), GFP_KERNEL);
	if (!cpa_work_data) {
		ret = -ENOMEM;
		goto free_workdata;
	}

	INIT_WORK(&(cpa_work_data->work), cpa_prepare_memory_for_work);

	ret = xr_dmabuf_pagepool_create(orders, order_flags, NUM_ORDERS);
	if (ret < 0) {
		ret = -ENOMEM;
		goto free_cpa_heap;
	}

	ret = xr_dmabuf_cmapool_create(cpa_heap);
	if (ret < 0) {
		ret = -ENOMEM;
		goto destroy_pagepools;
	}

	exp_info.name = data->heap_name;
	exp_info.ops = &xr_cpa_heap_ops;
	exp_info.priv = cpa_heap;

	helper_ops = devm_kzalloc(data->priv, sizeof(*helper_ops), GFP_KERNEL);
	helper_ops->name = exp_info.name;
	helper_ops->priv = cpa_heap;
	helper_ops->scene = cpa_heap->heap_scene;
	helper_ops->get_sfd = xr_cpa_get_sfd;
	helper_ops->change_mem_prot = NULL;
	helper_ops->pre_alloc = xr_cpa_pre_alloc;
	helper_ops->get_alloc_size = get_alloc_size_cpa_heap;
	helper_ops->get_buddy_consumed = get_buddy_consumed_cpa_heap;
	helper_ops->get_cma_consumed = get_cma_consumed_cpa_heap;
	helper_ops->get_pool_cached = get_pool_cached_cpa_heap;
	if (xr_dmabuf_helper_register(helper_ops)) {
		xrheap_err("dmabuf helper register failed\n");
		ret = -EINVAL;
		goto destroy_cmapools;
	}

	cpa_heap->cma = data->cma;
	cpa_heap->heap_attr = data->heap_attr;
	cpa_heap->heap = dma_heap_add(&exp_info);
	if (IS_ERR(cpa_heap->heap)) {
		xrheap_err("dma_heap_add fail\n");
		ret = -EINVAL;
		goto destroy_cmapools;
	}

	xrheap_info("success --- info:\n"
			"\t\t\t\t heap-name : %s\n"
			"\t\t\t\t heap-attr : %u\n"
			"\t\t\t\t heap-size : %lu MB\n"
			"\t\t\t\t cma size : 0x%lx\n",
			data->heap_name,
			cpa_heap->heap_attr,
			cpa_heap->heap_size / SZ_1M,
			data->cma_size / SZ_1M);

	cpa_work_data->cpa_heap = cpa_heap;
	cpa_mem_work_data = cpa_work_data;

	return 0;
destroy_cmapools:
	xr_dmabuf_cmapool_destroy(cpa_heap->pools);
destroy_pagepools:
	xr_dmabuf_pagepool_destroy(orders, NUM_ORDERS);
free_cpa_heap:
	kfree(cpa_work_data);
free_workdata:
	mutex_destroy(&cpa_heap->mutex);
	kfree(cpa_heap);

	return ret;
}
EXPORT_SYMBOL(xr_cpa_heap_create);
MODULE_LICENSE("GPL v2");
