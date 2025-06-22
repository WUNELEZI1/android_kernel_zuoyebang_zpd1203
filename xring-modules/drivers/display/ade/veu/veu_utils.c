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
#include <linux/errno.h>
#include <linux/dma-resv.h>
#include <linux/version.h>

#include "veu_drv.h"
#include "veu_utils.h"
#include "veu_defs.h"
#include "veu_enum.h"
#include "veu_format.h"
#include "veu_dmmu.h"
#include "veu_trace.h"

static int get_iova_by_dmabuf(struct veu_data *veu_dev,
		struct dma_buf *buf, int32_t buf_size, int module_type)
{
	struct dma_buf_attachment *attach = NULL;
	struct veu_mmu_object *obj = NULL;
	struct veu_layer *layer = NULL;
	struct sg_table *sgt = NULL;
	int ret;

	if (module_type == MODULE_RDMA) {
		obj = veu_dev->rdma_obj;
		layer = &(veu_dev->rdma_param.layer);
	} else if (module_type == MODULE_WDMA) {
		obj = veu_dev->wdma_obj;
		layer = &(veu_dev->wdma_param.layer);
	} else {
		veu_check_and_return(true, -1, "invalid module type");
	}

	attach = dma_buf_attach(buf, g_dma_dev);
	if (IS_ERR(attach)) {
		VEU_ERR("fail to attach dma-buf");
		return -1;
	}

	sgt = dma_buf_map_attachment_unlocked(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		VEU_ERR("fail to map dma-buf to get sgt, errno %ld", PTR_ERR(sgt));
		dma_buf_detach(buf, attach);
		return -1;
	}

	obj->sgt = sgt;
	obj->buf = buf;
	obj->attach = attach;

	obj->buf_size = buf_size;
	obj->width = layer->img_width;
	obj->height = layer->img_height;
	obj->is_afbc = layer->afbc_used;
	obj->afbc_layout = layer->afbc_layout;
	obj->iova_offsets[1] = layer->plane_offset1;
	obj->iova_offsets[2] = layer->plane_offset2;

	ret = veu_mmu_map_layer_buffer(obj);
	veu_check_and_return(ret, -1, "map iova fail %d", ret);

	return 0;
}

int veu_get_iova_by_sharefd(struct veu_data *veu_dev,
		int sharefd, uint64_t buf_size, enum VEU_MODULE_TYPE module_type)
{
	struct dma_buf *buf = NULL;
	int ret;

	veu_check_and_return(sharefd < 0, -1, "invalid sharefd %d", sharefd);
	trace_veu_get_iova_by_sharefd("veu get iova by sharefd start");

	buf = dma_buf_get(sharefd);
	if (IS_ERR_OR_NULL(buf)) {
		VEU_ERR("get dma buf fail, ret = %ld sharefd = %d",
				PTR_ERR(buf), sharefd);
		return -1;
	}

	ret = get_iova_by_dmabuf(veu_dev, buf, buf_size, module_type);
	if (ret < 0) {
		VEU_ERR("invalid iova");
		dma_buf_put(buf);
		return -1;
	}
	trace_veu_get_iova_by_sharefd("veu get iova by sharefd end");
	return 0;
}

uint32_t veu_set_bit(uint32_t old_val, uint32_t val, u8 bs, u8 bw)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp;

	tmp = old_val;
	tmp &= ~(mask << bs);

	return (tmp | ((val & mask) << bs));
}

void veu_clear_bit(u8 bit_start, u8 bit_width)
{

}

uint32_t veu_reg_read(struct veu_data *veu_dev, uint32_t offset)
{
	uint32_t val = 0;

	val = readl(veu_dev->base + offset);
	return val;
}

void cpu_bit_write(struct veu_data *veu_dev, uint32_t offset, uint32_t val, uint32_t bit_start, uint32_t bit_mask)
{
	uint32_t reg_val;

	reg_val = readl(veu_dev->base + offset);
	VEU_DBG("previous val: %x", reg_val);
	reg_val = MERGE_MASK_BITS(reg_val, val, bit_start, bit_mask);

	VEU_DBG("addr: %x, current val: %x", veu_dev->addr + offset, reg_val);
	writel(reg_val, veu_dev->base + offset);
}

bool is_bpp_32(uint32_t format)
{
	switch (format) {
	case VEU_PIXEL_FORMAT_ARGB8888:
	case VEU_PIXEL_FORMAT_ABGR8888:
	case VEU_PIXEL_FORMAT_RGBA8888:
	case VEU_PIXEL_FORMAT_BGRA8888:
	case VEU_PIXEL_FORMAT_XRGB8888:
	case VEU_PIXEL_FORMAT_XBGR8888:
	case VEU_PIXEL_FORMAT_RGBX8888:
	case VEU_PIXEL_FORMAT_BGRX8888:
		return true;
	default:
		return false;
	}
}

bool is_yuv422_rformat(uint32_t format)
{
	switch (format) {
	case VEU_PIXEL_FORMAT_VYUY_422_8:
	case VEU_PIXEL_FORMAT_YVYU_422_8:
		return true;
	default:
		return false;
	}
}

