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

#ifndef _DPU_HW_POSTPRE_PROC_REG_H_
#define _DPU_HW_POSTPRE_PROC_REG_H_

/* REGISTER saturn_post_pre_proc_reg_0 */
#define M_NPOST_PROC_EN_OFFSET                                        0x00
#define M_NPOST_PROC_EN_SHIFT                                         0
#define M_NPOST_PROC_EN_LENGTH                                        1
#define LTM_M_NGAIN_TO_FULL_EN_SHIFT                                  1
#define LTM_M_NGAIN_TO_FULL_EN_LENGTH                                 1
#define LTM_M_NMATRIX_EN_SHIFT                                        2
#define LTM_M_NMATRIX_EN_LENGTH                                       1
#define M_NENDMATRIX_EN_SHIFT                                         3
#define M_NENDMATRIX_EN_LENGTH                                        1
#define LTM_M_NFRONT_TMOOTF_EN_SHIFT                                  4
#define LTM_M_NFRONT_TMOOTF_EN_LENGTH                                 1
#define LTM_M_NEND_TMOOTF_EN_SHIFT                                    5
#define LTM_M_NEND_TMOOTF_EN_LENGTH                                   1
#define LTM_M_NEOTF_EN_SHIFT                                          6
#define LTM_M_NEOTF_EN_LENGTH                                         1
#define LTM_M_NOETF_EN_SHIFT                                          7
#define LTM_M_NOETF_EN_LENGTH                                         1

/* REGISTER saturn_post_pre_proc_reg_1 */
#define LTM_M_NEOTF_MODE_OFFSET                                       0x04
#define LTM_M_NEOTF_MODE_SHIFT                                        0
#define LTM_M_NEOTF_MODE_LENGTH                                       3

/* REGISTER saturn_post_pre_proc_reg_2 */
#define LTM_M_NOETF_MODE_OFFSET                                       0x08
#define LTM_M_NOETF_MODE_SHIFT                                        0
#define LTM_M_NOETF_MODE_LENGTH                                       3
#define LTM_M_NOETF_MAX_SHIFT                                         16
#define LTM_M_NOETF_MAX_LENGTH                                        12

/* REGISTER saturn_post_pre_proc_reg_3 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE0_OFFSET                        0x0c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE0_SHIFT                         0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE0_LENGTH                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE1_SHIFT                         16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE1_LENGTH                        16

/* REGISTER saturn_post_pre_proc_reg_4 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE2_OFFSET                        0x10
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE2_SHIFT                         0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE2_LENGTH                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE3_SHIFT                         16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE3_LENGTH                        16

/* REGISTER saturn_post_pre_proc_reg_5 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE4_OFFSET                        0x14
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE4_SHIFT                         0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE4_LENGTH                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE5_SHIFT                         16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE5_LENGTH                        16

/* REGISTER saturn_post_pre_proc_reg_6 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE6_OFFSET                        0x18
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE6_SHIFT                         0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE6_LENGTH                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE7_SHIFT                         16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE7_LENGTH                        16

/* REGISTER saturn_post_pre_proc_reg_7 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE8_OFFSET                        0x1c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE8_SHIFT                         0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE8_LENGTH                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE9_SHIFT                         16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE9_LENGTH                        16

/* REGISTER saturn_post_pre_proc_reg_8 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE10_OFFSET                       0x20
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE10_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE10_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE11_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE11_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_9 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE12_OFFSET                       0x24
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE12_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE12_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE13_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE13_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_10 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE14_OFFSET                       0x28
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE14_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE14_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE15_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE15_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_11 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE16_OFFSET                       0x2c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE16_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE16_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE17_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE17_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_12 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE18_OFFSET                       0x30
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE18_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE18_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE19_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE19_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_13 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE20_OFFSET                       0x34
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE20_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE20_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE21_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE21_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_14 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE22_OFFSET                       0x38
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE22_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE22_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE23_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE23_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_15 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE24_OFFSET                       0x3c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE24_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE24_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE25_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE25_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_16 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE26_OFFSET                       0x40
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE26_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE26_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE27_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE27_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_17 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE28_OFFSET                       0x44
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE28_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE28_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE29_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE29_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_18 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE30_OFFSET                       0x48
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE30_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE30_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE31_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE31_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_19 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE32_OFFSET                       0x4c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE32_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE32_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE33_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE33_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_20 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE34_OFFSET                       0x50
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE34_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE34_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE35_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE35_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_21 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE36_OFFSET                       0x54
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE36_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE36_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE37_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE37_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_22 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE38_OFFSET                       0x58
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE38_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE38_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE39_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE39_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_23 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE40_OFFSET                       0x5c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE40_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE40_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE41_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE41_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_24 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE42_OFFSET                       0x60
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE42_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE42_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE43_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE43_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_25 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE44_OFFSET                       0x64
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE44_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE44_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE45_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE45_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_26 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE46_OFFSET                       0x68
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE46_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE46_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE47_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE47_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_27 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE48_OFFSET                       0x6c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE48_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE48_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE49_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE49_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_28 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE50_OFFSET                       0x70
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE50_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE50_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE51_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE51_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_29 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE52_OFFSET                       0x74
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE52_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE52_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE53_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE53_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_30 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE54_OFFSET                       0x78
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE54_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE54_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE55_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE55_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_31 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE56_OFFSET                       0x7c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE56_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE56_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE57_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE57_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_32 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE58_OFFSET                       0x80
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE58_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE58_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE59_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE59_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_33 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE60_OFFSET                       0x84
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE60_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE60_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE61_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE61_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_34 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE62_OFFSET                       0x88
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE62_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE62_LENGTH                       16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE63_SHIFT                        16
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE63_LENGTH                       16

/* REGISTER saturn_post_pre_proc_reg_35 */
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE64_OFFSET                       0x8c
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE64_SHIFT                        0
#define LTM_M_PFRONT_TMOOTF_GAIN_TABLE64_LENGTH                       16
#define LTM_M_NFRONT_TMOOTF_SHIFT_BITS_SHIFT                          16
#define LTM_M_NFRONT_TMOOTF_SHIFT_BITS_LENGTH                         5
#define LTM_M_NFRONT_TMOOTF_RGB_MODE_SHIFT                            21
#define LTM_M_NFRONT_TMOOTF_RGB_MODE_LENGTH                           2

