// SPDX-License-Identifier: GPL-2.0-only
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

#include <linux/dma-buf.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <drm/drm.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>
#include <drm/drm_prime.h>

#include "dpu_gem.h"
#include "dksm_iommu.h"
#include "dpu_log.h"
#include "dpu_format.h"

void dpu_gem_get_framebuffer_info(struct drm_framebuffer *fb,
		struct framebuffer_info *fb_info)
{
	struct dpu_gem_object *gem_obj;
	struct iommu_format_info format_info;
	bool is_afbc;

	if (!fb) {
		DPU_ERROR("invalid parameters, %pK\n", fb);
		return;
	}

	is_afbc = fb->modifier && DPU_FMT_IS_AFBC(fb->modifier);

	gem_obj = to_dpu_gem_obj(fb->obj[0]);
	if (!gem_obj->iova_dom) {
		memcpy(format_info.offsets, fb->offsets, sizeof(format_info.offsets));
		format_info.is_afbc = is_afbc;
		gem_obj->iova_dom = dksm_iommu_map_dmabuf(gem_obj->dmabuf, gem_obj->sgt,
				&format_info);
		if (!gem_obj->iova_dom)
			return;
	}

	fb_info->plane_num = gem_obj->iova_dom->plane_num;
	fb_info->iova = gem_obj->iova_dom->iova;
	fb_info->tlb_pa = gem_obj->iova_dom->tlb_pa;
	fb_info->is_afbc = is_afbc;
	memcpy(fb_info->iova_offsets, gem_obj->iova_dom->iova_offsets,
			sizeof(fb_info->iova_offsets));
	memcpy(fb_info->tlb_offsets, gem_obj->iova_dom->tlb_offsets,
			sizeof(fb_info->tlb_offsets));
}

static int dpu_gem_sg_buf_map(struct sg_table *table,
		struct vm_area_struct *vma)
{
	struct scatterlist *sg;
	unsigned long vaddr;
	int ret;
	int len;
	int i;

	vaddr = vma->vm_start;

	for_each_sgtable_sg(table, sg, i) {
		len = min(sg->length, vma->vm_end - vaddr);
		ret = remap_pfn_range(vma, vaddr, page_to_pfn(sg_page(sg)), len,
				vma->vm_page_prot);
		if (ret) {
			DPU_ERROR("failed to remap_pfn_range! ret=%d\n", ret);
			return -1;
		}

		vaddr += len;
		if (vaddr >= vma->vm_end)
			break;
	}

	return 0;
}

static void dpu_gem_sg_buf_destroy(struct sg_table *table, size_t nents)
{
	struct scatterlist *sg;
	ssize_t i;

	for_each_sg(table->sgl, sg, nents, i)
		__free_page(sg_page(sg));

	sg_free_table(table);

	kfree(table);
}

static struct sg_table *dpu_gem_sg_buf_alloc(size_t size)
{
	struct sg_table *table;
	struct scatterlist *sg;
	struct page *page;
	size_t nents;
	ssize_t i;

	nents = size / PAGE_SIZE;

	if ((size > SZ_512M) || (size == 0)) {
		DPU_ERROR("Invalid size: %zu\n", size);
		return NULL;
	}

	table = kzalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		DPU_ERROR("failed to alloc sg_table\n");
		return NULL;
	}

	if (sg_alloc_table(table, nents, GFP_KERNEL)) {
		DPU_ERROR("failed to alloc sg_table lists\n");
		kfree(table);
		return NULL;
	}

	for_each_sgtable_sg(table, sg, i) {
		page = alloc_page(GFP_KERNEL);
		if (!page)
			goto error;

		sg_set_page(sg, page, PAGE_SIZE, 0);
		sg_dma_address(sg) = page_to_phys(page);
		sg_dma_len(sg) = sg->length;
	}

	return table;

