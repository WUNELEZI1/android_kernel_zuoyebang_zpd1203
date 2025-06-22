// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef DVS_REGIF_H
#define DVS_REGIF_H

#define DVS_VOLT_REQ_0                         0x0000
#define DVS_VOLT_REQ_0_SW_VOLT_REQ_0_SHIFT                                  0
#define DVS_VOLT_REQ_0_SW_VOLT_REQ_0_MASK                                   0x000000ff
#define DVS_VOLT_REQ_0_SW_VOLT_REQ_VALID_0_SHIFT                            8
#define DVS_VOLT_REQ_0_SW_VOLT_REQ_VALID_0_MASK                             0x00000100
#define DVS_VOLT_REQ_1                         0x0004
#define DVS_VOLT_REQ_1_SW_VOLT_REQ_1_SHIFT                                  0
#define DVS_VOLT_REQ_1_SW_VOLT_REQ_1_MASK                                   0x000000ff
#define DVS_VOLT_REQ_1_SW_VOLT_REQ_VALID_1_SHIFT                            8
#define DVS_VOLT_REQ_1_SW_VOLT_REQ_VALID_1_MASK                             0x00000100
#define DVS_VOLT_REQ_2                         0x0008
#define DVS_VOLT_REQ_2_SW_VOLT_REQ_2_SHIFT                                  0
#define DVS_VOLT_REQ_2_SW_VOLT_REQ_2_MASK                                   0x000000ff
#define DVS_VOLT_REQ_2_SW_VOLT_REQ_VALID_2_SHIFT                            8
#define DVS_VOLT_REQ_2_SW_VOLT_REQ_VALID_2_MASK                             0x00000100
#define DVS_VOLT_REQ_3                         0x000c
#define DVS_VOLT_REQ_3_SW_VOLT_REQ_3_SHIFT                                  0
#define DVS_VOLT_REQ_3_SW_VOLT_REQ_3_MASK                                   0x000000ff
#define DVS_VOLT_REQ_3_SW_VOLT_REQ_VALID_3_SHIFT                            8
#define DVS_VOLT_REQ_3_SW_VOLT_REQ_VALID_3_MASK                             0x00000100
#define DVS_VOLT_REQ_4                         0x0010
#define DVS_VOLT_REQ_4_SW_VOLT_REQ_4_SHIFT                                  0
#define DVS_VOLT_REQ_4_SW_VOLT_REQ_4_MASK                                   0x000000ff
#define DVS_VOLT_REQ_4_SW_VOLT_REQ_VALID_4_SHIFT                            8
#define DVS_VOLT_REQ_4_SW_VOLT_REQ_VALID_4_MASK                             0x00000100
#define DVS_VOLT_REQ_5                         0x0014
#define DVS_VOLT_REQ_5_SW_VOLT_REQ_5_SHIFT                                  0
#define DVS_VOLT_REQ_5_SW_VOLT_REQ_5_MASK                                   0x000000ff
#define DVS_VOLT_REQ_5_SW_VOLT_REQ_VALID_5_SHIFT                            8
#define DVS_VOLT_REQ_5_SW_VOLT_REQ_VALID_5_MASK                             0x00000100
#define DVS_VOLT_REQ_6                         0x0018
#define DVS_VOLT_REQ_6_SW_VOLT_REQ_6_SHIFT                                  0
#define DVS_VOLT_REQ_6_SW_VOLT_REQ_6_MASK                                   0x000000ff
#define DVS_VOLT_REQ_6_SW_VOLT_REQ_VALID_6_SHIFT                            8
#define DVS_VOLT_REQ_6_SW_VOLT_REQ_VALID_6_MASK                             0x00000100
#define DVS_VOLT_REQ_7                         0x001c
#define DVS_VOLT_REQ_7_SW_VOLT_REQ_7_SHIFT                                  0
#define DVS_VOLT_REQ_7_SW_VOLT_REQ_7_MASK                                   0x000000ff
#define DVS_VOLT_REQ_7_SW_VOLT_REQ_VALID_7_SHIFT                            8
#define DVS_VOLT_REQ_7_SW_VOLT_REQ_VALID_7_MASK                             0x00000100
#define DVS_HW_OFFSET_0                        0x0030
#define DVS_HW_OFFSET_0_HW_OFFSET_0_SHIFT                                   0
#define DVS_HW_OFFSET_0_HW_OFFSET_0_MASK                                    0x000001ff
#define DVS_HW_OFFSET_1                        0x0034
#define DVS_HW_OFFSET_1_HW_OFFSET_1_SHIFT                                   0
#define DVS_HW_OFFSET_1_HW_OFFSET_1_MASK                                    0x000001ff
#define DVS_HW_OFFSET_2                        0x0038
#define DVS_HW_OFFSET_2_HW_OFFSET_2_SHIFT                                   0
#define DVS_HW_OFFSET_2_HW_OFFSET_2_MASK                                    0x000001ff
#define DVS_SW_OFFSET_0                        0x003c
#define DVS_SW_OFFSET_0_SW_OFFSET_0_SHIFT                                   0
#define DVS_SW_OFFSET_0_SW_OFFSET_0_MASK                                    0x000001ff
#define DVS_SW_OFFSET_1                        0x0040
#define DVS_SW_OFFSET_1_SW_OFFSET_1_SHIFT                                   0
#define DVS_SW_OFFSET_1_SW_OFFSET_1_MASK                                    0x000001ff
#define DVS_SW_OFFSET_2                        0x0044
#define DVS_SW_OFFSET_2_SW_OFFSET_2_SHIFT                                   0
#define DVS_SW_OFFSET_2_SW_OFFSET_2_MASK                                    0x000001ff
#define DVS_SW_OFFSET_3                        0x0048
#define DVS_SW_OFFSET_3_SW_OFFSET_3_SHIFT                                   0
#define DVS_SW_OFFSET_3_SW_OFFSET_3_MASK                                    0x000001ff
#define DVS_SW_OFFSET_4                        0x004c
#define DVS_SW_OFFSET_4_SW_OFFSET_4_SHIFT                                   0
#define DVS_SW_OFFSET_4_SW_OFFSET_4_MASK                                    0x000001ff
#define DVS_SW_OFFSET_5                        0x0050
#define DVS_SW_OFFSET_5_SW_OFFSET_5_SHIFT                                   0
#define DVS_SW_OFFSET_5_SW_OFFSET_5_MASK                                    0x000001ff
#define DVS_SW_OFFSET_6                        0x0054
#define DVS_SW_OFFSET_6_SW_OFFSET_6_SHIFT                                   0
#define DVS_SW_OFFSET_6_SW_OFFSET_6_MASK                                    0x000001ff
#define DVS_SW_OFFSET_7                        0x0058
#define DVS_SW_OFFSET_7_SW_OFFSET_7_SHIFT                                   0
#define DVS_SW_OFFSET_7_SW_OFFSET_7_MASK                                    0x000001ff
#define DVS_BYPASS_0                           0x0060
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_0_SHIFT                             0
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_0_MASK                              0x00000001
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_1_SHIFT                             1
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_1_MASK                              0x00000002
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_2_SHIFT                             2
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_2_MASK                              0x00000004
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_3_SHIFT                             3
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_3_MASK                              0x00000008
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_4_SHIFT                             4
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_4_MASK                              0x00000010
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_5_SHIFT                             5
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_5_MASK                              0x00000020
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_6_SHIFT                             6
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_6_MASK                              0x00000040
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_7_SHIFT                             7
#define DVS_BYPASS_0_SW_VOLT_REQ_BYPASS_7_MASK                              0x00000080
#define DVS_BYPASS_1                           0x0064
#define DVS_BYPASS_1_HW_VOLT_REQ_BYPASS_0_SHIFT                             0
#define DVS_BYPASS_1_HW_VOLT_REQ_BYPASS_0_MASK                              0x00000001
#define DVS_BYPASS_1_HW_VOLT_REQ_BYPASS_1_SHIFT                             1
#define DVS_BYPASS_1_HW_VOLT_REQ_BYPASS_1_MASK                              0x00000002
#define DVS_BYPASS_1_HW_VOLT_REQ_BYPASS_2_SHIFT                             2
#define DVS_BYPASS_1_HW_VOLT_REQ_BYPASS_2_MASK                              0x00000004
#define DVS_BYPASS_1_OFFSET_BYPASS_SHIFT                                    3
#define DVS_BYPASS_1_OFFSET_BYPASS_MASK                                     0x00000008
#define DVS_BYPASS_1_RE_ARBIT_BYPASS_SHIFT                                  4
#define DVS_BYPASS_1_RE_ARBIT_BYPASS_MASK                                   0x00000010
#define DVS_BYPASS_1_SUPER_REQ_BYPASS_SHIFT                                 5
#define DVS_BYPASS_1_SUPER_REQ_BYPASS_MASK                                  0x00000020
#define DVS_BYPASS_1_SPMI_PER_DONE_BYPASS_SHIFT                             6
#define DVS_BYPASS_1_SPMI_PER_DONE_BYPASS_MASK                              0x00000040
#define DVS_ENABLE                             0x0068
#define DVS_ENABLE_FUNC_EN_SHIFT                                            0
#define DVS_ENABLE_FUNC_EN_MASK                                             0x00000001
#define DVS_ENABLE_OFFSET_ENABLE_SHIFT                                      1
#define DVS_ENABLE_OFFSET_ENABLE_MASK                                       0x00000002
#define DVS_SW_STATE_ACK                       0x0074
#define DVS_SW_STATE_ACK_SW_STATE_ACK_0_SHIFT                               0
#define DVS_SW_STATE_ACK_SW_STATE_ACK_0_MASK                                0x00000001
#define DVS_SW_STATE_ACK_SW_STATE_ACK_1_SHIFT                               1
#define DVS_SW_STATE_ACK_SW_STATE_ACK_1_MASK                                0x00000002
#define DVS_SW_STATE_ACK_SW_STATE_ACK_2_SHIFT                               2
#define DVS_SW_STATE_ACK_SW_STATE_ACK_2_MASK                                0x00000004
#define DVS_SW_STATE_ACK_SW_STATE_ACK_3_SHIFT                               3
#define DVS_SW_STATE_ACK_SW_STATE_ACK_3_MASK                                0x00000008
#define DVS_SW_STATE_ACK_SW_STATE_ACK_4_SHIFT                               4
#define DVS_SW_STATE_ACK_SW_STATE_ACK_4_MASK                                0x00000010
#define DVS_SW_STATE_ACK_SW_STATE_ACK_5_SHIFT                               5
#define DVS_SW_STATE_ACK_SW_STATE_ACK_5_MASK                                0x00000020
#define DVS_SW_STATE_ACK_SW_STATE_ACK_6_SHIFT                               6
#define DVS_SW_STATE_ACK_SW_STATE_ACK_6_MASK                                0x00000040
#define DVS_SW_STATE_ACK_SW_STATE_ACK_7_SHIFT                               7
#define DVS_SW_STATE_ACK_SW_STATE_ACK_7_MASK                                0x00000080
#define DVS_SW_WAIT_ARBIT                      0x0078
#define DVS_SW_WAIT_ARBIT_SW_WAIT_ARBIT_SHIFT                               0
#define DVS_SW_WAIT_ARBIT_SW_WAIT_ARBIT_MASK                                0x000000ff
#define DVS_SW_WAIT_ARBIT_SW_VOLT_RATIO_SHIFT                               8
#define DVS_SW_WAIT_ARBIT_SW_VOLT_RATIO_MASK                                0x003fff00
#define DVS_CUR_STATE                          0x007c
#define DVS_CUR_STATE_ARBIT_CUR_STATE_SHIFT                                 0
#define DVS_CUR_STATE_ARBIT_CUR_STATE_MASK                                  0x00000003
#define DVS_CUR_STATE_CTRL_CUR_STATE_SHIFT                                  2
#define DVS_CUR_STATE_CTRL_CUR_STATE_MASK                                   0x0000001c
#define DVS_CUR_STATE_VOLT_REQ_SHIFT                                        5
#define DVS_CUR_STATE_VOLT_REQ_MASK                                         0x00001fe0
#define DVS_CUR_STATE_VOLT_ARBIT_SHIFT                                      13
#define DVS_CUR_STATE_VOLT_ARBIT_MASK                                       0x001fe000
#define DVS_CUR_STATE_VOLT_CURRENT_SHIFT                                    21
#define DVS_CUR_STATE_VOLT_CURRENT_MASK                                     0x1fe00000
#define DVS_WAIT_TIME_UP                       0x0080
#define DVS_WAIT_TIME_UP_SW_WAIT_UP_T1_SHIFT                                0
#define DVS_WAIT_TIME_UP_SW_WAIT_UP_T1_MASK                                 0x0000ffff
#define DVS_WAIT_TIME_UP_SW_WAIT_UP_T2_SHIFT                                16
#define DVS_WAIT_TIME_UP_SW_WAIT_UP_T2_MASK                                 0xffff0000
#define DVS_THSREHOLD                          0x0084
#define DVS_THSREHOLD_LOWEST_THRESHOLD_SHIFT                                0
#define DVS_THSREHOLD_LOWEST_THRESHOLD_MASK                                 0x000000ff
#define DVS_THSREHOLD_LOW_THRESHOLD_SHIFT                                   8
#define DVS_THSREHOLD_LOW_THRESHOLD_MASK                                    0x0000ff00
#define DVS_DEFAULT_VOLT                       0x0088
#define DVS_US_CNT                             0x0090
#define DVS_US_CNT_US_CNT_SHIFT                                             0
#define DVS_US_CNT_US_CNT_MASK                                              0x000000ff
#define DVS_TIME_CFG                           0x0094
#define DVS_TIME_CFG_VOL_FIX_TIME_CFG_SHIFT                                 0
#define DVS_TIME_CFG_VOL_FIX_TIME_CFG_MASK                                  0x0000003f
#define DVS_WAIT_TIME_DOWN                     0x0110
#define DVS_WAIT_TIME_DOWN_SW_WAIT_DOWN_T1_SHIFT                            0
#define DVS_WAIT_TIME_DOWN_SW_WAIT_DOWN_T1_MASK                             0x0000ffff
#define DVS_WAIT_TIME_DOWN_SW_WAIT_DOWN_T2_SHIFT                            16
#define DVS_WAIT_TIME_DOWN_SW_WAIT_DOWN_T2_MASK                             0xffff0000
#define DVS_SW_INITIAL_0                       0x0120
#define DVS_SW_INITIAL_0_SW_INITIAL_VOLT_REQ_0_SHIFT                        0
#define DVS_SW_INITIAL_0_SW_INITIAL_VOLT_REQ_0_MASK                         0x000000ff
#define DVS_SW_INITIAL_0_SW_INITIAL_SEL_0_SHIFT                             8
#define DVS_SW_INITIAL_0_SW_INITIAL_SEL_0_MASK                              0x00000100
#define DVS_SW_INITIAL_0_SW_INITIAL_VOLT_REQ_1_SHIFT                        9
#define DVS_SW_INITIAL_0_SW_INITIAL_VOLT_REQ_1_MASK                         0x0001fe00
#define DVS_SW_INITIAL_0_SW_INITIAL_SEL_1_SHIFT                             17
#define DVS_SW_INITIAL_0_SW_INITIAL_SEL_1_MASK                              0x00020000
#define DVS_SW_INITIAL_0_SW_INITIAL_VOLT_REQ_2_SHIFT                        18
#define DVS_SW_INITIAL_0_SW_INITIAL_VOLT_REQ_2_MASK                         0x03fc0000
#define DVS_SW_INITIAL_0_SW_INITIAL_SEL_2_SHIFT                             26
#define DVS_SW_INITIAL_0_SW_INITIAL_SEL_2_MASK                              0x04000000
#define DVS_SW_INITIAL_1                       0x0130
#define DVS_SW_INITIAL_1_SW_INITIAL_ARBIT_SHIFT                             0
#define DVS_SW_INITIAL_1_SW_INITIAL_ARBIT_MASK                              0x000000ff
#define DVS_SW_INITIAL_1_SW_INITIAL_ARBIT_SEL_SHIFT                         8
#define DVS_SW_INITIAL_1_SW_INITIAL_ARBIT_SEL_MASK                          0x00000100
#define DVS_SW_INITIAL_1_SW_INITIAL_REQ_SHIFT                               9
#define DVS_SW_INITIAL_1_SW_INITIAL_REQ_MASK                                0x0001fe00
#define DVS_SW_INITIAL_1_SW_INITIAL_REQ_SEL_SHIFT                           17
#define DVS_SW_INITIAL_1_SW_INITIAL_REQ_SEL_MASK                            0x00020000
#define DVS_SW_INITIAL_1_SW_INITIAL_CURRENT_SHIFT                           18
#define DVS_SW_INITIAL_1_SW_INITIAL_CURRENT_MASK                            0x03fc0000
#define DVS_SW_INITIAL_1_SW_INITIAL_CURRENT_SEL_SHIFT                       26
#define DVS_SW_INITIAL_1_SW_INITIAL_CURRENT_SEL_MASK                        0x04000000
#define DVS_STATE_1                            0x0140
#define DVS_STATE_1_VOLT_PREVIOUS_SHIFT                                     0
#define DVS_STATE_1_VOLT_PREVIOUS_MASK                                      0x000000ff
#define DVS_STATE_1_ARBIT_NEXT_STATE_SHIFT                                  8
#define DVS_STATE_1_ARBIT_NEXT_STATE_MASK                                   0x00000300
#define DVS_STATE_1_CTRL_NEXT_STATE_SHIFT                                   10
#define DVS_STATE_1_CTRL_NEXT_STATE_MASK                                    0x00001c00
#define DVS_INTR_DONE_IRQ_INT_RAW              0x0200
#define DVS_INTR_DONE_IRQ_INT_RAW_INTR_DONE_IN_RAW_SHIFT                    0
#define DVS_INTR_DONE_IRQ_INT_FORCE            0x0204
#define DVS_INTR_DONE_IRQ_INT_FORCE_INTR_DONE_IN_RAW_SHIFT                  0
#define DVS_INTR_DONE_IRQ_INT_MASK             0x0208
#define DVS_INTR_DONE_IRQ_INT_MASK_INTR_DONE_IN_MASK_SHIFT                  0
#define DVS_INTR_DONE_IRQ_INT_MASK_INTR_DONE_IN_MASK_MASK                   0x00000001
#define DVS_INTR_DONE_IRQ_INT_STATUS           0x020c
#define DVS_INTR_DONE_IRQ_INT_STATUS_INTR_DONE_IN_STATUS_SHIFT              0
#define DVS_INTR_DONE_IRQ_INT_STATUS_INTR_DONE_IN_STATUS_MASK               0x00000001
#define DVS_INTR_TIMEOUT_IRQ_INT_RAW           0x0210
#define DVS_INTR_TIMEOUT_IRQ_INT_RAW_INTR_TIMEOUT_IN_RAW_SHIFT              0
#define DVS_INTR_TIMEOUT_IRQ_INT_FORCE         0x0214
#define DVS_INTR_TIMEOUT_IRQ_INT_FORCE_INTR_TIMEOUT_IN_RAW_SHIFT            0
#define DVS_INTR_TIMEOUT_IRQ_INT_MASK          0x0218
#define DVS_INTR_TIMEOUT_IRQ_INT_MASK_INTR_TIMEOUT_IN_MASK_SHIFT            0
#define DVS_INTR_TIMEOUT_IRQ_INT_MASK_INTR_TIMEOUT_IN_MASK_MASK             0x00000001
#define DVS_INTR_TIMEOUT_IRQ_INT_STATUS        0x021c
#define DVS_INTR_TIMEOUT_IRQ_INT_STATUS_INTR_TIMEOUT_IN_STATUS_SHIFT        0
#define DVS_INTR_TIMEOUT_IRQ_INT_STATUS_INTR_TIMEOUT_IN_STATUS_MASK         0x00000001
#define DVS_VOLT_REQ_8                         0x0800
#define DVS_VOLT_REQ_8_SW_VOLT_REQ_8_SHIFT                                  0
#define DVS_VOLT_REQ_8_SW_VOLT_REQ_8_MASK                                   0x000000ff
#define DVS_VOLT_REQ_8_SW_VOLT_REQ_VALID_8_SHIFT                            8
#define DVS_VOLT_REQ_8_SW_VOLT_REQ_VALID_8_MASK                             0x00000100
#define DVS_VOLT_REQ_9                         0x0804
#define DVS_VOLT_REQ_9_SW_VOLT_REQ_9_SHIFT                                  0
#define DVS_VOLT_REQ_9_SW_VOLT_REQ_9_MASK                                   0x000000ff
#define DVS_VOLT_REQ_9_SW_VOLT_REQ_VALID_9_SHIFT                            8
#define DVS_VOLT_REQ_9_SW_VOLT_REQ_VALID_9_MASK                             0x00000100
#define DVS_VOLT_REQ_10                        0x0808
#define DVS_VOLT_REQ_10_SW_VOLT_REQ_10_SHIFT                                0
#define DVS_VOLT_REQ_10_SW_VOLT_REQ_10_MASK                                 0x000000ff
#define DVS_VOLT_REQ_10_SW_VOLT_REQ_VALID_10_SHIFT                          8
#define DVS_VOLT_REQ_10_SW_VOLT_REQ_VALID_10_MASK                           0x00000100
#define DVS_VOLT_REQ_11                        0x080c
#define DVS_VOLT_REQ_11_SW_VOLT_REQ_11_SHIFT                                0
#define DVS_VOLT_REQ_11_SW_VOLT_REQ_11_MASK                                 0x000000ff
#define DVS_VOLT_REQ_11_SW_VOLT_REQ_VALID_11_SHIFT                          8
#define DVS_VOLT_REQ_11_SW_VOLT_REQ_VALID_11_MASK                           0x00000100
#define DVS_VOLT_REQ_12                        0x0810
#define DVS_VOLT_REQ_12_SW_VOLT_REQ_12_SHIFT                                0
#define DVS_VOLT_REQ_12_SW_VOLT_REQ_12_MASK                                 0x000000ff
#define DVS_VOLT_REQ_12_SW_VOLT_REQ_VALID_12_SHIFT                          8
#define DVS_VOLT_REQ_12_SW_VOLT_REQ_VALID_12_MASK                           0x00000100
#define DVS_VOLT_REQ_13                        0x0814
#define DVS_VOLT_REQ_13_SW_VOLT_REQ_13_SHIFT                                0
#define DVS_VOLT_REQ_13_SW_VOLT_REQ_13_MASK                                 0x000000ff
#define DVS_VOLT_REQ_13_SW_VOLT_REQ_VALID_13_SHIFT                          8
#define DVS_VOLT_REQ_13_SW_VOLT_REQ_VALID_13_MASK                           0x00000100
#define DVS_VOLT_REQ_14                        0x0818
#define DVS_VOLT_REQ_14_SW_VOLT_REQ_14_SHIFT                                0
#define DVS_VOLT_REQ_14_SW_VOLT_REQ_14_MASK                                 0x000000ff
#define DVS_VOLT_REQ_14_SW_VOLT_REQ_VALID_14_SHIFT                          8
#define DVS_VOLT_REQ_14_SW_VOLT_REQ_VALID_14_MASK                           0x00000100
#define DVS_VOLT_REQ_15                        0x081c
#define DVS_VOLT_REQ_15_SW_VOLT_REQ_15_SHIFT                                0
#define DVS_VOLT_REQ_15_SW_VOLT_REQ_15_MASK                                 0x000000ff
#define DVS_VOLT_REQ_15_SW_VOLT_REQ_VALID_15_SHIFT                          8
#define DVS_VOLT_REQ_15_SW_VOLT_REQ_VALID_15_MASK                           0x00000100
#define DVS_SW_OFFSET_8                        0x0900
#define DVS_SW_OFFSET_8_SW_OFFSET_8_SHIFT                                   0
#define DVS_SW_OFFSET_8_SW_OFFSET_8_MASK                                    0x000001ff
#define DVS_SW_OFFSET_9                        0x0904
#define DVS_SW_OFFSET_9_SW_OFFSET_9_SHIFT                                   0
#define DVS_SW_OFFSET_9_SW_OFFSET_9_MASK                                    0x000001ff
#define DVS_SW_OFFSET_10                       0x0908
#define DVS_SW_OFFSET_10_SW_OFFSET_10_SHIFT                                 0
#define DVS_SW_OFFSET_10_SW_OFFSET_10_MASK                                  0x000001ff
#define DVS_SW_OFFSET_11                       0x090c
#define DVS_SW_OFFSET_11_SW_OFFSET_11_SHIFT                                 0
#define DVS_SW_OFFSET_11_SW_OFFSET_11_MASK                                  0x000001ff
#define DVS_SW_OFFSET_12                       0x0910
#define DVS_SW_OFFSET_12_SW_OFFSET_12_SHIFT                                 0
#define DVS_SW_OFFSET_12_SW_OFFSET_12_MASK                                  0x000001ff
#define DVS_SW_OFFSET_13                       0x0914
#define DVS_SW_OFFSET_13_SW_OFFSET_13_SHIFT                                 0
#define DVS_SW_OFFSET_13_SW_OFFSET_13_MASK                                  0x000001ff
#define DVS_SW_OFFSET_14                       0x0918
#define DVS_SW_OFFSET_14_SW_OFFSET_14_SHIFT                                 0
#define DVS_SW_OFFSET_14_SW_OFFSET_14_MASK                                  0x000001ff
#define DVS_SW_OFFSET_15                       0x091c
#define DVS_SW_OFFSET_15_SW_OFFSET_15_SHIFT                                 0
#define DVS_SW_OFFSET_15_SW_OFFSET_15_MASK                                  0x000001ff
#define DVS_BYPASS_2                           0x0920
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_8_SHIFT                             0
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_8_MASK                              0x00000001
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_9_SHIFT                             1
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_9_MASK                              0x00000002
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_10_SHIFT                            2
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_10_MASK                             0x00000004
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_11_SHIFT                            3
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_11_MASK                             0x00000008
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_12_SHIFT                            4
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_12_MASK                             0x00000010
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_13_SHIFT                            5
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_13_MASK                             0x00000020
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_14_SHIFT                            6
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_14_MASK                             0x00000040
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_15_SHIFT                            7
#define DVS_BYPASS_2_SW_VOLT_REQ_BYPASS_15_MASK                             0x00000080
#define DVS_SW_STATE_ACK_1                     0x0928
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_8_SHIFT                             0
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_8_MASK                              0x00000001
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_9_SHIFT                             1
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_9_MASK                              0x00000002
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_10_SHIFT                            2
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_10_MASK                             0x00000004
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_11_SHIFT                            3
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_11_MASK                             0x00000008
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_12_SHIFT                            4
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_12_MASK                             0x00000010
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_13_SHIFT                            5
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_13_MASK                             0x00000020
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_14_SHIFT                            6
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_14_MASK                             0x00000040
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_15_SHIFT                            7
#define DVS_SW_STATE_ACK_1_SW_STATE_ACK_15_MASK                             0x00000080
#define DVS_CGBYPASS                           0x092c
#define DVS_CGBYPASS_CGBYPASS_SHIFT                                         0
#define DVS_CGBYPASS_CGBYPASS_MASK                                          0x00000001


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_0       :  8;
        unsigned int sw_volt_req_valid_0 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_1       :  8;
        unsigned int sw_volt_req_valid_1 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_2       :  8;
        unsigned int sw_volt_req_valid_2 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_3       :  8;
        unsigned int sw_volt_req_valid_3 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_4       :  8;
        unsigned int sw_volt_req_valid_4 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_5       :  8;
        unsigned int sw_volt_req_valid_5 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_6       :  8;
        unsigned int sw_volt_req_valid_6 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_7       :  8;
        unsigned int sw_volt_req_valid_7 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hw_offset_0 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_hw_offset_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hw_offset_1 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_hw_offset_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hw_offset_2 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_hw_offset_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_0 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_1 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_2 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_3 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_3_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_4 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_4_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_5 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_5_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_6 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_6_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_7 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_7_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_bypass_0 :  1;
        unsigned int sw_volt_req_bypass_1 :  1;
        unsigned int sw_volt_req_bypass_2 :  1;
        unsigned int sw_volt_req_bypass_3 :  1;
        unsigned int sw_volt_req_bypass_4 :  1;
        unsigned int sw_volt_req_bypass_5 :  1;
        unsigned int sw_volt_req_bypass_6 :  1;
        unsigned int sw_volt_req_bypass_7 :  1;
        unsigned int reserved_0           :  8;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_bypass_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int hw_volt_req_bypass_0 :  1;
        unsigned int hw_volt_req_bypass_1 :  1;
        unsigned int hw_volt_req_bypass_2 :  1;
        unsigned int dvs_offset_bypass    :  1;
        unsigned int dvs_re_arbit_bypass  :  1;
        unsigned int dvs_super_req_bypass :  1;
        unsigned int spmi_per_done_bypass :  1;
        unsigned int reserved_0           :  9;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_bypass_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_func_en       :  1;
        unsigned int dvs_offset_enable :  1;
        unsigned int reserved_0        : 14;
        unsigned int _bm_              : 16;
    } reg;
}dvs_enable_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_state_ack_0 :  1;
        unsigned int sw_state_ack_1 :  1;
        unsigned int sw_state_ack_2 :  1;
        unsigned int sw_state_ack_3 :  1;
        unsigned int sw_state_ack_4 :  1;
        unsigned int sw_state_ack_5 :  1;
        unsigned int sw_state_ack_6 :  1;
        unsigned int sw_state_ack_7 :  1;
        unsigned int reserved_0     : 24;
    } reg;
}dvs_sw_state_ack_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_wait_arbit :  8;
        unsigned int sw_volt_ratio : 14;
        unsigned int reserved_0    : 10;
    } reg;
}dvs_sw_wait_arbit_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_arbit_cur_state :  2;
        unsigned int dvs_ctrl_cur_state  :  3;
        unsigned int dvs_volt_req        :  8;
        unsigned int dvs_volt_arbit      :  8;
        unsigned int dvs_volt_current    :  8;
        unsigned int reserved_0          :  3;
    } reg;
}dvs_cur_state_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_sw_wait_up_t1 : 16;
        unsigned int dvs_sw_wait_up_t2 : 16;
    } reg;
}dvs_wait_time_up_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_lowest_threshold :  8;
        unsigned int dvs_low_threshold    :  8;
        unsigned int reserved_0           : 16;
    } reg;
}dvs_thsrehold_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int reserved_0 : 32;
    } reg;
}dvs_default_volt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_us_cnt :  8;
        unsigned int reserved_0 : 24;
    } reg;
}dvs_us_cnt_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int vol_fix_time_cfg :  6;
        unsigned int reserved_0       : 26;
    } reg;
}dvs_time_cfg_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_sw_wait_down_t1 : 16;
        unsigned int dvs_sw_wait_down_t2 : 16;
    } reg;
}dvs_wait_time_down_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_initial_volt_req_0 :  8;
        unsigned int sw_initial_sel_0      :  1;
        unsigned int sw_initial_volt_req_1 :  8;
        unsigned int sw_initial_sel_1      :  1;
        unsigned int sw_initial_volt_req_2 :  8;
        unsigned int sw_initial_sel_2      :  1;
        unsigned int reserved_0            :  5;
    } reg;
}dvs_sw_initial_0_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_initial_dvs_arbit   :  8;
        unsigned int sw_initial_arbit_sel   :  1;
        unsigned int sw_initial_dvs_req     :  8;
        unsigned int sw_initial_req_sel     :  1;
        unsigned int sw_initial_dvs_current :  8;
        unsigned int sw_initial_current_sel :  1;
        unsigned int reserved_0             :  5;
    } reg;
}dvs_sw_initial_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int dvs_volt_previous    :  8;
        unsigned int dvs_arbit_next_state :  2;
        unsigned int dvs_ctrl_next_state  :  3;
        unsigned int reserved_0           : 19;
    } reg;
}dvs_state_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_done_in_raw :  1;
        unsigned int reserved_0           : 31;
    } reg;
}dvs_intr_done_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_done_in_raw :  1;
        unsigned int reserved_0           : 31;
    } reg;
}dvs_intr_done_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_done_in_mask :  1;
        unsigned int reserved_0            : 31;
    } reg;
}dvs_intr_done_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_done_in_status :  1;
        unsigned int reserved_0              : 31;
    } reg;
}dvs_intr_done_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_timeout_in_raw :  1;
        unsigned int reserved_0              : 31;
    } reg;
}dvs_intr_timeout_irq_int_raw_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_timeout_in_raw :  1;
        unsigned int reserved_0              : 31;
    } reg;
}dvs_intr_timeout_irq_int_force_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_timeout_in_mask :  1;
        unsigned int reserved_0               : 31;
    } reg;
}dvs_intr_timeout_irq_int_mask_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int intr_dvs_timeout_in_status :  1;
        unsigned int reserved_0                 : 31;
    } reg;
}dvs_intr_timeout_irq_int_status_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_8       :  8;
        unsigned int sw_volt_req_valid_8 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_9       :  8;
        unsigned int sw_volt_req_valid_9 :  1;
        unsigned int reserved_0          :  7;
        unsigned int _bm_                : 16;
    } reg;
}dvs_volt_req_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_10       :  8;
        unsigned int sw_volt_req_valid_10 :  1;
        unsigned int reserved_0           :  7;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_volt_req_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_11       :  8;
        unsigned int sw_volt_req_valid_11 :  1;
        unsigned int reserved_0           :  7;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_volt_req_11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_12       :  8;
        unsigned int sw_volt_req_valid_12 :  1;
        unsigned int reserved_0           :  7;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_volt_req_12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_13       :  8;
        unsigned int sw_volt_req_valid_13 :  1;
        unsigned int reserved_0           :  7;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_volt_req_13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_14       :  8;
        unsigned int sw_volt_req_valid_14 :  1;
        unsigned int reserved_0           :  7;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_volt_req_14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_15       :  8;
        unsigned int sw_volt_req_valid_15 :  1;
        unsigned int reserved_0           :  7;
        unsigned int _bm_                 : 16;
    } reg;
}dvs_volt_req_15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_8 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_8_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_9 :  9;
        unsigned int reserved_0  :  7;
        unsigned int _bm_        : 16;
    } reg;
}dvs_sw_offset_9_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_10 :  9;
        unsigned int reserved_0   :  7;
        unsigned int _bm_         : 16;
    } reg;
}dvs_sw_offset_10_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_11 :  9;
        unsigned int reserved_0   :  7;
        unsigned int _bm_         : 16;
    } reg;
}dvs_sw_offset_11_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_12 :  9;
        unsigned int reserved_0   :  7;
        unsigned int _bm_         : 16;
    } reg;
}dvs_sw_offset_12_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_13 :  9;
        unsigned int reserved_0   :  7;
        unsigned int _bm_         : 16;
    } reg;
}dvs_sw_offset_13_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_14 :  9;
        unsigned int reserved_0   :  7;
        unsigned int _bm_         : 16;
    } reg;
}dvs_sw_offset_14_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_offset_15 :  9;
        unsigned int reserved_0   :  7;
        unsigned int _bm_         : 16;
    } reg;
}dvs_sw_offset_15_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_volt_req_bypass_8  :  1;
        unsigned int sw_volt_req_bypass_9  :  1;
        unsigned int sw_volt_req_bypass_10 :  1;
        unsigned int sw_volt_req_bypass_11 :  1;
        unsigned int sw_volt_req_bypass_12 :  1;
        unsigned int sw_volt_req_bypass_13 :  1;
        unsigned int sw_volt_req_bypass_14 :  1;
        unsigned int sw_volt_req_bypass_15 :  1;
        unsigned int reserved_0            :  8;
        unsigned int _bm_                  : 16;
    } reg;
}dvs_bypass_2_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int sw_state_ack_8  :  1;
        unsigned int sw_state_ack_9  :  1;
        unsigned int sw_state_ack_10 :  1;
        unsigned int sw_state_ack_11 :  1;
        unsigned int sw_state_ack_12 :  1;
        unsigned int sw_state_ack_13 :  1;
        unsigned int sw_state_ack_14 :  1;
        unsigned int sw_state_ack_15 :  1;
        unsigned int reserved_0      : 24;
    } reg;
}dvs_sw_state_ack_1_t;


typedef union {
    unsigned int val;
    struct {
        unsigned int cgbypass   :  1;
        unsigned int reserved_0 : 31;
    } reg;
}dvs_cgbypass_t;

#endif
