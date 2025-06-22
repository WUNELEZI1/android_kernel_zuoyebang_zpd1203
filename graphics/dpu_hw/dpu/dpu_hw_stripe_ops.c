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

#include "dpu_hw_stripe_ops.h"
#include "dpu_hw_mixer_ops.h"
#include "dpu_hw_rdma_path_reg.h"
#include "dpu_hw_mixer_reg.h"
#include "dpu_hw_wb_core_reg.h"
#include "dpu_hw_wb_scaler_reg.h"
#include "../color/hw_reg/dpu_hw_scaler_reg.h"

#define GET_CONTIG_REG_ADDR(base, index)    ((base) + (index) * 0x04)
#define GET_LAYER_REG_ADDR(base, index)    ((base) + (index) * 0x28)
#define GET_WRITE_VALUE(value, offset)    ((value) >> (offset))

#define RDMA_ROATION_LENGTH    (LEFT_ROT_MODE_LENGTH + LEFT_FLIP_MODE_LENGTH)
#define RDMA_FORMAT_LENGTH    8

#define RDMA_AFBC_ENABLE    1
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

/* calculate the increase offset between each layer register */
#define INCREASE_OFFSET (LAYER01_AREA_LEFT_OFFSET - LAYER00_AREA_LEFT_OFFSET)

