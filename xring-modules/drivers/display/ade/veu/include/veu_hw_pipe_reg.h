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

#ifndef _XRING_VEU_HW_PIPE_REG_REG_H_
#define _XRING_VEU_HW_PIPE_REG_REG_H_

/* REGISTER veu_pipe_reg_0 */
#define PREALPHA_EN_OFFSET                                            0x00
#define PREALPHA_EN_SHIFT                                             0
#define PREALPHA_EN_LENGTH                                            1
#define NONPREALPHA_EN_SHIFT                                          1
#define NONPREALPHA_EN_LENGTH                                         1
#define CSC_EN_SHIFT                                                  2
#define CSC_EN_LENGTH                                                 1
#define DITHER_EN_SHIFT                                               3
#define DITHER_EN_LENGTH                                              1
#define YUV2RGB_EN_SHIFT                                              4
#define YUV2RGB_EN_LENGTH                                             1
#define RGB2YUV_EN_SHIFT                                              5
#define RGB2YUV_EN_LENGTH                                             1
#define BIT_WITH_SHIFT                                                6
#define BIT_WITH_LENGTH                                               1
#define BIT_UP_SHIFT                                                  7
#define BIT_UP_LENGTH                                                 1
#define CROP1_EN_SHIFT                                                8
#define CROP1_EN_LENGTH                                               1
#define CROP2_EN_SHIFT                                                9
#define CROP2_EN_LENGTH                                               1
#define ALPHA_DISABLE_SHIFT                                           10
#define ALPHA_DISABLE_LENGTH                                          1
#define VEU_PIPE_REG_MEM_LP_AUTO_EN_SHIFT                             11
#define VEU_PIPE_REG_MEM_LP_AUTO_EN_LENGTH                            1

/* REGISTER veu_pipe_reg_1 */
#define DATA_FORMAT_OFFSET                                            0x04
#define DATA_FORMAT_SHIFT                                             0
#define DATA_FORMAT_LENGTH                                            2

/* REGISTER veu_pipe_reg_2 */
#define MHSIZE_CROP1_OFFSET                                           0x08
#define MHSIZE_CROP1_SHIFT                                            0
#define MHSIZE_CROP1_LENGTH                                           16
#define MVSIZE_CROP1_SHIFT                                            16
#define MVSIZE_CROP1_LENGTH                                           16

/* REGISTER veu_pipe_reg_3 */
#define MHSIZE_CROP2_OFFSET                                           0x0c
#define MHSIZE_CROP2_SHIFT                                            0
#define MHSIZE_CROP2_LENGTH                                           16
#define MVSIZE_CROP2_SHIFT                                            16
#define MVSIZE_CROP2_LENGTH                                           16

/* REGISTER veu_pipe_reg_4 */
#define NONPREALPHA_CG_AUTO_EN_OFFSET                                 0x10
#define NONPREALPHA_CG_AUTO_EN_SHIFT                                  0
#define NONPREALPHA_CG_AUTO_EN_LENGTH                                 1
#define PREALPHA_CG_AUTO_EN_SHIFT                                     1
#define PREALPHA_CG_AUTO_EN_LENGTH                                    1


/* REGISTER veu_pipe_reg_9 */
#define RGB2YUV_MATRIX00_OFFSET                                       0x24
#define RGB2YUV_MATRIX00_SHIFT                                        0
#define RGB2YUV_MATRIX00_LENGTH                                       16
#define RGB2YUV_MATRIX01_SHIFT                                        16
#define RGB2YUV_MATRIX01_LENGTH                                       16

/* REGISTER veu_pipe_reg_10 */
#define RGB2YUV_MATRIX02_OFFSET                                       0x28
#define RGB2YUV_MATRIX02_SHIFT                                        0
#define RGB2YUV_MATRIX02_LENGTH                                       16
#define RGB2YUV_MATRIX03_SHIFT                                        16
#define RGB2YUV_MATRIX03_LENGTH                                       16

