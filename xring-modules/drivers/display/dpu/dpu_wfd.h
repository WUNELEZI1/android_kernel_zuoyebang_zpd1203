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

#ifndef _DPU_WFD_H_
#define _DPU_WFD_H_

#include <display/xring_xplayer_uapi.h>
#include <drm/drm_writeback.h>
#include <drm/drm_framebuffer.h>
#include "dpu_plane.h"
#include "dpu_wb.h"

/**
 * dpu_wfd_source_create_slice_property - create wfd source writeback slice num property
 * @drm_wb_conn: drm_writeback_connector pointer
 * @dpu_wb_conn: dpu_wb_connector pointor
 *
 * return: zero on success, -errno on failure
 */
int dpu_wfd_source_create_slice_property(struct drm_writeback_connector *drm_wb_conn,
		struct dpu_wb_connector *dpu_wb_conn);

/**
 * dpu_wfd_source_config - config wfd source
 * @wb_cfg: wb_frame_cfg pointor
 * @wb_state: dpu_wb_connector_state pointor
 * @fb: drm_framebuffer pointor
 */
void dpu_wfd_source_config(struct wb_frame_cfg *wb_cfg,
		struct dpu_wb_connector_state *wb_state,
		struct drm_framebuffer *fb);

/**
 * dpu_wfd_sink_cmdlist_append - append wfd sink video slice cmdlist nodes
 * @plane: drm_plane pointor
 * @frame_id: id of cmdlist frame
 */
bool dpu_wfd_sink_cmdlist_append(struct drm_plane *plane, s64 frame_id);

/**
 * dpu_wfd_sink_get_slice_info - get slice info from dpu_plane_state
 * @layer_cfg: rdma_layer_cfg pointor
 * @plane_state: drm_plane_state pointor
 * @plane: drm_plane pointor
 */
void dpu_wfd_sink_get_slice_info(struct rdma_layer_cfg *layer_cfg,
		struct drm_plane_state *plane_state,
		struct drm_plane *plane);

/**
 * dpu_wfd_sink_update_iova - use wfd reserved iova if wfd enable
 * @iova: pointer of framebuffer iova
 * @wfd_slice_en: wfd enable
 */
void dpu_wfd_sink_update_iova(u64 *iova, bool wfd_slice_en);

/**
 * dpu_wfd_sink_notify_vsync - notify vsync to wfd sink
 */
void dpu_wfd_notify_event(enum xplayer_event ev);

/**
 * dpu_wfd_sink_cmdlist_init - init wfd sink cmdlist
 */
void dpu_wfd_sink_cmdlist_init(void);

/**
 * dpu_wfd_sink_cmdlist_deinit - deinit wfd sink cmdlist
 */
void dpu_wfd_sink_cmdlist_deinit(void);

#endif
