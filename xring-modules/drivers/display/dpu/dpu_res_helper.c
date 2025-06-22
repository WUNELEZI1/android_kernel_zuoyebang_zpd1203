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

#include <linux/kernel.h>
#include <drm/drm_atomic.h>
#include <drm/drm_crtc.h>
#include <drm/drm_util.h>
#include <drm/drm_mode_object.h>
#include "dpu_res_helper.h"
#include "dpu_res_mgr.h"
#include "dpu_kms.h"
#include "dpu_wb.h"
#include "dsi_connector.h"
#include "dsi_display.h"
#include "dpu_color.h"
#include "dp_drm.h"
#include "dpu_dsc_helper.h"
#include "dp_display.h"

/* global resource reservation table */
static const struct dpu_res_reservation_table dpu_res_table[] = { {
		.crtc_type = DPU_CRTC_NONE,
	}, {
		.crtc_type = DPU_CRTC_PRIMARY,
		.connectors = {
			{ DRM_MODE_CONNECTOR_DSI, },
			{ DRM_MODE_CONNECTOR_DisplayPort, },
		},
		.clone_wbs = {
			{ DRM_MODE_CONNECTOR_WRITEBACK, WB_CORE_0 },
			{ DRM_MODE_CONNECTOR_WRITEBACK, WB_CORE_1 }
		}
	}, {
		.crtc_type = DPU_CRTC_SECONDARY,
		.connectors = {
			{ DRM_MODE_CONNECTOR_DisplayPort, },
			{ DRM_MODE_CONNECTOR_DSI, },
			{ DRM_MODE_CONNECTOR_WRITEBACK, WB_CORE_0 }
		},
		.clone_wbs = {
			{ DRM_MODE_CONNECTOR_WRITEBACK, WB_CORE_0 },
			{ DRM_MODE_CONNECTOR_WRITEBACK, WB_CORE_1 }
		}
	}, {
		.crtc_type = DPU_CRTC_THIRD,
		.connectors = {
			{ DRM_MODE_CONNECTOR_WRITEBACK, WB_CORE_1 },
		},
	},
};

#define for_each_clone_wb(crtc_type, blk_id, i) \
	for (i = 0; i < MAX_CLONE_WB_COUNT; i++) \
		for_each_if((blk_id = dpu_res_table[crtc_type].clone_wbs[i].blk_id, \
			(dpu_res_table[crtc_type].clone_wbs[i].type == \
			DRM_MODE_CONNECTOR_WRITEBACK)))

static int dpu_res_helper_connector_type_get(struct drm_atomic_state *state,
		struct drm_crtc *crtc)
{
	struct drm_connector_state *conn_state;
	struct drm_connector *connector;
	int connector_type;
	int i;

	for_each_new_connector_in_state(state, connector, conn_state, i) {
		if (!conn_state->crtc || conn_state->crtc != crtc)
			continue;

		connector_type = connector->connector_type;

		if (is_clone_mode(state, conn_state))
			connector_type = get_cloned_connector_type(state,
				conn_state, NULL);

		return connector_type;
	}

	return 0;
}

static inline enum dpu_crtc_type dpu_res_helper_res_available_check(
		enum dpu_crtc_type crtc_type, u8 connector_type)
{
	const struct dpu_res_reservation_table *res_item = &dpu_res_table[crtc_type];
	int i;

	for (i = 0; i < MAX_CRTC_COUNT; i++)
		if (res_item->connectors[i].type == connector_type)
			return crtc_type;

	return DPU_CRTC_NONE;
}

static inline enum dpu_crtc_type dpu_res_helper_res_crtc_type_get(
		struct drm_atomic_state *state,
		struct drm_crtc_state *crtc_state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	u8 connector_type;
	int i;

	dpu_crtc_state = to_dpu_crtc_state(crtc_state);

	connector_type = dpu_res_helper_connector_type_get(state, crtc_state->crtc);
	if (!connector_type)
		return DPU_CRTC_NONE;

	if (dpu_crtc_state->type)
		return dpu_res_helper_res_available_check(dpu_crtc_state->type,
				connector_type);

	for (i = 0; i < ARRAY_SIZE(dpu_res_table); i++)
		if (dpu_res_table[i].connectors[0].type == connector_type)
			return dpu_res_table[i].crtc_type;

	return DPU_CRTC_NONE;
}

