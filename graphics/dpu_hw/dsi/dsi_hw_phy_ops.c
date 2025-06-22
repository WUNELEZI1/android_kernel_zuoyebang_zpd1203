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
#include "dsi_hw_phy.h"
#include "dsi_hw_phy_ops.h"
#include "dsi_phy_1v2_3t4l_reg.h"

#define LP_FREQ                (1740)
#define LPTX_IO_RESET_DELAY   (4)

static u32 eq_parm[16][4] = {
/* setr 00 */{2, 2, 2, 1},
/* setr 01 */{3, 2, 2, 1},
/* setr 02 */{3, 2, 2, 1},
/* setr 03 */{3, 2, 2, 1},
/* setr 04 */{3, 3, 2, 1},
/* setr 05 */{3, 3, 2, 1},
/* setr 06 */{3, 3, 2, 2},
/* setr 07 */{3, 3, 2, 2},
/* setr 08 */{4, 3, 2, 2},
/* setr 09 */{4, 3, 2, 2},
/* setr 10 */{4, 3, 2, 2},
/* setr 11 */{4, 3, 2, 2},
/* setr 12 */{4, 4, 3, 2},
/* setr 13 */{4, 4, 3, 2},
/* setr 14 */{4, 4, 3, 2},
/* setr 15 */{4, 4, 3, 2},
};

static struct phy_cfg_clk cfg_clk[6] = {
/* 19.2 */{19,  76, 23, 23, 191,  96},
/* 20.0 */{19,  79, 24, 24, 199,  99},
/* 24.0 */{23,  95, 29, 29, 239, 119},
/* 26.0 */{25, 103, 32, 32, 259, 129},
/* 27.0 */{26, 107, 33, 33, 269, 134},
/* 38.4 */{38, 153, 47, 47, 383, 191},
};

static void phy_calculate_asic(u64 lanerate, u8 phy_type, struct timing *tm)
{
	u32 d2a_hs_tx_dly;
	u32 tmp = 0;

	d2a_hs_tx_dly = (phy_type == DSI_PHY_TYPE_DPHY) ? D2A_HS_TX_DELAY_DPHY : D2A_HS_TX_DELAY_CPHY;
	tm->ui = 100000000 / (lanerate / 1000);
	/* 1000 / 38.4 */
	tm->cfg_clk_period = 2600;

	/**
	 * normal 200MHz, t_dco = 5.00, t_dco_min = 5.26,t_dco_max = 4.77
	 * clk_prepare_zero_min = 300
	 *  */

	/* tm->clk_prepare_zero_min = 300; */
	tm->hs_prepare_zero = 14500 + 10 * tm->ui;

	tm->t_word_clk = (phy_type == DSI_PHY_TYPE_CPHY) ? (tm->ui * 7) : (tm->ui * 8);

	tm->hs_exit = 110 * 100;
	tm->hs_exit_reg = DIV_ROUND_UP(tm->hs_exit, T_DCO_MAX) - 1;

	tm->clk_pre_reg = d2a_hs_tx_dly;

	/* int(38 + ((95 - 38) / 2)) * mult) = int(73.15) */
	tm->clk_prepare = 73 * 100;

	/* int((300 - 73.15) * mult)) = int(249.535) */
	tm->clk_zero = 249 * 100;

	/* 40 + 4 * tm->ui + (((85 + 6 * tm->ui) - (40 + 4 * tm->ui)) / 2) */
	tmp = (8500 + 6 * tm->ui)-(4000 + 4 * tm->ui);
	tm->hs_prepare_dco = 4000 + 4 * tm->ui + tmp / 2;

	tmp = (tm->hs_prepare_dco * 100) + LPTX_IO_SR0_FAIL_DELAY;

	tm->t_eot = 105 * 100 + 12 * tm->ui;

	tm->t_hs_trail = ((8 * tm->ui) > (6000 + 4 * tm->ui)) ?
		(8 * tm->ui) : (6000 + 4 * tm->ui);

	tm->hs_trail = tm->t_hs_trail + (tm->t_eot - tm->t_hs_trail) / 2;

	tmp = (tm->hs_trail_reg + 1) * tm->t_word_clk -
		tm->t_word_clk - 4 * T_DCO_MAX;

	/* tm->hs_trai_dco_reg = (tmp / T_DCO_MAX) - 1; */

	tm->hs_zero = (14500 + 10 * tm->ui) - tm->hs_prepare_dco;

	tm->clk_prepare_reg = 24;
	tm->clk_trail = 60 * 100 + (105 * 100 + 12 * tm->ui - 60 * 100) / 2;
	tm->clk_post = (60 * 100 + 52 * tm->ui) * MULT / 10;

	/**
	* t3_prepare_min = 38, t3_prepare_max = 95
	* t3_prepare = int((MIN + ((MAX - MIN) / 2)) * mult)
	* t3_prepare = int((38.0 + ((95.0 - 38.0) / 2)) * 1.1) = int(73.15) = 73
	 */
	tm->t3_prepare = 73 * 100;
	tm->t3_prepare_dco = 73 * 100;

	/**
	 * t3_prebegin_min = 7 * UI, t3_prebegin_max = 448 * UI
	 * t3_prebegin = (MIN + MAX-MIN) / 2 * UI
	 * t3_prebegin = int((7.0 + ((448.0 - 7.0) / 2)) * UI)
	 */
	tm->t3_prebegin =(7 + ((448 - 7) / 2)) * tm->ui;

	/**
	 * t3_calpremble_min = 7 * UI, t3_calpremble_max = 256 * 7 * UI
	 * t3_calpremble = (MIN + MAX-MIN) /2
	 * t3_calpremble_dco = int(38.0 + (95.0 - 38.0) / 2)
	 */
	tm->t3_calpremble = (7 + ((256 * 7) - 7) / 2) * tm->ui;

	/**
	 * t3_post_min = 7 * UI, t3_post_max = 224 * UI + D2A_HSTX_DLY
	 * t3_post = (MIN + MAX-MIN) /2
	 * t3_post_dco = int(38.0 + (95.0 - 38.0) / 2)
	 */
	tm->t3_post = (7 + (224 - 7) / 2) * tm->ui;
	tm->t3_post_rx = ((7 + (224 - 7) / 2)) * tm->ui * 2;

	DSI_DEBUG("ui %d\n",tm->ui);
	DSI_DEBUG("hs_exit_reg %d\n",tm->hs_exit_reg);
	DSI_DEBUG("hs_prepare_zero %d\n",tm->hs_prepare_zero);
	DSI_DEBUG("t_word_clk %d\n",tm->t_word_clk);
	DSI_DEBUG("t_eot %d\n",tm->t_eot);
	DSI_DEBUG("t_hs_trail %d\n",tm->t_hs_trail);
	DSI_DEBUG("hs_prepare_dco %d\n",tm->hs_prepare_dco);
	DSI_DEBUG("clk_prepare %d\n",tm->clk_prepare);
	DSI_DEBUG("clk_zero %d\n",tm->clk_zero);
	DSI_DEBUG("hs_zero %d\n",tm->hs_zero);
	DSI_DEBUG("clk_trail %d\n",tm->clk_trail);
	DSI_DEBUG("hs_trail %d\n",tm->hs_trail);
	DSI_DEBUG("hs_trail_reg %d\n",tm->hs_trail_reg);
	DSI_DEBUG("clk_post %d\n",tm->clk_post);
	DSI_DEBUG("t3_prepare_dco %d\n",tm->t3_prepare_dco);
	DSI_DEBUG("t3_prebegin %d\n",tm->t3_prebegin);
	DSI_DEBUG("t3_calpremble %d\n",tm->t3_calpremble);
	DSI_DEBUG("t3_post %d\n",tm->t3_post);

}

