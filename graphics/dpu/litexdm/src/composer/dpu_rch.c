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
#include "flowctrl_cfg_define.h"

#include "litexdm.h"
#include "dpu_hw_common.h"
#include "dpu_hw_rdma_ops.h"
#include "dpu_format.h"
#include "dpu_hw_cap.h"
#include "dpu_common_info.h"
#include "dpu_res_mgr.h"
#include "dpu_comp_mgr.h"
#include "dpu_log.h"
#include "dpu_hw_tpg_ops.h"
#include "dpu_pre_color.h"

#define GET_RDMA_AXI_PORT(rch_id) \
		((((rch_id) > RCH_V_1) && ((rch_id) < RCH_G_3)) ? \
		RDMA_AXI_1 : RDMA_AXI_0)
#define TPG_DEBUG 0
#define TPG_DEFAULT 0x2130000
#define LBUF_HALF 2048

static void dpu_plane_get_addr_info(struct rdma_layer_cfg *layer_cfg,
		disp_layer_t *layer_info)
{
	switch (layer_cfg->plane_count) {
	case PLANE_ONE:
		layer_cfg->layer_strides[STRIDE_Y_RGB] = layer_info->strides[STRIDE_Y_RGB];
		layer_cfg->layer_addrs[PLANE_Y] = layer_info->buf_addrs[PLANE_Y];
		break;
	case PLANE_TWO:
		layer_cfg->layer_strides[STRIDE_Y_RGB] = layer_info->strides[STRIDE_Y_RGB];
		layer_cfg->layer_strides[STRIDE_UV] = layer_info->strides[STRIDE_UV];
		layer_cfg->layer_addrs[PLANE_Y] = layer_info->buf_addrs[PLANE_Y];
		layer_cfg->layer_addrs[PLANE_U] = layer_info->buf_addrs[PLANE_U];
		break;
	case PLANE_THREE:
		layer_cfg->layer_strides[STRIDE_Y_RGB] = layer_info->strides[STRIDE_Y_RGB];
		layer_cfg->layer_strides[STRIDE_UV] = layer_info->strides[STRIDE_UV];
		layer_cfg->layer_addrs[PLANE_Y] = layer_info->buf_addrs[PLANE_Y];
		layer_cfg->layer_addrs[PLANE_U] = layer_info->buf_addrs[PLANE_U];
		layer_cfg->layer_addrs[PLANE_V] = layer_info->buf_addrs[PLANE_V];
		break;
	default:
		dpu_pr_err("invalid format plane count: %d\n", layer_cfg->plane_count);
		break;
	}
}

static void dpu_plane_get_exclusive_roi_info(struct rdma_layer_cfg *layer_cfg,
		disp_layer_t *layer_info)
{
	layer_cfg->roi0_en = false;
	layer_cfg->roi1_en = false;

	if ((layer_info->roi0.w > 0) && (layer_info->roi0.h > 0)) {
		layer_cfg->roi0_en = true;
		layer_cfg->roi0_area.x1 = layer_info->roi0.x;
		layer_cfg->roi0_area.x2 = layer_info->roi0.x + layer_info->roi0.w - 1;
		layer_cfg->roi0_area.y1 = layer_info->roi0.y;
		layer_cfg->roi0_area.y2 = layer_info->roi0.y + layer_info->roi0.h - 1;
	}

	if ((layer_info->roi1.w > 0) && (layer_info->roi1.h > 0)) {
		layer_cfg->roi1_en = true;
		layer_cfg->roi1_area.x1 = layer_info->roi1.x;
		layer_cfg->roi1_area.x2 = layer_info->roi1.x + layer_info->roi1.w - 1;
		layer_cfg->roi1_area.y1 = layer_info->roi1.y;
		layer_cfg->roi1_area.y2 = layer_info->roi1.y + layer_info->roi1.h - 1;
	}
}

/* should porting sram software algorithm */
static void dpu_rdma_lbuf_mem_cal(struct rdma_layer_cfg *layer_cfg, u32 id)
{
	layer_cfg->lbuf_size = LBUF_HALF;
	layer_cfg->lbuf_base_addr = (id % 2) * LBUF_HALF;
	layer_cfg->lbuf_mem_map = (id % 2) ? false : true;
	layer_cfg->left_lbuf_size = LBUF_HALF;
}

