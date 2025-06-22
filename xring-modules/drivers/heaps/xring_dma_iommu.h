/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __XRING_DMA_IOMMU_H__
#define __XRING_DMA_IOMMU_H__

int xring_dma_map_sgtable_lazy(struct device *dev, struct sg_table *sgt,
			enum dma_data_direction dir, unsigned long attrs,
			struct dma_buf *dma_buf);

int xring_dma_unmap_sgtable_lazy(struct device *dev, struct sg_table *sgt,
			enum dma_data_direction dir, unsigned long attrs,
			struct dma_buf *dma_buf);

void xring_dma_buf_clean(struct dma_buf *dmabuf);

void xring_dummy_smmu_sg_table(struct device *dev, struct sg_table *table);

#endif
