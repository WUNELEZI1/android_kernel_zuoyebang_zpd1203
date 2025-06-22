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

#ifndef _DPU_IDLE_HELPER_H_
#define _DPU_IDLE_HELPER_H_

#include "dpu_idle_core.h"
#include "dpu_kms.h"

/**
 * dpu_idle_enable_ctrl - control idle enable status
 * If a valid crtc is imported, only control IDLE in crtc unit,
 * and if you want direct control, pass in NULL.
 *
 * @enable: true for enable idle, false for disable idle
 */
void dpu_idle_enable_ctrl(bool enable);

/**
 * dpu_vsync_isr_idle_handler - idle handler for vsync isr
 *
 * Decrease expire_count in each vsync isr coming.
 * If expire_count equals zero, will queue a workqueue to enter idle.
 *
 * @crtc: only crtc of primary panel can enter idle
 */
void dpu_vsync_isr_idle_handler(struct drm_crtc *crtc);

/**
 * dpu_idle_helper_stop - vote to exit idle for user
 */
void dpu_idle_helper_stop(void);

/**
 * dpu_idle_helper_restart - vote to restart idle func for user
 */
void dpu_idle_helper_restart(struct drm_crtc *crtc);

/**
 * dpu_idle_register_primary_crtc - register primary online crtc when panel power on
 */
void dpu_idle_register_primary_crtc(void);

/**
 * dpu_idle_unregister_primary_crtc - unregister primary online crtc when panel power off
 */
void dpu_idle_unregister_primary_crtc(void);

/**
 * need_restore_register_after_idle - Need restore register after exit idle
 *
 * @crtc: pointer of struct drm_crtc, used to check primary crtc
 * Return true or false.
 */
bool need_restore_register_after_idle(struct drm_crtc *crtc);

/**
 * need_restart_dpu_display_after_idle - Need restart dpu after exit idle
 *
 * @crtc: pointer of struct drm_crtc, used to check primary crtc
 * Return true or false.
 */
bool need_restart_dpu_display_after_idle(struct drm_crtc *crtc);

/**
 * dpu_idle_event_init - dpu idle event initialize
 *
 * @crtc: the pointer of drm crtc
 * @event: the pointer of idle event
 * Return 0 on success, error number on failure
 */
int dpu_idle_event_init(struct drm_crtc *crtc, void *event);

/**
 * dpu_idle_status_dump - dump idle status for cmd or video idle
 */
void dpu_idle_status_dump(void);

/**
 * dpu_idle_policy_switch - switch policy between BASIC and NORMAL
 *
 * @new_level: policy between BASIC and NORMAL
 */
void dpu_idle_policy_switch(enum idle_policy_level new_level);

/**
 * dpu_idle_helper_postprocess - post process of exit idle
 *
 * @crtc: the pointer of drm  primary crtc
 */
void dpu_idle_helper_postprocess(struct drm_crtc *crtc);

#endif
