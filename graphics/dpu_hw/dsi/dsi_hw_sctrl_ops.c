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

#include "dpu_osal.h"
#include "dpu_hw_common.h"
#include "dsi_hw_ctrl_ops.h"
#include "dsi_sctrl_reg.h"
#include "dpu_hw_dsi.h"
#include "dsi_hw_ctrl.h"
#include "dsi_hw_phy.h"

#define PHY_CLOCK_READY   (0x20)
#define PHY_IS_READY      (0x2)

#define PHY_1_LANE_READY  (0x400)
#define PHY_2_LANE_READY  (0x800)
#define PHY_3_LANE_READY  (0x1000)
#define PHY_4_LANE_READY  (0x2000)

#define FRAME_UPDATE_DELAY_US (0x1)

#define PHY_INPUT_FREQ         (38400000UL)
#define PHY_INPUT_DIV_N        (2)

static struct freq_range_params freq_range[] = {
	/*        freq       vco    cpibas  p  gmp_cntrl*/
	{/* 00 */   40000, 0b101011, 0x00, 64, 0x1}, /* vco range 62.5 ~ 40 */
	{/* 01 */   44180, 0b101000, 0x00, 64, 0x1},
	{/* 02 */   53320, 0b101000, 0x00, 64, 0x2},
	{/* 03 */   60930, 0b101000, 0x00, 64, 0x1},
	{/* 04 */   62500, 0b100111, 0x00, 32, 0x1}, /* vco range 125 ~ 62.5 */
	{/* 05 */   73130, 0b100011, 0x00, 32, 0x1},
	{/* 06 */   88360, 0b100000, 0x00, 32, 0x1},
	{/* 07 */  106640, 0b100000, 0x00, 32, 0x2},
	{/* 08 */  121880, 0b100000, 0x00, 32, 0x1},
	{/* 09 */  125000, 0b011111, 0x00, 16, 0x1}, /* vco range 250 ~ 125 */
	{/* 10 */  146250, 0b011011, 0x00, 16, 0x1},
	{/* 11 */  176720, 0b011000, 0x00, 16, 0x1},
	{/* 12 */  213300, 0b011000, 0x00, 16, 0x2},
	{/* 13 */  243750, 0b011000, 0x00, 16, 0x1},
	{/* 14 */  250000, 0b010111, 0x00,  8, 0x1}, /* vco range 500 ~ 250 */
	{/* 15 */  292500, 0b010011, 0x00,  8, 0x1},
	{/* 16 */  353400, 0b010000, 0x00,  8, 0x1},
	{/* 17 */  426560, 0b010000, 0x00,  8, 0x2},
	{/* 18 */  487500, 0b010000, 0x00,  8, 0x1},
	{/* 19 */  500000, 0b001111, 0x00,  4, 0x1}, /* vco range 500 ~ 1000 */
	{/* 20 */  585000, 0b001011, 0x00,  4, 0x1},
	{/* 21 */  706875, 0b001000, 0x00,  4, 0x1},
	{/* 22 */  853125, 0b001000, 0x00,  4, 0x2},
	{/* 23 */  975000, 0b001000, 0x00,  4, 0x1},
	{/* 24 */ 1000000, 0b000111, 0x00,  2, 0x1}, /* vco range 1000 ~ 2250 */
	{/* 25 */ 1170000, 0b000011, 0x00,  2, 0x1},
	{/* 26 */ 1413750, 0b000000, 0x00,  2, 0x1},
	{/* 27 */ 1706250, 0b000000, 0x00,  2, 0x2},
	{/* 28 */ 2000000, 0b000000, 0x20,  2, 0x1},
	{/* 29 */ 2250000, 0b000000, 0x20,  2, 0x1}, /* vco range > 2250 */
	{/* 30 */ 2650000, 0b000000, 0x20,  2, 0x2},
	{/* 31 */ 4500001, 0b000000, 0x20,  2, 0x2}, /* range max */
};

void dsi_hw_sctrl_debug(struct dpu_hw_blk *hw)
{
	u32 phy_cfg1, st_clkgate;

	phy_cfg1 = DPU_REG_READ(hw, PHY_IF_CFG1);
	st_clkgate = DPU_REG_READ(hw, ST_CLKGATE_AUTO);

	DSI_INFO("scrl phy_cfg1 0x%x, clk_gate 0x%x\n", phy_cfg1, st_clkgate);
}

void dsi_hw_sctrl_pll_clk_sel(struct dpu_hw_blk *hw, bool enable)
{
	DPU_BITMASK_WRITE(hw, PHY_PLL_IF_CFG1, (enable ? 1 : 0),
		PHY_PLL_IF_CFG1_CFG_PLL_CLKSEL_SHIFT,
		PHY_PLL_IF_CFG1_CFG_PLL_CLKSEL_MASK, DIRECT_WRITE);
}

static u64 __maybe_unused dsi_hw_sctrl_clk_debug(struct dpu_hw_blk *hw,
		enum debug_clk clk)
{
	/* n: wait test time, m: ready_delay */
	u64 rate, t_apb_clk;
	u32 clk_circle, value, m;

