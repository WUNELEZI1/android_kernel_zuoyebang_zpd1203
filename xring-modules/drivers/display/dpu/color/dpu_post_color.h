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

#ifndef _DPU_POST_COLOR_H_
#define _DPU_POST_COLOR_H_

#include <drm/drm_crtc.h>
#include "dpu_color_internal.h"
#include "dpu_hw_post_gamma.h"
#include "dpu_hw_post_3dlut.h"
#include "dpu_hw_postpq_top.h"
#include "dpu_hw_postpre_proc.h"
#include "dpu_hw_post_acad.h"
#include "dpu_hw_post_hist.h"
#include "dpu_hw_post_scaler.h"

/* callback functions for registering the postPQ module to CRTC */
struct color_postpq_funcs {
	/**
	 * create_properties - create properties for all of the post PQ modules
	 * @crtc: pointer to drm_crtc
	 *
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*create_properties)(struct drm_crtc *crtc);

	/**
	 * set_property - set a post pq property
	 * @crtc: pointer to drm_crtc
	 * @state: pointer to drm_crtc
	 * @property: property that needs to be set
	 * @val: value of property
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*set_property)(struct drm_crtc *crtc,
			struct drm_crtc_state *state,
			struct drm_property *property, uint64_t val);

	/**
	 * get_property - get a property of postPQ
	 * @crtc: pointer to drm_crtc
	 * @property: property that configured before
	 * @val: value of property
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*get_property)(struct drm_crtc *crtc,
			struct drm_property *property, uint64_t *val);

	/**
	 * check_properties - verify the properties for postPQ
	 * @crtc: pointer to drm_crtc
	 *
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*check_properties)(struct drm_crtc *crtc,
			struct drm_crtc_state *state);

	/**
	 * destroy_state - clear state pq info in the state
	 * @crtc: pointer to drm_crtc
	 * @state: pointer to drm_crtc_state
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*destroy_state)(struct drm_crtc *crtc,
			struct drm_crtc_state *state);

	/**
	 * commit - commit properties to HW modules
	 * @crtc: pointer to drm_crtc
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*commit)(struct drm_crtc *crtc);

	/**
	 * pipe_init - init post pq pipe
	 * @crtc: pointer to drm_crtc
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*pipe_init)(struct drm_crtc *crtc);

	/**
	 * hw_restore - restore post pq pipe cfg
	 * @crtc: pointer to drm_crtc
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*hw_restore)(struct drm_crtc *crtc);

	/**
	 * status_dump - postpq hw module status dump
	 * @crtc: pointer to drm_crtc
	 */
	void (*status_dump)(struct drm_crtc *crtc);

	/**
	 * event_init - postpq event init
	 * @crtc: pointer to drm_crtc
	 * @event: pointer to dpu crtc event
	 */
	int (*event_init)(struct drm_crtc *crtc, void *event);

	/**
	 * clear - postpq property value clear
	 * @crtc: pointer to drm_crtc
	 * @state: pointer to drm_crtc_state
	 */
	int (*clear)(struct drm_crtc *crtc, struct drm_crtc_state *state);

	/**
	 * ping_pong_reset - reset ping_pong status in power down(such as enter cmd idle)
	 * @crtc: pointer to drm_crtc
	 */
	void (*ping_pong_reset)(struct drm_crtc *crtc);
};

struct pingpong_tag {
	bool pong;
	bool switched;
};

/**
 * color_postpq - the postpq color data structure
 * @hw_gamma: point to post gamma object
 * @hw_3dlut: point to post 3dlut object
 * @hw_pq_top: point to post pq_top object
 * @hw_pre_proc: point to post pre proc object
 * @hw_acad: point to acad object
 * @hw_hist: point to post hist object
 * @hw_scaler: point to post scaler object
 * @feature_list: post pq feature list
 * @dirty_list: dirty feature list
 * @history_list: store feature list
 * @enabled_features: enabled feature
 * @rc_region: rc region
 * @funcs: the callback functions for postpq
 * @lut3d_tag: 3dlut ping pong status record
 * @usrgma_tag:user gamma ping pong status record
 */
struct color_postpq {
	struct dpu_hw_post_gamma *hw_gamma;
	struct dpu_hw_post_3dlut *hw_3dlut;
	struct dpu_hw_postpq_top *hw_pq_top;
	struct dpu_hw_postpre_proc *hw_pre_proc;
	struct dpu_hw_post_acad *hw_acad;
	struct dpu_hw_post_hist *hw_hist;
	struct dpu_hw_post_scaler *hw_scaler;

	struct list_head feature_list;
	struct list_head dirty_list;
	struct list_head history_list;

	u32 enabled_features;
	struct dpu_region rc_region;
	struct color_postpq_funcs *funcs;

	struct color_hist_event hist_event;
	struct pingpong_tag lut3d_tag;
	struct pingpong_tag usrgma_tag;
};

/**
 * color_postpq_prop - the postpq property data structure
 * @flags: drm property flags
 * @value: property value
 * @blob_ptr: point to blob property
 */
struct color_postpq_prop {
	uint32_t flags;
	uint64_t value;
	void *blob_ptr;
};

/**
 * color_postpq_state - the postpq state data structure
 * @color_postpq_prop: post pq feature property
 * @dirty: dirt flag for each feature
 */
struct color_postpq_state {
	struct color_postpq_prop prop[DPU_CRTC_POSTPQ_FEATURE_MAX];
	u32 dirty[DPU_CRTC_POSTPQ_FEATURE_MAX];
};

/**
 * dpu_crtc_color_postpq_init - create the post-color instance for each crtc
 */
int dpu_crtc_color_postpq_init(struct drm_crtc *crtc);

/**
 * dpu_crtc_postpq_deinit - destroy the post-color instance
 */
void dpu_crtc_color_postpq_deinit(struct drm_crtc *crtc);

#endif /* _DPU_POST_COLOR_H_ */
