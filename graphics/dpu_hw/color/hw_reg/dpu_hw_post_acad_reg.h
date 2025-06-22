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

#ifndef _DPU_HW_POST_ACAD_REG_H_
#define _DPU_HW_POST_ACAD_REG_H_

/* REGISTER saturn_acad_reg_0 */
#define ACAD_EN_OFFSET                                                0x00
#define ACAD_EN_SHIFT                                                 0
#define ACAD_EN_LENGTH                                                1
#define M_N_HIST_MODE_SHIFT                                           2
#define M_N_HIST_MODE_LENGTH                                          1
#define M_N_CURVE_MODE_SHIFT                                          3
#define M_N_CURVE_MODE_LENGTH                                         1
#define M_N_ETM_MODE_SHIFT                                            4
#define M_N_ETM_MODE_LENGTH                                           1
#define MEM_LP_SD_EN_SHIFT                                            5
#define MEM_LP_SD_EN_LENGTH                                           1
#define MEM_LP_DS_EN_SHIFT                                            6
#define MEM_LP_DS_EN_LENGTH                                           1
#define TP_ALPHA_SHIFT                                                16
#define TP_ALPHA_LENGTH                                               9

/* REGISTER saturn_acad_reg_1 */
#define M_N_ROI_X_OFFSET                                              0x04
#define M_N_ROI_X_SHIFT                                               0
#define M_N_ROI_X_LENGTH                                              13
#define M_N_ROI_Y_SHIFT                                               16
#define M_N_ROI_Y_LENGTH                                              13

/* REGISTER saturn_acad_reg_2 */
#define M_N_ROI_W_OFFSET                                              0x08
#define M_N_ROI_W_SHIFT                                               0
#define M_N_ROI_W_LENGTH                                              13
#define M_N_ROI_H_SHIFT                                               16
#define M_N_ROI_H_LENGTH                                              13

/* REGISTER saturn_acad_reg_3 */
#define M_N_BLK_WIDTH_OFFSET                                          0x0c
#define M_N_BLK_WIDTH_SHIFT                                           0
#define M_N_BLK_WIDTH_LENGTH                                          9
#define M_N_BLK_NUM_HORI_SHIFT                                        9
#define M_N_BLK_NUM_HORI_LENGTH                                       6
#define M_N_BLK_HEIGHT_SHIFT                                          16
#define M_N_BLK_HEIGHT_LENGTH                                         9
#define M_N_BLK_NUM_VERT_SHIFT                                        25
#define M_N_BLK_NUM_VERT_LENGTH                                       6

/* REGISTER saturn_acad_reg_4 */
#define M_N_WEIGHT_SCALER_X_OFFSET                                    0x10
#define M_N_WEIGHT_SCALER_X_SHIFT                                     0
#define M_N_WEIGHT_SCALER_X_LENGTH                                    11
#define M_N_WEIGHT_SCALER_Y_SHIFT                                     16
#define M_N_WEIGHT_SCALER_Y_LENGTH                                    11

/* REGISTER saturn_acad_reg_5 */
#define M_N_SUB_ROI0_X_OFFSET                                         0x14
#define M_N_SUB_ROI0_X_SHIFT                                          0
#define M_N_SUB_ROI0_X_LENGTH                                         13
#define M_N_SUB_ROI0_Y_SHIFT                                          16
#define M_N_SUB_ROI0_Y_LENGTH                                         13

/* REGISTER saturn_acad_reg_6 */
#define M_N_SUB_ROI0_W_OFFSET                                         0x18
#define M_N_SUB_ROI0_W_SHIFT                                          0
#define M_N_SUB_ROI0_W_LENGTH                                         13
#define M_N_SUB_ROI0_H_SHIFT                                          16
#define M_N_SUB_ROI0_H_LENGTH                                         13

/* REGISTER saturn_acad_reg_7 */
#define M_N_SUB_ROI1_X_OFFSET                                         0x1c
#define M_N_SUB_ROI1_X_SHIFT                                          0
#define M_N_SUB_ROI1_X_LENGTH                                         13
#define M_N_SUB_ROI1_Y_SHIFT                                          16
#define M_N_SUB_ROI1_Y_LENGTH                                         13

/* REGISTER saturn_acad_reg_8 */
#define M_N_SUB_ROI1_W_OFFSET                                         0x20
#define M_N_SUB_ROI1_W_SHIFT                                          0
#define M_N_SUB_ROI1_W_LENGTH                                         13
#define M_N_SUB_ROI1_H_SHIFT                                          16
#define M_N_SUB_ROI1_H_LENGTH                                         13

/* REGISTER saturn_acad_reg_9 */
#define M_N_SUB_ROI2_X_OFFSET                                         0x24
#define M_N_SUB_ROI2_X_SHIFT                                          0
#define M_N_SUB_ROI2_X_LENGTH                                         13
#define M_N_SUB_ROI2_Y_SHIFT                                          16
#define M_N_SUB_ROI2_Y_LENGTH                                         13

/* REGISTER saturn_acad_reg_10 */
#define M_N_SUB_ROI2_W_OFFSET                                         0x28
#define M_N_SUB_ROI2_W_SHIFT                                          0
#define M_N_SUB_ROI2_W_LENGTH                                         13
#define M_N_SUB_ROI2_H_SHIFT                                          16
#define M_N_SUB_ROI2_H_LENGTH                                         13