static void phy_parameter_asic(u64 lanerate, u8 phy_type, struct phy_cfg *phy)
{
	u32 tlptxoverlap, tlpt11init_dco, tlp11end_dco;
	u32 tlpx_dco, thsexit_dco;
	struct timing tm = {0};
	int tmp = 0;
	int tmp1 = 0;
	phy_calculate_asic(lanerate, phy_type, &tm);

	/**
	 * T_DCO_MAX = 4.77 ns, for 209.64 MHz
	 * tlptxoverlap_reg: LPTX_EN_DELAY / T_DCO_MAX
	 * 5 / 4.77 = 1.048, round_up : 2
	 */
	tlptxoverlap =	DIV_ROUND_UP((LPTX_EN_DELAY * 100), T_DCO_MAX);

	/**
	 * hx_tx_10_tlpt11init_dco = 5 * T_ESC / T_DCO - 1
	 * hs_tx_6_tlp11end_dco = 5 * T_ESC / T_DCO - 1
	 * (5 * 1000000) / (LP_FREQ * 1000) = 250
	 * LP_FREQ: = 15, for FPGA test
	 * hx_tx_10_tlpt11init_dco = 69
	 */
	tmp = (5 * 100000000) / (LP_FREQ * 1000) * 100;
	/* tlpt11init_dco: 5 * T_ESC / T_DCO - 1 = 69 */
	tlpt11init_dco = DIV_ROUND_UP(tmp, T_DCO_MAX) - 1;
	/* tlp11end_dco: 5 * T_ESC / T_DCO - 1 = 69 */
	tlp11end_dco = DIV_ROUND_UP(tmp, T_DCO_MAX) - 1;

	/* tlpx_dco = (T_LPX / LPDCO clock period) - 1, T_LPX = 50 * mult = 50 * 1.1 = 55 */
	tlpx_dco = DIV_ROUND_UP(T_LPX * 100, T_DCO_MAX) - 1;

	/* (5 * T_ESC / T_DCO) - 1, (T_HS-EXIT / LPDCO clock period) - 1 */
	thsexit_dco = DIV_ROUND_UP(tm.hs_exit, T_DCO_MAX) - 1;

	/************ DPHY ****************************************************/

	phy->hs_tx_3_tlptxoverlap = tlptxoverlap;
	phy->hs_tx_10_tlpt11init_dco = tlpt11init_dco;
	phy->hs_tx_6_tlp11end_dco = tlp11end_dco;
	phy->hs_tx_4_tlpx_dco = tlpx_dco;
	phy->hs_tx_12_thsexit_dco = thsexit_dco;

	/* ((LPTX_IO_SR0_FAIL_DELAY + T_CLK/HS_PREPARE) / LPDCO clock period) */

	tmp = LPTX_IO_SR0_FAIL_DELAY + tm.hs_prepare_dco;
	phy->hs_tx_9_thsprpr_dco_d = DIV_ROUND_UP(tmp, T_DCO_MAX) - 1;

	tmp = LPTX_IO_SR0_FAIL_DELAY + tm.clk_prepare;
	phy->hs_tx_9_thsprpr_dco_c = DIV_ROUND_UP(tmp, T_DCO_MAX) - 1;

	/**
	 * ((T_LPX + T_CLK/HS-PREPARE + T_CLK/HS-ZERO +
	 * 5 * LPDCO clock period - 3 * word clock period) /
	 * word clock period) - 1
	 */
	tmp = T_LPX * 100  + tm.hs_prepare_dco + tm.hs_zero + 5 * T_DCO_MAX - 3 * tm.t_word_clk;
	tmp1 = DIV_ROUND_UP(tmp, tm.t_word_clk) - 1;
	phy->hs_tx_1_thszero_d =  (tmp1 < 0) ? 0 : tmp1;

	tmp = T_LPX * 100 + tm.clk_prepare + tm.clk_zero + 5 * T_DCO_MAX - 3 * tm.t_word_clk;
	tmp1 = DIV_ROUND_UP(tmp, tm.t_word_clk) - 1;

	phy->hs_tx_1_thszero_c = (tmp1 < 0) ? 0 : tmp1;

	/* CDPHY board is static */
	phy->hs_tx_2_tclkpre = 3;
	/* (T_CLK/HS-TRAIL / word clock period) - 1 + D2A_HS_TX delay */
	phy->hs_tx_0_thstrail_d = DIV_ROUND_UP(tm.hs_trail, tm.t_word_clk) - 1 + D2A_HS_TX_DELAY_DPHY;
	phy->hs_tx_0_thstrail_c = DIV_ROUND_UP(tm.clk_trail, tm.t_word_clk) - 1 + D2A_HS_TX_DELAY_DPHY;
	/**
	 * ((D2A_HS_TX delay * word clock period - T_CLK_HS_TRAIL
	 * 1 * word clock period - 4 * LPDCO clock period) / LPDCO clock period)
	 * - 1
	 * use 0, if negative
	 */
	tmp = (D2A_HS_TX_DELAY_DPHY * tm.t_word_clk) + tm.hs_trail - tm.t_word_clk - 4 * T_DCO_MAX;
	phy->hs_tx_5_thstrail_dco_d = (tmp / T_DCO_MAX) - 1;

	tmp = (D2A_HS_TX_DELAY_DPHY * tm.t_word_clk) + tm.clk_trail - tm.t_word_clk - 4 * T_DCO_MAX;
	phy->hs_tx_5_thstrail_dco_c = (tmp / T_DCO_MAX) - 1;

	/* (T_CLK-POST /  word clock period) - 3 */
	phy->hs_tx_8_tclkpost = DIV_ROUND_UP(tm.clk_post, tm.t_word_clk) - 3;
	/* (5 * T_ESC / T_DCO) - 1 */
	/* (T_HS-EXIT / LPDCO clock period) - 1 */
	phy->hs_tx_12_thsexit_dco = DIV_ROUND_UP(tm.hs_exit, T_DCO_MAX) - 1;

	if (phy_type == DSI_PHY_TYPE_DPHY) {
		DSI_DEBUG("CORE_DIG_DLANE_CLK_RW_HS_TX_ INFO\n");
		DSI_DEBUG("hs_tx_3_tlptxoverlap %d\n", phy->hs_tx_3_tlptxoverlap);
		DSI_DEBUG("hx_tx_10_tlpt11init_dco %d\n", phy->hs_tx_10_tlpt11init_dco);
		DSI_DEBUG("hs_tx_4_tlpx_dco %d\n", phy->hs_tx_4_tlpx_dco);
		DSI_DEBUG("hs_tx_9_thsprpr_dco_c %d\n", phy->hs_tx_9_thsprpr_dco_c);
		DSI_DEBUG("hs_tx_1_thszero_c %d\n", phy->hs_tx_1_thszero_c);
		DSI_DEBUG("hs_tx_2_tclkpre]: %d\n", phy->hs_tx_2_tclkpre);
		DSI_DEBUG("hs_tx_0_thstrail_c %d\n", phy->hs_tx_0_thstrail_c);
		DSI_DEBUG("hs_tx_5_thstrail_dco_c %d\n", phy->hs_tx_5_thstrail_dco_c);
		DSI_DEBUG("hs_tx_8_tclkpost %d\n", phy->hs_tx_8_tclkpost);
		DSI_DEBUG("hs_tx_6_tlp11end_dco %d\n", phy->hs_tx_6_tlp11end_dco);
		DSI_DEBUG("hs_tx_12_thsexit_dco %d\n", phy->hs_tx_12_thsexit_dco);
		DSI_DEBUG("CORE_DIG_DLANE_0/1/2/3_RW_HS_TX_ INFO\n");
		DSI_DEBUG("hs_tx_3_tlptxoverlap %d\n", phy->hs_tx_3_tlptxoverlap);
		DSI_DEBUG("hx_tx_10_tlpt11init_dco %d\n", phy->hs_tx_10_tlpt11init_dco);
		DSI_DEBUG("hs_tx_4_tlpx_dco %d\n", phy->hs_tx_4_tlpx_dco);
		DSI_DEBUG("hs_tx_9_thsprpr_dco_c %d\n", phy->hs_tx_9_thsprpr_dco_c);
		DSI_DEBUG("hs_tx_1_thszero_d %d\n", phy->hs_tx_1_thszero_d);
		DSI_DEBUG("hs_tx_0_thstrail_d %d\n", phy->hs_tx_0_thstrail_d);
		DSI_DEBUG("hs_tx_5_thstrail_dco_d %d\n", phy->hs_tx_5_thstrail_dco_d);
		DSI_DEBUG("hs_tx_6_tlp11end_dco %d\n", phy->hs_tx_6_tlp11end_dco);
		DSI_DEBUG("hs_tx_12_thsexit_dco %d\n", phy->hs_tx_12_thsexit_dco);
	}

	/************ CPHY ****************************************************/
	phy->hs_tx_13_tlptxoverlap   = tlptxoverlap;
	phy->hs_tx_12_tlpt11init_dco = tlpt11init_dco;
	phy->hs_tx_8_tlp11end_dco    = tlp11end_dco;
	phy->hs_tx_11_tlpx_dco       = tlpx_dco;
	phy->hs_tx_0_thsexit_dco     = thsexit_dco;

	/**
	 * ((T_LPX + T_CLK/HS-PREPARE + T_CLK/HS-ZERO +
	 * 5 * LPDCO clock period - 3 * word clock period) /
	 * word clock period) - 1
	 */
	tmp = T_LPX * 100 + tm.t3_prepare_dco + tm.t3_prebegin + 5 * T_DCO_MAX - 3 * tm.t_word_clk;
	phy->hs_tx_10_tprebegin = DIV_ROUND_UP(tmp, tm.t_word_clk) - 1;

	tmp = T_LPX * 100  + tm.t3_prepare_dco + tm.t3_calpremble + 5 * T_DCO_MAX - 3 * tm.t_word_clk;
	phy->hs_tx_2_tcalpreamble = DIV_ROUND_UP(tmp, tm.t_word_clk) - 1;

	phy->hs_tx_1_tpost = DIV_ROUND_UP(tm.t3_post, tm.t_word_clk) - 1 + D2A_HS_TX_DELAY_CPHY;
	phy->hs_tx_1_tpost_rx = DIV_ROUND_UP(tm.t3_post_rx, tm.t_word_clk) - 1 + D2A_HS_TX_DELAY_CPHY;
	/**
	 * ((D2A_HS_TX delay * word clock period - T_CLK_HS_TRAIL
	 * 1 * word clock period - 4 * LPDCO clock period) / LPDCO clock period)
	 * - 1
	 * use 0, if negative
	 */
	tmp = ((phy->hs_tx_1_tpost + 1) * tm.t_word_clk - tm.t_word_clk - 3 * T_DCO_MAX) / T_DCO_MAX - 1;

	tmp1 = (LPTX_IO_RESET_DELAY  * 100) / T_DCO_MAX + 1; // 8

	phy->hs_tx_9_t3post_dco = (tmp >= tmp1) ? tmp : tmp1;

	tmp = ((phy->hs_tx_1_tpost_rx + 1) * tm.t_word_clk - tm.t_word_clk - 3 * T_DCO_MAX) / T_DCO_MAX - 1;

	tmp1 = (LPTX_IO_RESET_DELAY  * 100) / T_DCO_MAX + 1; // 8

	phy->hs_tx_9_t3post_dco_rx = (tmp >= tmp1) ? tmp : tmp1;

	/* ((LPTX_IO_SR0_FAIL_DELAY + t3-prepare) / LPDCO clock period) - 1 */
	tmp = LPTX_IO_SR0_FAIL_DELAY + tm.t3_prepare_dco;
	phy->hs_tx_7_t3prpr_dco = DIV_ROUND_UP(tmp, T_DCO_MAX) - 1;

	/************ common ****************************************************/
	tmp = ((65 * tm.t_word_clk / 10) > (6 * tm.cfg_clk_period)) ? (65 * tm.t_word_clk / 10) - (6 * tm.cfg_clk_period) : 0;
	tmp1 = DIV_ROUND_UP(tmp, tm.cfg_clk_period);

	phy->hibernate_dly = (tmp1 > 1) ? tmp1 : 1;

	if (phy_type == DSI_PHY_TYPE_CPHY) {
		DSI_DEBUG("CORE_DIG_CLANE_0/1/2_RW_HS_TX_ INFO\n");
		DSI_DEBUG("hs_tx_13_tlptxoverlap %d\n", phy->hs_tx_13_tlptxoverlap);
		DSI_DEBUG("hs_tx_12_tlpt11init_dco %d\n", phy->hs_tx_12_tlpt11init_dco);
		DSI_DEBUG("hs_tx_11_tlpx_dco %d\n", phy->hs_tx_11_tlpx_dco);
		DSI_DEBUG("hs_tx_7_t3prpr_dco %d\n", phy->hs_tx_7_t3prpr_dco);
		DSI_DEBUG("hs_tx_10_tprebegin %d\n", phy->hs_tx_10_tprebegin);
		DSI_DEBUG("hs_tx_1_tpost %d\n", phy->hs_tx_1_tpost);
		DSI_DEBUG("hs_tx_2_tcalpreamble %d\n", phy->hs_tx_2_tcalpreamble);
		DSI_DEBUG("hs_tx_8_tlp11end_dco %d\n", phy->hs_tx_8_tlp11end_dco);
		DSI_DEBUG("hs_tx_9_t3post_dco %d\n", phy->hs_tx_9_t3post_dco);
		DSI_DEBUG("hs_tx_0_thsexit_dco %d\n", phy->hs_tx_0_thsexit_dco);
		DSI_DEBUG("======== t3_post settings (Loopback) ======== \n");
		DSI_DEBUG("hs_tx_1_tpost_rx]: %d\n", phy->hs_tx_1_tpost_rx);
		DSI_DEBUG("hs_tx_9_t3post_dco_rx]: %d\n", phy->hs_tx_9_t3post_dco_rx);
	}
	DSI_DEBUG("hibernate_dly %d\n", phy->hibernate_dly);
}

