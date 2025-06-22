// SPDX-License-Identifier: GPL-2.0
/*
 * X-Ring iommu mapping core.
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <linux/bitops.h>
#include <linux/debugfs.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/iommu.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <linux/version.h>

#include "xring_smmu.h"
#include "xring_iommu_map.h"
#include "xring_iommu_iova_cookie.h"
#include "soc/xring/xr_dmabuf_helper.h"

extern bool dumpmapunmap;

void xring_iommu_flush_tlb(struct device *dev,
				struct iommu_domain *domain)
{
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);

	if (!fwspec) {
		dev_err(dev, "%s: iommu_fwspec is null\n", __func__);
		return;
	}

	if (!fwspec->ops) {
		dev_err(dev, "%s: iommu_fwspec ops is null\n", __func__);
		return;
	}

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	if (fwspec->ops->flush_iotlb_all)
		fwspec->ops->flush_iotlb_all(domain);
#else
	if (fwspec->ops->default_domain_ops->flush_iotlb_all)
		fwspec->ops->default_domain_ops->flush_iotlb_all(domain);
#endif
}

unsigned long xring_iommu_map(struct device *dev, phys_addr_t paddr,
				size_t size, int prot)
{
	unsigned long iova;
	size_t iova_size;
	int ret;
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie  *iova_cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return 0;
	}

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: xring_iommu_cookie is null\n", __func__);
		return 0;
	}

	if (!iova_cookie->iova_pool) {
		dev_err(dev, "%s: iova_pool is null\n", __func__);
		return 0;
	}

	iova_size = ALIGN(size, PAGE_SIZE);
	iova = xring_iommu_iova_alloc(iova_cookie->iova_pool, iova_size,
					iova_cookie->iova_align);

	if (!iova && (iova_cookie->iova_free == DELAYED_FREE))
		iova = xring_iommu_realloc_iova(dev, domain, iova_size);

	if (!iova) {
		dev_err(dev, "%s: alloc iova failed! size 0x%zx\n",
			__func__, size);
		return 0;
	}
	prot |= IOMMU_READ | IOMMU_WRITE;
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	ret = iommu_map(domain, iova, paddr, size, prot);
#else
	ret = iommu_map(domain, iova, paddr, size, prot, GFP_KERNEL);
#endif
	if (ret) {
		dev_err(dev, "%s: map fail, address 0x%llx, size 0x%zx\n",
			__func__, paddr, size);
		if (iova_cookie->iova_free == DELAYED_FREE)
			xring_iommu_iova_free_delay(iova_cookie, iova, iova_size);
		else
			xring_iommu_iova_free_imme(iova_cookie->iova_pool, iova, iova_size);
	}

	return iova;
}

int xring_iommu_unmap(struct device *dev, unsigned long iova, size_t size)
{
	size_t iova_size;
	size_t unmap_size;
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return 0;
	}

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: iova_cookie is null\n", __func__);
		return 0;
	}

	if (!iova_cookie->iova_pool) {
		dev_err(dev, "%s: iova_pool is null\n", __func__);
		return 0;
	}

	iova_size = ALIGN(size, PAGE_SIZE);
	unmap_size = iommu_unmap(domain, iova, iova_size);
	if (unmap_size != iova_size) {
		dev_err(dev, "%s: unmap fail! size 0x%lx, unmap_size 0x%zx\n",
			__func__, size, unmap_size);
		return -EINVAL;
	}

	if (iova_cookie->iova_free == DELAYED_FREE) {
		xring_iommu_iova_free_delay(iova_cookie, iova, iova_size);
		return 0;
	}
	xring_iommu_iova_free_imme(iova_cookie->iova_pool, iova, size);

	return 0;
}

static size_t xring_iommu_sg_size_get(struct scatterlist *sgl, int nents)
{
	int i;
	size_t iova_size = 0;
	struct scatterlist *sg = NULL;

	for_each_sg(sgl, sg, nents, i)
		iova_size += (size_t)ALIGN(sg->length, PAGE_SIZE);

	return iova_size;
}

static ssize_t xring_map_sg_with_afbc(struct iommu_domain *domain, unsigned long iova,
		struct scatterlist *sgl, unsigned int nents, int prot, s64 header_size)
{
	struct scatterlist *sg;
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	const struct iommu_ops *ops = domain->ops;
#else
	const struct iommu_domain_ops *ops = domain->ops;
#endif
	size_t mapped = 0;
	int ret, i = 0;

	for_each_sg(sgl, sg, nents, i) {
		unsigned int sg_page_count = sg->length >> PAGE_SHIFT;

		if (header_size >= sg_page_count) {
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
			ret = iommu_map(domain, iova + mapped, sg_phys(sg), sg->length, prot);
#else
			ret = iommu_map(domain, iova + mapped, sg_phys(sg), sg->length, prot,
						GFP_KERNEL);
#endif
			if (ret)
				goto out_err;

			header_size -= sg_page_count;
		} else if (header_size > 0 && header_size < sg_page_count) {
			unsigned long long no_afbc_length = header_size << PAGE_SHIFT;

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
			ret = iommu_map(domain, iova + mapped, sg_phys(sg),
						no_afbc_length, prot);
			if (ret)
				goto out_err;
			ret = iommu_map(domain, iova + mapped + no_afbc_length,
						(sg_phys(sg) + no_afbc_length) | 1ULL << 38,
						sg->length - no_afbc_length, prot);
#else
			ret = iommu_map(domain, iova + mapped, sg_phys(sg),
						no_afbc_length, prot, GFP_KERNEL);
			if (ret)
				goto out_err;
			ret = iommu_map(domain, iova + mapped + no_afbc_length,
						(sg_phys(sg) + no_afbc_length) | 1ULL << 38,
						sg->length - no_afbc_length, prot, GFP_KERNEL);
#endif
			if (ret)
				goto out_err;
			header_size = 0;
		} else {
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
			ret = iommu_map(domain, iova + mapped, sg_phys(sg) | 1ULL << 38,
						sg->length, prot);
#else
			ret = iommu_map(domain, iova + mapped, sg_phys(sg) | 1ULL << 38,
						sg->length, prot, GFP_KERNEL);
#endif
			if (ret)
				goto out_err;
		}
		mapped += sg->length;
	}

	if (ops->iotlb_sync_map)
		ops->iotlb_sync_map(domain, iova, mapped);
	return mapped;

out_err:
	/* undo mappings already done */
	iommu_unmap(domain, iova, mapped);

	return ret;
}

