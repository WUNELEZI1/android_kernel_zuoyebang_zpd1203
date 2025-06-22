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

#ifndef _XRING_VEU_HW_RDMA_PATH_REG_H_
#define _XRING_VEU_HW_RDMA_PATH_REG_H_

/* REGISTER rdma_path_reg_0 */
#define LAYER_MODE_OFFSET                                             0x00
#define LAYER_MODE_SHIFT                                              0
#define LAYER_MODE_LENGTH                                             1
#define RDMA_PATH_OUTSTANDING_NUM_SHIFT                               1
#define RDMA_PATH_OUTSTANDING_NUM_LENGTH                              5
#define RDMA_PATH_REQ_CONTI_NUM_SHIFT                                 6
#define RDMA_PATH_REQ_CONTI_NUM_LENGTH                                2
#define RDMA_PATH_BURST_SPLIT_EN_SHIFT                                8
#define RDMA_PATH_BURST_SPLIT_EN_LENGTH                               1
#define RDMA_PATH_BURST_SPLIT_UNIT_SHIFT                              9
#define RDMA_PATH_BURST_SPLIT_UNIT_LENGTH                             1
#define LAYER_CMPSR_ID_SHIFT                                          10
#define LAYER_CMPSR_ID_LENGTH                                         3
#define RDMA_PATH_AXI_PORT_SEL_SHIFT                                  13
#define RDMA_PATH_AXI_PORT_SEL_LENGTH                                 1
#define RDMA_BURST_LEN_SHIFT                                          16
#define RDMA_BURST_LEN_LENGTH                                         6
#define IS_OFFLINE_SHIFT                                              22
#define IS_OFFLINE_LENGTH                                             1
#define IS_TWO_LAYERS_SHIFT                                           23
#define IS_TWO_LAYERS_LENGTH                                          1
#define IS_OFFLINE_WB0_SHIFT                                          24
#define IS_OFFLINE_WB0_LENGTH                                         1

/* REGISTER rdma_path_reg_1 */
#define LEFT_CMPSR_Y_OFFSET_OFFSET                                    0x04
#define LEFT_CMPSR_Y_OFFSET_SHIFT                                     0
#define LEFT_CMPSR_Y_OFFSET_LENGTH                                    16
#define RIGHT_CMPSR_Y_OFFSET_SHIFT                                    16
#define RIGHT_CMPSR_Y_OFFSET_LENGTH                                   16

/* REGISTER rdma_path_reg_2 */
#define RDMA_PATH_AXI_RCACHE_OFFSET                                   0x08
#define RDMA_PATH_AXI_RCACHE_SHIFT                                    0
#define RDMA_PATH_AXI_RCACHE_LENGTH                                   4
#define RDMA_PATH_AXI_RREGION_SHIFT                                   4
#define RDMA_PATH_AXI_RREGION_LENGTH                                  4

/* REGISTER rdma_path_reg_3 */
#define SLICE_BREAK_ENA_OFFSET                                        0x0c
#define SLICE_BREAK_ENA_SHIFT                                         0
#define SLICE_BREAK_ENA_LENGTH                                        1
#define SLICE_LINE_NUM_SHIFT                                          16
#define SLICE_LINE_NUM_LENGTH                                         16

/* REGISTER rdma_path_reg_4 */
#define LEFT_EXT_ROI0_ENA_OFFSET                                      0x10
#define LEFT_EXT_ROI0_ENA_SHIFT                                       0
#define LEFT_EXT_ROI0_ENA_LENGTH                                      1
#define LEFT_EXT_ROI1_ENA_SHIFT                                       1
#define LEFT_EXT_ROI1_ENA_LENGTH                                      1
#define RIGHT_EXT_ROI0_ENA_SHIFT                                      2
#define RIGHT_EXT_ROI0_ENA_LENGTH                                     1
#define RIGHT_EXT_ROI1_ENA_SHIFT                                      3
#define RIGHT_EXT_ROI1_ENA_LENGTH                                     1

/* REGISTER rdma_path_reg_5 */
#define LEFT_EXT_ROI0_START_X_OFFSET                                  0x14
#define LEFT_EXT_ROI0_START_X_SHIFT                                   0
#define LEFT_EXT_ROI0_START_X_LENGTH                                  16
#define LEFT_EXT_ROI0_END_X_SHIFT                                     16
#define LEFT_EXT_ROI0_END_X_LENGTH                                    16

