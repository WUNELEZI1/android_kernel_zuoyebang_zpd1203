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

#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>

#include "veu_drv.h"
#include "veu_uapi.h"
#include "veu_utils.h"
#include "include/veu_hw_wb_core_reg.h"
#include "include/veu_base_addr.h"
#include "veu_defs.h"
#include "veu_format.h"
#include "veu_enum.h"
#include "veu_dmmu_ops.h"
#include "veu_dmmu.h"

#define PER_AFBC_HEADER_SIZE            16
#define AFBC_HEADER_PIXEL_16x16         16
#define AFBC_HEADER_PIXEL_WIDTH_32x8    32
#define AFBC_HEADER_PIXEL_HEIGHT_32x8   8
#define AFBC_TILE_PER_SUPERBLOCK_NUM            8

static u32 veu_get_afbce_header_size(int superblock, int tile_mode, uint32_t width, uint32_t height)
{
	u32 superblock_header_width_align, superblock_header_height_align;
	u32 per_superblock_header_size = PER_AFBC_HEADER_SIZE;
	u32 header_size;

	switch (superblock) {
	case SUPERBLOCK_16x16:
	case SUPERBLOCK_16x8:
		superblock_header_width_align = AFBC_HEADER_PIXEL_16x16;
		superblock_header_height_align = AFBC_HEADER_PIXEL_16x16;
		break;
	case SUPERBLOCK_32x8:
	case SUPERBLOCK_32x4:
		superblock_header_width_align = AFBC_HEADER_PIXEL_WIDTH_32x8;
		superblock_header_height_align = AFBC_HEADER_PIXEL_HEIGHT_32x8;
		break;
	default:
		VEU_ERR("error afbc SUPERBLOCK layout %d", superblock);
		return 0;
	}

	switch (tile_mode) {
	case SCAN_MODE:
		break;
	case TILE_MODE:
		superblock_header_width_align =
				superblock_header_width_align * AFBC_TILE_PER_SUPERBLOCK_NUM;
		superblock_header_height_align =
				superblock_header_height_align * AFBC_TILE_PER_SUPERBLOCK_NUM;
		per_superblock_header_size *= (AFBC_TILE_PER_SUPERBLOCK_NUM * AFBC_TILE_PER_SUPERBLOCK_NUM);
		break;
	default:
		VEU_ERR("error afbc tile mode %d", tile_mode);
		return 0;
	}

	header_size = (ALIGN(ceil(width, superblock_header_width_align) *
			ceil(height, superblock_header_height_align) *
			per_superblock_header_size, PAGE_SIZE));
	VEU_DBG("wdma afbce header size %d", header_size);

	return header_size;
}

static void wdma_afbce_config(struct veu_wdma_param *wdma_param)
{
	struct afbce_param *afbce = &wdma_param->afbce;
	struct veu_layer *layer = &wdma_param->layer;

	VEU_DBG("enter");
	afbce->fbc_tile_split_en = layer->afbc_layout & BIT(0) ? 1 : 0;
	afbce->fbc_tile_wide = layer->afbc_layout & BIT(1) ? 1 : 0;
	afbce->fbc_yuv_transform_en = layer->afbc_yuvtransform;
	afbce->fbc_tile_hd_mode_on = layer->afbc_tile_mode;

	/* In arm afbc 1.2 stand, defalut color and copy mode was default enable */
	afbce->fbc_default_color_en = 1;
	afbce->fbc_copy_mode_en = 1;
	VEU_DBG("exit");
}

static void wdma_afbce_reset(struct veu_wdma_param *wdma_param)
{
	struct afbce_param afbce = wdma_param->afbce;

	memset(&afbce, 0, sizeof(struct afbce_param));
}

static void wdma_tranform_mode_calc(struct veu_wdma_param *wdma_param)
{
	struct veu_layer *layer = &wdma_param->layer;

	switch (layer->transform) {
	case VEU_TRANSFORM_FLIP_H:
		wdma_param->rot_mode = 0;
		wdma_param->flip_mode = 1;
		break;
	case VEU_TRANSFORM_FLIP_V:
		wdma_param->rot_mode = 0;
		wdma_param->flip_mode = 2;
		break;
	case VEU_TRANSFORM_ROT_90:
		wdma_param->rot_mode = 1;
		wdma_param->flip_mode = 0;
		break;
	case VEU_TRANSFORM_ROT_180:
		wdma_param->rot_mode = 2;
		wdma_param->flip_mode = 0;
		break;
	case VEU_TRANSFORM_ROT_270:
		wdma_param->rot_mode = 3;
		wdma_param->flip_mode = 0;
		break;
	case VEU_TRANSFORM_H_90:
		wdma_param->rot_mode = 1;
		wdma_param->flip_mode = 2;
		break;
	case VEU_TRANSFORM_V_90:
		wdma_param->rot_mode = 1;
		wdma_param->flip_mode = 1;
		break;
	default:
		wdma_param->rot_mode = 0;
		wdma_param->flip_mode = 0;
		break;
	}
}

