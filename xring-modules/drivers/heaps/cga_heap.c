// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring cga heap
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/version.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/genalloc.h>
#include <linux/mutex.h>
#include <linux/of_fdt.h>
#include <linux/highmem.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/cma.h>
#include <linux/sizes.h>
#include <linux/memblock.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#include <linux/dma-heap.h>

#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <asm/bug.h>
#include <linux/highmem.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/xhee_hvc/hvc_id.h>

#include "soc/xring/xring_mem_adapter.h"
#include "cga_heap.h"

#define XRING_GENPOOL_LOW_WATER_MARK 0x2000000 // 32MB

u32 xr_cga_get_sfd(struct dma_buf *dmabuf)
{
	struct cga_heap_buffer *buffer = NULL;
	struct cga_heap *heap = NULL;

	buffer = dmabuf->priv;
	if (!buffer) {
		xrheap_err("dmabuf has no dma buffer!\n");
		return 0;
	}

	heap = buffer->heap;
	if (!heap) {
		xrheap_err("invalid dma buffer's heap!\n");
		return 0;
	}

	return buffer->sfd;
}
EXPORT_SYMBOL(xr_cga_get_sfd);

static int cga_change_mem_prot(struct cga_heap *cga_heap,
				struct cga_heap_buffer *buffer, u32 cmd)
{
	struct sg_table *table = buffer->sg_table;
	struct tz_pageinfo *pageinfo = NULL;
	unsigned int nents = table->nents;
	struct scatterlist *sg = NULL;
	struct tz_info *info = NULL;
	int ret = 0;
	int i = 0;
	unsigned long total = 0;

	if (cga_heap->heap_attr == HEAP_NORMAL)
		return ret;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	if (cmd == XRING_SEC_CMD_ALLOC_TEE) {
		total = roundup(nents * sizeof(*pageinfo), PAGE_SIZE);
		pageinfo = kzalloc(total, GFP_KERNEL);
		if (!pageinfo) {
			xrheap_err("pageinfo alloc failed\n");
			ret = -ENOMEM;
			goto free_info;
		}

		for_each_sgtable_sg(table, sg, i) {
			struct page *page = sg_page(sg);

			pageinfo[i].addr = page_to_phys(page);
			pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
		}

		info->pageinfo = pageinfo;
		info->size = total;
		info->nents = nents;
	} else {
		info->sfd = buffer->sfd;
	}

	info->type = CGA_HEAP;
	ret = xr_change_mem_prot_tee(info, cmd);
	if (ret) {
		xrheap_err("TEE excute failed\n");
		goto err;
	}

	if (cmd == XRING_SEC_CMD_ALLOC_TEE) {
		buffer->sfd = info->sfd;
		xrheap_info("sfd: 0x%llx\n", buffer->sfd);
		kfree(pageinfo);
	}
	kfree(info);
	return 0;

err:
	if (cmd == XRING_SEC_CMD_ALLOC_TEE)
		kfree(pageinfo);
free_info:
	kfree(info);
	return ret;
}

static bool genpool_below_low_watermark(struct gen_pool *pool)
{
	return gen_pool_avail(pool) < XRING_GENPOOL_LOW_WATER_MARK;
}

static inline void free_alloc_list(struct list_head *head)
{
	struct alloc_list *pos = NULL;

	while (!list_empty(head)) {
		pos = list_first_entry(head, struct alloc_list, list);
		if (pos->addr || pos->size)
			xrheap_err("0x%pK 0x%x failed\n",
					(void *)(uintptr_t)pos->addr, pos->size);

		list_del(&pos->list);
		kfree(pos);
	}
}

static u32 count_list_nr(struct cga_heap *cga_heap)
{
	struct list_head *head = &cga_heap->allocate_head;
	struct list_head *pos = NULL;
	u32 nr = 0;

	list_for_each(pos, head)
		nr++;
	return nr;
}

static int cons_phys_struct(struct cga_heap *cga_heap, u32 nents,
			    struct list_head *entry, u32 cmd)
{
	struct tz_info *info = NULL;
	struct tz_pageinfo *pageinfo = NULL;
	unsigned long size = nents * sizeof(*pageinfo);
	struct alloc_list *tmp_list = list_entry(entry, struct alloc_list, list);
#ifdef CONFIG_XRING_HEAP_NOC_DFX
	struct sg_table *table = NULL;
	struct page *page = NULL;
#endif
	int ret = 0;