	/**
	 * clk [3:1]: cfg_dsi_clk_measure_sel
	 * 0- sel ipi_clk;
	 * 1- sel sys clk;
	 * 2- sel_hstx clk;
	 * 3- sel lprx clk;
	 * 4- sel lptx clk
	 * 5- sel pllout 90 left
	 * apb_clk = 69630000UL;
	 * clk_circle = 65536;
	 * t_apb_clk = apb_clk / clk_circle * 1000
	 * t_clk_ns = 941096, about 1ms
	*/

	clk_circle = 65536;
	t_apb_clk = 1062469;

	value = DPU_REG_READ(hw, DSI_CLK_FREQUCR_MEASURE);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_ENABLE_SHIFT,
		DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_ENABLE_MASK);
	value = MERGE_MASK_BITS(value, clk,
		DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_SEL_SHIFT,
		DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_SEL_MASK);
	value = MERGE_MASK_BITS(value, clk_circle,
		DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_NUM_SHIFT,
		DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_NUM_MASK);
	DPU_REG_WRITE(hw, DSI_CLK_FREQUCR_MEASURE, value, DIRECT_WRITE);

	DPU_USLEEP(1100);

	m = DPU_REG_READ(hw, DSI_CLK_FREQUCR_MEASURE_R);

	/* rate = 69630000 * m / clk_circle; */

	rate = t_apb_clk * m / 1000;

	DSI_DEBUG("rate: %lld, m = 0%x, value = 0x%x\n", rate, m, value);

	DPU_REG_WRITE(hw, DSI_CLK_FREQUCR_MEASURE, 0, DIRECT_WRITE);

	return rate;
}

void dsi_hw_clk_debug(struct dpu_hw_blk *hw)
{
	u64 rate;

	rate = dsi_hw_sctrl_clk_debug(hw, DEBUG_IPI_CLK);
	DSI_DEBUG("ipi_clk rate is %llu\n", rate);

	rate = dsi_hw_sctrl_clk_debug(hw, DEBUG_GP_CLK);
	DSI_DEBUG("gp_clk(pll out) rate is %llu\n", rate);

	rate = dsi_hw_sctrl_clk_debug(hw, DEBUG_SYS_CLK);
	DSI_DEBUG("sys_clk rate is %llu\n", rate);

	rate = dsi_hw_sctrl_clk_debug(hw, DEBUG_HSTX_CLK);
	DSI_DEBUG("hstx_clk rate is %llu\n", rate);

	rate = dsi_hw_sctrl_clk_debug(hw, DEBUG_LPTX_CLK);
	DSI_DEBUG("lptx_clk rate is %llu\n", rate);
}

void dsi_hw_sctrl_init_clk_set(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	u32 ip_rst_phy_n, ip_prst_phy_n;
	u32 value;

	if (cfg->phy_parms.phy_sel == XILINX_DPHY) {
		ip_rst_phy_n = 1;
		ip_prst_phy_n = 1;
	} else {
		ip_rst_phy_n = 0;
		ip_prst_phy_n = 0;
	}

	/* DIV 3 */
	DPU_REG_WRITE(hw, DSI_PCLK_DIV, 0x2, DIRECT_WRITE);

	value = DPU_REG_READ(hw, DSI_CLKGATE_W1S);

	DSI_DEBUG("DSI_CLKGATE_W1S:0x%08X\n", value);

	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_PHY_PLL_OUT_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_PHY_PLL_OUT_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_IPI_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_IPI_CLKGATE_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_PHY_APB_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_PHY_APB_CLKGATE_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_CTRL_APB_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_CTRL_APB_CLKGATE_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_SYS_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_SYS_CLKGATE_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_PHY_REF_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_PHY_REF_CLKGATE_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLKGATE_W1S_GT_DSI_PHY_CFG_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_PHY_CFG_CLKGATE_EN_MASK);
	value = MERGE_MASK_BITS(value, cfg->vg_en,
		DSI_CLKGATE_W1S_GT_DSI_VG_IPI_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_VG_IPI_CLKGATE_EN_MASK);
	DPU_REG_WRITE(hw, DSI_CLKGATE_W1S, value, DIRECT_WRITE);

	/* set auto-gate bypass */
	value = DPU_REG_READ(hw, DSI_CLK_GATE_CTRL);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLK_GATE_CTRL_CFG_IPI_CLK_GATE_BYPASS_SHIFT,
		DSI_CLK_GATE_CTRL_CFG_IPI_CLK_GATE_BYPASS_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLK_GATE_CTRL_CFG_SYS_CLK_GATE_BYPASS_SHIFT,
		DSI_CLK_GATE_CTRL_CFG_SYS_CLK_GATE_BYPASS_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_CLK_GATE_CTRL_CFG_HSTX_CLK_GATE_BYPASS_SHIFT,
		DSI_CLK_GATE_CTRL_CFG_HSTX_CLK_GATE_BYPASS_MASK);
	DPU_REG_WRITE(hw, DSI_CLK_GATE_CTRL, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, DSI_SRESET_W1S);
	value = MERGE_MASK_BITS(value, ip_rst_phy_n,
		DSI_SRESET_W1S_IP_RST_DSI_PHY_N_SHIFT,
		DSI_SRESET_W1S_IP_RST_DSI_PHY_N_MASK);
	value = MERGE_MASK_BITS(value, ip_prst_phy_n,
		DSI_SRESET_W1S_IP_PRST_DSI_PHY_N_SHIFT,
		DSI_SRESET_W1S_IP_PRST_DSI_PHY_N_MASK);
	value = MERGE_MASK_BITS(value, 1,
		DSI_SRESET_W1S_IP_PRST_DSI_CTRL_N_SHIFT,
		DSI_SRESET_W1S_IP_PRST_DSI_CTRL_N_MASK);
	value = MERGE_MASK_BITS(value, cfg->vg_en,
		DSI_SRESET_W1S_IP_RST_DSI_VG_IPI_N_SHIFT,
		DSI_SRESET_W1S_IP_RST_DSI_VG_IPI_N_MASK);
	DPU_REG_WRITE(hw, DSI_SRESET_W1S, value, DIRECT_WRITE);
}

