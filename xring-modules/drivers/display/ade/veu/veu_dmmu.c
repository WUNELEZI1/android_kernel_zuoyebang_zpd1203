// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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
#include <linux/genalloc.h>
#include <linux/mutex.h>
#include <linux/bitmap.h>
#include <linux/version.h>

#include <soc/xring/xr_dmabuf_helper.h>

#include "veu_dmmu.h"
#include "veu_utils.h"
#include "veu_defs.h"
#include "veu_enum.h"
#include "veu_dmmu_ops.h"
#include "veu_drv.h"

#define SIZE_16_BYTES 16
#define TLB_TOTAL_SIZE 0x400000
#define BUS_INTERLEAVE_FLAG_BIT (1 << 26)

static struct veu_mmu_mgr_ctx *g_mmu_mgr_ctx;

/**
 * afbc scramble only works on payload.
 *
 * PAGE_SHIFT + BUS_INTERLEAVE_FLAG_BIT = 38
 * mmu Page table 38bit set to 1 to enable afbc scramble
 */
#define veu_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, \
		afbc_header_page_count, is_addr_inc) \
do { \
	if (!is_scramble_en || afbc_header_page_count > 0) { \
		*tlb_vaddr++ = addr; \
		if (afbc_header_page_count > 0) \
			afbc_header_page_count--; \
	} else { \
		*tlb_vaddr++ = addr | BUS_INTERLEAVE_FLAG_BIT; \
	} \
	addr += is_addr_inc; \
} while (0)

static void veu_mmu_fill_pt_entry(struct veu_mmu_object *obj)
{
	int64_t afbc_header_page_count = 0;
	struct sg_table *sgt = obj->sgt;
	struct scatterlist *sg;
	bool is_scramble_en = false;
	uint64_t last_valid_entry;
	u32 page_num_offset[IOMMU_PLANE_MAX + 1] = {0};
	u32 page_num_i = 0;
	u8 plane_id = 0;
	u32 *tlb_vaddr;
	dma_addr_t addr;
	u32 length;
	size_t i;

	if (obj->is_afbc) {
		afbc_header_page_count = xr_dmabuf_get_headersize(obj->buf);
		if (afbc_header_page_count > 0) {
			is_scramble_en = true;
		} else {
			is_scramble_en = false;
			if (afbc_header_page_count < 0)
				VEU_ERR("get afbc header error!");
		}
	}

	obj->plane_num = 1;
	for (i = 1; i < IOMMU_PLANE_MAX && obj->iova_offsets[i]; i++) {
		obj->plane_num++;
		page_num_offset[i] = obj->iova_offsets[i] >> PAGE_SHIFT;
	}
	if (obj->is_afbc) {
		/**
		 * afbc format plane_num set 2, uesd in tbu hw reg config.
		 * afbc need map on one plane, so page_num_offset[1] set 0.
		 */
		obj->plane_num = 2;
		page_num_offset[1] = 0;
	}

	VEU_DBG("iommu info: plane_num = %d, is_scramble_en = %d\n",
			obj->plane_num, is_scramble_en);
	for (i = 0; i < IOMMU_PLANE_MAX; i++)
		VEU_DBG("\toffset[%zu]=0x%x, page_num_offset[%zu]=0x%x\n",
				i, obj->iova_offsets[i], i, page_num_offset[i]);

	tlb_vaddr = obj->tlb_vaddr;

	for_each_sgtable_sg(sgt, sg, i) {
		length = sg->length >> PAGE_SHIFT;
		addr = sg_phys(sg) >> PAGE_SHIFT;
		do {
			if (page_num_offset[plane_id + 1] == 0  || page_num_i < page_num_offset[plane_id + 1]) {
				veu_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, afbc_header_page_count, 1);
			} else {
				if (obj->iova_offsets[plane_id + 1] & ((1 << PAGE_SHIFT) - 1))
					veu_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, afbc_header_page_count, 0);
				last_valid_entry = *(tlb_vaddr - 1);
				while ((unsigned long)tlb_vaddr & 0xf)
					*tlb_vaddr++ = last_valid_entry;
				plane_id++;
				obj->tlb_offsets[plane_id] = (unsigned long)tlb_vaddr - (unsigned long)obj->tlb_vaddr;
				veu_iommu_write_tlb_entry(tlb_vaddr, addr, is_scramble_en, afbc_header_page_count, 1);
			}
			page_num_i++;
		} while (--length);
	}

	/**
	 * Due to the tlb alignment requirement, fill more mmu entries to
	 * avoid ddrc filter error. We simply use the last valid entry.
	 */
	last_valid_entry = *(tlb_vaddr - 1);
	for (i = 0; i < EXTRA_ENTRY; i++)
		*tlb_vaddr++ = last_valid_entry;
}

