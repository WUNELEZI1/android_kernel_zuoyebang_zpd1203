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

#include "dpu_hw_mixer_reg.h"
#include "dpu_hw_mixer_ops.h"

/* mixer max supported layer num */
#define MIXER_MAX_LAYER_NUM            16

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
#define LAYER_SOLID_COLOR_R_OFFSET(layer_zorder)                                   \
		(LAYER00_SOLID_COLOR_R_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_SOLID_COLOR_R_SHIFT      LAYER00_SOLID_COLOR_R_SHIFT
#define LAYER_SOLID_COLOR_R_LENGTH     LAYER00_SOLID_COLOR_R_LENGTH

/* the shift and length of each layer are equal */
#define LAYER_SOLID_COLOR_G_OFFSET(layer_zorder)                                   \
		(LAYER00_SOLID_COLOR_G_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_SOLID_COLOR_G_SHIFT      LAYER00_SOLID_COLOR_G_SHIFT
#define LAYER_SOLID_COLOR_G_LENGTH     LAYER00_SOLID_COLOR_G_LENGTH

/* the shift and length of each layer are equal */
#define LAYER_SOLID_COLOR_B_OFFSET(layer_zorder)                                   \
		(LAYER00_SOLID_COLOR_B_OFFSET + (layer_zorder * INCREASE_OFFSET))
#define LAYER_SOLID_COLOR_B_SHIFT      LAYER00_SOLID_COLOR_B_SHIFT
#define LAYER_SOLID_COLOR_B_LENGTH     LAYER00_SOLID_COLOR_B_LENGTH

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

/* the shift and length of each layer are equal */
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

#define GET_LOW_16BITS(value_u32)      (value_u32 & 0xffff)
#define GET_HIGH_16BITS(value_u32)     ((value_u32 >> 16) & 0xffff)

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

const char *to_blend_mode_str(enum dpu_blend_mode blend_mode)
{
	const char *dpu_blend_mode_name_list[3] = {
		"Premulti",
		"Coverage",
		"None",
	};

	return dpu_blend_mode_name_list[blend_mode];
}

static void dpu_hw_mixer_layer_area_setup(struct dpu_hw_blk *hw,
		u8 layer_zorder, struct dpu_rect_v2 *dst_pos_rect)
{
	u32 area_left_right;
	u32 area_top_bottom;
	u16 area_left;
	u16 area_right;
	u16 area_top;
	u16 area_bottom;

	area_left = dst_pos_rect->x;
	area_right = dst_pos_rect->x + dst_pos_rect->w - 1;
	area_top = dst_pos_rect->y;
	area_bottom = dst_pos_rect->y + dst_pos_rect->h - 1;

	/* merge left[0, 15] and right[16, 31], then write to register */
	area_left_right = area_left;
	area_left_right = MERGE_BITS(area_left_right, area_right,
			LAYER_AREA_RIGHT_SHIFT, LAYER_AREA_RIGHT_LENGTH);
	DPU_REG_WRITE(hw, LAYER_AREA_LEFT_OFFSET(layer_zorder), area_left_right, CMDLIST_WRITE);

	/* merge top[0, 15] and bottom[16, 31], then write to register */
	area_top_bottom = area_top;
	area_top_bottom = MERGE_BITS(area_top_bottom, area_bottom,
			LAYER_AREA_BOTTOM_SHIFT, LAYER_AREA_BOTTOM_LENGTH);
	DPU_REG_WRITE(hw, LAYER_AREA_TOP_OFFSET(layer_zorder), area_top_bottom, CMDLIST_WRITE);
}

static void dpu_hw_mixer_blend_to_hw_cfg(struct mixer_blend_cfg *blend_cfg,
		struct mixer_hw_blend_cfg *out_cfg)
{
	bool has_pixel_alpha;
	bool layer_alpha_valid;
	u8 hw_blend_cfg_id;

	has_pixel_alpha = blend_cfg->has_pixel_alpha;
	layer_alpha_valid = (blend_cfg->layer_alpha < 0xff) ? true : false;

	switch (blend_cfg->blend_mode) {
	case DPU_BLEND_MODE_PREMULTI:
		if (has_pixel_alpha)
			/* F1: PREMULTI & layer_alpha == 0 -> r = s + d, but dpu need r = d */
			hw_blend_cfg_id = (blend_cfg->layer_alpha == 0) ? DPU_BLEND_CFG4 :
					((layer_alpha_valid) ? DPU_BLEND_CFG10 : DPU_BLEND_SRC_OVER);
		else
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG10 :
					DPU_BLEND_SRC;
		break;
	case DPU_BLEND_MODE_COVERAGE:
		if (has_pixel_alpha)
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG4 :
					DPU_BLEND_CFG1;
		else
			hw_blend_cfg_id = (layer_alpha_valid) ? DPU_BLEND_CFG0 :
					DPU_BLEND_SRC;
		break;
	case DPU_BLEND_MODE_NONE:
		if (has_pixel_alpha)
			hw_blend_cfg_id = DPU_BLEND_SRC;
		else
			hw_blend_cfg_id = DPU_BLEND_CFG0;
		break;
	default:
		hw_blend_cfg_id = DPU_BLEND_SRC;
		break;
	}

	if (hw_blend_cfg_id >= DPU_BLEND_CFG_MAX) {
		DPU_ERROR("invalid hw_blend_cfg_id: %d\n", hw_blend_cfg_id);
		return;
	}

	out_cfg->alpha_mode = g_hw_blend_cfg[hw_blend_cfg_id].alpha_mode;
	out_cfg->blend_mode = g_hw_blend_cfg[hw_blend_cfg_id].blend_mode;

	switch (g_hw_blend_cfg[hw_blend_cfg_id].layer_alpha) {
	case ALPHA_FROM_USER:
		out_cfg->layer_alpha = blend_cfg->layer_alpha;
		break;
	case ALPHA_ZERO:
		out_cfg->layer_alpha = 0x0;
		break;
	case ALPHA_MAX:
	default:
		out_cfg->layer_alpha = 0xff;
		break;
	}
}

void dpu_hw_mixer_module_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, MODULE_ENABLE_OFFSET, enable, CMDLIST_WRITE);
}

