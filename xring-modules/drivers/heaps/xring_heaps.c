// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Xring heaps
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 * Based on the ION heap code
 * Copyright (C) 2011 Google, Inc.
 */

#include <linux/version.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/dma-heap.h>
#include <linux/genalloc.h>
#include <linux/dma-buf.h>
#include <linux/highmem.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/of.h>

#include "xr_dmabuf_pagepool.h"
#include "xr_heap_tee_ops.h"
#include "carveout_heap.h"
#include "xring_heaps.h"
#include "system_heap.h"
#include "cma_heap.h"
#include "cga_heap.h"
#include "sga_heap.h"
#include "cpa_heap.h"

#if KERNEL_VERSION(6, 5, 0) <= LINUX_VERSION_CODE
MODULE_IMPORT_NS(DMA_BUF);
#endif

#define XRING_HEAP_MAX 32

static struct xring_heap_platform_data xring_heap_list[XRING_HEAP_MAX];
static struct cma *xring_cma[MAX_CMA_AREAS];
static int xring_heap_count;
static int xring_cma_count;
static int xring_secure_heap_count;

static int save_all_cma(struct cma *cma, void *data)
{
	xring_cma[xring_cma_count] = cma;
	xring_cma_count++;
	return 0;
}

struct cma *xr_cma_get_by_name(const char *name)
{
	int i;

	for (i = 0; i < xring_cma_count; i++) {
		if (!strcmp(xring_cma[i]->name, name))
			return xring_cma[i];
	}

	return NULL;
}

static int xr_heap_create(struct device *dev, struct xring_heap_platform_data *heap_area)
{
	int ret = 0;

	heap_area->priv = dev;

	switch (heap_area->heap_type) {
	case CMA_HEAP:
		ret = xr_cma_heap_create(heap_area);
		if (ret < 0)
			xrheap_err("Xring heap: %s create failed\n", heap_area->heap_name);
		break;
	case CARVEOUT_HEAP:
		ret = xr_carveout_heap_create(heap_area);
		if (ret < 0)
			xrheap_err("Xring heap: %s create failed\n", heap_area->heap_name);
		break;
	case SGA_HEAP:
		ret = xr_sga_heap_create(heap_area);
		if (ret < 0)
			xrheap_err("Xring heap: %s create failed\n", heap_area->heap_name);
		break;
	case CGA_HEAP:
		ret = xr_cga_heap_create(heap_area);
		if (ret < 0)
			xrheap_err("Xring heap: %s create failed\n", heap_area->heap_name);
		break;
	case CPA_HEAP:
		ret = xr_cpa_heap_create(heap_area);
		if (ret < 0)
			xrheap_err("Xring heap: %s create failed\n", heap_area->heap_name);
		break;
	case SYSTEM_HEAP:
		ret = xr_system_heap_create(heap_area);
		if (ret < 0)
			xrheap_err("Xring heap: %s create failed\n", heap_area->heap_name);
		break;
	default:
		xrheap_err("Xring heap: wrong heap type: %d\n", heap_area->heap_type);
		return -1;
	}

	return ret;
}

static int xr_heap_parse_node(struct device_node *node, struct xring_heap_platform_data **data)
{
	struct xring_heap_platform_data *heap_data = NULL;
	struct device_node *mem_region_node = NULL;
	struct reserved_mem *rsv_mem = NULL;
	struct cma *cma = NULL;
	const char *status = "okay";
	u32 heap_type, heap_attr, heap_case;
	int ret;

	ret = of_property_read_string(node, "status", &status);
	if (ret < 0 || !strcmp(status, "disabled")) {
		xrheap_err("Xring heap: %s node status is %s\n", node->name, status);
		return -1;
	}

	ret = of_property_read_u32(node, "heap-type", &heap_type);
	if (ret < 0) {
		xrheap_err("Xring heap: %s parse heap-type failed\n", node->name);
		return ret;
	}

	ret = of_property_read_u32(node, "heap-attr", &heap_attr);
	if (ret < 0) {
		xrheap_err("Xring heap: %s parse heap-attr failed\n", node->name);
		return ret;
	}

	ret = of_property_read_u32(node, "heap-case", &heap_case);
	if (ret < 0) {
		xrheap_err("Xring heap: %s parse heap-case failed\n", node->name);
		return ret;
	}

	mem_region_node = of_parse_phandle(node, "memory-region", 0);
	if (!mem_region_node) {
		xrheap_err("Xring heap: memory region node %s not found\n", node->name);
		return -1;
	}

	rsv_mem = of_reserved_mem_lookup(mem_region_node);
	if (!rsv_mem) {
		xrheap_err("Xring heap: %s is not find in reserved memory list\n", node->name);
		return -1;
	}

	cma = xr_cma_get_by_name(rsv_mem->name);
	if (!cma) {
		xrheap_err("Xring heap: %s can't find cma region\n", node->name);
		return -1;
	}

	heap_data = &xring_heap_list[xring_heap_count];
	heap_data->heap_type = heap_type;
	heap_data->heap_attr = heap_attr;
	heap_data->heap_case = heap_case;
	heap_data->heap_name = node->name;
	heap_data->cma = cma;
	heap_data->cma_base = rsv_mem->base;
	heap_data->cma_size = rsv_mem->size;
	heap_data->node = node;
	*data = &xring_heap_list[xring_heap_count];
	xring_heap_count++;

	if (heap_attr == HEAP_SECURE || heap_attr == HEAP_MIX)
		xring_secure_heap_count++;

	return ret;
}

