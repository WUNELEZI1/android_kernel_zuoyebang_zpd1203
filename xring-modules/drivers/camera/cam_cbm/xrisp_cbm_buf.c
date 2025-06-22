// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "cbm_buf", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "cbm_buf", __func__, __LINE__

#include <linux/device.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/dma-heap.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_cbm.h"
#include "xrisp_cbm_api.h"
#include "xrisp_trace.h"
#include "xrisp_debug.h"
#include "xrisp_tee_ca_api.h"
#include "xrisp_rproc_api.h"
#include "soc/xring/xr_dmabuf_helper.h"
#include "xrisp_log.h"
#include "xrisp_common.h"

#define BUF_FREE_MAX_USE_TIME_MS (80)
#define DEBUG_USETIME_TRACK	 (true)

#ifdef DEBUG_USETIME_TRACK
#define TIME_TRACE_START(time_num)                                                                 \
	ktime_t time_trace[time_num];                                                              \
	int time_nums = time_num;                                                                  \
	int time_idx = 0

#define TIME_TRACE_ADD_POINT()                                                                     \
	do {                                                                                       \
		if (time_idx < time_nums)                                                          \
			time_trace[time_idx++] = ktime_get();                                      \
	} while (0)

#define TIME_TRACE_CHECK(max_time)                                                                 \
	do {                                                                                       \
		if (time_nums < 2) {                                                               \
			break;                                                                     \
		}                                                                                  \
		ktime_t total_time = time_trace[time_nums - 1] - time_trace[0];                    \
		long total_time_ms = ktime_to_ms(total_time);                                      \
		char trace_print[256] = { 0 };                                                     \
		size_t size;                                                                       \
		if (total_time_ms > max_time) {                                                    \
			size = sprintf(trace_print, "timeout: %ld ms > %u ms, ", total_time_ms,    \
				       max_time);                                                  \
			for (uint32_t i = 0; i < time_nums - 1; i++) {                             \
				ktime_t diff = time_trace[i + 1] - time_trace[i];                  \
				long diff_ms = ktime_to_ms(diff);                                  \
				size += sprintf(trace_print + size, "slot[%d] %ld ms, ", i,        \
						diff_ms);                                          \
			}                                                                          \
			XRISP_PR_INFO("%s", trace_print);                                          \
		}                                                                                  \
	} while (0)
#else
#define TIME_TRACE_START(time_num)
#define TIME_TRACE_POINT_ADD()
#define TIME_TRACE_CHECK(max_time)
#endif

static int cbm_dma_heaps_find(void)
{
	int ret = 0;
	struct device *dev;

	dev = g_cam_buf_mgr.dev;

	g_cam_buf_mgr.system_heap = dma_heap_find("system");
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.system_heap)) {
		ret = PTR_ERR(g_cam_buf_mgr.system_heap);
		XRISP_DEV_ERROR(dev, "system head not find, ret = %d(%pe)", ret,
			g_cam_buf_mgr.system_heap);
		g_cam_buf_mgr.system_heap = NULL;
		return -EPROBE_DEFER;
	}

	g_cam_buf_mgr.system_uncached_heap = dma_heap_find("system-uncached");
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.system_uncached_heap)) {
		ret = PTR_ERR(g_cam_buf_mgr.system_uncached_heap);
		XRISP_DEV_ERROR(dev, "system uncached head not find, ret = %d(%pe)", ret,
			g_cam_buf_mgr.system_uncached_heap);
		g_cam_buf_mgr.system_uncached_heap = NULL;
		return 0;
	}

	g_cam_buf_mgr.sec_heap = dma_heap_find("xring_isp_faceid");
	if (IS_ERR_OR_NULL(g_cam_buf_mgr.sec_heap)) {
		ret = PTR_ERR(g_cam_buf_mgr.sec_heap);
		XRISP_DEV_ERROR(dev, " secure head not find, ret = %d(%pe)", ret,
			g_cam_buf_mgr.sec_heap);
		g_cam_buf_mgr.sec_heap = NULL;
		return 0;
	}
	return ret;
}

static void cbm_dma_heap_put(void)
{
	if (g_cam_buf_mgr.system_heap)
		dma_heap_put(g_cam_buf_mgr.system_heap);

	if (g_cam_buf_mgr.system_uncached_heap)
		dma_heap_put(g_cam_buf_mgr.system_uncached_heap);

	if (g_cam_buf_mgr.sec_heap)
		dma_heap_put(g_cam_buf_mgr.sec_heap);
}

inline void cbm_buf_lock(struct cam_buf_element *cbm_buf)
{
	mutex_lock(&cbm_buf->lock);
}

inline void cbm_buf_unlock(struct cam_buf_element *cbm_buf)
{
	mutex_unlock(&cbm_buf->lock);
}


static void cbt_element_init(struct cam_buf_element *cbm_buf)
{
	mutex_init(&cbm_buf->lock);
	cbm_buf_lock(cbm_buf);
	INIT_LIST_HEAD(&cbm_buf->region_head);
	cbm_buf->buf_handle = 0;
	cbm_buf->fd = -1;
	cbm_buf->i_ino = 0;
	cbm_buf->size = 0;
	cbm_buf->buf = NULL;
	cbm_buf->iova = 0;
	cbm_buf->kva = 0;
	cbm_buf->status = CAM_BUF_ACTIVE;
	cbm_buf->is_internal = false;
	cbm_buf->is_imported = false;
	cbm_buf->iomap.region = NULL;
	memset(&cbm_buf->timestamp, 0, sizeof(struct timespec64));
	cbm_buf->entry = NULL;
	cbm_buf_unlock(cbm_buf);
}

