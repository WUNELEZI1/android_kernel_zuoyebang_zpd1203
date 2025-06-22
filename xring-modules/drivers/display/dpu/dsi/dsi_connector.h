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

#ifndef _DSI_CONNECTOR_H_
#define _DSI_CONNECTOR_H_

#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>
#include <drm/drm_device.h>

#include "dsi_display.h"
#include "dpu_intr_helper.h"

struct dpu_connector_intr_context;
/**
 * dsi_intr_context - dsi interrupt context
 * @intr_pipe: the point of dpu intr pipe
 * @base: the point of dpu connector intr context
 * @underflow_work: work for underflow intr
 * @underflow_frame_no: the frame no has underflow
 */
struct dsi_intr_context {
	struct dpu_intr_pipe *intr_pipe;

	struct dpu_connector_intr_context base;
	struct work_struct underflow_work;
	u64 underflow_frame_no;
};

/**
 * dsi_connector_features - dsi connector features
 */
enum dsi_connector_features {
	/* Append new dsi connector features before DSI_CONNECTOR_FEATURE_MAX */
	DSI_CONNECTOR_FEATURE_ROI,
	DSI_CONNECTOR_FEATURE_MAX
};

/**
 * dsi_connector_state - the status of dsi connector
 * @base: the drm connector state
 * @roi_cfg: the dsi roi config
 * @power_mode: power mode property setting from user space
 * @pm_changed: power mode value change
 * @node_id: cmdlist node id for dsi
 */
struct dsi_connector_state {
	struct drm_connector_state base;
	struct drm_clip_rect roi_cfg;
	bool bl_changed;
	u32 bl_level;
	bool need_at_once;
	bool has_cmdlist_cmd;
	bool already_get_mipi_state;

	u16 damage_x;
	u16 damage_y;
	u16 damage_width;
	u16 damage_height;

	u32 power_mode;
	bool pm_changed;

	s64 node_id;
	s64 ulps_exit_node_id;
	s64 ulps_enter_node_id;
};

#define to_dsi_connector_state(x) \
	container_of((x), struct dsi_connector_state, base)

/**
 * dsi_connector - the structure of dsi connector
 * @base: the drm connector
 * @current_power_mode: current power mode setting
 * @conn_lock: the dsi connector lock
 * @display: the dsi display phandle
 * @encoder: the drm encoder phandle
 * @drm_dev: the drm device phandle
 * @intr_ctx: the interrupt context of dsi connector
 * @roi_prop: the dsi roi property
 * @blob_caps: the dsi blob caps property
 * @hdr_prop: the dsi hdr property
 * @hdr_blob: the dsi hdr blob property
 * @panel_caps_prop: the panel caps property
 * @power_mode_prop: the dsi power mode property
 * @sw_clr_fail_cnt: counter for continous sw clr fail
 * @continous_succ_cnt: counter for continous present success
 */
struct dsi_connector {
	struct drm_connector base;

	u32 current_power_mode;
	struct mutex conn_lock;

	struct dsi_display *display;
	struct drm_encoder *encoder;
	struct drm_device *drm_dev;

	struct dsi_intr_context intr_ctx;

	struct drm_property *damage_align_size_prop;
	struct drm_property *damage_area_prop;
	struct drm_property *roi_prop;
	struct drm_property_blob *blob_caps;
	struct drm_property *hdr_prop;
	struct drm_property_blob *hdr_blob;
	struct drm_property *bl_prop;
	struct drm_property *panel_caps_prop;
	struct drm_property_blob *panel_caps_blob;
	struct drm_property *power_mode_prop;
	struct drm_property *at_once_prop;

	int sw_clr_fail_cnt;
	int continous_succ_cnt;
};

#define to_dsi_connector(x) container_of(x, struct dsi_connector, base)

inline void dsi_connector_lock(struct dsi_connector *dsi_conn);

inline void dsi_connector_unlock(struct dsi_connector *dsi_conn);

