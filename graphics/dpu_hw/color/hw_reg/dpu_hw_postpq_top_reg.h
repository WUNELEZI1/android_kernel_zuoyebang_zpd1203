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
/* File generation time: 2023-08-25 17:24:56. */

#ifndef _DPU_HW_POSTPQ_TOP_REG_H_
#define _DPU_HW_POSTPQ_TOP_REG_H_

/* REGISTER saturn_postpq_full_reg_0 */
#define POSTPQ_TOP_EN_OFFSET                                          0x00
#define POSTPQ_FULL_TOP_R2Y_EN_SHIFT                                  0
#define POSTPQ_FULL_TOP_R2Y_EN_LENGTH                                 1
#define POSTPQ_FULL_TOP_SCALE_EN_SHIFT                                1
#define POSTPQ_FULL_TOP_SCALE_EN_LENGTH                               1
#define POSTPQ_FULL_TOP_DITHER_EN_SHIFT                               2
#define POSTPQ_FULL_TOP_DITHER_EN_LENGTH                              1
#define POSTPQ_FULL_TOP_POSTPQ_EN_SHIFT                               3
#define POSTPQ_FULL_TOP_POSTPQ_EN_LENGTH                              1
#define DITHER_LITE_EN_SHIFT                                          4
#define DITHER_LITE_EN_LENGTH                                         1
#define POSTPQ_FULL_TOP_Y2R_EN_SHIFT                                  5
#define POSTPQ_FULL_TOP_Y2R_EN_LENGTH                                 1
#define POSTPQ_FULL_TOP_DITHER_MEM_LP_EN_SHIFT                        6
#define POSTPQ_FULL_TOP_DITHER_MEM_LP_EN_LENGTH                       1
#define MATRIX_EN_SHIFT                                               7
#define MATRIX_EN_LENGTH                                              1

/* REGISTER saturn_postpq_full_reg_1 */
#define POSTPQ_FULL_TOP_M_INWIDTH_OFFSET                              0x04
#define POSTPQ_FULL_TOP_M_INWIDTH_SHIFT                               0
#define POSTPQ_FULL_TOP_M_INWIDTH_LENGTH                              16
#define POSTPQ_FULL_TOP_M_INHEIGHT_SHIFT                              16
#define POSTPQ_FULL_TOP_M_INHEIGHT_LENGTH                             16

/* REGISTER saturn_postpq_full_reg_2 */
#define M_SCALE_WIDTH_OFFSET                                          0x08
#define M_SCALE_WIDTH_SHIFT                                           0
#define M_SCALE_WIDTH_LENGTH                                          16
#define M_SCALE_HEIGHT_SHIFT                                          16
#define M_SCALE_HEIGHT_LENGTH                                         16

/* REGISTER saturn_postpq_full_reg_3 */
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX00_OFFSET                       0x0c
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX00_SHIFT                        0
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX00_LENGTH                       16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX01_SHIFT                        16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX01_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_4 */
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX02_OFFSET                       0x10
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX02_SHIFT                        0
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX02_LENGTH                       16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX03_SHIFT                        16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX03_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_5 */
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX10_OFFSET                       0x14
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX10_SHIFT                        0
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX10_LENGTH                       16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX11_SHIFT                        16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX11_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_6 */
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX12_OFFSET                       0x18
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX12_SHIFT                        0
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX12_LENGTH                       16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX13_SHIFT                        16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX13_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_7 */
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX20_OFFSET                       0x1c
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX20_SHIFT                        0
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX20_LENGTH                       16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX21_SHIFT                        16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX21_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_8 */
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX22_OFFSET                       0x20
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX22_SHIFT                        0
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX22_LENGTH                       16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX23_SHIFT                        16
#define POSTPQ_FULL_TOP_RGB2YUV_MATRIX23_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_9 */
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX00_OFFSET                       0x24
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX00_SHIFT                        0
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX00_LENGTH                       16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX01_SHIFT                        16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX01_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_10 */
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX02_OFFSET                       0x28
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX02_SHIFT                        0
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX02_LENGTH                       16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX03_SHIFT                        16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX03_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_11 */
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX10_OFFSET                       0x2c
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX10_SHIFT                        0
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX10_LENGTH                       16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX11_SHIFT                        16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX11_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_12 */
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX12_OFFSET                       0x30
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX12_SHIFT                        0
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX12_LENGTH                       16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX13_SHIFT                        16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX13_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_13 */
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX20_OFFSET                       0x34
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX20_SHIFT                        0
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX20_LENGTH                       16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX21_SHIFT                        16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX21_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_14 */
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX22_OFFSET                       0x38
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX22_SHIFT                        0
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX22_LENGTH                       16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX23_SHIFT                        16
#define POSTPQ_FULL_TOP_YUV2RGB_MATRIX23_LENGTH                       16

