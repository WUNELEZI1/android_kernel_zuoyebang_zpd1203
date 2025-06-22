// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <drm/drm_atomic.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mode.h>

#include "dpu_crtc.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_cmdlist.h"
#include "dpu_cmdlist_node.h"
#include "dpu_log.h"
#include "dpu_power_helper.h"

struct hw_ulps_state {
	u64 enter_num;
	u64 exit_num;
	u64 enter_frame_no;
	u64 exit_frame_no;
	u32 vote_count;
};

#define HW_ULPS_REG_NUM 10
static struct hw_ulps_state g_state;
static DEFINE_MUTEX(g_ctrl_lock);

void dpu_hw_ulps_ctrl_dyn(bool enable)
{
	mutex_lock(&g_ctrl_lock);
	if (enable)
		g_state.vote_count--;
	else
		g_state.vote_count++;
	mutex_unlock(&g_ctrl_lock);
}

static bool need_enter_hw_ulps(struct drm_crtc_state *new_state,
		struct dpu_crtc *dpu_crtc, struct dpu_crtc_state *dpu_crtc_state)
{
	struct dpu_power_ctrl *power_ctrl;
	u32 new_frame_rate;

	if (g_state.vote_count > 0)
		return false;

	if ((dpu_crtc->underflow_frame_no > 0) &&
			(dpu_crtc_state->frame_no - dpu_crtc->underflow_frame_no < 10)) {
		DPU_INFO("underflow just occur, return. cur frm:%llu underflow frm:%llu\n",
				dpu_crtc_state->frame_no, dpu_crtc->underflow_frame_no);
		return false;
	}

	if (dpu_crtc->hw_ulps_enable)
		return false;

	new_frame_rate = drm_mode_vrefresh(&new_state->mode);
	power_ctrl = get_power_ctrl_inst(new_state->crtc);
	if (!power_ctrl)
		return false;

	if (!new_state->active) {
		DSI_DEBUG("no need enable: power down\n");
		return false;
	}

	if (new_frame_rate > power_ctrl->hw_ulps_max_refresh_rate) {
		DSI_DEBUG("no need enable: fps:%u rate:%d\n",
				new_frame_rate, power_ctrl->hw_ulps_max_refresh_rate);
		return false;
	}

	DSI_DEBUG("need enter hw ulps, new_frame_rate:%u\n", new_frame_rate);
	return true;
}

static bool need_exit_hw_ulps(struct dpu_crtc *dpu_crtc)
{
	return dpu_crtc->hw_ulps_enable;
}

static void dpu_connector_ulps_prepare(struct dsi_connector_state *dsi_conn_state,
		int cmps_id, bool enable)
{
	u16 node_len = ALIGN(HW_ULPS_REG_NUM, NODE_ITEM_ALIGN) / NODE_ITEM_ALIGN;

	if (enable)
		dsi_conn_state->ulps_enter_node_id = cmdlist_node_create(RCH_MAX + cmps_id,
				NORMAL_TYPE, node_len, 0);
	else
		dsi_conn_state->ulps_exit_node_id = cmdlist_node_create(RCH_MAX + cmps_id,
				NORMAL_TYPE, node_len, PRI_NO_BUSY0);
	DSI_DEBUG("create %s node: ch:%d len:%d\n",
			enable ? "enter" : "exit", RCH_MAX + cmps_id, node_len);
}

static int dpu_hw_ulps_config(struct drm_connector *connector, u8 transfer_type, bool enable)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct dsi_display *display;
	int ret;

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	dsi_conn_state = to_dsi_connector_state(connector->state);

	if (enable)
		ret = dsi_display_ulps_ctrl(display, ENTER_ULPS, transfer_type,
					dsi_conn_state->ulps_enter_node_id);
	else
		ret = dsi_display_ulps_ctrl(display, EXIT_ULPS, transfer_type,
					dsi_conn_state->ulps_exit_node_id);
	if (ret)
		DPU_ERROR("ulps_config failed, enable:%d", enable);

	return ret;
}

