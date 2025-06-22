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

#ifndef _DPU_HW_TMG_DP_REG_H_
#define _DPU_HW_TMG_DP_REG_H_

/* REGISTER tmg_dp_reg_0 */
#define TMG_DP_TMG_EN_OFFSET                                          0x00
#define TMG_DP_TMG_EN_SHIFT                                           0
#define TMG_DP_TMG_EN_LENGTH                                          1

/* REGISTER tmg_dp_reg_1 */
#define TMG_DP_HSA_OFFSET                                             0x04
#define TMG_DP_HSA_SHIFT                                              0
#define TMG_DP_HSA_LENGTH                                             14
#define TMG_DP_HBP_SHIFT                                              16
#define TMG_DP_HBP_LENGTH                                             14

/* REGISTER tmg_dp_reg_2 */
#define TMG_DP_HACT_OFFSET                                            0x08
#define TMG_DP_HACT_SHIFT                                             0
#define TMG_DP_HACT_LENGTH                                            14
#define TMG_DP_HFP_SHIFT                                              16
#define TMG_DP_HFP_LENGTH                                             14

/* REGISTER tmg_dp_reg_3 */
#define TMG_DP_VSA_OFFSET                                             0x0c
#define TMG_DP_VSA_SHIFT                                              0
#define TMG_DP_VSA_LENGTH                                             14
#define TMG_DP_VBP_SHIFT                                              16
#define TMG_DP_VBP_LENGTH                                             14

/* REGISTER tmg_dp_reg_4 */
#define TMG_DP_VACT_OFFSET                                            0x10
#define TMG_DP_VACT_SHIFT                                             0
#define TMG_DP_VACT_LENGTH                                            14

/* REGISTER tmg_dp_reg_5 */
#define TMG_DP_VFP_OFFSET                                             0x14
#define TMG_DP_VFP_SHIFT                                              0
#define TMG_DP_VFP_LENGTH                                             20

/* REGISTER tmg_dp_reg_6 */
#define TMG_DP_HACT_FSM_OFFSET                                        0x18
#define TMG_DP_HACT_FSM_SHIFT                                         0
#define TMG_DP_HACT_FSM_LENGTH                                        14

/* REGISTER tmg_dp_reg_7 */
#define TMG_DP_CORE_INIT_LN_CFG_NUM_OFFSET                            0x1c
#define TMG_DP_CORE_INIT_LN_CFG_NUM_SHIFT                             0
#define TMG_DP_CORE_INIT_LN_CFG_NUM_LENGTH                            16

/* REGISTER tmg_dp_reg_8 */
#define TMG_DP_SOF_CFG_LN_NUM_OFFSET                                  0x20
#define TMG_DP_SOF_CFG_LN_NUM_SHIFT                                   0
#define TMG_DP_SOF_CFG_LN_NUM_LENGTH                                  20

/* REGISTER tmg_dp_reg_9 */
#define TMG_DP_HALF_FRAME_SEL_OFFSET                                  0x24
#define TMG_DP_HALF_FRAME_SEL_SHIFT                                   0
#define TMG_DP_HALF_FRAME_SEL_LENGTH                                  2
#define TMG_DP_CFG_INT_LN_SHIFT                                       16
#define TMG_DP_CFG_INT_LN_LENGTH                                      14

/* REGISTER tmg_dp_reg_10 */
#define TMG_DP_TMG_FORMAT_OFFSET                                      0x28
#define TMG_DP_TMG_FORMAT_SHIFT                                       0
#define TMG_DP_TMG_FORMAT_LENGTH                                      2
#define TMG_DP_LN_OF_240_NUM_SHIFT                                    16
#define TMG_DP_LN_OF_240_NUM_LENGTH                                   15

/* REGISTER tmg_dp_reg_11 */
#define TMG_DP_SPLIT_EN_BUF_OFFSET                                    0x2c
#define TMG_DP_SPLIT_EN_BUF_SHIFT                                     0
#define TMG_DP_SPLIT_EN_BUF_LENGTH                                    1
#define TMG_DP_CORE_URGENT_VLD_EN_BUF_SHIFT                           1
#define TMG_DP_CORE_URGENT_VLD_EN_BUF_LENGTH                          1
#define TMG_DP_CORE_DFC_L_THRE_BUF_SHIFT                              16
#define TMG_DP_CORE_DFC_L_THRE_BUF_LENGTH                             13