void dsi_hw_phy_set_eq_debug(struct dpu_hw_blk *hw, u8 setr, u8 eqa, u8 eqb)
{
	DSI_DEBUG(" dsi debug: hstx_eq eqa %d, eqb %d,\n", eqa, eqb);

	DPU_BITMASK_WRITE(hw, PHY_0X1023_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_3,
		eqa, PHY_0X1023_LANE0_HSTX_EQA_SHIFT,
		PHY_0X1023_LANE0_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1223_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_3,
		eqa, PHY_0X1223_LANE1_HSTX_EQA_SHIFT,
		PHY_0X1223_LANE1_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1423_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_3,
		eqa, PHY_0X1423_LANE2_HSTX_EQA_SHIFT,
		PHY_0X1423_LANE2_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1623_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_3,
		eqa, PHY_0X1623_LANE3_HSTX_EQA_SHIFT,
		PHY_0X1623_LANE3_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1823_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_3,
		eqa, PHY_0X1823_LANE4_HSTX_EQA_SHIFT,
		PHY_0X1823_LANE4_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1024_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_4,
		eqb, PHY_0X1024_LANE0_HSTX_EQB_SHIFT,
		PHY_0X1024_LANE0_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1224_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_4,
		eqb, PHY_0X1224_LANE1_HSTX_EQB_SHIFT,
		PHY_0X1224_LANE1_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1424_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_4,
		eqb, PHY_0X1424_LANE2_HSTX_EQB_SHIFT,
		PHY_0X1424_LANE2_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1624_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_4,
		eqb, PHY_0X1624_LANE3_HSTX_EQB_SHIFT,
		PHY_0X1624_LANE3_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1824_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_4,
		eqb, PHY_0X1824_LANE4_HSTX_EQB_SHIFT,
		PHY_0X1824_LANE4_HSTX_EQB_MASK, DIRECT_WRITE);
}

static void dsi_hw_phy_set_eq(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	u8 hstx_eqa, hstx_eqb, setr;
	u64 hs_speed;

	setr = cfg->phy_parms.phy_oa_setr;
	hs_speed = cfg->phy_parms.phy_hs_speed;

	if (setr != 0) {
		if (hs_speed >= 3500000000) {
			hstx_eqa = eq_parm[setr][0];
			hstx_eqb = eq_parm[setr][1];
		} else if (hs_speed >= 2500000000) {
			hstx_eqa = eq_parm[setr][2];
			hstx_eqb = eq_parm[setr][3];
		} else {
			hstx_eqa = 0;
			hstx_eqb = 0;
		}
	} else {
		hstx_eqa = cfg->phy_parms.phy_eqa;
		hstx_eqb = cfg->phy_parms.phy_eqb;
	}

	DSI_DEBUG("oa_setr %d, eqa %d, eqb %d\n", setr, hstx_eqa, hstx_eqb);

	DPU_BITMASK_WRITE(hw, PHY_0X1023_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_3,
		hstx_eqa, PHY_0X1023_LANE0_HSTX_EQA_SHIFT,
		PHY_0X1023_LANE0_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1223_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_3,
		hstx_eqa, PHY_0X1223_LANE1_HSTX_EQA_SHIFT,
		PHY_0X1223_LANE1_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1423_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_3,
		hstx_eqa, PHY_0X1423_LANE2_HSTX_EQA_SHIFT,
		PHY_0X1423_LANE2_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1623_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_3,
		hstx_eqa, PHY_0X1623_LANE3_HSTX_EQA_SHIFT,
		PHY_0X1623_LANE3_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1823_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_3,
		hstx_eqa, PHY_0X1823_LANE4_HSTX_EQA_SHIFT,
		PHY_0X1823_LANE4_HSTX_EQA_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1024_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_4,
		hstx_eqb, PHY_0X1024_LANE0_HSTX_EQB_SHIFT,
		PHY_0X1024_LANE0_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1224_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_4,
		hstx_eqb, PHY_0X1224_LANE1_HSTX_EQB_SHIFT,
		PHY_0X1224_LANE1_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1424_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_4,
		hstx_eqb, PHY_0X1424_LANE2_HSTX_EQB_SHIFT,
		PHY_0X1424_LANE2_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1624_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_4,
		hstx_eqb, PHY_0X1624_LANE3_HSTX_EQB_SHIFT,
		PHY_0X1624_LANE3_HSTX_EQB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1824_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_4,
		hstx_eqb, PHY_0X1824_LANE4_HSTX_EQB_SHIFT,
		PHY_0X1824_LANE4_HSTX_EQB_MASK, DIRECT_WRITE);
}