static int dsi_hw_wait_phy_ready(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	enum dsi_phy_lane lane_num = cfg->phy_parms.lanes_num;
	enum dsi_phy_type phy_type = cfg->phy_parms.phy_type;
	u32 state, value;
	int ret = 0;
	int time;

	/**
	 * Wait phy ready by read sctrl status register,
	 * read cri status cyscical in 25000ns,
	 * 0: cri is not busy, 1: cri is busy
	 * default: 4 lane
	 */
	switch (lane_num) {
	case DSI_PHY_1LANE:
		state = PHY_1_LANE_READY;
		break;
	case DSI_PHY_2LANE:
		state = PHY_2_LANE_READY;
		break;
	case DSI_PHY_3LANE:
		state = PHY_3_LANE_READY;
		break;
	case DSI_PHY_4LANE:
		state = PHY_4_LANE_READY;
		break;
	default:
		state = PHY_4_LANE_READY;
		break;
	}

	if (phy_type == DSI_PHY_TYPE_CPHY)
		state = state | (1 << PHY_IF_CFG1_CFG_PHY_READY_SHIFT);
	else
		state = state | (1 << PHY_IF_CFG1_CFG_STOPSTATE_DCK_SHIFT) |
			(1 << PHY_IF_CFG1_CFG_PHY_READY_SHIFT);

	/* PHY state reg is : need to reconfirm */
	time = DPU_READ_POLL_TIMEOUT(hw, PHY_IF_CFG1, value, (value & state),
		1000, 25000);
	if (time < 0) {
		DSI_ERROR("wait phy ready timeout! status 0x%x\n", value);
		ret = -1;
	} else {
		DSI_INFO("PHY_IN_CFG1: 0x%x 0x%x, time %d\n", state, value, time);
	}

	return ret;
}

