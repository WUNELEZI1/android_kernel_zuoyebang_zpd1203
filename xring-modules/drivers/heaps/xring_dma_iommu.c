// SPDX-License-Identifier: GPL-2.0
/*
 * xring dma map API
 *
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include <linux/module.h>
#include <linux/rbtree.h>
#include <linux/iommu.h>
#include <linux/slab.h>
#include <linux/dma-buf.h>
#include <linux/kref.h>
#include <linux/dma-map-ops.h>

#include "xring_heaps.h"

//#define XRING_DMA_IOMMU_DUMP

static struct rb_root xring_dma_buf_root;
static DEFINE_MUTEX(xring_dma_buf_lock);

struct xring_dma_buf {
	struct rb_node node;
	struct dma_buf *dma_buf;
	struct kref ref;

	struct list_head xmaps;
	struct mutex lock;
};

struct xring_map {
	struct list_head list;
	struct iommu_domain *domain;
	struct kref ref;

	struct scatterlist *sgl;
	int nents;
	struct device *dev;
	enum dma_data_direction dir;
	unsigned long attrs;
};

static void xring_map_destroy(struct xring_map *xmap);

static struct xring_dma_buf *xring_dma_buf_lookup(struct dma_buf *dma_buf)
{
	struct rb_root *root = &xring_dma_buf_root;
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct xring_dma_buf *entry = NULL;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct xring_dma_buf, node);

		if (dma_buf < entry->dma_buf)
			p = &(*p)->rb_left;
		else if (dma_buf > entry->dma_buf)
			p = &(*p)->rb_right;
		else
			return entry;
	}

	return NULL;
}

static void xring_dma_buf_del(struct xring_dma_buf *xdmabuf)
{
	struct rb_root *root = &xring_dma_buf_root;

	if (!list_empty(&xdmabuf->xmaps))
		WARN(1, "xdmabuf->xmaps list is not empty\n");

	rb_erase(&xdmabuf->node, root);
}

static void xring_dma_buf_add(struct xring_dma_buf *xdmabuf)
{
	struct rb_root *root = &xring_dma_buf_root;
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct xring_dma_buf *entry = NULL;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct xring_dma_buf, node);

		if (xdmabuf->dma_buf < entry->dma_buf) {
			p = &(*p)->rb_left;
		} else if (xdmabuf->dma_buf > entry->dma_buf) {
			p = &(*p)->rb_right;
		} else {
			xrheap_err("dma_buf 0x%x already exist\n", xdmabuf->dma_buf);
			return;
		}
	}

	rb_link_node(&xdmabuf->node, parent, p);
	rb_insert_color(&xdmabuf->node, root);
}

static void xring_dma_buf_destroy(struct xring_dma_buf *xdmabuf)
{
	xring_dma_buf_del(xdmabuf);
	mutex_destroy(&xdmabuf->lock);
	kfree(xdmabuf);
}

static void xring_dma_buf_release(struct kref *ref)
{
	struct xring_dma_buf *xdmabuf = container_of(ref, struct xring_dma_buf, ref);

	xrheap_info("dma_buf: 0x%x\n", xdmabuf->dma_buf);

	xring_dma_buf_destroy(xdmabuf);
}

void xring_dma_buf_clean(struct dma_buf *dmabuf)
{
	struct xring_dma_buf *xdmabuf = NULL;
	struct xring_map *xmap = NULL, *tmp = NULL;

	mutex_lock(&xring_dma_buf_lock);
	xdmabuf = xring_dma_buf_lookup(dmabuf);
	if (!xdmabuf) {
		xrheap_err("no dma_buf find\n");
		mutex_unlock(&xring_dma_buf_lock);
		return;
	}
	mutex_unlock(&xring_dma_buf_lock);

	mutex_lock(&xdmabuf->lock);
	if (!list_empty(&xdmabuf->xmaps)) {
		xrheap_info("xdmabuf->xmaps list is not empty\n");

		list_for_each_entry_safe(xmap, tmp, &xdmabuf->xmaps, list) {
			xrheap_info("domain: 0x%x, ref: 0x%x\n",
					xmap->domain, kref_read(&xmap->ref));
			xring_map_destroy(xmap);
		}
	}
	mutex_unlock(&xdmabuf->lock);

	mutex_lock(&xring_dma_buf_lock);
	xrheap_info("dma_buf: 0x%x\n", xdmabuf->dma_buf);
	xring_dma_buf_destroy(xdmabuf);
	mutex_unlock(&xring_dma_buf_lock);
}
EXPORT_SYMBOL(xring_dma_buf_clean);

static struct xring_dma_buf *xring_dma_buf_create(struct dma_buf *dma_buf)
{
	struct xring_dma_buf *xdmabuf = NULL;

	xdmabuf = kzalloc(sizeof(struct xring_dma_buf), GFP_KERNEL);
	if (!xdmabuf)
		return NULL;

	INIT_LIST_HEAD(&xdmabuf->xmaps);
	mutex_init(&xdmabuf->lock);
	kref_init(&xdmabuf->ref);
	xdmabuf->dma_buf = dma_buf;

	xring_dma_buf_add(xdmabuf);
	xrheap_info("dma_buf: 0x%x\n", xdmabuf->dma_buf);
	return xdmabuf;
}

/* lookup xring_map by domain */
static struct xring_map *xring_map_lookup(struct xring_dma_buf *xdmabuf,
				struct iommu_domain *domain)
{
	struct xring_map *xmap = NULL;

