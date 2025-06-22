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

#ifndef _DSI_POWER_H_
#define _DSI_POWER_H_

#include "dsi_encoder.h"
#include "dsi_display.h"
#include <linux/clk.h>

#define F1_CLK_DSI_SYS_208_M_9  (208900000UL)

struct dsi_power_mgr {
	u32 count;
	struct mutex lock;
	struct clk *clk_dsi_sys;
	struct clk *clk_dpu_pll;
	struct dsi_display *display;
};

/**
 * dsi_power_get - power on dsi
 * @connector: the drm connector pointer for power control
 */
void dsi_power_get(struct drm_connector *connector);

/**
 * dsi_power_prepare_put - prepare dsi before do power put operation
 * @connector: the drm connector pointer for power control
 */
void dsi_power_prepare_put(struct drm_connector *connector);

/**
 * dsi_power_put - power off dsi
 * @connector: the drm connector pointer for power control
 */
bool dsi_power_put(struct drm_connector *connector);

/**
 * is_dsi_powered_on - whether the dsi&ddic is powered on
 *
 * Return: true on powered on, false on powered off
 */
bool is_dsi_powered_on(void);

/**
 * dsi_power_mgr_init - init dsi power mgr
 * @display: the phandle of dsi display
 *
 * Returns: 0: success, other values failure
 */
int dsi_power_mgr_init(struct dsi_display *display);

/**
 * dsi_power_mgr_deinit - deinit dsi power mgr
 */
void dsi_power_mgr_deinit(void);

/**
 * dsi_ipi_enable - dsi dpu ipi clk on.
 * @port: select dsi0 or dsi1
 */
void dsi_ipi_enable(u8 port);

/**
 * dsi_ipi_disable - dsi dpu ipi clk off.
 * @port: select dsi0 or dsi1
 */
void dsi_ipi_disable(u8 port);

/**
 * tmg_ipi_enable - dsi dpu ipi clk on.
 * @port: select dsi0 or dsi1
 */
void tmg_ipi_enable(u8 port);

/**
 * tmg_ipi_disable - dsi dpu ipi clk off.
 * @port: select dsi0 or dsi1
 */
void tmg_ipi_disable(u8 port);

#endif