/* REGISTER saturn_acad_reg_11 */
#define M_N_SUB_ROI3_X_OFFSET                                         0x2c
#define M_N_SUB_ROI3_X_SHIFT                                          0
#define M_N_SUB_ROI3_X_LENGTH                                         13
#define M_N_SUB_ROI3_Y_SHIFT                                          16
#define M_N_SUB_ROI3_Y_LENGTH                                         13

/* REGISTER saturn_acad_reg_12 */
#define M_N_SUB_ROI3_W_OFFSET                                         0x30
#define M_N_SUB_ROI3_W_SHIFT                                          0
#define M_N_SUB_ROI3_W_LENGTH                                         13
#define M_N_SUB_ROI3_H_SHIFT                                          16
#define M_N_SUB_ROI3_H_LENGTH                                         13

/* REGISTER saturn_acad_reg_13 */
#define M_N_CURVE_ALPHA_OFFSET                                        0x34
#define M_N_CURVE_ALPHA_SHIFT                                         0
#define M_N_CURVE_ALPHA_LENGTH                                        13

/* REGISTER saturn_acad_reg_14 */
#define M_N_OFFSET0_OFFSET                                            0x38
#define M_N_OFFSET0_SHIFT                                             0
#define M_N_OFFSET0_LENGTH                                            11
#define M_N_OFFSET1_SHIFT                                             16
#define M_N_OFFSET1_LENGTH                                            11

/* REGISTER saturn_acad_reg_15 */
#define M_N_DST_ALPHA_OFFSET                                          0x3c
#define M_N_DST_ALPHA_SHIFT                                           0
#define M_N_DST_ALPHA_LENGTH                                          8
#define M_N_ALPHA_STEP_SHIFT                                          8
#define M_N_ALPHA_STEP_LENGTH                                         8
#define M_N_SKIP_FRAMES_SHIFT                                         16
#define M_N_SKIP_FRAMES_LENGTH                                        8
#define M_P_SEGMENT_THR0_SHIFT                                        24
#define M_P_SEGMENT_THR0_LENGTH                                       8

/* REGISTER saturn_acad_reg_16 */
#define M_P_SEGMENT_THR1_OFFSET                                       0x40
#define M_P_SEGMENT_THR1_SHIFT                                        0
#define M_P_SEGMENT_THR1_LENGTH                                       8
#define M_P_SEGMENT_THR2_SHIFT                                        8
#define M_P_SEGMENT_THR2_LENGTH                                       8
#define M_P_SEGMENT_THR3_SHIFT                                        16
#define M_P_SEGMENT_THR3_LENGTH                                       8
#define M_P_SEGMENT_THR4_SHIFT                                        24
#define M_P_SEGMENT_THR4_LENGTH                                       8

/* REGISTER saturn_acad_reg_17 */
#define M_P_CONTRAST_CURVE0_0_OFFSET                                  0x44
#define M_P_CONTRAST_CURVE0_0_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_0_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_0_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_0_LENGTH                                  16

/* REGISTER saturn_acad_reg_18 */
#define M_P_CONTRAST_CURVE0_1_OFFSET                                  0x48
#define M_P_CONTRAST_CURVE0_1_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_1_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_1_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_1_LENGTH                                  16

/* REGISTER saturn_acad_reg_19 */
#define M_P_CONTRAST_CURVE0_2_OFFSET                                  0x4c
#define M_P_CONTRAST_CURVE0_2_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_2_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_2_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_2_LENGTH                                  16

/* REGISTER saturn_acad_reg_20 */
#define M_P_CONTRAST_CURVE0_3_OFFSET                                  0x50
#define M_P_CONTRAST_CURVE0_3_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_3_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_3_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_3_LENGTH                                  16

/* REGISTER saturn_acad_reg_21 */
#define M_P_CONTRAST_CURVE0_4_OFFSET                                  0x54
#define M_P_CONTRAST_CURVE0_4_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_4_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_4_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_4_LENGTH                                  16

/* REGISTER saturn_acad_reg_22 */
#define M_P_CONTRAST_CURVE0_5_OFFSET                                  0x58
#define M_P_CONTRAST_CURVE0_5_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_5_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_5_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_5_LENGTH                                  16

/* REGISTER saturn_acad_reg_23 */
#define M_P_CONTRAST_CURVE0_6_OFFSET                                  0x5c
#define M_P_CONTRAST_CURVE0_6_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_6_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_6_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_6_LENGTH                                  16

/* REGISTER saturn_acad_reg_24 */
#define M_P_CONTRAST_CURVE0_7_OFFSET                                  0x60
#define M_P_CONTRAST_CURVE0_7_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_7_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_7_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_7_LENGTH                                  16

/* REGISTER saturn_acad_reg_25 */
#define M_P_CONTRAST_CURVE0_8_OFFSET                                  0x64
#define M_P_CONTRAST_CURVE0_8_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_8_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_8_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_8_LENGTH                                  16

/* REGISTER saturn_acad_reg_26 */
#define M_P_CONTRAST_CURVE0_9_OFFSET                                  0x68
#define M_P_CONTRAST_CURVE0_9_SHIFT                                   0
#define M_P_CONTRAST_CURVE0_9_LENGTH                                  16
#define M_P_CONTRAST_CURVE1_9_SHIFT                                   16
#define M_P_CONTRAST_CURVE1_9_LENGTH                                  16

