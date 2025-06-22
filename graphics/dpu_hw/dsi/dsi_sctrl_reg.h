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

#ifndef _DPU_HW_DSI_SCTRL_REG_H_
#define _DPU_HW_DSI_SCTRL_REG_H_

#define PHY_IF_CFG0                                                   0x0000
#define PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_SHIFT                          0
#define PHY_IF_CFG0_CFG_PHY_SHUTDOWN_N_MASK                           0x00000001
#define PHY_IF_CFG0_CFG_PHY_MODE_SHIFT                                1
#define PHY_IF_CFG0_CFG_PHY_MODE_MASK                                 0x00000002
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE_DCK_SHIFT                 2
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE_DCK_MASK                  0x00000004
#define PHY_IF_CFG0_CFG_PHY_FORCERXMODE_DCK_SHIFT                     3
#define PHY_IF_CFG0_CFG_PHY_FORCERXMODE_DCK_MASK                      0x00000008
#define PHY_IF_CFG0_CFG_PHY_LANE0_TURNDISABLE_SHIFT                   4
#define PHY_IF_CFG0_CFG_PHY_LANE0_TURNDISABLE_MASK                    0x00000010
#define PHY_IF_CFG0_CFG_PHY_FORCERXMODE_SHIFT                         5
#define PHY_IF_CFG0_CFG_PHY_FORCERXMODE_MASK                          0x00000020
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE0_SHIFT                    6
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE0_MASK                     0x00000040
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE1_SHIFT                    7
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE1_MASK                     0x00000080
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE2_SHIFT                    8
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE2_MASK                     0x00000100
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE3_SHIFT                    9
#define PHY_IF_CFG0_CFG_PHY_FORCETXSTOPMODE3_MASK                     0x00000200
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS0_0_SHIFT                     10
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS0_0_MASK                      0x00001c00
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS1_0_SHIFT                     13
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS1_0_MASK                      0x0000e000
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS0_1_SHIFT                     16
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS0_1_MASK                      0x00070000
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS1_1_SHIFT                     19
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS1_1_MASK                      0x00380000
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS0_2_SHIFT                     22
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS0_2_MASK                      0x01c00000
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS1_2_SHIFT                     25
#define PHY_IF_CFG0_CFG_PHY_TXSYNCTYPEHS1_2_MASK                      0x0e000000
#define PHY_IF_CFG0_CFG_PHY_TXHSIDLECLKHS_SHIFT                       28
#define PHY_IF_CFG0_CFG_PHY_TXHSIDLECLKHS_MASK                        0x10000000
#define PHY_IF_CFG0_CFG_PHY_BIST_EN_SHIFT                             29
#define PHY_IF_CFG0_CFG_PHY_BIST_EN_MASK                              0x20000000
#define PHY_IF_CFG0_CFG_PHY_CONT_EN_SHIFT                             30
#define PHY_IF_CFG0_CFG_PHY_CONT_EN_MASK                              0x40000000
#define PHY_IF_CFG0_CFG_PHY_TEST_STOP_CLK_EN_SHIFT                    31
#define PHY_IF_CFG0_CFG_PHY_TEST_STOP_CLK_EN_MASK                     0x80000000

