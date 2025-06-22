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

#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/time.h>

#include "dpu_power_mgr.h"
#include "dpu_drv.h"
#include "dpu_crtc.h"
#include "dpu_log.h"
#include "dpu_tui.h"
#include "dpu_idle_helper.h"
#include "dpu_hw_ulps.h"

struct dpu_tui *g_tui_info;

int dpu_tui_init(struct dpu_tui **tui, struct drm_device *dev)
{
	struct dpu_tui *dpu_tui;

	if (*tui) {
		DPU_ERROR("dpu tui has been initialized\n");
		return -EINVAL;
	}
	DPU_INFO("tui init.");

	dpu_tui = kzalloc(sizeof(*dpu_tui), GFP_KERNEL);
	if (!dpu_tui)
		return -ENOMEM;

	dpu_tui->dev = dev;
	dpu_tui->crtc = NULL;
	dpu_tui->tui_state = false;
	mutex_init(&dpu_tui->tui_state_mutex);
	init_waitqueue_head(&dpu_tui->wait_queue);

	*tui = dpu_tui;
	g_tui_info = dpu_tui;

	return 0;
}

void dpu_tui_deinit(struct dpu_tui *tui)
{
	if (!tui)
		return;
	mutex_destroy(&g_tui_info->tui_state_mutex);
	kfree(tui);
	g_tui_info = NULL;
	DPU_INFO("tui is deinit.");
}

int dpu_tui_event_init(struct drm_crtc *crtc, void *event)
{
	struct dpu_crtc_event *tui_event;

	if (!crtc || !event) {
		DPU_ERROR("crtc or event is null ptr");
		return -EINVAL;
	}

	g_tui_info->crtc = crtc;
	tui_event = (struct dpu_crtc_event *)event;
	switch (tui_event->event_type) {
	case DRM_PRIVATE_EVENT_TUI:
		tui_event->cb = NULL;
		tui_event->enable = NULL;
		break;
	default:
		DPU_WARN("unknow tui type %d", tui_event->event_type);
		return -ENOENT;
	}

	return 0;
}

bool dpu_get_tui_state(void)
{
	bool ret_state;

	if (!g_tui_info) {
		DPU_WARN("tui not init!");
		return false;
	}
	mutex_lock(&g_tui_info->tui_state_mutex);
	ret_state = g_tui_info->tui_state;
	mutex_unlock(&g_tui_info->tui_state_mutex);
	return ret_state;
}

int dpu_tui_power_off_notice(struct drm_crtc *crtc)
{
	uint32_t exit_call_times = 1;
	int ret;

	if (crtc != g_tui_info->crtc)
		return 0;

	if (!dpu_get_tui_state())
		return 0;
	ktime_get_real_ts64(&g_tui_info->notice_tee_exit_tv);
CHECK_TUI_STATUE:
	g_tui_info->is_dpu_powering_down = true;
	DPU_INFO("tui is still present. attempted %dst notification tui exit.",
			exit_call_times++);
	if (g_tui_info->callback) {
		g_tui_info->callback();
	} else {
		DPU_ERROR("tui power off callback have not registered");
		return -EINVAL;
	}

	ret = wait_event_interruptible_timeout(g_tui_info->wait_queue,
			!dpu_get_tui_state(), 1 * HZ);
	if (ret > 0) {
		DPU_INFO("tui has exited");
		return 0;
	} else if (ret == 0) {
		goto CHECK_TUI_STATUE;
	} else {
		DPU_WARN("wait for tui exit to be interrupted and wait again");
		goto CHECK_TUI_STATUE;
	}
}

int display_enter_tui(void)
{
	struct dpu_power_state *power_state;
	struct dpu_drm_device *dpu_dev;
	struct drm_event event;
	struct dpu_kms *kms;
	u8 tui_event_type;
	u32 index;

	if (g_tui_info == NULL || g_tui_info->crtc == NULL) {
		DPU_ERROR("tui not init.");
		return -EINVAL;
	}

	index = drm_crtc_index(g_tui_info->crtc);
	dpu_dev = to_dpu_drm_dev(g_tui_info->dev);
	kms = dpu_dev->dpu_kms;
	power_state = &kms->virt_pipeline[index].power_state;

	if (power_state->dpu_need_power_on) {
		DPU_WARN("dpu is powering on.");
		return -EINVAL;
	}

	if (power_state->dpu_need_power_off) {
		DPU_WARN("dpu is powering down.");
		return -EINVAL;
	}

	if (!is_dpu_power_safe_with_dpu_idle()) {
		DPU_WARN("dpu already power down.");
		return -EINVAL;
	}

	/* idle locks can't be nested with tui locks, they are prone to deadlocks.*/
	dpu_idle_enable_ctrl(false);
	dpu_hw_ulps_ctrl_dyn(false);
	dpu_core_perf_clk_active();
	dpu_power_get_helper(DPU_ALL_PARTITIONS);

	mutex_lock(&g_tui_info->tui_state_mutex);
	if (g_tui_info->tui_state == true) {
		mutex_unlock(&g_tui_info->tui_state_mutex);
		DPU_WARN("tui already enter.");
		return 0;
	}

	DPU_INFO("tui enter");

	event.length = 1;
	event.type = DRM_PRIVATE_EVENT_TUI;
	tui_event_type = DPU_TUI_TYPE_ENTER;

	dpu_drm_event_notify(g_tui_info->dev, &event, (u8 *)&tui_event_type);

	g_tui_info->tui_state = true;
	mutex_unlock(&g_tui_info->tui_state_mutex);

	return 0;
}
EXPORT_SYMBOL(display_enter_tui);

