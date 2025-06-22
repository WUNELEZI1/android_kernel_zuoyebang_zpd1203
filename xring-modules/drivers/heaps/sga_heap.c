// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring sga heap
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the XRING heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/version.h>
#include <linux/cma.h>
#include <linux/err.h>
#include <linux/genalloc.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/sizes.h>
#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/dma-map-ops.h>
#include <linux/dma-direction.h>

#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/pgtable-types.h>
#include <asm/pgtable-prot.h>
#include <linux/highmem.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/xhee_hvc/hvc_id.h>

#include "linux/list.h"
#include "sga_heap.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "soc/xring/xring_mem_adapter.h"

/**
 * struct alloc_list - represents sga heap alloc cma memory
 * @addr		alloc cma base
 * @size		alloc cma size
 * @list	cma region list
 */
struct alloc_list {
	u64 addr;
	u32 size;
	struct list_head list;
};

/**
 * struct sga_heap - represents a sga heap in system
 * @dma_heap		point to the parent class of dma heap
 * @dev				point to the xring_heaps device
 * @cma				point to the related cma of this heap
 * @mutex			heap mutex
 * @heap_size		total heap size
 * @alloc_size		heap allocated size
 * @subregion_size	subregion align size
 * @per_alloc_sz	each sg section size
 * @heap_attr		heap attr, for different scene
 *
 * Represents a secure heap which alloc non-contiguous physic memory.
 */
struct sga_heap {
	struct dma_heap *heap;
	struct device *dev;
	struct cma *cma;
	struct mutex mutex;
	size_t heap_size;
	size_t alloc_size;
	size_t heap_scene;
	struct gen_pool *pool;
	u32 genpool_shift;
	u32 pre_alloc_attr;
	u64 append_size;
	struct work_struct pre_alloc_work;
	u64 subregion_size;
	u64 per_alloc_size;
	u32 heap_attr;
	struct mutex pre_alloc_mutex;
	struct list_head allocate_head;
	size_t cma_alloc_size;
};

/**
 * struct sga_heap_buffer - represents sga heap shared buffer
 * @heap			point to sga heap
 * @sg_table		all memory store in sg table
 * @mutex			buffer mutex
 * @attachments		devices that attch to this buffer
 * @vaddr			kernel address
 * @len				buffer alloc length
 * @sfd				secure fd
 * @vmap_cnt		vmap times
 * @mmap_cnt		mmap times
 * @TEE_setted		change mem prot in TEE
 */
struct sga_heap_buffer {
	struct xr_heap_buffer_header header;
	struct sga_heap *heap;
	struct sg_table *sg_table;
	struct mutex mutex;
	struct list_head attachments;
	void *vaddr;
	size_t len;
	u32 sfd;
	int vmap_cnt;
	int mmap_cnt;
	atomic_t TEE_setted;
};

static u32 count_list_nr(struct sga_heap *sga_heap)
{
	struct list_head *head = &sga_heap->allocate_head;
	struct list_head *pos = NULL;
	u32 nr = 0;

	list_for_each(pos, head)
		nr++;
	return nr;
}

static inline void free_alloc_list(struct list_head *head)
{
	struct alloc_list *pos = NULL;

	while (!list_empty(head)) {
		pos = list_first_entry(head, struct alloc_list, list);
		if (pos->addr || pos->size)
			xrheap_err("0x%pK 0x%x failed\n", (void *)(uintptr_t)pos->addr, pos->size);

		list_del(&pos->list);
		kfree(pos);
	}
}

