// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF System heap exporter
 *
 * Copyright (C) 2011 Google, Inc.
 * Copyright (C) 2019, 2020 Linaro Ltd.
 *
 * Portions based off of Andrew Davis' SRAM heap:
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 */

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
#include <linux/types.h>
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>

#include "linux/dma-direction.h"
#include "linux/smp.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "soc/xring/xring_mem_adapter.h"
#include "xring_dma_iommu.h"
#include "xr_dmabuf_pagepool.h"
#include "system_heap.h"
#include "page_pool.h"
#define CREATE_TRACE_POINTS
#include "xring_heap_trace.h"
#undef CREATE_TRACE_POINTS

#include "deferred-free-helper.h"

static struct dma_heap *sys_heap;
static struct dma_heap *sys_uncached_heap;

static atomic64_t total_allocated_system_heap; // total allocated in heap: system or system_uncached
#if IS_ENABLED(CONFIG_XRING_HEAP_ALLOC_MAX_TIMEOUT)
static unsigned int system_alloc_timeout = INT_MAX; // ms
#else
static unsigned int system_alloc_timeout = 1000; // ms
#endif

static unsigned int system_read_fragment = 32;

struct system_heap_buffer {
	struct xr_heap_buffer_header header;
	struct dma_heap *heap;
	struct list_head attachments;
	struct mutex lock;
	unsigned long len;
	struct sg_table sg_table;
	int vmap_cnt;
	void *vaddr;
	bool uncached;
};

struct dma_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
	bool mapped;
	bool uncached;
};

struct system_heap_deferred_buffer {
	struct deferred_freelist_item item;
	struct system_heap_buffer *buffer;
};

struct flush_cache_info {
	struct system_heap_buffer *buffer;
	enum dma_data_direction direction;
};

#define LOW_ORDER_GFP (GFP_HIGHUSER | __GFP_ZERO | __GFP_COMP)
#define MID_ORDER_GFP (LOW_ORDER_GFP | __GFP_NOWARN)
#define HIGH_ORDER_GFP  (((GFP_HIGHUSER | __GFP_ZERO | __GFP_NOWARN \
				| __GFP_NORETRY) & ~__GFP_RECLAIM) \
				| __GFP_COMP)
/*
 * The selection of the orders used for allocation (1MB, 64K, 4K) is designed
 * to match with the sizes often found in IOMMUs. Using order 4 pages instead
 * of order 0 pages can significantly improve the performance of many IOMMUs
 * by reducing TLB pressure and time spent updating page tables.
 */
#ifdef CONFIG_XRING_SMMU_CONTIGUOUS
static const unsigned int orders[] = {9, 8, 4, 0};
static gfp_t order_flags[] = {HIGH_ORDER_GFP, HIGH_ORDER_GFP, HIGH_ORDER_GFP, LOW_ORDER_GFP};
#else
static const unsigned int orders[] = {8, 4, 0};
static gfp_t order_flags[] = {HIGH_ORDER_GFP, HIGH_ORDER_GFP, LOW_ORDER_GFP};
#endif

#define NUM_ORDERS ARRAY_SIZE(orders)
struct dmabuf_page_pool *pools[NUM_ORDERS];

#define DYNAMIC_POOL_KTHREAD_NICE_VAL 10
static struct task_struct *refill_thread;
static atomic_t refill_thread_running;
static int system_pool_refill_enable;
static int system_pool_refill_record;
static int system_pool_fill_mark = 100 * SZ_1M;
static int system_pool_low_mark = 40 * SZ_1M;
static int system_pool_resv_pages = 40 * SZ_1M / PAGE_SIZE;

static void set_system_heap_resv_pages(int pages)
{
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (!IS_ERR(pools[i])) {
			/* only reserve pages for order > 0 */
			if (pools[i]->order)
				dmabuf_page_pool_set_reserved_pages(pools[i], pages);
		}
	}
}

static int heap_rsv_param_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	ret = kstrtouint(val, 10, &system_pool_resv_pages);
	if (ret) {
		xrheap_err("Invalid value for system_pool_resv_pages\n");
		return -EINVAL;
	}

	set_system_heap_resv_pages(system_pool_resv_pages);
	return 0;
}

static const struct kernel_param_ops heap_rsv_param_ops = {
	.set = heap_rsv_param_set,
	.get = param_get_int
};

static int get_dmabuf_pool_fillmark(struct dmabuf_page_pool *pool)
{
	return system_pool_fill_mark / (PAGE_SIZE << pool->order);
}

static bool dmabuf_pool_fillmark_reached(struct dmabuf_page_pool *pool)
{
	return pool->count[POOL_LOWPAGE] >= get_dmabuf_pool_fillmark(pool);
}

