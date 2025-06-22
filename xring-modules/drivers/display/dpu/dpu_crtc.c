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

#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_writeback.h>
#include <drm/drm_vblank.h>
#include <drm/drm_atomic.h>
#include <drm/drm_framebuffer.h>
#include <soc/xring/xr_timestamp.h>

#include "dpu_hw_tpg_ops.h"
#include "dsi_encoder.h"
#include "dpu_kms.h"
#include "dpu_crtc.h"
#include "dpu_plane.h"
#include "dpu_format.h"
#include "dpu_color.h"
#include "dpu_power_mgr.h"
#include "dpu_core_perf.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_idle_helper.h"
#include "dpu_cont_display.h"
#include "dp_drm.h"
#include "dsi_display.h"
#include "dpu_cmdlist_node.h"
#include "dpu_power_helper.h"
#include "dpu_trace.h"

#define PRINT_COMMIT_INFO_CNT 10
#define VSYNC_CALIBRATE_NUM 50
#define PU_REGION_BIT_FIELD 16

static u32 g_need_calib_vsync_count;

static void dpu_crtc_hw_vsync_activate(struct drm_crtc *crtc)
{
	if (!is_primary_display(crtc->state))
		return;

	g_need_calib_vsync_count = VSYNC_CALIBRATE_NUM;
}

bool is_primary_display(struct drm_crtc_state *new_state)
{
	struct drm_connector *primary_conn;
	struct drm_connector *connector;
	struct drm_crtc *crtc;
	u32 mask;

	crtc = new_state->crtc;
	primary_conn = dsi_primary_connector_get(crtc->dev);
	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector == primary_conn)
			return true;
	}

	return false;
}

void get_crtc_size(struct dpu_crtc_state *dpu_cstate, u32 *crtc_width,
		u32 *crtc_height)
{
	if (dpu_cstate->lm_width && dpu_cstate->lm_height) {
		*crtc_width = dpu_cstate->lm_width;
		*crtc_height = dpu_cstate->lm_height;
	} else {
		*crtc_width = dpu_cstate->base.mode.hdisplay;
		*crtc_height = dpu_cstate->base.mode.vdisplay;
	}
}

static int dpu_crtc_mixer_output_size_check(struct drm_crtc_state *crtc_state)
{
	struct dpu_crtc_state *dpu_cstate;

	dpu_cstate = to_dpu_crtc_state(crtc_state);
	if (dpu_cstate->lm_width > MAX_LAYER_MIXER_OUTPUT_WIDTH ||
			dpu_cstate->lm_height > MAX_LAYER_MIXER_OUTPUT_HEIGHT) {
		DPU_ERROR("mixer size invalid, lm_width = %u, lm_height = %u",
				dpu_cstate->lm_width, dpu_cstate->lm_height);
		return -EINVAL;
	}

	return 0;
}

static int dpu_crtc_zorder_check(struct drm_crtc_state *crtc_state,
		struct drm_atomic_state *state)
{
	const struct drm_plane_state *pstate = NULL;
	u8 zpos_list[MAX_PLANE_COUNT] = {0};
	struct dpu_crtc_state *dpu_cstate;
	struct drm_plane *plane = NULL;
	u8 zorder, i;

	drm_atomic_crtc_state_for_each_plane(plane, crtc_state) {
		pstate = drm_atomic_get_new_plane_state(state, plane);
		if (!pstate)
			continue;

		zorder = pstate->zpos;
		if (zorder > MAX_PLANE_ZPOS) {
			DPU_ERROR("zorder range[0, %u], RDMA(%u) exceeded\n",
					MAX_PLANE_ZPOS, zorder);
			return -EINVAL;
		}

		zpos_list[zorder] += 1;
	}

	dpu_cstate = to_dpu_crtc_state(crtc_state);
	dpu_crtc_for_each_solid_layer(dpu_cstate->solid_layer, i) {
		zorder = dpu_cstate->solid_layer.cfg[i].zpos;
		if (zorder > MAX_PLANE_ZPOS) {
			DPU_ERROR("zorder range[0, %u], SOLID(%u) exceeded\n",
					MAX_PLANE_ZPOS, zorder);
			return -EINVAL;
		}

		zpos_list[zorder] += 1;
	}

	for (i = 0; i < MAX_PLANE_COUNT; i++) {
		if (zpos_list[i] > 1) {
			DPU_ERROR("more than two layers using zorder:%u\n", i);
			return -EINVAL;
		}
	}

	return 0;
}

static int dpu_crtc_layer_count_check(struct drm_crtc_state *crtc_state)
{
	int32_t rdma_layer_count;
	int32_t solid_layer_count;
	struct dpu_crtc_state *dpu_cstate;

	dpu_cstate = to_dpu_crtc_state(crtc_state);
	rdma_layer_count = hweight32(crtc_state->plane_mask);
	solid_layer_count = dpu_cstate->solid_layer.count;
	DPU_DEBUG("frame:%llu : rdma count[%d], solid count[%d]\n",
			dpu_cstate->frame_no, rdma_layer_count, solid_layer_count);
	if ((rdma_layer_count + solid_layer_count) > MAX_MIXER_LAYER_COUNT) {
		DPU_ERROR("rdma count[%d], solid count[%d] exceeds max layer count\n",
				rdma_layer_count, solid_layer_count);
		return -EINVAL;
	}

	return 0;
}

static int dpu_crtc_rdma_layer_check(struct drm_crtc_state *crtc_state,
		struct drm_atomic_state *state)
{
	const struct drm_plane_state *pstate = NULL;
	uint32_t dst_x, dst_y, dst_w, dst_h;
	struct dpu_crtc_state *dpu_cstate;
	struct drm_plane *plane = NULL;
	u32 crtc_width, crtc_height;

	dpu_cstate = to_dpu_crtc_state(crtc_state);
	get_crtc_size(dpu_cstate, &crtc_width, &crtc_height);
	drm_atomic_crtc_state_for_each_plane(plane, crtc_state) {
		pstate = drm_atomic_get_new_plane_state(state, plane);
		if (!pstate)
			continue;

		dst_x = pstate->crtc_x;
		dst_y = pstate->crtc_y;
		dst_w = pstate->crtc_w;
		dst_h = pstate->crtc_h;

		if ((CHECK_LAYER_BOUNDS(dst_x, dst_w, crtc_width)) ||
				(CHECK_LAYER_BOUNDS(dst_y, dst_h, crtc_height))) {
			DPU_ERROR("layer zpos[%u] pos exceeds crtc size\n",
					pstate->zpos);
			DPU_ERROR("dst_x(%u) + dst_w(%u) > crtc_width(%u)\n",
					dst_x, dst_w, crtc_width);
			DPU_ERROR("dst_y(%u) + dst_h(%u) > crtc_height(%u)\n",
					dst_y, dst_h, crtc_height);
			return -EINVAL;
		}
	}

	return 0;
}