/* the shift and length of each layer are equal */
#define LAYER_AREA_LEFT_OFFSET(layer_zorder)                                       \
		(LAYER00_AREA_LEFT_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_AREA_LEFT_SHIFT          LAYER00_AREA_LEFT_SHIFT
#define LAYER_AREA_LEFT_LENGTH         LAYER00_AREA_LEFT_LENGTH
#define LAYER_AREA_RIGHT_SHIFT         LAYER00_AREA_RIGHT_SHIFT
#define LAYER_AREA_RIGHT_LENGTH        LAYER00_AREA_RIGHT_LENGTH

/* the shift and length of each layer are equal */
#define LAYER_AREA_TOP_OFFSET(layer_zorder)                                        \
		(LAYER00_AREA_TOP_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_AREA_TOP_SHIFT           LAYER00_AREA_TOP_SHIFT
#define LAYER_AREA_TOP_LENGTH          LAYER00_AREA_TOP_LENGTH
#define LAYER_AREA_BOTTOM_SHIFT        LAYER00_AREA_BOTTOM_SHIFT
#define LAYER_AREA_BOTTOM_LENGTH       LAYER00_AREA_BOTTOM_LENGTH

/* the shift and length of each layer are equal */
#define LAYER_SOLID_COLOR_A_OFFSET(layer_zorder)                                   \
		(LAYER00_SOLID_COLOR_A_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_SOLID_COLOR_A_SHIFT      LAYER00_SOLID_COLOR_A_SHIFT
#define LAYER_SOLID_COLOR_A_LENGTH     LAYER00_SOLID_COLOR_A_LENGTH
#define LAYER_BLEND_MODE_SHIFT         LAYER00_BLEND_MODE_SHIFT
#define LAYER_BLEND_MODE_LENGTH        LAYER00_BLEND_MODE_LENGTH
#define LAYER_SOLID_AREA_MODE_SHIFT    LAYER00_SOLID_AREA_MODE_SHIFT
#define LAYER_SOLID_AREA_MODE_LENGTH   LAYER00_SOLID_AREA_MODE_LENGTH
#define LAYER_SPE_ALPHA_MODE_SHIFT     LAYER00_SPE_ALPHA_MODE_SHIFT
#define LAYER_SPE_ALPHA_MODE_LENGTH    LAYER00_SPE_ALPHA_MODE_LENGTH
#define LAYER_COLORKEY_EN_SHIFT        LAYER00_COLORKEY_EN_SHIFT
#define LAYER_COLORKEY_EN_LENGTH       LAYER00_COLORKEY_EN_LENGTH

#define LAYER_EN_OFFSET(layer_zorder)                                              \
		(LAYER00_EN_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_EN_SHIFT                 LAYER00_EN_SHIFT
#define LAYER_EN_LENGTH                LAYER00_EN_LENGTH
#define LAYER_BLEND_SEL_SHIFT          LAYER00_BLEND_SEL_SHIFT
#define LAYER_BLEND_SEL_LENGTH         LAYER00_BLEND_SEL_LENGTH
#define LAYER_SOLID_EN_SHIFT           LAYER00_SOLID_EN_SHIFT
#define LAYER_SOLID_EN_LENGTH          LAYER00_SOLID_EN_LENGTH
#define LAYER_DMA_ID_SHIFT             LAYER00_DMA_ID_SHIFT
#define LAYER_DMA_ID_LENGTH            LAYER00_DMA_ID_LENGTH
#define LAYER_LAYER_ALPHA_SHIFT        LAYER00_LAYER_ALPHA_SHIFT
#define LAYER_LAYER_ALPHA_LENGTH       LAYER00_LAYER_ALPHA_LENGTH
#define LAYER_ALPHA_RATIO_SHIFT        LAYER00_ALPHA_RATIO_SHIFT
#define LAYER_ALPHA_RATIO_LENGTH       LAYER00_ALPHA_RATIO_LENGTH

/**
 * mixer_alpha_ratio_mode - set alpha ratio validity
 * @ALPHA_RATIO_ENABLE: enable use alpha_ratio under normal blend mode
 * @ALPHA_RATIO_DISABLE: disable use alpha_ratio under normal blend mode
 */
enum mixer_alpha_ratio_mode {
	ALPHA_RATIO_ENABLE = 0,
	ALPHA_RATIO_DISABLE = 1,
};

/**
 * mixer_alpha_mode - mixer alpha mode object
 * @ALPHA_MODE_LAYER: using layer alpha
 * @ALPHA_MODE_PIXEL: using pixel alpha
 * @ALPHA_MODE_BOTH: using layer & pixel alpha
 */
enum mixer_alpha_mode {
	ALPHA_MODE_LAYER = 0,
	ALPHA_MODE_PIXEL = 1,
	ALPHA_MODE_BOTH = 2,
};

/**
 * mixer_blend_mode - mixer blend mode object
 * @BLEND_MODE_NORMAL: normal blend mode
 * @BLEND_MODE_PREMULTI: pre_multiply blend mode
 * @BLEND_MODE_SPECIAL: special blend mode
 */
enum mixer_blend_mode {
	BLEND_MODE_NORMAL = 0,
	BLEND_MODE_PREMULTI = 1,
	BLEND_MODE_SPECIAL = 2,
};

/**
 * mixer_layer_alpha_use_mode - layer alpha use mode
 * @ALPHA_FROM_USER: set layer alpha by user: struct mixer_blend_cfg-layer_alpha
 * @ALPHA_MAX: set the layer alpha value to the 0xff
 * @ALPHA_ZERO: set the layer alpha value to 0
 * @ALPHA_NO_USER: don't set layer alpha
 */
enum mixer_layer_alpha_use_mode {
    ALPHA_FROM_USER = 0,
    ALPHA_MAX = 1,
    ALPHA_ZERO = 2,
    ALPHA_NO_USER = 3,
};

/**
 * mixer_hw_blend_cfg - mixer hardware blend mode configuration object
 * @blend_mode: composer blend mode
 * @alpha_mode: composer alpha mode
 * @layer_alpha: layer alpha value,
 */
struct mixer_hw_blend_cfg {
	u8 blend_mode;
	u8 alpha_mode;
	int8_t layer_alpha;
};

/**
 * mixer_hw_blend_cfg_id - select a configuration in g_hw_blend_cfg
 */
enum mixer_hw_blend_cfg_id {
	DPU_BLEND_SRC = 0,   /*  r = s                      */
	DPU_BLEND_DST,       /*  r = d                      */
	DPU_BLEND_SRC_OVER,  /*  r = s + d*(1-sa)           */
	DPU_BLEND_CFG0,      /*  r = s*la + d*(1-la)        */
	DPU_BLEND_CFG1,      /*  r = s*sa + d*(1-sa)        */
	DPU_BLEND_CFG2,      /*  r = s*sa + d*(1-sa)        */
	DPU_BLEND_CFG3,      /*  r = d                      */
	DPU_BLEND_CFG4,      /*  r = s*sa*la + d*(1-sa*la)  */
	DPU_BLEND_CFG5,      /*  r = s                      */
	DPU_BLEND_CFG6,      /*  r = s + d                  */
	DPU_BLEND_CFG7,      /*  r = s + d*(1-la)           */
	DPU_BLEND_CFG8,      /*  r = s + d*(1-sa)           */
	DPU_BLEND_CFG9,      /*  r = s + d                  */
	DPU_BLEND_CFG10,     /*  r = s*la + d*(1 - sa*la)   */
	DPU_BLEND_CFG11,     /*  r = s + d                  */
	DPU_BLEND_CFG12,     /*  r = s                      */
	DPU_BLEND_CFG13,     /*  r = s + d*la               */
	DPU_BLEND_CFG14,     /*  r = s + d*sa               */
	DPU_BLEND_CFG15,     /*  r = s + d*sa               */
	DPU_BLEND_CFG16,     /*  r = s                      */
	DPU_BLEND_CFG17,     /*  r = s + d*sa*la            */
	DPU_BLEND_CFG_MAX,
};

static struct mixer_hw_blend_cfg g_hw_blend_cfg[DPU_BLEND_CFG_MAX] = {
	{BLEND_MODE_NORMAL, ALPHA_MODE_LAYER, ALPHA_MAX},         /* DPU_BLEND_SRC */
	{BLEND_MODE_NORMAL, ALPHA_MODE_LAYER, ALPHA_ZERO},        /* DPU_BLEND_DST */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_BOTH, ALPHA_MAX},        /* DPU_BLEND_SRC_OVER */
	{BLEND_MODE_NORMAL, ALPHA_MODE_LAYER, ALPHA_FROM_USER},   /* DPU_BLEND_CFG0 */
	{BLEND_MODE_NORMAL, ALPHA_MODE_PIXEL, ALPHA_NO_USER},     /* DPU_BLEND_CFG1 */
	{BLEND_MODE_NORMAL, ALPHA_MODE_BOTH, ALPHA_MAX},          /* DPU_BLEND_CFG2 */
	{BLEND_MODE_NORMAL, ALPHA_MODE_BOTH, ALPHA_ZERO},         /* DPU_BLEND_CFG3 */
	{BLEND_MODE_NORMAL, ALPHA_MODE_BOTH, ALPHA_FROM_USER},    /* DPU_BLEND_CFG4 */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_LAYER, ALPHA_MAX},       /* DPU_BLEND_CFG5 */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_LAYER, ALPHA_ZERO},      /* DPU_BLEND_CFG6 */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_LAYER, ALPHA_FROM_USER}, /* DPU_BLEND_CFG7 */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_PIXEL, ALPHA_NO_USER},   /* DPU_BLEND_CFG8 */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_BOTH, ALPHA_ZERO},       /* DPU_BLEND_CFG9 */
	{BLEND_MODE_PREMULTI, ALPHA_MODE_BOTH, ALPHA_FROM_USER},  /* DPU_BLEND_CFG10 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_LAYER, ALPHA_MAX},        /* DPU_BLEND_CFG11 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_LAYER, ALPHA_ZERO},       /* DPU_BLEND_CFG12 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_LAYER, ALPHA_FROM_USER},  /* DPU_BLEND_CFG13 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_PIXEL, ALPHA_NO_USER},    /* DPU_BLEND_CFG14 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_BOTH, ALPHA_MAX},         /* DPU_BLEND_CFG15 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_BOTH, ALPHA_ZERO},        /* DPU_BLEND_CFG16 */
	{BLEND_MODE_SPECIAL, ALPHA_MODE_BOTH, ALPHA_FROM_USER},   /* DPU_BLEND_CFG17 */
};

