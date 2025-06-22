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
#include <drm/drm_plane.h>
#include <drm/drm_crtc.h>
#include <drm/drm_vblank.h>
#include <drm/drm_atomic_helper.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <uapi/linux/sched/types.h>
#include <linux/of.h>

#include "dpu_drv.h"
#include "dpu_kms.h"
#include "dpu_crtc.h"
#include "dpu_plane.h"
#include "dpu_wb.h"
#include "dsi_connector.h"
#include "dsi_encoder.h"
#include "dsi_display.h"
#include "dsi_tmg.h"
#include "dpu_cmdlist.h"
#include "dpu_color.h"
#include "dpu_res_helper.h"
#include "dpu_idle_helper.h"
#include "dpu_power_mgr.h"
#include "dpu_power_helper.h"
#include "dpu_cont_display.h"
#include "dp_drm.h"
#include "dpu_core_perf.h"
#include "dpu_exception.h"
#include "dksm_mem_pool.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_flow_ctrl.h"
#include "dsi_display.h"
#include "dpu_obuf.h"
#include "dpu_tui.h"
#include "dpu_gem.h"
#include "dksm_iommu.h"
#include "dpu_wfd.h"
#include "dpu_acquire_fence.h"
#include "dpu_release_fence.h"
#include "dpu_buf_sync.h"
#include "dpu_hw_ulps.h"
#include "dpu_intr_core.h"

#define CREATE_TRACE_POINTS
#include "dpu_trace.h"
#undef CREATE_TRACE_POINTS

#define DPU_ACQUIRE_FENCE_TIMEOUT_MS 3000
#define FRAME_DURATION_FPGA_MS 10000
#define FRAME_DURATION_ASIC_MS 300
#define WAIT_MIPI_DYN_FREQ_TIMEOUT_MS 1000

#define CONNECTOR_DISABLING	0
#define CONNECTOR_ENABLING	1
#define CONNECTOR_UNCHANGED	2

#define MAX_SW_CLR_FAIL_THRESHOLD 6
#define SUCC_PRESENT_THRESHOLD    6

static inline struct dpu_kms *to_dpu_kms(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;

	dpu_drm_dev = container_of(drm_dev, struct dpu_drm_device, drm_dev);

	return dpu_drm_dev->dpu_kms;
}

static inline bool need_enable_update(struct drm_crtc_state *new_state)
{
	struct dpu_crtc_state *dpu_crtc_state;

	if (is_cont_display_enabled() && is_primary_display(new_state)) {
		DPU_DEBUG("cont display, return\n");
		return true;
	}

	dpu_crtc_state = to_dpu_crtc_state(new_state);

	if (!new_state->active && !new_state->active_changed)
		return false;

	/**
	 * the following sequence don't need enable update:
	 * 1. don't need to display at power-on frame
	 * 2. AOD sequence, at this point plane_mask is true, but plane_changed is false
	 * 3. power-off frame
	 */
	if ((!new_state->plane_mask && !dpu_crtc_state->solid_layer.count &&
			need_enable_crtc(new_state)) ||
			(new_state->plane_mask && !new_state->planes_changed) ||
			need_disable_crtc(new_state))
		return false;
	else
		return true;
}

static inline u8 __is_connector_state_changed(struct drm_atomic_state *state,
		struct drm_crtc_state *new_crtc_state,
		struct drm_connector *connector)
{
	struct drm_connector_state *old_state, *new_state;

	if (!connector)
		return CONNECTOR_UNCHANGED;

	old_state = drm_atomic_get_old_connector_state(state, connector);
	new_state = drm_atomic_get_new_connector_state(state, connector);
	if (!old_state || !new_state) {
		DPU_WARN("NULL PTR. %pK %pK\n", old_state, new_state);
		return CONNECTOR_UNCHANGED;
	}

	if (old_state->crtc != new_state->crtc)
		return !!new_state->crtc;
	else if (new_crtc_state->active_changed)
		return new_crtc_state->active;
	else
		return CONNECTOR_UNCHANGED;
}

struct dpu_kms_connector_iter {
	u32 mask;
	u8 enabling;
};

/**
 * for_each_state_changed_connector_per_crtc - iterate the all state changed
 * (active state, connect state, and so on) connector per crtc by crtc old new state
 * @connector: drm connector pointer for each connector instance
 * @state: drm atomic state pointer
 * @old_state: drm crtc old state pointer
 * @new_state: drm crtc new state pointer
 * @iter: the variable of struct dpu_kms_connector_iter
 *
 * Note: internal macro function, do not move it outside of this file
 */
#define for_each_state_changed_connector_per_crtc(connector, state, old_state, new_state, iter) \
	for (iter.mask = old_state->connector_mask | new_state->connector_mask, \
		connector = __find_connector(state->dev, &iter.mask); \
		iter.enabling = __is_connector_state_changed(state, new_state, connector), \
		connector; connector = __find_connector(state->dev, &iter.mask)) \
		for_each_if(iter.enabling != CONNECTOR_UNCHANGED)

bool is_clone_mode(struct drm_atomic_state *state,
		struct drm_connector_state *connector_state)
{
	struct drm_crtc_state *crtc_state;

	if (!state || !connector_state || !connector_state->crtc)
		return false;

	crtc_state = drm_atomic_get_new_crtc_state(state,
			connector_state->crtc);

	return !!(hweight32(crtc_state->connector_mask) > 1);
}

int get_cloned_connector_type(struct drm_atomic_state *state,
		struct drm_connector_state *connector_state, int *id)
{
	struct drm_connector *connector;
	struct drm_crtc_state *crtc_state;
	u32 mask;

	if (!state || !connector_state || !connector_state->crtc)
		return -EINVAL;

	crtc_state = drm_atomic_get_new_crtc_state(state,
			connector_state->crtc);

	if (!is_clone_mode(state, connector_state))
		return -EINVAL;

	for_each_connector_per_crtc(connector, crtc_state, mask) {
		if (connector->connector_type != DRM_MODE_CONNECTOR_WRITEBACK) {
			if (id)
				*id = connector->connector_type_id;
			return connector->connector_type;
		}
	}

	return DRM_MODE_CONNECTOR_Unknown;
}

void dpu_force_refresh_event_notify(struct drm_device *drm_dev)
{
	struct drm_event event = {0};
	bool flag = true;

	if (!drm_dev) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	event.type = DRM_PRIVATE_EVENT_FORCE_REFRESH;
	event.length = sizeof(flag);
	dpu_drm_event_notify(drm_dev, &event, (u8 *)&flag);
	DPU_INFO("force refresh event success\n");
}

static int dpu_kms_components_init(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_crtc *temp_crtc;
	struct drm_plane *temp_plane;
	enum drm_plane_type plane_type;
	u32 possible_crtcs;
	int ret;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	possible_crtcs = BIT(MAX_CRTC_COUNT) - 1;

	for (i = 0; i < MAX_PLANE_COUNT; i++) {
		if (i < MAX_CRTC_COUNT)
			plane_type = DRM_PLANE_TYPE_PRIMARY;
		else
			plane_type = DRM_PLANE_TYPE_OVERLAY;

		temp_plane = dpu_plane_init(drm_dev,
				possible_crtcs, plane_type);
		if (IS_ERR_OR_NULL(temp_plane)) {
			DPU_ERROR("failed to init dpu plane %d\n", i);
			ret = PTR_ERR(temp_plane);
			goto error;
		}

		if (i >= MAX_CRTC_COUNT)
			continue;

		temp_crtc = dpu_crtc_init(drm_dev, temp_plane);
		if (IS_ERR_OR_NULL(temp_crtc)) {
			DPU_ERROR("failed to init crtc %d\n", i);
			ret = PTR_ERR(temp_crtc);
			goto error;
		}
	}

	return 0;
error:
	return ret;
}

static int dpu_kms_virtual_ctrl_init(struct dpu_kms *dpu_kms)
{
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_virt_ctrl *virt_ctrl;
	struct dpu_hw_blk *hw_blk;
	int ret;

	res_mgr_ctx = dpu_kms->res_mgr_ctx;
	virt_ctrl = &dpu_kms->virt_ctrl;

	hw_blk = dpu_res_mgr_block_reserve(res_mgr_ctx,
			VIRTUAL_CTRL_PIPE_ID, BLK_CTL_TOP, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("failed to reserve ctl top\n");
		return PTR_ERR(hw_blk);
	}
	virt_ctrl->hw_ctl_top = to_dpu_hw_ctl_top(hw_blk);

	hw_blk = dpu_res_mgr_block_reserve(res_mgr_ctx,
			VIRTUAL_CTRL_PIPE_ID, BLK_WB_TOP, false);
	if (IS_ERR_OR_NULL(hw_blk)) {
		DPU_ERROR("failed to reserve wb top\n");
		return PTR_ERR(hw_blk);
	}
	virt_ctrl->hw_wb_top = to_dpu_hw_wb_top(hw_blk);

	ret = dpu_interrupt_context_init(&virt_ctrl->intr_ctx,
			dpu_kms->drm_dev);
	if (ret) {
		DPU_ERROR("failed to initialize interrupt context\n");
		goto err_init;
	}

	return 0;

err_init:
	dpu_res_mgr_block_release(res_mgr_ctx, hw_blk);
	virt_ctrl->hw_ctl_top = NULL;
	return ret;
}

