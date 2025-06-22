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

#ifndef _DPU_HW_DSC_WDMA_REG_H_
#define _DPU_HW_DSC_WDMA_REG_H_

/* REGISTER dsc_wdma_reg_0 */
#define DSC_WDMA_HSIZE_OFFSET                                         0x00
#define DSC_WDMA_HSIZE_SHIFT                                          0
#define DSC_WDMA_HSIZE_LENGTH                                         16
#define DSC_WDMA_VSIZE_SHIFT                                          16
#define DSC_WDMA_VSIZE_LENGTH                                         16

/* REGISTER dsc_wdma_reg_1 */
#define DSC_WDMA_STRIDE_OFFSET                                        0x04
#define DSC_WDMA_STRIDE_SHIFT                                         0
#define DSC_WDMA_STRIDE_LENGTH                                        16

/* REGISTER dsc_wdma_reg_2 */
#define DSC_WDMA_BASE_ADDR_LOW_OFFSET                                 0x08

/* REGISTER dsc_wdma_reg_3 */
#define DSC_WDMA_BASE_ADDR_HIGH_OFFSET                                0x0c
#define DSC_WDMA_BASE_ADDR_HIGH_SHIFT                                 0
#define DSC_WDMA_BASE_ADDR_HIGH_LENGTH                                12

/* REGISTER dsc_wdma_reg_4 */
#define DSC_WDMA_OUTSTANDING_NUM_OFFSET                               0x10
#define DSC_WDMA_OUTSTANDING_NUM_SHIFT                                0
#define DSC_WDMA_OUTSTANDING_NUM_LENGTH                               5
#define DSC_WDMA_BURST_LEN_SHIFT                                      8
#define DSC_WDMA_BURST_LEN_LENGTH                                     5
#define DSC_WDMA_BOUNDARY_EN_SHIFT                                    13
#define DSC_WDMA_BOUNDARY_EN_LENGTH                                   1
#define DSC_WDMA_BURST_BOUNDARY_SHIFT                                 14
#define DSC_WDMA_BURST_BOUNDARY_LENGTH                                1

/* REGISTER dsc_wdma_reg_5 */
#define DSC_WDMA_QOS_OFFSET                                           0x14
#define DSC_WDMA_QOS_SHIFT                                            0
#define DSC_WDMA_QOS_LENGTH                                           4
#define DSC_WDMA_REGION_SHIFT                                         4
#define DSC_WDMA_REGION_LENGTH                                        4
#define DSC_WDMA_CACHE_SHIFT                                          8
#define DSC_WDMA_CACHE_LENGTH                                         4

/* REGISTER dsc_wdma_reg_6 */
#define DSC_WDMA_NSAID_OFFSET                                         0x18
#define DSC_WDMA_NSAID_SHIFT                                          0
#define DSC_WDMA_NSAID_LENGTH                                         16
#define DSCW_AXI_SECU_EN_SHIFT                                        16
#define DSCW_AXI_SECU_EN_LENGTH                                       1

/* REGISTER dsc_wdma_reg_7 */
#define DSC_WDMA_MEM_LP_AUTO_EN_OFFSET                                0x1c
#define DSC_WDMA_MEM_LP_AUTO_EN_SHIFT                                 0
#define DSC_WDMA_MEM_LP_AUTO_EN_LENGTH                                1

/* REGISTER dsc_wdma_reg_8 */
#define DSC_WDMA_TPC_OCM_ADDR_RANGE0_32H_OFFSET                       0x20

/* REGISTER dsc_wdma_reg_9 */
#define DSC_WDMA_TPC_OCM_ADDR_RANGE0_32L_OFFSET                       0x24

/* REGISTER dsc_wdma_reg_10 */
#define DSC_WDMA_TPC_OCM_ADDR_RANGE1_32H_OFFSET                       0x28

/* REGISTER dsc_wdma_reg_11 */
#define DSC_WDMA_TPC_OCM_ADDR_RANGE1_32L_OFFSET                       0x2c

