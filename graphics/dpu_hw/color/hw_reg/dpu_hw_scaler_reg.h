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

#ifndef _DPU_HW_SCALER_REG_H_
#define _DPU_HW_SCALER_REG_H_

/* REGISTER dpu_2dscl_reg_0 */
#define SCL_EN_OFFSET                                                 0x00
#define SCL_EN_SHIFT                                                  0
#define SCL_EN_LENGTH                                                 1

/* REGISTER dpu_2dscl_reg_1 */
#define ALPHA_DISABLE_OFFSET                                          0x04
#define DPU_2DSCL_ALPHA_DISABLE_SHIFT                                 0
#define DPU_2DSCL_ALPHA_DISABLE_LENGTH                                1

/* REGISTER dpu_2dscl_reg_2 */
#define INTERP_METHOD_OFFSET                                          0x08
#define INTERP_METHOD_SHIFT                                           0
#define INTERP_METHOD_LENGTH                                          1

/* REGISTER dpu_2dscl_reg_3 */
#define DETAIL_ENHANCE_ENABLE_OFFSET                                  0x0c
#define DETAIL_ENHANCE_ENABLE_SHIFT                                   0
#define DETAIL_ENHANCE_ENABLE_LENGTH                                  1

/* REGISTER dpu_2dscl_reg_4 */
#define OUTLIER_REMOVE_ENABLE_OFFSET                                  0x10
#define OUTLIER_REMOVE_ENABLE_SHIFT                                   0
#define OUTLIER_REMOVE_ENABLE_LENGTH                                  1

/* REGISTER dpu_2dscl_reg_5 */
#define STRIPE_HEIGHT_IN_OFFSET                                       0x14
#define STRIPE_HEIGHT_IN_SHIFT                                        0
#define STRIPE_HEIGHT_IN_LENGTH                                       14
#define STRIPE_WIDTH_IN_SHIFT                                         16
#define STRIPE_WIDTH_IN_LENGTH                                        15

/* REGISTER dpu_2dscl_reg_6 */
#define STRIPE_HEIGHT_OUT_OFFSET                                      0x18
#define STRIPE_HEIGHT_OUT_SHIFT                                       0
#define STRIPE_HEIGHT_OUT_LENGTH                                      14
#define STRIPE_WIDTH_OUT_SHIFT                                        16
#define STRIPE_WIDTH_OUT_LENGTH                                       15

/* REGISTER dpu_2dscl_reg_7 */
#define Y_STEP_OFFSET                                                 0x1c
#define Y_STEP_SHIFT                                                  0
#define Y_STEP_LENGTH                                                 18

/* REGISTER dpu_2dscl_reg_8 */
#define X_STEP_OFFSET                                                 0x20
#define X_STEP_SHIFT                                                  0
#define X_STEP_LENGTH                                                 18

/* REGISTER dpu_2dscl_reg_9 */
#define BASE_THRESHOLD_OFFSET                                         0x24
#define BASE_THRESHOLD_SHIFT                                          0
#define BASE_THRESHOLD_LENGTH                                         10

/* REGISTER dpu_2dscl_reg_10 */
#define SIGMOID_VAL0_OFFSET                                           0x28
#define SIGMOID_VAL0_SHIFT                                            0
#define SIGMOID_VAL0_LENGTH                                           3
#define SIGMOID_VAL1_SHIFT                                            3
#define SIGMOID_VAL1_LENGTH                                           3
#define SIGMOID_VAL2_SHIFT                                            8
#define SIGMOID_VAL2_LENGTH                                           3

/* REGISTER dpu_2dscl_reg_11 */
#define OUTLIER_THRESHOLD1_OFFSET                                     0x2c
#define OUTLIER_THRESHOLD1_SHIFT                                      0
#define OUTLIER_THRESHOLD1_LENGTH                                     4
#define OUTLIER_THRESHOLD2_SHIFT                                      8
#define OUTLIER_THRESHOLD2_LENGTH                                     5
#define THRESHOLD_NUMBER_SHIFT                                        16
#define THRESHOLD_NUMBER_LENGTH                                       4

/* REGISTER dpu_2dscl_reg_12 */
#define GRADIENT_THRESHOLD_OFFSET                                     0x30
#define GRADIENT_THRESHOLD_SHIFT                                      0
#define GRADIENT_THRESHOLD_LENGTH                                     10

/* REGISTER dpu_2dscl_reg_13 */
#define ARRAY_GAUSS_KERNEL_0_OFFSET                                   0x34
#define ARRAY_GAUSS_KERNEL_0_SHIFT                                    0
#define ARRAY_GAUSS_KERNEL_0_LENGTH                                   11

/* REGISTER dpu_2dscl_reg_19 */
#define ARRAY_GAUSS_KERNEL_1_OFFSET                                   0x4c
#define ARRAY_GAUSS_KERNEL_1_SHIFT                                    0
#define ARRAY_GAUSS_KERNEL_1_LENGTH                                   11

