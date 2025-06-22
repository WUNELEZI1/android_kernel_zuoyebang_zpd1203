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

#include <drm/drm_atomic.h>
#include <drm/drm_plane.h>
#include <drm/drm_crtc.h>
#include <linux/ktime.h>
#include <linux/pm_wakeup.h>

#include "dpu_power_helper.h"
#include "dpu_cont_display.h"
#include "dpu_plane.h"
#include "dpu_log.h"
#include "dpu_crtc.h"
#include "dsi_power.h"
#include "dpu_trace.h"
#include "dpu_idle_helper.h"
#include "dsi_display.h"
#include "dp_drm.h"
#include "dp_display.h"

/**
 * DPU partition1 include rch0 - rch5.
 * DPU partition2 include rch5 - rch9.
 */
#define PARTITION_1_RCH_MASK	(0x3f)
#define PARTITION_2_RCH_MASK	(0x3c0)

/**
 * if power up completion time exceeds power up intr time by this value
 * the frame may be meet hw cfg done timeout and failed to be displayed
 */
#define DPU_POWER_WORK_SAFE_DELAY_TIME (5)

struct dpu_power_intr_handler_state {
	u32 desired_power_mask;
	u32 actual_power_mask;
	u32 handle_mask;
	ktime_t enter_work_time;
	ktime_t get_lock_time;
	ktime_t handle_done_time;
};

static void dpu_power_prepare_for_connector(struct dpu_power_mgr *power_mgr,
		struct dpu_power_state *power_state,
		struct drm_crtc_state *new_state)
{
	struct drm_connector *connector;
	u32 mask;

	if (!power_state->dsi_need_power_on && !power_state->dsi_panel_need_power_on)
		return;

	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			if (power_state->dsi_panel_need_power_on)
				dsi_panel_power_get(connector);

			if (power_state->dsi_need_power_on)
				dsi_power_get(connector);
		}
	}
}

static void dpu_power_up_work_print_state(struct dpu_power_ctrl *power_ctrl,
		struct dpu_power_intr_handler_state *state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_intr_context *ctx;
	ktime_t hw_power_up_time;
	ktime_t delta;

	if (!power_ctrl->crtc)
		return;

	ctx = to_dpu_drm_dev(power_ctrl->crtc->dev)->dpu_kms->virt_ctrl.intr_ctx;
	hw_power_up_time = ctx->intr_status.curr_timestamp[INTR_ONLINE0_TMG_OTHER_PART_POWER_ON];
	dpu_crtc_state = to_dpu_crtc_state(power_ctrl->crtc->state);

	delta = ktime_sub(state->handle_done_time, hw_power_up_time);
	if (ktime_to_ms(delta) > DPU_POWER_WORK_SAFE_DELAY_TIME) {
		DPU_DEBUG("farme_no %llu: hw_time %lld handle_done %lld, %4lld(ms)\n",
			dpu_crtc_state->frame_no,
			ktime_to_us(hw_power_up_time),
			ktime_to_us(state->handle_done_time),
			ktime_to_ms(delta));

		atomic_set(&power_ctrl->power_up_timeout, 1);
	}

	POWER_DEBUG("frame no %llu: desired 0x%x, actual 0x%x, restore 0x%x\n",
			dpu_crtc_state->frame_no,
			state->desired_power_mask,
			state->actual_power_mask,
			state->handle_mask);
	POWER_DEBUG("delta to hw: enter %lld(us), get_lock %lld(us), handle_done %lld(us)\n",
			ktime_to_us(ktime_sub(state->enter_work_time, hw_power_up_time)),
			ktime_to_us(ktime_sub(state->get_lock_time, hw_power_up_time)),
			ktime_to_us(ktime_sub(state->handle_done_time, hw_power_up_time)));
}

static void dpu_power_up_work_handler(struct kthread_work *work)
{
	struct dpu_power_intr_handler_state state = {0};
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_power_client *client;
	u32 need_restore_mask = 0;

	power_ctrl = container_of(work, struct dpu_power_ctrl, power_up_work);
	if (!power_ctrl) {
		POWER_ERROR("power ctrl is null\n");
		return;
	}

	client = power_ctrl->client;

	state.enter_work_time = ktime_get();
	mutex_lock(&power_ctrl->lock);
	state.get_lock_time = ktime_get();
	if (power_ctrl->frame_ctrl_enabled) {
		need_restore_mask =
				power_ctrl->desired_power_mask & ~power_ctrl->actual_power_mask;
		if (need_restore_mask)
			client->ops->enable(client, need_restore_mask);
		state.desired_power_mask = power_ctrl->desired_power_mask;
		state.actual_power_mask = power_ctrl->actual_power_mask;
		state.handle_mask = need_restore_mask;

		power_ctrl->actual_power_mask |= need_restore_mask;
	}
	state.handle_done_time = ktime_get();
	mutex_unlock(&power_ctrl->lock);

	dpu_power_up_work_print_state(power_ctrl, &state);
}

static void dpu_power_down_work_print_state(struct dpu_power_ctrl *power_ctrl,
		struct dpu_power_intr_handler_state *state)
{
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_intr_context *ctx;
	ktime_t hw_power_down_time;

