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

#ifndef _DPU_HW_MIXER_REG_H_
#define _DPU_HW_MIXER_REG_H_

/* REGISTER cmps_reg_0 */
#define MODULE_ENABLE_OFFSET                                          0x00
#define MODULE_ENABLE_SHIFT                                           0
#define MODULE_ENABLE_LENGTH                                          1

/* REGISTER cmps_reg_1 */
#define DST_W_OFFSET                                                  0x04
#define DST_W_SHIFT                                                   0
#define DST_W_LENGTH                                                  16
#define DST_H_SHIFT                                                   16
#define DST_H_LENGTH                                                  16

/* REGISTER cmps_reg_2 */
#define BG_COLOR_R_OFFSET                                             0x08
#define BG_COLOR_R_SHIFT                                              0
#define BG_COLOR_R_LENGTH                                             10

/* REGISTER cmps_reg_3 */
#define BG_COLOR_G_OFFSET                                             0x0c
#define BG_COLOR_G_SHIFT                                              0
#define BG_COLOR_G_LENGTH                                             10

/* REGISTER cmps_reg_4 */
#define BG_COLOR_B_OFFSET                                             0x10
#define BG_COLOR_B_SHIFT                                              0
#define BG_COLOR_B_LENGTH                                             10

/* REGISTER cmps_reg_5 */
#define BG_COLOR_A_OFFSET                                             0x14
#define BG_COLOR_A_SHIFT                                              0
#define BG_COLOR_A_LENGTH                                             8