/* REGISTER dpu_2dscl_reg_34 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_0_OFFSET                         0x88
#define ARRAY_BICUBIC_KERNEL_COMBINE_0_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_0_LENGTH                         24

/* REGISTER dpu_2dscl_reg_35 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_1_OFFSET                         0x8c
#define ARRAY_BICUBIC_KERNEL_COMBINE_1_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_1_LENGTH                         24

/* REGISTER dpu_2dscl_reg_36 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_2_OFFSET                         0x90
#define ARRAY_BICUBIC_KERNEL_COMBINE_2_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_2_LENGTH                         24

/* REGISTER dpu_2dscl_reg_37 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_3_OFFSET                         0x94
#define ARRAY_BICUBIC_KERNEL_COMBINE_3_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_3_LENGTH                         24

/* REGISTER dpu_2dscl_reg_38 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_4_OFFSET                         0x98
#define ARRAY_BICUBIC_KERNEL_COMBINE_4_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_4_LENGTH                         24

/* REGISTER dpu_2dscl_reg_39 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_5_OFFSET                         0x9c
#define ARRAY_BICUBIC_KERNEL_COMBINE_5_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_5_LENGTH                         24

/* REGISTER dpu_2dscl_reg_40 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_6_OFFSET                         0xa0
#define ARRAY_BICUBIC_KERNEL_COMBINE_6_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_6_LENGTH                         24

/* REGISTER dpu_2dscl_reg_41 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_7_OFFSET                         0xa4
#define ARRAY_BICUBIC_KERNEL_COMBINE_7_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_7_LENGTH                         24

/* REGISTER dpu_2dscl_reg_42 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_8_OFFSET                         0xa8
#define ARRAY_BICUBIC_KERNEL_COMBINE_8_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_8_LENGTH                         24

/* REGISTER dpu_2dscl_reg_43 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_9_OFFSET                         0xac
#define ARRAY_BICUBIC_KERNEL_COMBINE_9_SHIFT                          0
#define ARRAY_BICUBIC_KERNEL_COMBINE_9_LENGTH                         24

/* REGISTER dpu_2dscl_reg_44 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_10_OFFSET                        0xb0
#define ARRAY_BICUBIC_KERNEL_COMBINE_10_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_10_LENGTH                        24

/* REGISTER dpu_2dscl_reg_45 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_11_OFFSET                        0xb4
#define ARRAY_BICUBIC_KERNEL_COMBINE_11_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_11_LENGTH                        24

/* REGISTER dpu_2dscl_reg_46 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_12_OFFSET                        0xb8
#define ARRAY_BICUBIC_KERNEL_COMBINE_12_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_12_LENGTH                        24

/* REGISTER dpu_2dscl_reg_47 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_13_OFFSET                        0xbc
#define ARRAY_BICUBIC_KERNEL_COMBINE_13_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_13_LENGTH                        24

/* REGISTER dpu_2dscl_reg_48 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_14_OFFSET                        0xc0
#define ARRAY_BICUBIC_KERNEL_COMBINE_14_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_14_LENGTH                        24

/* REGISTER dpu_2dscl_reg_49 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_15_OFFSET                        0xc4
#define ARRAY_BICUBIC_KERNEL_COMBINE_15_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_15_LENGTH                        24

/* REGISTER dpu_2dscl_reg_50 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_16_OFFSET                        0xc8
#define ARRAY_BICUBIC_KERNEL_COMBINE_16_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_16_LENGTH                        24

/* REGISTER dpu_2dscl_reg_51 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_17_OFFSET                        0xcc
#define ARRAY_BICUBIC_KERNEL_COMBINE_17_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_17_LENGTH                        24

/* REGISTER dpu_2dscl_reg_52 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_18_OFFSET                        0xd0
#define ARRAY_BICUBIC_KERNEL_COMBINE_18_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_18_LENGTH                        24

/* REGISTER dpu_2dscl_reg_53 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_19_OFFSET                        0xd4
#define ARRAY_BICUBIC_KERNEL_COMBINE_19_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_19_LENGTH                        24

/* REGISTER dpu_2dscl_reg_54 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_20_OFFSET                        0xd8
#define ARRAY_BICUBIC_KERNEL_COMBINE_20_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_20_LENGTH                        24

/* REGISTER dpu_2dscl_reg_55 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_21_OFFSET                        0xdc
#define ARRAY_BICUBIC_KERNEL_COMBINE_21_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_21_LENGTH                        24

/* REGISTER dpu_2dscl_reg_56 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_22_OFFSET                        0xe0
#define ARRAY_BICUBIC_KERNEL_COMBINE_22_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_22_LENGTH                        24

/* REGISTER dpu_2dscl_reg_57 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_23_OFFSET                        0xe4
#define ARRAY_BICUBIC_KERNEL_COMBINE_23_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_23_LENGTH                        24

/* REGISTER dpu_2dscl_reg_58 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_24_OFFSET                        0xe8
#define ARRAY_BICUBIC_KERNEL_COMBINE_24_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_24_LENGTH                        24

/* REGISTER dpu_2dscl_reg_59 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_25_OFFSET                        0xec
#define ARRAY_BICUBIC_KERNEL_COMBINE_25_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_25_LENGTH                        24

/* REGISTER dpu_2dscl_reg_60 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_26_OFFSET                        0xf0
#define ARRAY_BICUBIC_KERNEL_COMBINE_26_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_26_LENGTH                        24

/* REGISTER dpu_2dscl_reg_61 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_27_OFFSET                        0xf4
#define ARRAY_BICUBIC_KERNEL_COMBINE_27_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_27_LENGTH                        24

/* REGISTER dpu_2dscl_reg_62 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_28_OFFSET                        0xf8
#define ARRAY_BICUBIC_KERNEL_COMBINE_28_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_28_LENGTH                        24

/* REGISTER dpu_2dscl_reg_63 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_29_OFFSET                        0xfc
#define ARRAY_BICUBIC_KERNEL_COMBINE_29_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_29_LENGTH                        24

/* REGISTER dpu_2dscl_reg_64 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_30_OFFSET                        0x100
#define ARRAY_BICUBIC_KERNEL_COMBINE_30_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_30_LENGTH                        24

/* REGISTER dpu_2dscl_reg_65 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_31_OFFSET                        0x104
#define ARRAY_BICUBIC_KERNEL_COMBINE_31_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_31_LENGTH                        24

/* REGISTER dpu_2dscl_reg_66 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_32_OFFSET                        0x108
#define ARRAY_BICUBIC_KERNEL_COMBINE_32_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_32_LENGTH                        24

/* REGISTER dpu_2dscl_reg_67 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_33_OFFSET                        0x10c
#define ARRAY_BICUBIC_KERNEL_COMBINE_33_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_33_LENGTH                        24

/* REGISTER dpu_2dscl_reg_68 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_34_OFFSET                        0x110
#define ARRAY_BICUBIC_KERNEL_COMBINE_34_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_34_LENGTH                        24

/* REGISTER dpu_2dscl_reg_69 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_35_OFFSET                        0x114
#define ARRAY_BICUBIC_KERNEL_COMBINE_35_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_35_LENGTH                        24

/* REGISTER dpu_2dscl_reg_70 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_36_OFFSET                        0x118
#define ARRAY_BICUBIC_KERNEL_COMBINE_36_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_36_LENGTH                        24

/* REGISTER dpu_2dscl_reg_71 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_37_OFFSET                        0x11c
#define ARRAY_BICUBIC_KERNEL_COMBINE_37_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_37_LENGTH                        24

/* REGISTER dpu_2dscl_reg_72 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_38_OFFSET                        0x120
#define ARRAY_BICUBIC_KERNEL_COMBINE_38_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_38_LENGTH                        24

/* REGISTER dpu_2dscl_reg_73 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_39_OFFSET                        0x124
#define ARRAY_BICUBIC_KERNEL_COMBINE_39_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_39_LENGTH                        24

/* REGISTER dpu_2dscl_reg_74 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_40_OFFSET                        0x128
#define ARRAY_BICUBIC_KERNEL_COMBINE_40_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_40_LENGTH                        24

/* REGISTER dpu_2dscl_reg_75 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_41_OFFSET                        0x12c
#define ARRAY_BICUBIC_KERNEL_COMBINE_41_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_41_LENGTH                        24

/* REGISTER dpu_2dscl_reg_76 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_42_OFFSET                        0x130
#define ARRAY_BICUBIC_KERNEL_COMBINE_42_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_42_LENGTH                        24

/* REGISTER dpu_2dscl_reg_77 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_43_OFFSET                        0x134
#define ARRAY_BICUBIC_KERNEL_COMBINE_43_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_43_LENGTH                        24

/* REGISTER dpu_2dscl_reg_78 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_44_OFFSET                        0x138
#define ARRAY_BICUBIC_KERNEL_COMBINE_44_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_44_LENGTH                        24

/* REGISTER dpu_2dscl_reg_79 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_45_OFFSET                        0x13c
#define ARRAY_BICUBIC_KERNEL_COMBINE_45_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_45_LENGTH                        24

/* REGISTER dpu_2dscl_reg_80 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_46_OFFSET                        0x140
#define ARRAY_BICUBIC_KERNEL_COMBINE_46_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_46_LENGTH                        24

/* REGISTER dpu_2dscl_reg_81 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_47_OFFSET                        0x144
#define ARRAY_BICUBIC_KERNEL_COMBINE_47_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_47_LENGTH                        24

/* REGISTER dpu_2dscl_reg_82 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_48_OFFSET                        0x148
#define ARRAY_BICUBIC_KERNEL_COMBINE_48_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_48_LENGTH                        24

/* REGISTER dpu_2dscl_reg_83 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_49_OFFSET                        0x14c
#define ARRAY_BICUBIC_KERNEL_COMBINE_49_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_49_LENGTH                        24

/* REGISTER dpu_2dscl_reg_84 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_50_OFFSET                        0x150
#define ARRAY_BICUBIC_KERNEL_COMBINE_50_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_50_LENGTH                        24

/* REGISTER dpu_2dscl_reg_85 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_51_OFFSET                        0x154
#define ARRAY_BICUBIC_KERNEL_COMBINE_51_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_51_LENGTH                        24

/* REGISTER dpu_2dscl_reg_86 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_52_OFFSET                        0x158
#define ARRAY_BICUBIC_KERNEL_COMBINE_52_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_52_LENGTH                        24

/* REGISTER dpu_2dscl_reg_87 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_53_OFFSET                        0x15c
#define ARRAY_BICUBIC_KERNEL_COMBINE_53_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_53_LENGTH                        24

/* REGISTER dpu_2dscl_reg_88 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_54_OFFSET                        0x160
#define ARRAY_BICUBIC_KERNEL_COMBINE_54_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_54_LENGTH                        24

/* REGISTER dpu_2dscl_reg_89 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_55_OFFSET                        0x164
#define ARRAY_BICUBIC_KERNEL_COMBINE_55_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_55_LENGTH                        24

/* REGISTER dpu_2dscl_reg_90 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_56_OFFSET                        0x168
#define ARRAY_BICUBIC_KERNEL_COMBINE_56_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_56_LENGTH                        24

/* REGISTER dpu_2dscl_reg_91 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_57_OFFSET                        0x16c
#define ARRAY_BICUBIC_KERNEL_COMBINE_57_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_57_LENGTH                        24

/* REGISTER dpu_2dscl_reg_92 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_58_OFFSET                        0x170
#define ARRAY_BICUBIC_KERNEL_COMBINE_58_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_58_LENGTH                        24

/* REGISTER dpu_2dscl_reg_93 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_59_OFFSET                        0x174
#define ARRAY_BICUBIC_KERNEL_COMBINE_59_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_59_LENGTH                        24

/* REGISTER dpu_2dscl_reg_94 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_60_OFFSET                        0x178
#define ARRAY_BICUBIC_KERNEL_COMBINE_60_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_60_LENGTH                        24

/* REGISTER dpu_2dscl_reg_95 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_61_OFFSET                        0x17c
#define ARRAY_BICUBIC_KERNEL_COMBINE_61_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_61_LENGTH                        24

/* REGISTER dpu_2dscl_reg_96 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_62_OFFSET                        0x180
#define ARRAY_BICUBIC_KERNEL_COMBINE_62_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_62_LENGTH                        24

/* REGISTER dpu_2dscl_reg_97 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_63_OFFSET                        0x184
#define ARRAY_BICUBIC_KERNEL_COMBINE_63_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_63_LENGTH                        24

/* REGISTER dpu_2dscl_reg_98 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_64_OFFSET                        0x188
#define ARRAY_BICUBIC_KERNEL_COMBINE_64_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_64_LENGTH                        24

/* REGISTER dpu_2dscl_reg_99 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_65_OFFSET                        0x18c
#define ARRAY_BICUBIC_KERNEL_COMBINE_65_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_65_LENGTH                        24

/* REGISTER dpu_2dscl_reg_100 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_66_OFFSET                        0x190
#define ARRAY_BICUBIC_KERNEL_COMBINE_66_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_66_LENGTH                        24

/* REGISTER dpu_2dscl_reg_101 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_67_OFFSET                        0x194
#define ARRAY_BICUBIC_KERNEL_COMBINE_67_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_67_LENGTH                        24

/* REGISTER dpu_2dscl_reg_102 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_68_OFFSET                        0x198
#define ARRAY_BICUBIC_KERNEL_COMBINE_68_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_68_LENGTH                        24

/* REGISTER dpu_2dscl_reg_103 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_69_OFFSET                        0x19c
#define ARRAY_BICUBIC_KERNEL_COMBINE_69_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_69_LENGTH                        24

/* REGISTER dpu_2dscl_reg_104 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_70_OFFSET                        0x1a0
#define ARRAY_BICUBIC_KERNEL_COMBINE_70_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_70_LENGTH                        24

/* REGISTER dpu_2dscl_reg_105 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_71_OFFSET                        0x1a4
#define ARRAY_BICUBIC_KERNEL_COMBINE_71_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_71_LENGTH                        24

/* REGISTER dpu_2dscl_reg_106 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_72_OFFSET                        0x1a8
#define ARRAY_BICUBIC_KERNEL_COMBINE_72_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_72_LENGTH                        24

/* REGISTER dpu_2dscl_reg_107 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_73_OFFSET                        0x1ac
#define ARRAY_BICUBIC_KERNEL_COMBINE_73_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_73_LENGTH                        24

/* REGISTER dpu_2dscl_reg_108 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_74_OFFSET                        0x1b0
#define ARRAY_BICUBIC_KERNEL_COMBINE_74_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_74_LENGTH                        24

/* REGISTER dpu_2dscl_reg_109 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_75_OFFSET                        0x1b4
#define ARRAY_BICUBIC_KERNEL_COMBINE_75_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_75_LENGTH                        24

/* REGISTER dpu_2dscl_reg_110 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_76_OFFSET                        0x1b8
#define ARRAY_BICUBIC_KERNEL_COMBINE_76_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_76_LENGTH                        24

/* REGISTER dpu_2dscl_reg_111 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_77_OFFSET                        0x1bc
#define ARRAY_BICUBIC_KERNEL_COMBINE_77_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_77_LENGTH                        24

/* REGISTER dpu_2dscl_reg_112 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_78_OFFSET                        0x1c0
#define ARRAY_BICUBIC_KERNEL_COMBINE_78_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_78_LENGTH                        24

/* REGISTER dpu_2dscl_reg_113 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_79_OFFSET                        0x1c4
#define ARRAY_BICUBIC_KERNEL_COMBINE_79_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_79_LENGTH                        24

/* REGISTER dpu_2dscl_reg_114 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_80_OFFSET                        0x1c8
#define ARRAY_BICUBIC_KERNEL_COMBINE_80_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_80_LENGTH                        24

/* REGISTER dpu_2dscl_reg_115 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_81_OFFSET                        0x1cc
#define ARRAY_BICUBIC_KERNEL_COMBINE_81_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_81_LENGTH                        24

/* REGISTER dpu_2dscl_reg_116 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_82_OFFSET                        0x1d0
#define ARRAY_BICUBIC_KERNEL_COMBINE_82_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_82_LENGTH                        24

/* REGISTER dpu_2dscl_reg_117 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_83_OFFSET                        0x1d4
#define ARRAY_BICUBIC_KERNEL_COMBINE_83_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_83_LENGTH                        24

/* REGISTER dpu_2dscl_reg_118 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_84_OFFSET                        0x1d8
#define ARRAY_BICUBIC_KERNEL_COMBINE_84_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_84_LENGTH                        24

/* REGISTER dpu_2dscl_reg_119 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_85_OFFSET                        0x1dc
#define ARRAY_BICUBIC_KERNEL_COMBINE_85_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_85_LENGTH                        24

/* REGISTER dpu_2dscl_reg_120 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_86_OFFSET                        0x1e0
#define ARRAY_BICUBIC_KERNEL_COMBINE_86_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_86_LENGTH                        24

/* REGISTER dpu_2dscl_reg_121 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_87_OFFSET                        0x1e4
#define ARRAY_BICUBIC_KERNEL_COMBINE_87_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_87_LENGTH                        24

/* REGISTER dpu_2dscl_reg_122 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_88_OFFSET                        0x1e8
#define ARRAY_BICUBIC_KERNEL_COMBINE_88_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_88_LENGTH                        24

/* REGISTER dpu_2dscl_reg_123 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_89_OFFSET                        0x1ec
#define ARRAY_BICUBIC_KERNEL_COMBINE_89_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_89_LENGTH                        24

/* REGISTER dpu_2dscl_reg_124 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_90_OFFSET                        0x1f0
#define ARRAY_BICUBIC_KERNEL_COMBINE_90_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_90_LENGTH                        24

/* REGISTER dpu_2dscl_reg_125 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_91_OFFSET                        0x1f4
#define ARRAY_BICUBIC_KERNEL_COMBINE_91_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_91_LENGTH                        24

/* REGISTER dpu_2dscl_reg_126 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_92_OFFSET                        0x1f8
#define ARRAY_BICUBIC_KERNEL_COMBINE_92_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_92_LENGTH                        24

/* REGISTER dpu_2dscl_reg_127 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_93_OFFSET                        0x1fc
#define ARRAY_BICUBIC_KERNEL_COMBINE_93_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_93_LENGTH                        24

/* REGISTER dpu_2dscl_reg_128 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_94_OFFSET                        0x200
#define ARRAY_BICUBIC_KERNEL_COMBINE_94_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_94_LENGTH                        24

/* REGISTER dpu_2dscl_reg_129 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_95_OFFSET                        0x204
#define ARRAY_BICUBIC_KERNEL_COMBINE_95_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_95_LENGTH                        24

/* REGISTER dpu_2dscl_reg_130 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_96_OFFSET                        0x208
#define ARRAY_BICUBIC_KERNEL_COMBINE_96_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_96_LENGTH                        24

/* REGISTER dpu_2dscl_reg_131 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_97_OFFSET                        0x20c
#define ARRAY_BICUBIC_KERNEL_COMBINE_97_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_97_LENGTH                        24

/* REGISTER dpu_2dscl_reg_132 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_98_OFFSET                        0x210
#define ARRAY_BICUBIC_KERNEL_COMBINE_98_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_98_LENGTH                        24

/* REGISTER dpu_2dscl_reg_133 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_99_OFFSET                        0x214
#define ARRAY_BICUBIC_KERNEL_COMBINE_99_SHIFT                         0
#define ARRAY_BICUBIC_KERNEL_COMBINE_99_LENGTH                        24

/* REGISTER dpu_2dscl_reg_134 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_100_OFFSET                       0x218
#define ARRAY_BICUBIC_KERNEL_COMBINE_100_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_100_LENGTH                       24

/* REGISTER dpu_2dscl_reg_135 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_101_OFFSET                       0x21c
#define ARRAY_BICUBIC_KERNEL_COMBINE_101_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_101_LENGTH                       24

/* REGISTER dpu_2dscl_reg_136 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_102_OFFSET                       0x220
#define ARRAY_BICUBIC_KERNEL_COMBINE_102_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_102_LENGTH                       24

/* REGISTER dpu_2dscl_reg_137 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_103_OFFSET                       0x224
#define ARRAY_BICUBIC_KERNEL_COMBINE_103_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_103_LENGTH                       24

/* REGISTER dpu_2dscl_reg_138 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_104_OFFSET                       0x228
#define ARRAY_BICUBIC_KERNEL_COMBINE_104_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_104_LENGTH                       24

/* REGISTER dpu_2dscl_reg_139 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_105_OFFSET                       0x22c
#define ARRAY_BICUBIC_KERNEL_COMBINE_105_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_105_LENGTH                       24

/* REGISTER dpu_2dscl_reg_140 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_106_OFFSET                       0x230
#define ARRAY_BICUBIC_KERNEL_COMBINE_106_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_106_LENGTH                       24

/* REGISTER dpu_2dscl_reg_141 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_107_OFFSET                       0x234
#define ARRAY_BICUBIC_KERNEL_COMBINE_107_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_107_LENGTH                       24

/* REGISTER dpu_2dscl_reg_142 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_108_OFFSET                       0x238
#define ARRAY_BICUBIC_KERNEL_COMBINE_108_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_108_LENGTH                       24

/* REGISTER dpu_2dscl_reg_143 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_109_OFFSET                       0x23c
#define ARRAY_BICUBIC_KERNEL_COMBINE_109_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_109_LENGTH                       24

/* REGISTER dpu_2dscl_reg_144 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_110_OFFSET                       0x240
#define ARRAY_BICUBIC_KERNEL_COMBINE_110_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_110_LENGTH                       24

/* REGISTER dpu_2dscl_reg_145 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_111_OFFSET                       0x244
#define ARRAY_BICUBIC_KERNEL_COMBINE_111_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_111_LENGTH                       24

/* REGISTER dpu_2dscl_reg_146 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_112_OFFSET                       0x248
#define ARRAY_BICUBIC_KERNEL_COMBINE_112_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_112_LENGTH                       24

/* REGISTER dpu_2dscl_reg_147 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_113_OFFSET                       0x24c
#define ARRAY_BICUBIC_KERNEL_COMBINE_113_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_113_LENGTH                       24

/* REGISTER dpu_2dscl_reg_148 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_114_OFFSET                       0x250
#define ARRAY_BICUBIC_KERNEL_COMBINE_114_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_114_LENGTH                       24

/* REGISTER dpu_2dscl_reg_149 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_115_OFFSET                       0x254
#define ARRAY_BICUBIC_KERNEL_COMBINE_115_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_115_LENGTH                       24

/* REGISTER dpu_2dscl_reg_150 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_116_OFFSET                       0x258
#define ARRAY_BICUBIC_KERNEL_COMBINE_116_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_116_LENGTH                       24

/* REGISTER dpu_2dscl_reg_151 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_117_OFFSET                       0x25c
#define ARRAY_BICUBIC_KERNEL_COMBINE_117_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_117_LENGTH                       24

/* REGISTER dpu_2dscl_reg_152 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_118_OFFSET                       0x260
#define ARRAY_BICUBIC_KERNEL_COMBINE_118_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_118_LENGTH                       24

/* REGISTER dpu_2dscl_reg_153 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_119_OFFSET                       0x264
#define ARRAY_BICUBIC_KERNEL_COMBINE_119_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_119_LENGTH                       24

/* REGISTER dpu_2dscl_reg_154 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_120_OFFSET                       0x268
#define ARRAY_BICUBIC_KERNEL_COMBINE_120_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_120_LENGTH                       24

/* REGISTER dpu_2dscl_reg_155 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_121_OFFSET                       0x26c
#define ARRAY_BICUBIC_KERNEL_COMBINE_121_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_121_LENGTH                       24

/* REGISTER dpu_2dscl_reg_156 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_122_OFFSET                       0x270
#define ARRAY_BICUBIC_KERNEL_COMBINE_122_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_122_LENGTH                       24

/* REGISTER dpu_2dscl_reg_157 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_123_OFFSET                       0x274
#define ARRAY_BICUBIC_KERNEL_COMBINE_123_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_123_LENGTH                       24

/* REGISTER dpu_2dscl_reg_158 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_124_OFFSET                       0x278
#define ARRAY_BICUBIC_KERNEL_COMBINE_124_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_124_LENGTH                       24

/* REGISTER dpu_2dscl_reg_159 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_125_OFFSET                       0x27c
#define ARRAY_BICUBIC_KERNEL_COMBINE_125_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_125_LENGTH                       24

/* REGISTER dpu_2dscl_reg_160 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_126_OFFSET                       0x280
#define ARRAY_BICUBIC_KERNEL_COMBINE_126_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_126_LENGTH                       24

/* REGISTER dpu_2dscl_reg_161 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_127_OFFSET                       0x284
#define ARRAY_BICUBIC_KERNEL_COMBINE_127_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_127_LENGTH                       24

/* REGISTER dpu_2dscl_reg_162 */
#define ARRAY_BICUBIC_KERNEL_COMBINE_128_OFFSET                       0x288
#define ARRAY_BICUBIC_KERNEL_COMBINE_128_SHIFT                        0
#define ARRAY_BICUBIC_KERNEL_COMBINE_128_LENGTH                       24

