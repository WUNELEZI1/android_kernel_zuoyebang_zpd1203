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

#ifndef _DSI_HW_PHY_H_
#define _DSI_HW_PHY_H_

#include "dpu_hw_common.h"
#include "dpu_hw_dsi.h"

/* FPGA and ASIC common */
#define PHY_OUTPUT_RATE_MIN    (80000000UL)
#define PHY_OUTPUT_RATE_MAX    (2500000000UL)
#define PHY_CLOCK_DIV          (2)
#define PHY_QUOT_GEN           (100000)
/* (2^16) = 65536 */
#define PHY_MPLL_COE           (65536)
#define PHY_MPLL_COE_DIV       (32)
#define T_DCO_MAX              (477)
#define MULT                   (11)
#define T_LPX                  (55)
/* COMMON: LPTX_IO_SR0_FAIL_DELAY 12.5 */
#define LPTX_IO_SR0_FAIL_DELAY (1250)
#define D2A_HS_TX_DELAY_DPHY   (3)
#define D2A_HS_TX_DELAY_CPHY   (4)
#define LPTX_EN_DELAY          (5)

#define CDPHY_IS_READY         (0x0010)

enum phy_amplitude_r {
	AMPLITUDE_100MV = 0,
	AMPLITUDE_175MV = 2,
	AMPLITUDE_225MV = 3,
	AMPLITUDE_200MV = 4,
	AMPLITUDE_237MV = 7,
	AMPLITUDE_MAX = 0,
};

/* CDPHY cfg clk params, unit: MHz */
enum cfg_clk {
	CFG_CLK_19_2,
	/* FPGA(HASP) use this freq. */
	CFG_CLK_20_0,
	CFG_CLK_24_0,
	CFG_CLK_26_0,
	CFG_CLK_27_0,
	/* ASIC use this freq. */
	CFG_CLK_38_4,
};

struct phy_pll_dynamic {
	u32 phy_m_r;
	u32 phy_n_r;
	u32 vco_cntrl_r;
	u32 cpbias_cntrl_r;
	u32 int_cntrl_r;
};

struct timing {
	u32 ui;
	u32 t_esc;
	u32 t_eot;
	u32 t_eot_max;
	u32 t_hs_trail;
	/* word clock period */
	u32 t_word_clk;
	u32 cfg_clk_period;
	u32 hs_zero;
	u32 clk_zero;

	u32 hs_prepare_zero;
	u32 hs_prepare_dco;
	//u32 hs_prepare_reg;//hs_prepare_dco_reg
	u32 hs_prepare_min;
	u32 hs_prepare_max;
	u32 clk_pre_reg;
	u32 hs_trail;

	u32 hs_trail_reg;
	u32 clk_trail;
	u32 clk_trail_reg;
	u32 hs_trail_min;
	u32 hs_exit;
	u32 hs_exit_reg;
	u32 clk_prepare;
	u32 clk_prepare_reg;

	u32 clk_prepare_zero_min;

	u32 clk_prepare_min;
	u32 clk_prepare_max;

	u32 clk_trail_min;
	u32 clk_post;
	// cphy only
	u32 t3_prepare;
	u32 t3_prepare_dco;
	u32 t3_prebegin;
	u32 t3_calpremble;
	u32 t3_post;

	u32 t3_post_rx;
};

/* for CDPHY data lane and clock lane  dphy_config_t*/
struct phy_cfg {
	u32 hs_tx_3_tlptxoverlap; // D
	u32 hs_tx_13_tlptxoverlap; // C

	u32 hs_tx_10_tlpt11init_dco; // D
	u32 hs_tx_12_tlpt11init_dco; // C

	u32 hs_tx_4_tlpx_dco; // D
	u32 hs_tx_11_tlpx_dco; // C

	u32 hs_tx_9_thsprpr_dco_d; // D data lane
	u32 hs_tx_9_thsprpr_dco_c; // D clock lane
	u32 hs_tx_7_t3prpr_dco; // C

	u32 hs_tx_1_thszero_d;
	u32 hs_tx_1_thszero_c;
	u32 hs_tx_10_tprebegin; // C
	u32 hs_tx_2_tcalpreamble; // C

	u32 hs_tx_2_tclkpre; // D = 3

	u32 hs_tx_0_thstrail_d; // D data lane
	u32 hs_tx_0_thstrail_c; // D clock lane
	u32 hs_tx_1_tpost; // C

	u32 hs_tx_5_thstrail_dco_d; // D
	u32 hs_tx_5_thstrail_dco_c; // D
	u32 hs_tx_9_t3post_dco; // C

	u32 hs_tx_8_tclkpost; // D

	u32 hs_tx_6_tlp11end_dco; // D
	u32 hs_tx_8_tlp11end_dco; // C

	u32 hs_tx_12_thsexit_dco; // D
	u32 hs_tx_0_thsexit_dco; // C

	u32 hs_tx_1_tpost_rx;
	u32 hs_tx_9_t3post_dco_rx;

	u16 hibernate_dly;
};

/* For SNPS FPGA test: CDPHY Board PLL config */
struct phy_pll_static {
	u32 atb_sense_sel_r;
	u32 pll_shadow_control_r;
	u32 clksel_r;
	u32 shadow_clear_r0;
	u32 shadow_clear_r1;
	u32 shadow_clear_r2;
	u32 mpll_opmode_r;
	/* PLL_ANA_CTRL_5 */
	u32 mpll_prg_r0;
	/* PLL_ANA_CTRL_4 */
	u32 mpll_prg_r1;
	u32 th1_r;
	u32 th2_r;
	u32 th3_r;
	u32 gmp_cntrl_r;
	u32 prop_cntrl_r;
	u32 mpll_frac_quot_r;
	u32 mpll_frac_rem_r;
	u32 mpll_frac_den_r;
	u32 mpll_fracn_en_r;
	u32 mpll_ssc_en_r;
	u32 en_ssc_frac_r;
	u32 mpll_fracn_cfg_update_en_r;
	u32 onpll_r;
	u32 clksel_en_r;
	u32 updatepll_r0;
	u32 updatepll_r1;
	u32 updatepll_r2;
};

struct phy_cfg_clk{
	u16 termcal_timer;
	u16 lpcdcocal_timebase;
	u16 lpdcocal_twait_coarse;
	u16 lpdcocal_twait_fine;
	u16 pll_rst_time;
	u16 pll_gear_shift_time;
};

void dsi_hw_phy_deinit_fpga(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
int dsi_hw_phy_init_fpga(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
int dsi_hw_phy_wait_ready_fpga(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
int dsi_hw_phy_pll_config_fpga(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
void dsi_hw_phy_hibernate_exit(struct dpu_hw_blk *hw);

#endif /* _DSI_HW_PHY_H_ */