/* REGISTER saturn_post_pre_proc_reg_36 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE0_OFFSET                          0x90
#define LTM_M_PEND_TMOOTF_GAIN_TABLE0_SHIFT                           0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE0_LENGTH                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE1_SHIFT                           16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE1_LENGTH                          16

/* REGISTER saturn_post_pre_proc_reg_37 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE2_OFFSET                          0x94
#define LTM_M_PEND_TMOOTF_GAIN_TABLE2_SHIFT                           0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE2_LENGTH                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE3_SHIFT                           16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE3_LENGTH                          16

/* REGISTER saturn_post_pre_proc_reg_38 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE4_OFFSET                          0x98
#define LTM_M_PEND_TMOOTF_GAIN_TABLE4_SHIFT                           0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE4_LENGTH                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE5_SHIFT                           16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE5_LENGTH                          16

/* REGISTER saturn_post_pre_proc_reg_39 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE6_OFFSET                          0x9c
#define LTM_M_PEND_TMOOTF_GAIN_TABLE6_SHIFT                           0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE6_LENGTH                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE7_SHIFT                           16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE7_LENGTH                          16

/* REGISTER saturn_post_pre_proc_reg_40 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE8_OFFSET                          0xa0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE8_SHIFT                           0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE8_LENGTH                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE9_SHIFT                           16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE9_LENGTH                          16

/* REGISTER saturn_post_pre_proc_reg_41 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE10_OFFSET                         0xa4
#define LTM_M_PEND_TMOOTF_GAIN_TABLE10_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE10_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE11_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE11_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_42 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE12_OFFSET                         0xa8
#define LTM_M_PEND_TMOOTF_GAIN_TABLE12_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE12_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE13_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE13_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_43 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE14_OFFSET                         0xac
#define LTM_M_PEND_TMOOTF_GAIN_TABLE14_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE14_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE15_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE15_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_44 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE16_OFFSET                         0xb0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE16_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE16_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE17_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE17_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_45 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE18_OFFSET                         0xb4
#define LTM_M_PEND_TMOOTF_GAIN_TABLE18_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE18_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE19_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE19_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_46 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE20_OFFSET                         0xb8
#define LTM_M_PEND_TMOOTF_GAIN_TABLE20_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE20_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE21_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE21_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_47 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE22_OFFSET                         0xbc
#define LTM_M_PEND_TMOOTF_GAIN_TABLE22_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE22_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE23_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE23_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_48 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE24_OFFSET                         0xc0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE24_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE24_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE25_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE25_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_49 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE26_OFFSET                         0xc4
#define LTM_M_PEND_TMOOTF_GAIN_TABLE26_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE26_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE27_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE27_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_50 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE28_OFFSET                         0xc8
#define LTM_M_PEND_TMOOTF_GAIN_TABLE28_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE28_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE29_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE29_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_51 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE30_OFFSET                         0xcc
#define LTM_M_PEND_TMOOTF_GAIN_TABLE30_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE30_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE31_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE31_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_52 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE32_OFFSET                         0xd0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE32_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE32_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE33_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE33_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_53 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE34_OFFSET                         0xd4
#define LTM_M_PEND_TMOOTF_GAIN_TABLE34_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE34_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE35_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE35_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_54 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE36_OFFSET                         0xd8
#define LTM_M_PEND_TMOOTF_GAIN_TABLE36_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE36_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE37_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE37_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_55 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE38_OFFSET                         0xdc
#define LTM_M_PEND_TMOOTF_GAIN_TABLE38_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE38_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE39_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE39_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_56 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE40_OFFSET                         0xe0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE40_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE40_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE41_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE41_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_57 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE42_OFFSET                         0xe4
#define LTM_M_PEND_TMOOTF_GAIN_TABLE42_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE42_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE43_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE43_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_58 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE44_OFFSET                         0xe8
#define LTM_M_PEND_TMOOTF_GAIN_TABLE44_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE44_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE45_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE45_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_59 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE46_OFFSET                         0xec
#define LTM_M_PEND_TMOOTF_GAIN_TABLE46_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE46_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE47_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE47_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_60 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE48_OFFSET                         0xf0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE48_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE48_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE49_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE49_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_61 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE50_OFFSET                         0xf4
#define LTM_M_PEND_TMOOTF_GAIN_TABLE50_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE50_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE51_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE51_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_62 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE52_OFFSET                         0xf8
#define LTM_M_PEND_TMOOTF_GAIN_TABLE52_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE52_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE53_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE53_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_63 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE54_OFFSET                         0xfc
#define LTM_M_PEND_TMOOTF_GAIN_TABLE54_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE54_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE55_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE55_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_64 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE56_OFFSET                         0x100
#define LTM_M_PEND_TMOOTF_GAIN_TABLE56_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE56_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE57_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE57_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_65 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE58_OFFSET                         0x104
#define LTM_M_PEND_TMOOTF_GAIN_TABLE58_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE58_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE59_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE59_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_66 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE60_OFFSET                         0x108
#define LTM_M_PEND_TMOOTF_GAIN_TABLE60_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE60_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE61_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE61_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_67 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE62_OFFSET                         0x10c
#define LTM_M_PEND_TMOOTF_GAIN_TABLE62_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE62_LENGTH                         16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE63_SHIFT                          16
#define LTM_M_PEND_TMOOTF_GAIN_TABLE63_LENGTH                         16

/* REGISTER saturn_post_pre_proc_reg_68 */
#define LTM_M_PEND_TMOOTF_GAIN_TABLE64_OFFSET                         0x110
#define LTM_M_PEND_TMOOTF_GAIN_TABLE64_SHIFT                          0
#define LTM_M_PEND_TMOOTF_GAIN_TABLE64_LENGTH                         16
#define LTM_M_NEND_TMOOTF_SHIFT_BITS_SHIFT                            16
#define LTM_M_NEND_TMOOTF_SHIFT_BITS_LENGTH                           5
#define LTM_M_NEND_TMOOTF_RGB_MODE_SHIFT                              21
#define LTM_M_NEND_TMOOTF_RGB_MODE_LENGTH                             2

/* REGISTER saturn_post_pre_proc_reg_69 */
#define M_PMATRIX_TABLE0_OFFSET                                       0x114
#define LTM_M_PMATRIX_TABLE0_SHIFT                                    0
#define LTM_M_PMATRIX_TABLE0_LENGTH                                   16
#define LTM_M_PMATRIX_TABLE1_SHIFT                                    16
#define LTM_M_PMATRIX_TABLE1_LENGTH                                   16

