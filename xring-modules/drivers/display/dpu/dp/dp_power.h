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

#ifndef _DP_POWER_H_
#define _DP_POWER_H_

#include "dp_hw_hss1.h"
#include "dp_hw_sctrl.h"
#include "dp_hw_ctrl.h"

struct dp_power;
struct dp_display;

enum dp_power_state {
	DP_POWER_STATE_OFF,
	DP_POWER_STATE_ON,
};

enum hdcp_smc_id {
	SMC_ID_NONE,
	SMC_ID_POWER_ON,
	SMC_ID_POWER_OFF,
	SMC_ID_MAX,
};

struct dp_power_funcs {
	/**
	 * on - dptx power on
	 * @power: handle of dp power
	 */
	int (*on)(struct dp_power *power);

	/**
	 * off - dptx power off
	 * @power: handle of dp power
	 */
	void (*off)(struct dp_power *power);

	/**
	 * lpm_enable - dptx lower power mode enable
	 * @power: handle of dp power
	 *
	 * close aux, phy pll, set lane powermode to p3
	 */
	void (*lpm_enable)(struct dp_power *power);

	/**
	 * lpm_disable - dptx lower power mode disable
	 * @power: handle of dp power
	 *
	 * enable aux, phy pll, set lane powermode to p0
	 */
	void (*lpm_disable)(struct dp_power *power);
};

/**
 * dp_power - dp power structure, used to control dptx power
 * @state: state to record dptx power
 * @is_lpm_enable: state to record dptx lower power mode
 * @hw_hss1: the handle of hss1 hardware operation
 * @hw_sctrl: the handle of sctrl hardware operation
 * @hw_ctrl: the handle of dptx controller hardware operation
 * @display: pointer to record display
 * @funcs: the dp power supported functions
 */
struct dp_power {
	enum dp_power_state state;
	bool is_lpm_enable;

	struct dp_hw_hss1 *hw_hss1;
	struct dp_hw_sctrl *hw_sctrl;
	struct dp_hw_ctrl *hw_ctrl;

	struct dp_display *display;

	struct dp_power_funcs *funcs;
};

int dp_power_init(struct dp_display *display, struct dp_power **power);
void dp_power_deinit(struct dp_power *power);

#endif

