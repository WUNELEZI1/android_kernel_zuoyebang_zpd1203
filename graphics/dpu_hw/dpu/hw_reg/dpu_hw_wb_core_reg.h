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

#ifndef _DPU_HW_WB_CORE_REG_H_
#define _DPU_HW_WB_CORE_REG_H_

/* REGISTER dpu_wb_core_reg_0 */
#define WB_WDMA_FBC_EN_OFFSET                                         0x00
#define WB_WDMA_FBC_EN_SHIFT                                          0
#define WB_WDMA_FBC_EN_LENGTH                                         1
#define WB_SCL_EN_SHIFT                                               1
#define WB_SCL_EN_LENGTH                                              1
#define WB_DITHER_EN_SHIFT                                            2
#define WB_DITHER_EN_LENGTH                                           1
#define WB_TMG_EN_SHIFT                                               3
#define WB_TMG_EN_LENGTH                                              1
#define WB_SLICE_HEADER_WREN_SHIFT                                    4
#define WB_SLICE_HEADER_WREN_LENGTH                                   1
#define WB_OFFLINE_EN_SHIFT                                           5
#define WB_OFFLINE_EN_LENGTH                                          1
#define WB_OUT_FORMAT_SHIFT                                           8
#define WB_OUT_FORMAT_LENGTH                                          5
#define WB_ROT_MODE_SHIFT                                             13
#define WB_ROT_MODE_LENGTH                                            2
#define WB_FLIP_MODE_SHIFT                                            16
#define WB_FLIP_MODE_LENGTH                                           2
#define WB_CROP0_EN_SHIFT                                             18
#define WB_CROP0_EN_LENGTH                                            1
#define WB_CROP1_EN_SHIFT                                             19
#define WB_CROP1_EN_LENGTH                                            1
#define WB_CROP2_EN_SHIFT                                             20
#define WB_CROP2_EN_LENGTH                                            1

/* REGISTER dpu_wb_core_reg_1 */
#define WB_IN_WIDTH_OFFSET                                            0x04
#define WB_IN_WIDTH_SHIFT                                             0
#define WB_IN_WIDTH_LENGTH                                            16
#define WB_IN_HEIGHT_SHIFT                                            16
#define WB_IN_HEIGHT_LENGTH                                           16

/* REGISTER dpu_wb_core_reg_2 */
#define WB_OUT_ORI_WIDTH_OFFSET                                       0x08
#define WB_OUT_ORI_WIDTH_SHIFT                                        0
#define WB_OUT_ORI_WIDTH_LENGTH                                       16
#define WB_OUT_ORI_HEIGHT_SHIFT                                       16
#define WB_OUT_ORI_HEIGHT_LENGTH                                      16

/* REGISTER dpu_wb_core_reg_3 */
#define WB_OUT_SUBFRAME_LTOPX_OFFSET                                  0x0c
#define WB_OUT_SUBFRAME_LTOPX_SHIFT                                   0
#define WB_OUT_SUBFRAME_LTOPX_LENGTH                                  16
#define WB_OUT_SUBFRAME_LTOPY_SHIFT                                   16
#define WB_OUT_SUBFRAME_LTOPY_LENGTH                                  16

/* REGISTER dpu_wb_core_reg_4 */
#define WB_OUT_SUBFRAME_WIDTH_OFFSET                                  0x10
#define WB_OUT_SUBFRAME_WIDTH_SHIFT                                   0
#define WB_OUT_SUBFRAME_WIDTH_LENGTH                                  16
#define WB_OUT_SUBFRAME_HEIGHT_SHIFT                                  16
#define WB_OUT_SUBFRAME_HEIGHT_LENGTH                                 16

/* REGISTER dpu_wb_core_reg_5 */
#define WB_SLICE_SIZE_OFFSET                                          0x14
#define WB_SLICE_SIZE_SHIFT                                           0
#define WB_SLICE_SIZE_LENGTH                                          16
#define WB_SLICE_NUM_SHIFT                                            16
#define WB_SLICE_NUM_LENGTH                                           16

