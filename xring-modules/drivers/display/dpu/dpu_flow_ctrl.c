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

#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include "dpu_log.h"
#include "dpu_drv.h"
#include "dpu_kms.h"
#include "dpu_format.h"
#include "dpu_hw_rdma_ops.h"
#include "dpu_hw_wb_ops.h"
#include "dpu_hw_dmmu_tbu_ops.h"
#include "dpu_hw_cmdlist_ops.h"
#include "dp_hw_tmg_ops.h"
#include "dsi_hw_tmg_ops.h"
#include "dsi_display.h"
#include "dp_display.h"
#include "dpu_flow_ctrl.h"

struct dpu_flow_ctrl *dpu_flow_ctrl_get(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;

	if (!drm_dev) {
		DPU_ERROR("drm_dev is null\n");
		return NULL;
	}
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	return dpu_kms->flow_ctrl;
}

static void dpu_flow_line_buf_init(struct dpu_flow_ctrl *flow_ctrl)
{
	flow_ctrl->line_buf_ops.dsi_obuf_level_config = dsi_hw_tmg_obuffer_level_config;
	flow_ctrl->line_buf_ops.dp_obuf_level_config = dp_hw_tmg_obuffer_level_config;
	flow_ctrl->line_buf_ops.rch_line_buf_level_config = dpu_hw_rch_line_buf_level_config;
	flow_ctrl->line_buf_ops.wb_line_buf_level_config = dpu_hw_wb_line_buf_level_config;
}

int dpu_flow_ctrl_init(struct dpu_flow_ctrl **flow_ctrl, struct drm_device *dev)
{
	struct dpu_flow_ctrl *dpu_flow_ctrl = NULL;

	if (*flow_ctrl) {
		DPU_ERROR("dpu flowctrl has been initialized\n");
		return -EINVAL;
	}

	dpu_flow_ctrl = kzalloc(sizeof(struct dpu_flow_ctrl), GFP_KERNEL);
	if (!dpu_flow_ctrl)
		return -ENOMEM;

	dpu_flow_ctrl->dev = dev;

	dpu_flow_line_buf_init(dpu_flow_ctrl);

	*flow_ctrl = dpu_flow_ctrl;

	return 0;
}

