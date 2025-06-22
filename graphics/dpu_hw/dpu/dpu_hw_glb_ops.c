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

#include "dpu_hw_glb_ops.h"
#include "dpu_hw_disp_glb_reg.h"

#define HW_DVFS_SOFT_CLR_ENABLE                                0
#define HW_DVFS_SOFT_CLR_DISABLE                               1

#define CORE_CLK_SHIFT                                         0
#define CORE_CLK_LENGTH                                        6
#define VEU_CLK_SHIFT                                          6
#define VEU_CLK_LENGTH                                         6
#define AXI_CLK_SHIFT                                          12
#define AXI_CLK_LENGTH                                         6
#define APB_CLK_SHIFT                                          18
#define APB_CLK_LENGTH                                         6

#define HDC_ACK_TIMEOUT_CNT                                    12000
#define HDC_SYNC_CNT                                           30
#define HDC_DISABLE_TIMEOUT_CNT                                2000

static struct hdc_cfg g_hdc_cfg[] = {
	/* set profile 0 volt scaling enable */
	{ DPU_PROFILE_0, {5, 5, 5, 12}, {6, 6, 6, 13}, HDC_ACK_TIMEOUT_CNT, HDC_SYNC_CNT, 0x3c, 0x3c, 0x3c, 5,  0},
	{ DPU_PROFILE_1, {4, 4, 4,  9}, {5, 5, 5, 10}, HDC_ACK_TIMEOUT_CNT, HDC_SYNC_CNT, 0x3c, 0x46, 0x3c, 8,  0},
	{ DPU_PROFILE_2, {4, 4, 3,  6}, {5, 5, 4,  7}, HDC_ACK_TIMEOUT_CNT, HDC_SYNC_CNT, 0x3c, 0x5a, 0x46, 10, 0},
};

void dpu_hw_glb_hdc_update_avs_codes(u8 *avs_codes)
{
	if (!avs_codes) {
		PERF_ERROR("invalid parameters\n");
		return;
	}

	g_hdc_cfg[DPU_PROFILE_0].active_volt = avs_codes[DPU_PROFILE_0];
	g_hdc_cfg[DPU_PROFILE_0].active_low_volt = avs_codes[DPU_PROFILE_0];
	g_hdc_cfg[DPU_PROFILE_0].idle_volt = avs_codes[DPU_PROFILE_0];

	g_hdc_cfg[DPU_PROFILE_1].active_volt = avs_codes[DPU_PROFILE_1];
	g_hdc_cfg[DPU_PROFILE_1].active_low_volt = avs_codes[DPU_PROFILE_0];
	g_hdc_cfg[DPU_PROFILE_1].idle_volt = avs_codes[DPU_PROFILE_0];

	g_hdc_cfg[DPU_PROFILE_2].active_volt = avs_codes[DPU_PROFILE_2];
	g_hdc_cfg[DPU_PROFILE_2].active_low_volt = avs_codes[DPU_PROFILE_1];
	g_hdc_cfg[DPU_PROFILE_2].idle_volt = avs_codes[DPU_PROFILE_0];
}

void dpu_hw_glb_hdc_enable(struct dpu_hw_blk *hw)
{
	u32 value;

	PERF_DEBUG("hdc enable\n");

	value = BIT(AUTO_CG_EN_SHIFT) | BIT(IDLE_DVFS_EN_SHIFT) | BIT(ACTIVE_DVFS_EN_SHIFT);
	DPU_REG_WRITE(hw, SW_CLK_EN_OFFSET, value, DIRECT_WRITE);
}

void dpu_hw_glb_hdc_disable(struct dpu_hw_blk *hw)
{
	u32 timeout = HDC_DISABLE_TIMEOUT_CNT;
	u32 value;

	value = BIT(AUTO_CG_EN_SHIFT);
	DPU_REG_WRITE(hw, SW_CLK_EN_OFFSET, value, DIRECT_WRITE);

	do {
		value = DPU_BIT_READ(hw, DPU_DVFS_STATUS_OFFSET,
				DPU_DVFS_STATUS_SHIFT, DPU_DVFS_STATUS_LENGTH);
		if (value == 0)
			break;
		DPU_UDELAY(1);
		timeout--;
	} while (timeout > 0);

	if (!timeout) {
		PERF_ERROR("wait dvfs shutdown timeout!\n");
		DPU_BIT_WRITE(hw, DDR_IDLE_DVFS_EN_OFFSET, HW_DVFS_SOFT_CLR_ENABLE,
				DVFS_SOFT_CLR_SHIFT, DVFS_SOFT_CLR_LENGTH, DIRECT_WRITE);
		DPU_UDELAY(1);
		DPU_BIT_WRITE(hw, DDR_IDLE_DVFS_EN_OFFSET, HW_DVFS_SOFT_CLR_DISABLE,
			DVFS_SOFT_CLR_SHIFT, DVFS_SOFT_CLR_LENGTH, DIRECT_WRITE);
	}

	DPU_REG_WRITE(hw, SW_CLK_EN_OFFSET, 0, DIRECT_WRITE);
}