/* REGISTER dpu_2dscl_reg_163 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_0_OFFSET                        0x28c

/* REGISTER dpu_2dscl_reg_164 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_1_OFFSET                        0x290

/* REGISTER dpu_2dscl_reg_165 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_2_OFFSET                        0x294

/* REGISTER dpu_2dscl_reg_166 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_3_OFFSET                        0x298

/* REGISTER dpu_2dscl_reg_167 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_4_OFFSET                        0x29c

/* REGISTER dpu_2dscl_reg_168 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_5_OFFSET                        0x2a0

/* REGISTER dpu_2dscl_reg_169 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_6_OFFSET                        0x2a4

/* REGISTER dpu_2dscl_reg_170 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_7_OFFSET                        0x2a8

/* REGISTER dpu_2dscl_reg_171 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_8_OFFSET                        0x2ac

/* REGISTER dpu_2dscl_reg_172 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_9_OFFSET                        0x2b0

/* REGISTER dpu_2dscl_reg_173 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_10_OFFSET                       0x2b4

/* REGISTER dpu_2dscl_reg_174 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_11_OFFSET                       0x2b8

/* REGISTER dpu_2dscl_reg_175 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_12_OFFSET                       0x2bc

/* REGISTER dpu_2dscl_reg_176 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_13_OFFSET                       0x2c0

/* REGISTER dpu_2dscl_reg_177 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_14_OFFSET                       0x2c4

/* REGISTER dpu_2dscl_reg_178 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_15_OFFSET                       0x2c8

/* REGISTER dpu_2dscl_reg_179 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_16_OFFSET                       0x2cc

/* REGISTER dpu_2dscl_reg_180 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_17_OFFSET                       0x2d0

/* REGISTER dpu_2dscl_reg_181 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_18_OFFSET                       0x2d4

/* REGISTER dpu_2dscl_reg_182 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_19_OFFSET                       0x2d8

/* REGISTER dpu_2dscl_reg_183 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_20_OFFSET                       0x2dc

/* REGISTER dpu_2dscl_reg_184 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_21_OFFSET                       0x2e0

/* REGISTER dpu_2dscl_reg_185 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_22_OFFSET                       0x2e4

/* REGISTER dpu_2dscl_reg_186 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_23_OFFSET                       0x2e8

/* REGISTER dpu_2dscl_reg_187 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_24_OFFSET                       0x2ec

/* REGISTER dpu_2dscl_reg_188 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_25_OFFSET                       0x2f0

/* REGISTER dpu_2dscl_reg_189 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_26_OFFSET                       0x2f4

/* REGISTER dpu_2dscl_reg_190 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_27_OFFSET                       0x2f8

/* REGISTER dpu_2dscl_reg_191 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_28_OFFSET                       0x2fc

/* REGISTER dpu_2dscl_reg_192 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_29_OFFSET                       0x300

/* REGISTER dpu_2dscl_reg_193 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_30_OFFSET                       0x304

/* REGISTER dpu_2dscl_reg_194 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_31_OFFSET                       0x308

/* REGISTER dpu_2dscl_reg_195 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_32_OFFSET                       0x30c

/* REGISTER dpu_2dscl_reg_196 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_33_OFFSET                       0x310

/* REGISTER dpu_2dscl_reg_197 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_34_OFFSET                       0x314

/* REGISTER dpu_2dscl_reg_198 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_35_OFFSET                       0x318

/* REGISTER dpu_2dscl_reg_199 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_36_OFFSET                       0x31c

/* REGISTER dpu_2dscl_reg_200 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_37_OFFSET                       0x320

/* REGISTER dpu_2dscl_reg_201 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_38_OFFSET                       0x324

/* REGISTER dpu_2dscl_reg_202 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_39_OFFSET                       0x328

/* REGISTER dpu_2dscl_reg_203 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_40_OFFSET                       0x32c

/* REGISTER dpu_2dscl_reg_204 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_41_OFFSET                       0x330

/* REGISTER dpu_2dscl_reg_205 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_42_OFFSET                       0x334

/* REGISTER dpu_2dscl_reg_206 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_43_OFFSET                       0x338

/* REGISTER dpu_2dscl_reg_207 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_44_OFFSET                       0x33c

/* REGISTER dpu_2dscl_reg_208 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_45_OFFSET                       0x340

/* REGISTER dpu_2dscl_reg_209 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_46_OFFSET                       0x344

/* REGISTER dpu_2dscl_reg_210 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_47_OFFSET                       0x348

/* REGISTER dpu_2dscl_reg_211 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_48_OFFSET                       0x34c

/* REGISTER dpu_2dscl_reg_212 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_49_OFFSET                       0x350

/* REGISTER dpu_2dscl_reg_213 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_50_OFFSET                       0x354

/* REGISTER dpu_2dscl_reg_214 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_51_OFFSET                       0x358

/* REGISTER dpu_2dscl_reg_215 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_52_OFFSET                       0x35c

/* REGISTER dpu_2dscl_reg_216 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_53_OFFSET                       0x360

/* REGISTER dpu_2dscl_reg_217 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_54_OFFSET                       0x364

/* REGISTER dpu_2dscl_reg_218 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_55_OFFSET                       0x368

/* REGISTER dpu_2dscl_reg_219 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_56_OFFSET                       0x36c

/* REGISTER dpu_2dscl_reg_220 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_57_OFFSET                       0x370

/* REGISTER dpu_2dscl_reg_221 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_58_OFFSET                       0x374

/* REGISTER dpu_2dscl_reg_222 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_59_OFFSET                       0x378

/* REGISTER dpu_2dscl_reg_223 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_60_OFFSET                       0x37c

/* REGISTER dpu_2dscl_reg_224 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_61_OFFSET                       0x380

/* REGISTER dpu_2dscl_reg_225 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_62_OFFSET                       0x384

/* REGISTER dpu_2dscl_reg_226 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_63_OFFSET                       0x388

/* REGISTER dpu_2dscl_reg_227 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_64_OFFSET                       0x38c

/* REGISTER dpu_2dscl_reg_228 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_65_OFFSET                       0x390

/* REGISTER dpu_2dscl_reg_229 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_66_OFFSET                       0x394

/* REGISTER dpu_2dscl_reg_230 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_67_OFFSET                       0x398

/* REGISTER dpu_2dscl_reg_231 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_68_OFFSET                       0x39c

/* REGISTER dpu_2dscl_reg_232 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_69_OFFSET                       0x3a0

/* REGISTER dpu_2dscl_reg_233 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_70_OFFSET                       0x3a4

/* REGISTER dpu_2dscl_reg_234 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_71_OFFSET                       0x3a8

/* REGISTER dpu_2dscl_reg_235 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_72_OFFSET                       0x3ac

/* REGISTER dpu_2dscl_reg_236 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_73_OFFSET                       0x3b0

/* REGISTER dpu_2dscl_reg_237 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_74_OFFSET                       0x3b4

/* REGISTER dpu_2dscl_reg_238 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_75_OFFSET                       0x3b8

/* REGISTER dpu_2dscl_reg_239 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_76_OFFSET                       0x3bc

/* REGISTER dpu_2dscl_reg_240 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_77_OFFSET                       0x3c0

/* REGISTER dpu_2dscl_reg_241 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_78_OFFSET                       0x3c4

/* REGISTER dpu_2dscl_reg_242 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_79_OFFSET                       0x3c8

/* REGISTER dpu_2dscl_reg_243 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_80_OFFSET                       0x3cc

/* REGISTER dpu_2dscl_reg_244 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_81_OFFSET                       0x3d0

/* REGISTER dpu_2dscl_reg_245 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_82_OFFSET                       0x3d4

/* REGISTER dpu_2dscl_reg_246 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_83_OFFSET                       0x3d8

/* REGISTER dpu_2dscl_reg_247 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_84_OFFSET                       0x3dc

/* REGISTER dpu_2dscl_reg_248 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_85_OFFSET                       0x3e0

/* REGISTER dpu_2dscl_reg_249 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_86_OFFSET                       0x3e4

/* REGISTER dpu_2dscl_reg_250 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_87_OFFSET                       0x3e8

/* REGISTER dpu_2dscl_reg_251 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_88_OFFSET                       0x3ec

/* REGISTER dpu_2dscl_reg_252 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_89_OFFSET                       0x3f0

/* REGISTER dpu_2dscl_reg_253 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_90_OFFSET                       0x3f4

/* REGISTER dpu_2dscl_reg_254 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_91_OFFSET                       0x3f8

/* REGISTER dpu_2dscl_reg_255 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_92_OFFSET                       0x3fc

/* REGISTER dpu_2dscl_reg_256 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_93_OFFSET                       0x400

/* REGISTER dpu_2dscl_reg_257 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_94_OFFSET                       0x404

/* REGISTER dpu_2dscl_reg_258 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_95_OFFSET                       0x408

/* REGISTER dpu_2dscl_reg_259 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_96_OFFSET                       0x40c

/* REGISTER dpu_2dscl_reg_260 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_97_OFFSET                       0x410

/* REGISTER dpu_2dscl_reg_261 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_98_OFFSET                       0x414

/* REGISTER dpu_2dscl_reg_262 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_99_OFFSET                       0x418

/* REGISTER dpu_2dscl_reg_263 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_100_OFFSET                      0x41c

/* REGISTER dpu_2dscl_reg_264 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_101_OFFSET                      0x420

/* REGISTER dpu_2dscl_reg_265 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_102_OFFSET                      0x424

/* REGISTER dpu_2dscl_reg_266 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_103_OFFSET                      0x428

/* REGISTER dpu_2dscl_reg_267 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_104_OFFSET                      0x42c

/* REGISTER dpu_2dscl_reg_268 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_105_OFFSET                      0x430

/* REGISTER dpu_2dscl_reg_269 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_106_OFFSET                      0x434

/* REGISTER dpu_2dscl_reg_270 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_107_OFFSET                      0x438

/* REGISTER dpu_2dscl_reg_271 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_108_OFFSET                      0x43c

/* REGISTER dpu_2dscl_reg_272 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_109_OFFSET                      0x440

/* REGISTER dpu_2dscl_reg_273 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_110_OFFSET                      0x444

/* REGISTER dpu_2dscl_reg_274 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_111_OFFSET                      0x448

/* REGISTER dpu_2dscl_reg_275 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_112_OFFSET                      0x44c

/* REGISTER dpu_2dscl_reg_276 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_113_OFFSET                      0x450

/* REGISTER dpu_2dscl_reg_277 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_114_OFFSET                      0x454

/* REGISTER dpu_2dscl_reg_278 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_115_OFFSET                      0x458

/* REGISTER dpu_2dscl_reg_279 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_116_OFFSET                      0x45c

/* REGISTER dpu_2dscl_reg_280 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_117_OFFSET                      0x460

/* REGISTER dpu_2dscl_reg_281 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_118_OFFSET                      0x464

/* REGISTER dpu_2dscl_reg_282 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_119_OFFSET                      0x468

/* REGISTER dpu_2dscl_reg_283 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_120_OFFSET                      0x46c

/* REGISTER dpu_2dscl_reg_284 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_121_OFFSET                      0x470

/* REGISTER dpu_2dscl_reg_285 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_122_OFFSET                      0x474

/* REGISTER dpu_2dscl_reg_286 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_123_OFFSET                      0x478

/* REGISTER dpu_2dscl_reg_287 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_124_OFFSET                      0x47c

/* REGISTER dpu_2dscl_reg_288 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_125_OFFSET                      0x480

/* REGISTER dpu_2dscl_reg_289 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_126_OFFSET                      0x484

/* REGISTER dpu_2dscl_reg_290 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_127_OFFSET                      0x488

/* REGISTER dpu_2dscl_reg_291 */
#define ARRAY_LANCZOS3_KERNEL_COMBINE_128_OFFSET                      0x48c