void display_exit_tui(void)
{
	struct timespec64 tee_exit_tv, diff;
	struct drm_event event;
	u8 tui_event_type;
	u64 time_diff;

	mutex_lock(&g_tui_info->tui_state_mutex);
	if (g_tui_info->tui_state == false) {
		mutex_unlock(&g_tui_info->tui_state_mutex);
		DPU_WARN("tui already exit");
		return;
	}

	if (!is_dpu_powered_on()) {
		mutex_unlock(&g_tui_info->tui_state_mutex);
		DPU_WARN("tui already power down");
		return;
	}

	if (g_tui_info->is_dpu_powering_down) {
		g_tui_info->is_dpu_powering_down = false;

		ktime_get_real_ts64(&tee_exit_tv);
		diff = timespec64_sub(tee_exit_tv, g_tui_info->notice_tee_exit_tv);
		time_diff = timespec64_to_ns(&diff) / PSEC_PER_NSEC / PSEC_PER_NSEC;
		if (time_diff > TEE_TUI_EXIT_TIME_OVER)
			DPU_WARN("mitee tui exit processing time %llums exceeds %dms",
					time_diff, TEE_TUI_EXIT_TIME_OVER);
	}

	event.length = 1;
	event.type = DRM_PRIVATE_EVENT_TUI;
	tui_event_type = DPU_TUI_TYPE_EXIT;

	dpu_drm_event_notify(g_tui_info->dev, &event, (u8 *)&tui_event_type);

	g_tui_info->tui_state = false;
	mutex_unlock(&g_tui_info->tui_state_mutex);
	wake_up(&g_tui_info->wait_queue);

	dpu_core_perf_clk_deactive();
	dpu_idle_enable_ctrl(true);
	dpu_hw_ulps_ctrl_dyn(true);
	dpu_power_put_helper(DPU_ALL_PARTITIONS);

	DPU_INFO("tui exit <<<");
}
EXPORT_SYMBOL(display_exit_tui);

void display_get_frame_resolution(uint32_t *width, uint32_t *height,
		uint32_t *dpi_x, uint32_t *dpi_y)
{
	struct drm_display_mode *drm_mode = NULL;

	if (g_tui_info && g_tui_info->crtc) {
		drm_mode = &(g_tui_info->crtc->mode);
		if (drm_mode == NULL) {
			DPU_WARN("dpu panel not init");
			return;
		}
		if (width == NULL || height == NULL) {
			DPU_ERROR("width or height is an invalid entry parameter");
			return;
		}

		*width = drm_mode->hdisplay;
		*height = drm_mode->vdisplay;

		/**
		 * 1 inch == 25.4 mm
		 * kernel is not friendly to floating-point arithmetic,
		 * the dpi returned here is multiplied by 1000,
		 * the caller needs to divide by 1000 when using it.
		 */
		if (dpi_x != NULL && drm_mode->width_mm != 0)
			*dpi_x = (drm_mode->hdisplay * 1000) / drm_mode->width_mm / 254 * 10;
		if (dpi_y != NULL && drm_mode->height_mm != 0)
			*dpi_y = (drm_mode->vdisplay * 1000) / drm_mode->height_mm / 254 * 10;

		DPU_INFO("width=%u, height=%u, dpix=%u, dpi_y=%u",
				width  != NULL ? *width  : 0,
				height != NULL ? *height : 0,
				dpi_x  != NULL ? *dpi_x  : 0,
				dpi_y  != NULL ? *dpi_y  : 0);
	}
}
EXPORT_SYMBOL(display_get_frame_resolution);

void display_callback_regist(ree_power_off_notify callback)
{
	if (g_tui_info) {
		g_tui_info->callback = callback;
		DPU_INFO("dpu tui power off callback function is registered.");
	} else {
		DPU_ERROR("dpu tui not init, g_tui_info is null!!!");
	}
}
EXPORT_SYMBOL(display_callback_regist);
