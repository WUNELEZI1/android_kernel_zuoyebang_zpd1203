// SPDX-License-Identifier: GPL-2.0
/*
 * XRING Page Table implementation
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>

#include "../smmu/xring_iommu_dma.h"
#include "xring_mem_adapter.h"

#ifdef CONFIG_XRING_HEAP_DMA_MAP
int xring_heap_dma_map(struct device *dev, struct dma_buf *dmabuf,
	struct sg_table *sgt, enum dma_data_direction dir, unsigned long attrs)
{
	int ret = 0;

	if (!dev->dma_ops)
		ret = dma_map_sgtable(dev, sgt, dir, attrs);
	else
		ret = xring_iommu_map_dmabuf(dev, dmabuf, sgt, dir, attrs);

	return ret;
}
EXPORT_SYMBOL(xring_heap_dma_map);

void xring_heap_dma_unmap(struct device *dev, struct dma_buf *dmabuf,
	struct sg_table *sgt, enum dma_data_direction dir, unsigned long attrs)
{
	if (!dev->dma_ops)
		dma_unmap_sgtable(dev, sgt, dir, attrs);
	else
		xring_iommu_unmap_dmabuf(dev, dmabuf, sgt, dir, attrs);
}
EXPORT_SYMBOL(xring_heap_dma_unmap);
#else
int xring_heap_dma_map(struct device *dev, struct dma_buf *dmabuf,
	struct sg_table *sgt, enum dma_data_direction dir, unsigned long attrs)
{
	return dma_map_sgtable(dev, sgt, dir, attrs);
}
EXPORT_SYMBOL(xring_heap_dma_map);

void xring_heap_dma_unmap(struct device *dev, struct dma_buf *dmabuf,
	struct sg_table *sgt, enum dma_data_direction dir, unsigned long attrs)
{
	dma_unmap_sgtable(dev, sgt, dir, attrs);
}
EXPORT_SYMBOL(xring_heap_dma_unmap);
#endif

void xr_change_page_flag(struct sg_table *table, bool set)
{
	int i, j;
	struct page *page;
	struct scatterlist *sg = NULL;
	int nr_pages = 0;

	if (!table)
		return;

	for_each_sgtable_sg(table, sg, i) {
		nr_pages = sg->length / PAGE_SIZE;
		page = sg_page(sg);
		if (set) {
			for (j = 0; j < nr_pages; j++)
				set_bit(PG_oem_reserved_2, &page[j].flags);
		} else {
			for (j = 0; j < nr_pages; j++)
				clear_bit(PG_oem_reserved_2, &page[j].flags);
		}
	}
}
EXPORT_SYMBOL(xr_change_page_flag);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring MEM ADAPTER Driver");
MODULE_LICENSE("GPL v2");