/* REGISTER dpu_wb_core_reg_6 */
#define WB_WDMA_BASE_ADDR0_LOW_OFFSET                                 0x18

/* REGISTER dpu_wb_core_reg_7 */
#define WB_WDMA_BASE_ADDR0_HIGH_OFFSET                                0x1c
#define WB_WDMA_BASE_ADDR0_HIGH_SHIFT                                 0
#define WB_WDMA_BASE_ADDR0_HIGH_LENGTH                                12

/* REGISTER dpu_wb_core_reg_8 */
#define WB_WDMA_BASE_ADDR1_LOW_OFFSET                                 0x20

/* REGISTER dpu_wb_core_reg_9 */
#define WB_WDMA_BASE_ADDR1_HIGH_OFFSET                                0x24
#define WB_WDMA_BASE_ADDR1_HIGH_SHIFT                                 0
#define WB_WDMA_BASE_ADDR1_HIGH_LENGTH                                12

/* REGISTER dpu_wb_core_reg_10 */
#define WB_SLICEINFOR_ADDR_LOW_OFFSET                                 0x28

/* REGISTER dpu_wb_core_reg_11 */
#define WB_SLICEINFOR_ADDR_HIGH_OFFSET                                0x2c
#define WB_SLICEINFOR_ADDR_HIGH_SHIFT                                 0
#define WB_SLICEINFOR_ADDR_HIGH_LENGTH                                12

/* REGISTER dpu_wb_core_reg_12 */
#define WB_WDMA_STRIDE_OFFSET                                         0x30
#define WB_WDMA_STRIDE_SHIFT                                          0
#define WB_WDMA_STRIDE_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_13 */
#define WB_WDMA_OUTSTANDING_NUM_OFFSET                                0x34
#define WB_WDMA_OUTSTANDING_NUM_SHIFT                                 0
#define WB_WDMA_OUTSTANDING_NUM_LENGTH                                5
#define WB_WDMA_BURST_LEN_SHIFT                                       8
#define WB_WDMA_BURST_LEN_LENGTH                                      5
#define WB_WDMA_BOUNDARY_EN_SHIFT                                     13
#define WB_WDMA_BOUNDARY_EN_LENGTH                                    1
#define WB_WDMA_BURST_BOUNDARY_SHIFT                                  14
#define WB_WDMA_BURST_BOUNDARY_LENGTH                                 1

/* REGISTER dpu_wb_core_reg_14 */
#define WB_WDMA_QOS_OFFSET                                            0x38
#define WB_WDMA_QOS_SHIFT                                             0
#define WB_WDMA_QOS_LENGTH                                            4
#define WB_WDMA_REGION_SHIFT                                          4
#define WB_WDMA_REGION_LENGTH                                         4
#define WB_WDMA_CACHE_SHIFT                                           8
#define WB_WDMA_CACHE_LENGTH                                          4

/* REGISTER dpu_wb_core_reg_15 */
#define WB_WDMA_NSAID_OFFSET                                          0x3c
#define WB_WDMA_NSAID_SHIFT                                           0
#define WB_WDMA_NSAID_LENGTH                                          16
#define WB_AXI_SECU_SHIFT                                             16
#define WB_AXI_SECU_LENGTH                                            1

/* REGISTER dpu_wb_core_reg_16 */
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

/* REGISTER dpu_wb_core_reg_17 */
#define FMT_CVT_A0_OFFSET                                             0x44
#define FMT_CVT_A0_SHIFT                                              0
#define FMT_CVT_A0_LENGTH                                             8
#define FMT_CVT_A1_SHIFT                                              8
#define FMT_CVT_A1_LENGTH                                             8
#define FMT_CVT_A2_SHIFT                                              16
#define FMT_CVT_A2_LENGTH                                             8
#define FMT_CVT_NARROW_MODE_SHIFT                                     24
#define FMT_CVT_NARROW_MODE_LENGTH                                    1

/* REGISTER dpu_wb_core_reg_18 */
#define FMT_CVT_RGB2YUV_MATRIX00_OFFSET                               0x48
#define FMT_CVT_RGB2YUV_MATRIX00_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX00_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX01_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX01_LENGTH                               16

