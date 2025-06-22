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

#include "dpu_hw_common.h"
#include "dp_hw_msgbus_ops.h"

#define MSGBUS_PHY_TX_CONTROL_OFFSET2               (0x402 << 2)
#define DISABLE_SINGLE_TX_SHIFT                     6
#define DISABLE_SINGLE_TX_LEN                       1
#define TX_DEEMPH_5_0_SHIFT                         0
#define TX_DEEMPH_5_0_LEN                           6

#define MSGBUS_PHY_TX_CONTROL_OFFSET3               (0x403 << 2)
#define TX_DEEMPH_11_6_SHIFT                        0
#define TX_DEEMPH_11_6_LEN                          6

#define MSGBUS_PHY_TX_CONTROL_OFFSET4               (0x404 << 2)
#define TX_DEEMPH_17_12_SHIFT                       0
#define TX_DEEMPH_17_12_LEN                         6

#define MSGBUS_PHY_TX_CONTROL_OFFSET8               (0x408 << 2)
#define TX_MARGIN_SHIFT                             0
#define TX_MARGIN_LEN                               3
#define TX_SWING_SHIFT                              3
#define TX_SWING_LEN                                1

#define MSGBUS_PHY_HDP_TX_CONTROL_OFFSET2           (0x602 << 2)
#define MSGBUS_PHY_HDP_TX_CONTROL_OFFSET3           (0x603 << 2)
#define MSGBUS_PHY_HDP_TX_CONTROL_OFFSET4           (0x604 << 2)
#define MSGBUS_PHY_HDP_TX_CONTROL_OFFSET8           (0x608 << 2)

#define CR_APB_TX_PRE_OVRD_IN_OFFSET(i)             ((0x1005 + 0x200 * i) << 2)
#define CR_APB_TX_MAIN_OVRD_IN_OFFSET(i)            ((0x1004 + 0x200 * i) << 2)
#define TX_EQ_OVRD_EN_SHIFT                         14
#define TX_EQ_OVRD_EN_LEN                           1
#define TX_PRE_CURSOR_SHIFT                         0
#define TX_PRE_CURSOR_LEN                           7
#define TX_MAIN_CURSOR_SHIFT                        7
#define TX_MAIN_CURSOR_LEN                          7
#define TX_POST_CURSOR_SHIFT                        0
#define TX_POST_CURSOR_LEN                          7

#define CR_APB_VBOOST_LVL_OVRD_IN_OFFSET            (0x29 << 2)
#define TX_VBOOST_LVL_OVRN_EN_SHIFT                 9
#define TX_VBOOST_LVL_OVRN_EN_LEN                   1
#define TX_VBOOST_LVL_SHIFT                         6
#define TX_VBOOST_LVL_LEN                           3

#define CR_APB_MPLLB_SSC_OVRD_IN_0_OFFSET           (0x1e << 2)
#define MPLLB_SSC_STEP_SIZE_20_16_SHIFT             5
#define MPLLB_SSC_STEP_SIZE_20_16_LEN               5
#define MPLLB_SSC_EN_SHIFT                          2
#define MPLLB_SSC_EN_LEN                            1
#define CR_APB_MPLLB_SSC_OVRD_IN_1_OFFSET           (0x1F << 2)
#define MPLLB_SSC_STEP_SIZE_15_0_SHIFT              0
#define MPLLB_SSC_STEP_SIZE_15_0_LEN                16
#define CR_APB_MPLLB_SSC_OVRD_IN_2_OFFSET           (0x20 << 2)
#define MPLLB_SSC_PEAK_19_16_SHIFT                  0
#define MPLLB_SSC_PEAK_19_16_LEN                    4
#define CR_APB_MPLLB_SSC_OVRD_IN_3_OFFSET           (0x21 << 2)
#define MPLLB_SSC_PEAK_15_0_SHIFT                   0
#define MPLLB_SSC_PEAK_15_0_LEN                     16


extern enum dp_combo_mode g_combo_mode;
extern enum dp_orientation g_orientation;

static const u32 MSGBUS_OFFSET[2] = {0x0, 0x4000};

void _dp_hw_msgbus_set_vboost(struct dpu_hw_blk *cr_hw, u32 vboost)
{
	u32 reg;

	reg = DP_REG_READ(cr_hw, CR_APB_VBOOST_LVL_OVRD_IN_OFFSET);

	reg = MERGE_BITS(reg, 0x1,
			TX_VBOOST_LVL_OVRN_EN_SHIFT, TX_VBOOST_LVL_OVRN_EN_LEN);
	reg = MERGE_BITS(reg, vboost,
			TX_VBOOST_LVL_SHIFT, TX_VBOOST_LVL_LEN);

	DP_REG_WRITE(cr_hw, CR_APB_VBOOST_LVL_OVRD_IN_OFFSET, reg);
}