	if (!power_ctrl->crtc)
		return;

	ctx = to_dpu_drm_dev(power_ctrl->crtc->dev)->dpu_kms->virt_ctrl.intr_ctx;
	hw_power_down_time = ctx->intr_status.curr_timestamp[INTR_ONLINE0_TMG_ACTIVE_POWER_DOWN];
	dpu_crtc_state = to_dpu_crtc_state(power_ctrl->crtc->state);

	POWER_DEBUG("farme_no %llu: desired 0x%x, actual 0x%x, power_down 0x%x\n",
			dpu_crtc_state->frame_no,
			state->desired_power_mask,
			state->actual_power_mask,
			state->handle_mask);
	POWER_DEBUG("delta to hw: enter %lld(us), get_lock %lld(us), handle_done %lld(us)\n",
			ktime_to_us(ktime_sub(state->enter_work_time, hw_power_down_time)),
			ktime_to_us(ktime_sub(state->get_lock_time, hw_power_down_time)),
			ktime_to_us(ktime_sub(state->handle_done_time, hw_power_down_time)));
}

static bool _check_can_do_power_down(struct dpu_power_ctrl *power_ctrl)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_drm_device *dpu_dev;
	struct dpu_intr_context *ctx;
	struct dpu_kms *kms;
	ktime_t hw_power_down_time;
	ktime_t delta, current_time;
	u32 hw_status;
	bool is_idle;
	u32 index;

	index = drm_crtc_index(power_ctrl->crtc);
	dpu_dev = to_dpu_drm_dev(power_ctrl->crtc->dev);
	kms = dpu_dev->dpu_kms;
	virt_pipe = &kms->virt_pipeline[index];
	hw_scene_ctl = virt_pipe->hw_scene_ctl;
	dpu_crtc_state = to_dpu_crtc_state(power_ctrl->crtc->state);

	/* sense ctrl busy */
	if (hw_scene_ctl) {
		dpu_core_perf_clk_active();
		is_idle  = hw_scene_ctl->ops.is_idle(&hw_scene_ctl->hw, &hw_status);
		dpu_core_perf_clk_deactive();
		if (!is_idle) {
			POWER_DEBUG("sense ctrl busy(hw status: 0x%x), skip power down(frame no: %llu)\n",
				hw_status,
				dpu_crtc_state->frame_no);
			return false;
		}
	}

	/* last power up late */
	if (atomic_read(&power_ctrl->power_up_timeout)) {
		POWER_DEBUG("last power up late, skip power down(frame no: %llu)\n",
				dpu_crtc_state->frame_no);

		atomic_set(&power_ctrl->power_up_timeout, 0);
		return false;
	}

	ctx = kms->virt_ctrl.intr_ctx;
	hw_power_down_time = ctx->intr_status.curr_timestamp[INTR_ONLINE0_TMG_ACTIVE_POWER_DOWN];
	current_time = ktime_get();
	delta = ktime_sub(current_time, hw_power_down_time);

	/* power down work schedule late */
	if (ktime_to_ms(delta) > DPU_POWER_WORK_SAFE_DELAY_TIME) {
		POWER_DEBUG("schedule late, skip power down(frame no: %llu), delta %llu(us)\n",
			dpu_crtc_state->frame_no,
			ktime_to_us(delta));
		return false;
	}

	return true;
}

static void dpu_power_down_work_handler(struct kthread_work *work)
{
	struct dpu_power_intr_handler_state state = {0};
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_power_client *client;
	u32 power_down_mask = 0;
	bool check_pass = false;

	power_ctrl = container_of(work, struct dpu_power_ctrl, power_down_work);
	if (!power_ctrl) {
		POWER_ERROR("power ctrl is null\n");
		return;
	}

	check_pass = _check_can_do_power_down(power_ctrl);
	if (!check_pass)
		return;

	client = power_ctrl->client;
	state.enter_work_time = ktime_get();

	mutex_lock(&power_ctrl->lock);
	state.get_lock_time = ktime_get();

	if (power_ctrl->frame_ctrl_enabled) {
		power_down_mask = power_ctrl->desired_power_mask &
				power_ctrl->actual_power_mask & power_ctrl->frame_power_mask;
		client->ops->disable(client, power_down_mask);
		state.desired_power_mask = power_ctrl->desired_power_mask;
		state.actual_power_mask = power_ctrl->actual_power_mask;
		state.handle_mask = power_down_mask;

		power_ctrl->actual_power_mask &= ~power_down_mask;
	}
	state.handle_done_time = ktime_get();
	mutex_unlock(&power_ctrl->lock);

	dpu_power_down_work_print_state(power_ctrl, &state);
}

