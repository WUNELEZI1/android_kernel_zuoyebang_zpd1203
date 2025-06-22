// SPDX-License-Identifier: GPL-2.0-or-later
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/of_platform.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/genalloc.h>
#include <linux/dma-buf.h>
#include "xrp_hw.h"
#include <linux/iommu.h>
#include "soc/xring/xring_smmu_wrapper.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include <dt-bindings/xring/platform-specific/dfx_memory_layout.h>
#include <linux/dma-mapping.h>
#include <linux/mailbox_client.h>
#include "xrp_internal.h"
#include "xrp_kernel_defs.h"
#include <soc/xring/dfx_switch.h>

int rqs_sysncached_heap(struct xrp_task_device *xrp_task_dev)
{
	int ret = 0;
	struct dma_heap *sys_uncache_heap = NULL; //system_heap<-->system-uncached

	sys_uncache_heap = dma_heap_find("system-uncached");
	if (IS_ERR_OR_NULL(sys_uncache_heap)) {
		ret = PTR_ERR(sys_uncache_heap);
		xrp_print(LOG_LEVEL_ERROR, "system head not find, ret = %d(%pe)",
					ret, sys_uncache_heap);
		return -EINVAL;
	}
	xrp_task_dev->vdsp_system_uncached_heap = sys_uncache_heap;
	return ret;
}

void rls_sysncached_heap(struct xrp_task_device *xrp_task_dev)
{
	if (xrp_task_dev->vdsp_system_uncached_heap)
		dma_heap_put(xrp_task_dev->vdsp_system_uncached_heap);
}

static void sys_nocache_dma_buf_add(
	struct sys_nocache_dma_buf *xrp_sys_dma_buf,
	struct xrp_task_device *xrp_task_dev)
{
	mutex_lock(&xrp_task_dev->dma_buf_lock);
	list_add_tail(&xrp_sys_dma_buf->list, &xrp_task_dev->uncache_heap_list);
	mutex_unlock(&xrp_task_dev->dma_buf_lock);
}

static void sys_nocache_dma_buf_del(
	struct sys_nocache_dma_buf *xrp_sys_dma_buf,
	struct xrp_task_device *xrp_task_dev)
{
	mutex_lock(&xrp_task_dev->dma_buf_lock);
	list_del(&xrp_sys_dma_buf->list);
	mutex_unlock(&xrp_task_dev->dma_buf_lock);
}

static struct sys_nocache_dma_buf *sys_nocache_dma_buf_find(
	enum heap_flags_t data_heap_flag,
	struct xrp_task_device *xrp_task_dev)
{
	struct sys_nocache_dma_buf *xrp_sys_dma_buf = NULL;

	mutex_lock(&xrp_task_dev->dma_buf_lock);
	list_for_each_entry(xrp_sys_dma_buf, &xrp_task_dev->uncache_heap_list, list) {
		if (xrp_sys_dma_buf->heap_flag == data_heap_flag) {
			mutex_unlock(&xrp_task_dev->dma_buf_lock);
			return xrp_sys_dma_buf;
		}
	}
	mutex_unlock(&xrp_task_dev->dma_buf_lock);
	xrp_print(LOG_LEVEL_INFO, "%s done, dma_buf = %p\n", __func__, xrp_sys_dma_buf);
	return NULL;
}

void print_dma_buf_info(struct sys_nocache_dma_buf *dma_buf_info)
{
	if (dma_buf_info == NULL) {
		xrp_print(LOG_LEVEL_ERROR, "Error: dma_buf_info is NULL.\n");
		return;
	}

	// Print the heap flag
	xrp_print(LOG_LEVEL_DEBUG, "Heap Flag: ");
	switch (dma_buf_info->heap_flag) {
	case SRAM_HEAP:
		xrp_print(LOG_LEVEL_DEBUG, "SRAM_HEAP\n");
		break;
	case SROM_HEAP:
		xrp_print(LOG_LEVEL_DEBUG, "SROM_HEAP\n");
		break;
	case COMMON_HEAP:
		xrp_print(LOG_LEVEL_DEBUG, "COMMON_HEAP\n");
		break;
	default:
		xrp_print(LOG_LEVEL_ERROR, "Unknown heap flag\n");
	}

	// Print the buffer if it exists
	if (dma_buf_info->buf != NULL)
		xrp_print(LOG_LEVEL_DEBUG, "dma_buf_info->buf: %p\n", dma_buf_info->buf);
	else
		xrp_print(LOG_LEVEL_ERROR, "dma_buf_info->buf: NULL\n");

	xrp_print(LOG_LEVEL_DEBUG, "dma_buf_info->sgt->sgl->dma_address = %016llx", dma_buf_info->sgt->sgl->dma_address);
	xrp_print(LOG_LEVEL_DEBUG, "dma_buf_info->map->vaddr = %p", dma_buf_info->map.vaddr);
	xrp_print(LOG_LEVEL_DEBUG, "dma_buf_info->act_size = %08x", dma_buf_info->act_size);
}

