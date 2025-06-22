// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "cbm_iova", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "cbm_iova", __func__, __LINE__

#include <linux/list.h>
#include <linux/iommu.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/dma-buf.h>
#include <linux/completion.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_cbm.h"
#include "xrisp_cbm_api.h"
#include "xrisp_rproc_api.h"
#include "xrisp_tee_ca_api.h"
#include "xrisp_log.h"

#define XRISP_MPU_SET_TIMEOUT_MS (1000)

size_t sg_table_len(struct sg_table *sg_tlb)
{
	size_t size = 0;
	int i = 0;
	struct scatterlist *s = NULL;

	// XRISP_PR_DEBUG("sg nents %d orig_nents %d", sg_tlb->nents, sg_tlb->orig_nents);
	for_each_sg(sg_tlb->sgl, s, sg_tlb->orig_nents, i) {
		// XRISP_PR_DEBUG("sg %d, length : %d", i, s->length);
		// XRISP_PR_DEBUG("dma_address %llx, dma_length : %d", s->dma_address, s->dma_length);
		size += s->length;
	}
	return size;
}

inline size_t cbm_dynamic_region_base(uint32_t idx)
{
	if (idx < CAM_SHM_DYNAMIC_REGION_64MB_IDX_START)
		return CBM_SHM_IOVA_BASE +
			CAM_SHM_DYNAMIC_REGION_64MB_SIZE * CAM_SHM_DYNAMIC_REGION_64MB_NUM
			+ CAM_SHM_DYNAMIC_REGION_128MB_SIZE * CAM_SHM_DYNAMIC_REGION_128MB_NUM;
	else if (idx < CAM_SHM_DYNAMIC_REGION_128MB_IDX_START)
		return CBM_SHM_IOVA_BASE + CAM_SHM_DYNAMIC_REGION_64MB_SIZE *
			(idx - CAM_SHM_DYNAMIC_REGION_64MB_IDX_START);
	else
		return CBM_SHM_IOVA_BASE +
			CAM_SHM_DYNAMIC_REGION_64MB_SIZE * CAM_SHM_DYNAMIC_REGION_64MB_NUM
			+ CAM_SHM_DYNAMIC_REGION_128MB_SIZE *
			(idx - CAM_SHM_DYNAMIC_REGION_128MB_IDX_START);
}

inline size_t cbm_dynamic_region_size(uint32_t idx)
{
	if (idx < CAM_SHM_DYNAMIC_REGION_64MB_IDX_START)
		return CBM_SHM_IOVA_SIZE -
			CAM_SHM_DYNAMIC_REGION_64MB_SIZE * CAM_SHM_DYNAMIC_REGION_64MB_NUM
			- CAM_SHM_DYNAMIC_REGION_128MB_SIZE * CAM_SHM_DYNAMIC_REGION_128MB_NUM;
	else if (idx < CAM_SHM_DYNAMIC_REGION_128MB_IDX_START)
		return CAM_SHM_DYNAMIC_REGION_64MB_SIZE;
	else
		return CAM_SHM_DYNAMIC_REGION_128MB_SIZE;
}

static void iova_region_init(struct cam_share_buf_region *region, int region_id)
{
	if (region_id < 0 || region == NULL) {
		XRISP_PR_ERROR("invalid region id %d", region_id);
		return;
	}
	region->region_id = region_id;
	region->addr_base = cbm_dynamic_region_base(region_id);
	region->max_size = cbm_dynamic_region_size(region_id);
	region->cur_size = 0;
	INIT_LIST_HEAD(&region->buf_elem);
	mutex_init(&region->lock);

	XRISP_PR_DEBUG("region %d, base 0x%llx, size 0x%zx", region->region_id, region->addr_base, region->max_size);
}

static void iova_region_deinit(struct cam_share_buf_region *region)
{
	region->cur_size = 0;
}

