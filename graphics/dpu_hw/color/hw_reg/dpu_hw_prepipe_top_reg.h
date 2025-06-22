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
/* File generation time: 2023-10-19 10:11:22. */

#ifndef _DPU_HW_PREPIPE_TOP_REG_H_
#define _DPU_HW_PREPIPE_TOP_REG_H_

/* REGISTER saturn_v_prepipe_reg_0 */
#define V_PREPIPE_TOP_WB_RIGHT_SIGN_EN_OFFSET                         0x00
#define V_PREPIPE_TOP_WB_RIGHT_SIGN_EN_SHIFT                          0
#define V_PREPIPE_TOP_WB_RIGHT_SIGN_EN_LENGTH                         1
#define V_PREPIPE_TOP_ALPHA_DISABLE_SHIFT                             1
#define V_PREPIPE_TOP_ALPHA_DISABLE_LENGTH                            1
#define V_PREPIPE_TOP_R2Y_EN_SHIFT                                    2
#define V_PREPIPE_TOP_R2Y_EN_LENGTH                                   1
#define V_PREPIPE_TOP_Y2R_EN_SHIFT                                    3
#define V_PREPIPE_TOP_Y2R_EN_LENGTH                                   1
#define V_PREPIPE_TOP_NARROW_FULL_EN_SHIFT                            4
#define V_PREPIPE_TOP_NARROW_FULL_EN_LENGTH                           1
#define V_PREPIPE_TOP_SCALE_EN_SHIFT                                  5
#define V_PREPIPE_TOP_SCALE_EN_LENGTH                                 1
#define V_PREPIPE_TOP_DATA_FORMAT_SHIFT                               6
#define V_PREPIPE_TOP_DATA_FORMAT_LENGTH                              2
#define V_PREPIPE_TOP_SBS_EN_SHIFT                                    8
#define V_PREPIPE_TOP_SBS_EN_LENGTH                                   1


/* REGISTER saturn_v_prepipe_reg_16 */
#define PREALPHA_EN_OFFSET                                            0x40
#define PREALPHA_EN_SHIFT                                             0
#define PREALPHA_EN_LENGTH                                            1

/* REGISTER saturn_v_prepipe_reg_17 */
#define DPU_PREALPHA_CG2_AUTO_EN_OFFSET                               0x44
#define DPU_PREALPHA_CG2_AUTO_EN_SHIFT                                0
#define DPU_PREALPHA_CG2_AUTO_EN_LENGTH                               1

/* REGISTER saturn_v_prepipe_reg_18 */
#define V_PREPIPE_TOP_NON_PREALPHA_EN_OFFSET                          0x48
#define V_PREPIPE_TOP_NON_PREALPHA_EN_SHIFT                           0
#define V_PREPIPE_TOP_NON_PREALPHA_EN_LENGTH                          1

/* REGISTER saturn_v_prepipe_reg_19 */
#define V_PREPIPE_TOP_OFFSET_OFFSET                                   0x4c
#define V_PREPIPE_TOP_OFFSET_SHIFT                                    0
#define V_PREPIPE_TOP_OFFSET_LENGTH                                   10

/* REGISTER saturn_v_prepipe_reg_22 */
#define V_PREPIPE_TOP_DPU_NONPREALPHA_CG2_AUTO_EN_OFFSET              0x58
#define V_PREPIPE_TOP_DPU_NONPREALPHA_CG2_AUTO_EN_SHIFT               0
#define V_PREPIPE_TOP_DPU_NONPREALPHA_CG2_AUTO_EN_LENGTH              1

