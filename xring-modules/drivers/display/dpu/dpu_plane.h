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

#ifndef _DPU_PLANE_H_
#define _DPU_PLANE_H_

#include <linux/mutex.h>
#include <drm/drm_atomic.h>
#include <drm/drm_plane.h>
#include <display/xring_dpu_drm.h>
#include "dpu_hw_rch.h"
#include "dpu_pre_color.h"
#include "display/xring_stripe_common.h"

/**
 * dpu_plane_color_blks - the structure of dpu plane color blocks
 * @scaler_id: scaler_id that gets from rch id
 * @lut3d_id: lut3d_id that gets from rch id
 * @hist_id: hist_id that gets from rch id
 */
struct dpu_plane_color_blks {
	int scaler_id;
	int lut3d_id;
	int hist_id;
};

/**
 * dpu_plane - the structure of dpu plane
 * @base: the drm plane
 * @hw_rch: the hardware handle of rch
 * @hw_rch_top: the hardware handle of rch_top
 * @mutex: the mutex lock for plane
 * @rch_id_prop: the drm property pointer of rch_id
 * @sram_prop: the drm property pointer of rch sram mem
 * @extender_mode_prop: the drm property pointer of layer type
 * @roi_prop: the drm property pointer of exclusive roi
 * @wfd_sink_slice_prop: the drm property pointer of WFD sink slice num
 * @acquire_fence_fd_prop: the drm property pointer of acquire fence fd
 * @axi_prop: the drm property pointer of axi port
 * @sharefd_prop: sharefd propperty pointer for each plane dmabuf
 * @stripe_count_prop: pointer of stripe count property
 * @stripe_para_blob_prop: pointer of wb stripe para blob property
 * @stripe_para_blob_size: plane stripe blob size property
 * @protected_buffer_status_prop: plane protected buffer property
 * @plane_slice_node_id: the array for saveing node id in stripe cmdlist config scence
 * @color: the plane color(prepq) pointer
 * @pre_rch_id: init rch id with first frame
 * @pre_zpos: init zpos with first frame
 */
struct dpu_plane {
	struct drm_plane base;

	struct dpu_hw_rch *hw_rch;
	struct dpu_hw_rch_top *hw_rch_top;
	struct mutex mutex;

	struct drm_property *rch_id_prop;
	struct drm_property *sram_prop;
	struct drm_property *extender_mode_prop;
	struct drm_property *roi_prop;
	struct drm_property *wfd_sink_slice_prop;
	struct drm_property *acquire_fence_fd_prop;
	struct drm_property *axi_prop;
	struct drm_property *sharefd_prop;

	struct drm_property *stripe_count_prop;
	struct drm_property *stripe_para_blob_prop;
	u32 stripe_para_blob_size;

	struct drm_property *protected_buffer_status_prop;

	s64 plane_slice_node_id[MAX_STRIPE_COUNT];

	struct plane_color *color;

	u8 pre_rch_id;
	u8 pre_zpos;
};

#define to_dpu_plane(x) container_of(x, struct dpu_plane, base)

/**
 * dpu_plane_state - the state structure of dpu plane
 * @base: the drm plane
 * @rch_id: the id of rch
 * @auto_rch_id: the id of rch which is reserved by res_mgr automatically
 * @extender_mode: the layer extender mode, left_right or up_down mode
 * @sram_param: the rch sram param, sw, base_addr, left, right and total size
 * @color_state: the plane color state
 * @roi_cfg: the plane roi config
 * @fence: the acquire fence pointer
 * @is_protected_buffer: the plane is protected buffer or not
 * @stripe_para_blob_id: plane stripe para blob id
 * @stripe_para_blob_ptr: pointer of plane stripe para
 * @rch_node_id: cmdlist node id for plane rdma
 * @color_node_id: cmdlist node id for plane pq
 * @slice_cnt: WFD sink slice count
 * @axi_id: the id of axi
 */
struct dpu_plane_state {
	struct drm_plane_state base;

	u32 rch_id;
	u8 extender_mode;

	struct dpu_sram_param sram_param;
	struct plane_color_state color_state;
	struct dpu_plane_roi roi_cfg;
	struct dma_fence *fence;
	s64 sharefd;

	u8 is_protected_buffer;
	u64 stripe_para_blob_id;
	struct drm_property_blob *stripe_para_blob_ptr;

	s64 rch_node_id;
	s64 color_node_id;

	u8 slice_cnt;
	u32 axi_id;
};

#define to_dpu_plane_state(x) \
	container_of(x, struct dpu_plane_state, base)

/**
 * dpu_plane_init - init the dpu plane
 * @drm_dev: the pointer of drm device
 * @possible_crtcs: possible crtcs for plane
 * @plane_type: the type of plane
 *
 * Return: valid pointer on success, invalid pointer on failure
 */
struct drm_plane *dpu_plane_init(struct drm_device *drm_dev,
		u32 possible_crtcs, enum drm_plane_type plane_type);

static inline u32 _get_rch_mask(struct drm_atomic_state *state,
		u32 plane_mask, bool new_state)
{
	struct drm_plane_state *old_plane_state, *new_plane_state;
	struct drm_plane *plane;
	struct dpu_plane_state *dpu_pstate;
	u32 rch_mask = 0;
	int i;

	for_each_oldnew_plane_in_state(state, plane, old_plane_state,
			new_plane_state, i) {
		if (drm_plane_mask(plane) & plane_mask) {
			if (new_state)
				dpu_pstate = to_dpu_plane_state(new_plane_state);
			else
				dpu_pstate = to_dpu_plane_state(old_plane_state);

			rch_mask |= BIT(dpu_pstate->rch_id);
		}
	}

	return rch_mask;
}

#endif /* _DPU_PLANE_H_ */
