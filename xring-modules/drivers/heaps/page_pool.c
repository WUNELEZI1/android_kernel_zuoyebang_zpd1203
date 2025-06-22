// SPDX-License-Identifier: GPL-2.0
/*
 * DMA BUF page pool system
 *
 * Copyright (C) 2020 Linaro Ltd.
 *
 * Based on the ION page pool code
 * Copyright (C) 2011 Google, Inc.
 */

#include "page_pool.h"

#include <linux/list.h>
#include <linux/shrinker.h>
#include <linux/spinlock.h>
#include <linux/swap.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include "soc/xring/xring_mem_adapter.h"

static LIST_HEAD(pool_list);
static DEFINE_MUTEX(pool_list_lock);
static int page_pool_dump;
static int page_pool_resv_enabled;

static inline
struct page *dmabuf_page_pool_alloc_pages(struct dmabuf_page_pool *pool)
{
	struct page *page;

	if (fatal_signal_pending(current))
		return NULL;
	page = alloc_pages(pool->gfp_mask, pool->order);
	if (page)
		set_bit(PG_oem_reserved_1, &page->flags); //do not dump in xr_kdump
	return page;
}

static inline void dmabuf_page_pool_free_pages(struct dmabuf_page_pool *pool,
					       struct page *page)
{
	clear_bit(PG_oem_reserved_1, &page->flags);
	__free_pages(page, pool->order);
}

static void dmabuf_page_pool_add(struct dmabuf_page_pool *pool, struct page *page)
{
	int index;

	if (PageHighMem(page))
		index = POOL_HIGHPAGE;
	else
		index = POOL_LOWPAGE;

	spin_lock(&pool->lock);
	list_add_tail(&page->lru, &pool->items[index]);
	pool->count[index]++;
	spin_unlock(&pool->lock);
	mod_node_page_state(page_pgdat(page), NR_KERNEL_MISC_RECLAIMABLE,
			    1 << pool->order);
}

static struct page *dmabuf_page_pool_remove(struct dmabuf_page_pool *pool, int index)
{
	struct page *page;

	spin_lock(&pool->lock);
	page = list_first_entry_or_null(&pool->items[index], struct page, lru);
	if (page) {
		pool->count[index]--;
		list_del(&page->lru);
		spin_unlock(&pool->lock);
		mod_node_page_state(page_pgdat(page), NR_KERNEL_MISC_RECLAIMABLE,
				    -(1 << pool->order));
		goto out;
	}
	spin_unlock(&pool->lock);
out:
	return page;
}

static struct page *dmabuf_page_pool_fetch(struct dmabuf_page_pool *pool)
{
	struct page *page = NULL;

	page = dmabuf_page_pool_remove(pool, POOL_HIGHPAGE);
	if (!page)
		page = dmabuf_page_pool_remove(pool, POOL_LOWPAGE);

	return page;
}

struct page *xr_dmabuf_page_pool_alloc(struct dmabuf_page_pool *pool)
{
	struct page *page = NULL;

	if (WARN_ON(!pool))
		return NULL;

	page = dmabuf_page_pool_fetch(pool);

	if (!page)
		page = dmabuf_page_pool_alloc_pages(pool);
	return page;
}
EXPORT_SYMBOL_GPL(xr_dmabuf_page_pool_alloc);

void xr_dmabuf_page_pool_free(struct dmabuf_page_pool *pool, struct page *page)
{
	if (WARN_ON(pool->order != compound_order(page)))
		return;

	dmabuf_page_pool_add(pool, page);
}
EXPORT_SYMBOL_GPL(xr_dmabuf_page_pool_free);

struct page *xr_dmabuf_page_pool_alloc_from_buddy(struct dmabuf_page_pool *pool)
{
	return dmabuf_page_pool_alloc_pages(pool);
}

void xr_dmabuf_page_pool_free_to_buddy(struct dmabuf_page_pool *pool, struct page *page)
{
	if (WARN_ON(pool->order != compound_order(page)))
		return;

	dmabuf_page_pool_free_pages(pool, page);
}

static int dmabuf_page_pool_total(struct dmabuf_page_pool *pool, bool high)
{
	int count = pool->count[POOL_LOWPAGE];

	if (high)
		count += pool->count[POOL_HIGHPAGE];

	return count << pool->order;
}

struct dmabuf_page_pool *xr_dmabuf_page_pool_create(gfp_t gfp_mask, unsigned int order)
{
	struct dmabuf_page_pool *pool = kmalloc(sizeof(*pool), GFP_KERNEL);
	int i;

	if (!pool)
		return NULL;

	for (i = 0; i < POOL_TYPE_SIZE; i++) {
		pool->count[i] = 0;
		INIT_LIST_HEAD(&pool->items[i]);
	}
	pool->gfp_mask = gfp_mask | __GFP_COMP;
	pool->order = order;
	pool->reserved_pages = 0;
	spin_lock_init(&pool->lock);

	mutex_lock(&pool_list_lock);
	list_add(&pool->list, &pool_list);
	mutex_unlock(&pool_list_lock);

	return pool;
}
EXPORT_SYMBOL_GPL(xr_dmabuf_page_pool_create);

void xr_dmabuf_page_pool_destroy(struct dmabuf_page_pool *pool)
{
	struct page *page;
	int i;

	/* Remove us from the pool list */
	mutex_lock(&pool_list_lock);
	list_del(&pool->list);
	mutex_unlock(&pool_list_lock);

	/* Free any remaining pages in the pool */
	for (i = 0; i < POOL_TYPE_SIZE; i++) {
		while ((page = dmabuf_page_pool_remove(pool, i)))
			dmabuf_page_pool_free_pages(pool, page);
	}

	kfree(pool);
}
EXPORT_SYMBOL_GPL(xr_dmabuf_page_pool_destroy);