static int get_dmabuf_pool_lowmark(struct dmabuf_page_pool *pool)
{
	return system_pool_low_mark / (PAGE_SIZE << pool->order);
}

static bool dmabuf_pool_count_below_lowmark(struct dmabuf_page_pool *pool)
{
	return pool->count[POOL_LOWPAGE] < get_dmabuf_pool_lowmark(pool);
}

static bool dmabuf_pool_needs_refill(struct dmabuf_page_pool *pool)
{
	return pool->order && dmabuf_pool_count_below_lowmark(pool);
}

void wakeup_refill_thread(void)
{
	int i;

	if (system_pool_refill_enable) {
		for (i = 0; i < NUM_ORDERS; i++) {
			if (dmabuf_pool_needs_refill(pools[i]) && !atomic_read(&refill_thread_running)) {
				wake_up_process(refill_thread);
				return;
			}
		}
	}
}
static void dynamic_page_pool_refill(struct dmabuf_page_pool *pool)
{
	struct page *page;
	gfp_t gfp_refill = (pool->gfp_mask | __GFP_RECLAIM) & ~__GFP_NORETRY;

	/* skip refilling order 0 pools */
	if (!pool->order)
		return;

	while (!dmabuf_pool_fillmark_reached(pool) && xring_dmabuf_pool_fill_ok(pool)) {
		page = alloc_pages(gfp_refill, pool->order);
		if (!page)
			break;
		set_bit(PG_oem_reserved_1, &page->flags); /* do not dump dmabuf in xr_kdump */
		xr_dmabuf_page_pool_free(pool, page); /* free to pool */
	}
}

static int system_heap_refill_thread(void *data)
{
	struct dmabuf_page_pool **pool_list = data;
	int i;

	for (;;) {
		if (system_pool_refill_record) {
			xrheap_info("before refilling page pools\n");
			dmabuf_page_pool_dump();
		}
		if (system_pool_refill_enable) {
			for (i = 0; i < NUM_ORDERS; i++) {
				if (dmabuf_pool_needs_refill(pool_list[i]))
					dynamic_page_pool_refill(pool_list[i]);
			}
		}
		if (system_pool_refill_record) {
			xrheap_info("after refilling page pools\n");
			dmabuf_page_pool_dump();
		}

		set_current_state(TASK_INTERRUPTIBLE);
		atomic_set(&refill_thread_running, 0);
		if (unlikely(kthread_should_stop())) {
			set_current_state(TASK_RUNNING);
			break;
		}
		schedule();

		atomic_set(&refill_thread_running, 1);
		set_current_state(TASK_RUNNING);
	}

	return 0;
}

static int system_heap_create_refill_thread(const char *name)
{
	struct sched_attr attr = { .sched_nice = DYNAMIC_POOL_KTHREAD_NICE_VAL };
	int ret;

	refill_thread = kthread_run(system_heap_refill_thread, pools,
				    "%s-pool-refill-thread", name);
	if (IS_ERR(refill_thread)) {
		xrheap_err("%s: failed to create %s-pool-refill-thread: %ld\n",
			__func__, name, PTR_ERR(refill_thread));
		return PTR_ERR(refill_thread);
	}

	ret = sched_setattr(refill_thread, &attr);
	if (ret) {
		xrheap_err("failed to setattr for %s-pool-refill-thread: %d\n", name, ret);
		kthread_stop(refill_thread);
		return ret;
	}

	return 0;
}

static long system_get_pool_size(struct dma_heap *heap);

static u64 get_alloc_size_system_heap(void *data)
{
	return atomic64_read(&total_allocated_system_heap);
}

static u64 get_pool_cached_system_heap(void *data)
{
	return system_get_pool_size(0);
}

static u64 get_buddy_consumed_system_heap(void *data)
{
	return get_alloc_size_system_heap(data) + get_pool_cached_system_heap(data);
}

struct xr_dmabuf_helper_ops system_dmabuf_helper_ops = {
	.get_alloc_size = get_alloc_size_system_heap,
	.get_buddy_consumed = get_buddy_consumed_system_heap,
	.get_pool_cached = get_pool_cached_system_heap,
};

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table = NULL;
	int ret, i = 0;
	struct scatterlist *sg = NULL, *new_sg = NULL;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->orig_nents, GFP_KERNEL);
	if (ret) {
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

static int system_heap_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a;
	struct sg_table *table;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	table = dup_sg_table(&buffer->sg_table);
	if (IS_ERR(table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->table = table;
	a->dev = attachment->dev;
	INIT_LIST_HEAD(&a->list);
	a->mapped = false;
	a->uncached = buffer->uncached;
	attachment->priv = a;

	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	return 0;
}

static void system_heap_detach(struct dma_buf *dmabuf,
			       struct dma_buf_attachment *attachment)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = attachment->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);

	sg_free_table(a->table);
	kfree(a->table);
	kfree(a);
}