void cons_heapinfo(struct device *dev)
{
	struct xring_secure_heap_data *secure_heap = NULL;
	int i, j;

	if (!xring_secure_heap_count)
		return;

	secure_heap = devm_kcalloc(dev, xring_secure_heap_count, sizeof(*secure_heap), GFP_KERNEL);
	if (!secure_heap)
		return;

	for (i = 0, j = 0; i < xring_heap_count && j < xring_secure_heap_count; i++) {
		if (xring_heap_list[i].heap_attr == HEAP_SECURE ||
			xring_heap_list[i].heap_attr == HEAP_MIX) {
			secure_heap[j].heap_case = xring_heap_list[i].heap_case;
			secure_heap[j].heap_base = xring_heap_list[i].cma_base;
			secure_heap[j].heap_size = xring_heap_list[i].cma_size;
			j++;
		}
	}

	xr_send_heapinfo(secure_heap, xring_secure_heap_count);
}

static int xr_heap_create_by_node(struct platform_device *pdev)
{
	struct xring_heap_platform_data *heap_data = NULL;
	struct device_node *parent_node = NULL;
	struct device_node *child_node = NULL;
	int ret;

	parent_node = pdev->dev.of_node;
	if (!parent_node) {
		xrheap_err("Xring heap: xring_multi_heaps node not found\n");
		return -1;
	}

	for_each_child_of_node(parent_node, child_node) {
		ret = xr_heap_parse_node(child_node, &heap_data);
		if (ret < 0) {
			xrheap_err("Xring heap: parse node failed\n");
			continue;
		}

		ret = xr_heap_create(&pdev->dev, heap_data);
		if (ret < 0) {
			xrheap_err("Xring heap: create failed\n");
			continue;
		}
	}

#ifdef CONFIG_XRING_SECURE_HEAP
	cons_heapinfo(&pdev->dev);
#endif

	return 0;
}

#ifdef CONFIG_XRING_HEAP_DEBUG
static ssize_t xr_heap_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int i;
	int max_buf_len = 128;
	char *temp_buf = NULL;
	ssize_t temp_buf_len = 0;
	ssize_t buf_len = 0;

	temp_buf = kcalloc(max_buf_len, sizeof(*temp_buf), GFP_KERNEL);
	for (i = 0; i < xring_heap_count; i++) {
		temp_buf_len = snprintf(temp_buf, max_buf_len,
				"heap_name: %s, type: %d, size: %lx\n",
				xring_heap_list[i].heap_name, xring_heap_list[i].heap_type,
				xring_heap_list[i].cma_size);

		strcpy(buf + buf_len, temp_buf);
		buf_len += temp_buf_len;
		if (buf_len >= PAGE_SIZE) {
			xrheap_err("Xring heap: heap list log is out of range\n");
			break;
		}
	}
	kfree(temp_buf);

	return buf_len;
}

static DEVICE_ATTR_RO(xr_heap);

static int xr_sysfs_heap_init(struct platform_device *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_xr_heap);
	if (ret < 0) {
		xrheap_err("Xring heap: sysfs heap init failed\n");
		return ret;
	}
	return ret;
}
#endif

#ifdef CONFIG_XRING_HEAP_DEBUG
static int xr_heap_list_check(void)
{
	int i;

	for (i = 0; i < xring_heap_count; i++) {
		xrheap_info("heap_name: %s, type: %d, size: %lx\n",
			xring_heap_list[i].heap_name, xring_heap_list[i].heap_type,
			xring_heap_list[i].cma_size);
	}

	return 0;
}
#endif

static int xr_heap_probe(struct platform_device *pdev)
{
	int ret;

	cma_for_each_area(save_all_cma, NULL);
	ret = xr_heap_create_by_node(pdev);
	if (ret < 0) {
		xrheap_err("Xring heap: %s get parent node failed\n", pdev->name);
		return -1;
	}

#ifdef CONFIG_XRING_HEAP_DEBUG
	ret = xr_sysfs_heap_init(pdev);
	if (ret < 0) {
		xrheap_err("Xring heap: sysfs heap init failed\n");
		return -1;
	}
#endif

#ifdef CONFIG_XRING_HEAP_DEBUG
	xr_heap_list_check();
#endif
	return ret;
}

static int xr_heap_remove(struct platform_device *pdev)
{
	return 0;
}
static const struct of_device_id xring_heap_of_match[] = {
	{ .compatible = "xring,multi-heaps" },
	{}
};

MODULE_DEVICE_TABLE(of, xring_heap_of_match);

static struct platform_driver xring_heap_platform_driver = {
	.driver = {
	.name = "xring-heap-driver",
		.of_match_table = xring_heap_of_match,
	},
	.probe = xr_heap_probe,
	.remove = xr_heap_remove,
};

static int __init xring_heap_init(void)
{
#ifdef CONFIG_XRING_SECURE_HEAP
	xr_secmem_tee_init();
#endif
	xr_dmabuf_pagepool_init();
	xr_dmabuf_helper_init();
	platform_driver_register(&xring_heap_platform_driver);
	xr_meminfo_init();
	return 0;
}

static void __exit xring_heap_exit(void)
{
	xr_meminfo_exit();
#ifdef CONFIG_XRING_SECURE_HEAP
	xr_secmem_tee_destroy();
#endif
	xr_dmabuf_helper_exit();
	platform_driver_unregister(&xring_heap_platform_driver);
}
module_init(xring_heap_init);
module_exit(xring_heap_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring Xring Heap");
MODULE_LICENSE("GPL v2");