static int dpu_crtc_solid_layer_check(struct drm_crtc_state *crtc_state)
{
	uint32_t dst_x, dst_y, dst_w, dst_h;
	struct dpu_crtc_state *dpu_cstate;
	u32 crtc_width, crtc_height;
	u8 blend_mode, solid_mode;
	u8 i;

	dpu_cstate = to_dpu_crtc_state(crtc_state);
	get_crtc_size(dpu_cstate, &crtc_width, &crtc_height);
	dpu_crtc_for_each_solid_layer(dpu_cstate->solid_layer, i) {
		dst_x = dpu_cstate->solid_layer.cfg[i].rect.x;
		dst_y = dpu_cstate->solid_layer.cfg[i].rect.y;
		dst_w = dpu_cstate->solid_layer.cfg[i].rect.w;
		dst_h = dpu_cstate->solid_layer.cfg[i].rect.h;

		if ((CHECK_LAYER_BOUNDS(dst_x, dst_w, crtc_width)) ||
				(CHECK_LAYER_BOUNDS(dst_y, dst_h, crtc_height))) {
			DPU_ERROR("layer zpos[%u] pos exceeds crtc size\n",
					dpu_cstate->solid_layer.cfg[i].zpos);
			DPU_ERROR("dst_x(%u) + dst_w(%u) > crtc_width(%u)\n",
					dst_x, dst_w, crtc_width);
			DPU_ERROR("dst_y(%u) + dst_h(%u) > crtc_height(%u)\n",
					dst_y, dst_h, crtc_height);
			return -EINVAL;
		}

		blend_mode = dpu_cstate->solid_layer.cfg[i].blend_mode;
		if (blend_mode >= DPU_BLEND_MODE_MAX) {
			DPU_ERROR("invalid blend mode: %u\n", blend_mode);
			return -EINVAL;
		}

		solid_mode = dpu_cstate->solid_layer.cfg[i].solid_mode;
		if (solid_mode >= SOLID_AREA_MAX) {
			DPU_ERROR("invalid solid mode: %u\n", solid_mode);
			return -EINVAL;
		}
	}

	return 0;
}

static int dpu_crtc_atomic_check(struct drm_crtc *crtc,
		struct drm_atomic_state *state)
{
	struct drm_crtc_state *crtc_state = NULL;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_crtc *dpu_crtc;
	struct dpu_kms *dpu_kms;
	u32 crtc_index;
	int ret;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return -EINVAL;
	}

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dpu_crtc = to_dpu_crtc(crtc);
	crtc_index = drm_crtc_index(crtc);

	crtc_state = drm_atomic_get_new_crtc_state(state, crtc);
	if (!crtc_state) {
		DPU_ERROR("failed to get new crtc state\n");
		return -EINVAL;
	}

	ret = dpu_kms->funcs->check(dpu_kms, state);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do dpu kms check, ret %d\n",
				crtc_index,
				ret);
		return ret;
	}

	if (dpu_crtc->postpq) {
		ret = dpu_crtc->postpq->funcs->check_properties(crtc, crtc_state);
		if (ret) {
			DPU_ERROR("crtc %d: failed to do post pq check, ret %d\n",
					crtc_index,
					ret);
			return ret;
		}
	}

	ret = dpu_crtc_mixer_output_size_check(crtc_state);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do mixer size check: %d\n",
				crtc_index,
				ret);
		return ret;
	}

	ret = dpu_crtc_zorder_check(crtc_state, state);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do zorder check, ret %d\n",
				crtc_index,
				ret);
		return ret;
	}

	ret = dpu_crtc_rdma_layer_check(crtc_state, state);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do rdma layer check: %d\n",
				crtc_index,
				ret);
		return ret;
	}

	ret = dpu_crtc_solid_layer_check(crtc_state);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do solid layer check: %d\n",
				crtc_index,
				ret);
		return ret;
	}

	ret = dpu_crtc_layer_count_check(crtc_state);
	if (ret) {
		DPU_ERROR("crtc %d: failed to do layer count check: %d\n",
				crtc_index,
				ret);
		return ret;
	}

	return 0;
}

static void dpu_crtc_rdma_layer_commit(struct dpu_hw_mixer *hw_mixer,
		struct drm_plane *plane)
{
	struct mixer_dma_layer_cfg mixer_layer_cfg;
	const struct dpu_format_map *dpu_format_map;
	struct mixer_blend_cfg *blend_cfg;
	struct dpu_plane_state *dpu_pstate;
	struct dpu_plane *dpu_plane;

	dpu_plane = to_dpu_plane(plane);
	if (!dpu_plane->hw_rch) {
		DPU_DEBUG("no rch on scene ctrl, mixer bypass layer config\n");
		return;
	}

	dpu_pstate = to_dpu_plane_state(plane->state);

	mixer_layer_cfg.layer_zorder = TO_HW_ZORDER(plane->state->zpos);
	mixer_layer_cfg.rdma_id = dpu_pstate->rch_id;
	mixer_layer_cfg.dst_pos_rect.x = plane->state->crtc_x;
	mixer_layer_cfg.dst_pos_rect.y = plane->state->crtc_y;
	mixer_layer_cfg.dst_pos_rect.w = plane->state->crtc_w;
	mixer_layer_cfg.dst_pos_rect.h = plane->state->crtc_h;

	blend_cfg = &mixer_layer_cfg.blend_cfg;
	dpu_format_map = dpu_format_get(FMT_MODULE_RCH,
			plane->state->fb->format->format,
			plane->state->fb->modifier);

	blend_cfg->blend_mode = plane->state->pixel_blend_mode;
	blend_cfg->has_pixel_alpha = dpu_format_map->has_alpha;
	blend_cfg->layer_alpha = (plane->state->alpha & 0xff);

	mixer_layer_cfg.layer_en = true;

	LAYER_DEBUG("zorder=%u, rdma_id=%u blend_mode=%s lyr_alpha = 0x%x\n",
			plane->state->zpos, mixer_layer_cfg.rdma_id,
			to_blend_mode_str(blend_cfg->blend_mode), blend_cfg->layer_alpha);
	LAYER_DEBUG("dst-[x, y, w, h] = [%u, %u, %u, %u]\n",
			mixer_layer_cfg.dst_pos_rect.x,
			mixer_layer_cfg.dst_pos_rect.y,
			mixer_layer_cfg.dst_pos_rect.w,
			mixer_layer_cfg.dst_pos_rect.h);

	hw_mixer->ops.rch_layer_config(&hw_mixer->hw, &mixer_layer_cfg);
}

static void dpu_crtc_solid_layer_commit(struct dpu_hw_mixer *hw_mixer,
		struct dpu_solid_layer_cfg *dpu_solid_cfg)
{
	struct mixer_solid_layer_cfg cmps_solid_cfg;

	cmps_solid_cfg.layer_zorder = TO_HW_ZORDER(dpu_solid_cfg->zpos);
	cmps_solid_cfg.dst_pos_rect.x = dpu_solid_cfg->rect.x;
	cmps_solid_cfg.dst_pos_rect.y = dpu_solid_cfg->rect.y;
	cmps_solid_cfg.dst_pos_rect.w = dpu_solid_cfg->rect.w;
	cmps_solid_cfg.dst_pos_rect.h = dpu_solid_cfg->rect.h;

	cmps_solid_cfg.color.b = dpu_solid_cfg->color.b;
	cmps_solid_cfg.color.g = dpu_solid_cfg->color.g;
	cmps_solid_cfg.color.r = dpu_solid_cfg->color.r;
	cmps_solid_cfg.color.a = dpu_solid_cfg->color.a;
	cmps_solid_cfg.solid_area_mode = dpu_solid_cfg->solid_mode;

	cmps_solid_cfg.blend_cfg.blend_mode = dpu_solid_cfg->blend_mode;
	cmps_solid_cfg.blend_cfg.has_pixel_alpha = true;
	cmps_solid_cfg.blend_cfg.layer_alpha = (dpu_solid_cfg->plane_alpha & 0xff);

	cmps_solid_cfg.layer_en = true;

