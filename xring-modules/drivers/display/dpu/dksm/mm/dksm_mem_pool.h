/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _DPU_MEM_POOL_H_
#define _DPU_MEM_POOL_H_

#include <linux/dma-mapping.h>
#include <linux/genalloc.h>

/**
 * dksm_mem_pool - universal memory pool mgr for dpu drv
 *
 * @pool_size: size(byte num) of dpu mem pool
 * @pool_vir_addr: virtual addr of mem pool
 * @pool_phy_addr: physical addr for mem pool
 * @pool: point of gen_pool
 */
struct dksm_mem_pool {
	u32 pool_size;
	void *pool_vir_addr;
	dma_addr_t pool_phy_addr;
	struct gen_pool *pool;
};

/**
 * dksm_mem_pool_init - create dpu mem pool
 *
 * @dev: device
 * @out_mem_pool: pool to create
 *
 * Return: 0 for succ, -1 for failed
 */
int dksm_mem_pool_init(struct device *dev, struct dksm_mem_pool **out_mem_pool);

/**
 * dksm_mem_pool_alloc - Allocate the requested number of bytes from the specified pool
 * @mem_pool: pool to allocate from
 * @size: number of bytes to allocate from the pool
 * @dma: dma-view physical address return value.  Use %NULL if unneeded.
 *
 * Return: virtual address of the allocated memory, or %NULL on failure
 */
void *dksm_mem_pool_alloc(struct dksm_mem_pool *mem_pool, size_t size, dma_addr_t *dma);

/**
 * dksm_mem_pool_free - free allocated special memory back to the pool
 *
 * @mem_pool: pool to free to
 * @vir_addr: starting vir_addr of memory to free back to pool
 * @size: size in bytes of memory to free
 */
void dksm_mem_pool_free(struct dksm_mem_pool *mem_pool, void *vir_addr, size_t size);

/**
 * dksm_mem_pool_deinit - release dpu mem pool
 *
 * @dev: point of device, use to alloc dma buf
 * @mem_pool: pool to release
 */
void dksm_mem_pool_deinit(struct device *dev, struct dksm_mem_pool *mem_pool);

#endif /* _DPU_MEM_POOL_H_ */
