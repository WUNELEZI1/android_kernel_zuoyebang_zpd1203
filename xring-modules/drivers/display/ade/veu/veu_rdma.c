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

#include "veu_drv.h"
#include "veu_uapi.h"
#include "veu_utils.h"
#include "include/veu_base_addr.h"
#include "include/veu_hw_rdma_path_reg.h"
#include "veu_defs.h"
#include "veu_format.h"
#include "veu_enum.h"
#include "veu_dmmu.h"
#include "veu_dmmu_ops.h"

#ifndef DMA_ADDR_ALIGN
#define DMA_ADDR_ALIGN 16
#endif

#define LBUF_MEM_MAX_SIZE 0x600

#define RDMA_BURST_LEN 31
#define RDMA_OUTSTANDING_NUM 31
#define BUF_DEPTH (2048 * 32)
#define RDMA_INPUT_MAX_WIDTH 544
#define RDMA_INPUT_WIDTH 512

static int get_rdma_addr(struct veu_data *veu_dev, struct veu_layer *layer)
{
	struct veu_rdma_param *param = &veu_dev->rdma_param;
	int ret;

	ret = veu_get_iova_by_sharefd(veu_dev, layer->sharefd, layer->buf_size, MODULE_RDMA);
	if (ret)
		return ret;

	param->phy_addr = veu_dev->rdma_obj->iova;

	return 0;
}

static void get_afbc_tile_config(struct veu_rdma_param *param)
{
	struct veu_layer *layer = &param->layer;

	switch (layer->afbc_layout) {
	case SUPERBLOCK_16x16:
		param->afbcd.fbc_superblock_layout = 0;
		param->afbcd.fbc_split_mode = 0;
		break;
	case SUPERBLOCK_16x8:
		param->afbcd.fbc_superblock_layout = 0;
		param->afbcd.fbc_split_mode = 1;
		break;
	case SUPERBLOCK_32x8:
		param->afbcd.fbc_superblock_layout = 1;
		param->afbcd.fbc_split_mode = 0;
		break;
	case SUPERBLOCK_32x4:
		param->afbcd.fbc_superblock_layout = 1;
		param->afbcd.fbc_split_mode = 1;
		break;
	default:
		param->afbcd.fbc_superblock_layout = 0;
		param->afbcd.fbc_split_mode = 0;
	}

	param->afbcd.fbc_tile_type = layer->afbc_tile_mode;
	param->afbcd.fbc_yuv_transform = layer->afbc_yuvtransform;
}

static void afbc_tile_reset(struct veu_rdma_param *param)
{
	struct afbcd_param afbcd = param->afbcd;

	memset(&afbcd, 0, sizeof(struct afbcd_param));
}

int veu_rdma_config(struct veu_data *veu_dev, struct veu_layer *layer)
{
	struct veu_stripe_param *stripe_param = NULL;
	const struct veu_format_map *format_map;
	struct veu_rdma_param *param = NULL;
	int ret;

	veu_check_and_return(!veu_dev->veu_stripe, -1, "stripe null");
	veu_check_and_return(!veu_dev, -1, "veu_dev null");
	veu_check_and_return(!layer, -1, "layer null");

	format_map = veu_format_get(MODULE_RDMA, layer->format, layer->afbc_used);

	stripe_param = &(veu_dev->veu_stripe->stripe_param[0]);

	param = &(veu_dev->rdma_param);
	param->layer = *layer;
	param->layer.src_rect =
		stripe_rect_to_veu_rect_ltrb(stripe_param->layer_stripe_param.layer_crop_rect);

	param->format = format_map->veu_format;
	param->is_afbc_support = format_map->is_afbc_support;
	param->is_yuv = format_map->is_yuv;
	param->need_uv_swap = format_map->uv_swap;
	param->plane_count = format_map->plane_count;
	param->bpp = format_map->bpp;

	afbc_tile_reset(param);
	if (layer->afbc_used)
		get_afbc_tile_config(param);
	ret = get_rdma_addr(veu_dev, layer);
	veu_check_and_return(ret, ret, "get rdma addr fail");

	return 0;
}

