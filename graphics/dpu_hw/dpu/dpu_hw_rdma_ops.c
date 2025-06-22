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

#include "dpu_hw_rdma_ops.h"
#include "dpu_hw_rdma_top_reg.h"
#include "dpu_hw_rdma_path_reg.h"

#define GET_CONTIG_REG_ADDR(base, index)    ((base) + (index) * 0x04)
#define GET_LAYER_REG_ADDR(base, index)    ((base) + (index) * 0x28)
#define GET_WRITE_VALUE(value, offset)    ((value) >> (offset))

#define RDMA_ROATION_LENGTH    (LEFT_ROT_MODE_LENGTH + LEFT_FLIP_MODE_LENGTH)
#define RDMA_FORMAT_LENGTH    8

#define ADDR_LOW_BIT_SIZE    32
#define RDMA_LAYER_OFFSET_REG_NUM    2
#define RDMA_HW_REG_WB0_PARAMETERS    1
#define RDMA_HW_REG_WB1_PARAMETERS    0

#define ROI_REG_STRIDE    2
#define SIGNLE_LAYER_ROI_ENA_LENGTH    2
#define NEXT_ONE_REG    1
#define BIT_ENABLE    1
#define BIT_DISABLE    0
#define LEFT_LAYER    0
#define RIGHT_LAYER    1

#define DUMP_REG_BUF_SIZE 23
#define RDMA_BURST_LEN    31
#define RDMA_OUTSTANDING_NUM    31
#define RDMA_REQ_CONTI_NUM      1

void dpu_hw_rdma_channel_default_refine(struct rch_channel_param *channel_param)
{
	u32 value = 0;
	if (!channel_param) {
		DPU_ERROR("input dpu_hw_blk was invalid parameter\n");
		return;
	}

	/* rdma path channel reg value default value init */
	channel_param->roi_enable = 0x0;
	channel_param->format_common = 0x0;
	channel_param->sram_part_size = 0x0;
	channel_param->rch_y_offset_value = 0x0;

	value = MERGE_BITS(value, RDMA_OUTSTANDING_NUM,
			RDMA_PATH_OUTSTANDING_NUM_SHIFT, RDMA_PATH_OUTSTANDING_NUM_LENGTH);
	value = MERGE_BITS(value, RDMA_REQ_CONTI_NUM,
			RDMA_PATH_REQ_CONTI_NUM_SHIFT, RDMA_PATH_REQ_CONTI_NUM_LENGTH);
	value = MERGE_BITS(value, BIT_ENABLE,
			RDMA_PATH_BURST_SPLIT_EN_SHIFT, RDMA_PATH_BURST_SPLIT_EN_LENGTH);
	value = MERGE_BITS(value, RDMA_BURST_LEN,
			RDMA_BURST_LEN_SHIFT, RDMA_BURST_LEN_LENGTH);
	channel_param->rch_mode_value = value;
}

static void __config_rdma_base_mode(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg,
		struct rdma_pipe_cfg *pipe_cfg)
{
	u32 value;
	u8 wb_id;

	value = channel_param->rch_mode_value;

	value = MERGE_BITS(value, pipe_cfg->mixer_id,
			LAYER_CMPSR_ID_SHIFT, LAYER_CMPSR_ID_LENGTH);

	value = MERGE_BITS(value, pipe_cfg->axi_port,
			RDMA_PATH_AXI_PORT_SEL_SHIFT, RDMA_PATH_AXI_PORT_SEL_LENGTH);

	if (pipe_cfg->is_offline) {
		value = MERGE_BITS(value, BIT_ENABLE,
			IS_OFFLINE_SHIFT, IS_OFFLINE_LENGTH);

		wb_id = (pipe_cfg->offline_wb_id == WB1)
			? RDMA_HW_REG_WB1_PARAMETERS
			: RDMA_HW_REG_WB0_PARAMETERS;

		value = MERGE_BITS(value, wb_id,
			IS_OFFLINE_WB0_SHIFT, IS_OFFLINE_WB0_LENGTH);
	} else {
		value = MERGE_BITS(value, BIT_DISABLE,
			IS_OFFLINE_SHIFT, IS_OFFLINE_LENGTH);

		value = MERGE_BITS(value, BIT_DISABLE,
			IS_OFFLINE_WB0_SHIFT, IS_OFFLINE_WB0_LENGTH);
	}

