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
/* File generation time: 2024-01-26 16:59:09. */

#ifndef _XRING_HW_VEU_WB_CORE_REG_H_
#define _XRING_HW_VEU_WB_CORE_REG_H_

/* REGISTER veu_wb_core_reg_0 */
#define WB_WDMA_FBC_EN_OFFSET                                         0x00
#define WB_WDMA_FBC_EN_SHIFT                                          0
#define WB_WDMA_FBC_EN_LENGTH                                         1
#define WB_OUT_FORMAT_SHIFT                                           8
#define WB_OUT_FORMAT_LENGTH                                          5
#define WB_ROT_MODE_SHIFT                                             13
#define WB_ROT_MODE_LENGTH                                            2
#define WB_FLIP_MODE_SHIFT                                            16
#define WB_FLIP_MODE_LENGTH                                           2

/* REGISTER veu_wb_core_reg_2 */
#define WB_OUT_ORI_WIDTH_OFFSET                                       0x08
#define WB_OUT_ORI_WIDTH_SHIFT                                        0
#define WB_OUT_ORI_WIDTH_LENGTH                                       16
#define WB_OUT_ORI_HEIGHT_SHIFT                                       16
#define WB_OUT_ORI_HEIGHT_LENGTH                                      16

/* REGISTER veu_wb_core_reg_3 */
#define WB_OUT_SUBFRAME_LTOPX_OFFSET                                  0x0c
#define WB_OUT_SUBFRAME_LTOPX_SHIFT                                   0
#define WB_OUT_SUBFRAME_LTOPX_LENGTH                                  16
#define WB_OUT_SUBFRAME_LTOPY_SHIFT                                   16
#define WB_OUT_SUBFRAME_LTOPY_LENGTH                                  16

/* REGISTER veu_wb_core_reg_4 */
#define WB_OUT_SUBFRAME_WIDTH_OFFSET                                  0x10
#define WB_OUT_SUBFRAME_WIDTH_SHIFT                                   0
#define WB_OUT_SUBFRAME_WIDTH_LENGTH                                  16
#define WB_OUT_SUBFRAME_HEIGHT_SHIFT                                  16
#define WB_OUT_SUBFRAME_HEIGHT_LENGTH                                 16

/* REGISTER veu_wb_core_reg_6 */
#define WB_WDMA_BASE_ADDR0_LOW_OFFSET                                 0x18

/* REGISTER veu_wb_core_reg_7 */
#define WB_WDMA_BASE_ADDR0_HIGH_OFFSET                                0x1c
#define WB_WDMA_BASE_ADDR0_HIGH_SHIFT                                 0
#define WB_WDMA_BASE_ADDR0_HIGH_LENGTH                                12

/* REGISTER veu_wb_core_reg_8 */
#define WB_WDMA_BASE_ADDR1_LOW_OFFSET                                 0x20

/* REGISTER veu_wb_core_reg_9 */
#define WB_WDMA_BASE_ADDR1_HIGH_OFFSET                                0x24
#define WB_WDMA_BASE_ADDR1_HIGH_SHIFT                                 0
#define WB_WDMA_BASE_ADDR1_HIGH_LENGTH                                12

/* REGISTER veu_wb_core_reg_12 */
#define WB_WDMA_STRIDE_OFFSET                                         0x30
#define WB_WDMA_STRIDE_SHIFT                                          0
#define WB_WDMA_STRIDE_LENGTH                                         16

/* REGISTER veu_wb_core_reg_13 */
#define WB_WDMA_OUTSTANDING_NUM_OFFSET                                0x34
#define WB_WDMA_OUTSTANDING_NUM_SHIFT                                 0
#define WB_WDMA_OUTSTANDING_NUM_LENGTH                                5
#define WB_WDMA_BURST_LEN_SHIFT                                       8
#define WB_WDMA_BURST_LEN_LENGTH                                      5
#define WB_WDMA_BOUNDARY_EN_SHIFT                                     13
#define WB_WDMA_BOUNDARY_EN_LENGTH                                    1
#define WB_WDMA_BURST_BOUNDARY_SHIFT                                  14
#define WB_WDMA_BURST_BOUNDARY_LENGTH                                 1