#define PHY_IF_CFG1                                                   0x0004
#define PHY_IF_CFG1_CFG_PHY_PARITY_ERROR_SHIFT                        0
#define PHY_IF_CFG1_CFG_PHY_PARITY_ERROR_MASK                         0x00000001
#define PHY_IF_CFG1_CFG_PHY_READY_SHIFT                               1
#define PHY_IF_CFG1_CFG_PHY_READY_MASK                                0x00000002
#define PHY_IF_CFG1_CFG_TXHSIDLECLKREADYHS_SHIFT                      2
#define PHY_IF_CFG1_CFG_TXHSIDLECLKREADYHS_MASK                       0x00000004
#define PHY_IF_CFG1_CFG_PLL_VPL_DET_SHIFT                             3
#define PHY_IF_CFG1_CFG_PLL_VPL_DET_MASK                              0x00000008
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT_DCK_SHIFT                       4
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT_DCK_MASK                        0x00000010
#define PHY_IF_CFG1_CFG_STOPSTATE_DCK_SHIFT                           5
#define PHY_IF_CFG1_CFG_STOPSTATE_DCK_MASK                            0x00000020
#define PHY_IF_CFG1_CFG_DIRECTION0_SHIFT                              6
#define PHY_IF_CFG1_CFG_DIRECTION0_MASK                               0x00000040
#define PHY_IF_CFG1_CFG_DIRECTION1_SHIFT                              7
#define PHY_IF_CFG1_CFG_DIRECTION1_MASK                               0x00000080
#define PHY_IF_CFG1_CFG_DIRECTION2_SHIFT                              8
#define PHY_IF_CFG1_CFG_DIRECTION2_MASK                               0x00000100
#define PHY_IF_CFG1_CFG_DIRECTION3_SHIFT                              9
#define PHY_IF_CFG1_CFG_DIRECTION3_MASK                               0x00000200
#define PHY_IF_CFG1_CFG_STOPSTATE0_SHIFT                              10
#define PHY_IF_CFG1_CFG_STOPSTATE0_MASK                               0x00000400
#define PHY_IF_CFG1_CFG_STOPSTATE1_SHIFT                              11
#define PHY_IF_CFG1_CFG_STOPSTATE1_MASK                               0x00000800
#define PHY_IF_CFG1_CFG_STOPSTATE2_SHIFT                              12
#define PHY_IF_CFG1_CFG_STOPSTATE2_MASK                               0x00001000
#define PHY_IF_CFG1_CFG_STOPSTATE3_SHIFT                              13
#define PHY_IF_CFG1_CFG_STOPSTATE3_MASK                               0x00002000
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT0_SHIFT                          14
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT0_MASK                           0x00004000
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT1_SHIFT                          15
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT1_MASK                           0x00008000
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT2_SHIFT                          16
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT2_MASK                           0x00010000
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT3_SHIFT                          17
#define PHY_IF_CFG1_CFG_ULPSACTIVENOT3_MASK                           0x00020000
#define PHY_IF_CFG1_CFG_ERRESC0_SHIFT                                 18
#define PHY_IF_CFG1_CFG_ERRESC0_MASK                                  0x00040000
#define PHY_IF_CFG1_CFG_ERRSYNCESC0_SHIFT                             19
#define PHY_IF_CFG1_CFG_ERRSYNCESC0_MASK                              0x00080000
#define PHY_IF_CFG1_CFG_ERRCONTROL0_SHIFT                             20
#define PHY_IF_CFG1_CFG_ERRCONTROL0_MASK                              0x00100000
#define PHY_IF_CFG1_CFG_ERRCONTENTIONLP0_0_SHIFT                      21
#define PHY_IF_CFG1_CFG_ERRCONTENTIONLP0_0_MASK                       0x00200000
#define PHY_IF_CFG1_CFG_ERRCONTENTIONLP1_0_SHIFT                      22
#define PHY_IF_CFG1_CFG_ERRCONTENTIONLP1_0_MASK                       0x00400000
#define PHY_IF_CFG1_CFG_TXREADYHS0_SHIFT                              23
#define PHY_IF_CFG1_CFG_TXREADYHS0_MASK                               0x00800000
#define PHY_IF_CFG1_CFG_TXREADYHS1_SHIFT                              24
#define PHY_IF_CFG1_CFG_TXREADYHS1_MASK                               0x01000000
#define PHY_IF_CFG1_CFG_TXREADYHS2_SHIFT                              25
#define PHY_IF_CFG1_CFG_TXREADYHS2_MASK                               0x02000000
#define PHY_IF_CFG1_CFG_TXREADYHS3_SHIFT                              26
#define PHY_IF_CFG1_CFG_TXREADYHS3_MASK                               0x04000000
#define PHY_IF_CFG1_CFG_PHY_BIST_OK_SHIFT                             27
#define PHY_IF_CFG1_CFG_PHY_BIST_OK_MASK                              0x08000000
#define PHY_IF_CFG1_CFG_PHY_BIST_DONE_SHIFT                           28
#define PHY_IF_CFG1_CFG_PHY_BIST_DONE_MASK                            0x10000000
#define PHY_IF_CFG1_CFG_PLL_LOCK_SHIFT                                29
#define PHY_IF_CFG1_CFG_PLL_LOCK_MASK                                 0x20000000

#define PHY_PLL_IF_CFG1                                               0x0008
#define PHY_PLL_IF_CFG1_CFG_PLL_ATB_SENSE_SEL_SHIFT                   0
#define PHY_PLL_IF_CFG1_CFG_PLL_ATB_SENSE_SEL_MASK                    0x00000001
#define PHY_PLL_IF_CFG1_CFG_PLL_CPBIAS_CNTRL_SHIFT                    1
#define PHY_PLL_IF_CFG1_CFG_PLL_CPBIAS_CNTRL_MASK                     0x000000fe
#define PHY_PLL_IF_CFG1_CFG_PLL_GMP_CNTRL_SHIFT                       9
#define PHY_PLL_IF_CFG1_CFG_PLL_GMP_CNTRL_MASK                        0x00000600
#define PHY_PLL_IF_CFG1_CFG_PLL_CLKSEL_SHIFT                          11
#define PHY_PLL_IF_CFG1_CFG_PLL_CLKSEL_MASK                           0x00001800

#define PHY_IF_CFG2                                                   0x000c
#define PHY_IF_CFG2_CFG_PLL_GP_CLKEN_SHIFT                            0
#define PHY_IF_CFG2_CFG_PLL_GP_CLKEN_MASK                             0x00000001
#define PHY_IF_CFG2_CFG_PLL_INT_CNTRL_SHIFT                           1
#define PHY_IF_CFG2_CFG_PLL_INT_CNTRL_MASK                            0x0000007e
#define PHY_IF_CFG2_CFG_PLL_M_SHIFT                                   7
#define PHY_IF_CFG2_CFG_PLL_M_MASK                                    0x0007ff80
#define PHY_IF_CFG2_CFG_MPLL_OPMODE_SHIFT                             19
#define PHY_IF_CFG2_CFG_MPLL_OPMODE_MASK                              0x00f80000
#define PHY_IF_CFG2_CFG_PLL_N_SHIFT                                   24
#define PHY_IF_CFG2_CFG_PLL_N_MASK                                    0x3f000000