unsigned long xr_dmabuf_page_pool_get_size(struct dmabuf_page_pool *pool)
{
		int i;
		unsigned long num_pages = 0;

		spin_lock(&pool->lock);
		for (i = 0; i < POOL_TYPE_SIZE; ++i)
			num_pages += pool->count[i];
		spin_unlock(&pool->lock);
		num_pages <<= pool->order; /* pool order is immutable */

		return num_pages * PAGE_SIZE;
}
EXPORT_SYMBOL_GPL(xr_dmabuf_page_pool_get_size);

/* only fill pool when system watermark is ok */
bool xring_dmabuf_pool_fill_ok(struct dmabuf_page_pool *pool)
{
	enum zone_type classzone_idx = xring_gfp_zone(pool->gfp_mask);

	if (!xring_zone_watermark_ok(classzone_idx, pool->order))
		return false;

	return true;
}

void dmabuf_page_pool_set_reserved_pages(struct dmabuf_page_pool *pool, unsigned int reserved_pages)
{
	pool->reserved_pages = reserved_pages;
}

int total_pool_need_free_pages(void)
{
	struct dmabuf_page_pool *pool = NULL;
	int free_pages = 0;

	mutex_lock(&pool_list_lock);
	list_for_each_entry(pool, &pool_list, list) {
		if (page_pool_resv_enabled)
			free_pages += dmabuf_page_pool_total(pool, true) - pool->reserved_pages > 0 ?
				dmabuf_page_pool_total(pool, true) - pool->reserved_pages : 0;
		else
			free_pages += dmabuf_page_pool_total(pool, true);
	}
	mutex_unlock(&pool_list_lock);
	return free_pages;
}

void dmabuf_page_pool_dump(void)
{
	struct dmabuf_page_pool *pool = NULL;

	pr_info("page pool reserve enable status: %d\n", page_pool_resv_enabled);
	mutex_lock(&pool_list_lock);
	list_for_each_entry(pool, &pool_list, list) {
		int order = pool->order;
		int lowcount = pool->count[0];

		pr_info("[order]:%d [count]:%4d [pages]:%4d [size]:%luMB [resv-pages]:%d\n",
			order, lowcount, lowcount << order, (lowcount << order) * PAGE_SIZE / 1024 / 1024, pool->reserved_pages);
	}
	mutex_unlock(&pool_list_lock);
}

static int page_pool_dump_set(const char *val, const struct kernel_param *kp)
{
	dmabuf_page_pool_dump();
	return 0;
}

static const struct kernel_param_ops page_pool_dump_ops = {
	.set = page_pool_dump_set,
	.get = param_get_int,
};


static int dmabuf_page_pool_do_shrink(struct dmabuf_page_pool *pool, gfp_t gfp_mask,
				      int nr_to_scan)
{
	int freed = 0;
	bool high;

	if (current_is_kswapd())
		high = true;
	else
		high = !!(gfp_mask & __GFP_HIGHMEM);

	if (nr_to_scan == 0)
		return dmabuf_page_pool_total(pool, high);

	while (freed < nr_to_scan) {
		struct page *page;

		/* if pool has watermark, break */
		if (page_pool_resv_enabled && dmabuf_page_pool_total(pool, true) < pool->reserved_pages)
			break;

		/* Try to free low pages first */
		page = dmabuf_page_pool_remove(pool, POOL_LOWPAGE);
		if (!page)
			page = dmabuf_page_pool_remove(pool, POOL_HIGHPAGE);

		if (!page)
			break;

		dmabuf_page_pool_free_pages(pool, page);
		freed += (1 << pool->order);
	}

	return freed;
}

static int dmabuf_page_pool_shrink(gfp_t gfp_mask, int nr_to_scan)
{
	struct dmabuf_page_pool *pool = NULL;
	int nr_total = 0;
	int nr_freed;
	int only_scan = 0;

	if (!nr_to_scan)
		only_scan = 1;

	if (only_scan) {
		int need_free = total_pool_need_free_pages();

		return need_free > 0 ? need_free : 0;
	}

	mutex_lock(&pool_list_lock);
	list_for_each_entry(pool, &pool_list, list) {
		nr_freed = dmabuf_page_pool_do_shrink(pool, gfp_mask, nr_to_scan);
		nr_to_scan -= nr_freed;
		nr_total += nr_freed;
		if (nr_to_scan <= 0)
			break;
	}
	mutex_unlock(&pool_list_lock);

	return nr_total;
}

static unsigned long dmabuf_page_pool_shrink_count(struct shrinker *shrinker,
						   struct shrink_control *sc)
{
	return dmabuf_page_pool_shrink(sc->gfp_mask, 0);
}

static unsigned long dmabuf_page_pool_shrink_scan(struct shrinker *shrinker,
						  struct shrink_control *sc)
{
	int need_free = total_pool_need_free_pages();

	if (need_free <= 0)
		return SHRINK_STOP;

	if (sc->nr_to_scan == 0)
		return 0;

	return dmabuf_page_pool_shrink(sc->gfp_mask, sc->nr_to_scan);
}

struct shrinker pool_shrinker = {
	.count_objects = dmabuf_page_pool_shrink_count,
	.scan_objects = dmabuf_page_pool_shrink_scan,
	.seeks = DEFAULT_SEEKS,
	.batch = 0,
};

int xr_dmabuf_pagepool_init_shrinker(void)
{
	return register_shrinker(&pool_shrinker, "xr-dmabuf-pagepool-shrinker");
}
EXPORT_SYMBOL_GPL(xr_dmabuf_pagepool_init_shrinker);
module_param_cb(page_pool_dump, &page_pool_dump_ops, &page_pool_dump, 0644);
MODULE_LICENSE("GPL v2");