/* REGISTER veu_pipe_reg_11 */
#define RGB2YUV_MATRIX10_OFFSET                                       0x2c
#define RGB2YUV_MATRIX10_SHIFT                                        0
#define RGB2YUV_MATRIX10_LENGTH                                       16
#define RGB2YUV_MATRIX11_SHIFT                                        16
#define RGB2YUV_MATRIX11_LENGTH                                       16

/* REGISTER veu_pipe_reg_12 */
#define RGB2YUV_MATRIX12_OFFSET                                       0x30
#define RGB2YUV_MATRIX12_SHIFT                                        0
#define RGB2YUV_MATRIX12_LENGTH                                       16
#define RGB2YUV_MATRIX13_SHIFT                                        16
#define RGB2YUV_MATRIX13_LENGTH                                       16

/* REGISTER veu_pipe_reg_13 */
#define RGB2YUV_MATRIX20_OFFSET                                       0x34
#define RGB2YUV_MATRIX20_SHIFT                                        0
#define RGB2YUV_MATRIX20_LENGTH                                       16
#define RGB2YUV_MATRIX21_SHIFT                                        16
#define RGB2YUV_MATRIX21_LENGTH                                       16

/* REGISTER veu_pipe_reg_14 */
#define RGB2YUV_MATRIX22_OFFSET                                       0x38
#define RGB2YUV_MATRIX22_SHIFT                                        0
#define RGB2YUV_MATRIX22_LENGTH                                       16
#define RGB2YUV_MATRIX23_SHIFT                                        16
#define RGB2YUV_MATRIX23_LENGTH                                       16

/* REGISTER veu_pipe_reg_15 */
#define YUV2RGB_MATRIX00_OFFSET                                       0x3c
#define YUV2RGB_MATRIX00_SHIFT                                        0
#define YUV2RGB_MATRIX00_LENGTH                                       16
#define YUV2RGB_MATRIX01_SHIFT                                        16
#define YUV2RGB_MATRIX01_LENGTH                                       16

/* REGISTER veu_pipe_reg_16 */
#define YUV2RGB_MATRIX02_OFFSET                                       0x40
#define YUV2RGB_MATRIX02_SHIFT                                        0
#define YUV2RGB_MATRIX02_LENGTH                                       16
#define YUV2RGB_MATRIX03_SHIFT                                        16
#define YUV2RGB_MATRIX03_LENGTH                                       16

/* REGISTER veu_pipe_reg_17 */
#define YUV2RGB_MATRIX10_OFFSET                                       0x44
#define YUV2RGB_MATRIX10_SHIFT                                        0
#define YUV2RGB_MATRIX10_LENGTH                                       16
#define YUV2RGB_MATRIX11_SHIFT                                        16
#define YUV2RGB_MATRIX11_LENGTH                                       16

/* REGISTER veu_pipe_reg_18 */
#define YUV2RGB_MATRIX12_OFFSET                                       0x48
#define YUV2RGB_MATRIX12_SHIFT                                        0
#define YUV2RGB_MATRIX12_LENGTH                                       16
#define YUV2RGB_MATRIX13_SHIFT                                        16
#define YUV2RGB_MATRIX13_LENGTH                                       16

/* REGISTER veu_pipe_reg_19 */
#define YUV2RGB_MATRIX20_OFFSET                                       0x4c
#define YUV2RGB_MATRIX20_SHIFT                                        0
#define YUV2RGB_MATRIX20_LENGTH                                       16
#define YUV2RGB_MATRIX21_SHIFT                                        16
#define YUV2RGB_MATRIX21_LENGTH                                       16

/* REGISTER veu_pipe_reg_20 */
#define YUV2RGB_MATRIX22_OFFSET                                       0x50
#define YUV2RGB_MATRIX22_SHIFT                                        0
#define YUV2RGB_MATRIX22_LENGTH                                       16
#define YUV2RGB_MATRIX23_SHIFT                                        16
#define YUV2RGB_MATRIX23_LENGTH                                       16

