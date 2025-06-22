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
#include <drm/drm_probe_helper.h>
#include <drm/drm_vblank.h>

#include "dsi_connector.h"
#include "dpu_crtc.h"
#include "dpu_kms.h"
#include "dpu_trace.h"
#include "dpu_cmdlist_node.h"
#include "dpu_exception.h"
#include "dpu_wfd.h"
#include "dpu_idle_helper.h"
#include "dpu_hw_ulps.h"

static void dsi_interrupt_init(struct dsi_connector *dsi_conn);
static void dsi_interrupt_deinit(struct dsi_connector *dsi_conn);

void dsi_connector_finish(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *drm_conn;
	struct dsi_display *display;
	u32 mask;
	int ret;

	dsi_connector_mipi_cmd_state_clear(crtc, crtc_state);

	for_each_connector_per_crtc(drm_conn, crtc_state, mask) {
		if (drm_conn->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(drm_conn);
			dsi_conn_state = to_dsi_connector_state(drm_conn->state);
			display = dsi_connector->display;

			ret = dsi_display_finish(display);
			if (unlikely(ret))
				DSI_ERROR("failed to finish dsi display\n");
		}
	}
}

void dsi_connector_mipi_cmd_state_get(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *drm_conn;
	struct dsi_display *display;
	u32 mask;
	int ret;

	for_each_connector_per_crtc(drm_conn, crtc_state, mask) {
		if (drm_conn->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(drm_conn);
			dsi_conn_state = to_dsi_connector_state(drm_conn->state);
			display = dsi_connector->display;
			dsi_conn_state->already_get_mipi_state = false;
			if (!dsi_conn_state->has_cmdlist_cmd)
				continue;

			ret = dsi_display_mipi_cmd_state_get(display);
			if (ret)
				DSI_WARN("failed to get mipi cmd state in disp_commit\n");
			else
				dsi_conn_state->already_get_mipi_state = true;

			DSI_DEBUG("get mipi state in CONN-%d\n", drm_conn->base.id);
		}
	}
}

void dsi_connector_mipi_cmd_state_clear(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *drm_conn;
	struct dsi_display *display;
	u32 mask;
	int ret;

	for_each_connector_per_crtc(drm_conn, crtc_state, mask) {
		if (drm_conn->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(drm_conn);
			dsi_conn_state = to_dsi_connector_state(drm_conn->state);
			display = dsi_connector->display;

			if (!dsi_conn_state->has_cmdlist_cmd)
				continue;

			if (!dsi_conn_state->already_get_mipi_state) {
				DSI_WARN("skip clear mipi cmd state in disp_commit, %d, %d\n",
						dsi_conn_state->has_cmdlist_cmd,
						dsi_conn_state->already_get_mipi_state);
				continue;
			}

			ret = dsi_display_mipi_cmd_state_clear(display);
			if (ret)
				DSI_WARN("failed to clear mipi cmd state in disp_commit\n");
			DSI_DEBUG("clear mipi state in CONN-%d\n", drm_conn->base.id);
		}
	}
}

inline void dsi_connector_lock(struct dsi_connector *dsi_conn)
{
	static int count;

	DSI_DEBUG("request conn_lock %d\n", count);
	mutex_lock(&dsi_conn->conn_lock);
	count++;
}

inline void dsi_connector_unlock(struct dsi_connector *dsi_conn)
{
	static int count;

	DSI_DEBUG("release conn_lock %d\n", count);
	mutex_unlock(&dsi_conn->conn_lock);
	count++;
}

int dsi_connector_ulps_enter(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int ret;

	if (!connector)
		return -EINVAL;

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	if (!display) {
		DSI_ERROR("dsi_display is null\n");
		return -EINVAL;
	}

	ret = dsi_display_ulps_ctrl(display, ENTER_ULPS, USE_CPU, 0);
	if (ret) {
		DSI_INFO("ulps enter error, ret:%d\n", ret);
		return ret;
	}

	return 0;
}

int dsi_connector_ulps_exit(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int ret;

	if (!connector)
		return -EINVAL;

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	if (!display) {
		DSI_ERROR("dsi_display is null\n");
		return -EINVAL;
	}

	ret = dsi_display_ulps_ctrl(display, EXIT_ULPS, USE_CPU, 0);
	if (ret) {
		DSI_ERROR("ulps enter error, ret:%d\n", ret);
		return ret;
	}

	return 0;
}

int dsi_connector_check_ulps_state(struct drm_connector *connector, bool enter)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int ret;

	if (!connector)
		return -EINVAL;

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	if (!display) {
		DSI_ERROR("dsi_display is null\n");
		return -EINVAL;
	}

	ret = dsi_display_check_ulps_state(display, enter);

	return ret;
}

int dsi_connector_clk_ctrl(struct drm_connector *connector, bool enable)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int i;

	if (!connector)
		return -EINVAL;

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	if (!display) {
		DSI_ERROR("dsi_display is null\n");
		return -EINVAL;
	}

	display_for_each_active_port(i, display) {
		dpu_dsi_clk_ctrl(i, enable);
		DSI_DEBUG("%s dsi%d clk success\n", enable ? "enable" : "disable", i);
	}
	return 0;
}

