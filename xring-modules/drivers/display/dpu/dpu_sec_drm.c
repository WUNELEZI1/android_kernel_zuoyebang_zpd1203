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
#include <drm/drm_atomic_helper.h>
#include <drm/drm_plane.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_writeback.h>
#include "dpu_hw_rch.h"
#include "dpu_plane.h"
#include "dpu_wb.h"

int dpu_plane_create_sec_drm_properties(struct drm_plane *plane)
{
	struct drm_property *property;
	struct dpu_plane *dpu_plane;

	if (!plane) {
		DPU_ERROR("invalid parameters, %pK\n", plane);
		return -EINVAL;
	}
	dpu_plane = to_dpu_plane(plane);

	property = drm_property_create_range(plane->dev, 0,
					"rch_protected_buffer", 0, 1);
	if (!property) {
		DPU_ERROR("create failed\n");
		return -ENOMEM;
	}

	drm_object_attach_property(&plane->base, property, 0);
	dpu_plane->protected_buffer_status_prop = property;

	return 0;
}

int dpu_wb_create_sec_drm_properties(struct drm_writeback_connector *wb_conn)
{
	struct dpu_wb_connector *dpu_wb_conn;
	struct drm_property *property;

	if (!wb_conn) {
		DPU_ERROR("invalid parameters, %pK\n", wb_conn);
		return -EINVAL;
	}

	dpu_wb_conn = to_dpu_wb_connector(wb_conn);
	property = drm_property_create_range(wb_conn->base.dev, 0,
			"wb_protected_buffer_enable_prop", 0, 1);
	if (!property) {
		DPU_ERROR("drm_property_create fail: wb_protected_buffer_enable_prop\n");
		return -ENOMEM;
	}
	drm_object_attach_property(&wb_conn->base.base, property, 0);
	dpu_wb_conn->protected_buffer_enable_prop = property;

	return 0;
}

static bool dpu_plane_check_protected_enable(struct drm_plane *plane,
		struct drm_atomic_state *state)
{
	struct drm_plane_state *plane_state;
	struct dpu_plane_state *dpu_plane_state;
	bool is_protected_read_enable = false;

	if (!plane || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
				plane, state);
		return is_protected_read_enable;
	}

	plane_state = drm_atomic_get_old_plane_state(state, plane);
	dpu_plane_state = to_dpu_plane_state(plane_state);
	is_protected_read_enable = dpu_plane_state->is_protected_buffer;

	return is_protected_read_enable;
}

u8 dpu_sec_drm_rdma_cfg(struct drm_plane *plane, struct drm_atomic_state *state)
{
	enum protected_status drm_enable_status = HOLD;
	struct drm_plane_state *plane_state;
	struct dpu_plane_state *dpu_plane_state;

	plane_state = plane->state;
	dpu_plane_state = to_dpu_plane_state(plane_state);
	if (dpu_plane_check_protected_enable(plane, state))
		drm_enable_status = SWITCH_OFF;
	else
		drm_enable_status = HOLD;

	if (dpu_plane_state->is_protected_buffer)
		drm_enable_status = SWITCH_ON;

	return drm_enable_status;
}

void dpu_sec_drm_wb_cfg(struct wb_frame_cfg *wb_cfg, struct drm_atomic_state *state,
			struct drm_connector *connector)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector_state *wb_state;
	struct drm_connector_state *conn_state;
	struct dpu_wb_connector *dpu_wb_conn;

	if (!connector || !state) {
		DPU_ERROR("invalid parameters, %pK, %pK\n",
			connector, state);
		return;
	}
	// generally speaking, it need to do nothing
	wb_cfg->drm_enable_status = HOLD;

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	conn_state = connector->state;
	wb_state = to_dpu_wb_connector_state(conn_state);
	// if mid status is enable, it need to be changed to disable and reset mid register at first
	if (dpu_wb_conn->mid_enable) {
		wb_cfg->drm_enable_status = SWITCH_OFF;
		dpu_wb_conn->mid_enable = false;
	}

	// if current frame is protected, it need to set mid register and record mid enable
	if (wb_state->protected_buffer_enable) {
		wb_cfg->drm_enable_status = SWITCH_ON;
		dpu_wb_conn->mid_enable = true;
	}

}