int dsi_hw_sctrl_phy_pll_config(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	int ret, loop_div, m_int, div_int;
	u64 fout, mpll_frac_quot, m_dec, pll_prg;
	u8  size, range;
	u16 pll_m;
	u32 value;

	ret = 0;
	value = 0;
	/* M */
	loop_div = 0;
	/* M quot */
	m_dec = 0;
	/* M fran */
	m_int = 0;

	if (cfg->phy_parms.phy_hs_speed < PHY_OUTPUT_RATE_MIN) {
		DSI_ERROR("Speed too low %d\n", cfg->phy_parms.phy_hs_speed);
		ret = -1;
	} else if (cfg->phy_parms.phy_hs_speed > PHY_OUTPUT_RATE_MAX) {
		DSI_ERROR("Speed too high %d\n", cfg->phy_parms.phy_hs_speed);
		ret = -1;
	}

	/* lane output clk, lane_rate /2 */
	fout = cfg->phy_parms.phy_hs_speed / PHY_CLOCK_DIV;

	/* find fout range */
	size = ARRAY_SIZE(freq_range) - 1;

	DSI_DEBUG("size %d, fout %llu\n", size, fout);

	for (range = 0; range < size; range++) {
		if (((fout / 1000) > freq_range[range].freq) &&
			((fout / 1000) < freq_range[range + 1].freq))
			break;
	}

	if (range > size){
		DSI_ERROR("input speed does not support, size:%d\n", size);
		return -1;
	}

	DSI_DEBUG("freq_range[%u].freq = %ul, p %d\n",
		range, freq_range[range].freq, freq_range[range].p);

	loop_div = fout * (freq_range[range].p) * PHY_INPUT_DIV_N /
		(PHY_INPUT_FREQ / PHY_QUOT_GEN);

	div_int = loop_div / PHY_QUOT_GEN * PHY_QUOT_GEN;
	m_int = loop_div / PHY_QUOT_GEN;

	if ((loop_div - div_int) >= (PHY_QUOT_GEN / 2)) {
		pll_m = m_int * 2 - 32 + 1;
		/* m_dec >= 0.5 ,neet to -0.5 */
		m_dec = (u32)((loop_div - div_int) - (PHY_QUOT_GEN / 2));
	} else {
		pll_m = m_int * 2 - 32;
		m_dec = (u32)(loop_div - div_int);
	}

	/* mpll_mint = pll_m */
	mpll_frac_quot = m_dec * (PHY_MPLL_COE / PHY_MPLL_COE_DIV) /
		(PHY_QUOT_GEN / PHY_MPLL_COE_DIV);
	pll_prg = 0; /* fixed config */

	DSI_DEBUG("cdphy pll cfg start!\n");
	DSI_DEBUG("loop_div %d, div_int %d, m_int %d m_dec %llu\n",
		loop_div, div_int, m_int, m_dec);
	DSI_DEBUG("pll_m %d, pll_mint %d, pll_n %d, N %d\n", pll_m, pll_m,
		PHY_INPUT_DIV_N - 1, PHY_INPUT_DIV_N);
	DSI_DEBUG("pll_cpbias_ctrl = %d\n", freq_range[range].cpibas_cntrl);
	DSI_DEBUG("pll_vco_cntrl = %d\n", freq_range[range].vco_cntrl);
	DSI_DEBUG("pll_frac_quot = %llu\n", mpll_frac_quot);
	DSI_DEBUG("pll_gmp_cntrl = %d\n", freq_range[range].gmp_cntrl);
	DSI_DEBUG("pll_prop_cntrl = %d\n", 0x14);
	DSI_DEBUG("pll_int_cntrl = %d\n", 0x8);
	DSI_DEBUG("pll ssc_en %d, ssc_peak %d, stepsize %d\n", 0, 0, 0);
	DSI_DEBUG("pll_th1 %d, pll_th2 %d, pll_th3 %d\n", 0x12, 0xA, 0x1);
	DSI_DEBUG("th_delay = %d\n", 0);
	DSI_DEBUG("int_cntrl = %d\n", 0x8);
	DSI_DEBUG("pll_clksel = %d\n", 0x1);
	DSI_DEBUG("pll_atb_sense_sel = %d\n", 0x0);
	DSI_DEBUG("pll_opmode = %d\n", 0x10);
	DSI_DEBUG("pll_prg = %llu\n", pll_prg);
	DSI_DEBUG("pll_meas_iv = %d\n", 0);
	DSI_DEBUG("pll_clkouten_left = %d\n", 1);
	DSI_DEBUG("pll_spread_type = %d\n", 0);
	DSI_DEBUG("pll_frac_en %d, pll_frac_update_en %d\n", 1, 1);

	value = DPU_REG_READ(hw, PHY_PLL_IF_CFG1);
	value = MERGE_MASK_BITS(value, freq_range[range].gmp_cntrl,
		PHY_PLL_IF_CFG1_CFG_PLL_GMP_CNTRL_SHIFT,
		PHY_PLL_IF_CFG1_CFG_PLL_GMP_CNTRL_MASK);
	value = MERGE_MASK_BITS(value, freq_range[range].cpibas_cntrl,
		PHY_PLL_IF_CFG1_CFG_PLL_CPBIAS_CNTRL_SHIFT,
		PHY_PLL_IF_CFG1_CFG_PLL_CPBIAS_CNTRL_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_PLL_IF_CFG1_CFG_PLL_ATB_SENSE_SEL_SHIFT,
		PHY_PLL_IF_CFG1_CFG_PLL_ATB_SENSE_SEL_MASK);
	DPU_REG_WRITE(hw, PHY_PLL_IF_CFG1, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_IF_CFG2);
	value = MERGE_MASK_BITS(value, (PHY_INPUT_DIV_N - 1),
		PHY_IF_CFG2_CFG_PLL_N_SHIFT,
		PHY_IF_CFG2_CFG_PLL_N_MASK);
	value = MERGE_MASK_BITS(value, 0x10,
		PHY_IF_CFG2_CFG_MPLL_OPMODE_SHIFT,
		PHY_IF_CFG2_CFG_MPLL_OPMODE_MASK);
	value = MERGE_MASK_BITS(value, pll_m,
		PHY_IF_CFG2_CFG_PLL_M_SHIFT,
		PHY_IF_CFG2_CFG_PLL_M_MASK);
	value = MERGE_MASK_BITS(value, 0x8,
		PHY_IF_CFG2_CFG_PLL_INT_CNTRL_SHIFT,
		PHY_IF_CFG2_CFG_PLL_INT_CNTRL_MASK);
	DPU_REG_WRITE(hw, PHY_IF_CFG2, value, DIRECT_WRITE);

	/* PHY initial T1 - config mpll_cfg */
	DPU_REG_WRITE(hw, PHY_IF_CFG3, pll_prg, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_IF_CFG4);
	value = MERGE_MASK_BITS(value, 1,
		PHY_IF_CFG4_CFG_PLL_TH3_SHIFT,
		PHY_IF_CFG4_CFG_PLL_TH3_MASK);
	value = MERGE_MASK_BITS(value, 10,
		PHY_IF_CFG4_CFG_PLL_TH2_SHIFT,
		PHY_IF_CFG4_CFG_PLL_TH2_MASK);
	value = MERGE_MASK_BITS(value, 18,
		PHY_IF_CFG4_CFG_PLL_TH1_SHIFT,
		PHY_IF_CFG4_CFG_PLL_TH1_MASK);
	value = MERGE_MASK_BITS(value, 0x14,
		PHY_IF_CFG4_CFG_PLL_PROP_CNTRL_SHIFT,
		PHY_IF_CFG4_CFG_PLL_PROP_CNTRL_MASK);
	DPU_REG_WRITE(hw, PHY_IF_CFG4, value, DIRECT_WRITE);

	/* PHY initial T1 - PLL Hard Macro Interface */
	value = DPU_REG_READ(hw, PHY_IF_CFG5);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG5_CFG_PLL_MEAS_IV_SHIFT,
		PHY_IF_CFG5_CFG_PLL_MEAS_IV_MASK);
	value = MERGE_MASK_BITS(value, 1,
		PHY_IF_CFG5_CFG_PLL_CLKOUTEN_LEFT_SHIFT,
		PHY_IF_CFG5_CFG_PLL_CLKOUTEN_LEFT_MASK);
	/* PHY initial T1 - mpll spread_type set to 0 */
	value = MERGE_MASK_BITS(value, freq_range[range].vco_cntrl,
		PHY_IF_CFG5_CFG_PLL_VCO_CNTRL_SHIFT,
		PHY_IF_CFG5_CFG_PLL_VCO_CNTRL_MASK);
	DPU_REG_WRITE(hw, PHY_IF_CFG5, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_IF_CFG6);
	/* PHY initial T1 - mpll spread_type set to 0 */
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG6_CFG_MPLL_SPREAD_TYPE_SHIFT,
		PHY_IF_CFG6_CFG_MPLL_SPREAD_TYPE_MASK);
	value = MERGE_MASK_BITS(value, 1,
		PHY_IF_CFG6_CFG_MPLL_FRAC_EN_SHIFT,
		PHY_IF_CFG6_CFG_MPLL_FRAC_EN_MASK);
	/* PHY initial T1 - About SSC, not support!! */
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG6_CFG_MPLL_SSC_EN_SHIFT,
		PHY_IF_CFG6_CFG_MPLL_SSC_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		PHY_IF_CFG6_CFG_MPLL_FRACN_UPDATE_EN_SHIFT,
		PHY_IF_CFG6_CFG_MPLL_FRACN_UPDATE_EN_MASK);
	DPU_REG_WRITE(hw, PHY_IF_CFG6, value, DIRECT_WRITE);

	DPU_REG_WRITE(hw, PHY_IF_CFG7, 0, DIRECT_WRITE);

	DPU_REG_WRITE(hw, PHY_IF_CFG8, 0, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_IF_CFG9);
	value = MERGE_MASK_BITS(value, pll_m,
		PHY_IF_CFG9_CFG_MPLL_MINT_SHIFT,
		PHY_IF_CFG9_CFG_MPLL_MINT_MASK);
	value = MERGE_MASK_BITS(value, mpll_frac_quot,
		PHY_IF_CFG9_CFG_MPLL_FRAC_QUOT_SHIFT,
		PHY_IF_CFG9_CFG_MPLL_FRAC_QUOT_MASK);
	DPU_REG_WRITE(hw, PHY_IF_CFG9, value, DIRECT_WRITE);

	/* PHY initial T1 - config pll_gp_clken */
	value = DPU_REG_READ(hw, PHY_IF_CFG10);
	value = MERGE_MASK_BITS(value, 1,
		PHY_IF_CFG10_CFG_MPLL_FRAC_DEN_SHIFT,
		PHY_IF_CFG10_CFG_MPLL_FRAC_DEN_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG10_CFG_MPLL_FRAC_REM_SHIFT,
		PHY_IF_CFG10_CFG_MPLL_FRAC_REM_MASK);
	DPU_REG_WRITE(hw, PHY_IF_CFG10, value, DIRECT_WRITE);

	return 0;
}