static void dpu_kms_virtual_ctrl_deinit(struct dpu_kms *dpu_kms)
{
	struct dpu_res_mgr_ctx *res_mgr_ctx;
	struct dpu_virt_ctrl *virt_ctrl;

	res_mgr_ctx = dpu_kms->res_mgr_ctx;
	virt_ctrl = &dpu_kms->virt_ctrl;

	dpu_interrupt_context_deinit(virt_ctrl->intr_ctx);

	if (virt_ctrl->hw_ctl_top) {
		dpu_res_mgr_block_release(res_mgr_ctx,
				&virt_ctrl->hw_ctl_top->hw);
		virt_ctrl->hw_ctl_top = NULL;
	}

	if (virt_ctrl->hw_wb_top) {
		dpu_res_mgr_block_release(res_mgr_ctx,
				&virt_ctrl->hw_wb_top->hw);
		virt_ctrl->hw_wb_top = NULL;
	}
}

static int dpu_hw_platform_info_get(struct dpu_kms *dpu_kms)
{
	struct platform_device *pdev;
	struct device_node *np;
	u32 asic_flag;
	int ret;

	pdev = to_platform_device(dpu_kms->drm_dev->dev);
	np = pdev->dev.of_node;

	dpu_kms->is_asic = true;
	dpu_kms->default_frame_timeout_ms = FRAME_DURATION_ASIC_MS;
	ret = of_property_read_u32(np, "asic-flag", &asic_flag);
	if (!ret && !asic_flag) {
		dpu_kms->is_asic = false;
		dpu_kms->default_frame_timeout_ms = FRAME_DURATION_FPGA_MS;
	}

	DPU_DEBUG("get is_asic: %d, default_frame_timeout_ms: %d\n",
			dpu_kms->is_asic, dpu_kms->default_frame_timeout_ms);

	return 0;
}

static int dpu_kms_res_init(struct dpu_kms *dpu_kms)
{
	struct drm_device *drm_dev;
	int ret;

	if (!dpu_kms) {
		DPU_ERROR("invalid parameter %pK\n", dpu_kms);
		return -EINVAL;
	}

	drm_dev = dpu_kms->drm_dev;

	/* Attention: res mgr must be init firstly */
	ret = dpu_res_mgr_context_init(&dpu_kms->res_mgr_ctx, drm_dev);
	if (ret) {
		DPU_ERROR("failed to init res_mgr context, ret %d\n", ret);
		return ret;
	}

	ret = dksm_mem_pool_init(drm_dev->dev, &dpu_kms->mem_pool);
	if (ret) {
		DPU_ERROR("failed to init dpu mem pool\n");
		return ret;
	}

	ret = dpu_power_mgr_init(&dpu_kms->power_mgr, drm_dev);
	if (ret) {
		DPU_ERROR("failed to init dpu power mgr\n");
		return ret;
	}

	ret = dpu_flow_ctrl_init(&dpu_kms->flow_ctrl, drm_dev);
	if (ret) {
		DPU_ERROR("failed to init dpu flow ctrl\n");
		return ret;
	}

	ret = dksm_iommu_context_init(drm_dev->dev, dpu_kms->mem_pool);
	if (ret) {
		DPU_ERROR("failed to init iommu resource, ret %d\n", ret);
		return ret;
	}

	ret = dpu_cmdlist_init(drm_dev, dpu_kms->mem_pool);
	if (ret) {
		DPU_ERROR("failed to init cmdlist, ret %d\n", ret);
		return ret;
	}

	ret = dpu_color_init(&dpu_kms->color, drm_dev);
	if (ret) {
		DPU_ERROR("failed to init dpu color\n");
		return ret;
	}

	ret = dpu_kms_components_init(drm_dev);
	if (ret) {
		DPU_ERROR("failed to init kms components, ret %d\n", ret);
		return ret;
	}

	ret = dpu_kms_virtual_ctrl_init(dpu_kms);
	if (ret) {
		DPU_ERROR("failed to init virtual ctrl, ret %d\n", ret);
		return ret;
	}

	ret = dpu_core_perf_init(&dpu_kms->core_perf, dpu_kms);
	if (ret) {
		DPU_ERROR("failed to init core perf\n");
		return ret;
	}

	dpu_cont_display_status_get(drm_dev);

	ret = dpu_hw_platform_info_get(dpu_kms);
	if (ret) {
		DPU_ERROR("failed to get hw platform info, ret %d\n", ret);
		return ret;
	}

	ret = dpu_tui_init(&dpu_kms->tui, drm_dev);
	if (ret) {
		DPU_ERROR("failed to init tui, ret %d\n", ret);
		return ret;
	}

	dpu_wfd_sink_cmdlist_init();

	dpu_kms->auto_recovery_en = true;
	dpu_kms->dpu_in_exceptional_state = false;
	dpu_kms->flush_bg_at_recovery = true;

	return 0;
}

static void dpu_kms_res_deinit(struct dpu_kms *dpu_kms)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct drm_device *drm_dev;

	if (!dpu_kms) {
		DPU_ERROR("invalid parameter %pK\n", dpu_kms);
		return;
	}

	drm_dev = dpu_kms->drm_dev;
	dpu_drm_dev = to_dpu_drm_dev(drm_dev);

	dpu_wfd_sink_cmdlist_deinit();

	dpu_core_perf_deinit(dpu_kms->core_perf);

	dpu_kms_virtual_ctrl_deinit(dpu_kms);

	dpu_color_deinit(dpu_kms->color);

	dpu_res_mgr_context_cleanup(dpu_kms->res_mgr_ctx);

	dpu_cmdlist_deinit();

	dksm_iommu_context_deinit();

	dpu_flow_ctrl_deinit(dpu_kms->flow_ctrl);

	dpu_power_mgr_deinit(dpu_kms->power_mgr);

	dksm_mem_pool_deinit(drm_dev->dev, dpu_kms->mem_pool);

	dpu_tui_deinit(dpu_kms->tui);
}

static int dpu_kms_check(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state)
{
	bool ret;

	if (!dpu_kms || !state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", dpu_kms, state);
		return -EINVAL;
	}

	if (!dpu_kms->auto_recovery_en &&
			dpu_kms->dpu_in_exceptional_state)
		return -EINVAL;

	ret = dpu_power_check(state);
	if (ret) {
		DPU_ERROR("dpu power check failed\n");
		return -EINVAL;
	}

	return dpu_res_helper_resource_reserve(dpu_kms, state, true);
}

/* Attention:
 * Shall register primary crtc when primary panel on
 * because primary crtc will be changed every time power on.
 */
static void dpu_idle_prepare(struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_crtc_state *old_dpu_state = to_dpu_crtc_state(old_state);
	struct dpu_crtc_state *new_dpu_state = to_dpu_crtc_state(new_state);

	if (need_enable_crtc(new_state) && is_primary_display(new_state))
		dpu_idle_register_primary_crtc();

	if ((old_dpu_state->keep_power_on > 0) && (new_dpu_state->keep_power_on == 0))
		dpu_idle_policy_switch(NORMAL);

	if ((old_dpu_state->keep_power_on == 0) && (new_dpu_state->keep_power_on > 0))
		dpu_idle_policy_switch(BASIC);
}

static void dpu_kms_wait_vstart(struct dpu_kms *dpu_kms,
		struct dpu_virt_pipeline *pipeline,
		struct drm_atomic_state *state,
		struct drm_crtc_state *old_crtc_state,
		struct drm_crtc_state *new_crtc_state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct drm_connector *connector;
	bool is_video_mode = true;
	struct drm_crtc *crtc;
	int connector_type;
	int iter_mask;
	int ret;

	/**
	 * @brief The following scenarios do not require wait timing:
	 * 1. Offline compose
	 * 2. without display frame
	 * 3. only display off with unmount config
	 * 4. power on frame
	 */
	if (pipeline->is_offline || !pipeline->updt_en || !new_crtc_state->connector_mask ||
			need_enable_crtc(new_crtc_state)) {
		DPU_DEBUG("Pipe %d, no need to wait for vstart!\n", pipeline->index);
		return;
	}

	for_each_connector_per_crtc(connector, new_crtc_state, iter_mask) {
		connector_type = connector->connector_type;
		if (connector_type == DRM_MODE_CONNECTOR_DSI) {
			if (dsi_connector_is_cmd_mode(connector))
				is_video_mode = false;
		}
	}

	crtc = new_crtc_state->crtc;
	dpu_crtc_state = to_dpu_crtc_state(old_crtc_state);

	ret = wait_event_timeout(pipeline->vstart_wq,
			(atomic_read(&pipeline->vstart_flag) != 0) &&
			atomic_read(&pipeline->cfg_done_flag),
			msecs_to_jiffies(pipeline->frame_duration_ms));

	if (!ret) {
		DPU_WARN("[CRTC:%d:%s (%s)(%s)][frame_no:%llu]: wait timeout for vstart_flag: %d, done_flag: %d!\n",
				crtc->base.id,
				crtc->name,
				is_video_mode ? "video mode" : "cmd mode",
				(atomic_read(&pipeline->underflow_flag) == 1) ? "underflow" : "no intr",
				dpu_crtc_state->frame_no,
				atomic_read(&pipeline->vstart_flag),
				atomic_read(&pipeline->cfg_done_flag));
	}
	/* After the consumption needs to be cleared */
	atomic_set(&pipeline->vstart_flag, 0);
}

