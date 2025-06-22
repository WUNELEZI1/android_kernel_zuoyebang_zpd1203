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

#ifndef _DPU_HW_WB_SCALER_REG_H_
#define _DPU_HW_WB_SCALER_REG_H_

/* REGISTER scaler_8x6tap_reg_0 */
#define M_NSCL_HOR_ENABLE_OFFSET                                      0x00
#define M_NSCL_HOR_ENABLE_SHIFT                                       0
#define M_NSCL_HOR_ENABLE_LENGTH                                      1
#define M_NSCL_VER_ENABLE_SHIFT                                       1
#define M_NSCL_VER_ENABLE_LENGTH                                      1
#define M_NYUV_NARROW_SHIFT                                           2
#define M_NYUV_NARROW_LENGTH                                          1

/* REGISTER scaler_8x6tap_reg_1 */
#define M_NSCL_INPUT_WIDTH_OFFSET                                     0x04
#define M_NSCL_INPUT_WIDTH_SHIFT                                      0
#define M_NSCL_INPUT_WIDTH_LENGTH                                     16
#define M_NSCL_INPUT_HEIGHT_SHIFT                                     16
#define M_NSCL_INPUT_HEIGHT_LENGTH                                    16

/* REGISTER scaler_8x6tap_reg_2 */
#define M_NSCL_OUTPUT_WIDTH_OFFSET                                    0x08
#define M_NSCL_OUTPUT_WIDTH_SHIFT                                     0
#define M_NSCL_OUTPUT_WIDTH_LENGTH                                    16
#define M_NSCL_OUTPUT_HEIGHT_SHIFT                                    16
#define M_NSCL_OUTPUT_HEIGHT_LENGTH                                   16

/* REGISTER scaler_8x6tap_reg_3 */
#define M_NSCL_HOR_INIT_PHASE_L32B_OFFSET                             0x0c

/* REGISTER scaler_8x6tap_reg_4 */
#define M_NSCL_HOR_INIT_PHASE_H1B_OFFSET                              0x10
#define M_NSCL_HOR_INIT_PHASE_H1B_SHIFT                               0
#define M_NSCL_HOR_INIT_PHASE_H1B_LENGTH                              1

/* REGISTER scaler_8x6tap_reg_5 */
#define M_NSCL_VER_INIT_PHASE_L32B_OFFSET                             0x14

/* REGISTER scaler_8x6tap_reg_6 */
#define M_NSCL_VER_INIT_PHASE_H1B_OFFSET                              0x18
#define M_NSCL_VER_INIT_PHASE_H1B_SHIFT                               0
#define M_NSCL_VER_INIT_PHASE_H1B_LENGTH                              1

/* REGISTER scaler_8x6tap_reg_7 */
#define M_NSCL_HOR_DELTA_PHASE_OFFSET                                 0x1c
#define M_NSCL_HOR_DELTA_PHASE_SHIFT                                  0
#define M_NSCL_HOR_DELTA_PHASE_LENGTH                                 20

/* REGISTER scaler_8x6tap_reg_8 */
#define M_NSCL_VER_DELTA_PHASE_OFFSET                                 0x20
#define M_NSCL_VER_DELTA_PHASE_SHIFT                                  0
#define M_NSCL_VER_DELTA_PHASE_LENGTH                                 20