	if (layer_cfg->extender_mode >= EXTENDER_MODE_LEFT) {
		value = MERGE_BITS(value, BIT_ENABLE,
			IS_TWO_LAYERS_SHIFT, IS_TWO_LAYERS_LENGTH);
	} else {
		value = MERGE_BITS(value, BIT_DISABLE,
			IS_TWO_LAYERS_SHIFT, IS_TWO_LAYERS_LENGTH);
	}

	if (layer_cfg->is_afbc) {
		value = MERGE_BITS(value, BIT_ENABLE,
				LAYER_MODE_SHIFT, LAYER_MODE_LENGTH);
	} else {
		value = MERGE_BITS(value, BIT_DISABLE,
				LAYER_MODE_SHIFT, LAYER_MODE_LENGTH);
	}

	channel_param->rch_mode_value = value;
	DPU_REG_WRITE(hw, LAYER_MODE_OFFSET, value, CMDLIST_WRITE);
}

static void __config_rdma_y_offset(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg)
{
	u32 offset;
	u32 value;

	offset = (layer_cfg->extender_mode == EXTENDER_MODE_RIGHT)
		? RIGHT_CMPSR_Y_OFFSET_SHIFT : LEFT_CMPSR_Y_OFFSET_SHIFT;
	value = channel_param->rch_y_offset_value;
	value = MERGE_BITS(value, layer_cfg->layer_mixer_y_offset,
			offset, LEFT_CMPSR_Y_OFFSET_LENGTH);

	channel_param->rch_y_offset_value = value;
	DPU_REG_WRITE(hw, LEFT_CMPSR_Y_OFFSET_OFFSET, value, CMDLIST_WRITE);
}

static void __config_rdma_mem_size(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg)
{
	u32 value;

	value = channel_param->sram_part_size;

	if (layer_cfg->extender_mode == EXTENDER_MODE_RIGHT) {
		value = MERGE_BITS(value, layer_cfg->right_lbuf_size,
				RIGHT_LBUF_MEM_SIZE_SHIFT, RIGHT_LBUF_MEM_SIZE_LENGTH);
	} else {
		value = MERGE_BITS(value, layer_cfg->left_lbuf_size,
				LEFT_LBUF_MEM_SIZE_SHIFT, LEFT_LBUF_MEM_SIZE_LENGTH);
	}

	channel_param->sram_part_size = value;
	DPU_REG_WRITE(hw, LEFT_LBUF_MEM_SIZE_OFFSET, value, CMDLIST_WRITE);
}