/* REGISTER dpu_2dscl_reg_292 */
#define STRIPE_INIT_PHASE_OFFSET                                      0x490
#define STRIPE_INIT_PHASE_SHIFT                                       0
#define STRIPE_INIT_PHASE_LENGTH                                      26

/* REGISTER dpu_2dscl_reg_293 */
#define CROP_WIDTH_OUT_PHASE_OFFSET                                   0x494
#define CROP_WIDTH_OUT_PHASE_SHIFT                                    0
#define CROP_WIDTH_OUT_PHASE_LENGTH                                   30

/* REGISTER dpu_2dscl_reg_294 */
#define CROP_HEIGHT_OUT_PHASE_OFFSET                                  0x498
#define CROP_HEIGHT_OUT_PHASE_SHIFT                                   0
#define CROP_HEIGHT_OUT_PHASE_LENGTH                                  29

/* REGISTER dpu_2dscl_reg_295 */
#define CG1_AUTO_EN_OFFSET                                            0x49c
#define CG1_AUTO_EN_SHIFT                                             0
#define CG1_AUTO_EN_LENGTH                                            1
#define CG2_AUTO_EN_SHIFT                                             1
#define CG2_AUTO_EN_LENGTH                                            1
#define CG3_AUTO_EN_SHIFT                                             2
#define CG3_AUTO_EN_LENGTH                                            1