static void cbm_set_mpu_region_cb(void *priv, void *msg)
{
	struct cam_buf_manager *cam_buf_mgr = priv;
	struct xrisp_set_mpu_region *mpu_region = msg;

	if (mpu_region->msg == XRISP_RPROC_SET_MUP_REGION_DONE)
		complete(&cam_buf_mgr->set_region_complet);
}

static int cbm_set_mpu_region(struct cam_share_buf_region *region)
{
	int ret = 0;
	struct xrisp_set_mpu_region mpu_region;

	mpu_region.msg = XRISP_RPROC_SET_MUP_REGION;
	mpu_region.region_id = region->region_id;
	mpu_region.base = region->addr_base;
	mpu_region.size = region->cur_size;

	xrisp_rproc_mpu_lock();
	if (!xrisp_rproc_mpu_avail()) {
		xrisp_rproc_mpu_unlock();
		return 0;
	}

	reinit_completion(&g_cam_buf_mgr.set_region_complet);

	ret = xrisp_rproc_mbox_send_sync((unsigned int *)&mpu_region, sizeof(mpu_region));
	if (ret) {
		XRISP_PR_ERROR(
			"send set mpu region fail ret = %d, mpu region %d, base 0x%llx, size 0x%x",
			ret, mpu_region.region_id, mpu_region.base, mpu_region.size);
		xrisp_rproc_mpu_unlock();
		return ret;
	}

	ret = wait_for_completion_timeout(&g_cam_buf_mgr.set_region_complet,
					  msecs_to_jiffies(XRISP_MPU_SET_TIMEOUT_MS));
	if (ret) {
		XRISP_PR_DEBUG("set mpu region %d, base 0x%llx, size 0x%x, completion use %d ms",
			       mpu_region.region_id, mpu_region.base, mpu_region.size,
			       XRISP_MPU_SET_TIMEOUT_MS - jiffies_to_msecs(ret));
		xrisp_rproc_mpu_unlock();
		return 0;
	}

	xrisp_rproc_mpu_unlock();
	XRISP_PR_ERROR("set mpu region %d ,base 0x%llx, size 0x%x, timeout", region->region_id,
		       mpu_region.base, mpu_region.size);
	return -ETIMEDOUT;
}

int cbm_mpu_region_start(void *priv)
{
	int ret = 0;
	int i = 0;
	struct cam_share_buf_region *region;

	mutex_lock(&g_cam_buf_mgr.lock_region);
	for (i = CAM_SHM_STATIC_REGION_IDX_START; i < CAM_SHM_REGION_NUM; i++) {
		if (!test_bit(i, g_cam_buf_mgr.bitmap_region))
			continue;
		region = &g_cam_buf_mgr.sh_buf_region[i];
		if (region->cur_size == 0)
			continue;
		ret = cbm_set_mpu_region(region);
		if (ret)
			break;
	}
	mutex_unlock(&g_cam_buf_mgr.lock_region);

	return ret;
}

void cbm_mpu_region_stop(void *priv, bool crashed)
{

}