static void dpu_power_frame_ctrl_irq_handler(int intr_id, void *data)
{
	struct dpu_virt_pipeline *pipeline;
	struct dpu_power_mgr *power_mgr;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_intr_pipe *intr_pipe;
	enum dpu_intr_type intr_type;
	struct drm_crtc *crtc;
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
	pipeline = &dpu_drm_dev->dpu_kms->virt_pipeline[pipe_id];
	power_ctrl = &pipeline->power_ctrl;
	power_mgr = dpu_drm_dev->dpu_kms->power_mgr;

	if (intr_id == INTR_ONLINE0_TMG_OTHER_PART_POWER_ON)
		kthread_queue_work(&power_mgr->power_worker, &power_ctrl->power_up_work);
	else if (intr_id == INTR_ONLINE0_TMG_ACTIVE_POWER_DOWN)
		kthread_queue_work(&power_mgr->power_worker, &power_ctrl->power_down_work);
}

int dpu_power_check(struct drm_atomic_state *state)
{
	struct drm_plane_state *old_plane_state, *new_plane_state;
	struct drm_plane *plane;
	struct drm_crtc_state *new_crtc_state;
	int i;

	if (!state) {
		DPU_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	if (dpu_power_is_in_suspending()) {
		DPU_INFO("dpu is in suspending state\n");
		return -EINVAL;
	}

	for_each_oldnew_plane_in_state(state, plane, old_plane_state, new_plane_state, i) {
		if (new_plane_state->crtc) {
			new_crtc_state = drm_atomic_get_new_crtc_state(state, new_plane_state->crtc);
			if (!new_crtc_state->active) {
				DPU_ERROR("commit valid plane to off crtc\n");
				goto error;
			}
		}
	}

	return 0;
error:
	return -EINVAL;
}

static int dpu_power_frame_ctrl_int_enable(struct dpu_intr_context *intr_ctx)
{
	int ret;

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_TMG_OTHER_PART_POWER_ON,
			&dpu_power_frame_ctrl_irq_handler);
	if (ret) {
		PERF_ERROR("faild to register power up int %d\n", ret);
		goto err_power_up;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_TMG_ACTIVE_POWER_DOWN,
			&dpu_power_frame_ctrl_irq_handler);
	if (ret) {
		PERF_ERROR("faild to register power down int %d\n", ret);
		goto err_power_down;
	}

	return 0;

err_power_down:
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_TMG_OTHER_PART_POWER_ON);
err_power_up:
	return ret;
}

static void dpu_power_frame_ctrl_int_disable(struct dpu_intr_context *intr_ctx)
{
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_TMG_OTHER_PART_POWER_ON);
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_TMG_ACTIVE_POWER_DOWN);
}

static void dpu_power_frame_ctrl_power_off_cb(void *data)
{
	struct dpu_power_ctrl *power_ctrl = data;

	if (!power_ctrl) {
		POWER_ERROR("invalid cb data\n");
		return;
	}

	atomic_set(&power_ctrl->need_restore, 1);
}

static void dpu_power_frame_ctrl_enable(struct dpu_power_ctrl *power_ctrl)
{
	struct dpu_drm_device *dpu_drm_dev = to_dpu_drm_dev(power_ctrl->crtc->dev);
	struct dpu_virt_ctrl *virt_ctrl;
	int ret;

	dpu_idle_enable_ctrl(false);

	virt_ctrl = &dpu_drm_dev->dpu_kms->virt_ctrl;
	if (power_ctrl->frame_ctrl_enabled == true)
		DPU_WARN("why already enabled\n");

	mutex_lock(&power_ctrl->lock);

	atomic_set(&power_ctrl->power_up_timeout, 0);
	ret = dpu_power_register_power_off_cb(DPU_PARTITION_1, power_ctrl,
			dpu_power_frame_ctrl_power_off_cb);
	if (!ret) {
		ret = dpu_power_frame_ctrl_int_enable(virt_ctrl->intr_ctx);
		if (!ret)
			power_ctrl->frame_ctrl_enabled = true;
	}
	mutex_unlock(&power_ctrl->lock);
}

static void dpu_power_frame_ctrl_disable(struct dpu_power_ctrl *power_ctrl)
{
	struct dpu_drm_device *dpu_drm_dev = to_dpu_drm_dev(power_ctrl->crtc->dev);
	struct dpu_power_client *client;
	struct dpu_virt_ctrl *virt_ctrl;
	struct dpu_power_mgr *power_mgr;
	u32 need_restore_mask;

	if (!power_ctrl->frame_ctrl_enabled)
		DPU_WARN("why frame power ctrl is already disabled\n");

	virt_ctrl = &dpu_drm_dev->dpu_kms->virt_ctrl;
	client = power_ctrl->client;
	power_mgr = dpu_drm_dev->dpu_kms->power_mgr;

	mutex_lock(&power_ctrl->lock);
	dpu_power_frame_ctrl_int_disable(virt_ctrl->intr_ctx);
	need_restore_mask =
			power_ctrl->desired_power_mask & ~power_ctrl->actual_power_mask;
	if (need_restore_mask)
		client->ops->enable(client, need_restore_mask);
	POWER_DEBUG("need_restore_mask is 0x%x\n", need_restore_mask);
	power_ctrl->frame_ctrl_enabled = false;
	power_ctrl->actual_power_mask |= need_restore_mask;

	dpu_power_unregister_power_off_cb(DPU_PARTITION_1);
	mutex_unlock(&power_ctrl->lock);

	kthread_flush_worker(&power_mgr->power_worker);

	dpu_idle_enable_ctrl(true);
}