/* REGISTER tmg_dp_reg_12 */
#define TMG_DP_CORE_DFC_H_THRE_BUF_OFFSET                             0x30
#define TMG_DP_CORE_DFC_H_THRE_BUF_SHIFT                              0
#define TMG_DP_CORE_DFC_H_THRE_BUF_LENGTH                             13
#define TMG_DP_CORE_URGENT_L_THRE_BUF_SHIFT                           16
#define TMG_DP_CORE_URGENT_L_THRE_BUF_LENGTH                          13

/* REGISTER tmg_dp_reg_13 */
#define TMG_DP_CORE_URGENT_H_THRE_BUF_OFFSET                          0x34
#define TMG_DP_CORE_URGENT_H_THRE_BUF_SHIFT                           0
#define TMG_DP_CORE_URGENT_H_THRE_BUF_LENGTH                          13
#define TMG_DP_CORE_DPU_DVFS_L_THRE_BUF_SHIFT                         16
#define TMG_DP_CORE_DPU_DVFS_L_THRE_BUF_LENGTH                        13

/* REGISTER tmg_dp_reg_14 */
#define TMG_DP_CORE_DPU_DVFS_H_THRE_BUF_OFFSET                        0x38
#define TMG_DP_CORE_DPU_DVFS_H_THRE_BUF_SHIFT                         0
#define TMG_DP_CORE_DPU_DVFS_H_THRE_BUF_LENGTH                        13
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF0_SHIFT                          16
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF0_LENGTH                         16

/* REGISTER tmg_dp_reg_15 */
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF1_OFFSET                         0x3c
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF1_SHIFT                          0
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF1_LENGTH                         16
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF2_SHIFT                          16
#define TMG_DP_CORE_OBUF_LVL_THRE_BUF2_LENGTH                         16

/* REGISTER tmg_dp_reg_16 */
#define TMG_DP_CORE_OBUF_TH_BUF_OFFSET                                0x40
#define TMG_DP_CORE_OBUF_TH_BUF_SHIFT                                 0
#define TMG_DP_CORE_OBUF_TH_BUF_LENGTH                                16
#define TMG_DP_CORE_FM_TIMING_EN_SHIFT                                16
#define TMG_DP_CORE_FM_TIMING_EN_LENGTH                               1
#define TMG_DP_OBUF0_EN_SHIFT                                         17
#define TMG_DP_OBUF0_EN_LENGTH                                        1
#define TMG_DP_OBUF1_EN_SHIFT                                         18
#define TMG_DP_OBUF1_EN_LENGTH                                        1
#define TMG_DP_OBUF2_EN_SHIFT                                         19
#define TMG_DP_OBUF2_EN_LENGTH                                        1
#define TMG_DP_OBUF3_EN_SHIFT                                         20
#define TMG_DP_OBUF3_EN_LENGTH                                        1
#define TMG_DP_CORE_MEM_LP_AUTO_EN_BUF_SHIFT                          21
#define TMG_DP_CORE_MEM_LP_AUTO_EN_BUF_LENGTH                         1

/* REGISTER tmg_dp_reg_17 */
#define OBUF_NEAR_EMPTY_OFFSET                                        0x44
#define TMG_DP_OBUF_NEAR_EMPTY_SHIFT                                  0
#define TMG_DP_OBUF_NEAR_EMPTY_LENGTH                                 16

/* REGISTER tmg_dp_reg_18 */
#define TMG_DP_CMDLIST_LN_START_OFFSET                                0x48
#define TMG_DP_CMDLIST_LN_START_SHIFT                                 0
#define TMG_DP_CMDLIST_LN_START_LENGTH                                20

/* REGISTER tmg_dp_reg_19 */
#define TMG_DP_CMDLIST_LN_END_OFFSET                                  0x4c
#define TMG_DP_CMDLIST_LN_END_SHIFT                                   0
#define TMG_DP_CMDLIST_LN_END_LENGTH                                  20

