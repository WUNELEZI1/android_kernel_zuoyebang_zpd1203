// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 *
 * (C) COPYRIGHT 2019-2023 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 */

#include "mali_kbase_xr_csf_protected_memory.h"
#include "mali_kbase_xr_protected_memory_allocator.h"

#define PROTECTED_MEMORY_HEAP_NAME "xring_cpa"

int kbase_csf_protected_memory_init(struct kbase_device *const kbdev)
{
	int err = 0;

	kbdev->csf.pma_dev = xr_pma_dev_create();
	if (!kbdev->csf.pma_dev) {
		dev_err(kbdev->dev, "Protected memory allocator device is not created\n");
		return -EPROBE_DEFER;
	}

	kbdev->csf.protected_heap = dma_heap_find(PROTECTED_MEMORY_HEAP_NAME);
	if (IS_ERR_OR_NULL(kbdev->csf.protected_heap))
	{
		/* Open protected heap fail, print the error. And unprotect firmware running is not affected. */
		dev_err(kbdev->dev, "Protected heap not find");
	}

	return err;
}

void kbase_csf_protected_memory_term(struct kbase_device *const kbdev)
{
	if (kbdev->csf.pma_dev)
		kfree(kbdev->csf.pma_dev);

	if (IS_ERR_OR_NULL(kbdev->csf.protected_heap))
		return;

	dma_heap_put(kbdev->csf.protected_heap);
}

struct protected_memory_allocation **
kbase_csf_protected_memory_alloc(struct kbase_device *const kbdev, struct tagged_addr *phys,
				 size_t num_pages, bool is_small_page, struct protected_dma_buffer **pdb)
{
	size_t i = 0;
	struct protected_memory_allocator_device *pma_dev = kbdev->csf.pma_dev;
	struct protected_memory_allocation **pma = NULL;
	unsigned int order = KBASE_MEM_POOL_2MB_PAGE_TABLE_ORDER;
	unsigned int num_pages_order = 0;
	struct protected_memory_allocation *pma_one = NULL;
	struct protected_dma_buffer *pdb_new = NULL;
	if (!pma_dev || !phys)
	{
		dev_err(kbdev->dev, "xr protect memory fail because no pma_dev and phys");
		return NULL;
	}

	if (is_small_page)
		order = KBASE_MEM_POOL_SMALL_PAGE_TABLE_ORDER;
	else
		dev_err(kbdev->dev, "xr protect memory only support small page");

	num_pages_order = (1u << order);

	/* Ensure the requested num_pages is aligned with
	 * the order type passed as argument.
	 *
	 * pma_alloc_page() will then handle the granularity
	 * of the allocation based on order.
	 */
	num_pages = div64_u64(num_pages + num_pages_order - 1, num_pages_order);

	pma = kmalloc_array(num_pages, sizeof(struct protected_memory_allocation *), GFP_KERNEL);
	if (!pma)
	{
		dev_err(kbdev->dev, "pma malloc fail");
		return NULL;
	}

	pma_one = kmalloc_array(num_pages, sizeof(struct protected_memory_allocation), GFP_KERNEL);
	if (!pma_one)
	{
		dev_err(kbdev->dev, "pma_one malloc fail");
		goto free_pma;
	}

	for (i = 0; i < num_pages; i++) {
		pma[i] = pma_one + i;
	}

	pdb_new = devm_kzalloc(kbdev->dev, sizeof(*pdb_new), GFP_KERNEL);
	if (!pdb_new)
	{
		dev_err(kbdev->dev, "pdb_new malloc fail");
		goto free_pma_one;
	}

	/* 0 means succ */
	if (pma_dev->ops.pma_alloc_pages(kbdev, pma_dev, num_pages, order, pma, pdb_new)) {
		dev_err(kbdev->dev, "alloc protect dma bufer fail:(pma_dev=%px, num_pages=%zu)\n", (void *)pma_dev, num_pages);
		/* Only free pma. dma-buf exceptions have been handled in the pma_alloc_pages.
		 * There is no need to kbase_csf_protected_memory_free anymore.
		 */
		goto free_pdb;
	}

	*pdb = pdb_new;
	for (i = 0; i < num_pages; i++)
	{
		phys_addr_t phys_addr = pma_dev->ops.pma_get_phys_addr(kbdev, pma_dev, pma[i]);
		if (order) {
			size_t j;
			*phys++ = as_tagged_tag(phys_addr, HUGE_HEAD | HUGE_PAGE);
			for (j = 1; j < num_pages_order; j++) {
				*phys++ = as_tagged_tag(phys_addr + PAGE_SIZE * j, HUGE_PAGE);
			}
		} else {
			/* order == 0 means using small page */
			phys[i] = as_tagged(phys_addr);
		}
	}

	return pma;

free_pdb:
	devm_kfree(kbdev->dev, pdb_new);
free_pma_one:
	kfree(pma_one);
free_pma:
	kfree(pma);
	*pdb = NULL;
	return NULL;
}

void kbase_csf_protected_memory_free(struct kbase_device *const kbdev,
				     struct protected_memory_allocation **pma, size_t num_pages,
				     bool is_small_page, struct protected_dma_buffer *pdb)
{
	struct protected_memory_allocator_device *pma_dev = kbdev->csf.pma_dev;
	unsigned int num_pages_order = (1u << KBASE_MEM_POOL_2MB_PAGE_TABLE_ORDER);

	if (WARN_ON(!pma_dev))
		return;

	if (!pma || !pdb) {
		if (atomic_read(&pma_dev->alloc_protected_fw_done)) {
			dev_err(kbdev->dev, "no pma or pdb when free protect memory\n");
		} else {
			dev_info(kbdev->dev, "needn't free pma and pdb, because protect fw hasn't allocated\n");
		}

		return;
	}

	if (is_small_page)
		num_pages_order = (1u << KBASE_MEM_POOL_SMALL_PAGE_TABLE_ORDER);

	/* Ensure the requested num_pages is aligned with
	 * the order type passed as argument.
	 *
	 * pma_alloc_page() will then handle the granularity
	 * of the allocation based on order.
	 */
	num_pages = div64_u64(num_pages + num_pages_order - 1, num_pages_order);

	pma_dev->ops.pma_free_dma_buf(kbdev, pdb);

	devm_kfree(kbdev->dev, pdb);
	kfree(pma[0]);
	kfree(pma);
}
