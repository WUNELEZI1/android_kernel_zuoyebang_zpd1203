// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <linux/dma-buf.h>
#include <mali_kbase_xr_protected_memory_allocator.h>

#define ERR_PHYS_ADDR 0xFF
/**
 * xr_dma_buf_map_attachment - Allocate protect memory and Prepare attached dma-buf to get phys address for GPU mapping
 * @kbdev:      Kbase device where memory is used
 * @pma:        Pointer to protect memory to be allocated, NULL if fail.
 * @nr_pages:   Page numbers to be allcated
 * @pdb:        Pointer to protect dam buffer, must exist at the same time as pma.
 *
 * Allocate the protect dma-buf and prepare the page array with the physical
 * addresses for GPU mapping.
 *
 * Return: 0 on success, or negative error code
 */
static int xr_dma_buf_alloc_map_attachment(struct kbase_device *kbdev, struct protected_memory_allocation **pma,
				unsigned int nr_pages, struct protected_dma_buffer *pdb)
{
	struct scatterlist *s = NULL;
	int i = 0;
	size_t count = 0;
	struct dma_heap *system_heap = kbdev->csf.protected_heap;
	/* size in byte */
	size_t size = nr_pages << PAGE_SHIFT;

	if (IS_ERR_OR_NULL(system_heap)) {
		dev_err(kbdev->dev, "Protected heap:%pK not find, ret = %ld(%pe)", system_heap, PTR_ERR(system_heap), system_heap);
		return -EINVAL;
	}

	pdb->dma_buf = dma_heap_buffer_alloc(system_heap, size, O_RDWR, 0);
	if (IS_ERR(pdb->dma_buf)) {
		dev_err(kbdev->dev, "Protected dma buffer allocate fail, ret = %ld(%pe)", PTR_ERR(pdb->dma_buf), pdb->dma_buf);
		return -ENOMEM;
	}

	pdb->dma_attachment = dma_buf_attach(pdb->dma_buf, kbdev->dev);
	if (IS_ERR_OR_NULL(pdb->dma_attachment)) {
		dev_err(kbdev->dev, "dma_buf_attach fail, ret = %ld(%pe)", PTR_ERR(pdb->dma_attachment), pdb->dma_attachment);
		goto free;
	}

#if (KERNEL_VERSION(6, 1, 55) <= LINUX_VERSION_CODE)
	pdb->sgt = dma_buf_map_attachment_unlocked(pdb->dma_attachment, DMA_BIDIRECTIONAL);
#else
	pdb->sgt = dma_buf_map_attachment(pdb->dma_attachment, DMA_BIDIRECTIONAL);
#endif
	if (IS_ERR_OR_NULL(pdb->sgt)) {
		dev_err(kbdev->dev, "dma_buf_map_attachment fail, ret = %ld(%pe)", PTR_ERR(pdb->sgt), pdb->sgt);
		goto detach;
	}

	for_each_sg(pdb->sgt->sgl, s, pdb->sgt->nents, i) {
		size_t j, pages = PFN_UP(sg_dma_len(s));

		WARN_ONCE(sg_dma_len(s) & (PAGE_SIZE - 1),
			  "sg_dma_len(s)=%u is not a multiple of PAGE_SIZE\n", sg_dma_len(s));

		WARN_ONCE(sg_dma_address(s) & (PAGE_SIZE - 1),
			  "sg_dma_address(s)=%llx is not aligned to PAGE_SIZE\n",
			  (unsigned long long)sg_dma_address(s));

		for (j = 0; (j < pages) && (count < nr_pages); j++, count++) {
			if (!pma[count]) {
				dev_err(kbdev->dev, "Protected pma is null in idx = %zu, need size=%u", count, nr_pages);
				goto unmap;
			}
			pma[count]->pa = sg_dma_address(s) + (j << PAGE_SHIFT);
			pma[count]->order = 0;
		}

		/**
		 * Due to PMPU limitations, protect memory is aligned to 128K.
		 * This print may appear when size smaller than 128K.
		 */
		if (j < pages)
			dev_info(kbdev->dev, "sg list from dma_buf_map_attachment > dma_buf->size");
	}

	if (WARN_ONCE(count < nr_pages, "sg list from dma_buf_map_attachment < dma_buf->size"))
		goto unmap;

	return 0;

unmap:
#if (KERNEL_VERSION(6, 1, 55) <= LINUX_VERSION_CODE)
	dma_buf_unmap_attachment_unlocked(pdb->dma_attachment, pdb->sgt, DMA_BIDIRECTIONAL);
#else
	dma_buf_unmap_attachment(pdb->dma_attachment, pdb->sgt, DMA_BIDIRECTIONAL);
#endif
detach:
	dma_buf_detach(pdb->dma_buf, pdb->dma_attachment);
free:
	dma_heap_buffer_free(pdb->dma_buf);
	/**
	 * If xr_dma_buf_alloc_map_attachment returns error,
	 * kbase_csf_protected_memory_alloc will destroy pma** and pma* immediately.
	 */

	return -EINVAL;
}

int xr_pma_alloc_pages(struct kbase_device *kbdev, struct protected_memory_allocator_device *pma_dev,
			unsigned int num_pages, unsigned int order, struct protected_memory_allocation **pma,
			struct protected_dma_buffer *pdb)
{
	size_t num_pages_to_alloc = (size_t)num_pages << order;

	if (!kbdev || !pma || !pdb)
		return -EINVAL;

	return xr_dma_buf_alloc_map_attachment(kbdev, pma, num_pages_to_alloc, pdb);
}

static phys_addr_t xr_pma_get_phys_addr(struct kbase_device *kbdev, struct protected_memory_allocator_device *pma_dev,
					    struct protected_memory_allocation *pma)
{
	if (WARN_ONCE(!pma, "get phys address but pma is null"))
		return ERR_PHYS_ADDR;

	return pma->pa;
}

static void xr_pma_free_dma_buf(struct kbase_device *kbdev, struct protected_dma_buffer *pdb)
{
	if (!pdb) {
		dev_err(kbdev->dev, "no pdb when free protect dma buffer\n");
		return;
	}

#if (KERNEL_VERSION(6, 1, 55) <= LINUX_VERSION_CODE)
	dma_buf_unmap_attachment_unlocked(pdb->dma_attachment, pdb->sgt, DMA_BIDIRECTIONAL);
#else
	dma_buf_unmap_attachment(pdb->dma_attachment, pdb->sgt, DMA_BIDIRECTIONAL);
#endif
	dma_buf_detach(pdb->dma_buf, pdb->dma_attachment);
	dma_heap_buffer_free(pdb->dma_buf);
}

struct protected_memory_allocator_device *xr_pma_dev_create(void) {
	struct protected_memory_allocator_device *xr_pma_dev =
		kmalloc(sizeof(struct protected_memory_allocator_device), GFP_KERNEL);
	if (!xr_pma_dev)
		return NULL;

	xr_pma_dev->ops.pma_alloc_pages = xr_pma_alloc_pages;
	xr_pma_dev->ops.pma_get_phys_addr = xr_pma_get_phys_addr;
	xr_pma_dev->ops.pma_free_dma_buf = xr_pma_free_dma_buf;
	xr_pma_dev->owner = THIS_MODULE;
	xr_pma_dev->need_defer_alloc_protected_fw = false;
	atomic_set(&xr_pma_dev->alloc_protected_fw_done, 0);
	xr_pma_dev->alloc_protected_fw_retries = 0;
	return xr_pma_dev;
}

