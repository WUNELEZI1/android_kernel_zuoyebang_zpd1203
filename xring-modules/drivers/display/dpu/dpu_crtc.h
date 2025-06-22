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

#ifndef _DPU_CRTC_H_
#define _DPU_CRTC_H_

#include <drm/drm_crtc.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>
#include <display/xring_dpu_drm.h>
#include "display/xring_stripe_common.h"
#include "dpu_cap.h"
#include "dpu_hw_mixer.h"
#include "dpu_hw_dsc.h"
#include "dpu_post_color.h"
#include "dpu_core_perf.h"

#define ARGB_COMPONENT_B_SHIFT 0
#define ARGB_COMPONENT_B_LENGTH 10
#define ARGB_COMPONENT_G_SHIFT 10
#define ARGB_COMPONENT_G_LENGTH 10
#define ARGB_COMPONENT_R_SHIFT 20
#define ARGB_COMPONENT_R_LENGTH 10
#define ARGB_COMPONENT_A_SHIFT 30
#define ARGB_COMPONENT_A_LENGTH 8

#define ARGB_TO_COMPONENT_X(argb, offset, len) \
		(((argb) & (BITS_MASK(offset, len))) >> (offset))

/**
 *  @argb: A[30:37], R[20:29], G[10:19], B[0:9]
 */
#define ARGB_TO_COMPONENT_B(argb) ARGB_TO_COMPONENT_X(argb, \
		ARGB_COMPONENT_B_SHIFT, ARGB_COMPONENT_B_LENGTH)
#define ARGB_TO_COMPONENT_G(argb) ARGB_TO_COMPONENT_X(argb, \
		ARGB_COMPONENT_G_SHIFT, ARGB_COMPONENT_G_LENGTH)
#define ARGB_TO_COMPONENT_R(argb) ARGB_TO_COMPONENT_X(argb, \
		ARGB_COMPONENT_R_SHIFT, ARGB_COMPONENT_R_LENGTH)
#define ARGB_TO_COMPONENT_A(argb) ARGB_TO_COMPONENT_X(argb, \
		ARGB_COMPONENT_A_SHIFT, ARGB_COMPONENT_A_LENGTH)

/* HWC zorder 0 is the bottom layer, KMS zorder 0 is the top layer */
#define TO_HW_ZORDER(zpos) (MAX_PLANE_ZPOS - (zpos))

/* Check if the layer exceeds the boundary */
#define CHECK_LAYER_BOUNDS(offset, size, max_size) \
		(((size) > (max_size)) || ((offset) > ((max_size) - (size))))

/**
 * dpu_crtc_for_each_solid_layer - Traverse all solid layers
 * @solid_layer: struct dpu_solid_layer
 * @i: u8 type constant
 */
#define dpu_crtc_for_each_solid_layer(solid_layer, i) \
		for (i = 0; i < (solid_layer).count; i++)

enum dpu_crtc_type {
	DPU_CRTC_NONE = 0,
	DPU_CRTC_PRIMARY,
	DPU_CRTC_SECONDARY,
	DPU_CRTC_THIRD,
};

/**
 * dpu_crtc_event_work - interrupt event work data structure
 * @work: for interrupt work thread
 * @event_type: crtc event type
 * @dpu_crtc: point to dpu crtc
 * @cb: call back func pointer
 */
struct dpu_crtc_event_work {
	struct work_struct work;
	u32 event_type;
	void *dpu_crtc;
	void (*cb)(struct drm_crtc *crtc);
};

/**
 * dpu_crtc_event - dpu crtc events
 * @event_type: crtc event type
 * @intr_id: interrupt id
 * @enable: event enable func pointer
 * @cb: event call back func pointer
 * @head: head in crtc event list
 */
struct dpu_crtc_event {
	u32 event_type;
	int intr_id;
	int (*enable)(struct drm_crtc *crtc, bool enable, void *event);
	int (*cb)(struct drm_crtc *crtc);
	struct list_head head;
};

/**
 * dpu_crtc - the dpu crtc struture
 * @base: the drm crtc
 * @hw_mixer: the hardware handle of mixer
 * @hw_post_pipe_top: the hardware handle of post_pipe_top
 * @hw_dsc: the hardware handle of dsc
 * @dsc_cfg: dsc config parameters from panel
 * @type: current type of this crtc
 * @bg_color_prop: the pointer of background color property
 * @lm_width_prop: the layer mixer width property
 * @lm_height_prop: the layer mixer height property
 * @damage_size_prop: screen damage size property
 * @solid_layer_prop: the pointer of solid layer property
 * @type_prop: the pointer of mixer type property
 * @present_fence_ptr_prop: the pointer of present fence fd property
 * @release_fence_ptr_prop: the pointer of release fence fd property
 * @frame_number_prop: the pointer of frame number property
 * @keep_power_on_prop: the pointer of keep power on property
 * @postpq: the pointer to color postpq
 * @event_list: user event list
 * @event_lock: lock for user event list
 * @event_wq: workqueue for crtc events
 * @print_commit_info_cnt: used to count the number of printing commit info
 * @hw_ulps_enable: hw ulps status
 * @underflow_frame_no: the frame no has underflow
 */
struct dpu_crtc {
	struct drm_crtc base;