	COMPOSER_DEBUG("solid zorder = %u, solid_area_mode = %u\n",
			dpu_solid_cfg->zpos, cmps_solid_cfg.solid_area_mode);
	COMPOSER_DEBUG("blend_mode = %s\n",
			to_blend_mode_str(dpu_solid_cfg->blend_mode));
	COMPOSER_DEBUG("solid_color BGRA = [0x%x, 0x%x, 0x%x, 0x%x]\n",
			cmps_solid_cfg.color.b, cmps_solid_cfg.color.g,
			cmps_solid_cfg.color.r, cmps_solid_cfg.color.a);
	COMPOSER_DEBUG("dst-[x, y, w, h] = [%u, %u, %u, %u]\n",
			cmps_solid_cfg.dst_pos_rect.x,
			cmps_solid_cfg.dst_pos_rect.y,
			cmps_solid_cfg.dst_pos_rect.w,
			cmps_solid_cfg.dst_pos_rect.h);

	hw_mixer->ops.solid_layer_config(&hw_mixer->hw, &cmps_solid_cfg);
}

static void dpu_crtc_mixer_bg_color_commit(struct dpu_hw_mixer *hw_mixer,
		struct dpu_crtc_state *dpu_cstate)
{
	struct mixer_color_cfg bg_color;

	bg_color.b = ARGB_TO_COMPONENT_B(dpu_cstate->bg_color);
	bg_color.g = ARGB_TO_COMPONENT_G(dpu_cstate->bg_color);
	bg_color.r = ARGB_TO_COMPONENT_R(dpu_cstate->bg_color);
	bg_color.a = ARGB_TO_COMPONENT_A(dpu_cstate->bg_color);

	COMPOSER_DEBUG("bg_color BGRA = [0x%x, 0x%x, 0x%x, 0x%x]\n",
			bg_color.b, bg_color.g, bg_color.r, bg_color.a);

	hw_mixer->ops.bg_color_config(&hw_mixer->hw, &bg_color, CMDLIST_WRITE);
}

static void dpu_crtc_mixer_out_size_commit(struct dpu_hw_mixer *hw_mixer,
		struct dpu_crtc_state *dpu_cstate)
{
	u32 width, height;

	get_crtc_size(dpu_cstate, &width, &height);
	DPU_DEBUG("layer mixer out size = [%d x %d]\n", width, height);

	hw_mixer->ops.output_size_config(&hw_mixer->hw, width, height, CMDLIST_WRITE);
}

static void dpu_crtc_mixer_commit(struct drm_crtc *crtc)
{
	struct dpu_crtc_state *dpu_cstate;
	struct dpu_hw_mixer *hw_mixer;
	struct drm_plane *plane = NULL;
	struct dpu_crtc *dpu_crtc;
	u16 i;

	dpu_crtc = to_dpu_crtc(crtc);
	hw_mixer = dpu_crtc->hw_mixer;

	dpu_cstate = to_dpu_crtc_state(crtc->state);
	if (!dpu_cstate) {
		DPU_ERROR("failed to get dpu crtc state\n");
		return;
	}

	if (hw_mixer) {
		hw_mixer->ops.reset(&hw_mixer->hw, CMDLIST_WRITE);

		dpu_crtc_mixer_out_size_commit(hw_mixer, dpu_cstate);
		dpu_crtc_mixer_bg_color_commit(hw_mixer, dpu_cstate);

		if (!!crtc->state->plane_mask && crtc->state->planes_changed) {
			drm_atomic_crtc_for_each_plane(plane, crtc)
				dpu_crtc_rdma_layer_commit(hw_mixer, plane);
		}

		dpu_crtc_for_each_solid_layer(dpu_cstate->solid_layer, i)
			dpu_crtc_solid_layer_commit(hw_mixer,
					&dpu_cstate->solid_layer.cfg[i]);
	}
}

static inline bool dpu_crtc_crc_dump_enable(struct drm_crtc *crtc)
{
	struct dpu_drm_device *dpu_dev;
	struct drm_device *drm_dev;
	struct dpu_kms *kms;

	drm_dev = crtc->dev;
	dpu_dev = to_dpu_drm_dev(drm_dev);
	kms = dpu_dev->dpu_kms;

	return kms->crc_dump_enable;
}

static void dpu_crtc_post_pipe_top_commit(struct drm_crtc *crtc)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct drm_crtc_state *crtc_state;
	struct drm_connector *connector;
	struct drm_encoder *encoder;
	struct post_pipe_cfg cfg = {0};
	struct dpu_crtc *dpu_crtc;
	u32 tmg_mask;
	u32 mask;

	dpu_crtc = to_dpu_crtc(crtc);
	crtc_state = crtc->state;
	hw_post_pipe_top = dpu_crtc->hw_post_pipe_top;
	if (!hw_post_pipe_top)
		return;

	cfg.debug_ctrl.crc_en = dpu_crtc_crc_dump_enable(crtc);
	for_each_connector_per_crtc(connector, crtc_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			cfg.port0_sel = NONE;
			cfg.port1_sel = NONE;
			encoder = connector->encoder;
			dsi_encoder_tmg_mask_get(encoder, &tmg_mask);
			if ((tmg_mask & BIT(TMG0_DSI)) && (tmg_mask & BIT(TMG1_DSI))) {
				cfg.port0_sel = TMG0_DSI;
				cfg.port1_sel = TMG1_DSI;
				/* TODO: add dual mipi config in here */
				cfg.debug_ctrl.split_en = true;
			} else if (tmg_mask & BIT(TMG0_DSI)) {
				cfg.port0_sel = TMG0_DSI;
			} else if (tmg_mask & BIT(TMG1_DSI)) {
				cfg.port0_sel = TMG1_DSI;
			} else {
				DPU_ERROR("failed to select dsc port\n");
			}

			cfg.pack_format = dsi_encoder_pack_fmt_get(encoder);
			if (dpu_crtc->hw_dsc && dpu_crtc->dsc_cfg.valid) {
				cfg.dsc_cfg.dsc_port0_en = cfg.port0_sel == NONE ? false : true;
				cfg.dsc_cfg.dsc_port1_en = cfg.port1_sel == NONE ? false : true;
			}
			hw_post_pipe_top->ops.port_config(&hw_post_pipe_top->hw, &cfg);
		} else if (connector->connector_type ==
				DRM_MODE_CONNECTOR_DisplayPort) {
			cfg.port0_sel = TMG2_DP;
			cfg.port1_sel = NONE;
			cfg.pack_format = dp_connector_pack_fmt_get(connector);
			if (dpu_crtc->hw_dsc && dpu_crtc->dsc_cfg.valid) {
				cfg.pack_format = POSTPIPE_PACK_FMT_DSC_BYTE;
				cfg.dsc_cfg.dsc_port0_en = true;
			}
			hw_post_pipe_top->ops.port_config(&hw_post_pipe_top->hw, &cfg);
		}
	}
}