/* REGISTER dpu_wb_core_reg_19 */
#define FMT_CVT_RGB2YUV_MATRIX02_OFFSET                               0x4c
#define FMT_CVT_RGB2YUV_MATRIX02_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX02_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX03_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX03_LENGTH                               16

/* REGISTER dpu_wb_core_reg_20 */
#define FMT_CVT_RGB2YUV_MATRIX10_OFFSET                               0x50
#define FMT_CVT_RGB2YUV_MATRIX10_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX10_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX11_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX11_LENGTH                               16

/* REGISTER dpu_wb_core_reg_21 */
#define FMT_CVT_RGB2YUV_MATRIX12_OFFSET                               0x54
#define FMT_CVT_RGB2YUV_MATRIX12_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX12_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX13_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX13_LENGTH                               16

/* REGISTER dpu_wb_core_reg_22 */
#define FMT_CVT_RGB2YUV_MATRIX20_OFFSET                               0x58
#define FMT_CVT_RGB2YUV_MATRIX20_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX20_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX21_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX21_LENGTH                               16

/* REGISTER dpu_wb_core_reg_23 */
#define FMT_CVT_RGB2YUV_MATRIX22_OFFSET                               0x5c
#define FMT_CVT_RGB2YUV_MATRIX22_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX22_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX23_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX23_LENGTH                               16

/* REGISTER dpu_wb_core_reg_24 */
#define YUV_CVT_HOR_COEF0_OFFSET                                      0x60
#define YUV_CVT_HOR_COEF0_SHIFT                                       0
#define YUV_CVT_HOR_COEF0_LENGTH                                      10
#define YUV_CVT_HOR_COEF1_SHIFT                                       16
#define YUV_CVT_HOR_COEF1_LENGTH                                      10

/* REGISTER dpu_wb_core_reg_25 */
#define YUV_CVT_HOR_COEF2_OFFSET                                      0x64
#define YUV_CVT_HOR_COEF2_SHIFT                                       0
#define YUV_CVT_HOR_COEF2_LENGTH                                      10
#define YUV_CVT_VER_COEF_SHIFT                                        16
#define YUV_CVT_VER_COEF_LENGTH                                       10

/* REGISTER dpu_wb_core_reg_26 */
#define WB_DITHER_AUTO_TEMP_EN_OFFSET                                 0x68
#define WB_DITHER_AUTO_TEMP_EN_SHIFT                                  0
#define WB_DITHER_AUTO_TEMP_EN_LENGTH                                 1
#define WB_DITHER_ROT_MODE_SHIFT                                      1
#define WB_DITHER_ROT_MODE_LENGTH                                     2
#define WB_DITHER_MODE_SHIFT                                          3
#define WB_DITHER_MODE_LENGTH                                         1
#define WB_DITHER_OUT_DPTH0_SHIFT                                     4
#define WB_DITHER_OUT_DPTH0_LENGTH                                    4
#define WB_DITHER_OUT_DPTH1_SHIFT                                     8
#define WB_DITHER_OUT_DPTH1_LENGTH                                    4
#define WB_DITHER_OUT_DPTH2_SHIFT                                     12
#define WB_DITHER_OUT_DPTH2_LENGTH                                    4
#define WB_DITHER_TMP_VALUE_SHIFT                                     16
#define WB_DITHER_TMP_VALUE_LENGTH                                    8
#define WB_DITHER_PATTERN_BITS_SHIFT                                  24
#define WB_DITHER_PATTERN_BITS_LENGTH                                 3

/* REGISTER dpu_wb_core_reg_27 */
#define WB_DITHER_BAYER_MAP0_OFFSET                                   0x6c
#define WB_DITHER_BAYER_MAP0_SHIFT                                    0
#define WB_DITHER_BAYER_MAP0_LENGTH                                   8
#define WB_DITHER_BAYER_MAP1_SHIFT                                    8
#define WB_DITHER_BAYER_MAP1_LENGTH                                   8
#define WB_DITHER_BAYER_MAP2_SHIFT                                    16
#define WB_DITHER_BAYER_MAP2_LENGTH                                   8
#define WB_DITHER_BAYER_MAP3_SHIFT                                    24
#define WB_DITHER_BAYER_MAP3_LENGTH                                   8

