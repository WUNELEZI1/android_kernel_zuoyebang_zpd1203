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
/* File generation time: 2023-11-24 10:54:05. */

#ifndef _DPU_HW_POSTPIPE_LITE_TOP_REG_H_
#define _DPU_HW_POSTPIPE_LITE_TOP_REG_H_

/* REGISTER saturn_postpipe_lite_reg_0 */
#define POSTPIPE_LITE_TOP_CROP_EN_OFFSET                              0x00
#define POSTPIPE_LITE_TOP_CROP_EN_SHIFT                               0
#define POSTPIPE_LITE_TOP_CROP_EN_LENGTH                              1
#define POSTPIPE_LITE_TOP_INSPECT_EN_SHIFT                            1
#define POSTPIPE_LITE_TOP_INSPECT_EN_LENGTH                           1
#define POSTPIPE_LITE_TOP_CRC_EN_SHIFT                                2
#define POSTPIPE_LITE_TOP_CRC_EN_LENGTH                               1

/* REGISTER saturn_postpipe_lite_reg_1 */
#define POSTPIPE_LITE_TOP_TMG0_SEL_OFFSET                             0x04
#define POSTPIPE_LITE_TOP_TMG0_SEL_SHIFT                              0
#define POSTPIPE_LITE_TOP_TMG0_SEL_LENGTH                             2
#define POSTPIPE_LITE_TOP_PACK_DATA_FORMT_SHIFT                       4
#define POSTPIPE_LITE_TOP_PACK_DATA_FORMT_LENGTH                      2

/* REGISTER saturn_postpipe_lite_reg_3 */
#define POSTPIPE_LITE_TOP_CROP0_X_LEFT_OFFSET                         0x0c
#define POSTPIPE_LITE_TOP_CROP0_X_LEFT_SHIFT                          0
#define POSTPIPE_LITE_TOP_CROP0_X_LEFT_LENGTH                         16
#define POSTPIPE_LITE_TOP_CROP0_X_RIGHT_SHIFT                         16
#define POSTPIPE_LITE_TOP_CROP0_X_RIGHT_LENGTH                        16

/* REGISTER saturn_postpipe_lite_reg_4 */
#define POSTPIPE_LITE_TOP_CROP0_Y_TOP_OFFSET                          0x10
#define POSTPIPE_LITE_TOP_CROP0_Y_TOP_SHIFT                           0
#define POSTPIPE_LITE_TOP_CROP0_Y_TOP_LENGTH                          16
#define POSTPIPE_LITE_TOP_CROP0_Y_BOT_SHIFT                           16
#define POSTPIPE_LITE_TOP_CROP0_Y_BOT_LENGTH                          16

/* REGISTER saturn_postpipe_lite_reg_7 */
#define POSTPIPE_LITE_TOP_INSPECT0_X_LEFT_OFFSET                      0x1c
#define POSTPIPE_LITE_TOP_INSPECT0_X_LEFT_SHIFT                       0
#define POSTPIPE_LITE_TOP_INSPECT0_X_LEFT_LENGTH                      16
#define POSTPIPE_LITE_TOP_INSPECT0_Y_TOP_SHIFT                        16
#define POSTPIPE_LITE_TOP_INSPECT0_Y_TOP_LENGTH                       16

/* REGISTER saturn_postpipe_lite_reg_8 */
#define POSTPIPE_LITE_TOP_INSPECT0_WIN_HSIZE_OFFSET                   0x20
#define POSTPIPE_LITE_TOP_INSPECT0_WIN_HSIZE_SHIFT                    0
#define POSTPIPE_LITE_TOP_INSPECT0_WIN_HSIZE_LENGTH                   16
#define POSTPIPE_LITE_TOP_INSPECT0_WIN_VSIZE_SHIFT                    16
#define POSTPIPE_LITE_TOP_INSPECT0_WIN_VSIZE_LENGTH                   16

