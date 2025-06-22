/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _DPU_HW_RDMA_TOP_REG_H_
#define _DPU_HW_RDMA_TOP_REG_H_

/* REGISTER rdma_top_reg_0 */
#define ARB0_RR_MODE_OFFSET                                           0x00
#define ARB0_RR_MODE_SHIFT                                            0
#define ARB0_RR_MODE_LENGTH                                           1
#define ARB1_RR_MODE_SHIFT                                            1
#define ARB1_RR_MODE_LENGTH                                           1
#define IMAGE_RR_RATIO_SHIFT                                          8
#define IMAGE_RR_RATIO_LENGTH                                         8
#define RDMA_TOP_PIXEL_NUM_TH_SHIFT                                   16
#define RDMA_TOP_PIXEL_NUM_TH_LENGTH                                  6

/* REGISTER rdma_top_reg_1 */
#define DMAC0_ID_TRANS_OFFSET                                         0x04
#define DMAC0_ID_TRANS_SHIFT                                          0
#define DMAC0_ID_TRANS_LENGTH                                         1
#define DMAC0_BURST_LEN_SHIFT                                         1
#define DMAC0_BURST_LEN_LENGTH                                        3
#define DMAC0_QOS_MODE_SHIFT                                          4
#define DMAC0_QOS_MODE_LENGTH                                         1
#define DMAC0_OSD_NUM_SHIFT                                           8
#define DMAC0_OSD_NUM_LENGTH                                          8

/* REGISTER rdma_top_reg_2 */
#define DMAC1_ID_TRANS_OFFSET                                         0x08
#define DMAC1_ID_TRANS_SHIFT                                          0
#define DMAC1_ID_TRANS_LENGTH                                         1
#define DMAC1_BURST_LEN_SHIFT                                         1
#define DMAC1_BURST_LEN_LENGTH                                        3
#define DMAC1_QOS_MODE_SHIFT                                          4
#define DMAC1_QOS_MODE_LENGTH                                         1
#define DMAC1_OSD_NUM_SHIFT                                           8
#define DMAC1_OSD_NUM_LENGTH                                          8

/* REGISTER rdma_top_reg_3 */
#define RDMA_TIMEOUT_LIMIT_OFFSET                                     0x0c
#define RDMA_TIMEOUT_LIMIT_SHIFT                                      0
#define RDMA_TIMEOUT_LIMIT_LENGTH                                     16
#define WDMA_TIMEOUT_LIMIT_SHIFT                                      16
#define WDMA_TIMEOUT_LIMIT_LENGTH                                     16

/* REGISTER rdma_top_reg_4 */
#define RDMA_TOP_MEM_LP_AUTO_EN_OFFSET                                0x10
#define RDMA_TOP_MEM_LP_AUTO_EN_SHIFT                                 0
#define RDMA_TOP_MEM_LP_AUTO_EN_LENGTH                                29

/* REGISTER rdma_top_reg_5 */
#define ARB0_DBG_INFO_OFFSET                                          0x14

/* REGISTER rdma_top_reg_6 */
#define ARB1_DBG_INFO_OFFSET                                          0x18

/* REGISTER rdma_top_reg_7 */
#define TOP_DBG_IRQ_RAW_OFFSET                                        0x1c

/* REGISTER rdma_top_reg_8 */
#define TOP_DBG_IRQ_MASK_OFFSET                                       0x20

/* REGISTER rdma_top_reg_9 */
#define TOP_DBG_IRQ_STATUS_OFFSET                                     0x24

/* REGISTER rdma_top_reg_10 */
#define RDMA_TOP_TPC_OCM_ADDR_RANGE0_32H_OFFSET                       0x28

/* REGISTER rdma_top_reg_11 */
#define RDMA_TOP_TPC_OCM_ADDR_RANGE0_32L_OFFSET                       0x2c

/* REGISTER rdma_top_reg_12 */
#define RDMA_TOP_TPC_OCM_ADDR_RANGE1_32H_OFFSET                       0x30