int cbm_iova_region_init(void)
{
	int ret = 0;
	int i = 0;

	g_cam_buf_mgr.bitmap_region = devm_bitmap_zalloc(g_cam_buf_mgr.dev,
						  CAM_SHM_REGION_NUM,
						  __GFP_ZERO);
	if (!g_cam_buf_mgr.bitmap_region) {
		XRISP_PR_ERROR("iova region bitmap alloc fail");
		return -ENOMEM;
	}
	mutex_init(&g_cam_buf_mgr.lock_region);
	mutex_lock(&g_cam_buf_mgr.lock_region);
	g_cam_buf_mgr.region_num = BITS_TO_LONGS(CAM_SHM_REGION_NUM) *
			  sizeof(unsigned long) * BITS_PER_BYTE;
	XRISP_PR_INFO("region num %d, bits %zd", CAM_SHM_REGION_NUM, g_cam_buf_mgr.region_num);
	bitmap_zero(g_cam_buf_mgr.bitmap_region, g_cam_buf_mgr.region_num);
	set_bit(0, g_cam_buf_mgr.bitmap_region); /* reserve bit 0 */
	set_bit(CAM_SHM_STATIC_REGION_IDX_START, g_cam_buf_mgr.bitmap_region);
	for (i = CAM_SHM_STATIC_REGION_IDX_START; i < CAM_SHM_REGION_NUM; i++)
		iova_region_init(&g_cam_buf_mgr.sh_buf_region[i], i);

	init_completion(&g_cam_buf_mgr.set_region_complet);
	mutex_unlock(&g_cam_buf_mgr.lock_region);

	xrisp_rproc_msg_register(XRISP_RPROC_SET_MUP_REGION_DONE,
				 cbm_set_mpu_region_cb, &g_cam_buf_mgr);
	xrisp_register_rproc_mpu_ops(&g_cam_buf_mgr,
				      cbm_mpu_region_start,
				      cbm_mpu_region_stop);
	return ret;
}

void cbm_iova_region_exit(void)
{
	mutex_destroy(&g_cam_buf_mgr.lock_region);
	xrisp_rproc_msg_unregister(XRISP_RPROC_SET_MUP_REGION_DONE);
	xrisp_unregister_rproc_mpu_ops();
}

int cbm_iova_region_alloc(enum xrisp_cam_buf_region_type type)
{
	int ret = -EINVAL;
	uint32_t start_bit, end_bit;

	if (type == XRISP_CAM_SHM_REGION_64MB) {
		start_bit = CAM_SHM_DYNAMIC_REGION_64MB_IDX_START;
		end_bit = CAM_SHM_DYNAMIC_REGION_128MB_IDX_START;
	} else if (type == XRISP_CAM_SHM_REGION_128MB) {
		start_bit = CAM_SHM_DYNAMIC_REGION_128MB_IDX_START;
		end_bit = CAM_SHM_REGION_NUM;
	} else {
		XRISP_PR_ERROR("unkonwn region type");
		return -EINVAL;
	}

	mutex_lock(&g_cam_buf_mgr.lock_region);
	ret = find_next_zero_bit(g_cam_buf_mgr.bitmap_region,
				     end_bit, start_bit);

	if (ret < start_bit || ret >= end_bit) {
		XRISP_PR_ERROR("no available region %d for type %d", ret, type);
		ret = -EBADSLT;
		goto unlock;
	}

	XRISP_PR_DEBUG("alloc region : %d", ret);
	set_bit(ret, g_cam_buf_mgr.bitmap_region);
	iova_region_init(&g_cam_buf_mgr.sh_buf_region[ret], ret);

unlock:
	mutex_unlock(&g_cam_buf_mgr.lock_region);
	return ret;
}

int cbm_iova_region_free(uint32_t region_id)
{
	int ret = 0;
	struct cam_share_buf_region *region;
	struct cam_buf_element *buf = NULL, *tmp = NULL;
	struct cam_buf_desc desc;

	if (region_id < CAM_SHM_STATIC_REGION_IDX_START || region_id >= CAM_SHM_REGION_NUM) {
		XRISP_PR_ERROR("invaild region %d", region_id);
		return -EINVAL;
	}

	mutex_lock(&g_cam_buf_mgr.lock_region);
	if (!test_bit(region_id, g_cam_buf_mgr.bitmap_region)) {
		XRISP_PR_WARN("region %d has free", region_id);
		goto out;
	}
	region = &g_cam_buf_mgr.sh_buf_region[region_id];

	cbm_iova_region_lock(region);
	if (region->cur_size > 0) {
		region->cur_size = 0;
		ret = cbm_set_mpu_region(region);
		if (ret)
			goto iova_region_unlock;
	}

	list_for_each_entry_safe(buf, tmp, &region->buf_elem, region_head) {
		cbm_buf_del_form_region(buf);
		desc.buf_handle = buf->buf_handle;
		if (cbm_buf_put(&desc))
			XRISP_PR_ERROR("release buffer handle 0x%016llx fail", desc.buf_handle);
		else
			XRISP_PR_DEBUG("release buffer handle 0x%016llx", desc.buf_handle);
	}

	cbm_iova_region_unlock(region);
	iova_region_deinit(region);
	if (region_id != CAM_SHM_STATIC_REGION_IDX_START)
		clear_bit(region_id, g_cam_buf_mgr.bitmap_region);
	mutex_unlock(&g_cam_buf_mgr.lock_region);
	return 0;

iova_region_unlock:
	cbm_iova_region_unlock(region);
out:
	mutex_unlock(&g_cam_buf_mgr.lock_region);
	return ret;
}