/* REGISTER dpu_2dscl_reg_296 */
#define SD_EXIT_CNT_OFFSET                                            0x4a0
#define SD_EXIT_CNT_SHIFT                                             0
#define SD_EXIT_CNT_LENGTH                                            16

/* REGISTER dpu_2dscl_reg_297 */
#define DPU_2DSCL_MEM_LP_AUTO_EN_OFFSET                               0x4a4
#define DPU_2DSCL_MEM_LP_AUTO_EN_SHIFT                                0
#define DPU_2DSCL_MEM_LP_AUTO_EN_LENGTH                               1
#define EXTRA_LP_AUTO_EN_SHIFT                                        1
#define EXTRA_LP_AUTO_EN_LENGTH                                       1

/* REGISTER dpu_2dscl_reg_298 */
#define DEBUG_CNT_EN_OFFSET                                           0x4a8
#define DEBUG_CNT_EN_SHIFT                                            0
#define DEBUG_CNT_EN_LENGTH                                           1

/* REGISTER dpu_2dscl_reg_299 */
#define DEBUG_FRMID_CLR_OFFSET                                        0x4ac
#define DEBUG_FRMID_CLR_SHIFT                                         0
#define DEBUG_FRMID_CLR_LENGTH                                        1

/* REGISTER dpu_2dscl_reg_300 */
#define DEBUG_FIFO_FULL_CLR_OFFSET                                    0x4b0
#define DEBUG_FIFO_FULL_CLR_SHIFT                                     0
#define DEBUG_FIFO_FULL_CLR_LENGTH                                    1

