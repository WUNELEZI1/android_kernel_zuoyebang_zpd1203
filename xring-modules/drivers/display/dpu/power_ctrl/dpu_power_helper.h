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

#ifndef _DPU_POWER_HELPER_H_
#define _DPU_POWER_HELPER_H_

#include <drm/drm_atomic.h>

#include"dpu_power_mgr.h"

/**
 * dpu_power_request_type - the dpu power request type
 * PRESENT_POWER_REQUEST - power request from commit flow
 * IDLE_POWER_REQUEST - power request from idle flow
 */
enum dpu_power_request_type {
	PRESENT_POWER_REQUEST,
	IDLE_POWER_REQUEST,
};

/**
 * dpu_power_prepare - do runtime power on at dpu drm commit tail
 * @dpu_kms: the pointer of dpu_kms
 * @state: pointer of the current atomic state
 * @crtc: current crtc
 * @old_state: the crtc's old state
 * @new_state: the crtc's new state
 *
 * For each crtc, if old state doesn't need one partition's power, but new state
 * need it, then do power get on this partation.
 *
 * Note that this function just do power get, and must be called before touch
 * dpu hardware.
 */
void dpu_power_prepare(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state);

/**
 * dpu_power_finish - do runtime power off at dpu drm commit tail
 * @dpu_kms: the dpu kms pointer
 * @state: pointer of the current atomic state
 * @crtc: the crtc to do power vote
 * @old state: the crtc's old state
 * @new_state: the crtc's new state
 *
 * For each crtc, if new state doesn't need one partition's power, but old state
 * need it, then do power put on this partition.
 */
void dpu_power_finish(struct dpu_kms *dpu_kms,
		struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state);

/**
 * dpu_power_enable_for_crtc - enable crtc's power
 * @crtc: the crtc to do power enable
 * @mask: the dpu partition power mask
 * @type: the dpu power request type
 */
void dpu_power_enable_for_crtc(struct drm_crtc *crtc, u32 mask,
		enum dpu_power_request_type type);

/**
 * dpu_power_check - do power check before drm commit
 * @state: pointer of the current atomic state
 */
int dpu_power_check(struct drm_atomic_state *state);

/**
 * dpu_power_disable_for_crtc - disable crtc's power
 * @crtc: the crtc to do power disable
 * @mask: the dpu partition power mask
 * @type: the dpu power request type
 */
void dpu_power_disable_for_crtc(struct drm_crtc *crtc, u32 mask,
		enum dpu_power_request_type type);

/**
 * get_crtc_desired_power_mask - get crtc's desired power mask
 * @dpu_kms: the dpu kms pointer
 * @crtc: the drm crtc pointer
 *
 * Return: the crtc's desired power mask
 */
u32 get_crtc_desired_power_mask(struct dpu_kms *dpu_kms, struct drm_crtc *crtc);

/**
 * dpu_power_exception_recovery - do dpu power exception recover for frame power ctrl sence
 * @pipeline: the virt pipeline pointer
 */
void dpu_power_exception_recovery(struct dpu_virt_pipeline *pipeline);

/**
 * dpu_power_frame_ctrl_need_hw_restore - need do restore regs or not
 * @power_ctrl: the dpu power ctrl pointer
 *
 * Return: true for need restore, false for not
 */
bool dpu_power_frame_ctrl_need_hw_restore(struct dpu_power_ctrl *power_ctrl);

/**
 * get_power_ctrl_inst - Get the power ctrl inst object
 *
 * @crtc: the drm crtc pointer
 *
 * Return struct dpu_power_ctrl*
 */
struct dpu_power_ctrl *get_power_ctrl_inst(struct drm_crtc *crtc);

/**
 * dpu_power_ctrl_init - init per display power ctrl struct
 * @power_ctrl: the pointer of each display
 */
void dpu_power_ctrl_init(struct dpu_power_ctrl *power_ctrl);

/**
 * dpu_power_ctrl_deinit - deinit per display power ctrl struct
 * @power_ctrl: the pointer of each display
 */
void dpu_power_ctrl_deinit(struct dpu_power_ctrl *power_ctrl);

#endif