void dpu_hw_mixer_dst_w_h_config(struct dpu_hw_blk *hw,
		u16 dst_w, u16 dst_h, bool cfg_method)
{
	u32 dst_w_h;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	/* merge width[0, 15] and height[16, 31], then write to register */
	dst_w_h = dst_w;
	dst_w_h = MERGE_BITS(dst_w_h, dst_h, DST_H_SHIFT, DST_H_LENGTH);
	DPU_REG_WRITE(hw, DST_W_OFFSET, dst_w_h, cfg_method);
}

void dpu_hw_mixer_bg_color_config(struct dpu_hw_blk *hw,
		struct mixer_color_cfg *bg_color, bool cfg_method)
{
	if (!hw || !bg_color) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, BG_COLOR_R_OFFSET, bg_color->r, cfg_method);
	DPU_REG_WRITE(hw, BG_COLOR_G_OFFSET, bg_color->g, cfg_method);
	DPU_REG_WRITE(hw, BG_COLOR_B_OFFSET, bg_color->b, cfg_method);
	DPU_REG_WRITE(hw, BG_COLOR_A_OFFSET, bg_color->a, cfg_method);
}

void dpu_hw_mixer_dma_layer_config(struct dpu_hw_blk *hw,
		struct mixer_dma_layer_cfg *cmps_dma_cfg)
{
	struct mixer_hw_blend_cfg hw_blend_cfg;
	u32 solid_color_reg_value;
	u32 layer_en_reg_value;
	u8 id;

	if (!hw || !cmps_dma_cfg) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	id = cmps_dma_cfg->layer_zorder;

	if (!cmps_dma_cfg->layer_en) {
		/* Disable the use of layer */
		DPU_REG_WRITE(hw, LAYER_EN_OFFSET(id), false, CMDLIST_WRITE);
		return;
	}

	/* set layer area: left/right/top/bottom */
	dpu_hw_mixer_layer_area_setup(hw, id, &cmps_dma_cfg->dst_pos_rect);