/* REGISTER scaler_8x6tap_reg_9 */
#define M_NSCL_HOR_COEF0_OFFSET                                       0x24
#define M_NSCL_HOR_COEF0_SHIFT                                        0
#define M_NSCL_HOR_COEF0_LENGTH                                       16
#define M_NSCL_HOR_COEF1_SHIFT                                        16
#define M_NSCL_HOR_COEF1_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_10 */
#define M_NSCL_HOR_COEF2_OFFSET                                       0x28
#define M_NSCL_HOR_COEF2_SHIFT                                        0
#define M_NSCL_HOR_COEF2_LENGTH                                       16
#define M_NSCL_HOR_COEF3_SHIFT                                        16
#define M_NSCL_HOR_COEF3_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_11 */
#define M_NSCL_HOR_COEF4_OFFSET                                       0x2c
#define M_NSCL_HOR_COEF4_SHIFT                                        0
#define M_NSCL_HOR_COEF4_LENGTH                                       16
#define M_NSCL_HOR_COEF5_SHIFT                                        16
#define M_NSCL_HOR_COEF5_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_12 */
#define M_NSCL_HOR_COEF6_OFFSET                                       0x30
#define M_NSCL_HOR_COEF6_SHIFT                                        0
#define M_NSCL_HOR_COEF6_LENGTH                                       16
#define M_NSCL_HOR_COEF7_SHIFT                                        16
#define M_NSCL_HOR_COEF7_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_13 */
#define M_NSCL_HOR_COEF8_OFFSET                                       0x34
#define M_NSCL_HOR_COEF8_SHIFT                                        0
#define M_NSCL_HOR_COEF8_LENGTH                                       16
#define M_NSCL_HOR_COEF9_SHIFT                                        16
#define M_NSCL_HOR_COEF9_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_14 */
#define M_NSCL_HOR_COEF10_OFFSET                                      0x38
#define M_NSCL_HOR_COEF10_SHIFT                                       0
#define M_NSCL_HOR_COEF10_LENGTH                                      16
#define M_NSCL_HOR_COEF11_SHIFT                                       16
#define M_NSCL_HOR_COEF11_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_15 */
#define M_NSCL_HOR_COEF12_OFFSET                                      0x3c
#define M_NSCL_HOR_COEF12_SHIFT                                       0
#define M_NSCL_HOR_COEF12_LENGTH                                      16
#define M_NSCL_HOR_COEF13_SHIFT                                       16
#define M_NSCL_HOR_COEF13_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_16 */
#define M_NSCL_HOR_COEF14_OFFSET                                      0x40
#define M_NSCL_HOR_COEF14_SHIFT                                       0
#define M_NSCL_HOR_COEF14_LENGTH                                      16
#define M_NSCL_HOR_COEF15_SHIFT                                       16
#define M_NSCL_HOR_COEF15_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_17 */
#define M_NSCL_HOR_COEF16_OFFSET                                      0x44
#define M_NSCL_HOR_COEF16_SHIFT                                       0
#define M_NSCL_HOR_COEF16_LENGTH                                      16
#define M_NSCL_HOR_COEF17_SHIFT                                       16
#define M_NSCL_HOR_COEF17_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_18 */
#define M_NSCL_HOR_COEF18_OFFSET                                      0x48
#define M_NSCL_HOR_COEF18_SHIFT                                       0
#define M_NSCL_HOR_COEF18_LENGTH                                      16
#define M_NSCL_HOR_COEF19_SHIFT                                       16
#define M_NSCL_HOR_COEF19_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_19 */
#define M_NSCL_HOR_COEF20_OFFSET                                      0x4c
#define M_NSCL_HOR_COEF20_SHIFT                                       0
#define M_NSCL_HOR_COEF20_LENGTH                                      16
#define M_NSCL_HOR_COEF21_SHIFT                                       16
#define M_NSCL_HOR_COEF21_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_20 */
#define M_NSCL_HOR_COEF22_OFFSET                                      0x50
#define M_NSCL_HOR_COEF22_SHIFT                                       0
#define M_NSCL_HOR_COEF22_LENGTH                                      16
#define M_NSCL_HOR_COEF23_SHIFT                                       16
#define M_NSCL_HOR_COEF23_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_21 */
#define M_NSCL_HOR_COEF24_OFFSET                                      0x54
#define M_NSCL_HOR_COEF24_SHIFT                                       0
#define M_NSCL_HOR_COEF24_LENGTH                                      16
#define M_NSCL_HOR_COEF25_SHIFT                                       16
#define M_NSCL_HOR_COEF25_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_22 */
#define M_NSCL_HOR_COEF26_OFFSET                                      0x58
#define M_NSCL_HOR_COEF26_SHIFT                                       0
#define M_NSCL_HOR_COEF26_LENGTH                                      16
#define M_NSCL_HOR_COEF27_SHIFT                                       16
#define M_NSCL_HOR_COEF27_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_23 */
#define M_NSCL_HOR_COEF28_OFFSET                                      0x5c
#define M_NSCL_HOR_COEF28_SHIFT                                       0
#define M_NSCL_HOR_COEF28_LENGTH                                      16
#define M_NSCL_HOR_COEF29_SHIFT                                       16
#define M_NSCL_HOR_COEF29_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_24 */
#define M_NSCL_HOR_COEF30_OFFSET                                      0x60
#define M_NSCL_HOR_COEF30_SHIFT                                       0
#define M_NSCL_HOR_COEF30_LENGTH                                      16
#define M_NSCL_HOR_COEF31_SHIFT                                       16
#define M_NSCL_HOR_COEF31_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_25 */
#define M_NSCL_HOR_COEF32_OFFSET                                      0x64
#define M_NSCL_HOR_COEF32_SHIFT                                       0
#define M_NSCL_HOR_COEF32_LENGTH                                      16
#define M_NSCL_HOR_COEF33_SHIFT                                       16
#define M_NSCL_HOR_COEF33_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_26 */
#define M_NSCL_HOR_COEF34_OFFSET                                      0x68
#define M_NSCL_HOR_COEF34_SHIFT                                       0
#define M_NSCL_HOR_COEF34_LENGTH                                      16
#define M_NSCL_HOR_COEF35_SHIFT                                       16
#define M_NSCL_HOR_COEF35_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_27 */
#define M_NSCL_HOR_COEF36_OFFSET                                      0x6c
#define M_NSCL_HOR_COEF36_SHIFT                                       0
#define M_NSCL_HOR_COEF36_LENGTH                                      16
#define M_NSCL_HOR_COEF37_SHIFT                                       16
#define M_NSCL_HOR_COEF37_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_28 */
#define M_NSCL_HOR_COEF38_OFFSET                                      0x70
#define M_NSCL_HOR_COEF38_SHIFT                                       0
#define M_NSCL_HOR_COEF38_LENGTH                                      16
#define M_NSCL_HOR_COEF39_SHIFT                                       16
#define M_NSCL_HOR_COEF39_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_29 */
#define M_NSCL_HOR_COEF40_OFFSET                                      0x74
#define M_NSCL_HOR_COEF40_SHIFT                                       0
#define M_NSCL_HOR_COEF40_LENGTH                                      16
#define M_NSCL_HOR_COEF41_SHIFT                                       16
#define M_NSCL_HOR_COEF41_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_30 */
#define M_NSCL_HOR_COEF42_OFFSET                                      0x78
#define M_NSCL_HOR_COEF42_SHIFT                                       0
#define M_NSCL_HOR_COEF42_LENGTH                                      16
#define M_NSCL_HOR_COEF43_SHIFT                                       16
#define M_NSCL_HOR_COEF43_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_31 */
#define M_NSCL_HOR_COEF44_OFFSET                                      0x7c
#define M_NSCL_HOR_COEF44_SHIFT                                       0
#define M_NSCL_HOR_COEF44_LENGTH                                      16
#define M_NSCL_HOR_COEF45_SHIFT                                       16
#define M_NSCL_HOR_COEF45_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_32 */
#define M_NSCL_HOR_COEF46_OFFSET                                      0x80
#define M_NSCL_HOR_COEF46_SHIFT                                       0
#define M_NSCL_HOR_COEF46_LENGTH                                      16
#define M_NSCL_HOR_COEF47_SHIFT                                       16
#define M_NSCL_HOR_COEF47_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_33 */
#define M_NSCL_HOR_COEF48_OFFSET                                      0x84
#define M_NSCL_HOR_COEF48_SHIFT                                       0
#define M_NSCL_HOR_COEF48_LENGTH                                      16
#define M_NSCL_HOR_COEF49_SHIFT                                       16
#define M_NSCL_HOR_COEF49_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_34 */
#define M_NSCL_HOR_COEF50_OFFSET                                      0x88
#define M_NSCL_HOR_COEF50_SHIFT                                       0
#define M_NSCL_HOR_COEF50_LENGTH                                      16
#define M_NSCL_HOR_COEF51_SHIFT                                       16
#define M_NSCL_HOR_COEF51_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_35 */
#define M_NSCL_HOR_COEF52_OFFSET                                      0x8c
#define M_NSCL_HOR_COEF52_SHIFT                                       0
#define M_NSCL_HOR_COEF52_LENGTH                                      16
#define M_NSCL_HOR_COEF53_SHIFT                                       16
#define M_NSCL_HOR_COEF53_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_36 */
#define M_NSCL_HOR_COEF54_OFFSET                                      0x90
#define M_NSCL_HOR_COEF54_SHIFT                                       0
#define M_NSCL_HOR_COEF54_LENGTH                                      16
#define M_NSCL_HOR_COEF55_SHIFT                                       16
#define M_NSCL_HOR_COEF55_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_37 */
#define M_NSCL_HOR_COEF56_OFFSET                                      0x94
#define M_NSCL_HOR_COEF56_SHIFT                                       0
#define M_NSCL_HOR_COEF56_LENGTH                                      16
#define M_NSCL_HOR_COEF57_SHIFT                                       16
#define M_NSCL_HOR_COEF57_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_38 */
#define M_NSCL_HOR_COEF58_OFFSET                                      0x98
#define M_NSCL_HOR_COEF58_SHIFT                                       0
#define M_NSCL_HOR_COEF58_LENGTH                                      16
#define M_NSCL_HOR_COEF59_SHIFT                                       16
#define M_NSCL_HOR_COEF59_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_39 */
#define M_NSCL_HOR_COEF60_OFFSET                                      0x9c
#define M_NSCL_HOR_COEF60_SHIFT                                       0
#define M_NSCL_HOR_COEF60_LENGTH                                      16
#define M_NSCL_HOR_COEF61_SHIFT                                       16
#define M_NSCL_HOR_COEF61_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_40 */
#define M_NSCL_HOR_COEF62_OFFSET                                      0xa0
#define M_NSCL_HOR_COEF62_SHIFT                                       0
#define M_NSCL_HOR_COEF62_LENGTH                                      16
#define M_NSCL_HOR_COEF63_SHIFT                                       16
#define M_NSCL_HOR_COEF63_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_41 */
#define M_NSCL_HOR_COEF64_OFFSET                                      0xa4
#define M_NSCL_HOR_COEF64_SHIFT                                       0
#define M_NSCL_HOR_COEF64_LENGTH                                      16
#define M_NSCL_HOR_COEF65_SHIFT                                       16
#define M_NSCL_HOR_COEF65_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_42 */
#define M_NSCL_HOR_COEF66_OFFSET                                      0xa8
#define M_NSCL_HOR_COEF66_SHIFT                                       0
#define M_NSCL_HOR_COEF66_LENGTH                                      16
#define M_NSCL_HOR_COEF67_SHIFT                                       16
#define M_NSCL_HOR_COEF67_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_43 */
#define M_NSCL_HOR_COEF68_OFFSET                                      0xac
#define M_NSCL_HOR_COEF68_SHIFT                                       0
#define M_NSCL_HOR_COEF68_LENGTH                                      16
#define M_NSCL_HOR_COEF69_SHIFT                                       16
#define M_NSCL_HOR_COEF69_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_44 */
#define M_NSCL_HOR_COEF70_OFFSET                                      0xb0
#define M_NSCL_HOR_COEF70_SHIFT                                       0
#define M_NSCL_HOR_COEF70_LENGTH                                      16
#define M_NSCL_HOR_COEF71_SHIFT                                       16
#define M_NSCL_HOR_COEF71_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_45 */
#define M_NSCL_HOR_COEF72_OFFSET                                      0xb4
#define M_NSCL_HOR_COEF72_SHIFT                                       0
#define M_NSCL_HOR_COEF72_LENGTH                                      16
#define M_NSCL_HOR_COEF73_SHIFT                                       16
#define M_NSCL_HOR_COEF73_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_46 */
#define M_NSCL_HOR_COEF74_OFFSET                                      0xb8
#define M_NSCL_HOR_COEF74_SHIFT                                       0
#define M_NSCL_HOR_COEF74_LENGTH                                      16
#define M_NSCL_HOR_COEF75_SHIFT                                       16
#define M_NSCL_HOR_COEF75_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_47 */
#define M_NSCL_HOR_COEF76_OFFSET                                      0xbc
#define M_NSCL_HOR_COEF76_SHIFT                                       0
#define M_NSCL_HOR_COEF76_LENGTH                                      16
#define M_NSCL_HOR_COEF77_SHIFT                                       16
#define M_NSCL_HOR_COEF77_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_48 */
#define M_NSCL_HOR_COEF78_OFFSET                                      0xc0
#define M_NSCL_HOR_COEF78_SHIFT                                       0
#define M_NSCL_HOR_COEF78_LENGTH                                      16
#define M_NSCL_HOR_COEF79_SHIFT                                       16
#define M_NSCL_HOR_COEF79_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_49 */
#define M_NSCL_HOR_COEF80_OFFSET                                      0xc4
#define M_NSCL_HOR_COEF80_SHIFT                                       0
#define M_NSCL_HOR_COEF80_LENGTH                                      16
#define M_NSCL_HOR_COEF81_SHIFT                                       16
#define M_NSCL_HOR_COEF81_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_50 */
#define M_NSCL_HOR_COEF82_OFFSET                                      0xc8
#define M_NSCL_HOR_COEF82_SHIFT                                       0
#define M_NSCL_HOR_COEF82_LENGTH                                      16
#define M_NSCL_HOR_COEF83_SHIFT                                       16
#define M_NSCL_HOR_COEF83_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_51 */
#define M_NSCL_HOR_COEF84_OFFSET                                      0xcc
#define M_NSCL_HOR_COEF84_SHIFT                                       0
#define M_NSCL_HOR_COEF84_LENGTH                                      16
#define M_NSCL_HOR_COEF85_SHIFT                                       16
#define M_NSCL_HOR_COEF85_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_52 */
#define M_NSCL_HOR_COEF86_OFFSET                                      0xd0
#define M_NSCL_HOR_COEF86_SHIFT                                       0
#define M_NSCL_HOR_COEF86_LENGTH                                      16
#define M_NSCL_HOR_COEF87_SHIFT                                       16
#define M_NSCL_HOR_COEF87_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_53 */
#define M_NSCL_HOR_COEF88_OFFSET                                      0xd4
#define M_NSCL_HOR_COEF88_SHIFT                                       0
#define M_NSCL_HOR_COEF88_LENGTH                                      16
#define M_NSCL_HOR_COEF89_SHIFT                                       16
#define M_NSCL_HOR_COEF89_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_54 */
#define M_NSCL_HOR_COEF90_OFFSET                                      0xd8
#define M_NSCL_HOR_COEF90_SHIFT                                       0
#define M_NSCL_HOR_COEF90_LENGTH                                      16
#define M_NSCL_HOR_COEF91_SHIFT                                       16
#define M_NSCL_HOR_COEF91_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_55 */
#define M_NSCL_HOR_COEF92_OFFSET                                      0xdc
#define M_NSCL_HOR_COEF92_SHIFT                                       0
#define M_NSCL_HOR_COEF92_LENGTH                                      16
#define M_NSCL_HOR_COEF93_SHIFT                                       16
#define M_NSCL_HOR_COEF93_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_56 */
#define M_NSCL_HOR_COEF94_OFFSET                                      0xe0
#define M_NSCL_HOR_COEF94_SHIFT                                       0
#define M_NSCL_HOR_COEF94_LENGTH                                      16
#define M_NSCL_HOR_COEF95_SHIFT                                       16
#define M_NSCL_HOR_COEF95_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_57 */
#define M_NSCL_HOR_COEF96_OFFSET                                      0xe4
#define M_NSCL_HOR_COEF96_SHIFT                                       0
#define M_NSCL_HOR_COEF96_LENGTH                                      16
#define M_NSCL_HOR_COEF97_SHIFT                                       16
#define M_NSCL_HOR_COEF97_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_58 */
#define M_NSCL_HOR_COEF98_OFFSET                                      0xe8
#define M_NSCL_HOR_COEF98_SHIFT                                       0
#define M_NSCL_HOR_COEF98_LENGTH                                      16
#define M_NSCL_HOR_COEF99_SHIFT                                       16
#define M_NSCL_HOR_COEF99_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_59 */
#define M_NSCL_HOR_COEF100_OFFSET                                     0xec
#define M_NSCL_HOR_COEF100_SHIFT                                      0
#define M_NSCL_HOR_COEF100_LENGTH                                     16
#define M_NSCL_HOR_COEF101_SHIFT                                      16
#define M_NSCL_HOR_COEF101_LENGTH                                     16

