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
/* File generation time: 2023-11-24 10:54:05. */

#ifndef _DPU_HW_WDMA_TOP_REG_H_
#define _DPU_HW_WDMA_TOP_REG_H_

/* REGISTER saturn_dpu_wdma_top_reg_0 */
#define WDMA_TIMELIMIT_OFFSET                                         0x00
#define WDMA_TIMELIMIT_SHIFT                                          0
#define WDMA_TIMELIMIT_LENGTH                                         16

/* REGISTER saturn_dpu_wdma_top_reg_1 */
#define DMAC_ID_MODE_OFFSET                                           0x04
#define DMAC_ID_MODE_SHIFT                                            0
#define DMAC_ID_MODE_LENGTH                                           1
#define DMAC_BURST_LENGTH_SHIFT                                       1
#define DMAC_BURST_LENGTH_LENGTH                                      3

/* REGISTER saturn_dpu_wdma_top_reg_2 */
#define DMAC_WR_OUTS_NUM_OFFSET                                       0x08
#define DMAC_WR_OUTS_NUM_SHIFT                                        0
#define DMAC_WR_OUTS_NUM_LENGTH                                       8

/* REGISTER saturn_dpu_wdma_top_reg_3 */
#define WDMA_TOP_MEM_LP_AUTO_EN_OFFSET                                0x0c
#define WDMA_TOP_MEM_LP_AUTO_EN_SHIFT                                 0
#define WDMA_TOP_MEM_LP_AUTO_EN_LENGTH                                4

/* REGISTER saturn_dpu_wdma_top_reg_4 */
#define ARB_DEBUG_INFO_OFFSET                                         0x10

/* REGISTER saturn_dpu_wdma_top_reg_5 */
#define WDMA_DBG_IRQ_MASK_OFFSET                                      0x14

/* REGISTER saturn_dpu_wdma_top_reg_6 */
#define WDMA_DBG_IRQ_STATUS_OFFSET                                    0x18

/* REGISTER saturn_dpu_wdma_top_reg_7 */
#define WDMA_DBG_IRQ_RAW_OFFSET                                       0x1c

/* REGISTER saturn_dpu_wdma_top_reg_8 */
#define WB_IRQ_MASK_OFFSET                                            0x20

/* REGISTER saturn_dpu_wdma_top_reg_9 */
#define WB_IRQ_STATUS_OFFSET                                          0x24

/* REGISTER saturn_dpu_wdma_top_reg_10 */
#define WB_IRQ_RAW_OFFSET                                             0x28

/* REGISTER saturn_dpu_wdma_top_reg_11 */
#define WB01_TPC_OCM_ADDR_RANGE0_32H_OFFSET                           0x2c

/* REGISTER saturn_dpu_wdma_top_reg_12 */
#define WB01_TPC_OCM_ADDR_RANGE0_32L_OFFSET                           0x30

/* REGISTER saturn_dpu_wdma_top_reg_13 */
#define WB01_TPC_OCM_ADDR_RANGE1_32H_OFFSET                           0x34

/* REGISTER saturn_dpu_wdma_top_reg_14 */
#define WB01_TPC_OCM_ADDR_RANGE1_32L_OFFSET                           0x38

/* REGISTER saturn_dpu_wdma_top_reg_15 */
#define WB01_TPC_OCM_ADDR_RANGE2_32H_OFFSET                           0x3c

/* REGISTER saturn_dpu_wdma_top_reg_16 */
#define WB01_TPC_OCM_ADDR_RANGE2_32L_OFFSET                           0x40

/* REGISTER saturn_dpu_wdma_top_reg_17 */
#define WB01_TPC_BWC_DYN_QOS4_W_TARGET_OFFSET                         0x44
#define WB01_TPC_BWC_DYN_QOS4_W_TARGET_SHIFT                          0
#define WB01_TPC_BWC_DYN_QOS4_W_TARGET_LENGTH                         8
#define WB01_TPC_BWC_DYN_QOS5_W_TARGET_SHIFT                          8
#define WB01_TPC_BWC_DYN_QOS5_W_TARGET_LENGTH                         8
#define WB01_TPC_BWC_DYN_QOS6_W_TARGET_SHIFT                          16
#define WB01_TPC_BWC_DYN_QOS6_W_TARGET_LENGTH                         8
#define WB01_TPC_BWC_DYN_QOS7_W_TARGET_SHIFT                          24
#define WB01_TPC_BWC_DYN_QOS7_W_TARGET_LENGTH                         8

/* REGISTER saturn_dpu_wdma_top_reg_18 */
#define WB01_TPC_BWC_DYN_QOS0_W_TARGET_OFFSET                         0x48
#define WB01_TPC_BWC_DYN_QOS0_W_TARGET_SHIFT                          0
#define WB01_TPC_BWC_DYN_QOS0_W_TARGET_LENGTH                         8
#define WB01_TPC_BWC_DYN_QOS1_W_TARGET_SHIFT                          8
#define WB01_TPC_BWC_DYN_QOS1_W_TARGET_LENGTH                         8
#define WB01_TPC_BWC_DYN_QOS2_W_TARGET_SHIFT                          16
#define WB01_TPC_BWC_DYN_QOS2_W_TARGET_LENGTH                         8
#define WB01_TPC_BWC_DYN_QOS3_W_TARGET_SHIFT                          24
#define WB01_TPC_BWC_DYN_QOS3_W_TARGET_LENGTH                         8

