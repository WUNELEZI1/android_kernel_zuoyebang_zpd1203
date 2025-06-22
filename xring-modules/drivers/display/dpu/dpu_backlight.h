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

#ifndef _DPU_BACKLIGHT_H_
#define _DPU_BACKLIGHT_H_

#include <drm/drm_connector.h>
#include "dsi_display.h"

/**
 * dpu_backlight_init - initialize the backlight for display
 * @display: the dsi display structure pointer
 * @dev: the drm_device structure pointer
 * @drm_dsi_conn: the drm_connector structure pointer
 * @return returns 0 on success, a negative error code on failure.
 */
int dpu_backlight_init(struct dsi_display *display, struct drm_device *dev,
		struct drm_connector *drm_dsi_conn);

/**
 * dpu_backlight_deinit - deinitialize the backlight for display
 * @display: the dsi display structure pointer
 */
void dpu_backlight_deinit(struct dsi_display *display);

/**
 * dpu_backlight_parse_bl_info - parse backlight info from dts
 * @np: dsi_ctrl structure pointer
 * @config: config structure pointer
 */
int dpu_backlight_parse_bl_info(struct dsi_backlight_config *config,
		struct device_node *np);

#endif
