// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef MEDIA2_CTRL_REGIF_H
#define MEDIA2_CTRL_REGIF_H

#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL                    0x0040
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQ_BYPASS_SHIFT                0
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQ_BYPASS_MASK                 0x00000001
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQREQN_SOFT_SHIFT              1
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQREQN_SOFT_MASK               0x00000002
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_QSM_DELAY_BYPASS_SHIFT           2
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_QSM_DELAY_BYPASS_MASK            0x00000004
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_QSM_DELAY_CNT_SHIFT              3
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_QSM_DELAY_CNT_MASK               0x000007f8
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS                  0x0044
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACCEPTN_SHIFT             0
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACCEPTN_MASK              0x00000001
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_DENY_SHIFT                1
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_DENY_MASK                 0x00000002
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACTIVE_SHIFT              2
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACTIVE_MASK               0x00000004
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_QSMIDLE_SHIFT                  3
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_QSMIDLE_MASK                   0x00000008
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_QSM_CLKQREQN_SHIFT             4
#define MEDIA2_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_QSM_CLKQREQN_MASK              0x00000010
#define MEDIA2_CTRL_ADB400_SLV_PW_CTRL                      0x0050
#define MEDIA2_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_REQN_SHIFT                    0
#define MEDIA2_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_REQN_MASK                     0x00000001
#define MEDIA2_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_PERMIT_DENY_SAR_SHIFT         1
#define MEDIA2_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_PERMIT_DENY_SAR_MASK          0x00000002
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS                    0x0054
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_SHIFT               0
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_MASK                0x00000001
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_DENY_SHIFT                  1
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_DENY_MASK                   0x00000002
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACTIVE_SHIFT                2
#define MEDIA2_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACTIVE_MASK                 0x00000004
#define MEDIA2_CTRL_ADB400_SLV_WAKEUP_CTRL                  0x0058
#define MEDIA2_CTRL_ADB400_SLV_WAKEUP_CTRL_SC_ADB400_SLV_WAKEUP_SHIFT                   0
#define MEDIA2_CTRL_ADB400_SLV_WAKEUP_CTRL_SC_ADB400_SLV_WAKEUP_MASK                    0x00000001
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL                    0x0060
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQ_BYPASS_SHIFT                0
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQ_BYPASS_MASK                 0x00000001
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQREQN_SOFT_SHIFT              1
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQREQN_SOFT_MASK               0x00000002
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_QSM_DELAY_BYPASS_SHIFT           2
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_QSM_DELAY_BYPASS_MASK            0x00000004
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_QSM_DELAY_CNT_SHIFT              3
#define MEDIA2_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_QSM_DELAY_CNT_MASK               0x000007f8
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS                  0x0064
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACCEPTN_SHIFT             0
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACCEPTN_MASK              0x00000001
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_DENY_SHIFT                1
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_DENY_MASK                 0x00000002
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACTIVE_SHIFT              2
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACTIVE_MASK               0x00000004
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_QSMIDLE_SHIFT                  3
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_QSMIDLE_MASK                   0x00000008
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_QSM_CLKQREQN_SHIFT             4
#define MEDIA2_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_QSM_CLKQREQN_MASK              0x00000010
#define MEDIA2_CTRL_ADB400_GT_CTRL                          0x0068
#define MEDIA2_CTRL_ADB400_GT_CTRL_SC_ADB400_SLV_GT_BYPASS_SHIFT                        0
#define MEDIA2_CTRL_ADB400_GT_CTRL_SC_ADB400_SLV_GT_BYPASS_MASK                         0x00000001
#define MEDIA2_CTRL_ADB400_GT_CTRL_SC_ADB400_MST_GT_BYPASS_SHIFT                        1
#define MEDIA2_CTRL_ADB400_GT_CTRL_SC_ADB400_MST_GT_BYPASS_MASK                         0x00000002
#define MEDIA2_CTRL_ADB400_GT_STATUS                        0x006c
#define MEDIA2_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_SLV_SHIFT                      0
#define MEDIA2_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_SLV_MASK                       0x00000001
#define MEDIA2_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_MST_SHIFT                      1
#define MEDIA2_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_MST_MASK                       0x00000002
#define MEDIA2_CTRL_RAM_CTRL_BUS_0                          0x0080
#define MEDIA2_CTRL_RAM_CTRL_BUS_1                          0x0084
#define MEDIA2_CTRL_RAM_CTRL_BUS_2                          0x0088
#define MEDIA2_CTRL_NDB_AUTO_BYPASS                         0x00c0
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_0_MST_AUTOGATE_BYP_SHIFT                     0
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_0_MST_AUTOGATE_BYP_MASK                      0x00000001
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_1_MST_AUTOGATE_BYP_SHIFT                     1
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_1_MST_AUTOGATE_BYP_MASK                      0x00000002
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_0_SLV_AUTOGATE_BYP_SHIFT                     2
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_0_SLV_AUTOGATE_BYP_MASK                      0x00000004
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_1_SLV_AUTOGATE_BYP_SHIFT                     3
#define MEDIA2_CTRL_NDB_AUTO_BYPASS_SC_NDB_1_SLV_AUTOGATE_BYP_MASK                      0x00000008
#define MEDIA2_CTRL_NDB_BUSY_STATUS                         0x00c4
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_R_MST_BUSY_SHIFT                           0
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_R_MST_BUSY_MASK                            0x00000001
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_W_MST_BUSY_SHIFT                           1
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_W_MST_BUSY_MASK                            0x00000002
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_R_SLV_BUSY_SHIFT                           2
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_R_SLV_BUSY_MASK                            0x00000004
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_W_SLV_BUSY_SHIFT                           3
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_0_W_SLV_BUSY_MASK                            0x00000008
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_R_MST_BUSY_SHIFT                           4
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_R_MST_BUSY_MASK                            0x00000010
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_W_MST_BUSY_SHIFT                           5
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_W_MST_BUSY_MASK                            0x00000020
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_R_SLV_BUSY_SHIFT                           6
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_R_SLV_BUSY_MASK                            0x00000040
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_W_SLV_BUSY_SHIFT                           7
#define MEDIA2_CTRL_NDB_BUSY_STATUS_SC_NDB_1_W_SLV_BUSY_MASK                            0x00000080
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ                         0x00c8
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_0_R_SLV_PWR_REQ_SHIFT                        0
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_0_R_SLV_PWR_REQ_MASK                         0x00000001
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_0_W_SLV_PWR_REQ_SHIFT                        1
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_0_W_SLV_PWR_REQ_MASK                         0x00000002
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_1_R_SLV_PWR_REQ_SHIFT                        2
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_1_R_SLV_PWR_REQ_MASK                         0x00000004
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_1_W_SLV_PWR_REQ_SHIFT                        3
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_SC_NDB_1_W_SLV_PWR_REQ_MASK                         0x00000008
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT                  0x00cc
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_0_R_SLV_PWREQ_ACCEPT_SHIFT            0
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_0_R_SLV_PWREQ_ACCEPT_MASK             0x00000001
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_0_W_SLV_PWREQ_ACCEPT_SHIFT            1
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_0_W_SLV_PWREQ_ACCEPT_MASK             0x00000002
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_1_R_SLV_PWREQ_ACCEPT_SHIFT            2
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_1_R_SLV_PWREQ_ACCEPT_MASK             0x00000004
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_1_W_SLV_PWREQ_ACCEPT_SHIFT            3
#define MEDIA2_CTRL_NDB_SLV_PWR_REQ_ACCEPT_SC_NDB_1_W_SLV_PWREQ_ACCEPT_MASK             0x00000008
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE                  0x00d0
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_0_R_SLV_OST_ZERO_FORCE_SHIFT          0
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_0_R_SLV_OST_ZERO_FORCE_MASK           0x00000001
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_0_W_SLV_OST_ZERO_FORCE_SHIFT          1
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_0_W_SLV_OST_ZERO_FORCE_MASK           0x00000002
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_1_R_SLV_OST_ZERO_FORCE_SHIFT          2
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_1_R_SLV_OST_ZERO_FORCE_MASK           0x00000004
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_1_W_SLV_OST_ZERO_FORCE_SHIFT          3
#define MEDIA2_CTRL_NDB_SLV_OST_ZERO_FORCE_SC_NDB_1_W_SLV_OST_ZERO_FORCE_MASK           0x00000008
#define MEDIA2_CTRL_NDB_RW_MST_DFX                          0x00e0
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_0_R_MST_DFX_SHIFT                             0
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_0_R_MST_DFX_MASK                              0x0000001f
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_0_W_MST_DFX_SHIFT                             8
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_0_W_MST_DFX_MASK                              0x00001f00
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_1_R_MST_DFX_SHIFT                             16
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_1_R_MST_DFX_MASK                              0x001f0000
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_1_W_MST_DFX_SHIFT                             24
#define MEDIA2_CTRL_NDB_RW_MST_DFX_SC_NDB_1_W_MST_DFX_MASK                              0x1f000000
#define MEDIA2_CTRL_NDB_RW_SLV_DFX                          0x00e4
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_0_R_SLV_DFX_SHIFT                             0
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_0_R_SLV_DFX_MASK                              0x0000001f
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_0_W_SLV_DFX_SHIFT                             8
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_0_W_SLV_DFX_MASK                              0x00001f00
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_1_R_SLV_DFX_SHIFT                             16
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_1_R_SLV_DFX_MASK                              0x001f0000
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_1_W_SLV_DFX_SHIFT                             24
#define MEDIA2_CTRL_NDB_RW_SLV_DFX_SC_NDB_1_W_SLV_DFX_MASK                              0x1f000000
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL                         0x0400
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_NPU_IDLEREQ_SHIFT                     0
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_NPU_IDLEREQ_MASK                      0x00000001
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_MAIN_IDLEREQ_SHIFT                    1
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_MAIN_IDLEREQ_MASK                     0x00000002
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_MISP_DATA_IDLEREQ_SHIFT               2
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_MISP_DATA_IDLEREQ_MASK                0x00000004
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_MISP_CFG_IDLEREQ_SHIFT                3
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_MISP_CFG_IDLEREQ_MASK                 0x00000008
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_VENC_DATA_IDLEREQ_SHIFT               4
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_VENC_DATA_IDLEREQ_MASK                0x00000010
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_VENC_CFG_IDLEREQ_SHIFT                5
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_VENC_CFG_IDLEREQ_MASK                 0x00000020
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_SYSCFG_IDLEREQ_SHIFT                  6
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_SYSCFG_IDLEREQ_MASK                   0x00000040
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_TCU_IDLEREQ_SHIFT                     7
#define MEDIA2_CTRL_MEDIA2_BUS_CTRL_SC_MEDIA2_BUS_TCU_IDLEREQ_MASK                      0x00000080
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS                       0x0404
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLE_SHIFT                      0
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLE_MASK                       0x00000001
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLEACK_SHIFT                   1
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_NPU_IDLEACK_MASK                    0x00000002
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MAIN_IDLE_SHIFT                     2
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MAIN_IDLE_MASK                      0x00000004
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MAIN_IDLEACK_SHIFT                  3
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MAIN_IDLEACK_MASK                   0x00000008
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_DATA_IDLE_SHIFT                4
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_DATA_IDLE_MASK                 0x00000010
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_DATA_IDLEACK_SHIFT             5
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_DATA_IDLEACK_MASK              0x00000020
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_CFG_IDLE_SHIFT                 6
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_CFG_IDLE_MASK                  0x00000040
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_CFG_IDLEACK_SHIFT              7
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_MISP_CFG_IDLEACK_MASK               0x00000080
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_DATA_IDLE_SHIFT                8
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_DATA_IDLE_MASK                 0x00000100
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_DATA_IDLEACK_SHIFT             9
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_DATA_IDLEACK_MASK              0x00000200
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_CFG_IDLE_SHIFT                 10
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_CFG_IDLE_MASK                  0x00000400
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_CFG_IDLEACK_SHIFT              11
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_VENC_CFG_IDLEACK_MASK               0x00000800
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_SYSCFG_IDLE_SHIFT                   12
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_SYSCFG_IDLE_MASK                    0x00001000
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_SYSCFG_IDLEACK_SHIFT                13
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_SYSCFG_IDLEACK_MASK                 0x00002000
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_TCU_IDLE_SHIFT                      14
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_TCU_IDLE_MASK                       0x00004000
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_TCU_IDLEACK_SHIFT                   15
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_SC_MEDIA2_BUS_TCU_IDLEACK_MASK                    0x00008000
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_TIMEOUT_EN            0x040c
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_TIMEOUT_EN_SC_MEDIA2_BUS_TIMEOUT_EN_SHIFT         0
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_TIMEOUT_EN_SC_MEDIA2_BUS_TIMEOUT_EN_MASK          0x00000001
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_TIMEOUT               0x0410
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_TIMEOUT_SC_MEDIA2_BUS_TGT_TIMEOUT_SHIFT           0
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_TIMEOUT_SC_MEDIA2_BUS_TGT_TIMEOUT_MASK            0x000000ff
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_NOPENDING             0x0414
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_NOPENDING_SC_MEDIA2_BUS_TGT_NO_PENDING_TRANS_SHIFT 0
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_NOPENDING_SC_MEDIA2_BUS_TGT_NO_PENDING_TRANS_MASK  0x000000ff
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_NOPENDING_SC_MEDIA2_BUS_INI_NO_PENDING_TRANS_SHIFT 16
#define MEDIA2_CTRL_MEDIA2_BUS_STATUS_NOPENDING_SC_MEDIA2_BUS_INI_NO_PENDING_TRANS_MASK  0x003f0000
#define MEDIA2_CTRL_MISP_BUS_CTRL                           0x0440
#define MEDIA2_CTRL_MISP_BUS_CTRL_SC_MISP_BUS_M2_DATA_IDLEREQ_SHIFT                     0
#define MEDIA2_CTRL_MISP_BUS_CTRL_SC_MISP_BUS_M2_DATA_IDLEREQ_MASK                      0x00000001
#define MEDIA2_CTRL_MISP_BUS_STATUS                         0x0444
#define MEDIA2_CTRL_MISP_BUS_STATUS_SC_MISP_BUS_M2_DATA_IDLE_SHIFT                      0
#define MEDIA2_CTRL_MISP_BUS_STATUS_SC_MISP_BUS_M2_DATA_IDLE_MASK                       0x00000001
#define MEDIA2_CTRL_MISP_BUS_STATUS_SC_MISP_BUS_M2_DATA_IDLEACK_SHIFT                   1
#define MEDIA2_CTRL_MISP_BUS_STATUS_SC_MISP_BUS_M2_DATA_IDLEACK_MASK                    0x00000002
#define MEDIA2_CTRL_MISP_BUS_STATUS_TIMEOUT_EN              0x044c
#define MEDIA2_CTRL_MISP_BUS_STATUS_TIMEOUT_EN_SC_MISP_BUS_TIMEOUT_EN_SHIFT             0
#define MEDIA2_CTRL_MISP_BUS_STATUS_TIMEOUT_EN_SC_MISP_BUS_TIMEOUT_EN_MASK              0x00000001
#define MEDIA2_CTRL_MISP_BUS_STATUS_TIMEOUT                 0x0450
#define MEDIA2_CTRL_MISP_BUS_STATUS_TIMEOUT_SC_MISP_BUS_TGT_TIMEOUT_SHIFT               0
#define MEDIA2_CTRL_MISP_BUS_STATUS_TIMEOUT_SC_MISP_BUS_TGT_TIMEOUT_MASK                0x0000001f
#define MEDIA2_CTRL_MISP_BUS_STATUS_NOPENDING               0x0454
#define MEDIA2_CTRL_MISP_BUS_STATUS_NOPENDING_SC_MISP_BUS_TGT_NO_PENDING_TRANS_SHIFT    0
#define MEDIA2_CTRL_MISP_BUS_STATUS_NOPENDING_SC_MISP_BUS_TGT_NO_PENDING_TRANS_MASK     0x0000001f
#define MEDIA2_CTRL_MISP_BUS_STATUS_NOPENDING_SC_MISP_BUS_INI_NO_PENDING_TRANS_SHIFT    16
#define MEDIA2_CTRL_MISP_BUS_STATUS_NOPENDING_SC_MISP_BUS_INI_NO_PENDING_TRANS_MASK     0x003f0000
#define MEDIA2_CTRL_ISP_BUS_CTRL                            0x0480
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE0_IDLEREQ_SHIFT                           0
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE0_IDLEREQ_MASK                            0x00000001
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE1_IDLEREQ_SHIFT                           1
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE1_IDLEREQ_MASK                            0x00000002
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE2_IDLEREQ_SHIFT                           2
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE2_IDLEREQ_MASK                            0x00000004
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE3_IDLEREQ_SHIFT                           3
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FE3_IDLEREQ_MASK                            0x00000008
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_BE0_IDLEREQ_SHIFT                           4
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_BE0_IDLEREQ_MASK                            0x00000010
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_BE1_IDLEREQ_SHIFT                           5
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_BE1_IDLEREQ_MASK                            0x00000020
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_PE_IDLEREQ_SHIFT                            6
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_PE_IDLEREQ_MASK                             0x00000040
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_CVE_IDLEREQ_SHIFT                           7
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_CVE_IDLEREQ_MASK                            0x00000080
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_DMA_IDLEREQ_SHIFT                           8
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_DMA_IDLEREQ_MASK                            0x00000100
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_CDMA_CFG_IDLEREQ_SHIFT                      9
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_CDMA_CFG_IDLEREQ_MASK                       0x00000200
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_CDMA_DATA_IDLEREQ_SHIFT                     10
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_CDMA_DATA_IDLEREQ_MASK                      0x00000400
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_TO_MCU_CFG_IDLEREQ_SHIFT                    11
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_TO_MCU_CFG_IDLEREQ_MASK                     0x00000800
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_MCU_DATA_IDLEREQ_SHIFT                      12
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_MCU_DATA_IDLEREQ_MASK                       0x00001000
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FROM_MCU_CFG_IDLEREQ_SHIFT                  13
#define MEDIA2_CTRL_ISP_BUS_CTRL_SC_ISP_BUS_FROM_MCU_CFG_IDLEREQ_MASK                   0x00002000
#define MEDIA2_CTRL_ISP_BUS_STATUS                          0x0484
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE0_IDLE_SHIFT                            0
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE0_IDLE_MASK                             0x00000001
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE0_IDLEACK_SHIFT                         1
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE0_IDLEACK_MASK                          0x00000002
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE1_IDLE_SHIFT                            2
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE1_IDLE_MASK                             0x00000004
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE1_IDLEACK_SHIFT                         3
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE1_IDLEACK_MASK                          0x00000008
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE2_IDLE_SHIFT                            4
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE2_IDLE_MASK                             0x00000010
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE2_IDLEACK_SHIFT                         5
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE2_IDLEACK_MASK                          0x00000020
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE3_IDLE_SHIFT                            6
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE3_IDLE_MASK                             0x00000040
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE3_IDLEACK_SHIFT                         7
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FE3_IDLEACK_MASK                          0x00000080
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE0_IDLE_SHIFT                            8
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE0_IDLE_MASK                             0x00000100
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE0_IDLEACK_SHIFT                         9
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE0_IDLEACK_MASK                          0x00000200
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE1_IDLE_SHIFT                            10
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE1_IDLE_MASK                             0x00000400
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE1_IDLEACK_SHIFT                         11
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_BE1_IDLEACK_MASK                          0x00000800
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_PE_IDLE_SHIFT                             12
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_PE_IDLE_MASK                              0x00001000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_PE_IDLEACK_SHIFT                          13
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_PE_IDLEACK_MASK                           0x00002000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CVE_IDLE_SHIFT                            14
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CVE_IDLE_MASK                             0x00004000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CVE_IDLEACK_SHIFT                         15
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CVE_IDLEACK_MASK                          0x00008000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_DMA_IDLE_SHIFT                            16
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_DMA_IDLE_MASK                             0x00010000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_DMA_IDLEACK_SHIFT                         17
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_DMA_IDLEACK_MASK                          0x00020000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_CFG_IDLE_SHIFT                       18
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_CFG_IDLE_MASK                        0x00040000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_CFG_IDLEACK_SHIFT                    19
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_CFG_IDLEACK_MASK                     0x00080000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_DATA_IDLE_SHIFT                      20
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_DATA_IDLE_MASK                       0x00100000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_DATA_IDLEACK_SHIFT                   21
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_CDMA_DATA_IDLEACK_MASK                    0x00200000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_TO_MCU_CFG_IDLE_SHIFT                     22
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_TO_MCU_CFG_IDLE_MASK                      0x00400000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_TO_MCU_CFG_IDLEACK_SHIFT                  23
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_TO_MCU_CFG_IDLEACK_MASK                   0x00800000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_MCU_DATA_IDLE_SHIFT                       24
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_MCU_DATA_IDLE_MASK                        0x01000000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_MCU_DATA_IDLEACK_SHIFT                    25
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_MCU_DATA_IDLEACK_MASK                     0x02000000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FROM_MCU_CFG_IDLE_SHIFT                   26
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FROM_MCU_CFG_IDLE_MASK                    0x04000000
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FROM_MCU_CFG_IDLEACK_SHIFT                27
#define MEDIA2_CTRL_ISP_BUS_STATUS_SC_ISP_BUS_FROM_MCU_CFG_IDLEACK_MASK                 0x08000000
#define MEDIA2_CTRL_ISP_BUS_STATUS_TIMEOUT_EN               0x048c
#define MEDIA2_CTRL_ISP_BUS_STATUS_TIMEOUT_EN_SC_ISP_BUS_TIMEOUT_EN_SHIFT               0
#define MEDIA2_CTRL_ISP_BUS_STATUS_TIMEOUT_EN_SC_ISP_BUS_TIMEOUT_EN_MASK                0x00000001
#define MEDIA2_CTRL_ISP_BUS_STATUS_TIMEOUT                  0x0490
#define MEDIA2_CTRL_ISP_BUS_STATUS_TIMEOUT_SC_ISP_BUS_TGT_TIMEOUT_SHIFT                 0
#define MEDIA2_CTRL_ISP_BUS_STATUS_TIMEOUT_SC_ISP_BUS_TGT_TIMEOUT_MASK                  0x000003ff
#define MEDIA2_CTRL_ISP_BUS_STATUS_NOPENDING                0x0494
#define MEDIA2_CTRL_ISP_BUS_STATUS_NOPENDING_SC_ISP_BUS_TGT_NO_PENDING_TRANS_SHIFT      0
#define MEDIA2_CTRL_ISP_BUS_STATUS_NOPENDING_SC_ISP_BUS_TGT_NO_PENDING_TRANS_MASK       0x000003ff
#define MEDIA2_CTRL_ISP_BUS_STATUS_NOPENDING_SC_ISP_BUS_INI_NO_PENDING_TRANS_SHIFT      16
#define MEDIA2_CTRL_ISP_BUS_STATUS_NOPENDING_SC_ISP_BUS_INI_NO_PENDING_TRANS_MASK       0x3fff0000
#define MEDIA2_CTRL_VENC_BUS_CTRL                           0x04c0
#define MEDIA2_CTRL_VENC_BUS_CTRL_SC_VENC_BUS_M2_IDLEREQ_SHIFT                          0
#define MEDIA2_CTRL_VENC_BUS_CTRL_SC_VENC_BUS_M2_IDLEREQ_MASK                           0x00000001
#define MEDIA2_CTRL_VENC_BUS_CTRL_SC_VENC_BUS_IDLEREQ_SHIFT                             1
#define MEDIA2_CTRL_VENC_BUS_CTRL_SC_VENC_BUS_IDLEREQ_MASK                              0x00000002
#define MEDIA2_CTRL_VENC_BUS_STATUS                         0x04c4
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_M2_IDLE_SHIFT                           0
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_M2_IDLE_MASK                            0x00000001
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_M2_IDLEACK_SHIFT                        1
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_M2_IDLEACK_MASK                         0x00000002
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_IDLE_SHIFT                              2
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_IDLE_MASK                               0x00000004
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_IDLEACK_SHIFT                           3
#define MEDIA2_CTRL_VENC_BUS_STATUS_SC_VENC_BUS_IDLEACK_MASK                            0x00000008
#define MEDIA2_CTRL_VENC_BUS_STATUS_TIMEOUT_EN              0x04cc
#define MEDIA2_CTRL_VENC_BUS_STATUS_TIMEOUT_EN_SC_VENC_BUS_TIMEOUT_EN_SHIFT             0
#define MEDIA2_CTRL_VENC_BUS_STATUS_TIMEOUT_EN_SC_VENC_BUS_TIMEOUT_EN_MASK              0x00000001
#define MEDIA2_CTRL_VENC_BUS_STATUS_TIMEOUT                 0x04d0
#define MEDIA2_CTRL_VENC_BUS_STATUS_TIMEOUT_SC_VENC_BUS_TGT_TIMEOUT_SHIFT               0
#define MEDIA2_CTRL_VENC_BUS_STATUS_TIMEOUT_SC_VENC_BUS_TGT_TIMEOUT_MASK                0x00000007
#define MEDIA2_CTRL_VENC_BUS_STATUS_NOPENDING               0x04d4
#define MEDIA2_CTRL_VENC_BUS_STATUS_NOPENDING_SC_VENC_BUS_TGT_NO_PENDING_TRANS_SHIFT    0
#define MEDIA2_CTRL_VENC_BUS_STATUS_NOPENDING_SC_VENC_BUS_TGT_NO_PENDING_TRANS_MASK     0x00000007
#define MEDIA2_CTRL_VENC_BUS_STATUS_NOPENDING_SC_VENC_BUS_INI_NO_PENDING_TRANS_SHIFT    16
#define MEDIA2_CTRL_VENC_BUS_STATUS_NOPENDING_SC_VENC_BUS_INI_NO_PENDING_TRANS_MASK     0x00030000
#define MEDIA2_CTRL_NS_FIREWALL_ST                          0x4000
#define MEDIA2_CTRL_NS_FIREWALL_ST_NS_FW_CLEAR_SHIFT                                    0
#define MEDIA2_CTRL_NS_FIREWALL_ST_NS_FW_PERROR_SHIFT                                   1
#define MEDIA2_CTRL_NS_FIREWALL_ST_NS_FW_PERROR_MASK                                    0x00000002
#define MEDIA2_CTRL_NS_FIREWALL_ST_NS_FW_FAIL_PMID_SHIFT                                8
#define MEDIA2_CTRL_NS_FIREWALL_ST_NS_FW_FAIL_PMID_MASK                                 0x0000ff00
#define MEDIA2_CTRL_NS_FIREWALL_FAIL_ADDR                   0x4004
#define MEDIA2_CTRL_BIST_SEL_ST                             0x4080
#define MEDIA2_CTRL_BIST_SEL_ST_SC_MEDIA2_SEL_TCU_BIST_SHIFT                            0
#define MEDIA2_CTRL_BIST_SEL_ST_SC_MEDIA2_SEL_TCU_BIST_MASK                             0x00000001
#define MEDIA2_CTRL_REGFILE_CGBYPASS                        0x4084
#define MEDIA2_CTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                             0
#define MEDIA2_CTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                              0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_clkq_bypass      :  1;
        unsigned int sc_adb400_slv_clkqreqn_soft    :  1;
        unsigned int sc_adb400_slv_qsm_delay_bypass :  1;
        unsigned int sc_adb400_slv_qsm_delay_cnt    :  8;
        unsigned int reserved_0                     :  5;
        unsigned int _bm_                           : 16;
    } reg;
}media2_ctrl_adb400_slv_clkq_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_clkq_acceptn :  1;
        unsigned int sc_adb400_slv_clkq_deny    :  1;
        unsigned int sc_adb400_slv_clkq_active  :  1;
        unsigned int sc_adb400_slv_qsmidle      :  1;
        unsigned int sc_adb400_slv_qsm_clkqreqn :  1;
        unsigned int reserved_0                 : 27;
    } reg;
}media2_ctrl_adb400_slv_clkq_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_pwrq_reqn            :  1;
        unsigned int sc_adb400_slv_pwrq_permit_deny_sar :  1;
        unsigned int reserved_0                         : 14;
        unsigned int _bm_                               : 16;
    } reg;
}media2_ctrl_adb400_slv_pw_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_pwrq_acceptn :  1;
        unsigned int sc_adb400_slv_pwrq_deny    :  1;
        unsigned int sc_adb400_slv_pwrq_active  :  1;
        unsigned int reserved_0                 : 29;
    } reg;
}media2_ctrl_adb400_slv_pw_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_wakeup :  1;
        unsigned int reserved_0           : 31;
    } reg;
}media2_ctrl_adb400_slv_wakeup_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_mst_clkq_bypass      :  1;
        unsigned int sc_adb400_mst_clkqreqn_soft    :  1;
        unsigned int sc_adb400_mst_qsm_delay_bypass :  1;
        unsigned int sc_adb400_mst_qsm_delay_cnt    :  8;
        unsigned int reserved_0                     :  5;
        unsigned int _bm_                           : 16;
    } reg;
}media2_ctrl_adb400_mst_clkq_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_mst_clkq_acceptn :  1;
        unsigned int sc_adb400_mst_clkq_deny    :  1;
        unsigned int sc_adb400_mst_clkq_active  :  1;
        unsigned int sc_adb400_mst_qsmidle      :  1;
        unsigned int sc_adb400_mst_qsm_clkqreqn :  1;
        unsigned int reserved_0                 : 27;
    } reg;
}media2_ctrl_adb400_mst_clkq_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_gt_bypass :  1;
        unsigned int sc_adb400_mst_gt_bypass :  1;
        unsigned int reserved_0              : 14;
        unsigned int _bm_                    : 16;
    } reg;
}media2_ctrl_adb400_gt_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_aclk_adb400_data_slv :  1;
        unsigned int st_aclk_adb400_data_mst :  1;
        unsigned int reserved_0              : 30;
    } reg;
}media2_ctrl_adb400_gt_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ram_ctrl_bus_0 : 32;
    } reg;
}media2_ctrl_ram_ctrl_bus_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ram_ctrl_bus_1 : 32;
    } reg;
}media2_ctrl_ram_ctrl_bus_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ram_ctrl_bus_2 : 32;
    } reg;
}media2_ctrl_ram_ctrl_bus_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_mst_autogate_byp :  1;
        unsigned int sc_ndb_1_mst_autogate_byp :  1;
        unsigned int sc_ndb_0_slv_autogate_byp :  1;
        unsigned int sc_ndb_1_slv_autogate_byp :  1;
        unsigned int reserved_0                : 12;
        unsigned int _bm_                      : 16;
    } reg;
}media2_ctrl_ndb_auto_bypass_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_r_mst_busy :  1;
        unsigned int sc_ndb_0_w_mst_busy :  1;
        unsigned int sc_ndb_0_r_slv_busy :  1;
        unsigned int sc_ndb_0_w_slv_busy :  1;
        unsigned int sc_ndb_1_r_mst_busy :  1;
        unsigned int sc_ndb_1_w_mst_busy :  1;
        unsigned int sc_ndb_1_r_slv_busy :  1;
        unsigned int sc_ndb_1_w_slv_busy :  1;
        unsigned int reserved_0          : 24;
    } reg;
}media2_ctrl_ndb_busy_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_r_slv_pwr_req :  1;
        unsigned int sc_ndb_0_w_slv_pwr_req :  1;
        unsigned int sc_ndb_1_r_slv_pwr_req :  1;
        unsigned int sc_ndb_1_w_slv_pwr_req :  1;
        unsigned int reserved_0             : 12;
        unsigned int _bm_                   : 16;
    } reg;
}media2_ctrl_ndb_slv_pwr_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_r_slv_pwreq_accept :  1;
        unsigned int sc_ndb_0_w_slv_pwreq_accept :  1;
        unsigned int sc_ndb_1_r_slv_pwreq_accept :  1;
        unsigned int sc_ndb_1_w_slv_pwreq_accept :  1;
        unsigned int reserved_0                  : 28;
    } reg;
}media2_ctrl_ndb_slv_pwr_req_accept_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_r_slv_ost_zero_force :  1;
        unsigned int sc_ndb_0_w_slv_ost_zero_force :  1;
        unsigned int sc_ndb_1_r_slv_ost_zero_force :  1;
        unsigned int sc_ndb_1_w_slv_ost_zero_force :  1;
        unsigned int reserved_0                    : 12;
        unsigned int _bm_                          : 16;
    } reg;
}media2_ctrl_ndb_slv_ost_zero_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_r_mst_dfx :  5;
        unsigned int reserved_0         :  3;
        unsigned int sc_ndb_0_w_mst_dfx :  5;
        unsigned int reserved_1         :  3;
        unsigned int sc_ndb_1_r_mst_dfx :  5;
        unsigned int reserved_2         :  3;
        unsigned int sc_ndb_1_w_mst_dfx :  5;
        unsigned int reserved_3         :  3;
    } reg;
}media2_ctrl_ndb_rw_mst_dfx_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_0_r_slv_dfx :  5;
        unsigned int reserved_0         :  3;
        unsigned int sc_ndb_0_w_slv_dfx :  5;
        unsigned int reserved_1         :  3;
        unsigned int sc_ndb_1_r_slv_dfx :  5;
        unsigned int reserved_2         :  3;
        unsigned int sc_ndb_1_w_slv_dfx :  5;
        unsigned int reserved_3         :  3;
    } reg;
}media2_ctrl_ndb_rw_slv_dfx_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_bus_npu_idlereq       :  1;
        unsigned int sc_media2_bus_main_idlereq      :  1;
        unsigned int sc_media2_bus_misp_data_idlereq :  1;
        unsigned int sc_media2_bus_misp_cfg_idlereq  :  1;
        unsigned int sc_media2_bus_venc_data_idlereq :  1;
        unsigned int sc_media2_bus_venc_cfg_idlereq  :  1;
        unsigned int sc_media2_bus_syscfg_idlereq    :  1;
        unsigned int sc_media2_bus_tcu_idlereq       :  1;
        unsigned int reserved_0                      :  8;
        unsigned int _bm_                            : 16;
    } reg;
}media2_ctrl_media2_bus_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_bus_npu_idle          :  1;
        unsigned int sc_media2_bus_npu_idleack       :  1;
        unsigned int sc_media2_bus_main_idle         :  1;
        unsigned int sc_media2_bus_main_idleack      :  1;
        unsigned int sc_media2_bus_misp_data_idle    :  1;
        unsigned int sc_media2_bus_misp_data_idleack :  1;
        unsigned int sc_media2_bus_misp_cfg_idle     :  1;
        unsigned int sc_media2_bus_misp_cfg_idleack  :  1;
        unsigned int sc_media2_bus_venc_data_idle    :  1;
        unsigned int sc_media2_bus_venc_data_idleack :  1;
        unsigned int sc_media2_bus_venc_cfg_idle     :  1;
        unsigned int sc_media2_bus_venc_cfg_idleack  :  1;
        unsigned int sc_media2_bus_syscfg_idle       :  1;
        unsigned int sc_media2_bus_syscfg_idleack    :  1;
        unsigned int sc_media2_bus_tcu_idle          :  1;
        unsigned int sc_media2_bus_tcu_idleack       :  1;
        unsigned int reserved_0                      : 16;
    } reg;
}media2_ctrl_media2_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_bus_timeout_en :  1;
        unsigned int reserved_0               : 15;
        unsigned int _bm_                     : 16;
    } reg;
}media2_ctrl_media2_bus_status_timeout_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_bus_tgt_timeout :  8;
        unsigned int reserved_0                : 24;
    } reg;
}media2_ctrl_media2_bus_status_timeout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_bus_tgt_no_pending_trans :  8;
        unsigned int reserved_0                         :  8;
        unsigned int sc_media2_bus_ini_no_pending_trans :  6;
        unsigned int reserved_1                         : 10;
    } reg;
}media2_ctrl_media2_bus_status_nopending_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_misp_bus_m2_data_idlereq :  1;
        unsigned int reserved_0                  : 15;
        unsigned int _bm_                        : 16;
    } reg;
}media2_ctrl_misp_bus_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_misp_bus_m2_data_idle    :  1;
        unsigned int sc_misp_bus_m2_data_idleack :  1;
        unsigned int reserved_0                  : 30;
    } reg;
}media2_ctrl_misp_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_misp_bus_timeout_en :  1;
        unsigned int reserved_0             : 15;
        unsigned int _bm_                   : 16;
    } reg;
}media2_ctrl_misp_bus_status_timeout_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_misp_bus_tgt_timeout :  5;
        unsigned int reserved_0              : 27;
    } reg;
}media2_ctrl_misp_bus_status_timeout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_misp_bus_tgt_no_pending_trans :  5;
        unsigned int reserved_0                       : 11;
        unsigned int sc_misp_bus_ini_no_pending_trans :  6;
        unsigned int reserved_1                       : 10;
    } reg;
}media2_ctrl_misp_bus_status_nopending_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_isp_bus_fe0_idlereq          :  1;
        unsigned int sc_isp_bus_fe1_idlereq          :  1;
        unsigned int sc_isp_bus_fe2_idlereq          :  1;
        unsigned int sc_isp_bus_fe3_idlereq          :  1;
        unsigned int sc_isp_bus_be0_idlereq          :  1;
        unsigned int sc_isp_bus_be1_idlereq          :  1;
        unsigned int sc_isp_bus_pe_idlereq           :  1;
        unsigned int sc_isp_bus_cve_idlereq          :  1;
        unsigned int sc_isp_bus_dma_idlereq          :  1;
        unsigned int sc_isp_bus_cdma_cfg_idlereq     :  1;
        unsigned int sc_isp_bus_cdma_data_idlereq    :  1;
        unsigned int sc_isp_bus_to_mcu_cfg_idlereq   :  1;
        unsigned int sc_isp_bus_mcu_data_idlereq     :  1;
        unsigned int sc_isp_bus_from_mcu_cfg_idlereq :  1;
        unsigned int reserved_0                      :  2;
        unsigned int _bm_                            : 16;
    } reg;
}media2_ctrl_isp_bus_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_isp_bus_fe0_idle             :  1;
        unsigned int sc_isp_bus_fe0_idleack          :  1;
        unsigned int sc_isp_bus_fe1_idle             :  1;
        unsigned int sc_isp_bus_fe1_idleack          :  1;
        unsigned int sc_isp_bus_fe2_idle             :  1;
        unsigned int sc_isp_bus_fe2_idleack          :  1;
        unsigned int sc_isp_bus_fe3_idle             :  1;
        unsigned int sc_isp_bus_fe3_idleack          :  1;
        unsigned int sc_isp_bus_be0_idle             :  1;
        unsigned int sc_isp_bus_be0_idleack          :  1;
        unsigned int sc_isp_bus_be1_idle             :  1;
        unsigned int sc_isp_bus_be1_idleack          :  1;
        unsigned int sc_isp_bus_pe_idle              :  1;
        unsigned int sc_isp_bus_pe_idleack           :  1;
        unsigned int sc_isp_bus_cve_idle             :  1;
        unsigned int sc_isp_bus_cve_idleack          :  1;
        unsigned int sc_isp_bus_dma_idle             :  1;
        unsigned int sc_isp_bus_dma_idleack          :  1;
        unsigned int sc_isp_bus_cdma_cfg_idle        :  1;
        unsigned int sc_isp_bus_cdma_cfg_idleack     :  1;
        unsigned int sc_isp_bus_cdma_data_idle       :  1;
        unsigned int sc_isp_bus_cdma_data_idleack    :  1;
        unsigned int sc_isp_bus_to_mcu_cfg_idle      :  1;
        unsigned int sc_isp_bus_to_mcu_cfg_idleack   :  1;
        unsigned int sc_isp_bus_mcu_data_idle        :  1;
        unsigned int sc_isp_bus_mcu_data_idleack     :  1;
        unsigned int sc_isp_bus_from_mcu_cfg_idle    :  1;
        unsigned int sc_isp_bus_from_mcu_cfg_idleack :  1;
        unsigned int reserved_0                      :  4;
    } reg;
}media2_ctrl_isp_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_isp_bus_timeout_en :  1;
        unsigned int reserved_0            : 15;
        unsigned int _bm_                  : 16;
    } reg;
}media2_ctrl_isp_bus_status_timeout_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_isp_bus_tgt_timeout : 10;
        unsigned int reserved_0             : 22;
    } reg;
}media2_ctrl_isp_bus_status_timeout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_isp_bus_tgt_no_pending_trans : 10;
        unsigned int reserved_0                      :  6;
        unsigned int sc_isp_bus_ini_no_pending_trans : 14;
        unsigned int reserved_1                      :  2;
    } reg;
}media2_ctrl_isp_bus_status_nopending_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_venc_bus_m2_idlereq :  1;
        unsigned int sc_venc_bus_idlereq    :  1;
        unsigned int reserved_0             : 14;
        unsigned int _bm_                   : 16;
    } reg;
}media2_ctrl_venc_bus_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_venc_bus_m2_idle    :  1;
        unsigned int sc_venc_bus_m2_idleack :  1;
        unsigned int sc_venc_bus_idle       :  1;
        unsigned int sc_venc_bus_idleack    :  1;
        unsigned int reserved_0             : 28;
    } reg;
}media2_ctrl_venc_bus_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_venc_bus_timeout_en :  1;
        unsigned int reserved_0             : 15;
        unsigned int _bm_                   : 16;
    } reg;
}media2_ctrl_venc_bus_status_timeout_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_venc_bus_tgt_timeout :  3;
        unsigned int reserved_0              : 29;
    } reg;
}media2_ctrl_venc_bus_status_timeout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_venc_bus_tgt_no_pending_trans :  3;
        unsigned int reserved_0                       : 13;
        unsigned int sc_venc_bus_ini_no_pending_trans :  2;
        unsigned int reserved_1                       : 14;
    } reg;
}media2_ctrl_venc_bus_status_nopending_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_fw_clear     :  1;
        unsigned int ns_fw_perror    :  1;
        unsigned int reserved_0      :  6;
        unsigned int ns_fw_fail_pmid :  8;
        unsigned int reserved_1      : 16;
    } reg;
}media2_ctrl_ns_firewall_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_fw_fail_paddr : 32;
    } reg;
}media2_ctrl_ns_firewall_fail_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_media2_sel_tcu_bist :  1;
        unsigned int reserved_0             : 31;
    } reg;
}media2_ctrl_bist_sel_st_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}media2_ctrl_regfile_cgbypass_t;

#endif