static void dpu_tpg_crtc_enable(struct drm_crtc *crtc)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;
	struct tpg_config cfg;
	unsigned long mixer_feature;
	int tpg_pos;

	memset(&cfg, 0, sizeof(struct tpg_config));
	dpu_crtc = to_dpu_crtc(crtc);
	tpg_pos = GET_TPG_POSITION(g_dpu_tpg_ctrl);
	dpu_crtc_state = to_dpu_crtc_state(crtc->state);
	mixer_feature = dpu_crtc->hw_mixer->hw.features;

	if (test_bit(MIXER_PRIMARY, &mixer_feature)) {
		if (tpg_pos == TPG_POS_MIXER0) {
			cfg.height = dpu_crtc_state->lm_height;
			cfg.width = dpu_crtc_state->lm_width;
		} else if (tpg_pos == TPG_POS_POSTPQ0) {
			cfg.height = crtc->state->mode.vdisplay;
			cfg.width = crtc->state->mode.hdisplay;
		}
	} else if (test_bit(MIXER_SECONDARY, &mixer_feature)) {
		if (tpg_pos == TPG_POS_MIXER1) {
			cfg.height = dpu_crtc_state->lm_height;
			cfg.width = dpu_crtc_state->lm_width;
		} else if (tpg_pos == TPG_POS_POSTPQ1) {
			cfg.height = crtc->state->mode.vdisplay;
			cfg.width = crtc->state->mode.hdisplay;
		}
	} else if (test_bit(MIXER_WB, &mixer_feature)) {
		if (tpg_pos == TPG_POS_MIXER2) {
			cfg.height = dpu_crtc_state->lm_height;
			cfg.width = dpu_crtc_state->lm_width;
		}
	}

	if (!cfg.height || !cfg.width)
		return;

	cfg.mode_index = GET_TPG_MODE(g_dpu_tpg_ctrl);
	cfg.tpg_enable = GET_TPG_ENABLE(g_dpu_tpg_ctrl);

	dpu_tpg_param_set(&cfg, g_dpu_tpg_param);

	if (cfg.tpg_enable && (
			is_dpu_lp_enabled(DPU_LP_TOP_AUTO_CG_ENABLE) ||
			is_dpu_lp_enabled(DPU_LP_SRAM_LP_ENABLE)))
		DPU_ERROR("TPG need disable top clk auto cg & disable sram lp\n");

	dpu_hw_tpg_enable(dpu_crtc->hw_mixer->hw.iomem_base,
			tpg_pos, &cfg);
}

static inline bool dsc_height_changed(struct drm_crtc *crtc,
		struct drm_atomic_state *state,
		u32 *old_height)
{
	struct drm_crtc_state *old_crtc_state;
	struct dpu_crtc_state *last_dpu_cstate;
	struct dpu_crtc_state *curr_dpu_cstate;


	old_crtc_state = drm_atomic_get_old_crtc_state(state, crtc);
	last_dpu_cstate = to_dpu_crtc_state(old_crtc_state);
	if (!last_dpu_cstate)
		return true;

	curr_dpu_cstate = to_dpu_crtc_state(crtc->state);
	*old_height = last_dpu_cstate->damage_height;

	return last_dpu_cstate->damage_height != curr_dpu_cstate->damage_height;
}

static void dpu_crtc_dsc_update(struct drm_crtc *crtc, struct drm_atomic_state *state)
{
	struct dpu_crtc_state *dpu_cstate;
	struct dpu_crtc *dpu_crtc;
	struct dpu_drm_device *dpu_dev;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_hw_dsc *dsc;
	u32 pipe_id;
	u32 old_height;

	dpu_crtc = to_dpu_crtc(crtc);
	dpu_cstate = to_dpu_crtc_state(crtc->state);
	dsc = dpu_crtc->hw_dsc;
	pipe_id = drm_crtc_index(crtc);
	dpu_dev = to_dpu_drm_dev(crtc->dev);
	pipeline = &dpu_dev->dpu_kms->virt_pipeline[pipe_id];

	if (dsc && (dsc_height_changed(crtc, state, &old_height) ||
			need_restore_register_after_idle(crtc))) {
		DPU_DEBUG("dsc height changed: %d\n", dpu_cstate->damage_height);
		dsc->ops.height_set(&dsc->hw, dpu_cstate->damage_height,
				dpu_crtc->dsc_cfg.dual_port);

		trace_dpu_crtc_dsc_update("damage_height", (u64)dpu_cstate->damage_height);
		dpu_virt_pipe_partial_cfg_update(pipeline, old_height,
				dpu_cstate->damage_height);
	}
}

void dpu_crtc_tpg_fill_color(struct drm_crtc *crtc, u32 color, bool enable)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;
	struct tpg_config cfg;
	unsigned long mixer_feature;
	int tpg_pos = TPG_POS_MIXER0;

	memset(&cfg, 0, sizeof(struct tpg_config));
	dpu_crtc = to_dpu_crtc(crtc);
	dpu_crtc_state = to_dpu_crtc_state(crtc->state);
	mixer_feature = dpu_crtc->hw_mixer->hw.features;

	cfg.height = dpu_crtc_state->lm_height;
	cfg.width = dpu_crtc_state->lm_width;
	if (test_bit(MIXER_PRIMARY, &mixer_feature))
		tpg_pos = TPG_POS_MIXER0;
	else if (test_bit(MIXER_SECONDARY, &mixer_feature))
		tpg_pos = TPG_POS_MIXER1;
	else if (test_bit(MIXER_WB, &mixer_feature))
		tpg_pos = TPG_POS_MIXER2;

	if (!cfg.height || !cfg.width) {
		DPU_DEBUG("tpg pos%d size not set\n", tpg_pos);
		return;
	}

	DPU_DEBUG("fill color 0x%x, pos %d, enable %d\n", color, tpg_pos, enable);

	cfg.mode_index = TPG_MODE_2;
	cfg.tpg_enable = enable;

	dpu_tpg_param_set(&cfg, color);
	dpu_hw_tpg_enable(dpu_crtc->hw_mixer->hw.iomem_base,
			tpg_pos, &cfg);
}

static void dpu_crtc_print_commit_info(struct drm_crtc *crtc,
		struct drm_atomic_state *state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_device *drm_dev;
	struct dpu_crtc *dpu_crtc;
	struct dpu_kms *dpu_kms;

	drm_dev = state->dev;
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	dpu_crtc_state = to_dpu_crtc_state(crtc->state);
	dpu_crtc = to_dpu_crtc(crtc);

	pipeline = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];
	if ((!pipeline->is_offline) &&
			(dpu_crtc->print_commit_info_cnt < PRINT_COMMIT_INFO_CNT)) {
		DPU_INFO("[CRTC:%d:%s][frame_no:%llu] commit\n",
				crtc->base.id, crtc->name, dpu_crtc_state->frame_no);
		dpu_crtc->print_commit_info_cnt++;
	}
}

static void dpu_crtc_atomic_begin(struct drm_crtc *crtc,
		struct drm_atomic_state *state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return;
	}

	dpu_crtc = to_dpu_crtc(crtc);
	dpu_crtc_state = to_dpu_crtc_state(crtc->state);
	dpu_crtc_print_commit_info(crtc, state);

	/*
	 * If you send the display continuously and don't brush the frames,
	 * neither the rdma nor the mixer will match
	 */
	if (is_cont_display_enabled() &&
		(!crtc->state->plane_mask && dpu_crtc_state->solid_layer.count == 0) &&
		need_enable_crtc(crtc->state) && is_primary_display(crtc->state)) {
		DPU_INFO("cont primary display enable, do not need commit\n");
		return;
	}

	dpu_crtc_mixer_commit(crtc);
	dpu_crtc_post_pipe_top_commit(crtc);
	dpu_tpg_crtc_enable(crtc);
	if (dpu_crtc->postpq) {
		dpu_crtc->postpq->funcs->pipe_init(crtc);
		dpu_crtc->postpq->funcs->commit(crtc);
	}

	dpu_crtc_dsc_update(crtc, state);
}

static void dpu_crtc_atomic_flush(struct drm_crtc *crtc,
		struct drm_atomic_state *state)
{
	unsigned long flags;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return;
	}

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	if (crtc->state->event) {
		if (drm_crtc_vblank_get(crtc) != 0)
			drm_crtc_send_vblank_event(crtc, crtc->state->event);
		else
			drm_crtc_arm_vblank_event(crtc, crtc->state->event);

		crtc->state->event = NULL;
	}
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
}