static u32 veu_mmu_iova_bitmap_start_get(struct veu_mmu_mgr_ctx *mmu_ctx,
		u64 size)
{
	u32 start = 0, block_num = 0;
	u64 align_size = 0;

	align_size = roundup(size, IOVA_ONE_BLOCK_SIZE);
	block_num = align_size / IOVA_ONE_BLOCK_SIZE;

	mutex_lock(&mmu_ctx->lock);
	start = bitmap_find_next_zero_area(mmu_ctx->iova_pool_bitmap,
			IOVA_MAX_BLOCK_NUM, 0, block_num, 0);

	if (start <= IOVA_MAX_BLOCK_NUM)
		bitmap_set(mmu_ctx->iova_pool_bitmap, start, block_num);
	mutex_unlock(&mmu_ctx->lock);

	return start;
}

static u64 veu_mmu_iova_get(struct veu_mmu_mgr_ctx *mmu_ctx, u64 size)
{
	/* bit index means block index in iova space */
	return IOVA_BASE + veu_mmu_iova_bitmap_start_get(mmu_ctx, size) *
			IOVA_ONE_BLOCK_SIZE;
}

int veu_mmu_map_layer_buffer(struct veu_mmu_object *obj)
{
	struct veu_mmu_mgr_ctx *mmu_ctx;
	u32 tlb_buf_size;

	if (!obj) {
		VEU_ERR("invalid mmu object, %pK", obj);
		return -EINVAL;
	}

	mmu_ctx = g_mmu_mgr_ctx;
	tlb_buf_size = ((obj->buf_size >> PAGE_SHIFT) + EXTRA_ENTRY) * 4;
	mutex_lock(&mmu_ctx->lock);
	obj->tlb_vaddr = gen_pool_dma_alloc(mmu_ctx->pt_pool,
			tlb_buf_size, &obj->tlb_pa);
	mutex_unlock(&mmu_ctx->lock);
	if (!obj->tlb_vaddr) {
		VEU_ERR("fatal error, failed to alloc tlb buf");
		return -EINVAL;
	}

	obj->iova = veu_mmu_iova_get(mmu_ctx, obj->buf_size);
	veu_mmu_fill_pt_entry(obj);

	VEU_DBG("tlb_vaddr 0x%pK, tlb_pa 0x%llx, iova 0x%llx",
			obj->tlb_vaddr, obj->tlb_pa, obj->iova);

	return 0;
}