static void cbt_element_deinit(struct cam_buf_element *cbm_buf)
{
	cbm_buf->status = CAM_BUF_INVALID;
	cbm_buf->fd = -1;
	cbm_buf->buf_handle = 0;
	INIT_LIST_HEAD(&cbm_buf->region_head);
	cbm_buf->region_id = 0;
	cbm_buf->i_ino = 0;
	cbm_buf->size = 0;
	cbm_buf->buf = NULL;
	cbm_buf->mapio = false;
	cbm_buf->iova = 0;
	cbm_buf->mapk = false;
	cbm_buf->kva = 0;
	cbm_buf->sfd = 0;
	memset(&cbm_buf->iomap, 0, sizeof(cbm_buf->iomap));
	cbm_buf->is_internal = false;
	cbm_buf->is_imported = false;
	memset(&cbm_buf->timestamp, 0, sizeof(struct timespec64));
	cbm_buf->uncached = false;
	cbm_buf->entry = NULL;
}

static int cbt_get_element(void)
{
	int idx;
	struct cam_buf_element *cbm_buf;

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	idx = find_first_zero_bit(g_cam_buf_mgr.bitmap, g_cam_buf_mgr.bits);
	if (idx <= 0 || idx >= CBM_MAX_TABLE_LEN) {
		XRISP_PR_ERROR("no available cbm buf");
		mutex_unlock(&g_cam_buf_mgr.lock_tbl);
		return 0;
	}
	// XRISP_PR_DEBUG("get cbm_buf idx : %d", idx);
	cbm_buf = &g_cam_buf_mgr.buf_tbl[idx];
	if (cbm_buf->status != CAM_BUF_INVALID) {
		XRISP_PR_ERROR("cbm buf req error status");
		mutex_unlock(&g_cam_buf_mgr.lock_tbl);
		return 0;
	}
	cbt_element_init(cbm_buf);
	set_bit(idx, g_cam_buf_mgr.bitmap);
	trace_xrisp_cbm_tlb_status(__func__, idx, cbm_buf->status);
	trace_xrisp_cbm_tlb_get(__func__, idx, cbm_buf->status);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);

	return idx;
}

static void cbt_put_element(int idx)
{
	struct cam_buf_element *cbm_buf;

	cbm_buf = &g_cam_buf_mgr.buf_tbl[idx];

	// XRISP_PR_DEBUG("put cbm buf idx : %d", idx);
	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	cbm_buf_lock(cbm_buf);
	trace_xrisp_cbm_tlb_status(__func__, idx, cbm_buf->status);
	trace_xrisp_cbm_tlb_put(__func__, idx, cbm_buf->status);
	cbt_element_deinit(cbm_buf);
	cbm_buf_unlock(cbm_buf);
	mutex_destroy(&cbm_buf->lock);
	clear_bit(idx, g_cam_buf_mgr.bitmap);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);

}

/*this should never be used except when cmb exit or cdm close*/
static void cbt_put_element_unsafe(int idx)
{
	struct cam_buf_element *cbm_buf;

	cbm_buf = &g_cam_buf_mgr.buf_tbl[idx];

	XRISP_PR_INFO("put cbm buf idx : %d", idx);
	cbm_buf_lock(cbm_buf);
	trace_xrisp_cbm_tlb_status(__func__, idx, cbm_buf->status);
	trace_xrisp_cbm_tlb_put(__func__, idx, cbm_buf->status);
	cbt_element_deinit(cbm_buf);
	cbm_buf_unlock(cbm_buf);
	mutex_destroy(&cbm_buf->lock);
	clear_bit(idx, g_cam_buf_mgr.bitmap);
}

static struct dma_buf *cbm_dma_buf_alloc(struct dma_heap *heap, size_t size,
					 unsigned int fd_flags)
{
	struct dma_buf *buf;

	buf = dma_heap_buffer_alloc(heap, size, fd_flags, 0);
	if (IS_ERR(buf)) {
		XRISP_PR_ERROR("dma heap alloc buf fail, ret = %ld(%pe)", PTR_ERR(buf), buf);
		return NULL;
	}

	return buf;
}

static void cbm_dma_buf_release(struct dma_buf *buf)
{
	dma_heap_buffer_free(buf);
}

static int cbm_buf_free(struct cam_buf_element *cbm_buf)
{
	struct cam_buf_handle *hdl;
	struct iosys_map map;

	TIME_TRACE_START(4);

	if (!cbm_buf)
		return -EINVAL;
	hdl = &cbm_buf->hdl;

	if (cbm_buf->status == CAM_BUF_INVALID)
		XRISP_PR_ERROR("invalid buf status");

	TIME_TRACE_ADD_POINT();
	if (cbm_buf->mapk) {
		map.is_iomem = false;
		map.vaddr = (void *)cbm_buf->kva;
		dma_buf_vunmap(cbm_buf->buf, &map);
	}
	TIME_TRACE_ADD_POINT();
	if (cbm_buf->mapio) {
		if (cbm_buf->iomap.map_type == SECBUF_SSMMU_MAPED)
			sec_buf_iovunmap(&cbm_buf->iomap, cbm_buf->sfd);
		else
			cbm_buf_iovunmap(&cbm_buf->iomap);
	}
	TIME_TRACE_ADD_POINT();
	cbm_dma_buf_release(cbm_buf->buf);
	TIME_TRACE_ADD_POINT();
	TIME_TRACE_CHECK(BUF_FREE_MAX_USE_TIME_MS);
	if (cbm_buf->entry)
		cbt_debugfs_uninstall(cbm_buf);

	return 0;
}

