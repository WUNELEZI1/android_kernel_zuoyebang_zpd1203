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

#ifndef _DPU_CONT_DISPLAY_H_
#define _DPU_CONT_DISPLAY_H_

#include <linux/slab.h>
#include <drm/drm_device.h>

#include "dpu_kms.h"

/**
 * dpu_cont_display_init - Initialize the resources required to control kernel startup
 * @drm_dev: drm device
 */
int dpu_cont_display_init(struct drm_device *drm_dev);

/**
 * is_cont_display_enabled - enable continuous display flag
 */
bool is_cont_display_enabled(void);

/**
 * dpu_cont_display_res_release - release continuous display resource
 *
 * Display resource includes logo resource release and power manager.
 * @power_mgr: pointer of dpu_power_mgr
 */
void dpu_cont_display_res_release(struct dpu_power_mgr *power_mgr);

/**
 * dpu_cont_display_res_force_release - force release cont display resource
 * used for lastcolse and linux SR scene
 */
void dpu_cont_display_res_force_release(struct dpu_power_mgr *power_mgr);

/**
 * dpu_cont_display_status_get - get continus display status from dtsi
 *
 * @drm_dev: pointer of drm_dev
 */
void dpu_cont_display_status_get(struct drm_device *drm_dev);

#endif
