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

#ifndef _DPU_HW_POSTPIPE_FULL_TOP_REG_H_
#define _DPU_HW_POSTPIPE_FULL_TOP_REG_H_

/* REGISTER postpipe_full_reg_0 */
#define POSTPIPE_FULL_TOP_CROP_EN_OFFSET                              0x00
#define POSTPIPE_FULL_TOP_CROP_EN_SHIFT                               0
#define POSTPIPE_FULL_TOP_CROP_EN_LENGTH                              1
#define POSTPIPE_FULL_TOP_INSPECT_EN_SHIFT                            1
#define POSTPIPE_FULL_TOP_INSPECT_EN_LENGTH                           1
#define POSTPIPE_FULL_TOP_CRC_EN_SHIFT                                2
#define POSTPIPE_FULL_TOP_CRC_EN_LENGTH                               1
#define SPLIT_EN_SHIFT                                                3
#define SPLIT_EN_LENGTH                                               1

/* REGISTER postpipe_full_reg_1 */
#define POSTPIPE_FULL_TOP_TMG0_SEL_OFFSET                             0x04
#define POSTPIPE_FULL_TOP_TMG0_SEL_SHIFT                              0
#define POSTPIPE_FULL_TOP_TMG0_SEL_LENGTH                             2
#define TMG1_SEL_SHIFT                                                2
#define TMG1_SEL_LENGTH                                               2
#define POSTPIPE_FULL_TOP_PACK_DATA_FORMT_SHIFT                       4
#define POSTPIPE_FULL_TOP_PACK_DATA_FORMT_LENGTH                      2

/* REGISTER postpipe_full_reg_2 */
#define SPLIT_OVERLAP_OFFSET                                          0x08
#define SPLIT_OVERLAP_SHIFT                                           0
#define SPLIT_OVERLAP_LENGTH                                          8

/* REGISTER postpipe_full_reg_3 */
#define POSTPIPE_FULL_TOP_CROP0_X_LEFT_OFFSET                         0x0c
#define POSTPIPE_FULL_TOP_CROP0_X_LEFT_SHIFT                          0
#define POSTPIPE_FULL_TOP_CROP0_X_LEFT_LENGTH                         16
#define POSTPIPE_FULL_TOP_CROP0_X_RIGHT_SHIFT                         16
#define POSTPIPE_FULL_TOP_CROP0_X_RIGHT_LENGTH                        16

/* REGISTER postpipe_full_reg_4 */
#define POSTPIPE_FULL_TOP_CROP0_Y_TOP_OFFSET                          0x10
#define POSTPIPE_FULL_TOP_CROP0_Y_TOP_SHIFT                           0
#define POSTPIPE_FULL_TOP_CROP0_Y_TOP_LENGTH                          16
#define POSTPIPE_FULL_TOP_CROP0_Y_BOT_SHIFT                           16
#define POSTPIPE_FULL_TOP_CROP0_Y_BOT_LENGTH                          16

/* REGISTER postpipe_full_reg_5 */
#define CROP1_X_LEFT_OFFSET                                           0x14
#define CROP1_X_LEFT_SHIFT                                            0
#define CROP1_X_LEFT_LENGTH                                           16
#define CROP1_X_RIGHT_SHIFT                                           16
#define CROP1_X_RIGHT_LENGTH                                          16

/* REGISTER postpipe_full_reg_6 */
#define CROP1_Y_TOP_OFFSET                                            0x18
#define CROP1_Y_TOP_SHIFT                                             0
#define CROP1_Y_TOP_LENGTH                                            16
#define CROP1_Y_BOT_SHIFT                                             16
#define CROP1_Y_BOT_LENGTH                                            16

/* REGISTER postpipe_full_reg_7 */
#define POSTPIPE_FULL_TOP_INSPECT0_X_LEFT_OFFSET                      0x1c
#define POSTPIPE_FULL_TOP_INSPECT0_X_LEFT_SHIFT                       0
#define POSTPIPE_FULL_TOP_INSPECT0_X_LEFT_LENGTH                      16
#define POSTPIPE_FULL_TOP_INSPECT0_Y_TOP_SHIFT                        16
#define POSTPIPE_FULL_TOP_INSPECT0_Y_TOP_LENGTH                       16