/* REGISTER rdma_path_reg_6 */
#define LEFT_EXT_ROI0_START_Y_OFFSET                                  0x18
#define LEFT_EXT_ROI0_START_Y_SHIFT                                   0
#define LEFT_EXT_ROI0_START_Y_LENGTH                                  16
#define LEFT_EXT_ROI0_END_Y_SHIFT                                     16
#define LEFT_EXT_ROI0_END_Y_LENGTH                                    16

/* REGISTER rdma_path_reg_7 */
#define LEFT_EXT_ROI1_START_X_OFFSET                                  0x1c
#define LEFT_EXT_ROI1_START_X_SHIFT                                   0
#define LEFT_EXT_ROI1_START_X_LENGTH                                  16
#define LEFT_EXT_ROI1_END_X_SHIFT                                     16
#define LEFT_EXT_ROI1_END_X_LENGTH                                    16

/* REGISTER rdma_path_reg_8 */
#define LEFT_EXT_ROI1_START_Y_OFFSET                                  0x20
#define LEFT_EXT_ROI1_START_Y_SHIFT                                   0
#define LEFT_EXT_ROI1_START_Y_LENGTH                                  16
#define LEFT_EXT_ROI1_END_Y_SHIFT                                     16
#define LEFT_EXT_ROI1_END_Y_LENGTH                                    16

/* REGISTER rdma_path_reg_9 */
#define RIGHT_EXT_ROI0_START_X_OFFSET                                 0x24
#define RIGHT_EXT_ROI0_START_X_SHIFT                                  0
#define RIGHT_EXT_ROI0_START_X_LENGTH                                 16
#define RIGHT_EXT_ROI0_END_X_SHIFT                                    16
#define RIGHT_EXT_ROI0_END_X_LENGTH                                   16

/* REGISTER rdma_path_reg_10 */
#define RIGHT_EXT_ROI0_START_Y_OFFSET                                 0x28
#define RIGHT_EXT_ROI0_START_Y_SHIFT                                  0
#define RIGHT_EXT_ROI0_START_Y_LENGTH                                 16
#define RIGHT_EXT_ROI0_END_Y_SHIFT                                    16
#define RIGHT_EXT_ROI0_END_Y_LENGTH                                   16

/* REGISTER rdma_path_reg_11 */
#define RIGHT_EXT_ROI1_START_X_OFFSET                                 0x2c
#define RIGHT_EXT_ROI1_START_X_SHIFT                                  0
#define RIGHT_EXT_ROI1_START_X_LENGTH                                 16
#define RIGHT_EXT_ROI1_END_X_SHIFT                                    16
#define RIGHT_EXT_ROI1_END_X_LENGTH                                   16

/* REGISTER rdma_path_reg_12 */
#define RIGHT_EXT_ROI1_START_Y_OFFSET                                 0x30
#define RIGHT_EXT_ROI1_START_Y_SHIFT                                  0
#define RIGHT_EXT_ROI1_START_Y_LENGTH                                 16
#define RIGHT_EXT_ROI1_END_Y_SHIFT                                    16
#define RIGHT_EXT_ROI1_END_Y_LENGTH                                   16

/* REGISTER rdma_path_reg_13 */
#define LEFT_BASE_ADDR0_LOW_OFFSET                                    0x34

/* REGISTER rdma_path_reg_14 */
#define LEFT_BASE_ADDR0_HIGH_OFFSET                                   0x38
#define LEFT_BASE_ADDR0_HIGH_SHIFT                                    0
#define LEFT_BASE_ADDR0_HIGH_LENGTH                                   12

/* REGISTER rdma_path_reg_15 */
#define LEFT_BASE_ADDR1_LOW_OFFSET                                    0x3c

/* REGISTER rdma_path_reg_16 */
#define LEFT_BASE_ADDR1_HIGH_OFFSET                                   0x40
#define LEFT_BASE_ADDR1_HIGH_SHIFT                                    0
#define LEFT_BASE_ADDR1_HIGH_LENGTH                                   12

/* REGISTER rdma_path_reg_17 */
#define LEFT_BASE_ADDR2_LOW_OFFSET                                    0x44

/* REGISTER rdma_path_reg_18 */
#define LEFT_BASE_ADDR2_HIGH_OFFSET                                   0x48
#define LEFT_BASE_ADDR2_HIGH_SHIFT                                    0
#define LEFT_BASE_ADDR2_HIGH_LENGTH                                   12