static void dpu_crtc_hw_state_prepare(struct drm_crtc *crtc,
		bool *need_init_dsc, bool *need_restore_pq)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_kms *dpu_kms;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	power_ctrl = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)].power_ctrl;

	if (need_enable_crtc(crtc->state) ||
			dpu_power_frame_ctrl_need_hw_restore(power_ctrl)) {
		*need_init_dsc = true;
		*need_restore_pq = true;
	} else if (need_restore_register_after_idle(crtc)) {
		*need_init_dsc = true;
		*need_restore_pq = false;
	}
}

void dpu_crtc_hw_state_init(struct drm_crtc *crtc)
{
	struct drm_crtc_state *crtc_state;
	struct drm_connector *connector;
	struct dpu_hw_mixer *hw_mixer;
	struct dpu_crtc *dpu_crtc;
	struct dpu_hw_dsc *hw_dsc;
	bool need_init_dsc = false;
	bool need_restore_pq = false;
	u32 mask;

	crtc_state = crtc->state;
	dpu_crtc = to_dpu_crtc(crtc);
	hw_mixer = dpu_crtc->hw_mixer;

	dpu_crtc_hw_state_prepare(crtc, &need_init_dsc, &need_restore_pq);

	if (need_restore_register_after_idle(crtc))
		dpu_crtc->postpq->funcs->ping_pong_reset(crtc);

	if (hw_mixer)
		hw_mixer->ops.enable(&hw_mixer->hw, true);

	if (need_restore_pq && dpu_crtc->postpq)
		dpu_crtc->postpq->funcs->hw_restore(crtc);

	if (need_init_dsc) {
		hw_dsc = dpu_crtc->hw_dsc;
		if (!hw_dsc)
			return;

		for_each_connector_per_crtc(connector, crtc_state, mask) {
			if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
				if (dpu_crtc->dsc_cfg.valid && !is_cont_display_enabled())
					hw_dsc->ops.enable(&hw_dsc->hw, &dpu_crtc->dsc_cfg);
			} else if (connector->connector_type == DRM_MODE_CONNECTOR_DisplayPort) {
				if (dpu_crtc->dsc_cfg.valid)
					hw_dsc->ops.enable(&hw_dsc->hw, &dpu_crtc->dsc_cfg);
				else
					hw_dsc->ops.disable(&hw_dsc->hw);
			}
		}
	}
}

static void dpu_crtc_atomic_enable(struct drm_crtc *crtc,
		struct drm_atomic_state *state)
{
	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return;
	}

	drm_crtc_vblank_on(crtc);

	DPU_DEBUG("connectors_changed %d, mode_changed %d, active_changed %d\n",
			crtc->state->connectors_changed,
			crtc->state->mode_changed,
			crtc->state->active_changed);

	if (!crtc->state->active_changed)
		return;

	dpu_crtc_hw_vsync_activate(crtc);
	dpu_crtc_hw_state_init(crtc);
}

static void dpu_crtc_atomic_disable(struct drm_crtc *crtc,
		struct drm_atomic_state *state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct drm_crtc_state *crtc_state;
	struct dpu_crtc *dpu_crtc;
	struct dpu_hw_mixer *hw_mixer;
	unsigned long flags;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return;
	}

	crtc_state = crtc->state;
	dpu_crtc_state = to_dpu_crtc_state(crtc_state);

	dpu_crtc = to_dpu_crtc(crtc);
	hw_mixer = dpu_crtc->hw_mixer;

	drm_crtc_vblank_off(crtc);

	spin_lock_irqsave(&crtc->dev->event_lock, flags);
	if (crtc->state->event) {
		drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
	}
	spin_unlock_irqrestore(&crtc->dev->event_lock, flags);

	DPU_DEBUG("connectors_changed %d, mode_changed %d, active_changed %d\n",
			crtc->state->connectors_changed,
			crtc->state->mode_changed,
			crtc->state->active_changed);

	if (!crtc->state->active_changed)
		return;

	if (hw_mixer)
		hw_mixer->ops.reset(&hw_mixer->hw, CMDLIST_WRITE);

	dpu_crtc->print_commit_info_cnt = 0;
	dpu_crtc->type = DPU_CRTC_NONE;
	dpu_crtc_state->lm_width = 0;
	dpu_crtc_state->lm_height = 0;
	dpu_crtc_state->keep_power_on = 0;
	dpu_crtc->postpq->funcs->clear(crtc, crtc_state);
}

static struct drm_crtc_state *dpu_crtc_atomic_duplicate_state(
		struct drm_crtc *crtc)
{
	struct dpu_crtc_state *new_state, *old_state;

	if (!crtc || !crtc->state) {
		DPU_ERROR("invalid parameters, %pK\n", crtc);
		return NULL;
	}

	old_state = to_dpu_crtc_state(crtc->state);

	new_state = kzalloc(sizeof(*new_state), GFP_KERNEL);
	if (!new_state)
		return NULL;

	__drm_atomic_helper_crtc_duplicate_state(crtc, &new_state->base);

	new_state->lm_width = old_state->lm_width;
	new_state->lm_height = old_state->lm_height;
	new_state->type = old_state->type;
	new_state->keep_power_on = old_state->keep_power_on;
	new_state->damage_x = old_state->damage_x;
	new_state->damage_y = old_state->damage_y;
	new_state->damage_height = old_state->damage_height;
	new_state->damage_width = old_state->damage_width;
	new_state->mixer_need_update = false;

	return &new_state->base;
}

static void dpu_crtc_atomic_destroy_state(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct dpu_crtc_state *dpu_state;
	struct dpu_crtc *dpu_crtc;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return;
	}
	dpu_state = to_dpu_crtc_state(state);
	COMPOSER_DEBUG("frame no:%llu\n", dpu_state->frame_no);

	if (IS_CMDLIST_NODE(dpu_state->node_id) && CMDLIST_NODE_UNUSED(dpu_state->node_id))
		cmdlist_node_delete(dpu_state->node_id);

	dpu_crtc = to_dpu_crtc(crtc);
	if (dpu_crtc->postpq)
		dpu_crtc->postpq->funcs->destroy_state(crtc, state);

	__drm_atomic_helper_crtc_destroy_state(state);

	kfree(dpu_state);
	dpu_state = NULL;
}

static void dpu_crtc_destroy(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc;

	if (!crtc) {
		DPU_ERROR("invalid parameters %pK\n", crtc);
		return;
	}

	dpu_crtc = to_dpu_crtc(crtc);

	flush_workqueue(dpu_crtc->event_wq);
	destroy_workqueue(dpu_crtc->event_wq);

	dpu_crtc_color_postpq_deinit(crtc);

	drm_crtc_cleanup(crtc);

	kfree(dpu_crtc);
}

static void dpu_crtc_reset(struct drm_crtc *crtc)
{
	struct dpu_crtc_state *new_state;
	struct dpu_crtc *dpu_crtc;

	if (!crtc) {
		DPU_ERROR("invalid parameters, %pK\n", crtc);
		return;
	}

	dpu_crtc = to_dpu_crtc(crtc);

	if (crtc->state) {
		dpu_crtc_atomic_destroy_state(crtc, crtc->state);
		crtc->state = NULL;
	}

	new_state = kzalloc(sizeof(*new_state), GFP_KERNEL);
	if (!new_state)
		return;

	new_state->mixer_need_update = false;

	__drm_atomic_helper_crtc_reset(crtc, &new_state->base);
}

static int dpu_crtc_enable_vblank(struct drm_crtc *crtc)
{
	return 0;
}

static void dpu_crtc_disable_vblank(struct drm_crtc *crtc)
{
	/* Do Nothing */
}