/* REGISTER dsc_wdma_reg_12 */
#define DSC_WDMA_TPC_OCM_ADDR_RANGE2_32H_OFFSET                       0x30

/* REGISTER dsc_wdma_reg_13 */
#define DSC_WDMA_TPC_OCM_ADDR_RANGE2_32L_OFFSET                       0x34

/* REGISTER dsc_wdma_reg_14 */
#define TPC_BWC_DYN_QOS4_W_TARGET_OFFSET                              0x38
#define TPC_BWC_DYN_QOS4_W_TARGET_SHIFT                               0
#define TPC_BWC_DYN_QOS4_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS5_W_TARGET_SHIFT                               8
#define TPC_BWC_DYN_QOS5_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS6_W_TARGET_SHIFT                               16
#define TPC_BWC_DYN_QOS6_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS7_W_TARGET_SHIFT                               24
#define TPC_BWC_DYN_QOS7_W_TARGET_LENGTH                              8

/* REGISTER dsc_wdma_reg_15 */
#define TPC_BWC_DYN_QOS0_W_TARGET_OFFSET                              0x3c
#define TPC_BWC_DYN_QOS0_W_TARGET_SHIFT                               0
#define TPC_BWC_DYN_QOS0_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS1_W_TARGET_SHIFT                               8
#define TPC_BWC_DYN_QOS1_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS2_W_TARGET_SHIFT                               16
#define TPC_BWC_DYN_QOS2_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS3_W_TARGET_SHIFT                               24
#define TPC_BWC_DYN_QOS3_W_TARGET_LENGTH                              8

/* REGISTER dsc_wdma_reg_18 */
#define DSC_WDMA_TPC_EN_OFFSET                                        0x48
#define DSC_WDMA_TPC_EN_SHIFT                                         0
#define DSC_WDMA_TPC_EN_LENGTH                                        1
#define DSC_WDMA_TPC_BW_COUNT_EN_SHIFT                                1
#define DSC_WDMA_TPC_BW_COUNT_EN_LENGTH                               1
#define TPC_BWC_STATIC_W_EN_SHIFT                                     2
#define TPC_BWC_STATIC_W_EN_LENGTH                                    1
#define TPC_BWC_DYN_W_EN_SHIFT                                        3
#define TPC_BWC_DYN_W_EN_LENGTH                                       1
#define TPC_COUNT_AW_OUTSTANDING_EN_SHIFT                             4
#define TPC_COUNT_AW_OUTSTANDING_EN_LENGTH                            1
#define TPC_COUNT_AW_MAX_SHIFT                                        8
#define TPC_COUNT_AW_MAX_LENGTH                                       7
#define DSC_WDMA_TPC_QOS_REMAP_EN_SHIFT                               15
#define DSC_WDMA_TPC_QOS_REMAP_EN_LENGTH                              1
#define DSC_WDMA_TPC_QOS_ICG_EN_SHIFT                                 16
#define DSC_WDMA_TPC_QOS_ICG_EN_LENGTH                                1
#define DSC_WDMA_TPC_QOS_CFG_SWITCH_SHIFT                             17
#define DSC_WDMA_TPC_QOS_CFG_SWITCH_LENGTH                            1

/* REGISTER dsc_wdma_reg_19 */
#define TPC_BWC_STATIC_W_TARGET_OFFSET                                0x4c
#define TPC_BWC_STATIC_W_TARGET_SHIFT                                 0
#define TPC_BWC_STATIC_W_TARGET_LENGTH                                8

/* REGISTER dsc_wdma_reg_20 */
#define DSC_WDMA_TPC_QOS_0_OFFSET                                     0x50
#define DSC_WDMA_TPC_QOS_0_SHIFT                                      0
#define DSC_WDMA_TPC_QOS_0_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_1_SHIFT                                      4
#define DSC_WDMA_TPC_QOS_1_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_2_SHIFT                                      8
#define DSC_WDMA_TPC_QOS_2_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_3_SHIFT                                      12
#define DSC_WDMA_TPC_QOS_3_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_4_SHIFT                                      16
#define DSC_WDMA_TPC_QOS_4_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_5_SHIFT                                      20
#define DSC_WDMA_TPC_QOS_5_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_6_SHIFT                                      24
#define DSC_WDMA_TPC_QOS_6_LENGTH                                     4
#define DSC_WDMA_TPC_QOS_7_SHIFT                                      28
#define DSC_WDMA_TPC_QOS_7_LENGTH                                     4