	list_for_each_entry(xmap, &xdmabuf->xmaps, list) {
		if (xmap->domain == domain)
			return xmap;
	}

	return NULL;
}

static void xring_map_del(struct xring_map *xmap)
{
	list_del(&xmap->list);
}

static void xring_map_add(struct xring_dma_buf *xdmabuf,
				struct xring_map *xmap)
{
	INIT_LIST_HEAD(&xmap->list);
	list_add_tail(&xmap->list, &xdmabuf->xmaps);
}

static void xring_map_destroy(struct xring_map *xmap)
{
	xring_map_del(xmap);
	kfree(xmap);
}

static void xring_map_release(struct kref *ref)
{
	struct xring_map *xmap = container_of(ref, struct xring_map, ref);

	xrheap_info("domain: 0x%x\n", xmap->domain);

	if (!(xmap->attrs & DMA_ATTR_SKIP_CPU_SYNC))
		xmap->attrs |= DMA_ATTR_SKIP_CPU_SYNC;

	dma_unmap_sg_attrs(xmap->dev, xmap->sgl, xmap->nents,
			xmap->dir, xmap->attrs);

	xring_map_destroy(xmap);
}

static struct xring_map *xring_map_create(struct xring_dma_buf *xdmabuf,
					struct iommu_domain *domain)
{
	struct xring_map *xmap = NULL;

	xmap = kzalloc(sizeof(struct xring_map), GFP_KERNEL);
	if (!xmap)
		return NULL;

	xmap->domain = domain;
	kref_init(&xmap->ref);

	xring_map_add(xdmabuf, xmap);

	xrheap_info("domain: 0x%x\n", xmap->domain);
	return xmap;
}

#ifdef XRING_DMA_IOMMU_DUMP
static void xring_dump_all(void)
{
	struct rb_root *root = &xring_dma_buf_root;
	struct rb_node *p = NULL;
	struct xring_dma_buf *xdmabuf = NULL;
	struct xring_map *xmap = NULL;
	int i = 0, j = 0;

	mutex_lock(&xring_dma_buf_lock);
	for (p = rb_first(root); p; p = rb_next(p)) {
		xdmabuf = rb_entry(p, struct xring_dma_buf, node);
		xrheap_info("[%d] dma_buf: 0x%x, ref: 0x%x\n",
				i, xdmabuf->dma_buf, kref_read(&xdmabuf->ref));
		i++;

		mutex_lock(&xdmabuf->lock);
		list_for_each_entry(xmap, &xdmabuf->xmaps, list) {
			xrheap_info("[%d] domain: 0x%x, ref: 0x%x\n",
					j, xmap->domain, kref_read(&xmap->ref));
			j++;
		}
		mutex_unlock(&xdmabuf->lock);
	}
	mutex_unlock(&xring_dma_buf_lock);
}
#else
static void xring_dump_all(void) {}
#endif

static struct scatterlist *clone_sgl(struct scatterlist *sgl, int nents)
{
	struct sg_table sgt;
	struct scatterlist *tmp = NULL, *dst = NULL;
	int i = 0;

	if (sg_alloc_table(&sgt, nents, GFP_KERNEL))
		return NULL;