static bool need_disable_frame_power_ctrl(struct dpu_power_ctrl *power_ctrl,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_crtc_state *cstate;
	u32 new_frame_rate;

	cstate = to_dpu_crtc_state(new_state);
	new_frame_rate = drm_mode_vrefresh(&new_state->mode);

	if (!power_ctrl->frame_ctrl_enabled)
		return false;

	if (need_disable_crtc(new_state)) {
		POWER_DEBUG("need disable: display off\n");
		return true;
	}

	if (cstate->keep_power_on) {
		POWER_DEBUG("need disable: keep power on\n");
		return true;
	}

	if (power_ctrl->frame_power_max_refresh_rate < new_frame_rate) {
		POWER_DEBUG("need disable: refresh_rate %d\n", new_frame_rate);
		return true;
	}

	/* currently, the implementation synchronizes power control and display commit.
	 * if power control and display commit were controlled asynchronously, heavy
	 * system load could cause the sense ctrl to hang, leading to frame drops.
	 */
	return true;
}

static bool need_enable_frame_power_ctrl(struct dpu_power_ctrl *power_ctrl,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_crtc_state *cstate;
	u32 new_frame_rate;

	cstate = to_dpu_crtc_state(new_state);
	new_frame_rate = drm_mode_vrefresh(&new_state->mode);

	if (power_ctrl->frame_ctrl_enabled)
		return false;

	if (!new_state->active)
		return false;

	if (cstate->keep_power_on)
		return false;

	if (power_ctrl->frame_power_max_refresh_rate < new_frame_rate)
		return false;

	POWER_DEBUG("need enable: refresh_rate %d\n", new_frame_rate);
	return true;
}

void dpu_power_enable_for_crtc(struct drm_crtc *crtc, u32 mask,
		enum dpu_power_request_type type)
{
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_drm_device *dpu_dev;
	struct dpu_power_client *client;
	u32 direct_power_on_mask;
	struct dpu_kms *kms;
	u32 index;

	if (!crtc) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	if (!mask)
		return;

	index = drm_crtc_index(crtc);
	dpu_dev = to_dpu_drm_dev(crtc->dev);
	kms = dpu_dev->dpu_kms;
	virt_pipe = &kms->virt_pipeline[index];
	power_ctrl = &virt_pipe->power_ctrl;
	client = power_ctrl->client;

	POWER_DEBUG("crtc %d, mask 0x%x\n", index, mask);

	mutex_lock(&power_ctrl->lock);
	if (power_ctrl->frame_ctrl_enabled)
		direct_power_on_mask = mask & ~power_ctrl->frame_power_mask;
	else
		direct_power_on_mask = mask;

	if (type == PRESENT_POWER_REQUEST)
		power_ctrl->desired_power_mask |= mask;
	client->ops->enable(client, direct_power_on_mask);
	power_ctrl->actual_power_mask |= direct_power_on_mask;
	mutex_unlock(&power_ctrl->lock);

	POWER_DEBUG("crtc %d, mask 0x%x desired_mask:%#x actual_mask:%#x\n",
			index, mask, power_ctrl->desired_power_mask, power_ctrl->actual_power_mask);
}

void dpu_power_disable_for_crtc(struct drm_crtc *crtc, u32 mask,
		enum dpu_power_request_type type)
{
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_drm_device *dpu_dev;
	struct dpu_power_client *client;
	u32 direct_power_off_mask;
	struct dpu_kms *kms;
	u32 index;

	if (!crtc) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	index = drm_crtc_index(crtc);
	dpu_dev = to_dpu_drm_dev(crtc->dev);
	kms = dpu_dev->dpu_kms;
	virt_pipe = &kms->virt_pipeline[index];
	power_ctrl = &virt_pipe->power_ctrl;
	client = power_ctrl->client;

	POWER_DEBUG("crtc %d, mask 0x%x\n", index, mask);

	mutex_lock(&power_ctrl->lock);
	direct_power_off_mask = mask & power_ctrl->actual_power_mask;
	if (type == PRESENT_POWER_REQUEST)
		power_ctrl->desired_power_mask &= ~mask;
	if (direct_power_off_mask)
		client->ops->disable(client, direct_power_off_mask);
	power_ctrl->actual_power_mask &= ~direct_power_off_mask;
	mutex_unlock(&power_ctrl->lock);

	POWER_DEBUG("crtc %d, mask 0x%x desired_mask:%#x actual_mask:%#x\n",
			index, mask, power_ctrl->desired_power_mask, power_ctrl->actual_power_mask);
}

static void dpu_power_prepare_for_frame_ctrl(struct dpu_power_ctrl *power_ctrl,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		u32 mask)
{
	if (need_disable_frame_power_ctrl(power_ctrl, old_state, new_state))
		dpu_power_frame_ctrl_disable(power_ctrl);

	dpu_power_enable_for_crtc(crtc, mask, PRESENT_POWER_REQUEST);
}