static void __config_rdma_common_format(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg)
{
	u32 rot_value = 0;
	u32 reg_value;

	reg_value = channel_param->format_common;

	switch (layer_cfg->extender_mode) {
	case EXTENDER_MODE_NONE:
	case EXTENDER_MODE_V:
	case EXTENDER_MODE_LEFT:
		rot_value = MERGE_BITS(rot_value, layer_cfg->layer_rot_mode,
			LEFT_ROT_MODE_SHIFT, LEFT_ROT_MODE_LENGTH);
		rot_value = MERGE_BITS(rot_value, layer_cfg->layer_flip_mode,
			LEFT_FLIP_MODE_SHIFT, LEFT_FLIP_MODE_LENGTH);
		rot_value = GET_WRITE_VALUE(rot_value, LEFT_ROT_MODE_SHIFT);
		reg_value = MERGE_BITS(reg_value, rot_value,
			LEFT_ROT_MODE_SHIFT, RDMA_ROATION_LENGTH);
		break;
	case EXTENDER_MODE_RIGHT:
		rot_value = MERGE_BITS(rot_value, layer_cfg->layer_rot_mode,
			RIGHT_ROT_MODE_SHIFT, RIGHT_ROT_MODE_LENGTH);
		rot_value = MERGE_BITS(rot_value, layer_cfg->layer_flip_mode,
			RIGHT_FLIP_MODE_SHIFT, RIGHT_FLIP_MODE_LENGTH);
		rot_value = GET_WRITE_VALUE(rot_value, RIGHT_ROT_MODE_SHIFT);
		reg_value = MERGE_BITS(reg_value, rot_value,
			RIGHT_ROT_MODE_SHIFT, RDMA_ROATION_LENGTH);
		break;
	default:
		DPU_ERROR("invalid extender_mode: %d\n", layer_cfg->extender_mode);
		break;
	}

	reg_value = MERGE_BITS(reg_value, layer_cfg->pixel_format,
			PIXEL_FORMAT_SHIFT, PIXEL_FORMAT_LENGTH);
	if (layer_cfg->is_yuv) {
		if (layer_cfg->is_uv_swap) {
			reg_value = MERGE_BITS(reg_value, BIT_ENABLE, UV_SWAP_SHIFT,
					UV_SWAP_LENGTH);
		} else {
			reg_value = MERGE_BITS(reg_value, BIT_DISABLE, UV_SWAP_SHIFT,
					UV_SWAP_LENGTH);
		}
		if (layer_cfg->is_narrow_yuv) {
			reg_value = MERGE_BITS(reg_value, BIT_ENABLE,
					IS_NARROW_YUV_SHIFT, IS_NARROW_YUV_LENGTH);
		} else {
			reg_value = MERGE_BITS(reg_value, BIT_DISABLE,
					IS_NARROW_YUV_SHIFT, IS_NARROW_YUV_LENGTH);
		}
	} else {
		reg_value = MERGE_BITS(reg_value, BIT_DISABLE,
				UV_SWAP_SHIFT, UV_SWAP_LENGTH + IS_NARROW_YUV_LENGTH);
	}

	channel_param->format_common = reg_value;
	DPU_REG_WRITE(hw, PIXEL_FORMAT_OFFSET, reg_value, CMDLIST_WRITE);

}

void dpu_hw_rdma_channel_config(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg,
		struct rdma_pipe_cfg *pipe_cfg)
{
	if (!hw) {
		DPU_ERROR("input dpu_hw_blk was invalid parameter\n");
		return;
	} else if (!pipe_cfg || !layer_cfg) {
		DPU_ERROR("input rdma_cfg was invalid parameter\n");
		return;
	}
	__config_rdma_base_mode(hw, channel_param, layer_cfg, pipe_cfg);

	__config_rdma_y_offset(hw, channel_param, layer_cfg);

	__config_rdma_mem_size(hw, channel_param, layer_cfg);

	__config_rdma_common_format(hw, channel_param, layer_cfg);
}

static void __conf_rdma_layer_mid_set(struct dpu_hw_blk *hw,
		struct rdma_layer_cfg *layer_cfg)
{
	u8 rch_id;
	u8 drm_status;

	if (!hw || !layer_cfg) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	rch_id = (u8)hw->blk_id;
	drm_status = layer_cfg->drm_enable_status;

	if (drm_status == HOLD)
		return;

	DPU_SEC_RDMA(rch_id, drm_status);
}

static void __conf_rdma_layer_mode(struct dpu_hw_blk *hw,
		struct rdma_layer_cfg *layer_cfg, u8 index)
{
	u32 offset;
	u32 value;