/* REGISTER dpu_wb_core_reg_28 */
#define WB_DITHER_BAYER_MAP4_OFFSET                                   0x70
#define WB_DITHER_BAYER_MAP4_SHIFT                                    0
#define WB_DITHER_BAYER_MAP4_LENGTH                                   8
#define WB_DITHER_BAYER_MAP5_SHIFT                                    8
#define WB_DITHER_BAYER_MAP5_LENGTH                                   8
#define WB_DITHER_BAYER_MAP6_SHIFT                                    16
#define WB_DITHER_BAYER_MAP6_LENGTH                                   8
#define WB_DITHER_BAYER_MAP7_SHIFT                                    24
#define WB_DITHER_BAYER_MAP7_LENGTH                                   8

/* REGISTER dpu_wb_core_reg_29 */
#define WB_DITHER_BAYER_MAP8_OFFSET                                   0x74
#define WB_DITHER_BAYER_MAP8_SHIFT                                    0
#define WB_DITHER_BAYER_MAP8_LENGTH                                   8
#define WB_DITHER_BAYER_MAP9_SHIFT                                    8
#define WB_DITHER_BAYER_MAP9_LENGTH                                   8
#define WB_DITHER_BAYER_MAP10_SHIFT                                   16
#define WB_DITHER_BAYER_MAP10_LENGTH                                  8
#define WB_DITHER_BAYER_MAP11_SHIFT                                   24
#define WB_DITHER_BAYER_MAP11_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_30 */
#define WB_DITHER_BAYER_MAP12_OFFSET                                  0x78
#define WB_DITHER_BAYER_MAP12_SHIFT                                   0
#define WB_DITHER_BAYER_MAP12_LENGTH                                  8
#define WB_DITHER_BAYER_MAP13_SHIFT                                   8
#define WB_DITHER_BAYER_MAP13_LENGTH                                  8
#define WB_DITHER_BAYER_MAP14_SHIFT                                   16
#define WB_DITHER_BAYER_MAP14_LENGTH                                  8
#define WB_DITHER_BAYER_MAP15_SHIFT                                   24
#define WB_DITHER_BAYER_MAP15_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_31 */
#define WB_DITHER_BAYER_MAP16_OFFSET                                  0x7c
#define WB_DITHER_BAYER_MAP16_SHIFT                                   0
#define WB_DITHER_BAYER_MAP16_LENGTH                                  8
#define WB_DITHER_BAYER_MAP17_SHIFT                                   8
#define WB_DITHER_BAYER_MAP17_LENGTH                                  8
#define WB_DITHER_BAYER_MAP18_SHIFT                                   16
#define WB_DITHER_BAYER_MAP18_LENGTH                                  8
#define WB_DITHER_BAYER_MAP19_SHIFT                                   24
#define WB_DITHER_BAYER_MAP19_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_32 */
#define WB_DITHER_BAYER_MAP20_OFFSET                                  0x80
#define WB_DITHER_BAYER_MAP20_SHIFT                                   0
#define WB_DITHER_BAYER_MAP20_LENGTH                                  8
#define WB_DITHER_BAYER_MAP21_SHIFT                                   8
#define WB_DITHER_BAYER_MAP21_LENGTH                                  8
#define WB_DITHER_BAYER_MAP22_SHIFT                                   16
#define WB_DITHER_BAYER_MAP22_LENGTH                                  8
#define WB_DITHER_BAYER_MAP23_SHIFT                                   24
#define WB_DITHER_BAYER_MAP23_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_33 */
#define WB_DITHER_BAYER_MAP24_OFFSET                                  0x84
#define WB_DITHER_BAYER_MAP24_SHIFT                                   0
#define WB_DITHER_BAYER_MAP24_LENGTH                                  8
#define WB_DITHER_BAYER_MAP25_SHIFT                                   8
#define WB_DITHER_BAYER_MAP25_LENGTH                                  8
#define WB_DITHER_BAYER_MAP26_SHIFT                                   16
#define WB_DITHER_BAYER_MAP26_LENGTH                                  8
#define WB_DITHER_BAYER_MAP27_SHIFT                                   24
#define WB_DITHER_BAYER_MAP27_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_34 */
#define WB_DITHER_BAYER_MAP28_OFFSET                                  0x88
#define WB_DITHER_BAYER_MAP28_SHIFT                                   0
#define WB_DITHER_BAYER_MAP28_LENGTH                                  8
#define WB_DITHER_BAYER_MAP29_SHIFT                                   8
#define WB_DITHER_BAYER_MAP29_LENGTH                                  8
#define WB_DITHER_BAYER_MAP30_SHIFT                                   16
#define WB_DITHER_BAYER_MAP30_LENGTH                                  8
#define WB_DITHER_BAYER_MAP31_SHIFT                                   24
#define WB_DITHER_BAYER_MAP31_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_35 */
#define WB_DITHER_BAYER_MAP32_OFFSET                                  0x8c
#define WB_DITHER_BAYER_MAP32_SHIFT                                   0
#define WB_DITHER_BAYER_MAP32_LENGTH                                  8
#define WB_DITHER_BAYER_MAP33_SHIFT                                   8
#define WB_DITHER_BAYER_MAP33_LENGTH                                  8
#define WB_DITHER_BAYER_MAP34_SHIFT                                   16
#define WB_DITHER_BAYER_MAP34_LENGTH                                  8
#define WB_DITHER_BAYER_MAP35_SHIFT                                   24
#define WB_DITHER_BAYER_MAP35_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_36 */
#define WB_DITHER_BAYER_MAP36_OFFSET                                  0x90
#define WB_DITHER_BAYER_MAP36_SHIFT                                   0
#define WB_DITHER_BAYER_MAP36_LENGTH                                  8
#define WB_DITHER_BAYER_MAP37_SHIFT                                   8
#define WB_DITHER_BAYER_MAP37_LENGTH                                  8
#define WB_DITHER_BAYER_MAP38_SHIFT                                   16
#define WB_DITHER_BAYER_MAP38_LENGTH                                  8
#define WB_DITHER_BAYER_MAP39_SHIFT                                   24
#define WB_DITHER_BAYER_MAP39_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_37 */
#define WB_DITHER_BAYER_MAP40_OFFSET                                  0x94
#define WB_DITHER_BAYER_MAP40_SHIFT                                   0
#define WB_DITHER_BAYER_MAP40_LENGTH                                  8
#define WB_DITHER_BAYER_MAP41_SHIFT                                   8
#define WB_DITHER_BAYER_MAP41_LENGTH                                  8
#define WB_DITHER_BAYER_MAP42_SHIFT                                   16
#define WB_DITHER_BAYER_MAP42_LENGTH                                  8
#define WB_DITHER_BAYER_MAP43_SHIFT                                   24
#define WB_DITHER_BAYER_MAP43_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_38 */
#define WB_DITHER_BAYER_MAP44_OFFSET                                  0x98
#define WB_DITHER_BAYER_MAP44_SHIFT                                   0
#define WB_DITHER_BAYER_MAP44_LENGTH                                  8
#define WB_DITHER_BAYER_MAP45_SHIFT                                   8
#define WB_DITHER_BAYER_MAP45_LENGTH                                  8
#define WB_DITHER_BAYER_MAP46_SHIFT                                   16
#define WB_DITHER_BAYER_MAP46_LENGTH                                  8
#define WB_DITHER_BAYER_MAP47_SHIFT                                   24
#define WB_DITHER_BAYER_MAP47_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_39 */
#define WB_DITHER_BAYER_MAP48_OFFSET                                  0x9c
#define WB_DITHER_BAYER_MAP48_SHIFT                                   0
#define WB_DITHER_BAYER_MAP48_LENGTH                                  8
#define WB_DITHER_BAYER_MAP49_SHIFT                                   8
#define WB_DITHER_BAYER_MAP49_LENGTH                                  8
#define WB_DITHER_BAYER_MAP50_SHIFT                                   16
#define WB_DITHER_BAYER_MAP50_LENGTH                                  8
#define WB_DITHER_BAYER_MAP51_SHIFT                                   24
#define WB_DITHER_BAYER_MAP51_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_40 */
#define WB_DITHER_BAYER_MAP52_OFFSET                                  0xa0
#define WB_DITHER_BAYER_MAP52_SHIFT                                   0
#define WB_DITHER_BAYER_MAP52_LENGTH                                  8
#define WB_DITHER_BAYER_MAP53_SHIFT                                   8
#define WB_DITHER_BAYER_MAP53_LENGTH                                  8
#define WB_DITHER_BAYER_MAP54_SHIFT                                   16
#define WB_DITHER_BAYER_MAP54_LENGTH                                  8
#define WB_DITHER_BAYER_MAP55_SHIFT                                   24
#define WB_DITHER_BAYER_MAP55_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_41 */
#define WB_DITHER_BAYER_MAP56_OFFSET                                  0xa4
#define WB_DITHER_BAYER_MAP56_SHIFT                                   0
#define WB_DITHER_BAYER_MAP56_LENGTH                                  8
#define WB_DITHER_BAYER_MAP57_SHIFT                                   8
#define WB_DITHER_BAYER_MAP57_LENGTH                                  8
#define WB_DITHER_BAYER_MAP58_SHIFT                                   16
#define WB_DITHER_BAYER_MAP58_LENGTH                                  8
#define WB_DITHER_BAYER_MAP59_SHIFT                                   24
#define WB_DITHER_BAYER_MAP59_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_42 */
#define WB_DITHER_BAYER_MAP60_OFFSET                                  0xa8
#define WB_DITHER_BAYER_MAP60_SHIFT                                   0
#define WB_DITHER_BAYER_MAP60_LENGTH                                  8
#define WB_DITHER_BAYER_MAP61_SHIFT                                   8
#define WB_DITHER_BAYER_MAP61_LENGTH                                  8
#define WB_DITHER_BAYER_MAP62_SHIFT                                   16
#define WB_DITHER_BAYER_MAP62_LENGTH                                  8
#define WB_DITHER_BAYER_MAP63_SHIFT                                   24
#define WB_DITHER_BAYER_MAP63_LENGTH                                  8

