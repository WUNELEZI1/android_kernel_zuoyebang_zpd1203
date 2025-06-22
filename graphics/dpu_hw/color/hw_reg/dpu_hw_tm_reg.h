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

#ifndef _DPU_HW_TM_REG_H_
#define _DPU_HW_TM_REG_H_

/* REGISTER saturn_tm_reg_0 */
#define TM_M_NLAYER_PRO_EN_OFFSET                                     0x00
#define TM_M_NLAYER_PRO_EN_SHIFT                                      0
#define TM_M_NLAYER_PRO_EN_LENGTH                                     1
#define TM_M_NGAIN_TO_FULL_EN_SHIFT                                   1
#define TM_M_NGAIN_TO_FULL_EN_LENGTH                                  1
#define TM_M_NMATRIX_EN_SHIFT                                         2
#define TM_M_NMATRIX_EN_LENGTH                                        1
#define TM_M_NFRONT_TMOOTF_EN_SHIFT                                   3
#define TM_M_NFRONT_TMOOTF_EN_LENGTH                                  1
#define TM_M_NEND_TMOOTF_EN_SHIFT                                     4
#define TM_M_NEND_TMOOTF_EN_LENGTH                                    1
#define TM_M_NCOLOR_KEY_EN_SHIFT                                      5
#define TM_M_NCOLOR_KEY_EN_LENGTH                                     1
#define TM_M_NEOTF_EN_SHIFT                                           6
#define TM_M_NEOTF_EN_LENGTH                                          1
#define TM_M_NOETF_EN_SHIFT                                           7
#define TM_M_NOETF_EN_LENGTH                                          1

/* REGISTER saturn_tm_reg_1 */
#define TM_M_NEOTF_MODE_OFFSET                                        0x04
#define TM_M_NEOTF_MODE_SHIFT                                         0
#define TM_M_NEOTF_MODE_LENGTH                                        3
#define TM_M_NOETF_MODE_SHIFT                                         3
#define TM_M_NOETF_MODE_LENGTH                                        3
#define TM_M_NOETF_MAX_SHIFT                                          16
#define TM_M_NOETF_MAX_LENGTH                                         12

/* REGISTER saturn_tm_reg_2 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE0_OFFSET                         0x08
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE0_SHIFT                          0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE0_LENGTH                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE1_SHIFT                          16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE1_LENGTH                         16

/* REGISTER saturn_tm_reg_3 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE2_OFFSET                         0x0c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE2_SHIFT                          0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE2_LENGTH                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE3_SHIFT                          16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE3_LENGTH                         16

/* REGISTER saturn_tm_reg_4 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE4_OFFSET                         0x10
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE4_SHIFT                          0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE4_LENGTH                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE5_SHIFT                          16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE5_LENGTH                         16

/* REGISTER saturn_tm_reg_5 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE6_OFFSET                         0x14
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE6_SHIFT                          0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE6_LENGTH                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE7_SHIFT                          16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE7_LENGTH                         16

/* REGISTER saturn_tm_reg_6 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE8_OFFSET                         0x18
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE8_SHIFT                          0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE8_LENGTH                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE9_SHIFT                          16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE9_LENGTH                         16

/* REGISTER saturn_tm_reg_7 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE10_OFFSET                        0x1c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE10_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE10_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE11_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE11_LENGTH                        16

/* REGISTER saturn_tm_reg_8 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE12_OFFSET                        0x20
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE12_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE12_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE13_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE13_LENGTH                        16

/* REGISTER saturn_tm_reg_9 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE14_OFFSET                        0x24
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE14_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE14_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE15_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE15_LENGTH                        16

/* REGISTER saturn_tm_reg_10 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE16_OFFSET                        0x28
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE16_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE16_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE17_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE17_LENGTH                        16

/* REGISTER saturn_tm_reg_11 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE18_OFFSET                        0x2c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE18_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE18_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE19_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE19_LENGTH                        16

/* REGISTER saturn_tm_reg_12 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE20_OFFSET                        0x30
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE20_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE20_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE21_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE21_LENGTH                        16

/* REGISTER saturn_tm_reg_13 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE22_OFFSET                        0x34
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE22_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE22_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE23_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE23_LENGTH                        16

/* REGISTER saturn_tm_reg_14 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE24_OFFSET                        0x38
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE24_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE24_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE25_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE25_LENGTH                        16

/* REGISTER saturn_tm_reg_15 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE26_OFFSET                        0x3c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE26_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE26_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE27_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE27_LENGTH                        16

/* REGISTER saturn_tm_reg_16 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE28_OFFSET                        0x40
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE28_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE28_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE29_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE29_LENGTH                        16

/* REGISTER saturn_tm_reg_17 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE30_OFFSET                        0x44
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE30_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE30_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE31_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE31_LENGTH                        16

/* REGISTER saturn_tm_reg_18 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE32_OFFSET                        0x48
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE32_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE32_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE33_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE33_LENGTH                        16

/* REGISTER saturn_tm_reg_19 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE34_OFFSET                        0x4c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE34_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE34_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE35_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE35_LENGTH                        16

/* REGISTER saturn_tm_reg_20 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE36_OFFSET                        0x50
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE36_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE36_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE37_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE37_LENGTH                        16

/* REGISTER saturn_tm_reg_21 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE38_OFFSET                        0x54
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE38_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE38_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE39_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE39_LENGTH                        16

/* REGISTER saturn_tm_reg_22 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE40_OFFSET                        0x58
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE40_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE40_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE41_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE41_LENGTH                        16

/* REGISTER saturn_tm_reg_23 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE42_OFFSET                        0x5c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE42_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE42_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE43_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE43_LENGTH                        16

/* REGISTER saturn_tm_reg_24 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE44_OFFSET                        0x60
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE44_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE44_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE45_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE45_LENGTH                        16

/* REGISTER saturn_tm_reg_25 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE46_OFFSET                        0x64
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE46_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE46_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE47_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE47_LENGTH                        16

/* REGISTER saturn_tm_reg_26 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE48_OFFSET                        0x68
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE48_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE48_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE49_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE49_LENGTH                        16

/* REGISTER saturn_tm_reg_27 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE50_OFFSET                        0x6c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE50_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE50_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE51_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE51_LENGTH                        16

/* REGISTER saturn_tm_reg_28 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE52_OFFSET                        0x70
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE52_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE52_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE53_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE53_LENGTH                        16

/* REGISTER saturn_tm_reg_29 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE54_OFFSET                        0x74
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE54_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE54_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE55_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE55_LENGTH                        16

/* REGISTER saturn_tm_reg_30 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE56_OFFSET                        0x78
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE56_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE56_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE57_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE57_LENGTH                        16

/* REGISTER saturn_tm_reg_31 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE58_OFFSET                        0x7c
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE58_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE58_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE59_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE59_LENGTH                        16

/* REGISTER saturn_tm_reg_32 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE60_OFFSET                        0x80
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE60_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE60_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE61_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE61_LENGTH                        16

/* REGISTER saturn_tm_reg_33 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE62_OFFSET                        0x84
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE62_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE62_LENGTH                        16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE63_SHIFT                         16
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE63_LENGTH                        16

/* REGISTER saturn_tm_reg_34 */
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE64_OFFSET                        0x88
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE64_SHIFT                         0
#define TM_M_PFRONT_TMOOTF_GAIN_TABLE64_LENGTH                        16
#define TM_M_NFRONT_TMOOTF_SHIFT_BITS_SHIFT                           16
#define TM_M_NFRONT_TMOOTF_SHIFT_BITS_LENGTH                          5
#define TM_M_NFRONT_TMOOTF_RGB_MODE_SHIFT                             21
#define TM_M_NFRONT_TMOOTF_RGB_MODE_LENGTH                            2