/* REGISTER cmps_reg_8 */
#define LAYER00_AREA_LEFT_OFFSET                                      0x20
#define LAYER00_AREA_LEFT_SHIFT                                       0
#define LAYER00_AREA_LEFT_LENGTH                                      16
#define LAYER00_AREA_RIGHT_SHIFT                                      16
#define LAYER00_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_9 */
#define LAYER00_AREA_TOP_OFFSET                                       0x24
#define LAYER00_AREA_TOP_SHIFT                                        0
#define LAYER00_AREA_TOP_LENGTH                                       16
#define LAYER00_AREA_BOTTOM_SHIFT                                     16
#define LAYER00_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_10 */
#define LAYER00_SOLID_COLOR_R_OFFSET                                  0x28
#define LAYER00_SOLID_COLOR_R_SHIFT                                   0
#define LAYER00_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_11 */
#define LAYER00_SOLID_COLOR_G_OFFSET                                  0x2c
#define LAYER00_SOLID_COLOR_G_SHIFT                                   0
#define LAYER00_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_12 */
#define LAYER00_SOLID_COLOR_B_OFFSET                                  0x30
#define LAYER00_SOLID_COLOR_B_SHIFT                                   0
#define LAYER00_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_13 */
#define LAYER00_SOLID_COLOR_A_OFFSET                                  0x34
#define LAYER00_SOLID_COLOR_A_SHIFT                                   0
#define LAYER00_SOLID_COLOR_A_LENGTH                                  8
#define LAYER00_BLEND_MODE_SHIFT                                      14
#define LAYER00_BLEND_MODE_LENGTH                                     2
#define LAYER00_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER00_SOLID_AREA_MODE_LENGTH                                1
#define LAYER00_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER00_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER00_COLORKEY_EN_SHIFT                                     18
#define LAYER00_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_14 */
#define LAYER00_EN_OFFSET                                             0x38
#define LAYER00_EN_SHIFT                                              0
#define LAYER00_EN_LENGTH                                             1
#define LAYER00_BLEND_SEL_SHIFT                                       1
#define LAYER00_BLEND_SEL_LENGTH                                      2
#define LAYER00_SOLID_EN_SHIFT                                        3
#define LAYER00_SOLID_EN_LENGTH                                       1
#define LAYER00_DMA_ID_SHIFT                                          4
#define LAYER00_DMA_ID_LENGTH                                         4
#define LAYER00_LAYER_ALPHA_SHIFT                                     8
#define LAYER00_LAYER_ALPHA_LENGTH                                    8
#define LAYER00_ALPHA_RATIO_SHIFT                                     16
#define LAYER00_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_15 */
#define LAYER01_AREA_LEFT_OFFSET                                      0x3c
#define LAYER01_AREA_LEFT_SHIFT                                       0
#define LAYER01_AREA_LEFT_LENGTH                                      16
#define LAYER01_AREA_RIGHT_SHIFT                                      16
#define LAYER01_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_16 */
#define LAYER01_AREA_TOP_OFFSET                                       0x40
#define LAYER01_AREA_TOP_SHIFT                                        0
#define LAYER01_AREA_TOP_LENGTH                                       16
#define LAYER01_AREA_BOTTOM_SHIFT                                     16
#define LAYER01_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_17 */
#define LAYER01_SOLID_COLOR_R_OFFSET                                  0x44
#define LAYER01_SOLID_COLOR_R_SHIFT                                   0
#define LAYER01_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_18 */
#define LAYER01_SOLID_COLOR_G_OFFSET                                  0x48
#define LAYER01_SOLID_COLOR_G_SHIFT                                   0
#define LAYER01_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_19 */
#define LAYER01_SOLID_COLOR_B_OFFSET                                  0x4c
#define LAYER01_SOLID_COLOR_B_SHIFT                                   0
#define LAYER01_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_20 */
#define LAYER01_SOLID_COLOR_A_OFFSET                                  0x50
#define LAYER01_SOLID_COLOR_A_SHIFT                                   0
#define LAYER01_SOLID_COLOR_A_LENGTH                                  8
#define LAYER01_BLEND_MODE_SHIFT                                      14
#define LAYER01_BLEND_MODE_LENGTH                                     2
#define LAYER01_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER01_SOLID_AREA_MODE_LENGTH                                1
#define LAYER01_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER01_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER01_COLORKEY_EN_SHIFT                                     18
#define LAYER01_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_21 */
#define LAYER01_EN_OFFSET                                             0x54
#define LAYER01_EN_SHIFT                                              0
#define LAYER01_EN_LENGTH                                             1
#define LAYER01_BLEND_SEL_SHIFT                                       1
#define LAYER01_BLEND_SEL_LENGTH                                      2
#define LAYER01_SOLID_EN_SHIFT                                        3
#define LAYER01_SOLID_EN_LENGTH                                       1
#define LAYER01_DMA_ID_SHIFT                                          4
#define LAYER01_DMA_ID_LENGTH                                         4
#define LAYER01_LAYER_ALPHA_SHIFT                                     8
#define LAYER01_LAYER_ALPHA_LENGTH                                    8
#define LAYER01_ALPHA_RATIO_SHIFT                                     16
#define LAYER01_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_22 */
#define LAYER02_AREA_LEFT_OFFSET                                      0x58
#define LAYER02_AREA_LEFT_SHIFT                                       0
#define LAYER02_AREA_LEFT_LENGTH                                      16
#define LAYER02_AREA_RIGHT_SHIFT                                      16
#define LAYER02_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_23 */
#define LAYER02_AREA_TOP_OFFSET                                       0x5c
#define LAYER02_AREA_TOP_SHIFT                                        0
#define LAYER02_AREA_TOP_LENGTH                                       16
#define LAYER02_AREA_BOTTOM_SHIFT                                     16
#define LAYER02_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_24 */
#define LAYER02_SOLID_COLOR_R_OFFSET                                  0x60
#define LAYER02_SOLID_COLOR_R_SHIFT                                   0
#define LAYER02_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_25 */
#define LAYER02_SOLID_COLOR_G_OFFSET                                  0x64
#define LAYER02_SOLID_COLOR_G_SHIFT                                   0
#define LAYER02_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_26 */
#define LAYER02_SOLID_COLOR_B_OFFSET                                  0x68
#define LAYER02_SOLID_COLOR_B_SHIFT                                   0
#define LAYER02_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_27 */
#define LAYER02_SOLID_COLOR_A_OFFSET                                  0x6c
#define LAYER02_SOLID_COLOR_A_SHIFT                                   0
#define LAYER02_SOLID_COLOR_A_LENGTH                                  8
#define LAYER02_BLEND_MODE_SHIFT                                      14
#define LAYER02_BLEND_MODE_LENGTH                                     2
#define LAYER02_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER02_SOLID_AREA_MODE_LENGTH                                1
#define LAYER02_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER02_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER02_COLORKEY_EN_SHIFT                                     18
#define LAYER02_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_28 */
#define LAYER02_EN_OFFSET                                             0x70
#define LAYER02_EN_SHIFT                                              0
#define LAYER02_EN_LENGTH                                             1
#define LAYER02_BLEND_SEL_SHIFT                                       1
#define LAYER02_BLEND_SEL_LENGTH                                      2
#define LAYER02_SOLID_EN_SHIFT                                        3
#define LAYER02_SOLID_EN_LENGTH                                       1
#define LAYER02_DMA_ID_SHIFT                                          4
#define LAYER02_DMA_ID_LENGTH                                         4
#define LAYER02_LAYER_ALPHA_SHIFT                                     8
#define LAYER02_LAYER_ALPHA_LENGTH                                    8
#define LAYER02_ALPHA_RATIO_SHIFT                                     16
#define LAYER02_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_29 */
#define LAYER03_AREA_LEFT_OFFSET                                      0x74
#define LAYER03_AREA_LEFT_SHIFT                                       0
#define LAYER03_AREA_LEFT_LENGTH                                      16
#define LAYER03_AREA_RIGHT_SHIFT                                      16
#define LAYER03_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_30 */
#define LAYER03_AREA_TOP_OFFSET                                       0x78
#define LAYER03_AREA_TOP_SHIFT                                        0
#define LAYER03_AREA_TOP_LENGTH                                       16
#define LAYER03_AREA_BOTTOM_SHIFT                                     16
#define LAYER03_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_31 */
#define LAYER03_SOLID_COLOR_R_OFFSET                                  0x7c
#define LAYER03_SOLID_COLOR_R_SHIFT                                   0
#define LAYER03_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_32 */
#define LAYER03_SOLID_COLOR_G_OFFSET                                  0x80
#define LAYER03_SOLID_COLOR_G_SHIFT                                   0
#define LAYER03_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_33 */
#define LAYER03_SOLID_COLOR_B_OFFSET                                  0x84
#define LAYER03_SOLID_COLOR_B_SHIFT                                   0
#define LAYER03_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_34 */
#define LAYER03_SOLID_COLOR_A_OFFSET                                  0x88
#define LAYER03_SOLID_COLOR_A_SHIFT                                   0
#define LAYER03_SOLID_COLOR_A_LENGTH                                  8
#define LAYER03_BLEND_MODE_SHIFT                                      14
#define LAYER03_BLEND_MODE_LENGTH                                     2
#define LAYER03_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER03_SOLID_AREA_MODE_LENGTH                                1
#define LAYER03_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER03_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER03_COLORKEY_EN_SHIFT                                     18
#define LAYER03_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_35 */
#define LAYER03_EN_OFFSET                                             0x8c
#define LAYER03_EN_SHIFT                                              0
#define LAYER03_EN_LENGTH                                             1
#define LAYER03_BLEND_SEL_SHIFT                                       1
#define LAYER03_BLEND_SEL_LENGTH                                      2
#define LAYER03_SOLID_EN_SHIFT                                        3
#define LAYER03_SOLID_EN_LENGTH                                       1
#define LAYER03_DMA_ID_SHIFT                                          4
#define LAYER03_DMA_ID_LENGTH                                         4
#define LAYER03_LAYER_ALPHA_SHIFT                                     8
#define LAYER03_LAYER_ALPHA_LENGTH                                    8
#define LAYER03_ALPHA_RATIO_SHIFT                                     16
#define LAYER03_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_36 */
#define LAYER04_AREA_LEFT_OFFSET                                      0x90
#define LAYER04_AREA_LEFT_SHIFT                                       0
#define LAYER04_AREA_LEFT_LENGTH                                      16
#define LAYER04_AREA_RIGHT_SHIFT                                      16
#define LAYER04_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_37 */
#define LAYER04_AREA_TOP_OFFSET                                       0x94
#define LAYER04_AREA_TOP_SHIFT                                        0
#define LAYER04_AREA_TOP_LENGTH                                       16
#define LAYER04_AREA_BOTTOM_SHIFT                                     16
#define LAYER04_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_38 */
#define LAYER04_SOLID_COLOR_R_OFFSET                                  0x98
#define LAYER04_SOLID_COLOR_R_SHIFT                                   0
#define LAYER04_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_39 */
#define LAYER04_SOLID_COLOR_G_OFFSET                                  0x9c
#define LAYER04_SOLID_COLOR_G_SHIFT                                   0
#define LAYER04_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_40 */
#define LAYER04_SOLID_COLOR_B_OFFSET                                  0xa0
#define LAYER04_SOLID_COLOR_B_SHIFT                                   0
#define LAYER04_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_41 */
#define LAYER04_SOLID_COLOR_A_OFFSET                                  0xa4
#define LAYER04_SOLID_COLOR_A_SHIFT                                   0
#define LAYER04_SOLID_COLOR_A_LENGTH                                  8
#define LAYER04_BLEND_MODE_SHIFT                                      14
#define LAYER04_BLEND_MODE_LENGTH                                     2
#define LAYER04_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER04_SOLID_AREA_MODE_LENGTH                                1
#define LAYER04_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER04_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER04_COLORKEY_EN_SHIFT                                     18
#define LAYER04_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_42 */
#define LAYER04_EN_OFFSET                                             0xa8
#define LAYER04_EN_SHIFT                                              0
#define LAYER04_EN_LENGTH                                             1
#define LAYER04_BLEND_SEL_SHIFT                                       1
#define LAYER04_BLEND_SEL_LENGTH                                      2
#define LAYER04_SOLID_EN_SHIFT                                        3
#define LAYER04_SOLID_EN_LENGTH                                       1
#define LAYER04_DMA_ID_SHIFT                                          4
#define LAYER04_DMA_ID_LENGTH                                         4
#define LAYER04_LAYER_ALPHA_SHIFT                                     8
#define LAYER04_LAYER_ALPHA_LENGTH                                    8
#define LAYER04_ALPHA_RATIO_SHIFT                                     16
#define LAYER04_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_43 */
#define LAYER05_AREA_LEFT_OFFSET                                      0xac
#define LAYER05_AREA_LEFT_SHIFT                                       0
#define LAYER05_AREA_LEFT_LENGTH                                      16
#define LAYER05_AREA_RIGHT_SHIFT                                      16
#define LAYER05_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_44 */
#define LAYER05_AREA_TOP_OFFSET                                       0xb0
#define LAYER05_AREA_TOP_SHIFT                                        0
#define LAYER05_AREA_TOP_LENGTH                                       16
#define LAYER05_AREA_BOTTOM_SHIFT                                     16
#define LAYER05_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_45 */
#define LAYER05_SOLID_COLOR_R_OFFSET                                  0xb4
#define LAYER05_SOLID_COLOR_R_SHIFT                                   0
#define LAYER05_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_46 */
#define LAYER05_SOLID_COLOR_G_OFFSET                                  0xb8
#define LAYER05_SOLID_COLOR_G_SHIFT                                   0
#define LAYER05_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_47 */
#define LAYER05_SOLID_COLOR_B_OFFSET                                  0xbc
#define LAYER05_SOLID_COLOR_B_SHIFT                                   0
#define LAYER05_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_48 */
#define LAYER05_SOLID_COLOR_A_OFFSET                                  0xc0
#define LAYER05_SOLID_COLOR_A_SHIFT                                   0
#define LAYER05_SOLID_COLOR_A_LENGTH                                  8
#define LAYER05_BLEND_MODE_SHIFT                                      14
#define LAYER05_BLEND_MODE_LENGTH                                     2
#define LAYER05_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER05_SOLID_AREA_MODE_LENGTH                                1
#define LAYER05_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER05_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER05_COLORKEY_EN_SHIFT                                     18
#define LAYER05_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_49 */
#define LAYER05_EN_OFFSET                                             0xc4
#define LAYER05_EN_SHIFT                                              0
#define LAYER05_EN_LENGTH                                             1
#define LAYER05_BLEND_SEL_SHIFT                                       1
#define LAYER05_BLEND_SEL_LENGTH                                      2
#define LAYER05_SOLID_EN_SHIFT                                        3
#define LAYER05_SOLID_EN_LENGTH                                       1
#define LAYER05_DMA_ID_SHIFT                                          4
#define LAYER05_DMA_ID_LENGTH                                         4
#define LAYER05_LAYER_ALPHA_SHIFT                                     8
#define LAYER05_LAYER_ALPHA_LENGTH                                    8
#define LAYER05_ALPHA_RATIO_SHIFT                                     16
#define LAYER05_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_50 */
#define LAYER06_AREA_LEFT_OFFSET                                      0xc8
#define LAYER06_AREA_LEFT_SHIFT                                       0
#define LAYER06_AREA_LEFT_LENGTH                                      16
#define LAYER06_AREA_RIGHT_SHIFT                                      16
#define LAYER06_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_51 */
#define LAYER06_AREA_TOP_OFFSET                                       0xcc
#define LAYER06_AREA_TOP_SHIFT                                        0
#define LAYER06_AREA_TOP_LENGTH                                       16
#define LAYER06_AREA_BOTTOM_SHIFT                                     16
#define LAYER06_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_52 */
#define LAYER06_SOLID_COLOR_R_OFFSET                                  0xd0
#define LAYER06_SOLID_COLOR_R_SHIFT                                   0
#define LAYER06_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_53 */
#define LAYER06_SOLID_COLOR_G_OFFSET                                  0xd4
#define LAYER06_SOLID_COLOR_G_SHIFT                                   0
#define LAYER06_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_54 */
#define LAYER06_SOLID_COLOR_B_OFFSET                                  0xd8
#define LAYER06_SOLID_COLOR_B_SHIFT                                   0
#define LAYER06_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_55 */
#define LAYER06_SOLID_COLOR_A_OFFSET                                  0xdc
#define LAYER06_SOLID_COLOR_A_SHIFT                                   0
#define LAYER06_SOLID_COLOR_A_LENGTH                                  8
#define LAYER06_BLEND_MODE_SHIFT                                      14
#define LAYER06_BLEND_MODE_LENGTH                                     2
#define LAYER06_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER06_SOLID_AREA_MODE_LENGTH                                1
#define LAYER06_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER06_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER06_COLORKEY_EN_SHIFT                                     18
#define LAYER06_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_56 */
#define LAYER06_EN_OFFSET                                             0xe0
#define LAYER06_EN_SHIFT                                              0
#define LAYER06_EN_LENGTH                                             1
#define LAYER06_BLEND_SEL_SHIFT                                       1
#define LAYER06_BLEND_SEL_LENGTH                                      2
#define LAYER06_SOLID_EN_SHIFT                                        3
#define LAYER06_SOLID_EN_LENGTH                                       1
#define LAYER06_DMA_ID_SHIFT                                          4
#define LAYER06_DMA_ID_LENGTH                                         4
#define LAYER06_LAYER_ALPHA_SHIFT                                     8
#define LAYER06_LAYER_ALPHA_LENGTH                                    8
#define LAYER06_ALPHA_RATIO_SHIFT                                     16
#define LAYER06_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_57 */
#define LAYER07_AREA_LEFT_OFFSET                                      0xe4
#define LAYER07_AREA_LEFT_SHIFT                                       0
#define LAYER07_AREA_LEFT_LENGTH                                      16
#define LAYER07_AREA_RIGHT_SHIFT                                      16
#define LAYER07_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_58 */
#define LAYER07_AREA_TOP_OFFSET                                       0xe8
#define LAYER07_AREA_TOP_SHIFT                                        0
#define LAYER07_AREA_TOP_LENGTH                                       16
#define LAYER07_AREA_BOTTOM_SHIFT                                     16
#define LAYER07_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_59 */
#define LAYER07_SOLID_COLOR_R_OFFSET                                  0xec
#define LAYER07_SOLID_COLOR_R_SHIFT                                   0
#define LAYER07_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_60 */
#define LAYER07_SOLID_COLOR_G_OFFSET                                  0xf0
#define LAYER07_SOLID_COLOR_G_SHIFT                                   0
#define LAYER07_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_61 */
#define LAYER07_SOLID_COLOR_B_OFFSET                                  0xf4
#define LAYER07_SOLID_COLOR_B_SHIFT                                   0
#define LAYER07_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_62 */
#define LAYER07_SOLID_COLOR_A_OFFSET                                  0xf8
#define LAYER07_SOLID_COLOR_A_SHIFT                                   0
#define LAYER07_SOLID_COLOR_A_LENGTH                                  8
#define LAYER07_BLEND_MODE_SHIFT                                      14
#define LAYER07_BLEND_MODE_LENGTH                                     2
#define LAYER07_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER07_SOLID_AREA_MODE_LENGTH                                1
#define LAYER07_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER07_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER07_COLORKEY_EN_SHIFT                                     18
#define LAYER07_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_63 */
#define LAYER07_EN_OFFSET                                             0xfc
#define LAYER07_EN_SHIFT                                              0
#define LAYER07_EN_LENGTH                                             1
#define LAYER07_BLEND_SEL_SHIFT                                       1
#define LAYER07_BLEND_SEL_LENGTH                                      2
#define LAYER07_SOLID_EN_SHIFT                                        3
#define LAYER07_SOLID_EN_LENGTH                                       1
#define LAYER07_DMA_ID_SHIFT                                          4
#define LAYER07_DMA_ID_LENGTH                                         4
#define LAYER07_LAYER_ALPHA_SHIFT                                     8
#define LAYER07_LAYER_ALPHA_LENGTH                                    8
#define LAYER07_ALPHA_RATIO_SHIFT                                     16
#define LAYER07_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_64 */
#define LAYER08_AREA_LEFT_OFFSET                                      0x100
#define LAYER08_AREA_LEFT_SHIFT                                       0
#define LAYER08_AREA_LEFT_LENGTH                                      16
#define LAYER08_AREA_RIGHT_SHIFT                                      16
#define LAYER08_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_65 */
#define LAYER08_AREA_TOP_OFFSET                                       0x104
#define LAYER08_AREA_TOP_SHIFT                                        0
#define LAYER08_AREA_TOP_LENGTH                                       16
#define LAYER08_AREA_BOTTOM_SHIFT                                     16
#define LAYER08_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_66 */
#define LAYER08_SOLID_COLOR_R_OFFSET                                  0x108
#define LAYER08_SOLID_COLOR_R_SHIFT                                   0
#define LAYER08_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_67 */
#define LAYER08_SOLID_COLOR_G_OFFSET                                  0x10c
#define LAYER08_SOLID_COLOR_G_SHIFT                                   0
#define LAYER08_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_68 */
#define LAYER08_SOLID_COLOR_B_OFFSET                                  0x110
#define LAYER08_SOLID_COLOR_B_SHIFT                                   0
#define LAYER08_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_69 */
#define LAYER08_SOLID_COLOR_A_OFFSET                                  0x114
#define LAYER08_SOLID_COLOR_A_SHIFT                                   0
#define LAYER08_SOLID_COLOR_A_LENGTH                                  8
#define LAYER08_BLEND_MODE_SHIFT                                      14
#define LAYER08_BLEND_MODE_LENGTH                                     2
#define LAYER08_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER08_SOLID_AREA_MODE_LENGTH                                1
#define LAYER08_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER08_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER08_COLORKEY_EN_SHIFT                                     18
#define LAYER08_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_70 */
#define LAYER08_EN_OFFSET                                             0x118
#define LAYER08_EN_SHIFT                                              0
#define LAYER08_EN_LENGTH                                             1
#define LAYER08_BLEND_SEL_SHIFT                                       1
#define LAYER08_BLEND_SEL_LENGTH                                      2
#define LAYER08_SOLID_EN_SHIFT                                        3
#define LAYER08_SOLID_EN_LENGTH                                       1
#define LAYER08_DMA_ID_SHIFT                                          4
#define LAYER08_DMA_ID_LENGTH                                         4
#define LAYER08_LAYER_ALPHA_SHIFT                                     8
#define LAYER08_LAYER_ALPHA_LENGTH                                    8
#define LAYER08_ALPHA_RATIO_SHIFT                                     16
#define LAYER08_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_71 */
#define LAYER09_AREA_LEFT_OFFSET                                      0x11c
#define LAYER09_AREA_LEFT_SHIFT                                       0
#define LAYER09_AREA_LEFT_LENGTH                                      16
#define LAYER09_AREA_RIGHT_SHIFT                                      16
#define LAYER09_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_72 */
#define LAYER09_AREA_TOP_OFFSET                                       0x120
#define LAYER09_AREA_TOP_SHIFT                                        0
#define LAYER09_AREA_TOP_LENGTH                                       16
#define LAYER09_AREA_BOTTOM_SHIFT                                     16
#define LAYER09_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_73 */
#define LAYER09_SOLID_COLOR_R_OFFSET                                  0x124
#define LAYER09_SOLID_COLOR_R_SHIFT                                   0
#define LAYER09_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_74 */
#define LAYER09_SOLID_COLOR_G_OFFSET                                  0x128
#define LAYER09_SOLID_COLOR_G_SHIFT                                   0
#define LAYER09_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_75 */
#define LAYER09_SOLID_COLOR_B_OFFSET                                  0x12c
#define LAYER09_SOLID_COLOR_B_SHIFT                                   0
#define LAYER09_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_76 */
#define LAYER09_SOLID_COLOR_A_OFFSET                                  0x130
#define LAYER09_SOLID_COLOR_A_SHIFT                                   0
#define LAYER09_SOLID_COLOR_A_LENGTH                                  8
#define LAYER09_BLEND_MODE_SHIFT                                      14
#define LAYER09_BLEND_MODE_LENGTH                                     2
#define LAYER09_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER09_SOLID_AREA_MODE_LENGTH                                1
#define LAYER09_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER09_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER09_COLORKEY_EN_SHIFT                                     18
#define LAYER09_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_77 */
#define LAYER09_EN_OFFSET                                             0x134
#define LAYER09_EN_SHIFT                                              0
#define LAYER09_EN_LENGTH                                             1
#define LAYER09_BLEND_SEL_SHIFT                                       1
#define LAYER09_BLEND_SEL_LENGTH                                      2
#define LAYER09_SOLID_EN_SHIFT                                        3
#define LAYER09_SOLID_EN_LENGTH                                       1
#define LAYER09_DMA_ID_SHIFT                                          4
#define LAYER09_DMA_ID_LENGTH                                         4
#define LAYER09_LAYER_ALPHA_SHIFT                                     8
#define LAYER09_LAYER_ALPHA_LENGTH                                    8
#define LAYER09_ALPHA_RATIO_SHIFT                                     16
#define LAYER09_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_78 */
#define LAYER10_AREA_LEFT_OFFSET                                      0x138
#define LAYER10_AREA_LEFT_SHIFT                                       0
#define LAYER10_AREA_LEFT_LENGTH                                      16
#define LAYER10_AREA_RIGHT_SHIFT                                      16
#define LAYER10_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_79 */
#define LAYER10_AREA_TOP_OFFSET                                       0x13c
#define LAYER10_AREA_TOP_SHIFT                                        0
#define LAYER10_AREA_TOP_LENGTH                                       16
#define LAYER10_AREA_BOTTOM_SHIFT                                     16
#define LAYER10_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_80 */
#define LAYER10_SOLID_COLOR_R_OFFSET                                  0x140
#define LAYER10_SOLID_COLOR_R_SHIFT                                   0
#define LAYER10_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_81 */
#define LAYER10_SOLID_COLOR_G_OFFSET                                  0x144
#define LAYER10_SOLID_COLOR_G_SHIFT                                   0
#define LAYER10_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_82 */
#define LAYER10_SOLID_COLOR_B_OFFSET                                  0x148
#define LAYER10_SOLID_COLOR_B_SHIFT                                   0
#define LAYER10_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_83 */
#define LAYER10_SOLID_COLOR_A_OFFSET                                  0x14c
#define LAYER10_SOLID_COLOR_A_SHIFT                                   0
#define LAYER10_SOLID_COLOR_A_LENGTH                                  8
#define LAYER10_BLEND_MODE_SHIFT                                      14
#define LAYER10_BLEND_MODE_LENGTH                                     2
#define LAYER10_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER10_SOLID_AREA_MODE_LENGTH                                1
#define LAYER10_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER10_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER10_COLORKEY_EN_SHIFT                                     18
#define LAYER10_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_84 */
#define LAYER10_EN_OFFSET                                             0x150
#define LAYER10_EN_SHIFT                                              0
#define LAYER10_EN_LENGTH                                             1
#define LAYER10_BLEND_SEL_SHIFT                                       1
#define LAYER10_BLEND_SEL_LENGTH                                      2
#define LAYER10_SOLID_EN_SHIFT                                        3
#define LAYER10_SOLID_EN_LENGTH                                       1
#define LAYER10_DMA_ID_SHIFT                                          4
#define LAYER10_DMA_ID_LENGTH                                         4
#define LAYER10_LAYER_ALPHA_SHIFT                                     8
#define LAYER10_LAYER_ALPHA_LENGTH                                    8
#define LAYER10_ALPHA_RATIO_SHIFT                                     16
#define LAYER10_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_85 */
#define LAYER11_AREA_LEFT_OFFSET                                      0x154
#define LAYER11_AREA_LEFT_SHIFT                                       0
#define LAYER11_AREA_LEFT_LENGTH                                      16
#define LAYER11_AREA_RIGHT_SHIFT                                      16
#define LAYER11_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_86 */
#define LAYER11_AREA_TOP_OFFSET                                       0x158
#define LAYER11_AREA_TOP_SHIFT                                        0
#define LAYER11_AREA_TOP_LENGTH                                       16
#define LAYER11_AREA_BOTTOM_SHIFT                                     16
#define LAYER11_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_87 */
#define LAYER11_SOLID_COLOR_R_OFFSET                                  0x15c
#define LAYER11_SOLID_COLOR_R_SHIFT                                   0
#define LAYER11_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_88 */
#define LAYER11_SOLID_COLOR_G_OFFSET                                  0x160
#define LAYER11_SOLID_COLOR_G_SHIFT                                   0
#define LAYER11_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_89 */
#define LAYER11_SOLID_COLOR_B_OFFSET                                  0x164
#define LAYER11_SOLID_COLOR_B_SHIFT                                   0
#define LAYER11_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_90 */
#define LAYER11_SOLID_COLOR_A_OFFSET                                  0x168
#define LAYER11_SOLID_COLOR_A_SHIFT                                   0
#define LAYER11_SOLID_COLOR_A_LENGTH                                  8
#define LAYER11_BLEND_MODE_SHIFT                                      14
#define LAYER11_BLEND_MODE_LENGTH                                     2
#define LAYER11_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER11_SOLID_AREA_MODE_LENGTH                                1
#define LAYER11_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER11_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER11_COLORKEY_EN_SHIFT                                     18
#define LAYER11_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_91 */
#define LAYER11_EN_OFFSET                                             0x16c
#define LAYER11_EN_SHIFT                                              0
#define LAYER11_EN_LENGTH                                             1
#define LAYER11_BLEND_SEL_SHIFT                                       1
#define LAYER11_BLEND_SEL_LENGTH                                      2
#define LAYER11_SOLID_EN_SHIFT                                        3
#define LAYER11_SOLID_EN_LENGTH                                       1
#define LAYER11_DMA_ID_SHIFT                                          4
#define LAYER11_DMA_ID_LENGTH                                         4
#define LAYER11_LAYER_ALPHA_SHIFT                                     8
#define LAYER11_LAYER_ALPHA_LENGTH                                    8
#define LAYER11_ALPHA_RATIO_SHIFT                                     16
#define LAYER11_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_92 */
#define LAYER12_AREA_LEFT_OFFSET                                      0x170
#define LAYER12_AREA_LEFT_SHIFT                                       0
#define LAYER12_AREA_LEFT_LENGTH                                      16
#define LAYER12_AREA_RIGHT_SHIFT                                      16
#define LAYER12_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_93 */
#define LAYER12_AREA_TOP_OFFSET                                       0x174
#define LAYER12_AREA_TOP_SHIFT                                        0
#define LAYER12_AREA_TOP_LENGTH                                       16
#define LAYER12_AREA_BOTTOM_SHIFT                                     16
#define LAYER12_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_94 */
#define LAYER12_SOLID_COLOR_R_OFFSET                                  0x178
#define LAYER12_SOLID_COLOR_R_SHIFT                                   0
#define LAYER12_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_95 */
#define LAYER12_SOLID_COLOR_G_OFFSET                                  0x17c
#define LAYER12_SOLID_COLOR_G_SHIFT                                   0
#define LAYER12_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_96 */
#define LAYER12_SOLID_COLOR_B_OFFSET                                  0x180
#define LAYER12_SOLID_COLOR_B_SHIFT                                   0
#define LAYER12_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_97 */
#define LAYER12_SOLID_COLOR_A_OFFSET                                  0x184
#define LAYER12_SOLID_COLOR_A_SHIFT                                   0
#define LAYER12_SOLID_COLOR_A_LENGTH                                  8
#define LAYER12_BLEND_MODE_SHIFT                                      14
#define LAYER12_BLEND_MODE_LENGTH                                     2
#define LAYER12_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER12_SOLID_AREA_MODE_LENGTH                                1
#define LAYER12_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER12_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER12_COLORKEY_EN_SHIFT                                     18
#define LAYER12_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_98 */
#define LAYER12_EN_OFFSET                                             0x188
#define LAYER12_EN_SHIFT                                              0
#define LAYER12_EN_LENGTH                                             1
#define LAYER12_BLEND_SEL_SHIFT                                       1
#define LAYER12_BLEND_SEL_LENGTH                                      2
#define LAYER12_SOLID_EN_SHIFT                                        3
#define LAYER12_SOLID_EN_LENGTH                                       1
#define LAYER12_DMA_ID_SHIFT                                          4
#define LAYER12_DMA_ID_LENGTH                                         4
#define LAYER12_LAYER_ALPHA_SHIFT                                     8
#define LAYER12_LAYER_ALPHA_LENGTH                                    8
#define LAYER12_ALPHA_RATIO_SHIFT                                     16
#define LAYER12_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_99 */
#define LAYER13_AREA_LEFT_OFFSET                                      0x18c
#define LAYER13_AREA_LEFT_SHIFT                                       0
#define LAYER13_AREA_LEFT_LENGTH                                      16
#define LAYER13_AREA_RIGHT_SHIFT                                      16
#define LAYER13_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_100 */
#define LAYER13_AREA_TOP_OFFSET                                       0x190
#define LAYER13_AREA_TOP_SHIFT                                        0
#define LAYER13_AREA_TOP_LENGTH                                       16
#define LAYER13_AREA_BOTTOM_SHIFT                                     16
#define LAYER13_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_101 */
#define LAYER13_SOLID_COLOR_R_OFFSET                                  0x194
#define LAYER13_SOLID_COLOR_R_SHIFT                                   0
#define LAYER13_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_102 */
#define LAYER13_SOLID_COLOR_G_OFFSET                                  0x198
#define LAYER13_SOLID_COLOR_G_SHIFT                                   0
#define LAYER13_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_103 */
#define LAYER13_SOLID_COLOR_B_OFFSET                                  0x19c
#define LAYER13_SOLID_COLOR_B_SHIFT                                   0
#define LAYER13_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_104 */
#define LAYER13_SOLID_COLOR_A_OFFSET                                  0x1a0
#define LAYER13_SOLID_COLOR_A_SHIFT                                   0
#define LAYER13_SOLID_COLOR_A_LENGTH                                  8
#define LAYER13_BLEND_MODE_SHIFT                                      14
#define LAYER13_BLEND_MODE_LENGTH                                     2
#define LAYER13_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER13_SOLID_AREA_MODE_LENGTH                                1
#define LAYER13_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER13_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER13_COLORKEY_EN_SHIFT                                     18
#define LAYER13_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_105 */
#define LAYER13_EN_OFFSET                                             0x1a4
#define LAYER13_EN_SHIFT                                              0
#define LAYER13_EN_LENGTH                                             1
#define LAYER13_BLEND_SEL_SHIFT                                       1
#define LAYER13_BLEND_SEL_LENGTH                                      2
#define LAYER13_SOLID_EN_SHIFT                                        3
#define LAYER13_SOLID_EN_LENGTH                                       1
#define LAYER13_DMA_ID_SHIFT                                          4
#define LAYER13_DMA_ID_LENGTH                                         4
#define LAYER13_LAYER_ALPHA_SHIFT                                     8
#define LAYER13_LAYER_ALPHA_LENGTH                                    8
#define LAYER13_ALPHA_RATIO_SHIFT                                     16
#define LAYER13_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_106 */
#define LAYER14_AREA_LEFT_OFFSET                                      0x1a8
#define LAYER14_AREA_LEFT_SHIFT                                       0
#define LAYER14_AREA_LEFT_LENGTH                                      16
#define LAYER14_AREA_RIGHT_SHIFT                                      16
#define LAYER14_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_107 */
#define LAYER14_AREA_TOP_OFFSET                                       0x1ac
#define LAYER14_AREA_TOP_SHIFT                                        0
#define LAYER14_AREA_TOP_LENGTH                                       16
#define LAYER14_AREA_BOTTOM_SHIFT                                     16
#define LAYER14_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_108 */
#define LAYER14_SOLID_COLOR_R_OFFSET                                  0x1b0
#define LAYER14_SOLID_COLOR_R_SHIFT                                   0
#define LAYER14_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_109 */
#define LAYER14_SOLID_COLOR_G_OFFSET                                  0x1b4
#define LAYER14_SOLID_COLOR_G_SHIFT                                   0
#define LAYER14_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_110 */
#define LAYER14_SOLID_COLOR_B_OFFSET                                  0x1b8
#define LAYER14_SOLID_COLOR_B_SHIFT                                   0
#define LAYER14_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_111 */
#define LAYER14_SOLID_COLOR_A_OFFSET                                  0x1bc
#define LAYER14_SOLID_COLOR_A_SHIFT                                   0
#define LAYER14_SOLID_COLOR_A_LENGTH                                  8
#define LAYER14_BLEND_MODE_SHIFT                                      14
#define LAYER14_BLEND_MODE_LENGTH                                     2
#define LAYER14_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER14_SOLID_AREA_MODE_LENGTH                                1
#define LAYER14_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER14_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER14_COLORKEY_EN_SHIFT                                     18
#define LAYER14_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_112 */
#define LAYER14_EN_OFFSET                                             0x1c0
#define LAYER14_EN_SHIFT                                              0
#define LAYER14_EN_LENGTH                                             1
#define LAYER14_BLEND_SEL_SHIFT                                       1
#define LAYER14_BLEND_SEL_LENGTH                                      2
#define LAYER14_SOLID_EN_SHIFT                                        3
#define LAYER14_SOLID_EN_LENGTH                                       1
#define LAYER14_DMA_ID_SHIFT                                          4
#define LAYER14_DMA_ID_LENGTH                                         4
#define LAYER14_LAYER_ALPHA_SHIFT                                     8
#define LAYER14_LAYER_ALPHA_LENGTH                                    8
#define LAYER14_ALPHA_RATIO_SHIFT                                     16
#define LAYER14_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_113 */
#define LAYER15_AREA_LEFT_OFFSET                                      0x1c4
#define LAYER15_AREA_LEFT_SHIFT                                       0
#define LAYER15_AREA_LEFT_LENGTH                                      16
#define LAYER15_AREA_RIGHT_SHIFT                                      16
#define LAYER15_AREA_RIGHT_LENGTH                                     16

