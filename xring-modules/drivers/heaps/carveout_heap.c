// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Carveout heap exporter
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

#include "carveout_heap.h"
#include "xring_dma_iommu.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "soc/xring/xring_mem_adapter.h"

struct carveout_heap {
	struct dma_heap *heap;
	struct gen_pool *pool;
	phys_addr_t base;
	size_t size;
};

struct carveout_heap_buffer {
	struct xr_heap_buffer_header header;
	struct carveout_heap *carveout_heap;
	struct list_head attachments;
	struct mutex lock;
	struct sg_table *sgt;
	unsigned long len;
	int vmap_cnt;
	void *vaddr;
};

struct carveout_heap_buffer_attachment {
	struct device *dev;
	struct sg_table *sgt;
	struct list_head list;
	bool mapped;
};

static int carveout_heap_attach(struct dma_buf *dmabuf,
			struct dma_buf_attachment *attachment)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_buffer_attachment *a;
	int ret = 0;

	a = kzalloc(sizeof(struct carveout_heap_buffer_attachment), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	INIT_LIST_HEAD(&a->list);
	a->dev = attachment->dev;
	a->mapped = false;
	a->sgt = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!a->sgt) {
		xrheap_err("kmalloc for sgt fail\n");
		ret = -ENOMEM;
		goto free_attachment;
	}

	/* for carveout_heap_buffer, there is only one scatterlist ?? */
	ret = sg_alloc_table(a->sgt, 1, GFP_KERNEL);
	if (ret) {
		xrheap_err("sg_alloc_table fail\n");
		ret = -ENOMEM;
		goto free_sgt;
	}
	/* assign value to buffer->sgt->sgl */
	memcpy(a->sgt->sgl, buffer->sgt->sgl, sizeof(struct scatterlist));

	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	attachment->priv = a;

	return 0;

free_sgt:
	kfree(a->sgt);
free_attachment:
	kfree(a);

	return ret;
}

static void carveout_heap_detach(struct dma_buf *dmabuf,
			struct dma_buf_attachment *attachment)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_buffer_attachment *a = attachment->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);

	sg_free_table(a->sgt);
	kfree(a->sgt);
	kfree(a);
}

static struct sg_table *carveout_heap_map_dma_buf(
				struct dma_buf_attachment *attachment,
				enum dma_data_direction direction)
{
	struct carveout_heap_buffer_attachment *a = attachment->priv;
	int attrs = attachment->dma_map_attrs;
	int ret;

	attrs |= DMA_ATTR_SKIP_CPU_SYNC;

#ifdef XRING_DMA_MAP_LAZY
	ret = xring_dma_map_sgtable_lazy(attachment->dev, a->sgt, direction, 0,
					attachment->dmabuf);
	if (ret < 0) {
		xrheap_err("xring_dma_map_sgtable_lazy fail\n");
		return ERR_PTR(ret);
	}
#else
	ret = xring_heap_dma_map(attachment->dev, attachment->dmabuf, a->sgt, direction, attrs);
	if (ret) {
		xrheap_err("carveout heap map dma buf failed\n");
		return ERR_PTR(-ENOMEM);
	}
#endif

	a->mapped = true;
	return a->sgt;
}

static void carveout_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
				struct sg_table *sgt,
				enum dma_data_direction direction)
{
	struct carveout_heap_buffer_attachment *a = attachment->priv;
	int attrs = attachment->dma_map_attrs;

	attrs |= DMA_ATTR_SKIP_CPU_SYNC;
	a->mapped = false;

#ifdef XRING_DMA_MAP_LAZY
	xring_dma_unmap_sgtable_lazy(attachment->dev, sgt, direction, 0,
					attachment->dmabuf);
#else
	xring_heap_dma_unmap(attachment->dev, attachment->dmabuf, sgt, direction, attrs);
#endif
}

static int carveout_heap_begin_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction direction)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_buffer_attachment *a = NULL;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->len);

	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_cpu(a->dev, a->sgt, direction);
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int carveout_heap_end_cpu_access(struct dma_buf *dmabuf,
					enum dma_data_direction direction)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_buffer_attachment *a = NULL;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->len);

	list_for_each_entry(a, &buffer->attachments, list) {
		if (!a->mapped)
			continue;
		dma_sync_sgtable_for_device(a->dev, a->sgt, direction);
	}
	mutex_unlock(&buffer->lock);


	return 0;
}