static inline int dpu_res_helper_res_blk_id_get(enum dpu_crtc_type crtc_type,
		u8 connector_type)
{
	const struct dpu_connector_info *connectors = NULL;
	int i;

	connectors = &dpu_res_table[crtc_type].connectors[0];

	for (i = 0; i < MAX_CRTC_COUNT; i++)
		if (connectors[i].type == connector_type)
			return connectors[i].blk_id;

	return INVALID_BLK_ID;
}

static int dpu_res_helper_connector_wb_res_reserve(struct drm_atomic_state *state,
		struct drm_connector_state *conn_state,
		u32 pipe_id, u8 crtc_type, bool test_only)
{
	struct drm_connector *connector;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;
	int blk_id;
	int ret = 0;
	int i;

	connector = conn_state->connector;
	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	if (is_clone_mode(state, conn_state)) {
		for_each_clone_wb(crtc_type, blk_id, i) {
			ret = dpu_wb_resource_reserve(connector,
					pipe_id, blk_id, true, test_only);
			if (!ret)
				break;
		}

		if (ret) {
			DPU_ERROR("failed to reserve clone wb, %d\n", ret);
			goto error;
		}
	} else {
		blk_id = dpu_res_helper_res_blk_id_get(crtc_type,
				connector->connector_type);
		if (blk_id == INVALID_BLK_ID) {
			DPU_ERROR("unsupported type\n");
			goto error;
		}

		ret = dpu_wb_resource_reserve(connector,
				pipe_id, blk_id, false, test_only);
		if (ret) {
			DPU_ERROR("failed to reserve wb\n");
			goto error;
		}
		dpu_kms->virt_pipeline[pipe_id].is_offline = true;
	}

	return 0;
error:
	DPU_ERROR("crtc type %d, connector type %d\n",
			crtc_type, connector->connector_type);
	return -EINVAL;
}

static int dpu_res_helper_wb_res_reserve(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *new_crtc_state,
		u8 crtc_type, bool test_only)
{
	struct drm_connector_state *old_conn_state, *new_conn_state;
	struct drm_connector *connector;
	u32 index;
	int ret;
	u32 mask;

	index = drm_crtc_index(crtc);
	for_each_connector_per_crtc(connector, new_crtc_state, mask) {
		new_conn_state = drm_atomic_get_new_connector_state(state, connector);
		old_conn_state = drm_atomic_get_old_connector_state(state, connector);
		if (old_conn_state->crtc || !new_conn_state->crtc)
			continue;

		if (connector->connector_type != DRM_MODE_CONNECTOR_WRITEBACK)
			continue;

		ret = dpu_res_helper_connector_wb_res_reserve(state,
					new_conn_state, index, crtc_type, test_only);
		if (ret)
			return ret;
	}

	return 0;
}

static void dpu_res_helper_wb_res_release(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_crtc_state)
{
	struct drm_connector_state *old_conn_state, *new_conn_state;
	struct drm_connector *connector;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;
	u32 mask;
	int index;

	index = drm_crtc_index(crtc);
	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	for_each_connector_per_crtc(connector, old_crtc_state, mask) {
		new_conn_state = drm_atomic_get_new_connector_state(state, connector);
		old_conn_state = drm_atomic_get_old_connector_state(state, connector);
		if (!old_conn_state->crtc || new_conn_state->crtc)
			continue;

		if (connector->connector_type != DRM_MODE_CONNECTOR_WRITEBACK)
			continue;

		dpu_wb_resource_release(connector);
		dpu_kms->virt_pipeline[index].is_offline = false;
	}
}