/* REGISTER saturn_postpq_full_reg_15 */
#define POSTPQ_FULL_TOP_DITHER_MODE_OFFSET                            0x3c
#define POSTPQ_FULL_TOP_DITHER_MODE_SHIFT                             0
#define POSTPQ_FULL_TOP_DITHER_MODE_LENGTH                            1
#define POSTPQ_FULL_TOP_DITHER_AUTO_TEMP_SHIFT                        1
#define POSTPQ_FULL_TOP_DITHER_AUTO_TEMP_LENGTH                       1
#define POSTPQ_FULL_TOP_DITHER_ROTATE_MODE_SHIFT                      2
#define POSTPQ_FULL_TOP_DITHER_ROTATE_MODE_LENGTH                     2
#define POSTPQ_FULL_TOP_DITHER_OUT_DPTH0_SHIFT                        4
#define POSTPQ_FULL_TOP_DITHER_OUT_DPTH0_LENGTH                       4
#define POSTPQ_FULL_TOP_DITHER_OUT_DPTH1_SHIFT                        8
#define POSTPQ_FULL_TOP_DITHER_OUT_DPTH1_LENGTH                       4
#define POSTPQ_FULL_TOP_DITHER_OUT_DPTH2_SHIFT                        12
#define POSTPQ_FULL_TOP_DITHER_OUT_DPTH2_LENGTH                       4
#define POSTPQ_FULL_TOP_DITHER_TEMP_VALUE_SHIFT                       16
#define POSTPQ_FULL_TOP_DITHER_TEMP_VALUE_LENGTH                      8
#define POSTPQ_FULL_TOP_DITHER_PATTERN_BIT_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_PATTERN_BIT_LENGTH                     3

/* REGISTER saturn_postpq_full_reg_16 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP00_OFFSET                     0x40
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP00_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP00_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP01_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP01_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP02_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP02_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP03_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP03_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_17 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP04_OFFSET                     0x44
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP04_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP04_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP05_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP05_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP06_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP06_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP07_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP07_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_18 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP10_OFFSET                     0x48
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP10_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP10_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP11_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP11_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP12_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP12_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP13_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP13_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_19 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP14_OFFSET                     0x4c
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP14_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP14_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP15_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP15_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP16_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP16_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP17_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP17_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_20 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP20_OFFSET                     0x50
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP20_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP20_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP21_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP21_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP22_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP22_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP23_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP23_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_21 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP24_OFFSET                     0x54
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP24_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP24_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP25_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP25_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP26_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP26_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP27_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP27_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_22 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP30_OFFSET                     0x58
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP30_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP30_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP31_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP31_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP32_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP32_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP33_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP33_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_23 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP34_OFFSET                     0x5c
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP34_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP34_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP35_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP35_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP36_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP36_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP37_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP37_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_24 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP40_OFFSET                     0x60
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP40_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP40_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP41_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP41_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP42_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP42_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP43_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP43_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_25 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP44_OFFSET                     0x64
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP44_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP44_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP45_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP45_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP46_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP46_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP47_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP47_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_26 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP50_OFFSET                     0x68
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP50_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP50_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP51_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP51_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP52_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP52_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP53_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP53_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_27 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP54_OFFSET                     0x6c
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP54_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP54_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP55_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP55_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP56_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP56_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP57_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP57_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_28 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP60_OFFSET                     0x70
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP60_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP60_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP61_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP61_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP62_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP62_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP63_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP63_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_29 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP64_OFFSET                     0x74
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP64_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP64_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP65_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP65_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP66_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP66_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP67_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP67_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_30 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP70_OFFSET                     0x78
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP70_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP70_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP71_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP71_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP72_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP72_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP73_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP73_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_31 */
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP74_OFFSET                     0x7c
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP74_SHIFT                      0
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP74_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP75_SHIFT                      8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP75_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP76_SHIFT                      16
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP76_LENGTH                     8
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP77_SHIFT                      24
#define POSTPQ_FULL_TOP_DITHER_BAYER_MAP77_LENGTH                     8