/* REGISTER veu_pipe_reg_21 */
#define CROP2_X_LEFT_OFFSET                                           0x54
#define CROP2_X_LEFT_SHIFT                                            0
#define CROP2_X_LEFT_LENGTH                                           16
#define CROP2_X_RIGHT_SHIFT                                           16
#define CROP2_X_RIGHT_LENGTH                                          16

/* REGISTER veu_pipe_reg_22 */
#define CROP2_Y_TOP_OFFSET                                            0x58
#define CROP2_Y_TOP_SHIFT                                             0
#define CROP2_Y_TOP_LENGTH                                            16
#define CROP2_Y_BOT_SHIFT                                             16
#define CROP2_Y_BOT_LENGTH                                            16

/* REGISTER veu_pipe_reg_23 */
#define DITHER_MODE_OFFSET                                            0x5c
#define DITHER_MODE_SHIFT                                             0
#define DITHER_MODE_LENGTH                                            1
#define DITHER_AUTO_TEMP_SHIFT                                        1
#define DITHER_AUTO_TEMP_LENGTH                                       1
#define DITHER_ROTATE_MODE_SHIFT                                      2
#define DITHER_ROTATE_MODE_LENGTH                                     2
#define DITHER_OUT_DPTH0_SHIFT                                        4
#define DITHER_OUT_DPTH0_LENGTH                                       4
#define DITHER_OUT_DPTH1_SHIFT                                        8
#define DITHER_OUT_DPTH1_LENGTH                                       4
#define DITHER_OUT_DPTH2_SHIFT                                        12
#define DITHER_OUT_DPTH2_LENGTH                                       4
#define DITHER_TEMP_VALUE_SHIFT                                       16
#define DITHER_TEMP_VALUE_LENGTH                                      8
#define DITHER_PATTERN_BIT_SHIFT                                      24
#define DITHER_PATTERN_BIT_LENGTH                                     3

/* REGISTER veu_pipe_reg_24 */
#define DITHER_BAYER_MAP00_OFFSET                                     0x60
#define DITHER_BAYER_MAP00_SHIFT                                      0
#define DITHER_BAYER_MAP00_LENGTH                                     8
#define DITHER_BAYER_MAP01_SHIFT                                      8
#define DITHER_BAYER_MAP01_LENGTH                                     8
#define DITHER_BAYER_MAP02_SHIFT                                      16
#define DITHER_BAYER_MAP02_LENGTH                                     8
#define DITHER_BAYER_MAP03_SHIFT                                      24
#define DITHER_BAYER_MAP03_LENGTH                                     8

/* REGISTER veu_pipe_reg_25 */
#define DITHER_BAYER_MAP04_OFFSET                                     0x64
#define DITHER_BAYER_MAP04_SHIFT                                      0
#define DITHER_BAYER_MAP04_LENGTH                                     8
#define DITHER_BAYER_MAP05_SHIFT                                      8
#define DITHER_BAYER_MAP05_LENGTH                                     8
#define DITHER_BAYER_MAP06_SHIFT                                      16
#define DITHER_BAYER_MAP06_LENGTH                                     8
#define DITHER_BAYER_MAP07_SHIFT                                      24
#define DITHER_BAYER_MAP07_LENGTH                                     8

/* REGISTER veu_pipe_reg_26 */
#define DITHER_BAYER_MAP10_OFFSET                                     0x68
#define DITHER_BAYER_MAP10_SHIFT                                      0
#define DITHER_BAYER_MAP10_LENGTH                                     8
#define DITHER_BAYER_MAP11_SHIFT                                      8
#define DITHER_BAYER_MAP11_LENGTH                                     8
#define DITHER_BAYER_MAP12_SHIFT                                      16
#define DITHER_BAYER_MAP12_LENGTH                                     8
#define DITHER_BAYER_MAP13_SHIFT                                      24
#define DITHER_BAYER_MAP13_LENGTH                                     8