/* REGISTER dpu_2dscl_reg_301 */
#define DEBUG_SR_STATUS0_OFFSET                                       0x4b4
#define DEBUG_SR_STATUS0_SHIFT                                        0
#define DEBUG_SR_STATUS0_LENGTH                                       10
#define DEBUG_SR_STATUS1_SHIFT                                        16
#define DEBUG_SR_STATUS1_LENGTH                                       14

/* REGISTER dpu_2dscl_reg_302 */
#define DEBUG_SR_STATUS2_OFFSET                                       0x4b8
#define DEBUG_SR_STATUS2_SHIFT                                        0
#define DEBUG_SR_STATUS2_LENGTH                                       28

/* REGISTER dpu_2dscl_reg_303 */
#define DEBUG_SR_STATUS3_OFFSET                                       0x4bc
#define DEBUG_SR_STATUS3_SHIFT                                        0
#define DEBUG_SR_STATUS3_LENGTH                                       29

/* REGISTER dpu_2dscl_reg_304 */
#define DEBUG_SR_STATUS4_OFFSET                                       0x4c0
#define DEBUG_SR_STATUS4_SHIFT                                        0
#define DEBUG_SR_STATUS4_LENGTH                                       7

/* REGISTER dpu_2dscl_reg_305 */
#define DEBUG_SRC_IMG_ROW_COL_ID_OFFSET                               0x4c4
#define DEBUG_SRC_IMG_ROW_COL_ID_SHIFT                                0
#define DEBUG_SRC_IMG_ROW_COL_ID_LENGTH                               29

