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

#ifndef _VDEC_POWER_BASE_H_
#define _VDEC_POWER_BASE_H_

#include "vdec_power_common.h"
#include "vdec_log.h"

static inline unsigned long vdec_get_default_rate(vdec_clock *vdec_clk) {
    if (!vdec_clk) {
        vdec_pm_klog(LOGLVL_ERROR, "invalid parameter vdec_clk\n");
        return 0;
    }

    return vdec_clk->default_rate;
}

static inline unsigned long vdec_get_work_rate(vdec_clock *vdec_clk) {
    if (!vdec_clk) {
        vdec_pm_klog(LOGLVL_ERROR, "invalid parameter vdec_clk\n");
        return 0;
    }

    return vdec_clk->work_rate;
}

int vdec_power_on(vdec_power_mgr *power_mgr, vdec_power_domain_e pd);

void vdec_power_off(vdec_power_mgr *power_mgr, vdec_power_domain_e pd);

int vdec_clk_enable(vdec_power_mgr* power_mgr);

void vdec_clk_disable(vdec_power_mgr* power_mgr);

void vdec_clk_rate_config(vdec_power_mgr* power_mgr, unsigned long rate);

void vdec_force_power_off(vdec_power_mgr *power_mgr, vdec_power_domain_e pd);

int vdec_force_power_on(vdec_power_mgr *power_mgr, vdec_power_domain_e pd);

#endif