static void dpu_kms_prepare(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state)
{
	struct drm_crtc_state *old_state, *new_state;
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct drm_crtc *crtc;
	int pipe_id;
	int i;

	if (!dpu_kms || !state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", dpu_kms, state);
		return;
	}

	dpu_res_helper_resource_reserve(dpu_kms, state, false);

	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i) {
		pipe_id = drm_crtc_index(crtc);
		virt_pipeline = &dpu_kms->virt_pipeline[pipe_id];

		dpu_idle_prepare(old_state, new_state);

		dpu_hw_ulps_disable(new_state);

		dsi_display_dyn_freq_wait(old_state, WAIT_MIPI_DYN_FREQ_TIMEOUT_MS);

		dpu_power_prepare(dpu_kms, state, crtc, old_state, new_state);

		dpu_core_perf_prepare(dpu_kms->core_perf, crtc,
				old_state, new_state);

		dpu_res_helper_update_crtc_dsc_cfg(state, crtc, new_state);

		hw_scene_ctl = dpu_kms->virt_pipeline[pipe_id].hw_scene_ctl;
		if (hw_scene_ctl) {
			hw_scene_ctl->ops.reset(&hw_scene_ctl->hw);
			dpu_cmdlist_prepare(crtc, new_state);
		}
		dpu_kms_wait_vstart(dpu_kms, virt_pipeline, state, old_state, new_state);

		dpu_crtc_hw_state_init(crtc);
	}
}

static bool need_restart_dpu_display(struct dpu_kms *dpu_kms,
		struct dpu_virt_pipeline *virt_pipeline,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	if (virt_pipeline->sw_clear_flag) {
		virt_pipeline->sw_clear_flag = false;
		DPU_INFO("software clear done\n");
		return true;
	}

	if (need_restart_dpu_display_after_idle(crtc))
		return need_disable_crtc(new_state) ? false : true;

	/**
	 * 1. power on
	 * 2. the first frame after power on without mount tmg
	 * 3. the first frame after exit cmd idle, if not update en
	 */
	if (need_enable_crtc(new_state)) {
		DPU_INFO("need enable crtc\n");
		return is_primary_display(new_state) ? !is_cont_display_enabled() : true;
	}

	if (virt_pipeline->delay_mount_connector) {
		virt_pipeline->delay_mount_connector = false;
		DPU_INFO("delay mount connector finish\n");
		return true;
	}

	if (need_enable_crtc(old_state) && !need_enable_update(old_state)) {
		DPU_INFO("need restart when power on\n");
		return is_primary_display(new_state) ? !is_cont_display_enabled() : true;
	}
	return false;
}

static void dpu_kms_start_first_frame(struct dpu_kms *dpu_kms,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		u32 pipe_id)
{
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct drm_connector *connector;
	bool is_online;
	u32 mask;

	if (!dpu_kms || (pipe_id >= MAX_CRTC_COUNT)) {
		DPU_ERROR("invalid parameter %pK, %u\n", dpu_kms, pipe_id);
		return;
	}

	virt_pipeline = &dpu_kms->virt_pipeline[pipe_id];
	hw_scene_ctl = virt_pipeline->hw_scene_ctl;

	if (!need_restart_dpu_display(dpu_kms, virt_pipeline, crtc, old_state, new_state))
		return;

	for_each_connector_per_crtc(connector, new_state, mask) {
		switch (connector->connector_type) {
		case DRM_MODE_CONNECTOR_DSI:
		case DRM_MODE_CONNECTOR_DisplayPort:
			is_online = true;
			break;
		case DRM_MODE_CONNECTOR_WRITEBACK:
			is_online = false;
			break;
		default:
			is_online = false;
			DPU_ERROR("unsupported connector type %d\n",
					connector->connector_type);
			break;
		}

		if (is_online) {
			DPU_DEBUG("start the first frame\n");
			hw_scene_ctl->ops.first_frame_start(&hw_scene_ctl->hw);
			virt_pipeline->sw_start_flag = true;
		}
	}
}

static void dpu_virt_pipe_rch_update(struct dpu_virt_pipeline *pipeline,
		struct dpu_hw_rch *hw_rch,
		struct dpu_hw_rch_top *hw_rch_top)
{
	struct dpu_virt_pipe_state *pipe_state = &pipeline->pending_state;

	pipe_state->rch[hw_rch->hw.blk_id] = hw_rch;
	pipe_state->rch_top[hw_rch_top->hw.blk_id] = hw_rch_top;
}

static void dpu_virt_pipe_rch_mask_update(struct dpu_virt_pipeline *pipeline,
		u32 rch_mask)
{
	struct dpu_virt_pipe_state *pipe_state = &pipeline->pending_state;

	pipe_state->rch_mask = rch_mask;
}

static void dpu_virt_pipe_mode_info_update(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *new_state,
		struct drm_crtc_state *old_state)
{
	struct dpu_virt_pipe_state *pipe_state = &pipeline->pending_state;

	if (new_state) {
		pipe_state->new_connector_mask = new_state->connector_mask;
		pipe_state->new_vrefresh_rate = drm_mode_vrefresh(&new_state->mode);
	}

	if (old_state) {
		pipe_state->old_connector_mask = old_state->connector_mask;
		pipe_state->old_vrefresh_rate = drm_mode_vrefresh(&old_state->mode);
	}
}

static void dpu_virt_pipe_state_clear(struct dpu_virt_pipeline *pipeline)
{
	struct dpu_virt_pipe_state *pipe_state = &pipeline->pending_state;

	memset(pipe_state, 0x0, sizeof(*pipe_state));
}

void dpu_virt_pipe_state_swap(struct dpu_virt_pipeline *pipeline)
{
	memcpy(&pipeline->working_state, &pipeline->pending_state,
			sizeof(pipeline->working_state));
	memset(&pipeline->pending_state, 0x0, sizeof(pipeline->pending_state));
}

void dpu_virt_pipe_partial_cfg_update(struct dpu_virt_pipeline *pipeline,
		u32 old_height,
		u32 new_height)
{
	pipeline->pending_state.partial_update = true;
	pipeline->pending_state.old_height = old_height;
	pipeline->pending_state.new_height = new_height;
}

static bool need_update_plane_mount_state(struct drm_crtc_state *new_state)
{
	/* valid flip commit */
	if (new_state->plane_mask)
		return true;

	if (is_cont_display_enabled() &&
			need_enable_crtc(new_state) && is_primary_display(new_state)) {
		DPU_INFO("cont display enable, shouldn't umount rch\n");
		return false;
	} else {
		return true;
	}
}

static void dpu_kms_update_plane_mount_state(struct dpu_kms *dpu_kms,
		struct drm_crtc *crtc, struct drm_crtc_state *new_state)
{
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_dbg_ctx *dbg_ctx;
	struct dpu_plane *dpu_plane;
	struct drm_plane *plane;
	u32 pipe_id;
	u32 rch_mask = 0;
	bool rch_vrt = false;
	u32 intr_type;
	int ret;

	pipe_id = drm_crtc_index(crtc);
	virt_pipeline = &dpu_kms->virt_pipeline[pipe_id];
	hw_scene_ctl = virt_pipeline->hw_scene_ctl;
	dbg_ctx = dpu_kms->dbg_ctx;

	if (!need_update_plane_mount_state(new_state))
		return;

	drm_for_each_plane_mask(plane, dpu_kms->drm_dev, new_state->plane_mask) {
		dpu_plane = to_dpu_plane(plane);

		if (dpu_plane->hw_rch) {
			rch_mask |= BIT(dpu_plane->hw_rch->hw.blk_id);
			dpu_virt_pipe_rch_update(virt_pipeline,
					dpu_plane->hw_rch, dpu_plane->hw_rch_top);

			dpu_plane_state = to_dpu_plane_state(plane->state);
			if (dpu_plane_state->extender_mode == EXTENDER_MODE_V)
				rch_vrt = true;
			hw_scene_ctl->ops.rch_vrt_config(&hw_scene_ctl->hw,
					dpu_plane->hw_rch->hw.blk_id, rch_vrt);
		} else {
			DPU_INFO("plane[%d]: hw_rch is null!\n",
					drm_plane_index(plane));
		}
	}

