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

#ifndef _VENC_POWER_BASE_H_
#define _VENC_POWER_BASE_H_

#include "venc_power_common.h"


static inline unsigned long venc_get_default_rate(venc_clock *venc_clk) {
	if (!venc_clk) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter venc_clk\n");
		return 0;
	}
	return venc_clk->default_rate;
}

static inline unsigned long venc_get_work_rate(venc_clock *venc_clk) {
	if (!venc_clk) {
		venc_pm_klog(LOGLVL_ERROR, "invalid parameter venc_clk\n");
		return 0;
	}
	return venc_clk->work_rate;
}

int venc_clk_enable(venc_power_mgr* power_mgr);

void venc_clk_disable(venc_power_mgr* power_mgr);

void venc_clk_rate_config(venc_power_mgr* power_mgr, unsigned long rate);

void venc_clk_deinit(venc_clock *venc_clk);

int venc_clk_init(venc_power_mgr *pm);

#endif