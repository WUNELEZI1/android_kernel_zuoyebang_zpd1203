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

/* Warning: Please do not change this file yourself! */
/* File generation time: 2024-01-26 17:03:52. */

#ifndef _DPU_HW_DISP_GLB_REG_H_
#define _DPU_HW_DISP_GLB_REG_H_

/* REGISTER disp_glb_reg_0 */
#define SW_CLK_EN_OFFSET                                              0x00
#define SW_CLK_EN_SHIFT                                               0
#define SW_CLK_EN_LENGTH                                              1
#define AUTO_CG_EN_SHIFT                                              1
#define AUTO_CG_EN_LENGTH                                             1
#define IDLE_DVFS_EN_SHIFT                                            2
#define IDLE_DVFS_EN_LENGTH                                           1
#define ACTIVE_DVFS_EN_SHIFT                                          3
#define ACTIVE_DVFS_EN_LENGTH                                         1

/* REGISTER disp_glb_reg_1 */
#define IDLE_DIV_OFFSET                                               0x04

/* REGISTER disp_glb_reg_2 */
#define ACTIVE_DIV_OFFSET                                             0x08

/* REGISTER disp_glb_reg_3 */
#define SYNC_COUNTER_OFFSET                                           0x0c
#define SYNC_COUNTER_SHIFT                                            0
#define SYNC_COUNTER_LENGTH                                           16
#define TIMEOUT_COUNTER_SHIFT                                         16
#define TIMEOUT_COUNTER_LENGTH                                        16

/* REGISTER disp_glb_reg_4 */
#define FREQ_DIV_COUNTER_OFFSET                                       0x10
#define FREQ_DIV_COUNTER_SHIFT                                        0
#define FREQ_DIV_COUNTER_LENGTH                                       16
#define SCENE_VOLT_SHIFT                                              16
#define SCENE_VOLT_LENGTH                                             8
#define IDLE_VOLT_SHIFT                                               24
#define IDLE_VOLT_LENGTH                                              8

/* REGISTER disp_glb_reg_5 */
#define ACTIVE_LOW_VOLT_OFFSET                                        0x14
#define ACTIVE_LOW_VOLT_SHIFT                                         0
#define ACTIVE_LOW_VOLT_LENGTH                                        8

/* REGISTER disp_glb_reg_6 */
#define DDR_IDLE_DVFS_EN_OFFSET                                       0x18
#define DDR_IDLE_DVFS_EN_SHIFT                                        0
#define DDR_IDLE_DVFS_EN_LENGTH                                       1
#define DDR_ACTIVE_DVFS_EN_SHIFT                                      1
#define DDR_ACTIVE_DVFS_EN_LENGTH                                     1
#define DVFS_SOFT_CLR_SHIFT                                           2
#define DVFS_SOFT_CLR_LENGTH                                          1

/* REGISTER disp_glb_reg_7 */
#define VOLT_SCALING_DISABLE_OFFSET                                   0x1c
#define VOLT_SCALING_DISABLE_SHIFT                                    0
#define VOLT_SCALING_DISABLE_LENGTH                                   1

/* REGISTER disp_glb_reg_8 */
#define DDR_DVFS_STATUS_OFFSET                                        0x20
#define DDR_DVFS_STATUS_SHIFT                                         0
#define DDR_DVFS_STATUS_LENGTH                                        2
#define DVFS_FAIL_CNT_SHIFT                                           16
#define DVFS_FAIL_CNT_LENGTH                                          16

/* REGISTER disp_glb_reg_9 */
#define DPU_DVFS_STATUS_OFFSET                                        0x24
#define DPU_DVFS_STATUS_SHIFT                                         0
#define DPU_DVFS_STATUS_LENGTH                                        1

/* REGISTER disp_glb_reg_10 */
#define RAM_LS_OFFSET                                                 0x28
#define RAM_LS_SHIFT                                                  0
#define RAM_LS_LENGTH                                                 1
#define RAM_TPR_SHIFT                                                 1
#define RAM_TPR_LENGTH                                                1
#define RAM_WTPR_SHIFT                                                2
#define RAM_WTPR_LENGTH                                               1
#define DPU_TOP_MEM_DS_SHIFT                                          3
#define DPU_TOP_MEM_DS_LENGTH                                         1
#define DPU_TOP_P1H0_GRP0_MEM_DS_SHIFT                                4
#define DPU_TOP_P1H0_GRP0_MEM_DS_LENGTH                               1
#define DPU_TOP_P1H0_GRP1_MEM_DS_SHIFT                                5
#define DPU_TOP_P1H0_GRP1_MEM_DS_LENGTH                               1
#define DPU_TOP_P1H1_GRP0_MEM_DS_SHIFT                                6
#define DPU_TOP_P1H1_GRP0_MEM_DS_LENGTH                               1
#define DPU_TOP_P1H1_GRP1_MEM_DS_SHIFT                                7
#define DPU_TOP_P1H1_GRP1_MEM_DS_LENGTH                               1
#define DPU_P2_MEM_DS_SHIFT                                           8
#define DPU_P2_MEM_DS_LENGTH                                          1
#define VEU_MEM_DS_SHIFT                                              9
#define VEU_MEM_DS_LENGTH                                             1

/* REGISTER disp_glb_reg_11 */
#define DPU_TOP_MEM_DS_ACK_OFFSET                                     0x2c
#define DPU_TOP_MEM_DS_ACK_SHIFT                                      0
#define DPU_TOP_MEM_DS_ACK_LENGTH                                     1
#define DPU_TOP_P1H0_GRP0_MEM_DS_ACK_SHIFT                            1
#define DPU_TOP_P1H0_GRP0_MEM_DS_ACK_LENGTH                           1
#define DPU_TOP_P1H0_GRP1_MEM_DS_ACK_SHIFT                            2
#define DPU_TOP_P1H0_GRP1_MEM_DS_ACK_LENGTH                           1
#define DPU_TOP_P1H1_GRP0_MEM_DS_ACK_SHIFT                            3
#define DPU_TOP_P1H1_GRP0_MEM_DS_ACK_LENGTH                           1
#define DPU_TOP_P1H1_GRP1_MEM_DS_ACK_SHIFT                            4
#define DPU_TOP_P1H1_GRP1_MEM_DS_ACK_LENGTH                           1
#define DPU_P2_MEM_DS_ACK_SHIFT                                       5
#define DPU_P2_MEM_DS_ACK_LENGTH                                      1
#define VEU_MEM_DS_ACK_SHIFT                                          6
#define VEU_MEM_DS_ACK_LENGTH                                         1

/* REGISTER disp_glb_reg_12 */
#define HDC_DVFS_OK_OFFSET                                            0x30
#define HDC_DVFS_OK_SHIFT                                             0
#define HDC_DVFS_OK_LENGTH                                            1
#define OBUF_DVFS_OK_SHIFT                                            1
#define OBUF_DVFS_OK_LENGTH                                           3
#define WB_DVFS_OK_SHIFT                                              4
#define WB_DVFS_OK_LENGTH                                             2
#define DSCW_DVFS_OK_SHIFT                                            6
#define DSCW_DVFS_OK_LENGTH                                           2