/* REGISTER veu_pipe_reg_27 */
#define DITHER_BAYER_MAP14_OFFSET                                     0x6c
#define DITHER_BAYER_MAP14_SHIFT                                      0
#define DITHER_BAYER_MAP14_LENGTH                                     8
#define DITHER_BAYER_MAP15_SHIFT                                      8
#define DITHER_BAYER_MAP15_LENGTH                                     8
#define DITHER_BAYER_MAP16_SHIFT                                      16
#define DITHER_BAYER_MAP16_LENGTH                                     8
#define DITHER_BAYER_MAP17_SHIFT                                      24
#define DITHER_BAYER_MAP17_LENGTH                                     8

/* REGISTER veu_pipe_reg_28 */
#define DITHER_BAYER_MAP20_OFFSET                                     0x70
#define DITHER_BAYER_MAP20_SHIFT                                      0
#define DITHER_BAYER_MAP20_LENGTH                                     8
#define DITHER_BAYER_MAP21_SHIFT                                      8
#define DITHER_BAYER_MAP21_LENGTH                                     8
#define DITHER_BAYER_MAP22_SHIFT                                      16
#define DITHER_BAYER_MAP22_LENGTH                                     8
#define DITHER_BAYER_MAP23_SHIFT                                      24
#define DITHER_BAYER_MAP23_LENGTH                                     8

/* REGISTER veu_pipe_reg_29 */
#define DITHER_BAYER_MAP24_OFFSET                                     0x74
#define DITHER_BAYER_MAP24_SHIFT                                      0
#define DITHER_BAYER_MAP24_LENGTH                                     8
#define DITHER_BAYER_MAP25_SHIFT                                      8
#define DITHER_BAYER_MAP25_LENGTH                                     8
#define DITHER_BAYER_MAP26_SHIFT                                      16
#define DITHER_BAYER_MAP26_LENGTH                                     8
#define DITHER_BAYER_MAP27_SHIFT                                      24
#define DITHER_BAYER_MAP27_LENGTH                                     8

/* REGISTER veu_pipe_reg_30 */
#define DITHER_BAYER_MAP30_OFFSET                                     0x78
#define DITHER_BAYER_MAP30_SHIFT                                      0
#define DITHER_BAYER_MAP30_LENGTH                                     8
#define DITHER_BAYER_MAP31_SHIFT                                      8
#define DITHER_BAYER_MAP31_LENGTH                                     8
#define DITHER_BAYER_MAP32_SHIFT                                      16
#define DITHER_BAYER_MAP32_LENGTH                                     8
#define DITHER_BAYER_MAP33_SHIFT                                      24
#define DITHER_BAYER_MAP33_LENGTH                                     8

/* REGISTER veu_pipe_reg_31 */
#define DITHER_BAYER_MAP34_OFFSET                                     0x7c
#define DITHER_BAYER_MAP34_SHIFT                                      0
#define DITHER_BAYER_MAP34_LENGTH                                     8
#define DITHER_BAYER_MAP35_SHIFT                                      8
#define DITHER_BAYER_MAP35_LENGTH                                     8
#define DITHER_BAYER_MAP36_SHIFT                                      16
#define DITHER_BAYER_MAP36_LENGTH                                     8
#define DITHER_BAYER_MAP37_SHIFT                                      24
#define DITHER_BAYER_MAP37_LENGTH                                     8

/* REGISTER veu_pipe_reg_32 */
#define DITHER_BAYER_MAP40_OFFSET                                     0x80
#define DITHER_BAYER_MAP40_SHIFT                                      0
#define DITHER_BAYER_MAP40_LENGTH                                     8
#define DITHER_BAYER_MAP41_SHIFT                                      8
#define DITHER_BAYER_MAP41_LENGTH                                     8
#define DITHER_BAYER_MAP42_SHIFT                                      16
#define DITHER_BAYER_MAP42_LENGTH                                     8
#define DITHER_BAYER_MAP43_SHIFT                                      24
#define DITHER_BAYER_MAP43_LENGTH                                     8