/* REGISTER saturn_acad_reg_27 */
#define M_P_CONTRAST_CURVE0_10_OFFSET                                 0x6c
#define M_P_CONTRAST_CURVE0_10_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_10_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_10_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_10_LENGTH                                 16

/* REGISTER saturn_acad_reg_28 */
#define M_P_CONTRAST_CURVE0_11_OFFSET                                 0x70
#define M_P_CONTRAST_CURVE0_11_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_11_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_11_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_11_LENGTH                                 16

/* REGISTER saturn_acad_reg_29 */
#define M_P_CONTRAST_CURVE0_12_OFFSET                                 0x74
#define M_P_CONTRAST_CURVE0_12_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_12_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_12_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_12_LENGTH                                 16

/* REGISTER saturn_acad_reg_30 */
#define M_P_CONTRAST_CURVE0_13_OFFSET                                 0x78
#define M_P_CONTRAST_CURVE0_13_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_13_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_13_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_13_LENGTH                                 16

/* REGISTER saturn_acad_reg_31 */
#define M_P_CONTRAST_CURVE0_14_OFFSET                                 0x7c
#define M_P_CONTRAST_CURVE0_14_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_14_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_14_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_14_LENGTH                                 16

/* REGISTER saturn_acad_reg_32 */
#define M_P_CONTRAST_CURVE0_15_OFFSET                                 0x80
#define M_P_CONTRAST_CURVE0_15_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_15_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_15_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_15_LENGTH                                 16

/* REGISTER saturn_acad_reg_33 */
#define M_P_CONTRAST_CURVE0_16_OFFSET                                 0x84
#define M_P_CONTRAST_CURVE0_16_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_16_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_16_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_16_LENGTH                                 16

/* REGISTER saturn_acad_reg_34 */
#define M_P_CONTRAST_CURVE0_17_OFFSET                                 0x88
#define M_P_CONTRAST_CURVE0_17_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_17_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_17_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_17_LENGTH                                 16

/* REGISTER saturn_acad_reg_35 */
#define M_P_CONTRAST_CURVE0_18_OFFSET                                 0x8c
#define M_P_CONTRAST_CURVE0_18_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_18_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_18_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_18_LENGTH                                 16

/* REGISTER saturn_acad_reg_36 */
#define M_P_CONTRAST_CURVE0_19_OFFSET                                 0x90
#define M_P_CONTRAST_CURVE0_19_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_19_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_19_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_19_LENGTH                                 16

/* REGISTER saturn_acad_reg_37 */
#define M_P_CONTRAST_CURVE0_20_OFFSET                                 0x94
#define M_P_CONTRAST_CURVE0_20_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_20_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_20_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_20_LENGTH                                 16

/* REGISTER saturn_acad_reg_38 */
#define M_P_CONTRAST_CURVE0_21_OFFSET                                 0x98
#define M_P_CONTRAST_CURVE0_21_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_21_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_21_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_21_LENGTH                                 16

/* REGISTER saturn_acad_reg_39 */
#define M_P_CONTRAST_CURVE0_22_OFFSET                                 0x9c
#define M_P_CONTRAST_CURVE0_22_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_22_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_22_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_22_LENGTH                                 16

/* REGISTER saturn_acad_reg_40 */
#define M_P_CONTRAST_CURVE0_23_OFFSET                                 0xa0
#define M_P_CONTRAST_CURVE0_23_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_23_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_23_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_23_LENGTH                                 16

/* REGISTER saturn_acad_reg_41 */
#define M_P_CONTRAST_CURVE0_24_OFFSET                                 0xa4
#define M_P_CONTRAST_CURVE0_24_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_24_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_24_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_24_LENGTH                                 16

/* REGISTER saturn_acad_reg_42 */
#define M_P_CONTRAST_CURVE0_25_OFFSET                                 0xa8
#define M_P_CONTRAST_CURVE0_25_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_25_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_25_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_25_LENGTH                                 16

/* REGISTER saturn_acad_reg_43 */
#define M_P_CONTRAST_CURVE0_26_OFFSET                                 0xac
#define M_P_CONTRAST_CURVE0_26_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_26_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_26_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_26_LENGTH                                 16

/* REGISTER saturn_acad_reg_44 */
#define M_P_CONTRAST_CURVE0_27_OFFSET                                 0xb0
#define M_P_CONTRAST_CURVE0_27_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_27_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_27_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_27_LENGTH                                 16

/* REGISTER saturn_acad_reg_45 */
#define M_P_CONTRAST_CURVE0_28_OFFSET                                 0xb4
#define M_P_CONTRAST_CURVE0_28_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_28_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_28_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_28_LENGTH                                 16

/* REGISTER saturn_acad_reg_46 */
#define M_P_CONTRAST_CURVE0_29_OFFSET                                 0xb8
#define M_P_CONTRAST_CURVE0_29_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_29_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_29_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_29_LENGTH                                 16

/* REGISTER saturn_acad_reg_47 */
#define M_P_CONTRAST_CURVE0_30_OFFSET                                 0xbc
#define M_P_CONTRAST_CURVE0_30_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_30_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_30_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_30_LENGTH                                 16