error:
	DPU_ERROR("failed to alloc pages\n");
	dpu_gem_sg_buf_destroy(table, i);
	return NULL;
}

static int dpu_gem_dumb_buffer_create(u32 size, struct dpu_gem_object *dpu_gem_obj)
{
	struct sg_table *table;
	struct iommu_format_info format_info;
	int ret;

	table = dpu_gem_sg_buf_alloc(size);
	if (!table) {
		DPU_ERROR("alloc table failed");
		return -ENOMEM;
	}

	dpu_gem_obj->iova_dom = kzalloc(sizeof(*dpu_gem_obj->iova_dom), GFP_KERNEL);
	if (!dpu_gem_obj->iova_dom) {
		DPU_ERROR("alloc iova_dom failed\n");
		ret = PTR_ERR(dpu_gem_obj->iova_dom);
		goto alloc_iova_err;
	}

	ret = dksm_iommu_create_iova_domain(size, dpu_gem_obj->iova_dom);
	if (ret) {
		DPU_ERROR("create iova domain failed, ret=%d\n", ret);
		goto create_iova_dom_err;
	}

	/* Only has raw, single plane format. */
	memset(format_info.offsets, 0, sizeof(format_info.offsets));
	format_info.is_afbc = false;
	dksm_iommu_fill_pt_entry(NULL, table, &format_info, dpu_gem_obj->iova_dom);

	dpu_gem_obj->sgt = table;
	dpu_gem_obj->is_dumb_buf = true;
	return 0;

create_iova_dom_err:
	kfree(dpu_gem_obj->iova_dom);
alloc_iova_err:
	if (table)
		dpu_gem_sg_buf_destroy(table, table->nents);
	return ret;
}

static void dpu_gem_dumb_buffer_destory(struct dpu_gem_object *dpu_gem_obj)
{
	dksm_iommu_destory_iova_domain(dpu_gem_obj->iova_dom);
	kfree(dpu_gem_obj->iova_dom);
	dpu_gem_obj->iova_dom = NULL;

	dpu_gem_sg_buf_destroy(dpu_gem_obj->sgt, dpu_gem_obj->sgt->nents);
}

void dpu_gem_free_object(struct drm_gem_object *gem_obj)
{
	struct dpu_gem_object *dpu_gem_obj;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);
	if (gem_obj->import_attach)
		drm_prime_gem_destroy(gem_obj, dpu_gem_obj->sgt);
	else if (dpu_gem_obj->is_dumb_buf)
		dpu_gem_dumb_buffer_destory(dpu_gem_obj);

	drm_gem_object_release(gem_obj);

	kfree(dpu_gem_obj);
}

void dpu_gem_print_info(struct drm_printer *p, unsigned int indent,
		const struct drm_gem_object *gem_obj)
{
	const struct dpu_gem_object *dpu_gem_obj = to_dpu_gem_obj(gem_obj);

	drm_printf_indent(p, indent, "paddr=%pad\n", &dpu_gem_obj->paddr);
	drm_printf_indent(p, indent, "vaddr=%p\n", dpu_gem_obj->vaddr);
}

static struct sg_table *dpu_gem_get_sg_table(
		struct drm_gem_object *gem_obj)
{
	struct dpu_gem_object *dpu_gem_obj;
	struct sg_table *sgt;
	int ret;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return ERR_PTR(-ENOMEM);

	ret = dma_get_sgtable(gem_obj->dev->dev, sgt, dpu_gem_obj->vaddr,
			dpu_gem_obj->paddr, gem_obj->size);
	if (ret < 0)
		goto error;

	return sgt;
error:
	kfree(sgt);
	return ERR_PTR(ret);
}

