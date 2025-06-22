// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _VDEC_POWER_MANAGER_H_
#define _VDEC_POWER_MANAGER_H_

#include "vdec_power_common.h"
#include "hantrovcmd_priv.h"

vdec_power_mgr *vdec_power_mgr_init(struct device *dev);

void vdec_power_mgr_deinit(vdec_power_mgr *power_mgr);

int vdec_subsys_hw_init(vdec_power_mgr *pm);

void vdec_subsys_hw_deinit(vdec_power_mgr *pm);

int vdec_core_do_power_on(vdec_power_mgr *pm);

void vdec_core_do_power_off(vdec_power_mgr *pm);

void hantrodec_pm_runtime_get(struct device *dev, vdec_power_mgr *pm);

void hantrodec_pm_runtime_put(struct device *dev);

int vdec_core_hw_config(vdec_power_mgr *pm);

#endif