/* REGISTER rdma_top_reg_13 */
#define RDMA_TOP_TPC_OCM_ADDR_RANGE1_32L_OFFSET                       0x34

/* REGISTER rdma_top_reg_14 */
#define RDMA_TOP_TPC_OCM_ADDR_RANGE2_32H_OFFSET                       0x38

/* REGISTER rdma_top_reg_15 */
#define RDMA_TOP_TPC_OCM_ADDR_RANGE2_32L_OFFSET                       0x3c

/* REGISTER rdma_top_reg_16 */
#define RDMA_TOP_TPC_BWC_DYN_QOS4_R_TARGET_OFFSET                     0x40
#define RDMA_TOP_TPC_BWC_DYN_QOS4_R_TARGET_SHIFT                      0
#define RDMA_TOP_TPC_BWC_DYN_QOS4_R_TARGET_LENGTH                     8
#define RDMA_TOP_TPC_BWC_DYN_QOS5_R_TARGET_SHIFT                      8
#define RDMA_TOP_TPC_BWC_DYN_QOS5_R_TARGET_LENGTH                     8
#define RDMA_TOP_TPC_BWC_DYN_QOS6_R_TARGET_SHIFT                      16
#define RDMA_TOP_TPC_BWC_DYN_QOS6_R_TARGET_LENGTH                     8
#define RDMA_TOP_TPC_BWC_DYN_QOS7_R_TARGET_SHIFT                      24
#define RDMA_TOP_TPC_BWC_DYN_QOS7_R_TARGET_LENGTH                     8

/* REGISTER rdma_top_reg_17 */
#define RDMA_TOP_TPC_BWC_DYN_QOS0_R_TARGET_OFFSET                     0x44
#define RDMA_TOP_TPC_BWC_DYN_QOS0_R_TARGET_SHIFT                      0
#define RDMA_TOP_TPC_BWC_DYN_QOS0_R_TARGET_LENGTH                     8
#define RDMA_TOP_TPC_BWC_DYN_QOS1_R_TARGET_SHIFT                      8
#define RDMA_TOP_TPC_BWC_DYN_QOS1_R_TARGET_LENGTH                     8
#define RDMA_TOP_TPC_BWC_DYN_QOS2_R_TARGET_SHIFT                      16
#define RDMA_TOP_TPC_BWC_DYN_QOS2_R_TARGET_LENGTH                     8
#define RDMA_TOP_TPC_BWC_DYN_QOS3_R_TARGET_SHIFT                      24
#define RDMA_TOP_TPC_BWC_DYN_QOS3_R_TARGET_LENGTH                     8

/* REGISTER rdma_top_reg_18 */
#define RDMA_TOP_TPC_EN_OFFSET                                        0x48
#define RDMA_TOP_TPC_EN_SHIFT                                         0
#define RDMA_TOP_TPC_EN_LENGTH                                        1
#define RDMA_TOP_TPC_BW_COUNT_EN_SHIFT                                1
#define RDMA_TOP_TPC_BW_COUNT_EN_LENGTH                               1
#define RDMA_TOP_TPC_BWC_STATIC_R_EN_SHIFT                            2
#define RDMA_TOP_TPC_BWC_STATIC_R_EN_LENGTH                           1
#define RDMA_TOP_TPC_BWC_DYN_R_EN_SHIFT                               3
#define RDMA_TOP_TPC_BWC_DYN_R_EN_LENGTH                              1
#define RDMA_TOP_TPC_COUNT_AR_OUTSTANDING_EN_SHIFT                    4
#define RDMA_TOP_TPC_COUNT_AR_OUTSTANDING_EN_LENGTH                   1
#define RDMA_TOP_TPC_COUNT_AR_MAX_SHIFT                               8
#define RDMA_TOP_TPC_COUNT_AR_MAX_LENGTH                              7
#define RDMA_TOP_TPC_QOS_REMAP_EN_SHIFT                               15
#define RDMA_TOP_TPC_QOS_REMAP_EN_LENGTH                              1
#define RDMA_TOP_TPC_QOS_ICG_EN_SHIFT                                 16
#define RDMA_TOP_TPC_QOS_ICG_EN_LENGTH                                1
#define RDMA_TOP_TPC_QOS_CFG_SWITCH_SHIFT                             17
#define RDMA_TOP_TPC_QOS_CFG_SWITCH_LENGTH                            1