/* REGISTER saturn_tm_reg_35 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE0_OFFSET                           0x8c
#define TM_M_PEND_TMOOTF_GAIN_TABLE0_SHIFT                            0
#define TM_M_PEND_TMOOTF_GAIN_TABLE0_LENGTH                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE1_SHIFT                            16
#define TM_M_PEND_TMOOTF_GAIN_TABLE1_LENGTH                           16

/* REGISTER saturn_tm_reg_36 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE2_OFFSET                           0x90
#define TM_M_PEND_TMOOTF_GAIN_TABLE2_SHIFT                            0
#define TM_M_PEND_TMOOTF_GAIN_TABLE2_LENGTH                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE3_SHIFT                            16
#define TM_M_PEND_TMOOTF_GAIN_TABLE3_LENGTH                           16

/* REGISTER saturn_tm_reg_37 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE4_OFFSET                           0x94
#define TM_M_PEND_TMOOTF_GAIN_TABLE4_SHIFT                            0
#define TM_M_PEND_TMOOTF_GAIN_TABLE4_LENGTH                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE5_SHIFT                            16
#define TM_M_PEND_TMOOTF_GAIN_TABLE5_LENGTH                           16

/* REGISTER saturn_tm_reg_38 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE6_OFFSET                           0x98
#define TM_M_PEND_TMOOTF_GAIN_TABLE6_SHIFT                            0
#define TM_M_PEND_TMOOTF_GAIN_TABLE6_LENGTH                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE7_SHIFT                            16
#define TM_M_PEND_TMOOTF_GAIN_TABLE7_LENGTH                           16

/* REGISTER saturn_tm_reg_39 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE8_OFFSET                           0x9c
#define TM_M_PEND_TMOOTF_GAIN_TABLE8_SHIFT                            0
#define TM_M_PEND_TMOOTF_GAIN_TABLE8_LENGTH                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE9_SHIFT                            16
#define TM_M_PEND_TMOOTF_GAIN_TABLE9_LENGTH                           16

/* REGISTER saturn_tm_reg_40 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE10_OFFSET                          0xa0
#define TM_M_PEND_TMOOTF_GAIN_TABLE10_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE10_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE11_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE11_LENGTH                          16

/* REGISTER saturn_tm_reg_41 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE12_OFFSET                          0xa4
#define TM_M_PEND_TMOOTF_GAIN_TABLE12_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE12_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE13_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE13_LENGTH                          16

/* REGISTER saturn_tm_reg_42 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE14_OFFSET                          0xa8
#define TM_M_PEND_TMOOTF_GAIN_TABLE14_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE14_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE15_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE15_LENGTH                          16

/* REGISTER saturn_tm_reg_43 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE16_OFFSET                          0xac
#define TM_M_PEND_TMOOTF_GAIN_TABLE16_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE16_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE17_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE17_LENGTH                          16

/* REGISTER saturn_tm_reg_44 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE18_OFFSET                          0xb0
#define TM_M_PEND_TMOOTF_GAIN_TABLE18_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE18_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE19_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE19_LENGTH                          16

/* REGISTER saturn_tm_reg_45 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE20_OFFSET                          0xb4
#define TM_M_PEND_TMOOTF_GAIN_TABLE20_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE20_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE21_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE21_LENGTH                          16

/* REGISTER saturn_tm_reg_46 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE22_OFFSET                          0xb8
#define TM_M_PEND_TMOOTF_GAIN_TABLE22_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE22_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE23_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE23_LENGTH                          16

/* REGISTER saturn_tm_reg_47 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE24_OFFSET                          0xbc
#define TM_M_PEND_TMOOTF_GAIN_TABLE24_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE24_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE25_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE25_LENGTH                          16

/* REGISTER saturn_tm_reg_48 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE26_OFFSET                          0xc0
#define TM_M_PEND_TMOOTF_GAIN_TABLE26_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE26_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE27_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE27_LENGTH                          16

/* REGISTER saturn_tm_reg_49 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE28_OFFSET                          0xc4
#define TM_M_PEND_TMOOTF_GAIN_TABLE28_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE28_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE29_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE29_LENGTH                          16

/* REGISTER saturn_tm_reg_50 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE30_OFFSET                          0xc8
#define TM_M_PEND_TMOOTF_GAIN_TABLE30_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE30_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE31_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE31_LENGTH                          16

/* REGISTER saturn_tm_reg_51 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE32_OFFSET                          0xcc
#define TM_M_PEND_TMOOTF_GAIN_TABLE32_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE32_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE33_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE33_LENGTH                          16

/* REGISTER saturn_tm_reg_52 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE34_OFFSET                          0xd0
#define TM_M_PEND_TMOOTF_GAIN_TABLE34_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE34_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE35_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE35_LENGTH                          16

/* REGISTER saturn_tm_reg_53 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE36_OFFSET                          0xd4
#define TM_M_PEND_TMOOTF_GAIN_TABLE36_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE36_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE37_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE37_LENGTH                          16

/* REGISTER saturn_tm_reg_54 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE38_OFFSET                          0xd8
#define TM_M_PEND_TMOOTF_GAIN_TABLE38_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE38_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE39_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE39_LENGTH                          16

/* REGISTER saturn_tm_reg_55 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE40_OFFSET                          0xdc
#define TM_M_PEND_TMOOTF_GAIN_TABLE40_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE40_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE41_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE41_LENGTH                          16

/* REGISTER saturn_tm_reg_56 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE42_OFFSET                          0xe0
#define TM_M_PEND_TMOOTF_GAIN_TABLE42_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE42_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE43_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE43_LENGTH                          16

/* REGISTER saturn_tm_reg_57 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE44_OFFSET                          0xe4
#define TM_M_PEND_TMOOTF_GAIN_TABLE44_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE44_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE45_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE45_LENGTH                          16

/* REGISTER saturn_tm_reg_58 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE46_OFFSET                          0xe8
#define TM_M_PEND_TMOOTF_GAIN_TABLE46_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE46_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE47_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE47_LENGTH                          16

/* REGISTER saturn_tm_reg_59 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE48_OFFSET                          0xec
#define TM_M_PEND_TMOOTF_GAIN_TABLE48_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE48_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE49_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE49_LENGTH                          16

/* REGISTER saturn_tm_reg_60 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE50_OFFSET                          0xf0
#define TM_M_PEND_TMOOTF_GAIN_TABLE50_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE50_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE51_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE51_LENGTH                          16

/* REGISTER saturn_tm_reg_61 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE52_OFFSET                          0xf4
#define TM_M_PEND_TMOOTF_GAIN_TABLE52_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE52_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE53_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE53_LENGTH                          16

/* REGISTER saturn_tm_reg_62 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE54_OFFSET                          0xf8
#define TM_M_PEND_TMOOTF_GAIN_TABLE54_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE54_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE55_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE55_LENGTH                          16

/* REGISTER saturn_tm_reg_63 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE56_OFFSET                          0xfc
#define TM_M_PEND_TMOOTF_GAIN_TABLE56_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE56_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE57_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE57_LENGTH                          16

/* REGISTER saturn_tm_reg_64 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE58_OFFSET                          0x100
#define TM_M_PEND_TMOOTF_GAIN_TABLE58_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE58_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE59_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE59_LENGTH                          16

/* REGISTER saturn_tm_reg_65 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE60_OFFSET                          0x104
#define TM_M_PEND_TMOOTF_GAIN_TABLE60_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE60_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE61_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE61_LENGTH                          16

/* REGISTER saturn_tm_reg_66 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE62_OFFSET                          0x108
#define TM_M_PEND_TMOOTF_GAIN_TABLE62_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE62_LENGTH                          16
#define TM_M_PEND_TMOOTF_GAIN_TABLE63_SHIFT                           16
#define TM_M_PEND_TMOOTF_GAIN_TABLE63_LENGTH                          16

/* REGISTER saturn_tm_reg_67 */
#define TM_M_PEND_TMOOTF_GAIN_TABLE64_OFFSET                          0x10c
#define TM_M_PEND_TMOOTF_GAIN_TABLE64_SHIFT                           0
#define TM_M_PEND_TMOOTF_GAIN_TABLE64_LENGTH                          16
#define M_PEND_TMOOTF_GAIN_TABLE65_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE65_LENGTH                             16

/* REGISTER saturn_tm_reg_68 */
#define M_PEND_TMOOTF_GAIN_TABLE66_OFFSET                             0x110
#define M_PEND_TMOOTF_GAIN_TABLE66_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE66_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE67_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE67_LENGTH                             16

/* REGISTER saturn_tm_reg_69 */
#define M_PEND_TMOOTF_GAIN_TABLE68_OFFSET                             0x114
#define M_PEND_TMOOTF_GAIN_TABLE68_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE68_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE69_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE69_LENGTH                             16

/* REGISTER saturn_tm_reg_70 */
#define M_PEND_TMOOTF_GAIN_TABLE70_OFFSET                             0x118
#define M_PEND_TMOOTF_GAIN_TABLE70_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE70_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE71_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE71_LENGTH                             16

/* REGISTER saturn_tm_reg_71 */
#define M_PEND_TMOOTF_GAIN_TABLE72_OFFSET                             0x11c
#define M_PEND_TMOOTF_GAIN_TABLE72_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE72_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE73_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE73_LENGTH                             16

/* REGISTER saturn_tm_reg_72 */
#define M_PEND_TMOOTF_GAIN_TABLE74_OFFSET                             0x120
#define M_PEND_TMOOTF_GAIN_TABLE74_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE74_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE75_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE75_LENGTH                             16

/* REGISTER saturn_tm_reg_73 */
#define M_PEND_TMOOTF_GAIN_TABLE76_OFFSET                             0x124
#define M_PEND_TMOOTF_GAIN_TABLE76_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE76_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE77_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE77_LENGTH                             16

/* REGISTER saturn_tm_reg_74 */
#define M_PEND_TMOOTF_GAIN_TABLE78_OFFSET                             0x128
#define M_PEND_TMOOTF_GAIN_TABLE78_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE78_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE79_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE79_LENGTH                             16

/* REGISTER saturn_tm_reg_75 */
#define M_PEND_TMOOTF_GAIN_TABLE80_OFFSET                             0x12c
#define M_PEND_TMOOTF_GAIN_TABLE80_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE80_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE81_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE81_LENGTH                             16

/* REGISTER saturn_tm_reg_76 */
#define M_PEND_TMOOTF_GAIN_TABLE82_OFFSET                             0x130
#define M_PEND_TMOOTF_GAIN_TABLE82_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE82_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE83_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE83_LENGTH                             16

/* REGISTER saturn_tm_reg_77 */
#define M_PEND_TMOOTF_GAIN_TABLE84_OFFSET                             0x134
#define M_PEND_TMOOTF_GAIN_TABLE84_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE84_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE85_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE85_LENGTH                             16

/* REGISTER saturn_tm_reg_78 */
#define M_PEND_TMOOTF_GAIN_TABLE86_OFFSET                             0x138
#define M_PEND_TMOOTF_GAIN_TABLE86_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE86_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE87_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE87_LENGTH                             16

/* REGISTER saturn_tm_reg_79 */
#define M_PEND_TMOOTF_GAIN_TABLE88_OFFSET                             0x13c
#define M_PEND_TMOOTF_GAIN_TABLE88_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE88_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE89_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE89_LENGTH                             16

/* REGISTER saturn_tm_reg_80 */
#define M_PEND_TMOOTF_GAIN_TABLE90_OFFSET                             0x140
#define M_PEND_TMOOTF_GAIN_TABLE90_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE90_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE91_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE91_LENGTH                             16

/* REGISTER saturn_tm_reg_81 */
#define M_PEND_TMOOTF_GAIN_TABLE92_OFFSET                             0x144
#define M_PEND_TMOOTF_GAIN_TABLE92_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE92_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE93_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE93_LENGTH                             16

/* REGISTER saturn_tm_reg_82 */
#define M_PEND_TMOOTF_GAIN_TABLE94_OFFSET                             0x148
#define M_PEND_TMOOTF_GAIN_TABLE94_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE94_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE95_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE95_LENGTH                             16

/* REGISTER saturn_tm_reg_83 */
#define M_PEND_TMOOTF_GAIN_TABLE96_OFFSET                             0x14c
#define M_PEND_TMOOTF_GAIN_TABLE96_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE96_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE97_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE97_LENGTH                             16

/* REGISTER saturn_tm_reg_84 */
#define M_PEND_TMOOTF_GAIN_TABLE98_OFFSET                             0x150
#define M_PEND_TMOOTF_GAIN_TABLE98_SHIFT                              0
#define M_PEND_TMOOTF_GAIN_TABLE98_LENGTH                             16
#define M_PEND_TMOOTF_GAIN_TABLE99_SHIFT                              16
#define M_PEND_TMOOTF_GAIN_TABLE99_LENGTH                             16

/* REGISTER saturn_tm_reg_85 */
#define M_PEND_TMOOTF_GAIN_TABLE100_OFFSET                            0x154
#define M_PEND_TMOOTF_GAIN_TABLE100_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE100_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE101_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE101_LENGTH                            16

/* REGISTER saturn_tm_reg_86 */
#define M_PEND_TMOOTF_GAIN_TABLE102_OFFSET                            0x158
#define M_PEND_TMOOTF_GAIN_TABLE102_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE102_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE103_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE103_LENGTH                            16

/* REGISTER saturn_tm_reg_87 */
#define M_PEND_TMOOTF_GAIN_TABLE104_OFFSET                            0x15c
#define M_PEND_TMOOTF_GAIN_TABLE104_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE104_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE105_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE105_LENGTH                            16

/* REGISTER saturn_tm_reg_88 */
#define M_PEND_TMOOTF_GAIN_TABLE106_OFFSET                            0x160
#define M_PEND_TMOOTF_GAIN_TABLE106_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE106_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE107_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE107_LENGTH                            16