	dst = sgt.sgl;
	for_each_sg(sgl, tmp, nents, i) {
		*dst = *tmp;
		dst = sg_next(dst);
	}

	return sgt.sgl;
}

int xring_dma_map_sgtable_lazy(struct device *dev, struct sg_table *sgt,
			enum dma_data_direction dir, unsigned long attrs,
			struct dma_buf *dma_buf)
{
	struct xring_dma_buf *xdmabuf = NULL;
	struct xring_map *xmap = NULL;
	struct iommu_domain *domain = NULL;
	struct scatterlist *sgl = NULL, *tmp = NULL;
	bool new_xdmabuf = false;
	int nents, i = 0;
	int ret;

	if (IS_ERR_OR_NULL(dev)) {
		xrheap_err("dev pointer is invalid\n");
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(sgt)) {
		xrheap_err("sg table pointer is invalid\n");
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(dma_buf)) {
		xrheap_err("dma_buf pointer is invalid\n");
		return -EINVAL;
	}

	sgl = sgt->sgl;
	nents = sgt->orig_nents;

	/* if dev has no domain, treat it as a normal map case */
	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		ret = dma_map_sg_attrs(dev, sgl, nents, dir, attrs);
		if (ret < 0) {
			xrheap_err("dma_map_sg_attrs fail\n");
			return -1;
		}

		return 0;
	}

	mutex_lock(&xring_dma_buf_lock);
	xdmabuf = xring_dma_buf_lookup(dma_buf);
	if (!xdmabuf) {
		xdmabuf = xring_dma_buf_create(dma_buf);
		if (!xdmabuf) {
			xrheap_err("xring_dma_buf_create fail\n");
			mutex_unlock(&xring_dma_buf_lock);
			return -1;
		}
		new_xdmabuf = true;
	} else {
		kref_get(&xdmabuf->ref);
	}
	mutex_unlock(&xring_dma_buf_lock);

	mutex_lock(&xdmabuf->lock);
	xmap = xring_map_lookup(xdmabuf, domain);
	if (!xmap) {
		xrheap_debug("not find! create map info\n");

		ret = dma_map_sg_attrs(dev, sgl, nents, dir, attrs);
		if (ret < 0) {
			xrheap_err("dma_map_sg_attrs fail\n");
			goto fail_free_xdmabuf;
		}

		xmap = xring_map_create(xdmabuf, domain);
		if (!xmap) {
			xrheap_err("xring_map_create fail\n");
			goto fail_unmap;
		}

		xmap->sgl = clone_sgl(sgl, nents);
		if (!xmap->sgl) {
			xrheap_err("clone_sgl fail\n");
			goto fail_free_xmap;
		}
		xmap->nents = nents;
		xmap->dev = dev;
		xmap->dir = dir;
		xmap->attrs = attrs;
		mutex_unlock(&xdmabuf->lock);

		/* for debug */
		xring_dump_all();
		return 0;

fail_free_xmap:
		xring_map_destroy(xmap);
fail_unmap:
		dma_unmap_sg_attrs(dev, sgl, nents, dir, attrs);
fail_free_xdmabuf:
		mutex_unlock(&xdmabuf->lock);

		if (new_xdmabuf) {
			mutex_lock(&xring_dma_buf_lock);
			xring_dma_buf_destroy(xdmabuf);
			mutex_unlock(&xring_dma_buf_lock);
		}

		return -1;
	}

	/* find xmap which has the same dma-buf in domain */

	if ((dir != xmap->dir)
		|| (attrs != xmap->attrs)
		|| (nents != xmap->nents)
		|| (sg_phys(sgl) != sg_phys(xmap->sgl))) {
		xrheap_err("not match(current - origin)\n"
				"dir: 0x%x - 0x%x\n"
				"attrs: 0x%x - 0x%x\n"
				"nents: %d - %d\n",
				dir, xmap->dir,
				attrs, xmap->attrs,
				nents, xmap->nents);
		mutex_unlock(&xdmabuf->lock);
		return -1;
	}

	xrheap_info("find! reuse map info of dma_buf: 0x%x, domain: 0x%x\n",
						dma_buf, xmap->domain);

	kref_get(&xmap->ref);
	for_each_sg(xmap->sgl, tmp, nents, i) {
		sg_dma_address(sgl) = sg_dma_address(tmp);
		sg_dma_len(sgl) = sg_dma_len(tmp);
		sgl = sg_next(sgl);
	}

	if (!(attrs & DMA_ATTR_SKIP_CPU_SYNC))
		dma_sync_sg_for_device(dev, xmap->sgl, xmap->nents, dir);

	if (dev_is_dma_coherent(dev))
		dmb(ish);

	mutex_unlock(&xdmabuf->lock);

	return 0;
}
EXPORT_SYMBOL(xring_dma_map_sgtable_lazy);