static int dpu_res_helper_mixer_feature_get(struct drm_crtc *crtc,
		enum dpu_crtc_type crtc_type,
		unsigned long *feature)
{
	struct dpu_crtc_state *crtc_state;
	struct dpu_crtc *dpu_crtc;

	dpu_crtc = to_dpu_crtc(crtc);
	crtc_state = to_dpu_crtc_state(crtc->state);

	switch (crtc_type) {
	case DPU_CRTC_PRIMARY:
		*feature = MIXER_PRIMARY;
		break;
	case DPU_CRTC_SECONDARY:
		*feature = MIXER_SECONDARY;
		break;
	case DPU_CRTC_THIRD:
		*feature = MIXER_WB;
		break;
	default:
		DPU_ERROR("unsupported crtc type %u\n", crtc_type);
		return -EINVAL;
	}

	dpu_crtc->type = crtc_type;

	DPU_DEBUG("crtc type %u, mixer feature %lx\n", crtc_type, *feature);

	return 0;
}

static void dpu_res_helper_postpq_resource_release(struct dpu_res_mgr_ctx *ctx,
		struct dpu_crtc *dpu_crtc)
{
	if (!dpu_crtc->postpq)
		return;

	if (dpu_crtc->postpq->hw_gamma) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_gamma->hw);
		dpu_crtc->postpq->hw_gamma = NULL;
	}

	if (dpu_crtc->postpq->hw_3dlut) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_3dlut->hw);
		dpu_crtc->postpq->hw_3dlut = NULL;
	}

	if (dpu_crtc->postpq->hw_pq_top) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_pq_top->hw);
		dpu_crtc->postpq->hw_pq_top = NULL;
	}

	if (dpu_crtc->postpq->hw_pre_proc) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_pre_proc->hw);
		dpu_crtc->postpq->hw_pre_proc = NULL;
	}

	if (dpu_crtc->postpq->hw_acad) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_acad->hw);
		dpu_crtc->postpq->hw_acad = NULL;
	}

	if (dpu_crtc->postpq->hw_hist) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_hist->hw);
		dpu_crtc->postpq->hw_hist = NULL;
	}

	if (dpu_crtc->postpq->hw_scaler) {
		dpu_res_mgr_block_release(ctx,
				&dpu_crtc->postpq->hw_scaler->hw);
		dpu_crtc->postpq->hw_scaler = NULL;
	}
}

static void _reset_power_ctrl_state(struct dpu_power_ctrl *power_ctrl)
{
	power_ctrl->client = NULL;
	power_ctrl->desired_power_mask = 0;
	power_ctrl->actual_power_mask = 0;
	power_ctrl->crtc = NULL;
	power_ctrl->frame_ctrl_enabled = 0;
	power_ctrl->frame_power_max_refresh_rate = 0;
	power_ctrl->lowpower_ctrl = 0;
	power_ctrl->current_refresh_rate = 0;
	atomic_set(&power_ctrl->power_up_timeout, 0);
	atomic_set(&power_ctrl->need_restore, 0);
}

static void dpu_res_helper_power_res_release(struct dpu_power_ctrl *power_ctrl)
{
	dpu_power_client_free(power_ctrl->client);
	_reset_power_ctrl_state(power_ctrl);
}

static int dpu_res_helper_power_res_reserve(
		struct dpu_virt_pipeline *virt_pipeline,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc)
{
	struct dpu_power_client_info client_info = {0};
	struct dsi_display_info *display_info;
	struct drm_connector_state *old, *new;
	struct drm_connector *connector;
	struct dpu_power_ctrl *power_ctrl;
	int ret;
	int i;

	power_ctrl = &virt_pipeline->power_ctrl;
	_reset_power_ctrl_state(power_ctrl);