static void dphy_hw_tx_cfg(struct dpu_hw_blk *hw,
		u64 lanerate, struct dsi_ctrl_cfg *cfg)
{
	u8 phy_amplitude = cfg->phy_parms.phy_amplitude;
	struct phy_cfg dphy = {0};

	switch (cfg->phy_parms.phy_amplitude) {
	case PHY_AMPLITUDE_100MV:
		phy_amplitude = AMPLITUDE_100MV;
		break;
	case PHY_AMPLITUDE_175MV:
		phy_amplitude = AMPLITUDE_175MV;
		break;
	case PHY_AMPLITUDE_225MV:
		phy_amplitude = AMPLITUDE_225MV;
		break;
	case PHY_AMPLITUDE_200MV:
		phy_amplitude = AMPLITUDE_200MV;
		break;
	case PHY_AMPLITUDE_237MV:
		phy_amplitude = AMPLITUDE_237MV;
		break;
	default:
		phy_amplitude = AMPLITUDE_225MV;
		break;
	}

	DSI_DEBUG("amplitude %d %d\n",
		phy_amplitude, cfg->phy_parms.phy_amplitude);

	phy_parameter_asic(lanerate, DSI_PHY_TYPE_DPHY, &dphy);

	DPU_BITMASK_WRITE(hw, PHY_0X3040_CORE_DIG_DLANE_0_RW_LP_0,
		7, PHY_0X3040_LP_0_TTAGO_REG_SHIFT,
		PHY_0X3040_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3240_CORE_DIG_DLANE_1_RW_LP_0,
		7, PHY_0X3240_LP_0_TTAGO_REG_SHIFT,
		PHY_0X3240_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3440_CORE_DIG_DLANE_2_RW_LP_0,
		7, PHY_0X3440_LP_0_TTAGO_REG_SHIFT,
		PHY_0X3440_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3640_CORE_DIG_DLANE_3_RW_LP_0,
		7, PHY_0X3640_LP_0_TTAGO_REG_SHIFT,
		PHY_0X3640_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1022_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_2,
		0, PHY_0X1022_LANE0_SEL_LANE_CFG_SHIFT,
		PHY_0X1022_LANE0_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1222_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_2,
		0, PHY_0X1222_LANE1_SEL_LANE_CFG_SHIFT,
		PHY_0X1222_LANE1_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1422_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_2,
		1, PHY_0X1422_LANE2_SEL_LANE_CFG_SHIFT,
		PHY_0X1422_LANE2_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1622_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_2,
		0, PHY_0X1622_LANE3_SEL_LANE_CFG_SHIFT,
		PHY_0X1622_LANE3_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1822_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_2,
		0, PHY_0X1822_LANE4_SEL_LANE_CFG_SHIFT,
		PHY_0X1822_LANE4_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C24_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_4,
		phy_amplitude, PHY_0X1C24_OA_CB_SEL_VCOMM_PROG_SHIFT,
		PHY_0X1C24_OA_CB_SEL_VCOMM_PROG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1023_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_3,
		1, PHY_0X1023_LANE0_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1023_LANE0_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1223_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_3,
		1, PHY_0X1223_LANE1_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1223_LANE1_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1423_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_3,
		0, PHY_0X1423_LANE2_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1423_LANE2_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1623_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_3,
		1, PHY_0X1623_LANE3_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1623_LANE3_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1823_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_3,
		1, PHY_0X1823_LANE4_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1823_LANE4_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3103_CORE_DIG_DLANE_0_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3103_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3103_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3303_CORE_DIG_DLANE_1_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3303_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3303_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3503_CORE_DIG_DLANE_2_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3503_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3503_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3703_CORE_DIG_DLANE_3_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3703_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3703_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X310A_CORE_DIG_DLANE_0_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X310A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X310A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X330A_CORE_DIG_DLANE_1_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X330A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X330A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X350A_CORE_DIG_DLANE_2_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X350A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X350A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X370A_CORE_DIG_DLANE_3_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X370A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X370A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3104_CORE_DIG_DLANE_0_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3104_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3104_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3304_CORE_DIG_DLANE_1_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3304_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3304_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3504_CORE_DIG_DLANE_2_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3504_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3504_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3704_CORE_DIG_DLANE_3_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3704_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3704_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3109_CORE_DIG_DLANE_0_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3109_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3109_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3309_CORE_DIG_DLANE_1_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3309_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3309_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3509_CORE_DIG_DLANE_2_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3509_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3509_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3709_CORE_DIG_DLANE_3_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3709_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3709_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3101_CORE_DIG_DLANE_0_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3101_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3101_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3301_CORE_DIG_DLANE_1_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3301_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3301_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3501_CORE_DIG_DLANE_2_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3501_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3501_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3701_CORE_DIG_DLANE_3_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3701_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3701_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3100_CORE_DIG_DLANE_0_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3100_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3100_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3300_CORE_DIG_DLANE_1_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3300_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3300_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3500_CORE_DIG_DLANE_2_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3500_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3500_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3700_CORE_DIG_DLANE_3_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3700_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3700_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3105_CORE_DIG_DLANE_0_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3105_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3105_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3305_CORE_DIG_DLANE_1_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3305_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3305_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3505_CORE_DIG_DLANE_2_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3505_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3505_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3705_CORE_DIG_DLANE_3_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3705_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3705_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3106_CORE_DIG_DLANE_0_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3106_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3106_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3306_CORE_DIG_DLANE_1_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3306_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3306_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3506_CORE_DIG_DLANE_2_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3506_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3506_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3706_CORE_DIG_DLANE_3_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3706_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3706_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X310C_CORE_DIG_DLANE_0_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X310C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X310C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X330C_CORE_DIG_DLANE_1_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X330C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X330C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X350C_CORE_DIG_DLANE_2_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X350C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X350C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X370C_CORE_DIG_DLANE_3_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X370C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X370C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3903_CORE_DIG_DLANE_CLK_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3903_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3903_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X390A_CORE_DIG_DLANE_CLK_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X390A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X390A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3904_CORE_DIG_DLANE_CLK_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3904_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3904_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3909_CORE_DIG_DLANE_CLK_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_c,
		PHY_0X3909_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3909_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3901_CORE_DIG_DLANE_CLK_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_c,
		PHY_0X3901_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3901_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3902_CORE_DIG_DLANE_CLK_RW_HS_TX_2,
		3, /* hs_tx_2_tclkpre_reg */
		PHY_0X3902_HS_TX_2_TCLKPRE_REG_SHIFT,
		PHY_0X3902_HS_TX_2_TCLKPRE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3900_CORE_DIG_DLANE_CLK_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_c,
		PHY_0X3900_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3900_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3905_CORE_DIG_DLANE_CLK_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_c,
		PHY_0X3905_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3905_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3908_CORE_DIG_DLANE_CLK_RW_HS_TX_8,
		dphy.hs_tx_8_tclkpost,
		PHY_0X3908_HS_TX_8_TCLKPOST_REG_SHIFT,
		PHY_0X3908_HS_TX_8_TCLKPOST_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3906_CORE_DIG_DLANE_CLK_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3906_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3906_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X390C_CORE_DIG_DLANE_CLK_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X390C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X390C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3040_CORE_DIG_DLANE_0_RW_LP_0,
		1, PHY_0X3040_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X3040_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3240_CORE_DIG_DLANE_1_RW_LP_0,
		1, PHY_0X3240_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X3240_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3440_CORE_DIG_DLANE_2_RW_LP_0,
		1, PHY_0X3440_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X3440_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3640_CORE_DIG_DLANE_3_RW_LP_0,
		1, PHY_0X3640_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X3640_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	/* LP_2_FILTER_INPUT_SAMPLING_REG */
	DPU_REG_WRITE(hw, PHY_0X3042_CORE_DIG_DLANE_0_RW_LP_2, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3242_CORE_DIG_DLANE_1_RW_LP_2, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3442_CORE_DIG_DLANE_2_RW_LP_2, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3642_CORE_DIG_DLANE_3_RW_LP_2, 0, DIRECT_WRITE);


	DPU_BITMASK_WRITE(hw, PHY_0X3840_CORE_DIG_DLANE_CLK_RW_LP_0,
		1, PHY_0X3840_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X3840_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);
	/* LP_0_ITMINRX_REG */
	DPU_REG_WRITE(hw, PHY_0X3842_CORE_DIG_DLANE_CLK_RW_LP_2,
		0, DIRECT_WRITE);

	/*
	DPU_REG_WRITE(hw, PHY_0X3000_CORE_DIG_DLANE_0_RW_CFG_0, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3200_CORE_DIG_DLANE_1_RW_CFG_0, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3400_CORE_DIG_DLANE_2_RW_CFG_0, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3600_CORE_DIG_DLANE_3_RW_CFG_0, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X3800_CORE_DIG_DLANE_CLK_RW_CFG_0, 0,
		DIRECT_WRITE);
	*/

	dsi_hw_phy_set_eq(hw, cfg);
}

void cphy_hw_tx_cfg(struct dpu_hw_blk *hw, u32 lanerate)
{
	struct phy_cfg cphy = {0};
	u16 value = 0;

	phy_parameter_asic(lanerate, DSI_PHY_TYPE_CPHY, &cphy);

	DPU_BITMASK_WRITE(hw, PHY_0X5040_CORE_DIG_CLANE_0_RW_LP_0,
		7, PHY_0X5040_LP_0_TTAGO_REG_SHIFT,
		PHY_0X5040_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5240_CORE_DIG_CLANE_1_RW_LP_0,
		7, PHY_0X5240_LP_0_TTAGO_REG_SHIFT,
		PHY_0X5240_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5440_CORE_DIG_CLANE_2_RW_LP_0,
		7, PHY_0X5440_LP_0_TTAGO_REG_SHIFT,
		PHY_0X5440_LP_0_TTAGO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1022_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_2,
		1, PHY_0X1022_LANE0_SEL_LANE_CFG_SHIFT,
		PHY_0X1022_LANE0_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1222_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_2,
		0, PHY_0X1222_LANE1_SEL_LANE_CFG_SHIFT,
		PHY_0X1222_LANE1_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1422_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_2,
		1, PHY_0X1422_LANE2_SEL_LANE_CFG_SHIFT,
		PHY_0X1422_LANE2_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1622_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_2,
		1, PHY_0X1622_LANE3_SEL_LANE_CFG_SHIFT,
		PHY_0X1622_LANE3_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1822_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_2,
		0, PHY_0X1822_LANE4_SEL_LANE_CFG_SHIFT,
		PHY_0X1822_LANE4_SEL_LANE_CFG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C24_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_4,
		3, PHY_0X1C24_OA_CB_SEL_VCOMM_PROG_SHIFT,
		PHY_0X1C24_OA_CB_SEL_VCOMM_PROG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1023_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_3,
		0, PHY_0X1023_LANE0_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1023_LANE0_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1223_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_3,
		0, PHY_0X1223_LANE1_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1223_LANE1_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1423_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_3,
		0, PHY_0X1423_LANE2_HSTX_SEL_PHASE0_SHIFT,
		PHY_0X1423_LANE2_HSTX_SEL_PHASE0_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5100_CORE_DIG_CLANE_0_RW_HS_TX_0,
		cphy.hs_tx_0_thsexit_dco, //23,
		PHY_0X5100_HS_TX_0_THSEXIT_DCO_REG_SHIFT,
		PHY_0X5100_HS_TX_0_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5300_CORE_DIG_CLANE_1_RW_HS_TX_0,
		cphy.hs_tx_0_thsexit_dco, //23,
		PHY_0X5300_HS_TX_0_THSEXIT_DCO_REG_SHIFT,
		PHY_0X5300_HS_TX_0_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5500_CORE_DIG_CLANE_2_RW_HS_TX_0,
		cphy.hs_tx_0_thsexit_dco, //23,
		PHY_0X5500_HS_TX_0_THSEXIT_DCO_REG_SHIFT,
		PHY_0X5500_HS_TX_0_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X510D_CORE_DIG_CLANE_0_RW_HS_TX_13,
		cphy.hs_tx_13_tlptxoverlap, //2,
		PHY_0X510D_HS_TX_13_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X510D_HS_TX_13_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X530D_CORE_DIG_CLANE_1_RW_HS_TX_13,
		cphy.hs_tx_13_tlptxoverlap, //2,
		PHY_0X530D_HS_TX_13_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X530D_HS_TX_13_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X550D_CORE_DIG_CLANE_2_RW_HS_TX_13,
		cphy.hs_tx_13_tlptxoverlap, //2,
		PHY_0X550D_HS_TX_13_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X550D_HS_TX_13_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X510C_CORE_DIG_CLANE_0_RW_HS_TX_12,
		cphy.hs_tx_12_tlpt11init_dco, //52,
		PHY_0X510C_HS_TX_12_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X510C_HS_TX_12_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X530C_CORE_DIG_CLANE_1_RW_HS_TX_12,
		cphy.hs_tx_12_tlpt11init_dco, //52,
		PHY_0X530C_HS_TX_12_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X530C_HS_TX_12_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X550C_CORE_DIG_CLANE_2_RW_HS_TX_12,
		cphy.hs_tx_12_tlpt11init_dco, //52,
		PHY_0X550C_HS_TX_12_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X550C_HS_TX_12_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X510B_CORE_DIG_CLANE_0_RW_HS_TX_11,
		cphy.hs_tx_11_tlpx_dco, //11,
		PHY_0X510B_HS_TX_11_TLPX_DCO_REG_SHIFT,
		PHY_0X510B_HS_TX_11_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X530B_CORE_DIG_CLANE_1_RW_HS_TX_11,
		cphy.hs_tx_11_tlpx_dco, //11,
		PHY_0X530B_HS_TX_11_TLPX_DCO_REG_SHIFT,
		PHY_0X530B_HS_TX_11_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X550B_CORE_DIG_CLANE_2_RW_HS_TX_11,
		cphy.hs_tx_11_tlpx_dco, //11,
		PHY_0X550B_HS_TX_11_TLPX_DCO_REG_SHIFT,
		PHY_0X550B_HS_TX_11_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5107_CORE_DIG_CLANE_0_RW_HS_TX_7,
		cphy.hs_tx_7_t3prpr_dco, //17,
		PHY_0X5107_HS_TX_7_T3PRPR_DCO_REG_SHIFT,
		PHY_0X5107_HS_TX_7_T3PRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5307_CORE_DIG_CLANE_1_RW_HS_TX_7,
		cphy.hs_tx_7_t3prpr_dco, //17,
		PHY_0X5307_HS_TX_7_T3PRPR_DCO_REG_SHIFT,
		PHY_0X5307_HS_TX_7_T3PRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5507_CORE_DIG_CLANE_2_RW_HS_TX_7,
		cphy.hs_tx_7_t3prpr_dco, //17,
		PHY_0X5507_HS_TX_7_T3PRPR_DCO_REG_SHIFT,
		PHY_0X5507_HS_TX_7_T3PRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X510A_CORE_DIG_CLANE_0_RW_HS_TX_10,
		cphy.hs_tx_10_tprebegin,
		PHY_0X510A_HS_TX_10_TPREBEGIN_REG_SHIFT,
		PHY_0X510A_HS_TX_10_TPREBEGIN_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X530A_CORE_DIG_CLANE_1_RW_HS_TX_10,
		cphy.hs_tx_10_tprebegin,
		PHY_0X530A_HS_TX_10_TPREBEGIN_REG_SHIFT,
		PHY_0X530A_HS_TX_10_TPREBEGIN_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X550A_CORE_DIG_CLANE_2_RW_HS_TX_10,
		cphy.hs_tx_10_tprebegin,
		PHY_0X550A_HS_TX_10_TPREBEGIN_REG_SHIFT,
		PHY_0X550A_HS_TX_10_TPREBEGIN_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5101_CORE_DIG_CLANE_0_RW_HS_TX_1,
		cphy.hs_tx_1_tpost, //20,
		PHY_0X5101_HS_TX_1_TPOST_REG_SHIFT,
		PHY_0X5101_HS_TX_1_TPOST_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5301_CORE_DIG_CLANE_1_RW_HS_TX_1,
		cphy.hs_tx_1_tpost, //20,
		PHY_0X5301_HS_TX_1_TPOST_REG_SHIFT,
		PHY_0X5301_HS_TX_1_TPOST_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5501_CORE_DIG_CLANE_2_RW_HS_TX_1,
		cphy.hs_tx_1_tpost, //20,
		PHY_0X5501_HS_TX_1_TPOST_REG_SHIFT,
		PHY_0X5501_HS_TX_1_TPOST_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5102_CORE_DIG_CLANE_0_RW_HS_TX_2,
		cphy.hs_tx_2_tcalpreamble,
		PHY_0X5102_HS_TX_2_TCALPREAMBLE_REG_SHIFT,
		PHY_0X5102_HS_TX_2_TCALPREAMBLE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5302_CORE_DIG_CLANE_1_RW_HS_TX_2,
		cphy.hs_tx_2_tcalpreamble,
		PHY_0X5302_HS_TX_2_TCALPREAMBLE_REG_SHIFT,
		PHY_0X5302_HS_TX_2_TCALPREAMBLE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5502_CORE_DIG_CLANE_2_RW_HS_TX_2,
		cphy.hs_tx_2_tcalpreamble,
		PHY_0X5502_HS_TX_2_TCALPREAMBLE_REG_SHIFT,
		PHY_0X5502_HS_TX_2_TCALPREAMBLE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5108_CORE_DIG_CLANE_0_RW_HS_TX_8,
		cphy.hs_tx_8_tlp11end_dco, //52,
		PHY_0X5108_HS_TX_8_TLP11END_DCO_REG_SHIFT,
		PHY_0X5108_HS_TX_8_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5308_CORE_DIG_CLANE_1_RW_HS_TX_8,
		cphy.hs_tx_8_tlp11end_dco, //52,
		PHY_0X5308_HS_TX_8_TLP11END_DCO_REG_SHIFT,
		PHY_0X5308_HS_TX_8_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5508_CORE_DIG_CLANE_2_RW_HS_TX_8,
		cphy.hs_tx_8_tlp11end_dco, //52,
		PHY_0X5508_HS_TX_8_TLP11END_DCO_REG_SHIFT,
		PHY_0X5508_HS_TX_8_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5109_CORE_DIG_CLANE_0_RW_HS_TX_9,
		cphy.hs_tx_9_t3post_dco,
		PHY_0X5109_HS_TX_9_T3POST_DCO_REG_SHIFT,
		PHY_0X5109_HS_TX_9_T3POST_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5309_CORE_DIG_CLANE_1_RW_HS_TX_9,
		cphy.hs_tx_9_t3post_dco,
		PHY_0X5309_HS_TX_9_T3POST_DCO_REG_SHIFT,
		PHY_0X5309_HS_TX_9_T3POST_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5509_CORE_DIG_CLANE_2_RW_HS_TX_9,
		cphy.hs_tx_9_t3post_dco,
		PHY_0X5509_HS_TX_9_T3POST_DCO_REG_SHIFT,
		PHY_0X5509_HS_TX_9_T3POST_DCO_REG_MASK, DIRECT_WRITE);
	/* set static vale : 0 */
	DPU_BITMASK_WRITE(hw, PHY_0X5103_CORE_DIG_CLANE_0_RW_HS_TX_3,
		0, PHY_0X5103_HS_TX_3_BURST_TYPE_REG_SHIFT,
		PHY_0X5103_HS_TX_3_BURST_TYPE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5303_CORE_DIG_CLANE_1_RW_HS_TX_3,
		0, PHY_0X5303_HS_TX_3_BURST_TYPE_REG_SHIFT,
		PHY_0X5303_HS_TX_3_BURST_TYPE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5503_CORE_DIG_CLANE_2_RW_HS_TX_3,
		0, PHY_0X5503_HS_TX_3_BURST_TYPE_REG_SHIFT,
		PHY_0X5503_HS_TX_3_BURST_TYPE_REG_MASK, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5104_CORE_DIG_CLANE_0_RW_HS_TX_4);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB0_REG_SHIFT,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB0_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB1_REG_SHIFT,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB1_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB2_REG_SHIFT,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB2_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB3_REG_SHIFT,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB3_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB4_REG_SHIFT,
		PHY_0X5104_HS_TX_4_PROGSEQSYMB4_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5104_CORE_DIG_CLANE_0_RW_HS_TX_4, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5304_CORE_DIG_CLANE_1_RW_HS_TX_4);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB0_REG_SHIFT,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB0_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB1_REG_SHIFT,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB1_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB2_REG_SHIFT,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB2_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB3_REG_SHIFT,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB3_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB4_REG_SHIFT,
		PHY_0X5304_HS_TX_4_PROGSEQSYMB4_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5304_CORE_DIG_CLANE_1_RW_HS_TX_4, value, DIRECT_WRITE);


	value = DPU_REG_READ(hw, PHY_0X5504_CORE_DIG_CLANE_2_RW_HS_TX_4);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB0_REG_SHIFT,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB0_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB1_REG_SHIFT,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB1_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB2_REG_SHIFT,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB2_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB3_REG_SHIFT,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB3_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB4_REG_SHIFT,
		PHY_0X5504_HS_TX_4_PROGSEQSYMB4_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5504_CORE_DIG_CLANE_2_RW_HS_TX_4, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5105_CORE_DIG_CLANE_0_RW_HS_TX_5);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB5_REG_SHIFT,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB5_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB6_REG_SHIFT,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB6_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB7_REG_SHIFT,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB7_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB8_REG_SHIFT,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB8_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB9_REG_SHIFT,
		PHY_0X5105_HS_TX_5_PROGSEQSYMB9_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5105_CORE_DIG_CLANE_0_RW_HS_TX_5, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5305_CORE_DIG_CLANE_1_RW_HS_TX_5);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB5_REG_SHIFT,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB5_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB6_REG_SHIFT,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB6_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB7_REG_SHIFT,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB7_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB8_REG_SHIFT,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB8_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB9_REG_SHIFT,
		PHY_0X5305_HS_TX_5_PROGSEQSYMB9_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5305_CORE_DIG_CLANE_1_RW_HS_TX_5, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5505_CORE_DIG_CLANE_2_RW_HS_TX_5);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB5_REG_SHIFT,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB5_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB6_REG_SHIFT,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB6_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB7_REG_SHIFT,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB7_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB8_REG_SHIFT,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB8_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB9_REG_SHIFT,
		PHY_0X5505_HS_TX_5_PROGSEQSYMB9_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5505_CORE_DIG_CLANE_2_RW_HS_TX_5, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5106_CORE_DIG_CLANE_0_RW_HS_TX_6);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB10_REG_SHIFT,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB10_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB11_REG_SHIFT,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB11_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB12_REG_SHIFT,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB12_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB13_REG_SHIFT,
		PHY_0X5106_HS_TX_6_PROGSEQSYMB13_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5106_CORE_DIG_CLANE_0_RW_HS_TX_6, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5306_CORE_DIG_CLANE_1_RW_HS_TX_6);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB10_REG_SHIFT,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB10_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB11_REG_SHIFT,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB11_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB12_REG_SHIFT,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB12_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB13_REG_SHIFT,
		PHY_0X5306_HS_TX_6_PROGSEQSYMB13_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5306_CORE_DIG_CLANE_1_RW_HS_TX_6, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X5506_CORE_DIG_CLANE_2_RW_HS_TX_6);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB10_REG_SHIFT,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB10_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB11_REG_SHIFT,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB11_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB12_REG_SHIFT,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB12_REG_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB13_REG_SHIFT,
		PHY_0X5506_HS_TX_6_PROGSEQSYMB13_REG_MASK);
	DPU_REG_WRITE(hw, PHY_0X5506_CORE_DIG_CLANE_2_RW_HS_TX_6, value, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5040_CORE_DIG_CLANE_0_RW_LP_0,
		1, PHY_0X5040_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X5040_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5240_CORE_DIG_CLANE_1_RW_LP_0,
		1, PHY_0X5240_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X5240_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X5440_CORE_DIG_CLANE_2_RW_LP_0,
		1, PHY_0X5440_LP_0_ITMINRX_REG_SHIFT,
		PHY_0X5440_LP_0_ITMINRX_REG_MASK, DIRECT_WRITE);

	/* LP_2_FILTER_INPUT_SAMPLING_REG */
	DPU_REG_WRITE(hw, PHY_0X5042_CORE_DIG_CLANE_0_RW_LP_2, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5242_CORE_DIG_CLANE_1_RW_LP_2, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5442_CORE_DIG_CLANE_2_RW_LP_2, 0, DIRECT_WRITE);

	/* ????????
	DPU_REG_WRITE(hw, PHY_0X5000_CORE_DIG_CLANE_0_RW_CFG_0, 0xF0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5200_CORE_DIG_CLANE_1_RW_CFG_0, 0xF0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5400_CORE_DIG_CLANE_2_RW_CFG_0, 0xF0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5106_CORE_DIG_CLANE_0_RW_HS_TX_6, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5306_CORE_DIG_CLANE_1_RW_HS_TX_6, 0, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X5506_CORE_DIG_CLANE_2_RW_HS_TX_6, 0, DIRECT_WRITE);
	*/
}