#define PHY_IF_CFG3                                                   0x0010

#define PHY_IF_CFG4                                                   0x0014
#define PHY_IF_CFG4_CFG_PLL_PROP_CNTRL_SHIFT                          0
#define PHY_IF_CFG4_CFG_PLL_PROP_CNTRL_MASK                           0x0000003f
#define PHY_IF_CFG4_CFG_PLL_TH1_SHIFT                                 6
#define PHY_IF_CFG4_CFG_PLL_TH1_MASK                                  0x0000ffc0
#define PHY_IF_CFG4_CFG_PLL_TH2_SHIFT                                 16
#define PHY_IF_CFG4_CFG_PLL_TH2_MASK                                  0x00ff0000
#define PHY_IF_CFG4_CFG_PLL_TH3_SHIFT                                 24
#define PHY_IF_CFG4_CFG_PLL_TH3_MASK                                  0xff000000

#define PHY_IF_CFG5                                                   0x0018
#define PHY_IF_CFG5_CFG_PLL_VCO_CNTRL_SHIFT                           6
#define PHY_IF_CFG5_CFG_PLL_VCO_CNTRL_MASK                            0x00000fc0
#define PHY_IF_CFG5_CFG_PLL_CLKOUTEN_LEFT_SHIFT                       12
#define PHY_IF_CFG5_CFG_PLL_CLKOUTEN_LEFT_MASK                        0x00001000
#define PHY_IF_CFG5_CFG_PLL_MEAS_IV_SHIFT                             14
#define PHY_IF_CFG5_CFG_PLL_MEAS_IV_MASK                              0xffffc000

#define PHY_IF_CFG6                                                   0x001c
#define PHY_IF_CFG6_CFG_MPLL_FRACN_UPDATE_EN_SHIFT                    21
#define PHY_IF_CFG6_CFG_MPLL_FRACN_UPDATE_EN_MASK                     0x00200000
#define PHY_IF_CFG6_CFG_MPLL_SSC_EN_SHIFT                             22
#define PHY_IF_CFG6_CFG_MPLL_SSC_EN_MASK                              0x00400000
#define PHY_IF_CFG6_CFG_MPLL_FRAC_EN_SHIFT                            23
#define PHY_IF_CFG6_CFG_MPLL_FRAC_EN_MASK                             0x00800000
#define PHY_IF_CFG6_CFG_MPLL_SPREAD_TYPE_SHIFT                        24
#define PHY_IF_CFG6_CFG_MPLL_SPREAD_TYPE_MASK                         0x03000000

#define PHY_IF_CFG7                                                   0x0020
#define PHY_IF_CFG7_CFG_MPLL_SSC_PEAK_SHIFT                           0
#define PHY_IF_CFG7_CFG_MPLL_SSC_PEAK_MASK                            0x000fffff

#define PHY_IF_CFG8                                                   0x0024
#define PHY_IF_CFG8_CFG_MPLL_STEPSIZE_SHIFT                           0
#define PHY_IF_CFG8_CFG_MPLL_STEPSIZE_MASK                            0x001fffff

#define PHY_IF_CFG9                                                   0x0028
#define PHY_IF_CFG9_CFG_MPLL_MINT_SHIFT                               0
#define PHY_IF_CFG9_CFG_MPLL_MINT_MASK                                0x00000fff
#define PHY_IF_CFG9_CFG_MPLL_FRAC_QUOT_SHIFT                          12
#define PHY_IF_CFG9_CFG_MPLL_FRAC_QUOT_MASK                           0x0ffff000

#define PHY_IF_CFG10                                                  0x002c
#define PHY_IF_CFG10_CFG_MPLL_FRAC_REM_SHIFT                          0
#define PHY_IF_CFG10_CFG_MPLL_FRAC_REM_MASK                           0x0000ffff
#define PHY_IF_CFG10_CFG_MPLL_FRAC_DEN_SHIFT                          16
#define PHY_IF_CFG10_CFG_MPLL_FRAC_DEN_MASK                           0xffff0000
#define PHY_TEST_DATA                                                 0x0030
#define PHY_TEST_DATA_CFG_CONT_DATA_SHIFT                             0
#define PHY_TEST_DATA_CFG_CONT_DATA_MASK                              0x000007ff
#define ST_CLKGATE_AUTO                                               0x0034
#define ST_CLKGATE_AUTO_ST_CLK_IPI_AUTO_GATE_SHIFT                    0
#define ST_CLKGATE_AUTO_ST_CLK_IPI_AUTO_GATE_MASK                     0x00000001
#define ST_CLKGATE_AUTO_ST_CLK_SYS_AUTO_GATE_SHIFT                    1
#define ST_CLKGATE_AUTO_ST_CLK_SYS_AUTO_GATE_MASK                     0x00000002
#define ST_CLKGATE_AUTO_ST_CLK_HSTX_AUTO_GATE_SHIFT                   2
#define ST_CLKGATE_AUTO_ST_CLK_HSTX_AUTO_GATE_MASK                    0x00000004
#define ST_CLKGATE_AUTO_ST_CLK_DSI_SYS_SHIFT                          3
#define ST_CLKGATE_AUTO_ST_CLK_DSI_SYS_MASK                           0x00000008
#define ST_CLKGATE_AUTO_ST_CLK_DSI_PPI_SHIFT                          4
#define ST_CLKGATE_AUTO_ST_CLK_DSI_PPI_MASK                           0x00000010
#define ST_CLKGATE_AUTO_ST_CLK_DSI_CDPHY_REF_SHIFT                    5
#define ST_CLKGATE_AUTO_ST_CLK_DSI_CDPHY_REF_MASK                     0x00000020
#define ST_CLKGATE_AUTO_ST_CLK_DSI_CDPHY_CFG_SHIFT                    6
#define ST_CLKGATE_AUTO_ST_CLK_DSI_CDPHY_CFG_MASK                     0x00000040
#define ST_CLKGATE_AUTO_ST_CLK_DSI_IPI_SHIFT                          7
#define ST_CLKGATE_AUTO_ST_CLK_DSI_IPI_MASK                           0x00000080
#define ST_CLKGATE_AUTO_ST_CLK_DSI_VG_IPI_SHIFT                       8
#define ST_CLKGATE_AUTO_ST_CLK_DSI_VG_IPI_MASK                        0x00000100
#define ST_CLKGATE_AUTO_ST_CLK_DSI_PHY_HSTX_SHIFT                     9
#define ST_CLKGATE_AUTO_ST_CLK_DSI_PHY_HSTX_MASK                      0x00000200