static void __conf_rdma_stripe_crop_area(struct dpu_hw_blk *hw, s64 node_id,
		struct rdma_layer_stripe_cfg *layer_cfg, u8 reg_index)
{
	u32 offset = 0;
	u32 value;

	if (!hw || !layer_cfg) {
		DPU_ERROR("input dpu_hw_blk or rdma_layer_stripe_cfg was invalid parameter\n");
		return;
	}

	offset = GET_LAYER_REG_ADDR(LEFT_BBOX_START_X_OFFSET, reg_index);
	value = layer_cfg->crop_area.x1;
	value = MERGE_BITS(value, layer_cfg->crop_area.x2,
		LEFT_BBOX_END_X_SHIFT, LEFT_BBOX_END_X_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, offset, value);

	offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
	value = layer_cfg->crop_area.y1;
	value = MERGE_BITS(value, layer_cfg->crop_area.y2,
		LEFT_BBOX_END_Y_SHIFT, LEFT_BBOX_END_Y_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, offset, value);
}

static void __conf_rdma_stripe_exclusive_roi_area(struct dpu_hw_blk *hw,
		s64 node_id, struct rdma_layer_stripe_cfg *layer_cfg, enum rdma_extender_mode mode)
{
	u8 roi_ena = 0;
	u32 offset = 0;
	u8 shift = 0;
	u32 value = 0;

	if (!hw || !layer_cfg) {
		DPU_ERROR("input dpu_hw_blk or rdma_layer_stripe_cfg was invalid parameter\n");
		return;
	}

	switch (mode) {
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
		DPU_ERROR("invalid extender_mode: %d\n", mode);
		return;
	}

	if (layer_cfg->roi0_en) {
		roi_ena = MERGE_BITS(roi_ena, BIT_ENABLE,
			LEFT_EXT_ROI0_ENA_SHIFT, LEFT_EXT_ROI0_ENA_LENGTH);

		value = layer_cfg->roi0_area.x1;
		value = MERGE_BITS(value, layer_cfg->roi0_area.x2,
			LEFT_EXT_ROI0_END_X_SHIFT,
			LEFT_EXT_ROI0_END_X_LENGTH);
		DPU_STRIPE_REG_WRITE(hw, node_id, offset, value);

		value = layer_cfg->roi0_area.y1;
		value = MERGE_BITS(value, layer_cfg->roi0_area.y2,
			LEFT_EXT_ROI0_END_Y_SHIFT,
			LEFT_EXT_ROI0_END_Y_LENGTH);
		offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
		DPU_STRIPE_REG_WRITE(hw, node_id, offset, value);
	}

	if (layer_cfg->roi1_en) {
		roi_ena = MERGE_BITS(roi_ena, BIT_ENABLE,
			LEFT_EXT_ROI1_ENA_SHIFT, LEFT_EXT_ROI1_ENA_LENGTH);

		value = layer_cfg->roi1_area.x1;
		value = MERGE_BITS(value, layer_cfg->roi1_area.x2,
			LEFT_EXT_ROI1_END_X_SHIFT,
			LEFT_EXT_ROI1_END_X_LENGTH);
		offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
		DPU_STRIPE_REG_WRITE(hw, node_id, offset, value);

		value = layer_cfg->roi1_area.y1;
		value = MERGE_BITS(value, layer_cfg->roi1_area.y2,
			LEFT_EXT_ROI1_END_Y_SHIFT,
			LEFT_EXT_ROI1_END_Y_LENGTH);
		offset = GET_CONTIG_REG_ADDR(offset, NEXT_ONE_REG);
		DPU_STRIPE_REG_WRITE(hw, node_id, offset, value);
	}

	value = 0;
	value = MERGE_BITS(value, roi_ena, shift,
		SIGNLE_LAYER_ROI_ENA_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, LEFT_EXT_ROI0_ENA_OFFSET, value);
}