	if (cga_heap->heap_attr == HEAP_NORMAL)
		return ret;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	size = roundup(size, PAGE_SIZE);
	pageinfo = kzalloc(size, GFP_KERNEL);
	if (!pageinfo) {
		xrheap_err("pageinfo alloc failed\n");
		ret = -ENOMEM;
		goto free_info;
	}

	pageinfo[0].addr = tmp_list->addr;
	pageinfo[0].nr_pages = tmp_list->size / PAGE_SIZE;
	xrheap_debug("addr: 0x%pK, size: 0x%x\n", (void *)tmp_list->addr, tmp_list->size);

	info->pageinfo = pageinfo;
	info->size = size;
	info->nents = nents;

	/* only need identify if send free cmd */
	if (cmd == XRING_SEC_CMD_TABLE_CLEAN)
		info->sfd = tmp_list->id;

	if (cmd == XRING_SEC_CMD_TABLE_SET) {
		ret = xr_xhee_action(info, FID_XHEE_MEM_LEND);
		if (ret < 0) {
			xrheap_err("XHEE excute failed\n");
			goto err;
		}
	}

	info->type = CGA_HEAP;
	ret = xr_change_mem_prot_tee(info, cmd);
	if (ret < 0) {
		xrheap_err("TEE excute failed\n");
		goto err;
	}

	if (cmd == XRING_SEC_CMD_TABLE_CLEAN) {
		ret = xr_xhee_action(info, FID_XHEE_MEM_RECLAIM);
		if (ret < 0) {
			xrheap_err("XHEE excute failed\n");
			goto err;
		}
	}

	if (cmd == XRING_SEC_CMD_TABLE_SET) {
		tmp_list->id = info->sfd;
		xrheap_info("sfd: 0x%llx\n", tmp_list->id);
	}
	kfree(pageinfo);
	kfree(info);

#ifdef CONFIG_XRING_HEAP_NOC_DFX
	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return 0;

	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		kfree(table);
		xrheap_err("sg_alloc_table failed\n");
		return 0;
	}

	page = phys_to_page(tmp_list->addr);
	sg_set_page(table->sgl, page, (unsigned int)size, 0);

	if (cmd & XRING_SET_PROTECT_OR_SAFE)
		xr_change_page_flag(table, true);
	else if (cmd & XRING_CLEAR_PROTECT_OR_SAFE)
		xr_change_page_flag(table, false);

	sg_free_table(table);
	kfree(table);
#endif

	return 0;

err:
	kfree(pageinfo);
free_info:
	kfree(info);
	return ret;
}

static bool gen_pool_bulk_free(struct gen_pool *pool, u32 size)
{
	unsigned long offset = 0;
	u32 i;

	for (i = 0; i < (size / PAGE_SIZE); i++) {
		offset = gen_pool_alloc(pool, PAGE_SIZE);
		if (!offset) {
			xrheap_err("gen_pool_alloc failed!\n");
			return false;
		}
	}
	return true;
}

static void cga_free_pool_to_cma(struct cga_heap *cga_heap)
{
#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	unsigned long virt = 0;
#endif
	struct list_head *head = &cga_heap->allocate_head;
	struct list_head *location = head->prev;
	struct alloc_list *pos = NULL;
	struct list_head *tmp = NULL;
	unsigned long size_remain = 0;
	unsigned long offset = 0;
	u32 nents = 0;
	u32 size = 0;
	u64 addr = 0;
	int ret, i, j;

	nents = count_list_nr(cga_heap);
	if (cga_heap->heap_attr & HEAP_TEE) {
		for (i = 0; (i < nents) && (location != head); i++) {
			ret = cons_phys_struct(cga_heap, 1, location,
					XRING_SEC_CMD_TABLE_CLEAN);
			location = location->prev;
			if (ret < 0) {
				WARN(1, "TEE release fail, verify that other components are in use\n");
				break;
			}
		}
	}

	i = nents;

	if (!list_empty(&cga_heap->allocate_head)) {
		tmp = head->prev;
		for (j = 0; j < i; j++) {
			pos = list_entry(tmp, struct alloc_list, list);
			addr = pos->addr;
			size = pos->size;
			offset = gen_pool_alloc(cga_heap->pool, size);

			xrheap_debug("addr: 0x%pK, size: 0x%x\n", (void *)addr, size);
			if (!offset) {
				xrheap_err("gen_pool_alloc failed! %pK %x\n", (void *)addr, size);
				continue;
			}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
			if (cga_heap->heap_attr & HEAP_TEE) {
				virt = (unsigned long)__va(addr);
				xring_change_secpage_range(addr, virt, size, PAGE_KERNEL);
				flush_tlb_all();
			}
#endif
			cma_release(cga_heap->cma, phys_to_page(addr), size >> PAGE_SHIFT);

			pos->addr = 0;
			pos->size = 0;
			tmp = tmp->prev;
		}
	}

	if (!list_empty(&cga_heap->allocate_head)) {
		tmp = head->prev;
		for (j = 0; j < i; j++) {
			pos = list_entry(tmp, struct alloc_list, list);
			addr = pos->addr;
			size = pos->size;

			if (!addr || !size)
				continue;

			if (unlikely(!gen_pool_bulk_free(cga_heap->pool, size))) {
				xrheap_err("bulk_free failed! %pK %x\n", (void *)addr, size);
				continue;
			}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
			if (cga_heap->heap_attr & HEAP_TEE) {
				virt = (uintptr_t)__va(addr);
				xring_change_secpage_range(addr, virt, size, PAGE_KERNEL);
				flush_tlb_all();
			}
#endif
			cma_release(cga_heap->cma, phys_to_page(addr), size >> PAGE_SHIFT);

			pos->addr = 0;
			pos->size = 0;
			tmp = tmp->prev;
		}
		free_alloc_list(&cga_heap->allocate_head);
	}

	size_remain = gen_pool_avail(cga_heap->pool);
	if (size_remain)
		xrheap_err("size_remain = 0x%lx(0x%lx)\n", size_remain, offset);
}