/* REGISTER scaler_8x6tap_reg_60 */
#define M_NSCL_HOR_COEF102_OFFSET                                     0xf0
#define M_NSCL_HOR_COEF102_SHIFT                                      0
#define M_NSCL_HOR_COEF102_LENGTH                                     16
#define M_NSCL_HOR_COEF103_SHIFT                                      16
#define M_NSCL_HOR_COEF103_LENGTH                                     16

/* REGISTER scaler_8x6tap_reg_61 */
#define M_NSCL_HOR_COEF104_OFFSET                                     0xf4
#define M_NSCL_HOR_COEF104_SHIFT                                      0
#define M_NSCL_HOR_COEF104_LENGTH                                     16
#define M_NSCL_HOR_COEF105_SHIFT                                      16
#define M_NSCL_HOR_COEF105_LENGTH                                     16

/* REGISTER scaler_8x6tap_reg_62 */
#define M_NSCL_HOR_COEF106_OFFSET                                     0xf8
#define M_NSCL_HOR_COEF106_SHIFT                                      0
#define M_NSCL_HOR_COEF106_LENGTH                                     16
#define M_NSCL_HOR_COEF107_SHIFT                                      16
#define M_NSCL_HOR_COEF107_LENGTH                                     16

/* REGISTER scaler_8x6tap_reg_63 */
#define M_NSCL_HOR_COEF108_OFFSET                                     0xfc
#define M_NSCL_HOR_COEF108_SHIFT                                      0
#define M_NSCL_HOR_COEF108_LENGTH                                     16
#define M_NSCL_HOR_COEF109_SHIFT                                      16
#define M_NSCL_HOR_COEF109_LENGTH                                     16