void dpu_hw_stripe_rdma_layer_config(struct dpu_hw_blk *hw, s64 node_id,
		struct rdma_layer_stripe_cfg *layer_cfg, enum rdma_extender_mode mode)
{
	u8 index;

	if (!hw || !layer_cfg) {
		DPU_ERROR("input dpu_hw_blk or rdma_layer_stripe_cfg was invalid parameter\n");
		return;
	}

	index = (mode == EXTENDER_MODE_RIGHT) ?
		RIGHT_LAYER : LEFT_LAYER;

	__conf_rdma_stripe_crop_area(hw, node_id, layer_cfg, index);

	if(layer_cfg->roi0_en)
		__conf_rdma_stripe_exclusive_roi_area(hw, node_id, layer_cfg, mode);
}

void dpu_hw_stripe_pre_scaler_set(struct dpu_hw_blk *hw,
		s64 node_id, struct dpu_2d_scaler_cfg *hw_cfg)
{
	u32 val = 0;
	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	val = MERGE_BITS(val, hw_cfg->stripe_height_in, STRIPE_HEIGHT_IN_SHIFT,
			STRIPE_HEIGHT_IN_LENGTH);
	val = MERGE_BITS(val, hw_cfg->stripe_width_in, STRIPE_WIDTH_IN_SHIFT,
			STRIPE_WIDTH_IN_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, STRIPE_HEIGHT_IN_OFFSET, val);

	val = 0;
	val = MERGE_BITS(val, hw_cfg->stripe_height_out, STRIPE_HEIGHT_OUT_SHIFT,
			STRIPE_HEIGHT_OUT_LENGTH);
	val = MERGE_BITS(val, hw_cfg->stripe_width_out, STRIPE_WIDTH_OUT_SHIFT,
			STRIPE_WIDTH_OUT_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, STRIPE_HEIGHT_OUT_OFFSET, val);

	val = 0;
	val = MERGE_BITS(val, hw_cfg->y_step, Y_STEP_SHIFT, Y_STEP_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, Y_STEP_OFFSET, val);

	val = 0;
	val = MERGE_BITS(val, hw_cfg->x_step, X_STEP_SHIFT, X_STEP_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, X_STEP_OFFSET, val);

	val = 0;
	val = MERGE_BITS(val, hw_cfg->stripe_init_phase,
			STRIPE_INIT_PHASE_SHIFT, STRIPE_INIT_PHASE_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, STRIPE_INIT_PHASE_OFFSET, val);
}

