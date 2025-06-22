// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Carveout heap exporter
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on dma heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/err.h>

#include "linux/mmzone.h"
#include "xring_heaps.h"
#include "page_pool.h"
#include "xr_dmabuf_pagepool.h"
#include "soc/xring/xring_mem_adapter.h"

#define INIT_THIS 1
/* The current value of MAX_ORDER in the kernel is 10. */
static struct dmabuf_page_pool *xr_dmabuf_pagepool[MAX_ORDER + 1] = {0};
static atomic_t xr_dmabuf_pagepool_refcount[MAX_ORDER + 1] = {0};
static atomic_t user_count;

int xr_dmabuf_pagepool_create(const unsigned int *orders, gfp_t *gfp_mask, unsigned int len)
{
	int i;
	int index;

	if (!orders || !gfp_mask) {
		xrheap_err("orders and gfp_mask is null\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		index = orders[i];
		if (atomic_inc_return(&xr_dmabuf_pagepool_refcount[index]) == INIT_THIS) {
			xr_dmabuf_pagepool[index] = xr_dmabuf_page_pool_create(gfp_mask[i], orders[i]);
			if (!xr_dmabuf_pagepool[index]) {
				int j;

				atomic_dec(&xr_dmabuf_pagepool_refcount[index]);
				xrheap_err("order: %d pool created failed\n", orders[i]);
				for (j = 0; j < i; j++)
					xr_dmabuf_page_pool_destroy(xr_dmabuf_pagepool[j]);
				return -EINVAL;
			}
			xrheap_info("order: %d pool created success\n", orders[i]);
		}
	}
	atomic_inc(&user_count);
	return 0;
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_create);

int xr_dmabuf_pagepool_destroy(const unsigned int *orders, unsigned int len)
{
	int i;
	int index;

	if (!orders) {
		xrheap_err("orders is null\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		index = orders[i];
		if ((atomic_dec_return(&xr_dmabuf_pagepool_refcount[index]) == 0)
			&& (!!xr_dmabuf_pagepool[index])) {
			xr_dmabuf_page_pool_destroy(xr_dmabuf_pagepool[index]);
			xr_dmabuf_pagepool[index] = 0;
		}
	}
	xr_dmabuf_pagepool_dumpinfo();
	atomic_dec(&user_count);
	return 0;
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_destroy);

static struct page *xr_pagepool_alloc_internel(unsigned int order)
{
	struct page *page = NULL;

	if (!!xr_dmabuf_pagepool[order])
		page = xr_dmabuf_page_pool_alloc(xr_dmabuf_pagepool[order]);
	return page;
}

static void xr_pagepool_free_internel(unsigned int order, struct page *page)
{
	if (order > MAX_ORDER)
		return;

	if (!!xr_dmabuf_pagepool[order])
		xr_dmabuf_page_pool_free(xr_dmabuf_pagepool[order], page);
	else
		__free_pages(page, order);
}

int xr_dmabuf_pagepool_alloc(struct sg_table *table, unsigned int size, unsigned int align_order)
{
	int ret;
	struct page *page = NULL;
	struct page *tmp_page = NULL;
	struct scatterlist *sg = NULL;
	struct list_head pages;
	int order;

	unsigned long size_remaining = ALIGN(size, PAGE_SIZE << align_order);
	int num = 0;
	unsigned int max_order = MAX_ORDER;

	if (!table) {
		xrheap_err("sg_table is null\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&pages);

	while (size_remaining > 0) {
		page = NULL;
		if (fatal_signal_pending(current)) {
			ret = -EINTR;
			goto alloc_fail;
		}
		/* alloc for dmabuf pool */
		for (order = max_order; order >= 0; order--) {
			if (xr_dmabuf_pagepool[order] == NULL)
				continue;
			if (PAGE_SIZE << order > size_remaining)
				continue;
			if (order < align_order)
				continue;
			page = xr_pagepool_alloc_internel(order);
			if (!!page) {
				xrheap_debug("order: 0x%x\n", order);
				break;
			}
		}
		if (!page) {
			ret = -ENOMEM;
			goto alloc_fail;
		}

		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
		max_order = order;
		num++;
	}
	xr_dmabuf_pagepool_dumpinfo();

	if (sg_alloc_table(table, num, GFP_KERNEL)) {
		xrheap_err("table: %pK, num: %d\n", table, num);
		ret = -ENOMEM;
		goto alloc_fail;
	}

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}
	return 0;

alloc_fail:
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		xr_pagepool_free_internel(compound_order(page), page);
	}
	xrheap_err("allocating size: 0x%x fail\n", size);
	return ret;
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_alloc);

int xr_dmabuf_pagepool_alloc_for_buddy(struct sg_table *table, unsigned int size, unsigned int align_order)
{
	int ret;
	struct page *page = NULL;
	struct page *tmp_page = NULL;
	struct scatterlist *sg = NULL;
	struct list_head pages;
	int order;

	unsigned long size_remaining = ALIGN(size, PAGE_SIZE << align_order);
	int num = 0;
	unsigned int max_order = MAX_ORDER;

	if (!table) {
		xrheap_err("sg_table is null\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&pages);

	while (size_remaining > 0) {
		page = NULL;
		if (fatal_signal_pending(current)) {
			ret = -EINTR;
			goto alloc_fail;
		}

		/* alloc for buddy */
		for (order = max_order; order >= 0; order--) {
			if (xr_dmabuf_pagepool[order] == NULL)
				continue;
			if (PAGE_SIZE << order > size_remaining)
				continue;
			if (order < align_order)
				continue;
			if (!xring_dmabuf_pool_fill_ok(xr_dmabuf_pagepool[order]))
				continue;
			page = xr_dmabuf_page_pool_alloc_from_buddy(xr_dmabuf_pagepool[order]);
			if (!!page) {
				xrheap_debug("order: 0x%x\n", order);
				break;
			}
		}

		if (!page) {
			ret = -ENOMEM;
			goto alloc_fail;
		}

		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
		max_order = order;
		num++;
	}

	if (sg_alloc_table(table, num, GFP_KERNEL)) {
		xrheap_err("table: %pK, num: %d\n", table, num);
		ret = -ENOMEM;
		goto alloc_fail;
	}

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}
	return 0;

alloc_fail:
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		xr_pagepool_free_internel(compound_order(page), page);
	}
	xrheap_info("allocating size: 0x%x\n", size);
	return ret;
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_alloc_for_buddy);

void xr_dmabuf_pagepool_free(struct sg_table *table)
{
	int i = 0;
	struct scatterlist *sg = NULL;
	struct page *page;

	for_each_sgtable_sg(table, sg, i) {
		page = sg_page(sg);
		xr_pagepool_free_internel(compound_order(page), page);
	}
	sg_free_table(table);
	xr_dmabuf_pagepool_dumpinfo();
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_free);

long xr_dmabuf_pagepool_get_size_per_heap(void)
{
	int max_user = atomic_read(&user_count);

	if (max_user <= 0)
		return 0;
	return xr_dmabuf_pagepool_get_size() / max_user;
}

long xr_dmabuf_pagepool_get_size(void)
{
	int i;
	int lowpage_count;
	int highpage_count;

	long num_pages = 0;

	for (i = 0; i <= MAX_ORDER; i++) {
		if (!xr_dmabuf_pagepool[i])
			continue;
		lowpage_count = xr_dmabuf_pagepool[i]->count[POOL_LOWPAGE];
		highpage_count = xr_dmabuf_pagepool[i]->count[POOL_HIGHPAGE];
		num_pages += (lowpage_count + highpage_count) << xr_dmabuf_pagepool[i]->order;
	}
	return num_pages << PAGE_SHIFT;
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_get_size);

void xr_dmabuf_pagepool_dumpinfo(void)
{
	int i = 0;

	xrheap_debug("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	for (i = 0; i <= MAX_ORDER; i++) {
		if (xr_dmabuf_pagepool[i]) {
			int order = xr_dmabuf_pagepool[i]->order;
			int lowcount = xr_dmabuf_pagepool[i]->count[0];

			xrheap_debug("\t[order]:%4d\t[lowpage_count]:%4d\t[total_page]:%4d pages\n",
				order, lowcount, lowcount << order);
		}
	}
	xrheap_debug("================================\n");
}
EXPORT_SYMBOL(xr_dmabuf_pagepool_dumpinfo);

int xr_dmabuf_pagepool_init(void)
{
	xr_dmabuf_pagepool_init_shrinker();

	return 0;
}
MODULE_LICENSE("GPL v2");