/* REGISTER saturn_dpu_wdma_top_reg_21 */
#define WB0_TPC_EN_OFFSET                                             0x54
#define WB0_TPC_EN_SHIFT                                              0
#define WB0_TPC_EN_LENGTH                                             1
#define WB0_TPC_BW_COUNT_EN_SHIFT                                     1
#define WB0_TPC_BW_COUNT_EN_LENGTH                                    1
#define WB0_TPC_BWC_STATIC_W_EN_SHIFT                                 2
#define WB0_TPC_BWC_STATIC_W_EN_LENGTH                                1
#define WB0_TPC_BWC_DYN_W_EN_SHIFT                                    3
#define WB0_TPC_BWC_DYN_W_EN_LENGTH                                   1
#define WB0_TPC_COUNT_AW_OUTSTANDING_EN_SHIFT                         4
#define WB0_TPC_COUNT_AW_OUTSTANDING_EN_LENGTH                        1
#define WB0_TPC_COUNT_AW_MAX_SHIFT                                    8
#define WB0_TPC_COUNT_AW_MAX_LENGTH                                   7
#define WB0_TPC_QOS_REMAP_EN_SHIFT                                    15
#define WB0_TPC_QOS_REMAP_EN_LENGTH                                   1
#define WB0_TPC_QOS_ICG_EN_SHIFT                                      16
#define WB0_TPC_QOS_ICG_EN_LENGTH                                     1
#define WB0_TPC_QOS_CFG_SWITCH_SHIFT                                  17
#define WB0_TPC_QOS_CFG_SWITCH_LENGTH                                 1

/* REGISTER saturn_dpu_wdma_top_reg_22 */
#define WB0_TPC_BWC_STATIC_W_TARGET_OFFSET                            0x58
#define WB0_TPC_BWC_STATIC_W_TARGET_SHIFT                             0
#define WB0_TPC_BWC_STATIC_W_TARGET_LENGTH                            8

/* REGISTER saturn_dpu_wdma_top_reg_23 */
#define WB0_TPC_QOS_0_OFFSET                                          0x5c
#define WB0_TPC_QOS_0_SHIFT                                           0
#define WB0_TPC_QOS_0_LENGTH                                          4
#define WB0_TPC_QOS_1_SHIFT                                           4
#define WB0_TPC_QOS_1_LENGTH                                          4
#define WB0_TPC_QOS_2_SHIFT                                           8
#define WB0_TPC_QOS_2_LENGTH                                          4
#define WB0_TPC_QOS_3_SHIFT                                           12
#define WB0_TPC_QOS_3_LENGTH                                          4
#define WB0_TPC_QOS_4_SHIFT                                           16
#define WB0_TPC_QOS_4_LENGTH                                          4
#define WB0_TPC_QOS_5_SHIFT                                           20
#define WB0_TPC_QOS_5_LENGTH                                          4
#define WB0_TPC_QOS_6_SHIFT                                           24
#define WB0_TPC_QOS_6_LENGTH                                          4
#define WB0_TPC_QOS_7_SHIFT                                           28
#define WB0_TPC_QOS_7_LENGTH                                          4

/* REGISTER saturn_dpu_wdma_top_reg_24 */
#define WB0_TPC_BWC_STATIC_W_SAT_OFFSET                               0x60
#define WB0_TPC_BWC_STATIC_W_SAT_SHIFT                                0
#define WB0_TPC_BWC_STATIC_W_SAT_LENGTH                               16

/* REGISTER saturn_dpu_wdma_top_reg_25 */
#define WB0_TPC_CMD_W_BYTE_COUNTER_ALL_OFFSET                         0x64

/* REGISTER saturn_dpu_wdma_top_reg_27 */
#define WB0_TPC_CMD_CYCLE_COUNTER_ALL_OFFSET                          0x6c

/* REGISTER saturn_dpu_wdma_top_reg_29 */
#define WB1_TPC_EN_OFFSET                                             0x74
#define WB1_TPC_EN_SHIFT                                              0
#define WB1_TPC_EN_LENGTH                                             1
#define WB1_TPC_BW_COUNT_EN_SHIFT                                     1
#define WB1_TPC_BW_COUNT_EN_LENGTH                                    1
#define WB1_TPC_BWC_STATIC_W_EN_SHIFT                                 2
#define WB1_TPC_BWC_STATIC_W_EN_LENGTH                                1
#define WB1_TPC_BWC_DYN_W_EN_SHIFT                                    3
#define WB1_TPC_BWC_DYN_W_EN_LENGTH                                   1
#define WB1_TPC_COUNT_AW_OUTSTANDING_EN_SHIFT                         4
#define WB1_TPC_COUNT_AW_OUTSTANDING_EN_LENGTH                        1
#define WB1_TPC_COUNT_AW_MAX_SHIFT                                    8
#define WB1_TPC_COUNT_AW_MAX_LENGTH                                   7
#define WB1_TPC_QOS_REMAP_EN_SHIFT                                    15
#define WB1_TPC_QOS_REMAP_EN_LENGTH                                   1
#define WB1_TPC_QOS_ICG_EN_SHIFT                                      16
#define WB1_TPC_QOS_ICG_EN_LENGTH                                     1
#define WB1_TPC_QOS_CFG_SWITCH_SHIFT                                  17
#define WB1_TPC_QOS_CFG_SWITCH_LENGTH                                 1