static void dpu_power_finish_for_frame_ctrl(struct dpu_power_ctrl *power_ctrl,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		u32 mask)
{
	if (need_enable_frame_power_ctrl(power_ctrl, old_state, new_state))
		dpu_power_frame_ctrl_enable(power_ctrl);

	dpu_power_disable_for_crtc(crtc, mask, PRESENT_POWER_REQUEST);
}

static void _do_dpu_power_prepare_for_crtc_common(struct dpu_power_ctrl *power_ctrl,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		u32 mask)
{
	struct dpu_power_client *client;
	u32 crtc_index;

	if (!mask)
		return;

	crtc_index = drm_crtc_index(crtc);
	client = power_ctrl->client;
	if (!client) {
		DPU_ERROR("crtc %d: invalid client, old active %d, new active %d, mask 0x%x\n",
				crtc_index,
				old_state->active,
				new_state->active,
				mask);
		return;
	}

	POWER_DEBUG("crtc %d, mask 0x%x\n", crtc_index, mask);
	mutex_lock(&power_ctrl->lock);
	client->ops->enable(client, mask);
	power_ctrl->desired_power_mask |= mask;
	power_ctrl->actual_power_mask |= mask;
	mutex_unlock(&power_ctrl->lock);
}

static void _do_dpu_power_finish_for_crtc_common(struct dpu_power_ctrl *power_ctrl,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		u32 mask)
{
	struct dpu_power_client *client;
	u32 power_off_mask;
	u32 crtc_index;

	if (!mask)
		return;

	crtc_index = drm_crtc_index(crtc);
	client = power_ctrl->client;
	if (!client) {
		DPU_ERROR("crtc %d: invalid client, old active %d, new active %d, mask 0x%x\n",
				crtc_index,
				old_state->active,
				new_state->active,
				mask);
		return;
	}

	POWER_DEBUG("crtc %d, mask 0x%x\n", crtc_index, mask);
	mutex_lock(&power_ctrl->lock);
	power_off_mask = power_ctrl->desired_power_mask & mask;
	if (power_off_mask) {
		power_ctrl->desired_power_mask &= ~power_off_mask;
		power_ctrl->actual_power_mask &= ~power_off_mask;
		client->ops->disable(client, power_off_mask);
	} else {
		DPU_WARN("crtc %d, mask 0x%x already powered off\n",
				crtc_index, mask);
	}
	mutex_unlock(&power_ctrl->lock);
}

static inline u32 _get_power_prepare_mask(struct dpu_power_state *power_state)
{
	u32 mask = 0;

	if (power_state->dpu_need_power_on)
		mask |= BIT(DPU_PARTITION_0) | BIT(DPU_PARTITION_1) | BIT(DPU_PARTITION_2);

	return mask;
}

struct dpu_power_ctrl *get_power_ctrl_inst(struct drm_crtc *crtc)
{
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_kms *dpu_kms;

	if (!crtc) {
		DPU_ERROR("crtc is null\n");
		return NULL;
	}

	dpu_drm_dev = container_of(crtc->dev, struct dpu_drm_device, drm_dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	virt_pipe = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];

	return &virt_pipe->power_ctrl;
}

static void dpu_power_prepare_for_crtc(struct dpu_power_mgr *power_mgr,
		struct dpu_power_state *power_state,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_kms *dpu_kms;
	u32 index;
	u32 mask = 0;

	index = drm_crtc_index(crtc);
	dpu_kms = power_mgr->dpu_kms;
	virt_pipe = &dpu_kms->virt_pipeline[index];
	power_ctrl = &virt_pipe->power_ctrl;

	mask = _get_power_prepare_mask(power_state);

	if ((power_ctrl->lowpower_ctrl & DPU_LP_FRAME_POWER_CTRL_ENABLE) &&
			(is_dpu_lp_enabled(DPU_LP_FRAME_POWER_CTRL_ENABLE))) {
		dpu_power_prepare_for_frame_ctrl(power_ctrl, crtc, old_state, new_state, mask);
	} else {
		_do_dpu_power_prepare_for_crtc_common(power_ctrl, crtc, old_state, new_state, mask);
	}

	if (new_state->mode_changed)
		power_ctrl->current_refresh_rate = drm_mode_vrefresh(&new_state->mode);
}

static void dpu_power_prepare_for_encoder(struct drm_crtc_state *new_state)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *connector;
	u32 mask;

	if (new_state->active_changed || !new_state->active)
		return;

	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_conn_state = to_dsi_connector_state(connector->state);
			if (dsi_conn_state->pm_changed) {
				dsi_connector = to_dsi_connector(connector);
				dsi_display_set_power_mode_prepare(dsi_connector->display, dsi_conn_state->power_mode);
			}
		}
	}
}

static void dsi_power_prepare_state_update(struct dpu_power_state *power_state,
		struct drm_connector *connector,
		struct drm_crtc_state *new_state)
{
	struct dsi_connector_state *dsi_connector_state;
	struct dsi_connector *dsi_connector;
	u32 current_power_mode;
	u32 new_power_mode;

