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

#ifndef _DPU_HW_TPG_REG_H_
#define _DPU_HW_TPG_REG_H_

/* REGISTER testpattern_reg_0 */
#define PATTERN_EN_OFFSET                                             0x00
#define PATTERN_EN_SHIFT                                              0
#define PATTERN_EN_LENGTH                                             1

/* REGISTER testpattern_reg_1 */
#define PATTERN_MODE_OFFSET                                           0x04
#define PATTERN_MODE_SHIFT                                            0
#define PATTERN_MODE_LENGTH                                           3

/* REGISTER testpattern_reg_2 */
#define CLK_CG1_AUTO_EN_OFFSET                                        0x08
#define CLK_CG1_AUTO_EN_SHIFT                                         0
#define CLK_CG1_AUTO_EN_LENGTH                                        1
#define CLK_CG2_AUTO_EN_SHIFT                                         1
#define CLK_CG2_AUTO_EN_LENGTH                                        1

/* REGISTER testpattern_reg_3 */
#define H_BLANK_OFFSET                                                0x0c
#define H_BLANK_SHIFT                                                 0
#define H_BLANK_LENGTH                                                12

/* REGISTER testpattern_reg_4 */
#define IMAGE_WIDTH_OFFSET                                            0x10
#define IMAGE_WIDTH_SHIFT                                             0
#define IMAGE_WIDTH_LENGTH                                            13
#define IMAGE_HEIGHT_SHIFT                                            16
#define IMAGE_HEIGHT_LENGTH                                           13

/* REGISTER testpattern_reg_5 */
#define START_VALUE_OFFSET                                            0x14
#define START_VALUE_SHIFT                                             0
#define START_VALUE_LENGTH                                            10
#define END_VALUE_SHIFT                                               16
#define END_VALUE_LENGTH                                              10

/* REGISTER testpattern_reg_6 */
#define HORIZONTAL_STEP_SIZE_OFFSET                                   0x18
#define HORIZONTAL_STEP_SIZE_SHIFT                                    0
#define HORIZONTAL_STEP_SIZE_LENGTH                                   11
#define VERTICAL_STEP_SIZE_SHIFT                                      16
#define VERTICAL_STEP_SIZE_LENGTH                                     11

/* REGISTER testpattern_reg_7 */
#define HORIZONTAL_REPEAT_NUM_OFFSET                                  0x1c
#define HORIZONTAL_REPEAT_NUM_SHIFT                                   0
#define HORIZONTAL_REPEAT_NUM_LENGTH                                  10
#define VERTICAL_REPEAT_NUM_SHIFT                                     16
#define VERTICAL_REPEAT_NUM_LENGTH                                    9

/* REGISTER testpattern_reg_8 */
#define COLOR_OFFSET                                                  0x20
#define COLOR_SHIFT                                                   0
#define COLOR_LENGTH                                                  30

/* REGISTER testpattern_reg_9 */
#define FRM_NUM_OFFSET                                                0x24
#define FRM_NUM_SHIFT                                                 0
#define FRM_NUM_LENGTH                                                2

/* REGISTER testpattern_reg_10 */
#define ALPHA_VALUE_OFFSET                                            0x28
#define ALPHA_VALUE_SHIFT                                             0
#define ALPHA_VALUE_LENGTH                                            8
#define ALPHA_TYPE_SHIFT                                              8
#define ALPHA_TYPE_LENGTH                                             1

/* REGISTER testpattern_reg_11 */
#define SWITCH_MODE_OFFSET                                            0x2c
#define SWITCH_MODE_SHIFT                                             0
#define SWITCH_MODE_LENGTH                                            1
#define SWITCH_CHANNEL_SHIFT                                          1
#define SWITCH_CHANNEL_LENGTH                                         2

/* REGISTER testpattern_reg_12 */
#define BIT_POS_OFFSET                                                0x30
#define BIT_POS_SHIFT                                                 0
#define BIT_POS_LENGTH                                                2

/* REGISTER testpattern_reg_13 */
#define TPG_ICG_OVERRIDE_OFFSET                                       0x34
#define TPG_ICG_OVERRIDE_SHIFT                                        0
#define TPG_ICG_OVERRIDE_LENGTH                                       1

/* REGISTER testpattern_reg_14 */
#define TPG_FORCE_UPDATE_EN_OFFSET                                    0x38
#define TPG_FORCE_UPDATE_EN_SHIFT                                     0
#define TPG_FORCE_UPDATE_EN_LENGTH                                    1

/* REGISTER testpattern_reg_15 */
#define TPG_FORCE_UPDATE_PULSE_OFFSET                                 0x3c
#define TPG_FORCE_UPDATE_PULSE_SHIFT                                  0
#define TPG_FORCE_UPDATE_PULSE_LENGTH                                 1

/* REGISTER testpattern_reg_16 */
#define VSYNC_UPDATE_EN_OFFSET                                        0x40
#define TPG_VSYNC_UPDATE_EN_SHIFT                                     0
#define TPG_VSYNC_UPDATE_EN_LENGTH                                    1
#define TPG_SHADOW_READ_EN_SHIFT                                      1
#define TPG_SHADOW_READ_EN_LENGTH                                     1

/* REGISTER testpattern_reg_17 */
#define TPG_FORCE_UPDATE_EN_SE_OFFSET                                 0x44
#define TPG_FORCE_UPDATE_EN_SE_SHIFT                                  0
#define TPG_FORCE_UPDATE_EN_SE_LENGTH                                 1

/* REGISTER testpattern_reg_18 */
#define TPG_FORCE_UPDATE_PULSE_SE_OFFSET                              0x48
#define TPG_FORCE_UPDATE_PULSE_SE_SHIFT                               0
#define TPG_FORCE_UPDATE_PULSE_SE_LENGTH                              1

/* REGISTER testpattern_reg_19 */
#define VSYNC_UPDATE_EN_SE_OFFSET                                     0x4c
#define TPG_VSYNC_UPDATE_EN_SE_SHIFT                                  0
#define TPG_VSYNC_UPDATE_EN_SE_LENGTH                                 1
#define TPG_SHADOW_READ_EN_SE_SHIFT                                   1
#define TPG_SHADOW_READ_EN_SE_LENGTH                                  1

/* REGISTER testpattern_reg_24 */
#define TPG_SEC_EN_OFFSET                                             0x60
#define TPG_SEC_EN_SHIFT                                              0
#define TPG_SEC_EN_LENGTH                                             1

#endif