static struct sg_table *system_heap_map_dma_buf(struct dma_buf_attachment *attachment,
						enum dma_data_direction direction)
{
	struct dma_heap_attachment *a = attachment->priv;
	struct sg_table *table = a->table;
	struct dma_buf *dmabuf = attachment->dmabuf;
	int attr = attachment->dma_map_attrs;
	int ret;

	trace_map_dma_buf("begin", dev_name(attachment->dev), dmabuf, dmabuf->size, direction);
	if (a->uncached)
		attr |= DMA_ATTR_SKIP_CPU_SYNC;

#ifdef XRING_DMA_MAP_LAZY
	ret = xring_dma_map_sgtable_lazy(attachment->dev, table, direction, 0, dmabuf);
	if (ret < 0) {
		xrheap_err("xring_dma_map_sgtable_lazy fail\n");
		return ERR_PTR(ret);
	}
#else
	ret = xring_heap_dma_map(attachment->dev, dmabuf, table, direction, attr);
	trace_map_dma_buf("end", dev_name(attachment->dev), dmabuf, dmabuf->size, direction);
	if (ret)
		return ERR_PTR(ret);
#endif

	a->mapped = true;
	return table;
}

static void system_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
				      struct sg_table *table,
				      enum dma_data_direction direction)
{
	struct dma_heap_attachment *a = attachment->priv;
	int attr = attachment->dma_map_attrs;

	if (a->uncached)
		attr |= DMA_ATTR_SKIP_CPU_SYNC;
	a->mapped = false;

#ifdef XRING_DMA_MAP_LAZY
	xring_dma_unmap_sgtable_lazy(attachment->dev, table, direction, 0,
					attachment->dmabuf);
#else
	xring_heap_dma_unmap(attachment->dev, attachment->dmabuf, table, direction, attr);
#endif
}

static int sgl_sync_range(struct device *dev, struct scatterlist *sgl,
			  unsigned int nents, unsigned long offset,
			  unsigned long length,
			  enum dma_data_direction dir, bool for_cpu)
{
	int i;
	struct scatterlist *sg;
	unsigned int len = 0;

	/* explain each var, 'o' is the cache to sync
	 * |-----------------offset----------------|--length--|
	 *                             |--------sg->length--------|
	 * [--------------------] <--> [-----------ooooooooooo----] <--> [-------------------]
	 * |-----------------------len----------------------------|
	 *                             |-sg_offset-|---sg_left----|
	 *                                         |---size---|
	 */
	for_each_sg(sgl, sg, nents, i) {
		unsigned int sg_offset, sg_left, size = 0;

		len += sg->length;

		if (len <= offset)
			continue;

		sg_left = len - offset;
		sg_offset = sg->length - sg_left;
		size = (length < sg_left) ? length : sg_left;

		if (for_cpu)
			dma_sync_single_range_for_cpu(dev, sg_phys(sg), sg_offset, size, dir);
		else
			dma_sync_single_range_for_device(dev, sg_phys(sg), sg_offset, size, dir);

		offset += size;
		length -= size;

		if (length == 0)
			break;
	}

	return 0;
}

static void system_heap_dma_buf_begin_cpu_access_max_cpu(struct dma_buf *dmabuf,
						enum dma_data_direction direction);
static void system_heap_dma_buf_end_cpu_access_max_cpu(struct dma_buf *dmabuf,
						enum dma_data_direction direction);

void xr_invalidate_cache_by_va(void *info)
{
	struct flush_cache_info *flush_cache_info = (struct flush_cache_info *)info;
	struct system_heap_buffer *buffer = flush_cache_info->buffer;
	enum dma_data_direction direction = flush_cache_info->direction;
	struct dma_heap_attachment *a = NULL;

	trace_dma_buf_cpu_access("invalidate_cache_by_va start", NULL, 0, direction, 0, 0);
	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_cpu(a->dev, a->table, direction);
	}
	trace_dma_buf_cpu_access("invalidate_cache_by_va end", NULL, 0, direction, 0, 0);
}