static int alloc_range_update(struct page *pg, u64 size,
			struct cga_heap *cga_heap, unsigned int count)
{
#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	unsigned long virt = 0;
#endif
	struct alloc_list *alloc = NULL;
	int ret = 0;

	alloc = kzalloc(sizeof(*alloc), GFP_KERNEL);
	if (!alloc) {
		ret = -ENOMEM;
		goto err_out1;
	}
	alloc->addr = page_to_phys(pg);
	alloc->size = (unsigned int)size;
	alloc->id = 0;
	list_add_tail(&alloc->list, &cga_heap->allocate_head);
	xrheap_debug("addr: 0x%pK, size: 0x%x\n", (void *)alloc->addr, alloc->size);

	if (cga_heap->heap_attr & HEAP_TEE) {
#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
		dma_sync_single_for_device(cga_heap->dev, alloc->addr,
					alloc->size, DMA_BIDIRECTIONAL);
		virt = (uintptr_t)__va(alloc->addr);
		xring_change_secpage_range(alloc->addr, virt, size,
							__pgprot(PROT_DEVICE_nGnRE));
		flush_tlb_all();
#endif
		if (cons_phys_struct(cga_heap, 1, (&cga_heap->allocate_head)->prev,
					XRING_SEC_CMD_TABLE_SET)) {
			ret = -EINVAL;
			goto err_out2;
		}
	}

	cga_heap->cma_alloc_size +=  size;
	gen_pool_free(cga_heap->pool, page_to_phys(pg), size);
	xrheap_debug("heap attr: %d, cma alloc size: 0x%zx\n"
			"size: 0x%llx\n", cga_heap->heap_attr,
			cga_heap->cma_alloc_size, size);
	return 0;

err_out2:
#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	xring_change_secpage_range(alloc->addr, virt, size, PAGE_KERNEL);
	flush_tlb_all();
#endif
	list_del(&alloc->list);
	kfree(alloc);
err_out1:
	return ret;
}

static void cga_free_to_pool(struct cga_heap *cga_heap,
			      struct sg_table *table,
			      struct cga_heap_buffer *buffer)
{
	struct page *page = sg_page(table->sgl);
	phys_addr_t paddr = PFN_PHYS(page_to_pfn(page));

	cga_change_mem_prot(cga_heap, buffer, XRING_SEC_CMD_FREE_TEE);
	gen_pool_free(cga_heap->pool, paddr, buffer->len);
}

static void cga_contiguous_free_internal(struct cga_heap *cga_heap,
			 struct cga_heap_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;

	mutex_lock(&cga_heap->mutex);
	cga_free_to_pool(cga_heap, table, buffer);
	WARN_ON(cga_heap->alloc_size < buffer->len);
	cga_heap->alloc_size -= buffer->len;
	if (!cga_heap->alloc_size) {
		if (cga_heap->pre_alloc_attr)
			cancel_work_sync(&cga_heap->pre_alloc_work);
		cga_free_pool_to_cma(cga_heap);
		cga_heap->cma_alloc_size = 0;
	}
	mutex_unlock(&cga_heap->mutex);
	xrheap_debug("cga free: len=0x%lx, alloc size=0x%lx\n",
			buffer->len, cga_heap->alloc_size);
}