/* REGISTER scaler_8x6tap_reg_64 */
#define M_NSCL_HOR_COEF110_OFFSET                                     0x100
#define M_NSCL_HOR_COEF110_SHIFT                                      0
#define M_NSCL_HOR_COEF110_LENGTH                                     16
#define M_NSCL_HOR_COEF111_SHIFT                                      16
#define M_NSCL_HOR_COEF111_LENGTH                                     16

/* REGISTER scaler_8x6tap_reg_65 */
#define M_NSCL_VER_COEF0_OFFSET                                       0x104
#define M_NSCL_VER_COEF0_SHIFT                                        0
#define M_NSCL_VER_COEF0_LENGTH                                       16
#define M_NSCL_VER_COEF1_SHIFT                                        16
#define M_NSCL_VER_COEF1_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_66 */
#define M_NSCL_VER_COEF2_OFFSET                                       0x108
#define M_NSCL_VER_COEF2_SHIFT                                        0
#define M_NSCL_VER_COEF2_LENGTH                                       16
#define M_NSCL_VER_COEF3_SHIFT                                        16
#define M_NSCL_VER_COEF3_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_67 */
#define M_NSCL_VER_COEF4_OFFSET                                       0x10c
#define M_NSCL_VER_COEF4_SHIFT                                        0
#define M_NSCL_VER_COEF4_LENGTH                                       16
#define M_NSCL_VER_COEF5_SHIFT                                        16
#define M_NSCL_VER_COEF5_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_68 */
#define M_NSCL_VER_COEF6_OFFSET                                       0x110
#define M_NSCL_VER_COEF6_SHIFT                                        0
#define M_NSCL_VER_COEF6_LENGTH                                       16
#define M_NSCL_VER_COEF7_SHIFT                                        16
#define M_NSCL_VER_COEF7_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_69 */
#define M_NSCL_VER_COEF8_OFFSET                                       0x114
#define M_NSCL_VER_COEF8_SHIFT                                        0
#define M_NSCL_VER_COEF8_LENGTH                                       16
#define M_NSCL_VER_COEF9_SHIFT                                        16
#define M_NSCL_VER_COEF9_LENGTH                                       16