#define DSI_PCLK_DIV                                                  0x0040

#define DSI_DOZE_CTRL                                                 0x0044
#define DSI_DOZE_CTRL_CFG_DSI_FORCE_DOZE_REQ_SHIFT                    0
#define DSI_DOZE_CTRL_CFG_DSI_FORCE_DOZE_REQ_MASK                     0x00000001

#define MEM_SD_CTRL0                                                  0x0048
#define MEM_SD_CTRL0_DSI_MEM_DS_ACK_SHIFT                             0
#define MEM_SD_CTRL0_DSI_MEM_DS_ACK_MASK                              0x00000001

#define MEM_SD_CTRL1                                                  0x004c
#define MEM_SD_CTRL1_CFG_DSI_RAM_DSLP_SHIFT                           0
#define MEM_SD_CTRL1_CFG_DSI_RAM_DSLP_MASK                            0x00000001

#define DSI_PPI_CLK_DIV                                               0x0058
#define DSI_PPI_CLK_DIV_CFG_DSI_PPI_CLK_DIV_SHIFT                     0
#define DSI_PPI_CLK_DIV_CFG_DSI_PPI_CLK_DIV_MASK                      0x0000003f

#define DSI_CRG_CFG3                                                  0x005c
#define DSI_CRG_CFG3_CFG_DSI_CTRL_CLKRST_BYPASS_SHIFT                 0
#define DSI_CRG_CFG3_CFG_DSI_CTRL_CLKRST_BYPASS_MASK                  0x00000001
#define DSI_CRG_CFG3_CFG_DSI_PHY_CLKRST_BYPASS_SHIFT                  1
#define DSI_CRG_CFG3_CFG_DSI_PHY_CLKRST_BYPASS_MASK                   0x00000002

#define DSI_MEM_CTRL0                                                 0x0060
#define DSI_MEM_CTRL1                                                 0x0064
#define DSI_MEM_CTRL2                                                 0x0068
#define NS_FIREWALL_CTRL0                                             0x006c
#define NS_FIREWALL_CTRL0_CFG_DSI_NS_FIREWALL_CLEAR_SHIFT             0
#define NS_FIREWALL_CTRL1                                             0x0070
#define NS_FIREWALL_CTRL1_CFG_DSI_NS_FIREWALL_FAIL_PMID_SHIFT         0
#define NS_FIREWALL_CTRL1_CFG_DSI_NS_FIREWALL_FAIL_PMID_MASK          0x000000ff
#define NS_FIREWALL_CTRL1_CFG_DSI_NS_FIREWALL_PERROR_SHIFT            8
#define NS_FIREWALL_CTRL1_CFG_DSI_NS_FIREWALL_PERROR_MASK             0x00000100
#define NS_FIREWALL_CTRL2                                             0x0074
#define TESTPIN_SEL                                                   0x0078
#define TESTPIN_SEL_CFG_DSI_TESTPIN_SEL_SHIFT                         0
#define TESTPIN_SEL_CFG_DSI_TESTPIN_SEL_MASK                          0x00000001
#define TESTPIN_SEL_CFG_TESTPOINT_DATA_SEL_SHIFT                      1
#define TESTPIN_SEL_CFG_TESTPOINT_DATA_SEL_MASK                       0x0000000e
#define TESTPIN_SEL_CFG_TXWORDCLKHS_SEL_FOR_IPTEST_SHIFT              4
#define TESTPIN_SEL_CFG_TXWORDCLKHS_SEL_FOR_IPTEST_MASK               0x00000010
#define VFP_CNT_DEBUG                                                 0x007c
#define VFP_CNT_DEBUG_CFG_DSI_VFP_OVERFLOW_CNT_SHIFT                  0
#define VFP_CNT_DEBUG_CFG_DSI_VFP_OVERFLOW_CNT_MASK                   0x0000003f