bool is_yuv420_rformat(uint32_t format)
{
	switch (format) {
	case VEU_PIXEL_FORMAT_YUV420_P2_8:
	case VEU_PIXEL_FORMAT_YUV420_P3_8:
	case VEU_PIXEL_FORMAT_YUV420_P2_10:
	case VEU_PIXEL_FORMAT_YUV420_P3_10:
		return true;
	default:
		return false;
	}
}

bool is_yuv420_wformat(uint32_t format)
{
	switch (format) {
	case WB_PIXEL_FORMAT_NV21_8:
	case WB_PIXEL_FORMAT_NV12_8:
	case WB_PIXEL_FORMAT_NV12_10:
	case WB_PIXEL_FORMAT_NV21_10:
		return true;
	default:
		return false;
	}
}

bool is_yuv_planar(uint32_t format)
{
	switch (format) {
	case VEU_PIXEL_FORMAT_YUV420_P3_8:
	case VEU_PIXEL_FORMAT_YUV420_P3_10:
		return true;
	default:
		return false;
	}
}

bool is_yuv_semi_planar(uint32_t format)
{
	switch (format) {
	case VEU_PIXEL_FORMAT_YUV420_P2_8:
	case VEU_PIXEL_FORMAT_YUV420_P2_10:
		return true;
	default:
		return false;
	}
}

uint32_t get_bpp_by_format(uint32_t format)
{
	switch (format) {
	case VEU_PIXEL_FORMAT_ARGB2101010:
	case VEU_PIXEL_FORMAT_ABGR2101010:
	case VEU_PIXEL_FORMAT_RGBA1010102:
	case VEU_PIXEL_FORMAT_BGRA1010102:
	case VEU_PIXEL_FORMAT_ARGB8888:
	case VEU_PIXEL_FORMAT_ABGR8888:
	case VEU_PIXEL_FORMAT_RGBA8888:
	case VEU_PIXEL_FORMAT_BGRA8888:
	case VEU_PIXEL_FORMAT_XRGB8888:
	case VEU_PIXEL_FORMAT_XBGR8888:
	case VEU_PIXEL_FORMAT_RGBX8888:
	case VEU_PIXEL_FORMAT_BGRX8888:
		return 4;
	case VEU_PIXEL_FORMAT_RGB888:
	case VEU_PIXEL_FORMAT_BGR888:
		return 3;
	case VEU_PIXEL_FORMAT_RGBA5551:
	case VEU_PIXEL_FORMAT_BGRA5551:
	case VEU_PIXEL_FORMAT_ABGR1555:
	case VEU_PIXEL_FORMAT_ARGB1555:
	case VEU_PIXEL_FORMAT_RGBX5551:
	case VEU_PIXEL_FORMAT_BGRX5551:
	case VEU_PIXEL_FORMAT_XBGR1555:
	case VEU_PIXEL_FORMAT_XRGB1555:
	case VEU_PIXEL_FORMAT_RGB565:
	case VEU_PIXEL_FORMAT_BGR565:
		return 2;
	// todo: yuv格式怎么计算
	case VEU_PIXEL_FORMAT_XYUV_444_8:
	case VEU_PIXEL_FORMAT_XYUV_444_10:
	case VEU_PIXEL_FORMAT_VYUY_422_8:
	case VEU_PIXEL_FORMAT_YVYU_422_8:
	case VEU_PIXEL_FORMAT_YUV420_P2_8:
	case VEU_PIXEL_FORMAT_YUV420_P3_8:
	case VEU_PIXEL_FORMAT_YUV420_P2_10:
	case VEU_PIXEL_FORMAT_YUV420_P3_10:
		return 3;
	default:
		VEU_ERR("unknown format %d, bpp forced to 1", format);
		return 1;
	}
}

struct veu_rect to_veu_rect(struct veu_rect_ltrb in_rect)
{
	struct veu_rect out_rect;

	out_rect.x = in_rect.left;
	out_rect.y = in_rect.top;
	out_rect.w = in_rect.right - in_rect.left + 1;
	out_rect.h = in_rect.bottom - in_rect.top + 1;

	return out_rect;
}

struct veu_rect_ltrb to_veu_rect_ltrb(struct veu_rect in_rect)
{
	struct veu_rect_ltrb out_rect;

	out_rect.left = in_rect.x;
	out_rect.top = in_rect.y;
	out_rect.right = in_rect.x + in_rect.w - 1;
	out_rect.bottom = in_rect.y + in_rect.h - 1;

	return out_rect;
}

struct veu_rect_ltrb stripe_rect_to_veu_rect_ltrb(struct stripe_rect in_rect)
{
	struct veu_rect_ltrb out_rect;

	out_rect.left = in_rect.x;
	out_rect.top = in_rect.y;
	out_rect.right = in_rect.x + in_rect.w - 1;
	out_rect.bottom = in_rect.y + in_rect.h - 1;

	return out_rect;
}

ktime_t veu_get_timestamp(void)
{
	return ktime_get();
}

s64 veu_timestamp_diff(ktime_t begin)
{
	ktime_t end;

	end = ktime_get();

	return ktime_us_delta(end, begin);
}

void veu_rect_clear(struct veu_rect_ltrb *rect)
{
	if (rect == NULL) {
		VEU_ERR("input param null");
		return;
	}
	rect->left = 0;
	rect->right = 0;
	rect->top = 0;
	rect->bottom = 0;
}

MODULE_IMPORT_NS(DMA_BUF);