static void sga_free_pool_to_cma(struct sga_heap *sga_heap)
{
	struct list_head *head = &sga_heap->allocate_head;
	struct alloc_list *pos = NULL;
	struct list_head *tmp = NULL;
	unsigned long size_remain = 0;
	unsigned long offset = 0;
	u32 nents = 0;
	u32 size = 0;
	u64 addr = 0;
	int entry = 0;

	nents = count_list_nr(sga_heap);

	if (!list_empty(&sga_heap->allocate_head)) {
		tmp = head->prev;
		for (entry = 0; entry < nents; entry++) {
			pos = list_entry(tmp, struct alloc_list, list);
			addr = pos->addr;
			size = pos->size;
			offset = gen_pool_alloc(sga_heap->pool, size);
			if (!offset) {
				xrheap_err("gen_pool_alloc failed! %pK %x\n", (void *)addr, size);
				continue;
			}

			cma_release(sga_heap->cma, phys_to_page(addr), size >> PAGE_SHIFT);
			pos->addr = 0;
			pos->size = 0;
			tmp = tmp->prev;
		}
		free_alloc_list(&sga_heap->allocate_head);
	}

	size_remain = gen_pool_avail(sga_heap->pool);
	if (size_remain)
		xrheap_err("size_remain = 0x%lx(0x%lx)\n", size_remain, offset);
}

static int add_cma_to_pool_internel(struct sga_heap *sga_heap,
				unsigned long user_alloc_size)
{
	u64 subregion_sz = sga_heap->subregion_size;
	unsigned long allocated_size = 0;
	unsigned long size_remain = 0;
	unsigned long cma_remain = 0;
	struct page *pg = NULL;
	unsigned int count = 0;
	phys_addr_t cma_size = 0;
	size_t size = 0;
	int ret = 0;

	allocated_size = sga_heap->alloc_size;
	size_remain = gen_pool_avail(sga_heap->pool);
	cma_size = (sga_heap->cma->count) << PAGE_SHIFT;
	cma_remain = cma_size - (allocated_size + size_remain);
	if (sga_heap->heap_size <= (allocated_size + size_remain)) {
		xrheap_err("heap full! allocated_size: 0x%lx,\n"
			"remain_size: 0x%lx, heap_size: 0x%lx, cma_remain: 0x%lx\n",
			allocated_size, size_remain, sga_heap->heap_size, cma_remain);
		return -ENOMEM;
	}

	user_alloc_size = ALIGN(user_alloc_size, subregion_sz);
	size = (user_alloc_size > cma_remain) ? cma_remain : user_alloc_size;
	count = (unsigned int)size >> PAGE_SHIFT;
	pg = cma_alloc(sga_heap->cma, (size_t)count,
			(u32)get_order((u32)subregion_sz), false);
	if (!pg) {
		pg = cma_alloc(sga_heap->cma, (size_t)count,
				(u32)get_order((u32)subregion_sz), false);
		if (!pg) {
			xrheap_err("cma alloc failed, allocated size:0x%lx\n", allocated_size);
			return -ENOMEM;
		}
	}

	gen_pool_free(sga_heap->pool, page_to_phys(pg), size);

	return ret;
}

static int add_cma_to_pool(struct sga_heap *sga_heap,
			   unsigned long size)
{
	int ret = 0;

	if (sga_heap->pre_alloc_attr) {
		mutex_lock(&sga_heap->pre_alloc_mutex);
		if (gen_pool_avail(sga_heap->pool) < size &&
			sga_heap->cma_alloc_size < sga_heap->heap_size)
			ret = add_cma_to_pool_internel(sga_heap, size);
		mutex_unlock(&sga_heap->pre_alloc_mutex);
	} else {
		ret = add_cma_to_pool_internel(sga_heap, size);
	}

	return ret;
}

static void sga_pre_alloc_wk_func(struct work_struct *work)
{
	struct sga_heap *sga_heap = NULL;
	int ret = 0;

	sga_heap = container_of(work, struct sga_heap, pre_alloc_work);

	mutex_lock(&sga_heap->pre_alloc_mutex);
	while (gen_pool_size(sga_heap->pool) < sga_heap->heap_size) {
		ret = add_cma_to_pool_internel(sga_heap, sga_heap->append_size);
		if (ret < 0) {
			xrheap_err("alloc 0x%llx failed\n", sga_heap->append_size);
			break;
		}
	}
	mutex_unlock(&sga_heap->pre_alloc_mutex);
}