int xring_dma_unmap_sgtable_lazy(struct device *dev, struct sg_table *sgt,
			enum dma_data_direction dir, unsigned long attrs,
			struct dma_buf *dma_buf)
{
	struct xring_dma_buf *xdmabuf = NULL;
	struct xring_map *xmap = NULL;
	struct iommu_domain *domain = NULL;
	struct scatterlist *sgl = NULL;
	int nents;

	if (IS_ERR_OR_NULL(dev)) {
		xrheap_err("dev pointer is invalid\n");
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(sgt)) {
		xrheap_err("sg table pointer is invalid\n");
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(dma_buf)) {
		xrheap_err("dma_buf pointer is invalid\n");
		return -EINVAL;
	}

	sgl = sgt->sgl;
	nents = sgt->orig_nents;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dma_unmap_sg_attrs(dev, sgl, nents, dir, attrs);
		return 0;
	}

	mutex_lock(&xring_dma_buf_lock);
	xdmabuf = xring_dma_buf_lookup(dma_buf);
	if (!xdmabuf) {
		xrheap_err("dma_buf: 0x%x not mapped\n", dma_buf);
		mutex_unlock(&xring_dma_buf_lock);
		return -1;
	}
	mutex_unlock(&xring_dma_buf_lock);

	mutex_lock(&xdmabuf->lock);
	xmap = xring_map_lookup(xdmabuf, domain);
	if (!xmap) {
		xrheap_err("domain: 0x%x not mapped\n", domain);
		mutex_unlock(&xdmabuf->lock);
		return -1;
	}

	if ((dir != xmap->dir) || (attrs != xmap->attrs)
		|| (nents != xmap->nents)
		|| (sg_phys(sgl) != sg_phys(xmap->sgl))) {
		xrheap_err("not match(current - origin)\n"
				"dir: 0x%x - 0x%x\n"
				"attrs: 0x%x - 0x%x\n"
				"nents: %d - %d\n",
				dir, xmap->dir,
				attrs, xmap->attrs,
				nents, xmap->nents);

		mutex_unlock(&xdmabuf->lock);
		return -1;
	}

	if (!(attrs & DMA_ATTR_SKIP_CPU_SYNC))
		dma_sync_sg_for_cpu(dev, xmap->sgl, xmap->nents, dir);

	kref_put(&xmap->ref, xring_map_release);
	mutex_unlock(&xdmabuf->lock);

	mutex_lock(&xring_dma_buf_lock);
	kref_put(&xdmabuf->ref, xring_dma_buf_release);
	mutex_unlock(&xring_dma_buf_lock);

	/* for debug */
	xring_dump_all();

	return 0;
}
EXPORT_SYMBOL(xring_dma_unmap_sgtable_lazy);

void xring_dummy_smmu_sg_table(struct device *dev, struct sg_table *table)
{
#ifdef XRING_DUMMY_SMMU
	struct iommu_domain *domain = NULL;
	struct scatterlist *sgl = NULL;
	struct device_node *np = NULL;
	int i = 0;

	/* check if device is use dummy smmu */
	np = of_parse_phandle(dev->of_node, "iommus", 0);
	if (!np) {
		xrheap_err("no iommus find for %s\n", dev_name(dev));
		return;
	}

	if (strcmp(np->name, "dummy_smmu") != 0)
		return;

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		xrheap_err("can't get domain of dummy smmu\n");
		return;
	}

	for_each_sgtable_sg(table, sgl, i) {
		xrheap_info("convert: !!! iova=0x%lx\n", sg_dma_address(sgl));
		sg_dma_address(sgl) = iommu_iova_to_phys(domain,
							sg_dma_address(sgl));
		xrheap_info("convert: !!! dma_addr=0x%lx\n", sg_dma_address(sgl));
	}
#endif
}
EXPORT_SYMBOL(xring_dummy_smmu_sg_table);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("X-Ring Dummy SMMU Driver");
MODULE_LICENSE("GPL v2");