/* REGISTER saturn_tm_reg_89 */
#define M_PEND_TMOOTF_GAIN_TABLE108_OFFSET                            0x164
#define M_PEND_TMOOTF_GAIN_TABLE108_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE108_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE109_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE109_LENGTH                            16

/* REGISTER saturn_tm_reg_90 */
#define M_PEND_TMOOTF_GAIN_TABLE110_OFFSET                            0x168
#define M_PEND_TMOOTF_GAIN_TABLE110_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE110_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE111_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE111_LENGTH                            16

/* REGISTER saturn_tm_reg_91 */
#define M_PEND_TMOOTF_GAIN_TABLE112_OFFSET                            0x16c
#define M_PEND_TMOOTF_GAIN_TABLE112_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE112_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE113_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE113_LENGTH                            16

/* REGISTER saturn_tm_reg_92 */
#define M_PEND_TMOOTF_GAIN_TABLE114_OFFSET                            0x170
#define M_PEND_TMOOTF_GAIN_TABLE114_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE114_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE115_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE115_LENGTH                            16

/* REGISTER saturn_tm_reg_93 */
#define M_PEND_TMOOTF_GAIN_TABLE116_OFFSET                            0x174
#define M_PEND_TMOOTF_GAIN_TABLE116_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE116_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE117_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE117_LENGTH                            16

/* REGISTER saturn_tm_reg_94 */
#define M_PEND_TMOOTF_GAIN_TABLE118_OFFSET                            0x178
#define M_PEND_TMOOTF_GAIN_TABLE118_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE118_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE119_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE119_LENGTH                            16

/* REGISTER saturn_tm_reg_95 */
#define M_PEND_TMOOTF_GAIN_TABLE120_OFFSET                            0x17c
#define M_PEND_TMOOTF_GAIN_TABLE120_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE120_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE121_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE121_LENGTH                            16

/* REGISTER saturn_tm_reg_96 */
#define M_PEND_TMOOTF_GAIN_TABLE122_OFFSET                            0x180
#define M_PEND_TMOOTF_GAIN_TABLE122_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE122_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE123_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE123_LENGTH                            16

/* REGISTER saturn_tm_reg_97 */
#define M_PEND_TMOOTF_GAIN_TABLE124_OFFSET                            0x184
#define M_PEND_TMOOTF_GAIN_TABLE124_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE124_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE125_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE125_LENGTH                            16

/* REGISTER saturn_tm_reg_98 */
#define M_PEND_TMOOTF_GAIN_TABLE126_OFFSET                            0x188
#define M_PEND_TMOOTF_GAIN_TABLE126_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE126_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE127_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE127_LENGTH                            16

/* REGISTER saturn_tm_reg_99 */
#define M_PEND_TMOOTF_GAIN_TABLE128_OFFSET                            0x18c
#define M_PEND_TMOOTF_GAIN_TABLE128_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE128_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE129_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE129_LENGTH                            16

/* REGISTER saturn_tm_reg_100 */
#define M_PEND_TMOOTF_GAIN_TABLE130_OFFSET                            0x190
#define M_PEND_TMOOTF_GAIN_TABLE130_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE130_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE131_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE131_LENGTH                            16

/* REGISTER saturn_tm_reg_101 */
#define M_PEND_TMOOTF_GAIN_TABLE132_OFFSET                            0x194
#define M_PEND_TMOOTF_GAIN_TABLE132_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE132_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE133_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE133_LENGTH                            16

/* REGISTER saturn_tm_reg_102 */
#define M_PEND_TMOOTF_GAIN_TABLE134_OFFSET                            0x198
#define M_PEND_TMOOTF_GAIN_TABLE134_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE134_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE135_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE135_LENGTH                            16

/* REGISTER saturn_tm_reg_103 */
#define M_PEND_TMOOTF_GAIN_TABLE136_OFFSET                            0x19c
#define M_PEND_TMOOTF_GAIN_TABLE136_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE136_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE137_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE137_LENGTH                            16

/* REGISTER saturn_tm_reg_104 */
#define M_PEND_TMOOTF_GAIN_TABLE138_OFFSET                            0x1a0
#define M_PEND_TMOOTF_GAIN_TABLE138_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE138_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE139_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE139_LENGTH                            16

/* REGISTER saturn_tm_reg_105 */
#define M_PEND_TMOOTF_GAIN_TABLE140_OFFSET                            0x1a4
#define M_PEND_TMOOTF_GAIN_TABLE140_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE140_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE141_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE141_LENGTH                            16

/* REGISTER saturn_tm_reg_106 */
#define M_PEND_TMOOTF_GAIN_TABLE142_OFFSET                            0x1a8
#define M_PEND_TMOOTF_GAIN_TABLE142_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE142_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE143_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE143_LENGTH                            16

/* REGISTER saturn_tm_reg_107 */
#define M_PEND_TMOOTF_GAIN_TABLE144_OFFSET                            0x1ac
#define M_PEND_TMOOTF_GAIN_TABLE144_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE144_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE145_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE145_LENGTH                            16

/* REGISTER saturn_tm_reg_108 */
#define M_PEND_TMOOTF_GAIN_TABLE146_OFFSET                            0x1b0
#define M_PEND_TMOOTF_GAIN_TABLE146_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE146_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE147_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE147_LENGTH                            16

/* REGISTER saturn_tm_reg_109 */
#define M_PEND_TMOOTF_GAIN_TABLE148_OFFSET                            0x1b4
#define M_PEND_TMOOTF_GAIN_TABLE148_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE148_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE149_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE149_LENGTH                            16

/* REGISTER saturn_tm_reg_110 */
#define M_PEND_TMOOTF_GAIN_TABLE150_OFFSET                            0x1b8
#define M_PEND_TMOOTF_GAIN_TABLE150_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE150_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE151_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE151_LENGTH                            16

/* REGISTER saturn_tm_reg_111 */
#define M_PEND_TMOOTF_GAIN_TABLE152_OFFSET                            0x1bc
#define M_PEND_TMOOTF_GAIN_TABLE152_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE152_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE153_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE153_LENGTH                            16

/* REGISTER saturn_tm_reg_112 */
#define M_PEND_TMOOTF_GAIN_TABLE154_OFFSET                            0x1c0
#define M_PEND_TMOOTF_GAIN_TABLE154_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE154_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE155_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE155_LENGTH                            16

/* REGISTER saturn_tm_reg_113 */
#define M_PEND_TMOOTF_GAIN_TABLE156_OFFSET                            0x1c4
#define M_PEND_TMOOTF_GAIN_TABLE156_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE156_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE157_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE157_LENGTH                            16

/* REGISTER saturn_tm_reg_114 */
#define M_PEND_TMOOTF_GAIN_TABLE158_OFFSET                            0x1c8
#define M_PEND_TMOOTF_GAIN_TABLE158_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE158_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE159_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE159_LENGTH                            16

/* REGISTER saturn_tm_reg_115 */
#define M_PEND_TMOOTF_GAIN_TABLE160_OFFSET                            0x1cc
#define M_PEND_TMOOTF_GAIN_TABLE160_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE160_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE161_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE161_LENGTH                            16

/* REGISTER saturn_tm_reg_116 */
#define M_PEND_TMOOTF_GAIN_TABLE162_OFFSET                            0x1d0
#define M_PEND_TMOOTF_GAIN_TABLE162_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE162_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE163_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE163_LENGTH                            16

/* REGISTER saturn_tm_reg_117 */
#define M_PEND_TMOOTF_GAIN_TABLE164_OFFSET                            0x1d4
#define M_PEND_TMOOTF_GAIN_TABLE164_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE164_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE165_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE165_LENGTH                            16

/* REGISTER saturn_tm_reg_118 */
#define M_PEND_TMOOTF_GAIN_TABLE166_OFFSET                            0x1d8
#define M_PEND_TMOOTF_GAIN_TABLE166_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE166_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE167_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE167_LENGTH                            16

/* REGISTER saturn_tm_reg_119 */
#define M_PEND_TMOOTF_GAIN_TABLE168_OFFSET                            0x1dc
#define M_PEND_TMOOTF_GAIN_TABLE168_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE168_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE169_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE169_LENGTH                            16

/* REGISTER saturn_tm_reg_120 */
#define M_PEND_TMOOTF_GAIN_TABLE170_OFFSET                            0x1e0
#define M_PEND_TMOOTF_GAIN_TABLE170_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE170_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE171_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE171_LENGTH                            16

/* REGISTER saturn_tm_reg_121 */
#define M_PEND_TMOOTF_GAIN_TABLE172_OFFSET                            0x1e4
#define M_PEND_TMOOTF_GAIN_TABLE172_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE172_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE173_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE173_LENGTH                            16

/* REGISTER saturn_tm_reg_122 */
#define M_PEND_TMOOTF_GAIN_TABLE174_OFFSET                            0x1e8
#define M_PEND_TMOOTF_GAIN_TABLE174_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE174_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE175_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE175_LENGTH                            16

/* REGISTER saturn_tm_reg_123 */
#define M_PEND_TMOOTF_GAIN_TABLE176_OFFSET                            0x1ec
#define M_PEND_TMOOTF_GAIN_TABLE176_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE176_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE177_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE177_LENGTH                            16

/* REGISTER saturn_tm_reg_124 */
#define M_PEND_TMOOTF_GAIN_TABLE178_OFFSET                            0x1f0
#define M_PEND_TMOOTF_GAIN_TABLE178_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE178_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE179_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE179_LENGTH                            16

/* REGISTER saturn_tm_reg_125 */
#define M_PEND_TMOOTF_GAIN_TABLE180_OFFSET                            0x1f4
#define M_PEND_TMOOTF_GAIN_TABLE180_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE180_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE181_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE181_LENGTH                            16

/* REGISTER saturn_tm_reg_126 */
#define M_PEND_TMOOTF_GAIN_TABLE182_OFFSET                            0x1f8
#define M_PEND_TMOOTF_GAIN_TABLE182_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE182_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE183_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE183_LENGTH                            16

/* REGISTER saturn_tm_reg_127 */
#define M_PEND_TMOOTF_GAIN_TABLE184_OFFSET                            0x1fc
#define M_PEND_TMOOTF_GAIN_TABLE184_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE184_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE185_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE185_LENGTH                            16

/* REGISTER saturn_tm_reg_128 */
#define M_PEND_TMOOTF_GAIN_TABLE186_OFFSET                            0x200
#define M_PEND_TMOOTF_GAIN_TABLE186_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE186_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE187_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE187_LENGTH                            16

/* REGISTER saturn_tm_reg_129 */
#define M_PEND_TMOOTF_GAIN_TABLE188_OFFSET                            0x204
#define M_PEND_TMOOTF_GAIN_TABLE188_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE188_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE189_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE189_LENGTH                            16