/* REGISTER saturn_acad_reg_48 */
#define M_P_CONTRAST_CURVE0_31_OFFSET                                 0xc0
#define M_P_CONTRAST_CURVE0_31_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_31_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_31_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_31_LENGTH                                 16

/* REGISTER saturn_acad_reg_49 */
#define M_P_CONTRAST_CURVE0_32_OFFSET                                 0xc4
#define M_P_CONTRAST_CURVE0_32_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_32_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_32_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_32_LENGTH                                 16

/* REGISTER saturn_acad_reg_50 */
#define M_P_CONTRAST_CURVE0_33_OFFSET                                 0xc8
#define M_P_CONTRAST_CURVE0_33_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_33_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_33_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_33_LENGTH                                 16

/* REGISTER saturn_acad_reg_51 */
#define M_P_CONTRAST_CURVE0_34_OFFSET                                 0xcc
#define M_P_CONTRAST_CURVE0_34_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_34_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_34_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_34_LENGTH                                 16

/* REGISTER saturn_acad_reg_52 */
#define M_P_CONTRAST_CURVE0_35_OFFSET                                 0xd0
#define M_P_CONTRAST_CURVE0_35_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_35_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_35_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_35_LENGTH                                 16

/* REGISTER saturn_acad_reg_53 */
#define M_P_CONTRAST_CURVE0_36_OFFSET                                 0xd4
#define M_P_CONTRAST_CURVE0_36_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_36_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_36_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_36_LENGTH                                 16

/* REGISTER saturn_acad_reg_54 */
#define M_P_CONTRAST_CURVE0_37_OFFSET                                 0xd8
#define M_P_CONTRAST_CURVE0_37_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_37_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_37_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_37_LENGTH                                 16

/* REGISTER saturn_acad_reg_55 */
#define M_P_CONTRAST_CURVE0_38_OFFSET                                 0xdc
#define M_P_CONTRAST_CURVE0_38_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_38_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_38_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_38_LENGTH                                 16

/* REGISTER saturn_acad_reg_56 */
#define M_P_CONTRAST_CURVE0_39_OFFSET                                 0xe0
#define M_P_CONTRAST_CURVE0_39_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_39_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_39_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_39_LENGTH                                 16

/* REGISTER saturn_acad_reg_57 */
#define M_P_CONTRAST_CURVE0_40_OFFSET                                 0xe4
#define M_P_CONTRAST_CURVE0_40_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_40_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_40_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_40_LENGTH                                 16

/* REGISTER saturn_acad_reg_58 */
#define M_P_CONTRAST_CURVE0_41_OFFSET                                 0xe8
#define M_P_CONTRAST_CURVE0_41_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_41_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_41_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_41_LENGTH                                 16

/* REGISTER saturn_acad_reg_59 */
#define M_P_CONTRAST_CURVE0_42_OFFSET                                 0xec
#define M_P_CONTRAST_CURVE0_42_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_42_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_42_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_42_LENGTH                                 16

/* REGISTER saturn_acad_reg_60 */
#define M_P_CONTRAST_CURVE0_43_OFFSET                                 0xf0
#define M_P_CONTRAST_CURVE0_43_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_43_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_43_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_43_LENGTH                                 16

/* REGISTER saturn_acad_reg_61 */
#define M_P_CONTRAST_CURVE0_44_OFFSET                                 0xf4
#define M_P_CONTRAST_CURVE0_44_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_44_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_44_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_44_LENGTH                                 16

/* REGISTER saturn_acad_reg_62 */
#define M_P_CONTRAST_CURVE0_45_OFFSET                                 0xf8
#define M_P_CONTRAST_CURVE0_45_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_45_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_45_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_45_LENGTH                                 16

/* REGISTER saturn_acad_reg_63 */
#define M_P_CONTRAST_CURVE0_46_OFFSET                                 0xfc
#define M_P_CONTRAST_CURVE0_46_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_46_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_46_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_46_LENGTH                                 16

/* REGISTER saturn_acad_reg_64 */
#define M_P_CONTRAST_CURVE0_47_OFFSET                                 0x100
#define M_P_CONTRAST_CURVE0_47_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_47_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_47_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_47_LENGTH                                 16

/* REGISTER saturn_acad_reg_65 */
#define M_P_CONTRAST_CURVE0_48_OFFSET                                 0x104
#define M_P_CONTRAST_CURVE0_48_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_48_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_48_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_48_LENGTH                                 16

/* REGISTER saturn_acad_reg_66 */
#define M_P_CONTRAST_CURVE0_49_OFFSET                                 0x108
#define M_P_CONTRAST_CURVE0_49_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_49_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_49_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_49_LENGTH                                 16

/* REGISTER saturn_acad_reg_67 */
#define M_P_CONTRAST_CURVE0_50_OFFSET                                 0x10c
#define M_P_CONTRAST_CURVE0_50_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_50_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_50_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_50_LENGTH                                 16

/* REGISTER saturn_acad_reg_68 */
#define M_P_CONTRAST_CURVE0_51_OFFSET                                 0x110
#define M_P_CONTRAST_CURVE0_51_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_51_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_51_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_51_LENGTH                                 16

/* REGISTER saturn_acad_reg_69 */
#define M_P_CONTRAST_CURVE0_52_OFFSET                                 0x114
#define M_P_CONTRAST_CURVE0_52_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_52_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_52_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_52_LENGTH                                 16