/* REGISTER dpu_2dscl_reg_306 */
#define DEBUG_SRC_IMG_FRM_ID_OFFSET                                   0x4c8

/* REGISTER dpu_2dscl_reg_307 */
#define DEBUG_LAYSUM_IMG_ROW_COL_ID_OFFSET                            0x4cc
#define DEBUG_LAYSUM_IMG_ROW_COL_ID_SHIFT                             0
#define DEBUG_LAYSUM_IMG_ROW_COL_ID_LENGTH                            29

/* REGISTER dpu_2dscl_reg_308 */
#define DEBUG_LAYSUM_IMG_FRM_ID_OFFSET                                0x4d0

/* REGISTER dpu_2dscl_reg_309 */
#define DEBUG_ORI_IMG_ROW_COL_ID_OFFSET                               0x4d4
#define DEBUG_ORI_IMG_ROW_COL_ID_SHIFT                                0
#define DEBUG_ORI_IMG_ROW_COL_ID_LENGTH                               29

/* REGISTER dpu_2dscl_reg_310 */
#define DEBUG_ORI_IMG_FRM_ID_OFFSET                                   0x4d8

/* REGISTER dpu_2dscl_reg_311 */
#define DEBUG_YORILD_IMG_ROW_COL_ID_OFFSET                            0x4dc
#define DEBUG_YORILD_IMG_ROW_COL_ID_SHIFT                             0
#define DEBUG_YORILD_IMG_ROW_COL_ID_LENGTH                            29