#define DSI_VG_CFG0                                                   0x0080
#define DSI_VG_CFG0_CFG_DSI_VG_EN_SHIFT                               0
#define DSI_VG_CFG0_CFG_DSI_VG_EN_MASK                                0x00000001
#define DSI_VG_CFG0_CFG_DSI_VG_OPMODE_SHIFT                           1
#define DSI_VG_CFG0_CFG_DSI_VG_OPMODE_MASK                            0x00000002
#define DSI_VG_CFG0_CFG_DSI_VG_COLOR_DEPTH_SHIFT                      2
#define DSI_VG_CFG0_CFG_DSI_VG_COLOR_DEPTH_MASK                       0x0000000c
#define DSI_VG_CFG0_CFG_DSI_VG_DPMODE_SHIFT                           4
#define DSI_VG_CFG0_CFG_DSI_VG_DPMODE_MASK                            0x00000030
#define DSI_VG_CFG0_CFG_DSI_VG_SHUTDN_REQ_SHIFT                       6
#define DSI_VG_CFG0_CFG_DSI_VG_SHUTDN_REQ_MASK                        0x00000040
#define DSI_VG_CFG0_CFG_DSI_VG_COLORM_REQ_SHIFT                       7
#define DSI_VG_CFG0_CFG_DSI_VG_COLORM_REQ_MASK                        0x00000080
#define DSI_VG_CFG0_CFG_DSI_VG_HIBERNATE_EN_SHIFT                     8
#define DSI_VG_CFG0_CFG_DSI_VG_HIBERNATE_EN_MASK                      0x00000100
#define DSI_VG_CFG0_CFG_DSI_VG_TEAR_EN_SHIFT                          9
#define DSI_VG_CFG0_CFG_DSI_VG_TEAR_EN_MASK                           0x00000200
#define DSI_VG_CFG0_CFG_DSI_VG_HIB_NUM_SHIFT                          10
#define DSI_VG_CFG0_CFG_DSI_VG_HIB_NUM_MASK                           0x0003fc00
#define DSI_VG_CFG1                                                   0x0084
#define DSI_VG_CFG1_CFG_DSI_VG_INTR_HNUM_SHIFT                        0
#define DSI_VG_CFG1_CFG_DSI_VG_INTR_HNUM_MASK                         0x0000ffff
#define DSI_VG_CFG2                                                   0x0088
#define DSI_VG_CFG2_CFG_DSI_VG_HSA_SHIFT                              0
#define DSI_VG_CFG2_CFG_DSI_VG_HSA_MASK                               0x0000ffff
#define DSI_VG_CFG2_CFG_DSI_VG_HBP_SHIFT                              16
#define DSI_VG_CFG2_CFG_DSI_VG_HBP_MASK                               0xffff0000
#define DSI_VG_CFG3                                                   0x008c
#define DSI_VG_CFG3_CFG_DSI_VG_HACT_SHIFT                             0
#define DSI_VG_CFG3_CFG_DSI_VG_HACT_MASK                              0x0000ffff
#define DSI_VG_CFG3_CFG_DSI_VG_HFP_SHIFT                              16
#define DSI_VG_CFG3_CFG_DSI_VG_HFP_MASK                               0xffff0000
#define DSI_VG_CFG4                                                   0x0090
#define DSI_VG_CFG4_CFG_DSI_VG_VSA_SHIFT                              0
#define DSI_VG_CFG4_CFG_DSI_VG_VSA_MASK                               0x0000ffff
#define DSI_VG_CFG4_CFG_DSI_VG_VBP_SHIFT                              16
#define DSI_VG_CFG4_CFG_DSI_VG_VBP_MASK                               0xffff0000
#define DSI_VG_CFG5                                                   0x0094
#define DSI_VG_CFG5_CFG_DSI_VG_VACT_SHIFT                             0
#define DSI_VG_CFG5_CFG_DSI_VG_VACT_MASK                              0x0000ffff
#define DSI_VG_CFG5_CFG_DSI_VG_VFP_SHIFT                              16
#define DSI_VG_CFG5_CFG_DSI_VG_VFP_MASK                               0xffff0000

#define DSI_VG_STATE0                                                 0x0098
#define DSI_VG_STATE0_DSI_VG_STATE_SHIFT                              0
#define DSI_VG_STATE0_DSI_VG_STATE_MASK                               0x000001ff

#define DSI_VG_ERR0                                                   0x009c
#define DSI_VG_ERR0_DSI_VG_ERR_SHIFT                                  0
#define DSI_VG_ERR0_DSI_VG_ERR_MASK                                   0x0000001f

#define DSI_VG_TE_TRG                                                 0x00a0
#define DSI_VG_TE_TRG_CFG_DSI_VG_TE_TRIGGER_SHIFT                     0

#define DSI_CLK_GATE_CTRL                                             0x00a4
#define DSI_CLK_GATE_CTRL_CFG_IPI_CLK_GATE_BYPASS_SHIFT               0
#define DSI_CLK_GATE_CTRL_CFG_IPI_CLK_GATE_BYPASS_MASK                0x00000001
#define DSI_CLK_GATE_CTRL_CFG_SYS_CLK_GATE_BYPASS_SHIFT               1
#define DSI_CLK_GATE_CTRL_CFG_SYS_CLK_GATE_BYPASS_MASK                0x00000002
#define DSI_CLK_GATE_CTRL_CFG_HSTX_CLK_GATE_BYPASS_SHIFT              2
#define DSI_CLK_GATE_CTRL_CFG_HSTX_CLK_GATE_BYPASS_MASK               0x00000004