void dpu_hw_ulps_enable_with_cmdlist(struct drm_crtc_state *new_state)
{
	struct drm_connector *connector = NULL;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dsi_connector_state *dsi_conn_state;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_crtc *dpu_crtc;

	if (!new_state) {
		DPU_ERROR("new crtc state is null\n");
		return;
	}

	power_ctrl = get_power_ctrl_inst(new_state->crtc);
	if (!power_ctrl || !(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return;

	dpu_crtc_state = to_dpu_crtc_state(new_state);
	dpu_crtc = to_dpu_crtc(new_state->crtc);

	mutex_lock(&g_ctrl_lock);
	if (!need_enter_hw_ulps(new_state, dpu_crtc, dpu_crtc_state))
		goto exit;

	connector = find_connector(new_state->crtc->dev,
			new_state->connector_mask, DRM_MODE_CONNECTOR_DSI);
	if (connector) {
		dpu_drm_dev = to_dpu_drm_dev(connector->dev);
		pipeline = &dpu_drm_dev->dpu_kms->virt_pipeline[drm_crtc_index(new_state->crtc)];
		dsi_conn_state = to_dsi_connector_state(connector->state);
		dpu_idle_enable_ctrl(false);
		dpu_connector_ulps_prepare(dsi_conn_state, dpu_crtc->hw_mixer->hw.blk_id, true);
		dpu_hw_ulps_config(connector, USE_CMDLIST, true);
		cmdlist_frame_node_append(pipeline->cmdlist_frame_id,
				&dsi_conn_state->ulps_enter_node_id, DELETE_WITH_FRAME);
		dpu_crtc->hw_ulps_enable = true;
		g_state.enter_num++;
		g_state.enter_frame_no = dpu_crtc_state->frame_no;
		DPU_DEBUG("set hw_ulps_enable to %d, last_enter_frame:%llu enter_num:%llu\n",
				dpu_crtc->hw_ulps_enable, g_state.enter_frame_no, g_state.enter_num);
	}

exit:
	mutex_unlock(&g_ctrl_lock);
}

void dpu_hw_ulps_disable_with_cmdlist(struct drm_crtc_state *new_state)
{
	struct drm_connector *connector = NULL;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dsi_connector_state *dsi_conn_state;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_crtc *dpu_crtc;

	if (!new_state) {
		DPU_ERROR("new crtc state is null\n");
		return;
	}

	power_ctrl = get_power_ctrl_inst(new_state->crtc);
	if (!power_ctrl || !(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return;

	dpu_crtc_state = to_dpu_crtc_state(new_state);
	dpu_crtc = to_dpu_crtc(new_state->crtc);

	mutex_lock(&g_ctrl_lock);
	if (!need_exit_hw_ulps(dpu_crtc))
		goto exit;

	if (need_disable_crtc(new_state)) {
		DPU_INFO("disp off frame use cpu exit ulps, bypass\n");
		goto exit;
	}
	connector = find_connector(new_state->crtc->dev,
			new_state->connector_mask, DRM_MODE_CONNECTOR_DSI);
	if (connector) {
		dpu_drm_dev = to_dpu_drm_dev(connector->dev);
		pipeline = &dpu_drm_dev->dpu_kms->virt_pipeline[drm_crtc_index(new_state->crtc)];
		dsi_conn_state = to_dsi_connector_state(connector->state);
		dpu_idle_enable_ctrl(true);
		dpu_connector_ulps_prepare(dsi_conn_state, dpu_crtc->hw_mixer->hw.blk_id, false);
		dpu_hw_ulps_config(connector, USE_CMDLIST, false);
		cmdlist_frame_node_append(pipeline->cmdlist_frame_id,
				&dsi_conn_state->ulps_exit_node_id, DELETE_WITH_FRAME);
		dpu_crtc->hw_ulps_enable = false;
		g_state.exit_num++;
		g_state.exit_frame_no = dpu_crtc_state->frame_no;
		DPU_DEBUG("set hw_ulps_enable to %d, exit_frame:%llu exit_num:%llu\n",
				dpu_crtc->hw_ulps_enable, g_state.exit_frame_no, g_state.exit_num);
	}

exit:
	mutex_unlock(&g_ctrl_lock);
}

void dpu_hw_ulps_disable(struct drm_crtc_state *new_state)
{
	struct drm_connector *connector = NULL;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_crtc *dpu_crtc;

	if (!new_state) {
		DPU_ERROR("new crtc state is null\n");
		return;
	}

	power_ctrl = get_power_ctrl_inst(new_state->crtc);
	if (!power_ctrl || !(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return;

	dpu_crtc = to_dpu_crtc(new_state->crtc);

	mutex_lock(&g_ctrl_lock);
	if (!need_exit_hw_ulps(dpu_crtc) || !need_disable_crtc(new_state))
		goto exit;

	connector = dsi_primary_connector_get(new_state->crtc->dev);
	if (connector) {
		usleep_range(10000, 11000); /* Ensure the DPU hardware work is complete */
		dpu_hw_ulps_config(connector, USE_CPU, false);
		dpu_idle_enable_ctrl(true);
		dpu_crtc->hw_ulps_enable = false;
		g_state.exit_num++;
		DPU_INFO("disp off frame need exit ulps by cpu\n");
	}

exit:
	mutex_unlock(&g_ctrl_lock);
}

void dpu_hw_ulps_status_dump(struct drm_crtc_state *new_state)
{
	struct dpu_crtc *dpu_crtc;
	struct drm_connector *connector = NULL;
	struct dpu_power_ctrl *power_ctrl;
	int ret;

	if (!new_state) {
		DPU_ERROR("new crtc state is null\n");
		return;
	}

	power_ctrl = get_power_ctrl_inst(new_state->crtc);
	if (!power_ctrl || !(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return;

	dpu_crtc = to_dpu_crtc(new_state->crtc);
	DPU_INFO("[HW ULPS] status:%d last enter:%llu enter num:%llu last exit:%llu exit num:%llu\n",
			dpu_crtc->hw_ulps_enable,
			g_state.enter_frame_no, g_state.enter_num, g_state.exit_frame_no, g_state.exit_num);

	connector = dsi_primary_connector_get(new_state->crtc->dev);
	if (connector) {
		ret = dsi_connector_check_ulps_state(connector, ENTER_ULPS);
		if (!ret)
			DPU_INFO("dsi is in ulps state\n");
		else
			DPU_INFO("dsi is not in ulps state\n");
	}
}

void dpu_hw_ulps_exception_recovery(struct drm_crtc_state *new_state,
		struct drm_connector *connector)
{
	struct dpu_power_ctrl *power_ctrl;

	if (!new_state || !connector) {
		DPU_ERROR("invalid input:%pK %pK\n", new_state, connector);
		return;
	}

	power_ctrl = get_power_ctrl_inst(new_state->crtc);
	if (!power_ctrl || !(power_ctrl->lowpower_ctrl & DPU_LP_HW_ULPS) ||
			!is_dpu_lp_enabled(DPU_LP_HW_ULPS))
		return;

	dpu_hw_ulps_config(connector, USE_CPU, EXIT_ULPS);
}