/* REGISTER disp_glb_reg_13 */
#define OCM_AUTOCG_EN_OFFSET                                          0x34
#define OCM_AUTOCG_EN_SHIFT                                           0
#define OCM_AUTOCG_EN_LENGTH                                          1
#define DISP_PLL_DOZE_EN_SHIFT                                        1
#define DISP_PLL_DOZE_EN_LENGTH                                       1
#define HDC_DVFS_OK_VOTE_EN_SHIFT                                     2
#define HDC_DVFS_OK_VOTE_EN_LENGTH                                    1

/* REGISTER disp_glb_reg_14 */
#define WB0_DVFS_OK_VOTE_EN_OFFSET                                    0x38
#define WB0_DVFS_OK_VOTE_EN_SHIFT                                     0
#define WB0_DVFS_OK_VOTE_EN_LENGTH                                    1
#define WB0_DVFS_OK_VOTE_TH_SHIFT                                     16
#define WB0_DVFS_OK_VOTE_TH_LENGTH                                    16

/* REGISTER disp_glb_reg_15 */
#define WB1_DVFS_OK_VOTE_EN_OFFSET                                    0x3c
#define WB1_DVFS_OK_VOTE_EN_SHIFT                                     0
#define WB1_DVFS_OK_VOTE_EN_LENGTH                                    1
#define WB1_DVFS_OK_VOTE_TH_SHIFT                                     16
#define WB1_DVFS_OK_VOTE_TH_LENGTH                                    16

/* REGISTER disp_glb_reg_16 */
#define DSCW0_DVFS_OK_VOTE_EN_OFFSET                                  0x40
#define DSCW0_DVFS_OK_VOTE_EN_SHIFT                                   0
#define DSCW0_DVFS_OK_VOTE_EN_LENGTH                                  1
#define DSCW0_DVFS_OK_VOTE_TH_SHIFT                                   16
#define DSCW0_DVFS_OK_VOTE_TH_LENGTH                                  16

/* REGISTER disp_glb_reg_17 */
#define DSCW1_DVFS_OK_VOTE_EN_OFFSET                                  0x44
#define DSCW1_DVFS_OK_VOTE_EN_SHIFT                                   0
#define DSCW1_DVFS_OK_VOTE_EN_LENGTH                                  1
#define DSCW1_DVFS_OK_VOTE_TH_SHIFT                                   16
#define DSCW1_DVFS_OK_VOTE_TH_LENGTH                                  16

/* REGISTER disp_glb_reg_18 */
#define DDR_DVFS_INT_EN_OFFSET                                        0x48
#define DDR_DVFS_INT_EN_SHIFT                                         0
#define DDR_DVFS_INT_EN_LENGTH                                        4

/* REGISTER disp_glb_reg_19 */
#define DDR_DVFS_INT_RAW_OFFSET                                       0x4c
#define DDR_DVFS_INT_RAW_SHIFT                                        0
#define DDR_DVFS_INT_RAW_LENGTH                                       4

/* REGISTER disp_glb_reg_20 */
#define DDR_DVFS_INT_O_OFFSET                                         0x50
#define DDR_DVFS_INT_O_SHIFT                                          0
#define DDR_DVFS_INT_O_LENGTH                                         4


/* REGISTER disp_glb_reg_23 */
#define DPU_P0_TPC_BWC_STATIC_R_EN_OFFSET                             0x5c
#define DPU_P0_TPC_BWC_STATIC_R_EN_SHIFT                              0
#define DPU_P0_TPC_BWC_STATIC_R_EN_LENGTH                             1
#define DPU_P0_TPC_BWC_STATIC_W_EN_SHIFT                              1
#define DPU_P0_TPC_BWC_STATIC_W_EN_LENGTH                             1

/* REGISTER disp_glb_reg_24 */
#define DPU_P0_TPC_BWC_STATIC_R_TARGET_OFFSET                         0x60
#define DPU_P0_TPC_BWC_STATIC_R_TARGET_SHIFT                          0
#define DPU_P0_TPC_BWC_STATIC_R_TARGET_LENGTH                         8
#define DPU_P0_TPC_BWC_STATIC_W_TARGET_SHIFT                          8
#define DPU_P0_TPC_BWC_STATIC_W_TARGET_LENGTH                         8

/* REGISTER disp_glb_reg_25 */
#define DPU_P0_TPC_BWC_STATIC_R_SAT_OFFSET                            0x64
#define DPU_P0_TPC_BWC_STATIC_R_SAT_SHIFT                             0
#define DPU_P0_TPC_BWC_STATIC_R_SAT_LENGTH                            16
#define DPU_P0_TPC_BWC_STATIC_W_SAT_SHIFT                             16
#define DPU_P0_TPC_BWC_STATIC_W_SAT_LENGTH                            16

/* REGISTER disp_glb_reg_26 */
#define DPU_P0_TPC_QOS_0_OFFSET                                       0x68
#define DPU_P0_TPC_QOS_0_SHIFT                                        0
#define DPU_P0_TPC_QOS_0_LENGTH                                       4
#define DPU_P0_TPC_QOS_1_SHIFT                                        4
#define DPU_P0_TPC_QOS_1_LENGTH                                       4
#define DPU_P0_TPC_QOS_2_SHIFT                                        8
#define DPU_P0_TPC_QOS_2_LENGTH                                       4
#define DPU_P0_TPC_QOS_3_SHIFT                                        12
#define DPU_P0_TPC_QOS_3_LENGTH                                       4
#define DPU_P0_TPC_QOS_4_SHIFT                                        16
#define DPU_P0_TPC_QOS_4_LENGTH                                       4
#define DPU_P0_TPC_QOS_5_SHIFT                                        20
#define DPU_P0_TPC_QOS_5_LENGTH                                       4
#define DPU_P0_TPC_QOS_6_SHIFT                                        24
#define DPU_P0_TPC_QOS_6_LENGTH                                       4
#define DPU_P0_TPC_QOS_7_SHIFT                                        28
#define DPU_P0_TPC_QOS_7_LENGTH                                       4


/* REGISTER disp_glb_reg_29 */
#define DPU_P1_TPC_BWC_STATIC_R_EN_OFFSET                             0x74
#define DPU_P1_TPC_BWC_STATIC_R_EN_SHIFT                              0
#define DPU_P1_TPC_BWC_STATIC_R_EN_LENGTH                             1
#define DPU_P1_TPC_BWC_STATIC_W_EN_SHIFT                              1
#define DPU_P1_TPC_BWC_STATIC_W_EN_LENGTH                             1

/* REGISTER disp_glb_reg_30 */
#define DPU_P1_TPC_BWC_STATIC_R_TARGET_OFFSET                         0x78
#define DPU_P1_TPC_BWC_STATIC_R_TARGET_SHIFT                          0
#define DPU_P1_TPC_BWC_STATIC_R_TARGET_LENGTH                         8
#define DPU_P1_TPC_BWC_STATIC_W_TARGET_SHIFT                          8
#define DPU_P1_TPC_BWC_STATIC_W_TARGET_LENGTH                         8

/* REGISTER disp_glb_reg_31 */
#define DPU_P1_TPC_BWC_STATIC_R_SAT_OFFSET                            0x7c
#define DPU_P1_TPC_BWC_STATIC_R_SAT_SHIFT                             0
#define DPU_P1_TPC_BWC_STATIC_R_SAT_LENGTH                            16
#define DPU_P1_TPC_BWC_STATIC_W_SAT_SHIFT                             16
#define DPU_P1_TPC_BWC_STATIC_W_SAT_LENGTH                            16