/* REGISTER rdma_top_reg_19 */
#define RDMA_TOP_TPC_BWC_STATIC_R_TARGET_OFFSET                       0x4c
#define RDMA_TOP_TPC_BWC_STATIC_R_TARGET_SHIFT                        0
#define RDMA_TOP_TPC_BWC_STATIC_R_TARGET_LENGTH                       8

/* REGISTER rdma_top_reg_20 */
#define RDMA_TOP_TPC_QOS_0_OFFSET                                     0x50
#define RDMA_TOP_TPC_QOS_0_SHIFT                                      0
#define RDMA_TOP_TPC_QOS_0_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_1_SHIFT                                      4
#define RDMA_TOP_TPC_QOS_1_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_2_SHIFT                                      8
#define RDMA_TOP_TPC_QOS_2_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_3_SHIFT                                      12
#define RDMA_TOP_TPC_QOS_3_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_4_SHIFT                                      16
#define RDMA_TOP_TPC_QOS_4_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_5_SHIFT                                      20
#define RDMA_TOP_TPC_QOS_5_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_6_SHIFT                                      24
#define RDMA_TOP_TPC_QOS_6_LENGTH                                     4
#define RDMA_TOP_TPC_QOS_7_SHIFT                                      28
#define RDMA_TOP_TPC_QOS_7_LENGTH                                     4

/* REGISTER rdma_top_reg_21 */
#define RDMA_TOP_TPC_BWC_STATIC_R_SAT_OFFSET                          0x54
#define RDMA_TOP_TPC_BWC_STATIC_R_SAT_SHIFT                           0
#define RDMA_TOP_TPC_BWC_STATIC_R_SAT_LENGTH                          16

/* REGISTER rdma_top_reg_22 */
#define RDMA_TOP_TPC_CMD_R_BYTE_COUNTER_ALL_OFFSET                    0x58

/* REGISTER rdma_top_reg_23 */
#define RDMA_TOP_TPC_CMD_CYCLE_COUNTER_ALL_OFFSET                     0x5c

/* REGISTER rdma_top_reg_24 */
#define RDMA_TOP_ICG_OVERRIDE_OFFSET                                  0x60
#define RDMA_TOP_ICG_OVERRIDE_SHIFT                                   0
#define RDMA_TOP_ICG_OVERRIDE_LENGTH                                  1

/* REGISTER rdma_top_reg_25 */
#define RDMA_TOP_REGBANK_RESET_OFFSET                                 0x64
#define RDMA_TOP_REGBANK_RESET_SHIFT                                  0
#define RDMA_TOP_REGBANK_RESET_LENGTH                                 1

/* REGISTER rdma_top_reg_26 */
#define RO_TRIG_PULSE_OFFSET                                          0x68
#define RO_TRIG_PULSE_SHIFT                                           0
#define RO_TRIG_PULSE_LENGTH                                          1

/* REGISTER rdma_top_reg_27 */
#define RDMA_PATH_DBG_INFO0_OFFSET                                    0x6c

/* REGISTER rdma_top_reg_28 */
#define RDMA_PATH_DBG_INFO1_OFFSET                                    0x70

/* REGISTER rdma_top_reg_29 */
#define RDMA_PATH_DBG_INFO2_OFFSET                                    0x74

/* REGISTER rdma_top_reg_30 */
#define RDMA_PATH_DBG_INFO3_OFFSET                                    0x78

/* REGISTER rdma_top_reg_31 */
#define RDMA_PATH_DBG_INFO5_OFFSET                                    0x7c

/* REGISTER rdma_top_reg_32 */
#define RDMA_PATH_DBG_INFO4_OFFSET                                    0x80

#endif