	struct dpu_hw_mixer *hw_mixer;
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct dpu_hw_dsc *hw_dsc;
	struct dpu_dsc_config dsc_cfg;
	enum dpu_crtc_type type;

	struct drm_property *bg_color_prop;
	struct drm_property *lm_height_prop;
	struct drm_property *lm_width_prop;
	struct drm_property *damage_size_prop;
	struct drm_property *solid_layer_prop;
	struct drm_property *type_prop;
	struct drm_property *present_fence_ptr_prop;
	struct drm_property *release_fence_ptr_prop;
	struct drm_property *frame_number_prop;
	struct drm_property *keep_power_on_prop;

	struct color_postpq *postpq;
	bool need_postpq_restore;

	struct list_head event_list;
	spinlock_t event_lock;
	struct workqueue_struct *event_wq;
	u32 print_commit_info_cnt;
	atomic64_t vsync_timestamp;

	bool hw_ulps_enable;
	u64 underflow_frame_no;
};

#define to_dpu_crtc(x) container_of(x, struct dpu_crtc, base)

/**
 * dpu_crtc_state - the dpu crtc state structure
 * @base: the drm crtc state
 * @postpq_state: the post pq state data structure
 * @solid_layer: the solid layer data
 * @bg_color: the background color value, A[37:30], R[29:20], G[19:10], B[9:0]
 * @lm_width: the layer mixer output width
 * @lm_height: the layer mixer output height
 * @frame_no: frame number from hw composer
 * @type: the current crtc type
 * @present_fence_ptr: the fd addr for present fence
 * @release_fence_ptr: the fd addr for release fence
 * @cmps_slice_node_id: the array for saveing node id in stripe cmdlist config scence
 * @perf_params: the crtc's perf params
 * @node_id: cmdlist node id for crtc
 * @mixer_need_update: mixer need update
 * @keep_power_on: ctrl frame power feature
 */
struct dpu_crtc_state {
	struct drm_crtc_state base;

	struct color_postpq_state postpq_state;

	struct dpu_solid_layer solid_layer;
	u64 bg_color;
	u16 lm_height;
	u16 lm_width;
	u16 damage_x;
	u16 damage_y;
	u16 damage_width;
	u16 damage_height;
	u64 frame_no;
	enum dpu_crtc_type type;
	u64 present_fence_ptr;
	u64 release_fence_ptr;

	s64 cmps_slice_node_id[MAX_STRIPE_COUNT];
	s64 node_id;

	u32 mixer_need_update;
	u32 keep_power_on;
};

#define to_dpu_crtc_state(x) container_of(x, struct dpu_crtc_state, base)

/**
 * dpu_crtc_event_register - register crtc events
 * @crtc: the pointer of drm crtc
 * @func: call back func for intr thread
 * Return: 0 on success, error number on failure
 */
void dpu_crtc_intr_event_queue(struct drm_crtc *crtc,
		void (*func)(struct drm_crtc *crtc));

/**
 * dpu_crtc_event_register - register crtc events
 * @crtc: the pointer of drm crtc
 * @event_type: event in enum dpu_drm_event_type
 * @enable: true for enable event, or false fo disable event
 * Return: 0 on success, error number on failure
 */
int dpu_crtc_event_enable(struct drm_crtc *crtc,
		u32 event_type, bool enable);

/**
 * dpu_crtc_handle_events - handle crtc intr events
 * @crtc: pointer of drm_crtc
 * @intr_id: dpu interrupt id
 */
int dpu_crtc_handle_events(struct drm_crtc *crtc, u32 intr_id);

/**
 * dpu_crtc_init - init dpu crtc
 * @drm_dev: the pointer of drm device
 * @primary: the pointer of primary plane for crtc
 *
 * Return: valid pointer on success, error pointer on failure
 */
struct drm_crtc *dpu_crtc_init(struct drm_device *drm_dev,
		struct drm_plane *primary);

/**
 * dpu_crtc_tpg_fill_color - fill pure color for crtc
 * @crtc: pointer of drm_crtc
 * @color: color for fill
 * @enable: fill or clear the color
 */
void dpu_crtc_tpg_fill_color(struct drm_crtc *crtc, u32 color, bool enable);

/**
 * is_primary_display - check whether current crtc is primary display or not
 * @new_state: the pointer for drm_crtc_state
 *
 * Return true for primary display, false for other display
 */
bool is_primary_display(struct drm_crtc_state *new_state);

/**
 * need_mode_switch - check whether current crtc is need switch mode
 * @new_state: the pointer for drm_crtc_state
 *
 */
static inline bool need_mode_switch(struct drm_crtc_state *new_state)
{
	return new_state->active && new_state->mode_changed;
}

/**
 * get_crtc_size - get mixer output size
 * @dpu_cstate: the dpu crtc state structure
 * @crtc_width: mixer output width
 * @crtc_height: mixer output height
 */
void get_crtc_size(struct dpu_crtc_state *dpu_cstate, u32 *crtc_width,
		u32 *crtc_height);

/**
 * dpu_crtc_hw_state_init - init hw state of crtc
 *
 * @crtc: dpu_crtc_hw_state_init
 */
void dpu_crtc_hw_state_init(struct drm_crtc *crtc);

u64 get_calibrated_vsync_timestamp(struct drm_device *dev);

#endif /* _DPU_CRTC_H_ */
