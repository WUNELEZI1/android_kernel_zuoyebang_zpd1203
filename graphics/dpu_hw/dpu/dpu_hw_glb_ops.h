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

#ifndef _DPU_HW_GLB_OPS_H_
#define _DPU_HW_GLB_OPS_H_

#include "dpu_hw_common.h"

enum {
	READ_PRELOAD_VAL = 0,
	READ_SHADOW_VAL,
};

/**
 * hdc_cfg - hdc module configure info
 * @curr_profile: current profile id
 * @active_div_core: active core clk divider
 * @active_div_veu: active veu clk divider
 * @active_div_axi: active axi clk divider
 * @active_div_pclk: active pclk divider
 * @idle_div_core: idle core clk divider
 * @idle_div_veu: idle veu clk divider
 * @idle_div_axi: idle axi clk divider
 * @idle_div_pclk: idle pclk divider
 * @timeout_counter: hdc wait dvs timeout counter
 * @sync_counter: sync counter for media dvs
 * @idle_volt: voltage for idle
 * @active_volt: voltage for active
 * @active_low_volt: voltage for active low
 * @freq_div_counter: freq counter for bypass
 * @volt_scaling_disable: whether do volt change or not
 */
struct hdc_cfg {
	u32 curr_profile;

	struct {
		u32 active_div_core;
		u32 active_div_veu;
		u32 active_div_axi;
		u32 active_div_pclk;
	};
	struct {
		u32 idle_div_core;
		u32 idle_div_veu;
		u32 idle_div_axi;
		u32 idle_div_pclk;
	};

	u32 timeout_counter;
	u32 sync_counter;

	u32 idle_volt;
	u32 active_volt;
	u32 active_low_volt;

	u32 freq_div_counter;
	u32 volt_scaling_disable;
};

/**
 * hdc_status: hdc module work status
 * @cfg: hdc module current configure
 * @work_status: current hw dvfs working status
 * @fail_counter: number of hdc boost failures
 * @soft_clr_val: current soft clear reg value
 * @sw_clk_en: current sw clk en reg value
 * @auto_cg_en: current auto cg en reg value
 * @idle_dvfs_en: current idle dvfs en reg value
 * @active_dvfs_en: current active dvfs en reg value
 */
struct hdc_status {
	struct hdc_cfg cfg;

	u32 work_status;
	u32 fail_counter;
	u32 soft_clr_val;

	u32 sw_clk_en;
	u32 auto_cg_en;
	u32 idle_dvfs_en;
	u32 active_dvfs_en;
};

/**
 * dpu_hw_glb_hdc_enable - hdc hw dvfs enable
 * @hw：the glb module pointer
 */
void dpu_hw_glb_hdc_enable(struct dpu_hw_blk *hw);

/**
 * dpu_hw_glb_hdc_disable - hdc hw dvfs disable
 * @hw：the glb module pointer
 *
 * if hw dvfs disabld abnormal, will do hdc module reset
 */
void dpu_hw_glb_hdc_disable(struct dpu_hw_blk *hw);

/**
 * dpu_hw_glb_hdc_config - do hdc hw dvfs configure
 * @hw：the glb module pointer
 * @profile_id: profile id
 */
void dpu_hw_glb_hdc_config(struct dpu_hw_blk *hw, int profile_id);

/**
 * dpu_hw_glb_hdc_dump - dump hdc work status
 * @hw：the glb module pointer
 * @status: save current hdc work status
 */
void dpu_hw_glb_hdc_dump(struct dpu_hw_blk *hw, struct hdc_status *status, int flag);

/**
 * dpu_hw_glb_hdc_get_hw_vote_val - get current hw dvfs vote val
 * @hw：the glb module pointer
 *
 * Returns: current hw dvfs vote value
 */
u32 dpu_hw_glb_hdc_get_hw_vote_val(struct dpu_hw_blk *hw);

/**
 * dpu_hw_glb_hdc_update_avs_codes - update dpu avs codes
 * @avs_codes: the avs codes array
 */
void dpu_hw_glb_hdc_update_avs_codes(u8 *avs_codes);
#endif