void veu_mmu_unmap_layer_buffer(struct veu_mmu_object *obj)
{
	struct veu_mmu_mgr_ctx *mmu_ctx;
	u64 tlb_buf_size;
	u64 align_size;
	u64 block_num;
	u64 start;

	if (!obj) {
		VEU_ERR("invalid mmu object, %pK", obj);
		return;
	}

	align_size = roundup(obj->buf_size, IOVA_ONE_BLOCK_SIZE);
	block_num = align_size / IOVA_ONE_BLOCK_SIZE;
	start = (obj->iova - IOVA_BASE) / IOVA_ONE_BLOCK_SIZE;

	VEU_DBG("tlb_vaddr 0x%pK, tlb_pa 0x%llx, iova 0x%llx",
			obj->tlb_vaddr, obj->tlb_pa, obj->iova);

	mmu_ctx = g_mmu_mgr_ctx;
	mutex_lock(&mmu_ctx->lock);
	bitmap_clear(mmu_ctx->iova_pool_bitmap, start, block_num);
	mutex_unlock(&mmu_ctx->lock);

	tlb_buf_size = ((obj->buf_size >> PAGE_SHIFT) + EXTRA_ENTRY) * 4;
	mutex_lock(&mmu_ctx->lock);
	gen_pool_free(mmu_ctx->pt_pool, (uintptr_t)obj->tlb_vaddr, tlb_buf_size);
	mutex_unlock(&mmu_ctx->lock);

	dma_buf_unmap_attachment_unlocked(obj->attach, obj->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(obj->buf, obj->attach);
	dma_buf_put(obj->buf);

	obj->iova = 0;
}

static int veu_mmu_pt_pool_init(struct device *dev,
		struct veu_mmu_mgr_ctx *mmu_ctx, u64 pt_size)
{
	int ret = 0;

	/* reserve buffer for dmmu pt */
	mmu_ctx->pt_va = dma_alloc_coherent(dev, pt_size,
			&mmu_ctx->pt_pa, GFP_KERNEL);
	if (!mmu_ctx->pt_va) {
		VEU_ERR("failed to allocate page table memory!");
		return -ENOMEM;
	}
	mmu_ctx->pt_size = pt_size;

	VEU_DBG("pt_va is 0x%pK, pt_pa 0x%llx, pt_size 0x%llx",
			mmu_ctx->pt_va, mmu_ctx->pt_pa, mmu_ctx->pt_size);

	/* create gen pool for pt buffer */
	mmu_ctx->pt_pool = devm_gen_pool_create(dev, ilog2(SIZE_16_BYTES),
			-1, "veu_mmu_pt_pool");
	if (IS_ERR_OR_NULL(mmu_ctx->pt_pool)) {
		VEU_ERR("failed to create pt pool");
		ret = -EINVAL;
		goto err_pt_pool_create;
	}

	ret = gen_pool_add_virt(mmu_ctx->pt_pool, (unsigned long)mmu_ctx->pt_va,
			mmu_ctx->pt_pa, pt_size, -1);
	if (ret != 0) {
		VEU_ERR("failed to add virt to pt pool %d!", ret);
		goto err_tlb_pool_add;
	}

	return 0;

err_tlb_pool_add:
	gen_pool_destroy(mmu_ctx->pt_pool);

err_pt_pool_create:
	dma_free_coherent(dev, pt_size, mmu_ctx->pt_va,
			mmu_ctx->pt_pa);
	mmu_ctx->pt_va = NULL;
	mmu_ctx->pt_pa = 0;

	return ret;
}

/* clear bitmap to 0, means all iova space are idle */
static void veu_mmu_va_pool_init(struct veu_mmu_mgr_ctx *mmu_ctx)
{
	bitmap_zero(mmu_ctx->iova_pool_bitmap, IOVA_MAX_BLOCK_NUM);
}

int veu_mmu_mgr_ctx_init(struct device *dev)
{
	struct veu_mmu_mgr_ctx *mmu_ctx;
	static bool is_initialized;
	int ret;

	if (is_initialized) {
		VEU_DBG("mmu inited already");
		return 0;
	}

	VEU_DBG("enter");

	if (!dev) {
		VEU_ERR("invalid parameters, %pK", dev);
		return -EINVAL;
	}

	mmu_ctx = kzalloc(sizeof(*mmu_ctx), GFP_KERNEL);
	if (unlikely(!mmu_ctx))
		return -ENOMEM;

	mutex_init(&mmu_ctx->lock);

	/* 1G for iova size, 10M for tlb_buffer size */
	ret = veu_mmu_pt_pool_init(dev, mmu_ctx, TLB_TOTAL_SIZE);
	if (unlikely(ret != 0)) {
		VEU_ERR("failed to init pt pool");
		mutex_destroy(&mmu_ctx->lock);
		kfree(mmu_ctx);
		return ret;
	}
	mmu_ctx->dev = dev;
	veu_mmu_va_pool_init(mmu_ctx);

	g_mmu_mgr_ctx = mmu_ctx;

	is_initialized = true;

	VEU_DBG("exit");
	return 0;
}

void veu_mmu_mgr_ctx_cleanup(void)
{
	struct veu_mmu_mgr_ctx *mmu_ctx = g_mmu_mgr_ctx;

	if (!mmu_ctx) {
		VEU_ERR("not initialize for veu mmu");
		return;
	}

	mutex_destroy(&mmu_ctx->lock);

	dma_free_coherent(mmu_ctx->dev,
			mmu_ctx->pt_size, mmu_ctx->pt_va, mmu_ctx->pt_pa);

	kfree(mmu_ctx);
	g_mmu_mgr_ctx = NULL;
}

void veu_mmu_get_framebuffer_info(struct veu_data *veu_dev,
		struct buffer_info *info, enum VEU_MODULE_TYPE module_type)
{
	struct veu_mmu_object *obj = NULL;
	struct veu_layer *layer = NULL;

	if (!veu_dev) {
		VEU_ERR("invalid parameters, %pK", veu_dev);
		return;
	}

	if (!info) {
		VEU_ERR("invalid parameters, %pK", veu_dev);
		return;
	}

	if (module_type == MODULE_RDMA) {
		obj = veu_dev->rdma_obj;
		layer = &(veu_dev->rdma_param.layer);
	} else if (module_type == MODULE_WDMA) {
		obj = veu_dev->wdma_obj;
		layer = &(veu_dev->wdma_param.layer);
	} else {
		VEU_WARN("invalid module type, abort");
		return;
	}

	if (obj->iova == 0)
		return;

	info->iova = obj->iova;
	info->tlb_vaddr = obj->tlb_vaddr;
	info->tlb_pa = obj->tlb_pa;
	info->size = layer->buf_size;
	info->is_afbc = obj->is_afbc;
	memcpy(info->iova_offsets, obj->iova_offsets, sizeof(info->iova_offsets));
	memcpy(info->tlb_offsets, obj->tlb_offsets, sizeof(info->tlb_offsets));
	info->plane_num = obj->plane_num;
}

MODULE_IMPORT_NS(DMA_BUF);