/* REGISTER cmps_reg_114 */
#define LAYER15_AREA_TOP_OFFSET                                       0x1c8
#define LAYER15_AREA_TOP_SHIFT                                        0
#define LAYER15_AREA_TOP_LENGTH                                       16
#define LAYER15_AREA_BOTTOM_SHIFT                                     16
#define LAYER15_AREA_BOTTOM_LENGTH                                    16

/* REGISTER cmps_reg_115 */
#define LAYER15_SOLID_COLOR_R_OFFSET                                  0x1cc
#define LAYER15_SOLID_COLOR_R_SHIFT                                   0
#define LAYER15_SOLID_COLOR_R_LENGTH                                  10

/* REGISTER cmps_reg_116 */
#define LAYER15_SOLID_COLOR_G_OFFSET                                  0x1d0
#define LAYER15_SOLID_COLOR_G_SHIFT                                   0
#define LAYER15_SOLID_COLOR_G_LENGTH                                  10

/* REGISTER cmps_reg_117 */
#define LAYER15_SOLID_COLOR_B_OFFSET                                  0x1d4
#define LAYER15_SOLID_COLOR_B_SHIFT                                   0
#define LAYER15_SOLID_COLOR_B_LENGTH                                  10

/* REGISTER cmps_reg_118 */
#define LAYER15_SOLID_COLOR_A_OFFSET                                  0x1d8
#define LAYER15_SOLID_COLOR_A_SHIFT                                   0
#define LAYER15_SOLID_COLOR_A_LENGTH                                  8
#define LAYER15_BLEND_MODE_SHIFT                                      14
#define LAYER15_BLEND_MODE_LENGTH                                     2
#define LAYER15_SOLID_AREA_MODE_SHIFT                                 16
#define LAYER15_SOLID_AREA_MODE_LENGTH                                1
#define LAYER15_SPE_ALPHA_MODE_SHIFT                                  17
#define LAYER15_SPE_ALPHA_MODE_LENGTH                                 1
#define LAYER15_COLORKEY_EN_SHIFT                                     18
#define LAYER15_COLORKEY_EN_LENGTH                                    1