/* REGISTER veu_wb_core_reg_14 */
#define WB_WDMA_QOS_OFFSET                                            0x38
#define WB_WDMA_QOS_SHIFT                                             0
#define WB_WDMA_QOS_LENGTH                                            4
#define WB_WDMA_REGION_SHIFT                                          4
#define WB_WDMA_REGION_LENGTH                                         4
#define WB_WDMA_CACHE_SHIFT                                           8
#define WB_WDMA_CACHE_LENGTH                                          4

/* REGISTER veu_wb_core_reg_15 */
#define WB_WDMA_NSAID_OFFSET                                          0x3c
#define WB_WDMA_NSAID_SHIFT                                           0
#define WB_WDMA_NSAID_LENGTH                                          16
#define WB_AXI_SECU_SHIFT                                             16
#define WB_AXI_SECU_LENGTH                                            1

/* REGISTER veu_wb_core_reg_16 */
#define FBC_COPY_MODE_EN_OFFSET                                       0x40
#define FBC_COPY_MODE_EN_SHIFT                                        0
#define FBC_COPY_MODE_EN_LENGTH                                       1
#define FBC_DEFAULT_COLOR_EN_SHIFT                                    1
#define FBC_DEFAULT_COLOR_EN_LENGTH                                   1
#define FBC_YUV_TRANSFORM_EN_SHIFT                                    2
#define FBC_YUV_TRANSFORM_EN_LENGTH                                   1
#define FBC_TILE_HD_MODE_EN_SHIFT                                     3
#define FBC_TILE_HD_MODE_EN_LENGTH                                    1
#define FBC_TILE_WIDE_SHIFT                                           4
#define FBC_TILE_WIDE_LENGTH                                          1
#define FBC_TILE_SPLIT_EN_SHIFT                                       5
#define FBC_TILE_SPLIT_EN_LENGTH                                      1

/* REGISTER veu_wb_core_reg_17 */
#define WB_IRQ_MASK_OFFSET                                            0x44

/* REGISTER veu_wb_core_reg_18 */
#define WB_IRQ_STATUS_OFFSET                                          0x48

/* REGISTER veu_wb_core_reg_19 */
#define WB_IRQ_RAW_OFFSET                                             0x4c

/* REGISTER veu_wb_core_reg_20 */
#define MEM_LP_AUTO_EN_OFFSET                                         0x50
#define VEU_WB_CORE_MEM_LP_AUTO_EN_SHIFT                              0
#define VEU_WB_CORE_MEM_LP_AUTO_EN_LENGTH                             5

/* REGISTER veu_wb_core_reg_21 */
#define WB_DBG_BUS0_OFFSET                                            0x54

/* REGISTER veu_wb_core_reg_22 */
#define WB_DBG_BUS1_OFFSET                                            0x58

/* REGISTER veu_wb_core_reg_23 */
#define WB_DBG_BUS2_OFFSET                                            0x5c

/* REGISTER veu_wb_core_reg_24 */
#define WB_DBG_BUS3_OFFSET                                            0x60

/* REGISTER veu_wb_core_reg_25 */
#define WB_DBG_BUS4_OFFSET                                            0x64

/* REGISTER veu_wb_core_reg_26 */
#define TPC_OCM_ADDR_RANGE0_32H_OFFSET                                0x68

/* REGISTER veu_wb_core_reg_27 */
#define TPC_OCM_ADDR_RANGE0_32L_OFFSET                                0x6c

/* REGISTER veu_wb_core_reg_28 */
#define TPC_OCM_ADDR_RANGE1_32H_OFFSET                                0x70

/* REGISTER veu_wb_core_reg_29 */
#define TPC_OCM_ADDR_RANGE1_32L_OFFSET                                0x74