/* REGISTER rdma_path_reg_19 */
#define LEFT_RDMA_STRIDE0_OFFSET                                      0x4c
#define LEFT_RDMA_STRIDE0_SHIFT                                       0
#define LEFT_RDMA_STRIDE0_LENGTH                                      16
#define LEFT_RDMA_STRIDE1_SHIFT                                       16
#define LEFT_RDMA_STRIDE1_LENGTH                                      16

/* REGISTER rdma_path_reg_20 */
#define LEFT_IMG_WIDTH_OFFSET                                         0x50
#define LEFT_IMG_WIDTH_SHIFT                                          0
#define LEFT_IMG_WIDTH_LENGTH                                         16
#define LEFT_IMG_HEIGHT_SHIFT                                         16
#define LEFT_IMG_HEIGHT_LENGTH                                        16

/* REGISTER rdma_path_reg_21 */
#define LEFT_BBOX_START_X_OFFSET                                      0x54
#define LEFT_BBOX_START_X_SHIFT                                       0
#define LEFT_BBOX_START_X_LENGTH                                      16
#define LEFT_BBOX_END_X_SHIFT                                         16
#define LEFT_BBOX_END_X_LENGTH                                        16

/* REGISTER rdma_path_reg_22 */
#define LEFT_BBOX_START_Y_OFFSET                                      0x58
#define LEFT_BBOX_START_Y_SHIFT                                       0
#define LEFT_BBOX_START_Y_LENGTH                                      16
#define LEFT_BBOX_END_Y_SHIFT                                         16
#define LEFT_BBOX_END_Y_LENGTH                                        16

/* REGISTER rdma_path_reg_23 */
#define RIGHT_BASE_ADDR0_LOW_OFFSET                                   0x5c

/* REGISTER rdma_path_reg_24 */
#define RIGHT_BASE_ADDR0_HIGH_OFFSET                                  0x60
#define RIGHT_BASE_ADDR0_HIGH_SHIFT                                   0
#define RIGHT_BASE_ADDR0_HIGH_LENGTH                                  12

/* REGISTER rdma_path_reg_25 */
#define RIGHT_BASE_ADDR1_LOW_OFFSET                                   0x64

/* REGISTER rdma_path_reg_26 */
#define RIGHT_BASE_ADDR1_HIGH_OFFSET                                  0x68
#define RIGHT_BASE_ADDR1_HIGH_SHIFT                                   0
#define RIGHT_BASE_ADDR1_HIGH_LENGTH                                  12

/* REGISTER rdma_path_reg_27 */
#define RIGHT_BASE_ADDR2_LOW_OFFSET                                   0x6c

/* REGISTER rdma_path_reg_28 */
#define RIGHT_BASE_ADDR2_HIGH_OFFSET                                  0x70
#define RIGHT_BASE_ADDR2_HIGH_SHIFT                                   0
#define RIGHT_BASE_ADDR2_HIGH_LENGTH                                  12

/* REGISTER rdma_path_reg_29 */
#define RIGHT_RDMA_STRIDE0_OFFSET                                     0x74
#define RIGHT_RDMA_STRIDE0_SHIFT                                      0
#define RIGHT_RDMA_STRIDE0_LENGTH                                     16
#define RIGHT_RDMA_STRIDE1_SHIFT                                      16
#define RIGHT_RDMA_STRIDE1_LENGTH                                     16

/* REGISTER rdma_path_reg_30 */
#define RIGHT_IMG_WIDTH_OFFSET                                        0x78
#define RIGHT_IMG_WIDTH_SHIFT                                         0
#define RIGHT_IMG_WIDTH_LENGTH                                        16
#define RIGHT_IMG_HEIGHT_SHIFT                                        16
#define RIGHT_IMG_HEIGHT_LENGTH                                       16

/* REGISTER rdma_path_reg_31 */
#define RIGHT_BBOX_START_X_OFFSET                                     0x7c
#define RIGHT_BBOX_START_X_SHIFT                                      0
#define RIGHT_BBOX_START_X_LENGTH                                     16
#define RIGHT_BBOX_END_X_SHIFT                                        16
#define RIGHT_BBOX_END_X_LENGTH                                       16

/* REGISTER rdma_path_reg_32 */
#define RIGHT_BBOX_START_Y_OFFSET                                     0x80
#define RIGHT_BBOX_START_Y_SHIFT                                      0
#define RIGHT_BBOX_START_Y_LENGTH                                     16
#define RIGHT_BBOX_END_Y_SHIFT                                        16
#define RIGHT_BBOX_END_Y_LENGTH                                       16