/* REGISTER veu_pipe_reg_33 */
#define DITHER_BAYER_MAP44_OFFSET                                     0x84
#define DITHER_BAYER_MAP44_SHIFT                                      0
#define DITHER_BAYER_MAP44_LENGTH                                     8
#define DITHER_BAYER_MAP45_SHIFT                                      8
#define DITHER_BAYER_MAP45_LENGTH                                     8
#define DITHER_BAYER_MAP46_SHIFT                                      16
#define DITHER_BAYER_MAP46_LENGTH                                     8
#define DITHER_BAYER_MAP47_SHIFT                                      24
#define DITHER_BAYER_MAP47_LENGTH                                     8

/* REGISTER veu_pipe_reg_34 */
#define DITHER_BAYER_MAP50_OFFSET                                     0x88
#define DITHER_BAYER_MAP50_SHIFT                                      0
#define DITHER_BAYER_MAP50_LENGTH                                     8
#define DITHER_BAYER_MAP51_SHIFT                                      8
#define DITHER_BAYER_MAP51_LENGTH                                     8
#define DITHER_BAYER_MAP52_SHIFT                                      16
#define DITHER_BAYER_MAP52_LENGTH                                     8
#define DITHER_BAYER_MAP53_SHIFT                                      24
#define DITHER_BAYER_MAP53_LENGTH                                     8

/* REGISTER veu_pipe_reg_35 */
#define DITHER_BAYER_MAP54_OFFSET                                     0x8c
#define DITHER_BAYER_MAP54_SHIFT                                      0
#define DITHER_BAYER_MAP54_LENGTH                                     8
#define DITHER_BAYER_MAP55_SHIFT                                      8
#define DITHER_BAYER_MAP55_LENGTH                                     8
#define DITHER_BAYER_MAP56_SHIFT                                      16
#define DITHER_BAYER_MAP56_LENGTH                                     8
#define DITHER_BAYER_MAP57_SHIFT                                      24
#define DITHER_BAYER_MAP57_LENGTH                                     8

/* REGISTER veu_pipe_reg_36 */
#define DITHER_BAYER_MAP60_OFFSET                                     0x90
#define DITHER_BAYER_MAP60_SHIFT                                      0
#define DITHER_BAYER_MAP60_LENGTH                                     8
#define DITHER_BAYER_MAP61_SHIFT                                      8
#define DITHER_BAYER_MAP61_LENGTH                                     8
#define DITHER_BAYER_MAP62_SHIFT                                      16
#define DITHER_BAYER_MAP62_LENGTH                                     8
#define DITHER_BAYER_MAP63_SHIFT                                      24
#define DITHER_BAYER_MAP63_LENGTH                                     8

/* REGISTER veu_pipe_reg_37 */
#define DITHER_BAYER_MAP64_OFFSET                                     0x94
#define DITHER_BAYER_MAP64_SHIFT                                      0
#define DITHER_BAYER_MAP64_LENGTH                                     8
#define DITHER_BAYER_MAP65_SHIFT                                      8
#define DITHER_BAYER_MAP65_LENGTH                                     8
#define DITHER_BAYER_MAP66_SHIFT                                      16
#define DITHER_BAYER_MAP66_LENGTH                                     8
#define DITHER_BAYER_MAP67_SHIFT                                      24
#define DITHER_BAYER_MAP67_LENGTH                                     8

/* REGISTER veu_pipe_reg_38 */
#define DITHER_BAYER_MAP70_OFFSET                                     0x98
#define DITHER_BAYER_MAP70_SHIFT                                      0
#define DITHER_BAYER_MAP70_LENGTH                                     8
#define DITHER_BAYER_MAP71_SHIFT                                      8
#define DITHER_BAYER_MAP71_LENGTH                                     8
#define DITHER_BAYER_MAP72_SHIFT                                      16
#define DITHER_BAYER_MAP72_LENGTH                                     8
#define DITHER_BAYER_MAP73_SHIFT                                      24
#define DITHER_BAYER_MAP73_LENGTH                                     8