/* REGISTER dsc_wdma_reg_21 */
#define TPC_BWC_STATIC_W_SAT_OFFSET                                   0x54
#define TPC_BWC_STATIC_W_SAT_SHIFT                                    0
#define TPC_BWC_STATIC_W_SAT_LENGTH                                   16

/* REGISTER dsc_wdma_reg_22 */
#define TPC_CMD_W_BYTE_COUNTER_ALL_OFFSET                             0x58

/* REGISTER dsc_wdma_reg_24 */
#define DSC_WDMA_TPC_CMD_CYCLE_COUNTER_ALL_OFFSET                     0x60

/* REGISTER dsc_wdma_reg_27 */
#define DSC_WDMA_FORCE_UPDATE_EN_OFFSET                               0x6c
#define DSC_WDMA_FORCE_UPDATE_EN_SHIFT                                0
#define DSC_WDMA_FORCE_UPDATE_EN_LENGTH                               1
#define DSC_WDMA_VSYNC_UPDATE_EN_SHIFT                                1
#define DSC_WDMA_VSYNC_UPDATE_EN_LENGTH                               1
#define DSC_WDMA_SHADOW_READ_EN_SHIFT                                 2
#define DSC_WDMA_SHADOW_READ_EN_LENGTH                                1

/* REGISTER dsc_wdma_reg_28 */
#define DSC_WDMA_FORCE_UPDATE_PULSE_OFFSET                            0x70
#define DSC_WDMA_FORCE_UPDATE_PULSE_SHIFT                             0
#define DSC_WDMA_FORCE_UPDATE_PULSE_LENGTH                            1

/* REGISTER dsc_wdma_reg_29 */
#define DSC_WDMA_FORCE_UPDATE_EN_SE_OFFSET                            0x74
#define DSC_WDMA_FORCE_UPDATE_EN_SE_SHIFT                             0
#define DSC_WDMA_FORCE_UPDATE_EN_SE_LENGTH                            1
#define DSC_WDMA_VSYNC_UPDATE_EN_SE_SHIFT                             1
#define DSC_WDMA_VSYNC_UPDATE_EN_SE_LENGTH                            1
#define DSC_WDMA_SHADOW_READ_EN_SE_SHIFT                              2
#define DSC_WDMA_SHADOW_READ_EN_SE_LENGTH                             1

/* REGISTER dsc_wdma_reg_30 */
#define DSC_WDMA_FORCE_UPDATE_PULSE_SE_OFFSET                         0x78
#define DSC_WDMA_FORCE_UPDATE_PULSE_SE_SHIFT                          0
#define DSC_WDMA_FORCE_UPDATE_PULSE_SE_LENGTH                         1

/* REGISTER dsc_wdma_reg_31 */
#define DSC_WDMA_ICG_OVERRIDE_OFFSET                                  0x7c
#define DSC_WDMA_ICG_OVERRIDE_SHIFT                                   0
#define DSC_WDMA_ICG_OVERRIDE_LENGTH                                  1

/* REGISTER dsc_wdma_reg_32 */
#define DSC_WDMA_REG_VALUE_RST_OFFSET                                 0x80
#define DSC_WDMA_REG_VALUE_RST_SHIFT                                  0
#define DSC_WDMA_REG_VALUE_RST_LENGTH                                 1

/* REGISTER dsc_wdma_reg_33 */
#define DSC_WDMA_REG_VALUE_RST_SE_OFFSET                              0x84
#define DSC_WDMA_REG_VALUE_RST_SE_SHIFT                               0
#define DSC_WDMA_REG_VALUE_RST_SE_LENGTH                              1

#endif