static void set_veu_rdma_layer_addr(struct veu_data *veu_dev,
		struct veu_rdma_param *param, struct veu_layer *layer)
{
	struct afbcd_param afbcd;

	afbcd = param->afbcd;

	if (layer->afbc_used) {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			RDMA_PATH + LEFT_BASE_ADDR0_LOW_OFFSET, (param->phy_addr & 0xffffffff));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + FBC_SPLIT_MODE_OFFSET,
			(afbcd.fbc_split_mode << FBC_SPLIT_MODE_SHIFT |
			afbcd.fbc_yuv_transform << FBC_YUV_TRANSFORM_SHIFT |
			afbcd.fbc_superblock_layout << FBC_SB_LAYOUT_SHIFT |
			afbcd.fbc_tile_type << FBC_TILE_TYPE_SHIFT));
	} else {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			RDMA_PATH + LEFT_BASE_ADDR0_LOW_OFFSET, (param->phy_addr & 0xffffffff));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			RDMA_PATH + LEFT_BASE_ADDR0_HIGH_OFFSET, param->phy_addr >> 32);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			RDMA_PATH + LEFT_RDMA_STRIDE0_OFFSET, layer->stride0);
		if (is_yuv_semi_planar(param->format)) {
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_BASE_ADDR1_LOW_OFFSET,
				(param->phy_addr + layer->plane_offset1) & 0xffffffff);
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_BASE_ADDR1_HIGH_OFFSET,
				(param->phy_addr + layer->plane_offset1) >> 32);
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_RDMA_STRIDE0_OFFSET,
				layer->stride0 + (layer->stride1 << LEFT_RDMA_STRIDE1_SHIFT));
		}
		if (is_yuv_planar(param->format)) {
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_BASE_ADDR1_LOW_OFFSET,
				(param->phy_addr + layer->plane_offset1) & 0xffffffff);
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_BASE_ADDR1_HIGH_OFFSET,
				(param->phy_addr + layer->plane_offset1) >> 32);
			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_RDMA_STRIDE0_OFFSET,
				layer->stride0 + (layer->stride1 << LEFT_RDMA_STRIDE1_SHIFT));

			veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
				RDMA_PATH + LEFT_BASE_ADDR2_LOW_OFFSET,
				param->phy_addr + layer->plane_offset2);
		}
	}
}

static void veu_rdma_config_buf_lvl(struct veu_data *veu_dev, struct veu_layer *layer)
{
	struct veu_rdma_param *param = &veu_dev->rdma_param;
	uint32_t rdma_qos_down0;
	uint32_t rdma_qos_up0;
	uint32_t rdma_qos_down1;
	uint32_t rdma_qos_up1;
	uint32_t rdma_qos_down2;
	uint32_t rdma_qos_up2;
	uint32_t rdma_qos_down3;
	uint32_t rdma_qos_up3;
	uint32_t rdma_urgent_up;
	uint32_t rdma_urgent_down;
	uint32_t tmp;
	uint32_t qos_value;

	if (layer->afbc_used || ((!layer->afbc_used) && (layer->transform & VEU_TRANSFORM_ROT_90))) {
		rdma_qos_down0 = 2;
		rdma_qos_up0 = 2;
		rdma_qos_down1 = 5;
		rdma_qos_up1 = 5;
		rdma_qos_down2 = 9;
		rdma_qos_up2 = 9;
		rdma_qos_down3 = 9;
		rdma_qos_up3 = 9;

		rdma_urgent_up = 20;
		rdma_urgent_down = 15;
	} else {
		tmp = (BUF_DEPTH + RDMA_INPUT_MAX_WIDTH * param->bpp * 8) / (RDMA_INPUT_WIDTH * param->bpp * 8);
		qos_value = (1 + tmp) * 64 / 8;

		rdma_qos_down0 = qos_value / 4;
		rdma_qos_up0 = qos_value / 4;
		rdma_qos_down1 = qos_value / 2;
		rdma_qos_up1 = qos_value / 2;
		rdma_qos_down2 = qos_value * 3 / 4;
		rdma_qos_up2 = qos_value * 3 / 4;
		rdma_qos_down3 = qos_value * 3 / 4;
		rdma_qos_up3 = qos_value * 3 / 4;

		rdma_urgent_up = qos_value;
		rdma_urgent_down = qos_value * 6 / 5;
	}
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + RDMA_QOS_UP0_OFFSET,
		rdma_qos_up0 + (rdma_qos_down0 << RDMA_QOS_DOWN0_SHIFT) +
			(rdma_qos_up1 << RDMA_QOS_UP1_SHIFT) + (rdma_qos_down1 << RDMA_QOS_DOWN1_SHIFT));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + RDMA_QOS_UP2_OFFSET,
		rdma_qos_up2 + (rdma_qos_down2 << RDMA_QOS_DOWN2_SHIFT) +
			(rdma_qos_up3 << RDMA_QOS_UP3_SHIFT) + (rdma_qos_down3 << RDMA_QOS_DOWN3_SHIFT));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + RDMA_URGENT_UP_OFFSET,
		rdma_urgent_up + (rdma_urgent_down << RDMA_URGENT_DOWN_SHIFT));
}