	if (layer_cfg->is_afbc) {

		value = layer_cfg->is_fbc_split_mode ?
			BIT_ENABLE : BIT_DISABLE;
		value = MERGE_BITS(value, layer_cfg->is_fbc_yuv_transform ?
			BIT_ENABLE : BIT_DISABLE, FBC_YUV_TRANSFORM_SHIFT,
			FBC_YUV_TRANSFORM_LENGTH);
		value = MERGE_BITS(value, layer_cfg->is_fbc_32x8 ?
			BIT_ENABLE : BIT_DISABLE, FBC_SB_LAYOUT_SHIFT,
			FBC_SB_LAYOUT_LENGTH);
		value = MERGE_BITS(value, layer_cfg->is_fbc_tile_mode ?
			BIT_ENABLE : BIT_DISABLE, FBC_TILE_TYPE_SHIFT,
			FBC_TILE_TYPE_LENGTH);

		DPU_REG_WRITE(hw, FBC_SPLIT_MODE_OFFSET, value, CMDLIST_WRITE);

		value = layer_cfg->img_size.w;
		value = MERGE_BITS(value, layer_cfg->img_size.h,
			LEFT_IMG_HEIGHT_SHIFT, LEFT_IMG_HEIGHT_LENGTH);
		offset = GET_LAYER_REG_ADDR(LEFT_IMG_WIDTH_OFFSET, index);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
	} else {

		value = layer_cfg->layer_strides[STRIDE_Y_RGB];
		value = MERGE_BITS(value,
			layer_cfg->layer_strides[STRIDE_UV],
			LEFT_RDMA_STRIDE1_SHIFT, LEFT_RDMA_STRIDE1_LENGTH);
		offset = GET_LAYER_REG_ADDR(LEFT_RDMA_STRIDE0_OFFSET, index);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
	}

	value = layer_cfg->lbuf_size;
	value = MERGE_BITS(value, layer_cfg->lbuf_base_addr,
		LBUF_MEM_BASE_ADDR_SHIFT, LBUF_MEM_BASE_ADDR_LENGTH);
	value = MERGE_BITS(value, layer_cfg->lbuf_mem_map,
		LBUF_MEM_MAP_SHIFT, LBUF_MEM_MAP_LENGTH);
	DPU_REG_WRITE(hw, LBUF_MEM_SIZE_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_rdma_base_addr(struct dpu_hw_blk *hw,
		struct rdma_layer_cfg *layer_cfg)
{
	u32 base_reg_addr;
	u32 reg_addr;
	u32 value;
	int i;

	base_reg_addr = (layer_cfg->extender_mode == EXTENDER_MODE_RIGHT)
		? RIGHT_BASE_ADDR0_LOW_OFFSET : LEFT_BASE_ADDR0_LOW_OFFSET;

	for (i = 0; i < layer_cfg->plane_count; i++) {
		reg_addr = base_reg_addr;
		value = layer_cfg->layer_addrs[i];
		reg_addr = GET_CONTIG_REG_ADDR(reg_addr,
			(i * RDMA_LAYER_OFFSET_REG_NUM));
		DPU_REG_WRITE(hw, reg_addr, value, CMDLIST_WRITE);

		value = layer_cfg->layer_addrs[i] >> ADDR_LOW_BIT_SIZE;
		reg_addr = GET_CONTIG_REG_ADDR(reg_addr, NEXT_ONE_REG);
		DPU_REG_WRITE(hw, reg_addr, value, CMDLIST_WRITE);
	}
}

static void __conf_rdma_crop_area(struct dpu_hw_blk *hw,
		struct rdma_layer_cfg *layer_cfg, u8 reg_index)
{
	u32 offset = 0;
	u32 value;

	offset = GET_LAYER_REG_ADDR(LEFT_BBOX_START_X_OFFSET, reg_index);
	value = layer_cfg->crop_area.x1;
	value = MERGE_BITS(value, layer_cfg->crop_area.x2,
		LEFT_BBOX_END_X_SHIFT, LEFT_BBOX_END_X_LENGTH);
	DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);

	offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
	value = layer_cfg->crop_area.y1;
	value = MERGE_BITS(value, layer_cfg->crop_area.y2,
		LEFT_BBOX_END_Y_SHIFT, LEFT_BBOX_END_Y_LENGTH);
	DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
}

static void __conf_rdma_exclusive_roi_area(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg)
{
	u8 roi_ena = 0;
	u32 offset = 0;
	u8 shift = 0;
	u32 value;

	switch (layer_cfg->extender_mode) {
	case EXTENDER_MODE_NONE:
	case EXTENDER_MODE_V:
	case EXTENDER_MODE_LEFT:
		offset = LEFT_EXT_ROI0_START_X_OFFSET;
		shift = LEFT_EXT_ROI0_ENA_SHIFT;
		break;
	case EXTENDER_MODE_RIGHT:
		offset = RIGHT_EXT_ROI0_START_X_OFFSET;
		shift = RIGHT_EXT_ROI0_ENA_SHIFT;
		break;
	default:
		DPU_ERROR("invalid extender_mode: %d\n", layer_cfg->extender_mode);
		break;
	}

	if (layer_cfg->roi0_en) {
		roi_ena = MERGE_BITS(roi_ena, BIT_ENABLE,
			LEFT_EXT_ROI0_ENA_SHIFT, LEFT_EXT_ROI0_ENA_LENGTH);

		value = layer_cfg->roi0_area.x1;
		value = MERGE_BITS(value, layer_cfg->roi0_area.x2,
			LEFT_EXT_ROI0_END_X_SHIFT,
			LEFT_EXT_ROI0_END_X_LENGTH);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);

		value = layer_cfg->roi0_area.y1;
		value = MERGE_BITS(value, layer_cfg->roi0_area.y2,
			LEFT_EXT_ROI0_END_Y_SHIFT,
			LEFT_EXT_ROI0_END_Y_LENGTH);
		offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
	}