static int dpu_crtc_set_solid_layer(struct drm_crtc *crtc,
		struct dpu_crtc_state *state,
		void __user *user_ptr)
{
	struct dpu_solid_layer solid_layer;

	if (!user_ptr) {
		DPU_ERROR("invalid user pointer\n");
		return -EINVAL;
	}

	if (copy_from_user(&solid_layer, user_ptr, sizeof(solid_layer))) {
		DPU_ERROR("failed to copy solid layer\n");
		return -EINVAL;
	}

	if (solid_layer.count > MAX_DPU_SOLID_LAYER_COUNT) {
		DPU_ERROR("invalid solid layer count %u\n", solid_layer.count);
		return -EINVAL;
	}

	memcpy(&state->solid_layer, &solid_layer, sizeof(solid_layer));

	if (solid_layer.count)
		state->mixer_need_update = true;

	return 0;
}

static int dpu_crtc_atomic_set_property(struct drm_crtc *crtc,
		struct drm_crtc_state *state,
		struct drm_property *property,
		uint64_t val)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;
	int ret = -EINVAL;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return -EINVAL;
	}

	dpu_crtc_state = to_dpu_crtc_state(state);
	dpu_crtc = to_dpu_crtc(crtc);

	if (property == dpu_crtc->frame_number_prop) {
		dpu_crtc_state->frame_no = val;
	} else if (property == dpu_crtc->bg_color_prop) {
		dpu_crtc_state->bg_color = val & FMT_ARGB_VALUE_MASK;

		if (dpu_crtc_state->bg_color)
			dpu_crtc_state->mixer_need_update = true;

	} else if (property == dpu_crtc->lm_width_prop) {
		dpu_crtc_state->lm_width = val;
	} else if (property == dpu_crtc->lm_height_prop) {
		dpu_crtc_state->lm_height = val;
	} else if (property == dpu_crtc->damage_size_prop) {
		dpu_crtc_state->damage_x = val;
		dpu_crtc_state->damage_y = val >> 16;
		dpu_crtc_state->damage_width = val >> 32;
		dpu_crtc_state->damage_height = val >> 48;
	} else if (property == dpu_crtc->solid_layer_prop) {
		ret = dpu_crtc_set_solid_layer(crtc, dpu_crtc_state,
				(void __user *)(uintptr_t)val);
		if (ret) {
			DPU_ERROR("failed to set solid layer, ret %d\n", ret);
			return ret;
		}
	} else if (property == dpu_crtc->type_prop) {
		dpu_crtc_state->type = val;
	} else if (property == dpu_crtc->keep_power_on_prop) {
		dpu_crtc_state->keep_power_on = val;
	} else if (property == dpu_crtc->present_fence_ptr_prop) {
		dpu_crtc_state->present_fence_ptr = val;
	} else if (property == dpu_crtc->release_fence_ptr_prop) {
		dpu_crtc_state->release_fence_ptr = val;
	} else {
		if (dpu_crtc->postpq)
			ret = dpu_crtc->postpq->funcs->set_property(crtc,
				state, property, val);
		if (ret) {
			DPU_ERROR("unknown property %s\n", property->name);
			return -EINVAL;
		}
	}

	DPU_PROPERTY_DEBUG("set property %s: %llu\n", property->name, val);

	return 0;
}

static int dpu_crtc_atomic_get_property(struct drm_crtc *crtc,
		const struct drm_crtc_state *state,
		struct drm_property *property,
		uint64_t *val)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_crtc *dpu_crtc;
	int ret = -EINVAL;

	if (!crtc || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", crtc, state);
		return -EINVAL;
	}

	dpu_crtc_state = to_dpu_crtc_state(state);
	dpu_crtc = to_dpu_crtc(crtc);

	if (property == dpu_crtc->frame_number_prop) {
		*val = dpu_crtc_state->frame_no;
	} else if (property == dpu_crtc->bg_color_prop) {
		*val = dpu_crtc_state->bg_color;
	} else if (property == dpu_crtc->lm_width_prop) {
		*val = dpu_crtc_state->lm_width;
	} else if (property == dpu_crtc->lm_height_prop) {
		*val = dpu_crtc_state->lm_height;
	} else if (property == dpu_crtc->damage_size_prop) {
		*val = dpu_crtc_state->damage_height;
		*val = dpu_crtc_state->damage_width | (*val << PU_REGION_BIT_FIELD);
		*val = dpu_crtc_state->damage_y | (*val << PU_REGION_BIT_FIELD);
		*val = dpu_crtc_state->damage_x | (*val << PU_REGION_BIT_FIELD);
	} else if (property == dpu_crtc->solid_layer_prop) {
		*val = 0;
	} else if (property == dpu_crtc->type_prop) {
		*val = dpu_crtc->type;
	} else if (property == dpu_crtc->present_fence_ptr_prop) {
		*val = 0;
	} else if (property == dpu_crtc->release_fence_ptr_prop) {
		*val = 0;
	} else if (property == dpu_crtc->keep_power_on_prop) {
		*val = dpu_crtc_state->keep_power_on;
	} else {
		if (dpu_crtc->postpq)
			ret = dpu_crtc->postpq->funcs->get_property(crtc, property, val);

		if (ret) {
			DPU_ERROR("unknown property %s\n", property->name);
			return -EINVAL;
		}
	}

	DPU_PROPERTY_DEBUG("get property %s: %lld\n", property->name, *val);
	return 0;
}

static struct drm_property *dpu_crtc_create_range_property(struct drm_crtc *crtc,
		char *name, uint64_t min, uint64_t max)
{
	struct drm_property *prop;

	prop = drm_property_create_range(crtc->dev, 0,
			name, min, max);
	if (!prop)
		return NULL;

	drm_object_attach_property(&crtc->base, prop, 0);
	return prop;
}

static struct drm_property *dpu_crtc_create_enum_property(struct drm_crtc *crtc,
		char *name, const struct drm_prop_enum_list *props, int num_values)
{
	struct drm_property *prop;

	prop = drm_property_create_enum(crtc->dev, 0,
			name, props, num_values);
	if (!prop)
		return NULL;

	drm_object_attach_property(&crtc->base, prop, 0);

	return prop;
}

static struct drm_property *dpu_crtc_create_bool_property(struct drm_crtc *crtc,
		char *name)
{
	struct drm_property *prop;

	prop = drm_property_create_bool(crtc->dev, 0, name);
	if (!prop)
		return NULL;

	drm_object_attach_property(&crtc->base, prop, 0);
	return prop;
}