/* REGISTER saturn_postpipe_lite_reg_11 */
#define POSTPIPE_LITE_TOP_DSC0_BYTE_NUM_OFFSET                        0x2c
#define POSTPIPE_LITE_TOP_DSC0_BYTE_NUM_SHIFT                         0
#define POSTPIPE_LITE_TOP_DSC0_BYTE_NUM_LENGTH                        16
#define POSTPIPE_LITE_TOP_DSC1_BYTE_NUM_SHIFT                         16
#define POSTPIPE_LITE_TOP_DSC1_BYTE_NUM_LENGTH                        16

/* REGISTER saturn_postpipe_lite_reg_12 */
#define DSC1_P0_EN_OFFSET                                             0x30
#define DSC1_P0_EN_SHIFT                                              0
#define DSC1_P0_EN_LENGTH                                             1
#define DSC1_CLR_PER_FRM_EN_SHIFT                                     1
#define DSC1_CLR_PER_FRM_EN_LENGTH                                    1

/* REGISTER saturn_postpipe_lite_reg_13 */
#define DSC1_MEM_SD_DLY_OFFSET                                        0x34
#define DSC1_MEM_SD_DLY_SHIFT                                         0
#define DSC1_MEM_SD_DLY_LENGTH                                        16
#define DSC1_MEM_SD_CNT_MAX_SHIFT                                     16
#define DSC1_MEM_SD_CNT_MAX_LENGTH                                    16

/* REGISTER saturn_postpipe_lite_reg_14 */
#define DSC1_CLR_CNT_MAX_OFFSET                                       0x38
#define DSC1_CLR_CNT_MAX_SHIFT                                        0
#define DSC1_CLR_CNT_MAX_LENGTH                                       16
#define DSC1_RELOAD_CNT_MAX_SHIFT                                     16
#define DSC1_RELOAD_CNT_MAX_LENGTH                                    16

/* REGISTER saturn_postpipe_lite_reg_15 */
#define DSC1_P0_CRC_DAT_OFFSET                                        0x3c
#define DSC1_P0_CRC_DAT_SHIFT                                         0
#define DSC1_P0_CRC_DAT_LENGTH                                        16

/* REGISTER saturn_postpipe_lite_reg_16 */
#define DSC1_CLK_AUTO_EN_OFFSET                                       0x40
#define DSC1_CLK_AUTO_EN_SHIFT                                        0
#define DSC1_CLK_AUTO_EN_LENGTH                                       1
#define DSC1_MEM_SD_EN_SHIFT                                          1
#define DSC1_MEM_SD_EN_LENGTH                                         1

/* REGISTER saturn_postpipe_lite_reg_19 */
#define POSTPIPE_LITE_TOP_CHANNEL_OFFSET                              0x4c
#define POSTPIPE_LITE_TOP_CHANNEL_SHIFT                               0
#define POSTPIPE_LITE_TOP_CHANNEL_LENGTH                              3
#define POSTPIPE_LITE_TOP_CRC_WIDTH_SHIFT                             8
#define POSTPIPE_LITE_TOP_CRC_WIDTH_LENGTH                            7

/* REGISTER saturn_postpipe_lite_reg_20 */
#define POSTPIPE_LITE_TOP_M_PCRCINIT_OFFSET                           0x50
#define POSTPIPE_LITE_TOP_M_PCRCINIT_SHIFT                            0
#define POSTPIPE_LITE_TOP_M_PCRCINIT_LENGTH                           16

/* REGISTER saturn_postpipe_lite_reg_21 */
#define POSTPIPE_LITE_TOP_POLY_OFFSET                                 0x54
#define POSTPIPE_LITE_TOP_POLY_SHIFT                                  0
#define POSTPIPE_LITE_TOP_POLY_LENGTH                                 16

/* REGISTER saturn_postpipe_lite_reg_22 */
#define POSTPIPE_LITE_TOP_M_PXOROUT_OFFSET                            0x58
#define POSTPIPE_LITE_TOP_M_PXOROUT_SHIFT                             0
#define POSTPIPE_LITE_TOP_M_PXOROUT_LENGTH                            16

/* REGISTER saturn_postpipe_lite_reg_25 */
#define POSTPIPE_LITE_TOP_CRC0_PROC_LOCATION_OFFSET                   0x64

/* REGISTER saturn_postpipe_lite_reg_27 */
#define POSTPIPE_LITE_TOP_INSPECT0_AVG_DATA_OFFSET                    0x6c