	if (hw_scene_ctl) {
		dpu_virt_pipe_rch_mask_update(virt_pipeline, rch_mask);
		hw_scene_ctl->ops.rch_mount(&hw_scene_ctl->hw, rch_mask);
		DPU_DEBUG("mount rch 0x%x to scene control %u\n",
				rch_mask, hw_scene_ctl->hw.blk_id);
		if (dbg_ctx) {
			ret = dpu_intr_type_get(crtc, &intr_type);
			if (!ret) {
				dbg_ctx->work_data[intr_type].old_rch_mask =
						dbg_ctx->work_data[intr_type].new_rch_mask;
				dbg_ctx->work_data[intr_type].new_rch_mask = rch_mask;
			}
		}
	}
}

static void dpu_kms_update_connector_hw_state(struct dpu_kms *dpu_kms,
		struct drm_crtc *crtc, struct drm_connector *connector,
		struct drm_crtc_state *new_state, bool enable)
{
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	const char *state_str;
	bool auto_refresh_en;
	bool is_cmd_mode;
	u32 tmg_mask = 0;
	u32 pipe_id;
	u32 blk_id;

	pipe_id = drm_crtc_index(crtc);
	virt_pipeline = &dpu_kms->virt_pipeline[pipe_id];
	hw_scene_ctl = virt_pipeline->hw_scene_ctl;
	state_str = enable ? "mount" : "unmount";
	blk_id = hw_scene_ctl->hw.blk_id;

	switch (connector->connector_type) {
	case DRM_MODE_CONNECTOR_DSI:
		if (is_cont_display_enabled() && is_primary_display(new_state))
			return;

		if ((need_enable_crtc(new_state) || need_restart_dpu_display_after_idle(crtc)) &&
				!need_enable_update(new_state)) {
			virt_pipeline->delay_mount_connector = true;
			DPU_INFO("need mount connector delay a frame\n");
			return;
		}

		is_cmd_mode = dsi_connector_is_cmd_mode(connector);
		auto_refresh_en = !is_cmd_mode;
		hw_scene_ctl->ops.timing_engine_mode_set(&hw_scene_ctl->hw,
				is_cmd_mode, auto_refresh_en);

		if (enable) {
			dsi_encoder_tmg_mask_get(connector->encoder, &tmg_mask);
			dsi_connector_ctrl_mode_init(connector);
			dsi_encoder_tmg_enable(to_dsi_encoder(connector->encoder));
		}

		hw_scene_ctl->ops.timing_engine_mount(&hw_scene_ctl->hw, tmg_mask);
		DPU_DEBUG("%s dsi tmg mask: 0x%x to scene ctrl %u\n",
				state_str, tmg_mask, blk_id);
		break;
	case DRM_MODE_CONNECTOR_DisplayPort:
		tmg_mask = enable ? TMG_DP0 : 0;
		hw_scene_ctl->ops.timing_engine_mode_set(&hw_scene_ctl->hw,
				false, false);
		hw_scene_ctl->ops.timing_engine_mount(&hw_scene_ctl->hw,
				tmg_mask);
		DPU_DEBUG("%s dp-0 to scene ctrl %u\n", state_str, blk_id);
		break;
	case DRM_MODE_CONNECTOR_WRITEBACK:
		break;
	default:
		DPU_ERROR("unsupported connector type %u\n",
				connector->connector_type);
		break;
	}
}

static void dpu_kms_update_connector_mount_state(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_virt_pipeline *virt_pipeline;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct drm_connector *connector;
	struct dpu_kms_connector_iter iter;
	u32 mask;
	u32 pipe_id;

	pipe_id = drm_crtc_index(crtc);
	virt_pipeline = &dpu_kms->virt_pipeline[pipe_id];
	hw_scene_ctl = virt_pipeline->hw_scene_ctl;
	hw_ctl_top = dpu_kms->virt_ctrl.hw_ctl_top;

	if (!hw_scene_ctl || !hw_ctl_top)
		return;

	if (!drm_atomic_crtc_needs_modeset(new_state) &&
			!virt_pipeline->delay_mount_connector &&
			!need_restart_dpu_display_after_idle(crtc)) {
		return;
	}

	if (virt_pipeline->delay_mount_connector ||
			(need_restart_dpu_display_after_idle(crtc))) {
		for_each_connector_per_crtc(connector, new_state, mask)
			dpu_kms_update_connector_hw_state(dpu_kms,
					crtc, connector, new_state, true);
	}

	for_each_state_changed_connector_per_crtc(connector, state,
			old_state, new_state, iter)
		dpu_kms_update_connector_hw_state(dpu_kms, crtc,
				connector, new_state, iter.enabling);
}

static void dpu_kms_update_underflow_irq_state(struct dpu_kms *dpu_kms,
		struct drm_crtc_state *new_state)
{
	struct drm_connector *connector;
	struct dpu_hw_intr *hw_intr;
	u32 mask;

	hw_intr = dpu_kms->virt_ctrl.intr_ctx->hw_intr;
	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI)
			hw_intr->ops.enable(&(hw_intr->hw),
					INTR_ONLINE0_FRM_TIMING_UNDERFLOW, true);
	}
}

static void _update_offline_irq_state(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct dpu_intr_context *intr_ctx,
		bool before_cfg_done)
{
	struct drm_connector_state *old_conn_state, *new_conn_state;
	struct drm_connector *connector;
	u32 intr_type;
	int i;
	int ret;

	ret = dpu_intr_type_get(crtc, &intr_type);
	if (ret || intr_type != DPU_INTR_OFFLINE0) {
		DPU_ERROR("invalid offline intr type %d\n", intr_type);
		return;
	}

	dpu_core_perf_clk_active();
	for_each_oldnew_connector_in_state(state, connector, old_conn_state, new_conn_state, i) {
		if (connector->connector_type != DRM_MODE_CONNECTOR_WRITEBACK)
			continue;

		if (before_cfg_done && new_conn_state->crtc) {
			dpu_wb_interrupt_register(connector, intr_type);
			dpu_exception_offline_dbg_irq_register(intr_ctx);
			dpu_interrupt_irq_enable(intr_ctx, connector,
					intr_type, true);
		}

		if (!before_cfg_done && new_conn_state->crtc) {
			dpu_interrupt_irq_enable(intr_ctx, connector,
					intr_type, false);
			dpu_exception_offline_dbg_irq_unregister(intr_ctx);
			dpu_wb_interrupt_unregister(connector, intr_type);
		}
	}
	dpu_core_perf_clk_deactive();
}

static void dpu_kms_update_irq_state(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		bool before_cfg_done)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dpu_kms_connector_iter iter;
	struct dpu_intr_context *intr_ctx;
	struct drm_connector *connector;
	u32 intr_type;
	int ret;

	if (!is_dpu_powered_on())
		return;

	intr_ctx = dpu_kms->virt_ctrl.intr_ctx;
	virt_pipe = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];
	if (virt_pipe->is_offline) {
		_update_offline_irq_state(state, crtc, intr_ctx, before_cfg_done);
		return;
	}

	if (!drm_atomic_crtc_needs_modeset(new_state)) {
		if (before_cfg_done)
			dpu_kms_update_underflow_irq_state(dpu_kms, new_state);
		return;
	}

	ret = dpu_intr_type_get(crtc, &intr_type);
	if (ret) {
		DPU_ERROR("failed to get interrupt type\n");
		return;
	}

	for_each_state_changed_connector_per_crtc(connector, state,
			old_state, new_state, iter) {

		if (!((before_cfg_done && iter.enabling) ||
				(!before_cfg_done && !iter.enabling)))
			continue;

		dpu_core_perf_clk_active();
		switch (connector->connector_type) {
		case DRM_MODE_CONNECTOR_DSI:
			if (intr_type > DPU_INTR_ONLINE1) {
				DPU_ERROR("invalid interrupt type(%d) for dsi\n",
						intr_type);
				break;
			}

			if (iter.enabling) {
				dsi_interrupt_register(connector, intr_type);
				dpu_interrupt_irq_enable(intr_ctx, connector,
						intr_type, true);
				dpu_exception_online_dbg_irq_register(intr_ctx);
			} else {
				dpu_interrupt_irq_enable(intr_ctx, connector,
						intr_type, false);
				dsi_interrupt_unregister(connector, intr_type);
				dpu_exception_online_dbg_irq_unregister(intr_ctx);
			}
			break;
		case DRM_MODE_CONNECTOR_DisplayPort:
			if (intr_type > DPU_INTR_ONLINE1) {
				DPU_ERROR("invalid interrupt type(%d) for dp\n",
						intr_type);
				break;
			}

			if (iter.enabling) {
				dp_interrupt_register(connector, intr_type);
				dpu_interrupt_irq_enable(intr_ctx, connector,
						intr_type, true);
				dpu_exception_cmb_dbg_irq_register(intr_ctx);
			} else {
				dpu_interrupt_irq_enable(intr_ctx, connector,
						intr_type, false);
				dp_interrupt_unregister(connector, intr_type);
				dpu_exception_cmb_dbg_irq_unregister(intr_ctx);
			}
			break;
		case DRM_MODE_CONNECTOR_WRITEBACK:
			drm_wb_conn = drm_connector_to_writeback(connector);
			dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);

			if (iter.enabling) {
				if (dpu_wb_conn->is_online) {
					dpu_wb_interrupt_register(connector, intr_type);
					dpu_interrupt_clone_connector_update(intr_ctx,
							intr_type, connector);
				}
			} else {
				if (dpu_wb_conn->is_online) {
					dpu_interrupt_clone_connector_update(
							intr_ctx, intr_type, NULL);
					dpu_wb_interrupt_unregister(connector, intr_type);
				}
			}
			break;
		default:
			DPU_ERROR("unsupported connector type %u\n",
					connector->connector_type);
			break;
		}
		dpu_core_perf_clk_deactive();
	}
}