	/* upload hw layer blend:blend_mode/alpha_mode/layer_alpha */
	dpu_hw_mixer_blend_to_hw_cfg(&cmps_dma_cfg->blend_cfg, &hw_blend_cfg);

	/* --------------- solid_color_reg --------------- */
	solid_color_reg_value = 0;  /* register default value */

	/* merge layer blend mode: NORMAL, MULTIPLE, SPECIAL */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value, hw_blend_cfg.blend_mode,
			LAYER_BLEND_MODE_SHIFT, LAYER_BLEND_MODE_LENGTH);

	/* merge special alpha blend mode */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value, ALPHA_RATIO_DISABLE,
			LAYER_SPE_ALPHA_MODE_SHIFT, LAYER_SPE_ALPHA_MODE_LENGTH);

	DPU_REG_WRITE(hw, LAYER_SOLID_COLOR_A_OFFSET(id), solid_color_reg_value, CMDLIST_WRITE);

	/* --------------- layer_en_reg --------------- */
	layer_en_reg_value = 0;  /* register default value */

	/* enable the use of layer */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, true,
			LAYER_EN_SHIFT, LAYER_EN_LENGTH);

	/* merge alpha selection: LAYER, PIXEL, BOTH */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, hw_blend_cfg.alpha_mode,
			LAYER_BLEND_SEL_SHIFT, LAYER_BLEND_SEL_LENGTH);

	/* merge the RDMA ID of the layer */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, cmps_dma_cfg->rdma_id,
			LAYER_DMA_ID_SHIFT, LAYER_DMA_ID_LENGTH);

	/* when only use pixel alpha, don't merge layer alpha value */
	if (hw_blend_cfg.alpha_mode != ALPHA_MODE_PIXEL)
		layer_en_reg_value = MERGE_BITS(layer_en_reg_value, hw_blend_cfg.layer_alpha,
			LAYER_LAYER_ALPHA_SHIFT, LAYER_LAYER_ALPHA_LENGTH);

	DPU_REG_WRITE(hw, LAYER_EN_OFFSET(id), layer_en_reg_value, CMDLIST_WRITE);
}

void dpu_hw_mixer_solid_layer_config(struct dpu_hw_blk *hw,
		struct mixer_solid_layer_cfg *solid_cfg)
{
	struct mixer_hw_blend_cfg hw_blend_cfg;
	u32 solid_color_reg_value;
	u32 layer_en_reg_value;
	u8 id;

	if (!hw || !solid_cfg) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	id = solid_cfg->layer_zorder;

	if (!solid_cfg->layer_en) {
		/* disable the use of layer */
		DPU_REG_WRITE(hw, LAYER_EN_OFFSET(id), false, CMDLIST_WRITE);
		return;
	}

	/* set layer area: left/right/top/bottom */
	dpu_hw_mixer_layer_area_setup(hw, id, &solid_cfg->dst_pos_rect);

	/* set the R/G/B/ for solid */
	DPU_REG_WRITE(hw, LAYER_SOLID_COLOR_R_OFFSET(id), solid_cfg->color.r, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, LAYER_SOLID_COLOR_G_OFFSET(id), solid_cfg->color.g, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, LAYER_SOLID_COLOR_B_OFFSET(id), solid_cfg->color.b, CMDLIST_WRITE);

	/* upload hw layer blend:blend_mode/alpha_mode/layer_alpha */
	dpu_hw_mixer_blend_to_hw_cfg(&solid_cfg->blend_cfg, &hw_blend_cfg);

	/* --------------- solid_color_reg --------------- */
	solid_color_reg_value = 0;  /* register default value */