/* REGISTER postpipe_full_reg_8 */
#define POSTPIPE_FULL_TOP_INSPECT0_WIN_HSIZE_OFFSET                   0x20
#define POSTPIPE_FULL_TOP_INSPECT0_WIN_HSIZE_SHIFT                    0
#define POSTPIPE_FULL_TOP_INSPECT0_WIN_HSIZE_LENGTH                   16
#define POSTPIPE_FULL_TOP_INSPECT0_WIN_VSIZE_SHIFT                    16
#define POSTPIPE_FULL_TOP_INSPECT0_WIN_VSIZE_LENGTH                   16

/* REGISTER postpipe_full_reg_9 */
#define INSPECT1_X_LEFT_OFFSET                                        0x24
#define INSPECT1_X_LEFT_SHIFT                                         0
#define INSPECT1_X_LEFT_LENGTH                                        16
#define INSPECT1_Y_TOP_SHIFT                                          16
#define INSPECT1_Y_TOP_LENGTH                                         16

/* REGISTER postpipe_full_reg_10 */
#define INSPECT1_WIN_HSIZE_OFFSET                                     0x28
#define INSPECT1_WIN_HSIZE_SHIFT                                      0
#define INSPECT1_WIN_HSIZE_LENGTH                                     16
#define INSPECT1_WIN_VSIZE_SHIFT                                      16
#define INSPECT1_WIN_VSIZE_LENGTH                                     16

/* REGISTER postpipe_full_reg_11 */
#define POSTPIPE_FULL_TOP_DSC0_BYTE_NUM_OFFSET                        0x2c
#define POSTPIPE_FULL_TOP_DSC0_BYTE_NUM_SHIFT                         0
#define POSTPIPE_FULL_TOP_DSC0_BYTE_NUM_LENGTH                        16
#define POSTPIPE_FULL_TOP_DSC1_BYTE_NUM_SHIFT                         16
#define POSTPIPE_FULL_TOP_DSC1_BYTE_NUM_LENGTH                        16

/* REGISTER postpipe_full_reg_12 */
#define DSC0_P0_EN_OFFSET                                             0x30
#define DSC0_P0_EN_SHIFT                                              0
#define DSC0_P0_EN_LENGTH                                             1
#define DSC0_P1_EN_SHIFT                                              1
#define DSC0_P1_EN_LENGTH                                             1
#define DSC0_CLR_PER_FRM_EN_SHIFT                                     2
#define DSC0_CLR_PER_FRM_EN_LENGTH                                    1

/* REGISTER postpipe_full_reg_13 */
#define DSC0_MEM_SD_DLY_OFFSET                                        0x34
#define DSC0_MEM_SD_DLY_SHIFT                                         0
#define DSC0_MEM_SD_DLY_LENGTH                                        16
#define DSC0_MEM_SD_CNT_MAX_SHIFT                                     16
#define DSC0_MEM_SD_CNT_MAX_LENGTH                                    16

/* REGISTER postpipe_full_reg_14 */
#define DSC0_CLR_CNT_MAX_OFFSET                                       0x38
#define DSC0_CLR_CNT_MAX_SHIFT                                        0
#define DSC0_CLR_CNT_MAX_LENGTH                                       16
#define DSC0_RELOAD_CNT_MAX_SHIFT                                     16
#define DSC0_RELOAD_CNT_MAX_LENGTH                                    16

/* REGISTER postpipe_full_reg_15 */
#define DSC0_P0_CRC_DAT_OFFSET                                        0x3c
#define DSC0_P0_CRC_DAT_SHIFT                                         0
#define DSC0_P0_CRC_DAT_LENGTH                                        16
#define DSC0_P1_CRC_DAT_SHIFT                                         16
#define DSC0_P1_CRC_DAT_LENGTH                                        16

/* REGISTER postpipe_full_reg_16 */
#define DSC0_CLK_AUTO_EN_OFFSET                                       0x40
#define DSC0_CLK_AUTO_EN_SHIFT                                        0
#define DSC0_CLK_AUTO_EN_LENGTH                                       1
#define DSC0_MEM_SD_EN_SHIFT                                          1
#define DSC0_MEM_SD_EN_LENGTH                                         1


/* REGISTER postpipe_full_reg_18 */
#define RC_MEM0_AUTO_EN_OFFSET                                        0x48
#define RC_MEM0_AUTO_EN_SHIFT                                         0
#define RC_MEM0_AUTO_EN_LENGTH                                        1
#define RC_MEM1_AUTO_EN_SHIFT                                         1
#define RC_MEM1_AUTO_EN_LENGTH                                        1
#define RC_MEM1_HW_DS_SHIFT                                           2
#define RC_MEM1_HW_DS_LENGTH                                          1