/* REGISTER veu_wb_core_reg_30 */
#define TPC_OCM_ADDR_RANGE2_32H_OFFSET                                0x78

/* REGISTER veu_wb_core_reg_31 */
#define TPC_OCM_ADDR_RANGE2_32L_OFFSET                                0x7c

/* REGISTER veu_wb_core_reg_32 */
#define TPC_BWC_DYN_QOS4_W_TARGET_OFFSET                              0x80
#define TPC_BWC_DYN_QOS4_W_TARGET_SHIFT                               0
#define TPC_BWC_DYN_QOS4_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS5_W_TARGET_SHIFT                               8
#define TPC_BWC_DYN_QOS5_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS6_W_TARGET_SHIFT                               16
#define TPC_BWC_DYN_QOS6_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS7_W_TARGET_SHIFT                               24
#define TPC_BWC_DYN_QOS7_W_TARGET_LENGTH                              8

/* REGISTER veu_wb_core_reg_33 */
#define TPC_BWC_DYN_QOS0_W_TARGET_OFFSET                              0x84
#define TPC_BWC_DYN_QOS0_W_TARGET_SHIFT                               0
#define TPC_BWC_DYN_QOS0_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS1_W_TARGET_SHIFT                               8
#define TPC_BWC_DYN_QOS1_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS2_W_TARGET_SHIFT                               16
#define TPC_BWC_DYN_QOS2_W_TARGET_LENGTH                              8
#define TPC_BWC_DYN_QOS3_W_TARGET_SHIFT                               24
#define TPC_BWC_DYN_QOS3_W_TARGET_LENGTH                              8

/* REGISTER veu_wb_core_reg_36 */
#define TPC_EN_OFFSET                                                 0x90
#define TPC_EN_SHIFT                                                  0
#define TPC_EN_LENGTH                                                 1
#define TPC_BW_COUNT_EN_SHIFT                                         1
#define TPC_BW_COUNT_EN_LENGTH                                        1
#define TPC_BWC_STATIC_W_EN_SHIFT                                     2
#define TPC_BWC_STATIC_W_EN_LENGTH                                    1
#define TPC_BWC_DYN_W_EN_SHIFT                                        3
#define TPC_BWC_DYN_W_EN_LENGTH                                       1
#define TPC_COUNT_AW_OUTSTANDING_EN_SHIFT                             4
#define TPC_COUNT_AW_OUTSTANDING_EN_LENGTH                            1
#define TPC_COUNT_AW_MAX_SHIFT                                        8
#define TPC_COUNT_AW_MAX_LENGTH                                       7
#define TPC_QOS_REMAP_EN_SHIFT                                        15
#define TPC_QOS_REMAP_EN_LENGTH                                       1
#define TPC_QOS_ICG_EN_SHIFT                                          16
#define TPC_QOS_ICG_EN_LENGTH                                         1
#define TPC_QOS_CFG_SWITCH_SHIFT                                      17
#define TPC_QOS_CFG_SWITCH_LENGTH                                     1

/* REGISTER veu_wb_core_reg_37 */
#define TPC_BWC_STATIC_W_TARGET_OFFSET                                0x94
#define TPC_BWC_STATIC_W_TARGET_SHIFT                                 0
#define TPC_BWC_STATIC_W_TARGET_LENGTH                                8

/* REGISTER veu_wb_core_reg_38 */
#define TPC_QOS_0_OFFSET                                              0x98
#define TPC_QOS_0_SHIFT                                               0
#define TPC_QOS_0_LENGTH                                              4
#define TPC_QOS_1_SHIFT                                               4
#define TPC_QOS_1_LENGTH                                              4
#define TPC_QOS_2_SHIFT                                               8
#define TPC_QOS_2_LENGTH                                              4
#define TPC_QOS_3_SHIFT                                               12
#define TPC_QOS_3_LENGTH                                              4
#define TPC_QOS_4_SHIFT                                               16
#define TPC_QOS_4_LENGTH                                              4
#define TPC_QOS_5_SHIFT                                               20
#define TPC_QOS_5_LENGTH                                              4
#define TPC_QOS_6_SHIFT                                               24
#define TPC_QOS_6_LENGTH                                              4
#define TPC_QOS_7_SHIFT                                               28
#define TPC_QOS_7_LENGTH                                              4