	if (layer_cfg->roi1_en) {
		roi_ena = MERGE_BITS(roi_ena, BIT_ENABLE,
			LEFT_EXT_ROI1_ENA_SHIFT, LEFT_EXT_ROI1_ENA_LENGTH);

		value = layer_cfg->roi1_area.x1;
		value = MERGE_BITS(value, layer_cfg->roi1_area.x2,
			LEFT_EXT_ROI1_END_X_SHIFT,
			LEFT_EXT_ROI1_END_X_LENGTH);
		offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);

		value = layer_cfg->roi1_area.y1;
		value = MERGE_BITS(value, layer_cfg->roi1_area.y2,
			LEFT_EXT_ROI1_END_Y_SHIFT,
			LEFT_EXT_ROI1_END_Y_LENGTH);
		offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
	}

	value = channel_param->roi_enable;
	value = MERGE_BITS(value, roi_ena, shift,
		SIGNLE_LAYER_ROI_ENA_LENGTH);

	channel_param->roi_enable = value;
	DPU_REG_WRITE(hw, LEFT_EXT_ROI0_ENA_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_rdma_wfd(struct dpu_hw_blk *hw,
		struct rdma_layer_cfg *layer_cfg)
{
	u32 value;

	if (layer_cfg->slice_cnt < 2)
		value = 0;
	else
		value = MERGE_BITS(1, layer_cfg->slice_line_num,
				SLICE_LINE_NUM_SHIFT, SLICE_LINE_NUM_LENGTH);

	DPU_REG_WRITE(hw, SLICE_BREAK_ENA_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_rdma_qos(struct dpu_hw_blk *hw, struct rdma_qos_level rdma_qos)
{
	u32 value = 0;

	value = MERGE_BITS(value, rdma_qos.rdma_arqos_level0, RDMA_ARQOS_LVL0_SHIFT, RDMA_ARQOS_LVL0_LENGTH);
	value = MERGE_BITS(value, rdma_qos.rdma_arqos_level1, RDMA_ARQOS_LVL1_SHIFT, RDMA_ARQOS_LVL1_LENGTH);
	value = MERGE_BITS(value, rdma_qos.rdma_arqos_level2, RDMA_ARQOS_LVL2_SHIFT, RDMA_ARQOS_LVL2_LENGTH);
	value = MERGE_BITS(value, rdma_qos.rdma_arqos_level3, RDMA_ARQOS_LVL3_SHIFT, RDMA_ARQOS_LVL3_LENGTH);

	value = MERGE_BITS(value, rdma_qos.obuf_arqos_level0, OBUF_ARQOS_LVL0_SHIFT, OBUF_ARQOS_LVL0_LENGTH);
	value = MERGE_BITS(value, rdma_qos.obuf_arqos_level1, OBUF_ARQOS_LVL1_SHIFT, OBUF_ARQOS_LVL1_LENGTH);
	value = MERGE_BITS(value, rdma_qos.obuf_arqos_level2, OBUF_ARQOS_LVL2_SHIFT, OBUF_ARQOS_LVL2_LENGTH);
	value = MERGE_BITS(value, rdma_qos.obuf_arqos_level3, OBUF_ARQOS_LVL3_SHIFT, OBUF_ARQOS_LVL3_LENGTH);

	DPU_REG_WRITE(hw, OBUF_ARQOS_LVL0_OFFSET, value, CMDLIST_WRITE);
}

void dpu_hw_rdma_layer_config(struct dpu_hw_blk *hw,
		struct rch_channel_param *channel_param,
		struct rdma_layer_cfg *layer_cfg)
{
	u8 index;

	if (!hw) {
		DPU_ERROR("input dpu_hw_blk was invalid parameter\n");
		return;
	} else if (!layer_cfg) {
		DPU_ERROR("input rdma_layer_cfg was invalid parameter\n");
		return;
	}

	index = (layer_cfg->extender_mode == EXTENDER_MODE_RIGHT) ?
		RIGHT_LAYER : LEFT_LAYER;

	__conf_rdma_layer_mode(hw, layer_cfg, index);

	__conf_rdma_base_addr(hw, layer_cfg);

	__conf_rdma_crop_area(hw, layer_cfg, index);

	__conf_rdma_exclusive_roi_area(hw, channel_param, layer_cfg);

	__conf_rdma_wfd(hw, layer_cfg);

	__conf_rdma_qos(hw, layer_cfg->qos_cfg);

	__conf_rdma_layer_mid_set(hw, layer_cfg);
}

void dpu_hw_rdma_reset(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, RDMA_PATH_REGBANK_RESET_OFFSET, REG_BANK_RESET_VAL, CMDLIST_WRITE);
}

static void dpu_hw_rdma_path_parse_dbg_irq(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	DPU_DUMP_REG("rdma_path", hw->blk_id, RDMA_DBG_IRQ_RAW_OFFSET);

	value = DPU_REG_READ(hw, RDMA_DBG_IRQ_RAW_OFFSET);
	if (value & BIT(0))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "tlb miss\n");

	if (value & BIT(1))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "tlb size err\n");

	if (value & BIT(2))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "mmu rdma timeout\n");

	if (value & BIT(3))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "mmu rdma resp err\n");

	if (value & BIT(4))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rdma timeout\n");

	if (value & BIT(5))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rdma resp err\n");

	if (value & BIT(6))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "memory conflict err\n");

	if (value & BIT(7))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rdma memory size err\n");

	if (value & BIT(8))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "left layer rdma eof\n");

	if (value & BIT(9))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "right layer rdma eof\n");

	if (value & BIT(10))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "left layer all eof\n");

	if (value & BIT(11))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "right layer all eof\n");

	if (value & BIT(12))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "afbc dec eof for left layer\n");

	if (value & BIT(13))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "afbc dec eof for right layer\n");

	if (value & BIT(14))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "left layer afbc decode err\n");

	if (value & BIT(15))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "right layer afbc decode err\n");

	if (value & BIT(16))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "left layer output eof\n");

	if (value & BIT(17))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "right layer output eof\n");

	if (value & BIT(18))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rdma sof\n");

	if (value & BIT(19))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "slice_req_done of video slice mode\n");

	if (value & BIT(20))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "re-start of video slice mode\n");

	if (value & BIT(21))
		DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rdma urgent irq\n");
}