void dsi_connector_ctrl_mode_init(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;
	int ret;

	dsi_connector = to_dsi_connector(connector);

	ret = dsi_display_ctrl_mode_init(dsi_connector->display);
	if (ret)
		DSI_ERROR("dsi ctrl enable failed\n");
}

static const struct drm_prop_enum_list dpu_power_mode[] = {
	{DPU_POWER_MODE_ON,            "ON"},
	{DPU_POWER_MODE_DOZE,          "DOZE"},
	{DPU_POWER_MODE_DOZE_SUSPEND,  "DOZE_SUSPEND"},
	{DPU_POWER_MODE_OFF,           "OFF"},
};

struct drm_connector *dsi_primary_connector_get(struct drm_device *drm_dev)
{
	struct drm_connector_list_iter list_iter;
	struct dsi_connector *dsi_connector;
	struct drm_connector *connector;
	enum dsi_display_type type;
	bool found = false;

	if (!drm_dev) {
		DSI_ERROR("invalid pramter\n");
		return ERR_PTR(-EINVAL);
	}

	drm_connector_list_iter_begin(drm_dev, &list_iter);
	drm_for_each_connector_iter(connector, &list_iter) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(connector);
			type = dsi_connector->display->display_type;
			if (type == DSI_DISPLAY_PRIMARY) {
				found = true;
				break;
			}
		}
	}
	drm_connector_list_iter_end(&list_iter);
	return found ? connector : NULL;
}

bool dsi_connector_is_cmd_mode(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;
	enum dsi_ctrl_mode ctrl_mode;
	bool is_cmd_mode = true;
	int ret = 0;

	dsi_connector = to_dsi_connector(connector);
	ret = dsi_display_ctrl_mode_get(dsi_connector->display, &ctrl_mode);
	if (ret) {
		DSI_ERROR("failed to get dsi ctrl mode\n");
		return is_cmd_mode;
	}

	if (ctrl_mode == DSI_VIDEO_MODE)
		is_cmd_mode = false;

	return is_cmd_mode;
}

static int dsi_connector_set_roi_property(
		struct dsi_connector *dsi_connector,
		struct dsi_connector_state *dsi_state,
		void __user *user_ptr)
{
	struct drm_clip_rect roi_cfg;

	if (!dsi_connector || !dsi_state) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	if (!user_ptr) {
		DSI_ERROR("invalid user pointer, rois cleared\n");
		return 0;
	}

	if (copy_from_user(&roi_cfg, user_ptr, sizeof(roi_cfg))) {
		DSI_ERROR("failed to copy roi_cfg data from user\n");
		return -EFAULT;
	}

	memcpy(&dsi_state->roi_cfg, &roi_cfg, sizeof(roi_cfg));

	return 0;

}

static void handle_idle_with_power_mode(u32 old_power_mode, u32 new_power_mode)
{
	if (((old_power_mode == DPU_POWER_MODE_DOZE) ||
			(old_power_mode == DPU_POWER_MODE_DOZE_SUSPEND)) &&
			((new_power_mode == DPU_POWER_MODE_ON) ||
			(new_power_mode == DPU_POWER_MODE_OFF))) {
		dpu_idle_enable_ctrl(true);
		dpu_hw_ulps_ctrl_dyn(true);
		DPU_INFO("exit aod mode. old:%s new:%s\n",
				get_dpu_power_mode_name(old_power_mode), get_dpu_power_mode_name(new_power_mode));
	}

	if (((old_power_mode == DPU_POWER_MODE_ON) ||
			(old_power_mode == DPU_POWER_MODE_OFF)) &&
			((new_power_mode == DPU_POWER_MODE_DOZE) ||
			(new_power_mode == DPU_POWER_MODE_DOZE_SUSPEND))) {
		dpu_idle_enable_ctrl(false);
		dpu_hw_ulps_ctrl_dyn(false);
		DPU_INFO("enter aod mode. old:%s new:%s\n",
				get_dpu_power_mode_name(old_power_mode), get_dpu_power_mode_name(new_power_mode));
	}
}

static int dsi_connector_atomic_set_property(struct drm_connector *connector,
		struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t val)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_connector *dsi_connector;
	int ret = 0;

	if (!connector || !state || !property) {
		DSI_ERROR("invalid argument, conn %pK, state %pK, prop %pK\n",
				connector, state, property);
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(connector);
	dsi_state = to_dsi_connector_state(state);

	if (property == dsi_connector->roi_prop) {
		ret = dsi_connector_set_roi_property(dsi_connector, dsi_state,
				(void *)(uintptr_t)val);
		if (ret) {
			DSI_ERROR("failed to set dsi connector roi\n");
			return ret;
		}
	} else if (property == dsi_connector->bl_prop) {
		dsi_state->bl_level = val;
		dsi_state->bl_changed = true;
	} else if (property == dsi_connector->damage_area_prop) {
		dsi_state->damage_x = val;
		dsi_state->damage_y = val >> 16;
		dsi_state->damage_width = val >> 32;
		dsi_state->damage_height = val >> 48;
	} else if (property == dsi_connector->power_mode_prop) {
		dsi_state->power_mode = val;
		dsi_state->pm_changed = true;
		handle_idle_with_power_mode(dsi_connector->current_power_mode, dsi_state->power_mode);
	} else if (property == dsi_connector->at_once_prop) {
		dsi_state->need_at_once = !!val;
		DSI_DEBUG("need_at_once: %d\n", dsi_state->need_at_once);
	} else {
		DSI_ERROR("unknow property %s: %lld\n", property->name, val);
		return -EINVAL;
	}

	DPU_PROPERTY_DEBUG("set property %s: %lld\n", property->name, val);

	return 0;
}

