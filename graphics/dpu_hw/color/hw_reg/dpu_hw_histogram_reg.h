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

#ifndef _DPU_HW_HISTOGRAM_REG_H_
#define _DPU_HW_HISTOGRAM_REG_H_

/* REGISTER histogram_reg_0 */
#define HIST_EN_OFFSET                                                0x00
#define HIST_EN_SHIFT                                                 0
#define HIST_EN_LENGTH                                                1

/* REGISTER histogram_reg_1 */
#define HIST_POS_OFFSET                                               0x04
#define HIST_POS_SHIFT                                                0
#define HIST_POS_LENGTH                                               1

/* REGISTER histogram_reg_2 */
#define HIST_CG1_AUTO_EN_OFFSET                                       0x08
#define HIST_CG1_AUTO_EN_SHIFT                                        0
#define HIST_CG1_AUTO_EN_LENGTH                                       1
#define HIST_CG2_AUTO_EN_SHIFT                                        1
#define HIST_CG2_AUTO_EN_LENGTH                                       1

/* REGISTER histogram_reg_3 */
#define HIST_READ_MODE_OFFSET                                         0x0c
#define HIST_READ_MODE_SHIFT                                          0
#define HIST_READ_MODE_LENGTH                                         1

/* REGISTER histogram_reg_4 */
#define HIST_READ_DONE_OFFSET                                         0x10
#define HIST_READ_DONE_SHIFT                                          0
#define HIST_READ_DONE_LENGTH                                         1

/* REGISTER histogram_reg_5 */
#define HIST_ROI_PQ_EN_OFFSET                                         0x14
#define HIST_ROI_PQ_EN_SHIFT                                          0
#define HIST_ROI_PQ_EN_LENGTH                                         1

/* REGISTER histogram_reg_6 */
#define HIST_ROI_PQ_POS_X_BGN_OFFSET                                  0x18
#define HIST_ROI_PQ_POS_X_BGN_SHIFT                                   0
#define HIST_ROI_PQ_POS_X_BGN_LENGTH                                  14
#define HIST_ROI_PQ_POS_X_END_SHIFT                                   16
#define HIST_ROI_PQ_POS_X_END_LENGTH                                  14

/* REGISTER histogram_reg_7 */
#define HIST_ROI_PQ_POS_Y_BGN_OFFSET                                  0x1c
#define HIST_ROI_PQ_POS_Y_BGN_SHIFT                                   0
#define HIST_ROI_PQ_POS_Y_BGN_LENGTH                                  14
#define HIST_ROI_PQ_POS_Y_END_SHIFT                                   16
#define HIST_ROI_PQ_POS_Y_END_LENGTH                                  14

/* REGISTER histogram_reg_8 */
#define HIST_BLK_EN_OFFSET                                            0x20
#define HIST_BLK_EN_SHIFT                                             0
#define HIST_BLK_EN_LENGTH                                            1

/* REGISTER histogram_reg_9 */
#define HIST_BLK_POS_X_BGN_OFFSET                                     0x24
#define HIST_BLK_POS_X_BGN_SHIFT                                      0
#define HIST_BLK_POS_X_BGN_LENGTH                                     14
#define HIST_BLK_POS_X_END_SHIFT                                      16
#define HIST_BLK_POS_X_END_LENGTH                                     14

/* REGISTER histogram_reg_10 */
#define HIST_BLK_POS_Y_BGN_OFFSET                                     0x28
#define HIST_BLK_POS_Y_BGN_SHIFT                                      0
#define HIST_BLK_POS_Y_BGN_LENGTH                                     14
#define HIST_BLK_POS_Y_END_SHIFT                                      16
#define HIST_BLK_POS_Y_END_LENGTH                                     14

/* REGISTER histogram_reg_11 */
#define HIST_ROI_SEC_EN_OFFSET                                        0x2c
#define HIST_ROI_SEC_EN_SHIFT                                         0
#define HIST_ROI_SEC_EN_LENGTH                                        1

/* REGISTER histogram_reg_12 */
#define HIST_ROI_SEC_POS_X_BGN_OFFSET                                 0x30
#define HIST_ROI_SEC_POS_X_BGN_SHIFT                                  0
#define HIST_ROI_SEC_POS_X_BGN_LENGTH                                 14
#define HIST_ROI_SEC_POS_X_END_SHIFT                                  16
#define HIST_ROI_SEC_POS_X_END_LENGTH                                 14

/* REGISTER histogram_reg_13 */
#define HIST_ROI_SEC_POS_Y_BGN_OFFSET                                 0x34
#define HIST_ROI_SEC_POS_Y_BGN_SHIFT                                  0
#define HIST_ROI_SEC_POS_Y_BGN_LENGTH                                 14
#define HIST_ROI_SEC_POS_Y_END_SHIFT                                  16
#define HIST_ROI_SEC_POS_Y_END_LENGTH                                 14