/* REGISTER veu_wb_core_reg_39 */
#define TPC_BWC_STATIC_W_SAT_OFFSET                                   0x9c
#define TPC_BWC_STATIC_W_SAT_SHIFT                                    0
#define TPC_BWC_STATIC_W_SAT_LENGTH                                   16

/* REGISTER veu_wb_core_reg_40 */
#define TPC_CMD_W_BYTE_COUNTER_ALL_OFFSET                             0xa0

/* REGISTER veu_wb_core_reg_42 */
#define TPC_CMD_CYCLE_COUNTER_ALL_OFFSET                              0xa8

/* REGISTER veu_wb_core_reg_44 */
#define VEU_WB_CORE_FORCE_UPDATE_EN_OFFSET                            0xb0
#define VEU_WB_CORE_FORCE_UPDATE_EN_SHIFT                             0
#define VEU_WB_CORE_FORCE_UPDATE_EN_LENGTH                            1
#define VEU_WB_CORE_VSYNC_UPDATE_EN_SHIFT                             1
#define VEU_WB_CORE_VSYNC_UPDATE_EN_LENGTH                            1
#define VEU_WB_CORE_SHADOW_READ_EN_SHIFT                              2
#define VEU_WB_CORE_SHADOW_READ_EN_LENGTH                             1

/* REGISTER veu_wb_core_reg_45 */
#define VEU_WB_CORE_FORCE_UPDATE_PULSE_OFFSET                         0xb4
#define VEU_WB_CORE_FORCE_UPDATE_PULSE_SHIFT                          0
#define VEU_WB_CORE_FORCE_UPDATE_PULSE_LENGTH                         1

/* REGISTER veu_wb_core_reg_46 */
#define FORCE_UPDATE_EN_SE_OFFSET                                     0xb8
#define FORCE_UPDATE_EN_SE_SHIFT                                      0
#define FORCE_UPDATE_EN_SE_LENGTH                                     1
#define VSYNC_UPDATE_EN_SE_SHIFT                                      1
#define VSYNC_UPDATE_EN_SE_LENGTH                                     1
#define SHADOW_READ_EN_SE_SHIFT                                       2
#define SHADOW_READ_EN_SE_LENGTH                                      1

/* REGISTER veu_wb_core_reg_47 */
#define FORCE_UPDATE_PULSE_SE_OFFSET                                  0xbc
#define FORCE_UPDATE_PULSE_SE_SHIFT                                   0
#define FORCE_UPDATE_PULSE_SE_LENGTH                                  1

/* REGISTER veu_wb_core_reg_48 */
#define VEU_WB_CORE_ICG_OVERRIDE_OFFSET                               0xc0
#define VEU_WB_CORE_ICG_OVERRIDE_SHIFT                                0
#define VEU_WB_CORE_ICG_OVERRIDE_LENGTH                               1

/* REGISTER veu_wb_core_reg_49 */
#define REG_VALUE_RST_OFFSET                                          0xc4
#define REG_VALUE_RST_SHIFT                                           0
#define REG_VALUE_RST_LENGTH                                          1

/* REGISTER veu_wb_core_reg_50 */
#define REG_VALUE_RST_SE_OFFSET                                       0xc8
#define REG_VALUE_RST_SE_SHIFT                                        0
#define REG_VALUE_RST_SE_LENGTH                                       1

/* REGISTER veu_wb_core_reg_51 */
#define RO_TRIG_ENABLE_OFFSET                                         0xcc
#define RO_TRIG_ENABLE_SHIFT                                          0
#define RO_TRIG_ENABLE_LENGTH                                         1

#endif