static void __maybe_unused dsi_hw_sctrl_pll_en(struct dpu_hw_blk *hw,
		u8 enable)
{
	/* Wakeup-Core - Enable phy pll out en */
	DPU_BITMASK_WRITE(hw, DSI_CLKGATE_W1S, enable,
		DSI_CLKGATE_W1S_GT_DSI_PHY_PLL_OUT_EN_SHIFT,
		DSI_CLKGATE_W1S_GT_DSI_PHY_PLL_OUT_EN_MASK, DIRECT_WRITE);
}

int dsi_hw_sctrl_ppi_clk_config(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	u64 gp_clk, hs_clk, div_int, div_dec, div_tmp;
	u32 ipi_clk;
	int div = 0;

	if (cfg->ipi_clk <= 0) {
		DSI_ERROR("ipi clk cannont be negative, ipi clk: %u\n",
			cfg->ipi_clk);
		return -1;
	}

	ipi_clk = cfg->ipi_clk;
	hs_clk = cfg->phy_parms.phy_hs_speed;

	/* TODO : fomula */
	if ((hs_clk <= 4500000000) && (hs_clk >= 2000000000)) {
		gp_clk = hs_clk / 4;
	} else if ((hs_clk < 2000000000) && (hs_clk >= 1000000000)) {
		gp_clk = hs_clk / 2;
	} else if ((hs_clk < 1000000000) && (hs_clk >= 500000000)) {
		gp_clk = hs_clk;
	} else if ((hs_clk < 500000000) && (hs_clk >= 250000000)) {
		gp_clk = hs_clk * 2;
	} else if ((hs_clk < 250000000) && (hs_clk >= 125000000)) {
		gp_clk = hs_clk * 4;
	} else if ((hs_clk < 125000000) && (hs_clk >= 80000000)) {
		gp_clk = hs_clk * 8;
	} else {
		gp_clk = hs_clk;
		DSI_ERROR("Unsupported phy hs clock %llu\n", hs_clk);
		return -1;
	}

	DSI_DEBUG("hs_clk %llu, gp_clk %llu, ipi_clk %u, \n",
		hs_clk, gp_clk, ipi_clk);

	div_tmp = gp_clk * 1000 / ipi_clk;
	div_int = gp_clk / ipi_clk * 1000;
	div_dec = div_tmp - div_int;

	if (div_dec >= 500) {
		div = div_int / 1000;
	} else {
		div = div_int / 1000 - 1;
	}

	/* ipi_clk_div = 0, use formula; ipi_clk_div = 0, use dtsi set div */
	if (cfg->ipi_clk_div != 0)
		div = cfg->ipi_clk_div;

	DSI_DEBUG("div_tmp %llu, div_int %llu, div_dec %llu, div %d\n",
		div_tmp, div_int, div_dec, div);

	/* ipi clock generate selsect: 0x04 */
	DPU_BITMASK_WRITE(hw, PHY_IF_CFG2, 1,
		PHY_IF_CFG2_CFG_PLL_GP_CLKEN_SHIFT,
		PHY_IF_CFG2_CFG_PLL_GP_CLKEN_MASK, DIRECT_WRITE);

	/* Wakeup-Core - Set ipi clock divider:0x58 */
	DPU_REG_WRITE(hw, DSI_PPI_CLK_DIV, div, DIRECT_WRITE);

	/* for low power */
#ifdef LP_POWER
	DPU_BITMASK_WRITE(hw, DSI_CLKGATE_W1C, 1,
		DSI_CLKGATE_W1C_GT_DSI_PHY_APB_CLKGATE_EN_SHIFT,
		DSI_CLKGATE_W1C_GT_DSI_PHY_APB_CLKGATE_EN_MASK, DIRECT_WRITE);
#endif
	return 0;
}

