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

#ifndef _DPU_PRE_COLOR_H_
#define _DPU_PRE_COLOR_H_

#include <drm/drm_plane.h>
#include "dpu_color_internal.h"
#include "dpu_hw_prepipe_top.h"
#include "dpu_hw_pre_scaler.h"
#include "dpu_hw_tm.h"
#include "dpu_hw_pre_hist.h"
#include "dpu_hw_pre_3dlut.h"

/**
 * dpu_pre_color - the global private dpu pre color instance
 * @hist_event: the prepq hist event instance
 * @hw_hist: hw block of pre histogram in RCH-V0
 */
struct dpu_pre_color {
	struct color_hist_event hist_event;
	struct dpu_hw_pre_hist *hw_hist;
};

/* callback functions for registering the pre PQ modules to plane */
struct plane_color_funcs {
	/**
	 * create_properties - create properties for all of the post PQ modules
	 * @plane: pointer to drm_plane
	 *
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*create_properties)(struct drm_plane *plane);

	/**
	 * set_property - set a post pq property
	 * @plane: pointer to drm_plane
	 * @state: pointer to drm_plane_state
	 * @property: property that needs to be set
	 * @val: value of property
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*set_property)(struct drm_plane *plane,
			struct drm_plane_state *state,
			struct drm_property *property, uint64_t val);

	/**
	 * set_property - set a post pq property
	 * @plane: pointer to drm_plane
	 * @state: pointer to drm_plane_state
	 * @property: property that needs to be set
	 * @val: value of property
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*get_property)(struct drm_plane *plane,
		const struct drm_plane_state *state,
		struct drm_property *property, u64 *val);

	/**
	 * check_properties - verify the properties for prepq
	 * @plane: pointer to drm_plane
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*check_properties)(struct drm_plane *plane,
			struct drm_plane_state *state);

	/**
	 * duplicate_state - dpulicate the properties for prepq
	 * @new_state: pointer to new drm_plane_state
	 * @old_state: pointer to old drm_plane_state
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*duplicate_state)(struct drm_plane_state *new_state,
			struct drm_plane_state *old_state);

	/**
	 * destroy_state - dpulicate the properties for prepq
	 * @plane: pointer to drm_plane
	 * @state: ointer to drm_plane_state
	 * Return: zero on success, -ERRNO on failure
	 */
	void (*destroy_state)(struct drm_plane *plane,
			struct drm_plane_state *state);

	/**
	 * commit - commit properties to HW modules
	 * @plane: pointer to drm_plane
	 * Return: zero on success, -ERRNO on failure
	 */
	int (*commit)(struct drm_plane *plane);
};

/**
 * plane_color - the prepq color data structure
 * @property: prepq property array
 * @hw_prepipe_top: pointer to the hw module of prepipe top
 * @hw_scaler: pointer to the hw module of pre scaler
 * @hw_tm: pointer to the hw module of tm
 * @hw_hist: pointer to the hw module of histogram
 * @enabled_features: record all the features enabled
 * @funcs: the callback functions for prepq
 * @color_private: pointer to the global dpu_color instance
 */
struct plane_color {
	struct drm_property *property[PLANE_COLOR_PROP_MAX];

	struct dpu_hw_prepipe_top *hw_prepipe_top;
	struct dpu_hw_pre_scaler *hw_scaler;
	struct dpu_hw_tm *hw_tm;
	struct dpu_hw_pre_hist *hw_hist;
	struct dpu_hw_pre_3dlut *hw_3dlut;
	u32 enabled_features;

	void *color_private;

	struct plane_color_funcs *funcs;
};

/**
 * struct prepq_property_value - the member of property value array
 *                               for each drm object
 * @value: property value
 * @blob: Pointer to blob property, if available
 */
struct prepq_property_value {
	uint64_t value;
	struct drm_property_blob *blob;
};

struct plane_color_state {
	struct prepq_property_value prop_value[PLANE_COLOR_PROP_MAX];
};

void dpu_pre_color_hist_get(struct drm_device *dev,
		struct dpu_pre_color *pre_color);

/**
 * dpu_pre_color_init - create the pre-color instance for each plane
 */
int dpu_pre_color_init(struct drm_plane *plane);

/**
 * dpu_pre_color_deinit - destroy the pre-color instance
 */
void dpu_pre_color_deinit(struct drm_plane *plane);

#endif /* _DPU_PRE_COLOR_H_ */