static void dpu_kms_pipe_commit(struct list_head *cb_list,
		struct drm_atomic_state *state,
		struct drm_crtc_state *new_state)
{
	struct dpu_pipe_cb_info *cb_info;
	struct drm_connector *connector;
	struct drm_encoder *encoder;
	u32 iter_mask;

	if (!new_state->active)
		return;

	if (list_empty(cb_list))
		return;

	for_each_connector_per_crtc(connector, new_state, iter_mask) {
		list_for_each_entry(cb_info, cb_list, list) {
			encoder = connector->state->best_encoder;
			if (cb_info->data == connector || cb_info->data == encoder)
				cb_info->commit(state, cb_info->data);
		}
	}
}

static void dpu_kms_pre_commit(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state)
{
	struct drm_crtc_state *old_state;
	struct drm_crtc_state *new_state;
	struct drm_crtc *crtc;
	int i;

	if (!dpu_kms || !state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", dpu_kms, state);
		return;
	}

	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i)
		dpu_kms_pipe_commit(&dpu_kms->pre_commit_cb_list,
				state, new_state);
}

static int dpu_kms_wait_for_fences(struct drm_atomic_state *state)
{
	struct drm_plane_state *new_plane_state;
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_gem_object *gem_obj;
	struct drm_plane *plane;
	int ret;
	int i;

	trace_dpu_kms_wait_for_fences("wait fence start");
	for_each_new_plane_in_state(state, plane, new_plane_state, i) {
		dpu_plane_state = to_dpu_plane_state(new_plane_state);

		if (new_plane_state->fb) {
			gem_obj = to_dpu_gem_obj(new_plane_state->fb->obj[0]);
			if (gem_obj && gem_obj->is_dumb_buf && gem_obj->sgt)
				dma_sync_sg_for_device(state->dev->dev, gem_obj->sgt->sgl,
						gem_obj->sgt->nents, DMA_TO_DEVICE);
		}

		if (!dpu_plane_state->fence)
			continue;

		WARN_ON(!new_plane_state->fb);

		ret = dpu_acquire_fence_wait(dpu_plane_state->fence, DPU_ACQUIRE_FENCE_TIMEOUT_MS);
		if (ret < 0) {
			dpu_kms_fence_force_signal(new_plane_state->crtc, true);
		}
	}

	return 0;
}

static void dpu_kms_process_buf_lock(struct dpu_commit_ctx *commit_ctx,
		struct dpu_timeline *timeline, struct dpu_crtc_state *dpu_crtc_state, u64 sync_pt)
{
	struct dpu_frame_lock_info *frame_lock_info = NULL;
	struct dpu_gem_object *dpu_gem_obj;
	const struct drm_plane_state *pstate;
	struct dpu_plane_state *dpu_plane_state;
	struct dpu_buf_sync *node = NULL;
	struct dpu_buf_sync *_node_ = NULL;
	struct drm_plane *plane = NULL;
	struct list_head lock_list;
	int i;

	INIT_LIST_HEAD(&lock_list);
	for_each_new_plane_in_state(commit_ctx->state, plane, pstate, i) {
		if (!pstate || !pstate->fb)
			continue;

		node = kzalloc(sizeof(*node), GFP_KERNEL);
		if (!node) {
			DPU_WARN("no mem, alloc failed\n");
			continue;
		}
		dpu_gem_obj = to_dpu_gem_obj(pstate->fb->obj[0]);
		dpu_plane_state = to_dpu_plane_state(pstate);

		node->frame_no = dpu_crtc_state->frame_no;
		node->rch_id = dpu_plane_state->rch_id;
		if (dpu_plane_state->sharefd > 0) {
			node->dmabuf = dma_buf_get(dpu_plane_state->sharefd);
			if (node->dmabuf != dpu_gem_obj->dmabuf)
				DPU_WARN("commit plane not a same buffer!!!\n");

			IOMMU_DEBUG("lock fd:%d, dmabuf:0x%llx, frame_no:%llu, rch_id:%u\n",
					node->sharefd, (u64)(uintptr_t)node->dmabuf,
					node->frame_no, node->rch_id);
		}
		list_add_tail(&node->list_node, &lock_list);
	}

	frame_lock_info = dpu_buf_sync_create(timeline,
			&lock_list, sync_pt, dpu_crtc_state->frame_no);
	if (!frame_lock_info) {
		DPU_WARN("create frame lock info failed\n");
		goto error_lock_fail;
	}
	//  TODO: offline flow need refine
	frame_lock_info->cmdlist_frame_id = 0;

	drm_atomic_state_get(commit_ctx->state);
	IOMMU_DEBUG("lock state: %p ", commit_ctx->state);
	frame_lock_info->state = commit_ctx->state;

error_lock_fail:
	list_for_each_entry_safe(node, _node_, &lock_list, list_node) {
		list_del(&node->list_node);
		kfree(node);
	}
}

static int dpu_kms_process_fences(struct drm_crtc_state *crtc_state,
		struct dpu_commit_ctx *commit_ctx)
{
	struct dpu_crtc_state *dpu_crtc_state;
	int present_fence_fd = -1;
	int release_fence_fd = -1;
	u64 release_sync_pt;
	u64 sync_pt;
	int ret;

	dpu_crtc_state = to_dpu_crtc_state(crtc_state);
	if (!dpu_crtc_state)
		return -1;

	if (dpu_crtc_state->present_fence_ptr) {
		present_fence_fd = dpu_release_fence_create(&commit_ctx->timeline, &sync_pt, dpu_crtc_state->frame_no);
		if (present_fence_fd < 0) {
			DPU_WARN("create present fence fd failed\n");
			ret = -EFAULT;
			goto error_fence_handle;
		}

		ret = COPY_FD_TO_USER(dpu_crtc_state->present_fence_ptr, &present_fence_fd);
		if (ret) {
			DPU_WARN("copy to user failed\n");
			goto error_fence_handle;
		}

		dpu_kms_process_buf_lock(commit_ctx, &commit_ctx->timeline, dpu_crtc_state, sync_pt + 1);
	}

	if (dpu_crtc_state->release_fence_ptr) {
		release_fence_fd = dpu_release_fence_create(&commit_ctx->release_timeline, &release_sync_pt, dpu_crtc_state->frame_no);
		if (release_fence_fd < 0) {
			DPU_WARN("create release fence fd failed\n");
			ret = -EFAULT;
			goto error_fence_handle;
		}

		ret = COPY_FD_TO_USER(dpu_crtc_state->release_fence_ptr, &release_fence_fd);
		if (ret) {
			DPU_WARN("copy to user failed\n");
			goto error_fence_handle;
		}
	}

	return 0;

error_fence_handle:
	if (present_fence_fd >= 0) {
		dpu_acquire_fence_signal(present_fence_fd);
		put_unused_fd(present_fence_fd);
	}

	if (release_fence_fd >= 0) {
		dpu_acquire_fence_signal(release_fence_fd);
		put_unused_fd(release_fence_fd);
	}

	return ret;
}

static bool need_cfg_rdy_update(struct dpu_virt_pipeline *virt_pipe,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct drm_connector *connector;
	u32 mask;

	if (virt_pipe->is_offline && new_state->connector_mask == 0) {
		DPU_DEBUG("offline wb display off\n");
		return false;
	}

	for_each_connector_per_crtc(connector, new_state, mask)
		if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK)
			return dpu_wb_need_cfg_rdy_update(connector, virt_pipe->is_offline);

	/* power on but unmount tmg, not irq when power off */
	if (need_enable_crtc(old_state) && !need_enable_update(old_state) &&
			need_disable_crtc(new_state))
		return false;

	return true;
}

static bool need_init_en(struct dpu_virt_pipeline *virt_pipe, struct drm_crtc_state *new_state)
{
	if (virt_pipe->updt_en)
		return false;

	if (need_enable_crtc(new_state)) {
		DPU_INFO("need_enable_crtc, return true\n");
		return true;
	}

	if (need_restart_dpu_display_after_idle(new_state->crtc)) {
		DPU_INFO("just exit idle, init_en=%d\n", need_disable_crtc(new_state) ? false : true);
		return need_disable_crtc(new_state) ? false : true;
	}

	return false;
}