	for_each_oldnew_connector_in_state(state, connector, old, new, i) {
		if (old->crtc || !new->crtc)
			continue;

		if (connector == dsi_primary_connector_get(state->dev)) {
			display_info = dsi_display_info_get(connector);
			power_ctrl->lowpower_ctrl = display_info->lowpower_ctrl;
			power_ctrl->frame_power_max_refresh_rate =
					display_info->frame_power_max_refresh_rate;
			power_ctrl->frame_power_mask = display_info->frame_power_mask;
			power_ctrl->hw_ulps_max_refresh_rate =
					display_info->hw_ulps_max_refresh_rate;
			POWER_DEBUG("find primary display\n");
			POWER_DEBUG("lowpower_ctrl is 0x%x\n", power_ctrl->lowpower_ctrl);
			POWER_DEBUG("frame_power_max_refresh_rate is %d\n",
					power_ctrl->frame_power_max_refresh_rate);
			POWER_DEBUG("frame_power_mask is 0x%x\n",
					power_ctrl->frame_power_mask);
			POWER_DEBUG("hw_ulps_max_refresh_rate:%d\n",
					power_ctrl->hw_ulps_max_refresh_rate);
			break;
		}
	}

	client_info.sense_ctrl_id = virt_pipeline->hw_scene_ctl->hw.blk_id;

	power_ctrl->client = dpu_power_client_alloc(&client_info);
	if (IS_ERR_OR_NULL(power_ctrl->client)) {
		DPU_ERROR("failed to get power client\n");
		ret = -EINVAL;
		goto error;
	}
	power_ctrl->crtc = crtc;

	return 0;

error:
	return ret;
}

static void dpu_res_helper_postpq_resource_reserve(struct dpu_res_mgr_ctx *ctx,
		struct dpu_crtc *dpu_crtc, u32 index, int blk_id)
{
	struct dpu_hw_blk *hw_blk_post_gamma;
	struct dpu_hw_blk *hw_blk_post_3dlut;
	struct dpu_hw_blk *hw_blk_post_pq_top;
	struct dpu_hw_blk *hw_blk_postpre_proc;
	struct dpu_hw_blk *hw_blk_post_hist;
	struct dpu_hw_blk *hw_blk_acad;
	struct dpu_hw_blk *hw_blk_post_scaler;

	if (!dpu_crtc->postpq)
		return;

	if (blk_id < POST_GAMMA_MAX) {
		hw_blk_post_gamma = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_POST_GAMMA, blk_id, false);
		if (IS_ERR_OR_NULL(hw_blk_post_gamma)) {
			DPU_ERROR("failed to reserve post gamma\n");
			return;
		}