struct cam_share_buf_region *cbm_get_iova_region(uint32_t region_id)
{	mutex_lock(&g_cam_buf_mgr.lock_region);
	if (region_id >= CAM_SHM_REGION_NUM || region_id == 0 ||
	    !test_bit(region_id, g_cam_buf_mgr.bitmap_region)) {
		XRISP_PR_ERROR("fail get region %d", region_id);
		mutex_unlock(&g_cam_buf_mgr.lock_region);
		return NULL;
	}
	mutex_unlock(&g_cam_buf_mgr.lock_region);
	return &g_cam_buf_mgr.sh_buf_region[region_id];
}

void cbm_buf_add_to_region(struct cam_share_buf_region *region, struct cam_buf_element *buf)
{
	if (WARN_ON(!region || !buf))
		return;
	list_add(&buf->region_head, &region->buf_elem);
}

void cbm_buf_del_form_region(struct cam_buf_element *buf)
{
	if (WARN_ON(!buf))
		return;
	list_del(&buf->region_head);
	buf->region_id = 0;
}

void cbm_iova_region_lock(struct cam_share_buf_region *region)
{
	mutex_lock(&region->lock);
}

void cbm_iova_region_unlock(struct cam_share_buf_region *region)
{
	mutex_unlock(&region->lock);
}

// sec-mode: nsbuf ssmmu map
static int cbm_buf_ssmmu_iovmap(struct cam_buf_iomap *iomap)
{
	int ret;

	if (iomap->dynamic_ssmmu_map)
		ret = xrisp_ns_dmabuf_ssmmu_map(iomap->sgl, &iomap->iovaddr,
						TA_SMMU_MMAP_TYPE_FREE_IOVA);
	else
		ret = xrisp_ns_dmabuf_ssmmu_map(iomap->sgl, &iomap->iovaddr,
						TA_SMMU_MMAP_TYPE_FIX_IOVA);
	if (ret) {
		XRISP_PR_ERROR("nsbuf ssmmu %s map failed, iova=0x%llx, size=%zu, sg_num=%d",
			       (iomap->dynamic_ssmmu_map ? "dynamic" : "fix"), iomap->iovaddr,
			       iomap->size, iomap->sgl->nents);
		return -EINVAL;
	}
	iomap->map_type = CAMBUF_SSMMU_MAPED;
	return 0;
}

// sec-mode: nsbuf ssmmu unmap
static void cbm_buf_ssmmu_iovunmap(struct cam_buf_iomap *iomap)
{
	int ret = 0;

	if (iomap->dynamic_ssmmu_map)
		ret = xrisp_ns_dmabuf_ssmmu_unmap(iomap->sgl, &iomap->iovaddr,
						  TA_SMMU_MMAP_TYPE_FREE_IOVA);
	else
		ret = xrisp_ns_dmabuf_ssmmu_unmap(iomap->sgl, &iomap->iovaddr,
						  TA_SMMU_MMAP_TYPE_FIX_IOVA);

	if (ret)
		XRISP_PR_ERROR("nsbuf ssmmu unmap failed, iova=0x%llx", iomap->iovaddr);

	iomap->map_type = BUF_NOT_SSMMU_MAPED;
}