static void _disable_dsi_tmg_first(struct drm_crtc_state *new_state)
{
	struct drm_connector *connector;
	u32 mask;

	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI)
			dsi_encoder_tmg_disable(to_dsi_encoder(connector->encoder));
	}
}

static void dpu_kms_cfg_ready_update(struct dpu_virt_pipeline *virt_pipe,
		struct drm_crtc_state *old_state, struct drm_crtc_state *new_state)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	bool init_en;

	if (!need_cfg_rdy_update(virt_pipe, old_state, new_state))
		return;

	hw_scene_ctl = virt_pipe->hw_scene_ctl;

	atomic_set(&virt_pipe->cfg_done_flag, 0);
	atomic_set(&virt_pipe->vsync_flag, 0);
	virt_pipe->updt_en = need_enable_update(new_state);
	init_en = need_init_en(virt_pipe, new_state);
	DPU_DEBUG("updt en:%d init_en:%d\n", virt_pipe->updt_en, init_en);

	if (!virt_pipe->updt_en)
		DPU_INFO("pipe %d update_en is false\n", virt_pipe->index);

	if (init_en) {
		DPU_INFO("pipe %d init en is true, disable dsi tmg\n", virt_pipe->index);
		_disable_dsi_tmg_first(new_state);
	}

	hw_scene_ctl->ops.init_en(&hw_scene_ctl->hw, init_en);
	hw_scene_ctl->ops.cfg_ready_update(&hw_scene_ctl->hw,
			virt_pipe->updt_en, dpu_get_tui_state());
	virt_pipe->cfg_timestamp = ktime_get();
}

static void dpu_kms_post_commit(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_virt_pipeline *virt_pipe;
	struct drm_crtc_state *old_state;
	struct drm_crtc_state *new_state;
	struct dpu_crtc *dpu_crtc;
	struct drm_crtc *crtc;
	int pipe_id;
	int i;

	if (!dpu_kms || !state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", dpu_kms, state);
		return;
	}

	if (!is_dpu_powered_on())
		return;

	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i) {
		pipe_id = drm_crtc_index(crtc);
		dpu_crtc = to_dpu_crtc(crtc);
		virt_pipe = &dpu_kms->virt_pipeline[pipe_id];
		hw_scene_ctl = virt_pipe->hw_scene_ctl;

		dpu_kms_pipe_commit(&dpu_kms->post_commit_cb_list,
				state, new_state);

		if (virt_pipe->is_offline)
			dpu_wfd_notify_event(XPLAYER_EVENT_FRAME_BEGIN);

		dpu_kms_update_plane_mount_state(dpu_kms, crtc, new_state);

		dpu_exception_dbg_irq_enable(dpu_kms, crtc);

		dpu_kms_update_connector_mount_state(dpu_kms, state,
				crtc, old_state, new_state);

		dpu_cont_display_res_release(dpu_kms->power_mgr);

		/* enable && register irq before hw cfg done */
		dpu_kms_update_irq_state(dpu_kms, state, crtc, old_state, new_state, true);

		dpu_virt_pipe_mode_info_update(virt_pipe, new_state, old_state);

		dsi_connector_mipi_cmd_state_get(crtc, new_state);
		if (hw_scene_ctl) {
			dpu_cmdlist_commit(state, crtc, old_state, new_state);
			dpu_kms_cfg_ready_update(virt_pipe, old_state, new_state);
		}

		dpu_kms_start_first_frame(dpu_kms, crtc, old_state, new_state, pipe_id);
	}
}

static void dpu_idle_finish(struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state, struct drm_crtc *crtc)
{
	struct drm_connector *connector;
	u32 mask;

	dpu_idle_helper_postprocess(new_state->crtc);

	dpu_idle_helper_restart(crtc);

	/* Attention:
	 * Shall unregister primary crtc when primary panel off
	 * because primary crtc will be changed in next power on.
	 */
	if (need_disable_crtc(new_state)) {
		for_each_connector_per_crtc(connector, old_state, mask) {
			if (connector->connector_type == DRM_MODE_CONNECTOR_DSI)
				dpu_idle_unregister_primary_crtc();
		}
	}
}

static void dpu_kms_finish(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state)
{
	struct drm_crtc_state *old_state, *new_state;
	struct dpu_virt_pipeline *virt_pipeline;
	struct drm_crtc *crtc;
	int pipe;
	int i;

	if (!dpu_kms || !state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", dpu_kms, state);
		return;
	}

	/* add function: must do check is_dpu_powered_on, return if false */
	for_each_oldnew_crtc_in_state(state, crtc, old_state, new_state, i) {
		pipe = drm_crtc_index(crtc);
		virt_pipeline = &dpu_kms->virt_pipeline[pipe];

		dsi_connector_finish(crtc, new_state);

		/* disable && unregister irq after hw cfg done */
		dpu_kms_update_irq_state(dpu_kms, state, crtc, old_state, new_state, false);

		dpu_core_perf_finish(dpu_kms->core_perf, crtc, old_state, new_state);

		dpu_idle_finish(old_state, new_state, crtc);

		dpu_power_finish(dpu_kms, state, crtc, old_state, new_state);

		dsi_display_dyn_freq_task_wakeup(old_state);

		/* clear sw start flag */
		virt_pipeline->sw_start_flag = false;
	}

	dpu_res_helper_resource_release(dpu_kms, state, true);
}

static void dpu_kms_force_signal_internal(struct dpu_timeline *timeline,
		struct dpu_virt_pipeline *pipeline, bool need_dump)
{
	u64 time_stamp;

	if (need_dump) {
		dpu_timeline_dump(timeline);
		timeline_dump_flag_set(timeline, true);
	}

	mutex_lock(&timeline->list_lock);
	if (list_count_nodes(&timeline->listener_list) > 0) {
		DPU_DEBUG("timeline and fence force signal!\n");

		time_stamp = ktime_get_mono_fast_ns();
		dpu_timeline_resync_pt(timeline,
				timeline_get_next_value(timeline));

		timeline_force_signal_set(timeline, 1);
		dpu_isr_notify_listener(&pipeline->commit_ctx.isr,
				timeline->listening_id,
				time_stamp);
		timeline_force_signal_set(timeline, 0);

		trace_dpu_kms_fence_force_signal("fence_force_signal");
	}
	mutex_unlock(&timeline->list_lock);
}

void dpu_kms_fence_force_signal(struct drm_crtc *crtc, bool need_dump)
{
	struct dpu_virt_pipeline *pipeline;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;

	if (!crtc) {
		DPU_ERROR("crtc is null\n");
		return;
	}
	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	pipeline = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];

	atomic_set(&pipeline->commit_ctx.release_timeline.pending_frame_num, 0);
	dpu_kms_force_signal_internal(&pipeline->commit_ctx.timeline, pipeline, need_dump);
	dpu_kms_force_signal_internal(&pipeline->commit_ctx.release_timeline, pipeline, need_dump);
}

static bool need_do_dpu_exception_recovery(struct dpu_virt_pipeline *pipeline)
{
	return atomic_read(&pipeline->hw_hang) ? true : false;
}

static bool check_hw_cfg_state(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_crtc_state,
		struct drm_crtc_state *new_crtc_state)
{
	if (atomic_read(&pipeline->cfg_done_flag) == 1)
		return true;

	if (!old_crtc_state->active && !new_crtc_state->active)
		return true;

	if (!need_cfg_rdy_update(pipeline, old_crtc_state, new_crtc_state))
		return true;

	if (need_restart_dpu_display_after_idle(crtc) &&
			need_disable_crtc(new_crtc_state))
		return true;

	if (need_do_dpu_exception_recovery(pipeline))
		return true;

	return false;
}

static void dpu_unmount_offline_sense_ctrl(struct drm_crtc_state *new_state)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct drm_connector *connector;
	u32 mask;

	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
			DPU_DEBUG("unmount wb\n");
			drm_wb_conn = drm_connector_to_writeback(connector);
			dpu_wb_connector_job_clear(new_state->crtc, drm_wb_conn);
		}
	}
}

static void dpu_update_obufen_status(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state)
{
	struct drm_connector *connector;
	struct drm_crtc_state *state;
	u32 mask;
	int i;

	/* if current frame is to close display, use old state to get connector */
	state = crtc->state->active ? crtc->state : old_state;

	for_each_connector_per_crtc(connector, state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			display_for_each_active_port(i, to_dsi_connector(connector)->display) {
				dpu_obuf_update_obufen_done(i);
			}
		} else if (connector->connector_type == DRM_MODE_CONNECTOR_DisplayPort) {
			dpu_obuf_update_obufen_done(DPU_OBUF_CLIENT_DP);
		}
	}
}