static int dsi_connector_atomic_get_property(struct drm_connector *connector,
		const struct drm_connector_state *state,
		struct drm_property *property,
		uint64_t *val)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_connector *dsi_connector;
	struct dsi_panel_info *panel_info;
	struct dsi_display_info *info;
	struct dsi_display *display;
	u16 real_width;
	int ret = 0;

	if (!connector || !state) {
		DSI_ERROR("invalid argument, conn %pK, state %pK\n",
				connector, state);
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(connector);
	dsi_state = to_dsi_connector_state(state);

	if (property == dsi_connector->roi_prop) {
		*val = 0;
	} else if (property == dsi_connector->panel_caps_prop) {
		*val = dsi_connector->panel_caps_blob->base.id;
	} else if (property == dsi_connector->bl_prop) {
		*val = dsi_state->bl_level;
	} else if (property == dsi_connector->damage_area_prop) {
		*val = dsi_state->damage_height;
		*val = dsi_state->damage_width | (*val << 16);
		*val = dsi_state->damage_y | (*val << 16);
		*val = dsi_state->damage_x | (*val << 16);
	} else if (property == dsi_connector->damage_align_size_prop) {
		display = dsi_connector->display;
		panel_info = &display->panel->panel_info;
		info = &display->display_info;
		real_width = info->hdisplay << (info->is_dual_port ? 1 : 0);
		if (!panel_info->partial_update_enable) {
			*val = info->vdisplay;
			*val = (*val << 32) | real_width;
		} else if (info->dsc_en) {
			*val = display->panel->panel_info.host_info.dsc_cfg.slice_height;
			*val = (*val << 32) | real_width;
		} else {
			*val = 0;
		}
	} else if (property == dsi_connector->power_mode_prop) {
		*val = 0;
	} else if (property == dsi_connector->at_once_prop) {
		*val = dsi_state->need_at_once;
	} else {
		DSI_ERROR("failed to get property: %s", property->name);
		ret = -EINVAL;
		goto error;
	}

	DPU_PROPERTY_DEBUG("get property %s: %lld\n", property->name, *val);

error:
	return ret;
}

static int dsi_connector_get_modes(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;
	int mode_count = 0;

	if (!connector) {
		DSI_ERROR("invalid parameters, %pK\n", connector);
		return 0;
	}

	dsi_connector = to_dsi_connector(connector);

	mode_count = dsi_display_modes_get(connector, dsi_connector->display);
	if (!mode_count) {
		DSI_ERROR("failed to get modes\n");
		return 0;
	}

	DSI_DEBUG("get %d modes\n", mode_count);

	return mode_count;
}

static int dsi_connector_detect_ctx(struct drm_connector *connector,
	struct drm_modeset_acquire_ctx *ctx, bool force)
{
	enum drm_connector_status status = connector_status_unknown;
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;

	if (!connector || !ctx) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", connector, ctx);
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;

	status = connector_status_connected;

	connector->display_info.width_mm = display->display_info.width_mm;
	connector->display_info.height_mm = display->display_info.height_mm;
	DSI_DEBUG("connector id: %d, connection status: %d\n",
			connector->base.id, status);

	return (int)status;
}

static enum drm_mode_status dsi_connector_mode_valid(
		struct drm_connector *connector,
		struct drm_display_mode *mode)
{
	struct dsi_connector *dsi_connector;

	if (!connector || !mode) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", connector, mode);
		return MODE_ERROR;
	}

	dsi_connector = to_dsi_connector(connector);

	if (dsi_display_mode_validate(dsi_connector->display, mode))
		return MODE_BAD;

	return MODE_OK;
}

static int dsi_connector_power_mode_check(struct drm_connector *connector,
		struct drm_atomic_state *state)
{
	struct dsi_connector_state *dsi_connector_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector_state *new_state;
	u32 current_power_mode;
	u32 new_power_mode;
	bool support_aod;

	new_state = drm_atomic_get_new_connector_state(state, connector);
	dsi_connector_state = to_dsi_connector_state(new_state);

	if (dsi_connector_state->pm_changed == false)
		return 0;

	new_power_mode = dsi_connector_state->power_mode;
	dsi_connector = to_dsi_connector(connector);
	current_power_mode = dsi_connector->current_power_mode;

	if (new_power_mode > DPU_POWER_MODE_OFF) {
		DSI_ERROR("invalid power mode: %u\n", new_power_mode);
		return -EINVAL;
	}