static int dpu_crtc_create_properties(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc;
	const struct drm_prop_enum_list crtc_type_list[] = {
		{ DPU_CRTC_NONE, "none" },
		{ DPU_CRTC_PRIMARY, "primary" },
		{ DPU_CRTC_SECONDARY, "secondary" },
		{ DPU_CRTC_THIRD, "third" },
	};

	if (!crtc) {
		DPU_ERROR("invalid parameters, %pK\n", crtc);
		return -EINVAL;
	}

	dpu_crtc = to_dpu_crtc(crtc);

	dpu_crtc->bg_color_prop = dpu_crtc_create_range_property(crtc,
			"bg_color", 0, FMT_ARGB_VALUE_MASK);
	if (!dpu_crtc->bg_color_prop)
		return -ENOMEM;

	dpu_crtc->lm_width_prop = dpu_crtc_create_range_property(crtc,
			"lm_width", 0, MAX_LAYER_MIXER_OUTPUT_WIDTH);
	if (!dpu_crtc->lm_width_prop)
		return -ENOMEM;

	dpu_crtc->lm_height_prop = dpu_crtc_create_range_property(crtc,
			"lm_height", 0, MAX_LAYER_MIXER_OUTPUT_HEIGHT);
	if (!dpu_crtc->lm_height_prop)
		return -ENOMEM;

	dpu_crtc->damage_size_prop = dpu_crtc_create_range_property(crtc,
			"damage_size", 0, U64_MAX);
	if (!dpu_crtc->damage_size_prop)
		return -ENOMEM;

	dpu_crtc->solid_layer_prop = dpu_crtc_create_range_property(crtc,
			"solid_layer", 0, U64_MAX);
	if (!dpu_crtc->solid_layer_prop)
		return -ENOMEM;

	dpu_crtc->frame_number_prop = dpu_crtc_create_range_property(crtc,
			"DPU_CRTC_FRAME_NUMBER", 0, U64_MAX);
	if (!dpu_crtc->frame_number_prop)
		return -ENOMEM;

	dpu_crtc->type_prop = dpu_crtc_create_enum_property(crtc,
			"type", crtc_type_list, ARRAY_SIZE(crtc_type_list));
	if (!dpu_crtc->type_prop)
		return -ENOMEM;

	dpu_crtc->present_fence_ptr_prop = dpu_crtc_create_range_property(crtc,
			"present_fence_ptr", 0, U64_MAX);
	if (!dpu_crtc->present_fence_ptr_prop)
		return -ENOMEM;

	dpu_crtc->release_fence_ptr_prop = dpu_crtc_create_range_property(crtc,
			"release_fence_ptr", 0, U64_MAX);
	if (!dpu_crtc->release_fence_ptr_prop)
		return -ENOMEM;

	dpu_crtc->keep_power_on_prop = dpu_crtc_create_bool_property(crtc,
			"keep_power_on");
	if (!dpu_crtc->keep_power_on_prop)
		return -ENOMEM;

	return 0;
}

static const struct drm_crtc_helper_funcs dpu_crtc_helper_funcs = {
	.atomic_check = dpu_crtc_atomic_check,
	.atomic_begin = dpu_crtc_atomic_begin,
	.atomic_flush = dpu_crtc_atomic_flush,
	.atomic_enable = dpu_crtc_atomic_enable,
	.atomic_disable = dpu_crtc_atomic_disable,
};

static const struct drm_crtc_funcs dpu_crtc_funcs = {
	.set_config = drm_atomic_helper_set_config,
	.page_flip = drm_atomic_helper_page_flip,
	.destroy = dpu_crtc_destroy,
	.reset = dpu_crtc_reset,
	.enable_vblank = dpu_crtc_enable_vblank,
	.disable_vblank = dpu_crtc_disable_vblank,
	.atomic_duplicate_state = dpu_crtc_atomic_duplicate_state,
	.atomic_destroy_state = dpu_crtc_atomic_destroy_state,
	.atomic_set_property = dpu_crtc_atomic_set_property,
	.atomic_get_property = dpu_crtc_atomic_get_property,
};

static void dpu_crtc_event_work_handler(struct work_struct *work)
{
	struct dpu_crtc_event_work *event_work;
	struct dpu_crtc *dpu_crtc;

	if (!work) {
		DPU_ERROR("invalid work\n");
		return;
	}

	event_work = container_of(work, struct dpu_crtc_event_work, work);
	dpu_crtc = event_work->dpu_crtc;
	if (!dpu_crtc)
		return;

	if (event_work->cb)
		event_work->cb(&dpu_crtc->base);

	kfree(event_work);
}

void dpu_crtc_intr_event_queue(struct drm_crtc *crtc,
		void (*func)(struct drm_crtc *crtc))
{
	struct dpu_crtc_event_work *event_work;
	struct dpu_crtc *dpu_crtc;

	if (!crtc)
		return;

	event_work = kzalloc(sizeof(*event_work), GFP_ATOMIC);
	if (!event_work) {
		DPU_ERROR("failed to alloc intr event\n");
		return;
	}
	dpu_crtc = to_dpu_crtc(crtc);

	event_work->dpu_crtc = dpu_crtc;
	event_work->cb = func;
	INIT_WORK(&event_work->work, dpu_crtc_event_work_handler);

	queue_work(dpu_crtc->event_wq, &event_work->work);
}

static void dpu_post_pipe_dump_crc(struct dpu_crtc *dpu_crtc, u32 intr_id)
{
	struct dpu_hw_post_pipe_top *pipe_top;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_drm_device *dpu_dev;
	struct drm_device *drm_dev;
	struct dpu_kms *kms;
	int pipe;

	if (dpu_crtc_crc_dump_enable(&dpu_crtc->base) &&
			(intr_id == INTR_ONLINE0_FRM_TIMING_EOF ||
			intr_id == INTR_COMBO_FRM_TIMING_EOF)) {

		pipe = drm_crtc_index(&dpu_crtc->base);
		drm_dev = dpu_crtc->base.dev;
		dpu_dev = to_dpu_drm_dev(drm_dev);
		kms = dpu_dev->dpu_kms;
		pipeline = &kms->virt_pipeline[pipe];
		pipe_top = dpu_crtc->hw_post_pipe_top;
		pipeline->post_pipe_crc_value = pipe_top->ops.crc_dump(&pipe_top->hw, false);
	}
}

int dpu_crtc_handle_events(struct drm_crtc *crtc, u32 intr_id)
{
	struct dpu_crtc *dpu_crtc = to_dpu_crtc(crtc);
	struct dpu_crtc_event *event;
	unsigned long flags;

	dpu_post_pipe_dump_crc(dpu_crtc, intr_id);

	spin_lock_irqsave(&dpu_crtc->event_lock, flags);
	list_for_each_entry(event, &dpu_crtc->event_list, head) {
		if (event->intr_id != intr_id)
			continue;

		if (event->cb)
			event->cb(crtc);
	}
	spin_unlock_irqrestore(&dpu_crtc->event_lock, flags);

	return 0;
}

/*
 * Every time primary panel power on, the first VSYNC_CALIBRATE_NUM frames use software vsync,
 * and align both software and hardware vsync simultaneously.
 * After VSYNC_CALIBRATE_NUM frames, use hardware vsync.
 * If hardware vsync is not available (set to 0), then use software vsync instead.
 * (HW Constraints: hw vsync timestamp will clear when tmg disable)
 */
u64 get_calibrated_vsync_timestamp(struct drm_device *dev)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *drm_connector;
	u64 sw_time, hw_time, sys_time;
	static u64 diff_max;
	u64 new_timestamp;
	u32 current_power_mode, new_power_mode;

	drm_connector = dsi_primary_connector_get(dev);
	if (!drm_connector || !drm_connector->state) {
		DPU_ERROR("get drm_connector failed\n");
		return ktime_to_ns(ktime_get());
	}

	dsi_connector = to_dsi_connector(drm_connector);
	dsi_state = to_dsi_connector_state(drm_connector->state);
	current_power_mode = dsi_connector->current_power_mode;
	new_power_mode = dsi_state->power_mode;
	/* the vsync calibration request may be triggered while ESD recovery is in progress */
	if (current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND ||
			current_power_mode == DPU_POWER_MODE_OFF ||
			new_power_mode == DPU_POWER_MODE_DOZE_SUSPEND ||
			new_power_mode == DPU_POWER_MODE_OFF) {
		DPU_DEBUG("failed to calibration vsync, current power mode: %s, new power mode: %s\n",
				get_dpu_power_mode_name(current_power_mode),
				get_dpu_power_mode_name(new_power_mode));
		return ktime_to_ns(ktime_get());
	}

	/* get the timestamp closest to true vsync timestamp */
	if (g_need_calib_vsync_count > 0) {
		g_need_calib_vsync_count--;
		sys_time = xr_timestamp_gettime();
		sw_time = ktime_to_ns(ktime_get());
		hw_time = dsi_display_timestamp_get(dsi_connector->display);
		if ((hw_time > sw_time) && (hw_time - sw_time > diff_max)) {
			diff_max = hw_time - sw_time;
			DPU_ISR_DEBUG("sys:%lld hw:%lld sw:%lld diff:%llu\n",
					sys_time, hw_time, sw_time, diff_max);
		}
		return sw_time;
	}

	hw_time = dsi_display_timestamp_get(dsi_connector->display);
	new_timestamp = (hw_time == 0) ? ktime_to_ns(ktime_get()) : hw_time - diff_max;

	return new_timestamp;
}