/**
 * dsi_connector_ulps_enter - enter the ulps mode
 * @connector: drm connector phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_connector_ulps_enter(struct drm_connector *connector);

/**
 * dsi_connector_ulps_exit - exit the ulps mode
 * @connector: drm connector phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_connector_ulps_exit(struct drm_connector *connector);

/**
 * dsi_connector_create - create the dsi connector object
 * @drm_dev: the pointer of drm device
 * @drm_encoder: the connector corresponding encoder
 * @display: the dsi display
 *
 * Return: valid pointer on success, invalid pointer on failure
 */
struct drm_connector *dsi_connector_create(struct drm_device *drm_dev,
		struct drm_encoder *drm_encoder, struct dsi_display *display);

/**
 * dsi_connector_destroy - destroy the dsi connector instance and release
 * reserved resources.
 * @drm_connector: the drm connector phandle
 */
void dsi_connector_destroy(struct drm_connector *drm_connector);

/**
 * dsi_interrupt_register - regitster the dsi interrupt
 * @connector: the phandle of drm connector
 * @intr_type: the dpu interrupt type for this connector
 *
 * Return: 0 on success or negative error code on failure.
 */
int dsi_interrupt_register(struct drm_connector *connector, u32 intr_type);

/**
 * dsi_interrupt_unregister - unregitster the dsi interrupt
 * @connector: the phandle of drm connector
 * @intr_type: the dpu interrupt type for this connector
 */
void dsi_interrupt_unregister(struct drm_connector *connector, u32 intr_type);

/**
 * dsi_connector_is_cmd_mode - is dsi in cmd mode
 * @connector: the phandle of drm connector
 *
 * Return: true if the dsi is cmd mode or false otherwise.
 */
bool dsi_connector_is_cmd_mode(struct drm_connector *connector);

/**
 * dsi_primary_connector_get - get primary connector
 * @drm_dev: the phandle of drm device
 *
 * Return: valid pointer on success, error or null pointer on failure
 */
struct drm_connector *dsi_primary_connector_get(struct drm_device *drm_dev);

/**
 * dsi_connector_damage_area_update - update the damage area
 * @connector: the pointer of drm connector
 * @state: the pointer of drm connector state
 *
 * Return: zero on success, -errno on failure
 */
int dsi_connector_damage_area_update(struct dsi_connector *connector,
		struct drm_atomic_state *state);

/**
 * damage_area_retained - check the damage area is retained
 * @connector: the pointer of drm connector
 * @state: the pointer of drm connector state
 *
 * Return: zero on damage area unchanged, false on changed
 */
bool damage_area_retained(struct drm_connector *connector,
		struct drm_atomic_state *state);

/**
 * dsi_connector_clk_ctrl - ctrl dsi clk for connector
 *
 * @connector: the pointer of drm connector
 * @enable: bool for enable and disable
 * Return: zero on success, else on failure
 */
int dsi_connector_clk_ctrl(struct drm_connector *connector, bool enable);

/**
 * dsi_connector_ctrl_mode_init - dsi controller enable
 *
 * @connector: the pointer of drm connector
 * Return: void
 */
void dsi_connector_ctrl_mode_init(struct drm_connector *connector);

int dsi_connector_check_ulps_state(struct drm_connector *connector, bool enter);

void dsi_connector_mipi_cmd_state_get(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state);

void dsi_connector_mipi_cmd_state_clear(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state);

/**
 * need_update_damage_area - check whether current damage area need update
 * @connector: the pointer for drm_connector
 *
 */
static inline bool need_update_damage_area(struct drm_connector *connector,
		struct drm_atomic_state *state, struct dsi_display *display)
{
	if (!display->panel->panel_info.partial_update_enable)
		return false;

	if (damage_area_retained(connector, state))
		return false;

	return true;
}

void dpu_dsi_irq_status_dump(struct drm_connector *connector);

void dsi_connector_finish(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state);

#endif