struct sys_nocache_dma_buf *sys_nocache_map(
										struct xrp_task_device *xrp_task_dev,
										struct sys_nocache_heap_data *data)
{
	struct sys_nocache_dma_buf *xrp_sys_dma_buf = NULL;
	struct scatterlist *sgl = NULL;
	int i = 0;
	int ret = 0;
	int size = 0;
	struct dma_heap *sys_uncache_heap = NULL;
	struct dma_buf *nocache_dma_buf = NULL;
	unsigned long iova = data->addr;

	sys_uncache_heap = xrp_task_dev->vdsp_system_uncached_heap;

	xrp_sys_dma_buf = kzalloc(sizeof(struct sys_nocache_dma_buf), GFP_KERNEL);
	if (!xrp_sys_dma_buf) {
		xrp_print(LOG_LEVEL_INFO, "heap_test: alloc mem for xrp_sys_dma_buf fail\n");
		return NULL;
	}

	xrp_print(LOG_LEVEL_DEBUG, "before page size align, data size = %d\n", data->size);
	size = (data->size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	xrp_print(LOG_LEVEL_DEBUG, "after page size align, size = %d\n", size);

	nocache_dma_buf = dma_heap_buffer_alloc(sys_uncache_heap, size, O_RDWR, 0);
	xrp_sys_dma_buf->dma_buf = nocache_dma_buf;
	if (IS_ERR(xrp_sys_dma_buf->dma_buf)) {
		xrp_print(LOG_LEVEL_ERROR, "%s: dma_heap_buffer_alloc fail\n", __func__);
		goto fail_free;
	}

	get_dma_buf(xrp_sys_dma_buf->dma_buf);
	xr_dmabuf_kernel_account(xrp_sys_dma_buf->dma_buf, XR_DMABUF_KERNEL_NPU);

	xrp_sys_dma_buf->attachment = dma_buf_attach(xrp_sys_dma_buf->dma_buf, xrp_task_dev->dev);
	if (IS_ERR(xrp_sys_dma_buf->attachment)) {
		xrp_print(LOG_LEVEL_ERROR, "%s: dma_buf_attach fail\n", __func__);
		goto fail_put;
	}

	xrp_sys_dma_buf->sgt = dma_buf_map_attachment(xrp_sys_dma_buf->attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR(xrp_sys_dma_buf->sgt)) {
		xrp_print(LOG_LEVEL_ERROR, "%s: dma_buf_map_attachment fail\n", __func__);
		goto fail_detach;
	}

	/* map to kernel and access the mem */
	ret = dma_buf_vmap(xrp_sys_dma_buf->dma_buf, &xrp_sys_dma_buf->map);
	if (ret != 0) {
		xrp_print(LOG_LEVEL_ERROR, "%s: dma_buf_vmap fail\n", __func__);
		goto fail_unmap;
	}

	xrp_sys_dma_buf->buf = (char *)xrp_sys_dma_buf->map.vaddr;
	xrp_sys_dma_buf->len = 0;
	for_each_sg(xrp_sys_dma_buf->sgt->sgl, sgl, xrp_sys_dma_buf->sgt->nents, i) {
		xrp_sys_dma_buf->len += sg_dma_len(sgl);
	}
	memset(xrp_sys_dma_buf->buf, 0, xrp_sys_dma_buf->len);
	xrp_sys_dma_buf->len = xrp_sys_dma_buf->dma_buf->size;

	sys_nocache_dma_buf_add(xrp_sys_dma_buf, xrp_task_dev);
	xrp_sys_dma_buf->heap_flag = data->data_heap_flag;
	xrp_sys_dma_buf->act_size = size;

	print_dma_buf_info(xrp_sys_dma_buf);

	size = iommu_map_sgtable(xrp_task_dev->domain, iova, xrp_sys_dma_buf->sgt,
								IOMMU_READ | IOMMU_WRITE);
	if (size > xrp_sys_dma_buf->dma_buf->size) {
		xrp_print(LOG_LEVEL_ERROR, "iommu map failed, map size 0x%x, sgt size 0x%lx\n",
							size, xrp_sys_dma_buf->dma_buf->size);
		goto fail_iomap;
	}

	xrp_print(LOG_LEVEL_DEBUG, "%s: iova = %lx, after iommu map size = %d\n",
				__func__, iova, size);

	return xrp_sys_dma_buf;

fail_iomap:
	sys_nocache_dma_buf_del(xrp_sys_dma_buf, xrp_task_dev);
	dma_buf_vunmap(xrp_sys_dma_buf->dma_buf, &xrp_sys_dma_buf->map);
fail_unmap:
	dma_buf_unmap_attachment(xrp_sys_dma_buf->attachment, xrp_sys_dma_buf->sgt, DMA_BIDIRECTIONAL);
fail_detach:
	dma_buf_detach(xrp_sys_dma_buf->dma_buf, xrp_sys_dma_buf->attachment);
fail_put:
	xr_dmabuf_kernel_unaccount(xrp_sys_dma_buf->dma_buf, XR_DMABUF_KERNEL_NPU);
	dma_buf_put(xrp_sys_dma_buf->dma_buf);
	dma_heap_buffer_free(xrp_sys_dma_buf->dma_buf);
fail_free:
	kfree(xrp_sys_dma_buf);

	return NULL;
}

int sys_nocache_unmap(struct xrp_task_device *xrp_task_dev,
						struct sys_nocache_heap_data *heap_data)
{
	struct sys_nocache_dma_buf *xrp_sys_dma_buf = NULL;
	unsigned long size;