/* REGISTER veu_pipe_reg_39 */
#define DITHER_BAYER_MAP74_OFFSET                                     0x9c
#define DITHER_BAYER_MAP74_SHIFT                                      0
#define DITHER_BAYER_MAP74_LENGTH                                     8
#define DITHER_BAYER_MAP75_SHIFT                                      8
#define DITHER_BAYER_MAP75_LENGTH                                     8
#define DITHER_BAYER_MAP76_SHIFT                                      16
#define DITHER_BAYER_MAP76_LENGTH                                     8
#define DITHER_BAYER_MAP77_SHIFT                                      24
#define DITHER_BAYER_MAP77_LENGTH                                     8

/* REGISTER veu_pipe_reg_40 */
#define M_SCALE_WIDTH_OFFSET                                          0xa0
#define M_SCALE_WIDTH_SHIFT                                           0
#define M_SCALE_WIDTH_LENGTH                                          16
#define M_SCALE_HEIGHT_SHIFT                                          16
#define M_SCALE_HEIGHT_LENGTH                                         16

/* REGISTER veu_pipe_reg_41 */
#define CROP1_X_LEFT_OFFSET                                           0xa4
#define CROP1_X_LEFT_SHIFT                                            0
#define CROP1_X_LEFT_LENGTH                                           16
#define CROP1_X_RIGHT_SHIFT                                           16
#define CROP1_X_RIGHT_LENGTH                                          16

/* REGISTER veu_pipe_reg_42 */
#define CROP1_Y_TOP_OFFSET                                            0xa8
#define CROP1_Y_TOP_SHIFT                                             0
#define CROP1_Y_TOP_LENGTH                                            16
#define CROP1_Y_BOT_SHIFT                                             16
#define CROP1_Y_BOT_LENGTH                                            16

/* REGISTER veu_pipe_reg_43 */
#define FMT_CVT_A0_OFFSET                                             0xac
#define FMT_CVT_A0_SHIFT                                              0
#define FMT_CVT_A0_LENGTH                                             8
#define FMT_CVT_A1_SHIFT                                              8
#define FMT_CVT_A1_LENGTH                                             8
#define FMT_CVT_A2_SHIFT                                              16
#define FMT_CVT_A2_LENGTH                                             8
#define FMT_CVT_NARROW_MODE_SHIFT                                     24
#define FMT_CVT_NARROW_MODE_LENGTH                                    1

/* REGISTER veu_pipe_reg_44 */
#define FMT_CVT_RGB2YUV_MATRIX00_OFFSET                               0xb0
#define FMT_CVT_RGB2YUV_MATRIX00_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX00_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX01_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX01_LENGTH                               16

/* REGISTER veu_pipe_reg_45 */
#define FMT_CVT_RGB2YUV_MATRIX02_OFFSET                               0xb4
#define FMT_CVT_RGB2YUV_MATRIX02_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX02_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX03_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX03_LENGTH                               16

/* REGISTER veu_pipe_reg_46 */
#define FMT_CVT_RGB2YUV_MATRIX10_OFFSET                               0xb8
#define FMT_CVT_RGB2YUV_MATRIX10_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX10_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX11_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX11_LENGTH                               16

/* REGISTER veu_pipe_reg_47 */
#define FMT_CVT_RGB2YUV_MATRIX12_OFFSET                               0xbc
#define FMT_CVT_RGB2YUV_MATRIX12_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX12_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX13_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX13_LENGTH                               16

/* REGISTER veu_pipe_reg_48 */
#define FMT_CVT_RGB2YUV_MATRIX20_OFFSET                               0xc0
#define FMT_CVT_RGB2YUV_MATRIX20_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX20_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX21_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX21_LENGTH                               16