	support_aod = dsi_connector->display->panel->panel_info.support_aod;
	if ((!support_aod) && (new_power_mode == DPU_POWER_MODE_DOZE ||
			new_power_mode == DPU_POWER_MODE_DOZE_SUSPEND)) {
		DSI_ERROR("dsi not support power mode (%s), because not support aod\n",
				get_dpu_power_mode_name(new_power_mode));
		return -EINVAL;
	}

	if ((current_power_mode == DPU_POWER_MODE_OFF || current_power_mode == DPU_POWER_MODE_ON) &&
			(new_power_mode == DPU_POWER_MODE_DOZE_SUSPEND)) {
		DSI_ERROR("dsi not support transition power mode from (%s) to (%s)\n",
				get_dpu_power_mode_name(current_power_mode),
				get_dpu_power_mode_name(new_power_mode));
		return -EINVAL;
	}

	return 0;
}

static int dsi_connector_atomic_check(struct drm_connector *connector,
		struct drm_atomic_state *state)
{
	struct drm_connector_state *drm_conn_state;
	struct dsi_connector_state *dsi_state;
	struct dsi_connector *dsi_connector;
	struct dsi_panel_info *panel_info;
	struct dsi_display_info *info;
	struct dsi_display *display;
	u16 real_width;
	int ret;