/* REGISTER scaler_8x6tap_reg_70 */
#define M_NSCL_VER_COEF10_OFFSET                                      0x118
#define M_NSCL_VER_COEF10_SHIFT                                       0
#define M_NSCL_VER_COEF10_LENGTH                                      16
#define M_NSCL_VER_COEF11_SHIFT                                       16
#define M_NSCL_VER_COEF11_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_71 */
#define M_NSCL_VER_COEF12_OFFSET                                      0x11c
#define M_NSCL_VER_COEF12_SHIFT                                       0
#define M_NSCL_VER_COEF12_LENGTH                                      16
#define M_NSCL_VER_COEF13_SHIFT                                       16
#define M_NSCL_VER_COEF13_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_72 */
#define M_NSCL_VER_COEF14_OFFSET                                      0x120
#define M_NSCL_VER_COEF14_SHIFT                                       0
#define M_NSCL_VER_COEF14_LENGTH                                      16
#define M_NSCL_VER_COEF15_SHIFT                                       16
#define M_NSCL_VER_COEF15_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_73 */
#define M_NSCL_VER_COEF16_OFFSET                                      0x124
#define M_NSCL_VER_COEF16_SHIFT                                       0
#define M_NSCL_VER_COEF16_LENGTH                                      16
#define M_NSCL_VER_COEF17_SHIFT                                       16
#define M_NSCL_VER_COEF17_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_74 */
#define M_NSCL_VER_COEF18_OFFSET                                      0x128
#define M_NSCL_VER_COEF18_SHIFT                                       0
#define M_NSCL_VER_COEF18_LENGTH                                      16
#define M_NSCL_VER_COEF19_SHIFT                                       16
#define M_NSCL_VER_COEF19_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_75 */
#define M_NSCL_VER_COEF20_OFFSET                                      0x12c
#define M_NSCL_VER_COEF20_SHIFT                                       0
#define M_NSCL_VER_COEF20_LENGTH                                      16
#define M_NSCL_VER_COEF21_SHIFT                                       16
#define M_NSCL_VER_COEF21_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_76 */
#define M_NSCL_VER_COEF22_OFFSET                                      0x130
#define M_NSCL_VER_COEF22_SHIFT                                       0
#define M_NSCL_VER_COEF22_LENGTH                                      16
#define M_NSCL_VER_COEF23_SHIFT                                       16
#define M_NSCL_VER_COEF23_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_77 */
#define M_NSCL_VER_COEF24_OFFSET                                      0x134
#define M_NSCL_VER_COEF24_SHIFT                                       0
#define M_NSCL_VER_COEF24_LENGTH                                      16
#define M_NSCL_VER_COEF25_SHIFT                                       16
#define M_NSCL_VER_COEF25_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_78 */
#define M_NSCL_VER_COEF26_OFFSET                                      0x138
#define M_NSCL_VER_COEF26_SHIFT                                       0
#define M_NSCL_VER_COEF26_LENGTH                                      16
#define M_NSCL_VER_COEF27_SHIFT                                       16
#define M_NSCL_VER_COEF27_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_79 */
#define M_NSCL_VER_COEF28_OFFSET                                      0x13c
#define M_NSCL_VER_COEF28_SHIFT                                       0
#define M_NSCL_VER_COEF28_LENGTH                                      16
#define M_NSCL_VER_COEF29_SHIFT                                       16
#define M_NSCL_VER_COEF29_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_80 */
#define M_NSCL_VER_COEF30_OFFSET                                      0x140
#define M_NSCL_VER_COEF30_SHIFT                                       0
#define M_NSCL_VER_COEF30_LENGTH                                      16
#define M_NSCL_VER_COEF31_SHIFT                                       16
#define M_NSCL_VER_COEF31_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_81 */
#define M_NSCL_VER_COEF32_OFFSET                                      0x144
#define M_NSCL_VER_COEF32_SHIFT                                       0
#define M_NSCL_VER_COEF32_LENGTH                                      16
#define M_NSCL_VER_COEF33_SHIFT                                       16
#define M_NSCL_VER_COEF33_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_82 */
#define M_NSCL_VER_COEF34_OFFSET                                      0x148
#define M_NSCL_VER_COEF34_SHIFT                                       0
#define M_NSCL_VER_COEF34_LENGTH                                      16
#define M_NSCL_VER_COEF35_SHIFT                                       16
#define M_NSCL_VER_COEF35_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_83 */
#define M_NSCL_VER_COEF36_OFFSET                                      0x14c
#define M_NSCL_VER_COEF36_SHIFT                                       0
#define M_NSCL_VER_COEF36_LENGTH                                      16
#define M_NSCL_VER_COEF37_SHIFT                                       16
#define M_NSCL_VER_COEF37_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_84 */
#define M_NSCL_VER_COEF38_OFFSET                                      0x150
#define M_NSCL_VER_COEF38_SHIFT                                       0
#define M_NSCL_VER_COEF38_LENGTH                                      16
#define M_NSCL_VER_COEF39_SHIFT                                       16
#define M_NSCL_VER_COEF39_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_85 */
#define M_NSCL_VER_COEF40_OFFSET                                      0x154
#define M_NSCL_VER_COEF40_SHIFT                                       0
#define M_NSCL_VER_COEF40_LENGTH                                      16
#define M_NSCL_VER_COEF41_SHIFT                                       16
#define M_NSCL_VER_COEF41_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_86 */
#define M_NSCL_VER_COEF42_OFFSET                                      0x158
#define M_NSCL_VER_COEF42_SHIFT                                       0
#define M_NSCL_VER_COEF42_LENGTH                                      16
#define M_NSCL_VER_COEF43_SHIFT                                       16
#define M_NSCL_VER_COEF43_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_87 */
#define M_NSCL_VER_COEF44_OFFSET                                      0x15c
#define M_NSCL_VER_COEF44_SHIFT                                       0
#define M_NSCL_VER_COEF44_LENGTH                                      16
#define M_NSCL_VER_COEF45_SHIFT                                       16
#define M_NSCL_VER_COEF45_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_88 */
#define M_NSCL_VER_COEF46_OFFSET                                      0x160
#define M_NSCL_VER_COEF46_SHIFT                                       0
#define M_NSCL_VER_COEF46_LENGTH                                      16
#define M_NSCL_VER_COEF47_SHIFT                                       16
#define M_NSCL_VER_COEF47_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_89 */
#define M_NSCL_VER_COEF48_OFFSET                                      0x164
#define M_NSCL_VER_COEF48_SHIFT                                       0
#define M_NSCL_VER_COEF48_LENGTH                                      16
#define M_NSCL_VER_COEF49_SHIFT                                       16
#define M_NSCL_VER_COEF49_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_90 */
#define M_NSCL_VER_COEF50_OFFSET                                      0x168
#define M_NSCL_VER_COEF50_SHIFT                                       0
#define M_NSCL_VER_COEF50_LENGTH                                      16
#define M_NSCL_VER_COEF51_SHIFT                                       16
#define M_NSCL_VER_COEF51_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_91 */
#define M_NSCL_VER_COEF52_OFFSET                                      0x16c
#define M_NSCL_VER_COEF52_SHIFT                                       0
#define M_NSCL_VER_COEF52_LENGTH                                      16
#define M_NSCL_VER_COEF53_SHIFT                                       16
#define M_NSCL_VER_COEF53_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_92 */
#define M_NSCL_VER_COEF54_OFFSET                                      0x170
#define M_NSCL_VER_COEF54_SHIFT                                       0
#define M_NSCL_VER_COEF54_LENGTH                                      16
#define M_NSCL_VER_COEF55_SHIFT                                       16
#define M_NSCL_VER_COEF55_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_93 */
#define M_NSCL_VER_COEF56_OFFSET                                      0x174
#define M_NSCL_VER_COEF56_SHIFT                                       0
#define M_NSCL_VER_COEF56_LENGTH                                      16
#define M_NSCL_VER_COEF57_SHIFT                                       16
#define M_NSCL_VER_COEF57_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_94 */
#define M_NSCL_VER_COEF58_OFFSET                                      0x178
#define M_NSCL_VER_COEF58_SHIFT                                       0
#define M_NSCL_VER_COEF58_LENGTH                                      16
#define M_NSCL_VER_COEF59_SHIFT                                       16
#define M_NSCL_VER_COEF59_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_95 */
#define M_NSCL_VER_COEF60_OFFSET                                      0x17c
#define M_NSCL_VER_COEF60_SHIFT                                       0
#define M_NSCL_VER_COEF60_LENGTH                                      16
#define M_NSCL_VER_COEF61_SHIFT                                       16
#define M_NSCL_VER_COEF61_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_96 */
#define M_NSCL_VER_COEF62_OFFSET                                      0x180
#define M_NSCL_VER_COEF62_SHIFT                                       0
#define M_NSCL_VER_COEF62_LENGTH                                      16
#define M_NSCL_VER_COEF63_SHIFT                                       16
#define M_NSCL_VER_COEF63_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_97 */
#define M_NSCL_VER_COEF64_OFFSET                                      0x184
#define M_NSCL_VER_COEF64_SHIFT                                       0
#define M_NSCL_VER_COEF64_LENGTH                                      16
#define M_NSCL_VER_COEF65_SHIFT                                       16
#define M_NSCL_VER_COEF65_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_98 */
#define M_NSCL_VER_COEF66_OFFSET                                      0x188
#define M_NSCL_VER_COEF66_SHIFT                                       0
#define M_NSCL_VER_COEF66_LENGTH                                      16
#define M_NSCL_VER_COEF67_SHIFT                                       16
#define M_NSCL_VER_COEF67_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_99 */
#define M_NSCL_VER_COEF68_OFFSET                                      0x18c
#define M_NSCL_VER_COEF68_SHIFT                                       0
#define M_NSCL_VER_COEF68_LENGTH                                      16
#define M_NSCL_VER_COEF69_SHIFT                                       16
#define M_NSCL_VER_COEF69_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_100 */
#define M_NSCL_VER_COEF70_OFFSET                                      0x190
#define M_NSCL_VER_COEF70_SHIFT                                       0
#define M_NSCL_VER_COEF70_LENGTH                                      16
#define M_NSCL_VER_COEF71_SHIFT                                       16
#define M_NSCL_VER_COEF71_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_101 */
#define M_NSCL_VER_COEF72_OFFSET                                      0x194
#define M_NSCL_VER_COEF72_SHIFT                                       0
#define M_NSCL_VER_COEF72_LENGTH                                      16
#define M_NSCL_VER_COEF73_SHIFT                                       16
#define M_NSCL_VER_COEF73_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_102 */
#define M_NSCL_VER_COEF74_OFFSET                                      0x198
#define M_NSCL_VER_COEF74_SHIFT                                       0
#define M_NSCL_VER_COEF74_LENGTH                                      16
#define M_NSCL_VER_COEF75_SHIFT                                       16
#define M_NSCL_VER_COEF75_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_103 */
#define M_NSCL_VER_COEF76_OFFSET                                      0x19c
#define M_NSCL_VER_COEF76_SHIFT                                       0
#define M_NSCL_VER_COEF76_LENGTH                                      16
#define M_NSCL_VER_COEF77_SHIFT                                       16
#define M_NSCL_VER_COEF77_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_104 */
#define M_NSCL_VER_COEF78_OFFSET                                      0x1a0
#define M_NSCL_VER_COEF78_SHIFT                                       0
#define M_NSCL_VER_COEF78_LENGTH                                      16
#define M_NSCL_VER_COEF79_SHIFT                                       16
#define M_NSCL_VER_COEF79_LENGTH                                      16