int xr_sga_pre_alloc(void *heap, size_t len, int action)
{
	struct sga_heap *sga_heap = (struct sga_heap *)heap;
	int ret = 0;

	if (action & XR_ACTION_PREALLOC_START) {
		schedule_work(&sga_heap->pre_alloc_work);
	} else {
		ret = -1;
		xrheap_err("invalid action: 0x%x\n", action);
	}
	xrheap_debug("len: 0x%lx\n", len);

	return ret;
}

u32 xr_sga_get_sfd(struct dma_buf *dmabuf)
{
	struct sga_heap_buffer *buffer = NULL;
	struct sga_heap *heap = NULL;

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

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
static void sga_change_page_prot(struct sg_table *table,
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

	/*
	 * After change the pgtable prot, we need flush TLB and cache.
	 */
	flush_tlb_all();
}
#endif

static int sga_change_mem_prot(struct sga_heap *sga_heap,
				struct sga_heap_buffer *buffer, u32 cmd)
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
	if (!info)
		return -ENOMEM;

	if (cmd == XRING_SEC_CMD_ALLOC) {

		/*
		 * if TEE_setted == 0, means this buffer not setted in TEE, set it to 1
		 * if TEE_setted == 1, return 0
		 */
		if (!atomic_add_unless(&buffer->TEE_setted, 1, 1)) {
			xrheap_err("this buffer already set in TEE\n");
			ret = -EINVAL;
			goto free_info;
		}
	}
	total = roundup(nents * sizeof(*pageinfo), PAGE_SIZE);
	pageinfo = kzalloc(total, GFP_KERNEL);
	if (!pageinfo) {
		xrheap_err("pageinfo alloc failed\n");
		ret = -ENOMEM;
		goto free_info;
	}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	if (cmd == XRING_SEC_CMD_ALLOC) {
		for_each_sgtable_sg(buffer->sg_table, sg, i) {
			dma_sync_single_for_device(sga_heap->dev, sg_phys(sg),
						sg->length, DMA_BIDIRECTIONAL);
		}
		sga_change_page_prot(table, __pgprot(PROT_DEVICE_nGnRE));
	}
#endif

	for_each_sgtable_sg(table, sg, i) {
		struct page *page = sg_page(sg);

		pageinfo[i].addr = page_to_phys(page);
		pageinfo[i].nr_pages = sg->length / PAGE_SIZE;
	}

	info->pageinfo = pageinfo;
	info->size = total;
	info->nents = nents;

	if (cmd == XRING_SEC_CMD_ALLOC) {
		ret = xr_xhee_action(info, FID_XHEE_MEM_LEND);
		if (ret < 0) {
			xrheap_err("XHEE excute failed\n");
			goto err;
		}
	}

	if (cmd == XRING_SEC_CMD_FREE) {
		/* only need sfd if send free cmd */
		info->sfd = buffer->sfd;
	}

	info->type = SGA_HEAP;
	ret = xr_change_mem_prot_tee(info, cmd);
	if (ret < 0) {
		atomic_set(&buffer->TEE_setted, 0);
		xrheap_err("TEE excute failed\n");
		goto err;
	}

	if (cmd == XRING_SEC_CMD_FREE) {
		ret = xr_xhee_action(info, FID_XHEE_MEM_RECLAIM);
		if (ret < 0) {
			xrheap_err("XHEE excute failed\n");
			goto err;
		}
	}

#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	if (cmd == XRING_SEC_CMD_FREE)
		sga_change_page_prot(table, PAGE_KERNEL);
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

err:
#ifdef CONFIG_XRING_NEED_CHANGE_MAPPING
	if (cmd == XRING_SEC_CMD_ALLOC)
		sga_change_page_prot(table, PAGE_KERNEL);
#endif
	kfree(pageinfo);
free_info:
	kfree(info);
	return ret;
}