static void dsi_hw_sctrl_vg_set(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	u32 value, color_depth = 1;

	value = DPU_REG_READ(hw, DSI_VG_CFG2);

	value = MERGE_MASK_BITS(value, cfg->timing.hsa,
		DSI_VG_CFG2_CFG_DSI_VG_HSA_SHIFT,
		DSI_VG_CFG2_CFG_DSI_VG_HSA_MASK);
	value = MERGE_MASK_BITS(value, cfg->timing.hbp,
		DSI_VG_CFG2_CFG_DSI_VG_HBP_SHIFT,
		DSI_VG_CFG2_CFG_DSI_VG_HBP_MASK);
	DPU_REG_WRITE(hw, DSI_VG_CFG2, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, DSI_VG_CFG3);

	value = MERGE_MASK_BITS(value, cfg->timing.hact,
		DSI_VG_CFG3_CFG_DSI_VG_HACT_SHIFT,
		DSI_VG_CFG3_CFG_DSI_VG_HACT_MASK);
	value = MERGE_MASK_BITS(value, cfg->timing.hfp,
		DSI_VG_CFG3_CFG_DSI_VG_HFP_SHIFT,
		DSI_VG_CFG3_CFG_DSI_VG_HFP_MASK);
	DPU_REG_WRITE(hw, DSI_VG_CFG3, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, DSI_VG_CFG4);

	value = MERGE_MASK_BITS(value, cfg->timing.vsa,
		DSI_VG_CFG4_CFG_DSI_VG_VSA_SHIFT,
		DSI_VG_CFG4_CFG_DSI_VG_VSA_MASK);
	value = MERGE_MASK_BITS(value, cfg->timing.vbp,
		DSI_VG_CFG4_CFG_DSI_VG_VBP_SHIFT,
		DSI_VG_CFG4_CFG_DSI_VG_VBP_MASK);
	DPU_REG_WRITE(hw, DSI_VG_CFG4, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, DSI_VG_CFG5);

	value = MERGE_MASK_BITS(value, cfg->timing.vact,
		DSI_VG_CFG5_CFG_DSI_VG_VACT_SHIFT,
		DSI_VG_CFG5_CFG_DSI_VG_VACT_MASK);
	value = MERGE_MASK_BITS(value, cfg->timing.vbp,
		DSI_VG_CFG5_CFG_DSI_VG_VFP_SHIFT,
		DSI_VG_CFG5_CFG_DSI_VG_VFP_MASK);
	DPU_REG_WRITE(hw, DSI_VG_CFG5, value, DIRECT_WRITE);

	switch (cfg->pixel_fomat) {
	case DSI_FMT_RGB666:
		color_depth = 0;
		break;
	case DSI_FMT_RGB888:
		color_depth = 1;
		break;
	case DSI_FMT_RGB101010:
		color_depth = 2;
		break;
	case DSI_FMT_RGB565:
	case DSI_FMT_RGB121212:
	case DSI_FMT_YUV422:
	case DSI_FMT_YUV420:
	case DSI_FMT_YUV422_LOOSELY:
	case DSI_FMT_RGB_LOOSELY:
	case DSI_FMT_DSC:
		DPU_DEBUG("DSI VG mode does not support this pixel format: %d\n",
			cfg->pixel_fomat);
		break;
	default:
		color_depth = 1;
		break;
	}

	value = DPU_REG_READ(hw, DSI_VG_CFG0);

	value = MERGE_MASK_BITS(value, 1, DSI_VG_CFG0_CFG_DSI_VG_EN_SHIFT,
		DSI_VG_CFG0_CFG_DSI_VG_EN_MASK);
	value = MERGE_MASK_BITS(value, cfg->ctrl_mode,
		DSI_VG_CFG0_CFG_DSI_VG_OPMODE_SHIFT,
		DSI_VG_CFG0_CFG_DSI_VG_OPMODE_MASK);
	/* default RGB888 */
	value = MERGE_MASK_BITS(value, color_depth,
		DSI_VG_CFG0_CFG_DSI_VG_COLOR_DEPTH_SHIFT,
		DSI_VG_CFG0_CFG_DSI_VG_COLOR_DEPTH_MASK);
	value = MERGE_MASK_BITS(value, cfg->vg_dpmode,
		DSI_VG_CFG0_CFG_DSI_VG_DPMODE_SHIFT,
		DSI_VG_CFG0_CFG_DSI_VG_DPMODE_MASK);
	value = MERGE_MASK_BITS(value, 0,
		DSI_VG_CFG0_CFG_DSI_VG_HIBERNATE_EN_SHIFT,
		DSI_VG_CFG0_CFG_DSI_VG_HIBERNATE_EN_MASK);
	value = MERGE_MASK_BITS(value, 0,
		DSI_VG_CFG0_CFG_DSI_VG_HIB_NUM_SHIFT,
		DSI_VG_CFG0_CFG_DSI_VG_HIB_NUM_MASK);
	DPU_REG_WRITE(hw, DSI_VG_CFG0, value, DIRECT_WRITE);
}