/* REGISTER saturn_tm_reg_130 */
#define M_PEND_TMOOTF_GAIN_TABLE190_OFFSET                            0x208
#define M_PEND_TMOOTF_GAIN_TABLE190_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE190_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE191_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE191_LENGTH                            16

/* REGISTER saturn_tm_reg_131 */
#define M_PEND_TMOOTF_GAIN_TABLE192_OFFSET                            0x20c
#define M_PEND_TMOOTF_GAIN_TABLE192_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE192_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE193_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE193_LENGTH                            16

/* REGISTER saturn_tm_reg_132 */
#define M_PEND_TMOOTF_GAIN_TABLE194_OFFSET                            0x210
#define M_PEND_TMOOTF_GAIN_TABLE194_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE194_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE195_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE195_LENGTH                            16

/* REGISTER saturn_tm_reg_133 */
#define M_PEND_TMOOTF_GAIN_TABLE196_OFFSET                            0x214
#define M_PEND_TMOOTF_GAIN_TABLE196_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE196_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE197_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE197_LENGTH                            16

/* REGISTER saturn_tm_reg_134 */
#define M_PEND_TMOOTF_GAIN_TABLE198_OFFSET                            0x218
#define M_PEND_TMOOTF_GAIN_TABLE198_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE198_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE199_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE199_LENGTH                            16

/* REGISTER saturn_tm_reg_135 */
#define M_PEND_TMOOTF_GAIN_TABLE200_OFFSET                            0x21c
#define M_PEND_TMOOTF_GAIN_TABLE200_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE200_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE201_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE201_LENGTH                            16

/* REGISTER saturn_tm_reg_136 */
#define M_PEND_TMOOTF_GAIN_TABLE202_OFFSET                            0x220
#define M_PEND_TMOOTF_GAIN_TABLE202_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE202_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE203_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE203_LENGTH                            16

/* REGISTER saturn_tm_reg_137 */
#define M_PEND_TMOOTF_GAIN_TABLE204_OFFSET                            0x224
#define M_PEND_TMOOTF_GAIN_TABLE204_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE204_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE205_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE205_LENGTH                            16

/* REGISTER saturn_tm_reg_138 */
#define M_PEND_TMOOTF_GAIN_TABLE206_OFFSET                            0x228
#define M_PEND_TMOOTF_GAIN_TABLE206_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE206_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE207_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE207_LENGTH                            16

/* REGISTER saturn_tm_reg_139 */
#define M_PEND_TMOOTF_GAIN_TABLE208_OFFSET                            0x22c
#define M_PEND_TMOOTF_GAIN_TABLE208_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE208_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE209_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE209_LENGTH                            16

/* REGISTER saturn_tm_reg_140 */
#define M_PEND_TMOOTF_GAIN_TABLE210_OFFSET                            0x230
#define M_PEND_TMOOTF_GAIN_TABLE210_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE210_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE211_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE211_LENGTH                            16

/* REGISTER saturn_tm_reg_141 */
#define M_PEND_TMOOTF_GAIN_TABLE212_OFFSET                            0x234
#define M_PEND_TMOOTF_GAIN_TABLE212_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE212_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE213_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE213_LENGTH                            16

/* REGISTER saturn_tm_reg_142 */
#define M_PEND_TMOOTF_GAIN_TABLE214_OFFSET                            0x238
#define M_PEND_TMOOTF_GAIN_TABLE214_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE214_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE215_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE215_LENGTH                            16

/* REGISTER saturn_tm_reg_143 */
#define M_PEND_TMOOTF_GAIN_TABLE216_OFFSET                            0x23c
#define M_PEND_TMOOTF_GAIN_TABLE216_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE216_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE217_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE217_LENGTH                            16

/* REGISTER saturn_tm_reg_144 */
#define M_PEND_TMOOTF_GAIN_TABLE218_OFFSET                            0x240
#define M_PEND_TMOOTF_GAIN_TABLE218_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE218_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE219_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE219_LENGTH                            16

/* REGISTER saturn_tm_reg_145 */
#define M_PEND_TMOOTF_GAIN_TABLE220_OFFSET                            0x244
#define M_PEND_TMOOTF_GAIN_TABLE220_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE220_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE221_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE221_LENGTH                            16

/* REGISTER saturn_tm_reg_146 */
#define M_PEND_TMOOTF_GAIN_TABLE222_OFFSET                            0x248
#define M_PEND_TMOOTF_GAIN_TABLE222_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE222_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE223_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE223_LENGTH                            16

/* REGISTER saturn_tm_reg_147 */
#define M_PEND_TMOOTF_GAIN_TABLE224_OFFSET                            0x24c
#define M_PEND_TMOOTF_GAIN_TABLE224_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE224_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE225_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE225_LENGTH                            16

/* REGISTER saturn_tm_reg_148 */
#define M_PEND_TMOOTF_GAIN_TABLE226_OFFSET                            0x250
#define M_PEND_TMOOTF_GAIN_TABLE226_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE226_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE227_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE227_LENGTH                            16

/* REGISTER saturn_tm_reg_149 */
#define M_PEND_TMOOTF_GAIN_TABLE228_OFFSET                            0x254
#define M_PEND_TMOOTF_GAIN_TABLE228_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE228_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE229_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE229_LENGTH                            16

/* REGISTER saturn_tm_reg_150 */
#define M_PEND_TMOOTF_GAIN_TABLE230_OFFSET                            0x258
#define M_PEND_TMOOTF_GAIN_TABLE230_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE230_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE231_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE231_LENGTH                            16

/* REGISTER saturn_tm_reg_151 */
#define M_PEND_TMOOTF_GAIN_TABLE232_OFFSET                            0x25c
#define M_PEND_TMOOTF_GAIN_TABLE232_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE232_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE233_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE233_LENGTH                            16

/* REGISTER saturn_tm_reg_152 */
#define M_PEND_TMOOTF_GAIN_TABLE234_OFFSET                            0x260
#define M_PEND_TMOOTF_GAIN_TABLE234_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE234_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE235_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE235_LENGTH                            16

/* REGISTER saturn_tm_reg_153 */
#define M_PEND_TMOOTF_GAIN_TABLE236_OFFSET                            0x264
#define M_PEND_TMOOTF_GAIN_TABLE236_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE236_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE237_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE237_LENGTH                            16

/* REGISTER saturn_tm_reg_154 */
#define M_PEND_TMOOTF_GAIN_TABLE238_OFFSET                            0x268
#define M_PEND_TMOOTF_GAIN_TABLE238_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE238_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE239_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE239_LENGTH                            16

/* REGISTER saturn_tm_reg_155 */
#define M_PEND_TMOOTF_GAIN_TABLE240_OFFSET                            0x26c
#define M_PEND_TMOOTF_GAIN_TABLE240_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE240_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE241_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE241_LENGTH                            16

/* REGISTER saturn_tm_reg_156 */
#define M_PEND_TMOOTF_GAIN_TABLE242_OFFSET                            0x270
#define M_PEND_TMOOTF_GAIN_TABLE242_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE242_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE243_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE243_LENGTH                            16

/* REGISTER saturn_tm_reg_157 */
#define M_PEND_TMOOTF_GAIN_TABLE244_OFFSET                            0x274
#define M_PEND_TMOOTF_GAIN_TABLE244_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE244_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE245_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE245_LENGTH                            16

/* REGISTER saturn_tm_reg_158 */
#define M_PEND_TMOOTF_GAIN_TABLE246_OFFSET                            0x278
#define M_PEND_TMOOTF_GAIN_TABLE246_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE246_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE247_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE247_LENGTH                            16

/* REGISTER saturn_tm_reg_159 */
#define M_PEND_TMOOTF_GAIN_TABLE248_OFFSET                            0x27c
#define M_PEND_TMOOTF_GAIN_TABLE248_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE248_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE249_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE249_LENGTH                            16

/* REGISTER saturn_tm_reg_160 */
#define M_PEND_TMOOTF_GAIN_TABLE250_OFFSET                            0x280
#define M_PEND_TMOOTF_GAIN_TABLE250_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE250_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE251_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE251_LENGTH                            16

/* REGISTER saturn_tm_reg_161 */
#define M_PEND_TMOOTF_GAIN_TABLE252_OFFSET                            0x284
#define M_PEND_TMOOTF_GAIN_TABLE252_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE252_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE253_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE253_LENGTH                            16

/* REGISTER saturn_tm_reg_162 */
#define M_PEND_TMOOTF_GAIN_TABLE254_OFFSET                            0x288
#define M_PEND_TMOOTF_GAIN_TABLE254_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE254_LENGTH                            16
#define M_PEND_TMOOTF_GAIN_TABLE255_SHIFT                             16
#define M_PEND_TMOOTF_GAIN_TABLE255_LENGTH                            16

/* REGISTER saturn_tm_reg_163 */
#define M_PEND_TMOOTF_GAIN_TABLE256_OFFSET                            0x28c
#define M_PEND_TMOOTF_GAIN_TABLE256_SHIFT                             0
#define M_PEND_TMOOTF_GAIN_TABLE256_LENGTH                            16

/* REGISTER saturn_tm_reg_164 */
#define TM_M_NEND_TMOOTF_SHIFT_BITS_OFFSET                            0x290
#define TM_M_NEND_TMOOTF_SHIFT_BITS_SHIFT                             0
#define TM_M_NEND_TMOOTF_SHIFT_BITS_LENGTH                            5
#define TM_M_NEND_TMOOTF_RGB_MODE_SHIFT                               5
#define TM_M_NEND_TMOOTF_RGB_MODE_LENGTH                              2
#define TM_M_PMATRIX_TABLE0_SHIFT                                     16
#define TM_M_PMATRIX_TABLE0_LENGTH                                    16

/* REGISTER saturn_tm_reg_165 */
#define TM_M_PMATRIX_TABLE1_OFFSET                                    0x294
#define TM_M_PMATRIX_TABLE1_SHIFT                                     0
#define TM_M_PMATRIX_TABLE1_LENGTH                                    16
#define TM_M_PMATRIX_TABLE2_SHIFT                                     16
#define TM_M_PMATRIX_TABLE2_LENGTH                                    16

/* REGISTER saturn_tm_reg_166 */
#define TM_M_PMATRIX_TABLE3_OFFSET                                    0x298
#define TM_M_PMATRIX_TABLE3_SHIFT                                     0
#define TM_M_PMATRIX_TABLE3_LENGTH                                    16
#define TM_M_PMATRIX_TABLE4_SHIFT                                     16
#define TM_M_PMATRIX_TABLE4_LENGTH                                    16

/* REGISTER saturn_tm_reg_167 */
#define TM_M_PMATRIX_TABLE5_OFFSET                                    0x29c
#define TM_M_PMATRIX_TABLE5_SHIFT                                     0
#define TM_M_PMATRIX_TABLE5_LENGTH                                    16
#define TM_M_PMATRIX_TABLE6_SHIFT                                     16
#define TM_M_PMATRIX_TABLE6_LENGTH                                    16