int xr_perpare_change_mem_prot(struct dma_buf *dmabuf)
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	struct sga_heap *heap = buffer->heap;
	int ret = 0;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this buffer is used in REE, or has been setted in TEE\n");
		return 0;
	}

	mutex_lock(&buffer->mutex);
	if (buffer->vmap_cnt) {
		mutex_unlock(&buffer->mutex);
		xrheap_err("this heap be in use, can't be set in TEE\n");
		return -EINVAL;
	}
	mutex_unlock(&buffer->mutex);

	ret = sga_change_mem_prot(heap, buffer, XRING_SEC_CMD_ALLOC);
	if (ret < 0)
		xrheap_err("failed\n");
	else
		xrheap_info("success\n");

	return ret;
}
EXPORT_SYMBOL(xr_perpare_change_mem_prot);

static int sga_heap_free(struct sga_heap_buffer *buffer)
{
	struct sga_heap *sga_heap = buffer->heap;
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg = NULL;
	int len = buffer->len;
	int ret = 0;
	u32 i = 0;

	mutex_lock(&sga_heap->mutex);
	if (sga_heap->heap_attr & HEAP_TEE) {
		ret = sga_change_mem_prot(sga_heap, buffer, XRING_SEC_CMD_FREE);
		if (ret < 0) {
			mutex_unlock(&sga_heap->mutex);
			return -EINVAL;
		}
	}

	for_each_sg(table->sgl, sg, table->nents, i) {
		gen_pool_free(sga_heap->pool, PFN_PHYS(page_to_pfn(sg_page(sg))), sg->length);
	}

	sg_free_table(table);
	kfree(table);

	sga_heap->alloc_size -= buffer->len;

	if (!sga_heap->alloc_size) {
		if (sga_heap->pre_alloc_attr)
			cancel_work_sync(&sga_heap->pre_alloc_work);
		sga_free_pool_to_cma(sga_heap);
		sga_heap->cma_alloc_size = 0;
	}
	mutex_unlock(&sga_heap->mutex);
	xrheap_info("len=0x%x, alloc size=0x%lx\n",
			len, sga_heap->alloc_size);
	return 0;
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct scatterlist *sg = NULL;
	struct scatterlist *new_sg = NULL;
	struct sg_table *new_table = NULL;
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

static int sga_heap_map_user(struct sga_heap *heap, struct sga_heap_buffer *buffer,
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

static void *sga_heap_map_kernel(struct sga_heap *heap,
			  struct sga_heap_buffer *buffer)
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

	if (!vaddr) {
		xrheap_err("vmap failed\n");
		return ERR_PTR(-ENOMEM);
	}

	return vaddr;
}

struct dma_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
	bool mapped;
};

static int xr_sga_heap_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;
	struct sg_table *table = NULL;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this heap not used in REE, do not need attach.\n");
		return 0;
	}

	table = dup_sg_table(buffer->sg_table);
	if (IS_ERR(table)) {
		xrheap_err("duped sgtable failed\n");
		return -ENOMEM;
	}
	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

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

static void xr_sga_heap_detatch(struct dma_buf *dmabuf,
				struct dma_buf_attachment *attachment)
{
	struct dma_heap_attachment *a = attachment->priv;
	struct sga_heap_buffer *buffer = dmabuf->priv;

	if (atomic_read(&buffer->TEE_setted)) {
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

static struct sg_table *xr_sga_heap_map_dma_buf(struct dma_buf_attachment *attachment,
					enum dma_data_direction direction)
{
	struct sga_heap_buffer *buffer = attachment->dmabuf->priv;
	struct dma_heap_attachment *a = attachment->priv;
	struct sg_table *table = a->table;
	int attr = attachment->dma_map_attrs;
	int ret = 0;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this heap not used in REE, do not need map.\n");
		return NULL;
	}

	ret = xring_heap_dma_map(attachment->dev, attachment->dmabuf, table, direction, attr);
	if (ret) {
		xrheap_err("map dma buf failed\n");
		return ERR_PTR(ret);
	}

	a->mapped = true;
	return table;
}

static void xr_sga_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
			      struct sg_table *table,
			      enum dma_data_direction direction)
{
	struct sga_heap_buffer *buffer = attachment->dmabuf->priv;
	struct dma_heap_attachment *a = attachment->priv;
	int attr = attachment->dma_map_attrs;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this heap not used in REE, do not need unmap.\n");
		return;
	}

	a->mapped = false;
	xring_heap_dma_unmap(attachment->dev, attachment->dmabuf, table, direction, attr);
}