/* REGISTER saturn_acad_reg_70 */
#define M_P_CONTRAST_CURVE0_53_OFFSET                                 0x118
#define M_P_CONTRAST_CURVE0_53_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_53_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_53_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_53_LENGTH                                 16

/* REGISTER saturn_acad_reg_71 */
#define M_P_CONTRAST_CURVE0_54_OFFSET                                 0x11c
#define M_P_CONTRAST_CURVE0_54_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_54_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_54_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_54_LENGTH                                 16

/* REGISTER saturn_acad_reg_72 */
#define M_P_CONTRAST_CURVE0_55_OFFSET                                 0x120
#define M_P_CONTRAST_CURVE0_55_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_55_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_55_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_55_LENGTH                                 16

/* REGISTER saturn_acad_reg_73 */
#define M_P_CONTRAST_CURVE0_56_OFFSET                                 0x124
#define M_P_CONTRAST_CURVE0_56_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_56_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_56_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_56_LENGTH                                 16

/* REGISTER saturn_acad_reg_74 */
#define M_P_CONTRAST_CURVE0_57_OFFSET                                 0x128
#define M_P_CONTRAST_CURVE0_57_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_57_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_57_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_57_LENGTH                                 16

/* REGISTER saturn_acad_reg_75 */
#define M_P_CONTRAST_CURVE0_58_OFFSET                                 0x12c
#define M_P_CONTRAST_CURVE0_58_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_58_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_58_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_58_LENGTH                                 16

/* REGISTER saturn_acad_reg_76 */
#define M_P_CONTRAST_CURVE0_59_OFFSET                                 0x130
#define M_P_CONTRAST_CURVE0_59_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_59_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_59_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_59_LENGTH                                 16

/* REGISTER saturn_acad_reg_77 */
#define M_P_CONTRAST_CURVE0_60_OFFSET                                 0x134
#define M_P_CONTRAST_CURVE0_60_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_60_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_60_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_60_LENGTH                                 16

/* REGISTER saturn_acad_reg_78 */
#define M_P_CONTRAST_CURVE0_61_OFFSET                                 0x138
#define M_P_CONTRAST_CURVE0_61_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_61_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_61_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_61_LENGTH                                 16

/* REGISTER saturn_acad_reg_79 */
#define M_P_CONTRAST_CURVE0_62_OFFSET                                 0x13c
#define M_P_CONTRAST_CURVE0_62_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_62_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_62_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_62_LENGTH                                 16

/* REGISTER saturn_acad_reg_80 */
#define M_P_CONTRAST_CURVE0_63_OFFSET                                 0x140
#define M_P_CONTRAST_CURVE0_63_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_63_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_63_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_63_LENGTH                                 16

/* REGISTER saturn_acad_reg_81 */
#define M_P_CONTRAST_CURVE0_64_OFFSET                                 0x144
#define M_P_CONTRAST_CURVE0_64_SHIFT                                  0
#define M_P_CONTRAST_CURVE0_64_LENGTH                                 16
#define M_P_CONTRAST_CURVE1_64_SHIFT                                  16
#define M_P_CONTRAST_CURVE1_64_LENGTH                                 16

/* REGISTER saturn_acad_reg_82 */
#define M_P_CONTRAST_CURVE2_0_OFFSET                                  0x148
#define M_P_CONTRAST_CURVE2_0_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_0_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_0_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_0_LENGTH                                  16

/* REGISTER saturn_acad_reg_83 */
#define M_P_CONTRAST_CURVE2_1_OFFSET                                  0x14c
#define M_P_CONTRAST_CURVE2_1_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_1_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_1_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_1_LENGTH                                  16

/* REGISTER saturn_acad_reg_84 */
#define M_P_CONTRAST_CURVE2_2_OFFSET                                  0x150
#define M_P_CONTRAST_CURVE2_2_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_2_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_2_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_2_LENGTH                                  16

/* REGISTER saturn_acad_reg_85 */
#define M_P_CONTRAST_CURVE2_3_OFFSET                                  0x154
#define M_P_CONTRAST_CURVE2_3_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_3_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_3_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_3_LENGTH                                  16

/* REGISTER saturn_acad_reg_86 */
#define M_P_CONTRAST_CURVE2_4_OFFSET                                  0x158
#define M_P_CONTRAST_CURVE2_4_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_4_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_4_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_4_LENGTH                                  16

/* REGISTER saturn_acad_reg_87 */
#define M_P_CONTRAST_CURVE2_5_OFFSET                                  0x15c
#define M_P_CONTRAST_CURVE2_5_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_5_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_5_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_5_LENGTH                                  16

/* REGISTER saturn_acad_reg_88 */
#define M_P_CONTRAST_CURVE2_6_OFFSET                                  0x160
#define M_P_CONTRAST_CURVE2_6_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_6_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_6_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_6_LENGTH                                  16

/* REGISTER saturn_acad_reg_89 */
#define M_P_CONTRAST_CURVE2_7_OFFSET                                  0x164
#define M_P_CONTRAST_CURVE2_7_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_7_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_7_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_7_LENGTH                                  16

/* REGISTER saturn_acad_reg_90 */
#define M_P_CONTRAST_CURVE2_8_OFFSET                                  0x168
#define M_P_CONTRAST_CURVE2_8_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_8_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_8_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_8_LENGTH                                  16