int dpu_flow_ctrl_deinit(struct dpu_flow_ctrl *flow_ctrl)
{
	if (!flow_ctrl) {
		DPU_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	kfree(flow_ctrl);
	return 0;
}

static void dpu_flow_rdma_lbuf_level_get(void *data, struct rdma_line_buf_level *rdma_lbuf_level)
{
	struct rdma_layer_cfg *layer_cfg;
	u32 rdma_lvl_depth;
	u32 reorder_depth;
	u32 bpp;
	int layer_width;

	FLOWCTRL_DEBUG("Enter\n");
	if (!data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	layer_cfg = (struct rdma_layer_cfg *)data;
	if (layer_cfg->is_afbc || layer_cfg->layer_rot_mode == ROT_90 ||
			layer_cfg->layer_rot_mode == ROT_270) {
		rdma_lbuf_level->rdma_lvl_down0 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up0 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down1 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up1 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down2 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up2 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down3 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up3 = 0xFF;
		rdma_lbuf_level->rdma_urgent_up = 0xFF;
		rdma_lbuf_level->rdma_urgent_down = 0xFF;
	} else {
		bpp = dpu_get_drm_format_bpp(layer_cfg->drm_format);
		if (bpp == 0) {
			DPU_ERROR("layer bpp 0\n");
			return;
		}

		/* add one to get real width from config value crop_area.x2 */
		layer_width = layer_cfg->crop_area.x2 - layer_cfg->crop_area.x1 + 1;
		if (layer_width <= 0) {
			DPU_ERROR("invalid layer width %d\n", layer_width);
			return;
		}

		/* reorder buf depth is 2048 * 32 */
		reorder_depth = 2048 * 32 / (layer_width * bpp);
		/* line buf default request 1 line */
		rdma_lvl_depth = DIV_ROUND_UP((1 + reorder_depth) * 64, 8);
		FLOWCTRL_DEBUG("layer_width %u, bpp %u, reorder depth %u, rdma level depth %u\n",
				layer_width, bpp, reorder_depth, rdma_lvl_depth);

		/* rdma level2 up/down is same as level3 */
		rdma_lbuf_level->rdma_lvl_up0 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down0 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up1 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down1 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up2 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down2 = 0xFF;
		rdma_lbuf_level->rdma_lvl_up3 = 0xFF;
		rdma_lbuf_level->rdma_lvl_down3 = 0xFF;
		rdma_lbuf_level->rdma_urgent_up = 0xFF;
		rdma_lbuf_level->rdma_urgent_down = 0xFF;
	}
	rdma_lbuf_level->prefetch_line = 24;

	FLOWCTRL_DEBUG("rdma_lvl_up0 %u\n"
		"rdma_lvl_down0 %u\n"
		"rdma_lvl_up1 %u\n"
		"rdma_lvl_down1 %u\n"
		"rdma_lvl_up2 %u\n"
		"rdma_lvl_down2 %u\n"
		"rdma_lvl_up3 %u\n"
		"rdma_lvl_down3 %u\n"
		"rdma_urgent_up %u\n"
		"rdma_urgent_down %u\n"
		"prefetch_line %u\n",
		rdma_lbuf_level->rdma_lvl_up0,
		rdma_lbuf_level->rdma_lvl_down0,
		rdma_lbuf_level->rdma_lvl_up1,
		rdma_lbuf_level->rdma_lvl_down1,
		rdma_lbuf_level->rdma_lvl_up2,
		rdma_lbuf_level->rdma_lvl_down2,
		rdma_lbuf_level->rdma_lvl_up3,
		rdma_lbuf_level->rdma_lvl_down3,
		rdma_lbuf_level->rdma_urgent_up,
		rdma_lbuf_level->rdma_urgent_down,
		rdma_lbuf_level->prefetch_line
	);
	FLOWCTRL_DEBUG("Exit\n");
}

static void dpu_flow_wb_lbuf_level_get(struct wb_line_buf_level *wb_lbuf_level)
{
	/* 512 is the normalized value of wb line buf depth */
	wb_lbuf_level->buf_lvl_down0 = 512 * 3 / 16;
	wb_lbuf_level->buf_lvl_up0 = DIV_ROUND_UP(512 * 5, 16);
	wb_lbuf_level->buf_lvl_down1 = 512 * 7 / 16;
	wb_lbuf_level->buf_lvl_up1 = DIV_ROUND_UP(512 * 9, 16);
	wb_lbuf_level->buf_lvl_down2 = 512 * 11 / 16;
	wb_lbuf_level->buf_lvl_up2 = DIV_ROUND_UP(512 * 13, 16);
	wb_lbuf_level->buf_urgent_down = wb_lbuf_level->buf_lvl_down1;
	wb_lbuf_level->buf_urgent_up = wb_lbuf_level->buf_lvl_up1;

	FLOWCTRL_DEBUG(
		"buf_lvl_down0 %u\n"
		"buf_lvl_up0 %u\n"
		"buf_lvl_down1 %u\n"
		"buf_lvl_up1 %u\n"
		"buf_lvl_down2 %u\n"
		"buf_lvl_up2 %u\n"
		"buf_urgent_down %u\n"
		"buf_urgent_up %u\n",
		wb_lbuf_level->buf_lvl_down0,
		wb_lbuf_level->buf_lvl_up0,
		wb_lbuf_level->buf_lvl_down1,
		wb_lbuf_level->buf_lvl_up1,
		wb_lbuf_level->buf_lvl_down2,
		wb_lbuf_level->buf_lvl_up2,
		wb_lbuf_level->buf_urgent_down,
		wb_lbuf_level->buf_urgent_up);
}

static void dpu_flow_qos_wb_wdma_get(void *in_data, void *out_data)
{
	bool is_offline = false;

	if (!in_data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	is_offline = *((bool *)in_data);

	if (is_offline) {
		*(u32 *)out_data = g_dpu_qos_table[DPU_WB_WDMA0_1_ID].single_qos_val[DPU_NRT_QOS_PAT];
		FLOWCTRL_DEBUG("offline wb wdma qos %u", *(u32 *)out_data);
	} else {
		*(u32 *)out_data = g_dpu_qos_table[DPU_WB_WDMA0_1_ID].single_qos_val[DPU_RT_QOS_PAT];
		FLOWCTRL_DEBUG("online wb wdma qos %u", *(u32 *)out_data);
	}
}

static void dpu_flow_qos_rch_rdma_get(void *in_data, void *out_data)
{
	bool is_offline = false;
	struct rdma_qos_level *cfg = NULL;
	unsigned int (*rdma_arqos_level)[4] = g_dpu_qos_table[DPU_LAYER_RDMA0_9_ID].rdma_arqos_level;
	unsigned int (*obuf_arqos_level)[4] = g_dpu_qos_table[DPU_LAYER_RDMA0_9_ID].obuf_arqos_level;

	if (!in_data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	cfg = out_data;

	cfg->rdma_arqos_level0 = rdma_arqos_level[DPU_RT_QOS_PAT][0];
	cfg->rdma_arqos_level1 = rdma_arqos_level[DPU_RT_QOS_PAT][1];
	cfg->rdma_arqos_level2 = rdma_arqos_level[DPU_RT_QOS_PAT][2];
	cfg->rdma_arqos_level3 = rdma_arqos_level[DPU_RT_QOS_PAT][3];

	is_offline = *((bool *)in_data);
	if (is_offline) {
		cfg->obuf_arqos_level0 = obuf_arqos_level[DPU_NRT_QOS_PAT][0];
		cfg->obuf_arqos_level1 = obuf_arqos_level[DPU_NRT_QOS_PAT][1];
		cfg->obuf_arqos_level2 = obuf_arqos_level[DPU_NRT_QOS_PAT][2];
		cfg->obuf_arqos_level3 = obuf_arqos_level[DPU_NRT_QOS_PAT][3];
	} else {
		cfg->obuf_arqos_level0 = obuf_arqos_level[DPU_RT_QOS_PAT][0];
		cfg->obuf_arqos_level1 = obuf_arqos_level[DPU_RT_QOS_PAT][1];
		cfg->obuf_arqos_level2 = obuf_arqos_level[DPU_RT_QOS_PAT][2];
		cfg->obuf_arqos_level3 = obuf_arqos_level[DPU_RT_QOS_PAT][3];
	}

	FLOWCTRL_DEBUG("rdma_qos_lvl: %u %u %u %u\n"
		"obuf_qos_lvl: %u %u %u %u\n",
		cfg->rdma_arqos_level0,
		cfg->rdma_arqos_level1,
		cfg->rdma_arqos_level2,
		cfg->rdma_arqos_level3,
		cfg->obuf_arqos_level0,
		cfg->obuf_arqos_level1,
		cfg->obuf_arqos_level2,
		cfg->obuf_arqos_level3);
}

void dpu_flow_qos_get(u32 qos_master_type, void *input_data, void *out_data)
{
	FLOWCTRL_DEBUG("Enter, type %d\n", qos_master_type);

	if (!out_data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	switch (qos_master_type) {
	case FLOW_QOS_CMDLIST:
		*(u32 *)out_data = g_dpu_qos_table[DPU_CMDLSIT_ID].single_qos_val[DPU_RT_QOS_PAT];
		FLOWCTRL_DEBUG("cmdlist qos %u", g_dpu_qos_table[DPU_CMDLSIT_ID].single_qos_val[DPU_RT_QOS_PAT]);
		break;
	case FLOW_QOS_MMU:
		*(u32 *)out_data = g_dpu_qos_table[DPU_MMU_ID].single_qos_val[DPU_RT_QOS_PAT];
		FLOWCTRL_DEBUG("mmu qos %u", g_dpu_qos_table[DPU_MMU_ID].single_qos_val[DPU_RT_QOS_PAT]);
		break;
	case FLOW_QOS_DSC_RDMA:
		*(u32 *)out_data = g_dpu_qos_table[DPU_DSC_RDMA0_1_ID].single_qos_val[DPU_RT_QOS_PAT];
		FLOWCTRL_DEBUG("dsc rdma qos %u", g_dpu_qos_table[DPU_DSC_RDMA0_1_ID].single_qos_val[DPU_RT_QOS_PAT]);
		break;
	case FLOW_QOS_DSC_WDMA:
		*(u32 *)out_data = g_dpu_qos_table[DPU_DSC_WDMA0_1_ID].single_qos_val[DPU_RT_QOS_PAT];
		FLOWCTRL_DEBUG("dsc wdma qos %u", g_dpu_qos_table[DPU_DSC_WDMA0_1_ID].single_qos_val[DPU_RT_QOS_PAT]);
		break;
	case FLOW_QOS_WB_WDMA:
		dpu_flow_qos_wb_wdma_get(input_data, out_data);
		break;
	case FLOW_QOS_RCH_RDMA:
		dpu_flow_qos_rch_rdma_get(input_data, out_data);
		break;
	default:
		DPU_WARN("unspport qos type %u\n", qos_master_type);
		break;
	}
	FLOWCTRL_DEBUG("Exit, type %d\n", qos_master_type);
}

static void dpu_flow_obuf_level_get(void *data, struct obuffer_parms *obuf_cfg)
{
	struct obuf_depth_info *info;
	u32 obuf_depth;
	u64 urgent_obuf_depth;

	if (!data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	info = (struct obuf_depth_info *)data;

	obuf_depth = GET_BIT_VAL(info->obufen, 0) * OBUF_MEMORY0_1_SIZE +
			GET_BIT_VAL(info->obufen, 1) * OBUF_MEMORY0_1_SIZE +
			GET_BIT_VAL(info->obufen, 2) * OBUF_MEMORY2_3_SIZE +
			GET_BIT_VAL(info->obufen, 3) * OBUF_MEMORY2_3_SIZE;
	obuf_cfg->core_obuf_lvl_thre_buf0 = obuf_depth / 4;
	obuf_cfg->core_obuf_lvl_thre_buf1 = obuf_depth / 2;
	obuf_cfg->core_obuf_lvl_thre_buf2 = obuf_depth * 3 / 4;
	obuf_cfg->core_obuf_th_buf = obuf_depth / 20;

	/*
	 * urgent obuf depth must contain amount of data can support 60us
	 * and obuffer memory bit width is 240
	 */
	urgent_obuf_depth = 60 * (u64)info->fps * info->vtotal * info->bpp * info->width /
			1000000 / 240;
	FLOWCTRL_DEBUG("urgent depth %llu, fps %u, vtotal %u, bpp %u, width %u\n",
		urgent_obuf_depth, info->fps, info->vtotal, info->bpp, info->width);
	obuf_cfg->core_urgent_h_thre_buf = urgent_obuf_depth * 11 / 20;
	obuf_cfg->core_urgent_l_thre_buf = urgent_obuf_depth / 2;
	obuf_cfg->core_urgent_vld_en_buf = 1;

	obuf_cfg->core_dfc_h_thre_buf = urgent_obuf_depth;
	obuf_cfg->core_dfc_l_thre_buf = urgent_obuf_depth;

	FLOWCTRL_DEBUG(
		"core_urgent_vld_en_buf %u\n"
		"core_dfc_l_thre_buf %u\n"
		"core_dfc_h_thre_buf %u\n"
		"core_urgent_l_thre_buf %u\n"
		"core_urgent_h_thre_buf %u\n"
		"core_obuf_lvl_thre_buf0 %u\n"
		"core_obuf_lvl_thre_buf1 %u\n"
		"core_obuf_lvl_thre_buf2 %u\n"
		"core_obuf_th_buf %u\n"
		"obuf0_en: %u\n"
		"obuf1_en: %u\n"
		"obuf2_en: %u\n"
		"obuf3_en: %u\n",
		obuf_cfg->core_urgent_vld_en_buf,
		obuf_cfg->core_dfc_l_thre_buf,
		obuf_cfg->core_dfc_h_thre_buf,
		obuf_cfg->core_urgent_l_thre_buf,
		obuf_cfg->core_urgent_h_thre_buf,
		obuf_cfg->core_obuf_lvl_thre_buf0,
		obuf_cfg->core_obuf_lvl_thre_buf1,
		obuf_cfg->core_obuf_lvl_thre_buf2,
		obuf_cfg->core_obuf_th_buf,
		GET_BIT_VAL(info->obufen, 0),
		GET_BIT_VAL(info->obufen, 1),
		GET_BIT_VAL(info->obufen, 2),
		GET_BIT_VAL(info->obufen, 3));
}

void dpu_flow_line_buf_level_cfg(struct dpu_flow_ctrl *flow_ctrl, struct dpu_hw_blk *hw,
	u32 line_buf_level_type, void *data)
{
	struct obuffer_parms obuf_cfg;
	struct rdma_line_buf_level rdma_lbuf_level;
	struct wb_line_buf_level wb_lbuf_level;

	FLOWCTRL_DEBUG("Enter, type %d\n", line_buf_level_type);

	if (!flow_ctrl) {
		DPU_ERROR("flow ctrl is null\n");
		return;
	}

	if (!hw) {
		DPU_ERROR("hw is null\n");
		return;
	}

	switch (line_buf_level_type) {
	case FLOW_OBUF_LEVEL_DSI:
		dpu_flow_obuf_level_get(data, &obuf_cfg);
		flow_ctrl->line_buf_ops.dsi_obuf_level_config(hw, &obuf_cfg);
		break;
	case FLOW_OBUF_LEVEL_DP:
		dpu_flow_obuf_level_get(data, &obuf_cfg);
		flow_ctrl->line_buf_ops.dp_obuf_level_config(hw, &obuf_cfg);
		break;
	case FLOW_LBUF_LEVEL_RDMA:
		dpu_flow_rdma_lbuf_level_get(data, &rdma_lbuf_level);
		flow_ctrl->line_buf_ops.rch_line_buf_level_config(hw, rdma_lbuf_level);
		break;
	case FLOW_LBUF_LEVEL_WB:
		dpu_flow_wb_lbuf_level_get(&wb_lbuf_level);
		flow_ctrl->line_buf_ops.wb_line_buf_level_config(hw, wb_lbuf_level);
		break;
	default:
		DPU_WARN("unsupported line buf type %u\n", line_buf_level_type);
		break;
	}
	FLOWCTRL_DEBUG("Exit, type %d\n", line_buf_level_type);
}