void _dp_hw_msgbus_set_ssc_val(struct dpu_hw_blk *cr_hw, u32 ssc_step_size, u32 ssc_peak)
{
	DP_BIT_WRITE(cr_hw, CR_APB_MPLLB_SSC_OVRD_IN_0_OFFSET,
			GET_BITS_VAL(ssc_step_size, 16, MPLLB_SSC_STEP_SIZE_20_16_LEN),
			MPLLB_SSC_STEP_SIZE_20_16_SHIFT, MPLLB_SSC_STEP_SIZE_20_16_LEN);
	DP_BIT_WRITE(cr_hw, CR_APB_MPLLB_SSC_OVRD_IN_1_OFFSET,
			GET_BITS_VAL(ssc_step_size, 0, MPLLB_SSC_STEP_SIZE_15_0_LEN),
			MPLLB_SSC_STEP_SIZE_15_0_SHIFT, MPLLB_SSC_STEP_SIZE_15_0_LEN);

	DP_BIT_WRITE(cr_hw, CR_APB_MPLLB_SSC_OVRD_IN_2_OFFSET,
			GET_BITS_VAL(ssc_peak, 16, MPLLB_SSC_PEAK_19_16_LEN),
			MPLLB_SSC_PEAK_19_16_SHIFT, MPLLB_SSC_PEAK_19_16_LEN);
	DP_BIT_WRITE(cr_hw, CR_APB_MPLLB_SSC_OVRD_IN_3_OFFSET,
			GET_BITS_VAL(ssc_peak, 0, MPLLB_SSC_PEAK_15_0_LEN),
			MPLLB_SSC_PEAK_15_0_SHIFT, MPLLB_SSC_PEAK_15_0_LEN);

	DP_BIT_WRITE(cr_hw, CR_APB_MPLLB_SSC_OVRD_IN_0_OFFSET, 0x1,
			MPLLB_SSC_EN_SHIFT, MPLLB_SSC_EN_LEN);
}

static u32 TX_SWING_OFFSET_TABLE[DP_LANE_COUNT_MAX] = {
	MSGBUS_OFFSET[1] + MSGBUS_PHY_HDP_TX_CONTROL_OFFSET8,
	MSGBUS_OFFSET[1] + MSGBUS_PHY_TX_CONTROL_OFFSET8,
	MSGBUS_OFFSET[0] + MSGBUS_PHY_TX_CONTROL_OFFSET8,
	MSGBUS_OFFSET[0] + MSGBUS_PHY_HDP_TX_CONTROL_OFFSET8,
};

static u32 TX_DEEMP_5_0_OFFSET_TABLE[DP_LANE_COUNT_MAX] = {
	MSGBUS_OFFSET[1] + MSGBUS_PHY_HDP_TX_CONTROL_OFFSET2,
	MSGBUS_OFFSET[1] + MSGBUS_PHY_TX_CONTROL_OFFSET2,
	MSGBUS_OFFSET[0] + MSGBUS_PHY_TX_CONTROL_OFFSET2,
	MSGBUS_OFFSET[0] + MSGBUS_PHY_HDP_TX_CONTROL_OFFSET2,
};

/* PMA */
static u32 TX_PRE_OFFSET_TABLE[][DP_LANE_COUNT_MAX] = {
	{ /* observe */
		CR_APB_TX_PRE_OVRD_IN_OFFSET(2), CR_APB_TX_PRE_OVRD_IN_OFFSET(3),
		CR_APB_TX_PRE_OVRD_IN_OFFSET(0), CR_APB_TX_PRE_OVRD_IN_OFFSET(1),
	},
	{ /* reserve */
		CR_APB_TX_PRE_OVRD_IN_OFFSET(1), CR_APB_TX_PRE_OVRD_IN_OFFSET(0),
		CR_APB_TX_PRE_OVRD_IN_OFFSET(3), CR_APB_TX_PRE_OVRD_IN_OFFSET(2),
	},
};

