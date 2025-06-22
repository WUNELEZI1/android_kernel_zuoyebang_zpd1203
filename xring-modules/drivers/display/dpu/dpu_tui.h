/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef __DPU_TUI_H_
#define __DPU_TUI_H_

#include <linux/time64.h>

#include <drm/drm_device.h>
#include <display_tui.h>

#define TEE_TUI_EXIT_TIME_OVER 50

enum {
	DPU_TUI_TYPE_ENTER = 1,
	DPU_TUI_TYPE_EXIT,
};

/**
 * dpu_tui - dpu tui context
 * @dev: drm dev
 * @drm_crtc: primary panel crtc
 * @tui_state_mutex: tui Enter exit protection
 * @notice_tee_exit_tv : Callback Timeout Statistics
 * @tui_state: Whether currently in the tui
 * @is_dpu_powering_down: Is the power down?
 * @callback: power down  callback interface
 */
struct dpu_tui {
	struct drm_device *dev;
	struct drm_crtc *crtc;
	struct mutex tui_state_mutex;
	struct timespec64 notice_tee_exit_tv;
	bool tui_state;
	bool is_dpu_powering_down;
	wait_queue_head_t wait_queue;

	ree_power_off_notify callback;
};

/**
 * dpu_tui_init - initialize the tui on kms
 * @tui: tui global contex pointer
 * @dev: the drm_device structure pointer
 */
int dpu_tui_init(struct dpu_tui **tui, struct drm_device *dev);

/**
 * dpu_tui_event_init - tui primary panel register event
 * @crtc: crtc at registration
 * @event: Registered envent pointer
 */
int dpu_tui_event_init(struct drm_crtc *crtc, void *event);

/**
 * dpu_tui_deinit - tui deinit
 * @tui: tui global contex pointer
 */
void dpu_tui_deinit(struct dpu_tui *tui);

/**
 * dpu_get_tui_state - get current tui status
 */
bool dpu_get_tui_state(void);

/**
 * dpu_tui_power_off_notice - dpu power down notification tui quit
 * @crtc: crtc at registration
 */
int dpu_tui_power_off_notice(struct drm_crtc *crtc);

#endif