static void wdma_out_rect_config(struct veu_wdma_param *wdma_param,
		struct display_wb_stripe_info *stripe_output)
{
	struct veu_layer *layer = &wdma_param->layer;

	if (layer->vpu_crop) {
		VEU_DBG("vpu crop enable, use original layer crop area instead of imgwidth and imgheight");
		wdma_param->vpu_crop_width = layer->dst_rect.right - layer->dst_rect.left + 1;
		wdma_param->vpu_crop_height = layer->dst_rect.bottom - layer->dst_rect.top + 1;
	}

	layer->src_rect = stripe_rect_to_veu_rect_ltrb(stripe_output->wb_output_rect);

	layer->dst_rect = layer->src_rect;
	wdma_param->out_width = layer->img_width;
	wdma_param->out_height = layer->img_height;
	wdma_param->out_subframe_width = layer->dst_rect.right - layer->dst_rect.left + 1;
	wdma_param->out_subframe_height = layer->dst_rect.bottom - layer->dst_rect.top + 1;
	wdma_param->out_subframe_ltopx = layer->dst_rect.left;
	wdma_param->out_subframe_ltopy = layer->dst_rect.top;

	// wb rule
	if (layer->transform & VEU_TRANSFORM_ROT_90) {
		wdma_param->out_subframe_ltopx = ALIGN_DOWN(wdma_param->out_subframe_ltopx, 16);
		wdma_param->out_subframe_ltopy = ALIGN_DOWN(wdma_param->out_subframe_ltopy, 16);
	} else {
		if (layer->afbc_used) {
			switch (layer->afbc_layout) {
			case SUPERBLOCK_16x16:
			case SUPERBLOCK_16x8:
				wdma_param->out_subframe_ltopx =
					ALIGN_DOWN(wdma_param->out_subframe_ltopx, 16);
				wdma_param->out_subframe_ltopy =
					ALIGN_DOWN(wdma_param->out_subframe_ltopy, 16);
				break;
			case SUPERBLOCK_32x8:
			case SUPERBLOCK_32x4:
				wdma_param->out_subframe_ltopx =
					ALIGN_DOWN(wdma_param->out_subframe_ltopx, 32);
				wdma_param->out_subframe_ltopy =
					ALIGN_DOWN(wdma_param->out_subframe_ltopy, 8);
				break;
			default:
				VEU_ERR("output layer afbc param fail!");
				return;
			}
		}
	}
}

static int get_wdma_addr(struct veu_data *veu_dev)
{
	struct veu_wdma_param *wdma_param = &veu_dev->wdma_param;
	struct veu_layer *layer;
	int ret;

	layer = &wdma_param->layer;
	ret = veu_get_iova_by_sharefd(veu_dev, layer->sharefd, layer->buf_size, MODULE_WDMA);
	veu_check_and_return(ret, -1, "get wdma addr fail");
	wdma_param->phy_addr = veu_dev->wdma_obj->iova;
	if (layer->afbc_used)
		wdma_param->afbc_header_size =
			veu_get_afbce_header_size(layer->afbc_layout, layer->afbc_tile_mode,
			wdma_param->out_width, wdma_param->out_height);
	wdma_param->buf_size = layer->buf_size;

	return 0;
}

int veu_wdma_config(struct veu_data *veu_dev, struct veu_layer *layer)
{
	const struct veu_format_map *format_map = NULL;
	struct veu_stripe_param *stripe_param = NULL;
	struct veu_wdma_param *wdma_param;
	int ret;

	if (veu_dev == NULL || layer == NULL || veu_dev->veu_stripe == NULL) {
		VEU_ERR("invalid input param");
		return -1;
	}
	wdma_param = &(veu_dev->wdma_param);

	stripe_param = &(veu_dev->veu_stripe->stripe_param[0]);

	VEU_DBG("enter");

	veu_check_and_return(!veu_dev, -1, "veu dev null");
	veu_check_and_return(!layer, -1, "layer null");

	wdma_param->layer = *layer;

	format_map = veu_format_get(MODULE_WDMA, layer->format, layer->afbc_used);

	if (layer->afbc_used && layer->afbc_layout == SUPERBLOCK_32x4) {
		if (!is_format_support_tile32x4(format_map->veu_format)) {
			VEU_ERR("format %d dont support tile size 32x4",
				format_map->veu_format);
			return -1;
		}
	}

	wdma_param->format = format_map->veu_format;
	wdma_param->bpp = format_map->bpp;
	wdma_param->is_yuv = format_map->is_yuv;
	wdma_param->plane_count = format_map->plane_count;
	wdma_param->is_afbc_support = format_map->is_afbc_support;
	if (layer->afbc_used && !wdma_param->is_afbc_support) {
		VEU_ERR("format %d dont support afbce writeback", wdma_param->format);
		return -1;
	}

	if (layer->transform && layer->afbc_used) {
		if (layer->afbc_layout != SUPERBLOCK_16x16) {
			VEU_ERR("transform layer with afbce must have tile size 16x16");
			return -1;
		}
	}

	wdma_afbce_reset(wdma_param);
	if (layer->afbc_used)
		wdma_afbce_config(wdma_param);

	wdma_tranform_mode_calc(wdma_param);
	wdma_out_rect_config(wdma_param, &(stripe_param->wb_stripe_param));

	ret = get_wdma_addr(veu_dev);
	veu_check_and_return(ret, -1, "get wdma addr fail");

	VEU_DBG("exit");

	return 0;
}