int cbm_cbt_init(void)
{
	int ret = 0;
	int i;
	struct cam_buf_element *cbm_buf;

	ret = cbm_dma_heaps_find();
	if (ret)
		return ret;

	g_cam_buf_mgr.bitmap = devm_bitmap_zalloc(g_cam_buf_mgr.dev,
						  CBM_MAX_TABLE_LEN,
						  __GFP_ZERO);
	if (!g_cam_buf_mgr.bitmap) {
		XRISP_DEV_ERROR(g_cam_buf_mgr.dev, "bitmap alloc fail");
		return -ENOMEM;
	}
	g_cam_buf_mgr.bits = BITS_TO_LONGS(CBM_MAX_TABLE_LEN) *
			  sizeof(unsigned long) * BITS_PER_BYTE;
	bitmap_zero(g_cam_buf_mgr.bitmap, g_cam_buf_mgr.bits);
	set_bit(0, g_cam_buf_mgr.bitmap);
	mutex_init(&g_cam_buf_mgr.lock_tbl);
	for (i = 0; i < CBM_MAX_TABLE_LEN; i++) {
		cbm_buf = &g_cam_buf_mgr.buf_tbl[i];
		cbt_element_deinit(cbm_buf);
	}
	return 0;
}

void cbm_buf_release_all(void)
{
	int i;
	struct cam_buf_element *cbm_buf;

	if (!cbm_avlid())
		return;

	for (i = CAM_SHM_STATIC_REGION_IDX_START; i < CAM_SHM_REGION_NUM; i++)
		cbm_iova_region_free(i);

	mutex_lock(&g_cam_buf_mgr.lock_region);
	XRISP_PR_INFO("region bit map: %*pbl\n", (int)g_cam_buf_mgr.region_num,
		      g_cam_buf_mgr.bitmap_region);
	mutex_unlock(&g_cam_buf_mgr.lock_region);

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	for (i = 1; i < CBM_MAX_TABLE_LEN; i++) {
		if (!test_bit(i, g_cam_buf_mgr.bitmap))
			continue;

		cbm_buf = &g_cam_buf_mgr.buf_tbl[i];
		pr_cam_buf(cbm_buf, CAM_BUF_RELEASE);
		cbm_buf_lock(cbm_buf);
		cbm_buf_free(cbm_buf);
		cbm_buf_unlock(cbm_buf);
		cbt_put_element_unsafe(i);
	}

	XRISP_PR_INFO("bit map: %*pbl\n", (int)g_cam_buf_mgr.bits, g_cam_buf_mgr.bitmap);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);
}

void cbm_cbt_exit(void)
{
	cbm_dma_heap_put();
	mutex_destroy(&g_cam_buf_mgr.lock_tbl);
}

static bool to_sec_flag(uint32_t user_flags)
{
	return !!(user_flags & SEC_BUF_TYPE_PROTECT);
}

static int to_dma_buf_flags(uint32_t user_flags)
{
	if (user_flags & CAM_BUF_USER_READ_ONLY)
		return O_RDONLY;
	else if (user_flags & CAM_BUF_USER_WRITE_ONLY)
		return O_WRONLY;
	else if (user_flags & CAM_BUF_USER_READ_WRITE)
		return O_RDWR;

	return -EINVAL;
}

static int to_dma_dir(uint32_t user_flags)
{
	if (user_flags & CAM_BUF_HW_READ_ONLY)
		return DMA_TO_DEVICE;
	else if (user_flags & CAM_BUF_HW_WRITE_ONLY)
		return DMA_FROM_DEVICE;
	else if (user_flags & CAM_BUF_HW_READ_WRITE)
		return DMA_BIDIRECTIONAL;

	return -EINVAL;
}

static int to_dma_port(uint32_t user_flags)
{
	if (user_flags & CAM_BUF_HW_READ_ONLY)
		return IOMMU_READ;
	else if (user_flags & CAM_BUF_HW_WRITE_ONLY)
		return IOMMU_WRITE;
	else if (user_flags & CAM_BUF_HW_READ_WRITE)
		return IOMMU_READ | IOMMU_WRITE;

	return -EINVAL;
}

static struct dma_heap *to_dma_heap(uint32_t user_flags)
{
	if (to_sec_flag(user_flags)) {
		if (!xrisp_rproc_is_sec_running()) {
			XRISP_PR_ERROR("isp not in sec mode, sec_buf map failed");
			return NULL;
		}
		return g_cam_buf_mgr.sec_heap;
	} else if (user_flags & (CAM_BUF_HAL_UNCACHED)) {
		return g_cam_buf_mgr.system_uncached_heap;
	} else {
		return g_cam_buf_mgr.system_heap;
	}
}