/* REGISTER postpipe_full_reg_19 */
#define ACRP_CLK_CORE_EN_OFFSET                                       0x4c
#define ACRP_CLK_CORE_EN_SHIFT                                        0
#define ACRP_CLK_CORE_EN_LENGTH                                       1
#define ACRP_WHITE_MODE_EN_SHIFT                                      1
#define ACRP_WHITE_MODE_EN_LENGTH                                     1
#define ACRP_SPR_POS_SHIFT                                            2
#define ACRP_SPR_POS_LENGTH                                           1
#define ACRP_WORK_MODE_SHIFT                                          3
#define ACRP_WORK_MODE_LENGTH                                         1
#define ACRP_PIXEL_FORMAT_SHIFT                                       4
#define ACRP_PIXEL_FORMAT_LENGTH                                      1

/* REGISTER postpipe_full_reg_20 */
#define ACRP_BYPASS_OFFSET                                            0x50
#define ACRP_BYPASS_SHIFT                                             0
#define ACRP_BYPASS_LENGTH                                            1
#define ACRP_FRAME_HEIGHT_SHIFT                                       16
#define ACRP_FRAME_HEIGHT_LENGTH                                      13

/* REGISTER postpipe_full_reg_21 */
#define ACRP_FRAME_WIDTH_OFFSET                                       0x54
#define ACRP_FRAME_WIDTH_SHIFT                                        0
#define ACRP_FRAME_WIDTH_LENGTH                                       13

/* REGISTER postpipe_full_reg_22 */
#define ACRP_SETVAL_R_OFFSET                                          0x58
#define ACRP_SETVAL_R_SHIFT                                           0
#define ACRP_SETVAL_R_LENGTH                                          10
#define ACRP_SETVAL_G_SHIFT                                           16
#define ACRP_SETVAL_G_LENGTH                                          10

/* REGISTER postpipe_full_reg_23 */
#define ACRP_SETVAL_B_OFFSET                                          0x5c
#define ACRP_SETVAL_B_SHIFT                                           0
#define ACRP_SETVAL_B_LENGTH                                          10

/* REGISTER postpipe_full_reg_24 */
#define ACRP_BS_LEN_OFFSET                                            0x60
#define ACRP_BS_LEN_SHIFT                                             0
#define ACRP_BS_LEN_LENGTH                                            24

/* REGISTER postpipe_full_reg_25 */
#define POSTPIPE_FULL_TOP_CHANNEL_OFFSET                              0x64
#define POSTPIPE_FULL_TOP_CHANNEL_SHIFT                               0
#define POSTPIPE_FULL_TOP_CHANNEL_LENGTH                              3
#define POSTPIPE_FULL_TOP_CRC_WIDTH_SHIFT                             8
#define POSTPIPE_FULL_TOP_CRC_WIDTH_LENGTH                            7

/* REGISTER postpipe_full_reg_26 */
#define POSTPIPE_FULL_TOP_M_PCRCINIT_OFFSET                           0x68
#define POSTPIPE_FULL_TOP_M_PCRCINIT_SHIFT                            0
#define POSTPIPE_FULL_TOP_M_PCRCINIT_LENGTH                           16

/* REGISTER postpipe_full_reg_27 */
#define POSTPIPE_FULL_TOP_POLY_OFFSET                                 0x6c
#define POSTPIPE_FULL_TOP_POLY_SHIFT                                  0
#define POSTPIPE_FULL_TOP_POLY_LENGTH                                 16

/* REGISTER postpipe_full_reg_28 */
#define POSTPIPE_FULL_TOP_M_PXOROUT_OFFSET                            0x70
#define POSTPIPE_FULL_TOP_M_PXOROUT_SHIFT                             0
#define POSTPIPE_FULL_TOP_M_PXOROUT_LENGTH                            16

/* REGISTER postpipe_full_reg_29 */
#define SPLIT_MEM_LP_EN_OFFSET                                        0x74
#define SPLIT_MEM_LP_EN_SHIFT                                         0
#define SPLIT_MEM_LP_EN_LENGTH                                        1

/* REGISTER postpipe_full_reg_30 */
#define SPLIT_PROC_LOCATION_OFFSET                                    0x78

/* REGISTER postpipe_full_reg_31 */
#define POSTPIPE_FULL_TOP_CRC0_PROC_LOCATION_OFFSET                   0x7c