/* REGISTER cmps_reg_119 */
#define LAYER15_EN_OFFSET                                             0x1dc
#define LAYER15_EN_SHIFT                                              0
#define LAYER15_EN_LENGTH                                             1
#define LAYER15_BLEND_SEL_SHIFT                                       1
#define LAYER15_BLEND_SEL_LENGTH                                      2
#define LAYER15_SOLID_EN_SHIFT                                        3
#define LAYER15_SOLID_EN_LENGTH                                       1
#define LAYER15_DMA_ID_SHIFT                                          4
#define LAYER15_DMA_ID_LENGTH                                         4
#define LAYER15_LAYER_ALPHA_SHIFT                                     8
#define LAYER15_LAYER_ALPHA_LENGTH                                    8
#define LAYER15_ALPHA_RATIO_SHIFT                                     16
#define LAYER15_ALPHA_RATIO_LENGTH                                    8

/* REGISTER cmps_reg_121 */
#define LAYER_PROC_LOCATION_OFFSET                                    0x1e4

/* REGISTER cmps_reg_137 */
#define CMPS_PROC_LOCATION_OFFSET                                     0x224

/* REGISTER cmps_reg_138 */
#define MIXER_CFG_SE_OFFSET                                           0x228
#define MIXER_CFG_SE_SHIFT                                            0
#define MIXER_CFG_SE_LENGTH                                           1