static void cga_contiguous_free(struct cga_heap *cga_heap,
			 struct cga_heap_buffer *buffer)
{
	struct sg_table *table = buffer->sg_table;

	cga_contiguous_free_internal(cga_heap, buffer);
	sg_free_table(table);
	kfree(table);
}

static int add_cma_to_pool_internel(struct cga_heap *cga_heap,
			   unsigned long user_alloc_size)
{
	u64 subregion_sz = cga_heap->subregion_size;
	unsigned long allocated_size = 0;
	unsigned long size_remain = 0;
	unsigned long cma_remain = 0;
	struct page *pg = NULL;
	unsigned int count = 0;
	u64 cma_size = 0;
	u64 size = 0;
	int ret = 0;

	/* subregion_sz = 2MB/4MB */
	allocated_size = cga_heap->alloc_size;
	size_remain = gen_pool_avail(cga_heap->pool);
	cma_size = cga_heap->cma_size;
	cma_remain = cma_size - (allocated_size + size_remain);
	if (cga_heap->heap_size <= (allocated_size + size_remain)) {
		xrheap_err("heap full! allocated_size: 0x%lx,\n"
			"remain_size: 0x%lx, heap_size: 0x%lx, cma_remain: 0x%lx\n",
			allocated_size, size_remain, cga_heap->heap_size, cma_remain);
		return -ENOMEM;
	}

	user_alloc_size = ALIGN(user_alloc_size, subregion_sz);
	size = (user_alloc_size > cma_remain) ? cma_remain : user_alloc_size;
	/* we allocate as much contiguous memory as we can. */
	count = (unsigned int)size >> PAGE_SHIFT;
	pg = cma_alloc(cga_heap->cma, (size_t)count,
			(u32)get_order((u32)subregion_sz), false);

	if (!pg) {
		pg = cma_alloc(cga_heap->cma, (size_t)count,
			(u32)get_order((u32)subregion_sz), false);
		if (!pg) {
			xrheap_err("cma alloc failed, allocated size:0x%lx\n",
					allocated_size);
			return -ENOMEM;
		}
	}

	ret = alloc_range_update(pg, size, cga_heap, count);
	if (ret) {
		cma_release(cga_heap->cma, pg, count);
		xrheap_err("failed\n");
	}
	return ret;
}

static void cga_pre_alloc_wk_func(struct work_struct *work)
{
	struct cga_heap *cga_heap = NULL;
	int ret = 0;

	cga_heap = container_of(work, struct cga_heap, pre_alloc_work);

	mutex_lock(&cga_heap->pre_alloc_mutex);
	if (cga_heap->cma_alloc_size < cga_heap->heap_size) {
		ret = add_cma_to_pool_internel(cga_heap, cga_heap->append_size);
		if (ret)
			xrheap_err("pre alloc cma to fill heap pool failed!\n"
					"cma allocated sz(0x%lx), append size(0x%llx)\n",
					cga_heap->cma_alloc_size, cga_heap->append_size);
	}
	mutex_unlock(&cga_heap->pre_alloc_mutex);
}

static int add_cma_to_pool(struct cga_heap *cga_heap,
			   unsigned long size)
{
	int ret = 0;

	if (cga_heap->pre_alloc_attr) {
		mutex_lock(&cga_heap->pre_alloc_mutex);
		if (gen_pool_avail(cga_heap->pool) < size &&
			cga_heap->cma_alloc_size < cga_heap->heap_size)
			ret = add_cma_to_pool_internel(cga_heap, size);
		mutex_unlock(&cga_heap->pre_alloc_mutex);
	} else {
		ret = add_cma_to_pool_internel(cga_heap, size);
	}

	return ret;
}