#define CDPHY_DTB                                                     0x00a8
#define CDPHY_DTB_CFG_CDPHY_DTB_SHIFT                                 0
#define CDPHY_DTB_CFG_CDPHY_DTB_MASK                                  0x000000ff

#define DSI_CLKGATE_W1S                                               0x00b0
#define DSI_CLKGATE_W1S_GT_DSI_VG_IPI_CLKGATE_EN_SHIFT                0
#define DSI_CLKGATE_W1S_GT_DSI_VG_IPI_CLKGATE_EN_MASK                 0x00000001
#define DSI_CLKGATE_W1S_GT_DSI_PHY_CFG_CLKGATE_EN_SHIFT               1
#define DSI_CLKGATE_W1S_GT_DSI_PHY_CFG_CLKGATE_EN_MASK                0x00000002
#define DSI_CLKGATE_W1S_GT_DSI_PHY_REF_CLKGATE_EN_SHIFT               2
#define DSI_CLKGATE_W1S_GT_DSI_PHY_REF_CLKGATE_EN_MASK                0x00000004
#define DSI_CLKGATE_W1S_GT_DSI_SYS_CLKGATE_EN_SHIFT                   3
#define DSI_CLKGATE_W1S_GT_DSI_SYS_CLKGATE_EN_MASK                    0x00000008
#define DSI_CLKGATE_W1S_GT_DSI_CTRL_APB_CLKGATE_EN_SHIFT              4
#define DSI_CLKGATE_W1S_GT_DSI_CTRL_APB_CLKGATE_EN_MASK               0x00000010
#define DSI_CLKGATE_W1S_GT_DSI_PHY_APB_CLKGATE_EN_SHIFT               5
#define DSI_CLKGATE_W1S_GT_DSI_PHY_APB_CLKGATE_EN_MASK                0x00000020
#define DSI_CLKGATE_W1S_GT_DSI_IPI_CLKGATE_EN_SHIFT                   6
#define DSI_CLKGATE_W1S_GT_DSI_IPI_CLKGATE_EN_MASK                    0x00000040
#define DSI_CLKGATE_W1S_GT_DSI_PHY_PLL_OUT_EN_SHIFT                   7
#define DSI_CLKGATE_W1S_GT_DSI_PHY_PLL_OUT_EN_MASK                    0x00000080

#define DSI_CLKGATE_W1C                                               0x00b4
#define DSI_CLKGATE_W1C_GT_DSI_VG_IPI_CLKGATE_EN_SHIFT                0
#define DSI_CLKGATE_W1C_GT_DSI_VG_IPI_CLKGATE_EN_MASK                 0x00000001
#define DSI_CLKGATE_W1C_GT_DSI_PHY_CFG_CLKGATE_EN_SHIFT               1
#define DSI_CLKGATE_W1C_GT_DSI_PHY_CFG_CLKGATE_EN_MASK                0x00000002
#define DSI_CLKGATE_W1C_GT_DSI_PHY_REF_CLKGATE_EN_SHIFT               2
#define DSI_CLKGATE_W1C_GT_DSI_PHY_REF_CLKGATE_EN_MASK                0x00000004
#define DSI_CLKGATE_W1C_GT_DSI_SYS_CLKGATE_EN_SHIFT                   3
#define DSI_CLKGATE_W1C_GT_DSI_SYS_CLKGATE_EN_MASK                    0x00000008
#define DSI_CLKGATE_W1C_GT_DSI_CTRL_APB_CLKGATE_EN_SHIFT              4
#define DSI_CLKGATE_W1C_GT_DSI_CTRL_APB_CLKGATE_EN_MASK               0x00000010
#define DSI_CLKGATE_W1C_GT_DSI_PHY_APB_CLKGATE_EN_SHIFT               5
#define DSI_CLKGATE_W1C_GT_DSI_PHY_APB_CLKGATE_EN_MASK                0x00000020
#define DSI_CLKGATE_W1C_GT_DSI_IPI_CLKGATE_EN_SHIFT                   6
#define DSI_CLKGATE_W1C_GT_DSI_IPI_CLKGATE_EN_MASK                    0x00000030
#define DSI_CLKGATE_W1C_GT_DSI_PHY_PLL_OUT_EN_SHIFT                   7
#define DSI_CLKGATE_W1C_GT_DSI_PHY_PLL_OUT_EN_MASK                    0x00000080