/* REGISTER cmps_reg_139 */
#define MIXER_FORCE_UPDATE_EN_OFFSET                                  0x22c
#define MIXER_FORCE_UPDATE_EN_SHIFT                                   0
#define MIXER_FORCE_UPDATE_EN_LENGTH                                  1
#define MIXER_VSYNC_UPDATE_EN_SHIFT                                   1
#define MIXER_VSYNC_UPDATE_EN_LENGTH                                  1
#define MIXER_SHADOW_READ_EN_SHIFT                                    2
#define MIXER_SHADOW_READ_EN_LENGTH                                   1

/* REGISTER cmps_reg_140 */
#define MIXER_FORCE_UPDATE_PULSE_OFFSET                               0x230
#define MIXER_FORCE_UPDATE_PULSE_SHIFT                                0
#define MIXER_FORCE_UPDATE_PULSE_LENGTH                               1

/* REGISTER cmps_reg_141 */
#define MIXER_FORCE_UPDATE_EN_SE_OFFSET                               0x234
#define MIXER_FORCE_UPDATE_EN_SE_SHIFT                                0
#define MIXER_FORCE_UPDATE_EN_SE_LENGTH                               1
#define MIXER_VSYNC_UPDATE_EN_SE_SHIFT                                1
#define MIXER_VSYNC_UPDATE_EN_SE_LENGTH                               1
#define MIXER_SHADOW_READ_EN_SE_SHIFT                                 2
#define MIXER_SHADOW_READ_EN_SE_LENGTH                                1

/* REGISTER cmps_reg_142 */
#define MIXER_FORCE_UPDATE_PULSE_SE_OFFSET                            0x238
#define MIXER_FORCE_UPDATE_PULSE_SE_SHIFT                             0
#define MIXER_FORCE_UPDATE_PULSE_SE_LENGTH                            1

/* REGISTER cmps_reg_143 */
#define MIXER_ICG_OVERRIDE_OFFSET                                     0x23c
#define MIXER_ICG_OVERRIDE_SHIFT                                      0
#define MIXER_ICG_OVERRIDE_LENGTH                                     1

/* REGISTER cmps_reg_144 */
#define MIXER_TRIGGER_OFFSET                                          0x240
#define MIXER_TRIGGER_SHIFT                                           0
#define MIXER_TRIGGER_LENGTH                                          1

/* REGISTER cmps_reg_145 */
#define MIXER_TRIGGER2_OFFSET                                         0x244
#define MIXER_TRIGGER2_SHIFT                                          0
#define MIXER_TRIGGER2_LENGTH                                         1

#endif