void phy_hw_tx_extra_cfg(struct dpu_hw_blk *hw)
{
	u16 value;

	/* keep cfg ????? */

	value = DPU_REG_READ(hw,
		PHY_0X1427_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_7);
	value = MERGE_MASK_BITS(value, 1,
		PHY_0X1427_OA_LANE2_LPRX_LP_PON_OVR_EN_SHIFT,
		PHY_0X1427_OA_LANE2_LPRX_LP_PON_OVR_EN_MASK);
	value = MERGE_MASK_BITS(value, 1,
		PHY_0X1427_OA_LANE2_LPRX_CD_PON_OVR_EN_SHIFT,
		PHY_0X1427_OA_LANE2_LPRX_CD_PON_OVR_EN_MASK);
	DPU_REG_WRITE(hw,
		PHY_0X1427_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_7, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw,
		PHY_0X1428_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_8);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X1428_OA_LANE2_LPRX_LP_PON_OVR_VAL_SHIFT,
		PHY_0X1428_OA_LANE2_LPRX_LP_PON_OVR_VAL_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X1428_OA_LANE2_LPRX_CD_PON_OVR_VAL_SHIFT,
		PHY_0X1428_OA_LANE2_LPRX_CD_PON_OVR_VAL_MASK);
	DPU_REG_WRITE(hw,
		PHY_0X1428_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_8, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X1C25_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_5);
	value = MERGE_MASK_BITS(value, 1,
		PHY_0X1C25_OA_CB_CAL_SINK_EN_OVR_EN_SHIFT,
		PHY_0X1C25_OA_CB_CAL_SINK_EN_OVR_EN_MASK);
	DPU_REG_WRITE(hw,
		PHY_0X1C25_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_5, value, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C24_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_4, 0,
		PHY_0X1C24_OA_CB_CAL_SINK_EN_OVR_VAL_SHIFT,
		PHY_0X1C24_OA_CB_CAL_SINK_EN_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C41_CORE_DIG_RW_COMMON_1, 2,
		PHY_0X1C41_COMMON_1_OCLA_DATA_SEL_SHIFT,
		PHY_0X1C41_COMMON_1_OCLA_DATA_SEL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C43_CORE_DIG_RW_COMMON_3, 2,
		PHY_0X1C43_COMMON_3_OCLA_CLK_SEL_SHIFT,
		PHY_0X1C43_COMMON_3_OCLA_CLK_SEL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw,
		PHY_0X1203_CORE_DIG_IOCTRL_RW_DPHY_PPI_LANE1_OVR_0_3, 0,
		PHY_0X1203_LANE1_I_TXREQUESTESC_D1_OVR_VAL_SHIFT,
		PHY_0X1203_LANE1_I_TXREQUESTESC_D1_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw,
		PHY_0X1204_CORE_DIG_IOCTRL_RW_DPHY_PPI_LANE1_OVR_0_4, 1,
		PHY_0X1204_LANE1_I_TXREQUESTESC_D1_OVR_EN_SHIFT,
		PHY_0X1204_LANE1_I_TXREQUESTESC_D1_OVR_EN_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw,
		PHY_0X1403_CORE_DIG_IOCTRL_RW_DPHY_PPI_LANE2_OVR_0_3, 0,
		PHY_0X1403_LANE2_I_TXREQUESTESC_D2_OVR_VAL_SHIFT,
		PHY_0X1403_LANE2_I_TXREQUESTESC_D2_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw,
		PHY_0X1404_CORE_DIG_IOCTRL_RW_DPHY_PPI_LANE2_OVR_0_4, 1,
		PHY_0X1404_LANE2_I_TXREQUESTESC_D2_OVR_EN_SHIFT,
		PHY_0X1404_LANE2_I_TXREQUESTESC_D2_OVR_EN_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw,
		PHY_0X1603_CORE_DIG_IOCTRL_RW_DPHY_PPI_LANE3_OVR_0_3, 0,
		PHY_0X1603_LANE3_I_TXREQUESTESC_D2_OVR_VAL_SHIFT,
		PHY_0X1603_LANE3_I_TXREQUESTESC_D2_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw,
		PHY_0X1604_CORE_DIG_IOCTRL_RW_DPHY_PPI_LANE3_OVR_0_4, 1,
		PHY_0X1604_LANE3_I_TXREQUESTESC_D2_OVR_EN_SHIFT,
		PHY_0X1604_LANE3_I_TXREQUESTESC_D2_OVR_EN_MASK, DIRECT_WRITE);
}