static void dpu_hw_rdma_path_parse_info0(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	DPU_DUMP_REG("rdma_path", hw->blk_id, RDMA_PATH_INFO0_OFFSET);

	value = DPU_BIT_READ(hw, RDMA_PATH_INFO0_OFFSET, 0, 22);
	DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rcmd sent cnt is %d\n", value);
}

static void dpu_hw_rdma_path_parse_info1(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	DPU_DUMP_REG("rdma_path", hw->blk_id, RDMA_PATH_INFO1_OFFSET);

	value = DPU_BIT_READ(hw, RDMA_PATH_INFO1_OFFSET, 0, 22);
	DPU_DFX_INFO("rdma_path", hw->blk_id, s, "rdma urgent cmd sent cnt is %d\n", value);
}

void dpu_hw_rdma_path_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	dpu_hw_rdma_path_parse_dbg_irq(hw);
	dpu_hw_rdma_path_parse_info0(hw);
	dpu_hw_rdma_path_parse_info1(hw);
}

static void dpu_hw_rdma_top_parse_arb(struct dpu_hw_blk *hw, int axi_idx)
{
	u8 axi_rd_ostd_cnt;
	u8 axi_wr_ostd_cnt;
	u8 axi_wid_fifo_empty;
	u8 axi_wdata_fifo_empty;
	u32 offset;
	char s[128];

	if (axi_idx == 0)
		offset = ARB0_DBG_INFO_OFFSET;
	else
		offset = ARB1_DBG_INFO_OFFSET;

	DPU_DUMP_REG("rdma_top", hw->blk_id, offset);

	axi_rd_ostd_cnt = DPU_BIT_READ(hw, offset, 0, 8);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s,
			"axi%d read outstanding counter: %d\n", axi_idx, axi_rd_ostd_cnt);

	axi_wr_ostd_cnt = DPU_BIT_READ(hw, offset, 8, 8);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s,
			"axi%d write outstanding counter: %d\n", axi_idx, axi_wr_ostd_cnt);

	axi_wid_fifo_empty = DPU_BIT_READ(hw, offset, 16, 1);
	if (axi_wid_fifo_empty == 1)
		DPU_DFX_INFO("rdma_top", hw->blk_id, s, "axi%d wid fifo is empty\n", axi_idx);

	axi_wdata_fifo_empty = DPU_BIT_READ(hw, offset, 17, 1);
	if (axi_wdata_fifo_empty == 1)
		DPU_DFX_INFO("rdma_top", hw->blk_id, s, "axi%d wdata fifo is empty\n", axi_idx);
}