static void dpu_crtc_vsync_event(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc;
	struct drm_event event;
	s64 time;

	dpu_crtc = to_dpu_crtc(crtc);
	time = atomic64_read(&dpu_crtc->vsync_timestamp);
	DPU_ISR_DEBUG("VSYNC TIMESTAMP :%lld\n", time);

	event.length = sizeof(time);
	event.type = DRM_PRIVATE_EVENT_VSYNC;
	dpu_drm_event_notify(crtc->dev, &event, (u8 *)(&time));
}

static int dpu_crtc_vsync_event_cb(struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc;
	s64 vsync_timestamp;

	dpu_crtc = to_dpu_crtc(crtc);

	vsync_timestamp = get_calibrated_vsync_timestamp(crtc->dev);
	if (vsync_timestamp > 0) {
		atomic64_set(&dpu_crtc->vsync_timestamp, vsync_timestamp);
		dpu_crtc_intr_event_queue(crtc, dpu_crtc_vsync_event);
	}

	return 0;
}

int dpu_crtc_vsync_event_enable(struct drm_crtc *crtc, bool enable, void *event)
{
	DPU_DEBUG("vsync enable : %u\n", enable);
	return 0;
}

static int dpu_crtc_event_init(struct drm_crtc *crtc, struct dpu_crtc_event *event)
{
	if (!crtc || !event)
		return -EINVAL;

	switch (event->event_type) {
	case DRM_PRIVATE_EVENT_VSYNC:
		event->intr_id = INTR_ONLINE0_FRM_TIMING_VSYNC;
		event->cb = dpu_crtc_vsync_event_cb;
		event->enable = dpu_crtc_vsync_event_enable;
		break;
	default:
		DPU_WARN("unknow type %d", event->event_type);
		return -ENOENT;
	}

	return 0;
}

static struct dpu_crtc_event *dpu_crtc_event_create(struct drm_crtc *crtc,
		u32 event_type)
{
	struct dpu_crtc *dpu_crtc;
	struct dpu_crtc_event *event;
	int ret = -ENOENT;

	dpu_crtc = to_dpu_crtc(crtc);
	event = kzalloc(sizeof(*event), GFP_ATOMIC);
	if (!event) {
		DPU_ERROR("failed to alloc event 0x%x\n", event_type);
		return NULL;
	}
	event->event_type = event_type;

	switch (event_type) {
	case DRM_PRIVATE_EVENT_HISTOGRAM:
		if (dpu_crtc->postpq)
			ret = dpu_crtc->postpq->funcs->event_init(crtc, event);
		break;
	case DRM_PRIVATE_EVENT_VCHN0_HISTOGRAM:
		ret = dpu_color_pre_hist_event_init(crtc->dev, event);
		break;
	case DRM_PRIVATE_EVENT_VSYNC:
		ret = dpu_crtc_event_init(crtc, event);
		break;
	case DRM_PRIVATE_EVENT_IDLE:
		ret = dpu_idle_event_init(crtc, event);
		break;
	case DRM_PRIVATE_EVENT_TUI:
		ret = dpu_tui_event_init(crtc, event);
		break;
	default:
		break;
	}

	if (ret) {
		DPU_ERROR("failed to create event 0x%x", event_type);
		kfree(event);
		return NULL;
	}

	return event;
}

static void dpu_crtc_event_destroy(struct dpu_crtc_event *event)
{
	kfree(event);
}

int dpu_crtc_event_enable(struct drm_crtc *crtc,
		u32 event_type, bool enable)
{
	struct dpu_crtc *dpu_crtc;
	struct dpu_crtc_event *event;
	unsigned long flags;
	bool found = false;
	int ret = 0;

	dpu_crtc = to_dpu_crtc(crtc);

	spin_lock_irqsave(&dpu_crtc->event_lock, flags);
	list_for_each_entry(event, &dpu_crtc->event_list, head) {
		if (event->event_type == event_type) {
			found = true;
			break;
		}
	}
	if (!found && enable) {
		event = dpu_crtc_event_create(crtc, event_type);
		if (event)
			list_add_tail(&event->head, &dpu_crtc->event_list);
	}
	spin_unlock_irqrestore(&dpu_crtc->event_lock, flags);

	if (enable) {
		if (found)
			return 0;

		if (!event)
			return -EINVAL;

		if (event->enable) {
			ret = event->enable(crtc, true, event);
			if (ret) {
				spin_lock_irqsave(&dpu_crtc->event_lock, flags);
				list_del(&event->head);
				spin_unlock_irqrestore(&dpu_crtc->event_lock, flags);
				DPU_ERROR("failed to enable crtc event 0x%x\n", event_type);
			}
		}
	} else {
		if (!found)
			return 0;

		if (event->enable)
			ret = event->enable(crtc, false, event);

		spin_lock_irqsave(&dpu_crtc->event_lock, flags);
		list_del_init(&event->head);
		spin_unlock_irqrestore(&dpu_crtc->event_lock, flags);
		dpu_crtc_event_destroy(event);
	}

	EVENT_DEBUG("%s event 0x%x\n", enable ? "enable" : "disable", event_type);
	return ret;
}

struct drm_crtc *dpu_crtc_init(struct drm_device *drm_dev,
		struct drm_plane *primary)
{
	struct dpu_crtc *dpu_crtc;
	char name[64];
	int ret;

	DPU_DEBUG("crtc init\n");

	dpu_crtc = kzalloc(sizeof(*dpu_crtc), GFP_KERNEL);
	if (!dpu_crtc)
		return ERR_PTR(-ENOMEM);

	ret = drm_crtc_init_with_planes(drm_dev, &dpu_crtc->base,
			primary, NULL, &dpu_crtc_funcs, NULL);
	if (ret) {
		DPU_ERROR("failed to init crtc, ret %d\n", ret);
		goto error;
	}

	drm_crtc_helper_add(&dpu_crtc->base, &dpu_crtc_helper_funcs);

	ret = dpu_crtc_create_properties(&dpu_crtc->base);
	if (ret) {
		DPU_ERROR("failed to create crtc properties, ret %d\n", ret);
		goto error;
	}

	ret = dpu_crtc_color_postpq_init(&dpu_crtc->base);
	if (ret) {
		DPU_ERROR("failed to init color postpq, ret %d\n", ret);
		goto error;
	}

	if (dpu_crtc->postpq)
		dpu_crtc->postpq->funcs->create_properties(&dpu_crtc->base);

	spin_lock_init(&dpu_crtc->event_lock);
	INIT_LIST_HEAD(&dpu_crtc->event_list);

	snprintf(name, 10, "crtc%d_wq", drm_crtc_index(&dpu_crtc->base));
	dpu_crtc->event_wq = create_singlethread_workqueue(name);
	if (IS_ERR_OR_NULL(dpu_crtc->event_wq)) {
		DPU_ERROR("failed to create %s\n", name);
		goto error;
	}

	return &dpu_crtc->base;

error:
	kfree(dpu_crtc);
	return ERR_PTR(ret);
}
