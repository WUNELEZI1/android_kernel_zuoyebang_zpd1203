/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_HW_MIXER_OPS_H_
#define _DPU_HW_MIXER_OPS_H_

#include "dpu_hw_common.h"

/* the index of dpu mixer */
enum mixer_hw_id {
	MIXER_HW_0 = 0,
	MIXER_HW_1 = 1,
	MIXER_HW_2 = 2,
};

/**
 * dpu_blend_mode - dpu blend mode object
 * @DPU_BLEND_MODE_PREMULTI: This Layer has already been pre multiplied
 * @DPU_BLEND_MODE_COVERAGE: This layer is not pre multiplied
 * @DPU_BLEND_MODE_NONE: Do not mix, directly cover
 * @DPU_BLEND_MODE_MAX: object max value
 *
 * Attention: below enum values are same as DRM_MODE_BLEND_* of drm_blend.h
 */
enum dpu_blend_mode {
	DPU_BLEND_MODE_PREMULTI = 0,
	DPU_BLEND_MODE_COVERAGE = 1,
	DPU_BLEND_MODE_NONE = 2,
	DPU_BLEND_MODE_MAX = 3,
};

/**
 * mixer_solid_area_mode - mixer solid_cfg area object
 * @SOLID_AREA_INSIDE: inside the area of solid_cfg
 * @SOLID_AREA_OUTSIDE: outside the area of solid_cfg
 * @SOLID_AREA_MAX: object max value
 */
enum mixer_solid_area_mode {
	SOLID_AREA_INSIDE = 0,
	SOLID_AREA_OUTSIDE = 1,
	SOLID_AREA_MAX = 2,
};


/**
 * mixer_color_cfg - mixer color config object
 * @r: color red, 10 valid bits
 * @g: color green, 10 valid bits
 * @b: color blue, 10 valid bits
 * @a: color alpha, 8 valid bits
 */
struct mixer_color_cfg {
	u32 r : 10;
	u32 g : 10;
	u32 b : 10;
	u8 a;
};

/**
 * mixer_blend_cfg - mixer blend config object
 * @layer_alpha: layer alpha, 0 <= layer_alpha <= 0xff
 * @has_pixel_alpha: does the layer data has pixel alpha
 * @blend_mode: dpu blend mode object
*/
struct mixer_blend_cfg {
	u8 layer_alpha;
	bool has_pixel_alpha;
	enum dpu_blend_mode blend_mode;
};

/**
 * mixer_dma_layer_cfg - when mixer uses RDMA input，layer config object
 * @layer_zorder: the id value of the layer, 0 <= layer_zorder <= 15
 * @rdma_id: the id value of the RDMA, 0 <= rdma_id <= 9
 * @dst_pos_rect: The position of the layer on the dst, x/y/w/h
 * @blend_cfg: dpu blend config object
 * @layer_en: 1-enable or 0-disable a layer
 */
struct mixer_dma_layer_cfg {
	u32 layer_zorder : 4;
	u32 rdma_id : 4;
	struct dpu_rect_v2 dst_pos_rect;
	struct mixer_blend_cfg blend_cfg;
	bool layer_en;
};

/**
 * mixer_solid_layer_cfg - mixer solid_cfg params object
 * @layer_zorder: the id value of the layer, 0 <= layer_zorder <= 15
 * @dst_pos_rect: The position of the layer on the dst, x/y/w/h
 * @color: mixer color params object--solid color
 * @blend_cfg: dpu blend config object
 * @solid_area_mode: mixer solid_cfg area object
 * @layer_en: 1-enable or 0-disable a layer
 */
struct mixer_solid_layer_cfg {
	u32 layer_zorder : 4;
	struct dpu_rect_v2 dst_pos_rect;
	struct mixer_color_cfg color;
	struct mixer_blend_cfg blend_cfg;
	enum mixer_solid_area_mode solid_area_mode;
	bool layer_en;
};

/**
 * dpu_hw_mixer_module_enable - enable mixer module
 * @hw: the mixer hardware info
 * @enable: 0-disable, 1-enable
 */
void dpu_hw_mixer_module_enable(struct dpu_hw_blk *hw, bool enable);

/**
 * dpu_hw_mixer_dst_w_h_config - Set dst_width and dst_height values
 * @hw: the mixer hardware info
 * @dst_w: dst width value, 0 < dst_w < 4096
 * @dst_h: dst height value, 0 < dst_h < 4096
 * @cfg_method: use ACPU or cmdlist
 */
void dpu_hw_mixer_dst_w_h_config(struct dpu_hw_blk *hw,
		u16 dst_w, u16 dst_h, bool cfg_method);

/**
 * dpu_hw_mixer_bg_color_config - set the RGBA channel of background color
 * @hw: the mixer hardware info
 * @bg_color: the mixer background object
 * @cfg_method: use ACPU or cmdlist
 */
void dpu_hw_mixer_bg_color_config(struct dpu_hw_blk *hw,
		struct mixer_color_cfg *bg_color, bool cfg_method);

/**
 * dpu_hw_mixer_dma_layer_config - set config for a layer within a frame
 * @hw: the mixer hardware info
 * @rdma_cfg: when mixer uses RDMA input，layer config object
 */
void dpu_hw_mixer_dma_layer_config(struct dpu_hw_blk *hw,
		struct mixer_dma_layer_cfg *rdma_cfg);

/**
 * dpu_hw_mixer_solid_layer_config - config one solid layer
 * @hw: the mixer hardware info
 * @solid_cfg: when mixer uses solid mode，layer config object
 */
void dpu_hw_mixer_solid_layer_config(struct dpu_hw_blk *hw,
		struct mixer_solid_layer_cfg *solid_cfg);

/**
 * dpu_hw_mixer_reset - reset the mixer registers
 * @hw: the mixer hardware pointer
 * @cfg_method: use ACPU or cmdlist
 */
void dpu_hw_mixer_reset(struct dpu_hw_blk *hw, bool cfg_method);

/**
 * to_blend_mode_str - get str name of blend mode
 *
 * @blend_mode: hw blend mode
 * @return const char *
 */
const char *to_blend_mode_str(enum dpu_blend_mode blend_mode);

/**
 * dpu_hw_mixer_status_dump - dump mixer status
 */
void dpu_hw_mixer_status_dump(struct dpu_hw_blk *hw);

#endif