/* REGISTER dpu_wb_core_reg_43 */
#define WB_TMG_MODE_OFFSET                                            0xac
#define WB_TMG_MODE_SHIFT                                             0
#define WB_TMG_MODE_LENGTH                                            1
#define WB_TMG_FIRST_FRAME_SHIFT                                      1
#define WB_TMG_FIRST_FRAME_LENGTH                                     1
#define WB_TMG_LAST_FRAME_SHIFT                                       2
#define WB_TMG_LAST_FRAME_LENGTH                                      1

/* REGISTER dpu_wb_core_reg_44 */
#define WB_TMG_VSYNC_OFFSET                                           0xb0
#define WB_TMG_VSYNC_SHIFT                                            0
#define WB_TMG_VSYNC_LENGTH                                           16
#define WB_TMG_VFP_SHIFT                                              16
#define WB_TMG_VFP_LENGTH                                             16

/* REGISTER dpu_wb_core_reg_45 */
#define WB_TMG_VACTIVE_OFFSET                                         0xb4
#define WB_TMG_VACTIVE_SHIFT                                          0
#define WB_TMG_VACTIVE_LENGTH                                         16
#define WB_TMG_VBP_SHIFT                                              16
#define WB_TMG_VBP_LENGTH                                             16

/* REGISTER dpu_wb_core_reg_46 */
#define WB_TMG_HACTIVE_OFFSET                                         0xb8
#define WB_TMG_HACTIVE_SHIFT                                          0
#define WB_TMG_HACTIVE_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_47 */
#define BUF_LVL_QOS_TH_UP0_OFFSET                                     0xbc
#define BUF_LVL_QOS_TH_UP0_SHIFT                                      0
#define BUF_LVL_QOS_TH_UP0_LENGTH                                     16
#define BUF_LVL_QOS_TH_DOWN0_SHIFT                                    16
#define BUF_LVL_QOS_TH_DOWN0_LENGTH                                   16