/* REGISTER histogram_reg_14 */
#define HIST_CHNL_OFFSET                                              0x38
#define HIST_CHNL_SHIFT                                               0
#define HIST_CHNL_LENGTH                                              1
#define HIST_MODE_SHIFT                                               1
#define HIST_MODE_LENGTH                                              1
#define HIST_MODE_WGT_COEF0_SHIFT                                     16
#define HIST_MODE_WGT_COEF0_LENGTH                                    11

/* REGISTER histogram_reg_15 */
#define HIST_MODE_WGT_COEF1_OFFSET                                    0x3c
#define HIST_MODE_WGT_COEF1_SHIFT                                     0
#define HIST_MODE_WGT_COEF1_LENGTH                                    11
#define HIST_MODE_WGT_COEF2_SHIFT                                     16
#define HIST_MODE_WGT_COEF2_LENGTH                                    11

/* REGISTER histogram_reg_16 */
#define HIST_BINN_OFFSET                                              0x40

/* REGISTER histogram_reg_17 */
#define HIST_RE_READ_BIN_OFFSET                                       0x44
#define HIST_RE_READ_BIN_SHIFT                                        0
#define HIST_RE_READ_BIN_LENGTH                                       1

/* REGISTER histogram_reg_18 */
#define HIST_BIT_POS_OFFSET                                           0x48
#define HIST_BIT_POS_SHIFT                                            0
#define HIST_BIT_POS_LENGTH                                           2

/* REGISTER histogram_reg_19 */
#define HIST_READ_BIN_BASE_ADDR_OFFSET                                0x4c
#define HIST_READ_BIN_BASE_ADDR_SHIFT                                 0
#define HIST_READ_BIN_BASE_ADDR_LENGTH                                10

/* REGISTER histogram_reg_20 */
#define HIST_USE_LEFT_OFFSET                                          0x50
#define HIST_USE_LEFT_SHIFT                                           0
#define HIST_USE_LEFT_LENGTH                                          1


/* REGISTER histogram_reg_25 */
#define HISTOGRAM_SEC_EN_OFFSET                                       0x64
#define HISTOGRAM_SEC_EN_SHIFT                                        0
#define HISTOGRAM_SEC_EN_LENGTH                                       1

/* REGISTER histogram_reg_26 */
#define HISTOGRAM_FORCE_UPDATE_EN_OFFSET                              0x68
#define HISTOGRAM_FORCE_UPDATE_EN_SHIFT                               0
#define HISTOGRAM_FORCE_UPDATE_EN_LENGTH                              1
#define HISTOGRAM_VSYNC_UPDATE_EN_SHIFT                               1
#define HISTOGRAM_VSYNC_UPDATE_EN_LENGTH                              1
#define HISTOGRAM_SHADOW_READ_EN_SHIFT                                2
#define HISTOGRAM_SHADOW_READ_EN_LENGTH                               1

/* REGISTER histogram_reg_27 */
#define HISTOGRAM_FORCE_UPDATE_PULSE_OFFSET                           0x6c
#define HISTOGRAM_FORCE_UPDATE_PULSE_SHIFT                            0
#define HISTOGRAM_FORCE_UPDATE_PULSE_LENGTH                           1

/* REGISTER histogram_reg_28 */
#define HISTOGRAM_FORCE_UPDATE_EN_SE_OFFSET                           0x70
#define HISTOGRAM_FORCE_UPDATE_EN_SE_SHIFT                            0
#define HISTOGRAM_FORCE_UPDATE_EN_SE_LENGTH                           1
#define HISTOGRAM_VSYNC_UPDATE_EN_SE_SHIFT                            1
#define HISTOGRAM_VSYNC_UPDATE_EN_SE_LENGTH                           1
#define HISTOGRAM_SHADOW_READ_EN_SE_SHIFT                             2
#define HISTOGRAM_SHADOW_READ_EN_SE_LENGTH                            1

/* REGISTER histogram_reg_29 */
#define HISTOGRAM_FORCE_UPDATE_PULSE_SE_OFFSET                        0x74
#define HISTOGRAM_FORCE_UPDATE_PULSE_SE_SHIFT                         0
#define HISTOGRAM_FORCE_UPDATE_PULSE_SE_LENGTH                        1

/* REGISTER histogram_reg_30 */
#define HISTOGRAM_ICG_OVERRIDE_OFFSET                                 0x78
#define HISTOGRAM_ICG_OVERRIDE_SHIFT                                  0
#define HISTOGRAM_ICG_OVERRIDE_LENGTH                                 1

/* REGISTER histogram_reg_31 */
#define TIGGER_OFFSET                                                 0x7c
#define TIGGER_SHIFT                                                  0
#define TIGGER_LENGTH                                                 1

/* REGISTER histogram_reg_32 */
#define TIGGER2_OFFSET                                                0x80
#define TIGGER2_SHIFT                                                 0
#define TIGGER2_LENGTH                                                1

#endif