static unsigned long __xring_iommu_map_sg(struct device *dev,
				struct scatterlist *sgl, int prot,
				unsigned long *out_size, s64 header_size)
{
	struct gen_pool *iova_pool = NULL;
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;
	struct scatterlist *dump_use_sg = NULL;
	size_t iova_size, map_size;
	unsigned long iova;
	unsigned long dump_use_iova;
	int nents;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return 0;
	}

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: iommu domain cookie is null\n", __func__);
		return 0;
	}

	iova_pool = iova_cookie->iova_pool;
	if (!iova_pool) {
		dev_err(dev, "%s: iova pool is null\n", __func__);
		return 0;
	}

	nents = sg_nents(sgl);
	iova_size = xring_iommu_sg_size_get(sgl, nents);
	iova = xring_iommu_iova_alloc(iova_pool, iova_size, iova_cookie->iova_align);
	if (!iova && (iova_cookie->iova_free == DELAYED_FREE))
		iova = xring_iommu_realloc_iova(dev, domain, iova_size);

	if (!iova) {
		dev_err(dev, "%s: alloc iova failed, size 0x%zx\n",
			__func__, iova_size);
		return 0;
	}

	if (dumpmapunmap) {
		dump_use_sg = sgl;
		dump_use_iova = iova;
		for (uint i = 0; i < nents; i++) {
			dev_info(dev, "SMMU TEST LOG %s %d -- iova:0x%lx, size:0x%x, pa:0x%llx", __func__, i, dump_use_iova,
					dump_use_sg->length, sg_phys(dump_use_sg));
			dump_use_iova += dump_use_sg->length;
			dump_use_sg = sg_next(dump_use_sg);
		}
	}

	if (!header_size)
#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
		map_size = iommu_map_sg(domain, iova, sgl, nents, prot);
#else
		map_size = iommu_map_sg(domain, iova, sgl, nents, prot, GFP_KERNEL);
#endif
	else
		map_size = xring_map_sg_with_afbc(domain, iova, sgl, nents, prot, header_size);

	if (map_size != iova_size) {
		dev_err(dev, "%s: map fail! iova 0x%lx, iova_size 0x%zx\n",
			__func__, iova, iova_size);
		if (iova_cookie->iova_free == DELAYED_FREE)
			xring_iommu_iova_free_delay(iova_cookie, iova, iova_size);
		else
			xring_iommu_iova_free_imme(iova_pool, iova, iova_size);
		return 0;
	}
	*out_size = (unsigned long)iova_size;

	return iova;
}