static int xr_sga_heap_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction direction)
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this heap not used in REE, do not need invalid cache.\n");
		return -EINVAL;
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

static int xr_sga_heap_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
				      enum dma_data_direction direction)
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	struct dma_heap_attachment *a = NULL;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this heap not used in REE, do not need flush cache.\n");
		return -EINVAL;
	}

	mutex_lock(&buffer->mutex);
	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_device(a->dev, a->table, direction);
		break;
	}
	mutex_unlock(&buffer->mutex);

	return 0;
}

static int xr_sga_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	int ret = 0;

	if (atomic_read(&buffer->TEE_setted)) {
		xrheap_err("this heap not used in REE, do not need map userspace.\n");
		return -EINVAL;
	}

	/* now map it to userspace */
	ret = sga_heap_map_user(buffer->heap, buffer, vma);
	if (ret) {
		xrheap_err("failure mapping buffer to userspace\n");
		goto err;
	}
	mutex_lock(&buffer->mutex);
	buffer->mmap_cnt++;
	mutex_unlock(&buffer->mutex);
err:
	return ret;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int xr_sga_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static int xr_sga_heap_vmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	void *vaddr = NULL;
	int ret = 0;

	if (atomic_read(&buffer->TEE_setted)) {
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

	vaddr = sga_heap_map_kernel(buffer->heap, buffer);
	if (IS_ERR(vaddr)) {
		xrheap_err("map kernel failed\n");
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

	return 0;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static void xr_sga_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static void xr_sga_heap_vunmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct sga_heap_buffer *buffer = dmabuf->priv;

	if (atomic_read(&buffer->TEE_setted)) {
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

static void xr_sga_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct sga_heap_buffer *buffer = dmabuf->priv;
	int ret;

	if (buffer->vmap_cnt > 0) {
		WARN(1, "buffer still mapped in the kernel\n");
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}

	ret = sga_heap_free(buffer);
	if (ret < 0) {
		WARN(1, "TEE release fail, verify that other components are in use\n");
		return;
	}
	kfree(buffer);
}

static const struct dma_buf_ops sga_heap_buf_ops = {
	.attach = xr_sga_heap_attach,
	.detach = xr_sga_heap_detatch,
	.map_dma_buf = xr_sga_heap_map_dma_buf,
	.unmap_dma_buf = xr_sga_heap_unmap_dma_buf,
	.begin_cpu_access = xr_sga_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = xr_sga_heap_dma_buf_end_cpu_access,
	.mmap = xr_sga_heap_mmap,
	.vmap = xr_sga_heap_vmap,
	.vunmap = xr_sga_heap_vunmap,
	.release = xr_sga_heap_dma_buf_release,
};

static int sga_input_check(struct sga_heap *sga_heap,
				    unsigned long size)
{
	if (sga_heap->alloc_size + size <= sga_heap->alloc_size) {
		xrheap_err("size overflow! alloc_size=0x%lx, size=0x%lx,\n",
			sga_heap->alloc_size, size);
		return -EINVAL;
	}

	if ((sga_heap->alloc_size + size) > sga_heap->heap_size) {
		xrheap_err("heap fill!\n"
			"alloc size=0x%lx, size=0x%lx, heap size=0x%lx\n",
			sga_heap->alloc_size, size, sga_heap->heap_size);
		return -EINVAL;
	}

	return 0;
}

static struct page *sga_zalloc_large(struct sga_heap *sga_heap,
					unsigned long size,
					unsigned long align,
					struct list_head *list_head)
{
	struct page *page = NULL;
	unsigned long offset = 0;
	struct alloc_list *alloc = NULL;

	/* memory alloc from here is cacheable */
	mutex_lock(&sga_heap->mutex);
	offset = gen_pool_alloc(sga_heap->pool, size);
	if (!offset) {
		if (add_cma_to_pool(sga_heap, size)) {
			mutex_unlock(&sga_heap->mutex);
			goto out;
		}
		offset = gen_pool_alloc(sga_heap->pool, size);
		if (!offset) {
			mutex_unlock(&sga_heap->mutex);
			goto out;
		}

		alloc = kzalloc(sizeof(*alloc), GFP_KERNEL);
		if (!alloc) {
			mutex_unlock(&sga_heap->mutex);
			goto out;
		}

		alloc->addr = offset;
		alloc->size = size;
		list_add_tail(&alloc->list, list_head);
	}

	sga_heap->alloc_size += size;

	mutex_unlock(&sga_heap->mutex);

	page = pfn_to_page(PHYS_PFN(offset));
	memset(page_address(page), 0, (size_t)size);

out:
	return page;
}

int sga_zalloc_scatter(struct sga_heap *sga_heap,
			  struct sga_heap_buffer *buffer, unsigned long size)
{
	unsigned long per_alloc_sz = sga_heap->per_alloc_size;
	unsigned long nents = ALIGN(size, per_alloc_sz) / per_alloc_sz;
	unsigned long subregion_size = sga_heap->subregion_size;
	unsigned long size_remaining = ALIGN(size, subregion_size);
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	unsigned long alloc_size = 0;
	struct page *page = NULL;
	struct list_head tmp_head;
	struct alloc_list *pos, *tmp;
	u32 sg_num = 0;
	int ret = 0;

	INIT_LIST_HEAD(&tmp_head);

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table)
		return -ENOMEM;

	if (sg_alloc_table(table, (u32)nents, GFP_KERNEL)) {
		xrheap_err("scatterlist alloc failed\n");
		goto free_table;
	}

	/**
	 * In order to speed up the allocation, we will apply for memory
	 * in units of 2MB, and the memory portion of less than 2MB will
	 * be applied for one time.
	 */
	sg = table->sgl;

	while (size_remaining) {
		alloc_size = min(size_remaining, per_alloc_sz);

		page = sga_zalloc_large(sga_heap, alloc_size, subregion_size, &tmp_head);
		if (!page)
			goto free_pages;

		size_remaining -= alloc_size;
		sg_set_page(sg, page, (u32)alloc_size, 0);
		sg = sg_next(sg);
		sg_num++;
	}

	buffer->sg_table = table;
	if (sga_heap->heap_attr == HEAP_SECURE) {
		ret = sga_change_mem_prot(sga_heap, buffer, XRING_SEC_CMD_ALLOC);
		if (ret)
			goto free_pages;
	}

	mutex_lock(&sga_heap->mutex);
	list_splice(&tmp_head, &sga_heap->allocate_head);
	mutex_unlock(&sga_heap->mutex);

	return 0;

free_pages:
	nents = sg_num;
	for_each_sg(table->sgl, sg, nents, sg_num) {
		page = sg_page(sg);
		gen_pool_free(sga_heap->pool, PFN_PHYS(page_to_pfn(page)), sg->length);
	}
	sg_free_table(table);
	list_for_each_entry_safe(pos, tmp, &tmp_head, list) {
		list_del(&pos->list);
		kfree(pos);
	}
free_table:
	kfree(table);

	return -ENOMEM;
}

static struct dma_buf *xr_sga_heap_do_allocate(struct dma_heap *heap,
				   unsigned long len,
				   u32 fd_flags,
				   u64 heap_flags)
{
	struct sga_heap *sga_heap = dma_heap_get_drvdata(heap);
	size_t size = ALIGN(len, sga_heap->subregion_size);
	struct sga_heap_buffer *buffer = NULL;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf = NULL;
	int ret = 0;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->mutex);
	buffer->len = size;
	buffer->heap = sga_heap;

	mutex_lock(&sga_heap->mutex);
	if (sga_input_check(sga_heap, size)) {
		mutex_unlock(&sga_heap->mutex);
		xrheap_err("input params failed\n");
		ret = -EINVAL;
		goto free_buffer;
	}
	mutex_unlock(&sga_heap->mutex);

	xrheap_info("success: alloc size=0x%lx, size=0x%lx, len=0x%lx\n",
				sga_heap->alloc_size, size, len);

	ret = sga_zalloc_scatter(sga_heap, buffer, size);
	if (unlikely(ret))
		goto free_alloc;

	/* create the dmabuf */
	exp_info.exp_name = dma_heap_get_name(heap);
	exp_info.ops = &sga_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		xrheap_err("dma buffer export failed\n");
		ret = PTR_ERR(dmabuf);
		goto free_page;
	}

	return dmabuf;
