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

#ifndef _VENC_POWER_MANAGER_H_
#define _VENC_POWER_MANAGER_H_

#include "venc_power_common.h"
#include "vcx_vcmd_priv.h"

int hantroenc_get_vcmdmgr(struct device *dev, vcmd_mgr_t** vcmd_mgr);

venc_power_mgr *hantroenc_power_mgr_init(struct device *dev);

void hantroenc_power_mgr_deinit(venc_power_mgr *power_mgr);

int hantroenc_pm_hw_init(struct device *dev);

int hantroenc_power_on(venc_power_mgr *pm);

void hantroenc_power_off(venc_power_mgr *pm);

void hantroenc_disable_tcu(struct device *dev);

void hantroenc_disable_tbu(struct device *dev);

#endif