static int cga_contiguous_alloc(struct cga_heap *cga_heap,
			 struct cga_heap_buffer *buffer,
			 unsigned long size)
{
	struct sg_table *table = NULL;
	unsigned long offset = 0;
	struct page *page = NULL;
	int ret = 0;

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		xrheap_err("sg_alloc_table failed\n");
		ret = -ENOMEM;
		goto err_out1;
	}

	mutex_lock(&cga_heap->mutex);
	offset = gen_pool_alloc(cga_heap->pool, size);
	if (!offset) {
		ret = add_cma_to_pool(cga_heap, size);
		if (ret) {
			mutex_unlock(&cga_heap->mutex);
			goto err_out2;
		}
		offset = gen_pool_alloc(cga_heap->pool, size);
		if (!offset) {
			ret = -ENOMEM;
			mutex_unlock(&cga_heap->mutex);
			xrheap_err("gen_pool_alloc failed!\n");
			goto err_out2;
		}
	}
	cga_heap->alloc_size += size;

	if (cga_heap->pre_alloc_attr && genpool_below_low_watermark(cga_heap->pool))
		schedule_work(&cga_heap->pre_alloc_work);
	mutex_unlock(&cga_heap->mutex);

	page = pfn_to_page(PFN_DOWN(offset));
	sg_set_page(table->sgl, page, (unsigned int)size, 0);
	buffer->sg_table = table;
	ret = cga_change_mem_prot(cga_heap, buffer, XRING_SEC_CMD_ALLOC_TEE);
	if (ret < 0) {
		xrheap_err("change_mem_prot failed\n");
		goto err_out3;
	}
	return ret;

err_out3:
	cga_contiguous_free_internal(cga_heap, buffer);
err_out2:
	sg_free_table(table);
err_out1:
	kfree(table);
	return ret;
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table = NULL;
	struct scatterlist *new_sg = NULL;
	struct scatterlist *sg = NULL;
	int i = 0;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	if (sg_alloc_table(new_table, table->nents, GFP_KERNEL)) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sg(table->sgl, sg, table->nents, i) {
		memcpy(new_sg, sg, sizeof(*sg));
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static int cga_heap_map_user(struct cga_heap *heap,
				   struct cga_heap_buffer *buffer,
				   struct vm_area_struct *vma)
{
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	struct sg_table *table = buffer->sg_table;
	unsigned long addr = vma->vm_start;
	struct scatterlist *sg = NULL;
	int ret = 0;
	int i = 0;

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
			xrheap_err("remap_pfn_range fail\n");
			return ret;
		}
		addr += len;
		if (addr >= vma->vm_end)
			goto done;
	}

done:
	return 0;
}

static void *cga_heap_map_kernel(struct cga_heap *heap,
					struct cga_heap_buffer *buffer)
{
	int npages = PAGE_ALIGN(buffer->len) / PAGE_SIZE;
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	struct page **pages = NULL;
	struct page **tmp = NULL;
	void *vaddr = NULL;
	pgprot_t pgprot = PAGE_KERNEL;
	int i = 0, j = 0;
	int handled_page = 0;

	pages = vmalloc(sizeof(struct page *) * npages);
	if (!pages)
		return ERR_PTR(-ENOMEM);

	tmp = pages;

	for_each_sg(table->sgl, sg, table->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		handled_page += npages_this_entry;

		if (unlikely(i >= npages)) {
			xrheap_err("sgt entry nums is more than npages\n");
			vfree(pages);
			return ERR_PTR(-EINVAL);
		}
		if (unlikely(handled_page > npages)) {
			xrheap_err("sgt entry total nums is more than npages\n");
			vfree(pages);
			return ERR_PTR(-EINVAL);
		}

		for (j = 0; j < npages_this_entry; j++)
			*(tmp++) = page++;
	}
	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);

	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	return vaddr;
}

struct dma_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
	bool mapped;
};

static int xr_cga_heap_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct cga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;
	struct sg_table *table = NULL;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need attach.\n");
		return 0;
	}

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
	a->mapped = false;
	attachment->priv = a;

	INIT_LIST_HEAD(&a->list);
	mutex_lock(&buffer->mutex);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->mutex);

	return 0;
}

static void xr_cga_heap_detach(struct dma_buf *dmabuf,
				struct dma_buf_attachment *attachment)
{
	struct dma_heap_attachment *a = attachment->priv;
	struct cga_heap_buffer *buffer = dmabuf->priv;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need detach.\n");
		return;
	}

	mutex_lock(&buffer->mutex);
	list_del(&a->list);
	mutex_unlock(&buffer->mutex);
	sg_free_table(a->table);
	kfree(a->table);
	kfree(a);
}