/* REGISTER saturn_postpq_full_reg_32 */
#define PREPROC_LOC_OFFSET                                            0x80

/* REGISTER saturn_postpq_full_reg_33 */
#define POSTPQ_FULL_TOP_DITHER_PROC_LOC_OFFSET                        0x84

/* REGISTER saturn_postpq_full_reg_34 */
#define CSC_MATRIX00_OFFSET                                           0x88
#define CSC_MATRIX00_SHIFT                                            0
#define CSC_MATRIX00_LENGTH                                           12
#define CSC_MATRIX01_SHIFT                                            16
#define CSC_MATRIX01_LENGTH                                           12

/* REGISTER saturn_postpq_full_reg_35 */
#define CSC_MATRIX02_OFFSET                                           0x8c
#define CSC_MATRIX02_SHIFT                                            0
#define CSC_MATRIX02_LENGTH                                           12
#define CSC_MATRIX10_SHIFT                                            16
#define CSC_MATRIX10_LENGTH                                           12

/* REGISTER saturn_postpq_full_reg_36 */
#define CSC_MATRIX11_OFFSET                                           0x90
#define CSC_MATRIX11_SHIFT                                            0
#define CSC_MATRIX11_LENGTH                                           12
#define CSC_MATRIX12_SHIFT                                            16
#define CSC_MATRIX12_LENGTH                                           12

/* REGISTER saturn_postpq_full_reg_37 */
#define CSC_MATRIX20_OFFSET                                           0x94
#define CSC_MATRIX20_SHIFT                                            0
#define CSC_MATRIX20_LENGTH                                           12
#define CSC_MATRIX21_SHIFT                                            16
#define CSC_MATRIX21_LENGTH                                           12

/* REGISTER saturn_postpq_full_reg_38 */
#define CSC_MATRIX22_OFFSET                                           0x98
#define CSC_MATRIX22_SHIFT                                            0
#define CSC_MATRIX22_LENGTH                                           12


/* REGISTER saturn_postpq_full_reg_46 */
#define POSTPQ_FULL_TOP_FORCE_UPDATE_EN_OFFSET                        0xb8
#define POSTPQ_FULL_TOP_FORCE_UPDATE_EN_SHIFT                         0
#define POSTPQ_FULL_TOP_FORCE_UPDATE_EN_LENGTH                        1
#define POSTPQ_FULL_TOP_VSYNC_UPDATE_EN_SHIFT                         1
#define POSTPQ_FULL_TOP_VSYNC_UPDATE_EN_LENGTH                        1
#define POSTPQ_FULL_TOP_SHADOW_READ_EN_SHIFT                          2
#define POSTPQ_FULL_TOP_SHADOW_READ_EN_LENGTH                         1

/* REGISTER saturn_postpq_full_reg_47 */
#define POSTPQ_FULL_TOP_FORCE_UPDATE_PULSE_OFFSET                     0xbc
#define POSTPQ_FULL_TOP_FORCE_UPDATE_PULSE_SHIFT                      0
#define POSTPQ_FULL_TOP_FORCE_UPDATE_PULSE_LENGTH                     1

/* REGISTER saturn_postpq_full_reg_48 */
#define POSTPQ_FULL_TOP_ICG_OVERRIDE_OFFSET                           0xc0
#define POSTPQ_FULL_TOP_ICG_OVERRIDE_SHIFT                            0
#define POSTPQ_FULL_TOP_ICG_OVERRIDE_LENGTH                           1

/* REGISTER saturn_postpq_full_reg_49 */
#define POSTPQ_FULL_TOP_TRIGGER_OFFSET                                0xc4
#define POSTPQ_FULL_TOP_TRIGGER_SHIFT                                 0
#define POSTPQ_FULL_TOP_TRIGGER_LENGTH                                1

/* REGISTER saturn_postpq_full_reg_50 */
#define POSTPQ_FULL_TOP_TRIGGER2_OFFSET                               0xc8
#define POSTPQ_FULL_TOP_TRIGGER2_SHIFT                                0
#define POSTPQ_FULL_TOP_TRIGGER2_LENGTH                               1

#endif