void xr_clean_cache_by_va(void *info)
{
	struct flush_cache_info *flush_cache_info = (struct flush_cache_info *)info;
	struct system_heap_buffer *buffer = flush_cache_info->buffer;
	enum dma_data_direction direction = flush_cache_info->direction;
	struct dma_heap_attachment *a = NULL;

	trace_dma_buf_cpu_access("clean_cache_by_va start", NULL, 0, direction, 0, 0);
	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_device(a->dev, a->table, direction);
	}
	trace_dma_buf_cpu_access("clean_cache_by_va end", NULL, 0, direction, 0, 0);
}

static unsigned int current_max_cpuid(void)
{
	int cpu;
	unsigned int max_cpuid = 0;
	unsigned long max_capacity = 0;
	unsigned long capability;

	trace_dma_buf_cpu_access("find max capacity cpu begin", NULL, 0, 0, 0, 0);
	for_each_online_cpu(cpu) {
		capability = arch_scale_freq_capacity(cpu);
		if (capability > max_capacity) {
			max_capacity = capability;
			max_cpuid = cpu;
		}
	}
	trace_dma_buf_cpu_access("find max capacity cpu end", NULL, 0, 0, 0, 0);

	return max_cpuid;
}

static int system_heap_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
						enum dma_data_direction direction)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;

	trace_dma_buf_cpu_access("begin_cpu_access start", dmabuf, dmabuf->size, direction, 0, dmabuf->size);
	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		if (dmabuf->size < (SZ_1M * 80)) {
			list_for_each_entry(a, &buffer->attachments, list) {
				if (!a->mapped)
					continue;
				dma_sync_sgtable_for_cpu(a->dev, a->table, direction);
			}
		} else {
			system_heap_dma_buf_begin_cpu_access_max_cpu(dmabuf, direction);
		}
	}
	mutex_unlock(&buffer->lock);
	trace_dma_buf_cpu_access("begin_cpu_access end", dmabuf, dmabuf->size, direction, 0, dmabuf->size);

	return 0;
}

static int system_heap_dma_buf_begin_cpu_access_partial(struct dma_buf *dmabuf,
				enum dma_data_direction dir,
				unsigned int offset, unsigned int len)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;
	int ret = 0;

	if (buffer->uncached)
		return 0;

	trace_dma_buf_cpu_access("begin_cpu_access_partial start",
		dmabuf, dmabuf->size, dir, offset, len);
	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr + offset, len);

	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		ret = sgl_sync_range(a->dev, a->table->sgl, a->table->orig_nents,
					offset, len, dir, true);
	}
	mutex_unlock(&buffer->lock);
	trace_dma_buf_cpu_access("begin_cpu_access_partial end",
		dmabuf, dmabuf->size, dir, offset, len);

	return ret;
}

static void system_heap_dma_buf_begin_cpu_access_max_cpu(struct dma_buf *dmabuf,
						enum dma_data_direction direction)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	unsigned int max_cpuid = 0, cur_cpuid = 0;
	struct flush_cache_info info;

	info.buffer = buffer;
	info.direction = direction;

	trace_dma_buf_cpu_access("begin_cpu_access_max_cpu start", dmabuf, dmabuf->size, direction, 0, dmabuf->size);

	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		max_cpuid = current_max_cpuid();
		cur_cpuid = smp_processor_id();
		if ((cur_cpuid == 0 || cur_cpuid == 1) && (max_cpuid == 8 || max_cpuid == 9))
			smp_call_function_single(current_max_cpuid(), xr_invalidate_cache_by_va, &info, 1);
		else
			xr_invalidate_cache_by_va(&info);
	}

	trace_dma_buf_cpu_access("begin_cpu_access_max_cpu end", dmabuf, dmabuf->size, direction, 0, dmabuf->size);
}

static int system_heap_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
					      enum dma_data_direction direction)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;

	trace_dma_buf_cpu_access("end_cpu_access start", dmabuf, dmabuf->size, direction, 0, dmabuf->size);
	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		if (dmabuf->size < (SZ_1M * 80)) {
			list_for_each_entry(a, &buffer->attachments, list) {
				if (!a->mapped)
					continue;
				dma_sync_sgtable_for_device(a->dev, a->table, direction);
			}
		} else {
			system_heap_dma_buf_end_cpu_access_max_cpu(dmabuf, direction);
		}
	}
	mutex_unlock(&buffer->lock);
	trace_dma_buf_cpu_access("end_cpu_access end", dmabuf, dmabuf->size, direction, 0, dmabuf->size);

	return 0;
}