static struct sg_table *xr_cga_heap_map_dma_buf(struct dma_buf_attachment *attachment,
					enum dma_data_direction direction)
{
	struct cga_heap_buffer *buffer = attachment->dmabuf->priv;
	struct dma_heap_attachment *a = attachment->priv;
	int attr = attachment->dma_map_attrs;
	struct sg_table *table = a->table;
	int ret;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need map.\n");
		return NULL;
	}
	ret = xring_heap_dma_map(attachment->dev, attachment->dmabuf, table, direction, attr);
	if (ret)
		return ERR_PTR(ret);

	a->mapped = true;
	return table;
}

static void xr_cga_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
			      struct sg_table *table,
			      enum dma_data_direction direction)
{
	struct dma_buf *dmabuf = attachment->dmabuf;
	struct cga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = attachment->priv;
	int attr = attachment->dma_map_attrs;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need unmap.\n");
		return;
	}

	a->mapped = false;
	xring_heap_dma_unmap(attachment->dev, attachment->dmabuf, table, direction, attr);
}

static int xr_cga_heap_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction direction)
{
	struct cga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need invalid cache.\n");
		return 0;
	}

	mutex_lock(&buffer->mutex);
	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_cpu(a->dev, a->table, direction);
		break;
	}
	mutex_unlock(&buffer->mutex);

	return 0;
}

static int xr_cga_heap_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
				      enum dma_data_direction direction)
{
	struct cga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need flush cache.\n");
		return 0;
	}

	mutex_lock(&buffer->mutex);
	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_device(a->dev, a->table, direction);
	}
	mutex_unlock(&buffer->mutex);

	return 0;
}

static int xr_cga_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct cga_heap_buffer *buffer = dmabuf->priv;
	int ret = 0;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need map userspace.\n");
		return 0;
	}

	/* now map it to userspace */
	ret = cga_heap_map_user(buffer->heap, buffer, vma);
	if (ret)
		xrheap_err("failure mapping buffer to userspace\n");

	return ret;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int xr_cga_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static int xr_cga_heap_vmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct cga_heap_buffer *buffer = dmabuf->priv;
	void *vaddr = NULL;
	int ret = 0;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need vmap.\n");
		return 0;
	}

	mutex_lock(&buffer->mutex);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
		dma_buf_map_set_vaddr(map, buffer->vaddr);
#else
		iosys_map_set_vaddr(map, buffer->vaddr);
#endif
		goto out;
	}
	vaddr = cga_heap_map_kernel(buffer->heap, buffer);
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
	mutex_unlock(&buffer->mutex);

	return ret;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static void xr_cga_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static void xr_cga_heap_vunmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct cga_heap_buffer *buffer = dmabuf->priv;

	if (buffer->heap->heap_attr & HEAP_TEE) {
		xrheap_err("this heap not used in REE, do not need vunmap.\n");
		return;
	}

	mutex_lock(&buffer->mutex);
	buffer->vmap_cnt--;
	if (!buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->mutex);
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	dma_buf_map_clear(map);
#else
	iosys_map_clear(map);
#endif
}

static void xr_cga_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct cga_heap_buffer *buffer = dmabuf->priv;

	if (buffer->vmap_cnt > 0) {
		WARN(1, "buffer still mapped in the kernel\n");
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}

	cga_contiguous_free(buffer->heap, buffer);
	kfree(buffer);
}

static const struct dma_buf_ops cga_heap_buf_ops = {
	.attach = xr_cga_heap_attach,
	.detach = xr_cga_heap_detach,
	.map_dma_buf = xr_cga_heap_map_dma_buf,
	.unmap_dma_buf = xr_cga_heap_unmap_dma_buf,
	.begin_cpu_access = xr_cga_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = xr_cga_heap_dma_buf_end_cpu_access,
	.mmap = xr_cga_heap_mmap,
	.vmap = xr_cga_heap_vmap,
	.vunmap = xr_cga_heap_vunmap,
	.release = xr_cga_heap_dma_buf_release,
};