	/* merge the Alpha for solid */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value,
			solid_cfg->color.a, LAYER_SOLID_COLOR_A_SHIFT,
			LAYER_SOLID_COLOR_A_LENGTH);

	/* merge layer blend mode: NORMAL, MULTIPLE, SPECIAL */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value,
			hw_blend_cfg.blend_mode, LAYER_BLEND_MODE_SHIFT,
			LAYER_BLEND_MODE_LENGTH);

	/* merge the area_mode for solid */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value,
			solid_cfg->solid_area_mode, LAYER_SOLID_AREA_MODE_SHIFT,
			LAYER_SOLID_AREA_MODE_LENGTH);

	/* merge special alpha blend mode */
	solid_color_reg_value = MERGE_BITS(solid_color_reg_value, ALPHA_RATIO_DISABLE,
			LAYER_SPE_ALPHA_MODE_SHIFT, LAYER_SPE_ALPHA_MODE_LENGTH);

	DPU_REG_WRITE(hw, LAYER_SOLID_COLOR_A_OFFSET(id), solid_color_reg_value, CMDLIST_WRITE);

	/* --------------- layer_en_reg --------------- */
	layer_en_reg_value = 0;  /* register default value */

	/* enable the use of layer */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, true,
			LAYER_EN_SHIFT, LAYER_EN_LENGTH);

	/* merge alpha selection: LAYER, PIXEL, BOTH */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, hw_blend_cfg.alpha_mode,
			LAYER_BLEND_SEL_SHIFT, LAYER_BLEND_SEL_LENGTH);

	/* enable solid layer for using */
	layer_en_reg_value = MERGE_BITS(layer_en_reg_value, true,
			LAYER_SOLID_EN_SHIFT, LAYER_SOLID_EN_LENGTH);

	/* when only use pixel alpha, don't merge layer alpha value */
	if (hw_blend_cfg.alpha_mode != ALPHA_MODE_PIXEL)
		layer_en_reg_value = MERGE_BITS(layer_en_reg_value, hw_blend_cfg.layer_alpha,
			LAYER_LAYER_ALPHA_SHIFT, LAYER_LAYER_ALPHA_LENGTH);

	DPU_REG_WRITE(hw, LAYER_EN_OFFSET(id), layer_en_reg_value, CMDLIST_WRITE);
}

void dpu_hw_mixer_reset(struct dpu_hw_blk *hw, bool cfg_method)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, MIXER_TRIGGER2_OFFSET, REG_BANK_RESET_VAL, cfg_method);
}

void dpu_hw_mixer_status_dump(struct dpu_hw_blk *hw)
{
	char s[128] = {0};
	u32 value;
	u32 left_right_value;
	u32 top_bottom_value;
	u32 comps_width_height;
	u32 comps_proc_location;
	int i;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	comps_proc_location = DPU_REG_READ(hw, CMPS_PROC_LOCATION_OFFSET);

	for (i = 0; i < MIXER_MAX_LAYER_NUM; i++) {
		value = DPU_REG_READ(hw, LAYER_PROC_LOCATION_OFFSET + i * 4);
		if (!value)
			continue;

		/* left[0, 15] and right[16, 31] */
		left_right_value = DPU_REG_READ(hw, LAYER_AREA_LEFT_OFFSET(i));

		/* top[0, 15] and bottom[16, 31] */
		top_bottom_value = DPU_REG_READ(hw, LAYER_AREA_TOP_OFFSET(i));

		DPU_DFX_INFO("mixer", hw->blk_id, s,
				"layer_%d_ltrb(0x%x, 0x%x, 0x%x, 0x%x)_proc_location(0x%x)\n",
				i, GET_LOW_16BITS(left_right_value),
				GET_LOW_16BITS(top_bottom_value),
				GET_HIGH_16BITS(left_right_value),
				GET_HIGH_16BITS(top_bottom_value), value);
	}

	value = DPU_REG_READ(hw, CMPS_PROC_LOCATION_OFFSET);
	comps_width_height = DPU_REG_READ(hw, DST_W_OFFSET);
	DPU_DFX_INFO("mixer", hw->blk_id, s,
			"comps_wh(0x%x, 0x%x)_proc_location(0x%x)\n",
			GET_LOW_16BITS(comps_width_height),
			GET_HIGH_16BITS(comps_width_height), value);

	if (comps_proc_location != value)
		DPU_DFX_INFO("mixer", hw->blk_id, s, "mixer is working, pro_location(0x%x) -> (0x%x)",
				comps_proc_location, value);
}