static int system_heap_dma_buf_end_cpu_access_partial(struct dma_buf *dmabuf,
				enum dma_data_direction dir,
				unsigned int offset, unsigned int len)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;
	int ret = 0;

	if (buffer->uncached)
		return 0;

	trace_dma_buf_cpu_access("end_cpu_access_partial start",
		dmabuf, dmabuf->size, dir, offset, len);
	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr + offset, len);

	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		ret = sgl_sync_range(a->dev, a->table->sgl, a->table->orig_nents,
					offset, len, dir, false);
	}
	mutex_unlock(&buffer->lock);
	trace_dma_buf_cpu_access("end_cpu_access_partial end",
		dmabuf, dmabuf->size, dir, offset, len);

	return ret;
}

static void system_heap_dma_buf_end_cpu_access_max_cpu(struct dma_buf *dmabuf,
					      enum dma_data_direction direction)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	unsigned int max_cpuid = 0, cur_cpuid = 0;
	struct flush_cache_info info;

	info.buffer = buffer;
	info.direction = direction;

	trace_dma_buf_cpu_access("end_cpu_access_max_cpu start", dmabuf, dmabuf->size, direction, 0, dmabuf->size);

	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		max_cpuid = current_max_cpuid();
		cur_cpuid = smp_processor_id();
		if ((cur_cpuid == 0 || cur_cpuid == 1) && (max_cpuid == 8 || max_cpuid == 9))
			smp_call_function_single(current_max_cpuid(), xr_clean_cache_by_va, &info, 1);
		else
			xr_clean_cache_by_va(&info);
	}

	trace_dma_buf_cpu_access("end_cpu_access_max_cpu end", dmabuf, dmabuf->size, direction, 0, dmabuf->size);
}

static int system_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	struct sg_table *table = &buffer->sg_table;
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	unsigned long addr = vma->vm_start;
	struct scatterlist *sg = NULL;
	int ret = 0;
	int i = 0;

	if (buffer->uncached)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page = sg_page(sg);
		unsigned long remainder = vma->vm_end - addr;
		unsigned long len = sg->length;

		if (offset >= sg->length) {
			offset -= sg->length;
			continue;
		} else if (offset) {
			page += offset / PAGE_SIZE;
			len = sg->length - offset;
			offset = 0;
		}
		len = min(len, remainder);

		ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
				vma->vm_page_prot);
		if (ret) {
			xrheap_err("remap failed\n");
			return ret;
		}
		addr += len;
		if (addr >= vma->vm_end)
			goto done;
	}

done:
	return 0;
}

static void *system_heap_do_vmap(struct system_heap_buffer *buffer)
{
	struct sg_table *table = &buffer->sg_table;
	int npages = PAGE_ALIGN(buffer->len) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	struct sg_page_iter piter = {0};
	pgprot_t pgprot = PAGE_KERNEL;
	void *vaddr;

	if (!pages)
		return ERR_PTR(-ENOMEM);

	if (buffer->uncached)
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	for_each_sgtable_page(table, &piter, 0) {
		WARN_ON(tmp - pages >= npages);
		*tmp++ = sg_page_iter_page(&piter);
	}

	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);

	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	return vaddr;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int system_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static int system_heap_vmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct system_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;
	int ret = 0;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
		dma_buf_map_set_vaddr(map, buffer->vaddr);
#else
		iosys_map_set_vaddr(map, buffer->vaddr);
#endif
		goto out;
	}

	vaddr = system_heap_do_vmap(buffer);
	if (IS_ERR(vaddr)) {
		ret = PTR_ERR(vaddr);
		goto out;
	}

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	dma_buf_map_set_vaddr(map, buffer->vaddr);
#else
	iosys_map_set_vaddr(map, buffer->vaddr);
#endif
out:
	mutex_unlock(&buffer->lock);

	return ret;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static void system_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static void system_heap_vunmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct system_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	dma_buf_map_clear(map);
#else
	iosys_map_clear(map);
#endif
}

static int system_heap_zero_buffer(struct system_heap_buffer *buffer)
{
	struct sg_table *sgt = &buffer->sg_table;
	struct sg_page_iter piter = {0};
	struct page *p = NULL;
	void *vaddr = NULL;
	int ret = 0;

	for_each_sgtable_page(sgt, &piter, 0) {
		p = sg_page_iter_page(&piter);
		vaddr = kmap_atomic(p);
		memset(vaddr, 0, PAGE_SIZE);
		kunmap_atomic(vaddr);
	}

	return ret;
}

static void system_heap_dma_buf_release(
	struct deferred_freelist_item *freelist_item, enum df_reason reason)
{
	struct system_heap_deferred_buffer *deferred_buffer =
			container_of(freelist_item, struct system_heap_deferred_buffer, item);
	struct system_heap_buffer *buffer = deferred_buffer->buffer;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	int i = 0, j = 0;