void dpu_hw_stripe_mixer_wb_input_config(struct dpu_hw_blk *hw, s64 node_id,
		u16 w, u16 h)
{
	u32 dst_w_h;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	dst_w_h = 0;
	dst_w_h = MERGE_BITS(dst_w_h, w, DST_W_SHIFT, DST_W_LENGTH);
	dst_w_h = MERGE_BITS(dst_w_h, h, DST_H_SHIFT, DST_H_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, DST_W_OFFSET, dst_w_h);
}

static void __dpu_hw_stripe_mixer_dma_layer_config(struct dpu_hw_blk *hw, s64 node_id,
		struct dpu_rect_v2 *area_cfg, u32 zorder)
{
	u32 area_left_right;
	u32 area_top_bottom;
	u16 area_left;
	u16 area_right;
	u16 area_top;
	u16 area_bottom;

	if (!hw || !area_cfg) {
		DPU_ERROR("input dpu_hw_blk or dpu_rect_v2 was invalid parameter\n");
		return;
	}

	area_left = area_cfg->x;
	area_right = (area_cfg->w != 0) ?
		(area_cfg->x + area_cfg->w - 1) : area_cfg->x;
	area_top = area_cfg->y;
	area_bottom = (area_cfg->h != 0) ?
		(area_cfg->y + area_cfg->h - 1) : area_cfg->y;

	/* merge left and right, then write to register */
	area_left_right = 0;
	area_left_right = MERGE_BITS(area_left_right, area_left,
			LAYER_AREA_LEFT_SHIFT, LAYER_AREA_LEFT_LENGTH);
	area_left_right = MERGE_BITS(area_left_right, area_right,
			LAYER_AREA_RIGHT_SHIFT, LAYER_AREA_RIGHT_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, LAYER_AREA_LEFT_OFFSET(zorder), area_left_right);

	/* merge top and bottom, then write to register */
	area_top_bottom = 0;
	area_top_bottom = MERGE_BITS(area_top_bottom, area_top,
			LAYER_AREA_TOP_SHIFT, LAYER_AREA_TOP_LENGTH);
	area_top_bottom = MERGE_BITS(area_top_bottom, area_bottom,
			LAYER_AREA_BOTTOM_SHIFT, LAYER_AREA_BOTTOM_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, LAYER_AREA_TOP_OFFSET(zorder), area_top_bottom);
}