static void dpu_hw_rdma_top_parse_tpc(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_TOP_TPC_CMD_R_BYTE_COUNTER_ALL_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "TPC read bw is %d\n", value);

	value = DPU_REG_READ(hw, RDMA_TOP_TPC_CMD_CYCLE_COUNTER_ALL_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "TPC bw cycle is %d\n", value);
}

static void dpu_hw_rdma_top_parse_dbg_info0(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_PATH_DBG_INFO0_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "dbg info0 is %d\n", value);
}

static void dpu_hw_rdma_top_parse_dbg_info1(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_PATH_DBG_INFO1_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "dbg info1 is %d\n", value);
}

static void dpu_hw_rdma_top_parse_dbg_info2(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_PATH_DBG_INFO2_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "dbg info2 is %d\n", value);
}

static void dpu_hw_rdma_top_parse_dbg_info3(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_PATH_DBG_INFO3_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "dbg info3 is %d\n", value);
}

static void dpu_hw_rdma_top_parse_dbg_info5(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_PATH_DBG_INFO5_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "dbg info5 is %d\n", value);
}

static void dpu_hw_rdma_top_parse_dbg_info4(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, RDMA_PATH_DBG_INFO4_OFFSET);
	DPU_DFX_INFO("rdma_top", hw->blk_id, s, "dbg info4 is %d\n", value);
}

void dpu_hw_rdma_top_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	dpu_hw_rdma_top_parse_arb(hw, 0);
	dpu_hw_rdma_top_parse_arb(hw, 1);
	dpu_hw_rdma_top_parse_tpc(hw);
	dpu_hw_rdma_top_parse_dbg_info0(hw);
	dpu_hw_rdma_top_parse_dbg_info1(hw);
	dpu_hw_rdma_top_parse_dbg_info2(hw);
	dpu_hw_rdma_top_parse_dbg_info3(hw);
	dpu_hw_rdma_top_parse_dbg_info5(hw);
	dpu_hw_rdma_top_parse_dbg_info4(hw);
}

void dpu_hw_rdma_status_clear(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, RDMA_DBG_IRQ_STATUS_OFFSET, 0xFFFFFFFF, DIRECT_WRITE);
}