/* REGISTER tmg_dp_reg_20 */
#define TMG_DP_IPI_COLOR_DEPTH_OFFSET                                 0x50
#define TMG_DP_IPI_COLOR_DEPTH_SHIFT                                  0
#define TMG_DP_IPI_COLOR_DEPTH_LENGTH                                 5
#define TMG_DP_IPI_FORMAT_SHIFT                                       8
#define TMG_DP_IPI_FORMAT_LENGTH                                      5

/* REGISTER tmg_dp_reg_21 */
#define TMG_DP_DDR_DVFS_OK_LN_TRIG_OFFSET                             0x54
#define TMG_DP_DDR_DVFS_OK_LN_TRIG_SHIFT                              0
#define TMG_DP_DDR_DVFS_OK_LN_TRIG_LENGTH                             20
#define IPI_VSYNC_POL_SHIFT                                           20
#define IPI_VSYNC_POL_LENGTH                                          1
#define IPI_HSYNC_POL_SHIFT                                           21
#define IPI_HSYNC_POL_LENGTH                                          1
#define IPI_DATAEN_POL_SHIFT                                          22
#define IPI_DATAEN_POL_LENGTH                                         1

/* REGISTER tmg_dp_reg_22 */
#define TMG_DP_AUTO_SELFTEST_EN_OFFSET                                0x58
#define TMG_DP_AUTO_SELFTEST_EN_SHIFT                                 0
#define TMG_DP_AUTO_SELFTEST_EN_LENGTH                                1
#define TMG_DP_AUTO_SELFTEST_MODE_SHIFT                               1
#define TMG_DP_AUTO_SELFTEST_MODE_LENGTH                              2

/* REGISTER tmg_dp_reg_23 */
#define TMG_DP_AUTO_SELFTEST_MAN_RGB_OFFSET                           0x5c
#define TMG_DP_AUTO_SELFTEST_MAN_RGB_SHIFT                            0
#define TMG_DP_AUTO_SELFTEST_MAN_RGB_LENGTH                           24

/* REGISTER tmg_dp_reg_24 */
#define TMG_DP_SYS_TIMSTAMP_GRAY_VSYNC0_OFFSET                        0x60

/* REGISTER tmg_dp_reg_25 */
#define TMG_DP_SYS_TIMSTAMP_GRAY_VSYNC1_OFFSET                        0x64

/* REGISTER tmg_dp_reg_26 */
#define TMG_DP_UNDERFLOW_CNT_EN_OFFSET                                0x68
#define TMG_DP_UNDERFLOW_CNT_EN_SHIFT                                 0
#define TMG_DP_UNDERFLOW_CNT_EN_LENGTH                                1

/* REGISTER tmg_dp_reg_27 */
#define TMG_DP_UNDERFLOW_CNT_CLR_OFFSET                               0x6c
#define TMG_DP_UNDERFLOW_CNT_CLR_SHIFT                                0
#define TMG_DP_UNDERFLOW_CNT_CLR_LENGTH                               1

/* REGISTER tmg_dp_reg_28 */
#define TMG_DP_UNDERFLOW_EACH_FM_CNT_OFFSET                           0x70
#define TMG_DP_UNDERFLOW_EACH_FM_CNT_SHIFT                            0
#define TMG_DP_UNDERFLOW_EACH_FM_CNT_LENGTH                           14
#define TMG_DP_UNDERFLOW_FM_CNT_SHIFT                                 16
#define TMG_DP_UNDERFLOW_FM_CNT_LENGTH                                14

/* REGISTER tmg_dp_reg_29 */
#define TMG_DP_DPU_TOP_CG_GAP_NUM_OFFSET                              0x74
#define TMG_DP_DPU_TOP_CG_GAP_NUM_SHIFT                               0
#define TMG_DP_DPU_TOP_CG_GAP_NUM_LENGTH                              8

/* REGISTER tmg_dp_reg_30 */
#define TMG_DP_DEBUG_RD_OFFSET                                        0x78