/* REGISTER disp_glb_reg_32 */
#define DPU_P1_TPC_QOS_0_OFFSET                                       0x80
#define DPU_P1_TPC_QOS_0_SHIFT                                        0
#define DPU_P1_TPC_QOS_0_LENGTH                                       4
#define DPU_P1_TPC_QOS_1_SHIFT                                        4
#define DPU_P1_TPC_QOS_1_LENGTH                                       4
#define DPU_P1_TPC_QOS_2_SHIFT                                        8
#define DPU_P1_TPC_QOS_2_LENGTH                                       4
#define DPU_P1_TPC_QOS_3_SHIFT                                        12
#define DPU_P1_TPC_QOS_3_LENGTH                                       4
#define DPU_P1_TPC_QOS_4_SHIFT                                        16
#define DPU_P1_TPC_QOS_4_LENGTH                                       4
#define DPU_P1_TPC_QOS_5_SHIFT                                        20
#define DPU_P1_TPC_QOS_5_LENGTH                                       4
#define DPU_P1_TPC_QOS_6_SHIFT                                        24
#define DPU_P1_TPC_QOS_6_LENGTH                                       4
#define DPU_P1_TPC_QOS_7_SHIFT                                        28
#define DPU_P1_TPC_QOS_7_LENGTH                                       4


/* REGISTER disp_glb_reg_35 */
#define DPU_P2_TPC_BWC_STATIC_R_EN_OFFSET                             0x8c
#define DPU_P2_TPC_BWC_STATIC_R_EN_SHIFT                              0
#define DPU_P2_TPC_BWC_STATIC_R_EN_LENGTH                             1
#define DPU_P2_TPC_BWC_STATIC_W_EN_SHIFT                              1
#define DPU_P2_TPC_BWC_STATIC_W_EN_LENGTH                             1

/* REGISTER disp_glb_reg_36 */
#define DPU_P2_TPC_BWC_STATIC_R_TARGET_OFFSET                         0x90
#define DPU_P2_TPC_BWC_STATIC_R_TARGET_SHIFT                          0
#define DPU_P2_TPC_BWC_STATIC_R_TARGET_LENGTH                         8
#define DPU_P2_TPC_BWC_STATIC_W_TARGET_SHIFT                          8
#define DPU_P2_TPC_BWC_STATIC_W_TARGET_LENGTH                         8

/* REGISTER disp_glb_reg_37 */
#define DPU_P2_TPC_BWC_STATIC_R_SAT_OFFSET                            0x94
#define DPU_P2_TPC_BWC_STATIC_R_SAT_SHIFT                             0
#define DPU_P2_TPC_BWC_STATIC_R_SAT_LENGTH                            16
#define DPU_P2_TPC_BWC_STATIC_W_SAT_SHIFT                             16
#define DPU_P2_TPC_BWC_STATIC_W_SAT_LENGTH                            16

/* REGISTER disp_glb_reg_38 */
#define DPU_P2_TPC_QOS_0_OFFSET                                       0x98
#define DPU_P2_TPC_QOS_0_SHIFT                                        0
#define DPU_P2_TPC_QOS_0_LENGTH                                       4
#define DPU_P2_TPC_QOS_1_SHIFT                                        4
#define DPU_P2_TPC_QOS_1_LENGTH                                       4
#define DPU_P2_TPC_QOS_2_SHIFT                                        8
#define DPU_P2_TPC_QOS_2_LENGTH                                       4
#define DPU_P2_TPC_QOS_3_SHIFT                                        12
#define DPU_P2_TPC_QOS_3_LENGTH                                       4
#define DPU_P2_TPC_QOS_4_SHIFT                                        16
#define DPU_P2_TPC_QOS_4_LENGTH                                       4
#define DPU_P2_TPC_QOS_5_SHIFT                                        20
#define DPU_P2_TPC_QOS_5_LENGTH                                       4
#define DPU_P2_TPC_QOS_6_SHIFT                                        24
#define DPU_P2_TPC_QOS_6_LENGTH                                       4
#define DPU_P2_TPC_QOS_7_SHIFT                                        28
#define DPU_P2_TPC_QOS_7_LENGTH                                       4


/* REGISTER disp_glb_reg_41 */
#define DPU_P3_TPC_BWC_STATIC_R_EN_OFFSET                             0xa4
#define DPU_P3_TPC_BWC_STATIC_R_EN_SHIFT                              0
#define DPU_P3_TPC_BWC_STATIC_R_EN_LENGTH                             1
#define DPU_P3_TPC_BWC_STATIC_W_EN_SHIFT                              1
#define DPU_P3_TPC_BWC_STATIC_W_EN_LENGTH                             1

/* REGISTER disp_glb_reg_42 */
#define DPU_P3_TPC_BWC_STATIC_R_TARGET_OFFSET                         0xa8
#define DPU_P3_TPC_BWC_STATIC_R_TARGET_SHIFT                          0
#define DPU_P3_TPC_BWC_STATIC_R_TARGET_LENGTH                         8
#define DPU_P3_TPC_BWC_STATIC_W_TARGET_SHIFT                          8
#define DPU_P3_TPC_BWC_STATIC_W_TARGET_LENGTH                         8

/* REGISTER disp_glb_reg_43 */
#define DPU_P3_TPC_BWC_STATIC_R_SAT_OFFSET                            0xac
#define DPU_P3_TPC_BWC_STATIC_R_SAT_SHIFT                             0
#define DPU_P3_TPC_BWC_STATIC_R_SAT_LENGTH                            16
#define DPU_P3_TPC_BWC_STATIC_W_SAT_SHIFT                             16
#define DPU_P3_TPC_BWC_STATIC_W_SAT_LENGTH                            16

/* REGISTER disp_glb_reg_44 */
#define DPU_P3_TPC_QOS_0_OFFSET                                       0xb0
#define DPU_P3_TPC_QOS_0_SHIFT                                        0
#define DPU_P3_TPC_QOS_0_LENGTH                                       4
#define DPU_P3_TPC_QOS_1_SHIFT                                        4
#define DPU_P3_TPC_QOS_1_LENGTH                                       4
#define DPU_P3_TPC_QOS_2_SHIFT                                        8
#define DPU_P3_TPC_QOS_2_LENGTH                                       4
#define DPU_P3_TPC_QOS_3_SHIFT                                        12
#define DPU_P3_TPC_QOS_3_LENGTH                                       4
#define DPU_P3_TPC_QOS_4_SHIFT                                        16
#define DPU_P3_TPC_QOS_4_LENGTH                                       4
#define DPU_P3_TPC_QOS_5_SHIFT                                        20
#define DPU_P3_TPC_QOS_5_LENGTH                                       4
#define DPU_P3_TPC_QOS_6_SHIFT                                        24
#define DPU_P3_TPC_QOS_6_LENGTH                                       4
#define DPU_P3_TPC_QOS_7_SHIFT                                        28
#define DPU_P3_TPC_QOS_7_LENGTH                                       4