	dsi_connector_state = to_dsi_connector_state(connector->state);

	/* no active and power mode change */
	if (!new_state->active_changed && !dsi_connector_state->pm_changed)
		return;

	if (new_state->active_changed && !dsi_connector_state->pm_changed && new_state->active) {
		/* active change but not set power-mode-property, recovery */
		power_state->dpu_need_power_on = true;
		power_state->dsi_need_power_on = true;
		power_state->dsi_panel_need_power_on = true;

	} else {
		/* active and power-mode-property change */
		dsi_connector = to_dsi_connector(connector);
		current_power_mode = dsi_connector->current_power_mode;
		new_power_mode = dsi_connector_state->power_mode;

		switch (new_power_mode) {
		case DPU_POWER_MODE_ON:
		case DPU_POWER_MODE_DOZE:
			if ((current_power_mode == DPU_POWER_MODE_OFF) ||
					(current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND)) {
				power_state->dpu_need_power_on = true;
				power_state->dsi_need_power_on = true;
				power_state->dsi_panel_need_power_on =
						(current_power_mode == DPU_POWER_MODE_OFF) ? true : false;
			}

			break;
		case DPU_POWER_MODE_OFF:
			break;
		case DPU_POWER_MODE_DOZE_SUSPEND:
			power_state->dsi_panel_need_power_on =
					(current_power_mode == DPU_POWER_MODE_OFF) ? true : false;
			break;
		default:
			break;
		}
	}
}

void dsi_power_finish_state_update(struct dpu_power_state *power_state,
		struct drm_connector *connector,
		struct drm_crtc_state *new_state)
{
	struct dsi_connector_state *dsi_connector_state;
	struct dsi_connector *dsi_connector;
	u32 current_power_mode;
	u32 new_power_mode;

	dsi_connector_state = to_dsi_connector_state(connector->state);

	/* no active and power mode change */
	if (!new_state->active_changed && !dsi_connector_state->pm_changed)
		return;

	if (new_state->active_changed && !dsi_connector_state->pm_changed && !new_state->active) {
		/* active change but not set power-mode-property, recovery */
		power_state->dpu_need_power_off = true;
		power_state->dsi_need_power_off = true;
		power_state->dsi_panel_need_power_off = true;

	} else {
		/* active and power-mode-property change */
		dsi_connector = to_dsi_connector(connector);
		current_power_mode = dsi_connector->current_power_mode;
		new_power_mode = dsi_connector_state->power_mode;

		switch (new_power_mode) {
		case DPU_POWER_MODE_ON:
		case DPU_POWER_MODE_DOZE:
			break;
		case DPU_POWER_MODE_OFF:
			if ((current_power_mode == DPU_POWER_MODE_ON) ||
					(current_power_mode == DPU_POWER_MODE_DOZE)) {
				power_state->dpu_need_power_off = true;
				power_state->dsi_need_power_off = true;
			}
			if (current_power_mode == DPU_POWER_MODE_DOZE_SUSPEND) {
				power_state->dpu_need_power_off = true;
				power_state->dsi_need_power_off = true;

				/* Todo: move to dpu_power_prepare, for suspend -> off */
				power_state->dpu_need_power_on = true;
				power_state->dsi_need_power_on = true;
				dsi_power_get(connector);
			}
			power_state->dsi_panel_need_power_off = true;
			break;
		case DPU_POWER_MODE_DOZE_SUSPEND:
			if ((current_power_mode == DPU_POWER_MODE_ON) ||
					(current_power_mode == DPU_POWER_MODE_DOZE)) {
				power_state->dpu_need_power_off = true;
				power_state->dsi_need_power_off = true;
			}
			break;
		default:
			break;
		}
	}
}

static void dpu_power_prepare_state_update(struct dpu_power_state *power_state,
		struct drm_crtc_state *new_state,
		bool is_offline)
{
	struct drm_connector *connector;
	bool find_dsi_connector;
	u32 mask;

	if (is_offline) {
		POWER_DEBUG("offline display\n");
		power_state->dpu_need_power_on = true;
		return;
	}

	find_dsi_connector = false;
	for_each_connector_per_crtc(connector, new_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_power_prepare_state_update(power_state, connector, new_state);
			find_dsi_connector = true;
		}
	}

	if (!find_dsi_connector && new_state->active_changed)
		power_state->dpu_need_power_on = need_enable_crtc(new_state);
}

static void dpu_power_finish_state_update(struct dpu_power_state *power_state,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		bool is_offline)
{
	struct drm_connector *connector;
	bool find_dsi_connector;
	u32 mask;

	if (is_offline) {
		POWER_DEBUG("offline display\n");
		power_state->dpu_need_power_off = true;
		return;
	}

	find_dsi_connector = false;
	for_each_connector_per_crtc(connector, old_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_power_finish_state_update(power_state, connector, new_state);
			find_dsi_connector = true;
		}
	}

	if (!find_dsi_connector && new_state->active_changed)
		power_state->dpu_need_power_off = need_disable_crtc(new_state);
}

