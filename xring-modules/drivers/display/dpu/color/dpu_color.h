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

#ifndef _DPU_COLOR_H_
#define _DPU_COLOR_H_

#include "dpu_color_internal.h"
#include "dpu_pre_color.h"
#include "dpu_post_color.h"

/**
 * dpu_color - the global private dpu color instance
 * @dev: pointer to drm device
 * @pre_color: the prepq instance
 */
struct dpu_color {
	struct drm_device *dev;
	struct dpu_pre_color pre_color;
};

int dpu_color_pre_hist_event_init(struct drm_device *dev, void *event);

/**
 * dpu_color_init - create the global dpu color
 */
int dpu_color_init(struct dpu_color **color, struct drm_device *dev);

/**
 * dpu_color_deinit - destroy the global dpu color
 */
void dpu_color_deinit(struct dpu_color *color);

#endif /* _DPU_PRE_COLOR_H_ */