void dpu_hw_stripe_mixer_rch_layer_config(struct dpu_hw_blk *hw, s64 node_id,
		struct mixer_blend_cfg *blend_cfg, struct dpu_rect_v2 *area_cfg,
		u32 zorder, u32 dma_id)
{
	struct mixer_hw_blend_cfg *hw_blend_mode_cfg;
	enum mixer_alpha_mode alpha_mode;
	enum mixer_blend_mode blend_mode;
	u32 solid_color_reg_value;
	u32 layer_en_reg_value;
	bool layer_alpha_valid;
	bool has_pixel_alpha;
	u8 hw_blend_cfg_id;
	u8 layer_alpha;
	u16 area_left;
	u16 area_right;
	u16 area_top;
	u16 area_bottom;

	if (!hw || !blend_cfg || !area_cfg) {
		DPU_ERROR("input has invalid parameter\n");
		return;
	}

	has_pixel_alpha = blend_cfg->has_pixel_alpha;
	layer_alpha_valid = (blend_cfg->layer_alpha < 0xff) ? true : false;

	switch (blend_cfg->blend_mode) {
	case DPU_BLEND_MODE_PREMULTI:
		if (has_pixel_alpha)
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG10 :
					DPU_BLEND_SRC_OVER;
		else
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG7 :
					DPU_BLEND_SRC;
		break;
	case DPU_BLEND_MODE_COVERAGE:
	case DPU_BLEND_MODE_NONE:
		if (has_pixel_alpha)
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG4 :
					DPU_BLEND_CFG1;
		else
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG0 :
					DPU_BLEND_SRC;
		break;
	default:
		hw_blend_cfg_id = DPU_BLEND_SRC;
		break;
	}

	if (hw_blend_cfg_id >= DPU_BLEND_CFG_MAX) {
		DPU_ERROR("invalid hw_blend_cfg_id: %d\n", hw_blend_cfg_id);
		return;
	}

	hw_blend_mode_cfg = &g_hw_blend_cfg[hw_blend_cfg_id];
	blend_mode = hw_blend_mode_cfg->blend_mode;
	alpha_mode = hw_blend_mode_cfg->alpha_mode;

	switch (hw_blend_mode_cfg->layer_alpha) {
	case ALPHA_FROM_USER:
		layer_alpha = blend_cfg->layer_alpha;
		break;
	case ALPHA_ZERO:
		layer_alpha = 0x0;
		break;
	case ALPHA_MAX:
	default:
		layer_alpha = 0xff;
		break;
	}

	/**
	 * set layer blend mode: BLEND_MODE_NORMAL, BLEND_MODE_PRE_MULTIPLE,
	 * BLEND_MODE_SPECIAL
	 */
	solid_color_reg_value = 0;  /* register default value */

	/* merge layer blend mode: NORMAL, MULTIPLE, SPECIAL */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value, blend_mode,
			LAYER_BLEND_MODE_SHIFT, LAYER_BLEND_MODE_LENGTH);

	/* merge special alpha blend mode */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value, ALPHA_RATIO_DISABLE,
			LAYER_SPE_ALPHA_MODE_SHIFT, LAYER_SPE_ALPHA_MODE_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, LAYER_SOLID_COLOR_A_OFFSET(zorder),
				solid_color_reg_value);


	area_left = area_cfg->x;
	area_right = (area_cfg->w != 0) ?
		(area_cfg->x + area_cfg->w - 1) : area_cfg->x;
	area_top = area_cfg->y;
	area_bottom = (area_cfg->h != 0) ?
		(area_cfg->y + area_cfg->h - 1) : area_cfg->y;

	if (area_top == area_bottom || area_left == area_right) {
		DPU_STRIPE_REG_WRITE(hw, node_id, LAYER_EN_OFFSET(zorder), false);
		return;
	} else {
		/* enable the use of layer */
		layer_en_reg_value = 0;
		layer_en_reg_value = MERGE_BITS(layer_en_reg_value, true,
			LAYER_EN_SHIFT, LAYER_EN_LENGTH);
	}

	/* merge alpha selection: LAYER, PIXEL, BOTH */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, alpha_mode,
			LAYER_BLEND_SEL_SHIFT, LAYER_BLEND_SEL_LENGTH);

	/* merge the solid enable of the layer */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, false,
			LAYER_SOLID_EN_SHIFT, LAYER_SOLID_EN_LENGTH);

	/* merge the RDMA ID of the layer */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, dma_id,
			LAYER_DMA_ID_SHIFT, LAYER_DMA_ID_LENGTH);

	/* when only use pixel alpha, don't merge layer alpha value */
	if (hw_blend_mode_cfg->alpha_mode != ALPHA_MODE_PIXEL)
		layer_en_reg_value = MERGE_BITS(layer_en_reg_value, layer_alpha,
			LAYER_LAYER_ALPHA_SHIFT, LAYER_LAYER_ALPHA_LENGTH);

	DPU_STRIPE_REG_WRITE(hw, node_id, LAYER_EN_OFFSET(zorder), layer_en_reg_value);

	__dpu_hw_stripe_mixer_dma_layer_config(hw, node_id, area_cfg, zorder);
}

static void __conf_stripe_wb_top(struct dpu_hw_blk *hw, s64 node_id,
		struct wb_stripe_frame_cfg *cfg)
{
	u32 value = 0;