static int carveout_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct sg_table *sgt = buffer->sgt;
	struct scatterlist *sgl = NULL;
	unsigned long addr = vma->vm_start;
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	int i = 0, ret = 0;

	for_each_sg(sgt->sgl, sgl, sgt->nents, i) {
		struct page *page = sg_page(sgl);
		unsigned long remainder = vma->vm_end - addr;
		unsigned long len = sgl->length;

		/* FIXME: is there any problem with offset case */
		if (offset >= sgl->length) {
			offset -= sgl->length;
			continue;
		} else if (offset) {
			page += offset / PAGE_SIZE;
			len = sgl->length - offset;
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
			return 0;
	}

	return 0;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int carveout_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static int carveout_heap_vmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct sg_table *sgt = buffer->sgt;
	struct scatterlist *sgl = NULL;
	int npages = PAGE_ALIGN(buffer->len) / PAGE_SIZE;
	struct page **pages = NULL, **tmp = NULL;
	void *vaddr = NULL;
	int i = 0, j = 0, ret = 0;

	pages = vmalloc(array_size(npages, sizeof(struct page *)));
	if (!pages)
		return -ENOMEM;

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

	tmp = pages;
	for_each_sg(sgt->sgl, sgl, sgt->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sgl->length) / PAGE_SIZE;
		struct page *page = sg_page(sgl);

		if (unlikely(i >= npages)) {
			xrheap_err("sgt entry nums is more than npages\n");
			ret = -EINVAL;
			goto out;
		}

		for (j = 0; j < npages_this_entry; j++)
			*(tmp++) = page++;

	}

	vaddr = vmap(pages, npages, VM_MAP, PAGE_KERNEL);
	if (!vaddr) {
		xrheap_err("vmap fail\n");
		ret = -EINVAL;
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
	vfree(pages);

	return ret;
}

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static void carveout_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
#else
static void carveout_heap_vunmap(struct dma_buf *dmabuf, struct iosys_map *map)
#endif
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	dma_buf_map_set_vaddr(map, buffer->vaddr);
#else
	iosys_map_clear(map);
#endif
}

static void carveout_heap_release(struct dma_buf *dmabuf)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap *carveout_heap = buffer->carveout_heap;
	phys_addr_t paddr = sg_phys(buffer->sgt->sgl);
	int len = buffer->len;

	if (buffer->vmap_cnt > 0) {
		WARN(1, "buffer still mapped in the kernel\n");
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}

	gen_pool_free(carveout_heap->pool, paddr, len);
	sg_free_table(buffer->sgt);
	kfree(buffer->sgt);
	kfree(buffer);
}

static const struct dma_buf_ops carveout_heap_buf_ops = {
	.attach = carveout_heap_attach,
	.detach = carveout_heap_detach,
	.map_dma_buf = carveout_heap_map_dma_buf,
	.unmap_dma_buf = carveout_heap_unmap_dma_buf,
	.begin_cpu_access = carveout_heap_begin_cpu_access,
	.end_cpu_access = carveout_heap_end_cpu_access,
	.mmap = carveout_heap_mmap,
	.vmap = carveout_heap_vmap,
	.vunmap = carveout_heap_vunmap,
	.release = carveout_heap_release,
};

static struct dma_buf *carveout_heap_do_allocate(struct dma_heap *heap,
					unsigned long len,
					u32 fd_flags,
					u64 heap_flags)
{
	struct carveout_heap *carveout_heap = dma_heap_get_drvdata(heap);
	struct carveout_heap_buffer *buffer;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf = NULL;
	struct sg_table *sgt = NULL;
	phys_addr_t paddr;
	int ret = 0;

	buffer = kzalloc(sizeof(struct carveout_heap_buffer), GFP_KERNEL);
	if (!buffer)
		return NULL;

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);

	sgt = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!sgt)
		goto free_buffer;

	/* for carveout_heap_buffer, there is only one scatterlist */
	ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
	if (ret)
		goto free_sgt;

	paddr = gen_pool_alloc(carveout_heap->pool, len);
	if (!paddr) {
		xrheap_err("carveout_allocate fail\n");
		goto free_sg;
	}
	sg_set_page(sgt->sgl, pfn_to_page(PFN_DOWN(paddr)), len, 0);
	buffer->sgt = sgt;
	buffer->len = len;
	buffer->carveout_heap = carveout_heap;

	/* create the dmabuf */
	exp_info.exp_name = dma_heap_get_name(heap);
	exp_info.ops = &carveout_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		xrheap_err("dma_buf_export fail\n");
		goto free_pool;
	}

	return dmabuf;