/* REGISTER scaler_8x6tap_reg_121 */
#define SCALER_8X6TAP_FORCE_UPDATE_EN_OFFSET                          0x1e4
#define SCALER_8X6TAP_FORCE_UPDATE_EN_SHIFT                           0
#define SCALER_8X6TAP_FORCE_UPDATE_EN_LENGTH                          1
#define SCALER_8X6TAP_VSYNC_UPDATE_EN_SHIFT                           1
#define SCALER_8X6TAP_VSYNC_UPDATE_EN_LENGTH                          1
#define SCALER_8X6TAP_SHADOW_READ_EN_SHIFT                            2
#define SCALER_8X6TAP_SHADOW_READ_EN_LENGTH                           1

/* REGISTER scaler_8x6tap_reg_122 */
#define SCALER_8X6TAP_FORCE_UPDATE_PULSE_OFFSET                       0x1e8
#define SCALER_8X6TAP_FORCE_UPDATE_PULSE_SHIFT                        0
#define SCALER_8X6TAP_FORCE_UPDATE_PULSE_LENGTH                       1

/* REGISTER scaler_8x6tap_reg_123 */
#define SCALER_8X6TAP_ICG_OVERRIDE_OFFSET                             0x1ec
#define SCALER_8X6TAP_ICG_OVERRIDE_SHIFT                              0
#define SCALER_8X6TAP_ICG_OVERRIDE_LENGTH                             1

/* REGISTER scaler_8x6tap_reg_124 */
#define SCALER_8X6TAP_TRIGGER_OFFSET                                  0x1f0
#define SCALER_8X6TAP_TRIGGER_SHIFT                                   0
#define SCALER_8X6TAP_TRIGGER_LENGTH                                  1

/* REGISTER scaler_8x6tap_reg_125 */
#define SCALER_8X6TAP_TRIGGER2_OFFSET                                 0x1f4
#define SCALER_8X6TAP_TRIGGER2_SHIFT                                  0
#define SCALER_8X6TAP_TRIGGER2_LENGTH                                 1

#endif
