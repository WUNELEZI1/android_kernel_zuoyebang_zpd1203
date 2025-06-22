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

#include "dp_hw_hss1_ops.h"

/* HSS1_CRG reg configuration */
#define HSS1_CRG_CLK_GT_OFFSET0                          0x000
#define GT_PCLK_DPTX_MESSAGEBUS_FLAG                     BIT(31)
#define GT_CLK_DPU_DP_IPI_FLAG                           BIT(21)
#define GT_CLK_DPTX_IPI_FLAG                             BIT(20)
#define GT_PCLK_DPTX_CONTROLLER_APB0_FLAG                BIT(19)
#define GT_PCLK_DPTX_EXT_SDP_APB1_FLAG                   BIT(18)
#define GT_CLK_DPTX_AUX16MHZ_FLAG                        BIT(14)

#define HSS1_CRG_CLK_GT_OFFSET1                          0x800
#define GT_ACLK_DPTX_ESM_SHIFT                           2
#define GT_ACLK_DPTX_ESM_LEN                             1

#define HSS1_CRG_RST_OFFSET                              0x020
#define IP_RST_DPTX_SCTRL_N_FLAG                         BIT(17)
#define IP_RST_DPTX_VCC_N_FLAG                           BIT(19)

#define HSS1_CRG_CLK_DIV_OFFSET5                         0x054
#define DIV_CLK_DPU_DP_IPI_SHIFT                         0
#define DIV_CLK_DPU_DP_IPI_LEN                           6
#define SC_GT_CLK_DPU_DP_IPI_SHIFT                       12
#define SC_GT_CLK_DPU_DP_IPI_LEN                         1

#define HSS1_CRG_PERI_STAT_OFFSET0                       0x070
#define DIV_DONE_CLK_DPU_DP_INI_FLAG                     BIT(9)

/* read poll delay */
#define DIV_DONE_POLL_DELAY_US                           10
#define DIV_DONE_POLL_TIMEOUT_US                         100

/* ipi clock limitation */
#define DP_IPI_CLOCK_RATE_MAX                            576000000
#define DP_IPI_CLOCK_RATE_MIN                            24000000


static u32 reset_modules = IP_RST_DPTX_SCTRL_N_FLAG | IP_RST_DPTX_VCC_N_FLAG;

static u32 clk_gate_modules = GT_CLK_DPTX_IPI_FLAG |
			GT_PCLK_DPTX_CONTROLLER_APB0_FLAG |
			GT_CLK_DPTX_AUX16MHZ_FLAG |
			GT_PCLK_DPTX_EXT_SDP_APB1_FLAG |
			GT_PCLK_DPTX_MESSAGEBUS_FLAG;

/* div_ratio in [1, 0x40] */
int dp_hw_hss1_set_ipi_clock_div_ratio(struct dpu_hw_blk *hw, u32 div_ratio)
{
	u32 val;
	int ret;

	if (div_ratio > 0x40 || div_ratio == 0) {
		DP_ERROR("clock div ratio(0x%02x) is invalid, range: [0x1, 0x40]\n",
				div_ratio);
		return -EINVAL;
	}

	/* config ipi clock div ratio, BMRW, if set div ratio to be 4, should write 0x3 */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_DIV_OFFSET5,
			(BITS_MASK(DIV_CLK_DPU_DP_IPI_SHIFT, DIV_CLK_DPU_DP_IPI_LEN)
			<< 16) | (div_ratio - 1));

	/* wait for div ratio to be stable */
	ret = DP_READ_POLL_TIMEOUT(hw, HSS1_CRG_PERI_STAT_OFFSET0, val,
			(val & DIV_DONE_CLK_DPU_DP_INI_FLAG),
			DIV_DONE_POLL_DELAY_US, DIV_DONE_POLL_TIMEOUT_US);
	if (ret < 0) {
		DP_ERROR("timeout in waiting for clock div ratio stable\n");
		return -EBUSY;
	}

	return 0;
}

void dp_hw_hss1_power_off(struct dpu_hw_blk *hw)
{
	/* clock gate off */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_GT_OFFSET0 + 0x4, clk_gate_modules);

	/* reset state */
	DP_REG_WRITE(hw, HSS1_CRG_RST_OFFSET + 0x4, reset_modules);
}

int dp_hw_hss1_power_on(struct dpu_hw_blk *hw)
{
	/* reset dptx_vcc_n */
	DP_REG_WRITE(hw, HSS1_CRG_RST_OFFSET + 0x4, IP_RST_DPTX_VCC_N_FLAG);

	/* clock gate on */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_GT_OFFSET0, clk_gate_modules);

	/* clock gate off */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_GT_OFFSET0 + 0x4, clk_gate_modules);

	/* clear reset */
	DP_REG_WRITE(hw, HSS1_CRG_RST_OFFSET, reset_modules);

	/* clock gate on */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_GT_OFFSET0, clk_gate_modules);

	return 0;
}

void dp_hw_hss1_ipi_clock_gate_off(struct dpu_hw_blk *hw)
{
	/* SCR, dpu dp ipi clock gate and dptx ipi clock gate */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_GT_OFFSET0 + 0x4, GT_CLK_DPU_DP_IPI_FLAG | GT_CLK_DPTX_IPI_FLAG);

	/* BMRW, ipi div pre clock gate */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_DIV_OFFSET5,
			(BITS_MASK(SC_GT_CLK_DPU_DP_IPI_SHIFT,
			SC_GT_CLK_DPU_DP_IPI_LEN) << 16) | 0x0);
}

int dp_hw_hss1_ipi_clock_gate_on(struct dpu_hw_blk *hw)
{
	u32 val;
	int ret;

	/* BMRW, ipi div pre clock gate */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_DIV_OFFSET5,
			(BITS_MASK(SC_GT_CLK_DPU_DP_IPI_SHIFT,
			SC_GT_CLK_DPU_DP_IPI_LEN) << 16) |
			BITS_MASK(SC_GT_CLK_DPU_DP_IPI_SHIFT,
			SC_GT_CLK_DPU_DP_IPI_LEN));

	/* SCR, dpu dp ipi clock gate and dptx ipi clock gate */
	DP_REG_WRITE(hw, HSS1_CRG_CLK_GT_OFFSET0, GT_CLK_DPU_DP_IPI_FLAG | GT_CLK_DPTX_IPI_FLAG);

	/* wait for div ratio to be stable */
	ret = DP_READ_POLL_TIMEOUT(hw, HSS1_CRG_PERI_STAT_OFFSET0, val,
			(val & DIV_DONE_CLK_DPU_DP_INI_FLAG),
			DIV_DONE_POLL_DELAY_US, DIV_DONE_POLL_TIMEOUT_US);
	if (ret < 0) {
		DP_ERROR("timeout in waiting for clock div ratio stable\n");
		goto exit;
	}

	return 0;

exit:
	dp_hw_hss1_ipi_clock_gate_off(hw);
	return -EBUSY;
}