static void dpu_plane_get_layer_cfg(struct rdma_layer_cfg *layer_cfg,
		disp_layer_t *layer_info)
{
	dpu_pr_debug("+\n");

	layer_cfg->pixel_format = layer_info->format;
	layer_cfg->is_yuv = is_yuv(layer_info->format);
	layer_cfg->is_uv_swap = layer_info->is_uv_swap;
	layer_cfg->is_narrow_yuv = layer_info->is_narrow_yuv;
	layer_cfg->extender_mode = EXTENDER_MODE_NONE;

	layer_cfg->is_afbc = layer_info->is_afbc;
	layer_cfg->is_fbc_split_mode = layer_info->is_fbc_split_mode;
	layer_cfg->is_fbc_32x8 = layer_info->is_fbc_32x8;
	layer_cfg->is_fbc_yuv_transform = layer_info->is_fbc_yuv_transform;
	layer_cfg->is_fbc_tile_mode = layer_info->is_fbc_tile_mode;

	layer_cfg->plane_count = layer_info->plane_count;
	if (layer_cfg->is_afbc)
		layer_cfg->plane_count = PLANE_ONE;

	dpu_plane_get_addr_info(layer_cfg, layer_info);

	layer_cfg->crop_area.x1 = layer_info->src_rect.x;
	layer_cfg->crop_area.x2 = MINUS_ONE(layer_info->src_rect.x + layer_info->src_rect.w);
	layer_cfg->crop_area.y1 = layer_info->src_rect.y;
	layer_cfg->crop_area.y2 = MINUS_ONE(layer_info->src_rect.y + layer_info->src_rect.h);

	layer_cfg->img_size.w = layer_info->width;
	layer_cfg->img_size.h = layer_info->height;

	dpu_plane_get_exclusive_roi_info(layer_cfg, layer_info);

	layer_cfg->layer_flip_mode = layer_info->flip;
	layer_cfg->layer_rot_mode = layer_info->rotation;

	layer_cfg->layer_mixer_y_offset = 0;

	dpu_pr_debug("-\n");
}

static void __maybe_unused dpu_tpg_enable(struct dpu_hw_rch *hw_rch,
		disp_layer_t *layer_info, enum tpg_position tpg_pos)
{
	struct tpg_config cfg;

	switch (tpg_pos) {
	case TPG_POS_G0_PREPQ:
	case TPG_POS_V0_PREPQ:
	case TPG_POS_G1_PREPQ:
	case TPG_POS_V1_PREPQ:
	case TPG_POS_G2_PREPQ:
	case TPG_POS_G3_PREPQ:
	case TPG_POS_G4_PREPQ:
	case TPG_POS_V2_PREPQ:
	case TPG_POS_V3_PREPQ:
	case TPG_POS_G5_PREPQ:
		if (layer_info->chn_idx != tpg_pos) {
			DPU_DEBUG("tpg pos%d size not set\n", tpg_pos);
			return;
		}

	case TPG_POS_MIXER0:
	case TPG_POS_MIXER1:
	case TPG_POS_MIXER2:
	case TPG_POS_POSTPQ0:
	case TPG_POS_POSTPQ1:
		cfg.height = layer_info->src_rect.h;
		cfg.width = layer_info->src_rect.w;
		break;
	default:
		cfg.height = layer_info->src_rect.h;
		cfg.width = layer_info->src_rect.w;
		break;
	}

	cfg.mode_index = TPG_MODE_4;
	cfg.tpg_enable = true;
	dpu_tpg_param_set(&cfg, TPG_DEFAULT);
	dpu_hw_tpg_enable(hw_rch->hw.iomem_base,
			tpg_pos, &cfg);
}