unsigned long xring_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int nelems, int prot, unsigned long *out_size, s64 header_size)
{
	unsigned long iova;
	unsigned long mapped = 0;
	struct scatterlist *sg = NULL;
	int i = 0;

	prot |= IOMMU_READ | IOMMU_WRITE;
	iova = __xring_iommu_map_sg(dev, sgl, prot, out_size, header_size);
	if (!iova) {
		dev_err(dev, "%s map_sg: nelems:%d, prot:%x\n",
			__func__, nelems, prot);
		return 0;
	}

	for_each_sg(sgl, sg, nelems, i) {
		sg->dma_address = iova + mapped;
		sg_dma_len(sg) = sg->length;
		mapped += sg->length;
	}

	if (*out_size != mapped) {
		dev_err(dev, "%s nelem:%d, prot:%x, out_size:%lx, map:%lx\n",
			__func__, nelems, prot, *out_size, mapped);
		return 0;
	}

	return iova;
}

static int xring_iommu_unmap_sg_check(struct iommu_domain *domain,
				unsigned long iova, size_t iova_size)
{
	struct rb_node *n = NULL;
	struct iova_entry *iova_entry = NULL;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;
	bool is_overlap = false;
	int ret = 0;

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		xring_smmu_err("iommu domain iova_cookie is null\n");
		return -EINVAL;
	}

	spin_lock(&iova_cookie->lock);
	for (n = rb_first(&iova_cookie->iova_root); n; n = rb_next(n)) {
		iova_entry = rb_entry(n, struct iova_entry, node);
		is_overlap = max(iova_entry->iova, iova) <
			min(iova_entry->iova + iova_entry->size,
				iova + iova_size);
		/*lint -e548 */
		if (is_overlap) {
			xring_smmu_err("iova_entry iova: 0x%lx, size: 0x%lx\n"
				"iova: 0x%lx, size: 0x%lx\n",
				iova_entry->iova, iova_entry->size,
				iova, iova_size);
			ret = -EINVAL;
			WARN_ON(1);
			break;
		}
		/*lint +e548 */
	}
	spin_unlock(&iova_cookie->lock);

	return ret;
}

static int __xring_iommu_unmap_sg(struct device *dev, struct iommu_domain *domain,
				unsigned long iova, size_t iova_size)
{
	size_t unmap_size;
	int ret;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;
	struct gen_pool *iova_pool = NULL;

	if (dumpmapunmap)
		dev_info(dev, "SMMU TEST LOG, %s iova:0x%lx, size:0x%zx\n", __func__, iova, iova_size);

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: iommu domain iova_cookie is null\n", __func__);
		return -EINVAL;
	}

	iova_pool = iova_cookie->iova_pool;
	if (!iova_pool) {
		dev_err(dev, "%s: iova pool is null\n", __func__);
		return -EINVAL;
	}

	ret = gen_pool_has_addr(iova_pool, iova, iova_size);
	if (!ret) {
		dev_err(dev, "%s: illegal para, iova = %lx, size = %lx\n",
			__func__, iova, iova_size);
		return -EINVAL;
	}

	unmap_size = iommu_unmap(domain, iova, iova_size);
	if (unmap_size != iova_size) {
		dev_err(dev, "%s: unmap fail! size 0x%lx, unmap_size 0x%zx\n",
			__func__, iova_size, unmap_size);
		return -EINVAL;
	}

	if (iova_cookie->iova_free == DELAYED_FREE) {
		xring_iommu_iova_free_delay(iova_cookie, iova, iova_size);
		return 0;
	}
	xring_iommu_iova_free_imme(iova_pool, iova, iova_size);
	return 0;
}

int xring_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl, int nelems)
{
	int ret;
	unsigned long iova;
	size_t iova_size;
	struct iommu_domain *domain = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return -EINVAL;
	}

	iova = sgl->dma_address;
	iova_size = xring_iommu_sg_size_get(sgl, nelems);
	ret = xring_iommu_unmap_sg_check(domain, iova, iova_size);
	if (ret) {
		dev_err(dev, "%s: unmap_sg check fail!\n", __func__);
		return ret;
	}

	ret = __xring_iommu_unmap_sg(dev, domain, iova, iova_size);

	return ret;
}

static struct iova_entry *xring_iova_entry_get(struct rb_root *rbroot,
						struct dma_buf *dmabuf)
{
	struct rb_node *n = NULL;
	struct iova_entry *iova_entry = NULL;
	u64 key = (u64)(uintptr_t)dmabuf;