/* REGISTER rdma_path_reg_33 */
#define PIXEL_FORMAT_OFFSET                                           0x84
#define PIXEL_FORMAT_SHIFT                                            0
#define PIXEL_FORMAT_LENGTH                                           6
#define UV_SWAP_SHIFT                                                 6
#define UV_SWAP_LENGTH                                                1
#define IS_NARROW_YUV_SHIFT                                           7
#define IS_NARROW_YUV_LENGTH                                          1
#define LEFT_ROT_MODE_SHIFT                                           8
#define LEFT_ROT_MODE_LENGTH                                          2
#define LEFT_FLIP_MODE_SHIFT                                          10
#define LEFT_FLIP_MODE_LENGTH                                         2
#define RIGHT_ROT_MODE_SHIFT                                          12
#define RIGHT_ROT_MODE_LENGTH                                         2
#define RIGHT_FLIP_MODE_SHIFT                                         14
#define RIGHT_FLIP_MODE_LENGTH                                        2

/* REGISTER rdma_path_reg_34 */
#define FBC_SPLIT_MODE_OFFSET                                         0x88
#define FBC_SPLIT_MODE_SHIFT                                          0
#define FBC_SPLIT_MODE_LENGTH                                         1
#define FBC_YUV_TRANSFORM_SHIFT                                       1
#define FBC_YUV_TRANSFORM_LENGTH                                      1
#define FBC_SB_LAYOUT_SHIFT                                           2
#define FBC_SB_LAYOUT_LENGTH                                          1
#define FBC_TILE_TYPE_SHIFT                                           3
#define FBC_TILE_TYPE_LENGTH                                          1

/* REGISTER rdma_path_reg_35 */
#define LBUF_MEM_SIZE_OFFSET                                          0x8c
#define LBUF_MEM_SIZE_SHIFT                                           0
#define LBUF_MEM_SIZE_LENGTH                                          16
#define LBUF_MEM_BASE_ADDR_SHIFT                                      16
#define LBUF_MEM_BASE_ADDR_LENGTH                                     12
#define LBUF_MEM_MAP_SHIFT                                            28
#define LBUF_MEM_MAP_LENGTH                                           1
#define LINE_NUM_BY_SW_SHIFT                                          29
#define LINE_NUM_BY_SW_LENGTH                                         1
#define SW_DEC_LINE_NUM_SHIFT                                         30
#define SW_DEC_LINE_NUM_LENGTH                                        2

/* REGISTER rdma_path_reg_36 */
#define LEFT_LBUF_MEM_SIZE_OFFSET                                     0x90
#define LEFT_LBUF_MEM_SIZE_SHIFT                                      0
#define LEFT_LBUF_MEM_SIZE_LENGTH                                     16
#define RIGHT_LBUF_MEM_SIZE_SHIFT                                     16
#define RIGHT_LBUF_MEM_SIZE_LENGTH                                    16

/* REGISTER rdma_path_reg_37 */
#define NSAID_OFFSET                                                  0x94
#define RDMA_PATH_NSAID_SHIFT                                         0
#define RDMA_PATH_NSAID_LENGTH                                        16

/* REGISTER rdma_path_reg_38 */
#define LEFT_ALPHA0_OFFSET                                            0x98
#define LEFT_ALPHA0_SHIFT                                             0
#define LEFT_ALPHA0_LENGTH                                            10
#define LEFT_ALPHA1_SHIFT                                             16
#define LEFT_ALPHA1_LENGTH                                            10

/* REGISTER rdma_path_reg_39 */
#define LEFT_ALPHA2_OFFSET                                            0x9c
#define LEFT_ALPHA2_SHIFT                                             0
#define LEFT_ALPHA2_LENGTH                                            10
#define LEFT_ALPHA3_SHIFT                                             16
#define LEFT_ALPHA3_LENGTH                                            10

/* REGISTER rdma_path_reg_40 */
#define RIGHT_ALPHA0_OFFSET                                           0xa0
#define RIGHT_ALPHA0_SHIFT                                            0
#define RIGHT_ALPHA0_LENGTH                                           10
#define RIGHT_ALPHA1_SHIFT                                            16
#define RIGHT_ALPHA1_LENGTH                                           10

/* REGISTER rdma_path_reg_41 */
#define RIGHT_ALPHA2_OFFSET                                           0xa4
#define RIGHT_ALPHA2_SHIFT                                            0
#define RIGHT_ALPHA2_LENGTH                                           10
#define RIGHT_ALPHA3_SHIFT                                            16
#define RIGHT_ALPHA3_LENGTH                                           10

