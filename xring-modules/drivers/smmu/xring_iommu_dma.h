/* SPDX-License-Identifier: GPL-2.0 */
/*
 * X-Ring DMA-API to IOMMU-API.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __LINUX_XRING_IOMMU_DMA_H
#define __LINUX_XRING_IOMMU_DMA_H

#ifdef CONFIG_XRING_IOMMU_MAP
void xring_dma_map_ops_init(void);
int xring_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			struct sg_table *sgt, enum dma_data_direction dir,
			unsigned long attrs);
void xring_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			struct sg_table *sgt, enum dma_data_direction dir,
			unsigned long attrs);
#else
void xring_dma_map_ops_init(void)
{
}
int xring_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			struct sg_table *sgt, enum dma_data_direction dir,
			unsigned long attrs)
{
	return 0;
}
void xring_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			struct sg_table *sgt, enum dma_data_direction dir,
			unsigned long attrs)
{
}
#endif

#endif /* __LINUX_XRING_IOMMU_DMA_H */