static void phy_hw_tx_common_cfg(struct dpu_hw_blk *hw, u64 lanerate)
{
	struct phy_cfg_clk cfg = {0};
	struct phy_cfg phy = {0};
	u16 value;

	phy_parameter_asic(lanerate, DSI_PHY_TYPE_DPHY, &phy);

	cfg = cfg_clk[CFG_CLK_38_4];

	DPU_BITMASK_WRITE(hw, PHY_0X0C10_PPI_STARTUP_RW_COMMON_DPHY_10,
		47, PHY_0X0C10_COMMON_DPHY_10_PHY_READY_ADDR_SHIFT,
		PHY_0X0C10_COMMON_DPHY_10_PHY_READY_ADDR_MASK, DIRECT_WRITE);

	value = DPU_REG_READ(hw,
		PHY_0X1CF2_CORE_DIG_ANACTRL_RW_COMMON_ANACTRL_2);
	/* ref: dsi sf user guid */
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X1CF2_ANACTRL_2_GLOBAL_ULPS_OVR_VAL_SHIFT,
		PHY_0X1CF2_ANACTRL_2_GLOBAL_ULPS_OVR_VAL_MASK);
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X1CF2_ANACTRL_2_GLOBAL_ULPS_OVR_EN_SHIFT,
		PHY_0X1CF2_ANACTRL_2_GLOBAL_ULPS_OVR_EN_MASK);
	DPU_REG_WRITE(hw,
		PHY_0X1CF2_CORE_DIG_ANACTRL_RW_COMMON_ANACTRL_2, value,
		DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1CF3_CORE_DIG_ANACTRL_RW_COMMON_ANACTRL_3,
		phy.hibernate_dly, PHY_0X1CF3_HIBERNATE_DLY_SHIFT,
		PHY_0X1CF3_HIBERNATE_DLY_MASK, DIRECT_WRITE);
	DSI_DEBUG("COMMON:hibernate_dly: %d\n", phy.hibernate_dly);

	DPU_BITMASK_WRITE(hw, PHY_0X1CF0_CORE_DIG_ANACTRL_RW_COMMON_ANACTRL_0,
		63, PHY_0X1CF0_ANACTRL_0_CB_LP_DCO_EN_DLY_SHIFT,
		PHY_0X1CF0_ANACTRL_0_CB_LP_DCO_EN_DLY_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X0C11_PPI_STARTUP_RW_COMMON_STARTUP_1_1,
		563, PHY_0X0C11_COMMON_STARTUP_1_1_PHY_READY_DLY_SHIFT,
		PHY_0X0C11_COMMON_STARTUP_1_1_PHY_READY_DLY_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X0C02_PPI_STARTUP_RW_COMMON_DPHY_2,
		3, PHY_0X0C02_COMMON_DPHY_2_RCAL_ADDR_SHIFT,
		PHY_0X0C02_COMMON_DPHY_2_RCAL_ADDR_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X0C03_PPI_STARTUP_RW_COMMON_DPHY_3,
		0x26, PHY_0X0C03_COMMON_DPHY_3_PLL_START_ADDR_SHIFT,
		PHY_0X0C03_COMMON_DPHY_3_PLL_START_ADDR_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X0C06_PPI_STARTUP_RW_COMMON_DPHY_6,
		0x10, PHY_0X0C06_COMMON_DPHY_6_LP_DCO_CAL_ADDR_SHIFT,
		PHY_0X0C06_COMMON_DPHY_6_LP_DCO_CAL_ADDR_MASK, DIRECT_WRITE);

	/* HIBERNATE_ADDR */
	DPU_REG_WRITE(hw, PHY_0X0C0A_PPI_STARTUP_RW_COMMON_DPHY_A,
		35, DIRECT_WRITE);

	/* BG_MAX_COUNTER */
	DPU_REG_WRITE(hw, PHY_0X0C26_PPI_CALIBCTRL_RW_COMMON_BG_0,
		500, DIRECT_WRITE);

	/* TERMCAL_TIMER: 38, cfg_clk set to 38.4MHz */
	DPU_REG_WRITE(hw, PHY_0X0E40_PPI_RW_TERMCAL_CFG_0, 38, DIRECT_WRITE);

	/* LPCDCOCAL_TIMEBASE: 153, cfg_clk set to 38.4MHz */
	DPU_REG_WRITE(hw, PHY_0X0E01_PPI_RW_LPDCOCAL_TIMEBASE,
		153, DIRECT_WRITE);

	/* LPCDCOCAL_NREF */
	DPU_REG_WRITE(hw, PHY_0X0E02_PPI_RW_LPDCOCAL_NREF, 800, DIRECT_WRITE);

	/* LPCDCOCAL_NREF_RANGE */
	DPU_REG_WRITE(hw, PHY_0X0E03_PPI_RW_LPDCOCAL_NREF_RANGE,
		27, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X0E05_PPI_RW_LPDCOCAL_TWAIT_CONFIG);
	value = MERGE_MASK_BITS(value, 127,
		PHY_0X0E05_TWAIT_CONFIG_LPCDCOCAL_TWAIT_PON_SHIFT,
		PHY_0X0E05_TWAIT_CONFIG_LPCDCOCAL_TWAIT_PON_MASK);
	value = MERGE_MASK_BITS(value, 47,
		PHY_0X0E05_TWAIT_CONFIG_LPCDCOCAL_TWAIT_COARSE_SHIFT,
		PHY_0X0E05_TWAIT_CONFIG_LPCDCOCAL_TWAIT_COARSE_MASK);
	DPU_REG_WRITE(hw, PHY_0X0E05_PPI_RW_LPDCOCAL_TWAIT_CONFIG,
		value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, PHY_0X0E06_PPI_RW_LPDCOCAL_VT_CONFIG);
	value = MERGE_MASK_BITS(value, 47,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_TWAIT_FINE_SHIFT,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_TWAIT_FINE_MASK);
	value = MERGE_MASK_BITS(value, 27,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_VT_NREF_RANGE_SHIFT,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_VT_NREF_RANGE_MASK);
	value = MERGE_MASK_BITS(value, 1,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_USE_IDEAL_NREF_SHIFT,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_USE_IDEAL_NREF_MASK);
	/* set disable: DE double check done */
	value = MERGE_MASK_BITS(value, 0,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_VT_TRACKING_EN_SHIFT,
		PHY_0X0E06_VT_CONFIG_LPCDCOCAL_VT_TRACKING_EN_MASK);
	DPU_REG_WRITE(hw, PHY_0X0E06_PPI_RW_LPDCOCAL_VT_CONFIG, value,
		DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X0E08_PPI_RW_LPDCOCAL_COARSE_CFG,
		1, PHY_0X0E08_LPDCOCAL_COARSE_CFG_NCOARSE_START_SHIFT,
		PHY_0X0E08_LPDCOCAL_COARSE_CFG_NCOARSE_START_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C22_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_2,
		1, PHY_0X1C22_OA_CB_PLL_BUSTIEZ_SHIFT,
		PHY_0X1C22_OA_CB_PLL_BUSTIEZ_MASK, DIRECT_WRITE);

	/* PLL_RST_TIME: 383, cfg_clk set to 38.4MHz */
	DPU_REG_WRITE(hw, PHY_0X0E60_PPI_RW_PLL_STARTUP_CFG_0,
		383, DIRECT_WRITE);

	/* PLL_GEAR_SHIFT_TIME: 191, cfg_clk set to 38.4MHz */
	DPU_REG_WRITE(hw, PHY_0X0E61_PPI_RW_PLL_STARTUP_CFG_1,
		191, DIRECT_WRITE);

	/* PLL_LOCK_DET_TIME */
	DPU_REG_WRITE(hw, PHY_0X0E62_PPI_RW_PLL_STARTUP_CFG_2, 0, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1023_CORE_DIG_IOCTRL_RW_AFE_LANE0_CTRL_2_3,
		0, PHY_0X1023_LANE0_HSTX_SEL_CLKLB_SHIFT,
		PHY_0X1023_LANE0_HSTX_SEL_CLKLB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1223_CORE_DIG_IOCTRL_RW_AFE_LANE1_CTRL_2_3,
		0, PHY_0X1223_LANE1_HSTX_SEL_CLKLB_SHIFT,
		PHY_0X1223_LANE1_HSTX_SEL_CLKLB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1423_CORE_DIG_IOCTRL_RW_AFE_LANE2_CTRL_2_3,
		0, PHY_0X1423_LANE2_HSTX_SEL_CLKLB_SHIFT,
		PHY_0X1423_LANE2_HSTX_SEL_CLKLB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1623_CORE_DIG_IOCTRL_RW_AFE_LANE3_CTRL_2_3,
		0, PHY_0X1623_LANE3_HSTX_SEL_CLKLB_SHIFT,
		PHY_0X1623_LANE3_HSTX_SEL_CLKLB_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1823_CORE_DIG_IOCTRL_RW_AFE_LANE4_CTRL_2_3,
		0, PHY_0X1823_LANE4_HSTX_SEL_CLKLB_SHIFT,
		PHY_0X1823_LANE4_HSTX_SEL_CLKLB_MASK, DIRECT_WRITE);

	/* CFG_CLK_DIV_FACTOR: 2'b11 */
	DPU_REG_WRITE(hw, PHY_0X0E36_PPI_RW_COMMON_CFG, 3, DIRECT_WRITE);

	/* set 0, DE double check done */
	DPU_BITMASK_WRITE(hw, PHY_0X0E34_PPI_RW_HSTX_FIFO_CFG,
		0, PHY_0X0E34_TXDATATRANSFERNHS_SEL_SHIFT,
		PHY_0X0E34_TXDATATRANSFERNHS_SEL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C21_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_1,
		0, PHY_0X1C21_OA_CB_HSTXLB_DCO_CLK0_EN_OVR_VAL_SHIFT,
		PHY_0X1C21_OA_CB_HSTXLB_DCO_CLK0_EN_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C23_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_3,
		1, PHY_0X1C23_OA_CB_HSTXLB_DCO_CLK0_EN_OVR_EN_SHIFT,
		PHY_0X1C23_OA_CB_HSTXLB_DCO_CLK0_EN_OVR_EN_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C20_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_0,
		0, PHY_0X1C20_OA_CB_HSTXLB_DCO_CLK90_EN_OVR_VAL_SHIFT,
		PHY_0X1C20_OA_CB_HSTXLB_DCO_CLK90_EN_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C23_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_3,
		1, PHY_0X1C23_OA_CB_HSTXLB_DCO_CLK90_EN_OVR_EN_SHIFT,
		PHY_0X1C23_OA_CB_HSTXLB_DCO_CLK90_EN_OVR_EN_SHIFT, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C26_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_6,
		1, PHY_0X1C26_OA_CB_HSTXLB_DCO_EN_OVR_EN_SHIFT,
		PHY_0X1C26_OA_CB_HSTXLB_DCO_EN_OVR_EN_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C27_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_7,
		0, PHY_0X1C27_OA_CB_HSTXLB_DCO_EN_OVR_VAL_SHIFT,
		PHY_0X1C27_OA_CB_HSTXLB_DCO_EN_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C26_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_6,
		1, PHY_0X1C26_OA_CB_HSTXLB_DCO_PON_OVR_EN_SHIFT,
		PHY_0X1C26_OA_CB_HSTXLB_DCO_PON_OVR_EN_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C27_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_7,
		0, PHY_0X1C27_OA_CB_HSTXLB_DCO_PON_OVR_VAL_SHIFT,
		PHY_0X1C27_OA_CB_HSTXLB_DCO_PON_OVR_VAL_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C26_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_6,
		1, PHY_0X1C26_OA_CB_HSTXLB_DCO_TUNE_CLKDIG_EN_OVR_EN_SHIFT,
		PHY_0X1C26_OA_CB_HSTXLB_DCO_TUNE_CLKDIG_EN_OVR_EN_MASK,
		DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C27_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_7,
		0, PHY_0X1C27_OA_CB_HSTXLB_DCO_TUNE_CLKDIG_EN_OVR_VAL_SHIFT,
		PHY_0X1C27_OA_CB_HSTXLB_DCO_TUNE_CLKDIG_EN_OVR_VAL_MASK,
		DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X1C25_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_5,
		0, PHY_0X1C25_OA_CB_SEL_45OHM_50OHM_SHIFT,
		PHY_0X1C25_OA_CB_SEL_45OHM_50OHM_MASK, DIRECT_WRITE);
	/* common finished*/
	/* add by DE doubl check */
	DPU_BITMASK_WRITE(hw, PHY_0X1C23_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_3,
		1, PHY_0X1C23_OA_CB_SEL_MPLL_REG_VREF_SHIFT,
		PHY_0X1C23_OA_CB_SEL_MPLL_REG_VREF_MASK, DIRECT_WRITE);
}

