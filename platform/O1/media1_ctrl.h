// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef MEDIA1_CTRL_REGIF_H
#define MEDIA1_CTRL_REGIF_H

#define MEDIA1_CTRL_M1_BUS_IDLE_REQ                       0x0000
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_VDEC_IDLEREQ_SHIFT                   0
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_VDEC_IDLEREQ_MASK                    0x00000001
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_DISP_IDLEREQ_SHIFT                   1
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_DISP_IDLEREQ_MASK                    0x00000002
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_VDEC_BUS_IDLEREQ_SHIFT                          2
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_VDEC_BUS_IDLEREQ_MASK                           0x00000004
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_VEU_IDLEREQ_SHIFT                      3
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_VEU_IDLEREQ_MASK                       0x00000008
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_DPU3_IDLEREQ_SHIFT                     5
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_DPU3_IDLEREQ_MASK                      0x00000020
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_DPU2_IDLEREQ_SHIFT                     6
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_DPU2_IDLEREQ_MASK                      0x00000040
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_DPU014_IDLEREQ_SHIFT                   8
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_DISP_BUS_DPU014_IDLEREQ_MASK                    0x00000100
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_NPU_IDLEREQ_SHIFT                    9
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_NPU_IDLEREQ_MASK                     0x00000200
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_MAIN_IDLEREQ_SHIFT                   10
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_MAIN_IDLEREQ_MASK                    0x00000400
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_TCU_IDLEREQ_SHIFT                    11
#define MEDIA1_CTRL_M1_BUS_IDLE_REQ_MEDIA1_BUS_TCU_IDLEREQ_MASK                     0x00000800
#define MEDIA1_CTRL_M1_BUS_IDLE                           0x0004
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_VDEC_IDLEACK_SHIFT                       0
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_VDEC_IDLEACK_MASK                        0x00000001
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_VDEC_IDLE_SHIFT                          1
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_VDEC_IDLE_MASK                           0x00000002
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_DISP_IDLEACK_SHIFT                       2
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_DISP_IDLEACK_MASK                        0x00000004
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_DISP_IDLE_SHIFT                          3
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_DISP_IDLE_MASK                           0x00000008
#define MEDIA1_CTRL_M1_BUS_IDLE_VDEC_BUS_IDLEACK_SHIFT                              4
#define MEDIA1_CTRL_M1_BUS_IDLE_VDEC_BUS_IDLEACK_MASK                               0x00000010
#define MEDIA1_CTRL_M1_BUS_IDLE_VDEC_BUS_IDLE_SHIFT                                 5
#define MEDIA1_CTRL_M1_BUS_IDLE_VDEC_BUS_IDLE_MASK                                  0x00000020
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_VEU_IDLEACK_SHIFT                          6
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_VEU_IDLEACK_MASK                           0x00000040
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_VEU_IDLE_SHIFT                             7
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_VEU_IDLE_MASK                              0x00000080
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU3_IDLEACK_SHIFT                         10
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU3_IDLEACK_MASK                          0x00000400
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU3_IDLE_SHIFT                            11
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU3_IDLE_MASK                             0x00000800
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU2_IDLEACK_SHIFT                         12
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU2_IDLEACK_MASK                          0x00001000
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU2_IDLE_SHIFT                            13
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU2_IDLE_MASK                             0x00002000
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU014_IDLEACK_SHIFT                       16
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU014_IDLEACK_MASK                        0x00010000
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU014_IDLE_SHIFT                          17
#define MEDIA1_CTRL_M1_BUS_IDLE_DISP_BUS_DPU014_IDLE_MASK                           0x00020000
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_NPU_IDLEACK_SHIFT                        18
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_NPU_IDLEACK_MASK                         0x00040000
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_NPU_IDLE_SHIFT                           19
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_NPU_IDLE_MASK                            0x00080000
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_MAIN_IDLEACK_SHIFT                       20
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_MAIN_IDLEACK_MASK                        0x00100000
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_MAIN_IDLE_SHIFT                          21
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_MAIN_IDLE_MASK                           0x00200000
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_TCU_IDLEACK_SHIFT                        22
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_TCU_IDLEACK_MASK                         0x00400000
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_TCU_IDLE_SHIFT                           23
#define MEDIA1_CTRL_M1_BUS_IDLE_MEDIA1_BUS_TCU_IDLE_MASK                            0x00800000
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN                     0x0010
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN_MEDIA1_BUS_TIMEOUT_EN_SHIFT                   0
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN_MEDIA1_BUS_TIMEOUT_EN_MASK                    0x00000001
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN_VDEC_BUS_TIMEOUT_EN_SHIFT                     1
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN_VDEC_BUS_TIMEOUT_EN_MASK                      0x00000002
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN_DISP_BUS_TIMEOUT_EN_SHIFT                     2
#define MEDIA1_CTRL_M1_BUS_TIMEOUT_EN_DISP_BUS_TIMEOUT_EN_MASK                      0x00000004
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT                    0x0014
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT_MEDIA1_BUS_TGT_TIMEOUT_SHIFT                 0
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT_MEDIA1_BUS_TGT_TIMEOUT_MASK                  0x0000007f
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT_DISP_BUS_TGT_TIMEOUT_SHIFT                   8
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT_DISP_BUS_TGT_TIMEOUT_MASK                    0x00000f00
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT_VDEC_BUS_TGT_TIMEOUT_SHIFT                   12
#define MEDIA1_CTRL_M1_BUS_TGT_TIMEOUT_VDEC_BUS_TGT_TIMEOUT_MASK                    0x00007000
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS            0x0018
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS_MEDIA1_BUS_INI_NO_PENDING_TRANS_SHIFT 0
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS_MEDIA1_BUS_INI_NO_PENDING_TRANS_MASK  0x0000001f
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS_VDEC_BUS_INI_NO_PENDING_TRANS_SHIFT  6
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS_VDEC_BUS_INI_NO_PENDING_TRANS_MASK   0x000000c0
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS_DISP_BUS_INI_NO_PENDING_TRANS_SHIFT  8
#define MEDIA1_CTRL_M1_BUS_INI_NOPENDING_TRANS_DISP_BUS_INI_NO_PENDING_TRANS_MASK   0x00007f00
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS            0x001c
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS_MEDIA1_BUS_TGT_NO_PENDING_TRANS_SHIFT 0
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS_MEDIA1_BUS_TGT_NO_PENDING_TRANS_MASK  0x0000007f
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS_VDEC_BUS_TGT_NO_PENDING_TRANS_SHIFT  8
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS_VDEC_BUS_TGT_NO_PENDING_TRANS_MASK   0x00000700
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS_DISP_BUS_TGT_NO_PENDING_TRANS_SHIFT  12
#define MEDIA1_CTRL_M1_BUS_TGT_NOPENDING_TRANS_DISP_BUS_TGT_NO_PENDING_TRANS_MASK   0x0000f000
#define MEDIA1_CTRL_M1_NDB_SLV_CFG                        0x0100
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_SLV_AUTOGATE_BYP_SHIFT                    0
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_SLV_AUTOGATE_BYP_MASK                     0x00000001
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_R_SLV_OST_ZERO_FORCE_SHIFT                1
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_R_SLV_OST_ZERO_FORCE_MASK                 0x00000002
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_R_SLV_PWR_REQ_SHIFT                       2
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_R_SLV_PWR_REQ_MASK                        0x00000004
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_W_SLV_OST_ZERO_FORCE_SHIFT                3
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_W_SLV_OST_ZERO_FORCE_MASK                 0x00000008
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_W_SLV_PWR_REQ_SHIFT                       4
#define MEDIA1_CTRL_M1_NDB_SLV_CFG_SC_NDB_W_SLV_PWR_REQ_MASK                        0x00000010
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE                     0x0104
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_R_SLV_BUSY_0_SHIFT                     0
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_R_SLV_BUSY_0_MASK                      0x00000001
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_R_SLV_PWREQ_ACCEPT_0_SHIFT             1
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_R_SLV_PWREQ_ACCEPT_0_MASK              0x00000002
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_W_SLV_BUSY_0_SHIFT                     2
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_W_SLV_BUSY_0_MASK                      0x00000004
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_W_SLV_PWREQ_ACCEPT_0_SHIFT             3
#define MEDIA1_CTRL_M1_NDB_SLV0_STATE_SC_NDB_W_SLV_PWREQ_ACCEPT_0_MASK              0x00000008
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE                     0x0108
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_R_SLV_BUSY_1_SHIFT                     0
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_R_SLV_BUSY_1_MASK                      0x00000001
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_R_SLV_PWREQ_ACCEPT_1_SHIFT             1
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_R_SLV_PWREQ_ACCEPT_1_MASK              0x00000002
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_W_SLV_BUSY_1_SHIFT                     2
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_W_SLV_BUSY_1_MASK                      0x00000004
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_W_SLV_PWREQ_ACCEPT_1_SHIFT             3
#define MEDIA1_CTRL_M1_NDB_SLV1_STATE_SC_NDB_W_SLV_PWREQ_ACCEPT_1_MASK              0x00000008
#define MEDIA1_CTRL_M1_NDB_MST_CFG                        0x010c
#define MEDIA1_CTRL_M1_NDB_MST_CFG_SC_NDB_MST0_AUTOGATE_BYP_SHIFT                   0
#define MEDIA1_CTRL_M1_NDB_MST_CFG_SC_NDB_MST0_AUTOGATE_BYP_MASK                    0x00000001
#define MEDIA1_CTRL_M1_NDB_MST_CFG_SC_NDB_MST1_AUTOGATE_BYP_SHIFT                   1
#define MEDIA1_CTRL_M1_NDB_MST_CFG_SC_NDB_MST1_AUTOGATE_BYP_MASK                    0x00000002
#define MEDIA1_CTRL_M1_NDB_MST_STATE                      0x0110
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_R_MST0_BUSY_SHIFT                       0
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_R_MST0_BUSY_MASK                        0x00000001
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_W_MST0_BUSY_SHIFT                       1
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_W_MST0_BUSY_MASK                        0x00000002
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_R_MST1_BUSY_SHIFT                       2
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_R_MST1_BUSY_MASK                        0x00000004
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_W_MST1_BUSY_SHIFT                       3
#define MEDIA1_CTRL_M1_NDB_MST_STATE_SC_NDB_W_MST1_BUSY_MASK                        0x00000008
#define MEDIA1_CTRL_M1_NDB_SLV_DFX                        0x0114
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB0_SLV_R_DFX_SHIFT                             0
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB0_SLV_R_DFX_MASK                              0x0000001f
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB0_SLV_W_DFX_SHIFT                             8
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB0_SLV_W_DFX_MASK                              0x00001f00
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB1_SLV_R_DFX_SHIFT                             16
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB1_SLV_R_DFX_MASK                              0x001f0000
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB1_SLV_W_DFX_SHIFT                             24
#define MEDIA1_CTRL_M1_NDB_SLV_DFX_NDB1_SLV_W_DFX_MASK                              0x1f000000
#define MEDIA1_CTRL_M1_NDB_MST_DFX                        0x0118
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB0_MST_R_DFX_SHIFT                             0
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB0_MST_R_DFX_MASK                              0x0000001f
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB0_MST_W_DFX_SHIFT                             8
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB0_MST_W_DFX_MASK                              0x00001f00
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB1_MST_R_DFX_SHIFT                             16
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB1_MST_R_DFX_MASK                              0x001f0000
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB1_MST_W_DFX_SHIFT                             24
#define MEDIA1_CTRL_M1_NDB_MST_DFX_NDB1_MST_W_DFX_MASK                              0x1f000000
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_CTRL                  0x0120
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQ_BYPASS_SHIFT            0
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQ_BYPASS_MASK             0x00000001
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQREQN_SOFT_SHIFT          1
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_CTRL_SC_ADB400_SLV_CLKQREQN_SOFT_MASK           0x00000002
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS                0x0124
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACCEPTN_SHIFT         0
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACCEPTN_MASK          0x00000001
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_DENY_SHIFT            1
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_DENY_MASK             0x00000002
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACTIVE_SHIFT          2
#define MEDIA1_CTRL_ADB400_SLV_CLKQ_STATUS_SC_ADB400_SLV_CLKQ_ACTIVE_MASK           0x00000004
#define MEDIA1_CTRL_ADB400_SLV_PW_CTRL                    0x0128
#define MEDIA1_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_REQN_SHIFT                0
#define MEDIA1_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_REQN_MASK                 0x00000001
#define MEDIA1_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_PERMIT_DENY_SAR_SHIFT     1
#define MEDIA1_CTRL_ADB400_SLV_PW_CTRL_SC_ADB400_SLV_PWRQ_PERMIT_DENY_SAR_MASK      0x00000002
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS                  0x012c
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_SHIFT           0
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACCEPTN_MASK            0x00000001
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_DENY_SHIFT              1
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_DENY_MASK               0x00000002
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACTIVE_SHIFT            2
#define MEDIA1_CTRL_ADB400_SLV_PW_STATUS_SC_ADB400_SLV_PWRQ_ACTIVE_MASK             0x00000004
#define MEDIA1_CTRL_ADB400_SLV_WAKEUP_CTRL                0x0130
#define MEDIA1_CTRL_ADB400_SLV_WAKEUP_CTRL_SC_ADB400_SLV_WAKEUP_SHIFT               0
#define MEDIA1_CTRL_ADB400_SLV_WAKEUP_CTRL_SC_ADB400_SLV_WAKEUP_MASK                0x00000001
#define MEDIA1_CTRL_ADB400_SLV_QSM_DELAY                  0x0134
#define MEDIA1_CTRL_ADB400_SLV_QSM_DELAY_SC_ADB400_SLV_QSM_DELAY_BYPASS_SHIFT       0
#define MEDIA1_CTRL_ADB400_SLV_QSM_DELAY_SC_ADB400_SLV_QSM_DELAY_BYPASS_MASK        0x00000001
#define MEDIA1_CTRL_ADB400_SLV_QSM_DELAY_SC_ADB400_SLV_QSM_DELAY_CNT_SHIFT          8
#define MEDIA1_CTRL_ADB400_SLV_QSM_DELAY_SC_ADB400_SLV_QSM_DELAY_CNT_MASK           0x0000ff00
#define MEDIA1_CTRL_ADB400_SLV_QSM_IDLE                   0x0138
#define MEDIA1_CTRL_ADB400_SLV_QSM_IDLE_SC_ADB400_SLV_QSMIDLE_SHIFT                 0
#define MEDIA1_CTRL_ADB400_SLV_QSM_IDLE_SC_ADB400_SLV_QSMIDLE_MASK                  0x00000001
#define MEDIA1_CTRL_ADB400_SLV_QSM_IDLE_ADB400_SLV_CLKQ_REQN_SHIFT                  1
#define MEDIA1_CTRL_ADB400_SLV_QSM_IDLE_ADB400_SLV_CLKQ_REQN_MASK                   0x00000002
#define MEDIA1_CTRL_ADB400_MST_CLKQ_CTRL                  0x013c
#define MEDIA1_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQ_BYPASS_SHIFT            0
#define MEDIA1_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQ_BYPASS_MASK             0x00000001
#define MEDIA1_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQREQN_SOFT_SHIFT          1
#define MEDIA1_CTRL_ADB400_MST_CLKQ_CTRL_SC_ADB400_MST_CLKQREQN_SOFT_MASK           0x00000002
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS                0x0140
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACCEPTN_SHIFT         0
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACCEPTN_MASK          0x00000001
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_DENY_SHIFT            1
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_DENY_MASK             0x00000002
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACTIVE_SHIFT          2
#define MEDIA1_CTRL_ADB400_MST_CLKQ_STATUS_SC_ADB400_MST_CLKQ_ACTIVE_MASK           0x00000004
#define MEDIA1_CTRL_ADB400_GT_CTRL                        0x0144
#define MEDIA1_CTRL_ADB400_GT_CTRL_SC_ADB400_SLV_GT_BYPASS_SHIFT                    0
#define MEDIA1_CTRL_ADB400_GT_CTRL_SC_ADB400_SLV_GT_BYPASS_MASK                     0x00000001
#define MEDIA1_CTRL_ADB400_GT_CTRL_SC_ADB400_MST_GT_BYPASS_SHIFT                    1
#define MEDIA1_CTRL_ADB400_GT_CTRL_SC_ADB400_MST_GT_BYPASS_MASK                     0x00000002
#define MEDIA1_CTRL_ADB400_GT_STATUS                      0x0148
#define MEDIA1_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_SLV_SHIFT                  0
#define MEDIA1_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_SLV_MASK                   0x00000001
#define MEDIA1_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_MST_SHIFT                  1
#define MEDIA1_CTRL_ADB400_GT_STATUS_ST_ACLK_ADB400_DATA_MST_MASK                   0x00000002
#define MEDIA1_CTRL_ADB400_MST_QSM_DELAY                  0x014c
#define MEDIA1_CTRL_ADB400_MST_QSM_DELAY_SC_ADB400_MST_QSM_DELAY_BYPASS_SHIFT       0
#define MEDIA1_CTRL_ADB400_MST_QSM_DELAY_SC_ADB400_MST_QSM_DELAY_BYPASS_MASK        0x00000001
#define MEDIA1_CTRL_ADB400_MST_QSM_DELAY_SC_ADB400_MST_QSM_DELAY_CNT_SHIFT          8
#define MEDIA1_CTRL_ADB400_MST_QSM_DELAY_SC_ADB400_MST_QSM_DELAY_CNT_MASK           0x0000ff00
#define MEDIA1_CTRL_ADB400_MST_QSM_IDLE                   0x0150
#define MEDIA1_CTRL_ADB400_MST_QSM_IDLE_SC_ADB400_MST_QSMIDLE_SHIFT                 0
#define MEDIA1_CTRL_ADB400_MST_QSM_IDLE_SC_ADB400_MST_QSMIDLE_MASK                  0x00000001
#define MEDIA1_CTRL_ADB400_MST_QSM_IDLE_ADB400_MST_CLKQ_REQN_SHIFT                  1
#define MEDIA1_CTRL_ADB400_MST_QSM_IDLE_ADB400_MST_CLKQ_REQN_MASK                   0x00000002
#define MEDIA1_CTRL_MEM_CTRL_BUS_LOW                      0x0800
#define MEDIA1_CTRL_MEM_CTRL_BUS_MID                      0x0804
#define MEDIA1_CTRL_MEM_CTRL_BUS_HIGH                     0x0808
#define MEDIA1_CTRL_M1_BISR                               0x0c04
#define MEDIA1_CTRL_M1_BISR_MM1_BISR_CTRL_SHIFT                                     0
#define MEDIA1_CTRL_M1_BISR_MM1_BISR_CTRL_MASK                                      0x00000001
#define MEDIA1_CTRL_M1_NS_FIRWALL_ADDR                    0x0f00
#define MEDIA1_CTRL_M1_NS_FIRWALL_ERROR                   0x0f04
#define MEDIA1_CTRL_M1_NS_FIRWALL_ERROR_NS_FW_ERROR_SHIFT                           0
#define MEDIA1_CTRL_M1_NS_FIRWALL_ERROR_NS_FW_ERROR_MASK                            0x00000001
#define MEDIA1_CTRL_M1_NS_FIRWALL_PMID                    0x0f08
#define MEDIA1_CTRL_M1_NS_FIRWALL_PMID_NS_FW_FAIL_PMID_SHIFT                        0
#define MEDIA1_CTRL_M1_NS_FIRWALL_PMID_NS_FW_FAIL_PMID_MASK                         0x000000ff
#define MEDIA1_CTRL_M1_NS_FIRWALL_CLEAR                   0x0f0c
#define MEDIA1_CTRL_M1_NS_FIRWALL_CLEAR_NS_FW_CLEAR_SHIFT                           0
#define MEDIA1_CTRL_REGFILE_CGBYPASS                      0x0f10
#define MEDIA1_CTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_SHIFT                         0
#define MEDIA1_CTRL_REGFILE_CGBYPASS_REGFILE_CGBYPASS_MASK                          0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_bus_vdec_idlereq :  1;
        unsigned int media1_bus_disp_idlereq :  1;
        unsigned int vdec_bus_idlereq        :  1;
        unsigned int disp_bus_veu_idlereq    :  1;
        unsigned int reserved_0              :  1;
        unsigned int disp_bus_dpu3_idlereq   :  1;
        unsigned int disp_bus_dpu2_idlereq   :  1;
        unsigned int reserved_1              :  1;
        unsigned int disp_bus_dpu014_idlereq :  1;
        unsigned int media1_bus_npu_idlereq  :  1;
        unsigned int media1_bus_main_idlereq :  1;
        unsigned int media1_bus_tcu_idlereq  :  1;
        unsigned int reserved_2              :  4;
        unsigned int _bm_                    : 16;
    } reg;
}media1_ctrl_m1_bus_idle_req_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_bus_vdec_idleack :  1;
        unsigned int media1_bus_vdec_idle    :  1;
        unsigned int media1_bus_disp_idleack :  1;
        unsigned int media1_bus_disp_idle    :  1;
        unsigned int vdec_bus_idleack        :  1;
        unsigned int vdec_bus_idle           :  1;
        unsigned int disp_bus_veu_idleack    :  1;
        unsigned int disp_bus_veu_idle       :  1;
        unsigned int reserved_0              :  2;
        unsigned int disp_bus_dpu3_idleack   :  1;
        unsigned int disp_bus_dpu3_idle      :  1;
        unsigned int disp_bus_dpu2_idleack   :  1;
        unsigned int disp_bus_dpu2_idle      :  1;
        unsigned int reserved_1              :  2;
        unsigned int disp_bus_dpu014_idleack :  1;
        unsigned int disp_bus_dpu014_idle    :  1;
        unsigned int media1_bus_npu_idleack  :  1;
        unsigned int media1_bus_npu_idle     :  1;
        unsigned int media1_bus_main_idleack :  1;
        unsigned int media1_bus_main_idle    :  1;
        unsigned int media1_bus_tcu_idleack  :  1;
        unsigned int media1_bus_tcu_idle     :  1;
        unsigned int reserved_2              :  8;
    } reg;
}media1_ctrl_m1_bus_idle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_bus_timeout_en :  1;
        unsigned int vdec_bus_timeout_en   :  1;
        unsigned int disp_bus_timeout_en   :  1;
        unsigned int reserved_0            : 13;
        unsigned int _bm_                  : 16;
    } reg;
}media1_ctrl_m1_bus_timeout_en_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_bus_tgt_timeout :  7;
        unsigned int reserved_0             :  1;
        unsigned int disp_bus_tgt_timeout   :  4;
        unsigned int vdec_bus_tgt_timeout   :  3;
        unsigned int reserved_1             : 17;
    } reg;
}media1_ctrl_m1_bus_tgt_timeout_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_bus_ini_no_pending_trans :  5;
        unsigned int reserved_0                      :  1;
        unsigned int vdec_bus_ini_no_pending_trans   :  2;
        unsigned int disp_bus_ini_no_pending_trans   :  7;
        unsigned int reserved_1                      : 17;
    } reg;
}media1_ctrl_m1_bus_ini_nopending_trans_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_bus_tgt_no_pending_trans :  7;
        unsigned int reserved_0                      :  1;
        unsigned int vdec_bus_tgt_no_pending_trans   :  3;
        unsigned int reserved_1                      :  1;
        unsigned int disp_bus_tgt_no_pending_trans   :  4;
        unsigned int reserved_2                      : 16;
    } reg;
}media1_ctrl_m1_bus_tgt_nopending_trans_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_slv_autogate_byp     :  1;
        unsigned int sc_ndb_r_slv_ost_zero_force :  1;
        unsigned int sc_ndb_r_slv_pwr_req        :  1;
        unsigned int sc_ndb_w_slv_ost_zero_force :  1;
        unsigned int sc_ndb_w_slv_pwr_req        :  1;
        unsigned int reserved_0                  : 11;
        unsigned int _bm_                        : 16;
    } reg;
}media1_ctrl_m1_ndb_slv_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_r_slv_busy_0         :  1;
        unsigned int sc_ndb_r_slv_pwreq_accept_0 :  1;
        unsigned int sc_ndb_w_slv_busy_0         :  1;
        unsigned int sc_ndb_w_slv_pwreq_accept_0 :  1;
        unsigned int reserved_0                  : 28;
    } reg;
}media1_ctrl_m1_ndb_slv0_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_r_slv_busy_1         :  1;
        unsigned int sc_ndb_r_slv_pwreq_accept_1 :  1;
        unsigned int sc_ndb_w_slv_busy_1         :  1;
        unsigned int sc_ndb_w_slv_pwreq_accept_1 :  1;
        unsigned int reserved_0                  : 28;
    } reg;
}media1_ctrl_m1_ndb_slv1_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_mst0_autogate_byp :  1;
        unsigned int sc_ndb_mst1_autogate_byp :  1;
        unsigned int reserved_0               : 14;
        unsigned int _bm_                     : 16;
    } reg;
}media1_ctrl_m1_ndb_mst_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_ndb_r_mst0_busy :  1;
        unsigned int sc_ndb_w_mst0_busy :  1;
        unsigned int sc_ndb_r_mst1_busy :  1;
        unsigned int sc_ndb_w_mst1_busy :  1;
        unsigned int reserved_0         : 28;
    } reg;
}media1_ctrl_m1_ndb_mst_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ndb0_slv_r_dfx :  5;
        unsigned int reserved_0     :  3;
        unsigned int ndb0_slv_w_dfx :  5;
        unsigned int reserved_1     :  3;
        unsigned int ndb1_slv_r_dfx :  5;
        unsigned int reserved_2     :  3;
        unsigned int ndb1_slv_w_dfx :  5;
        unsigned int reserved_3     :  3;
    } reg;
}media1_ctrl_m1_ndb_slv_dfx_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ndb0_mst_r_dfx :  5;
        unsigned int reserved_0     :  3;
        unsigned int ndb0_mst_w_dfx :  5;
        unsigned int reserved_1     :  3;
        unsigned int ndb1_mst_r_dfx :  5;
        unsigned int reserved_2     :  3;
        unsigned int ndb1_mst_w_dfx :  5;
        unsigned int reserved_3     :  3;
    } reg;
}media1_ctrl_m1_ndb_mst_dfx_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_clkq_bypass   :  1;
        unsigned int sc_adb400_slv_clkqreqn_soft :  1;
        unsigned int reserved_0                  : 14;
        unsigned int _bm_                        : 16;
    } reg;
}media1_ctrl_adb400_slv_clkq_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_clkq_acceptn :  1;
        unsigned int sc_adb400_slv_clkq_deny    :  1;
        unsigned int sc_adb400_slv_clkq_active  :  1;
        unsigned int reserved_0                 : 29;
    } reg;
}media1_ctrl_adb400_slv_clkq_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_pwrq_reqn            :  1;
        unsigned int sc_adb400_slv_pwrq_permit_deny_sar :  1;
        unsigned int reserved_0                         : 14;
        unsigned int _bm_                               : 16;
    } reg;
}media1_ctrl_adb400_slv_pw_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_pwrq_acceptn :  1;
        unsigned int sc_adb400_slv_pwrq_deny    :  1;
        unsigned int sc_adb400_slv_pwrq_active  :  1;
        unsigned int reserved_0                 : 29;
    } reg;
}media1_ctrl_adb400_slv_pw_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_wakeup :  1;
        unsigned int reserved_0           : 31;
    } reg;
}media1_ctrl_adb400_slv_wakeup_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_qsm_delay_bypass :  1;
        unsigned int reserved_0                     :  7;
        unsigned int sc_adb400_slv_qsm_delay_cnt    :  8;
        unsigned int reserved_1                     : 16;
    } reg;
}media1_ctrl_adb400_slv_qsm_delay_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_qsmidle :  1;
        unsigned int adb400_slv_clkq_reqn  :  1;
        unsigned int reserved_0            : 30;
    } reg;
}media1_ctrl_adb400_slv_qsm_idle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_mst_clkq_bypass   :  1;
        unsigned int sc_adb400_mst_clkqreqn_soft :  1;
        unsigned int reserved_0                  : 14;
        unsigned int _bm_                        : 16;
    } reg;
}media1_ctrl_adb400_mst_clkq_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_mst_clkq_acceptn :  1;
        unsigned int sc_adb400_mst_clkq_deny    :  1;
        unsigned int sc_adb400_mst_clkq_active  :  1;
        unsigned int reserved_0                 : 29;
    } reg;
}media1_ctrl_adb400_mst_clkq_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_slv_gt_bypass :  1;
        unsigned int sc_adb400_mst_gt_bypass :  1;
        unsigned int reserved_0              : 14;
        unsigned int _bm_                    : 16;
    } reg;
}media1_ctrl_adb400_gt_ctrl_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int st_aclk_adb400_data_slv :  1;
        unsigned int st_aclk_adb400_data_mst :  1;
        unsigned int reserved_0              : 30;
    } reg;
}media1_ctrl_adb400_gt_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_mst_qsm_delay_bypass :  1;
        unsigned int reserved_0                     :  7;
        unsigned int sc_adb400_mst_qsm_delay_cnt    :  8;
        unsigned int reserved_1                     : 16;
    } reg;
}media1_ctrl_adb400_mst_qsm_delay_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sc_adb400_mst_qsmidle :  1;
        unsigned int adb400_mst_clkq_reqn  :  1;
        unsigned int reserved_0            : 30;
    } reg;
}media1_ctrl_adb400_mst_qsm_idle_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_mem_ctrl_bus_low : 32;
    } reg;
}media1_ctrl_mem_ctrl_bus_low_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_mem_ctrl_bus_mid : 32;
    } reg;
}media1_ctrl_mem_ctrl_bus_mid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int media1_mem_ctrl_bus_high : 32;
    } reg;
}media1_ctrl_mem_ctrl_bus_high_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int mm1_bisr_ctrl :  1;
        unsigned int reserved_0    : 31;
    } reg;
}media1_ctrl_m1_bisr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_fw_fail_paddr : 32;
    } reg;
}media1_ctrl_m1_ns_firwall_addr_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_fw_error :  1;
        unsigned int reserved_0  : 31;
    } reg;
}media1_ctrl_m1_ns_firwall_error_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_fw_fail_pmid :  8;
        unsigned int reserved_0      : 24;
    } reg;
}media1_ctrl_m1_ns_firwall_pmid_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int ns_fw_clear :  1;
        unsigned int reserved_0  : 31;
    } reg;
}media1_ctrl_m1_ns_firwall_clear_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int regfile_cgbypass :  1;
        unsigned int reserved_0       : 31;
    } reg;
}media1_ctrl_regfile_cgbypass_t;

#endif