	if (!hw || !cfg) {
		DPU_ERROR("input dpu_hw_blk or wb_stripe_frame_cfg was invalid parameter\n");
		return;
	}
	value = MERGE_BITS(value, cfg->scl_en,WB_SCL_EN_SHIFT,
				WB_SCL_EN_LENGTH);
	value = MERGE_BITS(value, cfg->is_offline_mode, WB_OFFLINE_EN_SHIFT,
				WB_OFFLINE_EN_LENGTH);
	value = MERGE_BITS(value, cfg->frame_rot_mode, WB_ROT_MODE_SHIFT,
				WB_ROT_MODE_LENGTH);
	value = MERGE_BITS(value, cfg->frame_flip_mode, WB_FLIP_MODE_SHIFT,
				WB_FLIP_MODE_LENGTH);
	value = MERGE_BITS(value, cfg->output_format, WB_OUT_FORMAT_SHIFT,
				WB_OUT_FORMAT_LENGTH);
	value = MERGE_BITS(value, cfg->dither_en, WB_DITHER_EN_SHIFT,
				WB_DITHER_EN_LENGTH);
	value = MERGE_BITS(value, cfg->crop0_en, WB_CROP0_EN_SHIFT,
				WB_CROP0_EN_LENGTH);
	value = MERGE_BITS(value, cfg->crop1_en, WB_CROP1_EN_SHIFT,
				WB_CROP1_EN_LENGTH);
	value = MERGE_BITS(value, cfg->crop2_en, WB_CROP2_EN_SHIFT,
				WB_CROP2_EN_LENGTH);
	value = MERGE_BITS(value, cfg->is_afbc, WB_WDMA_FBC_EN_SHIFT,
				WB_WDMA_FBC_EN_LENGTH);


	DPU_STRIPE_REG_WRITE(hw, node_id, WB_WDMA_FBC_EN_OFFSET, value);
}

static void __dpu_stripe_hw_wb_input_setup(struct dpu_hw_blk *hw, s64 node_id,
		struct wb_stripe_frame_cfg *cfg)
{
	u32 val;

	if (!hw || !cfg) {
		DPU_ERROR("input dpu_hw_blk or wb_stripe_frame_cfg was invalid parameter\n");
		return;
	}

	__conf_stripe_wb_top(hw, node_id, cfg);
	val = MERGE_BITS(cfg->input_width, cfg->input_height,
			WB_IN_HEIGHT_SHIFT, WB_IN_HEIGHT_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, WB_IN_WIDTH_OFFSET, val);
}

static void __conf_stripe_wb_output_subframe_area(struct dpu_hw_blk *hw, s64 node_id,
		struct wb_stripe_frame_cfg *cfg)
{
	u32 value;

	if (!hw || !cfg) {
		DPU_ERROR("input dpu_hw_blk or wb_stripe_frame_cfg was invalid parameter\n");
		return;
	}

	value = MERGE_BITS(cfg->output_subframe_rect.x,
			cfg->output_subframe_rect.y,
			WB_OUT_SUBFRAME_LTOPY_SHIFT,
			WB_OUT_SUBFRAME_LTOPY_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, WB_OUT_SUBFRAME_LTOPX_OFFSET, value);

	value = MERGE_BITS(cfg->output_subframe_rect.w,
			cfg->output_subframe_rect.h,
			WB_OUT_SUBFRAME_HEIGHT_SHIFT,
			WB_OUT_SUBFRAME_HEIGHT_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, WB_OUT_SUBFRAME_WIDTH_OFFSET, value);
}

static void __conf_stripe_wb_crop_area(struct dpu_hw_blk *hw, s64 node_id,
		struct wb_stripe_frame_cfg *cfg)
{
	u32 value;

	if (!hw || !cfg) {
		DPU_ERROR("input dpu_hw_blk or wb_stripe_frame_cfg was invalid parameter\n");
		return;
	}

	if (cfg->crop0_en) {
		value = cfg->crop0_area.x1;
		value = MERGE_BITS(value, cfg->crop0_area.y1, WB_CROP0_LTOPY_SHIFT,
			WB_CROP0_LTOPY_LENGTH);
			DPU_STRIPE_REG_WRITE(hw, node_id, WB_CROP0_LTOPX_OFFSET, value);

		value = cfg->crop0_area.x2;
		value = MERGE_BITS(value, cfg->crop0_area.y2, WB_CROP0_RBOTY_SHIFT,
			WB_CROP0_RBOTY_LENGTH);
			DPU_STRIPE_REG_WRITE(hw, node_id, WB_CROP0_RBOTX_OFFSET, value);
	}

	if (cfg->crop1_en) {
		value = cfg->crop1_area.x1;
		value = MERGE_BITS(value, cfg->crop1_area.y1, WB_CROP1_LTOPY_SHIFT,
			WB_CROP1_LTOPY_LENGTH);
		DPU_STRIPE_REG_WRITE(hw, node_id, WB_CROP1_LTOPX_OFFSET, value);

		value = cfg->crop1_area.x2;
		value = MERGE_BITS(value, cfg->crop1_area.y2, WB_CROP1_RBOTY_SHIFT,
			WB_CROP1_RBOTY_LENGTH);
		DPU_STRIPE_REG_WRITE(hw, node_id, WB_CROP1_RBOTX_OFFSET, value);
	}