/* REGISTER saturn_v_prepipe_reg_23 */
#define V_PREPIPE_TOP_RGB2YUV_MATRIX00_OFFSET                         0x5c
#define V_PREPIPE_TOP_RGB2YUV_MATRIX00_SHIFT                          0
#define V_PREPIPE_TOP_RGB2YUV_MATRIX00_LENGTH                         16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX01_SHIFT                          16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX01_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_24 */
#define V_PREPIPE_TOP_RGB2YUV_MATRIX02_OFFSET                         0x60
#define V_PREPIPE_TOP_RGB2YUV_MATRIX02_SHIFT                          0
#define V_PREPIPE_TOP_RGB2YUV_MATRIX02_LENGTH                         16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX03_SHIFT                          16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX03_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_25 */
#define V_PREPIPE_TOP_RGB2YUV_MATRIX10_OFFSET                         0x64
#define V_PREPIPE_TOP_RGB2YUV_MATRIX10_SHIFT                          0
#define V_PREPIPE_TOP_RGB2YUV_MATRIX10_LENGTH                         16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX11_SHIFT                          16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX11_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_26 */
#define V_PREPIPE_TOP_RGB2YUV_MATRIX12_OFFSET                         0x68
#define V_PREPIPE_TOP_RGB2YUV_MATRIX12_SHIFT                          0
#define V_PREPIPE_TOP_RGB2YUV_MATRIX12_LENGTH                         16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX13_SHIFT                          16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX13_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_27 */
#define V_PREPIPE_TOP_RGB2YUV_MATRIX20_OFFSET                         0x6c
#define V_PREPIPE_TOP_RGB2YUV_MATRIX20_SHIFT                          0
#define V_PREPIPE_TOP_RGB2YUV_MATRIX20_LENGTH                         16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX21_SHIFT                          16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX21_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_28 */
#define V_PREPIPE_TOP_RGB2YUV_MATRIX22_OFFSET                         0x70
#define V_PREPIPE_TOP_RGB2YUV_MATRIX22_SHIFT                          0
#define V_PREPIPE_TOP_RGB2YUV_MATRIX22_LENGTH                         16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX23_SHIFT                          16
#define V_PREPIPE_TOP_RGB2YUV_MATRIX23_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_29 */
#define V_PREPIPE_TOP_YUV2RGB_MATRIX00_OFFSET                         0x74
#define V_PREPIPE_TOP_YUV2RGB_MATRIX00_SHIFT                          0
#define V_PREPIPE_TOP_YUV2RGB_MATRIX00_LENGTH                         16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX01_SHIFT                          16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX01_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_30 */
#define V_PREPIPE_TOP_YUV2RGB_MATRIX02_OFFSET                         0x78
#define V_PREPIPE_TOP_YUV2RGB_MATRIX02_SHIFT                          0
#define V_PREPIPE_TOP_YUV2RGB_MATRIX02_LENGTH                         16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX03_SHIFT                          16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX03_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_31 */
#define V_PREPIPE_TOP_YUV2RGB_MATRIX10_OFFSET                         0x7c
#define V_PREPIPE_TOP_YUV2RGB_MATRIX10_SHIFT                          0
#define V_PREPIPE_TOP_YUV2RGB_MATRIX10_LENGTH                         16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX11_SHIFT                          16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX11_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_32 */
#define V_PREPIPE_TOP_YUV2RGB_MATRIX12_OFFSET                         0x80
#define V_PREPIPE_TOP_YUV2RGB_MATRIX12_SHIFT                          0
#define V_PREPIPE_TOP_YUV2RGB_MATRIX12_LENGTH                         16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX13_SHIFT                          16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX13_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_33 */
#define V_PREPIPE_TOP_YUV2RGB_MATRIX20_OFFSET                         0x84
#define V_PREPIPE_TOP_YUV2RGB_MATRIX20_SHIFT                          0
#define V_PREPIPE_TOP_YUV2RGB_MATRIX20_LENGTH                         16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX21_SHIFT                          16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX21_LENGTH                         16

/* REGISTER saturn_v_prepipe_reg_34 */
#define V_PREPIPE_TOP_YUV2RGB_MATRIX22_OFFSET                         0x88
#define V_PREPIPE_TOP_YUV2RGB_MATRIX22_SHIFT                          0
#define V_PREPIPE_TOP_YUV2RGB_MATRIX22_LENGTH                         16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX23_SHIFT                          16
#define V_PREPIPE_TOP_YUV2RGB_MATRIX23_LENGTH                         16


