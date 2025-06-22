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

#ifndef _DPU_STRIPE_H_
#define _DPU_STRIPE_H_

#include "dpu_hw_common.h"
#include "color/xring_dpu_color.h"
#include "display/xring_stripe_common.h"
#include <drm/drm_connector.h>
#include <drm/drm_writeback.h>
#include <drm/drm_framebuffer.h>
#include "dpu_plane.h"
#include "dpu_color.h"
#include "dpu_wb.h"
#include "dpu_crtc.h"

void dpu_hw_wb_cmps_stripe_cmdlist_prepare(struct dpu_crtc *dpu_crtc,
		struct dpu_wb_connector *dpu_wb, s64 blk_id);
void dpu_hw_plane_stripe_cmdlist_prepare(struct drm_plane *plane);
void dpu_hw_stripe_post_mixer_blks_cmdlist_prepare(struct dpu_crtc *dpu_crtc,
		struct drm_connector *connector);
void dpu_stripe_plane_cmdlist_append(struct drm_plane *plane, s64 frame_id);
void dpu_stripe_cmps_wb_cmdlist_append(struct drm_connector *connector, s64 frame_id, u32 cfg_rdy);

int dpu_plane_create_stripe_properties(struct drm_plane *plane);
int dpu_wb_create_stripe_properties(struct drm_writeback_connector *wb_conn);
void dpu_stripe_commit(struct drm_atomic_state *state);

#endif /* _DPU_STRIPE_H_ */