/* REGISTER saturn_tm_reg_168 */
#define TM_M_PMATRIX_TABLE7_OFFSET                                    0x2a0
#define TM_M_PMATRIX_TABLE7_SHIFT                                     0
#define TM_M_PMATRIX_TABLE7_LENGTH                                    16
#define TM_M_PMATRIX_TABLE8_SHIFT                                     16
#define TM_M_PMATRIX_TABLE8_LENGTH                                    16

/* REGISTER saturn_tm_reg_169 */
#define TM_M_PMATRIX_OFFSET0_OFFSET                                   0x2a4
#define TM_M_PMATRIX_OFFSET0_SHIFT                                    0
#define TM_M_PMATRIX_OFFSET0_LENGTH                                   25

/* REGISTER saturn_tm_reg_170 */
#define TM_M_PMATRIX_OFFSET1_OFFSET                                   0x2a8
#define TM_M_PMATRIX_OFFSET1_SHIFT                                    0
#define TM_M_PMATRIX_OFFSET1_LENGTH                                   25

/* REGISTER saturn_tm_reg_171 */
#define TM_M_PMATRIX_OFFSET2_OFFSET                                   0x2ac
#define TM_M_PMATRIX_OFFSET2_SHIFT                                    0
#define TM_M_PMATRIX_OFFSET2_LENGTH                                   25

/* REGISTER saturn_tm_reg_172 */
#define TM_M_NGAIN_TO_FULL_OFFSET                                     0x2b0
#define TM_M_NGAIN_TO_FULL_SHIFT                                      0
#define TM_M_NGAIN_TO_FULL_LENGTH                                     16
#define TM_M_PCOLOR_KEY_R_THR0_SHIFT                                  16
#define TM_M_PCOLOR_KEY_R_THR0_LENGTH                                 12

/* REGISTER saturn_tm_reg_173 */
#define TM_M_PCOLOR_KEY_R_THR1_OFFSET                                 0x2b4
#define TM_M_PCOLOR_KEY_R_THR1_SHIFT                                  0
#define TM_M_PCOLOR_KEY_R_THR1_LENGTH                                 12
#define TM_M_PCOLOR_KEY_G_THR0_SHIFT                                  16
#define TM_M_PCOLOR_KEY_G_THR0_LENGTH                                 12

/* REGISTER saturn_tm_reg_174 */
#define TM_M_PCOLOR_KEY_G_THR1_OFFSET                                 0x2b8
#define TM_M_PCOLOR_KEY_G_THR1_SHIFT                                  0
#define TM_M_PCOLOR_KEY_G_THR1_LENGTH                                 12
#define TM_M_PCOLOR_KEY_B_THR0_SHIFT                                  16
#define TM_M_PCOLOR_KEY_B_THR0_LENGTH                                 12

/* REGISTER saturn_tm_reg_175 */
#define TM_M_PCOLOR_KEY_B_THR1_OFFSET                                 0x2bc
#define TM_M_PCOLOR_KEY_B_THR1_SHIFT                                  0
#define TM_M_PCOLOR_KEY_B_THR1_LENGTH                                 12

/* REGISTER saturn_tm_reg_176 */
#define TM_LUT4_GROUP_0_OFFSET                                        0x2c0
#define TM_LUT4_GROUP_0_SHIFT                                         0
#define TM_LUT4_GROUP_0_LENGTH                                        24

/* REGISTER saturn_tm_reg_177 */
#define TM_LUT4_GROUP_1_OFFSET                                        0x2c4
#define TM_LUT4_GROUP_1_SHIFT                                         0
#define TM_LUT4_GROUP_1_LENGTH                                        24

/* REGISTER saturn_tm_reg_178 */
#define TM_LUT4_GROUP_2_OFFSET                                        0x2c8
#define TM_LUT4_GROUP_2_SHIFT                                         0
#define TM_LUT4_GROUP_2_LENGTH                                        24

/* REGISTER saturn_tm_reg_179 */
#define TM_LUT4_GROUP_3_OFFSET                                        0x2cc
#define TM_LUT4_GROUP_3_SHIFT                                         0
#define TM_LUT4_GROUP_3_LENGTH                                        24

/* REGISTER saturn_tm_reg_180 */
#define TM_LUT4_GROUP_4_OFFSET                                        0x2d0
#define TM_LUT4_GROUP_4_SHIFT                                         0
#define TM_LUT4_GROUP_4_LENGTH                                        24

/* REGISTER saturn_tm_reg_181 */
#define TM_LUT4_GROUP_5_OFFSET                                        0x2d4
#define TM_LUT4_GROUP_5_SHIFT                                         0
#define TM_LUT4_GROUP_5_LENGTH                                        24

/* REGISTER saturn_tm_reg_182 */
#define TM_LUT4_GROUP_6_OFFSET                                        0x2d8
#define TM_LUT4_GROUP_6_SHIFT                                         0
#define TM_LUT4_GROUP_6_LENGTH                                        24

/* REGISTER saturn_tm_reg_183 */
#define TM_LUT4_GROUP_7_OFFSET                                        0x2dc
#define TM_LUT4_GROUP_7_SHIFT                                         0
#define TM_LUT4_GROUP_7_LENGTH                                        24

/* REGISTER saturn_tm_reg_184 */
#define TM_LUT4_GROUP_8_OFFSET                                        0x2e0
#define TM_LUT4_GROUP_8_SHIFT                                         0
#define TM_LUT4_GROUP_8_LENGTH                                        24

/* REGISTER saturn_tm_reg_185 */
#define TM_LUT4_GROUP_9_OFFSET                                        0x2e4
#define TM_LUT4_GROUP_9_SHIFT                                         0
#define TM_LUT4_GROUP_9_LENGTH                                        24

/* REGISTER saturn_tm_reg_186 */
#define TM_LUT4_GROUP_10_OFFSET                                       0x2e8
#define TM_LUT4_GROUP_10_SHIFT                                        0
#define TM_LUT4_GROUP_10_LENGTH                                       24

/* REGISTER saturn_tm_reg_187 */
#define TM_LUT4_GROUP_11_OFFSET                                       0x2ec
#define TM_LUT4_GROUP_11_SHIFT                                        0
#define TM_LUT4_GROUP_11_LENGTH                                       24

/* REGISTER saturn_tm_reg_188 */
#define TM_LUT4_GROUP_12_OFFSET                                       0x2f0
#define TM_LUT4_GROUP_12_SHIFT                                        0
#define TM_LUT4_GROUP_12_LENGTH                                       24

/* REGISTER saturn_tm_reg_189 */
#define TM_LUT4_GROUP_13_OFFSET                                       0x2f4
#define TM_LUT4_GROUP_13_SHIFT                                        0
#define TM_LUT4_GROUP_13_LENGTH                                       24

/* REGISTER saturn_tm_reg_190 */
#define TM_LUT4_GROUP_14_OFFSET                                       0x2f8
#define TM_LUT4_GROUP_14_SHIFT                                        0
#define TM_LUT4_GROUP_14_LENGTH                                       24

/* REGISTER saturn_tm_reg_191 */
#define TM_LUT4_GROUP_15_OFFSET                                       0x2fc
#define TM_LUT4_GROUP_15_SHIFT                                        0
#define TM_LUT4_GROUP_15_LENGTH                                       24

/* REGISTER saturn_tm_reg_192 */
#define TM_LUT4_GROUP_16_OFFSET                                       0x300
#define TM_LUT4_GROUP_16_SHIFT                                        0
#define TM_LUT4_GROUP_16_LENGTH                                       24

/* REGISTER saturn_tm_reg_193 */
#define TM_LUT4_GROUP_17_OFFSET                                       0x304
#define TM_LUT4_GROUP_17_SHIFT                                        0
#define TM_LUT4_GROUP_17_LENGTH                                       24

/* REGISTER saturn_tm_reg_194 */
#define TM_LUT4_GROUP_18_OFFSET                                       0x308
#define TM_LUT4_GROUP_18_SHIFT                                        0
#define TM_LUT4_GROUP_18_LENGTH                                       24

/* REGISTER saturn_tm_reg_195 */
#define TM_LUT4_GROUP_19_OFFSET                                       0x30c
#define TM_LUT4_GROUP_19_SHIFT                                        0
#define TM_LUT4_GROUP_19_LENGTH                                       24

/* REGISTER saturn_tm_reg_196 */
#define TM_LUT4_GROUP_20_OFFSET                                       0x310
#define TM_LUT4_GROUP_20_SHIFT                                        0
#define TM_LUT4_GROUP_20_LENGTH                                       24

/* REGISTER saturn_tm_reg_197 */
#define TM_LUT4_GROUP_21_OFFSET                                       0x314
#define TM_LUT4_GROUP_21_SHIFT                                        0
#define TM_LUT4_GROUP_21_LENGTH                                       24

/* REGISTER saturn_tm_reg_198 */
#define TM_LUT4_GROUP_22_OFFSET                                       0x318
#define TM_LUT4_GROUP_22_SHIFT                                        0
#define TM_LUT4_GROUP_22_LENGTH                                       24

/* REGISTER saturn_tm_reg_199 */
#define TM_LUT4_GROUP_23_OFFSET                                       0x31c
#define TM_LUT4_GROUP_23_SHIFT                                        0
#define TM_LUT4_GROUP_23_LENGTH                                       24

/* REGISTER saturn_tm_reg_200 */
#define TM_LUT4_GROUP_24_OFFSET                                       0x320
#define TM_LUT4_GROUP_24_SHIFT                                        0
#define TM_LUT4_GROUP_24_LENGTH                                       24

/* REGISTER saturn_tm_reg_201 */
#define TM_LUT4_GROUP_25_OFFSET                                       0x324
#define TM_LUT4_GROUP_25_SHIFT                                        0
#define TM_LUT4_GROUP_25_LENGTH                                       24

/* REGISTER saturn_tm_reg_202 */
#define TM_LUT4_GROUP_26_OFFSET                                       0x328
#define TM_LUT4_GROUP_26_SHIFT                                        0
#define TM_LUT4_GROUP_26_LENGTH                                       24

/* REGISTER saturn_tm_reg_203 */
#define TM_LUT4_GROUP_27_OFFSET                                       0x32c
#define TM_LUT4_GROUP_27_SHIFT                                        0
#define TM_LUT4_GROUP_27_LENGTH                                       24

/* REGISTER saturn_tm_reg_204 */
#define TM_LUT4_GROUP_28_OFFSET                                       0x330
#define TM_LUT4_GROUP_28_SHIFT                                        0
#define TM_LUT4_GROUP_28_LENGTH                                       24