static void dpu_hw_cfg_done_timeout_get(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc *crtc, struct drm_crtc_state *new_crtc_state)
{
	struct dsi_connector *dsi_connector;
	struct drm_connector *connector;
	struct dsi_display *display;
	struct dpu_kms *dpu_kms;
	u32 mask;

	dpu_kms = pipeline->dpu_kms;
	if (unlikely(!dpu_kms->is_asic)) {
		pipeline->frame_duration_ms = dpu_kms->default_frame_timeout_ms;
		return;
	}

	if (!new_crtc_state->mode_changed)
		return;

	for_each_connector_per_crtc(connector, crtc->state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(connector);
			display = dsi_connector->display;
			display_frame_timeout_get(display, &pipeline->frame_duration_ms);
			break;
		}
	}

	if (pipeline->frame_duration_ms < dpu_kms->default_frame_timeout_ms)
		pipeline->frame_duration_ms = dpu_kms->default_frame_timeout_ms;
}

static void dpu_update_cfg_timeout_count(struct drm_crtc_state *new_crtc_state, struct drm_crtc *crtc, bool is_timeout)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *primary_conn;
	struct dsi_display *display;

	if (!is_primary_display(new_crtc_state))
		return;

	primary_conn = dsi_primary_connector_get(crtc->dev);
	dsi_connector = to_dsi_connector(primary_conn);
	display = dsi_connector->display;

	if (is_timeout) {
		dsi_connector->sw_clr_fail_cnt++;
		dsi_connector->continous_succ_cnt = 0;
		if (dsi_connector->sw_clr_fail_cnt >= MAX_SW_CLR_FAIL_THRESHOLD) {
			dpu_crtc_state = to_dpu_crtc_state(crtc->state);
			DPU_ERROR("%d continous frame is abnormal, current frame_no:%llu",
				MAX_SW_CLR_FAIL_THRESHOLD, dpu_crtc_state->frame_no);
			dsi_display_panel_dead_notify(display);
			dsi_connector->sw_clr_fail_cnt = 0;
		}
	} else {
		dsi_connector->continous_succ_cnt++;
		if (dsi_connector->continous_succ_cnt > SUCC_PRESENT_THRESHOLD)
			dsi_connector->sw_clr_fail_cnt = 0;
	}
}

void dpu_kms_exception_recovery(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *new_crtc_state,
		struct drm_crtc *crtc)
{
	if (!pipeline || !new_crtc_state || !crtc) {
		DPU_ERROR("invalid parameter %pK, %pK, %pK\n",
				pipeline, new_crtc_state, crtc);
		return;
	}

	if (!atomic_read(&pipeline->hw_hang))
		return;

	dpu_exception_recovery(pipeline, new_crtc_state);
	dpu_kms_fence_force_signal(crtc, true);

	atomic_set(&pipeline->cfg_done_flag, 1);
	atomic_set(&pipeline->hw_hang, 0);
	atomic_set(&pipeline->underflow_flag, 0);
	/* There's no need to wait after hardware clear for cmd mode */
	atomic_set(&pipeline->vstart_flag, 1);

	dpu_virt_pipe_state_clear(pipeline);
	dpu_update_cfg_timeout_count(new_crtc_state, crtc, true);
}

static void dpu_kms_wait_for_hw_cfg_done(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *old_state)
{
	struct drm_crtc_state *old_crtc_state, *new_crtc_state;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_virt_pipeline *pipeline;
	struct drm_crtc *crtc;
	int i, ret;
	u32 pipe_id;

	if (!dpu_kms || !old_state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", dpu_kms, old_state);
		return;
	}

	for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i) {
		pipe_id = drm_crtc_index(crtc);
		pipeline = &dpu_kms->virt_pipeline[pipe_id];
		dpu_crtc_state = to_dpu_crtc_state(crtc->state);

		dpu_hw_cfg_done_timeout_get(pipeline, crtc, new_crtc_state);
		ret = wait_event_timeout(pipeline->cfg_done_wq,
				check_hw_cfg_state(pipeline, crtc, old_crtc_state, new_crtc_state),
				msecs_to_jiffies(pipeline->frame_duration_ms));
		if (!ret || need_do_dpu_exception_recovery(pipeline)) {
			DPU_ERROR("[CRTC:%d:%s (%s)(%s)(%s)][curr_frame_no:%llu old_frame_no:%lld] wait hw cfg done timeout!\n",
					crtc->base.id,
					crtc->name,
					pipeline->is_offline ? "offline" : "online",
					need_disable_crtc(new_crtc_state) ?
					"disp off" : "normal flip",
					(atomic_read(&pipeline->underflow_flag) == 1) ?
					"underflow" : "no intr",
					dpu_crtc_state->frame_no,
					to_dpu_crtc_state(old_crtc_state)->frame_no);
			atomic_set(&pipeline->hw_hang, 1);
			dpu_kms_exception_recovery(pipeline, new_crtc_state, crtc);
		} else {
			dpu_virt_pipe_state_swap(pipeline);

			dpu_unmount_offline_sense_ctrl(new_crtc_state);

			dpu_update_obufen_status(crtc, old_crtc_state);
			DPU_DEBUG("[CRTC:%d:%s (%s)(%s)][frame_no:%llu] dpu start!\n",
					crtc->base.id, crtc->name,
					pipeline->is_offline ? "offline" : "online",
					need_disable_crtc(new_crtc_state) ? "disp off" : "normal flip",
					dpu_crtc_state->frame_no);
			dpu_update_cfg_timeout_count(new_crtc_state, crtc, false);
		}

		if (!pipeline->is_offline && need_disable_crtc(new_crtc_state)) {
			dpu_kms_fence_force_signal(crtc, false);
			atomic_set(&pipeline->vstart_flag, 1);
			pipeline->delay_mount_connector = false;
		}

		drm_crtc_handle_vblank(crtc);
	}
}

static void dpu_kms_handle_intr_events(struct dpu_kms *dpu_kms,
		struct drm_crtc *crtc, u32 intr_id)
{
	if (!dpu_kms || !crtc)
		return;

	dpu_crtc_handle_events(crtc, intr_id);
}

static int dpu_kms_event_enable(struct dpu_kms *dpu_kms,
		struct drm_mode_object *obj, u32 event_type, bool enable)
{
	struct drm_crtc *crtc;
	int ret = 0;

	if (!dpu_kms || !obj) {
		DPU_ERROR("failed to enable event, invalid dpu_kms %pK, obj %pK\n",
				dpu_kms, obj);
		return -EINVAL;
	}

	switch (obj->type) {
	case DRM_MODE_OBJECT_CRTC:
		crtc = obj_to_crtc(obj);
		ret = dpu_crtc_event_enable(crtc, event_type, enable);
		break;
	case DRM_MODE_OBJECT_CONNECTOR:
	default:
		break;
	}

	return ret;
}

static struct dpu_kms_funcs dpu_kms_funcs = {
	.init = dpu_kms_res_init,
	.deinit = dpu_kms_res_deinit,
	.check = dpu_kms_check,
	.prepare = dpu_kms_prepare,
	.pre_commit = dpu_kms_pre_commit,
	.post_commit = dpu_kms_post_commit,
	.finish = dpu_kms_finish,
	.wait_for_hw_cfg_done = dpu_kms_wait_for_hw_cfg_done,
	.handle_intr_events = dpu_kms_handle_intr_events,
	.enable_event = dpu_kms_event_enable,
	.process_fences = dpu_kms_process_fences,
};

static int dpu_kms_commit_cb_register(struct list_head *cb_list,
		void (*commit)(struct drm_atomic_state *state, void *data),
		void *data)
{
	struct dpu_pipe_cb_info *cb_info;

	cb_info = kzalloc(sizeof(*cb_info), GFP_KERNEL);
	if (!cb_info)
		return -ENOMEM;

	cb_info->data = data;
	cb_info->commit = commit;

	list_add_tail(&cb_info->list, cb_list);

	return 0;
}

int dpu_kms_pre_commit_cb_register(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *state, void *data),
		void *data)
{
	struct dpu_kms *dpu_kms;

	if (!drm_dev || !commit) {
		DPU_ERROR("invalid parameter %pK, %pK\n", drm_dev, commit);
		return -EINVAL;
	}

	dpu_kms = to_dpu_kms(drm_dev);

	return dpu_kms_commit_cb_register(&dpu_kms->pre_commit_cb_list,
			commit, data);
}

int dpu_kms_post_commit_cb_register(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *state, void *data),
		void *data)
{
	struct dpu_kms *dpu_kms;

	if (!drm_dev || !commit) {
		DPU_ERROR("invalid parameter %pK, %pK\n", drm_dev, commit);
		return -EINVAL;
	}

	dpu_kms = to_dpu_kms(drm_dev);

	return dpu_kms_commit_cb_register(&dpu_kms->post_commit_cb_list,
			commit, data);
}