/* REGISTER disp_glb_reg_47 */
#define DPU_P4_TPC_BWC_STATIC_R_EN_OFFSET                             0xbc
#define DPU_P4_TPC_BWC_STATIC_R_EN_SHIFT                              0
#define DPU_P4_TPC_BWC_STATIC_R_EN_LENGTH                             1
#define DPU_P4_TPC_BWC_STATIC_W_EN_SHIFT                              1
#define DPU_P4_TPC_BWC_STATIC_W_EN_LENGTH                             1

/* REGISTER disp_glb_reg_48 */
#define DPU_P4_TPC_BWC_STATIC_R_TARGET_OFFSET                         0xc0
#define DPU_P4_TPC_BWC_STATIC_R_TARGET_SHIFT                          0
#define DPU_P4_TPC_BWC_STATIC_R_TARGET_LENGTH                         8
#define DPU_P4_TPC_BWC_STATIC_W_TARGET_SHIFT                          8
#define DPU_P4_TPC_BWC_STATIC_W_TARGET_LENGTH                         8

/* REGISTER disp_glb_reg_49 */
#define DPU_P4_TPC_BWC_STATIC_R_SAT_OFFSET                            0xc4
#define DPU_P4_TPC_BWC_STATIC_R_SAT_SHIFT                             0
#define DPU_P4_TPC_BWC_STATIC_R_SAT_LENGTH                            16
#define DPU_P4_TPC_BWC_STATIC_W_SAT_SHIFT                             16
#define DPU_P4_TPC_BWC_STATIC_W_SAT_LENGTH                            16

/* REGISTER disp_glb_reg_50 */
#define DPU_P4_TPC_QOS_0_OFFSET                                       0xc8
#define DPU_P4_TPC_QOS_0_SHIFT                                        0
#define DPU_P4_TPC_QOS_0_LENGTH                                       4
#define DPU_P4_TPC_QOS_1_SHIFT                                        4
#define DPU_P4_TPC_QOS_1_LENGTH                                       4
#define DPU_P4_TPC_QOS_2_SHIFT                                        8
#define DPU_P4_TPC_QOS_2_LENGTH                                       4
#define DPU_P4_TPC_QOS_3_SHIFT                                        12
#define DPU_P4_TPC_QOS_3_LENGTH                                       4
#define DPU_P4_TPC_QOS_4_SHIFT                                        16
#define DPU_P4_TPC_QOS_4_LENGTH                                       4
#define DPU_P4_TPC_QOS_5_SHIFT                                        20
#define DPU_P4_TPC_QOS_5_LENGTH                                       4
#define DPU_P4_TPC_QOS_6_SHIFT                                        24
#define DPU_P4_TPC_QOS_6_LENGTH                                       4
#define DPU_P4_TPC_QOS_7_SHIFT                                        28
#define DPU_P4_TPC_QOS_7_LENGTH                                       4


/* REGISTER disp_glb_reg_53 */
#define VEU_P0_TPC_BWC_STATIC_R_EN_OFFSET                             0xd4
#define VEU_P0_TPC_BWC_STATIC_R_EN_SHIFT                              0
#define VEU_P0_TPC_BWC_STATIC_R_EN_LENGTH                             1
#define VEU_P0_TPC_BWC_STATIC_W_EN_SHIFT                              1
#define VEU_P0_TPC_BWC_STATIC_W_EN_LENGTH                             1

/* REGISTER disp_glb_reg_54 */
#define VEU_P0_TPC_BWC_STATIC_R_TARGET_OFFSET                         0xd8
#define VEU_P0_TPC_BWC_STATIC_R_TARGET_SHIFT                          0
#define VEU_P0_TPC_BWC_STATIC_R_TARGET_LENGTH                         8
#define VEU_P0_TPC_BWC_STATIC_W_TARGET_SHIFT                          8
#define VEU_P0_TPC_BWC_STATIC_W_TARGET_LENGTH                         8

/* REGISTER disp_glb_reg_55 */
#define VEU_P0_TPC_BWC_STATIC_R_SAT_OFFSET                            0xdc
#define VEU_P0_TPC_BWC_STATIC_R_SAT_SHIFT                             0
#define VEU_P0_TPC_BWC_STATIC_R_SAT_LENGTH                            16
#define VEU_P0_TPC_BWC_STATIC_W_SAT_SHIFT                             16
#define VEU_P0_TPC_BWC_STATIC_W_SAT_LENGTH                            16

/* REGISTER disp_glb_reg_56 */
#define VEU_P0_TPC_QOS_0_OFFSET                                       0xe0
#define VEU_P0_TPC_QOS_0_SHIFT                                        0
#define VEU_P0_TPC_QOS_0_LENGTH                                       4
#define VEU_P0_TPC_QOS_1_SHIFT                                        4
#define VEU_P0_TPC_QOS_1_LENGTH                                       4
#define VEU_P0_TPC_QOS_2_SHIFT                                        8
#define VEU_P0_TPC_QOS_2_LENGTH                                       4
#define VEU_P0_TPC_QOS_3_SHIFT                                        12
#define VEU_P0_TPC_QOS_3_LENGTH                                       4
#define VEU_P0_TPC_QOS_4_SHIFT                                        16
#define VEU_P0_TPC_QOS_4_LENGTH                                       4
#define VEU_P0_TPC_QOS_5_SHIFT                                        20
#define VEU_P0_TPC_QOS_5_LENGTH                                       4
#define VEU_P0_TPC_QOS_6_SHIFT                                        24
#define VEU_P0_TPC_QOS_6_LENGTH                                       4
#define VEU_P0_TPC_QOS_7_SHIFT                                        28
#define VEU_P0_TPC_QOS_7_LENGTH                                       4


/* REGISTER disp_glb_reg_59 */
#define TEST_TRIGGER_PIN_FORCE_OFFSET                                 0xec

/* REGISTER disp_glb_reg_98 */
#define TEST_TRIGGER_PIN_MASK_OFFSET                                  0x188

/* REGISTER disp_glb_reg_137 */
#define TEST_TRIGGER_SW1_OFFSET                                       0x224

/* REGISTER disp_glb_reg_328 */
#define TEST_TRIGGER_SW2_OFFSET                                       0x520
#define TEST_TRIGGER_SW2_SHIFT                                        0
#define TEST_TRIGGER_SW2_LENGTH                                       7

/* REGISTER disp_glb_reg_519 */
#define TEST_TRIGGER_SW3_OFFSET                                       0x81c

/* REGISTER disp_glb_reg_710 */
#define TEST_TRIGGER_SW4_OFFSET                                       0xb18
#define TEST_TRIGGER_SW4_SHIFT                                        0
#define TEST_TRIGGER_SW4_LENGTH                                       7

/* REGISTER disp_glb_reg_901 */
#define CAP_CROSS_TRIGCNT_TARGET_OFFSET                               0xe14
#define CAP_CROSS_TRIGCNT_TARGET_SHIFT                                0
#define CAP_CROSS_TRIGCNT_TARGET_LENGTH                               4

/* REGISTER disp_glb_reg_909 */
#define CAP_CROSS_SEL_OFFSET                                          0xe34
#define CAP_CROSS_SEL_SHIFT                                           0
#define CAP_CROSS_SEL_LENGTH                                          24

/* REGISTER disp_glb_reg_910 */
#define TEST_DFX_EN_OFFSET                                            0xe38
#define TEST_DFX_EN_SHIFT                                             0
#define TEST_DFX_EN_LENGTH                                            1