/* REGISTER saturn_tm_reg_205 */
#define TM_LUT4_GROUP_29_OFFSET                                       0x334
#define TM_LUT4_GROUP_29_SHIFT                                        0
#define TM_LUT4_GROUP_29_LENGTH                                       24

/* REGISTER saturn_tm_reg_206 */
#define TM_LUT4_GROUP_30_OFFSET                                       0x338
#define TM_LUT4_GROUP_30_SHIFT                                        0
#define TM_LUT4_GROUP_30_LENGTH                                       24

/* REGISTER saturn_tm_reg_207 */
#define TM_LUT4_GROUP_31_OFFSET                                       0x33c
#define TM_LUT4_GROUP_31_SHIFT                                        0
#define TM_LUT4_GROUP_31_LENGTH                                       24

/* REGISTER saturn_tm_reg_208 */
#define TM_LUT4_GROUP_32_OFFSET                                       0x340
#define TM_LUT4_GROUP_32_SHIFT                                        0
#define TM_LUT4_GROUP_32_LENGTH                                       24

/* REGISTER saturn_tm_reg_209 */
#define TM_LUT4_GROUP_33_OFFSET                                       0x344
#define TM_LUT4_GROUP_33_SHIFT                                        0
#define TM_LUT4_GROUP_33_LENGTH                                       24

/* REGISTER saturn_tm_reg_210 */
#define TM_LUT4_GROUP_34_OFFSET                                       0x348
#define TM_LUT4_GROUP_34_SHIFT                                        0
#define TM_LUT4_GROUP_34_LENGTH                                       24

/* REGISTER saturn_tm_reg_211 */
#define TM_LUT4_GROUP_35_OFFSET                                       0x34c
#define TM_LUT4_GROUP_35_SHIFT                                        0
#define TM_LUT4_GROUP_35_LENGTH                                       24

/* REGISTER saturn_tm_reg_212 */
#define TM_LUT4_GROUP_36_OFFSET                                       0x350
#define TM_LUT4_GROUP_36_SHIFT                                        0
#define TM_LUT4_GROUP_36_LENGTH                                       24

/* REGISTER saturn_tm_reg_213 */
#define TM_LUT4_GROUP_37_OFFSET                                       0x354
#define TM_LUT4_GROUP_37_SHIFT                                        0
#define TM_LUT4_GROUP_37_LENGTH                                       24

/* REGISTER saturn_tm_reg_214 */
#define TM_LUT4_GROUP_38_OFFSET                                       0x358
#define TM_LUT4_GROUP_38_SHIFT                                        0
#define TM_LUT4_GROUP_38_LENGTH                                       24

/* REGISTER saturn_tm_reg_215 */
#define TM_LUT4_GROUP_39_OFFSET                                       0x35c
#define TM_LUT4_GROUP_39_SHIFT                                        0
#define TM_LUT4_GROUP_39_LENGTH                                       24

/* REGISTER saturn_tm_reg_216 */
#define TM_LUT4_GROUP_40_OFFSET                                       0x360
#define TM_LUT4_GROUP_40_SHIFT                                        0
#define TM_LUT4_GROUP_40_LENGTH                                       24

/* REGISTER saturn_tm_reg_217 */
#define TM_LUT4_GROUP_41_OFFSET                                       0x364
#define TM_LUT4_GROUP_41_SHIFT                                        0
#define TM_LUT4_GROUP_41_LENGTH                                       24

/* REGISTER saturn_tm_reg_218 */
#define TM_LUT4_GROUP_42_OFFSET                                       0x368
#define TM_LUT4_GROUP_42_SHIFT                                        0
#define TM_LUT4_GROUP_42_LENGTH                                       24

/* REGISTER saturn_tm_reg_219 */
#define TM_LUT4_GROUP_43_OFFSET                                       0x36c
#define TM_LUT4_GROUP_43_SHIFT                                        0
#define TM_LUT4_GROUP_43_LENGTH                                       24

/* REGISTER saturn_tm_reg_220 */
#define TM_LUT4_GROUP_44_OFFSET                                       0x370
#define TM_LUT4_GROUP_44_SHIFT                                        0
#define TM_LUT4_GROUP_44_LENGTH                                       24

/* REGISTER saturn_tm_reg_221 */
#define TM_LUT4_GROUP_45_OFFSET                                       0x374
#define TM_LUT4_GROUP_45_SHIFT                                        0
#define TM_LUT4_GROUP_45_LENGTH                                       24

/* REGISTER saturn_tm_reg_222 */
#define TM_LUT4_GROUP_46_OFFSET                                       0x378
#define TM_LUT4_GROUP_46_SHIFT                                        0
#define TM_LUT4_GROUP_46_LENGTH                                       24

/* REGISTER saturn_tm_reg_223 */
#define TM_LUT4_GROUP_47_OFFSET                                       0x37c
#define TM_LUT4_GROUP_47_SHIFT                                        0
#define TM_LUT4_GROUP_47_LENGTH                                       24

/* REGISTER saturn_tm_reg_224 */
#define TM_LUT4_GROUP_48_OFFSET                                       0x380
#define TM_LUT4_GROUP_48_SHIFT                                        0
#define TM_LUT4_GROUP_48_LENGTH                                       24

/* REGISTER saturn_tm_reg_225 */
#define TM_LUT4_GROUP_49_OFFSET                                       0x384
#define TM_LUT4_GROUP_49_SHIFT                                        0
#define TM_LUT4_GROUP_49_LENGTH                                       24

/* REGISTER saturn_tm_reg_226 */
#define TM_LUT4_GROUP_50_OFFSET                                       0x388
#define TM_LUT4_GROUP_50_SHIFT                                        0
#define TM_LUT4_GROUP_50_LENGTH                                       24

/* REGISTER saturn_tm_reg_227 */
#define TM_LUT4_GROUP_51_OFFSET                                       0x38c
#define TM_LUT4_GROUP_51_SHIFT                                        0
#define TM_LUT4_GROUP_51_LENGTH                                       24

/* REGISTER saturn_tm_reg_228 */
#define TM_LUT4_GROUP_52_OFFSET                                       0x390
#define TM_LUT4_GROUP_52_SHIFT                                        0
#define TM_LUT4_GROUP_52_LENGTH                                       24

/* REGISTER saturn_tm_reg_229 */
#define TM_LUT4_GROUP_53_OFFSET                                       0x394
#define TM_LUT4_GROUP_53_SHIFT                                        0
#define TM_LUT4_GROUP_53_LENGTH                                       24

/* REGISTER saturn_tm_reg_230 */
#define TM_LUT4_GROUP_54_OFFSET                                       0x398
#define TM_LUT4_GROUP_54_SHIFT                                        0
#define TM_LUT4_GROUP_54_LENGTH                                       24

/* REGISTER saturn_tm_reg_231 */
#define TM_LUT4_GROUP_55_OFFSET                                       0x39c
#define TM_LUT4_GROUP_55_SHIFT                                        0
#define TM_LUT4_GROUP_55_LENGTH                                       24

/* REGISTER saturn_tm_reg_232 */
#define TM_LUT4_GROUP_56_OFFSET                                       0x3a0
#define TM_LUT4_GROUP_56_SHIFT                                        0
#define TM_LUT4_GROUP_56_LENGTH                                       24

/* REGISTER saturn_tm_reg_233 */
#define TM_LUT4_GROUP_57_OFFSET                                       0x3a4
#define TM_LUT4_GROUP_57_SHIFT                                        0
#define TM_LUT4_GROUP_57_LENGTH                                       24

/* REGISTER saturn_tm_reg_234 */
#define TM_LUT4_GROUP_58_OFFSET                                       0x3a8
#define TM_LUT4_GROUP_58_SHIFT                                        0
#define TM_LUT4_GROUP_58_LENGTH                                       24

/* REGISTER saturn_tm_reg_235 */
#define TM_LUT4_GROUP_59_OFFSET                                       0x3ac
#define TM_LUT4_GROUP_59_SHIFT                                        0
#define TM_LUT4_GROUP_59_LENGTH                                       24

/* REGISTER saturn_tm_reg_236 */
#define TM_LUT4_GROUP_60_OFFSET                                       0x3b0
#define TM_LUT4_GROUP_60_SHIFT                                        0
#define TM_LUT4_GROUP_60_LENGTH                                       24

/* REGISTER saturn_tm_reg_237 */
#define TM_LUT4_GROUP_61_OFFSET                                       0x3b4
#define TM_LUT4_GROUP_61_SHIFT                                        0
#define TM_LUT4_GROUP_61_LENGTH                                       24

/* REGISTER saturn_tm_reg_238 */
#define TM_LUT4_GROUP_62_OFFSET                                       0x3b8
#define TM_LUT4_GROUP_62_SHIFT                                        0
#define TM_LUT4_GROUP_62_LENGTH                                       24

/* REGISTER saturn_tm_reg_239 */
#define TM_LUT4_GROUP_63_OFFSET                                       0x3bc
#define TM_LUT4_GROUP_63_SHIFT                                        0
#define TM_LUT4_GROUP_63_LENGTH                                       24

/* REGISTER saturn_tm_reg_240 */
#define TM_LUT4_GROUP_64_OFFSET                                       0x3c0
#define TM_LUT4_GROUP_64_SHIFT                                        0
#define TM_LUT4_GROUP_64_LENGTH                                       24

/* REGISTER saturn_tm_reg_241 */
#define TM_LUT4_GROUP_65_OFFSET                                       0x3c4
#define TM_LUT4_GROUP_65_SHIFT                                        0
#define TM_LUT4_GROUP_65_LENGTH                                       24

/* REGISTER saturn_tm_reg_242 */
#define TM_LUT4_GROUP_66_OFFSET                                       0x3c8
#define TM_LUT4_GROUP_66_SHIFT                                        0
#define TM_LUT4_GROUP_66_LENGTH                                       24

/* REGISTER saturn_tm_reg_243 */
#define TM_LUT4_GROUP_67_OFFSET                                       0x3cc
#define TM_LUT4_GROUP_67_SHIFT                                        0
#define TM_LUT4_GROUP_67_LENGTH                                       24

/* REGISTER saturn_tm_reg_244 */
#define TM_LUT4_GROUP_68_OFFSET                                       0x3d0
#define TM_LUT4_GROUP_68_SHIFT                                        0
#define TM_LUT4_GROUP_68_LENGTH                                       24

/* REGISTER saturn_tm_reg_245 */
#define TM_LUT4_GROUP_69_OFFSET                                       0x3d4
#define TM_LUT4_GROUP_69_SHIFT                                        0
#define TM_LUT4_GROUP_69_LENGTH                                       24

/* REGISTER saturn_tm_reg_246 */
#define TM_LUT4_GROUP_70_OFFSET                                       0x3d8
#define TM_LUT4_GROUP_70_SHIFT                                        0
#define TM_LUT4_GROUP_70_LENGTH                                       24