static u32 TX_MAIN_OFFSET_TABLE[][DP_LANE_COUNT_MAX] = {
	{ /* observe */
		CR_APB_TX_MAIN_OVRD_IN_OFFSET(2), CR_APB_TX_MAIN_OVRD_IN_OFFSET(3),
		CR_APB_TX_MAIN_OVRD_IN_OFFSET(0), CR_APB_TX_MAIN_OVRD_IN_OFFSET(1),
	},
	{ /* reserve */
		CR_APB_TX_MAIN_OVRD_IN_OFFSET(1), CR_APB_TX_MAIN_OVRD_IN_OFFSET(0),
		CR_APB_TX_MAIN_OVRD_IN_OFFSET(3), CR_APB_TX_MAIN_OVRD_IN_OFFSET(2),
	},
};

void _dp_hw_msgbus_set_vswing_preemp(
		struct dpu_hw_blk *msgbus_hw,
		struct dpu_hw_blk *cr_hw,
		u32 (*signal_table)[4][3],
		u8 swing_level[DP_LANE_COUNT_MAX],
		u8 preemp_level[DP_LANE_COUNT_MAX])
{
	u8 lane_count = (g_combo_mode == DP_COMBO_MODE_DP4 ? 4 : 2);
	u8 ori_idx = (g_orientation == DP_ORIENTATION_OBSERVE ? 0 : 1);
	u8 i;

	for (i = 0; i < lane_count; i++) {
		u8 m = swing_level[i], n = preemp_level[i];
		u32 value;

		if (m + n > 3)
			DP_WARN("swing level: %d, preemp level: %d is an invalid combination\n",
					m, n);

		/* swing level */
		DP_BIT_WRITE(msgbus_hw, TX_SWING_OFFSET_TABLE[i], 0x1,
				TX_SWING_SHIFT, TX_SWING_LEN);
		DP_BIT_WRITE(msgbus_hw, TX_SWING_OFFSET_TABLE[i], m,
				TX_MARGIN_SHIFT, TX_MARGIN_LEN);

		/* preemp level */
		DP_BIT_WRITE(msgbus_hw, TX_DEEMP_5_0_OFFSET_TABLE[i], n,
				TX_DEEMPH_5_0_SHIFT, TX_DEEMPH_5_0_LEN);

		/* PMA method for pre / main / post */
		value = 0;
		value = MERGE_BITS(value, 0x1, TX_EQ_OVRD_EN_SHIFT, TX_EQ_OVRD_EN_LEN);
		value = MERGE_BITS(value, signal_table[m][n][1],
				TX_MAIN_CURSOR_SHIFT, TX_MAIN_CURSOR_LEN);
		value = MERGE_BITS(value, signal_table[m][n][2],
				TX_POST_CURSOR_SHIFT, TX_POST_CURSOR_LEN);
		DP_REG_WRITE(cr_hw, TX_MAIN_OFFSET_TABLE[ori_idx][i], value);

		value = 0;
		value = DP_REG_READ(cr_hw, TX_PRE_OFFSET_TABLE[ori_idx][i]);
		value = MERGE_BITS(value, signal_table[m][n][0],
				TX_PRE_CURSOR_SHIFT, TX_PRE_CURSOR_LEN);
		DP_REG_WRITE(cr_hw, TX_PRE_OFFSET_TABLE[ori_idx][i], value);
	}
}

static u32 LANE_CLOSE_OFFSET_TABLE[DP_LANE_COUNT_MAX] = {
	MSGBUS_OFFSET[1] + MSGBUS_PHY_HDP_TX_CONTROL_OFFSET2,
	MSGBUS_OFFSET[1] + MSGBUS_PHY_TX_CONTROL_OFFSET2,
	MSGBUS_OFFSET[0] + MSGBUS_PHY_TX_CONTROL_OFFSET2,
	MSGBUS_OFFSET[0] + MSGBUS_PHY_HDP_TX_CONTROL_OFFSET2
};

void dp_hw_msgbus_reserve_usb_tx_lane(struct dpu_hw_blk *hw,
		enum dp_lane_count lane_count)
{
	u8 flag[DP_LANE_COUNT_MAX];  /* 1 for disable, 0 for enable */
	u8 max_lane_count = (g_combo_mode == DP_COMBO_MODE_DP4 ? 4 : 2);
	u8 i;

	memset(flag, 1, DP_LANE_COUNT_MAX);
	memset(flag, 0, lane_count);

	for (i = 0; i < max_lane_count; i++)
		DP_BIT_WRITE(hw, LANE_CLOSE_OFFSET_TABLE[i],
				flag[i], DISABLE_SINGLE_TX_SHIFT, DISABLE_SINGLE_TX_LEN);
}