static void dpu_flow_rdma_qos_get(struct rdma_qos_level *cfg, uint32_t scene_id)
{
	unsigned int (*rdma_arqos_level)[4] = g_dpu_qos_table[DPU_LAYER_RDMA0_9_ID].rdma_arqos_level;
	unsigned int (*obuf_arqos_level)[4] = g_dpu_qos_table[DPU_LAYER_RDMA0_9_ID].obuf_arqos_level;

	cfg->rdma_arqos_level0 = rdma_arqos_level[DPU_RT_QOS_PAT][0];
	cfg->rdma_arqos_level1 = rdma_arqos_level[DPU_RT_QOS_PAT][1];
	cfg->rdma_arqos_level2 = rdma_arqos_level[DPU_RT_QOS_PAT][2];
	cfg->rdma_arqos_level3 = rdma_arqos_level[DPU_RT_QOS_PAT][3];

	if (is_offline_scene(scene_id)) {
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
}

void dpu_flow_rdma_lbuf_level_get(struct rdma_layer_cfg *layer_cfg,
	struct rdma_line_buf_level *rdma_lbuf_level)
{
	int layer_width;
	u32 rdma_lvl_depth;
	u32 reorder_depth;
	u32 bpp = 0;

	if (layer_cfg->pixel_format == RCH_FORMAT_RGB_565)
		bpp = 16;
	else if (layer_cfg->pixel_format == RCH_FORMAT_ARGB_8888)
		bpp = 32;
	else
		dpu_pr_err("invalid format %d\n", layer_cfg->pixel_format);

	layer_width = layer_cfg->crop_area.x2 - layer_cfg->crop_area.x1 + 1;
	if (layer_width <= 0) {
		dpu_pr_err("invalid layer width %d\n", layer_width);
		return;
	}

	/* reorder buf depth is 2048 * 32 */
	reorder_depth = 2048 * 32 / (layer_width * bpp);
	/* line buf default request 1 line */
	rdma_lvl_depth = DIV_ROUND_UP((1 + reorder_depth) * 64, 8);
	dpu_pr_info("layer_width %u, bpp %u, reorder depth %u, rdma level depth %u\n",
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

	rdma_lbuf_level->prefetch_line = 24;
}

int32_t dpu_rch_cfg(struct composer *comp, struct dpu_frame *frame)
{
	struct rdma_layer_cfg layer_cfg = {0};
	struct rdma_pipe_cfg pipe_cfg = {0};
	struct rdma_line_buf_level rdma_lbuf_level = {0};
	struct dpu_hw_blk *hw_blk;
	struct dpu_hw_rch *hw_rch;
	disp_layer_t *layer_info;
	uint32_t i;
	int32_t ret;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp || !frame, -1, "comp or frame is null\n");

	for (i = 0; i < frame->layer_nums; i++) {
		layer_info = &frame->layer_infos[i];
		hw_blk = get_hw_blk(BLK_RCH, layer_info->chn_idx);
		dpu_check_and_return(!hw_blk, -1, "get rch blk failed\n");
		hw_rch = to_dpu_hw_rch(hw_blk);

		pipe_cfg.mixer_id = comp->id;
		pipe_cfg.axi_port = GET_RDMA_AXI_PORT(layer_info->chn_idx);
		if (is_offline_scene(frame->scene_id)) {
			pipe_cfg.is_offline = true;
			pipe_cfg.offline_wb_id =
				get_conn_id_from_scene_id(frame->scene_id) -  CONNECTOR_WB0;
		} else {
			pipe_cfg.is_offline = false;
			pipe_cfg.offline_wb_id = 0;
		}

		dpu_plane_get_layer_cfg(&layer_cfg, layer_info);
		dpu_rdma_lbuf_mem_cal(&layer_cfg, layer_info->chn_idx);
		dpu_flow_rdma_qos_get(&layer_cfg.qos_cfg, frame->scene_id);
		dpu_flow_rdma_lbuf_level_get(&layer_cfg, &rdma_lbuf_level);

		hw_rch->ops.reset(&hw_rch->hw);
		hw_rch->ops.layer_config(&hw_rch->hw, &hw_rch->channel_param, &layer_cfg);
		hw_rch->ops.channel_config(&hw_rch->hw, &hw_rch->channel_param,
				&layer_cfg, &pipe_cfg);
		hw_rch->ops.layer_line_buf_level_config(&hw_rch->hw, rdma_lbuf_level);

		ret = dpu_pre_color_cfg(layer_info);
		dpu_check_and_return(ret, -1, "pre color cfg failed\n");
	}

#if TPG_DEBUG
	dpu_tpg_enable(hw_rch, layer_info, TPG_POS_POSTPQ0);
#endif

	dpu_pr_debug("-\n");
	return 0;
}