/* REGISTER saturn_tm_reg_248 */
#define TM_LUT5_GROUP_0_OFFSET                                        0x3e0
#define TM_LUT5_GROUP_0_SHIFT                                         0
#define TM_LUT5_GROUP_0_LENGTH                                        24

/* REGISTER saturn_tm_reg_249 */
#define TM_LUT5_GROUP_1_OFFSET                                        0x3e4
#define TM_LUT5_GROUP_1_SHIFT                                         0
#define TM_LUT5_GROUP_1_LENGTH                                        24

/* REGISTER saturn_tm_reg_250 */
#define TM_LUT5_GROUP_2_OFFSET                                        0x3e8
#define TM_LUT5_GROUP_2_SHIFT                                         0
#define TM_LUT5_GROUP_2_LENGTH                                        24

/* REGISTER saturn_tm_reg_251 */
#define TM_LUT5_GROUP_3_OFFSET                                        0x3ec
#define TM_LUT5_GROUP_3_SHIFT                                         0
#define TM_LUT5_GROUP_3_LENGTH                                        24

/* REGISTER saturn_tm_reg_252 */
#define TM_LUT5_GROUP_4_OFFSET                                        0x3f0
#define TM_LUT5_GROUP_4_SHIFT                                         0
#define TM_LUT5_GROUP_4_LENGTH                                        24

/* REGISTER saturn_tm_reg_253 */
#define TM_LUT5_GROUP_5_OFFSET                                        0x3f4
#define TM_LUT5_GROUP_5_SHIFT                                         0
#define TM_LUT5_GROUP_5_LENGTH                                        24

/* REGISTER saturn_tm_reg_254 */
#define TM_LUT5_GROUP_6_OFFSET                                        0x3f8
#define TM_LUT5_GROUP_6_SHIFT                                         0
#define TM_LUT5_GROUP_6_LENGTH                                        24

/* REGISTER saturn_tm_reg_255 */
#define TM_LUT5_GROUP_7_OFFSET                                        0x3fc
#define TM_LUT5_GROUP_7_SHIFT                                         0
#define TM_LUT5_GROUP_7_LENGTH                                        24

/* REGISTER saturn_tm_reg_256 */
#define TM_LUT5_GROUP_8_OFFSET                                        0x400
#define TM_LUT5_GROUP_8_SHIFT                                         0
#define TM_LUT5_GROUP_8_LENGTH                                        24

/* REGISTER saturn_tm_reg_257 */
#define TM_LUT5_GROUP_9_OFFSET                                        0x404
#define TM_LUT5_GROUP_9_SHIFT                                         0
#define TM_LUT5_GROUP_9_LENGTH                                        24

/* REGISTER saturn_tm_reg_258 */
#define TM_LUT5_GROUP_10_OFFSET                                       0x408
#define TM_LUT5_GROUP_10_SHIFT                                        0
#define TM_LUT5_GROUP_10_LENGTH                                       24

/* REGISTER saturn_tm_reg_259 */
#define TM_LUT5_GROUP_11_OFFSET                                       0x40c
#define TM_LUT5_GROUP_11_SHIFT                                        0
#define TM_LUT5_GROUP_11_LENGTH                                       24

/* REGISTER saturn_tm_reg_260 */
#define TM_LUT5_GROUP_12_OFFSET                                       0x410
#define TM_LUT5_GROUP_12_SHIFT                                        0
#define TM_LUT5_GROUP_12_LENGTH                                       24

/* REGISTER saturn_tm_reg_261 */
#define TM_LUT5_GROUP_13_OFFSET                                       0x414
#define TM_LUT5_GROUP_13_SHIFT                                        0
#define TM_LUT5_GROUP_13_LENGTH                                       24

/* REGISTER saturn_tm_reg_262 */
#define TM_LUT5_GROUP_14_OFFSET                                       0x418
#define TM_LUT5_GROUP_14_SHIFT                                        0
#define TM_LUT5_GROUP_14_LENGTH                                       24

/* REGISTER saturn_tm_reg_263 */
#define TM_LUT5_GROUP_15_OFFSET                                       0x41c
#define TM_LUT5_GROUP_15_SHIFT                                        0
#define TM_LUT5_GROUP_15_LENGTH                                       24

/* REGISTER saturn_tm_reg_264 */
#define TM_LUT5_GROUP_16_OFFSET                                       0x420
#define TM_LUT5_GROUP_16_SHIFT                                        0
#define TM_LUT5_GROUP_16_LENGTH                                       24

/* REGISTER saturn_tm_reg_265 */
#define TM_LUT5_GROUP_17_OFFSET                                       0x424
#define TM_LUT5_GROUP_17_SHIFT                                        0
#define TM_LUT5_GROUP_17_LENGTH                                       24

/* REGISTER saturn_tm_reg_266 */
#define TM_LUT5_GROUP_18_OFFSET                                       0x428
#define TM_LUT5_GROUP_18_SHIFT                                        0
#define TM_LUT5_GROUP_18_LENGTH                                       24

/* REGISTER saturn_tm_reg_267 */
#define TM_LUT5_GROUP_19_OFFSET                                       0x42c
#define TM_LUT5_GROUP_19_SHIFT                                        0
#define TM_LUT5_GROUP_19_LENGTH                                       24

/* REGISTER saturn_tm_reg_268 */
#define TM_LUT5_GROUP_20_OFFSET                                       0x430
#define TM_LUT5_GROUP_20_SHIFT                                        0
#define TM_LUT5_GROUP_20_LENGTH                                       24

/* REGISTER saturn_tm_reg_269 */
#define TM_LUT5_GROUP_21_OFFSET                                       0x434
#define TM_LUT5_GROUP_21_SHIFT                                        0
#define TM_LUT5_GROUP_21_LENGTH                                       24

/* REGISTER saturn_tm_reg_270 */
#define TM_LUT5_GROUP_22_OFFSET                                       0x438
#define TM_LUT5_GROUP_22_SHIFT                                        0
#define TM_LUT5_GROUP_22_LENGTH                                       24

/* REGISTER saturn_tm_reg_271 */
#define TM_LUT5_GROUP_23_OFFSET                                       0x43c
#define TM_LUT5_GROUP_23_SHIFT                                        0
#define TM_LUT5_GROUP_23_LENGTH                                       24

/* REGISTER saturn_tm_reg_272 */
#define TM_LUT5_GROUP_24_OFFSET                                       0x440
#define TM_LUT5_GROUP_24_SHIFT                                        0
#define TM_LUT5_GROUP_24_LENGTH                                       24

/* REGISTER saturn_tm_reg_273 */
#define TM_LUT5_GROUP_25_OFFSET                                       0x444
#define TM_LUT5_GROUP_25_SHIFT                                        0
#define TM_LUT5_GROUP_25_LENGTH                                       24

/* REGISTER saturn_tm_reg_274 */
#define TM_LUT5_GROUP_26_OFFSET                                       0x448
#define TM_LUT5_GROUP_26_SHIFT                                        0
#define TM_LUT5_GROUP_26_LENGTH                                       24

/* REGISTER saturn_tm_reg_275 */
#define TM_LUT5_GROUP_27_OFFSET                                       0x44c
#define TM_LUT5_GROUP_27_SHIFT                                        0
#define TM_LUT5_GROUP_27_LENGTH                                       24

/* REGISTER saturn_tm_reg_276 */
#define TM_LUT5_GROUP_28_OFFSET                                       0x450
#define TM_LUT5_GROUP_28_SHIFT                                        0
#define TM_LUT5_GROUP_28_LENGTH                                       24

/* REGISTER saturn_tm_reg_277 */
#define TM_LUT5_GROUP_29_OFFSET                                       0x454
#define TM_LUT5_GROUP_29_SHIFT                                        0
#define TM_LUT5_GROUP_29_LENGTH                                       24

/* REGISTER saturn_tm_reg_278 */
#define TM_LUT5_GROUP_30_OFFSET                                       0x458
#define TM_LUT5_GROUP_30_SHIFT                                        0
#define TM_LUT5_GROUP_30_LENGTH                                       24

/* REGISTER saturn_tm_reg_279 */
#define TM_LUT5_GROUP_31_OFFSET                                       0x45c
#define TM_LUT5_GROUP_31_SHIFT                                        0
#define TM_LUT5_GROUP_31_LENGTH                                       24

/* REGISTER saturn_tm_reg_280 */
#define TM_LUT5_GROUP_32_OFFSET                                       0x460
#define TM_LUT5_GROUP_32_SHIFT                                        0
#define TM_LUT5_GROUP_32_LENGTH                                       24

/* REGISTER saturn_tm_reg_281 */
#define TM_LUT5_GROUP_33_OFFSET                                       0x464
#define TM_LUT5_GROUP_33_SHIFT                                        0
#define TM_LUT5_GROUP_33_LENGTH                                       24

/* REGISTER saturn_tm_reg_282 */
#define TM_LUT5_GROUP_34_OFFSET                                       0x468
#define TM_LUT5_GROUP_34_SHIFT                                        0
#define TM_LUT5_GROUP_34_LENGTH                                       24

/* REGISTER saturn_tm_reg_283 */
#define TM_LUT5_GROUP_35_OFFSET                                       0x46c
#define TM_LUT5_GROUP_35_SHIFT                                        0
#define TM_LUT5_GROUP_35_LENGTH                                       24

/* REGISTER saturn_tm_reg_284 */
#define TM_LUT5_GROUP_36_OFFSET                                       0x470
#define TM_LUT5_GROUP_36_SHIFT                                        0
#define TM_LUT5_GROUP_36_LENGTH                                       24

/* REGISTER saturn_tm_reg_285 */
#define TM_LUT5_GROUP_37_OFFSET                                       0x474
#define TM_LUT5_GROUP_37_SHIFT                                        0
#define TM_LUT5_GROUP_37_LENGTH                                       24

/* REGISTER saturn_tm_reg_286 */
#define TM_LUT5_GROUP_38_OFFSET                                       0x478
#define TM_LUT5_GROUP_38_SHIFT                                        0
#define TM_LUT5_GROUP_38_LENGTH                                       24

/* REGISTER saturn_tm_reg_287 */
#define TM_LUT5_GROUP_39_OFFSET                                       0x47c
#define TM_LUT5_GROUP_39_SHIFT                                        0
#define TM_LUT5_GROUP_39_LENGTH                                       24

/* REGISTER saturn_tm_reg_288 */
#define TM_LUT5_GROUP_40_OFFSET                                       0x480
#define TM_LUT5_GROUP_40_SHIFT                                        0
#define TM_LUT5_GROUP_40_LENGTH                                       24

/* REGISTER saturn_tm_reg_289 */
#define TM_LUT5_GROUP_41_OFFSET                                       0x484
#define TM_LUT5_GROUP_41_SHIFT                                        0
#define TM_LUT5_GROUP_41_LENGTH                                       24