/* REGISTER postpipe_full_reg_32 */
#define CRC1_PROC_LOCATION_OFFSET                                     0x80

/* REGISTER postpipe_full_reg_33 */
#define POSTPIPE_FULL_TOP_INSPECT0_AVG_DATA_OFFSET                    0x84

/* REGISTER postpipe_full_reg_36 */
#define INSPECT1_AVG_DATA_OFFSET                                      0x90

/* REGISTER postpipe_full_reg_39 */
#define POSTPIPE_FULL_TOP_CRC0_STA_DATA_OFFSET                        0x9c

/* REGISTER postpipe_full_reg_40 */
#define CRC1_STA_DATA_OFFSET                                          0xa0

/* REGISTER postpipe_full_reg_41 */
#define POSTPIPE_FULL_TOP_INSPECT_CRC_DONE_OFFSET                     0xa4
#define POSTPIPE_FULL_TOP_INSPECT_CRC_DONE_SHIFT                      0
#define POSTPIPE_FULL_TOP_INSPECT_CRC_DONE_LENGTH                     4

/* REGISTER postpipe_full_reg_42 */
#define POSTPIPE_FULL_TOP_IRQ_MASK_OFFSET                             0xa8

/* REGISTER postpipe_full_reg_43 */
#define POSTPIPE_FULL_TOP_IRQ_RAW_OFFSET                              0xac

/* REGISTER postpipe_full_reg_44 */
#define POSTPIPE_FULL_TOP_IRQ_STATUS_OFFSET                           0xb0

/* REGISTER postpipe_full_reg_45 */
#define POSTPIPE_FULL_TOP_CFG_SE_OFFSET                               0xb4
#define POSTPIPE_FULL_TOP_CFG_SE_SHIFT                                0
#define POSTPIPE_FULL_TOP_CFG_SE_LENGTH                               1

/* REGISTER postpipe_full_reg_46 */
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_OFFSET                      0xb8
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_SHIFT                       0
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_LENGTH                      1
#define POSTPIPE_FULL_TOP_VSYNC_UPDATE_EN_SHIFT                       1
#define POSTPIPE_FULL_TOP_VSYNC_UPDATE_EN_LENGTH                      1
#define POSTPIPE_FULL_TOP_SHADOW_READ_EN_SHIFT                        2
#define POSTPIPE_FULL_TOP_SHADOW_READ_EN_LENGTH                       1

/* REGISTER postpipe_full_reg_47 */
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_PULSE_OFFSET                   0xbc
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_PULSE_SHIFT                    0
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_PULSE_LENGTH                   1

/* REGISTER postpipe_full_reg_48 */
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_SE_OFFSET                   0xc0
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_SE_SHIFT                    0
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_SE_LENGTH                   1
#define POSTPIPE_FULL_TOP_VSYNC_UPDATE_EN_SE_SHIFT                    1
#define POSTPIPE_FULL_TOP_VSYNC_UPDATE_EN_SE_LENGTH                   1
#define POSTPIPE_FULL_TOP_SHADOW_READ_EN_SE_SHIFT                     2
#define POSTPIPE_FULL_TOP_SHADOW_READ_EN_SE_LENGTH                    1

/* REGISTER postpipe_full_reg_49 */
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_PULSE_SE_OFFSET                0xc4
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_PULSE_SE_SHIFT                 0
#define POSTPIPE_FULL_TOP_FORCE_UPDATE_PULSE_SE_LENGTH                1

/* REGISTER postpipe_full_reg_50 */
#define POSTPIPE_FULL_TOP_ICG_OVERRIDE_OFFSET                         0xc8
#define POSTPIPE_FULL_TOP_ICG_OVERRIDE_SHIFT                          0
#define POSTPIPE_FULL_TOP_ICG_OVERRIDE_LENGTH                         1

/* REGISTER postpipe_full_reg_51 */
#define POSTPIPE_FULL_TOP_TRIGGER_OFFSET                              0xcc
#define POSTPIPE_FULL_TOP_TRIGGER_SHIFT                               0
#define POSTPIPE_FULL_TOP_TRIGGER_LENGTH                              1

/* REGISTER postpipe_full_reg_52 */
#define POSTPIPE_FULL_TOP_TRIGGER2_OFFSET                             0xd0
#define POSTPIPE_FULL_TOP_TRIGGER2_SHIFT                              0
#define POSTPIPE_FULL_TOP_TRIGGER2_LENGTH                             1

#endif
