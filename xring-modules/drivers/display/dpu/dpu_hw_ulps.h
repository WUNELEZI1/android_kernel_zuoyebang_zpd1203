/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef DPU_HW_ULPS_H
#define DPU_HW_ULPS_H

struct drm_crtc_state;

/**
 * dpu_hw_ulps_enable_with_cmdlist - enable hw ulps with cmdlist
 *
 * @new_state: new crtc state
 */
void dpu_hw_ulps_enable_with_cmdlist(struct drm_crtc_state *new_state);

/**
 * dpu_hw_ulps_disable_with_cmdlist - disable hw ulps with cmdlist
 *
 * @new_state: new crtc state
 */
void dpu_hw_ulps_disable_with_cmdlist(struct drm_crtc_state *new_state);

/**
 * dpu_hw_ulps_disable - disable hw ulps with cpu
 *
 * @new_state: new crtc state
 */
void dpu_hw_ulps_disable(struct drm_crtc_state *new_state);

/**
 * dpu_hw_ulps_status_dump - dump hw ulps status
 *
 * @new_state: new crtc state
 */
void dpu_hw_ulps_status_dump(struct drm_crtc_state *new_state);

/**
 * dpu_hw_ulps_ctrl_dyn - dynamic ctrl hw ulps
 *
 * @enable: true:enable, false:disable
 */
void dpu_hw_ulps_ctrl_dyn(bool enable);

/**
 * dpu_hw_ulps_exception_recovery - recovery hw ulps status when exception happens
 *
 * @new_state: point of new crtc state
 * @connector: point of drm connector
 */
void dpu_hw_ulps_exception_recovery(struct drm_crtc_state *new_state,
		struct drm_connector *connector);

#endif