	if (cfg->crop2_en) {
		value = cfg->crop2_area.x1;
		value = MERGE_BITS(value, cfg->crop2_area.y1, WB_CROP2_LTOPY_SHIFT,
			WB_CROP2_LTOPY_LENGTH);
		DPU_STRIPE_REG_WRITE(hw, node_id, WB_CROP2_LTOPX_OFFSET, value);

		value = cfg->crop2_area.x2;
		value = MERGE_BITS(value, cfg->crop2_area.y2, WB_CROP2_RBOTY_SHIFT,
			WB_CROP2_RBOTY_LENGTH);
		DPU_STRIPE_REG_WRITE(hw, node_id, WB_CROP2_RBOTX_OFFSET, value);
	}
}

static void __dpu_stripe_hw_wb_output_setup(struct dpu_hw_blk *hw, s64 node_id,
		struct wb_stripe_frame_cfg *cfg)
{
	__conf_stripe_wb_output_subframe_area(hw, node_id, cfg);

	__conf_stripe_wb_crop_area(hw, node_id, cfg);
}

void dpu_stripe_hw_wb_cfg_setup(struct dpu_hw_blk *hw, s64 node_id, struct wb_stripe_frame_cfg *cfg)
{
	if (!hw || !cfg) {
		DPU_ERROR("input dpu_hw_blk or wb_frame_cfg was invalid parameter\n");
		return;
	}

	__dpu_stripe_hw_wb_input_setup(hw, node_id, cfg);

	__dpu_stripe_hw_wb_output_setup(hw, node_id, cfg);
}

void dpu_hw_stripe_wb_scaler_set(struct dpu_hw_blk *hw, s64 node_id,
		struct dpu_1d_scaler_cfg *scaler)
{
	u32 value;

	if (!hw || !scaler) {
		DPU_ERROR("input dpu_hw_blk or dpu_1d_scaler_cfg was invalid parameter\n");
		return;
	}

	DPU_DEBUG("scaler->input_width:%d, scaler->input_height:%d\n",
			scaler->input_width, scaler->input_height);
	DPU_DEBUG("scaler->output_width:%d, scaler->output_height):%d\n",
			scaler->output_width, scaler->output_height);

	value = MERGE_BITS(scaler->input_width, scaler->input_height,
			M_NSCL_INPUT_HEIGHT_SHIFT, M_NSCL_INPUT_HEIGHT_LENGTH);

	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_INPUT_WIDTH_OFFSET, value);

	value = MERGE_BITS(scaler->output_width, scaler->output_height,
			M_NSCL_OUTPUT_HEIGHT_SHIFT, M_NSCL_OUTPUT_HEIGHT_LENGTH);

	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_OUTPUT_WIDTH_OFFSET, value);

	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_HOR_INIT_PHASE_L32B_OFFSET,
			scaler->hor_init_phase_l32b);

	value = 0;
	value = MERGE_BITS(value, scaler->hor_init_phase_h1b, M_NSCL_HOR_INIT_PHASE_H1B_SHIFT,
				M_NSCL_HOR_INIT_PHASE_H1B_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_HOR_INIT_PHASE_H1B_OFFSET, value);

	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_VER_INIT_PHASE_L32B_OFFSET,
			scaler->ver_init_phase_l32b);

	value = 0;
	value = MERGE_BITS(value, scaler->ver_init_phase_h1b, M_NSCL_VER_INIT_PHASE_H1B_SHIFT,
				M_NSCL_VER_INIT_PHASE_H1B_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_VER_INIT_PHASE_H1B_OFFSET, value);

	value = 0;
	value = MERGE_BITS(value, scaler->hor_delta_phase, M_NSCL_HOR_DELTA_PHASE_SHIFT,
				M_NSCL_HOR_DELTA_PHASE_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_HOR_DELTA_PHASE_OFFSET, value);

	value = 0;
	value = MERGE_BITS(value, scaler->ver_delta_phase, M_NSCL_VER_DELTA_PHASE_SHIFT,
				M_NSCL_VER_DELTA_PHASE_LENGTH);
	DPU_STRIPE_REG_WRITE(hw, node_id, M_NSCL_VER_DELTA_PHASE_OFFSET, value);

	DPU_STRIPE_REG_WRITE(hw, node_id, SCALER_8X6TAP_FORCE_UPDATE_EN_OFFSET, 0x2);
}