	/* Zero the buffer pages before adding back to the pool */
	system_heap_zero_buffer(buffer);
	table = &buffer->sg_table;
	for_each_sgtable_sg(table, sg, i) {
		struct page *page = sg_page(sg);

		for (j = 0; j < NUM_ORDERS; j++) {
			if (compound_order(page) == orders[j])
				break;
		}
		if (j < NUM_ORDERS) {
			if (reason == DF_UNDER_PRESSURE)
				xr_dmabuf_page_pool_free_to_buddy(pools[j], page);
			else
				xr_dmabuf_page_pool_free(pools[j], page);
		}
	}
	sg_free_table(table);
	atomic64_sub(buffer->len, &total_allocated_system_heap);
	kfree(buffer);
	kfree(deferred_buffer);
}

static void system_heap_dma_buf_deferred_release(struct dma_buf *dmabuf)
{
	struct system_heap_deferred_buffer *deferred_buffer;

	deferred_buffer = kzalloc(sizeof(*deferred_buffer), GFP_KERNEL);
	if (!deferred_buffer)
		return;

	deferred_buffer->buffer = dmabuf->priv;
	deferred_free(&deferred_buffer->item,
				system_heap_dma_buf_release, dmabuf->size / PAGE_SIZE);
}

static const struct dma_buf_ops system_heap_buf_ops = {
	.attach = system_heap_attach,
	.detach = system_heap_detach,
	.map_dma_buf = system_heap_map_dma_buf,
	.unmap_dma_buf = system_heap_unmap_dma_buf,
	.begin_cpu_access = system_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = system_heap_dma_buf_end_cpu_access,
	.begin_cpu_access_partial = system_heap_dma_buf_begin_cpu_access_partial,
	.end_cpu_access_partial = system_heap_dma_buf_end_cpu_access_partial,
	.mmap = system_heap_mmap,
	.vmap = system_heap_vmap,
	.vunmap = system_heap_vunmap,
	.release = system_heap_dma_buf_deferred_release,
};

static struct page *alloc_largest_available(unsigned long size,
					    unsigned int max_order)
{
	struct page *page;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		if (size < (PAGE_SIZE << orders[i]))
			continue;
		if (max_order < orders[i])
			continue;
		page = xr_dmabuf_page_pool_alloc(pools[i]);
		if (!page)
			continue;

		return page;
	}
	return NULL;
}

static struct dma_buf *system_heap_do_allocate(struct dma_heap *heap,
					       unsigned long len,
					       u32 fd_flags,
					       u64 heap_flags,
					       bool uncached)
{
	struct system_heap_buffer *buffer;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf = NULL;
	int ret = -ENOMEM;
	unsigned long size_remaining = len;
	unsigned int max_order = orders[0];
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct list_head pages;
	struct page *page = NULL, *tmp_page = NULL;
	int i;

	if (len >= SZ_1G)
		xrheap_info("%s system%s alloc %lu bytes\n", current->comm, uncached ? " uncached" : "", len);
	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->heap = heap;
	buffer->len = len;
	buffer->uncached = uncached;

	INIT_LIST_HEAD(&pages);
	i = 0;
	while (size_remaining > 0) {
		/*
		 * Avoid trying to allocate memory if the process
		 * has been killed by SIGKILL
		 */
		if (fatal_signal_pending(current)) {
			ret = -EINTR;
			goto free_buffer;
		}

		page = alloc_largest_available(size_remaining, max_order);
		if (!page) {
			xrheap_err("failed\n");
			goto free_buffer;
		}
		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
		max_order = compound_order(page);
		i++;
	}

	table = &buffer->sg_table;
	if (sg_alloc_table(table, i, GFP_KERNEL))
		goto free_buffer;

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	/* create the dmabuf */
	exp_info.exp_name = dma_heap_get_name(heap);
	exp_info.ops = &system_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto free_pages;
	}
	atomic64_add(dmabuf->size, &total_allocated_system_heap);
	/*
	 * For uncached buffers, we need to initially flush cpu cache, since
	 * the __GFP_ZERO on the allocation means the zeroing was done by the
	 * cpu and thus it is likely cached. Map (and implicitly flush) and
	 * unmap it now so we don't get corruption later on.
	 */
	if (buffer && buffer->uncached) {
		dma_map_sgtable(dma_heap_get_dev(heap), &buffer->sg_table, DMA_BIDIRECTIONAL, 0);
		dma_unmap_sgtable(dma_heap_get_dev(heap), &buffer->sg_table, DMA_BIDIRECTIONAL, 0);
	}
	wakeup_refill_thread();

	return dmabuf;