/* REGISTER disp_glb_reg_912 */
#define DISP_GLB_ICG_OVERRIDE_OFFSET                                  0xe40
#define DISP_GLB_ICG_OVERRIDE_SHIFT                                   0
#define DISP_GLB_ICG_OVERRIDE_LENGTH                                  1

/* REGISTER disp_glb_reg_913 */
#define TEST_CAP_MASK_PAST_OFFSET                                     0xe44

/* REGISTER disp_glb_reg_1104 */
#define TEST_CAP_PIN_0_SYNC_OFFSET                                    0x1140

/* REGISTER disp_glb_reg_1105 */
#define TEST_CAP_PIN_1_SYNC_OFFSET                                    0x1144

/* REGISTER disp_glb_reg_1106 */
#define TEST_CAP_PIN_2_SYNC_OFFSET                                    0x1148

/* REGISTER disp_glb_reg_1107 */
#define TEST_CAP_PIN_3_SYNC_OFFSET                                    0x114c

/* REGISTER disp_glb_reg_1108 */
#define TEST_CAP_PIN_4_SYNC_OFFSET                                    0x1150

/* REGISTER disp_glb_reg_1109 */
#define TEST_CAP_PIN_5_SYNC_OFFSET                                    0x1154

/* REGISTER disp_glb_reg_1110 */
#define TEST_CAP_PIN_6_SYNC_OFFSET                                    0x1158

/* REGISTER disp_glb_reg_1111 */
#define TEST_CAP_PIN_7_SYNC_OFFSET                                    0x115c

/* REGISTER disp_glb_reg_1112 */
#define TEST_CAP_PIN_8_SYNC_OFFSET                                    0x1160

/* REGISTER disp_glb_reg_1113 */
#define TEST_CAP_PIN_9_SYNC_OFFSET                                    0x1164

/* REGISTER disp_glb_reg_1114 */
#define TEST_CAP_PIN_10_SYNC_OFFSET                                   0x1168

/* REGISTER disp_glb_reg_1115 */
#define TEST_CAP_PIN_11_SYNC_OFFSET                                   0x116c

/* REGISTER disp_glb_reg_1116 */
#define TEST_CAP_PIN_12_SYNC_OFFSET                                   0x1170

/* REGISTER disp_glb_reg_1117 */
#define TEST_CAP_PIN_13_SYNC_OFFSET                                   0x1174

/* REGISTER disp_glb_reg_1118 */
#define TEST_CAP_PIN_14_SYNC_OFFSET                                   0x1178

/* REGISTER disp_glb_reg_1119 */
#define TEST_CAP_PIN_15_SYNC_OFFSET                                   0x117c

/* REGISTER disp_glb_reg_1120 */
#define TEST_CAP_PIN_16_SYNC_OFFSET                                   0x1180

/* REGISTER disp_glb_reg_1121 */
#define TEST_CAP_PIN_17_SYNC_OFFSET                                   0x1184

/* REGISTER disp_glb_reg_1122 */
#define TEST_CAP_PIN_18_SYNC_OFFSET                                   0x1188

/* REGISTER disp_glb_reg_1123 */
#define TEST_CAP_PIN_19_SYNC_OFFSET                                   0x118c

/* REGISTER disp_glb_reg_1124 */
#define TEST_CAP_PIN_20_SYNC_OFFSET                                   0x1190

/* REGISTER disp_glb_reg_1125 */
#define TEST_CAP_PIN_21_SYNC_OFFSET                                   0x1194

/* REGISTER disp_glb_reg_1126 */
#define TEST_CAP_PIN_22_SYNC_OFFSET                                   0x1198

/* REGISTER disp_glb_reg_1127 */
#define TEST_CAP_PIN_23_SYNC_OFFSET                                   0x119c

/* REGISTER disp_glb_reg_1128 */
#define TEST_CAP_PIN_24_SYNC_OFFSET                                   0x11a0

/* REGISTER disp_glb_reg_1129 */
#define TEST_CAP_PIN_25_SYNC_OFFSET                                   0x11a4

/* REGISTER disp_glb_reg_1130 */
#define TEST_CAP_PIN_26_SYNC_OFFSET                                   0x11a8

/* REGISTER disp_glb_reg_1131 */
#define TEST_CAP_PIN_27_SYNC_OFFSET                                   0x11ac

/* REGISTER disp_glb_reg_1132 */
#define TEST_CAP_PIN_28_SYNC_OFFSET                                   0x11b0

/* REGISTER disp_glb_reg_1133 */
#define TEST_CAP_PIN_29_SYNC_OFFSET                                   0x11b4

/* REGISTER disp_glb_reg_1134 */
#define TEST_CAP_PIN_30_SYNC_OFFSET                                   0x11b8

/* REGISTER disp_glb_reg_1135 */
#define TEST_CAP_PIN_31_SYNC_OFFSET                                   0x11bc

/* REGISTER disp_glb_reg_1136 */
#define TEST_CAP_PIN_32_SYNC_OFFSET                                   0x11c0

/* REGISTER disp_glb_reg_1137 */
#define TEST_CAP_PIN_33_SYNC_OFFSET                                   0x11c4

/* REGISTER disp_glb_reg_1138 */
#define TEST_CAP_PIN_34_SYNC_OFFSET                                   0x11c8

/* REGISTER disp_glb_reg_1139 */
#define TEST_CAP_PIN_35_SYNC_OFFSET                                   0x11cc

/* REGISTER disp_glb_reg_1140 */
#define TEST_CAP_PIN_36_SYNC_OFFSET                                   0x11d0

/* REGISTER disp_glb_reg_1141 */
#define TEST_CAP_PIN_37_SYNC_OFFSET                                   0x11d4

/* REGISTER disp_glb_reg_1142 */
#define TEST_CAP_PIN_38_SYNC_OFFSET                                   0x11d8

/* REGISTER disp_glb_reg_1143 */
#define TEST_CAP_PIN_39_SYNC_OFFSET                                   0x11dc

/* REGISTER disp_glb_reg_1144 */
#define TEST_CAP_PIN_40_SYNC_OFFSET                                   0x11e0

/* REGISTER disp_glb_reg_1145 */
#define TEST_CAP_PIN_41_SYNC_OFFSET                                   0x11e4

/* REGISTER disp_glb_reg_1146 */
#define TEST_CAP_PIN_42_SYNC_OFFSET                                   0x11e8

/* REGISTER disp_glb_reg_1147 */
#define TEST_CAP_PIN_43_SYNC_OFFSET                                   0x11ec

/* REGISTER disp_glb_reg_1148 */
#define TEST_CAP_PIN_44_SYNC_OFFSET                                   0x11f0

/* REGISTER disp_glb_reg_1149 */
#define TEST_CAP_PIN_45_SYNC_OFFSET                                   0x11f4

/* REGISTER disp_glb_reg_1150 */
#define TEST_CAP_PIN_46_SYNC_OFFSET                                   0x11f8

/* REGISTER disp_glb_reg_1151 */
#define TEST_CAP_PIN_47_SYNC_OFFSET                                   0x11fc

/* REGISTER disp_glb_reg_1152 */
#define TEST_CAP_PIN_48_SYNC_OFFSET                                   0x1200