static int cga_create_pool(struct cga_heap *cga_heap)
{
	u64 cma_base = 0;
	u64 cma_size = 0;
	int ret = 0;

	cga_heap->pool = gen_pool_create((int)cga_heap->genpool_shift, -1);
	if (!cga_heap->pool) {
		xrheap_err("genpool create failed\n");
		return -ENOMEM;
	}
	gen_pool_set_algo(cga_heap->pool, gen_pool_best_fit, NULL);

	/* Add all memory to genpool first，one chunk only */
	cma_base = cga_heap->cma_base;
	cma_size = cga_heap->cma_size;

	if (gen_pool_add(cga_heap->pool, cma_base, cma_size, -1)) {
		xrheap_err("genpool add cma_size 0x%llx\n", cma_size);
		ret = -ENOMEM;
		goto err_add;
	}

	if (!gen_pool_alloc(cga_heap->pool, cma_size)) {
		xrheap_err("genpool alloc failed\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	return ret;
err_alloc:
	gen_pool_destroy(cga_heap->pool);
err_add:
	cga_heap->pool = NULL;
	return ret;
}

static int cga_input_check(struct cga_heap *cga_heap,
				    unsigned long size)
{
	if (cga_heap->alloc_size + size <= cga_heap->alloc_size) {
		xrheap_err("size overflow! alloc_size = 0x%lx, size = 0x%lx,\n",
			cga_heap->alloc_size, size);
		return -EINVAL;
	}

	if ((cga_heap->alloc_size + size) > cga_heap->heap_size) {
		xrheap_err("heap fill!\n"
			"alloc_size = 0x%lx, size = 0x%lx, heap_size = 0x%lx\n",
			cga_heap->alloc_size, size, cga_heap->heap_size);
		return -EINVAL;
	}

	return 0;
}

static struct dma_buf *xr_cga_heap_do_allocate(struct dma_heap *heap,
				   unsigned long len,
				   u32 fd_flags,
				   u64 heap_flags)
{
	struct cga_heap *cga_heap = dma_heap_get_drvdata(heap);
	struct cga_heap_buffer *buffer = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	size_t size = PAGE_ALIGN(len);
	struct dma_buf *dmabuf = NULL;
	int ret = 0;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->mutex);
	buffer->len = size;
	buffer->heap = cga_heap;

	mutex_lock(&cga_heap->mutex);
	if (cga_input_check(cga_heap, size)) {
		mutex_unlock(&cga_heap->mutex);
		xrheap_err("input params failed\n");
		ret = -EINVAL;
		goto free_buffer;
	}
	mutex_unlock(&cga_heap->mutex);

	ret = cga_contiguous_alloc(cga_heap, buffer, size);
	if (unlikely(ret))
		goto free_buffer;

	xrheap_info("success: alloc size=0x%lx, size=0x%lx, len=0x%lx\n",
		cga_heap->alloc_size, size, len);

	/* create the dmabuf */
	exp_info.exp_name = dma_heap_get_name(heap);
	exp_info.ops = &cga_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto free_page;
	}

	return dmabuf;
free_page:
	cga_contiguous_free(cga_heap, buffer);
free_buffer:
	kfree(buffer);

	return ERR_PTR(ret);
}

static struct dma_buf *xr_cga_heap_allocate(struct dma_heap *heap, unsigned long len,
				   u32 fd_flags, u64 heap_flags)
{
	struct dma_buf *dmabuf;

	dmabuf = xr_cga_heap_do_allocate(heap, len, fd_flags, heap_flags);
	if (IS_ERR(dmabuf)) {
		xrheap_err("dmabuf error %ld\n", PTR_ERR(dmabuf));
		xr_meminfo_show(true, FROM_DMAHEAP);
	}
	return dmabuf;
}

static struct dma_heap_ops cga_heap_ops = {
	.allocate = xr_cga_heap_allocate,
};

static int cga_parse_dt(struct cga_heap *cga_heap,
			    struct xring_heap_platform_data *heap_data)
{
	struct device_node *nd = heap_data->node;
	u32 genpool_shift = XRING_GP_SHIFT;
	u32 pre_alloc_attr = 0;
	u64 subregion_size = 0;
	u64 append_size = 0;
	u32 heap_size = 0;
	int ret = 0;

	ret = of_property_read_u32(nd, "pre-alloc-attr", &pre_alloc_attr);
	if (ret < 0) {
		xrheap_err("can't find prop: pre-alloc-attr.\n");
		pre_alloc_attr = 0;
	}
	cga_heap->pre_alloc_attr = pre_alloc_attr;

	ret = of_property_read_u64(nd, "subregion-size", &subregion_size);
	if (ret < 0) {
		xrheap_err("can't find prop: subregion-size\n");
		goto out;
	}
	cga_heap->subregion_size = PAGE_ALIGN(subregion_size);

	ret = of_property_read_u64(nd, "append-size", &append_size);
	if (ret < 0) {
		xrheap_err("can't find prop: append-size\n");
		goto out;
	}
	cga_heap->append_size = PAGE_ALIGN(append_size);