	for (n = rb_first(rbroot); n; n = rb_next(n)) {
		iova_entry = rb_entry(n, struct iova_entry, node);
		if (iova_entry->key == key)
			return iova_entry;
	}

	return NULL;
}

static int xring_iommu_get_dmabuf_iova(struct device *dev,
				       struct xring_iommu_dma_cookie *iova_cookie,
				       struct dma_buf *dmabuf, unsigned long *iova)
{
	struct iova_entry *iova_entry = NULL;
	u64 ref;

	*iova = 0;
	iova_entry = xring_iova_entry_get(&iova_cookie->iova_root, dmabuf);
	if (!iova_entry)
		return 0;

	if (iova_entry->size < dmabuf->size) {
		dev_err(dev, "%s: size error, iova_entry: %lu, dmabuf: %zu\n",
			__func__, iova_entry->size, dmabuf->size);
		return -EINVAL;
	}

	ref = atomic64_inc_return(&iova_entry->ref);
	dev_dbg(dev, "%s: get iova_entry, ref:%llu\n", __func__, ref);
	*iova = iova_entry->iova;

	return 0;
}

void xring_iova_entry_info(struct device *dev)
{
	struct rb_node *n = NULL;
	struct iova_entry *iova_entry = NULL;
	unsigned long total_size = 0;
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return;
	}

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: iommu domain cookie is null\n", __func__);
		return;
	}

	spin_lock(&iova_cookie->lock);
	for (n = rb_first(&iova_cookie->iova_root); n; n = rb_next(n)) {
		iova_entry = rb_entry(n, struct iova_entry, node);
		total_size += iova_entry->size;
		dev_err(dev, "%s: iova: 0x%lx, size: 0x%lx\n",
			__func__, iova_entry->iova, iova_entry->size);
	}
	spin_unlock(&iova_cookie->lock);

	dev_err(dev, "%s: domain total size: %lu", __func__, total_size);
}

static void iova_entry_add(struct rb_root *rb_root,
				struct iova_entry *iova_entry)
{
	struct rb_node **p = &rb_root->rb_node;
	struct rb_node *parent = NULL;
	struct iova_entry *entry = NULL;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct iova_entry, node);

		if (iova_entry < entry) {
			p = &(*p)->rb_left;
		} else if (iova_entry > entry) {
			p = &(*p)->rb_right;
		} else {
			xring_smmu_err("iova already in tree\n");
			WARN_ON(1);
			return;
		}
	}

	rb_link_node(&iova_entry->node, parent, p);
	rb_insert_color(&iova_entry->node, rb_root);
}

static struct iova_entry *init_iova_entry(struct dma_buf *dmabuf)
{
	struct iova_entry *iova_entry = NULL;

	iova_entry = kzalloc(sizeof(*iova_entry), GFP_KERNEL);
	if (!iova_entry)
		return ERR_PTR(-ENOMEM);

	atomic64_set(&iova_entry->ref, 1);
	iova_entry->key = (u64)(uintptr_t)dmabuf;
	return iova_entry;
}

long long xr_dmabuf_get_headersize_internal(struct dma_buf *dmabuf)
{
	struct xr_heap_buffer_header *header;
	long long headersize;

	if (!dmabuf || !dmabuf->priv) {
		xring_smmu_err("input parameter error\n");
		return -EINVAL;
	}

	header = (struct xr_heap_buffer_header *)(dmabuf->priv);
	headersize = header->header_size;

	return headersize;
}

unsigned long xring_iommu_map_dmabuf_internal(struct device *dev,
			struct dma_buf *dmabuf, struct sg_table *sgt, int prot)
{
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;
	struct iova_entry *iova_entry = NULL;
	struct scatterlist *sg = NULL;
	unsigned long iova = 0;
	unsigned long iova_temp = 0;
	unsigned long mapped = 0;
	int i = 0;
	int j = 0;
	unsigned long size = 0;

	for_each_sg(sgt->sgl, sg, sgt->orig_nents, i) {
		size += sg->length;
	}