/* REGISTER disp_glb_reg_1153 */
#define TEST_CAP_PIN_49_SYNC_OFFSET                                   0x1204

/* REGISTER disp_glb_reg_1154 */
#define TEST_CAP_PIN_50_SYNC_OFFSET                                   0x1208

/* REGISTER disp_glb_reg_1155 */
#define TEST_CAP_PIN_51_SYNC_OFFSET                                   0x120c

/* REGISTER disp_glb_reg_1156 */
#define TEST_CAP_PIN_52_SYNC_OFFSET                                   0x1210

/* REGISTER disp_glb_reg_1157 */
#define TEST_CAP_PIN_53_SYNC_OFFSET                                   0x1214

/* REGISTER disp_glb_reg_1158 */
#define TEST_CAP_PIN_54_SYNC_OFFSET                                   0x1218

/* REGISTER disp_glb_reg_1159 */
#define TEST_CAP_PIN_55_SYNC_OFFSET                                   0x121c

/* REGISTER disp_glb_reg_1160 */
#define TEST_CAP_PIN_56_SYNC_OFFSET                                   0x1220

/* REGISTER disp_glb_reg_1161 */
#define TEST_CAP_PIN_57_SYNC_OFFSET                                   0x1224

/* REGISTER disp_glb_reg_1162 */
#define TEST_CAP_PIN_58_SYNC_OFFSET                                   0x1228

/* REGISTER disp_glb_reg_1163 */
#define TEST_CAP_PIN_59_SYNC_OFFSET                                   0x122c

/* REGISTER disp_glb_reg_1164 */
#define TEST_CAP_PIN_60_SYNC_OFFSET                                   0x1230

/* REGISTER disp_glb_reg_1165 */
#define TEST_CAP_PIN_61_SYNC_OFFSET                                   0x1234

/* REGISTER disp_glb_reg_1166 */
#define TEST_CAP_PIN_62_SYNC_OFFSET                                   0x1238

/* REGISTER disp_glb_reg_1167 */
#define TEST_CAP_PIN_63_SYNC_OFFSET                                   0x123c

/* REGISTER disp_glb_reg_1168 */
#define TEST_CAP_PIN_64_SYNC_OFFSET                                   0x1240

/* REGISTER disp_glb_reg_1169 */
#define TEST_CAP_PIN_65_SYNC_OFFSET                                   0x1244

/* REGISTER disp_glb_reg_1170 */
#define TEST_CAP_PIN_66_SYNC_OFFSET                                   0x1248

/* REGISTER disp_glb_reg_1171 */
#define TEST_CAP_PIN_67_SYNC_OFFSET                                   0x124c

/* REGISTER disp_glb_reg_1172 */
#define TEST_CAP_PIN_68_SYNC_OFFSET                                   0x1250

/* REGISTER disp_glb_reg_1173 */
#define TEST_CAP_PIN_69_SYNC_OFFSET                                   0x1254

/* REGISTER disp_glb_reg_1174 */
#define TEST_CAP_PIN_70_SYNC_OFFSET                                   0x1258

/* REGISTER disp_glb_reg_1175 */
#define TEST_CAP_PIN_71_SYNC_OFFSET                                   0x125c

/* REGISTER disp_glb_reg_1176 */
#define TEST_CAP_PIN_72_SYNC_OFFSET                                   0x1260

/* REGISTER disp_glb_reg_1177 */
#define TEST_CAP_PIN_73_SYNC_OFFSET                                   0x1264

/* REGISTER disp_glb_reg_1178 */
#define TEST_CAP_PIN_74_SYNC_OFFSET                                   0x1268

/* REGISTER disp_glb_reg_1179 */
#define TEST_CAP_PIN_75_SYNC_OFFSET                                   0x126c

/* REGISTER disp_glb_reg_1180 */
#define TEST_CAP_PIN_76_SYNC_OFFSET                                   0x1270

/* REGISTER disp_glb_reg_1181 */
#define TEST_CAP_PIN_77_SYNC_OFFSET                                   0x1274

/* REGISTER disp_glb_reg_1182 */
#define TEST_CAP_PIN_78_SYNC_OFFSET                                   0x1278

/* REGISTER disp_glb_reg_1183 */
#define TEST_CAP_PIN_79_SYNC_OFFSET                                   0x127c

/* REGISTER disp_glb_reg_1184 */
#define TEST_CAP_PIN_80_SYNC_OFFSET                                   0x1280

/* REGISTER disp_glb_reg_1185 */
#define TEST_CAP_PIN_81_SYNC_OFFSET                                   0x1284

/* REGISTER disp_glb_reg_1186 */
#define TEST_CAP_PIN_82_SYNC_OFFSET                                   0x1288

/* REGISTER disp_glb_reg_1187 */
#define TEST_CAP_PIN_83_SYNC_OFFSET                                   0x128c

/* REGISTER disp_glb_reg_1188 */
#define TEST_CAP_PIN_84_SYNC_OFFSET                                   0x1290

/* REGISTER disp_glb_reg_1189 */
#define TEST_CAP_PIN_85_SYNC_OFFSET                                   0x1294

/* REGISTER disp_glb_reg_1190 */
#define TEST_CAP_PIN_86_SYNC_OFFSET                                   0x1298

/* REGISTER disp_glb_reg_1191 */
#define TEST_CAP_PIN_87_SYNC_OFFSET                                   0x129c

/* REGISTER disp_glb_reg_1192 */
#define TEST_CAP_PIN_88_SYNC_OFFSET                                   0x12a0

/* REGISTER disp_glb_reg_1193 */
#define TEST_CAP_PIN_89_SYNC_OFFSET                                   0x12a4

/* REGISTER disp_glb_reg_1194 */
#define TEST_CAP_PIN_90_SYNC_OFFSET                                   0x12a8

/* REGISTER disp_glb_reg_1195 */
#define TEST_CAP_PIN_91_SYNC_OFFSET                                   0x12ac

/* REGISTER disp_glb_reg_1196 */
#define TEST_CAP_PIN_92_SYNC_OFFSET                                   0x12b0

/* REGISTER disp_glb_reg_1197 */
#define TEST_CAP_PIN_93_SYNC_OFFSET                                   0x12b4

/* REGISTER disp_glb_reg_1198 */
#define TEST_CAP_PIN_94_SYNC_OFFSET                                   0x12b8

/* REGISTER disp_glb_reg_1199 */
#define TEST_CAP_PIN_95_SYNC_OFFSET                                   0x12bc

/* REGISTER disp_glb_reg_1200 */
#define TEST_CAP_PIN_96_SYNC_OFFSET                                   0x12c0

/* REGISTER disp_glb_reg_1201 */
#define TEST_CAP_PIN_97_SYNC_OFFSET                                   0x12c4

/* REGISTER disp_glb_reg_1202 */
#define TEST_CAP_PIN_98_SYNC_OFFSET                                   0x12c8

/* REGISTER disp_glb_reg_1203 */
#define TEST_CAP_PIN_99_SYNC_OFFSET                                   0x12cc

/* REGISTER disp_glb_reg_1204 */
#define TEST_CAP_PIN_100_SYNC_OFFSET                                  0x12d0

/* REGISTER disp_glb_reg_1205 */
#define TEST_CAP_PIN_101_SYNC_OFFSET                                  0x12d4