	ret = of_property_read_u32(nd, "genpool-shift", &genpool_shift);
	if (ret < 0) {
		xrheap_err("can not find prop: genpool-shift.\n");
		genpool_shift = XRING_GP_SHIFT;
	}
	cga_heap->genpool_shift = genpool_shift;

	ret = of_property_read_u32(nd, "heap-size", &heap_size);
	if (ret < 0) {
		xrheap_err("can't find prop: heap-size.\n");
		goto out;
	}
	cga_heap->heap_size = heap_size;

out:
	return ret;
}

static u64 get_alloc_size_cga_heap(void *data)
{
	struct cga_heap *cga_heap = (struct cga_heap *)data;
	u64 size = 0;

	size = cga_heap->alloc_size;

	return size;
}

static u64 get_cma_consumed_cga_heap(void *data)
{
	struct cga_heap *cga_heap = (struct cga_heap *)data;
	u64 size = 0;

	size = cga_heap->cma_alloc_size;

	return size;
}

static u64 get_pool_cached_cga_heap(void *data)
{
	struct cga_heap *cga_heap = (struct cga_heap *)data;
	size_t avail = gen_pool_avail(cga_heap->pool);
	return avail;
}


int xr_cga_heap_create(struct xring_heap_platform_data *heap_data)
{
	struct dma_heap_export_info exp_info;
	struct cga_heap *cga_heap = NULL;
	struct xr_dmabuf_helper_ops *helper_ops;
	int ret = 0;

	cga_heap = devm_kzalloc(heap_data->priv, sizeof(*cga_heap), GFP_KERNEL);
	if (!cga_heap)
		return -ENOMEM;

	mutex_init(&cga_heap->mutex);
	mutex_init(&cga_heap->pre_alloc_mutex);

	cga_heap->pool = NULL;
	cga_heap->alloc_size = 0;
	cga_heap->cma_alloc_size = 0;
	cga_heap->dev = heap_data->priv;
	cga_heap->cma = heap_data->cma;
	cga_heap->cma_base = heap_data->cma_base;
	cga_heap->cma_size = heap_data->cma_size;
	cga_heap->heap_attr = heap_data->heap_attr;
	INIT_LIST_HEAD(&cga_heap->allocate_head);

	if (cga_parse_dt(cga_heap, heap_data))
		return -EINVAL;

	helper_ops = devm_kzalloc(heap_data->priv, sizeof(*helper_ops), GFP_KERNEL);
	helper_ops->name = heap_data->heap_name;
	if (cga_heap->heap_attr & HEAP_TEE) {
		helper_ops->get_sfd = xr_cga_get_sfd;
		helper_ops->change_mem_prot = NULL;
	}
	helper_ops->get_alloc_size = get_alloc_size_cga_heap;
	helper_ops->get_buddy_consumed = NULL;
	helper_ops->get_cma_consumed = get_cma_consumed_cga_heap;
	helper_ops->get_pool_cached = get_pool_cached_cga_heap;
	helper_ops->priv = cga_heap;
	if (xr_dmabuf_helper_register(helper_ops)) {
		xrheap_err("xr_dmabuf_helper_register failed\n");
		return -EINVAL;
	}

	ret = cga_create_pool(cga_heap);
	if (ret) {
		xrheap_err("pool create failed.\n");
		return -EINVAL;
	}

	if (cga_heap->pre_alloc_attr)
		INIT_WORK(&cga_heap->pre_alloc_work, cga_pre_alloc_wk_func);

	exp_info.name = heap_data->heap_name;
	exp_info.ops = &cga_heap_ops;
	exp_info.priv = cga_heap;
	cga_heap->heap = dma_heap_add(&exp_info);

	xrheap_info("%s info:\n"
		  "\t\t\t\theap attr: %u\n"
		  "\t\t\t\tpre alloc attr: %u\n"
		  "\t\t\t\tpool shift: %u\n"
		  "\t\t\t\theap size: %lu MB\n"
		  "\t\t\t\tappend size:  %llu MB\n"
		  "\t\t\t\tsubregion size: %llu MB\n"
		  "\t\t\t\tcma size: %lu MB\n",
		  heap_data->heap_name,
		  cga_heap->heap_attr,
		  cga_heap->pre_alloc_attr,
		  cga_heap->genpool_shift,
		  cga_heap->heap_size / SZ_1M,
		  cga_heap->append_size / SZ_1M,
		  cga_heap->subregion_size / SZ_1M,
		  cga_heap->cma_size / SZ_1M);

	return 0;
}
MODULE_LICENSE("GPL v2");