	xrp_print(LOG_LEVEL_DEBUG, "data_heap_flag = %d, data size = %d\n",
			heap_data->data_heap_flag, heap_data->size);

	xrp_sys_dma_buf = sys_nocache_dma_buf_find(heap_data->data_heap_flag, xrp_task_dev);
	if (!xrp_sys_dma_buf) {
		xrp_print(LOG_LEVEL_ERROR, "dma_buf_find nothing, data_heap_flag = %d\n",
					heap_data->data_heap_flag);
		return -EINVAL;
	}
	print_dma_buf_info(xrp_sys_dma_buf);
	xrp_print(LOG_LEVEL_DEBUG, "dmabuf->size 0x%lx, act_size 0x%08x",
				xrp_sys_dma_buf->dma_buf->size, xrp_sys_dma_buf->act_size);

	if (xrp_sys_dma_buf->dma_buf->size != ALIGN(heap_data->size, PAGE_SIZE)) {
		xrp_print(LOG_LEVEL_ERROR,
			"dmabuf->size size 0x%lx not match heap_data->size 0x%08x",
				xrp_sys_dma_buf->dma_buf->size, heap_data->size);
		return -EINVAL;
	}

	size = iommu_unmap(xrp_task_dev->domain, heap_data->addr, xrp_sys_dma_buf->dma_buf->size);
	if (size > xrp_sys_dma_buf->dma_buf->size) {
		xrp_print(LOG_LEVEL_ERROR, "iommu unmap failed, map size 0x%lx, sgt size 0x%lx\n",
									size, xrp_sys_dma_buf->dma_buf->size);
		return -ENOMEM;
	}
	xrp_print(LOG_LEVEL_DEBUG, "iommu unmap success, map size 0x%lx, sgt size 0x%lx\n",
								size, xrp_sys_dma_buf->dma_buf->size);

	dma_buf_vunmap(xrp_sys_dma_buf->dma_buf, &xrp_sys_dma_buf->map);
	dma_buf_unmap_attachment(xrp_sys_dma_buf->attachment, xrp_sys_dma_buf->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(xrp_sys_dma_buf->dma_buf, xrp_sys_dma_buf->attachment);

	sys_nocache_dma_buf_del(xrp_sys_dma_buf, xrp_task_dev);
	xr_dmabuf_kernel_unaccount(xrp_sys_dma_buf->dma_buf, XR_DMABUF_KERNEL_NPU);
	dma_buf_put(xrp_sys_dma_buf->dma_buf);
	dma_heap_buffer_free(xrp_sys_dma_buf->dma_buf);
	kfree(xrp_sys_dma_buf);

	return true;
}