	if (!arm_smmu_is_shared_domain(dev)) {
		iova = xring_iommu_map_sg(dev, sgt->sgl, sgt->orig_nents, prot,
		&size, xr_dmabuf_get_headersize_internal(dmabuf));
		if (!iova) {
			dev_err(dev, "map dmabuf sg fail!\n");
			return 0;
		}
		return iova;
	}

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return 0;
	}

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: iova iova_cookie is null\n", __func__);
		return 0;
	}

	/* first check */
	spin_lock(&iova_cookie->lock);
	if (xring_iommu_get_dmabuf_iova(dev, iova_cookie, dmabuf, &iova)) {
		dev_err(dev, "%s: get dmabuf iova error\n", __func__);
		spin_unlock(&iova_cookie->lock);
		goto fail_info;
	}
	spin_unlock(&iova_cookie->lock);

	if (iova) {
		dev_dbg(dev, "%s: dmabuf already mapped, get iova: %lx",
			__func__, iova);
		for_each_sg(sgt->sgl, sg, sgt->orig_nents, j) {
			sg->dma_address = iova + mapped;
			sg_dma_len(sg) = sg->length;
			mapped += sg->length;
		}
		return iova;
	}

	iova_entry = init_iova_entry(dmabuf);
	if (IS_ERR(iova_entry)) {
		dev_err(dev, "%s: init_iova_entry fail!\n", __func__);
		return 0;
	}

	iova_temp = xring_iommu_map_sg(dev, sgt->sgl, sgt->orig_nents, prot,
		&iova_entry->size, xr_dmabuf_get_headersize_internal(dmabuf));
	if (!iova_temp) {
		dev_err(dev, "map dmabuf sg fail!\n");
		goto fail_out;
	}

	/* second check */
	spin_lock(&iova_cookie->lock);
	if (xring_iommu_get_dmabuf_iova(dev, iova_cookie, dmabuf, &iova)) {
		spin_unlock(&iova_cookie->lock);
		dev_err(dev, "%s: get dmabuf iova error\n", __func__);
		goto fail_out;
	}

	if (iova) {
		spin_unlock(&iova_cookie->lock);
		dev_info(dev, "%s: dmabuf already mapped, get iova: %lx\n"
				"unmap iova_temp: %lx\n", __func__, iova, iova_temp);

		if (__xring_iommu_unmap_sg(dev, domain, iova_temp, iova_entry->size)) {
			dev_err(dev, "%s: unmap iova_temp failed!\n", __func__);
			goto fail_out;
		}

		for_each_sg(sgt->sgl, sg, sgt->orig_nents, j) {
			sg->dma_address = iova + mapped;
			sg_dma_len(sg) = sg->length;
			mapped += sg->length;
		}
		kfree(iova_entry);
	} else {
		iova_entry->iova = iova_temp;
		iova_entry->dev = dev;
		iova_entry_add(&iova_cookie->iova_root, iova_entry);
		spin_unlock(&iova_cookie->lock);
		return iova_temp;
	}

	return iova;

fail_out:
	kfree(iova_entry);
fail_info:
	xring_iova_entry_info(dev);

	return 0;
}

int xring_iommu_unmap_dmabuf_internal(struct device *dev, struct dma_buf *dmabuf,
				      struct sg_table *sgt)
{
	int ret;
	struct iova_entry *iova_entry = NULL;
	struct iommu_domain *domain = NULL;
	struct xring_iommu_dma_cookie *iova_cookie = NULL;
	unsigned long iova = 0;
	struct scatterlist *sg = NULL;
	int i = 0;
	unsigned long size = 0;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s: iommu domain is null\n", __func__);
		return -EINVAL;
	}

	iova = sgt->sgl->dma_address;
	for_each_sg(sgt->sgl, sg, sgt->orig_nents, i) {
		size += sg->length;
	}
	if (!arm_smmu_is_shared_domain(dev)) {
		ret = __xring_iommu_unmap_sg(dev, domain, iova, size);
		return ret;
	}

	iova_cookie = to_smmu_domain(domain)->iova_cookie;
	if (!iova_cookie) {
		dev_err(dev, "%s: iova iova_cookie is null\n", __func__);
		return -EINVAL;
	}
	spin_lock(&iova_cookie->lock);
	iova_entry = xring_iova_entry_get(&iova_cookie->iova_root, dmabuf);
	if (!iova_entry) {
		spin_unlock(&iova_cookie->lock);
		dev_err(dev, "%s: unmap buf no map data!\n", __func__);
		return -EINVAL;
	}

	if (!atomic64_dec_and_test(&iova_entry->ref)) {
		spin_unlock(&iova_cookie->lock);
		return 0;
	}

	rb_erase(&iova_entry->node, &iova_cookie->iova_root);
	spin_unlock(&iova_cookie->lock);
	ret = __xring_iommu_unmap_sg(dev, domain, iova_entry->iova, iova_entry->size);
	kfree(iova_entry);

	return ret;
}
