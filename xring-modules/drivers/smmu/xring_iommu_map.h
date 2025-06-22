/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * X-Ring iommu mapping core.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#ifndef __LINUX_XRING_IOMMU_MAP_H
#define __LINUX_XRING_IOMMU_MAP_H

#include <linux/iommu.h>
#include <linux/dma-buf.h>

struct iova_entry {
	struct rb_node node;
	unsigned long iova;
	unsigned long size;
	atomic_long_t ref;
	u64 key;
	struct device *dev;
};

void xring_iommu_flush_tlb(struct device *dev,
				struct iommu_domain *domain);

unsigned long xring_iommu_map(struct device *dev, phys_addr_t paddr,
				size_t size, int prot);
int xring_iommu_unmap(struct device *dev, unsigned long iova, size_t size);
unsigned long xring_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int nelems, int prot, unsigned long *out_size, s64 header_size);
int xring_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl, int nelems);

unsigned long xring_iommu_map_dmabuf_internal(struct device *dev, struct dma_buf *dmabuf,
					struct sg_table *sgt, int prot);
int xring_iommu_unmap_dmabuf_internal(struct device *dev, struct dma_buf *dmabuf,
				      struct sg_table *sgt);

#endif /* __LINUX_XRING_IOMMU_MAP_H */