void dpu_hw_glb_hdc_config(struct dpu_hw_blk *hw, int profile_id)
{
	struct hdc_cfg *cfg;
	u32 value;

	if (profile_id >= DPU_PROFILE_MAX_NUM || profile_id < DPU_PROFILE_0) {
		PERF_ERROR("fatal error: invalid profile %d\n", profile_id);
		return;
	}

	cfg = &g_hdc_cfg[profile_id];

	value = 0;
	value = MERGE_BITS(value, cfg->active_div_core, CORE_CLK_SHIFT, CORE_CLK_LENGTH);
	value = MERGE_BITS(value, cfg->active_div_veu, VEU_CLK_SHIFT, VEU_CLK_LENGTH);
	value = MERGE_BITS(value, cfg->active_div_axi, AXI_CLK_SHIFT, AXI_CLK_LENGTH);
	value = MERGE_BITS(value, cfg->active_div_pclk, APB_CLK_SHIFT, APB_CLK_LENGTH);
	DPU_REG_WRITE(hw, ACTIVE_DIV_OFFSET, value, DIRECT_WRITE);

	value = 0;
	value = MERGE_BITS(value, cfg->idle_div_core, CORE_CLK_SHIFT, CORE_CLK_LENGTH);
	value = MERGE_BITS(value, cfg->idle_div_veu, VEU_CLK_SHIFT, VEU_CLK_LENGTH);
	value = MERGE_BITS(value, cfg->idle_div_axi, AXI_CLK_SHIFT, AXI_CLK_LENGTH);
	value = MERGE_BITS(value, cfg->idle_div_pclk, APB_CLK_SHIFT, APB_CLK_LENGTH);
	DPU_REG_WRITE(hw, IDLE_DIV_OFFSET, value, DIRECT_WRITE);

	value = cfg->sync_counter | (cfg->timeout_counter << TIMEOUT_COUNTER_SHIFT);
	DPU_REG_WRITE(hw, SYNC_COUNTER_OFFSET, value, DIRECT_WRITE);

	value = cfg->freq_div_counter |
			(cfg->active_volt << SCENE_VOLT_SHIFT) |
			(cfg->idle_volt << IDLE_VOLT_SHIFT);
	DPU_REG_WRITE(hw, FREQ_DIV_COUNTER_OFFSET, value, DIRECT_WRITE);

	DPU_REG_WRITE(hw, ACTIVE_LOW_VOLT_OFFSET, cfg->active_low_volt, DIRECT_WRITE);

	DPU_BIT_WRITE(hw, DDR_IDLE_DVFS_EN_OFFSET, HW_DVFS_SOFT_CLR_DISABLE,
			DVFS_SOFT_CLR_SHIFT, DVFS_SOFT_CLR_LENGTH, DIRECT_WRITE);

	DPU_REG_WRITE(hw, VOLT_SCALING_DISABLE_OFFSET, cfg->volt_scaling_disable,
			DIRECT_WRITE);
}

