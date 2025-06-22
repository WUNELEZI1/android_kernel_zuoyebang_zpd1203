// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <linux/of.h>

#include "dksm_mem_pool.h"
#include "dpu_log.h"

/* Allocated buffer node is aligned in 4K by default */
#define MIN_ALLOC_ORDER 12

int dksm_mem_pool_init(struct device *dev, struct dksm_mem_pool **out_mem_pool)
{
	struct dksm_mem_pool *mem_pool;
	int ret;

	if (!dev || !out_mem_pool) {
		DPU_ERROR("invalid params:%pK %pK\n", dev, out_mem_pool);
		return -1;
	}

	mem_pool = kzalloc(sizeof(struct dksm_mem_pool), GFP_KERNEL);
	if (!mem_pool) {
		DPU_ERROR("alloc mem pool failed\n");
		return -1;
	}
	ret = of_property_read_u32(dev->of_node, "mem-pool-size", &mem_pool->pool_size);
	if (ret) {
		DPU_ERROR("failed to parse mem pool size\n");
		goto error;
	}

	ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(64));
	if (ret) {
		DPU_ERROR("dma set mask and coherent failed %d!\n", ret);
		goto error;
	}

	mem_pool->pool_vir_addr = dma_alloc_coherent(dev, mem_pool->pool_size,
			&mem_pool->pool_phy_addr, GFP_KERNEL);
	if (!mem_pool->pool_vir_addr) {
		DPU_ERROR("mem pool alloc dma buffer failed\n");
		goto error;
	}

	mem_pool->pool = devm_gen_pool_create(dev, MIN_ALLOC_ORDER, -1, "dpu-mem-pool");
	if (!mem_pool->pool) {
		DPU_ERROR("create mem pool failed\n");
		goto mem_pool_err;
	}

	ret = gen_pool_add_virt(mem_pool->pool, (u64)mem_pool->pool_vir_addr,
			mem_pool->pool_phy_addr, mem_pool->pool_size, -1);
	if (ret) {
		DPU_ERROR("mem pool add failed\n");
		goto mem_pool_err;
	}

	DPU_INFO("create mem phy: 0x%llx, size:%#x\n",
			mem_pool->pool_phy_addr, mem_pool->pool_size);

	*out_mem_pool = mem_pool;
	return 0;

mem_pool_err:
	dma_free_coherent(dev, mem_pool->pool_size, mem_pool->pool_vir_addr,
			mem_pool->pool_phy_addr);
error:
	kfree(mem_pool);
	*out_mem_pool = NULL;
	return -1;
}

void *dksm_mem_pool_alloc(struct dksm_mem_pool *mem_pool, size_t size, dma_addr_t *dma)
{
	void *vir_addr;

	if (!mem_pool) {
		DPU_ERROR("mem_pool is null\n");
		return NULL;
	}

	vir_addr = gen_pool_dma_alloc(mem_pool->pool, size, dma);
	if (!vir_addr) {
		DPU_ERROR("pool alloc failed, size:%#lx\n", size);
		return NULL;
	}
	memset(vir_addr, 0, size);

	return vir_addr;
}

void dksm_mem_pool_free(struct dksm_mem_pool *mem_pool, void *vir_addr, size_t size)
{
	if (!mem_pool || !vir_addr) {
		DPU_ERROR("mem_pool or vir_addr is null\n");
		return;
	}

	gen_pool_free(mem_pool->pool, (u64)vir_addr, size);
}

void dksm_mem_pool_deinit(struct device *dev, struct dksm_mem_pool *mem_pool)
{
	if (!dev || !mem_pool) {
		DPU_ERROR("invalid params:%pK %pK\n", dev, mem_pool);
		return;
	}
	DPU_INFO("free mem pool:%p: vir: %p, phy: 0x%llx, size:%#x!\n",
			mem_pool->pool, mem_pool->pool_vir_addr,
			mem_pool->pool_phy_addr, mem_pool->pool_size);
	dma_free_coherent(dev, mem_pool->pool_size, mem_pool->pool_vir_addr,
			mem_pool->pool_phy_addr);
	kfree(mem_pool);
}