/* REGISTER veu_pipe_reg_49 */
#define FMT_CVT_RGB2YUV_MATRIX22_OFFSET                               0xc4
#define FMT_CVT_RGB2YUV_MATRIX22_SHIFT                                0
#define FMT_CVT_RGB2YUV_MATRIX22_LENGTH                               16
#define FMT_CVT_RGB2YUV_MATRIX23_SHIFT                                16
#define FMT_CVT_RGB2YUV_MATRIX23_LENGTH                               16

/* REGISTER veu_pipe_reg_50 */
#define YUV_CVT_HOR_COEF0_OFFSET                                      0xc8
#define YUV_CVT_HOR_COEF0_SHIFT                                       0
#define YUV_CVT_HOR_COEF0_LENGTH                                      10
#define YUV_CVT_HOR_COEF1_SHIFT                                       16
#define YUV_CVT_HOR_COEF1_LENGTH                                      10

/* REGISTER veu_pipe_reg_51 */
#define YUV_CVT_HOR_COEF2_OFFSET                                      0xcc
#define YUV_CVT_HOR_COEF2_SHIFT                                       0
#define YUV_CVT_HOR_COEF2_LENGTH                                      10

/* REGISTER veu_pipe_reg_52 */
#define YUV_CVT_VER_COEF_OFFSET                                       0xd0
#define YUV_CVT_VER_COEF_SHIFT                                        0
#define YUV_CVT_VER_COEF_LENGTH                                       10
#define DAT_CVT_OUT_FORMAT_SHIFT                                      10
#define DAT_CVT_OUT_FORMAT_LENGTH                                     5

/* REGISTER saturn_veu_pipe_reg_53 */
#define VEU_PIPE_REG_FORCE_UPDATE_EN_OFFSET                           0xd4
#define VEU_PIPE_REG_FORCE_UPDATE_EN_SHIFT                            0
#define VEU_PIPE_REG_FORCE_UPDATE_EN_LENGTH                           1
#define VEU_PIPE_REG_VSYNC_UPDATE_EN_SHIFT                            1
#define VEU_PIPE_REG_VSYNC_UPDATE_EN_LENGTH                           1
#define VEU_PIPE_REG_SHADOW_READ_EN_SHIFT                             2
#define VEU_PIPE_REG_SHADOW_READ_EN_LENGTH                            1

/* REGISTER saturn_veu_pipe_reg_54 */
#define VEU_PIPE_REG_FORCE_UPDATE_PULSE_OFFSET                        0xd8
#define VEU_PIPE_REG_FORCE_UPDATE_PULSE_SHIFT                         0
#define VEU_PIPE_REG_FORCE_UPDATE_PULSE_LENGTH                        1

/* REGISTER saturn_veu_pipe_reg_55 */
#define VEU_PIPE_REG_ICG_OVERRIDE_OFFSET                              0xdc
#define VEU_PIPE_REG_ICG_OVERRIDE_SHIFT                               0
#define VEU_PIPE_REG_ICG_OVERRIDE_LENGTH                              1

/* REGISTER saturn_veu_pipe_reg_56 */
#define VEU_PIPE_REG_TRIGGER2_OFFSET                                  0xe0
#define VEU_PIPE_REG_TRIGGER2_SHIFT                                   0
#define VEU_PIPE_REG_TRIGGER2_LENGTH                                  1

/* REGISTER veu_pipe_reg_57 */
#define NONPREALPHA_OFFSET1_OFFSET                                    0xe4
#define NONPREALPHA_OFFSET1_SHIFT                                     0
#define NONPREALPHA_OFFSET1_LENGTH                                    10
#define NONPREALPHA_OFFSET2_SHIFT                                     16
#define NONPREALPHA_OFFSET2_LENGTH                                    10

/* REGISTER veu_pipe_reg_58 */
#define NONPREALPHA_OFFSET3_OFFSET                                    0xe8
#define NONPREALPHA_OFFSET3_SHIFT                                     0
#define NONPREALPHA_OFFSET3_LENGTH                                    10

#endif // _XRING_VEU_HW_PIPE_REG_REG_H_