/* REGISTER saturn_acad_reg_91 */
#define M_P_CONTRAST_CURVE2_9_OFFSET                                  0x16c
#define M_P_CONTRAST_CURVE2_9_SHIFT                                   0
#define M_P_CONTRAST_CURVE2_9_LENGTH                                  16
#define M_P_CONTRAST_CURVE3_9_SHIFT                                   16
#define M_P_CONTRAST_CURVE3_9_LENGTH                                  16

/* REGISTER saturn_acad_reg_92 */
#define M_P_CONTRAST_CURVE2_10_OFFSET                                 0x170
#define M_P_CONTRAST_CURVE2_10_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_10_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_10_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_10_LENGTH                                 16

/* REGISTER saturn_acad_reg_93 */
#define M_P_CONTRAST_CURVE2_11_OFFSET                                 0x174
#define M_P_CONTRAST_CURVE2_11_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_11_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_11_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_11_LENGTH                                 16

/* REGISTER saturn_acad_reg_94 */
#define M_P_CONTRAST_CURVE2_12_OFFSET                                 0x178
#define M_P_CONTRAST_CURVE2_12_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_12_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_12_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_12_LENGTH                                 16

/* REGISTER saturn_acad_reg_95 */
#define M_P_CONTRAST_CURVE2_13_OFFSET                                 0x17c
#define M_P_CONTRAST_CURVE2_13_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_13_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_13_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_13_LENGTH                                 16

/* REGISTER saturn_acad_reg_96 */
#define M_P_CONTRAST_CURVE2_14_OFFSET                                 0x180
#define M_P_CONTRAST_CURVE2_14_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_14_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_14_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_14_LENGTH                                 16

/* REGISTER saturn_acad_reg_97 */
#define M_P_CONTRAST_CURVE2_15_OFFSET                                 0x184
#define M_P_CONTRAST_CURVE2_15_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_15_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_15_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_15_LENGTH                                 16

/* REGISTER saturn_acad_reg_98 */
#define M_P_CONTRAST_CURVE2_16_OFFSET                                 0x188
#define M_P_CONTRAST_CURVE2_16_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_16_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_16_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_16_LENGTH                                 16

/* REGISTER saturn_acad_reg_99 */
#define M_P_CONTRAST_CURVE2_17_OFFSET                                 0x18c
#define M_P_CONTRAST_CURVE2_17_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_17_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_17_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_17_LENGTH                                 16

/* REGISTER saturn_acad_reg_100 */
#define M_P_CONTRAST_CURVE2_18_OFFSET                                 0x190
#define M_P_CONTRAST_CURVE2_18_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_18_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_18_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_18_LENGTH                                 16

/* REGISTER saturn_acad_reg_101 */
#define M_P_CONTRAST_CURVE2_19_OFFSET                                 0x194
#define M_P_CONTRAST_CURVE2_19_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_19_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_19_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_19_LENGTH                                 16

/* REGISTER saturn_acad_reg_102 */
#define M_P_CONTRAST_CURVE2_20_OFFSET                                 0x198
#define M_P_CONTRAST_CURVE2_20_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_20_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_20_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_20_LENGTH                                 16

/* REGISTER saturn_acad_reg_103 */
#define M_P_CONTRAST_CURVE2_21_OFFSET                                 0x19c
#define M_P_CONTRAST_CURVE2_21_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_21_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_21_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_21_LENGTH                                 16

/* REGISTER saturn_acad_reg_104 */
#define M_P_CONTRAST_CURVE2_22_OFFSET                                 0x1a0
#define M_P_CONTRAST_CURVE2_22_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_22_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_22_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_22_LENGTH                                 16

/* REGISTER saturn_acad_reg_105 */
#define M_P_CONTRAST_CURVE2_23_OFFSET                                 0x1a4
#define M_P_CONTRAST_CURVE2_23_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_23_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_23_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_23_LENGTH                                 16

/* REGISTER saturn_acad_reg_106 */
#define M_P_CONTRAST_CURVE2_24_OFFSET                                 0x1a8
#define M_P_CONTRAST_CURVE2_24_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_24_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_24_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_24_LENGTH                                 16

/* REGISTER saturn_acad_reg_107 */
#define M_P_CONTRAST_CURVE2_25_OFFSET                                 0x1ac
#define M_P_CONTRAST_CURVE2_25_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_25_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_25_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_25_LENGTH                                 16

/* REGISTER saturn_acad_reg_108 */
#define M_P_CONTRAST_CURVE2_26_OFFSET                                 0x1b0
#define M_P_CONTRAST_CURVE2_26_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_26_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_26_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_26_LENGTH                                 16

/* REGISTER saturn_acad_reg_109 */
#define M_P_CONTRAST_CURVE2_27_OFFSET                                 0x1b4
#define M_P_CONTRAST_CURVE2_27_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_27_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_27_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_27_LENGTH                                 16

/* REGISTER saturn_acad_reg_110 */
#define M_P_CONTRAST_CURVE2_28_OFFSET                                 0x1b8
#define M_P_CONTRAST_CURVE2_28_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_28_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_28_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_28_LENGTH                                 16