static void dpu_kms_commit_cb_unregister(struct list_head *cb_list,
		void (*commit)(struct drm_atomic_state *state, void *data))
{
	struct dpu_pipe_cb_info *cb_info, *temp_info;

	list_for_each_entry_safe(cb_info, temp_info, cb_list, list) {
		if (cb_info->commit == commit) {
			list_del(&cb_info->list);
			kfree(cb_info);
			break;
		}
	}
}

void dpu_kms_pre_commit_cb_unregister(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *state, void *data))
{
	struct dpu_kms *dpu_kms;

	if (!drm_dev || !commit) {
		DPU_ERROR("invalid parameter %pK, %pK\n", drm_dev, commit);
		return;
	}

	dpu_kms = to_dpu_kms(drm_dev);

	return dpu_kms_commit_cb_unregister(&dpu_kms->pre_commit_cb_list, commit);
}

void dpu_kms_post_commit_cb_unregister(struct drm_device *drm_dev,
		void (*commit)(struct drm_atomic_state *state, void *data))
{
	struct dpu_kms *dpu_kms;

	if (!drm_dev || !commit) {
		DPU_ERROR("invalid parameter %pK, %pK\n", drm_dev, commit);
		return;
	}

	dpu_kms = to_dpu_kms(drm_dev);

	return dpu_kms_commit_cb_unregister(&dpu_kms->post_commit_cb_list, commit);
}

static void dpu_kms_commit_work(struct kthread_work *work)
{
	const struct drm_mode_config_helper_funcs *funcs;
	struct drm_atomic_state *old_state;
	struct dpu_commit_ctx *commit_ctx;
	struct drm_device *dev;

	commit_ctx = container_of(work, struct dpu_commit_ctx, commit_work);
	old_state = commit_ctx->state;
	dev = old_state->dev;

	drm_atomic_helper_wait_for_dependencies(old_state);

	mutex_lock(&commit_ctx->commit_lock);

	trace_dpu_kms_commit_work("commit work start", commit_ctx->ctx_frame_no);
	DPU_DEBUG("----- commit work: %llu start -----", commit_ctx->ctx_frame_no);

	dpu_kms_wait_for_fences(old_state);

	dpu_acquire_fences_debug(commit_ctx, old_state);

	drm_atomic_helper_prepare_planes(dev, old_state);

	funcs = dev->mode_config.helper_private;
	if (funcs && funcs->atomic_commit_tail)
		funcs->atomic_commit_tail(old_state);
	else
		drm_atomic_helper_commit_tail(old_state);

	drm_atomic_helper_commit_cleanup_done(old_state);

	DPU_DEBUG("----- commit work: %llu end -----", commit_ctx->ctx_frame_no);

	mutex_unlock(&commit_ctx->commit_lock);
}

static int dpu_kms_commit_thread_init(struct dpu_kms *dpu_kms)
{
	struct dpu_commit_ctx *commit_ctx;
	struct dpu_virt_pipeline *pipeline;
	struct sched_param sp;
	char tmp_name[256] = {0};
	int ret = 0;
	int i;

	for (i = 0; i < MAX_PIPELINE_COUNT; i++) {
		pipeline = &dpu_kms->virt_pipeline[i];
		commit_ctx = &pipeline->commit_ctx;

		kthread_init_worker(&commit_ctx->commit_worker);
		kthread_init_work(&commit_ctx->commit_work,
				&dpu_kms_commit_work);

		(void)snprintf(tmp_name, sizeof(tmp_name), "disp_commit_%d", i);
		commit_ctx->commit_task = kthread_run(kthread_worker_fn,
				&commit_ctx->commit_worker, tmp_name);
		if (IS_ERR(commit_ctx->commit_task)) {
			DPU_ERROR("failed to run display_commit_%d thread\n", i);
			ret = PTR_ERR(commit_ctx->commit_task);
			goto err_task;
		}
		sp.sched_priority = MAX_RT_PRIO - 1;
		WARN_ON_ONCE(sched_setscheduler_nocheck(commit_ctx->commit_task,
				SCHED_FIFO, &sp) != 0);

		dpu_isr_setup(&commit_ctx->isr);
		mutex_init(&commit_ctx->commit_lock);

		(void)snprintf(tmp_name, sizeof(tmp_name), "present_timeline_%d", i);
		dpu_timeline_init(&commit_ctx->timeline,
				tmp_name, pipeline, INTS_FRM_TIMING_VSYNC, &commit_ctx->isr);

		(void)snprintf(tmp_name, sizeof(tmp_name), "release_timeline_%d", i);
		dpu_timeline_init(&commit_ctx->release_timeline,
				tmp_name, pipeline, INTS_FRM_TIMING_VSYNC, &commit_ctx->isr);
	}

	return 0;

err_task:
	for (i -= 1; i >= 0; i--) {
		commit_ctx = &dpu_kms->virt_pipeline[i].commit_ctx;
		kthread_stop(commit_ctx->commit_task);
		mutex_destroy(&commit_ctx->commit_lock);
		dpu_timeline_deinit(&commit_ctx->timeline, &commit_ctx->isr);
		dpu_timeline_deinit(&commit_ctx->release_timeline, &commit_ctx->isr);
	}

	return ret;
}

int dpu_kms_obj_init(struct drm_device *drm_dev)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_kms *dpu_kms;
	int ret;
	int i;

	if (!drm_dev) {
		DPU_ERROR("invalid parameter %pK\n", drm_dev);
		return -EINVAL;
	}

	dpu_kms = kzalloc(sizeof(*dpu_kms), GFP_KERNEL);
	if (!dpu_kms)
		return -ENOMEM;

	dpu_kms->drm_dev = drm_dev;
	dpu_kms->funcs = &dpu_kms_funcs;
	INIT_LIST_HEAD(&dpu_kms->pre_commit_cb_list);
	INIT_LIST_HEAD(&dpu_kms->post_commit_cb_list);

	for (i = 0; i < MAX_PIPELINE_COUNT; i++) {
		pipeline = &dpu_kms->virt_pipeline[i];
		pipeline->dpu_kms = dpu_kms;
		pipeline->index = i;
		atomic_set(&pipeline->underflow_flag, 0);
		atomic_set(&pipeline->vsync_flag, 0);
		pipeline->delay_mount_connector = false;
		memset(&pipeline->power_state, 0, sizeof(pipeline->power_state));

		dpu_power_ctrl_init(&pipeline->power_ctrl);
		/* There is no need to wait for first frame commit */
		atomic_set(&pipeline->vstart_flag, 1);
		init_waitqueue_head(&pipeline->vstart_wq);

		atomic_set(&pipeline->cfg_done_flag, 0);
		init_waitqueue_head(&pipeline->cfg_done_wq);

		pipeline->pipe_wq = alloc_workqueue("pipe_wq", WQ_HIGHPRI, 1);
		if (!pipeline->pipe_wq) {
			DPU_ERROR("failed to create pipe wq %d\n", i);
			ret = -ENOMEM;
			goto error;
		}
	}

	ret = dpu_kms_commit_thread_init(dpu_kms);
	if (ret) {
		kfree(dpu_kms);
		return ret;
	}

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_drm_dev->dpu_kms = dpu_kms;

	return 0;

error:
	for (--i; i >= 0 ; i--) {
		pipeline = &dpu_kms->virt_pipeline[i];
		dpu_power_ctrl_deinit(&pipeline->power_ctrl);
		destroy_workqueue(pipeline->pipe_wq);
	}
	return ret;
}

static inline void dpu_kms_cb_list_destroy(struct list_head *cb_list)
{
	struct dpu_pipe_cb_info *cb_info, *temp_info;

	list_for_each_entry_safe(cb_info, temp_info, cb_list, list) {
		list_del(&cb_info->list);
		kfree(cb_info);
	}
}

void dpu_kms_obj_deinit(struct drm_device *drm_dev)
{
	struct dpu_virt_pipeline *pipeline;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_commit_ctx *commit_ctx;
	struct dpu_kms *dpu_kms;
	int i;

	if (!drm_dev) {
		DPU_ERROR("invalid parameter %pK\n", drm_dev);
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;

	for (i = 0; i < MAX_PIPELINE_COUNT; i++) {
		pipeline = &dpu_kms->virt_pipeline[i];
		destroy_workqueue(pipeline->pipe_wq);

		dpu_power_ctrl_deinit(&pipeline->power_ctrl);
		commit_ctx = &pipeline->commit_ctx;

		if (commit_ctx->state) {
			drm_atomic_state_put(commit_ctx->state);
			commit_ctx->state = NULL;
		}

		kthread_stop(commit_ctx->commit_task);
		mutex_destroy(&commit_ctx->commit_lock);

		dpu_timeline_deinit(&commit_ctx->timeline, &commit_ctx->isr);
	}
	dpu_kms_cb_list_destroy(&dpu_kms->pre_commit_cb_list);
	dpu_kms_cb_list_destroy(&dpu_kms->post_commit_cb_list);

	kfree(dpu_kms);
	dpu_drm_dev->dpu_kms = NULL;
}