int cbm_buf_iovmap(struct cam_buf_iomap *iomap)
{
	int ret = 0;
	struct cam_share_buf_region *region;


	if (!iomap)
		return -EINVAL;

	region = iomap->region;

	iomap->attach = dma_buf_attach(iomap->buf, g_cam_buf_mgr.dev);
	if (IS_ERR_OR_NULL(iomap->attach)) {
		ret = PTR_ERR(iomap->attach);
		XRISP_PR_ERROR("dma buf attach failed %d", ret);
		goto exit;
	}
	iomap->sgl = dma_buf_map_attachment(iomap->attach, iomap->direction);
	if (IS_ERR_OR_NULL(iomap->sgl)) {
		ret = PTR_ERR(iomap->sgl);
		XRISP_PR_ERROR("dma buf attachment map failed %d", ret);
		goto dma_detach;
	}

	iomap->iovaddr = sg_dma_address(iomap->sgl->sgl);
	iomap->size = sg_table_len(iomap->sgl);

	if (region) {
		if (iomap->size > region->max_size - region->cur_size) {
			XRISP_PR_ERROR("region %d overflow, maxsize 0x%lx, cur_size 0x%lx, map size 0x%lx",
				region->region_id, region->max_size, region->cur_size, iomap->size);
			ret = -ENOMEM;
			goto unmap_attachment;
		}

		iomap->iommuaddr = region->addr_base + region->cur_size;
		if (xrisp_rproc_is_sec_running()) {
			iomap->iovaddr = iomap->iommuaddr;
			iomap->dynamic_ssmmu_map = false;
			ret = cbm_buf_ssmmu_iovmap(iomap);
			if (ret)
				goto unmap_attachment;
			iomap->iommu_size = iomap->size;
		} else
			iomap->iommu_size = iommu_map_sgtable(g_cam_buf_mgr.i_domain, iomap->iommuaddr,
				iomap->sgl, iomap->iommu_port);

		region->cur_size += iomap->iommu_size;
		if (iomap->iommu_size <= 0) {
			XRISP_PR_ERROR("iommu map fail ret = %ld", iomap->iommu_size);
			ret = -ENOMEM;
			goto unmap_attachment;
		}
		iomap->iovaddr = iomap->iommuaddr;

		mutex_lock(&g_cam_buf_mgr.lock_region);
		ret = cbm_set_mpu_region(region);
		mutex_unlock(&g_cam_buf_mgr.lock_region);
		if (ret)
			goto iommu_unmap;
	} else {
		if (xrisp_rproc_is_sec_running()) {
			iomap->dynamic_ssmmu_map = true;
			ret = cbm_buf_ssmmu_iovmap(iomap);
			if (ret)
				goto unmap_attachment;
		}
	}

	ret = dma_buf_begin_cpu_access(iomap->buf, iomap->direction);
	if (ret) {
		XRISP_PR_ERROR("dma buf begin cpu access fail ret = %d", ret);
		goto iommu_unmap;
	}

	return 0;
iommu_unmap:
	if (region) {
		region->cur_size -= iomap->iommu_size;
		if (iomap->map_type == CAMBUF_SSMMU_MAPED)
			cbm_buf_ssmmu_iovunmap(iomap);
		else
			iommu_unmap(g_cam_buf_mgr.i_domain, iomap->iommuaddr,
				    iomap->iommu_size);
	}
unmap_attachment:
	dma_buf_unmap_attachment(iomap->attach, iomap->sgl, iomap->direction);
dma_detach:
	dma_buf_detach(iomap->buf, iomap->attach);
exit:
	return ret;
}