/* REGISTER dpu_wb_core_reg_48 */
#define BUF_LVL_QOS_TH_UP1_OFFSET                                     0xc0
#define BUF_LVL_QOS_TH_UP1_SHIFT                                      0
#define BUF_LVL_QOS_TH_UP1_LENGTH                                     16
#define BUF_LVL_QOS_TH_DOWN1_SHIFT                                    16
#define BUF_LVL_QOS_TH_DOWN1_LENGTH                                   16

/* REGISTER dpu_wb_core_reg_49 */
#define BUF_LVL_QOS_TH_UP2_OFFSET                                     0xc4
#define BUF_LVL_QOS_TH_UP2_SHIFT                                      0
#define BUF_LVL_QOS_TH_UP2_LENGTH                                     16
#define BUF_LVL_QOS_TH_DOWN2_SHIFT                                    16
#define BUF_LVL_QOS_TH_DOWN2_LENGTH                                   16

/* REGISTER dpu_wb_core_reg_50 */
#define BUF_LVL_URGENT_TH_UP_OFFSET                                   0xc8
#define BUF_LVL_URGENT_TH_UP_SHIFT                                    0
#define BUF_LVL_URGENT_TH_UP_LENGTH                                   16
#define BUF_LVL_URGENT_TH_DOWN_SHIFT                                  16
#define BUF_LVL_URGENT_TH_DOWN_LENGTH                                 16