/* REGISTER saturn_post_pre_proc_reg_70 */
#define M_PMATRIX_TABLE2_OFFSET                                       0x118
#define LTM_M_PMATRIX_TABLE2_SHIFT                                    0
#define LTM_M_PMATRIX_TABLE2_LENGTH                                   16
#define LTM_M_PMATRIX_TABLE3_SHIFT                                    16
#define LTM_M_PMATRIX_TABLE3_LENGTH                                   16

/* REGISTER saturn_post_pre_proc_reg_71 */
#define M_PMATRIX_TABLE4_OFFSET                                       0x11c
#define LTM_M_PMATRIX_TABLE4_SHIFT                                    0
#define LTM_M_PMATRIX_TABLE4_LENGTH                                   16
#define LTM_M_PMATRIX_TABLE5_SHIFT                                    16
#define LTM_M_PMATRIX_TABLE5_LENGTH                                   16

/* REGISTER saturn_post_pre_proc_reg_72 */
#define M_PMATRIX_TABLE6_OFFSET                                       0x120
#define LTM_M_PMATRIX_TABLE6_SHIFT                                    0
#define LTM_M_PMATRIX_TABLE6_LENGTH                                   16
#define LTM_M_PMATRIX_TABLE7_SHIFT                                    16
#define LTM_M_PMATRIX_TABLE7_LENGTH                                   16

/* REGISTER saturn_post_pre_proc_reg_73 */
#define M_PMATRIX_TABLE8_OFFSET                                       0x124
#define LTM_M_PMATRIX_TABLE8_SHIFT                                    0
#define LTM_M_PMATRIX_TABLE8_LENGTH                                   16

/* REGISTER saturn_post_pre_proc_reg_74 */
#define LTM_M_PMATRIX_OFFSET0_OFFSET                                  0x128
#define LTM_M_PMATRIX_OFFSET0_SHIFT                                   0
#define LTM_M_PMATRIX_OFFSET0_LENGTH                                  25

/* REGISTER saturn_post_pre_proc_reg_75 */
#define LTM_M_PMATRIX_OFFSET1_OFFSET                                  0x12c
#define LTM_M_PMATRIX_OFFSET1_SHIFT                                   0
#define LTM_M_PMATRIX_OFFSET1_LENGTH                                  25

/* REGISTER saturn_post_pre_proc_reg_76 */
#define LTM_M_PMATRIX_OFFSET2_OFFSET                                  0x130
#define LTM_M_PMATRIX_OFFSET2_SHIFT                                   0
#define LTM_M_PMATRIX_OFFSET2_LENGTH                                  25

/* REGISTER saturn_post_pre_proc_reg_77 */
#define LTM_M_NGAIN_TO_FULL_OFFSET                                    0x134
#define LTM_M_NGAIN_TO_FULL_SHIFT                                     0
#define LTM_M_NGAIN_TO_FULL_LENGTH                                    16

/* REGISTER saturn_post_pre_proc_reg_78 */
#define M_PENDMATRIX_TABLE0_OFFSET                                    0x138
#define M_PENDMATRIX_TABLE0_SHIFT                                     0
#define M_PENDMATRIX_TABLE0_LENGTH                                    16
#define M_PENDMATRIX_TABLE1_SHIFT                                     16
#define M_PENDMATRIX_TABLE1_LENGTH                                    16

/* REGISTER saturn_post_pre_proc_reg_79 */
#define M_PENDMATRIX_TABLE2_OFFSET                                    0x13c
#define M_PENDMATRIX_TABLE2_SHIFT                                     0
#define M_PENDMATRIX_TABLE2_LENGTH                                    16
#define M_PENDMATRIX_TABLE3_SHIFT                                     16
#define M_PENDMATRIX_TABLE3_LENGTH                                    16

/* REGISTER saturn_post_pre_proc_reg_80 */
#define M_PENDMATRIX_TABLE4_OFFSET                                    0x140
#define M_PENDMATRIX_TABLE4_SHIFT                                     0
#define M_PENDMATRIX_TABLE4_LENGTH                                    16
#define M_PENDMATRIX_TABLE5_SHIFT                                     16
#define M_PENDMATRIX_TABLE5_LENGTH                                    16

/* REGISTER saturn_post_pre_proc_reg_81 */
#define M_PENDMATRIX_TABLE6_OFFSET                                    0x144
#define M_PENDMATRIX_TABLE6_SHIFT                                     0
#define M_PENDMATRIX_TABLE6_LENGTH                                    16
#define M_PENDMATRIX_TABLE7_SHIFT                                     16
#define M_PENDMATRIX_TABLE7_LENGTH                                    16

/* REGISTER saturn_post_pre_proc_reg_82 */
#define M_PENDMATRIX_TABLE8_OFFSET                                    0x148
#define M_PENDMATRIX_TABLE8_SHIFT                                     0
#define M_PENDMATRIX_TABLE8_LENGTH                                    16

/* REGISTER saturn_post_pre_proc_reg_83 */
#define M_PENDMATRIX_OFFSET0_OFFSET                                   0x14c
#define M_PENDMATRIX_OFFSET0_SHIFT                                    0
#define M_PENDMATRIX_OFFSET0_LENGTH                                   13

/* REGISTER saturn_post_pre_proc_reg_84 */
#define M_PENDMATRIX_OFFSET1_OFFSET                                   0x150
#define M_PENDMATRIX_OFFSET1_SHIFT                                    0
#define M_PENDMATRIX_OFFSET1_LENGTH                                   13

/* REGISTER saturn_post_pre_proc_reg_85 */
#define M_PENDMATRIX_OFFSET2_OFFSET                                   0x154
#define M_PENDMATRIX_OFFSET2_SHIFT                                    0
#define M_PENDMATRIX_OFFSET2_LENGTH                                   13