/* REGISTER saturn_dpu_wdma_top_reg_30 */
#define WB1_TPC_BWC_STATIC_W_TARGET_OFFSET                            0x78
#define WB1_TPC_BWC_STATIC_W_TARGET_SHIFT                             0
#define WB1_TPC_BWC_STATIC_W_TARGET_LENGTH                            8

/* REGISTER saturn_dpu_wdma_top_reg_31 */
#define WB1_TPC_QOS_0_OFFSET                                          0x7c
#define WB1_TPC_QOS_0_SHIFT                                           0
#define WB1_TPC_QOS_0_LENGTH                                          4
#define WB1_TPC_QOS_1_SHIFT                                           4
#define WB1_TPC_QOS_1_LENGTH                                          4
#define WB1_TPC_QOS_2_SHIFT                                           8
#define WB1_TPC_QOS_2_LENGTH                                          4
#define WB1_TPC_QOS_3_SHIFT                                           12
#define WB1_TPC_QOS_3_LENGTH                                          4
#define WB1_TPC_QOS_4_SHIFT                                           16
#define WB1_TPC_QOS_4_LENGTH                                          4
#define WB1_TPC_QOS_5_SHIFT                                           20
#define WB1_TPC_QOS_5_LENGTH                                          4
#define WB1_TPC_QOS_6_SHIFT                                           24
#define WB1_TPC_QOS_6_LENGTH                                          4
#define WB1_TPC_QOS_7_SHIFT                                           28
#define WB1_TPC_QOS_7_LENGTH                                          4

/* REGISTER saturn_dpu_wdma_top_reg_32 */
#define WB1_TPC_BWC_STATIC_W_SAT_OFFSET                               0x80
#define WB1_TPC_BWC_STATIC_W_SAT_SHIFT                                0
#define WB1_TPC_BWC_STATIC_W_SAT_LENGTH                               16

/* REGISTER saturn_dpu_wdma_top_reg_33 */
#define WB1_TPC_CMD_W_BYTE_COUNTER_ALL_OFFSET                         0x84

/* REGISTER saturn_dpu_wdma_top_reg_35 */
#define WB1_TPC_CMD_CYCLE_COUNTER_ALL_OFFSET                          0x8c

/* REGISTER saturn_dpu_wdma_top_reg_40 */
#define WDMA_TOP_FORCE_UPDATE_EN_OFFSET                               0xa0
#define WDMA_TOP_FORCE_UPDATE_EN_SHIFT                                0
#define WDMA_TOP_FORCE_UPDATE_EN_LENGTH                               1
#define WDMA_TOP_VSYNC_UPDATE_EN_SHIFT                                1
#define WDMA_TOP_VSYNC_UPDATE_EN_LENGTH                               1
#define WDMA_TOP_SHADOW_READ_EN_SHIFT                                 2
#define WDMA_TOP_SHADOW_READ_EN_LENGTH                                1

/* REGISTER saturn_dpu_wdma_top_reg_41 */
#define WDMA_TOP_FORCE_UPDATE_PULSE_OFFSET                            0xa4
#define WDMA_TOP_FORCE_UPDATE_PULSE_SHIFT                             0
#define WDMA_TOP_FORCE_UPDATE_PULSE_LENGTH                            1

/* REGISTER saturn_dpu_wdma_top_reg_42 */
#define WDMA_TOP_ICG_OVERRIDE_OFFSET                                  0xa8
#define WDMA_TOP_ICG_OVERRIDE_SHIFT                                   0
#define WDMA_TOP_ICG_OVERRIDE_LENGTH                                  1

/* REGISTER saturn_dpu_wdma_top_reg_43 */
#define WDMA_TOP_REG_VALUE_RST_OFFSET                                 0xac
#define WDMA_TOP_REG_VALUE_RST_SHIFT                                  0
#define WDMA_TOP_REG_VALUE_RST_LENGTH                                 1

/* REGISTER saturn_dpu_wdma_top_reg_44 */
#define WDMA_TOP_REG_VALUE_RST_SE_OFFSET                              0xb0
#define WDMA_TOP_REG_VALUE_RST_SE_SHIFT                               0
#define WDMA_TOP_REG_VALUE_RST_SE_LENGTH                              1

/* REGISTER dpu_wdma_top_reg_45 */
#define WDMA_TOP_RO_TRIG_ENABLE_OFFSET                                0xb4
#define WDMA_TOP_RO_TRIG_ENABLE_SHIFT                                 0
#define WDMA_TOP_RO_TRIG_ENABLE_LENGTH                                1

#endif