		dpu_crtc->postpq->hw_gamma = to_dpu_hw_post_gamma(
				hw_blk_post_gamma);
	} else {
		dpu_crtc->postpq->hw_gamma = NULL;
	}

	if (blk_id < POST_3DLUT_MAX) {
		hw_blk_post_3dlut = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_POST_3DLUT, blk_id, false);
		if (IS_ERR_OR_NULL(hw_blk_post_3dlut)) {
			DPU_ERROR("failed to reserve post 3dlut\n");
			return;
		}

		dpu_crtc->postpq->hw_3dlut =
			to_dpu_hw_post_3dlut(hw_blk_post_3dlut);
	} else {
		dpu_crtc->postpq->hw_3dlut = NULL;
	}

	if (blk_id < POST_PQ_TOP_MAX) {
		hw_blk_post_pq_top = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_POST_PQ_TOP, blk_id, false);
		if (IS_ERR_OR_NULL(hw_blk_post_pq_top)) {
			DPU_ERROR("failed to reserve post pq top\n");
			return;
		}

		dpu_crtc->postpq->hw_pq_top =
			to_dpu_hw_postpq_top(hw_blk_post_pq_top);
	} else {
		dpu_crtc->postpq->hw_pq_top = NULL;
	}

	if (blk_id < POST_PRE_PROC_MAX) {
		hw_blk_postpre_proc = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_POST_PRE_PROC, blk_id, false);
		if (IS_ERR_OR_NULL(hw_blk_postpre_proc)) {
			DPU_ERROR("failed to reserve post pre proc\n");
			return;
		}

		dpu_crtc->postpq->hw_pre_proc =
			to_dpu_hw_postpre_proc(hw_blk_postpre_proc);
	} else {
		dpu_crtc->postpq->hw_pre_proc = NULL;
	}

	if (blk_id < ACAD_MAX) {
		hw_blk_acad = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_ACAD, blk_id, false);
		if (IS_ERR_OR_NULL(hw_blk_acad)) {
			DPU_ERROR("failed to reserve acad\n");
			return;
		}

		dpu_crtc->postpq->hw_acad =
			to_dpu_hw_post_acad(hw_blk_acad);
	} else {
		dpu_crtc->postpq->hw_acad = NULL;
	}

	if (blk_id < POST_HIST_MAX) {
		hw_blk_post_hist = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_POST_HIST, blk_id, false);
		if (IS_ERR_OR_NULL(hw_blk_post_hist)) {
			DPU_ERROR("failed to reserve histogram\n");
			return;
		}

		dpu_crtc->postpq->hw_hist =
			to_dpu_hw_post_hist(hw_blk_post_hist);
	} else {
		dpu_crtc->postpq->hw_hist = NULL;
	}

	if (blk_id < POST_SCALER_MAX) {
		hw_blk_post_scaler = dpu_res_mgr_block_reserve_id(ctx,
				index, BLK_POST_SCALER, blk_id, false);

		dpu_crtc->postpq->hw_scaler =
				to_dpu_hw_post_scaler(hw_blk_post_scaler);
	} else {
		dpu_crtc->postpq->hw_scaler = NULL;
	}
}

void dpu_res_helper_resource_release(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state, bool finish)
{
	struct drm_crtc_state *new_state, *old_state;
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_crtc *dpu_crtc;
	struct drm_crtc *crtc;
	int index;
	int i;

	res_mgr_ctx = dpu_kms->res_mgr_ctx;

	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i) {
		if (!drm_atomic_crtc_needs_modeset(new_state))
			continue;

		if (finish)
			dpu_res_helper_wb_res_release(state, crtc, old_state);

		if (finish && (new_state->enable))
			continue;

		index = drm_crtc_index(crtc);
		dpu_crtc = to_dpu_crtc(crtc);
		virt_pipeline = &dpu_kms->virt_pipeline[index];

		if (virt_pipeline->hw_scene_ctl) {
			dpu_res_mgr_block_release(res_mgr_ctx,
					&virt_pipeline->hw_scene_ctl->hw);
			virt_pipeline->hw_scene_ctl = NULL;
		}

		if (dpu_crtc->hw_mixer) {
			dpu_res_mgr_block_release(res_mgr_ctx,
					&dpu_crtc->hw_mixer->hw);
			dpu_crtc->hw_mixer = NULL;
		}

		dpu_res_helper_postpq_resource_release(res_mgr_ctx, dpu_crtc);

		if (dpu_crtc->hw_post_pipe_top) {
			dpu_res_mgr_block_release(res_mgr_ctx,
					&dpu_crtc->hw_post_pipe_top->hw);
			dpu_crtc->hw_post_pipe_top = NULL;
		}

		if (dpu_crtc->hw_dsc) {
			dpu_res_mgr_block_release(res_mgr_ctx,
					&dpu_crtc->hw_dsc->hw);
			dpu_crtc->hw_dsc = NULL;
		}

		if (need_disable_crtc(new_state))
			dpu_res_helper_power_res_release(&virt_pipeline->power_ctrl);
	}
}