int cbm_buf_get(struct cam_buf_request *req, struct cam_buf_desc *desc)
{
	int ret = 0;
	struct dma_heap *heap;
	int idx;
	struct cam_buf_element *cbm_buf;
	struct cam_share_buf_region *region = NULL;
	struct dma_buf *buf;
	struct iosys_map kmap;
	struct cam_buf_iomap iomap = { 0 };
	uint32_t sfd = 0;
	bool mapio = false;
	bool mapk = false;

	if (!cbm_avlid())
		return -ENODEV;

	if (!req || !desc)
		return -EINVAL;

	if (!(to_dma_dir(req->flags) > 0 ||
	    to_dma_port(req->flags) > 0 ||
	    to_dma_buf_flags(req->flags) > 0)) {
		XRISP_PR_ERROR("inval cam buf flags 0x%x", req->flags);
		return -EINVAL;
	}

	heap = to_dma_heap(req->flags);
	if (heap == NULL) {
		XRISP_PR_ERROR("get heap failure.");
		return -ENODEV;
	}

	if (req->buf_region >= CAM_SHM_STATIC_REGION_IDX_START) {
		region = cbm_get_iova_region(req->buf_region);
		if (!region) {
			XRISP_PR_ERROR("invalid iova region %d", req->buf_region);
			return -EINVAL;
		}
		cbm_iova_region_lock(region);
	}

	buf = cbm_dma_buf_alloc(heap, req->size, to_dma_buf_flags(req->flags));
	if (!buf) {
		ret = -ENOBUFS;
		goto exit;
	}
	get_dma_buf(buf);

	if (req->flags & CAM_BUF_KERNEL_ACCESS) {
		ret = dma_buf_vmap(buf, &kmap);
		if (ret) {
			XRISP_PR_ERROR("dma buf vmap fail ret = %d", ret);
			goto buf_release;
		}
		mapk = true;
	}

	if (to_dma_dir(req->flags) >= 0) {
		iomap.buf = buf;
		iomap.direction = to_dma_dir(req->flags);
		iomap.iommu_port = to_dma_port(req->flags);
		iomap.region = region;
		iomap.size = req->size;

		if (to_sec_flag(req->flags)) {
			sfd = xr_dmabuf_helper_get_sfd(buf);
			if (!sfd) {
				XRISP_PR_ERROR("get secbuf sfd failed");
				goto buf_vunmap;
			}
			if (sec_buf_iovmap(&iomap, sfd, req->flags)) {
				XRISP_PR_ERROR("secbuf map failed, sfd=%d", sfd);
				goto buf_vunmap;
			}
		} else {
			// nsbuf iomap
			ret = cbm_buf_iovmap(&iomap);
			if (ret) {
				XRISP_PR_ERROR("dma buf iomap fail ret = %d", ret);
				goto buf_vunmap;
			}
		}
		mapio = true;
	}

	idx = cbt_get_element();
	if (idx == 0) {
		ret = -EBADSLT;
		goto buf_iounmap;
	}
	cbm_buf = &g_cam_buf_mgr.buf_tbl[idx];

	cbm_buf_lock(cbm_buf);
	cbm_buf->fd = dma_buf_fd(buf, req->flags);
	cbm_buf->i_ino = file_inode(buf->file)->i_ino;
	cbm_buf->buf = buf;
	cbm_buf->is_internal = true;
	cbm_buf->is_imported = false;
	cbm_buf->uncached = !!(req->flags & CAM_BUF_HAL_UNCACHED);
	cbm_buf->size = req->size;
	cbm_buf->sfd = sfd;
	if (mapk) {
		cbm_buf->kva = (uintptr_t)kmap.vaddr;
		cbm_buf->mapk = true;
	}
	if (mapio) {
		memcpy(&cbm_buf->iomap, &iomap, sizeof(iomap));
		cbm_buf->iova = iomap.iovaddr;
		cbm_buf->mapio = true;
	}

	if (mapio || mapk)
		cbm_buf->status = CAM_BUF_MAPED;

	cbm_buf->hdl.fd = cbm_buf->fd;
	cbm_buf->hdl.idx = idx;

	if (req->buf_region >= CAM_SHM_STATIC_REGION_IDX_START) {
		cbm_buf_add_to_region(region, cbm_buf);
		cbm_iova_region_unlock(region);
		cbm_buf->iomap.region = region;
		cbm_buf->region_id = region->region_id;
	}

	cbm_buf_unlock(cbm_buf);

	cbt_debugfs_install(g_cam_buf_mgr.cbt_entry, cbm_buf);

	pr_cam_buf(cbm_buf, CAM_BUF_ALLOC);

	desc->buf_handle = cbm_buf->buf_handle;
	desc->iovaddr = cbm_buf->iova;
	desc->kvaddr = cbm_buf->kva;
	return 0;
buf_iounmap:
	if (mapio) {
		if (iomap.map_type == SECBUF_SSMMU_MAPED)
			sec_buf_iovunmap(&iomap, sfd);
		else
			cbm_buf_iovunmap(&iomap);
	}
buf_vunmap:
	if (req->flags & CAM_BUF_KERNEL_ACCESS)
		dma_buf_vunmap(buf, &kmap);
buf_release:
	dma_buf_put(buf);
	cbm_dma_buf_release(buf);
exit:
	if (req->buf_region >= CAM_SHM_STATIC_REGION_IDX_START)
		cbm_iova_region_unlock(region);

	desc->buf_handle = -1;
	desc->iovaddr = 0;
	desc->kvaddr = 0;
	return ret;
}

int cbm_buf_put(struct cam_buf_desc *desc)
{
	int idx;
	struct cam_buf_handle *hdl;
	struct cam_buf_element *cbm_buf;

	TIME_TRACE_START(4);

	if (!cbm_avlid())
		return -ENODEV;

	if (!desc)
		return -EINVAL;
	hdl = (struct cam_buf_handle *)&desc->buf_handle;
	idx = hdl->idx;
	if (hdl->idx <= 0 || hdl->idx >= CBM_MAX_TABLE_LEN) {
		XRISP_PR_ERROR("invalid table idx : %d", hdl->idx);
		return -EINVAL;
	}

	TIME_TRACE_ADD_POINT();

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	if (!test_bit(idx, g_cam_buf_mgr.bitmap)) {
		XRISP_PR_ERROR("test bit fail");
		mutex_unlock(&g_cam_buf_mgr.lock_tbl);
		return -EINVAL;
	}
	cbm_buf = &g_cam_buf_mgr.buf_tbl[hdl->idx];
	cbm_buf_lock(cbm_buf);
	cbm_buf->status = CAM_BUF_PUTTED;
	cbm_buf_unlock(cbm_buf);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);

	if (cbm_buf->buf_handle != desc->buf_handle) {
		XRISP_PR_ERROR("invalid buf handle");
		return -EINVAL;
	}

	pr_cam_buf(cbm_buf, CAM_BUF_RELEASE);

	cbm_buf_lock(cbm_buf);
	if (cbm_buf->region_id >= CAM_SHM_STATIC_REGION_IDX_START) {
		XRISP_PR_DEBUG("buf in region %d, defer release", cbm_buf->region_id);
		cbm_buf_unlock(cbm_buf);
		return 0;
	}

	TIME_TRACE_ADD_POINT();

	cbm_buf_free(cbm_buf);

	TIME_TRACE_ADD_POINT();

	cbm_buf_unlock(cbm_buf);
	cbt_put_element(idx);

	TIME_TRACE_ADD_POINT();
	TIME_TRACE_CHECK(BUF_FREE_MAX_USE_TIME_MS);
	return 0;
}