/* REGISTER tmg_dp_reg_31 */
#define TMG_DP_MCLK_CG2_AUTO_EN_OFFSET                                0x7c
#define TMG_DP_MCLK_CG2_AUTO_EN_SHIFT                                 0
#define TMG_DP_MCLK_CG2_AUTO_EN_LENGTH                                1
#define TMG_DP_AFIFO_FUNC_RESET_DISABLE_SHIFT                         1
#define TMG_DP_AFIFO_FUNC_RESET_DISABLE_LENGTH                        1
#define TMG_DP_UNDERFLOW_DRIFT_EN_SHIFT                               2
#define TMG_DP_UNDERFLOW_DRIFT_EN_LENGTH                              1

/* REGISTER tmg_dp_reg_32 */
#define TMG_DP_TMG_EN_SW_SDW_EN_OFFSET                                0x80
#define TMG_DP_TMG_EN_SW_SDW_EN_SHIFT                                 0
#define TMG_DP_TMG_EN_SW_SDW_EN_LENGTH                                1
#define TMG_DP_TMG_RSVD_SHIFT                                         16
#define TMG_DP_TMG_RSVD_LENGTH                                        16

/* REGISTER tmg_dp_reg_33 */
#define TMG_DP_SEC_EN_OFFSET                                          0x84
#define TMG_DP_SEC_EN_SHIFT                                           0
#define TMG_DP_SEC_EN_LENGTH                                          1

/* REGISTER tmg_dp_reg_34 */
#define TMG_DP_FORCE_UPDATE_EN_OFFSET                                 0x88
#define TMG_DP_FORCE_UPDATE_EN_SHIFT                                  0
#define TMG_DP_FORCE_UPDATE_EN_LENGTH                                 1
#define TMG_DP_VSYNC_UPDATE_EN_SHIFT                                  1
#define TMG_DP_VSYNC_UPDATE_EN_LENGTH                                 1
#define TMG_DP_SHADOW_READ_EN_SHIFT                                   2
#define TMG_DP_SHADOW_READ_EN_LENGTH                                  1

/* REGISTER tmg_dp_reg_35 */
#define TMG_DP_FORCE_UPDATE_PULSE_OFFSET                              0x8c
#define TMG_DP_FORCE_UPDATE_PULSE_SHIFT                               0
#define TMG_DP_FORCE_UPDATE_PULSE_LENGTH                              1

/* REGISTER tmg_dp_reg_36 */
#define TMG_DP_FORCE_UPDATE_EN_SE_OFFSET                              0x90
#define TMG_DP_FORCE_UPDATE_EN_SE_SHIFT                               0
#define TMG_DP_FORCE_UPDATE_EN_SE_LENGTH                              1
#define TMG_DP_VSYNC_UPDATE_EN_SE_SHIFT                               1
#define TMG_DP_VSYNC_UPDATE_EN_SE_LENGTH                              1
#define TMG_DP_SHADOW_READ_EN_SE_SHIFT                                2
#define TMG_DP_SHADOW_READ_EN_SE_LENGTH                               1

/* REGISTER tmg_dp_reg_37 */
#define TMG_DP_FORCE_UPDATE_PULSE_SE_OFFSET                           0x94
#define TMG_DP_FORCE_UPDATE_PULSE_SE_SHIFT                            0
#define TMG_DP_FORCE_UPDATE_PULSE_SE_LENGTH                           1

/* REGISTER tmg_dp_reg_38 */
#define TMG_DP_ICG_OVERRIDE_OFFSET                                    0x98
#define TMG_DP_ICG_OVERRIDE_SHIFT                                     0
#define TMG_DP_ICG_OVERRIDE_LENGTH                                    1

/* REGISTER tmg_dp_reg_39 */
#define TMG_DP_TRIGGER_OFFSET                                         0x9c
#define TMG_DP_TRIGGER_SHIFT                                          0
#define TMG_DP_TRIGGER_LENGTH                                         1

/* REGISTER tmg_dp_reg_40 */
#define TMG_DP_TRIGGER2_OFFSET                                        0xa0
#define TMG_DP_TRIGGER2_SHIFT                                         0
#define TMG_DP_TRIGGER2_LENGTH                                        1

#endif