/* REGISTER saturn_post_pre_proc_reg_86 */
#define LTM_LUT4_GROUP_0_OFFSET                                       0x158
#define LTM_LUT4_GROUP_0_SHIFT                                        0
#define LTM_LUT4_GROUP_0_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_87 */
#define LTM_LUT4_GROUP_1_OFFSET                                       0x15c
#define LTM_LUT4_GROUP_1_SHIFT                                        0
#define LTM_LUT4_GROUP_1_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_88 */
#define LTM_LUT4_GROUP_2_OFFSET                                       0x160
#define LTM_LUT4_GROUP_2_SHIFT                                        0
#define LTM_LUT4_GROUP_2_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_89 */
#define LTM_LUT4_GROUP_3_OFFSET                                       0x164
#define LTM_LUT4_GROUP_3_SHIFT                                        0
#define LTM_LUT4_GROUP_3_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_90 */
#define LTM_LUT4_GROUP_4_OFFSET                                       0x168
#define LTM_LUT4_GROUP_4_SHIFT                                        0
#define LTM_LUT4_GROUP_4_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_91 */
#define LTM_LUT4_GROUP_5_OFFSET                                       0x16c
#define LTM_LUT4_GROUP_5_SHIFT                                        0
#define LTM_LUT4_GROUP_5_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_92 */
#define LTM_LUT4_GROUP_6_OFFSET                                       0x170
#define LTM_LUT4_GROUP_6_SHIFT                                        0
#define LTM_LUT4_GROUP_6_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_93 */
#define LTM_LUT4_GROUP_7_OFFSET                                       0x174
#define LTM_LUT4_GROUP_7_SHIFT                                        0
#define LTM_LUT4_GROUP_7_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_94 */
#define LTM_LUT4_GROUP_8_OFFSET                                       0x178
#define LTM_LUT4_GROUP_8_SHIFT                                        0
#define LTM_LUT4_GROUP_8_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_95 */
#define LTM_LUT4_GROUP_9_OFFSET                                       0x17c
#define LTM_LUT4_GROUP_9_SHIFT                                        0
#define LTM_LUT4_GROUP_9_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_96 */
#define LTM_LUT4_GROUP_10_OFFSET                                      0x180
#define LTM_LUT4_GROUP_10_SHIFT                                       0
#define LTM_LUT4_GROUP_10_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_97 */
#define LTM_LUT4_GROUP_11_OFFSET                                      0x184
#define LTM_LUT4_GROUP_11_SHIFT                                       0
#define LTM_LUT4_GROUP_11_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_98 */
#define LTM_LUT4_GROUP_12_OFFSET                                      0x188
#define LTM_LUT4_GROUP_12_SHIFT                                       0
#define LTM_LUT4_GROUP_12_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_99 */
#define LTM_LUT4_GROUP_13_OFFSET                                      0x18c
#define LTM_LUT4_GROUP_13_SHIFT                                       0
#define LTM_LUT4_GROUP_13_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_100 */
#define LTM_LUT4_GROUP_14_OFFSET                                      0x190
#define LTM_LUT4_GROUP_14_SHIFT                                       0
#define LTM_LUT4_GROUP_14_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_101 */
#define LTM_LUT4_GROUP_15_OFFSET                                      0x194
#define LTM_LUT4_GROUP_15_SHIFT                                       0
#define LTM_LUT4_GROUP_15_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_102 */
#define LTM_LUT4_GROUP_16_OFFSET                                      0x198
#define LTM_LUT4_GROUP_16_SHIFT                                       0
#define LTM_LUT4_GROUP_16_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_103 */
#define LTM_LUT4_GROUP_17_OFFSET                                      0x19c
#define LTM_LUT4_GROUP_17_SHIFT                                       0
#define LTM_LUT4_GROUP_17_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_104 */
#define LTM_LUT4_GROUP_18_OFFSET                                      0x1a0
#define LTM_LUT4_GROUP_18_SHIFT                                       0
#define LTM_LUT4_GROUP_18_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_105 */
#define LTM_LUT4_GROUP_19_OFFSET                                      0x1a4
#define LTM_LUT4_GROUP_19_SHIFT                                       0
#define LTM_LUT4_GROUP_19_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_106 */
#define LTM_LUT4_GROUP_20_OFFSET                                      0x1a8
#define LTM_LUT4_GROUP_20_SHIFT                                       0
#define LTM_LUT4_GROUP_20_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_107 */
#define LTM_LUT4_GROUP_21_OFFSET                                      0x1ac
#define LTM_LUT4_GROUP_21_SHIFT                                       0
#define LTM_LUT4_GROUP_21_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_108 */
#define LTM_LUT4_GROUP_22_OFFSET                                      0x1b0
#define LTM_LUT4_GROUP_22_SHIFT                                       0
#define LTM_LUT4_GROUP_22_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_109 */
#define LTM_LUT4_GROUP_23_OFFSET                                      0x1b4
#define LTM_LUT4_GROUP_23_SHIFT                                       0
#define LTM_LUT4_GROUP_23_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_110 */
#define LTM_LUT4_GROUP_24_OFFSET                                      0x1b8
#define LTM_LUT4_GROUP_24_SHIFT                                       0
#define LTM_LUT4_GROUP_24_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_111 */
#define LTM_LUT4_GROUP_25_OFFSET                                      0x1bc
#define LTM_LUT4_GROUP_25_SHIFT                                       0
#define LTM_LUT4_GROUP_25_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_112 */
#define LTM_LUT4_GROUP_26_OFFSET                                      0x1c0
#define LTM_LUT4_GROUP_26_SHIFT                                       0
#define LTM_LUT4_GROUP_26_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_113 */
#define LTM_LUT4_GROUP_27_OFFSET                                      0x1c4
#define LTM_LUT4_GROUP_27_SHIFT                                       0
#define LTM_LUT4_GROUP_27_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_114 */
#define LTM_LUT4_GROUP_28_OFFSET                                      0x1c8
#define LTM_LUT4_GROUP_28_SHIFT                                       0
#define LTM_LUT4_GROUP_28_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_115 */
#define LTM_LUT4_GROUP_29_OFFSET                                      0x1cc
#define LTM_LUT4_GROUP_29_SHIFT                                       0
#define LTM_LUT4_GROUP_29_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_116 */
#define LTM_LUT4_GROUP_30_OFFSET                                      0x1d0
#define LTM_LUT4_GROUP_30_SHIFT                                       0
#define LTM_LUT4_GROUP_30_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_117 */
#define LTM_LUT4_GROUP_31_OFFSET                                      0x1d4
#define LTM_LUT4_GROUP_31_SHIFT                                       0
#define LTM_LUT4_GROUP_31_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_118 */
#define LTM_LUT4_GROUP_32_OFFSET                                      0x1d8
#define LTM_LUT4_GROUP_32_SHIFT                                       0
#define LTM_LUT4_GROUP_32_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_119 */
#define LTM_LUT4_GROUP_33_OFFSET                                      0x1dc
#define LTM_LUT4_GROUP_33_SHIFT                                       0
#define LTM_LUT4_GROUP_33_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_120 */
#define LTM_LUT4_GROUP_34_OFFSET                                      0x1e0
#define LTM_LUT4_GROUP_34_SHIFT                                       0
#define LTM_LUT4_GROUP_34_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_121 */
#define LTM_LUT4_GROUP_35_OFFSET                                      0x1e4
#define LTM_LUT4_GROUP_35_SHIFT                                       0
#define LTM_LUT4_GROUP_35_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_122 */
#define LTM_LUT4_GROUP_36_OFFSET                                      0x1e8
#define LTM_LUT4_GROUP_36_SHIFT                                       0
#define LTM_LUT4_GROUP_36_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_123 */
#define LTM_LUT4_GROUP_37_OFFSET                                      0x1ec
#define LTM_LUT4_GROUP_37_SHIFT                                       0
#define LTM_LUT4_GROUP_37_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_124 */
#define LTM_LUT4_GROUP_38_OFFSET                                      0x1f0
#define LTM_LUT4_GROUP_38_SHIFT                                       0
#define LTM_LUT4_GROUP_38_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_125 */
#define LTM_LUT4_GROUP_39_OFFSET                                      0x1f4
#define LTM_LUT4_GROUP_39_SHIFT                                       0
#define LTM_LUT4_GROUP_39_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_126 */
#define LTM_LUT4_GROUP_40_OFFSET                                      0x1f8
#define LTM_LUT4_GROUP_40_SHIFT                                       0
#define LTM_LUT4_GROUP_40_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_127 */
#define LTM_LUT4_GROUP_41_OFFSET                                      0x1fc
#define LTM_LUT4_GROUP_41_SHIFT                                       0
#define LTM_LUT4_GROUP_41_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_128 */
#define LTM_LUT4_GROUP_42_OFFSET                                      0x200
#define LTM_LUT4_GROUP_42_SHIFT                                       0
#define LTM_LUT4_GROUP_42_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_129 */
#define LTM_LUT4_GROUP_43_OFFSET                                      0x204
#define LTM_LUT4_GROUP_43_SHIFT                                       0
#define LTM_LUT4_GROUP_43_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_130 */
#define LTM_LUT4_GROUP_44_OFFSET                                      0x208
#define LTM_LUT4_GROUP_44_SHIFT                                       0
#define LTM_LUT4_GROUP_44_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_131 */
#define LTM_LUT4_GROUP_45_OFFSET                                      0x20c
#define LTM_LUT4_GROUP_45_SHIFT                                       0
#define LTM_LUT4_GROUP_45_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_132 */
#define LTM_LUT4_GROUP_46_OFFSET                                      0x210
#define LTM_LUT4_GROUP_46_SHIFT                                       0
#define LTM_LUT4_GROUP_46_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_133 */
#define LTM_LUT4_GROUP_47_OFFSET                                      0x214
#define LTM_LUT4_GROUP_47_SHIFT                                       0
#define LTM_LUT4_GROUP_47_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_134 */
#define LTM_LUT4_GROUP_48_OFFSET                                      0x218
#define LTM_LUT4_GROUP_48_SHIFT                                       0
#define LTM_LUT4_GROUP_48_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_135 */
#define LTM_LUT4_GROUP_49_OFFSET                                      0x21c
#define LTM_LUT4_GROUP_49_SHIFT                                       0
#define LTM_LUT4_GROUP_49_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_136 */
#define LTM_LUT4_GROUP_50_OFFSET                                      0x220
#define LTM_LUT4_GROUP_50_SHIFT                                       0
#define LTM_LUT4_GROUP_50_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_137 */
#define LTM_LUT4_GROUP_51_OFFSET                                      0x224
#define LTM_LUT4_GROUP_51_SHIFT                                       0
#define LTM_LUT4_GROUP_51_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_138 */
#define LTM_LUT4_GROUP_52_OFFSET                                      0x228
#define LTM_LUT4_GROUP_52_SHIFT                                       0
#define LTM_LUT4_GROUP_52_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_139 */
#define LTM_LUT4_GROUP_53_OFFSET                                      0x22c
#define LTM_LUT4_GROUP_53_SHIFT                                       0
#define LTM_LUT4_GROUP_53_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_140 */
#define LTM_LUT4_GROUP_54_OFFSET                                      0x230
#define LTM_LUT4_GROUP_54_SHIFT                                       0
#define LTM_LUT4_GROUP_54_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_141 */
#define LTM_LUT4_GROUP_55_OFFSET                                      0x234
#define LTM_LUT4_GROUP_55_SHIFT                                       0
#define LTM_LUT4_GROUP_55_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_142 */
#define LTM_LUT4_GROUP_56_OFFSET                                      0x238
#define LTM_LUT4_GROUP_56_SHIFT                                       0
#define LTM_LUT4_GROUP_56_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_143 */
#define LTM_LUT4_GROUP_57_OFFSET                                      0x23c
#define LTM_LUT4_GROUP_57_SHIFT                                       0
#define LTM_LUT4_GROUP_57_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_144 */
#define LTM_LUT4_GROUP_58_OFFSET                                      0x240
#define LTM_LUT4_GROUP_58_SHIFT                                       0
#define LTM_LUT4_GROUP_58_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_145 */
#define LTM_LUT4_GROUP_59_OFFSET                                      0x244
#define LTM_LUT4_GROUP_59_SHIFT                                       0
#define LTM_LUT4_GROUP_59_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_146 */
#define LTM_LUT4_GROUP_60_OFFSET                                      0x248
#define LTM_LUT4_GROUP_60_SHIFT                                       0
#define LTM_LUT4_GROUP_60_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_147 */
#define LTM_LUT4_GROUP_61_OFFSET                                      0x24c
#define LTM_LUT4_GROUP_61_SHIFT                                       0
#define LTM_LUT4_GROUP_61_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_148 */
#define LTM_LUT4_GROUP_62_OFFSET                                      0x250
#define LTM_LUT4_GROUP_62_SHIFT                                       0
#define LTM_LUT4_GROUP_62_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_149 */
#define LTM_LUT4_GROUP_63_OFFSET                                      0x254
#define LTM_LUT4_GROUP_63_SHIFT                                       0
#define LTM_LUT4_GROUP_63_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_150 */
#define LTM_LUT4_GROUP_64_OFFSET                                      0x258
#define LTM_LUT4_GROUP_64_SHIFT                                       0
#define LTM_LUT4_GROUP_64_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_151 */
#define LTM_LUT4_GROUP_65_OFFSET                                      0x25c
#define LTM_LUT4_GROUP_65_SHIFT                                       0
#define LTM_LUT4_GROUP_65_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_152 */
#define LTM_LUT4_GROUP_66_OFFSET                                      0x260
#define LTM_LUT4_GROUP_66_SHIFT                                       0
#define LTM_LUT4_GROUP_66_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_153 */
#define LTM_LUT4_GROUP_67_OFFSET                                      0x264
#define LTM_LUT4_GROUP_67_SHIFT                                       0
#define LTM_LUT4_GROUP_67_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_154 */
#define LTM_LUT4_GROUP_68_OFFSET                                      0x268
#define LTM_LUT4_GROUP_68_SHIFT                                       0
#define LTM_LUT4_GROUP_68_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_155 */
#define LTM_LUT4_GROUP_69_OFFSET                                      0x26c
#define LTM_LUT4_GROUP_69_SHIFT                                       0
#define LTM_LUT4_GROUP_69_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_156 */
#define LTM_LUT4_GROUP_70_OFFSET                                      0x270
#define LTM_LUT4_GROUP_70_SHIFT                                       0
#define LTM_LUT4_GROUP_70_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_157 */
#define LTM_LUT5_GROUP_0_OFFSET                                       0x274
#define LTM_LUT5_GROUP_0_SHIFT                                        0
#define LTM_LUT5_GROUP_0_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_158 */
#define LTM_LUT5_GROUP_1_OFFSET                                       0x278
#define LTM_LUT5_GROUP_1_SHIFT                                        0
#define LTM_LUT5_GROUP_1_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_159 */
#define LTM_LUT5_GROUP_2_OFFSET                                       0x27c
#define LTM_LUT5_GROUP_2_SHIFT                                        0
#define LTM_LUT5_GROUP_2_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_160 */
#define LTM_LUT5_GROUP_3_OFFSET                                       0x280
#define LTM_LUT5_GROUP_3_SHIFT                                        0
#define LTM_LUT5_GROUP_3_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_161 */
#define LTM_LUT5_GROUP_4_OFFSET                                       0x284
#define LTM_LUT5_GROUP_4_SHIFT                                        0
#define LTM_LUT5_GROUP_4_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_162 */
#define LTM_LUT5_GROUP_5_OFFSET                                       0x288
#define LTM_LUT5_GROUP_5_SHIFT                                        0
#define LTM_LUT5_GROUP_5_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_163 */
#define LTM_LUT5_GROUP_6_OFFSET                                       0x28c
#define LTM_LUT5_GROUP_6_SHIFT                                        0
#define LTM_LUT5_GROUP_6_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_164 */
#define LTM_LUT5_GROUP_7_OFFSET                                       0x290
#define LTM_LUT5_GROUP_7_SHIFT                                        0
#define LTM_LUT5_GROUP_7_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_165 */
#define LTM_LUT5_GROUP_8_OFFSET                                       0x294
#define LTM_LUT5_GROUP_8_SHIFT                                        0
#define LTM_LUT5_GROUP_8_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_166 */
#define LTM_LUT5_GROUP_9_OFFSET                                       0x298
#define LTM_LUT5_GROUP_9_SHIFT                                        0
#define LTM_LUT5_GROUP_9_LENGTH                                       24

