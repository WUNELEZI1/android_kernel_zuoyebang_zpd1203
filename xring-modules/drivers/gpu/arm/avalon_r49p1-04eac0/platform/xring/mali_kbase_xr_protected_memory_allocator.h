// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _PROTECTED_MEMORY_ALLOCATOR_H_
#define _PROTECTED_MEMORY_ALLOCATOR_H_

#include <linux/atomic.h>
#include <linux/mm.h>
#include <linux/dma-heap.h>
#include <mali_kbase.h>
#include <mali_kbase_defs.h>

/**
 * struct protected_dma_buffer - Protected dma buffer allocation.
 * Description of the corresponding dma buf. Record this information in order to free memory.
 */
struct protected_dma_buffer {
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *dma_attachment;
	struct sg_table *sgt;
};

/**
 * struct protected_memory_allocation - Protected memory allocation
 *
 * @pa:    Physical address of the protected memory allocation.
 * @order: Size of memory allocation in pages, as a base-2 logarithm.

 * Each protected_memory_allocation only is a page-sized. It is one page in @dma_buf.
 */
struct protected_memory_allocation {
	phys_addr_t pa;
	unsigned int order;
};

struct protected_memory_allocator_device;

/**
 * struct protected_memory_allocator_ops - Callbacks for protected memory
 *                                         allocator operations
 *
 * @pma_alloc_page:    Callback to allocate protected memory
 * @pma_get_phys_addr: Callback to get the physical address of an allocation
 * @pma_free_dma_buf:  Callback to free protected dam buf
 * @pma_free_pages:     Callback to free protected memory
 */
struct protected_memory_allocator_ops {
	/*
	 * pma_alloc_page - Allocate protected memory pages
	 *
	 * @kbdev:     Kbase device where memory is used.
	 * @pma_dev:   The protected memory allocator the request is being made
	 *             through.
	 * @num_pages: How many pages to allocate about order.
	 * @order:     Whether it is a large page, as a base-2 logarithm.
	 * @pma:       Pointer to protected memory allocated in this function with num_pages, or NULL if allocation failed.
	 * @pdb:       Pointer to protected dma buffer allocated in this function with num_pages, or NULL if allocation failed.

	 * Return:     Whether the allocation is successful. Return 0 if allocation is successful, non-0 if failure..
	 */
	int (*pma_alloc_pages)(struct kbase_device *kbdev,
		struct protected_memory_allocator_device *pma_dev, unsigned int num_pages, unsigned int order,
		struct protected_memory_allocation **pma, struct protected_dma_buffer *pdb);

	/*
	 * pma_get_phys_addr - Get the physical address of the protected memory
	 *                     allocation
	 *
	 * @kbdev:   Kbase device where memory is used.
	 * @pma_dev: The protected memory allocator the request is being made
	 *           through.
	 * @pma:     The protected memory allocation whose physical address
	 *           shall be retrieved
	 *
	 * Return: The physical address of the given allocation.
	 */
	phys_addr_t (*pma_get_phys_addr)(struct kbase_device *kbdev, struct protected_memory_allocator_device *pma_dev,
					 struct protected_memory_allocation *pma);

	/*
	 * pma_free_dma_buf - Free protected dam buffer
	 *
	 * @kbdev:     Kbase device where memory is used.
	 * @pdb:       The protected dam buffer to free.
	 */
	void (*pma_free_dma_buf)(struct kbase_device *kbdev, struct protected_dma_buffer *pdb);
};

/**
 * struct protected_memory_allocator_device - Device structure for protected
 *                                            memory allocator
 *
 * @ops:        Callbacks associated with this device
 * @owner:      Pointer to the module owner
 * @need_defer_alloc_protected_fw:
 *              Whether to delay allocating protect memory used by the firmware
 * @alloc_protected_fw_done:
 *              Whether firmware protect memory allcaoted completely
 * @alloc_protected_fw_retries:
 *              retry to allocate for fw when ENOMEM
 * This structure should be create using
 * xr_pma_dev_create().
 */
struct protected_memory_allocator_device {
	struct protected_memory_allocator_ops ops;
	struct module *owner;
	bool need_defer_alloc_protected_fw;
	atomic_t alloc_protected_fw_done;
	u8 alloc_protected_fw_retries;
};
struct protected_memory_allocator_device *xr_pma_dev_create(void);

#endif /* _PROTECTED_MEMORY_ALLOCATOR_H_ */