void cbm_buf_iovunmap(struct cam_buf_iomap *iomap)
{
	if (WARN_ON(!iomap || !iomap->attach || !iomap->buf || !iomap->sgl))
		return;

	if (iomap->region) {
		if (iomap->map_type == BUF_NOT_SSMMU_MAPED)
			iommu_unmap(g_cam_buf_mgr.i_domain, iomap->iommuaddr,
				    iomap->iommu_size);
	}
	if (iomap->map_type == CAMBUF_SSMMU_MAPED)
		cbm_buf_ssmmu_iovunmap(iomap);

	dma_buf_unmap_attachment(iomap->attach, iomap->sgl, iomap->direction);
	dma_buf_detach(iomap->buf, iomap->attach);
}

int sec_buf_iovmap(struct cam_buf_iomap *iomap, uint32_t sfd, uint32_t flags)
{
	if (!iomap || !sfd || !flags) {
		XRISP_PR_ERROR("invalid input");
		return -EINVAL;
	}

	if (iomap->size == 0)
		iomap->size = iomap->buf->size;

	if (xrisp_secbuf_map(sfd, iomap->size, flags, &iomap->iovaddr)) {
		XRISP_PR_ERROR("secbuf ssmmu map failed, sfd=%d", sfd);
		return -EINVAL;
	}

	iomap->map_type = SECBUF_SSMMU_MAPED;
	return 0;
}

void sec_buf_iovunmap(struct cam_buf_iomap *iomap, uint32_t sfd)
{
	xrisp_secbuf_unmap(sfd, iomap->size);
	iomap->map_type = BUF_NOT_SSMMU_MAPED;
}

int xrisp_provider_buf_ssmmu_map_all(atomic_t *stop)
{
	uint32_t i = 0;
	int ret = 0;

	if (!xrisp_rproc_is_sec_running() || !stop)
		return -EINVAL;

	mutex_lock(&g_cam_buf_mgr.lock_tbl);

	for_each_set_bit(i, g_cam_buf_mgr.bitmap, g_cam_buf_mgr.bits) {
		if (g_cam_buf_mgr.buf_tbl[i].region_id == 1 &&
		    g_cam_buf_mgr.buf_tbl[i].iomap.map_type == BUF_NOT_SSMMU_MAPED) {
			if (atomic_read(stop)) {
				XRISP_PR_ERROR("ssmmu map thread stop, idx=%d", i);
				ret = -ETIMEDOUT;
				goto fail_map;
			}

			if (!g_cam_buf_mgr.buf_tbl[i].iomap.iovaddr)
				continue;

			g_cam_buf_mgr.buf_tbl[i].iomap.dynamic_ssmmu_map = false;
			ret = cbm_buf_ssmmu_iovmap(&g_cam_buf_mgr.buf_tbl[i].iomap);
			if (ret) {
				XRISP_PR_ERROR("idx[%d] provider_buf ssmmu map failed", i);
				goto fail_map;
			}
		}
	}

	mutex_unlock(&g_cam_buf_mgr.lock_tbl);
	XRISP_PR_INFO("all provider_buf ssmmu map success");
	return 0;

fail_map:
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);
	XRISP_PR_ERROR("all provider_buf ssmmu map failed");
	return ret;
}

void xrisp_ssmmu_unmap_all(void)
{
	uint32_t i = 0;

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	for_each_set_bit(i, g_cam_buf_mgr.bitmap, g_cam_buf_mgr.bits) {
		if (g_cam_buf_mgr.buf_tbl[i].iomap.map_type == CAMBUF_SSMMU_MAPED)
			cbm_buf_ssmmu_iovunmap(&g_cam_buf_mgr.buf_tbl[i].iomap);

		if (g_cam_buf_mgr.buf_tbl[i].iomap.map_type == SECBUF_SSMMU_MAPED)
			sec_buf_iovunmap(&g_cam_buf_mgr.buf_tbl[i].iomap,
					 g_cam_buf_mgr.buf_tbl[i].sfd);

		g_cam_buf_mgr.buf_tbl[i].iomap.map_type = BUF_NOT_SSMMU_MAPED;
	}
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);
	XRISP_PR_INFO("all ssmmu unmap finish");
}
