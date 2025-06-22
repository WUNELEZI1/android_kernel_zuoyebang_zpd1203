// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef PCIE_PHY_REGIF_H
#define PCIE_PHY_REGIF_H

#define PCIE_PHY_PHY_MISC_CFG                      0x0000
#define PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_SHIFT                 0
#define PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BYPASS_MODE_MASK                  0x00000003
#define PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT        2
#define PCIE_PHY_PHY_MISC_CFG_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK         0x0000000c
#define PCIE_PHY_PHY_MISC_CFG_PHY0_TEST_FLYOVER_EN_SHIFT                  4
#define PCIE_PHY_PHY_MISC_CFG_PHY0_TEST_FLYOVER_EN_MASK                   0x00000010
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_TX_REF_CLK_EN_SHIFT                5
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_TX_REF_CLK_EN_MASK                 0x00000020
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_STOP_CLK_EN_SHIFT                  6
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_STOP_CLK_EN_MASK                   0x00000040
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_POWERDOWN_SHIFT                    7
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_POWERDOWN_MASK                     0x00000080
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_BURNIN_SHIFT                       8
#define PCIE_PHY_PHY_MISC_CFG_PHY_TEST_BURNIN_MASK                        0x00000100
#define PCIE_PHY_PHY_MISC_CFG_EXT_PCLK_REQ_SHIFT                          9
#define PCIE_PHY_PHY_MISC_CFG_EXT_PCLK_REQ_MASK                           0x00000200
#define PCIE_PHY_PHY_MISC_CFG_PG_MODE_EN_SHIFT                            10
#define PCIE_PHY_PHY_MISC_CFG_PG_MODE_EN_MASK                             0x00000400
#define PCIE_PHY_PIPE_RX0_CFG                      0x0004
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_TX0_PATTERN_SHIFT                      0
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_TX0_PATTERN_MASK                       0x00000003
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_TX0_ONES_ZEROS_SHIFT                   2
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_TX0_ONES_ZEROS_MASK                    0x00000004
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_TERMINATION_SHIFT                  3
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_TERMINATION_MASK                   0x00000008
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_SRIS_MODE_EN_SHIFT                 4
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_SRIS_MODE_EN_MASK                  0x00000010
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_ES0_CMN_REFCLK_MODE_SHIFT          5
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_ES0_CMN_REFCLK_MODE_MASK           0x00000020
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_EQ_TRAINING_SHIFT                  6
#define PCIE_PHY_PIPE_RX0_CFG_PIPE_RX0_EQ_TRAINING_MASK                   0x00000040
#define PCIE_PHY_PIPE_RX1_CFG                      0x0008
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_TX1_PATTERN_SHIFT                      0
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_TX1_PATTERN_MASK                       0x00000003
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_TX1_ONES_ZEROS_SHIFT                   2
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_TX1_ONES_ZEROS_MASK                    0x00000004
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_TERMINATION_SHIFT                  3
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_TERMINATION_MASK                   0x00000008
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_SRIS_MODE_EN_SHIFT                 4
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_SRIS_MODE_EN_MASK                  0x00000010
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_ES0_CMN_REFCLK_MODE_SHIFT          5
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_ES0_CMN_REFCLK_MODE_MASK           0x00000020
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_EQ_TRAINING_SHIFT                  6
#define PCIE_PHY_PIPE_RX1_CFG_PIPE_RX1_EQ_TRAINING_MASK                   0x00000040
#define PCIE_PHY_PHY_LINE0_CFG                     0x000c
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_TX2RX_LOOPBK_SHIFT              0
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_TX2RX_LOOPBK_MASK               0x00000001
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_PROTOCOL_SHIFT                  1
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_PROTOCOL_MASK                   0x0000001e
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_PHY_SRC_SEL_SHIFT               5
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_PHY_SRC_SEL_MASK                0x00000060
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_LINK_NUM_SHIFT                  7
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_LINK_NUM_MASK                   0x00000780
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_ASYNCPOWERCHANGEACK_SHIFT       11
#define PCIE_PHY_PHY_LINE0_CFG_PIPE_LANE0_ASYNCPOWERCHANGEACK_MASK        0x00000800
#define PCIE_PHY_PHY_LINE1_CFG                     0x0010
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_TX2RX_LOOPBK_SHIFT              0
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_TX2RX_LOOPBK_MASK               0x00000001
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_PROTOCOL_SHIFT                  1
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_PROTOCOL_MASK                   0x0000001e
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_PHY_SRC_SEL_SHIFT               5
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_PHY_SRC_SEL_MASK                0x00000060
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_LINK_NUM_SHIFT                  7
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_LINK_NUM_MASK                   0x00000780
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_ASYNCPOWERCHANGEACK_SHIFT       11
#define PCIE_PHY_PHY_LINE1_CFG_PIPE_LANE1_ASYNCPOWERCHANGEACK_MASK        0x00000800
#define PCIE_PHY_PHY_LINE_RPT                      0x0014
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE1_POWER_PRESENT_SHIFT              0
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE1_POWER_PRESENT_MASK               0x00000001
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE1_DATABUSWIDTH_SHIFT               1
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE1_DATABUSWIDTH_MASK                0x00000006
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE1_CLKACK_N_SHIFT                   3
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE1_CLKACK_N_MASK                    0x00000008
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE0_POWER_PRESENT_SHIFT              4
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE0_POWER_PRESENT_MASK               0x00000010
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE0_DATABUSWIDTH_SHIFT               5
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE0_DATABUSWIDTH_MASK                0x00000060
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE0_CLKACK_N_SHIFT                   7
#define PCIE_PHY_PHY_LINE_RPT_PIPE_LANE0_CLKACK_N_MASK                    0x00000080
#define PCIE_PHY_PHY_MISC_RPT                      0x0018
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX1_EBUFF_LOCATION_SHIFT               0
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX1_EBUFF_LOCATION_MASK                0x000001ff
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX1_ALIGN_DETECT_SHIFT                 9
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX1_ALIGN_DETECT_MASK                  0x00000200
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX0_EBUFF_LOCATION_SHIFT               10
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX0_EBUFF_LOCATION_MASK                0x0007fc00
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX0_ALIGN_DETECT_SHIFT                 19
#define PCIE_PHY_PHY_MISC_RPT_PIPE_RX0_ALIGN_DETECT_MASK                  0x00080000
#define PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_SHIFT                   20
#define PCIE_PHY_PHY_MISC_RPT_PHY0_SRAM_INIT_DONE_MASK                    0x00100000
#define PCIE_PHY_PHY_MISC_RPT_PHY0_DTB_OUT_SHIFT                          21
#define PCIE_PHY_PHY_MISC_RPT_PHY0_DTB_OUT_MASK                           0x00600000
#define PCIE_PHY_PHY_MISC_CFG1                     0x001c
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_REF_REPEAT_CLK_EN_SHIFT               0
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_REF_REPEAT_CLK_EN_MASK                0x00000001
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_REF_CLKDET_EN_SHIFT                   1
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_REF_CLKDET_EN_MASK                    0x00000002
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_REF_USE_PAD_SHIFT                     2
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_REF_USE_PAD_MASK                      0x00000004
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLB_FORCE_EN_SHIFT                  3
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLB_FORCE_EN_MASK                   0x00000008
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLB_SSC_EN_SHIFT                    4
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLB_SSC_EN_MASK                     0x00000010
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLA_FORCE_EN_SHIFT                  5
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLA_FORCE_EN_MASK                   0x00000020
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLA_SSC_EN_SHIFT                    6
#define PCIE_PHY_PHY_MISC_CFG1_PHY0_MPLLA_SSC_EN_MASK                     0x00000040
#define PCIE_PHY_PHY_MISC_CFG1_PHY_RTUNE_REQ_SHIFT                        7
#define PCIE_PHY_PHY_MISC_CFG1_PHY_RTUNE_REQ_MASK                         0x00000080
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE1_RX2TX_PAR_LB_EN_SHIFT            8
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE1_RX2TX_PAR_LB_EN_MASK             0x00000100
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE1_POWER_PRESENT_SHIFT              9
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE1_POWER_PRESENT_MASK               0x00000200
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE0_RX2TX_PAR_LB_EN_SHIFT            10
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE0_RX2TX_PAR_LB_EN_MASK             0x00000400
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE0_POWER_PRESENT_SHIFT              11
#define PCIE_PHY_PHY_MISC_CFG1_PHY_LANE0_POWER_PRESENT_MASK               0x00000800
#define PCIE_PHY_PHY_MISC_CFG2                     0x0020
#define PCIE_PHY_PHY_MISC_CFG2_PHY0_SRAM_EXT_LD_DONE_SHIFT                0
#define PCIE_PHY_PHY_MISC_CFG2_PHY0_SRAM_EXT_LD_DONE_MASK                 0x00000001
#define PCIE_PHY_PHY_RCX_CFG                       0x0024
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX1_FLYOVER_DATA_P_SHIFT                 0
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX1_FLYOVER_DATA_P_MASK                  0x00000001
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX1_FLYOVER_DATA_M_SHIFT                 1
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX1_FLYOVER_DATA_M_MASK                  0x00000002
#define PCIE_PHY_PHY_RCX_CFG_PHY_RX1_TERM_ACDC_SHIFT                      2
#define PCIE_PHY_PHY_RCX_CFG_PHY_RX1_TERM_ACDC_MASK                       0x00000004
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX0_FLYOVER_DATA_P_SHIFT                 3
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX0_FLYOVER_DATA_P_MASK                  0x00000008
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX0_FLYOVER_DATA_M_SHIFT                 4
#define PCIE_PHY_PHY_RCX_CFG_PHY_TX0_FLYOVER_DATA_M_MASK                  0x00000010
#define PCIE_PHY_PHY_RCX_CFG_PHY_RX0_TERM_ACDC_SHIFT                      5
#define PCIE_PHY_PHY_RCX_CFG_PHY_RX0_TERM_ACDC_MASK                       0x00000020
#define PCIE_PHY_PHY_MISC_RPT1                     0x0028
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_PPM_DRIFT_VLD_SHIFT                0
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_PPM_DRIFT_VLD_MASK                 0x00000001
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_PPM_DRIFT_SHIFT                    1
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_PPM_DRIFT_MASK                     0x0000007e
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_FLYOVER_DATA_P_SHIFT               7
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_FLYOVER_DATA_P_MASK                0x00000080
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_FLYOVER_DATA_M_SHIFT               8
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX1_FLYOVER_DATA_M_MASK                0x00000100
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_PPM_DRIFT_VLD_SHIFT                9
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_PPM_DRIFT_VLD_MASK                 0x00000200
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_PPM_DRIFT_SHIFT                    10
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_PPM_DRIFT_MASK                     0x0000fc00
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_FLYOVER_DATA_P_SHIFT               16
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_FLYOVER_DATA_P_MASK                0x00010000
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_FLYOVER_DATA_M_SHIFT               17
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RX0_FLYOVER_DATA_M_MASK                0x00020000
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RTUNE_ACK_SHIFT                        18
#define PCIE_PHY_PHY_MISC_RPT1_PHY_RTUNE_ACK_MASK                         0x00040000
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_REF_CLKDET_RESULT_SHIFT               19
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_REF_CLKDET_RESULT_MASK                0x00080000
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLB_STATE_SHIFT                     20
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLB_STATE_MASK                      0x00100000
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLB_FORCE_ACK_SHIFT                 21
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLB_FORCE_ACK_MASK                  0x00200000
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLA_STATE_SHIFT                     22
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLA_STATE_MASK                      0x00400000
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLA_FORCE_ACK_SHIFT                 23
#define PCIE_PHY_PHY_MISC_RPT1_PHY0_MPLLA_FORCE_ACK_MASK                  0x00800000
#define PCIE_PHY_PTL0_EXTREF                       0x002c
#define PCIE_PHY_PTL0_EXTREF_PROTOCOL0_EXT_REF_CLK_DIV2_EN_SHIFT          0
#define PCIE_PHY_PTL0_EXTREF_PROTOCOL0_EXT_REF_CLK_DIV2_EN_MASK           0x00000001
#define PCIE_PHY_PTL0_EXTREF_PROTOCOL0_EXT_REF_RANGE_SHIFT                1
#define PCIE_PHY_PTL0_EXTREF_PROTOCOL0_EXT_REF_RANGE_MASK                 0x0000000e
#define PCIE_PHY_PTL0_EXT_TX_CFG1                  0x0030
#define PCIE_PHY_PTL0_EXT_TX_CFG1_PROTOCOL0_EXT_TX_EQ_MAIN_G1_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG1_PROTOCOL0_EXT_TX_EQ_MAIN_G1_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG2                  0x0034
#define PCIE_PHY_PTL0_EXT_TX_CFG2_PROTOCOL0_EXT_TX_EQ_MAIN_G2_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG2_PROTOCOL0_EXT_TX_EQ_MAIN_G2_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG3                  0x0038
#define PCIE_PHY_PTL0_EXT_TX_CFG3_PROTOCOL0_EXT_TX_EQ_MAIN_G3_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG3_PROTOCOL0_EXT_TX_EQ_MAIN_G3_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG4                  0x003c
#define PCIE_PHY_PTL0_EXT_TX_CFG4_PROTOCOL0_EXT_TX_EQ_MAIN_G4_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG4_PROTOCOL0_EXT_TX_EQ_MAIN_G4_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG5                  0x0040
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G4_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G4_MASK        0x00000003
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G3_SHIFT       2
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G3_MASK        0x0000000c
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G2_SHIFT       4
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G2_MASK        0x00000030
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G1_SHIFT       6
#define PCIE_PHY_PTL0_EXT_TX_CFG5_PROTOCOL0_EXT_TX_EQ_OVRD_G1_MASK        0x000000c0
#define PCIE_PHY_PTL0_EXT_TX_CFG6                  0x0044
#define PCIE_PHY_PTL0_EXT_TX_CFG6_PROTOCOL0_EXT_TX_EQ_POST_G1_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG6_PROTOCOL0_EXT_TX_EQ_POST_G1_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG7                  0x0048
#define PCIE_PHY_PTL0_EXT_TX_CFG7_PROTOCOL0_EXT_TX_EQ_POST_G2_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG7_PROTOCOL0_EXT_TX_EQ_POST_G2_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG8                  0x004c
#define PCIE_PHY_PTL0_EXT_TX_CFG8_PROTOCOL0_EXT_TX_EQ_POST_G3_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG8_PROTOCOL0_EXT_TX_EQ_POST_G3_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG9                  0x0050
#define PCIE_PHY_PTL0_EXT_TX_CFG9_PROTOCOL0_EXT_TX_EQ_POST_G4_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG9_PROTOCOL0_EXT_TX_EQ_POST_G4_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG10                 0x0054
#define PCIE_PHY_PTL0_EXT_TX_CFG10_PROTOCOL0_EXT_TX_EQ_PRE_G1_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG10_PROTOCOL0_EXT_TX_EQ_PRE_G1_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG11                 0x0058
#define PCIE_PHY_PTL0_EXT_TX_CFG11_PROTOCOL0_EXT_TX_EQ_PRE_G2_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG11_PROTOCOL0_EXT_TX_EQ_PRE_G2_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG12                 0x005c
#define PCIE_PHY_PTL0_EXT_TX_CFG12_PROTOCOL0_EXT_TX_EQ_PRE_G3_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG12_PROTOCOL0_EXT_TX_EQ_PRE_G3_MASK        0x00000fff
#define PCIE_PHY_PTL0_EXT_TX_CFG13                 0x0060
#define PCIE_PHY_PTL0_EXT_TX_CFG13_PROTOCOL0_EXT_TX_EQ_PRE_G4_SHIFT       0
#define PCIE_PHY_PTL0_EXT_TX_CFG13_PROTOCOL0_EXT_TX_EQ_PRE_G4_MASK        0x00000fff
#define PCIE_PHY_ATE_CTRL0                         0x0064
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_APB1_PSEL_SHIFT                       0
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_APB1_PSEL_MASK                        0x00000001
#define PCIE_PHY_ATE_CTRL0_ATE_PG_MODE_EN_SHIFT                           1
#define PCIE_PHY_ATE_CTRL0_ATE_PG_MODE_EN_MASK                            0x00000002
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_REF_ALT_CLK_LP_SEL_SHIFT              2
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_REF_ALT_CLK_LP_SEL_MASK               0x00000004
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_TEST_TX_REF_CLK_EN_SHIFT               3
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_TEST_TX_REF_CLK_EN_MASK                0x00000008
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_TEST_FLYOVER_EN_SHIFT                 4
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_TEST_FLYOVER_EN_MASK                  0x00000010
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_SCAN_PMA_OCC_EN_SHIFT                 5
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_SCAN_PMA_OCC_EN_MASK                  0x00000020
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_TX0_FLYOVER_EN_SHIFT                   6
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_TX0_FLYOVER_EN_MASK                    0x00000040
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_TX1_FLYOVER_EN_SHIFT                   7
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_TX1_FLYOVER_EN_MASK                    0x00000080
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_RX0_FLYOVER_EN_SHIFT                   8
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_RX0_FLYOVER_EN_MASK                    0x00000100
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_RX1_FLYOVER_EN_SHIFT                   9
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_RX1_FLYOVER_EN_MASK                    0x00000200
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_PMA_PWR_STABLE_SHIFT                  10
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_PMA_PWR_STABLE_MASK                   0x00000400
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_REF_PRE_VREG_BYPASS_SHIFT             11
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_REF_PRE_VREG_BYPASS_MASK              0x00000800
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_SHIFT       12
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_SRAM_BOOTLOAD_BYPASS_MODE_MASK        0x00003000
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_SRAM_EXT_LD_DONE_SHIFT                14
#define PCIE_PHY_ATE_CTRL0_ATE_PHY0_SRAM_EXT_LD_DONE_MASK                 0x00004000
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_EXT_REF_OVRD_SEL_SHIFT                 15
#define PCIE_PHY_ATE_CTRL0_ATE_PHY_EXT_REF_OVRD_SEL_MASK                  0x00008000
#define PCIE_PHY_ATE_CTRL1                         0x0068
#define PCIE_PHY_ATE_CTRL1_ATE_PATTERN_GEN_SIGS_VALUE_EN_SHIFT            0
#define PCIE_PHY_ATE_CTRL1_ATE_PATTERN_GEN_SIGS_VALUE_EN_MASK             0x00000001
#define PCIE_PHY_ATE_CTRL1_ATE_PHY0_SRAM_BYPASS_MODE_SHIFT                1
#define PCIE_PHY_ATE_CTRL1_ATE_PHY0_SRAM_BYPASS_MODE_MASK                 0x00000006
#define PCIE_PHY_ATE_CTRL1_ATE_PHY_LANE0_RX2TX_PAR_LB_EN_SHIFT            3
#define PCIE_PHY_ATE_CTRL1_ATE_PHY_LANE0_RX2TX_PAR_LB_EN_MASK             0x00000008
#define PCIE_PHY_ATE_CTRL1_ATE_PHY_LANE1_RX2TX_PAR_LB_EN_SHIFT            4
#define PCIE_PHY_ATE_CTRL1_ATE_PHY_LANE1_RX2TX_PAR_LB_EN_MASK             0x00000010
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE0_RESET_N_SHIFT                   5
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE0_RESET_N_MASK                    0x00000020
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE1_RESET_N_SHIFT                   6
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE1_RESET_N_MASK                    0x00000040
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE0_POWERDOWN_SHIFT                 7
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE0_POWERDOWN_MASK                  0x00000780
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE1_POWERDOWN_SHIFT                 11
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE1_POWERDOWN_MASK                  0x00007800
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE0_TX2RX_LOOPBK_SHIFT              15
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE0_TX2RX_LOOPBK_MASK               0x00008000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE1_TX2RX_LOOPBK_SHIFT              16
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_LANE1_TX2RX_LOOPBK_MASK               0x00010000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_ONES_ZEROS_SHIFT                  17
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_ONES_ZEROS_MASK                   0x00020000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_ONES_ZEROS_SHIFT                  18
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_ONES_ZEROS_MASK                   0x00040000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_ELECIDLE_SHIFT                    19
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_ELECIDLE_MASK                     0x00080000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_ELECIDLE_SHIFT                    20
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_ELECIDLE_MASK                     0x00100000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_DETECTRX_SHIFT                    21
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_DETECTRX_MASK                     0x00200000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_DETECTRX_SHIFT                    22
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_DETECTRX_MASK                     0x00400000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_COMPLIANCE_SHIFT                  23
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX0_COMPLIANCE_MASK                   0x00800000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_COMPLIANCE_SHIFT                  24
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_TX1_COMPLIANCE_MASK                   0x01000000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX0_STANDBY_SHIFT                     25
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX0_STANDBY_MASK                      0x02000000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX1_STANDBY_SHIFT                     26
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX1_STANDBY_MASK                      0x04000000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX0_EQ_EVAL_SHIFT                     27
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX0_EQ_EVAL_MASK                      0x08000000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX1_EQ_EVAL_SHIFT                     28
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX1_EQ_EVAL_MASK                      0x10000000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX0_DISABLE_SHIFT                     29
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX0_DISABLE_MASK                      0x20000000
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX1_DISABLE_SHIFT                     30
#define PCIE_PHY_ATE_CTRL1_ATE_PIPE_RX1_DISABLE_MASK                      0x40000000
#define PCIE_PHY_PHY_RESET_OVRD_EN                 0x006c
#define PCIE_PHY_PHY_RESET_OVRD_EN_PHY_RESET_OVRD_EN_SHIFT                0
#define PCIE_PHY_PHY_RESET_OVRD_EN_PHY_RESET_OVRD_EN_MASK                 0x00000001
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE0_RESET_N_OVRD_EN_SHIFT       1
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE0_RESET_N_OVRD_EN_MASK        0x00000002
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE1_RESET_N_OVRD_EN_SHIFT       2
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE1_RESET_N_OVRD_EN_MASK        0x00000004
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE0_PERST_N_OVRD_EN_SHIFT       3
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE0_PERST_N_OVRD_EN_MASK        0x00000008
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE1_PERST_N_OVRD_EN_SHIFT       4
#define PCIE_PHY_PHY_RESET_OVRD_EN_PIPE_LANE1_PERST_N_OVRD_EN_MASK        0x00000010
#define PCIE_PHY_PHY_RESET_OVRD_EN_PHY_RESET_SRC_SEL_SHIFT                5
#define PCIE_PHY_PHY_RESET_OVRD_EN_PHY_RESET_SRC_SEL_MASK                 0x00000020
#define PCIE_PHY_PHY_RESET_OVRD_VAL                0x0070
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PHY_RESET_OVRD_VAL_SHIFT              0
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PHY_RESET_OVRD_VAL_MASK               0x00000001
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE0_RESET_N_OVRD_VAL_SHIFT     1
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE0_RESET_N_OVRD_VAL_MASK      0x00000002
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE1_RESET_N_OVRD_VAL_SHIFT     2
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE1_RESET_N_OVRD_VAL_MASK      0x00000004
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE0_PERST_N_OVRD_VAL_SHIFT     3
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE0_PERST_N_OVRD_VAL_MASK      0x00000008
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE1_PERST_N_OVRD_VAL_SHIFT     4
#define PCIE_PHY_PHY_RESET_OVRD_VAL_PIPE_LANE1_PERST_N_OVRD_VAL_MASK      0x00000010
#define PCIE_PHY_APP_PHY_PMA_PWR_EN                0x0074
#define PCIE_PHY_APP_PHY_PMA_PWR_EN_APP_PHY_PMA_PWR_EN_SHIFT              0
#define PCIE_PHY_APP_PHY_PMA_PWR_EN_APP_PHY_PMA_PWR_EN_MASK               0x00000001
#define PCIE_PHY_PCS_DBGSIGS                       0x0078
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_STATE_SHIFT        0
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_STATE_MASK         0x0000001f
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_RX_ACK_SHIFT   5
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_RX_ACK_MASK    0x00000020
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_TX_ACK_SHIFT   6
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_TX_ACK_MASK    0x00000040
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_RX_PSTATE_SHIFT 7
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_RX_PSTATE_MASK  0x00000180
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_TX_PSTATE_SHIFT 9
#define PCIE_PHY_PCS_DBGSIGS_PCS_DBGSIGS_LANE0_PWR_CTL_PHY_TX_PSTATE_MASK  0x00000600
#define PCIE_PHY_PHY0_REF_RES                      0x007c
#define PCIE_PHY_PHY0_REF_RES_PHY_EXT_REF_OVRD_SEL_SHIFT                  0
#define PCIE_PHY_PHY0_REF_RES_PHY_EXT_REF_OVRD_SEL_MASK                   0x00000001
#define PCIE_PHY_PHY0_REF_RES_PHY0_RES_EXT_RCAL_SHIFT                     1
#define PCIE_PHY_PHY0_REF_RES_PHY0_RES_EXT_RCAL_MASK                      0x0000007e
#define PCIE_PHY_PHY0_REF_RES_PHY0_RES_EXT_EN_SHIFT                       7
#define PCIE_PHY_PHY0_REF_RES_PHY0_RES_EXT_EN_MASK                        0x00000080
#define PCIE_PHY_PHY0_REF_RES_PHY0_REF_PRE_VREG_BYPASS_SHIFT              8
#define PCIE_PHY_PHY0_REF_RES_PHY0_REF_PRE_VREG_BYPASS_MASK               0x00000100
#define PCIE_PHY_PHY0_REF_RES_PHY0_REF_ALT_CLK_LP_SEL_SHIFT               9
#define PCIE_PHY_PHY0_REF_RES_PHY0_REF_ALT_CLK_LP_SEL_MASK                0x00000200
#define PCIE_PHY_PHY_FLYOVER_TEST                  0x0080
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX1_HP_PROT_EN_SHIFT                0
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX1_HP_PROT_EN_MASK                 0x00000001
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX1_FLYOVER_EN_SHIFT                1
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX1_FLYOVER_EN_MASK                 0x00000002
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX0_HP_PROT_EN_SHIFT                2
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX0_HP_PROT_EN_MASK                 0x00000004
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX0_FLYOVER_EN_SHIFT                3
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_TX0_FLYOVER_EN_MASK                 0x00000008
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_RX1_FLYOVER_EN_SHIFT                4
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_RX1_FLYOVER_EN_MASK                 0x00000010
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_RX0_FLYOVER_EN_SHIFT                5
#define PCIE_PHY_PHY_FLYOVER_TEST_PHY_RX0_FLYOVER_EN_MASK                 0x00000020
#define PCIE_PHY_PIPE_LEGACY_PIN_MODE              0x0084
#define PCIE_PHY_PIPE_LEGACY_PIN_MODE_PIPE_LANE1_LEGACY_PIN_MODE_SHIFT    0
#define PCIE_PHY_PIPE_LEGACY_PIN_MODE_PIPE_LANE1_LEGACY_PIN_MODE_MASK     0x00000001
#define PCIE_PHY_PIPE_LEGACY_PIN_MODE_PIPE_LANE0_LEGACY_PIN_MODE_SHIFT    1
#define PCIE_PHY_PIPE_LEGACY_PIN_MODE_PIPE_LANE0_LEGACY_PIN_MODE_MASK     0x00000002
#define PCIE_PHY_PIPE_SERDES                       0x0088
#define PCIE_PHY_PIPE_SERDES_PIPE_RX1_IF_WIDTH_SHIFT                      0
#define PCIE_PHY_PIPE_SERDES_PIPE_RX1_IF_WIDTH_MASK                       0x00000003
#define PCIE_PHY_PIPE_SERDES_PIPE_RX0_IF_WIDTH_SHIFT                      2
#define PCIE_PHY_PIPE_SERDES_PIPE_RX0_IF_WIDTH_MASK                       0x0000000c
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE1_TURNOFF_SHIFT                     4
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE1_TURNOFF_MASK                      0x00000010
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE1_SERDES_MODE_SHIFT                 5
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE1_SERDES_MODE_MASK                  0x00000020
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE1_RXREADY_SHIFT                     6
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE1_RXREADY_MASK                      0x00000040
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE0_TURNOFF_SHIFT                     7
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE0_TURNOFF_MASK                      0x00000080
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE0_SERDES_MODE_SHIFT                 8
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE0_SERDES_MODE_MASK                  0x00000100
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE0_RXREADY_SHIFT                     9
#define PCIE_PHY_PIPE_SERDES_PIPE_LANE0_RXREADY_MASK                      0x00000200
#define PCIE_PHY_APB0_IF                           0x008c
#define PCIE_PHY_APB0_IF_PHY0_APB0_IF_SEL_SHIFT                           0
#define PCIE_PHY_APB0_IF_PHY0_APB0_IF_SEL_MASK                            0x00000001
#define PCIE_PHY_APB0_IF_PHY0_APB0_IF_MODE_SHIFT                          1
#define PCIE_PHY_APB0_IF_PHY0_APB0_IF_MODE_MASK                           0x00000006
#define PCIE_PHY_PCIE_SYS_TOP_CFG                  0x0090
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLK_100_SHIFT                    0
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLK_100_MASK                     0x00000001
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_CLKREQ_N_SHIFT                       1
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_CLKREQ_N_MASK                        0x00000002
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_SHIFT                     2
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_MUX_CLKREQ_MASK                      0x00000004
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_TX_EN_SHIFT                          3
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_TX_EN_MASK                           0x00000008
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_PULL_DOWN_SHIFT                      4
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_PULL_DOWN_MASK                       0x00000010
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_IMP_CTRL_SHIFT                       5
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_IMP_CTRL_MASK                        0x000003e0
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_CLK_SEL_SHIFT                        10
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_CLK_SEL_MASK                         0x00000c00
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_EN_SHIFT                      12
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_EN_MASK                       0x00001000
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_SHIFT                         13
#define PCIE_PHY_PCIE_SYS_TOP_CFG_SW_DE_EMP_MASK                          0x0000e000
#define PCIE_PHY_PCIE_IO_IPTEST_ATE                0x0094
#define PCIE_PHY_PCIE_IO_IPTEST_ATE_SW_MUX_ATE_PIPE_LANE0_RESET_N_SHIFT   0
#define PCIE_PHY_PCIE_IO_IPTEST_ATE_SW_MUX_ATE_PIPE_LANE0_RESET_N_MASK    0x00000001
#define PCIE_PHY_PCIE_IO_IPTEST_ATE_SW_MUX_ATE_PIPE_LANE1_RESET_N_SHIFT   1
#define PCIE_PHY_PCIE_IO_IPTEST_ATE_SW_MUX_ATE_PIPE_LANE1_RESET_N_MASK    0x00000002
#define PCIE_PHY_PCIE_IO_IPTEST_APB_IF_SEL         0x0098
#define PCIE_PHY_PCIE_IO_IPTEST_APB_IF_SEL_SW_MUX_APB_IF_SEL_SHIFT        0
#define PCIE_PHY_PCIE_IO_IPTEST_APB_IF_SEL_SW_MUX_APB_IF_SEL_MASK         0x00000001
#define PCIE_PHY_PHY0_NOMINAL_SEL                  0x009c
#define PCIE_PHY_PHY0_NOMINAL_SEL_PHY0_NOMINAL_VP_SEL_SHIFT               0
#define PCIE_PHY_PHY0_NOMINAL_SEL_PHY0_NOMINAL_VP_SEL_MASK                0x00000003
#define PCIE_PHY_PHY0_NOMINAL_SEL_PHY0_NOMINAL_VPH_SEL_SHIFT              2
#define PCIE_PHY_PHY0_NOMINAL_SEL_PHY0_NOMINAL_VPH_SEL_MASK               0x0000000c
#define PCIE_PHY_PWR_STABLE                        0x00a0
#define PCIE_PHY_PWR_STABLE_UPCS_PWR_STABLE_SHIFT                         0
#define PCIE_PHY_PWR_STABLE_UPCS_PWR_STABLE_MASK                          0x00000001
#define PCIE_PHY_PWR_STABLE_PHY_PCS_PWR_STABLE_SHIFT                      1
#define PCIE_PHY_PWR_STABLE_PHY_PCS_PWR_STABLE_MASK                       0x00000002
#define PCIE_PHY_ATE_CTRL_1                        0x00a4
#define PCIE_PHY_ATE_CTRL_1_ATE_PHY0_NOMINAL_VPH_SEL_SHIFT                0
#define PCIE_PHY_ATE_CTRL_1_ATE_PHY0_NOMINAL_VPH_SEL_MASK                 0x00000003
#define PCIE_PHY_ATE_CTRL_1_ATE_PHY0_NOMINAL_VP_SEL_SHIFT                 2
#define PCIE_PHY_ATE_CTRL_1_ATE_PHY0_NOMINAL_VP_SEL_MASK                  0x0000000c
#define PCIE_PHY_REGFILE_CGBYPASS                  0x00a8
#define PCIE_PHY_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                  0
#define PCIE_PHY_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                   0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_sram_bypass_mode          :  2;
        unsigned int phy0_sram_bootload_bypass_mode :  2;
        unsigned int phy0_test_flyover_en           :  1;
        unsigned int phy_test_tx_ref_clk_en         :  1;
        unsigned int phy_test_stop_clk_en           :  1;
        unsigned int phy_test_powerdown             :  1;
        unsigned int phy_test_burnin                :  1;
        unsigned int ext_pclk_req                   :  1;
        unsigned int pg_mode_en                     :  1;
        unsigned int reserved_0                     : 21;
    } reg;
}pcie_phy_phy_misc_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_tx0_pattern             :  2;
        unsigned int pipe_tx0_ones_zeros          :  1;
        unsigned int pipe_rx0_termination         :  1;
        unsigned int pipe_rx0_sris_mode_en        :  1;
        unsigned int pipe_rx0_es0_cmn_refclk_mode :  1;
        unsigned int pipe_rx0_eq_training         :  1;
        unsigned int reserved_0                   : 25;
    } reg;
}pcie_phy_pipe_rx0_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_tx1_pattern             :  2;
        unsigned int pipe_tx1_ones_zeros          :  1;
        unsigned int pipe_rx1_termination         :  1;
        unsigned int pipe_rx1_sris_mode_en        :  1;
        unsigned int pipe_rx1_es0_cmn_refclk_mode :  1;
        unsigned int pipe_rx1_eq_training         :  1;
        unsigned int reserved_0                   : 25;
    } reg;
}pcie_phy_pipe_rx1_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_lane0_tx2rx_loopbk        :  1;
        unsigned int pipe_lane0_protocol            :  4;
        unsigned int pipe_lane0_phy_src_sel         :  2;
        unsigned int pipe_lane0_link_num            :  4;
        unsigned int pipe_lane0_asyncpowerchangeack :  1;
        unsigned int reserved_0                     : 20;
    } reg;
}pcie_phy_phy_line0_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_lane1_tx2rx_loopbk        :  1;
        unsigned int pipe_lane1_protocol            :  4;
        unsigned int pipe_lane1_phy_src_sel         :  2;
        unsigned int pipe_lane1_link_num            :  4;
        unsigned int pipe_lane1_asyncpowerchangeack :  1;
        unsigned int reserved_0                     : 20;
    } reg;
}pcie_phy_phy_line1_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_lane1_power_present :  1;
        unsigned int pipe_lane1_databuswidth  :  2;
        unsigned int pipe_lane1_clkack_n      :  1;
        unsigned int pipe_lane0_power_present :  1;
        unsigned int pipe_lane0_databuswidth  :  2;
        unsigned int pipe_lane0_clkack_n      :  1;
        unsigned int reserved_0               : 24;
    } reg;
}pcie_phy_phy_line_rpt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_rx1_ebuff_location :  9;
        unsigned int pipe_rx1_align_detect   :  1;
        unsigned int pipe_rx0_ebuff_location :  9;
        unsigned int pipe_rx0_align_detect   :  1;
        unsigned int phy0_sram_init_done     :  1;
        unsigned int phy0_dtb_out            :  2;
        unsigned int reserved_0              :  9;
    } reg;
}pcie_phy_phy_misc_rpt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_ref_repeat_clk_en    :  1;
        unsigned int phy0_ref_clkdet_en        :  1;
        unsigned int phy0_ref_use_pad          :  1;
        unsigned int phy0_mpllb_force_en       :  1;
        unsigned int phy0_mpllb_ssc_en         :  1;
        unsigned int phy0_mplla_force_en       :  1;
        unsigned int phy0_mplla_ssc_en         :  1;
        unsigned int phy_rtune_req             :  1;
        unsigned int phy_lane1_rx2tx_par_lb_en :  1;
        unsigned int phy_lane1_power_present   :  1;
        unsigned int phy_lane0_rx2tx_par_lb_en :  1;
        unsigned int phy_lane0_power_present   :  1;
        unsigned int reserved_0                : 20;
    } reg;
}pcie_phy_phy_misc_cfg1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_sram_ext_ld_done :  1;
        unsigned int reserved_0            : 31;
    } reg;
}pcie_phy_phy_misc_cfg2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_tx1_flyover_data_p :  1;
        unsigned int phy_tx1_flyover_data_m :  1;
        unsigned int phy_rx1_term_acdc      :  1;
        unsigned int phy_tx0_flyover_data_p :  1;
        unsigned int phy_tx0_flyover_data_m :  1;
        unsigned int phy_rx0_term_acdc      :  1;
        unsigned int reserved_0             : 26;
    } reg;
}pcie_phy_phy_rcx_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_rx1_ppm_drift_vld  :  1;
        unsigned int phy_rx1_ppm_drift      :  6;
        unsigned int phy_rx1_flyover_data_p :  1;
        unsigned int phy_rx1_flyover_data_m :  1;
        unsigned int phy_rx0_ppm_drift_vld  :  1;
        unsigned int phy_rx0_ppm_drift      :  6;
        unsigned int phy_rx0_flyover_data_p :  1;
        unsigned int phy_rx0_flyover_data_m :  1;
        unsigned int phy_rtune_ack          :  1;
        unsigned int phy0_ref_clkdet_result :  1;
        unsigned int phy0_mpllb_state       :  1;
        unsigned int phy0_mpllb_force_ack   :  1;
        unsigned int phy0_mplla_state       :  1;
        unsigned int phy0_mplla_force_ack   :  1;
        unsigned int reserved_0             :  8;
    } reg;
}pcie_phy_phy_misc_rpt1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_ref_clk_div2_en :  1;
        unsigned int protocol0_ext_ref_range       :  3;
        unsigned int reserved_0                    : 28;
    } reg;
}pcie_phy_ptl0_extref_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_main_g1 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_main_g2 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_main_g3 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_main_g4 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_ovrd_g4 :  2;
        unsigned int protocol0_ext_tx_eq_ovrd_g3 :  2;
        unsigned int protocol0_ext_tx_eq_ovrd_g2 :  2;
        unsigned int protocol0_ext_tx_eq_ovrd_g1 :  2;
        unsigned int reserved_0                  : 24;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_post_g1 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_post_g2 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_post_g3 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_post_g4 : 12;
        unsigned int reserved_0                  : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_pre_g1 : 12;
        unsigned int reserved_0                 : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_pre_g2 : 12;
        unsigned int reserved_0                 : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_pre_g3 : 12;
        unsigned int reserved_0                 : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int protocol0_ext_tx_eq_pre_g4 : 12;
        unsigned int reserved_0                 : 20;
    } reg;
}pcie_phy_ptl0_ext_tx_cfg13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ate_phy0_apb1_psel                 :  1;
        unsigned int ate_pg_mode_en                     :  1;
        unsigned int ate_phy0_ref_alt_clk_lp_sel        :  1;
        unsigned int ate_phy_test_tx_ref_clk_en         :  1;
        unsigned int ate_phy0_test_flyover_en           :  1;
        unsigned int ate_phy0_scan_pma_occ_en           :  1;
        unsigned int ate_phy_tx0_flyover_en             :  1;
        unsigned int ate_phy_tx1_flyover_en             :  1;
        unsigned int ate_phy_rx0_flyover_en             :  1;
        unsigned int ate_phy_rx1_flyover_en             :  1;
        unsigned int ate_phy0_pma_pwr_stable            :  1;
        unsigned int ate_phy0_ref_pre_vreg_bypass       :  1;
        unsigned int ate_phy0_sram_bootload_bypass_mode :  2;
        unsigned int ate_phy0_sram_ext_ld_done          :  1;
        unsigned int ate_phy_ext_ref_ovrd_sel           :  1;
        unsigned int reserved_0                         : 16;
    } reg;
}pcie_phy_ate_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ate_pattern_gen_sigs_value_en :  1;
        unsigned int ate_phy0_sram_bypass_mode     :  2;
        unsigned int ate_phy_lane0_rx2tx_par_lb_en :  1;
        unsigned int ate_phy_lane1_rx2tx_par_lb_en :  1;
        unsigned int ate_pipe_lane0_reset_n        :  1;
        unsigned int ate_pipe_lane1_reset_n        :  1;
        unsigned int ate_pipe_lane0_powerdown      :  4;
        unsigned int ate_pipe_lane1_powerdown      :  4;
        unsigned int ate_pipe_lane0_tx2rx_loopbk   :  1;
        unsigned int ate_pipe_lane1_tx2rx_loopbk   :  1;
        unsigned int ate_pipe_tx0_ones_zeros       :  1;
        unsigned int ate_pipe_tx1_ones_zeros       :  1;
        unsigned int ate_pipe_tx0_elecidle         :  1;
        unsigned int ate_pipe_tx1_elecidle         :  1;
        unsigned int ate_pipe_tx0_detectrx         :  1;
        unsigned int ate_pipe_tx1_detectrx         :  1;
        unsigned int ate_pipe_tx0_compliance       :  1;
        unsigned int ate_pipe_tx1_compliance       :  1;
        unsigned int ate_pipe_rx0_standby          :  1;
        unsigned int ate_pipe_rx1_standby          :  1;
        unsigned int ate_pipe_rx0_eq_eval          :  1;
        unsigned int ate_pipe_rx1_eq_eval          :  1;
        unsigned int ate_pipe_rx0_disable          :  1;
        unsigned int ate_pipe_rx1_disable          :  1;
        unsigned int reserved_0                    :  1;
    } reg;
}pcie_phy_ate_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_reset_ovrd_en          :  1;
        unsigned int pipe_lane0_reset_n_ovrd_en :  1;
        unsigned int pipe_lane1_reset_n_ovrd_en :  1;
        unsigned int pipe_lane0_perst_n_ovrd_en :  1;
        unsigned int pipe_lane1_perst_n_ovrd_en :  1;
        unsigned int phy_reset_src_sel          :  1;
        unsigned int reserved_0                 : 26;
    } reg;
}pcie_phy_phy_reset_ovrd_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_reset_ovrd_val          :  1;
        unsigned int pipe_lane0_reset_n_ovrd_val :  1;
        unsigned int pipe_lane1_reset_n_ovrd_val :  1;
        unsigned int pipe_lane0_perst_n_ovrd_val :  1;
        unsigned int pipe_lane1_perst_n_ovrd_val :  1;
        unsigned int reserved_0                  : 27;
    } reg;
}pcie_phy_phy_reset_ovrd_val_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int app_phy_pma_pwr_en :  1;
        unsigned int reserved_0         : 31;
    } reg;
}pcie_phy_app_phy_pma_pwr_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pcs_dbgsigs_lane0_pwr_ctl_state         :  5;
        unsigned int pcs_dbgsigs_lane0_pwr_ctl_phy_rx_ack    :  1;
        unsigned int pcs_dbgsigs_lane0_pwr_ctl_phy_tx_ack    :  1;
        unsigned int pcs_dbgsigs_lane0_pwr_ctl_phy_rx_pstate :  2;
        unsigned int pcs_dbgsigs_lane0_pwr_ctl_phy_tx_pstate :  2;
        unsigned int reserved_0                              : 21;
    } reg;
}pcie_phy_pcs_dbgsigs_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_ext_ref_ovrd_sel     :  1;
        unsigned int phy0_res_ext_rcal        :  6;
        unsigned int phy0_res_ext_en          :  1;
        unsigned int phy0_ref_pre_vreg_bypass :  1;
        unsigned int phy0_ref_alt_clk_lp_sel  :  1;
        unsigned int reserved_0               : 22;
    } reg;
}pcie_phy_phy0_ref_res_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy_tx1_hp_prot_en :  1;
        unsigned int phy_tx1_flyover_en :  1;
        unsigned int phy_tx0_hp_prot_en :  1;
        unsigned int phy_tx0_flyover_en :  1;
        unsigned int phy_rx1_flyover_en :  1;
        unsigned int phy_rx0_flyover_en :  1;
        unsigned int reserved_0         : 26;
    } reg;
}pcie_phy_phy_flyover_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_lane1_legacy_pin_mode :  1;
        unsigned int pipe_lane0_legacy_pin_mode :  1;
        unsigned int reserved_0                 : 30;
    } reg;
}pcie_phy_pipe_legacy_pin_mode_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int pipe_rx1_if_width      :  2;
        unsigned int pipe_rx0_if_width      :  2;
        unsigned int pipe_lane1_turnoff     :  1;
        unsigned int pipe_lane1_serdes_mode :  1;
        unsigned int pipe_lane1_rxready     :  1;
        unsigned int pipe_lane0_turnoff     :  1;
        unsigned int pipe_lane0_serdes_mode :  1;
        unsigned int pipe_lane0_rxready     :  1;
        unsigned int reserved_0             : 22;
    } reg;
}pcie_phy_pipe_serdes_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_apb0_if_sel  :  1;
        unsigned int phy0_apb0_if_mode :  2;
        unsigned int reserved_0        : 29;
    } reg;
}pcie_phy_apb0_if_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_mux_clk_100 :  1;
        unsigned int sw_clkreq_n    :  1;
        unsigned int sw_mux_clkreq  :  1;
        unsigned int sw_tx_en       :  1;
        unsigned int sw_pull_down   :  1;
        unsigned int sw_imp_ctrl    :  5;
        unsigned int sw_clk_sel     :  2;
        unsigned int sw_de_emp_en   :  1;
        unsigned int sw_de_emp      :  3;
        unsigned int reserved_0     : 16;
    } reg;
}pcie_phy_pcie_sys_top_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_mux_ate_pipe_lane0_reset_n :  1;
        unsigned int sw_mux_ate_pipe_lane1_reset_n :  1;
        unsigned int reserved_0                    : 30;
    } reg;
}pcie_phy_pcie_io_iptest_ate_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_mux_apb_if_sel :  1;
        unsigned int reserved_0        : 31;
    } reg;
}pcie_phy_pcie_io_iptest_apb_if_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int phy0_nominal_vp_sel  :  2;
        unsigned int phy0_nominal_vph_sel :  2;
        unsigned int reserved_0           : 28;
    } reg;
}pcie_phy_phy0_nominal_sel_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int upcs_pwr_stable    :  1;
        unsigned int phy_pcs_pwr_stable :  1;
        unsigned int reserved_0         : 30;
    } reg;
}pcie_phy_pwr_stable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ate_phy0_nominal_vph_sel :  2;
        unsigned int ate_phy0_nominal_vp_sel  :  2;
        unsigned int reserved_0               : 28;
    } reg;
}pcie_phy_ate_ctrl_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}pcie_phy_regfile_cgbypass_t;

#endif