free_pages:
	for_each_sgtable_sg(table, sg, i) {
		struct page *p = sg_page(sg);

		clear_bit(PG_oem_reserved_1, &p->flags);
		__free_pages(p, compound_order(p));
	}
	sg_free_table(table);
free_buffer:
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		clear_bit(PG_oem_reserved_1, &page->flags);
		__free_pages(page, compound_order(page));
	}

	kfree(buffer);

	return ERR_PTR(ret);
}

static struct dma_buf *system_heap_allocate(struct dma_heap *heap,
					    unsigned long len,
					    u32 fd_flags,
					    u64 heap_flags)
{
	struct dma_buf *dmabuf;
	bool alloc_timeout = false;
	bool alloc_failed = false;

	KTIME_DEFINE_GET_START(start, end, duration_ms);

	dmabuf = system_heap_do_allocate(heap, len, fd_flags, heap_flags, false);
	KTIME_GET_END_DURATION(start, end, duration_ms);

	alloc_timeout = duration_ms > system_alloc_timeout ? true : false;
	alloc_failed = IS_ERR(dmabuf) ? true : false;

	if (alloc_timeout || alloc_failed) {
		xrheap_info("len: 0x%lx bytes, duration_ms: %lld, dmabuf ok? %c:%ld\n",
			len, duration_ms, alloc_failed ? 'N' : 'Y', alloc_failed ? PTR_ERR(dmabuf) : 0);
		xr_meminfo_show(true, FROM_DMAHEAP);

		if (alloc_timeout)
			xring_memory_event_report(MM_SYSTEM_HEAP_TIMEOUT);
		if (alloc_failed)
			xring_memory_event_report(MM_SYSTEM_HEAP_ERR);
	}
	return dmabuf;
}

static long system_get_pool_size(struct dma_heap *heap)
{
	int i;
	long num_pages = 0;
	struct dmabuf_page_pool **pool;

	pool = pools;
	for (i = 0; i < NUM_ORDERS; i++, pool++) {
		num_pages += ((*pool)->count[POOL_LOWPAGE] +
			      (*pool)->count[POOL_HIGHPAGE]) << (*pool)->order;
	}

	return num_pages << PAGE_SHIFT;
}

static const struct dma_heap_ops system_heap_ops = {
	.allocate = system_heap_allocate,
	.get_pool_size = system_get_pool_size,
};

static struct dma_buf *system_uncached_heap_allocate(struct dma_heap *heap,
						     unsigned long len,
						     u32 fd_flags,
						     u64 heap_flags)
{
	struct dma_buf *dmabuf;

	KTIME_DEFINE_GET_START(start, end, duration_ms);

	dmabuf = system_heap_do_allocate(heap, len, fd_flags, heap_flags, true);
	KTIME_GET_END_DURATION(start, end, duration_ms);

	if (duration_ms > system_alloc_timeout || IS_ERR(dmabuf)) {
		xrheap_info("len: 0x%lx bytes, duration_ms: %lld, dmabuf ok? %c:%ld\n",
			len, duration_ms, !IS_ERR(dmabuf) ? 'Y' : 'N', !IS_ERR(dmabuf) ? 0 : PTR_ERR(dmabuf));
		xr_meminfo_show(true, FROM_DMAHEAP);
	}
	return dmabuf;
}

/* Dummy function to be used until we can call coerce_mask_and_coherent */
static struct dma_buf *system_uncached_heap_not_initialized(struct dma_heap *heap,
							    unsigned long len,
							    u32 fd_flags,
							    u64 heap_flags)
{
	return ERR_PTR(-EBUSY);
}

static struct dma_heap_ops system_uncached_heap_ops = {
	/* After system_heap_create is complete, we will swap this */
	.allocate = system_uncached_heap_not_initialized,
};