/* REGISTER dpu_2dscl_reg_312 */
#define DEBUG_YORILD_IMG_FRM_ID_OFFSET                                0x4e0

/* REGISTER dpu_2dscl_reg_313 */
#define DEBUG_DST_IMG_ROW_COL_ID_OFFSET                               0x4e4
#define DEBUG_DST_IMG_ROW_COL_ID_SHIFT                                0
#define DEBUG_DST_IMG_ROW_COL_ID_LENGTH                               29

/* REGISTER dpu_2dscl_reg_314 */
#define DEBUG_DST_IMG_FRM_ID_OFFSET                                   0x4e8

/* REGISTER dpu_2dscl_reg_315 */
#define CFG_SE2_OFFSET                                                0x4ec
#define CFG_SE2_SHIFT                                                 0
#define CFG_SE2_LENGTH                                                1

/* REGISTER dpu_2dscl_reg_316 */
#define DPU_2DSCL_FORCE_UPDATE_EN_OFFSET                              0x4f0
#define DPU_2DSCL_FORCE_UPDATE_EN_SHIFT                               0
#define DPU_2DSCL_FORCE_UPDATE_EN_LENGTH                              1
#define DPU_2DSCL_VSYNC_UPDATE_EN_SHIFT                               1
#define DPU_2DSCL_VSYNC_UPDATE_EN_LENGTH                              1
#define DPU_2DSCL_SHADOW_READ_EN_SHIFT                                2
#define DPU_2DSCL_SHADOW_READ_EN_LENGTH                               1

/* REGISTER dpu_2dscl_reg_317 */
#define DPU_2DSCL_FORCE_UPDATE_PULSE_OFFSET                           0x4f4
#define DPU_2DSCL_FORCE_UPDATE_PULSE_SHIFT                            0
#define DPU_2DSCL_FORCE_UPDATE_PULSE_LENGTH                           1

/* REGISTER dpu_2dscl_reg_318 */
#define DPU_2DSCL_FORCE_UPDATE_EN_SE_OFFSET                           0x4f8
#define DPU_2DSCL_FORCE_UPDATE_EN_SE_SHIFT                            0
#define DPU_2DSCL_FORCE_UPDATE_EN_SE_LENGTH                           1
#define DPU_2DSCL_VSYNC_UPDATE_EN_SE_SHIFT                            1
#define DPU_2DSCL_VSYNC_UPDATE_EN_SE_LENGTH                           1
#define DPU_2DSCL_SHADOW_READ_EN_SE_SHIFT                             2
#define DPU_2DSCL_SHADOW_READ_EN_SE_LENGTH                            1

/* REGISTER dpu_2dscl_reg_319 */
#define DPU_2DSCL_FORCE_UPDATE_PULSE_SE_OFFSET                        0x4fc
#define DPU_2DSCL_FORCE_UPDATE_PULSE_SE_SHIFT                         0
#define DPU_2DSCL_FORCE_UPDATE_PULSE_SE_LENGTH                        1

/* REGISTER dpu_2dscl_reg_320 */
#define DPU_2DSCL_ICG_OVERRIDE_OFFSET                                 0x500
#define DPU_2DSCL_ICG_OVERRIDE_SHIFT                                  0
#define DPU_2DSCL_ICG_OVERRIDE_LENGTH                                 1

/* REGISTER dpu_2dscl_reg_321 */
#define DPU_2DSCL_TRIGGER_OFFSET                                      0x504
#define DPU_2DSCL_TRIGGER_SHIFT                                       0
#define DPU_2DSCL_TRIGGER_LENGTH                                      1

/* REGISTER dpu_2dscl_reg_322 */
#define DPU_2DSCL_TRIGGER2_OFFSET                                     0x508
#define DPU_2DSCL_TRIGGER2_SHIFT                                      0
#define DPU_2DSCL_TRIGGER2_LENGTH                                     1

#endif