static void dsi_hw_sctrl_phy_shutdown(struct dpu_hw_blk *hw,
		u8 shutdown)
{
	DPU_BITMASK_WRITE(hw, PHY_IF_CFG0, shutdown,
		PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_SHIFT,
		PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_MASK, DIRECT_WRITE);
}

static void dsi_hw_sctrl_phy_forcestop(struct dpu_hw_blk *hw)
{
	u32 value;

	value = DPU_REG_READ(hw, PHY_IF_CFG0);

	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE3_SHIFT,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE3_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE2_SHIFT,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE2_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE1_SHIFT,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE1_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE0_SHIFT,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE0_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE_DCK_SHIFT,
		PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE_DCK_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_IF_CFG0_CFG_PHY_FORCERXMODE_DCK_SHIFT,
		PHY_IF_CFG0_CFG_PHY_FORCERXMODE_DCK_MASK);

	DPU_REG_WRITE(hw, PHY_IF_CFG0, value, DIRECT_WRITE);
}

void dsi_hw_sctrl_init_phy_pre(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	u32 value;

	if (cfg->phy_parms.phy_sel != XILINX_DPHY) {
		value = DPU_REG_READ(hw, PHY_IF_CFG0);
		value = MERGE_MASK_BITS(value, 0,
			PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_SHIFT,
			PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_MASK);
		value = MERGE_MASK_BITS(value, 1,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE3_SHIFT,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE3_MASK);
		value = MERGE_MASK_BITS(value, 1,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE2_SHIFT,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE2_MASK);
		value = MERGE_MASK_BITS(value, 1,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE1_SHIFT,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE1_MASK);
		value = MERGE_MASK_BITS(value, 1,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE0_SHIFT,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE0_MASK);
		value = MERGE_MASK_BITS(value, 1,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE_DCK_SHIFT,
			PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE_DCK_MASK);
		value = MERGE_MASK_BITS(value, cfg->phy_parms.phy_type,
			PHY_IF_CFG0_CFG_PHY_MODE_SHIFT,
			PHY_IF_CFG0_CFG_PHY_MODE_MASK);
		DPU_REG_WRITE(hw, PHY_IF_CFG0, value, DIRECT_WRITE);
		/* step 3: PHY initial T1 - 2 PLL config */
	}

	if (cfg->phy_parms.phy_sel == SNPS_CDPHY_ASIC) {
		dsi_hw_sctrl_phy_pll_config(hw, cfg);
		dsi_hw_sctrl_pll_clk_sel(hw, true);
	}

	DPU_BITMASK_WRITE(hw, DSI_SRESET_W1S, 1,
		DSI_SRESET_W1S_IP_PRST_DSI_PHY_N_SHIFT,
		DSI_SRESET_W1S_IP_PRST_DSI_PHY_N_MASK, DIRECT_WRITE);
}