#define DSI_CLKGATE_RO                                                0x00b8
#define DSI_CLKGATE_RO_GT_DSI_VG_IPI_CLKGATE_EN_SHIFT                 0
#define DSI_CLKGATE_RO_GT_DSI_VG_IPI_CLKGATE_EN_MASK                  0x00000001
#define DSI_CLKGATE_RO_GT_DSI_PHY_CFG_CLKGATE_EN_SHIFT                1
#define DSI_CLKGATE_RO_GT_DSI_PHY_CFG_CLKGATE_EN_MASK                 0x00000002
#define DSI_CLKGATE_RO_GT_DSI_PHY_REF_CLKGATE_EN_SHIFT                2
#define DSI_CLKGATE_RO_GT_DSI_PHY_REF_CLKGATE_EN_MASK                 0x00000004
#define DSI_CLKGATE_RO_GT_DSI_SYS_CLKGATE_EN_SHIFT                    3
#define DSI_CLKGATE_RO_GT_DSI_SYS_CLKGATE_EN_MASK                     0x00000008
#define DSI_CLKGATE_RO_GT_DSI_CTRL_APB_CLKGATE_EN_SHIFT               4
#define DSI_CLKGATE_RO_GT_DSI_CTRL_APB_CLKGATE_EN_MASK                0x00000010
#define DSI_CLKGATE_RO_GT_DSI_PHY_APB_CLKGATE_EN_SHIFT                5
#define DSI_CLKGATE_RO_GT_DSI_PHY_APB_CLKGATE_EN_MASK                 0x00000020
#define DSI_CLKGATE_RO_GT_DSI_IPI_CLKGATE_EN_SHIFT                    6
#define DSI_CLKGATE_RO_GT_DSI_IPI_CLKGATE_EN_MASK                     0x00000040
#define DSI_CLKGATE_RO_GT_DSI_PHY_PLL_OUT_EN_SHIFT                    7
#define DSI_CLKGATE_RO_GT_DSI_PHY_PLL_OUT_EN_MASK                     0x00000080

#define DSI_SRESET_W1S                                                0x00c0
#define DSI_SRESET_W1S_IP_RST_DSI_VG_IPI_N_SHIFT                      0
#define DSI_SRESET_W1S_IP_RST_DSI_VG_IPI_N_MASK                       0x00000001
#define DSI_SRESET_W1S_IP_PRST_DSI_CTRL_N_SHIFT                       1
#define DSI_SRESET_W1S_IP_PRST_DSI_CTRL_N_MASK                        0x00000002
#define DSI_SRESET_W1S_IP_PRST_DSI_PHY_N_SHIFT                        2
#define DSI_SRESET_W1S_IP_PRST_DSI_PHY_N_MASK                         0x00000004
#define DSI_SRESET_W1S_IP_RST_DSI_PHY_N_SHIFT                         3
#define DSI_SRESET_W1S_IP_RST_DSI_PHY_N_MASK                          0x00000008

#define DSI_SRESET_W1C                                                0x00c4
#define DSI_SRESET_W1C_IP_RST_DSI_VG_IPI_N_SHIFT                      0
#define DSI_SRESET_W1C_IP_RST_DSI_VG_IPI_N_MASK                       0x00000001
#define DSI_SRESET_W1C_IP_PRST_DSI_CTRL_N_SHIFT                       1
#define DSI_SRESET_W1C_IP_PRST_DSI_CTRL_N_MASK                        0x00000002
#define DSI_SRESET_W1C_IP_PRST_DSI_PHY_N_SHIFT                        2
#define DSI_SRESET_W1C_IP_PRST_DSI_PHY_N_MASK                         0x00000004
#define DSI_SRESET_W1C_IP_RST_DSI_PHY_N_SHIFT                         3
#define DSI_SRESET_W1C_IP_RST_DSI_PHY_N_MASK                          0x00000008

#define DSI_SRESET_RO                                                 0x00c8
#define DSI_SRESET_RO_IP_RST_DSI_VG_IPI_N_SHIFT                       0
#define DSI_SRESET_RO_IP_RST_DSI_VG_IPI_N_MASK                        0x00000001
#define DSI_SRESET_RO_IP_PRST_DSI_CTRL_N_SHIFT                        1
#define DSI_SRESET_RO_IP_PRST_DSI_CTRL_N_MASK                         0x00000002
#define DSI_SRESET_RO_IP_PRST_DSI_PHY_N_SHIFT                         2
#define DSI_SRESET_RO_IP_PRST_DSI_PHY_N_MASK                          0x00000004
#define DSI_SRESET_RO_IP_RST_DSI_PHY_N_SHIFT                          3
#define DSI_SRESET_RO_IP_RST_DSI_PHY_N_MASK                           0x00000008

#define DSI_CLK_FREQUCR_MEASURE                                       0x00d0
#define DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_ENABLE_SHIFT      0
#define DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_ENABLE_MASK       0x00000001
#define DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_SEL_SHIFT         1
#define DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_SEL_MASK          0x0000000e
#define DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_NUM_SHIFT         4
#define DSI_CLK_FREQUCR_MEASURE_CFG_DSI_CLK_MEASURE_NUM_MASK          0xfffffff0

#define DSI_CLK_FREQUCR_MEASURE_R                                     0x00d4

#define PHY_MON_OUT_VLD                                               0x00d8
#define PHY_MON_OUT_VLD_ST_PHY_MON_OUT_VALID_SHIFT                    0
#define PHY_MON_OUT_VLD_ST_PHY_MON_OUT_VALID_MASK                     0x00000001

#define PHY_MON_OUT0                                                  0x00dc

#define PHY_MON_OUT1                                                  0x00e0