	if (!connector || !state) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", connector, state);
		return -EINVAL;
	}
	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	if (!display) {
		DSI_ERROR("invalid dsi connector, %pK\n", display);
		return -EINVAL;
	}

	panel_info = &display->panel->panel_info;
	info = &display->display_info;
	if (!panel_info->partial_update_enable)
		return 0;

	drm_conn_state = drm_atomic_get_new_connector_state(state, connector);
	dsi_state = to_dsi_connector_state(drm_conn_state);
	if (!dsi_state) {
		DSI_ERROR("invalid dsi connector state, %pK\n", dsi_state);
		return -EINVAL;
	}

	real_width = info->hdisplay << (info->is_dual_port ? 1 : 0);
	if (!dsi_state->damage_width && !dsi_state->damage_height &&
			!dsi_state->damage_x && !dsi_state->damage_y) {
		dsi_state->damage_width = real_width;
		dsi_state->damage_height = info->vdisplay;
	} else if (!dsi_state->damage_width || !dsi_state->damage_height ||
			dsi_state->damage_x + dsi_state->damage_width > real_width ||
			dsi_state->damage_y + dsi_state->damage_height > info->vdisplay) {
		DSI_ERROR("invalid damage area: %d %d %d %d\n",
				dsi_state->damage_x, dsi_state->damage_y,
				dsi_state->damage_width, dsi_state->damage_height);
		return -EINVAL;
	}

	ret = dsi_connector_power_mode_check(connector, state);
	if (ret) {
		DPU_ERROR("failed to dsi power mode check: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct drm_connector_helper_funcs dsi_connector_helper_ops = {
	.get_modes = dsi_connector_get_modes,
	.detect_ctx = dsi_connector_detect_ctx,
	.mode_valid = dsi_connector_mode_valid,
	.atomic_check = dsi_connector_atomic_check,
};

void dsi_connector_destroy(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;

	if (!connector) {
		DSI_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	dsi_connector = to_dsi_connector(connector);

	dsi_interrupt_deinit(dsi_connector);

	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);

	kfree(dsi_connector);
}

static int dsi_connector_fill_modes(struct drm_connector *connector,
		uint32_t max_width, uint32_t max_height)
{
	int mode_count;

	if (!connector) {
		DSI_ERROR("invalid parameter\n");
		return 0;
	}

	mode_count = drm_helper_probe_single_connector_modes(connector,
			max_width, max_height);

	DSI_DEBUG("success probe %d modes\n", mode_count);

	return mode_count;
}

static struct drm_connector_state *dsi_connector_atomic_duplicate_state(
		struct drm_connector *connector)
{
	struct dsi_connector_state *dsi_conn_state, *dsi_old_conn_state;

	if (!connector) {
		DSI_ERROR("invalid parameters\n");
		return NULL;
	}

	dsi_old_conn_state = to_dsi_connector_state(connector->state);

	dsi_conn_state = kzalloc(sizeof(*dsi_conn_state),
			GFP_KERNEL);
	if (!dsi_conn_state)
		return NULL;

	memcpy(&dsi_conn_state->roi_cfg, &dsi_old_conn_state->roi_cfg,
			sizeof(dsi_conn_state->roi_cfg));

	__drm_atomic_helper_connector_duplicate_state(connector,
			&dsi_conn_state->base);

	dsi_conn_state->damage_x = dsi_old_conn_state->damage_x;
	dsi_conn_state->damage_y = dsi_old_conn_state->damage_y;
	dsi_conn_state->damage_width = dsi_old_conn_state->damage_width;
	dsi_conn_state->damage_height = dsi_old_conn_state->damage_height;

	return &dsi_conn_state->base;
}

static void dsi_connector_atomic_destroy_state(
		struct drm_connector *connector,
		struct drm_connector_state *state)
{
	struct dsi_connector_state *dsi_conn_state;

	if (!connector || !state) {
		DSI_ERROR("invalid parameters, %pK, %pK\n",
				connector, state);
		return;
	}

	dsi_conn_state = to_dsi_connector_state(state);

	if (IS_CMDLIST_NODE(dsi_conn_state->node_id) &&
			CMDLIST_NODE_UNUSED(dsi_conn_state->node_id))
		cmdlist_node_delete(dsi_conn_state->node_id);

	__drm_atomic_helper_connector_destroy_state(
			&dsi_conn_state->base);

	kfree(dsi_conn_state);
}

static void dsi_connector_reset(struct drm_connector *connector)
{
	struct dsi_connector_state *dsi_conn_state;

	if (!connector) {
		DSI_ERROR("invalid parameters, %pK\n", connector);
		return;
	}

	if (connector->state) {
		dsi_connector_atomic_destroy_state(connector, connector->state);
		connector->state = NULL;
	}

	dsi_conn_state = kzalloc(sizeof(*dsi_conn_state),
			GFP_KERNEL);
	if (!dsi_conn_state)
		return;

	dsi_conn_state->power_mode = DPU_POWER_MODE_OFF;
	dsi_conn_state->pm_changed = false;
	dsi_conn_state->bl_changed = false;
	dsi_conn_state->need_at_once = false;
	dsi_conn_state->has_cmdlist_cmd = false;
	dsi_conn_state->already_get_mipi_state = false;

	__drm_atomic_helper_connector_reset(connector,
			&dsi_conn_state->base);
}

static const struct drm_connector_funcs dsi_connector_ops = {
	.reset = dsi_connector_reset,
	.destroy = dsi_connector_destroy,
	.fill_modes = dsi_connector_fill_modes,
	.atomic_duplicate_state = dsi_connector_atomic_duplicate_state,
	.atomic_destroy_state = dsi_connector_atomic_destroy_state,
	.atomic_set_property = dsi_connector_atomic_set_property,
	.atomic_get_property = dsi_connector_atomic_get_property,
};

static struct drm_property *dsi_connector_create_range_property(
		struct drm_connector *connector,
		char *name, uint64_t min, uint64_t max)
{
	struct drm_property *property;

	property = drm_property_create_range(connector->dev, 0,
			name, min, max);
	if (!property) {
		DSI_ERROR("drm_property_create_range fail: %s\n", name);
		return NULL;
	}

	drm_object_attach_property(&connector->base, property, 0);

	return property;
}

static int dsi_connector_create_blob_property(struct drm_connector *connector,
		struct drm_property **property, struct drm_property_blob **blob,
		const char *name, size_t blob_size)
{
	struct dsi_connector *dsi_connector;

	if (!connector || !property || !blob || !name) {
		DSI_ERROR("get invalid parameters, %pK, %pK, %pK, %s\n",
				connector, property, blob, name);
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(connector);
	*blob = drm_property_create_blob(connector->dev, blob_size, NULL);
	if (!(*blob)) {
		DSI_ERROR("failed to create blob: %s\n", name);
		return -ENOMEM;
	}

	*property = drm_property_create(connector->dev,
			DRM_MODE_PROP_BLOB, name, 0);
	if (!(*property)) {
		DSI_ERROR("drm_property_create %s failed", name);
		return -ENOMEM;
	}

	drm_object_attach_property(&connector->base, *property,
			(*blob)->base.id);

	DSI_DEBUG("create blob %s, size %lu\n", name, blob_size);
	return 0;
}

static struct drm_property *dsi_connector_create_enum_property(
		struct drm_connector *connector, char *name,
		const struct drm_prop_enum_list *props, int num_values)
{
	struct drm_property *prop;

	prop = drm_property_create_enum(connector->dev, 0,
			name, props, num_values);
	if (!prop)
		return NULL;

	drm_object_attach_property(&connector->base, prop, 0);
	return prop;
}

static int dsi_connector_create_properties(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int ret = 0;

	if (!connector) {
		DSI_ERROR("invalid connector %pK\n", connector);
		return -EINVAL;
	}

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;

	/* create roi property */
	dsi_connector->roi_prop = dsi_connector_create_range_property(connector,
			"roi", 0, ~0);
	if (IS_ERR_OR_NULL(dsi_connector->roi_prop)) {
		DSI_ERROR("failed to create roi property\n");
		return -ENOMEM;
	}

	ret = dsi_connector_create_blob_property(connector,
			&dsi_connector->panel_caps_prop,
			&dsi_connector->panel_caps_blob,
			"PANEL_CAPS",
			sizeof(struct dsi_panel_caps));

	dsi_connector->damage_align_size_prop = dsi_connector_create_range_property(connector,
			"damage_align_size", 0, U64_MAX);
	if (!dsi_connector->damage_align_size_prop)
		return -ENOMEM;

	dsi_connector->damage_area_prop = dsi_connector_create_range_property(connector,
			"damage_area", 0, U64_MAX);
	if (!dsi_connector->damage_area_prop)
		return -ENOMEM;

	dsi_connector->bl_prop = dsi_connector_create_range_property(connector,
			"brightness_level", 0, DSI_BL_LEVEL_MAX);
	if (IS_ERR_OR_NULL(dsi_connector->bl_prop)) {
		DSI_ERROR("failed to create bl_prop\n");
		return -ENOMEM;
	}

	dsi_connector->power_mode_prop = dsi_connector_create_enum_property(
			connector, "power_mode", dpu_power_mode,
			ARRAY_SIZE(dpu_power_mode));
	if (IS_ERR_OR_NULL(dsi_connector->power_mode_prop)) {
		DSI_ERROR("failed to create low power property\n");
		return -ENOMEM;
	}

	dsi_connector->at_once_prop = dsi_connector_create_range_property(connector,
			"at_once", 0, U64_MAX);
	if (!dsi_connector->at_once_prop)
		return -ENOMEM;

	return ret;
}

static int dsi_connector_update_panel_caps_property(struct drm_connector *connector)
{
	struct dsi_connector *dsi_connector = to_dsi_connector(connector);
	struct dsi_display *display = dsi_connector->display;
	struct dsi_panel_hdr_caps *hdr_caps;
	struct dsi_panel_info *panel_info;
	struct dsi_panel_caps panel_caps;
	size_t panel_name_len;
	int i;

	panel_info = &display->panel->panel_info;
	hdr_caps = &display->panel->panel_info.hdr_info;
	memset(&panel_caps, 0, sizeof(panel_caps));
	if (hdr_caps->hdr_enabled) {
		memcpy(&panel_caps.hdr_caps, (void *)hdr_caps,
				sizeof(panel_caps.hdr_caps));
		DSI_DEBUG("hdr_caps: %d, %d, name: %s\n",
				panel_caps.hdr_caps.blackness_level,
				panel_caps.hdr_caps.peak_brightness,
				panel_info->name);
	}

	panel_name_len = strlen(panel_info->name);
	if (!panel_name_len || panel_name_len >= MAX_DSI_PANEL_NAME_LENGTH)
		DSI_WARN("invalid panel name length %zu\n", panel_name_len);
	else
		strcpy(panel_caps.name, panel_info->name);

	panel_caps.min_brightness = panel_info->bl_config.brightness_min_level;

	for (i = 0; i < panel_info->num_modes; i++) {
		panel_caps.vrr_caps[i].frame_rate =
				panel_info->modes[i].timing.framerate;
		panel_caps.vrr_caps[i].h_skew =
				panel_info->modes[i].timing.h_skew;
		panel_caps.vrr_caps[i].te_rate = panel_info->modes[i].timing.te_rate;
		panel_caps.vrr_caps[i].vrr_type = panel_info->modes[i].vrr_type;
		panel_caps.vrr_caps[i].base_rate = panel_info->modes[i].timing.base_rate;
		DSI_DEBUG("get vrr caps %d, %d, %d, %d, %d\n",
				panel_caps.vrr_caps[i].frame_rate,
				panel_caps.vrr_caps[i].h_skew,
				panel_caps.vrr_caps[i].te_rate,
				panel_caps.vrr_caps[i].base_rate,
				panel_caps.vrr_caps[i].vrr_type);
	}
	panel_caps.num_modes = panel_info->num_modes;

	/* Todo: support video aod */
	if (panel_info->support_aod)
		panel_caps.aod_mode = AOD_MODE_DOZE_MASK | AOD_MODE_SUSPEND_MASK;

	panel_caps.init_brightness = panel_info->bl_config.current_brightness;
	DSI_DEBUG("set get uefi brightness %d\n", panel_caps.init_brightness);

	memcpy(dsi_connector->panel_caps_blob->data, (void *)&panel_caps,
				dsi_connector->panel_caps_blob->length);
	return 0;
}

struct drm_connector *dsi_connector_create(struct drm_device *drm_dev,
		struct drm_encoder *drm_encoder, struct dsi_display *display)
{
	struct dsi_connector *dsi_connector;
	int ret = 0;

	dsi_connector = kzalloc(sizeof(*dsi_connector), GFP_KERNEL);
	if (!dsi_connector)
		return ERR_PTR(-ENOMEM);

	ret = drm_connector_init(drm_dev, &dsi_connector->base,
			&dsi_connector_ops,
			DRM_MODE_CONNECTOR_DSI);
	if (ret) {
		DSI_ERROR("failed to init dsi drm connector\n");
		goto error;
	}

	drm_connector_helper_add(&dsi_connector->base,
			&dsi_connector_helper_ops);

	dsi_connector->encoder = drm_encoder;
	dsi_connector->display = display;
	dsi_connector->drm_dev = drm_dev;
	dsi_connector->current_power_mode = DPU_POWER_MODE_OFF;
	dsi_connector->sw_clr_fail_cnt = 0;
	dsi_connector->continous_succ_cnt = 0;

	/* TODO: add dsi connector property init in here */
	ret = dsi_connector_create_properties(&dsi_connector->base);
	if (ret) {
		DSI_ERROR("failed to create connector properties\n");
		goto attach_fail;
	}

	dsi_connector_update_panel_caps_property(&dsi_connector->base);
	dsi_interrupt_init(dsi_connector);
	mutex_init(&dsi_connector->conn_lock);

	return &dsi_connector->base;

attach_fail:
	drm_connector_cleanup(&dsi_connector->base);
error:
	kfree(dsi_connector);
	return ERR_PTR(ret);
}

static void dsi_underflow_work(struct work_struct *work)
{
	struct dsi_intr_context *dsi_intr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_pipe *intr_pipe;
	struct drm_connector *connector;
	struct dpu_kms *dpu_kms;
	struct dpu_commit_ctx *commit_ctx;
	struct dpu_virt_pipeline *virt_pipe;
	struct drm_crtc *crtc;
	u32 vrefresh_rate = 0;

	dsi_intr_ctx = container_of(work, struct dsi_intr_context, underflow_work);
	if (!dsi_intr_ctx) {
		DPU_ERROR("dsi_intr_ctx is NULL\n");
		return;
	}

	intr_pipe = dsi_intr_ctx->intr_pipe;
	crtc = intr_pipe->crtc;
	connector = intr_pipe->connector;
	if (!crtc) {
		DPU_DEBUG("crtc is null\n");
		return;
	}

	dpu_kms = to_dpu_drm_dev(crtc->dev)->dpu_kms;
	virt_pipe = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];
	commit_ctx = &virt_pipe->commit_ctx;

	if (connector) {
		if (!dsi_connector_is_cmd_mode(connector)) {
			if (crtc->state)
				vrefresh_rate = drm_mode_vrefresh(&crtc->state->mode);
			DPU_WARN("pipe %d, %s(frame_no %lld): underflow, vrefresh_rate %d(hz)\n",
					drm_crtc_index(crtc),
					connector->name,
					dsi_intr_ctx->underflow_frame_no,
					vrefresh_rate);
			dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
			dpu_core_perf_dump(dpu_drm_dev->dpu_kms);
			dpu_ddr_dvfs_ok_dump();
		} else {
			mutex_lock(&commit_ctx->commit_lock);
			if (atomic_read(&virt_pipe->hw_hang)) {
				DPU_ERROR("pipe %d, %s(frame_no %lld): underflow\n", drm_crtc_index(crtc),
						connector->name,
						dsi_intr_ctx->underflow_frame_no);
				dpu_kms_exception_recovery(virt_pipe, crtc->state, crtc);
			}
			mutex_unlock(&commit_ctx->commit_lock);
		}
	}
}

static void dsi_interrupt_init(struct dsi_connector *dsi_conn)
{
	INIT_WORK(&dsi_conn->intr_ctx.underflow_work, dsi_underflow_work);
}

static void dsi_interrupt_deinit(struct dsi_connector *dsi_conn)
{
	/* do dsi interrupt deinit */
}

bool damage_area_retained(struct drm_connector *connector,
		struct drm_atomic_state *state)
{
	struct drm_connector_state *old_drm_conn_state;
	struct dsi_connector_state *last_state;
	struct dsi_connector_state *curr_state;

	old_drm_conn_state = drm_atomic_get_old_connector_state(state, connector);
	last_state = to_dsi_connector_state(old_drm_conn_state);
	curr_state = to_dsi_connector_state(connector->state);

	if (need_restore_register_after_idle(connector->state->crtc))
		return false;

	return last_state && last_state->damage_x == curr_state->damage_x &&
			last_state->damage_y == curr_state->damage_y &&
			last_state->damage_width == curr_state->damage_width &&
			last_state->damage_height == curr_state->damage_height;
}

int dsi_connector_damage_area_update(struct dsi_connector *connector, struct drm_atomic_state *state)
{
	struct dsi_connector_state *dsi_state;
	struct dsi_display *display;

	if (!connector) {
		DSI_ERROR("nullptr!\n");
		return -EINVAL;
	}

	display = connector->display;
	dsi_state = to_dsi_connector_state(connector->base.state);

	display->tmg_cfg.partial.xs = dsi_state->damage_x;
	display->tmg_cfg.partial.ys = dsi_state->damage_y;
	display->tmg_cfg.partial.width = dsi_state->damage_width >>
			(display->display_info.is_dual_port ? 1 : 0);
	display->tmg_cfg.partial.height = dsi_state->damage_height;
	DPU_DEBUG("damage area: %d %d %d %d\n",
			dsi_state->damage_x, dsi_state->damage_y,
			dsi_state->damage_width, dsi_state->damage_height);

	if (dsi_state->damage_height < display->display_info.vdisplay)
		display->tmg_cfg.partial_en = 1;
	else
		display->tmg_cfg.partial_en = 0;

	dsi_display_damage_area_update(display);

	return 0;
}

void dpu_dsi_irq_status_dump(struct drm_connector *connector)
{
	struct dpu_connector_intr_context *conn_intr_ctx;
	struct dsi_intr_context *dsi_intr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;

	if (!connector) {
		DPU_ERROR("invalid parameters\n");
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;
	dsi_intr_ctx = &to_dsi_connector(connector)->intr_ctx;
	conn_intr_ctx = &dsi_intr_ctx->base;

	dpu_online_irq_status_dump(intr_ctx, conn_intr_ctx);
}

static void dsi_irq_handler(int intr_id, void *data)
{
	struct dpu_connector_intr_context *conn_intr_ctx;
	struct dsi_intr_context *dsi_intr_ctx;
	struct dpu_crtc_state *cstate = NULL;
	struct dsi_connector *dsi_connector;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	struct dpu_intr_pipe *intr_pipe;
	enum dpu_intr_type intr_type;
	struct dpu_crtc *dpu_crtc;
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;
	char vsync_name[128];
	u64 timestamp;
	int pipe_id;

	intr_ctx = data;
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX)
		return;

	intr_pipe = &intr_ctx->intr_pipe[intr_type];
	crtc = intr_pipe->crtc;
	if (!crtc) {
		DPU_ISR_DEBUG("crtc is null\n");
		return;
	}

	pipe_id = drm_crtc_index(crtc);
	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	pipeline = &dpu_kms->virt_pipeline[pipe_id];
	dsi_connector = to_dsi_connector(intr_pipe->connector);
	dsi_intr_ctx = &dsi_connector->intr_ctx;
	conn_intr_ctx = &dsi_intr_ctx->base;
	cstate = to_dpu_crtc_state(crtc->state);
	dpu_crtc = to_dpu_crtc(crtc);

	if (intr_id == conn_intr_ctx->intr_id[DPU_VSYNC_INT]) {
		DPU_ISR_DEBUG("vsync interrupt\n");
		timestamp = ktime_get_mono_fast_ns();
		snprintf(vsync_name, sizeof(vsync_name), "vsync_%llu", timestamp);
		trace_dsi_irq_handler(vsync_name, intr_type);
		dsi_display_dyn_freq_work_trigger(dsi_connector->display);
		dpu_intr_online_vsync_handler(pipeline, timestamp);
		dpu_vsync_isr_idle_handler(crtc);
		dpu_wfd_notify_event(XPLAYER_EVENT_VSYNC);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_UNDERFLOW_INT]) {
		DPU_ISR_DEBUG("underflow interrupt\n");
		trace_dsi_irq_handler("underflow", intr_id);
		/* cmd mode underflow, need do dpu execption recovery */
		if (dsi_connector_is_cmd_mode(intr_pipe->connector)) {
			atomic_set(&pipeline->hw_hang, 1);
			wake_up(&pipeline->cfg_done_wq);
		}

		dpu_intr_online_underflow_handler(intr_ctx, pipeline, intr_id);

		dsi_intr_ctx->underflow_frame_no = cstate->frame_no;
		dpu_crtc->underflow_frame_no = cstate->frame_no;
		queue_work(pipeline->pipe_wq, &dsi_intr_ctx->underflow_work);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_VSTART_INT]) {
		DPU_ISR_DEBUG("vact_start interrupt\n");
		trace_dsi_irq_handler("vact_start", intr_type);
		dpu_intr_online_vstart_handler(pipeline);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_CFG_RDY_CLR_INT]) {
		DPU_ISR_DEBUG("cfg_rdy_clr interrupt\n");
		trace_dsi_irq_handler("cfg_rdy_clr", intr_id);
		dpu_intr_online_cfg_rdy_clr_handler(pipeline);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_EOF_INT]) {
		DPU_ISR_DEBUG("eof interrupt\n");
		if (!atomic_read(&pipeline->underflow_flag))
			dpu_exception_dbg_irq_clear(pipeline, crtc);
		trace_dsi_irq_handler("eof interrupt", intr_id);
		if (!dsi_connector_is_cmd_mode(intr_pipe->connector))
			atomic_set(&pipeline->vstart_flag, 0);
	} else if (intr_id == conn_intr_ctx->intr_id[DPU_CFG_WIN_START_INT]) {
		DPU_ISR_DEBUG("cfg win start interrupt\n");
		trace_dsi_irq_handler("cfg win start interrupt", intr_id);
	}

	/* this func needs rely on EOF INTR */
	dpu_kms->funcs->handle_intr_events(dpu_kms, crtc, intr_id);
}