static bool cbm_buf_is_internal(int fd, unsigned long i_ino)
{
	uint32_t i = 0;
	bool is_internal = false;

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	for_each_set_bit(i, g_cam_buf_mgr.bitmap, g_cam_buf_mgr.bits) {
		if ((g_cam_buf_mgr.buf_tbl[i].fd == fd) &&
			(g_cam_buf_mgr.buf_tbl[i].i_ino == i_ino)) {
			is_internal = g_cam_buf_mgr.buf_tbl[i].is_internal;
			break;
		}
	}
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);
	return is_internal;
}

int cbm_buf_map(struct cam_buf_map *map_info, struct cam_buf_desc *desc)
{
	int ret = 0;
	int idx;
	unsigned long i_ino;
	bool is_internal;
	struct cam_buf_element *cbm_buf;
	struct cam_share_buf_region *region = NULL;
	struct dma_buf *buf;
	bool mapk = false;
	struct iosys_map kmap;
	struct cam_buf_iomap iomap = { 0 };
	uint32_t sfd = 0;

	if (!cbm_avlid())
		return -ENODEV;

	if (!map_info || !desc)
		return -EINVAL;
	if (to_dma_dir(map_info->flags) < 0 ||
	    to_dma_port(map_info->flags) < 0) {
		XRISP_PR_ERROR("inval cam buf flags 0x%x", map_info->flags);
		return -EINVAL;
	}

	if (map_info->buf_region >= CAM_SHM_STATIC_REGION_IDX_START) {
		region = cbm_get_iova_region(map_info->buf_region);
		if (!region) {
			XRISP_PR_ERROR("invalid iova region %d", map_info->buf_region);
			return -EINVAL;
		}
		cbm_iova_region_lock(region);
	}

	buf = dma_buf_get(map_info->fd);
	if (IS_ERR_OR_NULL(buf)) {
		ret = PTR_ERR(buf);
		XRISP_PR_ERROR("import dma_buf fd fail ret = %d", ret);
		goto exit;
	}

	i_ino = file_inode(buf->file)->i_ino;
	is_internal = cbm_buf_is_internal(map_info->fd, i_ino);
	if (map_info->flags & CAM_BUF_KERNEL_ACCESS) {
		ret = dma_buf_vmap(buf, &kmap);
		if (ret) {
			XRISP_PR_ERROR("dma buf vmap fail ret = %d", ret);
			goto buf_put;
		}
		mapk = true;
	}

	iomap.buf = buf;
	iomap.region = region;
	iomap.direction = to_dma_dir(map_info->flags);
	iomap.iommu_port = to_dma_port(map_info->flags);
	iomap.size = 0;

	if (to_sec_flag(map_info->flags)) {
		sfd = xr_dmabuf_helper_get_sfd(buf);
		if (!sfd) {
			XRISP_PR_ERROR("get sec_buf sfd failed, fd=%d", map_info->fd);
			goto buf_vunmap;
		}
		if (sec_buf_iovmap(&iomap, sfd, map_info->flags))
			goto buf_vunmap;
	} else {
		ret = cbm_buf_iovmap(&iomap);
		if (ret) {
			XRISP_PR_ERROR("dma buf iomap fail ret = %d", ret);
			goto buf_vunmap;
		}
	}

	idx = cbt_get_element();
	if (idx == 0) {
		ret = -EBADSLT;
		goto buf_iounmap;
	}
	cbm_buf = &g_cam_buf_mgr.buf_tbl[idx];

	cbm_buf_lock(cbm_buf);
	cbm_buf->fd = map_info->fd;
	cbm_buf->i_ino = i_ino;
	cbm_buf->buf = buf;
	cbm_buf->is_internal = is_internal;
	cbm_buf->is_imported = true;
	cbm_buf->uncached = !!(map_info->flags & CAM_BUF_HAL_UNCACHED);
	cbm_buf->size = iomap.size;
	if (mapk) {
		cbm_buf->kva = (uintptr_t)kmap.vaddr;
		cbm_buf->mapk = true;
	}
	memcpy(&cbm_buf->iomap, &iomap, sizeof(iomap));
	cbm_buf->iova = iomap.iovaddr;
	cbm_buf->sfd = sfd;
	cbm_buf->mapio = true;
	cbm_buf->status = CAM_BUF_MAPED;
	cbm_buf->hdl.fd = cbm_buf->fd;
	cbm_buf->hdl.idx = idx;
	if (map_info->buf_region >= CAM_SHM_STATIC_REGION_IDX_START) {
		cbm_buf_add_to_region(region, cbm_buf);
		cbm_iova_region_unlock(region);
		cbm_buf->iomap.region = region;
		cbm_buf->region_id = region->region_id;
	}

	cbm_buf_unlock(cbm_buf);

	cbt_debugfs_install(g_cam_buf_mgr.cbt_entry, cbm_buf);

	pr_cam_buf(cbm_buf, CAM_BUF_MAP);

	desc->buf_handle = cbm_buf->buf_handle;
	desc->iovaddr = cbm_buf->iova;
	desc->kvaddr = cbm_buf->kva;
	return 0;

buf_iounmap:
	if (iomap.map_type == SECBUF_SSMMU_MAPED)
		sec_buf_iovunmap(&iomap, sfd);
	else
		cbm_buf_iovunmap(&iomap);
buf_vunmap:
	if (map_info->flags & CAM_BUF_KERNEL_ACCESS)
		dma_buf_vunmap(buf, &kmap);
buf_put:
	dma_buf_put(buf);
exit:
	if (map_info->buf_region >= CAM_SHM_STATIC_REGION_IDX_START)
		cbm_iova_region_unlock(region);

	desc->buf_handle = -1;
	desc->iovaddr = 0;
	desc->kvaddr = 0;

	return ret;
}