void dpu_res_helper_update_crtc_dsc_cfg(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *new_state)
{
	struct drm_connector_state *conn_state, *old, *new;
	struct dsi_connector *dsi_connector;
	struct dsi_panel_info *panel_info;
	struct drm_connector *connector;
	struct dp_connector *dp_connector;
	struct dsc_caps_sink *dsc_caps_sink;
	struct dsc_caps_src dsc_caps_src;
	struct dsc_parms dsc_parms = {0};
	struct dpu_crtc *dpu_crtc;
	int connector_type;
	int i;

	if (!drm_atomic_crtc_needs_modeset(new_state)
			|| !new_state->enable)
		return;

	dpu_crtc = to_dpu_crtc(crtc);
	if (!dpu_crtc->hw_dsc)
		return;

	dpu_crtc->hw_dsc->ops.get_caps(&dpu_crtc->hw_dsc->hw, &dsc_caps_src);
	connector_type = DRM_MODE_CONNECTOR_Unknown;

	for_each_oldnew_connector_in_state(state, connector, old, new, i) {
		conn_state = new;
		if (!conn_state->crtc || (&dpu_crtc->base != conn_state->crtc))
			continue;

		connector_type = connector->connector_type;
		switch (connector_type) {
		case DRM_MODE_CONNECTOR_DSI:
			if (dpu_crtc->dsc_cfg.valid)
				break;
			dsi_connector = to_dsi_connector(connector);
			panel_info = &dsi_connector->display->panel->panel_info;
			if (panel_info->host_info.dsc_cfg.dsc_en) {
				dpu_dsc_get_config(
						&panel_info->host_info.dsc_cfg,
						NULL, &dsc_caps_src,
						&dpu_crtc->dsc_cfg);
				dpu_crtc->dsc_cfg.dual_port =
					panel_info->host_info.sbs_en;
				DPU_INFO("dpu_crtc->dsc_cfg.dual_port:%d\n", dpu_crtc->dsc_cfg.dual_port);
			} else {
				dpu_crtc->dsc_cfg.valid = false;
			}
			break;
		case DRM_MODE_CONNECTOR_DisplayPort:
			dp_connector = to_dp_connector(connector);
			dsc_caps_sink = &dp_connector->display->panel->sink_caps.dsc_caps;
			dsc_parms.bits_per_component = dp_connector->display->panel->video_info.bpc;
			dsc_parms.pic_height = dpu_crtc->base.state->mode.vdisplay;
			dsc_parms.pic_width = dpu_crtc->base.state->mode.hdisplay;
			if (dp_connector->display->module_ctrl.dsc_enable)
				dpu_dsc_get_config(&dsc_parms, dsc_caps_sink, &dsc_caps_src,
						&dpu_crtc->dsc_cfg);
			else
				dpu_crtc->dsc_cfg.valid = false;
			break;
		default:
			DPU_DEBUG("unsupported connector type %u\n",
					connector_type);
			return;
		}
	}
}

