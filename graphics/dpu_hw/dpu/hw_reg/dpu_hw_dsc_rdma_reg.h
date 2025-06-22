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

#ifndef _DPU_HW_DSC_RDMA_REG_H_
#define _DPU_HW_DSC_RDMA_REG_H_

/* REGISTER dsc_rdma_reg_0 */
#define OUTSTANDING_NUM_OFFSET                                        0x00
#define DSC_RDMA_OUTSTANDING_NUM_SHIFT                                0
#define DSC_RDMA_OUTSTANDING_NUM_LENGTH                               5
#define DSC_RDMA_REQ_CONTI_NUM_SHIFT                                  5
#define DSC_RDMA_REQ_CONTI_NUM_LENGTH                                 2
#define DSC_RDMA_BURST_SPLIT_EN_SHIFT                                 7
#define DSC_RDMA_BURST_SPLIT_EN_LENGTH                                1
#define DSC_RDMA_BURST_SPLIT_UNIT_SHIFT                               8
#define DSC_RDMA_BURST_SPLIT_UNIT_LENGTH                              1

/* REGISTER dsc_rdma_reg_1 */
#define DSC_RDMA_AXI_RCACHE_OFFSET                                    0x04
#define DSC_RDMA_AXI_RCACHE_SHIFT                                     0
#define DSC_RDMA_AXI_RCACHE_LENGTH                                    4
#define DSC_RDMA_AXI_RREGION_SHIFT                                    4
#define DSC_RDMA_AXI_RREGION_LENGTH                                   4
#define AXI_RQOS_SHIFT                                                8
#define AXI_RQOS_LENGTH                                               4
#define DSC_RDMA_NSAID_SHIFT                                          16
#define DSC_RDMA_NSAID_LENGTH                                         16

/* REGISTER dsc_rdma_reg_2 */
#define RD_BASE_LOW_ADDR_OFFSET                                       0x08

/* REGISTER dsc_rdma_reg_3 */
#define RD_BASE_HEIGHT_ADDR_OFFSET                                    0x0c
#define RD_BASE_HEIGHT_ADDR_SHIFT                                     0
#define RD_BASE_HEIGHT_ADDR_LENGTH                                    12

/* REGISTER dsc_rdma_reg_4 */
#define RD_BURST_LEN_OFFSET                                           0x10
#define RD_BURST_LEN_SHIFT                                            0
#define RD_BURST_LEN_LENGTH                                           6

/* REGISTER dsc_rdma_reg_5 */
#define RD_DATA_WIDTH_OFFSET                                          0x14
#define RD_DATA_WIDTH_SHIFT                                           0
#define RD_DATA_WIDTH_LENGTH                                          16
#define RD_DATA_HEIGHT_SHIFT                                          16
#define RD_DATA_HEIGHT_LENGTH                                         16

/* REGISTER dsc_rdma_reg_6 */
#define RD_DATA_STRIDE_OFFSET                                         0x18
#define RD_DATA_STRIDE_SHIFT                                          0
#define RD_DATA_STRIDE_LENGTH                                         16

/* REGISTER dsc_rdma_reg_7 */
#define DEBUG_STA_OFFSET                                              0x1c

/* REGISTER dsc_rdma_reg_8 */
#define DSC_RDMA_IRQ_RAW_OFFSET                                       0x20

/* REGISTER dsc_rdma_reg_9 */
#define DSC_RDMA_IRQ_MASK_OFFSET                                      0x24

/* REGISTER dsc_rdma_reg_10 */
#define DSC_RDMA_IRQ_STATUS_OFFSET                                    0x28

/* REGISTER dsc_rdma_reg_11 */
#define RDMA_SECURITY_OFFSET                                          0x2c
#define RDMA_SECURITY_SHIFT                                           0
#define RDMA_SECURITY_LENGTH                                          1

/* REGISTER dsc_rdma_reg_12 */
#define DSC_RDMA_TPC_OCM_ADDR_RANGE0_32H_OFFSET                       0x30

/* REGISTER dsc_rdma_reg_13 */
#define DSC_RDMA_TPC_OCM_ADDR_RANGE0_32L_OFFSET                       0x34

/* REGISTER dsc_rdma_reg_14 */
#define DSC_RDMA_TPC_OCM_ADDR_RANGE1_32H_OFFSET                       0x38

/* REGISTER dsc_rdma_reg_15 */
#define DSC_RDMA_TPC_OCM_ADDR_RANGE1_32L_OFFSET                       0x3c

/* REGISTER dsc_rdma_reg_16 */
#define DSC_RDMA_TPC_OCM_ADDR_RANGE2_32H_OFFSET                       0x40

/* REGISTER dsc_rdma_reg_17 */
#define DSC_RDMA_TPC_OCM_ADDR_RANGE2_32L_OFFSET                       0x44


/* REGISTER dsc_rdma_reg_20 */
#define TPC_BWC_DYN_QOS4_R_TARGET_OFFSET                              0x50
#define TPC_BWC_DYN_QOS4_R_TARGET_SHIFT                               0
#define TPC_BWC_DYN_QOS4_R_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS5_R_TARGET_SHIFT                               8
#define TPC_BWC_DYN_QOS5_R_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS6_R_TARGET_SHIFT                               16
#define TPC_BWC_DYN_QOS6_R_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS7_R_TARGET_SHIFT                               24
#define TPC_BWC_DYN_QOS7_R_TARGET_LENGTH                              8

