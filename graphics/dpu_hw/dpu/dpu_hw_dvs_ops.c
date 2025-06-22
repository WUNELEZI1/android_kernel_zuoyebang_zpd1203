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

#include "dpu_hw_dvs_ops.h"

#define DVS_DPU_SW_DVFS_CHANNEL_ID                        0
#define DVS_DPU_HW_DVFS_CHANNEL_ID                        0

#define DVS_VOLT_REQ_0_OFFSET                             0
#define DVS_VOLT_REQ_0_SHIFT                              0
#define DVS_VOLT_REQ_0_LENGTH                             8
#define DVS_VOLT_REQ_VALID_0_SHIFT                        8
#define DVS_VOLT_REQ_VALID_0_LENGTH                       1

#define DVS_BYPASS_0_OFFSET                               0x060
#define SW_VOLT_REQ_BYPASS_0_SHIFT                        0
#define SW_VOLT_REQ_BYPASS_0_LENGTH                       1

#define DVS_BYPASS_1_OFFSET                               0x064
#define HW_VOLT_REQ_BYPASS_0_SHIFT                        0
#define HW_VOLT_REQ_BYPASS_0_LENGTH                       1

#define DVS_SW_INITIAL_0_OFFSET                           0x120
#define DVS_SW_INITIAL_VOLT_REQ_0_SHIFT                   0
#define DVS_SW_INITIAL_VOLT_REQ_0_LENGTH                  8
#define DVS_SW_INITIAL_SEL_0_SHIFT                        8
#define DVS_SW_INITIAL_SEL_0_LENGHT                       1

#define DVS_SW_STATE_ACK_OFFSET                           0x074
#define DVS_SW_STATE_ACK_0_SHIFT                          0
#define DVS_SW_STATE_ACK_0_LENGTH                         1

#define DVS_CUR_STATE_OFFSET                              0x07C
#define DVS_VOLT_CURRENT_SHIFT                            21
#define DVS_VOLT_CURRENT_LENGTH                           8

#define DVS_REG_MASK_FILED_SHIFT                          16
#define DVS_REG_MASK_FILED_LENGTH                         16

#define DVS_INTR_TIMEOUT_IRQ_INT_RAW                      0x0210
#define DVS_VLD_TIMEOUT                                   2000

#define DPU_HIGHEST_PROFILE_VOLT                          0x5A

static u32 dvs_get_sw_vote(struct dpu_hw_blk *hw)
{
	u32 sw_volt;

	sw_volt = DPU_BIT_READ(hw, DVS_VOLT_REQ_0_OFFSET, DVS_VOLT_REQ_0_SHIFT,
			DVS_VOLT_REQ_0_LENGTH);

	PERF_DEBUG("sw_volt is 0x%x\n", sw_volt);
	if (!sw_volt) {
		PERF_ERROR("sw_volt is 0, hardcode to DPU_HIGHEST_PROFILE_VOLT\n");
		sw_volt = DPU_HIGHEST_PROFILE_VOLT;
	}
	return sw_volt;
}

static void dvs_bypass_hw_channel(struct dpu_hw_blk *hw, bool bypass)
{
	u32 value;

	value = 1 << (DVS_REG_MASK_FILED_SHIFT + DVS_DPU_HW_DVFS_CHANNEL_ID);

	if (bypass) {
		value = value | (1 << DVS_DPU_HW_DVFS_CHANNEL_ID);
		DPU_REG_WRITE(hw, DVS_BYPASS_1_OFFSET, value, DIRECT_WRITE);
	} else {
		DPU_REG_WRITE(hw, DVS_BYPASS_1_OFFSET, value, DIRECT_WRITE);
	}
}

static void dvs_bypass_sw_channel(struct dpu_hw_blk *hw, bool bypass)
{
	u32 value;

	value = 1 << (DVS_REG_MASK_FILED_SHIFT + DVS_DPU_SW_DVFS_CHANNEL_ID);
	if (bypass) {
		value = value | (1 << DVS_DPU_SW_DVFS_CHANNEL_ID);
		DPU_REG_WRITE(hw, DVS_BYPASS_0_OFFSET, value, DIRECT_WRITE);
	} else {
		DPU_REG_WRITE(hw, DVS_BYPASS_0_OFFSET, value, DIRECT_WRITE);
	}
}