/* REGISTER saturn_postpipe_lite_reg_33 */
#define POSTPIPE_LITE_TOP_CRC0_STA_DATA_OFFSET                        0x84

/* REGISTER saturn_postpipe_lite_reg_35 */
#define POSTPIPE_LITE_TOP_INSPECT_CRC_DONE_OFFSET                     0x8c
#define POSTPIPE_LITE_TOP_INSPECT_CRC_DONE_SHIFT                      0
#define POSTPIPE_LITE_TOP_INSPECT_CRC_DONE_LENGTH                     4

/* REGISTER saturn_postpipe_lite_reg_36 */
#define POSTPIPE_LITE_TOP_IRQ_MASK_OFFSET                             0x90

/* REGISTER saturn_postpipe_lite_reg_37 */
#define POSTPIPE_LITE_TOP_IRQ_RAW_OFFSET                              0x94

/* REGISTER saturn_postpipe_lite_reg_38 */
#define POSTPIPE_LITE_TOP_IRQ_STATUS_OFFSET                           0x98

/* REGISTER saturn_postpipe_lite_reg_39 */
#define POSTPIPE_LITE_TOP_CFG_SE_OFFSET                               0x9c
#define POSTPIPE_LITE_TOP_CFG_SE_SHIFT                                0
#define POSTPIPE_LITE_TOP_CFG_SE_LENGTH                               1

/* REGISTER saturn_postpipe_lite_reg_40 */
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_EN_OFFSET                      0xa0
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_EN_SHIFT                       0
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_EN_LENGTH                      1
#define POSTPIPE_LITE_TOP_VSYNC_UPDATE_EN_SHIFT                       1
#define POSTPIPE_LITE_TOP_VSYNC_UPDATE_EN_LENGTH                      1
#define POSTPIPE_LITE_TOP_SHADOW_READ_EN_SHIFT                        2
#define POSTPIPE_LITE_TOP_SHADOW_READ_EN_LENGTH                       1

/* REGISTER saturn_postpipe_lite_reg_41 */
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_PULSE_OFFSET                   0xa4
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_PULSE_SHIFT                    0
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_PULSE_LENGTH                   1

/* REGISTER saturn_postpipe_lite_reg_42 */
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_EN_SE_OFFSET                   0xa8
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_EN_SE_SHIFT                    0
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_EN_SE_LENGTH                   1
#define POSTPIPE_LITE_TOP_VSYNC_UPDATE_EN_SE_SHIFT                    1
#define POSTPIPE_LITE_TOP_VSYNC_UPDATE_EN_SE_LENGTH                   1
#define POSTPIPE_LITE_TOP_SHADOW_READ_EN_SE_SHIFT                     2
#define POSTPIPE_LITE_TOP_SHADOW_READ_EN_SE_LENGTH                    1

/* REGISTER saturn_postpipe_lite_reg_43 */
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_PULSE_SE_OFFSET                0xac
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_PULSE_SE_SHIFT                 0
#define POSTPIPE_LITE_TOP_FORCE_UPDATE_PULSE_SE_LENGTH                1

/* REGISTER saturn_postpipe_lite_reg_44 */
#define POSTPIPE_LITE_TOP_ICG_OVERRIDE_OFFSET                         0xb0
#define POSTPIPE_LITE_TOP_ICG_OVERRIDE_SHIFT                          0
#define POSTPIPE_LITE_TOP_ICG_OVERRIDE_LENGTH                         1

/* REGISTER saturn_postpipe_lite_reg_45 */
#define POSTPIPE_LITE_TOP_TRIGGER_OFFSET                              0xb4
#define POSTPIPE_LITE_TOP_TRIGGER_SHIFT                               0
#define POSTPIPE_LITE_TOP_TRIGGER_LENGTH                              1

/* REGISTER saturn_postpipe_lite_reg_46 */
#define POSTPIPE_LITE_TOP_TRIGGER2_OFFSET                             0xb8
#define POSTPIPE_LITE_TOP_TRIGGER2_SHIFT                              0
#define POSTPIPE_LITE_TOP_TRIGGER2_LENGTH                             1

#endif