void dsi_hw_sctrl_init_phy(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	int ret;

	if (cfg->phy_parms.phy_sel != XILINX_DPHY) {

		/* step 3: PHY initial T4: 0x00 [0] */
		DPU_BITMASK_WRITE(hw, PHY_IF_CFG0, 1,
			PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_SHIFT,
			PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_MASK, DIRECT_WRITE);
		/*  0xB0 : move to dsi_hw_sctrl_init_clk_set */
		// dsi_hw_sctrl_pll_en(hw, 1);

		/* step 3: PHY initial T4: sctr phy reset:0xC0 [3]*/
		DPU_BITMASK_WRITE(hw, DSI_SRESET_W1S, 1,
			DSI_SRESET_W1S_IP_RST_DSI_PHY_N_SHIFT,
			DSI_SRESET_W1S_IP_RST_DSI_PHY_N_MASK, DIRECT_WRITE);

		/* step 3: PHY initial T5: Wait for PHY ready: 0x04 */
		ret = dsi_hw_wait_phy_ready(hw, cfg);
		if (ret)
			DSI_ERROR("PHY init failled\n");
	}

	if (cfg->phy_parms.phy_sel == SNPS_CDPHY_ASIC) {
		/* step 3: PHY initial T6 - stopstate */
		dsi_hw_sctrl_phy_forcestop(hw);

		/* step 4: ipi_clock config div */
		dsi_hw_sctrl_ppi_clk_config(hw, cfg);
	}

	DSI_DEBUG("dsi_hw_sctrl_init_phy\n");
}

static void __maybe_unused dsi_hw_sctrl_shutdown(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	dsi_hw_sctrl_phy_shutdown(hw, 0);

	if (cfg->vg_en)
		DPU_BITMASK_WRITE(hw, DSI_VG_CFG0, 0,
			DSI_VG_CFG0_CFG_DSI_VG_EN_SHIFT,
			DSI_VG_CFG0_CFG_DSI_VG_EN_MASK, DIRECT_WRITE);
}

void dsi_hw_sctrl_phy_reset(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	dsi_hw_sctrl_phy_shutdown(hw, 1);

	DPU_USLEEP(FRAME_UPDATE_DELAY_US);

	dsi_hw_sctrl_phy_shutdown(hw, 0);

	DPU_USLEEP(FRAME_UPDATE_DELAY_US);

	dsi_hw_sctrl_phy_shutdown(hw, 1);
}

void dsi_hw_sctrl_frame_update(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	DSI_DEBUG("xring_dsi_sctrl_frame_update\n");

	DPU_REG_WRITE(hw, DSI_VG_TE_TRG, 0, DIRECT_WRITE);

	/* hardware need delay 1us */
	DPU_USLEEP(FRAME_UPDATE_DELAY_US);

	DPU_REG_WRITE(hw, DSI_VG_TE_TRG, 1, DIRECT_WRITE);
}

void dsi_hw_sctrl_vg_en(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	int i;

	DSI_DEBUG("vg_en %u\n", cfg->vg_en);

	if (cfg->vg_en) {
		dsi_hw_sctrl_vg_set(hw, cfg);
		for (i = 0; i < 10; i++) {
			DSI_DEBUG("update %d\n", i);
			dsi_hw_sctrl_frame_update(hw, cfg);
		}
	}
}

void dsi_hw_sctrl_deinit(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	/* TODO */
}

int dsi_hw_sctrl_dyn_freq_wait_pll_lock(struct dpu_hw_blk *hw)
{
	int ret;
	u32 value;
	ret = DPU_READ_POLL_TIMEOUT(hw, PHY_IF_CFG1, value,
			(value & PHY_IF_CFG1_CFG_PLL_LOCK_MASK), 50, 400);
	if (ret < 0)
		DSI_ERROR("wait sctrl pll lock timeout, value 0x%x!\n", value);
	return ret;
}

int dsi_hw_sctrl_dyn_freq_wait_phy_ready(struct dpu_hw_blk *hw)
{
	int ret;
	u32 value;
	ret = DPU_READ_POLL_TIMEOUT(hw, PHY_IF_CFG1, value,
			(value & PHY_IF_CFG1_CFG_PHY_READY_MASK), 1000, 25000);
	if (ret < 0)
		DSI_ERROR("wait phy ready timeout, value 0x%x!\n", value);
	return ret;
}
