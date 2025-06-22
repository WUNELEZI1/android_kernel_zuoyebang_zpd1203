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
#include <linux/backlight.h>

#include "dpu_cont_display.h"
#include "dsi_connector.h"
#include "dsi_encoder.h"
#include "dpu_crtc.h"
#include "dpu_kms.h"
#include "dpu_flow_ctrl.h"
#include "dpu_obuf.h"
#include "dsi_panel_event_notifier.h"

#ifdef MI_DISPLAY_MODIFY
#include "mi_dsi_display.h"
#endif

enum pack_data_format dsi_encoder_pack_fmt_get(struct drm_encoder *encoder)
{
	struct dsi_encoder *dsi_encoder;
	enum dsi_pixel_format dsi_format;

	if (!encoder) {
		DSI_ERROR("invalid parameter\n");
		return POSTPIPE_PACK_FMT_MAX;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_format = dsi_encoder->display->panel->panel_info.host_info.color_fmt;

	switch (dsi_format) {
	case DSI_FMT_RGB666:
		return POSTPIPE_PACK_FMT_RGB666;
	case DSI_FMT_RGB888:
		return POSTPIPE_PACK_FMT_RGB888;
	case DSI_FMT_RGB101010:
		return POSTPIPE_PACK_FMT_RGB101010;
	case DSI_FMT_DSC:
		return POSTPIPE_PACK_FMT_DSC_BYTE;
	default:
		DSI_ERROR("unsupported dsi_format: %d\n", dsi_format);
		return POSTPIPE_PACK_FMT_MAX;
	}
}

int dsi_encoder_tmg_mask_get(struct drm_encoder *encoder, u32 *tmg_mask)
{
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	struct dsi_display *display;

	if (!encoder) {
		DSI_ERROR("get invalid params\n");
		return -EINVAL;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_connector = to_dsi_connector(dsi_encoder->connector);
	display = dsi_connector->display;
	*tmg_mask = display->display_info.tmg_mask;
	return 0;
}

static void dsi_encoder_pre_commit(struct drm_atomic_state *state, void *data)
{
	struct dsi_connector *dsi_connector;
	struct drm_encoder *encoder = data;
	struct drm_crtc_state *crtc_state;
	struct dsi_encoder *dsi_encoder;
	struct dsi_connector_state *dsi_state;
	struct dsi_display *display;
	bool obufen_updated = false;
	u32 obufen;
	int ret = 0;
	int i;

	if (!encoder) {
		DSI_ERROR("invalid parameter\n");
		return;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_connector = to_dsi_connector(dsi_encoder->connector);
	display = dsi_connector->display;
	crtc_state = encoder->crtc->state;
	dsi_state = to_dsi_connector_state(dsi_connector->base.state);
	dsi_state->has_cmdlist_cmd = false;
	dsi_state->already_get_mipi_state = false;

	if (need_mode_switch(crtc_state)) {
		ret = dsi_display_panel_timing_switch(display);
		if (ret)
			DSI_ERROR("failed to switch mode\n");
		dsi_state->has_cmdlist_cmd = true;
	}

	if (dsi_state->bl_changed) {
		ret = dsi_display_backlight_set(display, dsi_state->bl_level, USE_CMDLIST);
		dsi_state->bl_changed = false;
		if (ret)
			DSI_ERROR("failed to set bl_level\n");
		dsi_state->has_cmdlist_cmd = true;
	}

	if (need_update_damage_area(dsi_encoder->connector, state, display)) {
		dsi_connector_damage_area_update(dsi_connector, state);
		dsi_state->has_cmdlist_cmd = true;
	}

	display_for_each_active_port(i, display) {
		obufen = dpu_obuf_check_obufen(i, &obufen_updated);
		/* obufen = 0 only allowed to be updated in disp off cmt (encoder_disable) */
		if (obufen_updated && obufen != 0) {
			dpu_obuf_update_obufen_cmt(i, obufen_updated);
			dsi_hw_tmg_obufen_config(&display->port[i].tmg->tmg_hw.hw_blk, obufen);
			dsi_display_line_buf_level_cfg(display, i);
		}
	}

	if (dsi_state->has_cmdlist_cmd)
		DSI_DEBUG("has cmdlist cmd in this farme\n");

	if (display->panel->cmd_mgr_enabled) {
		ret = dsi_cmd_mgr_flush();
		if (ret)
			DSI_ERROR("failed to flush all cmd set, ret = %d\n", ret);
		dsi_cmd_mgr_quota_reset();
	}
}

static bool need_tmg_frame_update(struct dsi_encoder *dsi_encoder,
		struct drm_atomic_state *state)
{
	struct dsi_panel_info *panel_info;
	struct drm_crtc_state *crtc_state;

	crtc_state = dsi_encoder->base.crtc->state;

	if (need_mode_switch(crtc_state))
		return true;

	panel_info = dsi_panel_info_get(dsi_encoder->connector);

	if (panel_info && panel_info->partial_update_enable)
		if (!damage_area_retained(dsi_encoder->connector, state))
			return true;
	return false;
}

static void dsi_encoder_post_commit(struct drm_atomic_state *state, void *data)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_connector *dsi_connector;
	struct drm_encoder *encoder = data;
	struct drm_crtc_state *crtc_state;
	struct dsi_encoder *dsi_encoder;
	struct dsi_display *display;
	struct dsi_tmg *tmg;
	int ret = 0;
	int i;

	if (!encoder) {
		DSI_ERROR("invalid parameter\n");
		return;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_connector = to_dsi_connector(dsi_encoder->connector);
	dsi_state = to_dsi_connector_state(dsi_connector->base.state);
	display = dsi_connector->display;
	if (dsi_state->need_at_once)
		display->tmg_cfg.at_once = true;

	crtc_state = encoder->crtc->state;
	if (need_tmg_frame_update(dsi_encoder, state)) {
		dsi_display_tmg_cfg_update(display, USE_CMDLIST);
		display_for_each_active_port(i, display) {
			tmg = display->port[i].tmg;
			dsi_display_line_buf_level_cfg(display, i);
			ret = dsi_tmg_frame_update(tmg, &display->tmg_cfg);
			if (ret)
				DSI_ERROR("failed to update tmg-%d\n",
						display->port[i].dsi_id);
		}
	}

	dsi_state->need_at_once = false;
	display->tmg_cfg.at_once = false;
}

int __maybe_unused dsi_encoder_tmg_enable(struct dsi_encoder *dsi_encoder)
{
	struct dsi_display *display;
	struct dsi_tmg *tmg;
	int ret = 0;
	int i;

	if (!dsi_encoder) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	display = dsi_encoder->display;
	display_for_each_active_port(i, display) {
		tmg = display->port[i].tmg;
		ret = dsi_tmg_enable(tmg);
		if (ret) {
			DSI_ERROR("failed to enable %d-th tmg\n", i);
			goto error;
		}
	}
error:
	return ret;
}

int dsi_encoder_tmg_doze_enable(struct dsi_encoder *dsi_encoder)
{
	struct dsi_display *display;
	struct dsi_tmg *tmg;
	int ret = 0;
	int i;

	if (!dsi_encoder) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	display = dsi_encoder->display;
	display_for_each_active_port(i, display) {
		tmg = display->port[i].tmg;
		ret = dsi_tmg_doze_enable(tmg);
		if (ret) {
			DSI_ERROR("failed to enable %d-th tmg doze\n", i);
			goto error;
		}
	}
error:
	return ret;
}

int dsi_encoder_tmg_status_dump(struct dsi_encoder *dsi_encoder)
{
	struct dsi_display *display;
	struct dsi_tmg *tmg;
	int ret = 0;
	int i;

	if (!dsi_encoder) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	display = dsi_encoder->display;
	display_for_each_active_port(i, display) {
		tmg = display->port[i].tmg;
		ret = dsi_tmg_status_dump(tmg);
		if (ret) {
			DSI_ERROR("failed to dump %d-th tmg's status\n", i);
			goto error;
		}
	}
error:
	return ret;
}

static void dsi_encoder_enable(struct drm_encoder *encoder)
{
	struct dsi_connector *dsi_connector;
	struct drm_crtc_state *crtc_state;
	struct dsi_encoder *dsi_encoder;
	struct dsi_display *display;
	struct drm_crtc *crtc;
	int ret = 0;

	if (!encoder) {
		DSI_ERROR("invalid parameters, %pK\n", encoder);
		return;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_connector = to_dsi_connector(dsi_encoder->connector);
	crtc_state = encoder->crtc->state;
	crtc = dsi_encoder->connector->state->crtc;
	display = dsi_connector->display;
	DSI_DEBUG("active %d, active_changed %d, connectors_changed %d, mode_changed %d\n",
			crtc_state->active, crtc_state->active_changed,
			crtc_state->connectors_changed, crtc_state->mode_changed);
	if (crtc && (crtc->state->connectors_changed && !crtc->state->active_changed)) {
		DSI_DEBUG("connectors_changed, return\n");
		return;
	}

	if (is_cont_display_enabled()) {
		DSI_INFO("fastboot dsi already enabled\n");
		dsi_connector->current_power_mode = DPU_POWER_MODE_ON;
		dsi_panel_event_notifier_trigger(DRM_PANEL_EVENT_UNBLANK, display->panel);
#ifdef MI_DISPLAY_MODIFY
		mi_dsi_display_read_panel_infos(display);
		mi_dsi_panel_enable(display->panel);
#endif

		dsi_display_esd_ctrl(display, true);
		return;
	}

	if (crtc_state->active_changed && crtc_state->active) {
		ret = dsi_display_enable(display);
		if (ret) {
			DSI_ERROR("failed to enable dsi display\n");
			return;
		}
	}
}

int dsi_encoder_tmg_disable(struct dsi_encoder *dsi_encoder)
{
	struct dsi_display *display;
	struct dsi_tmg *tmg;
	int ret = 0;
	int i;

	if (!dsi_encoder) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	display = dsi_encoder->display;
	display_for_each_active_port(i, display) {
		tmg = display->port[i].tmg;
		ret = dsi_tmg_disable(tmg);
		if (ret) {
			DSI_ERROR("failed to disable %d-th tmg\n", i);
			goto error;
		}
	}

error:
	return ret;
}

static void dsi_encoder_disable(struct drm_encoder *encoder)
{
	struct dsi_connector *dsi_connector;
	struct drm_crtc_state *crtc_state;
	struct dsi_encoder *dsi_encoder;
	struct drm_crtc *crtc;
	int ret;

	if (!encoder) {
		DSI_ERROR("invalid parameters, %pK\n", encoder);
		return;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_connector = to_dsi_connector(dsi_encoder->connector);
	crtc_state = encoder->crtc->state;
	crtc = dsi_encoder->connector->state->crtc;
	DSI_DEBUG("active %d, active_changed %d, connectors_changed %d\n",
			crtc_state->active, crtc_state->active_changed,
			crtc_state->connectors_changed);
	if (crtc && (crtc->state->connectors_changed && !crtc->state->active_changed)) {
		DSI_DEBUG("connectors_changed, return\n");
		return;
	}

	if (crtc_state->active_changed && !crtc_state->active) {
		dsi_connector->sw_clr_fail_cnt = 0;
		dsi_connector->continous_succ_cnt = 0;
		ret = dsi_display_disable(dsi_connector->display);
		if (ret) {
			DSI_ERROR("filed to disable dsi display\n");
			return;
		}
	}
}

static void dsi_encoder_mode_set(struct drm_encoder *encoder,
		struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode)
{
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	struct dsi_display *display;
	int ret = 0;

	if (!encoder || !mode || !adjusted_mode) {
		DSI_ERROR("invalid parameters, %pK, %pK, %pK\n",
				encoder, mode, adjusted_mode);
		return;
	}

	dsi_encoder = to_dsi_encoder(encoder);
	dsi_connector = to_dsi_connector(dsi_encoder->connector);
	display = dsi_connector->display;
	/* update panel current mode */
	ret = dsi_display_mode_set(display, mode, adjusted_mode);
	if (ret)
		DSI_ERROR("failed to set current mode\n");
}

static int dsi_encoder_check_roi(struct dsi_encoder *dsi_encoder,
		struct dpu_crtc_state *dpu_cstate,
		struct dsi_connector_state *dsi_state)
{
	/* TODO: check roi <= crtc size in here */

	/* TODO: check roi alignment with panel in here */
	return 0;
}

static int dsi_encoder_atomic_check(struct drm_encoder *encoder,
		struct drm_crtc_state *crtc_state,
		struct drm_connector_state *conn_state)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_encoder *dsi_encoder;
	struct dpu_crtc_state *dpu_cstate;
	int ret;

	if (!encoder || !crtc_state || !conn_state) {
		DSI_ERROR("invalid parameters, %pK, %pK, %pK\n",
				encoder, crtc_state, conn_state);
		return -EINVAL;
	}

	dpu_cstate = to_dpu_crtc_state(crtc_state);
	dsi_state = to_dsi_connector_state(conn_state);
	dsi_encoder = to_dsi_encoder(encoder);

	ret = dsi_encoder_check_roi(dsi_encoder, dpu_cstate, dsi_state);
	if (ret) {
		DSI_ERROR("failed to check roi\n");
		return -EINVAL;
	}

	return 0;
}

static const struct drm_encoder_helper_funcs dsi_encoder_helper_funcs = {
	.enable = dsi_encoder_enable,
	.disable = dsi_encoder_disable,
	.mode_set = dsi_encoder_mode_set,
	.atomic_check = dsi_encoder_atomic_check,
};

static const struct drm_encoder_funcs dsi_encoder_funcs = {
	.destroy = dsi_encoder_destroy,
};

struct drm_encoder *dsi_encoder_create(struct drm_device *drm_dev,
		struct dsi_display *display)
{
	struct dsi_encoder *dsi_encoder;
	int ret;

	dsi_encoder = kzalloc(sizeof(struct dsi_encoder), GFP_KERNEL);
	if (!dsi_encoder)
		return ERR_PTR(-ENOMEM);

	dsi_encoder->base.possible_crtcs = BIT(MAX_CRTC_COUNT) - 1;

	ret = drm_encoder_init(drm_dev, &dsi_encoder->base,
			&dsi_encoder_funcs,
			DRM_MODE_ENCODER_DSI, NULL);
	if (ret) {
		DSI_ERROR("failed to init dsi drm encoder\n");
		goto error;
	}

	drm_encoder_helper_add(&dsi_encoder->base,
			&dsi_encoder_helper_funcs);

	ret = dpu_kms_pre_commit_cb_register(drm_dev,
			&dsi_encoder_pre_commit, &dsi_encoder->base);
	if (ret) {
		DSI_ERROR("failed to register pre-commit cb\n");
		goto pre_cb_fail;
	}

	ret = dpu_kms_post_commit_cb_register(drm_dev,
			&dsi_encoder_post_commit, &dsi_encoder->base);
	if (ret) {
		DSI_ERROR("failed to register post-commit cb\n");
		goto post_cb_fail;
	}

	dsi_encoder->display = display;

	return &dsi_encoder->base;

post_cb_fail:
	dpu_kms_pre_commit_cb_unregister(drm_dev, &dsi_encoder_pre_commit);
pre_cb_fail:
	drm_encoder_cleanup(&dsi_encoder->base);
error:
	kfree(dsi_encoder);
	return ERR_PTR(ret);
}

void dsi_encoder_destroy(struct drm_encoder *drm_encoder)
{
	struct dsi_encoder *dsi_encoder;

	DSI_DEBUG("dsi encoder destroy\n");

	if (!drm_encoder) {
		DSI_ERROR("invalid parameters, %pK\n", drm_encoder);
		return;
	}

	/* the tmg object will destroy with the dsi_display object */
	dsi_encoder = to_dsi_encoder(drm_encoder);

	drm_encoder_cleanup(drm_encoder);

	kfree(dsi_encoder);
}