/* REGISTER saturn_post_pre_proc_reg_167 */
#define LTM_LUT5_GROUP_10_OFFSET                                      0x29c
#define LTM_LUT5_GROUP_10_SHIFT                                       0
#define LTM_LUT5_GROUP_10_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_168 */
#define LTM_LUT5_GROUP_11_OFFSET                                      0x2a0
#define LTM_LUT5_GROUP_11_SHIFT                                       0
#define LTM_LUT5_GROUP_11_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_169 */
#define LTM_LUT5_GROUP_12_OFFSET                                      0x2a4
#define LTM_LUT5_GROUP_12_SHIFT                                       0
#define LTM_LUT5_GROUP_12_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_170 */
#define LTM_LUT5_GROUP_13_OFFSET                                      0x2a8
#define LTM_LUT5_GROUP_13_SHIFT                                       0
#define LTM_LUT5_GROUP_13_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_171 */
#define LTM_LUT5_GROUP_14_OFFSET                                      0x2ac
#define LTM_LUT5_GROUP_14_SHIFT                                       0
#define LTM_LUT5_GROUP_14_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_172 */
#define LTM_LUT5_GROUP_15_OFFSET                                      0x2b0
#define LTM_LUT5_GROUP_15_SHIFT                                       0
#define LTM_LUT5_GROUP_15_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_173 */
#define LTM_LUT5_GROUP_16_OFFSET                                      0x2b4
#define LTM_LUT5_GROUP_16_SHIFT                                       0
#define LTM_LUT5_GROUP_16_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_174 */
#define LTM_LUT5_GROUP_17_OFFSET                                      0x2b8
#define LTM_LUT5_GROUP_17_SHIFT                                       0
#define LTM_LUT5_GROUP_17_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_175 */
#define LTM_LUT5_GROUP_18_OFFSET                                      0x2bc
#define LTM_LUT5_GROUP_18_SHIFT                                       0
#define LTM_LUT5_GROUP_18_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_176 */
#define LTM_LUT5_GROUP_19_OFFSET                                      0x2c0
#define LTM_LUT5_GROUP_19_SHIFT                                       0
#define LTM_LUT5_GROUP_19_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_177 */
#define LTM_LUT5_GROUP_20_OFFSET                                      0x2c4
#define LTM_LUT5_GROUP_20_SHIFT                                       0
#define LTM_LUT5_GROUP_20_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_178 */
#define LTM_LUT5_GROUP_21_OFFSET                                      0x2c8
#define LTM_LUT5_GROUP_21_SHIFT                                       0
#define LTM_LUT5_GROUP_21_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_179 */
#define LTM_LUT5_GROUP_22_OFFSET                                      0x2cc
#define LTM_LUT5_GROUP_22_SHIFT                                       0
#define LTM_LUT5_GROUP_22_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_180 */
#define LTM_LUT5_GROUP_23_OFFSET                                      0x2d0
#define LTM_LUT5_GROUP_23_SHIFT                                       0
#define LTM_LUT5_GROUP_23_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_181 */
#define LTM_LUT5_GROUP_24_OFFSET                                      0x2d4
#define LTM_LUT5_GROUP_24_SHIFT                                       0
#define LTM_LUT5_GROUP_24_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_182 */
#define LTM_LUT5_GROUP_25_OFFSET                                      0x2d8
#define LTM_LUT5_GROUP_25_SHIFT                                       0
#define LTM_LUT5_GROUP_25_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_183 */
#define LTM_LUT5_GROUP_26_OFFSET                                      0x2dc
#define LTM_LUT5_GROUP_26_SHIFT                                       0
#define LTM_LUT5_GROUP_26_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_184 */
#define LTM_LUT5_GROUP_27_OFFSET                                      0x2e0
#define LTM_LUT5_GROUP_27_SHIFT                                       0
#define LTM_LUT5_GROUP_27_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_185 */
#define LTM_LUT5_GROUP_28_OFFSET                                      0x2e4
#define LTM_LUT5_GROUP_28_SHIFT                                       0
#define LTM_LUT5_GROUP_28_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_186 */
#define LTM_LUT5_GROUP_29_OFFSET                                      0x2e8
#define LTM_LUT5_GROUP_29_SHIFT                                       0
#define LTM_LUT5_GROUP_29_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_187 */
#define LTM_LUT5_GROUP_30_OFFSET                                      0x2ec
#define LTM_LUT5_GROUP_30_SHIFT                                       0
#define LTM_LUT5_GROUP_30_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_188 */
#define LTM_LUT5_GROUP_31_OFFSET                                      0x2f0
#define LTM_LUT5_GROUP_31_SHIFT                                       0
#define LTM_LUT5_GROUP_31_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_189 */
#define LTM_LUT5_GROUP_32_OFFSET                                      0x2f4
#define LTM_LUT5_GROUP_32_SHIFT                                       0
#define LTM_LUT5_GROUP_32_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_190 */
#define LTM_LUT5_GROUP_33_OFFSET                                      0x2f8
#define LTM_LUT5_GROUP_33_SHIFT                                       0
#define LTM_LUT5_GROUP_33_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_191 */
#define LTM_LUT5_GROUP_34_OFFSET                                      0x2fc
#define LTM_LUT5_GROUP_34_SHIFT                                       0
#define LTM_LUT5_GROUP_34_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_192 */
#define LTM_LUT5_GROUP_35_OFFSET                                      0x300
#define LTM_LUT5_GROUP_35_SHIFT                                       0
#define LTM_LUT5_GROUP_35_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_193 */
#define LTM_LUT5_GROUP_36_OFFSET                                      0x304
#define LTM_LUT5_GROUP_36_SHIFT                                       0
#define LTM_LUT5_GROUP_36_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_194 */
#define LTM_LUT5_GROUP_37_OFFSET                                      0x308
#define LTM_LUT5_GROUP_37_SHIFT                                       0
#define LTM_LUT5_GROUP_37_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_195 */
#define LTM_LUT5_GROUP_38_OFFSET                                      0x30c
#define LTM_LUT5_GROUP_38_SHIFT                                       0
#define LTM_LUT5_GROUP_38_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_196 */
#define LTM_LUT5_GROUP_39_OFFSET                                      0x310
#define LTM_LUT5_GROUP_39_SHIFT                                       0
#define LTM_LUT5_GROUP_39_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_197 */
#define LTM_LUT5_GROUP_40_OFFSET                                      0x314
#define LTM_LUT5_GROUP_40_SHIFT                                       0
#define LTM_LUT5_GROUP_40_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_198 */
#define LTM_LUT5_GROUP_41_OFFSET                                      0x318
#define LTM_LUT5_GROUP_41_SHIFT                                       0
#define LTM_LUT5_GROUP_41_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_199 */
#define LTM_LUT5_GROUP_42_OFFSET                                      0x31c
#define LTM_LUT5_GROUP_42_SHIFT                                       0
#define LTM_LUT5_GROUP_42_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_200 */
#define LTM_LUT5_GROUP_43_OFFSET                                      0x320
#define LTM_LUT5_GROUP_43_SHIFT                                       0
#define LTM_LUT5_GROUP_43_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_201 */
#define LTM_LUT5_GROUP_44_OFFSET                                      0x324
#define LTM_LUT5_GROUP_44_SHIFT                                       0
#define LTM_LUT5_GROUP_44_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_202 */
#define LTM_LUT5_GROUP_45_OFFSET                                      0x328
#define LTM_LUT5_GROUP_45_SHIFT                                       0
#define LTM_LUT5_GROUP_45_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_203 */
#define LTM_LUT5_GROUP_46_OFFSET                                      0x32c
#define LTM_LUT5_GROUP_46_SHIFT                                       0
#define LTM_LUT5_GROUP_46_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_204 */
#define LTM_LUT5_GROUP_47_OFFSET                                      0x330
#define LTM_LUT5_GROUP_47_SHIFT                                       0
#define LTM_LUT5_GROUP_47_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_205 */
#define LTM_LUT5_GROUP_48_OFFSET                                      0x334
#define LTM_LUT5_GROUP_48_SHIFT                                       0
#define LTM_LUT5_GROUP_48_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_206 */
#define LTM_LUT5_GROUP_49_OFFSET                                      0x338
#define LTM_LUT5_GROUP_49_SHIFT                                       0
#define LTM_LUT5_GROUP_49_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_207 */
#define LTM_LUT5_GROUP_50_OFFSET                                      0x33c
#define LTM_LUT5_GROUP_50_SHIFT                                       0
#define LTM_LUT5_GROUP_50_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_208 */
#define LTM_LUT5_GROUP_51_OFFSET                                      0x340
#define LTM_LUT5_GROUP_51_SHIFT                                       0
#define LTM_LUT5_GROUP_51_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_209 */
#define LTM_LUT5_GROUP_52_OFFSET                                      0x344
#define LTM_LUT5_GROUP_52_SHIFT                                       0
#define LTM_LUT5_GROUP_52_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_210 */
#define LTM_LUT5_GROUP_53_OFFSET                                      0x348
#define LTM_LUT5_GROUP_53_SHIFT                                       0
#define LTM_LUT5_GROUP_53_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_211 */
#define LTM_LUT5_GROUP_54_OFFSET                                      0x34c
#define LTM_LUT5_GROUP_54_SHIFT                                       0
#define LTM_LUT5_GROUP_54_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_212 */
#define LTM_LUT5_GROUP_55_OFFSET                                      0x350
#define LTM_LUT5_GROUP_55_SHIFT                                       0
#define LTM_LUT5_GROUP_55_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_213 */
#define LTM_LUT5_GROUP_56_OFFSET                                      0x354
#define LTM_LUT5_GROUP_56_SHIFT                                       0
#define LTM_LUT5_GROUP_56_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_214 */
#define LTM_LUT5_GROUP_57_OFFSET                                      0x358
#define LTM_LUT5_GROUP_57_SHIFT                                       0
#define LTM_LUT5_GROUP_57_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_215 */
#define LTM_LUT5_GROUP_58_OFFSET                                      0x35c
#define LTM_LUT5_GROUP_58_SHIFT                                       0
#define LTM_LUT5_GROUP_58_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_216 */
#define LTM_LUT5_GROUP_59_OFFSET                                      0x360
#define LTM_LUT5_GROUP_59_SHIFT                                       0
#define LTM_LUT5_GROUP_59_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_217 */
#define LTM_LUT5_GROUP_60_OFFSET                                      0x364
#define LTM_LUT5_GROUP_60_SHIFT                                       0
#define LTM_LUT5_GROUP_60_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_218 */
#define LTM_LUT5_GROUP_61_OFFSET                                      0x368
#define LTM_LUT5_GROUP_61_SHIFT                                       0
#define LTM_LUT5_GROUP_61_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_219 */
#define LTM_LUT5_GROUP_62_OFFSET                                      0x36c
#define LTM_LUT5_GROUP_62_SHIFT                                       0
#define LTM_LUT5_GROUP_62_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_220 */
#define LTM_LUT5_GROUP_63_OFFSET                                      0x370
#define LTM_LUT5_GROUP_63_SHIFT                                       0
#define LTM_LUT5_GROUP_63_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_221 */
#define LTM_LUT5_GROUP_64_OFFSET                                      0x374
#define LTM_LUT5_GROUP_64_SHIFT                                       0
#define LTM_LUT5_GROUP_64_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_222 */
#define LTM_LUT5_GROUP_65_OFFSET                                      0x378
#define LTM_LUT5_GROUP_65_SHIFT                                       0
#define LTM_LUT5_GROUP_65_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_223 */
#define LTM_LUT5_GROUP_66_OFFSET                                      0x37c
#define LTM_LUT5_GROUP_66_SHIFT                                       0
#define LTM_LUT5_GROUP_66_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_224 */
#define LTM_LUT5_GROUP_67_OFFSET                                      0x380
#define LTM_LUT5_GROUP_67_SHIFT                                       0
#define LTM_LUT5_GROUP_67_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_225 */
#define LTM_LUT5_GROUP_68_OFFSET                                      0x384
#define LTM_LUT5_GROUP_68_SHIFT                                       0
#define LTM_LUT5_GROUP_68_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_226 */
#define LTM_LUT5_GROUP_69_OFFSET                                      0x388
#define LTM_LUT5_GROUP_69_SHIFT                                       0
#define LTM_LUT5_GROUP_69_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_227 */
#define LTM_LUT5_GROUP_70_OFFSET                                      0x38c
#define LTM_LUT5_GROUP_70_SHIFT                                       0
#define LTM_LUT5_GROUP_70_LENGTH                                      24