void static dsi_hw_phy_is_ready(struct dpu_hw_blk *hw)
{
	u16 phy_0x1c0c, phy_0x0e65, phy_0x0e41, phy_0x1c2f;
	u16 phy_0x0e07, phy_0x0e89, phy_0x1c2e;
	int ret;

	/* wait phy_ready state : read 0x1c0c == 0x0010 */
	ret = DPU_READ_POLL_TIMEOUT(hw,
		PHY_0X1C0C_CORE_DIG_IOCTRL_R_COMMON_PPI_OVR_0_12, phy_0x1c0c,
		(phy_0x1c0c & CDPHY_IS_READY), 100, 25000);
	if (ret < 0)
		DSI_ERROR("wait phy ready timeout, value 0x%x!\n", phy_0x1c0c);

	phy_0x0e65 = DPU_REG_READ(hw, PHY_0X0E65_PPI_R_PLL_STARTUP_DEBUG);
	phy_0x0e41 = DPU_REG_READ(hw, PHY_0X0E41_PPI_R_TERMCAL_DEBUG_0);
	phy_0x1c2f = DPU_REG_READ(hw, PHY_0X1C2F_CORE_DIG_IOCTRL_R_AFE_CB_CTRL_2_15);
	phy_0x0e07 = DPU_REG_READ(hw, PHY_0X0E07_PPI_R_LPDCOCAL_DEBUG_RB);
	phy_0x0e89 = DPU_REG_READ(hw, PHY_0X0E89_PPI_R_HSDCOCOCAL_DEBUG_RB);
	phy_0x1c2e = DPU_REG_READ(hw, PHY_0X1C2E_CORE_DIG_IOCTRL_R_AFE_CB_CTRL_2_14);

	DSI_DEBUG("0X1C0C 0x%08X, 0X0E65 0x%08X, 0X0E41 0x%08X, 0X1C2F 0x%08X, "
		"0X0E07 0x%08X, 0X0E89 0x%08X, 0X1C2E 0x%08X\n",
		phy_0x1c0c, phy_0x0e65, phy_0x0e41, phy_0x1c2f,
		phy_0x0e07, phy_0x0e89, phy_0x1c2e);
}

void dsi_hw_phy_set_amplitude(struct dpu_hw_blk *hw, u32 amplitude)
{
	DPU_BITMASK_WRITE(hw, PHY_0X1C24_CORE_DIG_IOCTRL_RW_AFE_CB_CTRL_2_4,
		amplitude, PHY_0X1C24_OA_CB_SEL_VCOMM_PROG_SHIFT,
		PHY_0X1C24_OA_CB_SEL_VCOMM_PROG_MASK, DIRECT_WRITE);

	DSI_DEBUG("set phy amplitude as %d\n", amplitude);
}

u32 dsi_hw_phy_get_eq_parms(struct dpu_hw_blk *hw)
{
	u32 setr;

	setr = DPU_REG_READ(hw, PHY_0X1C2F_CORE_DIG_IOCTRL_R_AFE_CB_CTRL_2_15);

	setr = setr & PHY_0X1C2F_OA_SETR_MASK >> PHY_0X1C2F_OA_SETR_SHIFT;

	return setr;

}

void dsi_hw_phy_hibernate_exit(struct dpu_hw_blk *hw)
{
	/* for ulps exit */
	DPU_REG_WRITE(hw, PHY_0X0E70_PPI_RW_TX_HIBERNATE_CFG_0, 1, DIRECT_WRITE);
	DPU_REG_WRITE(hw, PHY_0X0E70_PPI_RW_TX_HIBERNATE_CFG_0, 0, DIRECT_WRITE);
}