/* REGISTER saturn_acad_reg_111 */
#define M_P_CONTRAST_CURVE2_29_OFFSET                                 0x1bc
#define M_P_CONTRAST_CURVE2_29_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_29_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_29_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_29_LENGTH                                 16

/* REGISTER saturn_acad_reg_112 */
#define M_P_CONTRAST_CURVE2_30_OFFSET                                 0x1c0
#define M_P_CONTRAST_CURVE2_30_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_30_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_30_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_30_LENGTH                                 16

/* REGISTER saturn_acad_reg_113 */
#define M_P_CONTRAST_CURVE2_31_OFFSET                                 0x1c4
#define M_P_CONTRAST_CURVE2_31_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_31_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_31_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_31_LENGTH                                 16

/* REGISTER saturn_acad_reg_114 */
#define M_P_CONTRAST_CURVE2_32_OFFSET                                 0x1c8
#define M_P_CONTRAST_CURVE2_32_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_32_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_32_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_32_LENGTH                                 16

/* REGISTER saturn_acad_reg_115 */
#define M_P_CONTRAST_CURVE2_33_OFFSET                                 0x1cc
#define M_P_CONTRAST_CURVE2_33_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_33_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_33_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_33_LENGTH                                 16

/* REGISTER saturn_acad_reg_116 */
#define M_P_CONTRAST_CURVE2_34_OFFSET                                 0x1d0
#define M_P_CONTRAST_CURVE2_34_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_34_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_34_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_34_LENGTH                                 16

/* REGISTER saturn_acad_reg_117 */
#define M_P_CONTRAST_CURVE2_35_OFFSET                                 0x1d4
#define M_P_CONTRAST_CURVE2_35_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_35_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_35_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_35_LENGTH                                 16

/* REGISTER saturn_acad_reg_118 */
#define M_P_CONTRAST_CURVE2_36_OFFSET                                 0x1d8
#define M_P_CONTRAST_CURVE2_36_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_36_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_36_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_36_LENGTH                                 16

/* REGISTER saturn_acad_reg_119 */
#define M_P_CONTRAST_CURVE2_37_OFFSET                                 0x1dc
#define M_P_CONTRAST_CURVE2_37_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_37_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_37_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_37_LENGTH                                 16

/* REGISTER saturn_acad_reg_120 */
#define M_P_CONTRAST_CURVE2_38_OFFSET                                 0x1e0
#define M_P_CONTRAST_CURVE2_38_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_38_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_38_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_38_LENGTH                                 16

/* REGISTER saturn_acad_reg_121 */
#define M_P_CONTRAST_CURVE2_39_OFFSET                                 0x1e4
#define M_P_CONTRAST_CURVE2_39_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_39_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_39_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_39_LENGTH                                 16

/* REGISTER saturn_acad_reg_122 */
#define M_P_CONTRAST_CURVE2_40_OFFSET                                 0x1e8
#define M_P_CONTRAST_CURVE2_40_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_40_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_40_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_40_LENGTH                                 16

/* REGISTER saturn_acad_reg_123 */
#define M_P_CONTRAST_CURVE2_41_OFFSET                                 0x1ec
#define M_P_CONTRAST_CURVE2_41_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_41_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_41_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_41_LENGTH                                 16

/* REGISTER saturn_acad_reg_124 */
#define M_P_CONTRAST_CURVE2_42_OFFSET                                 0x1f0
#define M_P_CONTRAST_CURVE2_42_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_42_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_42_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_42_LENGTH                                 16

/* REGISTER saturn_acad_reg_125 */
#define M_P_CONTRAST_CURVE2_43_OFFSET                                 0x1f4
#define M_P_CONTRAST_CURVE2_43_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_43_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_43_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_43_LENGTH                                 16

/* REGISTER saturn_acad_reg_126 */
#define M_P_CONTRAST_CURVE2_44_OFFSET                                 0x1f8
#define M_P_CONTRAST_CURVE2_44_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_44_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_44_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_44_LENGTH                                 16

/* REGISTER saturn_acad_reg_127 */
#define M_P_CONTRAST_CURVE2_45_OFFSET                                 0x1fc
#define M_P_CONTRAST_CURVE2_45_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_45_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_45_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_45_LENGTH                                 16

/* REGISTER saturn_acad_reg_128 */
#define M_P_CONTRAST_CURVE2_46_OFFSET                                 0x200
#define M_P_CONTRAST_CURVE2_46_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_46_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_46_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_46_LENGTH                                 16

/* REGISTER saturn_acad_reg_129 */
#define M_P_CONTRAST_CURVE2_47_OFFSET                                 0x204
#define M_P_CONTRAST_CURVE2_47_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_47_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_47_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_47_LENGTH                                 16

/* REGISTER saturn_acad_reg_130 */
#define M_P_CONTRAST_CURVE2_48_OFFSET                                 0x208
#define M_P_CONTRAST_CURVE2_48_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_48_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_48_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_48_LENGTH                                 16

/* REGISTER saturn_acad_reg_131 */
#define M_P_CONTRAST_CURVE2_49_OFFSET                                 0x20c
#define M_P_CONTRAST_CURVE2_49_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_49_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_49_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_49_LENGTH                                 16

/* REGISTER saturn_acad_reg_132 */
#define M_P_CONTRAST_CURVE2_50_OFFSET                                 0x210
#define M_P_CONTRAST_CURVE2_50_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_50_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_50_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_50_LENGTH                                 16