void dpu_hw_glb_hdc_dump(struct dpu_hw_blk *hw, struct hdc_status *status, int flag)
{
	struct hdc_cfg *cfg = &status->cfg;

	if (flag) {
		PERF_DEBUG("read work regs\n");
		DPU_REG_WRITE(hw, DISP_GLB_FORCE_UPDATE_EN_OFFSET, 0x6, DIRECT_WRITE);
	}
	else {
		PERF_DEBUG("read preload regs\n");
		DPU_REG_WRITE(hw, DISP_GLB_FORCE_UPDATE_EN_OFFSET, 0x2, DIRECT_WRITE);
	}

	cfg->idle_div_core = DPU_BIT_READ(hw, IDLE_DIV_OFFSET, CORE_CLK_SHIFT, CORE_CLK_LENGTH);
	cfg->idle_div_veu = DPU_BIT_READ(hw, IDLE_DIV_OFFSET, VEU_CLK_SHIFT, VEU_CLK_LENGTH);
	cfg->idle_div_axi = DPU_BIT_READ(hw, IDLE_DIV_OFFSET, AXI_CLK_SHIFT, AXI_CLK_LENGTH);
	cfg->idle_div_pclk = DPU_BIT_READ(hw, IDLE_DIV_OFFSET, APB_CLK_SHIFT, APB_CLK_LENGTH);

	cfg->active_div_core = DPU_BIT_READ(hw, ACTIVE_DIV_OFFSET, CORE_CLK_SHIFT, CORE_CLK_LENGTH);
	cfg->active_div_veu = DPU_BIT_READ(hw, ACTIVE_DIV_OFFSET, VEU_CLK_SHIFT, VEU_CLK_LENGTH);
	cfg->active_div_axi = DPU_BIT_READ(hw, ACTIVE_DIV_OFFSET, AXI_CLK_SHIFT, AXI_CLK_LENGTH);
	cfg->active_div_pclk = DPU_BIT_READ(hw, ACTIVE_DIV_OFFSET, APB_CLK_SHIFT, APB_CLK_LENGTH);

	cfg->sync_counter = DPU_BIT_READ(hw, SYNC_COUNTER_OFFSET,
			SYNC_COUNTER_SHIFT, SYNC_COUNTER_LENGTH);
	cfg->timeout_counter = DPU_BIT_READ(hw, SYNC_COUNTER_OFFSET,
			TIMEOUT_COUNTER_SHIFT, TIMEOUT_COUNTER_LENGTH);
	cfg->freq_div_counter = DPU_BIT_READ(hw, FREQ_DIV_COUNTER_OFFSET,
			FREQ_DIV_COUNTER_SHIFT, FREQ_DIV_COUNTER_LENGTH);
	cfg->active_volt = DPU_BIT_READ(hw, FREQ_DIV_COUNTER_OFFSET,
			SCENE_VOLT_SHIFT, SCENE_VOLT_LENGTH);
	cfg->idle_volt = DPU_BIT_READ(hw, FREQ_DIV_COUNTER_OFFSET,
			IDLE_VOLT_SHIFT, IDLE_VOLT_LENGTH);
	cfg->active_low_volt = DPU_REG_READ(hw, ACTIVE_LOW_VOLT_OFFSET);
	cfg->volt_scaling_disable = DPU_REG_READ(hw, VOLT_SCALING_DISABLE_OFFSET);

	status->work_status =  DPU_BIT_READ(hw, DPU_DVFS_STATUS_OFFSET,
			DPU_DVFS_STATUS_SHIFT, DPU_DVFS_STATUS_LENGTH);
	status->fail_counter = DPU_BIT_READ(hw, DDR_DVFS_STATUS_OFFSET,
			DVFS_FAIL_CNT_SHIFT, DVFS_FAIL_CNT_LENGTH);
	status->soft_clr_val = DPU_BIT_READ(hw, DDR_IDLE_DVFS_EN_OFFSET,
			DVFS_SOFT_CLR_SHIFT, DVFS_SOFT_CLR_LENGTH);
	status->auto_cg_en = DPU_BIT_READ(hw, SW_CLK_EN_OFFSET,
			AUTO_CG_EN_SHIFT, AUTO_CG_EN_LENGTH);
	status->idle_dvfs_en = DPU_BIT_READ(hw, SW_CLK_EN_OFFSET,
			IDLE_DVFS_EN_SHIFT, IDLE_DVFS_EN_LENGTH);
	status->active_dvfs_en = DPU_BIT_READ(hw, SW_CLK_EN_OFFSET,
			ACTIVE_DVFS_EN_SHIFT, ACTIVE_DVFS_EN_LENGTH);
}

u32 dpu_hw_glb_hdc_get_hw_vote_val(struct dpu_hw_blk *hw)
{
	DPU_REG_WRITE(hw, DISP_GLB_FORCE_UPDATE_EN_OFFSET, 0x2, DIRECT_WRITE);

	return DPU_BIT_READ(hw, FREQ_DIV_COUNTER_OFFSET,
			SCENE_VOLT_SHIFT, SCENE_VOLT_LENGTH);
}