void veu_rdma_set_reg(struct veu_data *veu_dev)
{
	struct veu_rdma_param *param;
	struct veu_layer *layer;
	struct buffer_info info;
	u32 is_offline_wb0 = BIT(24);
	u32 is_offline = BIT(22);
	u32 burst_split_en = BIT(8);
	u32 req_conti_num = BIT(6);
	u32 layer_mode;
	u32 value;

	veu_check_and_void_return(!veu_dev, "veu device null");

	param = &(veu_dev->rdma_param);

	layer = &(param->layer);

	/* config mmu for rch */
	veu_mmu_get_framebuffer_info(veu_dev, &info, MODULE_RDMA);
	veu_mmu_tbu_config(veu_dev, &info, MODULE_RDMA);

	outp32(veu_dev->base + RDMA_PATH + RDMA_PATH_REGBANK_RESET_OFFSET, 0x1);
	value = inp32(veu_dev->base + RDMA_PATH + LAYER_MODE_OFFSET);
	outp32(veu_dev->base + RDMA_PATH + LAYER_MODE_OFFSET, value | 0x001F0000);

	set_veu_rdma_layer_addr(veu_dev, param, layer);

	layer_mode = layer->afbc_used;

	/* reconfigure rdma burst len after reset */
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + LAYER_MODE_OFFSET,
		is_offline_wb0 + is_offline + (RDMA_BURST_LEN << RDMA_BURST_LEN_SHIFT) +
		burst_split_en + req_conti_num +
		(RDMA_OUTSTANDING_NUM << RDMA_PATH_OUTSTANDING_NUM_SHIFT) + layer_mode);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + LEFT_IMG_WIDTH_OFFSET,
		(layer->img_height << LEFT_IMG_HEIGHT_SHIFT | layer->img_width));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + PIXEL_FORMAT_OFFSET,
		(param->format + (param->need_uv_swap << UV_SWAP_SHIFT)));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + LEFT_BBOX_START_X_OFFSET,
		(layer->src_rect.left | (layer->src_rect.right << LEFT_BBOX_END_X_SHIFT)));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + LEFT_BBOX_START_Y_OFFSET,
		(layer->src_rect.top | (layer->src_rect.bottom << LEFT_BBOX_END_Y_SHIFT)));
	if (param->is_yuv && (!layer->afbc_used)) {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + LBUF_MEM_SIZE_OFFSET,
			(LBUF_MEM_MAX_SIZE | BIT(28)));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			RDMA_PATH + LEFT_LBUF_MEM_SIZE_OFFSET, LBUF_MEM_MAX_SIZE / 2);
	} else {
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + LBUF_MEM_SIZE_OFFSET,
			(LBUF_MEM_MAX_SIZE | BIT(28)));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			RDMA_PATH + LEFT_LBUF_MEM_SIZE_OFFSET, LBUF_MEM_MAX_SIZE);
	}

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, RDMA_PATH + OBUF_ARQOS_LVL0_OFFSET, 0x11002200);

	veu_rdma_config_buf_lvl(veu_dev, layer);

	outp32(veu_dev->base + RDMA_PATH + RDMA_PATH_FORCE_UPDATE_EN_OFFSET,
		BIT(1));
}