/* REGISTER dsc_rdma_reg_21 */
#define TPC_BWC_DYN_QOS0_R_TARGET_OFFSET                              0x54
#define TPC_BWC_DYN_QOS0_R_TARGET_SHIFT                               0
#define TPC_BWC_DYN_QOS0_R_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS1_R_TARGET_SHIFT                               8
#define TPC_BWC_DYN_QOS1_R_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS2_R_TARGET_SHIFT                               16
#define TPC_BWC_DYN_QOS2_R_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS3_R_TARGET_SHIFT                               24
#define TPC_BWC_DYN_QOS3_R_TARGET_LENGTH                              8

/* REGISTER dsc_rdma_reg_22 */
#define DSC_RDMA_TPC_EN_OFFSET                                        0x58
#define DSC_RDMA_TPC_EN_SHIFT                                         0
#define DSC_RDMA_TPC_EN_LENGTH                                        1
#define DSC_RDMA_TPC_BW_COUNT_EN_SHIFT                                1
#define DSC_RDMA_TPC_BW_COUNT_EN_LENGTH                               1
#define TPC_BWC_STATIC_R_EN_SHIFT                                     2
#define TPC_BWC_STATIC_R_EN_LENGTH                                    1
#define TPC_BWC_DYN_R_EN_SHIFT                                        3
#define TPC_BWC_DYN_R_EN_LENGTH                                       1
#define TPC_COUNT_AR_OUTSTANDING_EN_SHIFT                             4
#define TPC_COUNT_AR_OUTSTANDING_EN_LENGTH                            1
#define TPC_COUNT_AR_MAX_SHIFT                                        8
#define TPC_COUNT_AR_MAX_LENGTH                                       7
#define DSC_RDMA_TPC_QOS_REMAP_EN_SHIFT                               15
#define DSC_RDMA_TPC_QOS_REMAP_EN_LENGTH                              1
#define DSC_RDMA_TPC_QOS_ICG_EN_SHIFT                                 16
#define DSC_RDMA_TPC_QOS_ICG_EN_LENGTH                                1
#define DSC_RDMA_TPC_QOS_CFG_SWITCH_SHIFT                             17
#define DSC_RDMA_TPC_QOS_CFG_SWITCH_LENGTH                            1

/* REGISTER dsc_rdma_reg_23 */
#define TPC_BWC_STATIC_R_TARGET_OFFSET                                0x5c
#define TPC_BWC_STATIC_R_TARGET_SHIFT                                 0
#define TPC_BWC_STATIC_R_TARGET_LENGTH                                8

/* REGISTER dsc_rdma_reg_24 */
#define DSC_RDMA_TPC_QOS_0_OFFSET                                     0x60
#define DSC_RDMA_TPC_QOS_0_SHIFT                                      0
#define DSC_RDMA_TPC_QOS_0_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_1_SHIFT                                      4
#define DSC_RDMA_TPC_QOS_1_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_2_SHIFT                                      8
#define DSC_RDMA_TPC_QOS_2_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_3_SHIFT                                      12
#define DSC_RDMA_TPC_QOS_3_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_4_SHIFT                                      16
#define DSC_RDMA_TPC_QOS_4_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_5_SHIFT                                      20
#define DSC_RDMA_TPC_QOS_5_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_6_SHIFT                                      24
#define DSC_RDMA_TPC_QOS_6_LENGTH                                     4
#define DSC_RDMA_TPC_QOS_7_SHIFT                                      28
#define DSC_RDMA_TPC_QOS_7_LENGTH                                     4

/* REGISTER dsc_rdma_reg_25 */
#define TPC_BWC_STATIC_R_SAT_OFFSET                                   0x64
#define TPC_BWC_STATIC_R_SAT_SHIFT                                    0
#define TPC_BWC_STATIC_R_SAT_LENGTH                                   16

/* REGISTER dsc_rdma_reg_27 */
#define TPC_CMD_R_BYTE_COUNTER_ALL_OFFSET                             0x6c

/* REGISTER dsc_rdma_reg_28 */
#define DSC_RDMA_TPC_CMD_CYCLE_COUNTER_ALL_OFFSET                     0x70

/* REGISTER dsc_rdma_reg_31 */
#define DSC_RDMA_FORCE_UPDATE_EN_OFFSET                               0x7c
#define DSC_RDMA_FORCE_UPDATE_EN_SHIFT                                0
#define DSC_RDMA_FORCE_UPDATE_EN_LENGTH                               1
#define DSC_RDMA_VSYNC_UPDATE_EN_SHIFT                                1
#define DSC_RDMA_VSYNC_UPDATE_EN_LENGTH                               1
#define DSC_RDMA_SHADOW_READ_EN_SHIFT                                 2
#define DSC_RDMA_SHADOW_READ_EN_LENGTH                                1

/* REGISTER dsc_rdma_reg_32 */
#define DSC_RDMA_FORCE_UPDATE_PULSE_OFFSET                            0x80
#define DSC_RDMA_FORCE_UPDATE_PULSE_SHIFT                             0
#define DSC_RDMA_FORCE_UPDATE_PULSE_LENGTH                            1

/* REGISTER dsc_rdma_reg_33 */
#define DSC_RDMA_ICG_OVERRIDE_OFFSET                                  0x84
#define DSC_RDMA_ICG_OVERRIDE_SHIFT                                   0
#define DSC_RDMA_ICG_OVERRIDE_LENGTH                                  1

/* REGISTER dsc_rdma_reg_34 */
#define DSC_RDMA_TRIGGER_OFFSET                                       0x88
#define DSC_RDMA_TRIGGER_SHIFT                                        0
#define DSC_RDMA_TRIGGER_LENGTH                                       1

/* REGISTER dsc_rdma_reg_35 */
#define DSC_RDMA_TRIGGER2_OFFSET                                      0x8c
#define DSC_RDMA_TRIGGER2_SHIFT                                       0
#define DSC_RDMA_TRIGGER2_LENGTH                                      1

#endif