static void cdm_dma_buf_partial_cache_ops(struct device *dev,
		dma_addr_t phys,
		size_t size,
		enum dma_data_direction dir,
		enum cam_buf_cache_ops op)
{
	// XRISP_PR_INFO("%s cache phys:0x%lx size:%zu.",
	//	((op == CAM_BUF_CPU_ACCESS_BEGIN) ? "invalid" : "flush"),
	//	(unsigned long)phys,
	//	size);

	switch (op) {
	case CAM_BUF_CPU_ACCESS_BEGIN:
		dma_sync_single_for_cpu(dev, phys, size, dir);
		break;
	case CAM_BUF_CPU_ACCESS_END:
		dma_sync_single_for_device(dev, phys, size, dir);
		break;
	default:
		XRISP_PR_ERROR("unknown cam buf cache ops");
		break;
	}
}

static int cbm_dma_buf_partial_cache(struct cam_buf_cache *cache,
		struct cam_buf_element *cbm_buf)
{
	int ret = 0;
	struct device *dev;
	struct scatterlist *sgl;
	struct scatterlist *sg;
	int i;
	struct dma_buf *dmabuf;
	struct cam_buf_iomap *iomap;
	struct dma_buf_attachment *attach;
	phys_addr_t paddr;
	int offset = 0;
	int size = 0;
	int nents = 0;
	int sg_offset = 0;
	int pre_len = 0;
	int len = 0;
	int updata_size = 0;

	if ((!cbm_buf) || (!cache) || (!cbm_buf->buf))
		return -EINVAL;

	iomap = &cbm_buf->iomap;
	if ((!iomap->sgl) || (!iomap->attach) || (!iomap->sgl->sgl))
		return -EINVAL;

	sgl    = iomap->sgl->sgl;
	nents  = iomap->sgl->orig_nents;
	attach = iomap->attach;
	dev    = attach->dev;
	dmabuf = cbm_buf->buf;

	if (cbm_buf->uncached) {
		XRISP_PR_INFO("dmabuf is uncached.");
		return ret;
	}

	offset = cache->offset;
	size   = cache->size;
	// XRISP_PR_INFO("offset:%d size:%d nents:%d dmabuf_size:%d. cache line:%d",
	//	offset, size, nents, (unsigned int)cbm_buf->size,
	//	dma_get_cache_alignment());
	if ((size <= 0) || (offset + size > cbm_buf->size))
		return -EINVAL;

	for_each_sg(sgl, sg, nents, i) {
		paddr = sg_phys(sg);
		pre_len = len;
		len += sg->length;
		// XRISP_PR_INFO("nents:%d i:%d paddr:0x%lx sg_len:%d total_len:%d.",
		//	nents, i, (unsigned long)paddr, sg->length, len);
		if (offset >= len) {
			continue;
		} else if ((offset < len) && (offset + size <= len)) {
			if (offset > pre_len) {
				sg_offset = offset - pre_len;
				updata_size = size;
			} else {
				sg_offset = 0;
				updata_size = offset + size - pre_len;
			}
			cdm_dma_buf_partial_cache_ops(dev,
				paddr + sg_offset, updata_size,
				cbm_buf->iomap.direction, cache->ops);
			break;
		} else if ((offset < len) && (offset + size > len)) {
			if (offset > pre_len) {
				sg_offset = offset - pre_len;
				updata_size = len - offset;
			} else {
				sg_offset = 0;
				updata_size = sg->length;
			}
			cdm_dma_buf_partial_cache_ops(dev,
				paddr + sg_offset, updata_size,
				cbm_buf->iomap.direction, cache->ops);
		}
	}

	return ret;
}

int cbm_dma_buf_cache_ops(struct cam_buf_cache *cache)
{
	int ret = 0;
	int idx;
	struct cam_buf_handle *hdl;
	struct cam_buf_element *cbm_buf;

	if (!cbm_avlid())
		return -ENODEV;

	if (!cache)
		return -EINVAL;
	hdl = (struct cam_buf_handle *)&cache->buf_handle;
	idx = hdl->idx;
	if (hdl->idx <= 0 || hdl->idx >= CBM_MAX_TABLE_LEN) {
		XRISP_PR_ERROR("invalid table idx : %d", hdl->idx);
		return -EINVAL;
	}

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	if (!test_bit(idx, g_cam_buf_mgr.bitmap)) {
		XRISP_PR_ERROR("test bit fail");
		mutex_unlock(&g_cam_buf_mgr.lock_tbl);
		return -EINVAL;
	}
	cbm_buf = &g_cam_buf_mgr.buf_tbl[hdl->idx];
	cbm_buf_lock(cbm_buf);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);

	if (cbm_buf->status == CAM_BUF_PUTTED || cbm_buf->status == CAM_BUF_INVALID ||
	    cbm_buf->buf == NULL) {
		XRISP_PR_INFO("refresh already put buf cache");
		cbm_buf_unlock(cbm_buf);
		return -EINVAL;
	}

	if (!cache->is_partial) {
		switch (cache->ops) {
		case CAM_BUF_CPU_ACCESS_BEGIN:
			ret = dma_buf_begin_cpu_access(cbm_buf->buf, cbm_buf->iomap.direction);
			if (ret)
				XRISP_PR_ERROR("dma buf begin cpu access fail ret = %d", ret);
			break;
		case CAM_BUF_CPU_ACCESS_END:
			ret = dma_buf_end_cpu_access(cbm_buf->buf, cbm_buf->iomap.direction);
			if (ret)
				XRISP_PR_ERROR("dma buf end cpu access fail ret = %d", ret);
			break;
		default:
			XRISP_PR_ERROR("unknown cam buf cache ops");
			break;
		}
	} else {
		ret = cbm_dma_buf_partial_cache(cache, cbm_buf);
		if (ret)
			XRISP_PR_ERROR("dma buf cache %s fail ret = %d",
			    ((cache->ops == CAM_BUF_CPU_ACCESS_BEGIN) ? "invalid" : "flush"),
			    ret);
	}

	cbm_buf_unlock(cbm_buf);
	return ret;
}