/* REGISTER disp_glb_reg_1206 */
#define TEST_CAP_PIN_102_SYNC_OFFSET                                  0x12d8

/* REGISTER disp_glb_reg_1207 */
#define TEST_CAP_PIN_103_SYNC_OFFSET                                  0x12dc

/* REGISTER disp_glb_reg_1208 */
#define TEST_CAP_PIN_104_SYNC_OFFSET                                  0x12e0

/* REGISTER disp_glb_reg_1209 */
#define TEST_CAP_PIN_105_SYNC_OFFSET                                  0x12e4

/* REGISTER disp_glb_reg_1210 */
#define TEST_CAP_PIN_106_SYNC_OFFSET                                  0x12e8

/* REGISTER disp_glb_reg_1211 */
#define TEST_CAP_PIN_107_SYNC_OFFSET                                  0x12ec

/* REGISTER disp_glb_reg_1212 */
#define TEST_CAP_PIN_108_SYNC_OFFSET                                  0x12f0

/* REGISTER disp_glb_reg_1213 */
#define TEST_CAP_PIN_109_SYNC_OFFSET                                  0x12f4

/* REGISTER disp_glb_reg_1214 */
#define TEST_CAP_PIN_110_SYNC_OFFSET                                  0x12f8

/* REGISTER disp_glb_reg_1215 */
#define TEST_CAP_PIN_111_SYNC_OFFSET                                  0x12fc

/* REGISTER disp_glb_reg_1216 */
#define TEST_CAP_PIN_112_SYNC_OFFSET                                  0x1300

/* REGISTER disp_glb_reg_1217 */
#define TEST_CAP_PIN_113_SYNC_OFFSET                                  0x1304

/* REGISTER disp_glb_reg_1218 */
#define TEST_CAP_PIN_114_SYNC_OFFSET                                  0x1308

/* REGISTER disp_glb_reg_1219 */
#define TEST_CAP_PIN_115_SYNC_OFFSET                                  0x130c

/* REGISTER disp_glb_reg_1220 */
#define TEST_CAP_PIN_116_SYNC_OFFSET                                  0x1310

/* REGISTER disp_glb_reg_1221 */
#define TEST_CAP_PIN_117_SYNC_OFFSET                                  0x1314

/* REGISTER disp_glb_reg_1222 */
#define TEST_CAP_PIN_118_SYNC_OFFSET                                  0x1318

/* REGISTER disp_glb_reg_1223 */
#define TEST_CAP_PIN_119_SYNC_OFFSET                                  0x131c

/* REGISTER disp_glb_reg_1224 */
#define TEST_CAP_PIN_120_SYNC_OFFSET                                  0x1320

/* REGISTER disp_glb_reg_1225 */
#define TEST_CAP_PIN_121_SYNC_OFFSET                                  0x1324

/* REGISTER disp_glb_reg_1226 */
#define TEST_CAP_PIN_122_SYNC_OFFSET                                  0x1328

/* REGISTER disp_glb_reg_1227 */
#define TEST_CAP_PIN_123_SYNC_OFFSET                                  0x132c

/* REGISTER disp_glb_reg_1228 */
#define TEST_CAP_PIN_124_SYNC_OFFSET                                  0x1330

/* REGISTER disp_glb_reg_1229 */
#define TEST_CAP_PIN_125_SYNC_OFFSET                                  0x1334

/* REGISTER disp_glb_reg_1230 */
#define TEST_CAP_PIN_126_SYNC_OFFSET                                  0x1338

/* REGISTER disp_glb_reg_1231 */
#define TEST_CAP_PIN_127_SYNC_OFFSET                                  0x133c

/* REGISTER disp_glb_reg_1232 */
#define TEST_CAP_PIN_128_SYNC_OFFSET                                  0x1340

/* REGISTER disp_glb_reg_1233 */
#define TEST_CAP_PIN_129_SYNC_OFFSET                                  0x1344

/* REGISTER disp_glb_reg_1234 */
#define TEST_CAP_PIN_130_SYNC_OFFSET                                  0x1348

/* REGISTER disp_glb_reg_1235 */
#define TEST_CAP_PIN_131_SYNC_OFFSET                                  0x134c

/* REGISTER disp_glb_reg_1236 */
#define TEST_CAP_PIN_132_SYNC_OFFSET                                  0x1350

/* REGISTER disp_glb_reg_1237 */
#define TEST_CAP_PIN_133_SYNC_OFFSET                                  0x1354

/* REGISTER disp_glb_reg_1238 */
#define TEST_CAP_PIN_134_SYNC_OFFSET                                  0x1358

/* REGISTER disp_glb_reg_1239 */
#define TEST_CAP_PIN_135_SYNC_OFFSET                                  0x135c

/* REGISTER disp_glb_reg_1240 */
#define TEST_CAP_PIN_136_SYNC_OFFSET                                  0x1360

/* REGISTER disp_glb_reg_1241 */
#define TEST_CAP_PIN_137_SYNC_OFFSET                                  0x1364

/* REGISTER disp_glb_reg_1242 */
#define TEST_CAP_PIN_138_SYNC_OFFSET                                  0x1368

/* REGISTER disp_glb_reg_1243 */
#define TEST_CAP_PIN_139_SYNC_OFFSET                                  0x136c

/* REGISTER disp_glb_reg_1244 */
#define TEST_CAP_PIN_140_SYNC_OFFSET                                  0x1370

/* REGISTER disp_glb_reg_1245 */
#define TEST_CAP_PIN_141_SYNC_OFFSET                                  0x1374

/* REGISTER disp_glb_reg_1246 */
#define TEST_CAP_PIN_142_SYNC_OFFSET                                  0x1378

/* REGISTER disp_glb_reg_1247 */
#define TEST_CAP_PIN_143_SYNC_OFFSET                                  0x137c

/* REGISTER disp_glb_reg_1248 */
#define TEST_CAP_PIN_144_SYNC_OFFSET                                  0x1380

/* REGISTER disp_glb_reg_1249 */
#define TEST_CAP_PIN_145_SYNC_OFFSET                                  0x1384

/* REGISTER disp_glb_reg_1250 */
#define TEST_CAP_PIN_146_SYNC_OFFSET                                  0x1388

/* REGISTER disp_glb_reg_1251 */
#define TEST_CAP_PIN_147_SYNC_OFFSET                                  0x138c

/* REGISTER disp_glb_reg_1252 */
#define TEST_CAP_PIN_148_SYNC_OFFSET                                  0x1390

/* REGISTER disp_glb_reg_1253 */
#define TEST_CAP_PIN_149_SYNC_OFFSET                                  0x1394

/* REGISTER disp_glb_reg_1254 */
#define TEST_CAP_PIN_150_SYNC_OFFSET                                  0x1398

/* REGISTER disp_glb_reg_1255 */
#define TEST_CAP_PIN_151_SYNC_OFFSET                                  0x139c

/* REGISTER disp_glb_reg_1256 */
#define TEST_CAP_PIN_152_SYNC_OFFSET                                  0x13a0

/* REGISTER disp_glb_reg_1257 */
#define TEST_CAP_PIN_153_SYNC_OFFSET                                  0x13a4

/* REGISTER disp_glb_reg_1258 */
#define TEST_CAP_PIN_154_SYNC_OFFSET                                  0x13a8