int dpu_res_helper_resource_reserve(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state, bool test_only)
{
	struct drm_crtc_state *old_state, *new_state;
	struct dpu_hw_blk *hw_blk_post_pipe_top;
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_crtc_state *crtc_state;
	struct dpu_hw_blk *hw_blk_scene_ctl;
	struct dpu_hw_blk *hw_blk_mixer;
	struct dpu_hw_blk *hw_blk_dsc;
	enum dpu_crtc_type crtc_type;
	struct dpu_crtc *dpu_crtc;
	struct drm_crtc *crtc;
	unsigned long blk_feature;
	u32 index;
	int i;
	int ret;

	RES_DEBUG("Enter, test only %d\n", test_only);

	res_mgr_ctx = dpu_kms->res_mgr_ctx;

	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i) {
		WARN_ON(test_only && !drm_modeset_is_locked(&crtc->mutex));

		if (!drm_atomic_crtc_needs_modeset(new_state)
				|| !new_state->enable)
			continue;

		index = drm_crtc_index(crtc);
		dpu_crtc = to_dpu_crtc(crtc);
		virt_pipeline = &dpu_kms->virt_pipeline[index];
		crtc_state = to_dpu_crtc_state(new_state);
		crtc_type = dpu_res_helper_res_crtc_type_get(state, new_state);

		if (!crtc_type && test_only) {
			DPU_ERROR("failed to get crtc type\n");
			return -EINVAL;
		} else if (!crtc_type) {
			DPU_ERROR("unknow crtc type\n");
			continue;
		}

		ret = dpu_res_helper_wb_res_reserve(state, crtc, new_state,
				crtc_type, test_only);
		if (ret) {
			DPU_ERROR("failed to reserve wb resource\n");
			return ret;
		}

		if (dpu_crtc->hw_mixer && virt_pipeline->hw_scene_ctl)
			continue;

		ret = dpu_res_helper_mixer_feature_get(crtc, crtc_type,
				&blk_feature);
		if (ret) {
			DPU_ERROR("failed to get feature\n");
			return ret;
		}

		hw_blk_mixer = dpu_res_mgr_block_reserve_feature(res_mgr_ctx,
				index, BLK_MIXER, blk_feature, test_only);
		if (IS_ERR_OR_NULL(hw_blk_mixer)) {
			DPU_ERROR("failed to reserve mixer\n");
			ret = PTR_ERR(hw_blk_mixer);
			goto error;
		}

		hw_blk_scene_ctl = dpu_res_mgr_block_reserve_id(res_mgr_ctx,
				index, BLK_SCENE_CTL, hw_blk_mixer->blk_id,
				test_only);
		if (IS_ERR_OR_NULL(hw_blk_scene_ctl)) {
			DPU_ERROR("failed to reserve scene ctrl\n");
			ret = PTR_ERR(hw_blk_scene_ctl);
			goto error;
		}

		if (test_only)
			continue;

		dpu_res_helper_postpq_resource_reserve(res_mgr_ctx,
				dpu_crtc, index, hw_blk_mixer->blk_id);

		if (hw_blk_mixer->blk_id < POST_PIPE_TOP_MAX) {
			hw_blk_post_pipe_top = dpu_res_mgr_block_reserve_id(
					res_mgr_ctx, index, BLK_POST_PIPE_TOP,
					hw_blk_mixer->blk_id, false);
			if (IS_ERR_OR_NULL(hw_blk_post_pipe_top)) {
				DPU_ERROR("failed to reserve hw blk post pipe top\n");
				ret = PTR_ERR(hw_blk_post_pipe_top);
				goto error;
			}
			dpu_crtc->hw_post_pipe_top = to_dpu_hw_post_pipe_top(
					hw_blk_post_pipe_top);
		} else {
			dpu_crtc->hw_post_pipe_top = NULL;
		}

		if (hw_blk_mixer->blk_id < DSC_MAX)
			hw_blk_dsc = dpu_res_mgr_block_reserve_id(res_mgr_ctx,
				index, BLK_DSC, hw_blk_mixer->blk_id, false);

		virt_pipeline->hw_scene_ctl = to_dpu_hw_scene_ctl(
				hw_blk_scene_ctl);
		RES_DEBUG("pipe[%u]: reserved scene-ctl-%u\n",
				index, virt_pipeline->hw_scene_ctl->hw.blk_id);

		dpu_crtc->hw_mixer = to_dpu_hw_mixer(hw_blk_mixer);
		RES_DEBUG("pipe[%u]: reserved mixer-%u\n",
				index, dpu_crtc->hw_mixer->hw.blk_id);

		if (!IS_ERR_OR_NULL(hw_blk_dsc)) {
			dpu_crtc->hw_dsc = to_dpu_hw_dsc(hw_blk_dsc);
			RES_DEBUG("pipe[%u]: reserved dsc-%u\n", index,
					dpu_crtc->hw_dsc->hw.blk_id);
		}

		if (need_enable_crtc(new_state)) {
			ret = dpu_res_helper_power_res_reserve(virt_pipeline, state, crtc);
			if (ret) {
				DPU_ERROR("failed to reserve power res\n");
				goto error;
			}
		}
	}

	return 0;

error:
	dpu_res_helper_resource_release(dpu_kms, state, false);

	return ret;
}
