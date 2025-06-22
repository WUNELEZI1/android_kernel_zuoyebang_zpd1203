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

#include "litexdm.h"
#include "dpu_log.h"
#include "dpu_format.h"
#include "dpu_comp_mgr.h"
#include "dpu_res_mgr.h"

/**
 * dpu_format_map - the dpu format mapping structure
 * @dpu_format: the mapping for alpha check
 * @has_alpha: the format has alpha component or not
 */
struct dpu_format_map {
	u8 dpu_format;
	bool has_alpha;
};

/* the supported modules for this format util */
enum dpu_fmt_module_type {
	FMT_MODULE_RCH = 0,
	FMT_MODULE_WB,
};

static const struct dpu_format_map rch_format_map[] = {
	{RCH_FORMAT_ARGB_2101010, true},
	{RCH_FORMAT_ABGR_2101010, true},
	{RCH_FORMAT_RGBA_1010102, true},
	{RCH_FORMAT_BGRA_1010102, true},
	{RCH_FORMAT_ARGB_8888, true},
	{RCH_FORMAT_ABGR_8888, true},
	{RCH_FORMAT_RGBA_8888, true},
	{RCH_FORMAT_BGRA_8888, true},
	{RCH_FORMAT_XRGB_8888, false},
	{RCH_FORMAT_XBGR_8888, false},
	{RCH_FORMAT_RGBX_8888, false},
	{RCH_FORMAT_BGRX_8888, false},
	{RCH_FORMAT_RGB_888, false},
	{RCH_FORMAT_BGR_888, false},
	{RCH_FORMAT_RGB_565, false},
	{RCH_FORMAT_BGR_565, false},
	{RCH_FORMAT_RGBA_5551, true},
	{RCH_FORMAT_BGRA_5551, true},
	{RCH_FORMAT_ARGB_1555, true},
	{RCH_FORMAT_ABGR_1555, true},
	{RCH_FORMAT_RGBX_5551, false},
	{RCH_FORMAT_BGRX_5551, false},
	{RCH_FORMAT_XRGB_5551, false},
	{RCH_FORMAT_XBGR_5551, false},
	{RCH_FORMAT_XYUV_444_P1_8, false},
	{RCH_FORMAT_XYUV_444_P1_10, false},
	{RCH_FORMAT_VYUY_422_P1_8, false},
	{RCH_FORMAT_YVYU_422_P1_8, false},
	{RCH_FORMAT_YUV_420_P2_8, false},
	{RCH_FORMAT_YUV_420_P2_8, false},
	{RCH_FORMAT_YUV_420_P3_8, false},
	{RCH_FORMAT_YUV_420_P3_8, false},
	{RCH_FORMAT_YUV_420_P2_10, false},
	{RCH_FORMAT_YUV_420_P2_10, false},
};

static const struct dpu_format_map *dpu_format_get(
		enum dpu_fmt_module_type module_type, u32 dpu_format)
{
	const struct dpu_format_map *format_map;
	bool format_found = false;
	u32 count;
	int i;

	switch (module_type) {
	case FMT_MODULE_RCH:
		count = ARRAY_SIZE(rch_format_map);
		format_map = rch_format_map;
		break;
	default:
		dpu_pr_err("wrong module type %d\n", module_type);
		count = 0;
		break;
	}

	for (i = 0; i < count; i++) {
		if (format_map[i].dpu_format == dpu_format) {
			format_found = true;
			break;
		}
	}

	if (!format_found)
		return NULL;

	return &format_map[i];
}

static void dpu_mixer_get_lm_size(struct composer *comp, struct dpu_frame *frame)
{
	comp->lm_width = frame->width;
	comp->lm_height = frame->height;
}

static inline void dpu_mixer_get_size(struct composer *comp, u32 *mixer_width,
		u32 *mixer_height)
{
	if (comp->lm_width && comp->lm_height) {
		*mixer_width = comp->lm_width;
		*mixer_height = comp->lm_height;
	} else {
		dpu_check_and_no_retval(!comp->pinfo, "comp->pinfo is null\n");
		*mixer_width = comp->pinfo->xres;
		*mixer_height = comp->pinfo->yres;
	}
}

static void dpu_mixer_layer_commit(struct dpu_hw_mixer *hw_mixer,
		disp_layer_t *layer_info)
{
	struct mixer_dma_layer_cfg mixer_layer_cfg = {0};
	const struct dpu_format_map *dpu_format_map = NULL;

	mixer_layer_cfg.rdma_id = layer_info->chn_idx;
	mixer_layer_cfg.layer_zorder = layer_info->zorder;
	mixer_layer_cfg.blend_cfg.blend_mode = layer_info->blending_mode;
	mixer_layer_cfg.blend_cfg.layer_alpha = layer_info->glb_alpha;

	dpu_format_map = dpu_format_get(FMT_MODULE_RCH, layer_info->format);
	mixer_layer_cfg.blend_cfg.has_pixel_alpha =
			!dpu_format_map ? false : dpu_format_map->has_alpha;

	mixer_layer_cfg.dst_pos_rect.x = layer_info->dst_rect.x;
	mixer_layer_cfg.dst_pos_rect.y = layer_info->dst_rect.y;
	mixer_layer_cfg.dst_pos_rect.w = layer_info->dst_rect.w;
	mixer_layer_cfg.dst_pos_rect.h = layer_info->dst_rect.h;

	mixer_layer_cfg.layer_en = true;
	dpu_pr_debug("mixer rdma_layer config\n");
	dpu_pr_debug("zorder = %u, rdma_id = %u, layer_info->format = %u"
			"mixer_layer_cfg.blend_cfg.has_pixel_alpha = %u\n",
			mixer_layer_cfg.layer_zorder,
			mixer_layer_cfg.rdma_id,
			layer_info->format,
			mixer_layer_cfg.blend_cfg.has_pixel_alpha);
	dpu_pr_debug("dst-[x, y, w, h] = [%u, %u, %u, %u]\n",
			mixer_layer_cfg.dst_pos_rect.x,
			mixer_layer_cfg.dst_pos_rect.y,
			mixer_layer_cfg.dst_pos_rect.w,
			mixer_layer_cfg.dst_pos_rect.h);

	hw_mixer->ops.rch_layer_config(&hw_mixer->hw, &mixer_layer_cfg);
}

int32_t dpu_mixer_cfg(struct composer *comp, struct dpu_frame *frame)
{
	struct dpu_hw_mixer *hw_mixer;
	uint32_t mixer_width, mixer_height;
	uint32_t i;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp || !frame, -1, "comp or frame is null\n");

	hw_mixer = comp->hw_mixer;
	dpu_mixer_get_lm_size(comp, frame);

	if (hw_mixer) {
		dpu_mixer_get_size(comp, &mixer_width, &mixer_height);
		hw_mixer->ops.output_size_config(&hw_mixer->hw, mixer_width, mixer_height, DIRECT_WRITE);

		for (i = 0; i < frame->layer_nums; i++)
			dpu_mixer_layer_commit(hw_mixer, &frame->layer_infos[i]);
	}

	if (comp->postpq.enable) {
		dpu_pr_debug("postpq_top init\n");
		comp->postpq.funcs->pipe_init(comp);
		comp->postpq.funcs->commit(comp);
	}

	dpu_pr_debug("-\n");
	return 0;
}