static inline int cbm_buf_cpu_access_begin(struct xrisp_cam_buf_cache cache_req)
{
	struct cam_buf_cache cache;

	cache.buf_handle = cache_req.buf_handle;
	cache.is_partial = cache_req.is_partial;
	cache.offset     = cache_req.offset;
	cache.size       = cache_req.size;
	cache.ops        = CAM_BUF_CPU_ACCESS_BEGIN;
	return cbm_dma_buf_cache_ops(&cache);
}

static inline int cbm_buf_cpu_access_end(struct xrisp_cam_buf_cache cache_req)
{
	struct cam_buf_cache cache;

	cache.buf_handle = cache_req.buf_handle;
	cache.is_partial = cache_req.is_partial;
	cache.offset     = cache_req.offset;
	cache.size       = cache_req.size;
	cache.ops        = CAM_BUF_CPU_ACCESS_END;
	return cbm_dma_buf_cache_ops(&cache);
}

static int xrisp_cam_buf_alloc_map(struct xrisp_cam_buf_req *buf_req)
{
	int ret = 0;
	struct cam_buf_request req;
	struct cam_buf_desc desc;
	struct cam_buf_handle *hdl;

	req.size = buf_req->size;
	req.flags = buf_req->flags;
	req.buf_region = buf_req->buf_region;

	ret = cbm_buf_get(&req, &desc);
	if (ret) {
		XRISP_PR_ERROR("alloc buf fail ret = %d", ret);
		return ret;
	}
	hdl = (struct cam_buf_handle *)&desc.buf_handle;
	buf_req->desc.buf_handle = desc.buf_handle;
	buf_req->desc.fd = hdl->fd;
	buf_req->desc.hwaddr = desc.iovaddr;

	return 0;
}

static int xrisp_cam_buf_map(struct xrisp_cam_buf_map *buf_map)
{
	int ret = 0;
	struct cam_buf_map map;
	struct cam_buf_desc desc;
	struct cam_buf_handle *hdl;

	map.fd = buf_map->fd;
	map.flags = buf_map->flags;
	map.buf_region = buf_map->buf_region;

	ret = cbm_buf_map(&map, &desc);
	if (ret) {
		XRISP_PR_ERROR("map buf fail ret = %d", ret);
		return ret;
	}
	hdl = (struct cam_buf_handle *)&desc.buf_handle;
	buf_map->desc.buf_handle = desc.buf_handle;
	buf_map->desc.fd = hdl->fd;
	buf_map->desc.hwaddr = desc.iovaddr;

	return 0;
}