int dpu_gem_vmap(struct drm_gem_object *gem_obj,
	struct iosys_map *map)
{
	struct dpu_gem_object *dpu_gem_obj;
	int ret = 0;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);

	mutex_lock(&dpu_gem_obj->vmap_lock);
	if (dpu_gem_obj->vmap_use_count > 0) {
		iosys_map_set_vaddr(map, dpu_gem_obj->vaddr);
		goto done;
	}

	if (gem_obj->import_attach) {
		ret = dma_buf_vmap_unlocked(gem_obj->import_attach->dmabuf, map);
		if (ret)
			goto error;

		if (WARN_ON(map->is_iomem)) {
			ret = -EIO;
			goto error;
		}

		dpu_gem_obj->vaddr = map->vaddr;
	}

	dpu_gem_obj->vmap_use_count++;

error:
	DPU_ERROR("failed to do vmap, ret %d\n", ret);
	dpu_gem_obj->vmap_use_count = 0;
done:
	mutex_unlock(&dpu_gem_obj->vmap_lock);
	return ret;
}

void dpu_gem_vunmap(struct drm_gem_object *gem_obj,
	struct iosys_map *map)
{
	struct dpu_gem_object *dpu_gem_obj = to_dpu_gem_obj(gem_obj);

	mutex_lock(&dpu_gem_obj->vmap_lock);
	dpu_gem_obj->vmap_use_count--;
	if (!dpu_gem_obj->vmap_use_count && gem_obj->import_attach)
		dma_buf_vunmap_unlocked(gem_obj->import_attach->dmabuf, map);
	mutex_unlock(&dpu_gem_obj->vmap_lock);
}

static int dpu_gem_mmap(struct drm_gem_object *gem_obj,
		struct vm_area_struct *vma)
{
	struct dpu_gem_object *dpu_gem_obj;
	int ret;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);

	vma->vm_pgoff -= drm_vma_node_start(&gem_obj->vma_node);
	vm_flags_set(vma, VM_DONTEXPAND);

	if (gem_obj->import_attach) {
		/* Drop the reference drm_gem_mmap_obj() acquired.*/
		drm_gem_object_put(gem_obj);
		vma->vm_private_data = NULL;

		return dma_buf_mmap(gem_obj->dma_buf, vma, vma->vm_pgoff);
	}

	vm_flags_clear(vma, VM_PFNMAP);

	ret = dpu_gem_sg_buf_map(dpu_gem_obj->sgt, vma);
	if (ret) {
		drm_gem_vm_close(vma);
		DPU_ERROR("failed to mmap cma memory\n");
	}

	return ret;
}

static const struct vm_operations_struct dpu_gem_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static const struct drm_gem_object_funcs dpu_gem_default_funcs = {
	.free = dpu_gem_free_object,
	.print_info = dpu_gem_print_info,
	.get_sg_table = dpu_gem_get_sg_table,
	.vmap = dpu_gem_vmap,
	.vunmap = dpu_gem_vunmap,
	.mmap = dpu_gem_mmap,
	.vm_ops = &dpu_gem_vm_ops,
};

static struct drm_gem_object *__dpu_gem_obj_create(
		struct drm_device *drm_dev, size_t size)
{
	struct dpu_gem_object *dpu_gem_obj;
	struct drm_gem_object *gem_obj;

	dpu_gem_obj = kzalloc(sizeof(*dpu_gem_obj), GFP_KERNEL);
	if (!dpu_gem_obj)
		return ERR_PTR(-ENOMEM);

	gem_obj = &dpu_gem_obj->base;
	gem_obj->funcs = &dpu_gem_default_funcs;

	drm_gem_private_object_init(drm_dev, gem_obj, size);

	mutex_init(&dpu_gem_obj->vmap_lock);
	return gem_obj;
}

static struct drm_gem_object *dpu_gem_obj_create(
		struct drm_device *drm_dev, size_t size)
{
	struct drm_gem_object *gem_obj;
	struct dpu_gem_object *dpu_gem_obj;
	int ret;