static void dpu_power_state_begin_update(struct dpu_power_state *power_state,
		struct drm_atomic_state *state,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state,
		bool is_offline)
{
	dpu_power_prepare_state_update(power_state, new_state, is_offline);

	dpu_power_finish_state_update(power_state, old_state, new_state, is_offline);
}

static void dpu_power_state_end_update(struct dpu_power_state *power_state)
{
	power_state->dpu_need_power_on = false;
	power_state->dsi_need_power_on = false;
	power_state->dsi_panel_need_power_on = false;
	power_state->dpu_need_power_off = false;
	power_state->dsi_need_power_off = false;
	power_state->dsi_panel_need_power_off = false;
}

static void dpu_power_prepare_for_aod(struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *connector;
	struct drm_device *drm_dev;
	struct drm_crtc_state *crtc_state;
	u32 current_power_mode, new_power_mode;
	u32 mask;

	crtc_state = new_state->connector_mask ? new_state : old_state;
	for_each_connector_per_crtc(connector, crtc_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_conn_state = to_dsi_connector_state(connector->state);
			if (!dsi_conn_state->pm_changed)
				return;

			dsi_connector = to_dsi_connector(connector);
			current_power_mode = dsi_connector->current_power_mode;
			new_power_mode = dsi_conn_state->power_mode;

			drm_dev = connector->dev;
			if (!drm_dev) {
				DPU_ERROR("why dev is null %pK\n", drm_dev);
				return;
			}

			if ((new_power_mode == DPU_POWER_MODE_DOZE) &&
					(current_power_mode != DPU_POWER_MODE_DOZE)) {
				pm_stay_awake(drm_dev->dev);
				DPU_INFO("pm stay awake\n");
			}

			if ((new_power_mode != DPU_POWER_MODE_DOZE) &&
					(current_power_mode == DPU_POWER_MODE_DOZE)) {
				pm_relax(drm_dev->dev);
				DPU_INFO("pm relax\n");
			}
		}
	}
}

void dpu_power_prepare(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_power_state *power_state;
	u32 index;

	index = drm_crtc_index(crtc);
	power_state = &dpu_kms->virt_pipeline[index].power_state;

	dpu_power_prepare_for_aod(old_state, new_state);

	dpu_power_state_begin_update(power_state, state, old_state, new_state,
			dpu_kms->virt_pipeline[index].is_offline);

	if (power_state->dpu_need_power_off)
		dpu_tui_power_off_notice(crtc);

	/* HW Constraint:
	 * dsi/dp power up needs to be ealier than dpu partition.
	 */
	dpu_power_prepare_for_connector(dpu_kms->power_mgr, power_state, new_state);
	dpu_power_prepare_for_crtc(dpu_kms->power_mgr, power_state,
			state, crtc, old_state, new_state);

	dpu_power_prepare_for_encoder(new_state);
}

static inline u32 _get_power_finish_mask(struct dpu_power_state *power_state)
{
	u32 mask = 0;

	if (power_state->dpu_need_power_off)
		mask |= BIT(DPU_PARTITION_0) | BIT(DPU_PARTITION_1) | BIT(DPU_PARTITION_2);

	return mask;
}

static void dpu_power_finish_for_crtc(struct dpu_power_mgr *power_mgr,
		struct dpu_power_state *power_state,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_virt_pipeline *virt_pipe;
	struct dpu_power_ctrl *power_ctrl;
	struct dpu_kms *dpu_kms;
	u32 index;
	u32 mask = 0;

	index = drm_crtc_index(crtc);
	dpu_kms = power_mgr->dpu_kms;
	virt_pipe = &dpu_kms->virt_pipeline[index];
	power_ctrl = &virt_pipe->power_ctrl;

	mask = _get_power_finish_mask(power_state);

	if ((power_ctrl->lowpower_ctrl & DPU_LP_FRAME_POWER_CTRL_ENABLE) &&
			(is_dpu_lp_enabled(DPU_LP_FRAME_POWER_CTRL_ENABLE))) {
		dpu_power_finish_for_frame_ctrl(power_ctrl, crtc, old_state, new_state, mask);
	} else {
		_do_dpu_power_finish_for_crtc_common(power_ctrl, crtc, old_state, new_state, mask);
	}
}

static void dpu_power_finish_prepare_for_connector(struct dpu_power_mgr *power_mgr,
		struct dpu_power_state *power_state,
		struct drm_crtc_state *old_state)
{
	struct drm_connector *connector;
	u32 mask;

	if (!power_state->dsi_panel_need_power_off)
		return;

	for_each_connector_per_crtc(connector, old_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			if (power_state->dsi_panel_need_power_off)
				dsi_power_prepare_put(connector);
		}
	}
}

static void dpu_power_finish_for_encoder(struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dsi_connector_state *dsi_conn_state;
	struct dsi_connector *dsi_connector;
	struct drm_connector *connector;
	u32 mask;

	if (new_state->active_changed || new_state->active)
		return;

	for_each_connector_per_crtc(connector, old_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_conn_state = to_dsi_connector_state(connector->state);
			if (dsi_conn_state->pm_changed) {
				dsi_connector = to_dsi_connector(connector);
				dsi_display_set_power_mode_finish(dsi_connector->display, dsi_conn_state->power_mode);
			}
		}
	}
}