void veu_wdma_set_reg(struct veu_data *veu_dev)
{
	struct veu_wdma_param *wparam = &veu_dev->wdma_param;
	struct veu_layer *layer = &wparam->layer;
	struct buffer_info info;

	VEU_DBG("+");

	veu_check_and_void_return(!veu_dev, "veu_dev null");

	/* config mmu for wch */
	veu_mmu_get_framebuffer_info(veu_dev, &info, MODULE_WDMA);
	veu_mmu_tbu_config(veu_dev, &info, MODULE_WDMA);

	if (layer->afbc_used) {
		// afbce addr config
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_BASE_ADDR0_LOW_OFFSET,
			(wparam->phy_addr & 0xFFFFFFFF));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_BASE_ADDR0_HIGH_OFFSET,
			(wparam->phy_addr >> 32));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_BASE_ADDR1_LOW_OFFSET,
			((wparam->afbc_header_size +  wparam->phy_addr) & 0xFFFFFFFF));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_BASE_ADDR1_HIGH_OFFSET,
			((wparam->afbc_header_size +  wparam->phy_addr) >> 32));
	} else {
		// raw addr config
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_BASE_ADDR0_LOW_OFFSET,
			(wparam->phy_addr & 0xFFFFFFFF));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_BASE_ADDR0_HIGH_OFFSET,
			(wparam->phy_addr >> 32));
		// wdma only support yuv420_P2
		if (wparam->is_yuv) {
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				WB_CORE + WB_WDMA_BASE_ADDR1_LOW_OFFSET,
				((wparam->phy_addr + layer->plane_offset1) & 0xFFFFFFFF));
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				WB_CORE + WB_WDMA_BASE_ADDR1_HIGH_OFFSET,
				(wparam->phy_addr + layer->plane_offset1) >> 32);
		}
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			WB_CORE + WB_WDMA_STRIDE_OFFSET, layer->stride0);
	}
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
		WB_CORE + WB_WDMA_OUTSTANDING_NUM_OFFSET, 0x2F0F);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, WB_CORE + WB_WDMA_FBC_EN_OFFSET,
		((layer->afbc_used << WB_WDMA_FBC_EN_SHIFT) +
		(wparam->format << WB_OUT_FORMAT_SHIFT)	+
		(wparam->flip_mode << WB_FLIP_MODE_SHIFT) +
		(wparam->rot_mode << WB_ROT_MODE_SHIFT)));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, WB_CORE + FBC_COPY_MODE_EN_OFFSET,
		((wparam->afbce.fbc_tile_split_en << FBC_TILE_SPLIT_EN_SHIFT) +
		(wparam->afbce.fbc_tile_wide << FBC_TILE_WIDE_SHIFT) +
		(wparam->afbce.fbc_tile_hd_mode_on << FBC_TILE_HD_MODE_EN_SHIFT) +
		(wparam->afbce.fbc_yuv_transform_en << FBC_YUV_TRANSFORM_EN_SHIFT) +
		(wparam->afbce.fbc_default_color_en << FBC_DEFAULT_COLOR_EN_SHIFT) +
		(wparam->afbce.fbc_copy_mode_en << FBC_COPY_MODE_EN_SHIFT)));
	if (layer->vpu_crop) {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, WB_CORE + WB_OUT_ORI_WIDTH_OFFSET,
			(wparam->vpu_crop_width << WB_OUT_ORI_WIDTH_SHIFT) +
			(wparam->vpu_crop_height << WB_OUT_ORI_HEIGHT_SHIFT));
	} else {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, WB_CORE + WB_OUT_ORI_WIDTH_OFFSET,
			((wparam->out_width << WB_OUT_ORI_WIDTH_SHIFT) +
			(wparam->out_height << WB_OUT_ORI_HEIGHT_SHIFT)));
	}
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
		WB_CORE + WB_OUT_SUBFRAME_WIDTH_OFFSET,
		((wparam->out_subframe_width << WB_OUT_SUBFRAME_WIDTH_SHIFT) +
		(wparam->out_subframe_height << WB_OUT_SUBFRAME_HEIGHT_SHIFT)));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
		WB_CORE + WB_OUT_SUBFRAME_LTOPX_OFFSET,
		((wparam->out_subframe_ltopx << WB_OUT_SUBFRAME_LTOPX_SHIFT) +
		(wparam->out_subframe_ltopy << WB_OUT_SUBFRAME_LTOPY_SHIFT)));

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, WB_CORE + WB_WDMA_QOS_OFFSET, 0x2);

	outp32(veu_dev->base + WB_CORE + VEU_WB_CORE_FORCE_UPDATE_EN_OFFSET, BIT(1));
	VEU_DBG("-");
}