#define INTR_DSI_MISC_CTRL_IRQ_INT_RAW                                0x0100
#define INTR_DSI_MISC_CTRL_IRQ_INT_RAW_VG_TE_INT_RAW_SHIFT            0
#define INTR_DSI_MISC_CTRL_IRQ_INT_RAW_VG_SYNC_INT_RAW_SHIFT          1
#define INTR_DSI_MISC_CTRL_IRQ_INT_RAW_DSI_ULPS_ENTRY_INT_RAW_SHIFT   2
#define INTR_DSI_MISC_CTRL_IRQ_INT_RAW_DSI_ULPS_EXIT_INT_RAW_SHIFT    3

#define INTR_DSI_MISC_CTRL_IRQ_INT_FORCE                              0x0104
#define INTR_DSI_MISC_CTRL_IRQ_INT_FORCE_VG_TE_INT_RAW_SHIFT          0
#define INTR_DSI_MISC_CTRL_IRQ_INT_FORCE_VG_SYNC_INT_RAW_SHIFT        1
#define INTR_DSI_MISC_CTRL_IRQ_INT_FORCE_DSI_ULPS_ENTRY_INT_RAW_SHIFT 2
#define INTR_DSI_MISC_CTRL_IRQ_INT_FORCE_DSI_ULPS_EXIT_INT_RAW_SHIFT  3

#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK                               0x0108
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_VG_TE_INT_MASK_SHIFT          0
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_VG_TE_INT_MASK_MASK           0x00000001
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_VG_SYNC_INT_MASK_SHIFT        1
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_VG_SYNC_INT_MASK_MASK         0x00000002
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_DSI_ULPS_ENTRY_INT_MASK_SHIFT 2
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_DSI_ULPS_ENTRY_INT_MASK_MASK  0x00000004
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_DSI_ULPS_EXIT_INT_MASK_SHIFT  3
#define INTR_DSI_MISC_CTRL_IRQ_INT_MASK_DSI_ULPS_EXIT_INT_MASK_MASK   0x00000008

#define INTR_DSI_MISC_CTRL_IRQ_INT_STATUS                             0x010c
#define INTR_DSI_MISC_CTRL_VG_TE_INT_STATUS_SHIFT                     0
#define INTR_DSI_MISC_CTRL_VG_TE_INT_STATUS_MASK                      0x00000001
#define INTR_DSI_MISC_CTRL_VG_SYNC_INT_STATUS_SHIFT                   1
#define INTR_DSI_MISC_CTRL_VG_SYNC_INT_STATUS_MASK                    0x00000002
#define INTR_DSI_MISC_CTRL_DSI_ULPS_ENTRY_INT_STATUS_SHIFT            2
#define INTR_DSI_MISC_CTRL_DSI_ULPS_ENTRY_INT_STATUS_MASK             0x00000004
#define INTR_DSI_MISC_CTRL_DSI_ULPS_EXIT_INT_STATUS_SHIFT             3
#define INTR_DSI_MISC_CTRL_DSI_ULPS_EXIT_INT_STATUS_MASK              0x00000008

#define INTR_DSI_CTRL_ACPU_SIRQ_INT_MASK                              0x0110
#define INTR_DSI_CTRL_ACPU_SIRQ_INT_MASK_INT_DSI_CTRL0_MASK_SHIFT     0
#define INTR_DSI_CTRL_ACPU_SIRQ_INT_MASK_INT_DSI_CTRL0_MASK_MASK      0x00000001

#define INTR_DSI_CTRL_ACPU_SIRQ_INT_STATUS                            0x0114
#define INTR_DSI_CTRL_ACPU_SIRQ_INT_STATUS_INT_DSI_CTRL0_STATUS_SHIFT 0
#define INTR_DSI_CTRL_ACPU_SIRQ_INT_STATUS_INT_DSI_CTRL0_STATUS_MASK  0x00000001

#define INTR_DSI_CTRL_LPMCU_SIRQ_INT_MASK                             0x0118
#define INTR_DSI_CTRL_LPMCU_SIRQ_INT_MASK_INT_DSI_CTRL1_MASK_SHIFT    0
#define INTR_DSI_CTRL_LPMCU_SIRQ_INT_MASK_INT_DSI_CTRL1_MASK_MASK     0x00000001

#define INTR_DSI_CTRL_LPMCU_SIRQ_INT_STATUS                           0x011c
#define INTR_DSI_CTRL_INT_DSI_CTRL1_STATUS_SHIFT                      0
#define INTR_DSI_CTRL_INT_DSI_CTRL1_STATUS_MASK                       0x00000001

#define INTR_DSI_CTRL_SENSORHUB_SIRQ_INT_MASK                         0x0120
#define INTR_DSI_CTRL_INT_DSI_CTRL2_MASK_SHIFT                        0
#define INTR_DSI_CTRL_INT_DSI_CTRL2_MASK_MASK                         0x00000001

#define INTR_DSI_CTRL_SENSORHUB_SIRQ_INT_STATUS                       0x0124
#define INTR_DSI_CTRL_INT_DSI_CTRL2_STATUS_SHIFT                      0
#define INTR_DSI_CTRL_INT_DSI_CTRL2_STATUS_MASK                       0x00000001

#define CGBYPASS                                                      0x0128
#define CGBYPASS_CGBYPASS_SHIFT                                       0
#define CGBYPASS_CGBYPASS_MASK                                        0x00000001
#endif /* _DPU_HW_DSI_SCTRL_REG_H_ */