/* REGISTER rdma_path_reg_42 */
#define M_NCOEFTAP4_0_OFFSET                                          0xa8
#define M_NCOEFTAP4_0_SHIFT                                           0
#define M_NCOEFTAP4_0_LENGTH                                          10
#define M_NCOEFTAP4_1_SHIFT                                           16
#define M_NCOEFTAP4_1_LENGTH                                          10

/* REGISTER rdma_path_reg_43 */
#define M_NCOEFTAP4_2_OFFSET                                          0xac
#define M_NCOEFTAP4_2_SHIFT                                           0
#define M_NCOEFTAP4_2_LENGTH                                          10
#define M_NCOEFTAP4_3_SHIFT                                           16
#define M_NCOEFTAP4_3_LENGTH                                          10

/* REGISTER rdma_path_reg_44 */
#define M_NCOEFTAP4_4_OFFSET                                          0xb0
#define M_NCOEFTAP4_4_SHIFT                                           0
#define M_NCOEFTAP4_4_LENGTH                                          10
#define M_NCOEFTAP4_5_SHIFT                                           16
#define M_NCOEFTAP4_5_LENGTH                                          10

/* REGISTER rdma_path_reg_45 */
#define M_NCOEFTAP2_0_OFFSET                                          0xb4
#define M_NCOEFTAP2_0_SHIFT                                           0
#define M_NCOEFTAP2_0_LENGTH                                          10
#define M_NCOEFTAP2_1_SHIFT                                           16
#define M_NCOEFTAP2_1_LENGTH                                          10

/* REGISTER rdma_path_reg_46 */
#define OBUF_ARQOS_LVL0_OFFSET                                        0xb8
#define OBUF_ARQOS_LVL0_SHIFT                                         0
#define OBUF_ARQOS_LVL0_LENGTH                                        4
#define OBUF_ARQOS_LVL1_SHIFT                                         4
#define OBUF_ARQOS_LVL1_LENGTH                                        4
#define OBUF_ARQOS_LVL2_SHIFT                                         8
#define OBUF_ARQOS_LVL2_LENGTH                                        4
#define OBUF_ARQOS_LVL3_SHIFT                                         12
#define OBUF_ARQOS_LVL3_LENGTH                                        4
#define RDMA_ARQOS_LVL0_SHIFT                                         16
#define RDMA_ARQOS_LVL0_LENGTH                                        4
#define RDMA_ARQOS_LVL1_SHIFT                                         20
#define RDMA_ARQOS_LVL1_LENGTH                                        4
#define RDMA_ARQOS_LVL2_SHIFT                                         24
#define RDMA_ARQOS_LVL2_LENGTH                                        4
#define RDMA_ARQOS_LVL3_SHIFT                                         28
#define RDMA_ARQOS_LVL3_LENGTH                                        4

/* REGISTER rdma_path_reg_47 */
#define RDMA_QOS_UP0_OFFSET                                           0xbc
#define RDMA_QOS_UP0_SHIFT                                            0
#define RDMA_QOS_UP0_LENGTH                                           8
#define RDMA_QOS_DOWN0_SHIFT                                          8
#define RDMA_QOS_DOWN0_LENGTH                                         8
#define RDMA_QOS_UP1_SHIFT                                            16
#define RDMA_QOS_UP1_LENGTH                                           8
#define RDMA_QOS_DOWN1_SHIFT                                          24
#define RDMA_QOS_DOWN1_LENGTH                                         8

/* REGISTER rdma_path_reg_48 */
#define RDMA_QOS_UP2_OFFSET                                           0xc0
#define RDMA_QOS_UP2_SHIFT                                            0
#define RDMA_QOS_UP2_LENGTH                                           8
#define RDMA_QOS_DOWN2_SHIFT                                          8
#define RDMA_QOS_DOWN2_LENGTH                                         8
#define RDMA_QOS_UP3_SHIFT                                            16
#define RDMA_QOS_UP3_LENGTH                                           8
#define RDMA_QOS_DOWN3_SHIFT                                          24
#define RDMA_QOS_DOWN3_LENGTH                                         8

/* REGISTER rdma_path_reg_49 */
#define RDMA_URGENT_UP_OFFSET                                         0xc4
#define RDMA_URGENT_UP_SHIFT                                          0
#define RDMA_URGENT_UP_LENGTH                                         8
#define RDMA_URGENT_DOWN_SHIFT                                        8
#define RDMA_URGENT_DOWN_LENGTH                                       8