/* REGISTER dpu_wb_core_reg_51 */
#define WB_CROP0_LTOPX_OFFSET                                         0xcc
#define WB_CROP0_LTOPX_SHIFT                                          0
#define WB_CROP0_LTOPX_LENGTH                                         16
#define WB_CROP0_LTOPY_SHIFT                                          16
#define WB_CROP0_LTOPY_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_52 */
#define WB_CROP0_RBOTX_OFFSET                                         0xd0
#define WB_CROP0_RBOTX_SHIFT                                          0
#define WB_CROP0_RBOTX_LENGTH                                         16
#define WB_CROP0_RBOTY_SHIFT                                          16
#define WB_CROP0_RBOTY_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_53 */
#define WB_CROP1_LTOPX_OFFSET                                         0xd4
#define WB_CROP1_LTOPX_SHIFT                                          0
#define WB_CROP1_LTOPX_LENGTH                                         16
#define WB_CROP1_LTOPY_SHIFT                                          16
#define WB_CROP1_LTOPY_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_54 */
#define WB_CROP1_RBOTX_OFFSET                                         0xd8
#define WB_CROP1_RBOTX_SHIFT                                          0
#define WB_CROP1_RBOTX_LENGTH                                         16
#define WB_CROP1_RBOTY_SHIFT                                          16
#define WB_CROP1_RBOTY_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_55 */
#define WB_CROP2_LTOPX_OFFSET                                         0xdc
#define WB_CROP2_LTOPX_SHIFT                                          0
#define WB_CROP2_LTOPX_LENGTH                                         16
#define WB_CROP2_LTOPY_SHIFT                                          16
#define WB_CROP2_LTOPY_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_56 */
#define WB_CROP2_RBOTX_OFFSET                                         0xe0
#define WB_CROP2_RBOTX_SHIFT                                          0
#define WB_CROP2_RBOTX_LENGTH                                         16
#define WB_CROP2_RBOTY_SHIFT                                          16
#define WB_CROP2_RBOTY_LENGTH                                         16

/* REGISTER dpu_wb_core_reg_57 */
#define WB_CORE_MEM_LP_AUTO_EN_OFFSET                                 0xe4
#define WB_CORE_MEM_LP_AUTO_EN_SHIFT                                  0
#define WB_CORE_MEM_LP_AUTO_EN_LENGTH                                 6