/* REGISTER saturn_tm_reg_290 */
#define TM_LUT5_GROUP_42_OFFSET                                       0x488
#define TM_LUT5_GROUP_42_SHIFT                                        0
#define TM_LUT5_GROUP_42_LENGTH                                       24

/* REGISTER saturn_tm_reg_291 */
#define TM_LUT5_GROUP_43_OFFSET                                       0x48c
#define TM_LUT5_GROUP_43_SHIFT                                        0
#define TM_LUT5_GROUP_43_LENGTH                                       24

/* REGISTER saturn_tm_reg_292 */
#define TM_LUT5_GROUP_44_OFFSET                                       0x490
#define TM_LUT5_GROUP_44_SHIFT                                        0
#define TM_LUT5_GROUP_44_LENGTH                                       24

/* REGISTER saturn_tm_reg_293 */
#define TM_LUT5_GROUP_45_OFFSET                                       0x494
#define TM_LUT5_GROUP_45_SHIFT                                        0
#define TM_LUT5_GROUP_45_LENGTH                                       24

/* REGISTER saturn_tm_reg_294 */
#define TM_LUT5_GROUP_46_OFFSET                                       0x498
#define TM_LUT5_GROUP_46_SHIFT                                        0
#define TM_LUT5_GROUP_46_LENGTH                                       24

/* REGISTER saturn_tm_reg_295 */
#define TM_LUT5_GROUP_47_OFFSET                                       0x49c
#define TM_LUT5_GROUP_47_SHIFT                                        0
#define TM_LUT5_GROUP_47_LENGTH                                       24

/* REGISTER saturn_tm_reg_296 */
#define TM_LUT5_GROUP_48_OFFSET                                       0x4a0
#define TM_LUT5_GROUP_48_SHIFT                                        0
#define TM_LUT5_GROUP_48_LENGTH                                       24

/* REGISTER saturn_tm_reg_297 */
#define TM_LUT5_GROUP_49_OFFSET                                       0x4a4
#define TM_LUT5_GROUP_49_SHIFT                                        0
#define TM_LUT5_GROUP_49_LENGTH                                       24

/* REGISTER saturn_tm_reg_298 */
#define TM_LUT5_GROUP_50_OFFSET                                       0x4a8
#define TM_LUT5_GROUP_50_SHIFT                                        0
#define TM_LUT5_GROUP_50_LENGTH                                       24

/* REGISTER saturn_tm_reg_299 */
#define TM_LUT5_GROUP_51_OFFSET                                       0x4ac
#define TM_LUT5_GROUP_51_SHIFT                                        0
#define TM_LUT5_GROUP_51_LENGTH                                       24

/* REGISTER saturn_tm_reg_300 */
#define TM_LUT5_GROUP_52_OFFSET                                       0x4b0
#define TM_LUT5_GROUP_52_SHIFT                                        0
#define TM_LUT5_GROUP_52_LENGTH                                       24

/* REGISTER saturn_tm_reg_301 */
#define TM_LUT5_GROUP_53_OFFSET                                       0x4b4
#define TM_LUT5_GROUP_53_SHIFT                                        0
#define TM_LUT5_GROUP_53_LENGTH                                       24

/* REGISTER saturn_tm_reg_302 */
#define TM_LUT5_GROUP_54_OFFSET                                       0x4b8
#define TM_LUT5_GROUP_54_SHIFT                                        0
#define TM_LUT5_GROUP_54_LENGTH                                       24

/* REGISTER saturn_tm_reg_303 */
#define TM_LUT5_GROUP_55_OFFSET                                       0x4bc
#define TM_LUT5_GROUP_55_SHIFT                                        0
#define TM_LUT5_GROUP_55_LENGTH                                       24

/* REGISTER saturn_tm_reg_304 */
#define TM_LUT5_GROUP_56_OFFSET                                       0x4c0
#define TM_LUT5_GROUP_56_SHIFT                                        0
#define TM_LUT5_GROUP_56_LENGTH                                       24

/* REGISTER saturn_tm_reg_305 */
#define TM_LUT5_GROUP_57_OFFSET                                       0x4c4
#define TM_LUT5_GROUP_57_SHIFT                                        0
#define TM_LUT5_GROUP_57_LENGTH                                       24

/* REGISTER saturn_tm_reg_306 */
#define TM_LUT5_GROUP_58_OFFSET                                       0x4c8
#define TM_LUT5_GROUP_58_SHIFT                                        0
#define TM_LUT5_GROUP_58_LENGTH                                       24

/* REGISTER saturn_tm_reg_307 */
#define TM_LUT5_GROUP_59_OFFSET                                       0x4cc
#define TM_LUT5_GROUP_59_SHIFT                                        0
#define TM_LUT5_GROUP_59_LENGTH                                       24

/* REGISTER saturn_tm_reg_308 */
#define TM_LUT5_GROUP_60_OFFSET                                       0x4d0
#define TM_LUT5_GROUP_60_SHIFT                                        0
#define TM_LUT5_GROUP_60_LENGTH                                       24

/* REGISTER saturn_tm_reg_309 */
#define TM_LUT5_GROUP_61_OFFSET                                       0x4d4
#define TM_LUT5_GROUP_61_SHIFT                                        0
#define TM_LUT5_GROUP_61_LENGTH                                       24

/* REGISTER saturn_tm_reg_310 */
#define TM_LUT5_GROUP_62_OFFSET                                       0x4d8
#define TM_LUT5_GROUP_62_SHIFT                                        0
#define TM_LUT5_GROUP_62_LENGTH                                       24

/* REGISTER saturn_tm_reg_311 */
#define TM_LUT5_GROUP_63_OFFSET                                       0x4dc
#define TM_LUT5_GROUP_63_SHIFT                                        0
#define TM_LUT5_GROUP_63_LENGTH                                       24

/* REGISTER saturn_tm_reg_312 */
#define TM_LUT5_GROUP_64_OFFSET                                       0x4e0
#define TM_LUT5_GROUP_64_SHIFT                                        0
#define TM_LUT5_GROUP_64_LENGTH                                       24

/* REGISTER saturn_tm_reg_313 */
#define TM_LUT5_GROUP_65_OFFSET                                       0x4e4
#define TM_LUT5_GROUP_65_SHIFT                                        0
#define TM_LUT5_GROUP_65_LENGTH                                       24

/* REGISTER saturn_tm_reg_314 */
#define TM_LUT5_GROUP_66_OFFSET                                       0x4e8
#define TM_LUT5_GROUP_66_SHIFT                                        0
#define TM_LUT5_GROUP_66_LENGTH                                       24

/* REGISTER saturn_tm_reg_315 */
#define TM_LUT5_GROUP_67_OFFSET                                       0x4ec
#define TM_LUT5_GROUP_67_SHIFT                                        0
#define TM_LUT5_GROUP_67_LENGTH                                       24

/* REGISTER saturn_tm_reg_316 */
#define TM_LUT5_GROUP_68_OFFSET                                       0x4f0
#define TM_LUT5_GROUP_68_SHIFT                                        0
#define TM_LUT5_GROUP_68_LENGTH                                       24

/* REGISTER saturn_tm_reg_317 */
#define TM_LUT5_GROUP_69_OFFSET                                       0x4f4
#define TM_LUT5_GROUP_69_SHIFT                                        0
#define TM_LUT5_GROUP_69_LENGTH                                       24

/* REGISTER saturn_tm_reg_318 */
#define TM_LUT5_GROUP_70_OFFSET                                       0x4f8
#define TM_LUT5_GROUP_70_SHIFT                                        0
#define TM_LUT5_GROUP_70_LENGTH                                       24

/* REGISTER saturn_tm_reg_319 */
#define TM_M_TMOOTF_WEIGHTYR_OFFSET                                   0x4fc
#define TM_M_TMOOTF_WEIGHTYR_SHIFT                                    0
#define TM_M_TMOOTF_WEIGHTYR_LENGTH                                   12
#define TM_M_TMOOTF_WEIGHTYG_SHIFT                                    16
#define TM_M_TMOOTF_WEIGHTYG_LENGTH                                   12

/* REGISTER saturn_tm_reg_320 */
#define TM_M_TMOOTF_WEIGHTYB_OFFSET                                   0x500
#define TM_M_TMOOTF_WEIGHTYB_SHIFT                                    0
#define TM_M_TMOOTF_WEIGHTYB_LENGTH                                   12

/* REGISTER saturn_tm_reg_321 */
#define TM_FORCE_UPDATE_EN_OFFSET                                     0x504
#define TM_FORCE_UPDATE_EN_SHIFT                                      0
#define TM_FORCE_UPDATE_EN_LENGTH                                     1
#define TM_VSYNC_UPDATE_EN_SHIFT                                      1
#define TM_VSYNC_UPDATE_EN_LENGTH                                     1
#define TM_SHADOW_READ_EN_SHIFT                                       2
#define TM_SHADOW_READ_EN_LENGTH                                      1

/* REGISTER saturn_tm_reg_322 */
#define TM_FORCE_UPDATE_PULSE_OFFSET                                  0x508
#define TM_FORCE_UPDATE_PULSE_SHIFT                                   0
#define TM_FORCE_UPDATE_PULSE_LENGTH                                  1

/* REGISTER saturn_tm_reg_323 */
#define TM_FORCE_UPDATE_EN_SE_OFFSET                                  0x50c
#define TM_FORCE_UPDATE_EN_SE_SHIFT                                   0
#define TM_FORCE_UPDATE_EN_SE_LENGTH                                  1
#define TM_VSYNC_UPDATE_EN_SE_SHIFT                                   1
#define TM_VSYNC_UPDATE_EN_SE_LENGTH                                  1
#define TM_SHADOW_READ_EN_SE_SHIFT                                    2
#define TM_SHADOW_READ_EN_SE_LENGTH                                   1

/* REGISTER saturn_tm_reg_324 */
#define TM_FORCE_UPDATE_PULSE_SE_OFFSET                               0x510
#define TM_FORCE_UPDATE_PULSE_SE_SHIFT                                0
#define TM_FORCE_UPDATE_PULSE_SE_LENGTH                               1

/* REGISTER saturn_tm_reg_325 */
#define TM_ICG_OVERRIDE_OFFSET                                        0x514
#define TM_ICG_OVERRIDE_SHIFT                                         0
#define TM_ICG_OVERRIDE_LENGTH                                        1

/* REGISTER saturn_tm_reg_326 */
#define TM_TRIGGER_OFFSET                                             0x518
#define TM_TRIGGER_SHIFT                                              0
#define TM_TRIGGER_LENGTH                                             1

/* REGISTER saturn_tm_reg_327 */
#define TM_TRIGGER2_OFFSET                                            0x51c
#define TM_TRIGGER2_SHIFT                                             0
#define TM_TRIGGER2_LENGTH                                            1

#endif