	size = PAGE_ALIGN(size);
	gem_obj = __dpu_gem_obj_create(drm_dev, size);
	if (IS_ERR(gem_obj))
		return gem_obj;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);
	ret = dpu_gem_dumb_buffer_create(size, dpu_gem_obj);
	if (ret)
		goto error;

	return gem_obj;
error:
	drm_gem_object_put(&dpu_gem_obj->base);
	return NULL;
}

static struct drm_gem_object *dpu_gem_create_with_handle(
		struct drm_file *file_priv, struct drm_device *drm_dev,
		size_t size, uint32_t *handle)
{
	struct dpu_gem_object *dpu_gem_obj;
	struct drm_gem_object *gem_obj;
	int ret;

	gem_obj = dpu_gem_obj_create(drm_dev, size);
	if (IS_ERR_OR_NULL(gem_obj))
		return gem_obj;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);

	/**
	 * allocate a id of idr table where the obj is registered
	 * and handle has the id what user can see.
	 */
	ret = drm_gem_handle_create(file_priv, gem_obj, handle);
	/* drop reference from allocate - handle holds it now. */
	drm_gem_object_put(gem_obj);
	if (ret)
		return ERR_PTR(ret);

	return gem_obj;
}

int dpu_gem_dumb_create(struct drm_file *file_priv,
		struct drm_device *drm_dev,
		struct drm_mode_create_dumb *args)
{
	struct drm_gem_object *gem_obj;

	args->pitch = DIV_ROUND_UP(args->width * args->bpp, 8);
	/* hardware limitation: pitch must be 16-byte aligned */
	args->pitch = ALIGN(args->pitch, 4);
	args->size = args->pitch * args->height;

	gem_obj = dpu_gem_create_with_handle(file_priv, drm_dev,
			args->size, &args->handle);

	return PTR_ERR_OR_ZERO(gem_obj);
}

struct drm_gem_object *
dpu_gem_prime_import_sg_table(struct drm_device *drm_dev,
		struct dma_buf_attachment *attach,
		struct sg_table *sgt)
{
	struct drm_gem_object *gem_obj;
	struct dpu_gem_object *dpu_gem_obj;

	gem_obj = __dpu_gem_obj_create(drm_dev, attach->dmabuf->size);
	if (IS_ERR(gem_obj))
		return gem_obj;

	dpu_gem_obj = to_dpu_gem_obj(gem_obj);
	dpu_gem_obj->sgt = sgt;
	dpu_gem_obj->dmabuf = attach->dmabuf;
	dpu_gem_obj->paddr = sg_dma_address(sgt->sgl);

	return gem_obj;
}

static const struct drm_framebuffer_funcs dpu_drm_gem_fb_funcs = {
	.destroy        = drm_gem_fb_destroy,
	.create_handle  = drm_gem_fb_create_handle,
};

struct drm_framebuffer *
dpu_drm_gem_fb_create(struct drm_device *dev, struct drm_file *file,
		const struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct drm_framebuffer *fb;
	struct dpu_gem_object *obj;
	struct iommu_format_info format_info;

	fb = drm_gem_fb_create_with_funcs(dev, file, mode_cmd,
			&dpu_drm_gem_fb_funcs);
	if (IS_ERR_OR_NULL(fb)) {
		DPU_ERROR("fb is null\n");
		return ERR_PTR(-ENOMEM);
	}

	if (fb->obj[0] == NULL) {
		DPU_ERROR("obj is null\n");
		return ERR_PTR(-ENOMEM);
	}

	obj = to_dpu_gem_obj(fb->obj[0]);
	if (!obj->iova_dom) {
		memcpy(format_info.offsets, fb->offsets, sizeof(format_info.offsets));
		format_info.is_afbc = fb->modifier && DPU_FMT_IS_AFBC(fb->modifier);
		obj->iova_dom = dksm_iommu_map_dmabuf(obj->dmabuf, obj->sgt, &format_info);
	}
	return fb;
}

MODULE_IMPORT_NS(DMA_BUF);
