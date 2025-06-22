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

#ifndef _XRING_HW_DPU_SCENE_CTL_REG_H_
#define _XRING_HW_DPU_SCENE_CTL_REG_H_

/* REGISTER dpu_ctl_reg_0 */
#define NML_RCH_EN_OFFSET                                             0x00
#define NML_RCH_EN_SHIFT                                              0
#define NML_RCH_EN_LENGTH                                             10
#define NML_WB_EN_SHIFT                                               16
#define NML_WB_EN_LENGTH                                              2
#define NML_OUTCTL_EN_SHIFT                                           18
#define NML_OUTCTL_EN_LENGTH                                          1
#define NML_DSCW_EN_SHIFT                                             19
#define NML_DSCW_EN_LENGTH                                            2
#define NML_DSCR_EN_SHIFT                                             21
#define NML_DSCR_EN_LENGTH                                            2
#define NML_FRM_TIMING_EN_SHIFT                                       23
#define NML_FRM_TIMING_EN_LENGTH                                      3

/* REGISTER dpu_ctl_reg_1 */
#define NML_CMD_UPDT_EN_OFFSET                                        0x04
#define NML_CMD_UPDT_EN_SHIFT                                         0
#define NML_CMD_UPDT_EN_LENGTH                                        1

/* REGISTER dpu_ctl_reg_2 */
#define BOTH_CFG_RDY_OFFSET                                           0x08
#define BOTH_CFG_RDY_SHIFT                                            0
#define BOTH_CFG_RDY_LENGTH                                           1
#define SW_CLR_SHIFT                                                  1
#define SW_CLR_LENGTH                                                 1
#define SW_CLR_ST_SHIFT                                               2
#define SW_CLR_ST_LENGTH                                              1
#define NML_CFG_RDY_SHIFT                                             3
#define NML_CFG_RDY_LENGTH                                            1

/* REGISTER dpu_ctl_reg_3 */
#define VIDEO_MOD_OFFSET                                              0xc
#define VIDEO_MOD_SHIFT                                               0
#define VIDEO_MOD_LENGTH                                              1
#define DBG_MOD_SHIFT                                                 1
#define DBG_MOD_LENGTH                                                1
#define TIMING_INTER0_SHIFT                                           2
#define TIMING_INTER0_LENGTH                                          6
#define TIMING_INTER1_SHIFT                                           8
#define TIMING_INTER1_LENGTH                                          6

/* REGISTER dpu_ctl_reg_4 */
#define SW_START_OFFSET                                               0x10
#define SW_START_SHIFT                                                0
#define SW_START_LENGTH                                               1
#define DBG_CLR_SHIFT                                                 1
#define DBG_CLR_LENGTH                                                1

/* REGISTER dpu_ctl_reg_5 */
#define REG_NML_VALUE_RST_OFFSET                                      0x14
#define REG_NML_VALUE_RST_SHIFT                                       0
#define REG_NML_VALUE_RST_LENGTH                                      1

/* REGISTER dpu_ctl_reg_6 */
#define NML_INIT_EN_OFFSET                                            0x18
#define NML_INIT_EN_SHIFT                                             0
#define NML_INIT_EN_LENGTH                                            1

/* REGISTER dpu_ctl_reg_7 */
#define NML_RCH_VRT_REUSE_OFFSET                                      0x1c
#define NML_RCH_VRT_REUSE_SHIFT                                       0
#define NML_RCH_VRT_REUSE_LENGTH                                      10
#define CMD_AUTO_REFRESH_EN_SHIFT                                     10
#define CMD_AUTO_REFRESH_EN_LENGTH                                    1

/* REGISTER dpu_ctl_reg_8 */
#define SECU_RCH_EN_OFFSET                                            0x20
#define SECU_RCH_EN_SHIFT                                             0
#define SECU_RCH_EN_LENGTH                                            10
#define SECU_RCH_VRT_REUSE_SHIFT                                      10
#define SECU_RCH_VRT_REUSE_LENGTH                                     10

/* REGISTER dpu_ctl_reg_9 */
#define SECU_CMD_UPDT_EN_OFFSET                                       0x24
#define SECU_CMD_UPDT_EN_SHIFT                                        0
#define SECU_CMD_UPDT_EN_LENGTH                                       1
#define SCENE_CTL_SECU_CFG_ICG_OVERRIDE_SHIFT                         1
#define SCENE_CTL_SECU_CFG_ICG_OVERRIDE_LENGTH                        1

/* REGISTER dpu_ctl_reg_10 */
#define SECU_CFG_RDY_OFFSET                                           0x28
#define SECU_CFG_RDY_SHIFT                                            0
#define SECU_CFG_RDY_LENGTH                                           1

/* REGISTER dpu_ctl_reg_11 */
#define SECU_DSCW_DISABLE_OFFSET                                      0x2c
#define SECU_DSCW_DISABLE_SHIFT                                       0
#define SECU_DSCW_DISABLE_LENGTH                                      2
#define SECU_WB_DISABLE_SHIFT                                         2
#define SECU_WB_DISABLE_LENGTH                                        2

/* REGISTER dpu_ctl_reg_12 */
#define REG_SECU_VALUE_RST_OFFSET                                     0x30
#define REG_SECU_VALUE_RST_SHIFT                                      0
#define REG_SECU_VALUE_RST_LENGTH                                     1

/* REGISTER dpu_ctl_reg_13 */
#define SCENE_CTL_DBG0_OFFSET                                         0x34
#define SCENE_CTL_DBG0_SHIFT                                          0
#define SCENE_CTL_DBG0_LENGTH                                         29

/* REGISTER dpu_ctl_reg_14 */
#define SCENE_CTL_DBG1_OFFSET                                         0x38
#define SCENE_CTL_DBG1_SHIFT                                          0
#define SCENE_CTL_DBG1_LENGTH                                         29

#endif