free_pool:
	gen_pool_free(carveout_heap->pool, paddr, len);
free_sg:
	sg_free_table(sgt);
free_sgt:
	kfree(sgt);
free_buffer:
	kfree(buffer);

	return NULL;
}

static struct dma_buf *carveout_heap_allocate(struct dma_heap *heap,
					unsigned long len, u32 fd_flags, u64 heap_flags)
{
	struct dma_buf *dmabuf;

	dmabuf = carveout_heap_do_allocate(heap, len, fd_flags, heap_flags);
	if (IS_ERR(dmabuf)) {
		xrheap_err("dmabuf error %ld\n", PTR_ERR(dmabuf));
		xr_meminfo_show(true, FROM_DMAHEAP);
	}

	return dmabuf;
}

static const struct dma_heap_ops carveout_heap_ops = {
	.allocate = carveout_heap_allocate,
};

static u64 get_alloc_size_carveout_heap(void *data)
{
	struct carveout_heap *carveout_heap = (struct carveout_heap *)data;
	size_t avail = gen_pool_avail(carveout_heap->pool);
	size_t size = gen_pool_size(carveout_heap->pool);

	return size - avail;
}

static u64 get_cma_consumed_carveout_heap(void *data)
{
	struct carveout_heap *carveout_heap = (struct carveout_heap *)data;

	return carveout_heap->size;
}

static u64 get_pool_cached_carveout_heap(void *data)
{
	struct carveout_heap *carveout_heap = (struct carveout_heap *)data;
	size_t avail = gen_pool_avail(carveout_heap->pool);
	return avail;
}

struct xr_dmabuf_helper_ops carveout_helper_ops = {
	.get_alloc_size = get_alloc_size_carveout_heap,
	.get_cma_consumed = get_cma_consumed_carveout_heap,
	.get_pool_cached = get_pool_cached_carveout_heap,
};

int xr_carveout_heap_create(struct xring_heap_platform_data *data)
{
	struct carveout_heap *carveout_heap = NULL;
	struct dma_heap_export_info exp_info;
	int ret = 0;

	carveout_heap = kzalloc(sizeof(struct carveout_heap), GFP_KERNEL);
	if (!carveout_heap)
		return -ENOMEM;

	carveout_heap->pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!carveout_heap->pool) {
		ret = -ENOMEM;
		goto free_carveout_heap;
	}
	carveout_heap->base = data->cma_base;
	carveout_heap->size = data->cma_size;

	ret = gen_pool_add(carveout_heap->pool, carveout_heap->base,
						carveout_heap->size, -1);
	if (ret < 0) {
		ret = -ENOMEM;
		goto destroy_pool;
	}
	exp_info.name = data->heap_name;
	exp_info.ops = &carveout_heap_ops;
	exp_info.priv = carveout_heap;

	carveout_helper_ops.name = exp_info.name;
	carveout_helper_ops.priv = carveout_heap;
	if (xr_dmabuf_helper_register(&carveout_helper_ops)) {
		xrheap_err("xr_dmabuf_helper_register failed\n");
		ret = -EINVAL;
		goto destroy_pool;
	}

	carveout_heap->heap = dma_heap_add(&exp_info);
	if (IS_ERR(carveout_heap->heap)) {
		xrheap_err("dma_heap_add fail\n");
		ret = -EINVAL;
		goto destroy_pool;
	}

	dma_coerce_mask_and_coherent(dma_heap_get_dev(carveout_heap->heap),
				DMA_BIT_MASK(64));

	return 0;

destroy_pool:
	gen_pool_destroy(carveout_heap->pool);
free_carveout_heap:
	kfree(carveout_heap);

	return ret;
}
EXPORT_SYMBOL(xr_carveout_heap_create);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring Carveout Heap");
MODULE_LICENSE("GPL v2");