free_page:
	sga_heap_free(buffer);

free_alloc:
	xrheap_err("failed\n");
free_buffer:
	kfree(buffer);

	return ERR_PTR(ret);
}

static struct dma_buf *xr_sga_heap_allocate(struct dma_heap *heap, unsigned long len,
					u32 fd_flags, u64 heap_flags)
{
	struct dma_buf *dmabuf;

	dmabuf = xr_sga_heap_do_allocate(heap, len, fd_flags, heap_flags);
	if (IS_ERR(dmabuf)) {
		xrheap_err("dmabuf error %ld\n", PTR_ERR(dmabuf));
		xr_meminfo_show(true, FROM_DMAHEAP);
	}

	return dmabuf;
}

static struct dma_heap_ops sga_heap_ops = {
	.allocate = xr_sga_heap_allocate,
};

static int sga_create_pool(struct sga_heap *sga_heap)
{
	int ret = 0;
	phys_addr_t cma_base = 0;
	u64 cma_size = 0;

	sga_heap->pool = gen_pool_create((int)sga_heap->genpool_shift, -1);
	if (!sga_heap->pool) {
		xrheap_err("gen pool create failed\n");
		return -ENOMEM;
	}
	gen_pool_set_algo(sga_heap->pool, gen_pool_best_fit, NULL);

	/* Add all memory to genpool first，one chunk only */
	cma_base = PFN_PHYS(sga_heap->cma->base_pfn);
	cma_size = (sga_heap->cma->count) << PAGE_SHIFT;

	if (gen_pool_add(sga_heap->pool, cma_base, cma_size, -1)) {
		xrheap_err("genpool add cma_size 0x%llx\n", cma_size);
		ret = -ENOMEM;
		goto err_add;
	}

	xrheap_debug("success -- genpool cma_size 0x%llx\n", cma_size);

	if (!gen_pool_alloc(sga_heap->pool, cma_size)) {
		xrheap_err("gen pool alloc failed\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	return ret;
err_alloc:
	gen_pool_destroy(sga_heap->pool);
err_add:
	sga_heap->pool = NULL;

	return ret;
}

static int sga_parse_dt(struct sga_heap *sga_heap,
			    struct xring_heap_platform_data *heap_data)
{
	struct device_node *nd = heap_data->node;
	u64 per_alloc_size = 0;
	u32 pre_alloc_attr = 0;
	u64 subregion_size = 0;
	u64 append_size = 0;
	u32 heap_size = 0;
	u32 heap_scene = 0;
	int ret;

	ret = of_property_read_u32(nd, "pre-alloc-attr", &pre_alloc_attr);
	if (ret < 0) {
		xrheap_err("can't find prop: pre-alloc-attr.\n");
		pre_alloc_attr = 0;
	}
	sga_heap->pre_alloc_attr = pre_alloc_attr;

	ret = of_property_read_u32(nd, "heap-scene", &heap_scene);
	if (ret < 0)
		xrheap_err("can't find prop: heap-scene\n");

	sga_heap->heap_scene = heap_scene;

	ret = of_property_read_u64(nd, "append-size", &append_size);
	if (ret < 0)
		xrheap_err("can't find prop: append-size\n");

	sga_heap->append_size = PAGE_ALIGN(append_size);

	ret = of_property_read_u64(nd, "per-alloc-size", &per_alloc_size);
	if (ret < 0)
		xrheap_err("can't find prop: per-alloc-size\n");

	sga_heap->per_alloc_size = PAGE_ALIGN(per_alloc_size);

	ret = of_property_read_u64(nd, "subregion-size", &subregion_size);
	if (ret < 0) {
		xrheap_err("can't find prop: subregion-size\n");
		goto out;
	}
	sga_heap->subregion_size = PAGE_ALIGN(subregion_size);

	ret = of_property_read_u32(nd, "heap-size", &heap_size);
	if (ret < 0) {
		xrheap_err("can't find prop: heap-size\n");
		goto out;
	}
	sga_heap->heap_size = heap_size;

out:
	return ret;
}

static u64 get_alloc_size_sga_heap(void *data)
{
	struct sga_heap *sga_heap = (struct sga_heap *)data;
	u64 size;

	size = sga_heap->alloc_size;

	return size;
}


static u64 get_cma_consumed_sga_heap(void *data)
{
	return get_alloc_size_sga_heap(data);
}

static u64 get_pool_cached_sga_heap(void *data)
{
	return 0;
}

int xr_sga_heap_create(struct xring_heap_platform_data *heap_data)
{
	struct dma_heap_export_info exp_info;
	struct sga_heap *sga_heap = NULL;
	struct xr_dmabuf_helper_ops *helper_ops;
	int ret;

	sga_heap = devm_kzalloc(heap_data->priv, sizeof(*sga_heap), GFP_KERNEL);
	if (!sga_heap)
		return -ENOMEM;

	mutex_init(&sga_heap->mutex);
	mutex_init(&sga_heap->pre_alloc_mutex);

	INIT_LIST_HEAD(&sga_heap->allocate_head);
	sga_heap->alloc_size = 0;
	sga_heap->dev = heap_data->priv;
	sga_heap->cma = heap_data->cma;
	sga_heap->heap_attr = heap_data->heap_attr;
	sga_heap->pool = NULL;

	if (sga_parse_dt(sga_heap, heap_data))
		return -EINVAL;

	helper_ops = devm_kzalloc(heap_data->priv, sizeof(*helper_ops), GFP_KERNEL);
	helper_ops->name = heap_data->heap_name;
	if (sga_heap->heap_attr & HEAP_TEE) {
		helper_ops->get_sfd = xr_sga_get_sfd;
		helper_ops->change_mem_prot = xr_perpare_change_mem_prot;
	}
	helper_ops->get_alloc_size = get_alloc_size_sga_heap;
	helper_ops->get_cma_consumed = get_cma_consumed_sga_heap;
	helper_ops->get_pool_cached = get_pool_cached_sga_heap;
	helper_ops->priv = sga_heap;
	helper_ops->scene = sga_heap->heap_scene;
	helper_ops->pre_alloc = xr_sga_pre_alloc;
	if (xr_dmabuf_helper_register(helper_ops)) {
		xrheap_err("dmabuf helper register failed\n");
		return -EINVAL;
	}

	ret = sga_create_pool(sga_heap);
	if (ret) {
		xrheap_err("pool create failed.\n");
		return -EINVAL;
	}

	if (sga_heap->pre_alloc_attr)
		INIT_WORK(&sga_heap->pre_alloc_work, sga_pre_alloc_wk_func);

	exp_info.name = heap_data->heap_name;
	exp_info.ops = &sga_heap_ops;
	exp_info.priv = sga_heap;
	sga_heap->heap = dma_heap_add(&exp_info);

	xrheap_info("%s info:\n"
		  "\t\t\t\theap attr: %u\n"
		  "\t\t\t\theap size: %lu MB\n"
		  "\t\t\t\tsubregion size: %llu MB\n"
		  "\t\t\t\tper alloc size: %llu MB\n"
		  "\t\t\t\tcma size: %lu MB\n",
		  heap_data->heap_name,
		  sga_heap->heap_attr,
		  sga_heap->heap_size / SZ_1M,
		  sga_heap->subregion_size / SZ_1M,
		  sga_heap->per_alloc_size / SZ_1M,
		  heap_data->cma_size / SZ_1M);

	return 0;
}
MODULE_LICENSE("GPL v2");