int dsi_hw_phy_init(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_phy_cfg *phy;

	phy = &cfg->phy_parms;

	if(phy->phy_sel == SNPS_CDPHY_ASIC){
		DSI_DEBUG("Select ASIC CDPHY!\n");
		phy_hw_tx_common_cfg(hw, phy->phy_hs_speed);

		if (phy->phy_type == DSI_PHY_TYPE_DPHY)
			dphy_hw_tx_cfg(hw, phy->phy_hs_speed, cfg);
		else if (phy->phy_type == DSI_PHY_TYPE_CPHY)
			cphy_hw_tx_cfg(hw, phy->phy_hs_speed);

		DSI_DEBUG("ASIC CDPHY init done!\n");

	} else if (phy->phy_sel == SNPS_CDPHY_FPGA) {
		dsi_hw_phy_init_fpga(hw, phy);
	} else {
		DSI_DEBUG("Select Xilinx FPGA DPHY!\n");
	}

	return 0;
}

int dsi_hw_phy_wait_ready(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg)
{
	int ret = 0;

	if(cfg->phy_sel == SNPS_CDPHY_FPGA)
		ret = dsi_hw_phy_wait_ready_fpga(hw, cfg);
	if(cfg->phy_sel == SNPS_CDPHY_ASIC)
		dsi_hw_phy_is_ready(hw);

	return ret;
}

void dsi_hw_phy_deinit(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg)
{
	if (cfg->phy_sel == SNPS_CDPHY_FPGA)
		dsi_hw_phy_deinit_fpga(hw, cfg);
	else if (cfg->phy_sel == SNPS_CDPHY_ASIC)
		DSI_DEBUG("DSI phy hw deinit\n");

}

int dsi_hw_phy_pll_cfg(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg)
{
	int ret = 0;

	if (cfg->phy_sel == SNPS_CDPHY_FPGA)
		ret = dsi_hw_phy_pll_config_fpga(hw, cfg);

	return ret;
}

void dsi_hw_phy_dyn_freq_pll_disable(struct dpu_hw_blk *hw)
{
	/* disable lp-dco clock */
	DPU_BITMASK_WRITE(hw, PHY_0X1C49_CORE_DIG_RW_COMMON_9, 0,
		PHY_0X1C49_LP_DCO_CLK_DYN_GATING_SHIFT,
		PHY_0X1C49_LP_DCO_CLK_DYN_GATING_MASK, DIRECT_WRITE);

	/* clear entire ssc logic */
	DPU_BITMASK_WRITE(hw, PHY_0X0E64_PPI_RW_PLL_STARTUP_CFG_4, 0,
		PHY_0X0E64_SSC_RESET_N_SHIFT,
		PHY_0X0E64_SSC_RESET_N_MASK, DIRECT_WRITE);
}

void dsi_hw_phy_dyn_freq_pll_enable(struct dpu_hw_blk *hw)
{
	/* enable lp-dco clock */
	DPU_BITMASK_WRITE(hw, PHY_0X1C49_CORE_DIG_RW_COMMON_9, 1,
		PHY_0X1C49_LP_DCO_CLK_DYN_GATING_SHIFT,
		PHY_0X1C49_LP_DCO_CLK_DYN_GATING_MASK, DIRECT_WRITE);

	/* enable ssc logic */
	DPU_BITMASK_WRITE(hw, PHY_0X0E64_PPI_RW_PLL_STARTUP_CFG_4, 1,
		PHY_0X0E64_SSC_RESET_N_SHIFT,
		PHY_0X0E64_SSC_RESET_N_MASK, DIRECT_WRITE);
}

void dsi_hw_dphy_mipi_freq_update(struct dpu_hw_blk *hw, u32 lanerate)
{
	struct phy_cfg dphy = {0};

	phy_parameter_asic(lanerate, DSI_PHY_TYPE_DPHY, &dphy);

	DPU_BITMASK_WRITE(hw, PHY_0X3103_CORE_DIG_DLANE_0_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3103_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3103_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3303_CORE_DIG_DLANE_1_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3303_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3303_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3503_CORE_DIG_DLANE_2_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3503_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3503_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3703_CORE_DIG_DLANE_3_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3703_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3703_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X310A_CORE_DIG_DLANE_0_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X310A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X310A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X330A_CORE_DIG_DLANE_1_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X330A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X330A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X350A_CORE_DIG_DLANE_2_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X350A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X350A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X370A_CORE_DIG_DLANE_3_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X370A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X370A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3104_CORE_DIG_DLANE_0_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3104_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3104_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3304_CORE_DIG_DLANE_1_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3304_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3304_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3504_CORE_DIG_DLANE_2_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3504_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3504_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3704_CORE_DIG_DLANE_3_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3704_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3704_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3109_CORE_DIG_DLANE_0_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3109_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3109_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3309_CORE_DIG_DLANE_1_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3309_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3309_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3509_CORE_DIG_DLANE_2_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3509_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3509_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3709_CORE_DIG_DLANE_3_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_d,
		PHY_0X3709_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3709_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3101_CORE_DIG_DLANE_0_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3101_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3101_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3301_CORE_DIG_DLANE_1_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3301_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3301_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3501_CORE_DIG_DLANE_2_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3501_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3501_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3701_CORE_DIG_DLANE_3_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_d,
		PHY_0X3701_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3701_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3100_CORE_DIG_DLANE_0_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3100_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3100_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3300_CORE_DIG_DLANE_1_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3300_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3300_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3500_CORE_DIG_DLANE_2_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3500_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3500_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3700_CORE_DIG_DLANE_3_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_d,
		PHY_0X3700_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3700_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3105_CORE_DIG_DLANE_0_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3105_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3105_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3305_CORE_DIG_DLANE_1_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3305_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3305_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3505_CORE_DIG_DLANE_2_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3505_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3505_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3705_CORE_DIG_DLANE_3_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_d,
		PHY_0X3705_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3705_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3106_CORE_DIG_DLANE_0_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3106_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3106_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3306_CORE_DIG_DLANE_1_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3306_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3306_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3506_CORE_DIG_DLANE_2_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3506_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3506_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3706_CORE_DIG_DLANE_3_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3706_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3706_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X310C_CORE_DIG_DLANE_0_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X310C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X310C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X330C_CORE_DIG_DLANE_1_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X330C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X330C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X350C_CORE_DIG_DLANE_2_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X350C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X350C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X370C_CORE_DIG_DLANE_3_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X370C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X370C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3903_CORE_DIG_DLANE_CLK_RW_HS_TX_3,
		dphy.hs_tx_3_tlptxoverlap,
		PHY_0X3903_HS_TX_3_TLPTXOVERLAP_REG_SHIFT,
		PHY_0X3903_HS_TX_3_TLPTXOVERLAP_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X390A_CORE_DIG_DLANE_CLK_RW_HS_TX_10,
		dphy.hs_tx_10_tlpt11init_dco,
		PHY_0X390A_HS_TX_10_TLP11INIT_DCO_REG_SHIFT,
		PHY_0X390A_HS_TX_10_TLP11INIT_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3904_CORE_DIG_DLANE_CLK_RW_HS_TX_4,
		dphy.hs_tx_4_tlpx_dco,
		PHY_0X3904_HS_TX_4_TLPX_DCO_REG_SHIFT,
		PHY_0X3904_HS_TX_4_TLPX_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3909_CORE_DIG_DLANE_CLK_RW_HS_TX_9,
		dphy.hs_tx_9_thsprpr_dco_c,
		PHY_0X3909_HS_TX_9_THSPRPR_DCO_REG_SHIFT,
		PHY_0X3909_HS_TX_9_THSPRPR_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3901_CORE_DIG_DLANE_CLK_RW_HS_TX_1,
		dphy.hs_tx_1_thszero_c,
		PHY_0X3901_HS_TX_1_THSZERO_REG_SHIFT,
		PHY_0X3901_HS_TX_1_THSZERO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3902_CORE_DIG_DLANE_CLK_RW_HS_TX_2,
		3, /* hs_tx_2_tclkpre_reg */
		PHY_0X3902_HS_TX_2_TCLKPRE_REG_SHIFT,
		PHY_0X3902_HS_TX_2_TCLKPRE_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3900_CORE_DIG_DLANE_CLK_RW_HS_TX_0,
		dphy.hs_tx_0_thstrail_c,
		PHY_0X3900_HS_TX_0_THSTRAIL_REG_SHIFT,
		PHY_0X3900_HS_TX_0_THSTRAIL_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3905_CORE_DIG_DLANE_CLK_RW_HS_TX_5,
		dphy.hs_tx_5_thstrail_dco_c,
		PHY_0X3905_HS_TX_5_THSTRAIL_DCO_REG_SHIFT,
		PHY_0X3905_HS_TX_5_THSTRAIL_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3908_CORE_DIG_DLANE_CLK_RW_HS_TX_8,
		dphy.hs_tx_8_tclkpost,
		PHY_0X3908_HS_TX_8_TCLKPOST_REG_SHIFT,
		PHY_0X3908_HS_TX_8_TCLKPOST_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X3906_CORE_DIG_DLANE_CLK_RW_HS_TX_6,
		dphy.hs_tx_6_tlp11end_dco,
		PHY_0X3906_HS_TX_6_TLP11END_DCO_REG_SHIFT,
		PHY_0X3906_HS_TX_6_TLP11END_DCO_REG_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, PHY_0X390C_CORE_DIG_DLANE_CLK_RW_HS_TX_12,
		dphy.hs_tx_12_thsexit_dco,
		PHY_0X390C_HS_TX_12_THSEXIT_DCO_REG_SHIFT,
		PHY_0X390C_HS_TX_12_THSEXIT_DCO_REG_MASK, DIRECT_WRITE);
}

int dsi_hw_phy_mipi_freq_update(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	struct dsi_phy_cfg *phy;

	phy = &cfg->phy_parms;

	if(phy->phy_sel == SNPS_CDPHY_ASIC){
		DSI_DEBUG("Select ASIC CDPHY!\n");
		if (phy->phy_type == DSI_PHY_TYPE_DPHY)
			dsi_hw_dphy_mipi_freq_update(hw, phy->phy_hs_speed);
		else if (phy->phy_type == DSI_PHY_TYPE_CPHY) /* CPHY */
			cphy_hw_tx_cfg(hw, phy->phy_hs_speed);

		DSI_DEBUG("ASIC CDPHY init done!\n");
	} else {
		DSI_DEBUG("Select Xilinx FPGA DPHY!\n");
	}

	return 0;
}
