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

#include <linux/types.h>
#include <linux/slab.h>
#include <drm/drm_atomic.h>
#include <display/xring_dpu_drm.h>

#include "dpu_idle_helper.h"
#include "dpu_idle_cmd.h"
#include "dpu_idle_utils.h"
#include "dpu_kms.h"
#include "dpu_crtc.h"
#include "dpu_log.h"

void dpu_idle_enable_ctrl(bool enable)
{
	if (!is_idle_enabled())
		return;
	if (enable)
		dpu_idle_vote_restart(CTRL_CH);
	else
		dpu_idle_vote_stop(CTRL_CH);
}

void dpu_idle_helper_stop(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;
	if (!is_idle_policy_valid(idle_ctx))
		return;

	dpu_idle_vote_stop(PRESENT_CH);
}

void dpu_idle_helper_restart(struct drm_crtc *crtc)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;
	if (!is_idle_policy_valid(idle_ctx))
		return;

	if (idle_ctx->crtc && (idle_ctx->crtc == crtc))
		dpu_idle_vote_restart(PRESENT_CH);
	else
		dpu_idle_vote_restart(CTRL_CH);
}

void dpu_vsync_isr_idle_handler(struct drm_crtc *crtc)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;

	if (!idle_ctx->crtc || (idle_ctx->crtc != crtc))
		return;

	dpu_idle_task_commit();
}

bool need_restore_register_after_idle(struct drm_crtc *crtc)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return false;
	if (!crtc) {
		IDLE_ERROR("crtc is null\n");
		return false;
	}
	if (!idle_ctx->crtc || (idle_ctx->crtc != crtc))
		return false;

	return idle_ctx->restore_mode & RESTORE_REGISITER;
}

bool need_restart_dpu_display_after_idle(struct drm_crtc *crtc)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return false;
	if (!crtc) {
		IDLE_ERROR("crtc is null\n");
		return false;
	}
	if (!idle_ctx->crtc || (idle_ctx->crtc != crtc))
		return false;

	return idle_ctx->restore_mode & RESTART_DPU_DISPLAY;
}

void dpu_idle_register_primary_crtc(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;
	mutex_lock(&idle_ctx->idle_lock);
	idle_ctx->crtc = idle_ctx->conn->state->crtc;
	if (WARN_ON(!idle_ctx->crtc))
		IDLE_ERROR("register primary crtc failed\n");
	dpu_idle_input_handler_register();
	mutex_unlock(&idle_ctx->idle_lock);
}

void dpu_idle_unregister_primary_crtc(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;
	mutex_lock(&idle_ctx->idle_lock);
	idle_ctx->crtc = NULL;
	idle_ctx->restore_mode = NO_NEED_RESTORE;
	dpu_idle_input_handler_unregister();
	mutex_unlock(&idle_ctx->idle_lock);
}

static void dpu_idle_event(struct drm_crtc *crtc)
{
	struct drm_connector *conn = NULL;
	struct drm_event event;
	u32 idle_type;

	conn = dsi_primary_connector_get(crtc->dev);
	if (!conn) {
		DPU_ERROR("conn is null\n");
		return;
	}

	idle_type = dsi_connector_is_cmd_mode(conn) ? CMD_IDLE : VIDEO_IDLE;
	IDLE_DEBUG("idle_type:%u\n", idle_type);
	event.length = sizeof(u32);
	event.type = DRM_PRIVATE_EVENT_IDLE;
	dpu_drm_event_notify(crtc->dev, &event, (u8 *)(&idle_type));
}

static int dpu_idle_event_cb(struct drm_crtc *crtc)
{
	dpu_crtc_intr_event_queue(crtc, dpu_idle_event);
	return 0;
}

int dpu_idle_event_init(struct drm_crtc *crtc, void *event)
{
	struct dpu_crtc_event *idle_event;

	if (!crtc || !event)
		return -EINVAL;

	idle_event = (struct dpu_crtc_event *)event;
	switch (idle_event->event_type) {
	case DRM_PRIVATE_EVENT_IDLE:
		idle_event->intr_id = INTR_SIMULATE_IDLE_ENTER;
		idle_event->cb = dpu_idle_event_cb;
		idle_event->enable = NULL;
		break;
	default:
		DPU_WARN("unknow idle event type %d", idle_event->event_type);
		return -ENOENT;
	}

	return 0;
}

void dpu_idle_status_dump(void)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;

	DFX_INFO("idle_policy:%#x vote_count:%u active:%d restore_mode:%#x\n",
			idle_ctx->idle_policy, idle_ctx->vote_count,
			idle_ctx->active, idle_ctx->restore_mode);
	DFX_INFO("trace: enter_frame_no:%u enter_time:%llu exit_time:%llu\n",
			idle_ctx->trace.enter_frame_no,
			idle_ctx->trace.enter_time, idle_ctx->trace.exit_time);
	if (idle_ctx->ops.status_dump)
		idle_ctx->ops.status_dump(idle_ctx);
}

void dpu_idle_policy_switch(enum idle_policy_level new_policy)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();
	u32 basic_policy;

	if (!is_idle_enabled())
		return;

	if (!idle_ctx->ops.get_basic_policy)
		return;

	if (new_policy == BASIC) {
		basic_policy = idle_ctx->ops.get_basic_policy();
		dpu_idle_set_idle_policy(basic_policy);
	} else {
		dpu_idle_set_idle_policy(idle_ctx->pinfo->idle_policy);
	}

	IDLE_INFO("switch to %s level, policy:%#x\n",
		(new_policy == BASIC) ? "basic" : "normal", idle_ctx->idle_policy);
}

void dpu_idle_helper_postprocess(struct drm_crtc *crtc)
{
	struct dpu_idle_ctx *idle_ctx = dpu_get_idle_ctx();

	if (!is_idle_enabled())
		return;

	if (!crtc) {
		IDLE_ERROR("crtc is null\n");
		return;
	}
	if (!idle_ctx->crtc || (idle_ctx->crtc != crtc))
		return;

	if (idle_ctx->ops.postprocess)
		idle_ctx->ops.postprocess(idle_ctx);
}