#if defined(CONFIG_XRING_DEBUG)
void dpu_hw_rdma_path_dump_to_file(struct dpu_hw_blk *hw, struct file *f)
{
	char buf[DUMP_REG_BUF_SIZE];
	u32 value;
	u32 offset;

	if (!hw || !f) {
		DPU_ERROR("nullptr!\n");
		return;
	}

	offset = LAYER_MODE_OFFSET;
	while (offset <= SW_QOS_EN_OFFSET) {
		value = DPU_REG_READ(hw, offset);
		sprintf(buf, "0x%08x:0x%08x\n", hw->base_addr + hw->blk_offset + offset, value);
		kernel_write(f, buf, sizeof(buf), &f->f_pos);
		offset += 0x4;
	}
}
#endif

void dpu_hw_rch_line_buf_level_config(struct dpu_hw_blk *hw, struct rdma_line_buf_level rdma_lbuf_level)
{
	u32 value = 0;

	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_up0, RDMA_QOS_UP0_SHIFT, RDMA_QOS_UP0_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_down0, RDMA_QOS_DOWN0_SHIFT, RDMA_QOS_DOWN0_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_up1, RDMA_QOS_UP1_SHIFT, RDMA_QOS_UP1_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_down1, RDMA_QOS_DOWN1_SHIFT, RDMA_QOS_DOWN1_LENGTH);
	DPU_REG_WRITE(hw, RDMA_QOS_UP0_OFFSET, value, CMDLIST_WRITE);

	value = 0;
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_up2, RDMA_QOS_UP2_SHIFT, RDMA_QOS_UP2_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_down2, RDMA_QOS_DOWN2_SHIFT, RDMA_QOS_DOWN2_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_up3, RDMA_QOS_UP3_SHIFT, RDMA_QOS_UP3_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_lvl_down3, RDMA_QOS_DOWN3_SHIFT, RDMA_QOS_DOWN3_LENGTH);
	DPU_REG_WRITE(hw, RDMA_QOS_UP2_OFFSET, value, CMDLIST_WRITE);

	value = 0;
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_urgent_up, RDMA_URGENT_UP_SHIFT, RDMA_URGENT_UP_LENGTH);
	value = MERGE_BITS(value, rdma_lbuf_level.rdma_urgent_down, RDMA_URGENT_DOWN_SHIFT, RDMA_URGENT_DOWN_LENGTH);
	DPU_REG_WRITE(hw, RDMA_URGENT_UP_OFFSET, value, CMDLIST_WRITE);

	value = 0;
	value = MERGE_BITS(value, rdma_lbuf_level.prefetch_line, OFFL_PREFETCH_LINES_SHIFT, OFFL_PREFETCH_LINES_LENGTH);
	DPU_REG_WRITE(hw, SW_QOS_EN_OFFSET, value, CMDLIST_WRITE);
}

void dpu_hw_rdma_path_dbg_irq_enable(struct dpu_hw_blk *hw, bool enable)
{
	u32 value;

	/* unmask below error irq:
	 * [1] tbu_size_err
	 * [2] mmu_rdma_timeout
	 * [3] mmu_rdma_resp_err
	 * [4] rdma timeout
	 * [5] rdma resp err
	 * [6] rdma memory conflict error
	 * [7] rdma memory size error
	 * [14] left layer afbc decode error
	 * [15] right layer afbc decode error
	 * [23] sw_mem_size_error
	 */
	if (enable) {
		value = 0x80C0FE;
		DPU_REG_WRITE(hw, RDMA_DBG_IRQ_MASK_OFFSET, value, CMDLIST_WRITE);
	} else {
		value = 0;
		DPU_REG_WRITE(hw, RDMA_DBG_IRQ_MASK_OFFSET, value, DIRECT_WRITE);
	}
}

void dpu_hw_rdma_path_dbg_irq_clear(struct dpu_hw_blk *hw)
{
	dpu_hw_rdma_status_clear(hw);
}

u32 dpu_hw_rdma_path_dbg_irq_dump(struct dpu_hw_blk *hw)
{
	return DPU_REG_READ(hw, RDMA_DBG_IRQ_RAW_OFFSET);
}

u32 dpu_hw_rdma_path_dbg_irq_status(struct dpu_hw_blk *hw)
{
	return DPU_REG_READ(hw, RDMA_DBG_IRQ_STATUS_OFFSET);
}
