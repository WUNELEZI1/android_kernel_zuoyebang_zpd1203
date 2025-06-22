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

#ifndef _DPU_CMDLIST_H_
#define _DPU_CMDLIST_H_

#include <linux/types.h>
#include <drm/drm_crtc.h>
#include "dksm_mem_pool.h"
#include "dpu_plane.h"

/**
 * dpu_hw_cmdlist_enable - control cmdlist ch enable or disable
 *
 * @dpu_kms: point of dpu_kms
 * @channel_mask: bit mask of cmdlist channel
 * @enable: true or false
 */
void dpu_hw_cmdlist_enable(struct dpu_kms *dpu_kms, u32 channel_mask, bool enable);

/**
 * dpu_cmdlist_prepare - create new cmdlist_frame, create cmdlist_node for each
 * plane and crtc
 * @crtc: drm_crtc
 * @crtc_state: drm crtc_state
 */
void dpu_cmdlist_prepare(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state);
/**
 * dpu_cmdlist_commit - add all cmdlist_node to cmdlist_frame
 * @state: the drm atomic state
 * @crtc: drm_crtc
 * @old_state: the old crtc state
 * @new_state: the new crtc state
 */
void dpu_cmdlist_commit(struct drm_atomic_state *state,
		struct drm_crtc *crtc,
		struct drm_crtc_state *old_state,
		struct drm_crtc_state *new_state);

/**
 * dpu_cmdlist_plane_node_update - if un down reuse, update hw_blk->cmd_node
 * @dpu_plane: the structure pointer of dpu plane
 */
void dpu_cmdlist_plane_node_update(struct dpu_plane *dpu_plane);

/**
 * dpu_cmdlist_init - init cmdlist module
 * @drm_dev: drm device
 * @mem_pool: pointer of dksm_mem_pool
 */
int dpu_cmdlist_init(struct drm_device *drm_dev, struct dksm_mem_pool *mem_pool);

/**
 * dpu_cmdlist_deinit - init cmdlist module
 */
void dpu_cmdlist_deinit(void);

static inline struct drm_connector *find_connector(struct drm_device *drm_dev,
		u32 connector_mask, int conn_type)
{
	struct list_head *connector_list;
	struct drm_connector *connector;

	connector_list = &drm_dev->mode_config.connector_list;

	list_for_each_entry(connector, connector_list, head) {
		if (((connector_mask) & drm_connector_mask(connector)) &&
				connector->connector_type == conn_type)
			return connector;
	}

	return NULL;
}

#endif
