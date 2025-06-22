// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef LMS_CRG_REGIF_H
#define LMS_CRG_REGIF_H

#define LMS_CRG_CLKGT0_W1S                   0x0000
#define LMS_CRG_CLKGT0_W1S_GT_CLK_LMS_RTC1_SHIFT                              0
#define LMS_CRG_CLKGT0_W1S_GT_PCLK_LMS_RTC1_SHIFT                             1
#define LMS_CRG_CLKGT0_W1S_GT_CLK_VAD_SHIFT                                   2
#define LMS_CRG_CLKGT0_W1S_GT_CLK_VAD_PDM_SHIFT                               3
#define LMS_CRG_CLKGT0_W1S_GT_CLK_VAD_I2S_SHIFT                               4
#define LMS_CRG_CLKGT0_W1S_GT_PCLK_LMS_TIMER2_SHIFT                           5
#define LMS_CRG_CLKGT0_W1S_GT_CLK_LMS_TIMER00_SHIFT                           6
#define LMS_CRG_CLKGT0_W1S_GT_CLK_LMS_TIMER60_SHIFT                           7
#define LMS_CRG_CLKGT0_W1S_GT_CLK_LMS_TIMER80_SHIFT                           8
#define LMS_CRG_CLKGT0_W1S_GT_CLK_LMS_TIMER81_SHIFT                           9
#define LMS_CRG_CLKGT0_W1S_GT_CLK_CODEC_SHIFT                                 10
#define LMS_CRG_CLKGT0_W1C                   0x0004
#define LMS_CRG_CLKGT0_W1C_GT_CLK_LMS_RTC1_SHIFT                              0
#define LMS_CRG_CLKGT0_W1C_GT_PCLK_LMS_RTC1_SHIFT                             1
#define LMS_CRG_CLKGT0_W1C_GT_CLK_VAD_SHIFT                                   2
#define LMS_CRG_CLKGT0_W1C_GT_CLK_VAD_PDM_SHIFT                               3
#define LMS_CRG_CLKGT0_W1C_GT_CLK_VAD_I2S_SHIFT                               4
#define LMS_CRG_CLKGT0_W1C_GT_PCLK_LMS_TIMER2_SHIFT                           5
#define LMS_CRG_CLKGT0_W1C_GT_CLK_LMS_TIMER00_SHIFT                           6
#define LMS_CRG_CLKGT0_W1C_GT_CLK_LMS_TIMER60_SHIFT                           7
#define LMS_CRG_CLKGT0_W1C_GT_CLK_LMS_TIMER80_SHIFT                           8
#define LMS_CRG_CLKGT0_W1C_GT_CLK_LMS_TIMER81_SHIFT                           9
#define LMS_CRG_CLKGT0_W1C_GT_CLK_CODEC_SHIFT                                 10
#define LMS_CRG_CLKGT0_RO                    0x0008
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_RTC1_SHIFT                               0
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_RTC1_MASK                                0x00000001
#define LMS_CRG_CLKGT0_RO_GT_PCLK_LMS_RTC1_SHIFT                              1
#define LMS_CRG_CLKGT0_RO_GT_PCLK_LMS_RTC1_MASK                               0x00000002
#define LMS_CRG_CLKGT0_RO_GT_CLK_VAD_SHIFT                                    2
#define LMS_CRG_CLKGT0_RO_GT_CLK_VAD_MASK                                     0x00000004
#define LMS_CRG_CLKGT0_RO_GT_CLK_VAD_PDM_SHIFT                                3
#define LMS_CRG_CLKGT0_RO_GT_CLK_VAD_PDM_MASK                                 0x00000008
#define LMS_CRG_CLKGT0_RO_GT_CLK_VAD_I2S_SHIFT                                4
#define LMS_CRG_CLKGT0_RO_GT_CLK_VAD_I2S_MASK                                 0x00000010
#define LMS_CRG_CLKGT0_RO_GT_PCLK_LMS_TIMER2_SHIFT                            5
#define LMS_CRG_CLKGT0_RO_GT_PCLK_LMS_TIMER2_MASK                             0x00000020
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER00_SHIFT                            6
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER00_MASK                             0x00000040
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER60_SHIFT                            7
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER60_MASK                             0x00000080
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER80_SHIFT                            8
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER80_MASK                             0x00000100
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER81_SHIFT                            9
#define LMS_CRG_CLKGT0_RO_GT_CLK_LMS_TIMER81_MASK                             0x00000200
#define LMS_CRG_CLKGT0_RO_GT_CLK_CODEC_SHIFT                                  10
#define LMS_CRG_CLKGT0_RO_GT_CLK_CODEC_MASK                                   0x00000400
#define LMS_CRG_CLKST0                       0x000c
#define LMS_CRG_CLKST0_ST_CLK_LMS_RTC1_SHIFT                                  0
#define LMS_CRG_CLKST0_ST_CLK_LMS_RTC1_MASK                                   0x00000001
#define LMS_CRG_CLKST0_ST_PCLK_LMS_RTC1_SHIFT                                 1
#define LMS_CRG_CLKST0_ST_PCLK_LMS_RTC1_MASK                                  0x00000002
#define LMS_CRG_CLKST0_ST_CLK_VAD_SHIFT                                       2
#define LMS_CRG_CLKST0_ST_CLK_VAD_MASK                                        0x00000004
#define LMS_CRG_CLKST0_ST_CLK_VAD_PDM_SHIFT                                   3
#define LMS_CRG_CLKST0_ST_CLK_VAD_PDM_MASK                                    0x00000008
#define LMS_CRG_CLKST0_ST_CLK_VAD_I2S_SHIFT                                   4
#define LMS_CRG_CLKST0_ST_CLK_VAD_I2S_MASK                                    0x00000010
#define LMS_CRG_CLKST0_ST_PCLK_LMS_TIMER2_SHIFT                               5
#define LMS_CRG_CLKST0_ST_PCLK_LMS_TIMER2_MASK                                0x00000020
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER00_SHIFT                               6
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER00_MASK                                0x00000040
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER60_SHIFT                               7
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER60_MASK                                0x00000080
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER80_SHIFT                               8
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER80_MASK                                0x00000100
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER81_SHIFT                               9
#define LMS_CRG_CLKST0_ST_CLK_LMS_TIMER81_MASK                                0x00000200
#define LMS_CRG_CLKST0_ST_CLK_CODEC_SHIFT                                     10
#define LMS_CRG_CLKST0_ST_CLK_CODEC_MASK                                      0x00000400
#define LMS_CRG_RST0_W1S                     0x0050
#define LMS_CRG_RST0_W1S_IP_RST_LMS_RTC1_N_SHIFT                              0
#define LMS_CRG_RST0_W1S_IP_PRST_LMS_RTC1_N_SHIFT                             1
#define LMS_CRG_RST0_W1S_IP_RST_VAD_N_SHIFT                                   2
#define LMS_CRG_RST0_W1S_IP_RST_VAD_PDM_N_SHIFT                               3
#define LMS_CRG_RST0_W1S_IP_RST_VAD_I2S_N_SHIFT                               4
#define LMS_CRG_RST0_W1S_IP_PRST_LMS_TIMER2_N_SHIFT                           5
#define LMS_CRG_RST0_W1S_IP_RST_LMS_TIMER00_N_SHIFT                           6
#define LMS_CRG_RST0_W1S_IP_RST_LMS_TIMER60_N_SHIFT                           7
#define LMS_CRG_RST0_W1S_IP_RST_LMS_TIMER80_N_SHIFT                           8
#define LMS_CRG_RST0_W1S_IP_RST_LMS_TIMER81_N_SHIFT                           9
#define LMS_CRG_RST0_W1C                     0x0054
#define LMS_CRG_RST0_W1C_IP_RST_LMS_RTC1_N_SHIFT                              0
#define LMS_CRG_RST0_W1C_IP_PRST_LMS_RTC1_N_SHIFT                             1
#define LMS_CRG_RST0_W1C_IP_RST_VAD_N_SHIFT                                   2
#define LMS_CRG_RST0_W1C_IP_RST_VAD_PDM_N_SHIFT                               3
#define LMS_CRG_RST0_W1C_IP_RST_VAD_I2S_N_SHIFT                               4
#define LMS_CRG_RST0_W1C_IP_PRST_LMS_TIMER2_N_SHIFT                           5
#define LMS_CRG_RST0_W1C_IP_RST_LMS_TIMER00_N_SHIFT                           6
#define LMS_CRG_RST0_W1C_IP_RST_LMS_TIMER60_N_SHIFT                           7
#define LMS_CRG_RST0_W1C_IP_RST_LMS_TIMER80_N_SHIFT                           8
#define LMS_CRG_RST0_W1C_IP_RST_LMS_TIMER81_N_SHIFT                           9
#define LMS_CRG_RST0_RO                      0x0058
#define LMS_CRG_RST0_RO_IP_RST_LMS_RTC1_N_SHIFT                               0
#define LMS_CRG_RST0_RO_IP_RST_LMS_RTC1_N_MASK                                0x00000001
#define LMS_CRG_RST0_RO_IP_PRST_LMS_RTC1_N_SHIFT                              1
#define LMS_CRG_RST0_RO_IP_PRST_LMS_RTC1_N_MASK                               0x00000002
#define LMS_CRG_RST0_RO_IP_RST_VAD_N_SHIFT                                    2
#define LMS_CRG_RST0_RO_IP_RST_VAD_N_MASK                                     0x00000004
#define LMS_CRG_RST0_RO_IP_RST_VAD_PDM_N_SHIFT                                3
#define LMS_CRG_RST0_RO_IP_RST_VAD_PDM_N_MASK                                 0x00000008
#define LMS_CRG_RST0_RO_IP_RST_VAD_I2S_N_SHIFT                                4
#define LMS_CRG_RST0_RO_IP_RST_VAD_I2S_N_MASK                                 0x00000010
#define LMS_CRG_RST0_RO_IP_PRST_LMS_TIMER2_N_SHIFT                            5
#define LMS_CRG_RST0_RO_IP_PRST_LMS_TIMER2_N_MASK                             0x00000020
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER00_N_SHIFT                            6
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER00_N_MASK                             0x00000040
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER60_N_SHIFT                            7
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER60_N_MASK                             0x00000080
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER80_N_SHIFT                            8
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER80_N_MASK                             0x00000100
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER81_N_SHIFT                            9
#define LMS_CRG_RST0_RO_IP_RST_LMS_TIMER81_N_MASK                             0x00000200
#define LMS_CRG_CLKDIV0                      0x0080
#define LMS_CRG_CLKDIV0_DIV_CLK_VAD_SHIFT                                     0
#define LMS_CRG_CLKDIV0_DIV_CLK_VAD_MASK                                      0x0000003f
#define LMS_CRG_CLKDIV0_DIV_CLK_VAD_PDM_SHIFT                                 6
#define LMS_CRG_CLKDIV0_DIV_CLK_VAD_PDM_MASK                                  0x00000fc0
#define LMS_CRG_CLKDIV0_SC_GT_CLK_VAD_SHIFT                                   12
#define LMS_CRG_CLKDIV0_SC_GT_CLK_VAD_MASK                                    0x00001000
#define LMS_CRG_CLKDIV0_SC_GT_CLK_VAD_PDM_SHIFT                               13
#define LMS_CRG_CLKDIV0_SC_GT_CLK_VAD_PDM_MASK                                0x00002000
#define LMS_CRG_CLKDIV1                      0x0084
#define LMS_CRG_CLKDIV1_DIV_CLK_VAD_I2S_PRE_SHIFT                             0
#define LMS_CRG_CLKDIV1_DIV_CLK_VAD_I2S_PRE_MASK                              0x0000003f
#define LMS_CRG_CLKDIV1_DIV_CLK_VAD_I2S_SHIFT                                 6
#define LMS_CRG_CLKDIV1_DIV_CLK_VAD_I2S_MASK                                  0x000000c0
#define LMS_CRG_CLKDIV1_DIV_CLK_CODEC_SHIFT                                   8
#define LMS_CRG_CLKDIV1_DIV_CLK_CODEC_MASK                                    0x00003f00
#define LMS_CRG_CLKDIV1_SC_GT_CLK_VAD_I2S_SHIFT                               14
#define LMS_CRG_CLKDIV1_SC_GT_CLK_VAD_I2S_MASK                                0x00004000
#define LMS_CRG_CLKDIV1_SC_GT_CLK_CODEC_SHIFT                                 15
#define LMS_CRG_CLKDIV1_SC_GT_CLK_CODEC_MASK                                  0x00008000
#define LMS_CRG_PERISTAT0                    0x0100
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_SHIFT                              0
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_MASK                               0x00000001
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_PDM_SHIFT                          1
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_PDM_MASK                           0x00000002
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_PAD_WRAP_DB_SHIFT                      2
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_PAD_WRAP_DB_MASK                       0x00000004
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_CODEC_SHIFT                            3
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_CODEC_MASK                             0x00000008
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_I2S_PRE_SHIFT                      4
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_I2S_PRE_MASK                       0x00000010
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_I2S_SHIFT                          5
#define LMS_CRG_PERISTAT0_DIV_DONE_CLK_VAD_I2S_MASK                           0x00000020
#define LMS_CRG_PERISTAT1                    0x0104
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_SPMI_SHIFT                         0
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_SPMI_MASK                          0x00000001
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_TIMER_H_SHIFT                      1
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_TIMER_H_MASK                       0x00000002
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_TIMESTAMP_SHIFT                    5
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_TIMESTAMP_MASK                     0x00000020
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_SYSCNT_SHIFT                       7
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_LMS_SYSCNT_MASK                        0x00000080
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_PLL_LOGIC_SHIFT                        8
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_PLL_LOGIC_MASK                         0x00000100
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_PLL_LOGIC_LP32KPLL_DIV_SHIFT           9
#define LMS_CRG_PERISTAT1_DIV_DONE_CLK_PLL_LOGIC_LP32KPLL_DIV_MASK            0x00000200
#define LMS_CRG_PERISTAT2                    0x0108
#define LMS_CRG_PERISTAT2_SW_ACK_PCLK_LMS_SW_SHIFT                            0
#define LMS_CRG_PERISTAT2_SW_ACK_PCLK_LMS_SW_MASK                             0x00000007
#define LMS_CRG_PERISTAT2_SW_ACK_CLK_LMS_SPMI_SW_SHIFT                        3
#define LMS_CRG_PERISTAT2_SW_ACK_CLK_LMS_SPMI_SW_MASK                         0x00000018
#define LMS_CRG_PERISTAT2_SW_ACK_CLK_PLL_LOGIC_SHIFT                          5
#define LMS_CRG_PERISTAT2_SW_ACK_CLK_PLL_LOGIC_MASK                           0x00000060
#define LMS_CRG_CLKDIV2                      0x0110
#define LMS_CRG_CLKDIV2_SEL_CLK_VAD_SHIFT                                     0
#define LMS_CRG_CLKDIV2_SEL_CLK_VAD_MASK                                      0x00000003
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER00_SHIFT                             2
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER00_MASK                              0x00000004
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER60_SHIFT                             3
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER60_MASK                              0x00000008
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER80_SHIFT                             4
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER80_MASK                              0x00000010
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER81_SHIFT                             5
#define LMS_CRG_CLKDIV2_SEL_CLK_LMS_TIMER81_MASK                              0x00000020
#define LMS_CRG_CLKDIV2_SEL_CLK_CODEC_SHIFT                                   6
#define LMS_CRG_CLKDIV2_SEL_CLK_CODEC_MASK                                    0x000000c0
#define LMS_CRG_CLKDIV2_SEL_UFS_RST_ISO_VALUE_SHIFT                           8
#define LMS_CRG_CLKDIV2_SEL_UFS_RST_ISO_VALUE_MASK                            0x00000100
#define LMS_CRG_PLL_LOCK_STATE               0x0200
#define LMS_CRG_PLL_LOCK_STATE_LP32KPLL_LOCK_SHIFT                            0
#define LMS_CRG_PLL_LOCK_STATE_LP32KPLL_LOCK_MASK                             0x00000001
#define LMS_CRG_PLL_LOCK_STATE_LPPLL_LOCK_SHIFT                               1
#define LMS_CRG_PLL_LOCK_STATE_LPPLL_LOCK_MASK                                0x00000002
#define LMS_CRG_INTR_CLEAR_LPPLL             0x0210
#define LMS_CRG_INTR_CLEAR_LPPLL_INTR_CLEAR_PLL_INI_ERR_LPPLL_SHIFT           0
#define LMS_CRG_INTR_CLEAR_LPPLL_INTR_CLEAR_PLL_INI_ERR_LPPLL_MASK            0x00000001
#define LMS_CRG_INTR_CLEAR_LPPLL_INTR_CLEAR_PLL_UNLOCK_LPPLL_SHIFT            1
#define LMS_CRG_INTR_CLEAR_LPPLL_INTR_CLEAR_PLL_UNLOCK_LPPLL_MASK             0x00000002
#define LMS_CRG_INTR_CLEAR_LPPLL_INTR_CLEAR_VOTE_REQ_ERR_LPPLL_SHIFT          2
#define LMS_CRG_INTR_CLEAR_LPPLL_INTR_CLEAR_VOTE_REQ_ERR_LPPLL_MASK           0x00000004
#define LMS_CRG_INTR_MASK_LPPLL              0x0214
#define LMS_CRG_INTR_MASK_LPPLL_INTR_MASK_PLL_INI_ERR_LPPLL_SHIFT             0
#define LMS_CRG_INTR_MASK_LPPLL_INTR_MASK_PLL_INI_ERR_LPPLL_MASK              0x00000001
#define LMS_CRG_INTR_MASK_LPPLL_INTR_MASK_PLL_UNLOCK_LPPLL_SHIFT              1
#define LMS_CRG_INTR_MASK_LPPLL_INTR_MASK_PLL_UNLOCK_LPPLL_MASK               0x00000002
#define LMS_CRG_INTR_MASK_LPPLL_INTR_MASK_VOTE_REQ_ERR_LPPLL_SHIFT            2
#define LMS_CRG_INTR_MASK_LPPLL_INTR_MASK_VOTE_REQ_ERR_LPPLL_MASK             0x00000004
#define LMS_CRG_INTR_STATUS_PLL              0x0218
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_LPPLL_SHIFT           0
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_LPPLL_MASK            0x00000001
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_LPPLL_SHIFT            1
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_LPPLL_MASK             0x00000002
#define LMS_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_LPPLL_SHIFT          2
#define LMS_CRG_INTR_STATUS_PLL_INTR_VOTE_REQ_ERR_STATUS_LPPLL_MASK           0x00000004
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_LP32KPLL_SHIFT        3
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_INI_ERR_STATUS_LP32KPLL_MASK         0x00000008
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_LP32KPLL_SHIFT         4
#define LMS_CRG_INTR_STATUS_PLL_INTR_PLL_UNLOCK_STATUS_LP32KPLL_MASK          0x00000010
#define LMS_CRG_INTR_STATUS_PLL_MSK          0x021c
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_LPPLL_SHIFT   0
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_LPPLL_MASK    0x00000001
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_LPPLL_SHIFT    1
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_LPPLL_MASK     0x00000002
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_LPPLL_SHIFT  2
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_VOTE_REQ_ERR_STATUS_MSK_LPPLL_MASK   0x00000004
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_LP32KPLL_SHIFT 3
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_INI_ERR_STATUS_MSK_LP32KPLL_MASK  0x00000008
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_LP32KPLL_SHIFT 4
#define LMS_CRG_INTR_STATUS_PLL_MSK_INTR_PLL_UNLOCK_STATUS_MSK_LP32KPLL_MASK  0x00000010
#define LMS_CRG_INTR_CLEAR_LP32KPLL          0x0220
#define LMS_CRG_INTR_CLEAR_LP32KPLL_INTR_CLEAR_PLL_INI_ERR_LP32KPLL_SHIFT     0
#define LMS_CRG_INTR_CLEAR_LP32KPLL_INTR_CLEAR_PLL_INI_ERR_LP32KPLL_MASK      0x00000001
#define LMS_CRG_INTR_CLEAR_LP32KPLL_INTR_CLEAR_PLL_UNLOCK_LP32KPLL_SHIFT      1
#define LMS_CRG_INTR_CLEAR_LP32KPLL_INTR_CLEAR_PLL_UNLOCK_LP32KPLL_MASK       0x00000002
#define LMS_CRG_INTR_MASK_LP32KPLL           0x0224
#define LMS_CRG_INTR_MASK_LP32KPLL_INTR_MASK_PLL_INI_ERR_LP32KPLL_SHIFT       0
#define LMS_CRG_INTR_MASK_LP32KPLL_INTR_MASK_PLL_INI_ERR_LP32KPLL_MASK        0x00000001
#define LMS_CRG_INTR_MASK_LP32KPLL_INTR_MASK_PLL_UNLOCK_LP32KPLL_SHIFT        1
#define LMS_CRG_INTR_MASK_LP32KPLL_INTR_MASK_PLL_UNLOCK_LP32KPLL_MASK         0x00000002
#define LMS_CRG_CLKDIV10                     0x0300
#define LMS_CRG_CLKDIV10_DIV_CLK_LPPLL_TEST_SHIFT                             0
#define LMS_CRG_CLKDIV10_DIV_CLK_LPPLL_TEST_MASK                              0x0000003f
#define LMS_CRG_CLKDIV10_SC_GT_CLK_LPPLL_TEST_SHIFT                           12
#define LMS_CRG_CLKDIV10_SC_GT_CLK_LPPLL_TEST_MASK                            0x00001000
#define LMS_CRG_PERICTRL6                    0x0310
#define LMS_CRG_PERICTRL6_SEL_CLK_LMS_PLL_TEST_SHIFT                          0
#define LMS_CRG_PERICTRL6_SEL_CLK_LMS_PLL_TEST_MASK                           0x00000001
#define LMS_CRG_LPPLL_VOTE_EN                0x0400
#define LMS_CRG_LPPLL_VOTE_EN_LPPLL_VOTE_EN_SHIFT                             0
#define LMS_CRG_LPPLL_VOTE_EN_LPPLL_VOTE_EN_MASK                              0x000000ff
#define LMS_CRG_CLKGT1_W1S                   0x0800
#define LMS_CRG_CLKGT1_W1S_GT_CLK_LMS_SPMI_SHIFT                              0
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_PAD_WRAP_SHIFT                             1
#define LMS_CRG_CLKGT1_W1S_GT_CLK_PAD_WRAP_DB_SHIFT                           2
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_SYSCNT_SHIFT                           6
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_RTC0_SHIFT                             8
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_TIMER1_SHIFT                           11
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_TIMER0_SHIFT                           12
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_WKUPMNG_SHIFT                          14
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_SPMI_SHIFT                             15
#define LMS_CRG_CLKGT1_W1S_GT_CLK_LMS_RTC0_SHIFT                              18
#define LMS_CRG_CLKGT1_W1S_GT_CLKIN_REF_LPIS_SHIFT                            21
#define LMS_CRG_CLKGT1_W1S_GT_CLK_LPPLL_PERI_SHIFT                            22
#define LMS_CRG_CLKGT1_W1S_GT_CLK_LPPLL_LPIS_SHIFT                            23
#define LMS_CRG_CLKGT1_W1S_GT_CLK_SYS_PERI_SHIFT                              24
#define LMS_CRG_CLKGT1_W1S_GT_CLK_SYS_LPIS_SHIFT                              25
#define LMS_CRG_CLKGT1_W1S_GT_CLK_LMS_SYSCNT_SHIFT                            26
#define LMS_CRG_CLKGT1_W1S_GT_CLK_LMS_TIMESTAMP_SHIFT                         28
#define LMS_CRG_CLKGT1_W1S_GT_PCLK_LMS_TIMESTAMP_SHIFT                        29
#define LMS_CRG_CLKGT1_W1C                   0x0804
#define LMS_CRG_CLKGT1_W1C_GT_CLK_LMS_SPMI_SHIFT                              0
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_PAD_WRAP_SHIFT                             1
#define LMS_CRG_CLKGT1_W1C_GT_CLK_PAD_WRAP_DB_SHIFT                           2
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_SYSCNT_SHIFT                           6
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_RTC0_SHIFT                             8
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_TIMER1_SHIFT                           11
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_TIMER0_SHIFT                           12
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_WKUPMNG_SHIFT                          14
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_SPMI_SHIFT                             15
#define LMS_CRG_CLKGT1_W1C_GT_CLK_LMS_RTC0_SHIFT                              18
#define LMS_CRG_CLKGT1_W1C_GT_CLKIN_REF_LPIS_SHIFT                            21
#define LMS_CRG_CLKGT1_W1C_GT_CLK_LPPLL_PERI_SHIFT                            22
#define LMS_CRG_CLKGT1_W1C_GT_CLK_LPPLL_LPIS_SHIFT                            23
#define LMS_CRG_CLKGT1_W1C_GT_CLK_SYS_PERI_SHIFT                              24
#define LMS_CRG_CLKGT1_W1C_GT_CLK_SYS_LPIS_SHIFT                              25
#define LMS_CRG_CLKGT1_W1C_GT_CLK_LMS_SYSCNT_SHIFT                            26
#define LMS_CRG_CLKGT1_W1C_GT_CLK_LMS_TIMESTAMP_SHIFT                         28
#define LMS_CRG_CLKGT1_W1C_GT_PCLK_LMS_TIMESTAMP_SHIFT                        29
#define LMS_CRG_CLKGT1_RO                    0x0808
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_SPMI_SHIFT                               0
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_SPMI_MASK                                0x00000001
#define LMS_CRG_CLKGT1_RO_GT_PCLK_PAD_WRAP_SHIFT                              1
#define LMS_CRG_CLKGT1_RO_GT_PCLK_PAD_WRAP_MASK                               0x00000002
#define LMS_CRG_CLKGT1_RO_GT_CLK_PAD_WRAP_DB_SHIFT                            2
#define LMS_CRG_CLKGT1_RO_GT_CLK_PAD_WRAP_DB_MASK                             0x00000004
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_SYSCNT_SHIFT                            6
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_SYSCNT_MASK                             0x00000040
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_RTC0_SHIFT                              8
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_RTC0_MASK                               0x00000100
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_TIMER1_SHIFT                            11
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_TIMER1_MASK                             0x00000800
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_TIMER0_SHIFT                            12
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_TIMER0_MASK                             0x00001000
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_WKUPMNG_SHIFT                           14
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_WKUPMNG_MASK                            0x00004000
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_SPMI_SHIFT                              15
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_SPMI_MASK                               0x00008000
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_RTC0_SHIFT                               18
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_RTC0_MASK                                0x00040000
#define LMS_CRG_CLKGT1_RO_GT_CLKIN_REF_LPIS_SHIFT                             21
#define LMS_CRG_CLKGT1_RO_GT_CLKIN_REF_LPIS_MASK                              0x00200000
#define LMS_CRG_CLKGT1_RO_GT_CLK_LPPLL_PERI_SHIFT                             22
#define LMS_CRG_CLKGT1_RO_GT_CLK_LPPLL_PERI_MASK                              0x00400000
#define LMS_CRG_CLKGT1_RO_GT_CLK_LPPLL_LPIS_SHIFT                             23
#define LMS_CRG_CLKGT1_RO_GT_CLK_LPPLL_LPIS_MASK                              0x00800000
#define LMS_CRG_CLKGT1_RO_GT_CLK_SYS_PERI_SHIFT                               24
#define LMS_CRG_CLKGT1_RO_GT_CLK_SYS_PERI_MASK                                0x01000000
#define LMS_CRG_CLKGT1_RO_GT_CLK_SYS_LPIS_SHIFT                               25
#define LMS_CRG_CLKGT1_RO_GT_CLK_SYS_LPIS_MASK                                0x02000000
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_SYSCNT_SHIFT                             26
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_SYSCNT_MASK                              0x04000000
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_TIMESTAMP_SHIFT                          28
#define LMS_CRG_CLKGT1_RO_GT_CLK_LMS_TIMESTAMP_MASK                           0x10000000
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_TIMESTAMP_SHIFT                         29
#define LMS_CRG_CLKGT1_RO_GT_PCLK_LMS_TIMESTAMP_MASK                          0x20000000
#define LMS_CRG_CLKST1                       0x080c
#define LMS_CRG_CLKST1_ST_CLK_LMS_SPMI_SHIFT                                  0
#define LMS_CRG_CLKST1_ST_CLK_LMS_SPMI_MASK                                   0x00000001
#define LMS_CRG_CLKST1_ST_PCLK_PAD_WRAP_SHIFT                                 1
#define LMS_CRG_CLKST1_ST_PCLK_PAD_WRAP_MASK                                  0x00000002
#define LMS_CRG_CLKST1_ST_CLK_PAD_WRAP_DB_SHIFT                               2
#define LMS_CRG_CLKST1_ST_CLK_PAD_WRAP_DB_MASK                                0x00000004
#define LMS_CRG_CLKST1_ST_PCLK_LMS_SYSCNT_SHIFT                               6
#define LMS_CRG_CLKST1_ST_PCLK_LMS_SYSCNT_MASK                                0x00000040
#define LMS_CRG_CLKST1_ST_PCLK_LMS_RTC0_SHIFT                                 8
#define LMS_CRG_CLKST1_ST_PCLK_LMS_RTC0_MASK                                  0x00000100
#define LMS_CRG_CLKST1_ST_PCLK_LMS_TIMER1_SHIFT                               11
#define LMS_CRG_CLKST1_ST_PCLK_LMS_TIMER1_MASK                                0x00000800
#define LMS_CRG_CLKST1_ST_PCLK_LMS_TIMER0_SHIFT                               12
#define LMS_CRG_CLKST1_ST_PCLK_LMS_TIMER0_MASK                                0x00001000
#define LMS_CRG_CLKST1_ST_PCLK_LMS_WKUPMNG_SHIFT                              14
#define LMS_CRG_CLKST1_ST_PCLK_LMS_WKUPMNG_MASK                               0x00004000
#define LMS_CRG_CLKST1_ST_PCLK_LMS_SPMI_SHIFT                                 15
#define LMS_CRG_CLKST1_ST_PCLK_LMS_SPMI_MASK                                  0x00008000
#define LMS_CRG_CLKST1_ST_CLK_LMS_RTC0_SHIFT                                  18
#define LMS_CRG_CLKST1_ST_CLK_LMS_RTC0_MASK                                   0x00040000
#define LMS_CRG_CLKST1_ST_CLKIN_REF_LPIS_SHIFT                                21
#define LMS_CRG_CLKST1_ST_CLKIN_REF_LPIS_MASK                                 0x00200000
#define LMS_CRG_CLKST1_ST_CLK_LPPLL_PERI_SHIFT                                22
#define LMS_CRG_CLKST1_ST_CLK_LPPLL_PERI_MASK                                 0x00400000
#define LMS_CRG_CLKST1_ST_CLK_LPPLL_LPIS_SHIFT                                23
#define LMS_CRG_CLKST1_ST_CLK_LPPLL_LPIS_MASK                                 0x00800000
#define LMS_CRG_CLKST1_ST_CLK_SYS_PERI_SHIFT                                  24
#define LMS_CRG_CLKST1_ST_CLK_SYS_PERI_MASK                                   0x01000000
#define LMS_CRG_CLKST1_ST_CLK_SYS_LPIS_SHIFT                                  25
#define LMS_CRG_CLKST1_ST_CLK_SYS_LPIS_MASK                                   0x02000000
#define LMS_CRG_CLKST1_ST_CLK_LMS_SYSCNT_SHIFT                                26
#define LMS_CRG_CLKST1_ST_CLK_LMS_SYSCNT_MASK                                 0x04000000
#define LMS_CRG_CLKST1_ST_CLK_LMS_TIMESTAMP_SHIFT                             28
#define LMS_CRG_CLKST1_ST_CLK_LMS_TIMESTAMP_MASK                              0x10000000
#define LMS_CRG_CLKST1_ST_PCLK_LMS_TIMESTAMP_SHIFT                            29
#define LMS_CRG_CLKST1_ST_PCLK_LMS_TIMESTAMP_MASK                             0x20000000
#define LMS_CRG_CLKGT2_W1S                   0x0810
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER01_SHIFT                           1
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER10_SHIFT                           2
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER11_SHIFT                           3
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER20_SHIFT                           4
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER21_SHIFT                           5
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER30_SHIFT                           6
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER31_SHIFT                           7
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER40_SHIFT                           8
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER41_SHIFT                           9
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER50_SHIFT                           10
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER51_SHIFT                           11
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER61_SHIFT                           13
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER70_SHIFT                           14
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_TIMER71_SHIFT                           15
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LPPLL_LOGIC_SHIFT                           17
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LP32KPLL_LOGIC_SHIFT                        18
#define LMS_CRG_CLKGT2_W1S_GT_CLK_SYS_HSS1_SHIFT                              19
#define LMS_CRG_CLKGT2_W1S_GT_CLK_LMS_WDT_SHIFT                               20
#define LMS_CRG_CLKGT2_W1S_GT_PCLK_LMS_WDT_SHIFT                              21
#define LMS_CRG_CLKGT2_W1S_GT_CLK_PLL_LOGIC_SHIFT                             22
#define LMS_CRG_CLKGT2_W1C                   0x0814
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER01_SHIFT                           1
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER10_SHIFT                           2
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER11_SHIFT                           3
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER20_SHIFT                           4
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER21_SHIFT                           5
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER30_SHIFT                           6
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER31_SHIFT                           7
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER40_SHIFT                           8
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER41_SHIFT                           9
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER50_SHIFT                           10
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER51_SHIFT                           11
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER61_SHIFT                           13
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER70_SHIFT                           14
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_TIMER71_SHIFT                           15
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LPPLL_LOGIC_SHIFT                           17
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LP32KPLL_LOGIC_SHIFT                        18
#define LMS_CRG_CLKGT2_W1C_GT_CLK_SYS_HSS1_SHIFT                              19
#define LMS_CRG_CLKGT2_W1C_GT_CLK_LMS_WDT_SHIFT                               20
#define LMS_CRG_CLKGT2_W1C_GT_PCLK_LMS_WDT_SHIFT                              21
#define LMS_CRG_CLKGT2_W1C_GT_CLK_PLL_LOGIC_SHIFT                             22
#define LMS_CRG_CLKGT2_RO                    0x0818
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER01_SHIFT                            1
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER01_MASK                             0x00000002
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER10_SHIFT                            2
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER10_MASK                             0x00000004
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER11_SHIFT                            3
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER11_MASK                             0x00000008
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER20_SHIFT                            4
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER20_MASK                             0x00000010
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER21_SHIFT                            5
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER21_MASK                             0x00000020
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER30_SHIFT                            6
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER30_MASK                             0x00000040
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER31_SHIFT                            7
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER31_MASK                             0x00000080
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER40_SHIFT                            8
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER40_MASK                             0x00000100
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER41_SHIFT                            9
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER41_MASK                             0x00000200
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER50_SHIFT                            10
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER50_MASK                             0x00000400
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER51_SHIFT                            11
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER51_MASK                             0x00000800
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER61_SHIFT                            13
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER61_MASK                             0x00002000
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER70_SHIFT                            14
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER70_MASK                             0x00004000
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER71_SHIFT                            15
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_TIMER71_MASK                             0x00008000
#define LMS_CRG_CLKGT2_RO_GT_CLK_LPPLL_LOGIC_SHIFT                            17
#define LMS_CRG_CLKGT2_RO_GT_CLK_LPPLL_LOGIC_MASK                             0x00020000
#define LMS_CRG_CLKGT2_RO_GT_CLK_LP32KPLL_LOGIC_SHIFT                         18
#define LMS_CRG_CLKGT2_RO_GT_CLK_LP32KPLL_LOGIC_MASK                          0x00040000
#define LMS_CRG_CLKGT2_RO_GT_CLK_SYS_HSS1_SHIFT                               19
#define LMS_CRG_CLKGT2_RO_GT_CLK_SYS_HSS1_MASK                                0x00080000
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_WDT_SHIFT                                20
#define LMS_CRG_CLKGT2_RO_GT_CLK_LMS_WDT_MASK                                 0x00100000
#define LMS_CRG_CLKGT2_RO_GT_PCLK_LMS_WDT_SHIFT                               21
#define LMS_CRG_CLKGT2_RO_GT_PCLK_LMS_WDT_MASK                                0x00200000
#define LMS_CRG_CLKGT2_RO_GT_CLK_PLL_LOGIC_SHIFT                              22
#define LMS_CRG_CLKGT2_RO_GT_CLK_PLL_LOGIC_MASK                               0x00400000
#define LMS_CRG_CLKST2                       0x081c
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER01_SHIFT                               1
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER01_MASK                                0x00000002
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER10_SHIFT                               2
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER10_MASK                                0x00000004
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER11_SHIFT                               3
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER11_MASK                                0x00000008
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER20_SHIFT                               4
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER20_MASK                                0x00000010
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER21_SHIFT                               5
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER21_MASK                                0x00000020
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER30_SHIFT                               6
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER30_MASK                                0x00000040
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER31_SHIFT                               7
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER31_MASK                                0x00000080
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER40_SHIFT                               8
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER40_MASK                                0x00000100
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER41_SHIFT                               9
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER41_MASK                                0x00000200
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER50_SHIFT                               10
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER50_MASK                                0x00000400
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER51_SHIFT                               11
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER51_MASK                                0x00000800
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER61_SHIFT                               13
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER61_MASK                                0x00002000
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER70_SHIFT                               14
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER70_MASK                                0x00004000
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER71_SHIFT                               15
#define LMS_CRG_CLKST2_ST_CLK_LMS_TIMER71_MASK                                0x00008000
#define LMS_CRG_CLKST2_ST_CLK_LPPLL_LOGIC_SHIFT                               17
#define LMS_CRG_CLKST2_ST_CLK_LPPLL_LOGIC_MASK                                0x00020000
#define LMS_CRG_CLKST2_ST_CLK_LP32KPLL_LOGIC_SHIFT                            18
#define LMS_CRG_CLKST2_ST_CLK_LP32KPLL_LOGIC_MASK                             0x00040000
#define LMS_CRG_CLKST2_ST_CLK_SYS_HSS1_SHIFT                                  19
#define LMS_CRG_CLKST2_ST_CLK_SYS_HSS1_MASK                                   0x00080000
#define LMS_CRG_CLKST2_ST_CLK_LMS_WDT_SHIFT                                   20
#define LMS_CRG_CLKST2_ST_CLK_LMS_WDT_MASK                                    0x00100000
#define LMS_CRG_CLKST2_ST_PCLK_LMS_WDT_SHIFT                                  21
#define LMS_CRG_CLKST2_ST_PCLK_LMS_WDT_MASK                                   0x00200000
#define LMS_CRG_CLKST2_ST_CLK_PLL_LOGIC_SHIFT                                 22
#define LMS_CRG_CLKST2_ST_CLK_PLL_LOGIC_MASK                                  0x00400000
#define LMS_CRG_CLKST2_ST_CLK_SYS_LMS_SHIFT                                   30
#define LMS_CRG_CLKST2_ST_CLK_SYS_LMS_MASK                                    0x40000000
#define LMS_CRG_CLKST2_ST_PCLK_LMS_LPIS_SHIFT                                 31
#define LMS_CRG_CLKST2_ST_PCLK_LMS_LPIS_MASK                                  0x80000000
#define LMS_CRG_RST1_W1S                     0x0820
#define LMS_CRG_RST1_W1S_IP_RST_LMS_SPMI_N_SHIFT                              0
#define LMS_CRG_RST1_W1S_IP_PRST_PAD_WRAP_N_SHIFT                             1
#define LMS_CRG_RST1_W1S_IP_RST_PAD_WRAP_DB_N_SHIFT                           2
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_SYSCNT_N_SHIFT                           5
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_TIMESTAMP_N_SHIFT                        6
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_RTC0_N_SHIFT                             7
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_TIMER1_N_SHIFT                           10
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_TIMER0_N_SHIFT                           11
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_WKUPMNG_N_SHIFT                          13
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_SPMI_N_SHIFT                             14
#define LMS_CRG_RST1_W1S_IP_PRST_LMS_N_SHIFT                                  15
#define LMS_CRG_RST1_W1S_IP_RST_LMS_RTC0_N_SHIFT                              16
#define LMS_CRG_RST1_W1S_IP_RST_LMS_TIMESTAMP_N_SHIFT                         20
#define LMS_CRG_RST1_W1S_IP_RST_LMS_SYSCNT_N_SHIFT                            21
#define LMS_CRG_RST1_W1S_IP_RST_LP32KPLL_LOGIC_N_SHIFT                        22
#define LMS_CRG_RST1_W1S_IP_RST_LPPLL_LOGIC_N_SHIFT                           23
#define LMS_CRG_RST1_W1C                     0x0824
#define LMS_CRG_RST1_W1C_IP_RST_LMS_SPMI_N_SHIFT                              0
#define LMS_CRG_RST1_W1C_IP_PRST_PAD_WRAP_N_SHIFT                             1
#define LMS_CRG_RST1_W1C_IP_RST_PAD_WRAP_DB_N_SHIFT                           2
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_SYSCNT_N_SHIFT                           5
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_TIMESTAMP_N_SHIFT                        6
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_RTC0_N_SHIFT                             7
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_TIMER1_N_SHIFT                           10
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_TIMER0_N_SHIFT                           11
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_WKUPMNG_N_SHIFT                          13
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_SPMI_N_SHIFT                             14
#define LMS_CRG_RST1_W1C_IP_PRST_LMS_N_SHIFT                                  15
#define LMS_CRG_RST1_W1C_IP_RST_LMS_RTC0_N_SHIFT                              16
#define LMS_CRG_RST1_W1C_IP_RST_LMS_TIMESTAMP_N_SHIFT                         20
#define LMS_CRG_RST1_W1C_IP_RST_LMS_SYSCNT_N_SHIFT                            21
#define LMS_CRG_RST1_W1C_IP_RST_LP32KPLL_LOGIC_N_SHIFT                        22
#define LMS_CRG_RST1_W1C_IP_RST_LPPLL_LOGIC_N_SHIFT                           23
#define LMS_CRG_RST1_RO                      0x0828
#define LMS_CRG_RST1_RO_IP_RST_LMS_SPMI_N_SHIFT                               0
#define LMS_CRG_RST1_RO_IP_RST_LMS_SPMI_N_MASK                                0x00000001
#define LMS_CRG_RST1_RO_IP_PRST_PAD_WRAP_N_SHIFT                              1
#define LMS_CRG_RST1_RO_IP_PRST_PAD_WRAP_N_MASK                               0x00000002
#define LMS_CRG_RST1_RO_IP_RST_PAD_WRAP_DB_N_SHIFT                            2
#define LMS_CRG_RST1_RO_IP_RST_PAD_WRAP_DB_N_MASK                             0x00000004
#define LMS_CRG_RST1_RO_IP_PRST_LMS_SYSCNT_N_SHIFT                            5
#define LMS_CRG_RST1_RO_IP_PRST_LMS_SYSCNT_N_MASK                             0x00000020
#define LMS_CRG_RST1_RO_IP_PRST_LMS_TIMESTAMP_N_SHIFT                         6
#define LMS_CRG_RST1_RO_IP_PRST_LMS_TIMESTAMP_N_MASK                          0x00000040
#define LMS_CRG_RST1_RO_IP_PRST_LMS_RTC0_N_SHIFT                              7
#define LMS_CRG_RST1_RO_IP_PRST_LMS_RTC0_N_MASK                               0x00000080
#define LMS_CRG_RST1_RO_IP_PRST_LMS_TIMER1_N_SHIFT                            10
#define LMS_CRG_RST1_RO_IP_PRST_LMS_TIMER1_N_MASK                             0x00000400
#define LMS_CRG_RST1_RO_IP_PRST_LMS_TIMER0_N_SHIFT                            11
#define LMS_CRG_RST1_RO_IP_PRST_LMS_TIMER0_N_MASK                             0x00000800
#define LMS_CRG_RST1_RO_IP_PRST_LMS_WKUPMNG_N_SHIFT                           13
#define LMS_CRG_RST1_RO_IP_PRST_LMS_WKUPMNG_N_MASK                            0x00002000
#define LMS_CRG_RST1_RO_IP_PRST_LMS_SPMI_N_SHIFT                              14
#define LMS_CRG_RST1_RO_IP_PRST_LMS_SPMI_N_MASK                               0x00004000
#define LMS_CRG_RST1_RO_IP_PRST_LMS_N_SHIFT                                   15
#define LMS_CRG_RST1_RO_IP_PRST_LMS_N_MASK                                    0x00008000
#define LMS_CRG_RST1_RO_IP_RST_LMS_RTC0_N_SHIFT                               16
#define LMS_CRG_RST1_RO_IP_RST_LMS_RTC0_N_MASK                                0x00010000
#define LMS_CRG_RST1_RO_IP_RST_LMS_TIMESTAMP_N_SHIFT                          20
#define LMS_CRG_RST1_RO_IP_RST_LMS_TIMESTAMP_N_MASK                           0x00100000
#define LMS_CRG_RST1_RO_IP_RST_LMS_SYSCNT_N_SHIFT                             21
#define LMS_CRG_RST1_RO_IP_RST_LMS_SYSCNT_N_MASK                              0x00200000
#define LMS_CRG_RST1_RO_IP_RST_LP32KPLL_LOGIC_N_SHIFT                         22
#define LMS_CRG_RST1_RO_IP_RST_LP32KPLL_LOGIC_N_MASK                          0x00400000
#define LMS_CRG_RST1_RO_IP_RST_LPPLL_LOGIC_N_SHIFT                            23
#define LMS_CRG_RST1_RO_IP_RST_LPPLL_LOGIC_N_MASK                             0x00800000
#define LMS_CRG_RST2_W1S                     0x0830
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER01_N_SHIFT                           1
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER10_N_SHIFT                           2
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER11_N_SHIFT                           3
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER20_N_SHIFT                           4
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER21_N_SHIFT                           5
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER30_N_SHIFT                           6
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER31_N_SHIFT                           7
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER40_N_SHIFT                           8
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER41_N_SHIFT                           9
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER50_N_SHIFT                           10
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER51_N_SHIFT                           11
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER61_N_SHIFT                           13
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER70_N_SHIFT                           14
#define LMS_CRG_RST2_W1S_IP_RST_LMS_TIMER71_N_SHIFT                           15
#define LMS_CRG_RST2_W1S_IP_PRST_LMS_WDT_N_SHIFT                              16
#define LMS_CRG_RST2_W1S_IP_RST_LMS_WDT_N_SHIFT                               17
#define LMS_CRG_RST2_W1C                     0x0834
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER01_N_SHIFT                           1
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER10_N_SHIFT                           2
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER11_N_SHIFT                           3
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER20_N_SHIFT                           4
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER21_N_SHIFT                           5
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER30_N_SHIFT                           6
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER31_N_SHIFT                           7
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER40_N_SHIFT                           8
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER41_N_SHIFT                           9
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER50_N_SHIFT                           10
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER51_N_SHIFT                           11
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER61_N_SHIFT                           13
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER70_N_SHIFT                           14
#define LMS_CRG_RST2_W1C_IP_RST_LMS_TIMER71_N_SHIFT                           15
#define LMS_CRG_RST2_W1C_IP_PRST_LMS_WDT_N_SHIFT                              16
#define LMS_CRG_RST2_W1C_IP_RST_LMS_WDT_N_SHIFT                               17
#define LMS_CRG_RST2_RO                      0x0838
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER01_N_SHIFT                            1
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER01_N_MASK                             0x00000002
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER10_N_SHIFT                            2
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER10_N_MASK                             0x00000004
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER11_N_SHIFT                            3
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER11_N_MASK                             0x00000008
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER20_N_SHIFT                            4
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER20_N_MASK                             0x00000010
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER21_N_SHIFT                            5
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER21_N_MASK                             0x00000020
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER30_N_SHIFT                            6
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER30_N_MASK                             0x00000040
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER31_N_SHIFT                            7
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER31_N_MASK                             0x00000080
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER40_N_SHIFT                            8
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER40_N_MASK                             0x00000100
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER41_N_SHIFT                            9
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER41_N_MASK                             0x00000200
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER50_N_SHIFT                            10
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER50_N_MASK                             0x00000400
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER51_N_SHIFT                            11
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER51_N_MASK                             0x00000800
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER61_N_SHIFT                            13
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER61_N_MASK                             0x00002000
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER70_N_SHIFT                            14
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER70_N_MASK                             0x00004000
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER71_N_SHIFT                            15
#define LMS_CRG_RST2_RO_IP_RST_LMS_TIMER71_N_MASK                             0x00008000
#define LMS_CRG_RST2_RO_IP_PRST_LMS_WDT_N_SHIFT                               16
#define LMS_CRG_RST2_RO_IP_PRST_LMS_WDT_N_MASK                                0x00010000
#define LMS_CRG_RST2_RO_IP_RST_LMS_WDT_N_SHIFT                                17
#define LMS_CRG_RST2_RO_IP_RST_LMS_WDT_N_MASK                                 0x00020000
#define LMS_CRG_CLKDIV3                      0x0880
#define LMS_CRG_CLKDIV3_DIV_CLK_LMS_SPMI_SHIFT                                0
#define LMS_CRG_CLKDIV3_DIV_CLK_LMS_SPMI_MASK                                 0x0000003f
#define LMS_CRG_CLKDIV3_DIV_CLK_LMS_TIMER_H_SHIFT                             6
#define LMS_CRG_CLKDIV3_DIV_CLK_LMS_TIMER_H_MASK                              0x00000fc0
#define LMS_CRG_CLKDIV3_SC_GT_CLK_LMS_SPMI_SHIFT                              12
#define LMS_CRG_CLKDIV3_SC_GT_CLK_LMS_SPMI_MASK                               0x00001000
#define LMS_CRG_CLKDIV3_SC_GT_CLK_LMS_TIMER_H_SHIFT                           13
#define LMS_CRG_CLKDIV3_SC_GT_CLK_LMS_TIMER_H_MASK                            0x00002000
#define LMS_CRG_CLKDIV4                      0x0884
#define LMS_CRG_CLKDIV4_DIV_CLK_PLL_LOGIC_DIV_SHIFT                           0
#define LMS_CRG_CLKDIV4_DIV_CLK_PLL_LOGIC_DIV_MASK                            0x0000003f
#define LMS_CRG_CLKDIV4_DIV_CLK_PLL_LOGIC_LP32KPLL_DIV_SHIFT                  6
#define LMS_CRG_CLKDIV4_DIV_CLK_PLL_LOGIC_LP32KPLL_DIV_MASK                   0x00000fc0
#define LMS_CRG_CLKDIV4_SC_GT_CLK_PLL_LOGIC_DIV_SHIFT                         12
#define LMS_CRG_CLKDIV4_SC_GT_CLK_PLL_LOGIC_DIV_MASK                          0x00001000
#define LMS_CRG_CLKDIV4_SC_GT_CLK_PLL_LOGIC_LP32KPLL_DIV_SHIFT                13
#define LMS_CRG_CLKDIV4_SC_GT_CLK_PLL_LOGIC_LP32KPLL_DIV_MASK                 0x00002000
#define LMS_CRG_CLKDIV5                      0x0888
#define LMS_CRG_CLKDIV5_DIV_CLK_LMS_TIMESTAMP_SHIFT                           0
#define LMS_CRG_CLKDIV5_DIV_CLK_LMS_TIMESTAMP_MASK                            0x0000003f
#define LMS_CRG_CLKDIV5_DIV_CLK_PAD_WRAP_DB_DIV_SHIFT                         6
#define LMS_CRG_CLKDIV5_DIV_CLK_PAD_WRAP_DB_DIV_MASK                          0x00000fc0
#define LMS_CRG_CLKDIV5_SC_GT_CLK_LMS_TIMESTAMP_SHIFT                         12
#define LMS_CRG_CLKDIV5_SC_GT_CLK_LMS_TIMESTAMP_MASK                          0x00001000
#define LMS_CRG_CLKDIV5_SC_GT_CLK_PAD_WRAP_DB_DIV_SHIFT                       13
#define LMS_CRG_CLKDIV5_SC_GT_CLK_PAD_WRAP_DB_DIV_MASK                        0x00002000
#define LMS_CRG_CLKDIV6                      0x088c
#define LMS_CRG_CLKDIV6_DIV_CLK_LMS_SYSCNT_SHIFT                              6
#define LMS_CRG_CLKDIV6_DIV_CLK_LMS_SYSCNT_MASK                               0x00000fc0
#define LMS_CRG_CLKDIV6_SC_GT_CLK_LMS_SYSCNT_SHIFT                            13
#define LMS_CRG_CLKDIV6_SC_GT_CLK_LMS_SYSCNT_MASK                             0x00002000
#define LMS_CRG_CLKDIV7                      0x0910
#define LMS_CRG_CLKDIV7_SEL_CLKREF_LPPLL_SHIFT                                0
#define LMS_CRG_CLKDIV7_SEL_CLKREF_LPPLL_MASK                                 0x00000001
#define LMS_CRG_CLKDIV7_SEL_PCLK_LMS_SW_SHIFT                                 1
#define LMS_CRG_CLKDIV7_SEL_PCLK_LMS_SW_MASK                                  0x00000006
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER_SHIFT                               3
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER_MASK                                0x00000008
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER01_SHIFT                             5
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER01_MASK                              0x00000020
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER10_SHIFT                             6
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER10_MASK                              0x00000040
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER11_SHIFT                             7
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER11_MASK                              0x00000080
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER20_SHIFT                             8
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER20_MASK                              0x00000100
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER21_SHIFT                             9
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER21_MASK                              0x00000200
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER30_SHIFT                             10
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER30_MASK                              0x00000400
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER31_SHIFT                             11
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER31_MASK                              0x00000800
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER40_SHIFT                             12
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER40_MASK                              0x00001000
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER41_SHIFT                             13
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER41_MASK                              0x00002000
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER50_SHIFT                             14
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER50_MASK                              0x00004000
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER51_SHIFT                             15
#define LMS_CRG_CLKDIV7_SEL_CLK_LMS_TIMER51_MASK                              0x00008000
#define LMS_CRG_CLKDIV8                      0x0914
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_TIMER61_SHIFT                             1
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_TIMER61_MASK                              0x00000002
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_TIMER70_SHIFT                             2
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_TIMER70_MASK                              0x00000004
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_TIMER71_SHIFT                             3
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_TIMER71_MASK                              0x00000008
#define LMS_CRG_CLKDIV8_SEL_CLK_PAD_WRAP_DB_SHIFT                             6
#define LMS_CRG_CLKDIV8_SEL_CLK_PAD_WRAP_DB_MASK                              0x00000040
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_SPMI_SHIFT                                7
#define LMS_CRG_CLKDIV8_SEL_CLK_LMS_SPMI_MASK                                 0x00000080
#define LMS_CRG_CLKDIV8_SEL_CLK_PLL_LOGIC_SHIFT                               8
#define LMS_CRG_CLKDIV8_SEL_CLK_PLL_LOGIC_MASK                                0x00000100
#define LMS_CRG_PERICTRL0                    0x0924
#define LMS_CRG_PERICTRL0_DDR_AUTO_REFRESH_BYPASS_CFG_SHIFT                   0
#define LMS_CRG_PERICTRL0_DDR_AUTO_REFRESH_BYPASS_CFG_MASK                    0x00000001
#define LMS_CRG_PERICTRL0_XRSE_POWER_UP_BYPASS_SHIFT                          2
#define LMS_CRG_PERICTRL0_XRSE_POWER_UP_BYPASS_MASK                           0x00000004
#define LMS_CRG_PERICTRL7                    0x0928
#define LMS_CRG_PERICTRL7_RST_REQ_SOFT_SHIFT                                  0
#define LMS_CRG_PERICTRL7_RST_REQ_SOFT_MASK                                   0x00000001
#define LMS_CRG_PERICTRL8                    0x092c
#define LMS_CRG_PERICTRL8_RST_REGION_CTRL_SHIFT                               0
#define LMS_CRG_PERICTRL8_RST_REGION_CTRL_MASK                                0x00000001
#define LMS_CRG_PERICTRL1                    0x0930
#define LMS_CRG_PERICTRL1_SEL_SCTRL_PCLK_LMS_BYPASS_SHIFT                     0
#define LMS_CRG_PERICTRL1_SEL_SCTRL_PCLK_LMS_BYPASS_MASK                      0x00000001
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_LPPLL_PERI_BYPASS_SHIFT            1
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_LPPLL_PERI_BYPASS_MASK             0x00000002
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_SYS_PERI_BYPASS_SHIFT              2
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_SYS_PERI_BYPASS_MASK               0x00000004
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_LPPLL_LPIS_BYPASS_SHIFT            3
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_LPPLL_LPIS_BYPASS_MASK             0x00000008
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLKIN_REF_LPIS_BYPASS_SHIFT            4
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLKIN_REF_LPIS_BYPASS_MASK             0x00000010
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_SYS_HSS1_BYPASS_SHIFT              5
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_SYS_HSS1_BYPASS_MASK               0x00000020
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_PLL_LOGIC_BYPASS_SHIFT             6
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_CLK_PLL_LOGIC_BYPASS_MASK              0x00000040
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_PCLK_LMS_LPIS_BYPASS_SHIFT             7
#define LMS_CRG_PERICTRL1_AUTOGT_SCTRL_PCLK_LMS_LPIS_BYPASS_MASK              0x00000080
#define LMS_CRG_PERICTRL2                    0x0934
#define LMS_CRG_PERICTRL2_SC_XRSE_MTCMOS_EN_SHIFT                             0
#define LMS_CRG_PERICTRL2_SC_XRSE_MTCMOS_EN_MASK                              0x00000001
#define LMS_CRG_PERICTRL2_SC_XRSE_ISO_EN_SHIFT                                1
#define LMS_CRG_PERICTRL2_SC_XRSE_ISO_EN_MASK                                 0x00000002
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_DR_POFF_SHIFT                           2
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_DR_POFF_MASK                            0x0000000c
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_POFF_SHIFT                              4
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_POFF_MASK                               0x00000030
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_DR_SD_SHIFT                             6
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_DR_SD_MASK                              0x00000040
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_SD_SHIFT                                7
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_SD_MASK                                 0x00000080
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_DR_DS_SHIFT                             8
#define LMS_CRG_PERICTRL2_SC_XRSE_MEM_DR_DS_MASK                              0x00000100
#define LMS_CRG_PERICTRL3                    0x0938
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_SPMI_BYPASS_SHIFT                       0
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_SPMI_BYPASS_MASK                        0x00000001
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_WKUP_MNG_BYPASS_SHIFT                   1
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_WKUP_MNG_BYPASS_MASK                    0x00000002
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIMER0_BYPASS_SHIFT                     2
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIMER0_BYPASS_MASK                      0x00000004
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIMER1_BYPASS_SHIFT                     3
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIMER1_BYPASS_MASK                      0x00000008
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIMER2_BYPASS_SHIFT                     4
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIMER2_BYPASS_MASK                      0x00000010
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_WDT_BYPASS_SHIFT                        5
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_WDT_BYPASS_MASK                         0x00000020
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_RTC0_BYPASS_SHIFT                       6
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_RTC0_BYPASS_MASK                        0x00000040
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_RTC1_BYPASS_SHIFT                       7
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_RTC1_BYPASS_MASK                        0x00000080
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_VAD_BYPASS_SHIFT                        8
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_VAD_BYPASS_MASK                         0x00000100
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_SYSCNT_BYPASS_SHIFT                     9
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_SYSCNT_BYPASS_MASK                      0x00000200
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_PAD_WRAP_BYPASS_SHIFT                   11
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_PAD_WRAP_BYPASS_MASK                    0x00000800
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIME_STAMP_BYPASS_SHIFT                 12
#define LMS_CRG_PERICTRL3_CLKRST_FLAG_TIME_STAMP_BYPASS_MASK                  0x00001000
#define LMS_CRG_PERICTRL4                    0x093c
#define LMS_CRG_PERICTRL4_RST_REQ_WD_LPCTRL_MASK_SHIFT                        0
#define LMS_CRG_PERICTRL4_RST_REQ_WD_LPCTRL_MASK_MASK                         0x00000001
#define LMS_CRG_PERICTRL4_RST_REQ_WD_SENSORHUB_MASK_SHIFT                     1
#define LMS_CRG_PERICTRL4_RST_REQ_WD_SENSORHUB_MASK_MASK                      0x00000002
#define LMS_CRG_PERICTRL4_RST_REQ_WD_XCTRL_CPU_MASK_SHIFT                     2
#define LMS_CRG_PERICTRL4_RST_REQ_WD_XCTRL_CPU_MASK_MASK                      0x00000004
#define LMS_CRG_PERICTRL4_RST_REQ_WD_XCTRL_DDR_MASK_SHIFT                     3
#define LMS_CRG_PERICTRL4_RST_REQ_WD_XCTRL_DDR_MASK_MASK                      0x00000008
#define LMS_CRG_PERICTRL4_RST_REQ_WD_XRSE_MASK_SHIFT                          4
#define LMS_CRG_PERICTRL4_RST_REQ_WD_XRSE_MASK_MASK                           0x00000010
#define LMS_CRG_PERICTRL4_RST_REQ_WD_LMS_MASK_SHIFT                           5
#define LMS_CRG_PERICTRL4_RST_REQ_WD_LMS_MASK_MASK                            0x00000020
#define LMS_CRG_PERICTRL4_RST_REQ_WD0_PERI_MASK_SHIFT                         6
#define LMS_CRG_PERICTRL4_RST_REQ_WD0_PERI_MASK_MASK                          0x00000040
#define LMS_CRG_PERICTRL4_RST_REQ_WD1_PERI_MASK_SHIFT                         7
#define LMS_CRG_PERICTRL4_RST_REQ_WD1_PERI_MASK_MASK                          0x00000080
#define LMS_CRG_PERICTRL4_RST_REQ_WD2_PERI_MASK_SHIFT                         8
#define LMS_CRG_PERICTRL4_RST_REQ_WD2_PERI_MASK_MASK                          0x00000100
#define LMS_CRG_PERICTRL4_RST_REQ_WD3_PERI_MASK_SHIFT                         9
#define LMS_CRG_PERICTRL4_RST_REQ_WD3_PERI_MASK_MASK                          0x00000200
#define LMS_CRG_PERICTRL4_RST_REQ_WD_AUDIO_MASK_SHIFT                         10
#define LMS_CRG_PERICTRL4_RST_REQ_WD_AUDIO_MASK_MASK                          0x00000400
#define LMS_CRG_PERICTRL4_RST_REQ_WD_ISP_MASK_SHIFT                           11
#define LMS_CRG_PERICTRL4_RST_REQ_WD_ISP_MASK_MASK                            0x00000800
#define LMS_CRG_PERICTRL4_RST_REQ_WD_NPU_M85_MASK_SHIFT                       12
#define LMS_CRG_PERICTRL4_RST_REQ_WD_NPU_M85_MASK_MASK                        0x00001000
#define LMS_CRG_PERICTRL4_RST_REQ_WD_NPU_DSP_MASK_SHIFT                       13
#define LMS_CRG_PERICTRL4_RST_REQ_WD_NPU_DSP_MASK_MASK                        0x00002000
#define LMS_CRG_PERICTRL4_RST_REQ_TSENSOR_MASK_SHIFT                          14
#define LMS_CRG_PERICTRL4_RST_REQ_TSENSOR_MASK_MASK                           0x00004000
#define LMS_CRG_PERISTAT3                    0x0940
#define LMS_CRG_PERISTAT3_XRSE_MTCMOS_ACK_SHIFT                               0
#define LMS_CRG_PERISTAT3_XRSE_MTCMOS_ACK_MASK                                0x00000001
#define LMS_CRG_PERISTAT3_XRSE_MEM_DR_SD_ACK_SHIFT                            1
#define LMS_CRG_PERISTAT3_XRSE_MEM_DR_SD_ACK_MASK                             0x00000002
#define LMS_CRG_PERISTAT3_XRSE_MEM_SD_ACK_SHIFT                               2
#define LMS_CRG_PERISTAT3_XRSE_MEM_SD_ACK_MASK                                0x00000004
#define LMS_CRG_PERISTAT3_XRSE_MTCMOS_ACK_TIMEOUT_SHIFT                       3
#define LMS_CRG_PERISTAT3_XRSE_MTCMOS_ACK_TIMEOUT_MASK                        0x00000008
#define LMS_CRG_PERISTAT3_EFUSE_DONE_TIMEOUT_SHIFT                            4
#define LMS_CRG_PERISTAT3_EFUSE_DONE_TIMEOUT_MASK                             0x00000010
#define LMS_CRG_PERISTAT3_MEMREPAIR_DONE_TIMEOUT_SHIFT                        5
#define LMS_CRG_PERISTAT3_MEMREPAIR_DONE_TIMEOUT_MASK                         0x00000020
#define LMS_CRG_PERISTAT3_XRSE_MEM_DR_DS_ACK_SHIFT                            6
#define LMS_CRG_PERISTAT3_XRSE_MEM_DR_DS_ACK_MASK                             0x00000040
#define LMS_CRG_PERICTRL5                    0x0950
#define LMS_CRG_PERICTRL5_RST_REQ_RECODER_CLR_SHIFT                           0
#define LMS_CRG_PERICTRL5_RST_REQ_RECODER_CLR_MASK                            0x00000001
#define LMS_CRG_PERISTAT4                    0x0954
#define LMS_CRG_CLKDIV9                      0x0960
#define LMS_CRG_CLKDIV9_DIV_CLK_RTC_NUM_SHIFT                                 0
#define LMS_CRG_CLKDIV9_DIV_CLK_RTC_NUM_MASK                                  0x0000ffff
#define LMS_CRG_CLKDIV9_DIV_CLK_RTC_DEN_SHIFT                                 16
#define LMS_CRG_CLKDIV9_DIV_CLK_RTC_DEN_MASK                                  0xffff0000
#define LMS_CRG_LP32KPLL_PRESS_TEST          0x0a00
#define LMS_CRG_LP32KPLL_PRESS_TEST_LP32KPLL_PRESS_TEST_EN_SHIFT              0
#define LMS_CRG_LP32KPLL_PRESS_TEST_LP32KPLL_PRESS_TEST_EN_MASK               0x00000001
#define LMS_CRG_LP32KPLL_PRESS_TEST_LP32KPLL_PRESS_TEST_CNT_SHIFT             1
#define LMS_CRG_LP32KPLL_PRESS_TEST_LP32KPLL_PRESS_TEST_CNT_MASK              0x01fffffe
#define LMS_CRG_LP32KPLL_PRESS_TEST_LP32KPLL_CFG_RSV0_SHIFT                   25
#define LMS_CRG_LP32KPLL_PRESS_TEST_LP32KPLL_CFG_RSV0_MASK                    0xfe000000
#define LMS_CRG_LP32KPLL_ERR_BOOT_CNT        0x0a04
#define LMS_CRG_LP32KPLL_ERR_BOOT_CNT_LP32KPLL_ERR_BOOT_CNT_SHIFT             0
#define LMS_CRG_LP32KPLL_ERR_BOOT_CNT_LP32KPLL_ERR_BOOT_CNT_MASK              0x00ffffff
#define LMS_CRG_LP32KPLL_ERR_BOOT_CNT_LP32KPLL_PRESS_TEST_END_SHIFT           24
#define LMS_CRG_LP32KPLL_ERR_BOOT_CNT_LP32KPLL_PRESS_TEST_END_MASK            0x01000000
#define LMS_CRG_LP32KPLL_LOCKTIMEOUT         0x0a08
#define LMS_CRG_LP32KPLL_LOCKTIMEOUT_LP32KPLL_LOCK_TIMEOUT_SHIFT              0
#define LMS_CRG_LP32KPLL_LOCKTIMEOUT_LP32KPLL_LOCK_TIMEOUT_MASK               0x00001fff
#define LMS_CRG_LP32KPLL_LOCKTIMEOUT_LP32KPLL_CFG_RSV2_SHIFT                  13
#define LMS_CRG_LP32KPLL_LOCKTIMEOUT_LP32KPLL_CFG_RSV2_MASK                   0xffffe000
#define LMS_CRG_LP32KPLL_CTRL                0x0a0c
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_EN_SHIFT                               0
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_EN_MASK                                0x00000001
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_BYPASS_SHIFT                           1
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_BYPASS_MASK                            0x00000002
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_FBDIV_SHIFT                            2
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_FBDIV_MASK                             0x0000fffc
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_POSTDIV_SHIFT                          16
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_POSTDIV_MASK                           0x000f0000
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_CFG_VALID_SHIFT                        20
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_CFG_VALID_MASK                         0x00100000
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_GT_SHIFT                               21
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_GT_MASK                                0x00200000
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_CFG_RSV3_SHIFT                         22
#define LMS_CRG_LP32KPLL_CTRL_LP32KPLL_CFG_RSV3_MASK                          0xffc00000
#define LMS_CRG_LP32KPLL_DEBUG               0x0a10
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_LOCKCOUNT_SHIFT                       0
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_LOCKCOUNT_MASK                        0x00000003
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_SPARE_SHIFT                           2
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_SPARE_MASK                            0x00000ffc
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_FCALBYP_SHIFT                         12
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_FCALBYP_MASK                          0x00001000
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_FCALTEST_SHIFT                        13
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_FCALTEST_MASK                         0x00002000
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_FCALIN_SHIFT                          14
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_FCALIN_MASK                           0x003fc000
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_CFG_RSV4_SHIFT                        22
#define LMS_CRG_LP32KPLL_DEBUG_LP32KPLL_CFG_RSV4_MASK                         0xffc00000
#define LMS_CRG_LP32KPLL_DEBUG_STATE         0x0a14
#define LMS_CRG_LP32KPLL_DEBUG_STATE_LP32KPLL_EN_STATE_SHIFT                  0
#define LMS_CRG_LP32KPLL_DEBUG_STATE_LP32KPLL_EN_STATE_MASK                   0x00000001
#define LMS_CRG_LP32KPLL_DEBUG_STATE_LP32KPLL_FCALOUT_SHIFT                   1
#define LMS_CRG_LP32KPLL_DEBUG_STATE_LP32KPLL_FCALOUT_MASK                    0x000001fe
#define LMS_CRG_LP32KPLL_DEBUG_STATE_ST_CLK_32KPLL_SHIFT                      9
#define LMS_CRG_LP32KPLL_DEBUG_STATE_ST_CLK_32KPLL_MASK                       0x00000200
#define LMS_CRG_LPPLL_VOTE_CFG               0x0a18
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_START_FSM_BYPASS_SHIFT                   0
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_START_FSM_BYPASS_MASK                    0x00000001
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_LOCK_TIMEOUT_SHIFT                       1
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_LOCK_TIMEOUT_MASK                        0x000000fe
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_RETRY_NUM_SHIFT                          8
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_RETRY_NUM_MASK                           0x00000700
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_FSM_EN_SHIFT                             11
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_FSM_EN_MASK                              0x00000800
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_CFG_RSV0_SHIFT                           12
#define LMS_CRG_LPPLL_VOTE_CFG_LPPLL_CFG_RSV0_MASK                            0xfffff000
#define LMS_CRG_LPPLL_PRESS_TEST             0x0a1c
#define LMS_CRG_LPPLL_PRESS_TEST_LPPLL_PRESS_TEST_EN_SHIFT                    0
#define LMS_CRG_LPPLL_PRESS_TEST_LPPLL_PRESS_TEST_EN_MASK                     0x00000001
#define LMS_CRG_LPPLL_PRESS_TEST_LPPLL_PRESS_TEST_CNT_SHIFT                   1
#define LMS_CRG_LPPLL_PRESS_TEST_LPPLL_PRESS_TEST_CNT_MASK                    0x01fffffe
#define LMS_CRG_LPPLL_PRESS_TEST_LPPLL_CFG_RSV1_SHIFT                         25
#define LMS_CRG_LPPLL_PRESS_TEST_LPPLL_CFG_RSV1_MASK                          0xfe000000
#define LMS_CRG_LPPLL_ERR_BOOT_CNT           0x0a20
#define LMS_CRG_LPPLL_ERR_BOOT_CNT_LPPLL_ERR_BOOT_CNT_SHIFT                   0
#define LMS_CRG_LPPLL_ERR_BOOT_CNT_LPPLL_ERR_BOOT_CNT_MASK                    0x00ffffff
#define LMS_CRG_LPPLL_ERR_BOOT_CNT_LPPLL_PRESS_TEST_END_SHIFT                 24
#define LMS_CRG_LPPLL_ERR_BOOT_CNT_LPPLL_PRESS_TEST_END_MASK                  0x01000000
#define LMS_CRG_LPPLL_VOTE_MASK              0x0a24
#define LMS_CRG_LPPLL_VOTE_MASK_LPPLL_HW_VOTE_MASK_SHIFT                      0
#define LMS_CRG_LPPLL_VOTE_MASK_LPPLL_HW_VOTE_MASK_MASK                       0x000000ff
#define LMS_CRG_LPPLL_VOTE_MASK_LPPLL_SFT_VOTE_MASK_SHIFT                     8
#define LMS_CRG_LPPLL_VOTE_MASK_LPPLL_SFT_VOTE_MASK_MASK                      0x0000ff00
#define LMS_CRG_LPPLL_VOTE_MASK_LPPLL_CFG_RSV2_SHIFT                          16
#define LMS_CRG_LPPLL_VOTE_MASK_LPPLL_CFG_RSV2_MASK                           0xffff0000
#define LMS_CRG_LPPLL_VOTE_BYPASS            0x0a2c
#define LMS_CRG_LPPLL_VOTE_BYPASS_LPPLL_VOTE_BYPASS_SHIFT                     0
#define LMS_CRG_LPPLL_VOTE_BYPASS_LPPLL_VOTE_BYPASS_MASK                      0x000000ff
#define LMS_CRG_LPPLL_VOTE_GT                0x0a30
#define LMS_CRG_LPPLL_VOTE_GT_LPPLL_VOTE_GT_SHIFT                             0
#define LMS_CRG_LPPLL_VOTE_GT_LPPLL_VOTE_GT_MASK                              0x000000ff
#define LMS_CRG_LPPLL_CTRL0                  0x0a34
#define LMS_CRG_LPPLL_CTRL0_LPPLL_EN_SHIFT                                    0
#define LMS_CRG_LPPLL_CTRL0_LPPLL_EN_MASK                                     0x00000001
#define LMS_CRG_LPPLL_CTRL0_LPPLL_BYPASS_SHIFT                                1
#define LMS_CRG_LPPLL_CTRL0_LPPLL_BYPASS_MASK                                 0x00000002
#define LMS_CRG_LPPLL_CTRL0_LPPLL_REFDIV_SHIFT                                2
#define LMS_CRG_LPPLL_CTRL0_LPPLL_REFDIV_MASK                                 0x000000fc
#define LMS_CRG_LPPLL_CTRL0_LPPLL_FBDIV_SHIFT                                 8
#define LMS_CRG_LPPLL_CTRL0_LPPLL_FBDIV_MASK                                  0x0003ff00
#define LMS_CRG_LPPLL_CTRL0_LPPLL_POSTDIV_SHIFT                               18
#define LMS_CRG_LPPLL_CTRL0_LPPLL_POSTDIV_MASK                                0x001c0000
#define LMS_CRG_LPPLL_CTRL0_LPPLL_CFG_VALID_SHIFT                             21
#define LMS_CRG_LPPLL_CTRL0_LPPLL_CFG_VALID_MASK                              0x00200000
#define LMS_CRG_LPPLL_CTRL0_LPPLL_GT_SHIFT                                    22
#define LMS_CRG_LPPLL_CTRL0_LPPLL_GT_MASK                                     0x00400000
#define LMS_CRG_LPPLL_CTRL0_LPPLL_CFG_RSV3_SHIFT                              23
#define LMS_CRG_LPPLL_CTRL0_LPPLL_CFG_RSV3_MASK                               0xff800000
#define LMS_CRG_LPPLL_CTRL1                  0x0a38
#define LMS_CRG_LPPLL_CTRL1_LPPLL_FRAC_SHIFT                                  0
#define LMS_CRG_LPPLL_CTRL1_LPPLL_FRAC_MASK                                   0x00ffffff
#define LMS_CRG_LPPLL_CTRL1_LPPLL_DSM_EN_SHIFT                                24
#define LMS_CRG_LPPLL_CTRL1_LPPLL_DSM_EN_MASK                                 0x01000000
#define LMS_CRG_LPPLL_CTRL1_LPPLL_CFG_RSV4_SHIFT                              25
#define LMS_CRG_LPPLL_CTRL1_LPPLL_CFG_RSV4_MASK                               0xfe000000
#define LMS_CRG_LPPLL_DEBUG                  0x0a3c
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALBYP_SHIFT                          0
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALBYP_MASK                           0x00000001
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALCNT_SHIFT                          1
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALCNT_MASK                           0x0000000e
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALEN_SHIFT                           4
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALEN_MASK                            0x00000010
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALIN_SHIFT                           5
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETCALIN_MASK                            0x0001ffe0
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETFASTCAL_SHIFT                         17
#define LMS_CRG_LPPLL_DEBUG_LPPLL_OFFSETFASTCAL_MASK                          0x00020000
#define LMS_CRG_LPPLL_DEBUG_LPPLL_CFG_RSV5_SHIFT                              18
#define LMS_CRG_LPPLL_DEBUG_LPPLL_CFG_RSV5_MASK                               0xfffc0000
#define LMS_CRG_LPPLL_DEBUG_STATE            0x0a40
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_EN_STATE_SHIFT                        0
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_EN_STATE_MASK                         0x00000001
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_OFFSETCALOUT_SHIFT                    1
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_OFFSETCALOUT_MASK                     0x00001ffe
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_OFFSETCALLOCK_SHIFT                   13
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_OFFSETCALLOCK_MASK                    0x00002000
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_START_FSM_STATE_SHIFT                 14
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_START_FSM_STATE_MASK                  0x0001c000
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_REAL_RETRY_CNT_SHIFT                  17
#define LMS_CRG_LPPLL_DEBUG_STATE_LPPLL_REAL_RETRY_CNT_MASK                   0x000e0000
#define LMS_CRG_LPPLL_DEBUG_STATE_ST_CLK_LPPLL_SHIFT                          20
#define LMS_CRG_LPPLL_DEBUG_STATE_ST_CLK_LPPLL_MASK                           0x00100000
#define LMS_CRG_REGFILE_CGBYPASS             0x0a44
#define LMS_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                       0
#define LMS_CRG_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                        0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_lms_rtc1    :  1;
        unsigned int gt_pclk_lms_rtc1   :  1;
        unsigned int gt_clk_vad         :  1;
        unsigned int gt_clk_vad_pdm     :  1;
        unsigned int gt_clk_vad_i2s     :  1;
        unsigned int gt_pclk_lms_timer2 :  1;
        unsigned int gt_clk_lms_timer00 :  1;
        unsigned int gt_clk_lms_timer60 :  1;
        unsigned int gt_clk_lms_timer80 :  1;
        unsigned int gt_clk_lms_timer81 :  1;
        unsigned int gt_clk_codec       :  1;
        unsigned int reserved_0         : 21;
    } reg;
}lms_crg_clkgt0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_lms_rtc1    :  1;
        unsigned int gt_pclk_lms_rtc1   :  1;
        unsigned int gt_clk_vad         :  1;
        unsigned int gt_clk_vad_pdm     :  1;
        unsigned int gt_clk_vad_i2s     :  1;
        unsigned int gt_pclk_lms_timer2 :  1;
        unsigned int gt_clk_lms_timer00 :  1;
        unsigned int gt_clk_lms_timer60 :  1;
        unsigned int gt_clk_lms_timer80 :  1;
        unsigned int gt_clk_lms_timer81 :  1;
        unsigned int gt_clk_codec       :  1;
        unsigned int reserved_0         : 21;
    } reg;
}lms_crg_clkgt0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_lms_rtc1    :  1;
        unsigned int gt_pclk_lms_rtc1   :  1;
        unsigned int gt_clk_vad         :  1;
        unsigned int gt_clk_vad_pdm     :  1;
        unsigned int gt_clk_vad_i2s     :  1;
        unsigned int gt_pclk_lms_timer2 :  1;
        unsigned int gt_clk_lms_timer00 :  1;
        unsigned int gt_clk_lms_timer60 :  1;
        unsigned int gt_clk_lms_timer80 :  1;
        unsigned int gt_clk_lms_timer81 :  1;
        unsigned int gt_clk_codec       :  1;
        unsigned int reserved_0         : 21;
    } reg;
}lms_crg_clkgt0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_lms_rtc1    :  1;
        unsigned int st_pclk_lms_rtc1   :  1;
        unsigned int st_clk_vad         :  1;
        unsigned int st_clk_vad_pdm     :  1;
        unsigned int st_clk_vad_i2s     :  1;
        unsigned int st_pclk_lms_timer2 :  1;
        unsigned int st_clk_lms_timer00 :  1;
        unsigned int st_clk_lms_timer60 :  1;
        unsigned int st_clk_lms_timer80 :  1;
        unsigned int st_clk_lms_timer81 :  1;
        unsigned int st_clk_codec       :  1;
        unsigned int reserved_0         : 21;
    } reg;
}lms_crg_clkst0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lms_rtc1_n    :  1;
        unsigned int ip_prst_lms_rtc1_n   :  1;
        unsigned int ip_rst_vad_n         :  1;
        unsigned int ip_rst_vad_pdm_n     :  1;
        unsigned int ip_rst_vad_i2s_n     :  1;
        unsigned int ip_prst_lms_timer2_n :  1;
        unsigned int ip_rst_lms_timer00_n :  1;
        unsigned int ip_rst_lms_timer60_n :  1;
        unsigned int ip_rst_lms_timer80_n :  1;
        unsigned int ip_rst_lms_timer81_n :  1;
        unsigned int reserved_0           : 22;
    } reg;
}lms_crg_rst0_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lms_rtc1_n    :  1;
        unsigned int ip_prst_lms_rtc1_n   :  1;
        unsigned int ip_rst_vad_n         :  1;
        unsigned int ip_rst_vad_pdm_n     :  1;
        unsigned int ip_rst_vad_i2s_n     :  1;
        unsigned int ip_prst_lms_timer2_n :  1;
        unsigned int ip_rst_lms_timer00_n :  1;
        unsigned int ip_rst_lms_timer60_n :  1;
        unsigned int ip_rst_lms_timer80_n :  1;
        unsigned int ip_rst_lms_timer81_n :  1;
        unsigned int reserved_0           : 22;
    } reg;
}lms_crg_rst0_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lms_rtc1_n    :  1;
        unsigned int ip_prst_lms_rtc1_n   :  1;
        unsigned int ip_rst_vad_n         :  1;
        unsigned int ip_rst_vad_pdm_n     :  1;
        unsigned int ip_rst_vad_i2s_n     :  1;
        unsigned int ip_prst_lms_timer2_n :  1;
        unsigned int ip_rst_lms_timer00_n :  1;
        unsigned int ip_rst_lms_timer60_n :  1;
        unsigned int ip_rst_lms_timer80_n :  1;
        unsigned int ip_rst_lms_timer81_n :  1;
        unsigned int reserved_0           : 22;
    } reg;
}lms_crg_rst0_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_vad       :  6;
        unsigned int div_clk_vad_pdm   :  6;
        unsigned int sc_gt_clk_vad     :  1;
        unsigned int sc_gt_clk_vad_pdm :  1;
        unsigned int reserved_0        :  2;
        unsigned int _bm_              : 16;
    } reg;
}lms_crg_clkdiv0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_vad_i2s_pre :  6;
        unsigned int div_clk_vad_i2s     :  2;
        unsigned int div_clk_codec       :  6;
        unsigned int sc_gt_clk_vad_i2s   :  1;
        unsigned int sc_gt_clk_codec     :  1;
        unsigned int _bm_                : 16;
    } reg;
}lms_crg_clkdiv1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_vad         :  1;
        unsigned int div_done_clk_vad_pdm     :  1;
        unsigned int div_done_clk_pad_wrap_db :  1;
        unsigned int div_done_clk_codec       :  1;
        unsigned int div_done_clk_vad_i2s_pre :  1;
        unsigned int div_done_clk_vad_i2s     :  1;
        unsigned int reserved_0               : 26;
    } reg;
}lms_crg_peristat0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_done_clk_lms_spmi               :  1;
        unsigned int div_done_clk_lms_timer_h            :  1;
        unsigned int reserved_0                          :  3;
        unsigned int div_done_clk_lms_timestamp          :  1;
        unsigned int reserved_1                          :  1;
        unsigned int div_done_clk_lms_syscnt             :  1;
        unsigned int div_done_clk_pll_logic              :  1;
        unsigned int div_done_clk_pll_logic_lp32kpll_div :  1;
        unsigned int reserved_2                          : 22;
    } reg;
}lms_crg_peristat1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_ack_pclk_lms_sw     :  3;
        unsigned int sw_ack_clk_lms_spmi_sw :  2;
        unsigned int sw_ack_clk_pll_logic   :  2;
        unsigned int reserved_0             : 25;
    } reg;
}lms_crg_peristat2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_vad           :  2;
        unsigned int sel_clk_lms_timer00   :  1;
        unsigned int sel_clk_lms_timer60   :  1;
        unsigned int sel_clk_lms_timer80   :  1;
        unsigned int sel_clk_lms_timer81   :  1;
        unsigned int sel_clk_codec         :  2;
        unsigned int sel_ufs_rst_iso_value :  1;
        unsigned int reserved_0            :  7;
        unsigned int _bm_                  : 16;
    } reg;
}lms_crg_clkdiv2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_lock :  1;
        unsigned int lppll_lock    :  1;
        unsigned int reserved_0    : 30;
    } reg;
}lms_crg_pll_lock_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_lppll  :  1;
        unsigned int intr_clear_pll_unlock_lppll   :  1;
        unsigned int intr_clear_vote_req_err_lppll :  1;
        unsigned int reserved_0                    : 29;
    } reg;
}lms_crg_intr_clear_lppll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_lppll  :  1;
        unsigned int intr_mask_pll_unlock_lppll   :  1;
        unsigned int intr_mask_vote_req_err_lppll :  1;
        unsigned int reserved_0                   : 29;
    } reg;
}lms_crg_intr_mask_lppll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_lppll    :  1;
        unsigned int intr_pll_unlock_status_lppll     :  1;
        unsigned int intr_vote_req_err_status_lppll   :  1;
        unsigned int intr_pll_ini_err_status_lp32kpll :  1;
        unsigned int intr_pll_unlock_status_lp32kpll  :  1;
        unsigned int reserved_0                       : 27;
    } reg;
}lms_crg_intr_status_pll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_pll_ini_err_status_msk_lppll    :  1;
        unsigned int intr_pll_unlock_status_msk_lppll     :  1;
        unsigned int intr_vote_req_err_status_msk_lppll   :  1;
        unsigned int intr_pll_ini_err_status_msk_lp32kpll :  1;
        unsigned int intr_pll_unlock_status_msk_lp32kpll  :  1;
        unsigned int reserved_0                           : 27;
    } reg;
}lms_crg_intr_status_pll_msk_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_clear_pll_ini_err_lp32kpll :  1;
        unsigned int intr_clear_pll_unlock_lp32kpll  :  1;
        unsigned int reserved_0                      : 30;
    } reg;
}lms_crg_intr_clear_lp32kpll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_mask_pll_ini_err_lp32kpll :  1;
        unsigned int intr_mask_pll_unlock_lp32kpll  :  1;
        unsigned int reserved_0                     : 30;
    } reg;
}lms_crg_intr_mask_lp32kpll_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_lppll_test   :  6;
        unsigned int reserved_0           :  6;
        unsigned int sc_gt_clk_lppll_test :  1;
        unsigned int reserved_1           :  3;
        unsigned int _bm_                 : 16;
    } reg;
}lms_crg_clkdiv10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clk_lms_pll_test :  1;
        unsigned int reserved_0           : 15;
        unsigned int _bm_                 : 16;
    } reg;
}lms_crg_perictrl6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_vote_en :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}lms_crg_lppll_vote_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_lms_spmi       :  1;
        unsigned int gt_pclk_pad_wrap      :  1;
        unsigned int gt_clk_pad_wrap_db    :  1;
        unsigned int reserved_0            :  3;
        unsigned int gt_pclk_lms_syscnt    :  1;
        unsigned int reserved_1            :  1;
        unsigned int gt_pclk_lms_rtc0      :  1;
        unsigned int reserved_2            :  2;
        unsigned int gt_pclk_lms_timer1    :  1;
        unsigned int gt_pclk_lms_timer0    :  1;
        unsigned int reserved_3            :  1;
        unsigned int gt_pclk_lms_wkupmng   :  1;
        unsigned int gt_pclk_lms_spmi      :  1;
        unsigned int reserved_4            :  2;
        unsigned int gt_clk_lms_rtc0       :  1;
        unsigned int reserved_5            :  2;
        unsigned int gt_clkin_ref_lpis     :  1;
        unsigned int gt_clk_lppll_peri     :  1;
        unsigned int gt_clk_lppll_lpis     :  1;
        unsigned int gt_clk_sys_peri       :  1;
        unsigned int gt_clk_sys_lpis       :  1;
        unsigned int gt_clk_lms_syscnt     :  1;
        unsigned int reserved_6            :  1;
        unsigned int gt_clk_lms_timestamp  :  1;
        unsigned int gt_pclk_lms_timestamp :  1;
        unsigned int reserved_7            :  2;
    } reg;
}lms_crg_clkgt1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_lms_spmi       :  1;
        unsigned int gt_pclk_pad_wrap      :  1;
        unsigned int gt_clk_pad_wrap_db    :  1;
        unsigned int reserved_0            :  3;
        unsigned int gt_pclk_lms_syscnt    :  1;
        unsigned int reserved_1            :  1;
        unsigned int gt_pclk_lms_rtc0      :  1;
        unsigned int reserved_2            :  2;
        unsigned int gt_pclk_lms_timer1    :  1;
        unsigned int gt_pclk_lms_timer0    :  1;
        unsigned int reserved_3            :  1;
        unsigned int gt_pclk_lms_wkupmng   :  1;
        unsigned int gt_pclk_lms_spmi      :  1;
        unsigned int reserved_4            :  2;
        unsigned int gt_clk_lms_rtc0       :  1;
        unsigned int reserved_5            :  2;
        unsigned int gt_clkin_ref_lpis     :  1;
        unsigned int gt_clk_lppll_peri     :  1;
        unsigned int gt_clk_lppll_lpis     :  1;
        unsigned int gt_clk_sys_peri       :  1;
        unsigned int gt_clk_sys_lpis       :  1;
        unsigned int gt_clk_lms_syscnt     :  1;
        unsigned int reserved_6            :  1;
        unsigned int gt_clk_lms_timestamp  :  1;
        unsigned int gt_pclk_lms_timestamp :  1;
        unsigned int reserved_7            :  2;
    } reg;
}lms_crg_clkgt1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int gt_clk_lms_spmi       :  1;
        unsigned int gt_pclk_pad_wrap      :  1;
        unsigned int gt_clk_pad_wrap_db    :  1;
        unsigned int reserved_0            :  3;
        unsigned int gt_pclk_lms_syscnt    :  1;
        unsigned int reserved_1            :  1;
        unsigned int gt_pclk_lms_rtc0      :  1;
        unsigned int reserved_2            :  2;
        unsigned int gt_pclk_lms_timer1    :  1;
        unsigned int gt_pclk_lms_timer0    :  1;
        unsigned int reserved_3            :  1;
        unsigned int gt_pclk_lms_wkupmng   :  1;
        unsigned int gt_pclk_lms_spmi      :  1;
        unsigned int reserved_4            :  2;
        unsigned int gt_clk_lms_rtc0       :  1;
        unsigned int reserved_5            :  2;
        unsigned int gt_clkin_ref_lpis     :  1;
        unsigned int gt_clk_lppll_peri     :  1;
        unsigned int gt_clk_lppll_lpis     :  1;
        unsigned int gt_clk_sys_peri       :  1;
        unsigned int gt_clk_sys_lpis       :  1;
        unsigned int gt_clk_lms_syscnt     :  1;
        unsigned int reserved_6            :  1;
        unsigned int gt_clk_lms_timestamp  :  1;
        unsigned int gt_pclk_lms_timestamp :  1;
        unsigned int reserved_7            :  2;
    } reg;
}lms_crg_clkgt1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_clk_lms_spmi       :  1;
        unsigned int st_pclk_pad_wrap      :  1;
        unsigned int st_clk_pad_wrap_db    :  1;
        unsigned int reserved_0            :  3;
        unsigned int st_pclk_lms_syscnt    :  1;
        unsigned int reserved_1            :  1;
        unsigned int st_pclk_lms_rtc0      :  1;
        unsigned int reserved_2            :  2;
        unsigned int st_pclk_lms_timer1    :  1;
        unsigned int st_pclk_lms_timer0    :  1;
        unsigned int reserved_3            :  1;
        unsigned int st_pclk_lms_wkupmng   :  1;
        unsigned int st_pclk_lms_spmi      :  1;
        unsigned int reserved_4            :  2;
        unsigned int st_clk_lms_rtc0       :  1;
        unsigned int reserved_5            :  2;
        unsigned int st_clkin_ref_lpis     :  1;
        unsigned int st_clk_lppll_peri     :  1;
        unsigned int st_clk_lppll_lpis     :  1;
        unsigned int st_clk_sys_peri       :  1;
        unsigned int st_clk_sys_lpis       :  1;
        unsigned int st_clk_lms_syscnt     :  1;
        unsigned int reserved_6            :  1;
        unsigned int st_clk_lms_timestamp  :  1;
        unsigned int st_pclk_lms_timestamp :  1;
        unsigned int reserved_7            :  2;
    } reg;
}lms_crg_clkst1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0            :  1;
        unsigned int gt_clk_lms_timer01    :  1;
        unsigned int gt_clk_lms_timer10    :  1;
        unsigned int gt_clk_lms_timer11    :  1;
        unsigned int gt_clk_lms_timer20    :  1;
        unsigned int gt_clk_lms_timer21    :  1;
        unsigned int gt_clk_lms_timer30    :  1;
        unsigned int gt_clk_lms_timer31    :  1;
        unsigned int gt_clk_lms_timer40    :  1;
        unsigned int gt_clk_lms_timer41    :  1;
        unsigned int gt_clk_lms_timer50    :  1;
        unsigned int gt_clk_lms_timer51    :  1;
        unsigned int reserved_1            :  1;
        unsigned int gt_clk_lms_timer61    :  1;
        unsigned int gt_clk_lms_timer70    :  1;
        unsigned int gt_clk_lms_timer71    :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_lppll_logic    :  1;
        unsigned int gt_clk_lp32kpll_logic :  1;
        unsigned int gt_clk_sys_hss1       :  1;
        unsigned int gt_clk_lms_wdt        :  1;
        unsigned int gt_pclk_lms_wdt       :  1;
        unsigned int gt_clk_pll_logic      :  1;
        unsigned int reserved_3            :  9;
    } reg;
}lms_crg_clkgt2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0            :  1;
        unsigned int gt_clk_lms_timer01    :  1;
        unsigned int gt_clk_lms_timer10    :  1;
        unsigned int gt_clk_lms_timer11    :  1;
        unsigned int gt_clk_lms_timer20    :  1;
        unsigned int gt_clk_lms_timer21    :  1;
        unsigned int gt_clk_lms_timer30    :  1;
        unsigned int gt_clk_lms_timer31    :  1;
        unsigned int gt_clk_lms_timer40    :  1;
        unsigned int gt_clk_lms_timer41    :  1;
        unsigned int gt_clk_lms_timer50    :  1;
        unsigned int gt_clk_lms_timer51    :  1;
        unsigned int reserved_1            :  1;
        unsigned int gt_clk_lms_timer61    :  1;
        unsigned int gt_clk_lms_timer70    :  1;
        unsigned int gt_clk_lms_timer71    :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_lppll_logic    :  1;
        unsigned int gt_clk_lp32kpll_logic :  1;
        unsigned int gt_clk_sys_hss1       :  1;
        unsigned int gt_clk_lms_wdt        :  1;
        unsigned int gt_pclk_lms_wdt       :  1;
        unsigned int gt_clk_pll_logic      :  1;
        unsigned int reserved_3            :  9;
    } reg;
}lms_crg_clkgt2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0            :  1;
        unsigned int gt_clk_lms_timer01    :  1;
        unsigned int gt_clk_lms_timer10    :  1;
        unsigned int gt_clk_lms_timer11    :  1;
        unsigned int gt_clk_lms_timer20    :  1;
        unsigned int gt_clk_lms_timer21    :  1;
        unsigned int gt_clk_lms_timer30    :  1;
        unsigned int gt_clk_lms_timer31    :  1;
        unsigned int gt_clk_lms_timer40    :  1;
        unsigned int gt_clk_lms_timer41    :  1;
        unsigned int gt_clk_lms_timer50    :  1;
        unsigned int gt_clk_lms_timer51    :  1;
        unsigned int reserved_1            :  1;
        unsigned int gt_clk_lms_timer61    :  1;
        unsigned int gt_clk_lms_timer70    :  1;
        unsigned int gt_clk_lms_timer71    :  1;
        unsigned int reserved_2            :  1;
        unsigned int gt_clk_lppll_logic    :  1;
        unsigned int gt_clk_lp32kpll_logic :  1;
        unsigned int gt_clk_sys_hss1       :  1;
        unsigned int gt_clk_lms_wdt        :  1;
        unsigned int gt_pclk_lms_wdt       :  1;
        unsigned int gt_clk_pll_logic      :  1;
        unsigned int reserved_3            :  9;
    } reg;
}lms_crg_clkgt2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0            :  1;
        unsigned int st_clk_lms_timer01    :  1;
        unsigned int st_clk_lms_timer10    :  1;
        unsigned int st_clk_lms_timer11    :  1;
        unsigned int st_clk_lms_timer20    :  1;
        unsigned int st_clk_lms_timer21    :  1;
        unsigned int st_clk_lms_timer30    :  1;
        unsigned int st_clk_lms_timer31    :  1;
        unsigned int st_clk_lms_timer40    :  1;
        unsigned int st_clk_lms_timer41    :  1;
        unsigned int st_clk_lms_timer50    :  1;
        unsigned int st_clk_lms_timer51    :  1;
        unsigned int reserved_1            :  1;
        unsigned int st_clk_lms_timer61    :  1;
        unsigned int st_clk_lms_timer70    :  1;
        unsigned int st_clk_lms_timer71    :  1;
        unsigned int reserved_2            :  1;
        unsigned int st_clk_lppll_logic    :  1;
        unsigned int st_clk_lp32kpll_logic :  1;
        unsigned int st_clk_sys_hss1       :  1;
        unsigned int st_clk_lms_wdt        :  1;
        unsigned int st_pclk_lms_wdt       :  1;
        unsigned int st_clk_pll_logic      :  1;
        unsigned int reserved_3            :  7;
        unsigned int st_clk_sys_lms        :  1;
        unsigned int st_pclk_lms_lpis      :  1;
    } reg;
}lms_crg_clkst2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lms_spmi_n       :  1;
        unsigned int ip_prst_pad_wrap_n      :  1;
        unsigned int ip_rst_pad_wrap_db_n    :  1;
        unsigned int reserved_0              :  2;
        unsigned int ip_prst_lms_syscnt_n    :  1;
        unsigned int ip_prst_lms_timestamp_n :  1;
        unsigned int ip_prst_lms_rtc0_n      :  1;
        unsigned int reserved_1              :  2;
        unsigned int ip_prst_lms_timer1_n    :  1;
        unsigned int ip_prst_lms_timer0_n    :  1;
        unsigned int reserved_2              :  1;
        unsigned int ip_prst_lms_wkupmng_n   :  1;
        unsigned int ip_prst_lms_spmi_n      :  1;
        unsigned int ip_prst_lms_n           :  1;
        unsigned int ip_rst_lms_rtc0_n       :  1;
        unsigned int reserved_3              :  3;
        unsigned int ip_rst_lms_timestamp_n  :  1;
        unsigned int ip_rst_lms_syscnt_n     :  1;
        unsigned int ip_rst_lp32kpll_logic_n :  1;
        unsigned int ip_rst_lppll_logic_n    :  1;
        unsigned int reserved_4              :  8;
    } reg;
}lms_crg_rst1_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lms_spmi_n       :  1;
        unsigned int ip_prst_pad_wrap_n      :  1;
        unsigned int ip_rst_pad_wrap_db_n    :  1;
        unsigned int reserved_0              :  2;
        unsigned int ip_prst_lms_syscnt_n    :  1;
        unsigned int ip_prst_lms_timestamp_n :  1;
        unsigned int ip_prst_lms_rtc0_n      :  1;
        unsigned int reserved_1              :  2;
        unsigned int ip_prst_lms_timer1_n    :  1;
        unsigned int ip_prst_lms_timer0_n    :  1;
        unsigned int reserved_2              :  1;
        unsigned int ip_prst_lms_wkupmng_n   :  1;
        unsigned int ip_prst_lms_spmi_n      :  1;
        unsigned int ip_prst_lms_n           :  1;
        unsigned int ip_rst_lms_rtc0_n       :  1;
        unsigned int reserved_3              :  3;
        unsigned int ip_rst_lms_timestamp_n  :  1;
        unsigned int ip_rst_lms_syscnt_n     :  1;
        unsigned int ip_rst_lp32kpll_logic_n :  1;
        unsigned int ip_rst_lppll_logic_n    :  1;
        unsigned int reserved_4              :  8;
    } reg;
}lms_crg_rst1_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ip_rst_lms_spmi_n       :  1;
        unsigned int ip_prst_pad_wrap_n      :  1;
        unsigned int ip_rst_pad_wrap_db_n    :  1;
        unsigned int reserved_0              :  2;
        unsigned int ip_prst_lms_syscnt_n    :  1;
        unsigned int ip_prst_lms_timestamp_n :  1;
        unsigned int ip_prst_lms_rtc0_n      :  1;
        unsigned int reserved_1              :  2;
        unsigned int ip_prst_lms_timer1_n    :  1;
        unsigned int ip_prst_lms_timer0_n    :  1;
        unsigned int reserved_2              :  1;
        unsigned int ip_prst_lms_wkupmng_n   :  1;
        unsigned int ip_prst_lms_spmi_n      :  1;
        unsigned int ip_prst_lms_n           :  1;
        unsigned int ip_rst_lms_rtc0_n       :  1;
        unsigned int reserved_3              :  3;
        unsigned int ip_rst_lms_timestamp_n  :  1;
        unsigned int ip_rst_lms_syscnt_n     :  1;
        unsigned int ip_rst_lp32kpll_logic_n :  1;
        unsigned int ip_rst_lppll_logic_n    :  1;
        unsigned int reserved_4              :  8;
    } reg;
}lms_crg_rst1_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           :  1;
        unsigned int ip_rst_lms_timer01_n :  1;
        unsigned int ip_rst_lms_timer10_n :  1;
        unsigned int ip_rst_lms_timer11_n :  1;
        unsigned int ip_rst_lms_timer20_n :  1;
        unsigned int ip_rst_lms_timer21_n :  1;
        unsigned int ip_rst_lms_timer30_n :  1;
        unsigned int ip_rst_lms_timer31_n :  1;
        unsigned int ip_rst_lms_timer40_n :  1;
        unsigned int ip_rst_lms_timer41_n :  1;
        unsigned int ip_rst_lms_timer50_n :  1;
        unsigned int ip_rst_lms_timer51_n :  1;
        unsigned int reserved_1           :  1;
        unsigned int ip_rst_lms_timer61_n :  1;
        unsigned int ip_rst_lms_timer70_n :  1;
        unsigned int ip_rst_lms_timer71_n :  1;
        unsigned int ip_prst_lms_wdt_n    :  1;
        unsigned int ip_rst_lms_wdt_n     :  1;
        unsigned int reserved_2           : 14;
    } reg;
}lms_crg_rst2_w1s_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           :  1;
        unsigned int ip_rst_lms_timer01_n :  1;
        unsigned int ip_rst_lms_timer10_n :  1;
        unsigned int ip_rst_lms_timer11_n :  1;
        unsigned int ip_rst_lms_timer20_n :  1;
        unsigned int ip_rst_lms_timer21_n :  1;
        unsigned int ip_rst_lms_timer30_n :  1;
        unsigned int ip_rst_lms_timer31_n :  1;
        unsigned int ip_rst_lms_timer40_n :  1;
        unsigned int ip_rst_lms_timer41_n :  1;
        unsigned int ip_rst_lms_timer50_n :  1;
        unsigned int ip_rst_lms_timer51_n :  1;
        unsigned int reserved_1           :  1;
        unsigned int ip_rst_lms_timer61_n :  1;
        unsigned int ip_rst_lms_timer70_n :  1;
        unsigned int ip_rst_lms_timer71_n :  1;
        unsigned int ip_prst_lms_wdt_n    :  1;
        unsigned int ip_rst_lms_wdt_n     :  1;
        unsigned int reserved_2           : 14;
    } reg;
}lms_crg_rst2_w1c_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           :  1;
        unsigned int ip_rst_lms_timer01_n :  1;
        unsigned int ip_rst_lms_timer10_n :  1;
        unsigned int ip_rst_lms_timer11_n :  1;
        unsigned int ip_rst_lms_timer20_n :  1;
        unsigned int ip_rst_lms_timer21_n :  1;
        unsigned int ip_rst_lms_timer30_n :  1;
        unsigned int ip_rst_lms_timer31_n :  1;
        unsigned int ip_rst_lms_timer40_n :  1;
        unsigned int ip_rst_lms_timer41_n :  1;
        unsigned int ip_rst_lms_timer50_n :  1;
        unsigned int ip_rst_lms_timer51_n :  1;
        unsigned int reserved_1           :  1;
        unsigned int ip_rst_lms_timer61_n :  1;
        unsigned int ip_rst_lms_timer70_n :  1;
        unsigned int ip_rst_lms_timer71_n :  1;
        unsigned int ip_prst_lms_wdt_n    :  1;
        unsigned int ip_rst_lms_wdt_n     :  1;
        unsigned int reserved_2           : 14;
    } reg;
}lms_crg_rst2_ro_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_lms_spmi      :  6;
        unsigned int div_clk_lms_timer_h   :  6;
        unsigned int sc_gt_clk_lms_spmi    :  1;
        unsigned int sc_gt_clk_lms_timer_h :  1;
        unsigned int reserved_0            :  2;
        unsigned int _bm_                  : 16;
    } reg;
}lms_crg_clkdiv3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_pll_logic_div            :  6;
        unsigned int div_clk_pll_logic_lp32kpll_div   :  6;
        unsigned int sc_gt_clk_pll_logic_div          :  1;
        unsigned int sc_gt_clk_pll_logic_lp32kpll_div :  1;
        unsigned int reserved_0                       :  2;
        unsigned int _bm_                             : 16;
    } reg;
}lms_crg_clkdiv4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_lms_timestamp     :  6;
        unsigned int div_clk_pad_wrap_db_div   :  6;
        unsigned int sc_gt_clk_lms_timestamp   :  1;
        unsigned int sc_gt_clk_pad_wrap_db_div :  1;
        unsigned int reserved_0                :  2;
        unsigned int _bm_                      : 16;
    } reg;
}lms_crg_clkdiv5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0           :  6;
        unsigned int div_clk_lms_syscnt   :  6;
        unsigned int reserved_1           :  1;
        unsigned int sc_gt_clk_lms_syscnt :  1;
        unsigned int reserved_2           :  2;
        unsigned int _bm_                 : 16;
    } reg;
}lms_crg_clkdiv6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_clkref_lppll    :  1;
        unsigned int sel_pclk_lms_sw     :  2;
        unsigned int sel_clk_lms_timer   :  1;
        unsigned int reserved_0          :  1;
        unsigned int sel_clk_lms_timer01 :  1;
        unsigned int sel_clk_lms_timer10 :  1;
        unsigned int sel_clk_lms_timer11 :  1;
        unsigned int sel_clk_lms_timer20 :  1;
        unsigned int sel_clk_lms_timer21 :  1;
        unsigned int sel_clk_lms_timer30 :  1;
        unsigned int sel_clk_lms_timer31 :  1;
        unsigned int sel_clk_lms_timer40 :  1;
        unsigned int sel_clk_lms_timer41 :  1;
        unsigned int sel_clk_lms_timer50 :  1;
        unsigned int sel_clk_lms_timer51 :  1;
        unsigned int _bm_                : 16;
    } reg;
}lms_crg_clkdiv7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0          :  1;
        unsigned int sel_clk_lms_timer61 :  1;
        unsigned int sel_clk_lms_timer70 :  1;
        unsigned int sel_clk_lms_timer71 :  1;
        unsigned int reserved_1          :  2;
        unsigned int sel_clk_pad_wrap_db :  1;
        unsigned int sel_clk_lms_spmi    :  1;
        unsigned int sel_clk_pll_logic   :  1;
        unsigned int reserved_2          :  7;
        unsigned int _bm_                : 16;
    } reg;
}lms_crg_clkdiv8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ddr_auto_refresh_bypass_cfg :  1;
        unsigned int reserved_0                  :  1;
        unsigned int xrse_power_up_bypass        :  1;
        unsigned int reserved_1                  : 13;
        unsigned int _bm_                        : 16;
    } reg;
}lms_crg_perictrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rst_req_soft :  1;
        unsigned int reserved_0   : 31;
    } reg;
}lms_crg_perictrl7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rst_region_ctrl :  1;
        unsigned int reserved_0      : 31;
    } reg;
}lms_crg_perictrl8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sel_sctrl_pclk_lms_bypass          :  1;
        unsigned int autogt_sctrl_clk_lppll_peri_bypass :  1;
        unsigned int autogt_sctrl_clk_sys_peri_bypass   :  1;
        unsigned int autogt_sctrl_clk_lppll_lpis_bypass :  1;
        unsigned int autogt_sctrl_clkin_ref_lpis_bypass :  1;
        unsigned int autogt_sctrl_clk_sys_hss1_bypass   :  1;
        unsigned int autogt_sctrl_clk_pll_logic_bypass  :  1;
        unsigned int autogt_sctrl_pclk_lms_lpis_bypass  :  1;
        unsigned int reserved_0                         : 24;
    } reg;
}lms_crg_perictrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_xrse_mtcmos_en   :  1;
        unsigned int sc_xrse_iso_en      :  1;
        unsigned int sc_xrse_mem_dr_poff :  2;
        unsigned int sc_xrse_mem_poff    :  2;
        unsigned int sc_xrse_mem_dr_sd   :  1;
        unsigned int sc_xrse_mem_sd      :  1;
        unsigned int sc_xrse_mem_dr_ds   :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}lms_crg_perictrl2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int clkrst_flag_spmi_bypass       :  1;
        unsigned int clkrst_flag_wkup_mng_bypass   :  1;
        unsigned int clkrst_flag_timer0_bypass     :  1;
        unsigned int clkrst_flag_timer1_bypass     :  1;
        unsigned int clkrst_flag_timer2_bypass     :  1;
        unsigned int clkrst_flag_wdt_bypass        :  1;
        unsigned int clkrst_flag_rtc0_bypass       :  1;
        unsigned int clkrst_flag_rtc1_bypass       :  1;
        unsigned int clkrst_flag_vad_bypass        :  1;
        unsigned int clkrst_flag_syscnt_bypass     :  1;
        unsigned int reserved_0                    :  1;
        unsigned int clkrst_flag_pad_wrap_bypass   :  1;
        unsigned int clkrst_flag_time_stamp_bypass :  1;
        unsigned int reserved_1                    : 19;
    } reg;
}lms_crg_perictrl3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rst_req_wd_lpctrl_mask    :  1;
        unsigned int rst_req_wd_sensorhub_mask :  1;
        unsigned int rst_req_wd_xctrl_cpu_mask :  1;
        unsigned int rst_req_wd_xctrl_ddr_mask :  1;
        unsigned int rst_req_wd_xrse_mask      :  1;
        unsigned int rst_req_wd_lms_mask       :  1;
        unsigned int rst_req_wd0_peri_mask     :  1;
        unsigned int rst_req_wd1_peri_mask     :  1;
        unsigned int rst_req_wd2_peri_mask     :  1;
        unsigned int rst_req_wd3_peri_mask     :  1;
        unsigned int rst_req_wd_audio_mask     :  1;
        unsigned int rst_req_wd_isp_mask       :  1;
        unsigned int rst_req_wd_npu_m85_mask   :  1;
        unsigned int rst_req_wd_npu_dsp_mask   :  1;
        unsigned int rst_req_tsensor_mask      :  1;
        unsigned int reserved_0                : 17;
    } reg;
}lms_crg_perictrl4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int xrse_mtcmos_ack         :  1;
        unsigned int xrse_mem_dr_sd_ack      :  1;
        unsigned int xrse_mem_sd_ack         :  1;
        unsigned int xrse_mtcmos_ack_timeout :  1;
        unsigned int efuse_done_timeout      :  1;
        unsigned int memrepair_done_timeout  :  1;
        unsigned int xrse_mem_dr_ds_ack      :  1;
        unsigned int reserved_0              : 25;
    } reg;
}lms_crg_peristat3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rst_req_recoder_clr :  1;
        unsigned int reserved_0          : 31;
    } reg;
}lms_crg_perictrl5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int rst_req_recoder : 32;
    } reg;
}lms_crg_peristat4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int div_clk_rtc_num : 16;
        unsigned int div_clk_rtc_den : 16;
    } reg;
}lms_crg_clkdiv9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_press_test_en  :  1;
        unsigned int lp32kpll_press_test_cnt : 24;
        unsigned int lp32kpll_cfg_rsv0       :  7;
    } reg;
}lms_crg_lp32kpll_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_err_boot_cnt   : 24;
        unsigned int lp32kpll_press_test_end :  1;
        unsigned int reserved_0              :  7;
    } reg;
}lms_crg_lp32kpll_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_lock_timeout : 13;
        unsigned int lp32kpll_cfg_rsv2     : 19;
    } reg;
}lms_crg_lp32kpll_locktimeout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_en        :  1;
        unsigned int lp32kpll_bypass    :  1;
        unsigned int lp32kpll_fbdiv     : 14;
        unsigned int lp32kpll_postdiv   :  4;
        unsigned int lp32kpll_cfg_valid :  1;
        unsigned int lp32kpll_gt        :  1;
        unsigned int lp32kpll_cfg_rsv3  : 10;
    } reg;
}lms_crg_lp32kpll_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_lockcount :  2;
        unsigned int lp32kpll_spare     : 10;
        unsigned int lp32kpll_fcalbyp   :  1;
        unsigned int lp32kpll_fcaltest  :  1;
        unsigned int lp32kpll_fcalin    :  8;
        unsigned int lp32kpll_cfg_rsv4  : 10;
    } reg;
}lms_crg_lp32kpll_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lp32kpll_en_state :  1;
        unsigned int lp32kpll_fcalout  :  8;
        unsigned int st_clk_32kpll     :  1;
        unsigned int reserved_0        : 22;
    } reg;
}lms_crg_lp32kpll_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_start_fsm_bypass :  1;
        unsigned int lppll_lock_timeout     :  7;
        unsigned int lppll_retry_num        :  3;
        unsigned int lppll_fsm_en           :  1;
        unsigned int lppll_cfg_rsv0         : 20;
    } reg;
}lms_crg_lppll_vote_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_press_test_en  :  1;
        unsigned int lppll_press_test_cnt : 24;
        unsigned int lppll_cfg_rsv1       :  7;
    } reg;
}lms_crg_lppll_press_test_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_err_boot_cnt   : 24;
        unsigned int lppll_press_test_end :  1;
        unsigned int reserved_0           :  7;
    } reg;
}lms_crg_lppll_err_boot_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_hw_vote_mask  :  8;
        unsigned int lppll_sft_vote_mask :  8;
        unsigned int lppll_cfg_rsv2      : 16;
    } reg;
}lms_crg_lppll_vote_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_vote_bypass :  8;
        unsigned int reserved_0        :  8;
        unsigned int _bm_              : 16;
    } reg;
}lms_crg_lppll_vote_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_vote_gt :  8;
        unsigned int reserved_0    :  8;
        unsigned int _bm_          : 16;
    } reg;
}lms_crg_lppll_vote_gt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_en        :  1;
        unsigned int lppll_bypass    :  1;
        unsigned int lppll_refdiv    :  6;
        unsigned int lppll_fbdiv     : 10;
        unsigned int lppll_postdiv   :  3;
        unsigned int lppll_cfg_valid :  1;
        unsigned int lppll_gt        :  1;
        unsigned int lppll_cfg_rsv3  :  9;
    } reg;
}lms_crg_lppll_ctrl0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_frac     : 24;
        unsigned int lppll_dsm_en   :  1;
        unsigned int lppll_cfg_rsv4 :  7;
    } reg;
}lms_crg_lppll_ctrl1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_offsetcalbyp  :  1;
        unsigned int lppll_offsetcalcnt  :  3;
        unsigned int lppll_offsetcalen   :  1;
        unsigned int lppll_offsetcalin   : 12;
        unsigned int lppll_offsetfastcal :  1;
        unsigned int lppll_cfg_rsv5      : 14;
    } reg;
}lms_crg_lppll_debug_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int lppll_en_state        :  1;
        unsigned int lppll_offsetcalout    : 12;
        unsigned int lppll_offsetcallock   :  1;
        unsigned int lppll_start_fsm_state :  3;
        unsigned int lppll_real_retry_cnt  :  3;
        unsigned int st_clk_lppll          :  1;
        unsigned int reserved_0            : 11;
    } reg;
}lms_crg_lppll_debug_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}lms_crg_regfile_cgbypass_t;

#endif