/* REGISTER saturn_post_pre_proc_reg_228 */
#define LTM_M_TMOOTF_WEIGHTYR_OFFSET                                  0x390
#define LTM_M_TMOOTF_WEIGHTYR_SHIFT                                   0
#define LTM_M_TMOOTF_WEIGHTYR_LENGTH                                  12
#define LTM_M_TMOOTF_WEIGHTYG_SHIFT                                   16
#define LTM_M_TMOOTF_WEIGHTYG_LENGTH                                  12

/* REGISTER saturn_post_pre_proc_reg_229 */
#define LTM_M_TMOOTF_WEIGHTYB_OFFSET                                  0x394
#define LTM_M_TMOOTF_WEIGHTYB_SHIFT                                   0
#define LTM_M_TMOOTF_WEIGHTYB_LENGTH                                  12

/* REGISTER saturn_post_pre_proc_reg_230 */
#define LTM_FORCE_UPDATE_EN_OFFSET                                    0x398
#define LTM_FORCE_UPDATE_EN_SHIFT                                     0
#define LTM_FORCE_UPDATE_EN_LENGTH                                    1
#define LTM_VSYNC_UPDATE_EN_SHIFT                                     1
#define LTM_VSYNC_UPDATE_EN_LENGTH                                    1
#define LTM_SHADOW_READ_EN_SHIFT                                      2
#define LTM_SHADOW_READ_EN_LENGTH                                     1

/* REGISTER saturn_post_pre_proc_reg_231 */
#define LTM_FORCE_UPDATE_PULSE_OFFSET                                 0x39c
#define LTM_FORCE_UPDATE_PULSE_SHIFT                                  0
#define LTM_FORCE_UPDATE_PULSE_LENGTH                                 1

/* REGISTER saturn_post_pre_proc_reg_232 */
#define LTM_ICG_OVERRIDE_OFFSET                                       0x3a0
#define LTM_ICG_OVERRIDE_SHIFT                                        0
#define LTM_ICG_OVERRIDE_LENGTH                                       1

/* REGISTER saturn_post_pre_proc_reg_233 */
#define LTM_TRIGGER_OFFSET                                            0x3a4
#define LTM_TRIGGER_SHIFT                                             0
#define LTM_TRIGGER_LENGTH                                            1

/* REGISTER saturn_post_pre_proc_reg_234 */
#define LTM_TRIGGER2_OFFSET                                           0x3a8
#define LTM_TRIGGER2_SHIFT                                            0
#define LTM_TRIGGER2_LENGTH                                           1

#endif