int dsi_interrupt_register(struct drm_connector *connector, u32 intr_type)
{
	struct dpu_connector_intr_context *conn_intr_ctx;
	struct dsi_intr_context *dsi_intr_ctx;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;

	dsi_intr_ctx = &to_dsi_connector(connector)->intr_ctx;
	conn_intr_ctx = &dsi_intr_ctx->base;
	dpu_drm_dev = to_dpu_drm_dev(connector->dev);
	intr_ctx = dpu_drm_dev->dpu_kms->virt_ctrl.intr_ctx;

	if (intr_type >= DPU_INTR_TYPE_MAX) {
		DPU_ERROR("invalid intr_type %d\n", intr_type);
		return -EINVAL;
	}

	dsi_intr_ctx->intr_pipe = &intr_ctx->intr_pipe[intr_type];
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_CFG_RDY_CLR_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_VSYNC_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_VSTART_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_UNDERFLOW_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_EOF_INT);
	INIT_ONLINE_CONN_INTR(conn_intr_ctx, intr_type, DPU_CFG_WIN_START_INT);

	return dpu_intr_online_irq_register(connector, conn_intr_ctx, dsi_irq_handler);
}

void dsi_interrupt_unregister(struct drm_connector *connector, u32 intr_type)
{
	struct dsi_intr_context *dsi_intr_ctx;
	struct dsi_connector *dsi_connector;

	dsi_connector = to_dsi_connector(connector);
	dsi_intr_ctx = &dsi_connector->intr_ctx;

	dpu_intr_online_irq_unregister(connector, &dsi_intr_ctx->base);
}
