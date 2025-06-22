/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _DP_LINK_H_
#define _DP_LINK_H_

#include <linux/version.h>
#include <linux/types.h>
#include <drm/display/drm_dp_helper.h>

#include "dp_hw_ctrl.h"
#include "dp_aux.h"

struct dp_link;

/**
 * dp_link_phy_param - phy parameter referenced in link training
 * @init_link_rate: link training initial link rate
 * @init_lane_count: link training initial lane count
 * @init_preemp_level: initial pre-emphasis level when link training cr
 * @init_swing_level: initial voltage swing level when link training cr
 * @fec_en: if fec is enabled
 * @ssc_en: if ssc is enabled
 * @enhance_frame_en: if enhance framing is enabled
 * @eq_pattern: training pattern for eq
 * @signal_table: signal table for swing and preemp level
 */
struct dp_link_phy_param {
	enum dp_link_rate init_link_rate;
	enum dp_lane_count init_lane_count;

	u8 init_preemp_level[DP_LANE_COUNT_MAX];
	u8 init_swing_level[DP_LANE_COUNT_MAX];

	bool fec_en;
	bool ssc_en;
	bool enhance_frame_en;

	enum dp_training_pattern eq_pattern;

	u32 (*signal_table)[4][4][3];
};

/**
 * dp_link_phy_status - phy status in link training
 * @link_rate: current link rate
 * @lane_count: current lane count
 * @preemp_level: pre-emphasis level of link
 * @swing_level: voltage swing level of link
 * @link_rate_mbps: data transferred rate of link, in unit of mbps
 * @link_clk_khz: link clock frequency, in unit of khz
 * @fec_en: current fec status of link
 */
struct dp_link_phy_status {
	enum dp_link_rate link_rate;
	enum dp_lane_count lane_count;

	u8 preemp_level[DP_LANE_COUNT_MAX];
	u8 swing_level[DP_LANE_COUNT_MAX];

	u32 link_rate_mbps;
	u32 link_clk_khz;

	bool fec_en;
};

#define DP_LINK_CAPS_TPS3                          BIT(0)
#define DP_LINK_CAPS_TPS4                          BIT(1)
#define DP_LINK_CAPS_ENHANCE_FRAME                 BIT(2)

struct dp_link_auto_test {
	bool test_training_flag;
	u8 test_lane_count;
	u8 test_link_rate;

	bool test_pattern_flag;
	u8 test_pattern;
	u8 test_cus_pattern_80b[10];
};

struct dp_link_irq_vector {
	/* raw irq vector */
	u8 irq_vector;

	bool auto_test_flag;
	struct dp_link_auto_test auto_test;

	bool cp_irq_flag;
};

struct dp_link_funcs {
	/**
	 * training_init - do initialization operation of link training
	 * @link: handle of dp link
	 *
	 * returns 0 for success, negative error code for failure.
	 */
	int (*training_init)(struct dp_link *link);

	/**
	 * training_cr - do clock recovery training
	 * @link: handle of dp link
	 *
	 * returns 0 for success, -EPROTO for failure, -EAGAIN for reducing lane
	 * count or link rate and retry
	 */
	int (*training_cr)(struct dp_link *link);

	/**
	 * training_eq - do channel equalization training
	 * @link: handle of dp link
	 *
	 * returns 0 for success, -EPROTO for failure, -EAGAIN for reducing lane
	 * count or link rate and retry
	 */
	int (*training_eq)(struct dp_link *link);

	/**
	 * training_completed - do finishing operation of link training
	 * @link: handle of dp link
	 * @success: if link training is successful
	 *
	 * returns 0 for success, negative error code for failure.
	 */
	int (*training_completed)(struct dp_link *link, bool success);

	/**
	 * dump_link_status - dump link status
	 * @link: handle of dp link
	 */
	void (*dump_link_status)(struct dp_link *link);

	/**
	 * check_link_status - check link status
	 * @link: handle of dp link
	 */
	bool (*check_link_status)(struct dp_link *link);

	/*
	 * parser_sink_irq_vector - parser sink irq vector
	 * @link: handle of dp link
	 */
	int (*parser_sink_irq_vector)(struct dp_link *link);
};

/**
 * dp_link - the structure to maintain link status, do link training
 * @link_status: the status of link, raw data of dpcd
 * @dpcd_caps: the raw data of link capabilities
 * @irq_vector: irq vector of sink
 * @phy_param: the parameters referenced in link training
 * @phy_status: the status of phy parameters
 * @hw_ctrl: the handle of dp controller hardware operation
 * @hw_sctrl: the handle of sctrl hardware operation
 * @hw_msgbus: the handle of msgbus hardware operation
 * @aux: the handle of aux chanel transaction
 * @funcs: the dp link supported functions
 */
struct dp_link {
	u8 link_status[DP_LINK_STATUS_SIZE];
	u8 dpcd_caps[DP_RECEIVER_CAP_SIZE];

	struct dp_link_irq_vector irq_vector;
	struct dp_link_phy_param phy_param;
	struct dp_link_phy_status phy_status;

	struct dp_hw_ctrl *hw_ctrl;
	struct dp_hw_sctrl *hw_sctrl;
	struct dp_hw_msgbus *hw_msgbus;
	struct dp_aux *aux;

	struct dp_link_funcs *funcs;
};

/**
 * dp_link_set_vswing_preemp - set voltage swing and pre-emphasis
 * @link - pointer of dp link
 */
void dp_link_set_vswing_preemp(struct dp_link *link);

/**
 * dp_link_reset_to_default_param - reset link param to default parameter
 */
void dp_link_reset_to_default_param(struct dp_link *link);

/**
 * dp_link_set_default_param - set default parameter of link
 * @name: name of parameter
 * @value: value of parameter
 */
void dp_link_set_default_param(const char *name, void *value);

/* set default signal value in specific swing level, preemp level and link rate*/
void dp_link_set_default_signal_table(u8 link_rate, u8 swing_level,
		u8 preemp_level, u32 pre, u32 main, u32 post);

/* get default link parameter */
struct dp_link_phy_param *dp_link_get_default_param(void);

u8 dp_link_rate_to_dptx_code(u8 dpcd_bw);

bool dp_link_check_link_status(struct dp_link *link);

int dp_link_init(struct dp_aux *aux, struct dp_hw_ctrl *hw_ctrl,
		struct dp_hw_sctrl *hw_sctrl, struct dp_hw_msgbus *hw_msgbus,
		struct dp_link **link);

void dp_link_deinit(struct dp_link *dp_link);

#endif