static void dpu_power_finish_for_connector(
		struct dpu_power_mgr *power_mgr,
		struct dpu_power_state *power_state,
		struct drm_crtc_state *old_state)
{
	struct dsi_connector *dsi_conn;
	struct drm_connector *connector;
	struct dsi_display *dsi_display;
	struct dp_connector *dp_conn;
	struct dp_display *dp_display;
	u32 mask;

	for_each_connector_per_crtc(connector, old_state, mask) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			if (!power_state->dsi_need_power_off &&
					!power_state->dsi_panel_need_power_off)
				return;

			if (power_state->dsi_need_power_off) {
				dsi_conn = to_dsi_connector(connector);
				dsi_display = dsi_conn->display;
				/* dsi should be reinitialize after next power up */
				dsi_display_lock(dsi_display);
				if (dsi_power_put(connector))
					dsi_display_initialized_set(dsi_display, false);

				dsi_display_unlock(dsi_display);
			}

			if (power_state->dsi_panel_need_power_off)
				dsi_panel_power_put(connector);

		} else if (connector->connector_type == DRM_MODE_CONNECTOR_DisplayPort) {
			if (!power_state->dpu_need_power_off)
				return;

			dp_conn = to_dp_connector(connector);
			dp_display = dp_conn->display;

			complete(&dp_display->hpd->off_completed);
		}
	}
}

void dpu_power_finish(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state)
{
	struct dpu_power_state *power_state;
	u32 index;

	index = drm_crtc_index(crtc);
	power_state = &dpu_kms->virt_pipeline[index].power_state;

	dpu_power_finish_for_encoder(old_state, new_state);

	dpu_power_finish_prepare_for_connector(dpu_kms->power_mgr, power_state, old_state);

	/*
	 * HW Constraint:
	 * dsi/dp power off needs to be later than dpu partition.
	 */
	dpu_power_finish_for_crtc(dpu_kms->power_mgr, power_state,
			state, crtc, old_state, new_state);

	dpu_power_finish_for_connector(dpu_kms->power_mgr, power_state, old_state);

	dpu_power_state_end_update(power_state);
}

void dpu_power_exception_recovery(struct dpu_virt_pipeline *pipeline)
{
	struct dpu_power_ctrl *power_ctrl;

	if (!pipeline) {
		POWER_ERROR("invalid parameters: %pK\n", pipeline);
		return;
	}

	power_ctrl = &pipeline->power_ctrl;
	if (!power_ctrl->frame_ctrl_enabled)
		return;

	POWER_INFO("do power exception recovery\n");
	dpu_power_frame_ctrl_disable(power_ctrl);
}

bool is_crtc_powered_on(struct dpu_kms *dpu_kms, struct drm_crtc *crtc)
{
	struct dpu_virt_pipeline *pipe_line;
	struct dpu_power_client *client;
	u32 index;

	if (!dpu_kms || !crtc) {
		POWER_ERROR("invalid parameters: %pK, %pK\n", dpu_kms, crtc);
		return false;
	}

	index = drm_crtc_index(crtc);
	pipe_line = &dpu_kms->virt_pipeline[index];
	client = pipe_line->power_ctrl.client;

	if (!client)
		return false;
	else
		return client->ops->is_powered_on(client);
}

u32 get_crtc_desired_power_mask(struct dpu_kms *dpu_kms, struct drm_crtc *crtc)
{
	struct dpu_virt_pipeline *pipe_line;
	struct dpu_power_ctrl *power_ctrl;
	u32 index;

	if (!dpu_kms || !crtc) {
		POWER_ERROR("invalid parameters: %pK, %pK\n", dpu_kms, crtc);
		return false;
	}

	index = drm_crtc_index(crtc);
	pipe_line = &dpu_kms->virt_pipeline[index];
	power_ctrl = &pipe_line->power_ctrl;

	return power_ctrl->desired_power_mask;
}

bool dpu_power_frame_ctrl_need_hw_restore(struct dpu_power_ctrl *power_ctrl)
{
	if (!power_ctrl) {
		POWER_ERROR("invalid parameters\n");
		return false;
	}

	if (atomic_read(&power_ctrl->need_restore)) {
		POWER_DEBUG("need disable, restore\n");
		atomic_set(&power_ctrl->need_restore, 0);
		return true;
	}

	return false;
}

void dpu_power_ctrl_init(struct dpu_power_ctrl *power_ctrl)
{
	if (!power_ctrl) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	mutex_init(&power_ctrl->lock);

	kthread_init_work(&power_ctrl->power_up_work,
			dpu_power_up_work_handler);
	kthread_init_work(&power_ctrl->power_down_work,
			dpu_power_down_work_handler);
}

void dpu_power_ctrl_deinit(struct dpu_power_ctrl *power_ctrl)
{
	if (!power_ctrl) {
		POWER_ERROR("invalid parameters\n");
		return;
	}

	mutex_destroy(&power_ctrl->lock);
}