/* REGISTER saturn_v_prepipe_reg_36 */
#define V_PREPIPE_TOP_DITHER_CG2_AUTO_EN_OFFSET                       0x90
#define V_PREPIPE_TOP_DITHER_CG2_AUTO_EN_SHIFT                        0
#define V_PREPIPE_TOP_DITHER_CG2_AUTO_EN_LENGTH                       1

/* REGISTER saturn_v_prepipe_reg_37 */
#define V_PREPIPE_TOP_DITHER_EN_OFFSET                                0x94
#define V_PREPIPE_TOP_DITHER_EN_SHIFT                                 0
#define V_PREPIPE_TOP_DITHER_EN_LENGTH                                1


/* REGISTER saturn_v_prepipe_reg_43 */
#define V_PREPIPE_TOP_PREPIPE_PROC_LOC_OFFSET                         0xac

/* REGISTER saturn_v_prepipe_reg_44 */
#define V_PREPIPE_TOP_FORCE_UPDATE_EN_OFFSET                          0xb0
#define V_PREPIPE_TOP_FORCE_UPDATE_EN_SHIFT                           0
#define V_PREPIPE_TOP_FORCE_UPDATE_EN_LENGTH                          1
#define V_PREPIPE_TOP_VSYNC_UPDATE_EN_SHIFT                           1
#define V_PREPIPE_TOP_VSYNC_UPDATE_EN_LENGTH                          1
#define V_PREPIPE_TOP_SHADOW_READ_EN_SHIFT                            2
#define V_PREPIPE_TOP_SHADOW_READ_EN_LENGTH                           1

/* REGISTER saturn_v_prepipe_reg_45 */
#define V_PREPIPE_TOP_FORCE_UPDATE_PULSE_OFFSET                       0xb4
#define V_PREPIPE_TOP_FORCE_UPDATE_PULSE_SHIFT                        0
#define V_PREPIPE_TOP_FORCE_UPDATE_PULSE_LENGTH                       1

/* REGISTER saturn_v_prepipe_reg_46 */
#define V_PREPIPE_TOP_FORCE_UPDATE_EN_SE_OFFSET                       0xb8
#define V_PREPIPE_TOP_FORCE_UPDATE_EN_SE_SHIFT                        0
#define V_PREPIPE_TOP_FORCE_UPDATE_EN_SE_LENGTH                       1
#define V_PREPIPE_TOP_VSYNC_UPDATE_EN_SE_SHIFT                        1
#define V_PREPIPE_TOP_VSYNC_UPDATE_EN_SE_LENGTH                       1
#define V_PREPIPE_TOP_SHADOW_READ_EN_SE_SHIFT                         2
#define V_PREPIPE_TOP_SHADOW_READ_EN_SE_LENGTH                        1

/* REGISTER saturn_v_prepipe_reg_47 */
#define V_PREPIPE_TOP_FORCE_UPDATE_PULSE_SE_OFFSET                    0xbc
#define V_PREPIPE_TOP_FORCE_UPDATE_PULSE_SE_SHIFT                     0
#define V_PREPIPE_TOP_FORCE_UPDATE_PULSE_SE_LENGTH                    1

/* REGISTER saturn_v_prepipe_reg_48 */
#define V_PREPIPE_TOP_ICG_OVERRIDE_OFFSET                             0xc0
#define V_PREPIPE_TOP_ICG_OVERRIDE_SHIFT                              0
#define V_PREPIPE_TOP_ICG_OVERRIDE_LENGTH                             1

/* REGISTER saturn_v_prepipe_reg_49 */
#define V_PREPIPE_TOP_TRIGGER_OFFSET                                  0xc4
#define V_PREPIPE_TOP_TRIGGER_SHIFT                                   0
#define V_PREPIPE_TOP_TRIGGER_LENGTH                                  1

/* REGISTER saturn_v_prepipe_reg_50 */
#define V_PREPIPE_TOP_TRIGGER2_OFFSET                                 0xc8
#define V_PREPIPE_TOP_TRIGGER2_SHIFT                                  0
#define V_PREPIPE_TOP_TRIGGER2_LENGTH                                 1

#endif