int sgl_direct_read_range(int file_fd, int dmabuf_fd, loff_t mem_offset,
								loff_t file_offset, size_t length)
{
	struct scatterlist *sg = NULL;
	struct dma_buf *dmabuf = NULL;
	struct system_heap_buffer *buffer = NULL;
	struct file *file = NULL;
	struct scatterlist *sgl = NULL;
	unsigned int nents;
	unsigned int len = 0;
	struct iov_iter iter;
	struct bio_vec *bvec;
	size_t read_length = 0;
	size_t length_tmp = length;
	size_t offset_tmp = mem_offset;
	int i, index = 0, bvec_index = 0, bvec_count = 0, bvec_nents = 0;
	int ret = 0;

	xrheap_info("enter\n");

	file = fget(file_fd);
	dmabuf = dma_buf_get(dmabuf_fd);
	if (IS_ERR(dmabuf)) {
		xrheap_err("%s fd is not dmabuf\n", __func__);
		ret = -ENOENT;
		goto out;
	}

	buffer = dmabuf->priv;
	sgl = buffer->sg_table.sgl;
	nents = buffer->sg_table.orig_nents;

	mutex_lock(&buffer->lock);

	for_each_sg(sgl, sg, nents, i) {
		unsigned int sg_offset, sg_left, size = 0;

		len += sg->length;
		if (len <= mem_offset)
			continue;

		sg_left = len - mem_offset;
		sg_offset = sg->length - sg_left;
		size = (length < sg_left) ? length : sg_left;

		bvec_nents++;

		length -= size;
		mem_offset += size;
		if (length == 0)
			break;
	}

	bvec = vmalloc(bvec_nents * sizeof(struct bio_vec));
	if (!bvec) {
		ret = -ENOMEM;
		goto alloc_err;
	}

	length = length_tmp;
	mem_offset = offset_tmp;
	len = 0;
	i = 0;

	for_each_sg(sgl, sg, nents, i) {
		unsigned int sg_offset, sg_left, size = 0;

		len += sg->length;
		if (len <= mem_offset)
			continue;

		if (fatal_signal_pending(current)) {
			ret = -EINTR;
			break;
		}

		sg_left = len - mem_offset;
		sg_offset = sg->length - sg_left;
		size = (length < sg_left) ? length : sg_left;

		bvec_set_page(&bvec[bvec_index], sg_page(sg), size, sg_offset);
		bvec_index++;
		bvec_count++;
		read_length += size;
		if (read_length >= system_read_fragment * SZ_1M || size == length) {
			iov_iter_bvec(&iter, 0, &bvec[index], bvec_count, read_length);
			ret = vfs_iter_read(file, &iter, &file_offset, 0);
			if (ret < 0) {
				xrheap_err("read returned %d\n", ret);
				break;
			}
			index += bvec_count;
			read_length = 0;
			bvec_count = 0;
		}

		length -= size;
		mem_offset += size;
		if (length == 0)
			break;
	}
	mutex_unlock(&buffer->lock);

	vfree(bvec);
alloc_err:
	dma_buf_put(dmabuf);
out:
	fput(file);

	return ret;
}

int xr_system_heap_create(struct xring_heap_platform_data *data)
{
	struct dma_heap_export_info exp_info;
	int i;

	for (i = 0; i < NUM_ORDERS; i++) {
		pools[i] = xr_dmabuf_page_pool_create(order_flags[i], orders[i]);

		if (IS_ERR(pools[i])) {
			int j;

			xrheap_err("page pool creation failed!\n");
			for (j = 0; j < i; j++)
				xr_dmabuf_page_pool_destroy(pools[j]);
			return PTR_ERR(pools[i]);
		}
	}

	set_system_heap_resv_pages(system_pool_resv_pages);

	exp_info.name = "system";
	exp_info.ops = &system_heap_ops;
	exp_info.priv = NULL;

	sys_heap = dma_heap_add(&exp_info);
	if (IS_ERR(sys_heap))
		return PTR_ERR(sys_heap);

	system_dmabuf_helper_ops.name = exp_info.name;
	system_dmabuf_helper_ops.priv = (void *)sys_heap;
	system_dmabuf_helper_ops.dio_alloc = sgl_direct_read_range;
	system_dmabuf_helper_ops.scene = HEAP_SYSTEM_DIO_ALLOC;
	if (xr_dmabuf_helper_register(&system_dmabuf_helper_ops)) {
		xrheap_err("xr_dmabuf_helper_register failed\n");
		return PTR_ERR(sys_heap);
	}

	exp_info.name = "system-uncached";
	exp_info.ops = &system_uncached_heap_ops;
	exp_info.priv = NULL;

	sys_uncached_heap = dma_heap_add(&exp_info);
	if (IS_ERR(sys_uncached_heap))
		return PTR_ERR(sys_uncached_heap);

	dma_coerce_mask_and_coherent(dma_heap_get_dev(sys_uncached_heap), DMA_BIT_MASK(64));
	mb(); /* make sure we only set allocate after dma_mask is set */
	system_uncached_heap_ops.allocate = system_uncached_heap_allocate;

	system_heap_create_refill_thread("system");

	return 0;
}

module_param(system_alloc_timeout, uint, 0644);
module_param(system_read_fragment, uint, 0644);
module_param(system_pool_refill_record, int, 0644);
module_param(system_pool_fill_mark, int, 0644);
module_param(system_pool_low_mark, int, 0644);
module_param_cb(system_pool_resv_pages, &heap_rsv_param_ops, &system_pool_resv_pages, 0644);
MODULE_LICENSE("GPL v2");