/* REGISTER dpu_wb_core_reg_58 */
#define WB_CORE_FORCE_UPDATE_EN_OFFSET                                0xe8
#define WB_CORE_FORCE_UPDATE_EN_SHIFT                                 0
#define WB_CORE_FORCE_UPDATE_EN_LENGTH                                1
#define WB_CORE_VSYNC_UPDATE_EN_SHIFT                                 1
#define WB_CORE_VSYNC_UPDATE_EN_LENGTH                                1
#define WB_CORE_SHADOW_READ_EN_SHIFT                                  2
#define WB_CORE_SHADOW_READ_EN_LENGTH                                 1

/* REGISTER dpu_wb_core_reg_59 */
#define WB_CORE_FORCE_UPDATE_PULSE_OFFSET                             0xec
#define WB_CORE_FORCE_UPDATE_PULSE_SHIFT                              0
#define WB_CORE_FORCE_UPDATE_PULSE_LENGTH                             1

/* REGISTER dpu_wb_core_reg_60 */
#define WB_CORE_FORCE_UPDATE_EN_SE_OFFSET                             0xf0
#define WB_CORE_FORCE_UPDATE_EN_SE_SHIFT                              0
#define WB_CORE_FORCE_UPDATE_EN_SE_LENGTH                             1
#define WB_CORE_VSYNC_UPDATE_EN_SE_SHIFT                              1
#define WB_CORE_VSYNC_UPDATE_EN_SE_LENGTH                             1
#define WB_CORE_SHADOW_READ_EN_SE_SHIFT                               2
#define WB_CORE_SHADOW_READ_EN_SE_LENGTH                              1

/* REGISTER dpu_wb_core_reg_61 */
#define WB_CORE_FORCE_UPDATE_PULSE_SE_OFFSET                          0xf4
#define WB_CORE_FORCE_UPDATE_PULSE_SE_SHIFT                           0
#define WB_CORE_FORCE_UPDATE_PULSE_SE_LENGTH                          1

/* REGISTER dpu_wb_core_reg_62 */
#define WB_CORE_ICG_OVERRIDE_OFFSET                                   0xf8
#define WB_CORE_ICG_OVERRIDE_SHIFT                                    0
#define WB_CORE_ICG_OVERRIDE_LENGTH                                   1

/* REGISTER dpu_wb_core_reg_63 */
#define WB_CORE_REG_VALUE_RST_OFFSET                                  0xfc
#define WB_CORE_REG_VALUE_RST_SHIFT                                   0
#define WB_CORE_REG_VALUE_RST_LENGTH                                  1

/* REGISTER dpu_wb_core_reg_64 */
#define WB_CORE_REG_VALUE_RST_SE_OFFSET                               0x100
#define WB_CORE_REG_VALUE_RST_SE_SHIFT                                0
#define WB_CORE_REG_VALUE_RST_SE_LENGTH                               1

/* REGISTER dpu_wb_core_reg_65 */
#define WB_DBG_BUS0_OFFSET                                            0x104

/* REGISTER dpu_wb_core_reg_66 */
#define WB_DBG_BUS1_OFFSET                                            0x108

/* REGISTER dpu_wb_core_reg_67 */
#define WB_DBG_BUS2_OFFSET                                            0x10c

/* REGISTER dpu_wb_core_reg_68 */
#define WB_DBG_BUS3_OFFSET                                            0x110

/* REGISTER dpu_wb_core_reg_69 */
#define WB_DBG_BUS4_OFFSET                                            0x114

/* REGISTER dpu_wb_core_reg_70 */
#define WB_TMG_SW_CLR_OFFSET                                          0x118
#define WB_TMG_SW_CLR_SHIFT                                           0
#define WB_TMG_SW_CLR_LENGTH                                          1

/* REGISTER dpu_wb_core_reg_71 */
#define WB_CORE_RO_TRIG_ENABLE_OFFSET                                 0x11c
#define WB_CORE_RO_TRIG_ENABLE_SHIFT                                  0
#define WB_CORE_RO_TRIG_ENABLE_LENGTH                                 1

#endif