int xrisp_cbm_ioctl(void *cmd)
{
	int ret = 0;
	struct xrisp_control_arg *arg;
	uint32_t log_info = EDR_RAMLOG | EDR_LOGCAT | EDR_KERNEL_KMSG | EDR_OFFLINELOG;
	ktime_t ioctl_start, ioctl_use;

	if (!cmd)
		return -EINVAL;

	arg = (struct xrisp_control_arg *)cmd;

	ioctl_start = ktime_get();
	switch (arg->op_code) {
	case XRISP_CBM_BUF_ALLOC_MAP: {
		struct xrisp_cam_buf_req req;

		if (arg->size != sizeof(req))
			return -EINVAL;
		if (copy_from_user(&req, u64_to_user_ptr(arg->handle),
		    sizeof(req))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;
		}
		if (trigger_edr_report == EDR_BUF_ALLOC_MAP_ERROR) {
			edr_drv_submit_api(EDR_MCU, EDR_BUF_ALLOC_MAP_ERROR, log_info, 1, 0);
			trigger_edr_report = 0;
			XRISP_PR_ERROR("alloc buffer fail");
			return -ENOMEM;
		}
		ret = xrisp_cam_buf_alloc_map(&req);
		if (ret) {
			edr_drv_submit_api(EDR_MCU, EDR_BUF_ALLOC_MAP_ERROR, log_info, 1, 0);
			XRISP_PR_ERROR("alloc buffer fail");
		}

		if (copy_to_user(u64_to_user_ptr(arg->handle), &req,
		    sizeof(req))) {
			XRISP_PR_ERROR("fail copy to user");
			return -EFAULT;
		}
	}
	break;

	case XRISP_CBM_BUF_MAP: {
		struct xrisp_cam_buf_map map;

		if (arg->size != sizeof(map))
			return -EINVAL;
		if (copy_from_user(&map, u64_to_user_ptr(arg->handle),
		    sizeof(map))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;
		}
		if (trigger_edr_report == EDR_BUF_MAP_ERROR) {
			edr_drv_submit_api(EDR_MCU, EDR_BUF_MAP_ERROR, log_info, 1, 0);
			trigger_edr_report = 0;
			XRISP_PR_ERROR("map buffer fail");
			return -ENOMEM;
		}
		ret = xrisp_cam_buf_map(&map);
		if (ret) {
			edr_drv_submit_api(EDR_MCU, EDR_BUF_MAP_ERROR, log_info, 1, 0);
			pr_err("map buffer fail");
		}

		if (copy_to_user(u64_to_user_ptr(arg->handle), &map,
		    sizeof(map))) {
			XRISP_PR_ERROR("fail copy to user");
			return -EFAULT;
		}
	}
	break;

	case XRISP_CBM_BUF_RELEASE: {
		struct xrisp_cam_buf_release release;
		struct cam_buf_desc desc;

		if (arg->size != sizeof(release))
			return -EINVAL;
		if (copy_from_user(&release, u64_to_user_ptr(arg->handle),
		    sizeof(release))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;
		}

		desc.buf_handle = release.buf_handle;
		ret = cbm_buf_put(&desc);
		if (ret)
			XRISP_PR_ERROR("release buffer handle 0x%016llx fail", release.buf_handle);
	}
	break;

	case XRISP_CBM_BUF_CPU_ACCESS_BEGIN: {
		struct xrisp_cam_buf_cache cache;

		if (arg->size != sizeof(cache))
			return -EINVAL;
		if (copy_from_user(&cache, u64_to_user_ptr(arg->handle),
		    sizeof(cache))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;

		}
		ret = cbm_buf_cpu_access_begin(cache);
	}
	break;

	case XRISP_CBM_BUF_CPU_ACCESS_END: {
		struct xrisp_cam_buf_cache cache;

		if (arg->size != sizeof(cache))
			return -EINVAL;
		if (copy_from_user(&cache, u64_to_user_ptr(arg->handle),
		    sizeof(cache))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;
		}
		ret = cbm_buf_cpu_access_end(cache);
	}
	break;

	case XRISP_CBM_BUF_FREE_IOVA_REGION: {
		struct xrisp_cam_buf_region region;

		if (arg->size != sizeof(region))
			return -EINVAL;
		if (copy_from_user(&region, u64_to_user_ptr(arg->handle),
		    sizeof(region))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;
		}
		ret = cbm_iova_region_free(region.region);
	}
	break;

	case XRISP_CBM_BUF_ALLOC_IOVA_REGION: {
		struct xrisp_cam_buf_region region;
		int ret = 0;

		if (arg->size != sizeof(region))
			return -EINVAL;
		if (copy_from_user(&region, u64_to_user_ptr(arg->handle),
		    sizeof(region))) {
			XRISP_PR_ERROR("fail copy from user");
			return -EFAULT;
		}
		if (trigger_edr_report == EDR_IOVA_REGION_ALLOC_ERROR) {
			edr_drv_submit_api(EDR_MCU, EDR_IOVA_REGION_ALLOC_ERROR, log_info, 1, 0);
			trigger_edr_report = 0;
			XRISP_PR_ERROR("iova region alloc fail");
			return -ENOMEM;
		}
		ret = cbm_iova_region_alloc(region.region_type);
		if (ret < 0) {
			if (region.region_type == CAM_SHM_DYNAMIC_REGION_128MB_SIZE)
				edr_drv_submit_api(EDR_MCU, EDR_IOVA_REGION_ALLOC_ERROR, log_info, 1, 0);
			XRISP_PR_ERROR("iova region alloc fail");
			return ret;
		}

		region.region = ret;
		if (copy_to_user(u64_to_user_ptr(arg->handle), &region,
		    sizeof(region))) {
			XRISP_PR_ERROR("fail copy to user");
			return -EFAULT;
		}
	}
	break;

	default:
		XRISP_PR_ERROR("no such cmd");
		return -ENOIOCTLCMD;
	}

	ioctl_use = ktime_ms_delta(ktime_get(), ioctl_start);
	if (ioctl_use > ISP_IOCTL_TIMEMS_MAX)
		XRISP_PR_INFO("cbm ioctl %d use long time: %lldms", arg->op_code, ioctl_use);

	return ret;
}

int cbm_buf_get_by_handle(uint64_t buf_handle, struct cam_buf_desc *desc, size_t *size)
{
	uint32_t idx = 0;
	struct cam_buf_handle *s_handle;
	struct cam_buf_element *buf_element;

	if (!cbm_avlid())
		return -ENODEV;

	s_handle = (struct cam_buf_handle *)&buf_handle;
	idx = s_handle->idx;

	if (idx >= CBM_MAX_TABLE_LEN) {
		XRISP_PR_ERROR("input idx = %d, invalid", idx);
		return -EINVAL;
	}

	mutex_lock(&g_cam_buf_mgr.lock_tbl);
	if (test_bit(idx, g_cam_buf_mgr.bitmap) == 0) {
		XRISP_PR_ERROR("input idx = %d buf no alloc and map", idx);
		mutex_unlock(&g_cam_buf_mgr.lock_tbl);
		return -EINVAL;
	}

	buf_element = &g_cam_buf_mgr.buf_tbl[idx];
	cbm_buf_lock(buf_element);
	desc->buf_handle = buf_element->buf_handle;
	desc->iovaddr = buf_element->iova;
	desc->kvaddr = buf_element->kva;
	*size = buf_element->size;
	cbm_buf_unlock(buf_element);
	mutex_unlock(&g_cam_buf_mgr.lock_tbl);

	XRISP_PR_INFO("buf handle find, hdl=0x%llx, iova=0x%llx, kva=0x%pK, size=%zu",
		      desc->buf_handle, desc->iovaddr, (void *)desc->kvaddr, buf_element->size);

	return 0;
}

int xrisp_cbm_buf_need_release(void)
{
	int id = 0;

	if (!cbm_avlid())
		return -ENODEV;
	id = find_next_bit(g_cam_buf_mgr.bitmap, CBM_MAX_TABLE_LEN, 1);
	XRISP_PR_INFO("get first buf id=%d", id);
	if (id < CBM_MAX_TABLE_LEN) {
		XRISP_PR_INFO("need to release buf");
		return true;
	}
	return false;
}
