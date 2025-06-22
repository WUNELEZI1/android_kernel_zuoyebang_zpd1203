// SPDX-License-Identifier: GPL-2.0
/*
 * X-Ring DMA-API to IOMMU-API.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <linux/acpi.h>
#include <linux/cache.h>
#include <linux/dma-direct.h>
#include <linux/dma-mapping.h>
#include <linux/dma-map-ops.h>
#include <linux/export.h>
#include <linux/genalloc.h>
#include <linux/gfp.h>
#include <linux/iommu.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <trace/hooks/iommu.h>
#include <soc/xring/xring_smmu_wrapper.h>
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
#include <linux/dma-iommu.h>
#else
#include "../../../common/drivers/iommu/dma-iommu.h"
#endif
#include "xring_smmu.h"
#include "xring_iommu_map.h"

extern bool dumpmapunmap;

static int xring_dma_info_to_prot(enum dma_data_direction dir, bool coherent,
				unsigned long attrs)
{
	int prot = coherent ? IOMMU_CACHE : 0;

	switch (dir) {
	case DMA_BIDIRECTIONAL:
		return prot | IOMMU_READ | IOMMU_WRITE;
	case DMA_TO_DEVICE:
		return prot | IOMMU_READ;
	case DMA_FROM_DEVICE:
		return prot | IOMMU_WRITE;
	default:
		return 0;
	}
}

static void xring_iommu_dma_sync_single_for_cpu(struct device *dev,
						dma_addr_t phys, size_t size,
						enum dma_data_direction dir)
{
	struct arm_smmu_device *smmu;

	if (dev_is_dma_coherent(dev))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu)
		return;

	dma_sync_single_for_cpu(smmu->dev, phys, size, dir);
}

static void xring_iommu_dma_sync_single_for_device(struct device *dev,
						dma_addr_t phys, size_t size,
						enum dma_data_direction dir)
{
	struct arm_smmu_device *smmu;

	if (dev_is_dma_coherent(dev))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu)
		return;

	dma_sync_single_for_device(smmu->dev, phys, size, dir);
}

static void xring_iommu_dma_sync_sg_for_cpu(struct device *dev,
					struct scatterlist *sgl, int nelems,
					enum dma_data_direction dir)
{
	struct scatterlist *sg;
	struct arm_smmu_device *smmu;
	int i;

	if (dev_is_dma_coherent(dev))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu)
		return;

	for_each_sg(sgl, sg, nelems, i)
		dma_sync_single_for_cpu(smmu->dev, sg_phys(sg), sg->length, dir);
}

static void xring_iommu_dma_sync_sg_for_device(struct device *dev,
					struct scatterlist *sgl, int nelems,
					enum dma_data_direction dir)
{
	struct scatterlist *sg;
	struct arm_smmu_device *smmu;
	int i;

	if (dev_is_dma_coherent(dev))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu)
		return;

	for_each_sg(sgl, sg, nelems, i)
		dma_sync_single_for_device(smmu->dev, sg_phys(sg), sg->length, dir);
}

static phys_addr_t xring_iommu_iova_to_phys(struct device *dev, dma_addr_t iova)
{
	struct iommu_domain *domain = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s has no iommu domain!\n", __func__);
		return 0;
	}

	return iommu_iova_to_phys(domain, iova);
}

static void *xring_iommu_dma_alloc(struct device *dev, size_t size,
				dma_addr_t *dma_handle, gfp_t gfp,
				unsigned long attrs)
{
	void *addr;
	struct arm_smmu_device *smmu;
	size_t iosize = PAGE_ALIGN(size);

	if (WARN(!dev, "cannot alloc for unknown device\n"))
		return NULL;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu)
		return NULL;

	addr = dma_alloc_attrs(smmu->dev, iosize, dma_handle, gfp, attrs);
	if (!addr) {
		dev_err(dev, "%s alloc failed:size:%zx, %#x, attrs:%lx\n",
			__func__, size, gfp, attrs);
		return NULL;
	}

	return addr;
}

static void xring_iommu_dma_free(struct device *dev, size_t size, void *vaddr,
				dma_addr_t dma_handle, unsigned long attrs)
{
	struct arm_smmu_device *smmu;
	size_t iosize = PAGE_ALIGN(size);

	if (WARN(!dev, "cannot free for unknown device\n"))
		return;

	if (WARN(!vaddr, "cannot free for null pointer\n"))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu)
		return;

	dma_free_attrs(smmu->dev, iosize, vaddr, dma_handle, attrs);
}

static dma_addr_t xring_iommu_dma_map_page(struct device *dev, struct page *page,
					unsigned long offset, size_t size,
					enum dma_data_direction dir,
					unsigned long attrs)
{
	phys_addr_t phys;
	size_t iosize;
	int prot;
	unsigned long iova;

	if (!dev || !page) {
		xring_smmu_err("input error, dev %pK, page %pK,\n", dev, page);
		return DMA_MAPPING_ERROR;
	}

	if (!size) {
		xring_smmu_err("size is 0, no need to map\n");
		return DMA_MAPPING_ERROR;
	}

	phys = page_to_phys(page);
	iosize = PAGE_ALIGN(size + offset);
	prot = xring_dma_info_to_prot(dir, dev_is_dma_coherent(dev), attrs);
	iova = xring_iommu_map(dev, phys, iosize, prot);
	if (!iova) {
		dev_err(dev, "%s map:offset:%lu,size:%lu,dir:%d,attrs:%lx\n",
			__func__, offset, size, dir, attrs);
		return DMA_MAPPING_ERROR;
	}

	if (dumpmapunmap)
		dev_info(dev, "SMMU TEST LOG, %s iova:0x%lx, pa:0x%llx, size:0x%zx\n",
			__func__, iova, phys, size);
	return iova + offset;
}

static void xring_iommu_dma_unmap_page(struct device *dev, dma_addr_t iova,
					size_t size, enum dma_data_direction dir,
					unsigned long attrs)
{
	phys_addr_t phys;
	size_t iosize;

	if (dumpmapunmap)
		dev_info(dev, "SMMU TEST LOG, %s iova:0x%llx, size:0x%zx\n",
			__func__, iova, size);

	if (!dev || !iova) {
		xring_smmu_err("input error, dev %pK, iova= 0x%llx\n", dev, iova);
		return;
	}

	if (!size) {
		xring_smmu_err("size is 0, no need to unmap\n");
		return;
	}

	phys = xring_iommu_iova_to_phys(dev, iova);
	if (!phys) {
		dev_err(dev, "%s iova-0x%llx not exist\n", __func__, iova);
		return;
	}

	iosize = PAGE_ALIGN(size + iova - ALIGN_DOWN(iova, PAGE_SIZE));
	if (xring_iommu_unmap(dev, ALIGN_DOWN(iova, PAGE_SIZE), iosize)) {
		dev_err(dev, "%s failed to unmap iommu:iova:%llx, size:%zx\n",
			__func__, iova, size);
		return;
	}
}

static int xring_iommu_dma_map_sg(struct device *dev,
				struct scatterlist *sgl, int nelems,
				enum dma_data_direction dir,
				unsigned long attrs)
{
	unsigned long iova;
	unsigned long out_size = 0;
	int prot;

	if (!dev || !sgl || !nelems) {
		xring_smmu_err("dev %pK, sgl %pK, or nelems is 0\n", dev, sgl);
		return 0;
	}

	prot = xring_dma_info_to_prot(dir, dev_is_dma_coherent(dev), attrs);
	iova = xring_iommu_map_sg(dev, sgl, nelems, prot, &out_size, 0);
	if (!iova)
		return 0;

	return nelems;
}

static void xring_iommu_dma_unmap_sg(struct device *dev, struct scatterlist *sgl,
				int nelems, enum dma_data_direction dir,
				unsigned long attrs)
{
	if (!dev || !sgl || !nelems) {
		xring_smmu_err("dev %pK, sgl %pK, or nelems is 0\n", dev, sgl);
		return;
	}

	if (xring_iommu_unmap_sg(dev, sgl, nelems))
		dev_err(dev, "%s:sg:%pK, nelems:%d, dir:%d, attrs:%lx\n",
			__func__, sgl, nelems, dir, attrs);
}

int xring_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			struct sg_table *sgt, enum dma_data_direction dir,
			unsigned long attrs)
{
	int prot;
	unsigned long iova = 0;

	if (!dev || !dmabuf || !sgt) {
		xring_smmu_err("input err, dev %pK, dmabuf %pK, sgt %pK\n",
			dev, dmabuf, sgt);
		return -EINVAL;
	}

	prot = xring_dma_info_to_prot(dir, dev_is_dma_coherent(dev), attrs);
	iova = xring_iommu_map_dmabuf_internal(dev, dmabuf, sgt, prot);
	if (!iova) {
		dev_err(dev, "map dmabuf fail!\n");
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL(xring_iommu_map_dmabuf);

void xring_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			struct sg_table *sgt, enum dma_data_direction dir,
			unsigned long attrs)
{
	if (!dev || !dmabuf || !sgt) {
		xring_smmu_err("input err, dev %pK, dmabuf %pK, sgt %pK\n",
			dev, dmabuf, sgt);
		return;
	}

	xring_iommu_unmap_dmabuf_internal(dev, dmabuf, sgt);
}
EXPORT_SYMBOL(xring_iommu_unmap_dmabuf);

static const struct dma_map_ops xring_iommu_dma_ops = {
	.alloc = xring_iommu_dma_alloc,
	.free = xring_iommu_dma_free,
	.map_page = xring_iommu_dma_map_page,
	.unmap_page = xring_iommu_dma_unmap_page,
	.map_sg = xring_iommu_dma_map_sg,
	.unmap_sg = xring_iommu_dma_unmap_sg,
	.sync_single_for_cpu = xring_iommu_dma_sync_single_for_cpu,
	.sync_single_for_device = xring_iommu_dma_sync_single_for_device,
	.sync_sg_for_cpu = xring_iommu_dma_sync_sg_for_cpu,
	.sync_sg_for_device = xring_iommu_dma_sync_sg_for_device,
};

static void xring_iommu_setup_dma_ops(void *data, struct device *dev,
						u64 dma_base, u64 size)
{
	if (!dev)
		return;

	dev->dma_ops = &xring_iommu_dma_ops;
	dev_info(dev, "dev setup dma_map_ops\n");
}

void xring_dma_map_ops_init(void)
{
	int ret = register_trace_android_rvh_iommu_setup_dma_ops(
		xring_iommu_setup_dma_ops, NULL);

	xring_smmu_info("re=%d\n", ret);
}