/* REGISTER saturn_acad_reg_133 */
#define M_P_CONTRAST_CURVE2_51_OFFSET                                 0x214
#define M_P_CONTRAST_CURVE2_51_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_51_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_51_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_51_LENGTH                                 16

/* REGISTER saturn_acad_reg_134 */
#define M_P_CONTRAST_CURVE2_52_OFFSET                                 0x218
#define M_P_CONTRAST_CURVE2_52_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_52_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_52_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_52_LENGTH                                 16

/* REGISTER saturn_acad_reg_135 */
#define M_P_CONTRAST_CURVE2_53_OFFSET                                 0x21c
#define M_P_CONTRAST_CURVE2_53_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_53_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_53_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_53_LENGTH                                 16

/* REGISTER saturn_acad_reg_136 */
#define M_P_CONTRAST_CURVE2_54_OFFSET                                 0x220
#define M_P_CONTRAST_CURVE2_54_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_54_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_54_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_54_LENGTH                                 16

/* REGISTER saturn_acad_reg_137 */
#define M_P_CONTRAST_CURVE2_55_OFFSET                                 0x224
#define M_P_CONTRAST_CURVE2_55_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_55_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_55_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_55_LENGTH                                 16

/* REGISTER saturn_acad_reg_138 */
#define M_P_CONTRAST_CURVE2_56_OFFSET                                 0x228
#define M_P_CONTRAST_CURVE2_56_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_56_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_56_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_56_LENGTH                                 16

/* REGISTER saturn_acad_reg_139 */
#define M_P_CONTRAST_CURVE2_57_OFFSET                                 0x22c
#define M_P_CONTRAST_CURVE2_57_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_57_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_57_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_57_LENGTH                                 16

/* REGISTER saturn_acad_reg_140 */
#define M_P_CONTRAST_CURVE2_58_OFFSET                                 0x230
#define M_P_CONTRAST_CURVE2_58_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_58_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_58_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_58_LENGTH                                 16

/* REGISTER saturn_acad_reg_141 */
#define M_P_CONTRAST_CURVE2_59_OFFSET                                 0x234
#define M_P_CONTRAST_CURVE2_59_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_59_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_59_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_59_LENGTH                                 16

/* REGISTER saturn_acad_reg_142 */
#define M_P_CONTRAST_CURVE2_60_OFFSET                                 0x238
#define M_P_CONTRAST_CURVE2_60_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_60_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_60_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_60_LENGTH                                 16

/* REGISTER saturn_acad_reg_143 */
#define M_P_CONTRAST_CURVE2_61_OFFSET                                 0x23c
#define M_P_CONTRAST_CURVE2_61_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_61_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_61_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_61_LENGTH                                 16

/* REGISTER saturn_acad_reg_144 */
#define M_P_CONTRAST_CURVE2_62_OFFSET                                 0x240
#define M_P_CONTRAST_CURVE2_62_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_62_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_62_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_62_LENGTH                                 16

/* REGISTER saturn_acad_reg_145 */
#define M_P_CONTRAST_CURVE2_63_OFFSET                                 0x244
#define M_P_CONTRAST_CURVE2_63_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_63_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_63_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_63_LENGTH                                 16

/* REGISTER saturn_acad_reg_146 */
#define M_P_CONTRAST_CURVE2_64_OFFSET                                 0x248
#define M_P_CONTRAST_CURVE2_64_SHIFT                                  0
#define M_P_CONTRAST_CURVE2_64_LENGTH                                 16
#define M_P_CONTRAST_CURVE3_64_SHIFT                                  16
#define M_P_CONTRAST_CURVE3_64_LENGTH                                 16

/* REGISTER saturn_acad_reg_147 */
#define ACAD_FORCE_UPDATE_EN_OFFSET                                   0x24c
#define ACAD_FORCE_UPDATE_EN_SHIFT                                    0
#define ACAD_FORCE_UPDATE_EN_LENGTH                                   1
#define ACAD_VSYNC_UPDATE_EN_SHIFT                                    1
#define ACAD_VSYNC_UPDATE_EN_LENGTH                                   1
#define ACAD_SHADOW_READ_EN_SHIFT                                     2
#define ACAD_SHADOW_READ_EN_LENGTH                                    1

/* REGISTER saturn_acad_reg_148 */
#define ACAD_FORCE_UPDATE_PULSE_OFFSET                                0x250
#define ACAD_FORCE_UPDATE_PULSE_SHIFT                                 0
#define ACAD_FORCE_UPDATE_PULSE_LENGTH                                1

/* REGISTER saturn_acad_reg_149 */
#define ACAD_ICG_OVERRIDE_OFFSET                                      0x254
#define ACAD_ICG_OVERRIDE_SHIFT                                       0
#define ACAD_ICG_OVERRIDE_LENGTH                                      1

/* REGISTER saturn_acad_reg_150 */
#define ACAD_TRIGGER_OFFSET                                           0x258
#define ACAD_TRIGGER_SHIFT                                            0
#define ACAD_TRIGGER_LENGTH                                           1

/* REGISTER saturn_acad_reg_151 */
#define ACAD_TRIGGER2_OFFSET                                          0x25c
#define ACAD_TRIGGER2_SHIFT                                           0
#define ACAD_TRIGGER2_LENGTH                                          1

#endif