/* REGISTER disp_glb_reg_1259 */
#define TEST_CAP_PIN_155_SYNC_OFFSET                                  0x13ac

/* REGISTER disp_glb_reg_1260 */
#define TEST_CAP_PIN_156_SYNC_OFFSET                                  0x13b0

/* REGISTER disp_glb_reg_1261 */
#define TEST_CAP_PIN_157_SYNC_OFFSET                                  0x13b4

/* REGISTER disp_glb_reg_1262 */
#define TEST_CAP_PIN_158_SYNC_OFFSET                                  0x13b8

/* REGISTER disp_glb_reg_1263 */
#define TEST_CAP_PIN_159_SYNC_OFFSET                                  0x13bc

/* REGISTER disp_glb_reg_1264 */
#define TEST_CAP_PIN_160_SYNC_OFFSET                                  0x13c0

/* REGISTER disp_glb_reg_1265 */
#define TEST_CAP_PIN_161_SYNC_OFFSET                                  0x13c4

/* REGISTER disp_glb_reg_1266 */
#define TEST_CAP_PIN_162_SYNC_OFFSET                                  0x13c8

/* REGISTER disp_glb_reg_1267 */
#define TEST_CAP_PIN_163_SYNC_OFFSET                                  0x13cc

/* REGISTER disp_glb_reg_1268 */
#define TEST_CAP_PIN_164_SYNC_OFFSET                                  0x13d0

/* REGISTER disp_glb_reg_1269 */
#define TEST_CAP_PIN_165_SYNC_OFFSET                                  0x13d4

/* REGISTER disp_glb_reg_1270 */
#define TEST_CAP_PIN_166_SYNC_OFFSET                                  0x13d8

/* REGISTER disp_glb_reg_1271 */
#define TEST_CAP_PIN_167_SYNC_OFFSET                                  0x13dc

/* REGISTER disp_glb_reg_1272 */
#define TEST_CAP_PIN_168_SYNC_OFFSET                                  0x13e0

/* REGISTER disp_glb_reg_1273 */
#define TEST_CAP_PIN_169_SYNC_OFFSET                                  0x13e4

/* REGISTER disp_glb_reg_1274 */
#define TEST_CAP_PIN_170_SYNC_OFFSET                                  0x13e8

/* REGISTER disp_glb_reg_1275 */
#define TEST_CAP_PIN_171_SYNC_OFFSET                                  0x13ec

/* REGISTER disp_glb_reg_1276 */
#define TEST_CAP_PIN_172_SYNC_OFFSET                                  0x13f0

/* REGISTER disp_glb_reg_1277 */
#define TEST_CAP_PIN_173_SYNC_OFFSET                                  0x13f4

/* REGISTER disp_glb_reg_1278 */
#define TEST_CAP_PIN_174_SYNC_OFFSET                                  0x13f8

/* REGISTER disp_glb_reg_1279 */
#define TEST_CAP_PIN_175_SYNC_OFFSET                                  0x13fc

/* REGISTER disp_glb_reg_1280 */
#define TEST_CAP_PIN_176_SYNC_OFFSET                                  0x1400

/* REGISTER disp_glb_reg_1281 */
#define TEST_CAP_PIN_177_SYNC_OFFSET                                  0x1404

/* REGISTER disp_glb_reg_1282 */
#define TEST_CAP_PIN_178_SYNC_OFFSET                                  0x1408

/* REGISTER disp_glb_reg_1283 */
#define TEST_CAP_PIN_179_SYNC_OFFSET                                  0x140c

/* REGISTER disp_glb_reg_1284 */
#define TEST_CAP_PIN_180_SYNC_OFFSET                                  0x1410

/* REGISTER disp_glb_reg_1285 */
#define TEST_CAP_PIN_181_SYNC_OFFSET                                  0x1414

/* REGISTER disp_glb_reg_1286 */
#define TEST_CAP_PIN_182_SYNC_OFFSET                                  0x1418

/* REGISTER disp_glb_reg_1287 */
#define TEST_CAP_PIN_183_SYNC_OFFSET                                  0x141c

/* REGISTER disp_glb_reg_1288 */
#define TEST_CAP_PIN_184_SYNC_OFFSET                                  0x1420

/* REGISTER disp_glb_reg_1289 */
#define TEST_CAP_PIN_185_SYNC_OFFSET                                  0x1424

/* REGISTER disp_glb_reg_1290 */
#define TEST_CAP_PIN_186_SYNC_OFFSET                                  0x1428

/* REGISTER disp_glb_reg_1291 */
#define TEST_CAP_PIN_187_SYNC_OFFSET                                  0x142c

/* REGISTER disp_glb_reg_1292 */
#define TEST_CAP_PIN_188_SYNC_OFFSET                                  0x1430

/* REGISTER disp_glb_reg_1293 */
#define TEST_CAP_PIN_189_SYNC_OFFSET                                  0x1434

/* REGISTER disp_glb_reg_1294 */
#define TEST_CAP_PIN_190_SYNC_OFFSET                                  0x1438

/* REGISTER disp_glb_reg_1295 */
#define TRIG_RECORD1_OFFSET                                           0x143c

/* REGISTER disp_glb_reg_1296 */
#define TRIG_RECORD2_OFFSET                                           0x1440

/* REGISTER disp_glb_reg_1297 */
#define TRIG_RECORD3_OFFSET                                           0x1444

/* REGISTER disp_glb_reg_1298 */
#define TRIG_RECORD4_OFFSET                                           0x1448

/* REGISTER disp_glb_reg_1299 */
#define TRIG_RECORD5_OFFSET                                           0x144c

/* REGISTER disp_glb_reg_1300 */
#define TRIG_RECORD6_OFFSET                                           0x1450
#define TRIG_RECORD6_SHIFT                                            0
#define TRIG_RECORD6_LENGTH                                           31

/* REGISTER disp_glb_reg_1301 */
#define TEST_PIN_OUT_SEL_OFFSET                                       0x1454
#define TEST_PIN_OUT_SEL_SHIFT                                        0
#define TEST_PIN_OUT_SEL_LENGTH                                       8

/* REGISTER disp_glb_reg_1302 */
#define DISP_GLB_FORCE_UPDATE_EN_OFFSET                               0x1458
#define DISP_GLB_FORCE_UPDATE_EN_SHIFT                                0
#define DISP_GLB_FORCE_UPDATE_EN_LENGTH                               1
#define DISP_GLB_VSYNC_UPDATE_EN_SHIFT                                1
#define DISP_GLB_VSYNC_UPDATE_EN_LENGTH                               1
#define DISP_GLB_SHADOW_READ_EN_SHIFT                                 2
#define DISP_GLB_SHADOW_READ_EN_LENGTH                                1

/* REGISTER disp_glb_reg_1303 */
#define DISP_GLB_FORCE_UPDATE_PULSE_OFFSET                            0x145c
#define DISP_GLB_FORCE_UPDATE_PULSE_SHIFT                             0
#define DISP_GLB_FORCE_UPDATE_PULSE_LENGTH                            1

/* REGISTER disp_glb_reg_1304 */
#define CLR_PULSE_OFFSET                                              0x1460
#define CLR_PULSE_SHIFT                                               0
#define CLR_PULSE_LENGTH                                              1

#endif