static int dvs_check_intr(struct dpu_hw_blk *hw)
{
	u32 value;

	value = DPU_REG_READ(hw, DVS_INTR_TIMEOUT_IRQ_INT_RAW);
	if (value)
		PERF_ERROR("dvs timeout happened, 0x%x, data 0x%x\n",
				DVS_INTR_TIMEOUT_IRQ_INT_RAW, value);

	return value ? -1 : 0;
}

static void dvs_do_sw_vote(struct dpu_hw_blk *hw, u32 voltage)
{
	u32 value;
	int timeout = DVS_VLD_TIMEOUT;

	if (dvs_check_intr(hw))
		return;

	value = BITS_MASK(DVS_VOLT_REQ_0_SHIFT, DVS_VOLT_REQ_0_LENGTH +
			DVS_VOLT_REQ_VALID_0_LENGTH) << DVS_REG_MASK_FILED_SHIFT;
	value = value | (BIT(DVS_VOLT_REQ_VALID_0_SHIFT));
	value = value | (voltage & 0xff);

	PERF_DEBUG("value is 0x%x\n", value);
	DPU_REG_WRITE(hw, DVS_VOLT_REQ_0_OFFSET, value, DIRECT_WRITE);

	do {
		DPU_UDELAY(5);
		value = DPU_BIT_READ(hw, DVS_SW_STATE_ACK_OFFSET, DVS_SW_STATE_ACK_0_SHIFT,
				DVS_SW_STATE_ACK_0_LENGTH);
		if (value)
			break;
		timeout--;
	} while (timeout > 0);

	if (timeout == 0) {
		PERF_ERROR("hdc wait dvs ack timeout\n");
	}
	value = BIT(DVS_VOLT_REQ_VALID_0_SHIFT) << DVS_REG_MASK_FILED_SHIFT;
	DPU_REG_WRITE(hw, DVS_VOLT_REQ_0_OFFSET, value, DIRECT_WRITE);
}

static void dvs_do_hw_vote(struct dpu_hw_blk *hw, u32 sw_volt)
{
	u32 value;

	value = BIT(DVS_SW_INITIAL_SEL_0_SHIFT);
	value = value | (sw_volt & 0xff);
	DPU_REG_WRITE(hw, DVS_SW_INITIAL_0_OFFSET, value, DIRECT_WRITE);

	value = sw_volt & 0xff;
	DPU_REG_WRITE(hw, DVS_SW_INITIAL_0_OFFSET, value, DIRECT_WRITE);

	dvs_do_sw_vote(hw, sw_volt);
}

void dpu_hw_dvs_sw_to_hw_vote(struct dpu_hw_blk *hw)
{
	u32 sw_volt;

	sw_volt = dvs_get_sw_vote(hw);

	dvs_bypass_hw_channel(hw, false);

	dvs_do_hw_vote(hw, sw_volt);

	dvs_bypass_sw_channel(hw, true);
}

void dpu_hw_dvs_hw_to_sw_vote(struct dpu_hw_blk *hw, u32 hw_volt)
{
	if (!hw_volt) {
		PERF_ERROR("fatal error hw volt is zero\n");
		return;
	}

	PERF_DEBUG("hw volt is 0x%x\n", hw_volt);

	dvs_bypass_sw_channel(hw, false);

	dvs_do_sw_vote(hw, hw_volt);

	dvs_bypass_hw_channel(hw, true);
}

void dpu_hw_dvs_state_dump(struct dpu_hw_blk *hw, int cnt)
{
	u32 value;

	PERF_INFO("cnt is %d\n", cnt);
	do {
		value = DPU_REG_READ(hw, DVS_CUR_STATE_OFFSET);
		PERF_INFO("dvs state 0x%x, curr_volt 0x%x\n",
				value,
				value >> DVS_VOLT_CURRENT_SHIFT);
		cnt--;
	} while(cnt > 0);
}