/* REGISTER rdma_path_reg_50 */
#define SW_QOS_EN_OFFSET                                              0xc8
#define SW_QOS_EN_SHIFT                                               0
#define SW_QOS_EN_LENGTH                                              1
#define SW_URGENT_EN_SHIFT                                            1
#define SW_URGENT_EN_LENGTH                                           1
#define SW_URGENT_SHIFT                                               2
#define SW_URGENT_LENGTH                                              1
#define OFFL_PREFETCH_LINES_SHIFT                                     8
#define OFFL_PREFETCH_LINES_LENGTH                                    8
#define RCMD_COUNT_EN_SHIFT                                           16
#define RCMD_COUNT_EN_LENGTH                                          1

/* REGISTER rdma_path_reg_51 */
#define RDMA_DBG_IRQ_RAW_OFFSET                                       0xcc

/* REGISTER rdma_path_reg_52 */
#define RDMA_DBG_IRQ_MASK_OFFSET                                      0xd0

/* REGISTER rdma_path_reg_53 */
#define RDMA_DBG_IRQ_STATUS_OFFSET                                    0xd4

/* REGISTER rdma_path_reg_54 */
#define SECU_EN_OFFSET                                                0xd8
#define SECU_EN_SHIFT                                                 0
#define SECU_EN_LENGTH                                                1

/* REGISTER rdma_path_reg_55 */
#define RDMA_PATH_FORCE_UPDATE_EN_OFFSET                              0xdc
#define RDMA_PATH_FORCE_UPDATE_EN_SHIFT                               0
#define RDMA_PATH_FORCE_UPDATE_EN_LENGTH                              1
#define RDMA_PATH_VSYNC_UPDATE_EN_SHIFT                               1
#define RDMA_PATH_VSYNC_UPDATE_EN_LENGTH                              1
#define RDMA_PATH_SHADOW_READ_EN_SHIFT                                2
#define RDMA_PATH_SHADOW_READ_EN_LENGTH                               1

/* REGISTER rdma_path_reg_56 */
#define RDMA_PATH_FORCE_UPDATE_PULSE_OFFSET                           0xe0
#define RDMA_PATH_FORCE_UPDATE_PULSE_SHIFT                            0
#define RDMA_PATH_FORCE_UPDATE_PULSE_LENGTH                           1

/* REGISTER rdma_path_reg_57 */
#define RDMA_PATH_FORCE_UPDATE_EN_SE_OFFSET                           0xe4
#define RDMA_PATH_FORCE_UPDATE_EN_SE_SHIFT                            0
#define RDMA_PATH_FORCE_UPDATE_EN_SE_LENGTH                           1
#define RDMA_PATH_VSYNC_UPDATE_EN_SE_SHIFT                            1
#define RDMA_PATH_VSYNC_UPDATE_EN_SE_LENGTH                           1
#define RDMA_PATH_SHADOW_READ_EN_SE_SHIFT                             2
#define RDMA_PATH_SHADOW_READ_EN_SE_LENGTH                            1

/* REGISTER rdma_path_reg_58 */
#define RDMA_PATH_FORCE_UPDATE_PULSE_SE_OFFSET                        0xe8
#define RDMA_PATH_FORCE_UPDATE_PULSE_SE_SHIFT                         0
#define RDMA_PATH_FORCE_UPDATE_PULSE_SE_LENGTH                        1

/* REGISTER rdma_path_reg_59 */
#define RDMA_PATH_ICG_OVERRIDE_OFFSET                                 0xec
#define RDMA_PATH_ICG_OVERRIDE_SHIFT                                  0
#define RDMA_PATH_ICG_OVERRIDE_LENGTH                                 1

/* REGISTER rdma_path_reg_60 */
#define RDMA_PATH_REGBANK_RESET_OFFSET                                0xf0
#define RDMA_PATH_REGBANK_RESET_SHIFT                                 0
#define RDMA_PATH_REGBANK_RESET_LENGTH                                1

/* REGISTER rdma_path_reg_61 */
#define RDMA_PATH_REGBANK_RESET2_OFFSET                               0xf4
#define RDMA_PATH_REGBANK_RESET2_SHIFT                                0
#define RDMA_PATH_REGBANK_RESET2_LENGTH                               1

/* REGISTER rdma_path_reg_62 */
#define RDMA_PATH_INFO0_OFFSET                                        0xf8

/* REGISTER rdma_path_reg_63 */
#define RDMA_PATH_INFO1_OFFSET                                        0xfc

#endif
