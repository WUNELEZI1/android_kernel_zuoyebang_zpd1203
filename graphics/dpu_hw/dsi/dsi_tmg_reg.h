/* SPDX-License-Identifier: GPL-2.0 */
/* Warning: Please do not change this file yourself! */
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

#ifndef _DSI_TMG_REG_H_
#define _DSI_TMG_REG_H_

/* REGISTER tmg_dsi_reg_0 */
#define TMG_DSI_TMG_EN_OFFSET                                         0x00
#define TMG_DSI_TMG_EN_SHIFT                                          0
#define TMG_DSI_TMG_EN_LENGTH                                         1

/* REGISTER tmg_dsi_reg_1 */
#define SCREEN_MODE_OFFSET                                            0x04
#define SCREEN_MODE_SHIFT                                             0
#define SCREEN_MODE_LENGTH                                            1
#define SBS_EN_SHIFT                                                  1
#define SBS_EN_LENGTH                                                 1

/* REGISTER tmg_dsi_reg_2 */
#define BASE_SEL_OFFSET                                               0x08
#define BASE_SEL_SHIFT                                                0
#define BASE_SEL_LENGTH                                               2

/* REGISTER tmg_dsi_reg_3 */
#define TMG_DSI_HACT_OFFSET                                           0x0c
#define TMG_DSI_HACT_SHIFT                                            0
#define TMG_DSI_HACT_LENGTH                                           14
#define TMG_DSI_HACT_FSM_SHIFT                                        16
#define TMG_DSI_HACT_FSM_LENGTH                                       14

/* REGISTER tmg_dsi_reg_4 */
#define TMG_DSI_VACT_OFFSET                                           0x10
#define TMG_DSI_VACT_SHIFT                                            0
#define TMG_DSI_VACT_LENGTH                                           14

/* REGISTER tmg_dsi_reg_5 */
#define HSA0_OFFSET                                                   0x14
#define HSA0_SHIFT                                                    0
#define HSA0_LENGTH                                                   14
#define HBP0_SHIFT                                                    16
#define HBP0_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_6 */
#define HFP0_OFFSET                                                   0x18
#define HFP0_SHIFT                                                    0
#define HFP0_LENGTH                                                   14
#define VSA0_SHIFT                                                    16
#define VSA0_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_7 */
#define VBP0_OFFSET                                                   0x1c
#define VBP0_SHIFT                                                    0
#define VBP0_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_8 */
#define VFP0_OFFSET                                                   0x20
#define VFP0_SHIFT                                                    0
#define VFP0_LENGTH                                                   20

/* REGISTER tmg_dsi_reg_9 */
#define HSA1_OFFSET                                                   0x24
#define HSA1_SHIFT                                                    0
#define HSA1_LENGTH                                                   14
#define HBP1_SHIFT                                                    16
#define HBP1_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_10 */
#define HFP1_OFFSET                                                   0x28
#define HFP1_SHIFT                                                    0
#define HFP1_LENGTH                                                   14
#define VSA1_SHIFT                                                    16
#define VSA1_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_11 */
#define VBP1_OFFSET                                                   0x2c
#define VBP1_SHIFT                                                    0
#define VBP1_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_12 */
#define VFP1_OFFSET                                                   0x30
#define VFP1_SHIFT                                                    0
#define VFP1_LENGTH                                                   20

/* REGISTER tmg_dsi_reg_13 */
#define HSA2_OFFSET                                                   0x34
#define HSA2_SHIFT                                                    0
#define HSA2_LENGTH                                                   14
#define HBP2_SHIFT                                                    16
#define HBP2_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_14 */
#define HFP2_OFFSET                                                   0x38
#define HFP2_SHIFT                                                    0
#define HFP2_LENGTH                                                   14
#define VSA2_SHIFT                                                    16
#define VSA2_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_15 */
#define VBP2_OFFSET                                                   0x3c
#define VBP2_SHIFT                                                    0
#define VBP2_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_16 */
#define VFP2_OFFSET                                                   0x40
#define VFP2_SHIFT                                                    0
#define VFP2_LENGTH                                                   20

/* REGISTER tmg_dsi_reg_17 */
#define HSA3_OFFSET                                                   0x44
#define HSA3_SHIFT                                                    0
#define HSA3_LENGTH                                                   14
#define HBP3_SHIFT                                                    16
#define HBP3_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_18 */
#define HFP3_OFFSET                                                   0x48
#define HFP3_SHIFT                                                    0
#define HFP3_LENGTH                                                   14
#define VSA3_SHIFT                                                    16
#define VSA3_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_19 */
#define VBP3_OFFSET                                                   0x4c
#define VBP3_SHIFT                                                    0
#define VBP3_LENGTH                                                   14

/* REGISTER tmg_dsi_reg_20 */
#define VFP3_OFFSET                                                   0x50
#define VFP3_SHIFT                                                    0
#define VFP3_LENGTH                                                   20

/* REGISTER tmg_dsi_reg_21 */
#define TMG_DSI_CORE_INIT_LN_CFG_NUM_OFFSET                           0x54
#define TMG_DSI_CORE_INIT_LN_CFG_NUM_SHIFT                            0
#define TMG_DSI_CORE_INIT_LN_CFG_NUM_LENGTH                           16

/* REGISTER tmg_dsi_reg_22 */
#define TMG_DSI_SOF_CFG_LN_NUM_OFFSET                                 0x58
#define TMG_DSI_SOF_CFG_LN_NUM_SHIFT                                  0
#define TMG_DSI_SOF_CFG_LN_NUM_LENGTH                                 20

/* REGISTER tmg_dsi_reg_23 */
#define TMG_DSI_HALF_FRAME_SEL_OFFSET                                 0x5c
#define TMG_DSI_HALF_FRAME_SEL_SHIFT                                  0
#define TMG_DSI_HALF_FRAME_SEL_LENGTH                                 2
#define TMG_DSI_CFG_INT_LN_SHIFT                                      16
#define TMG_DSI_CFG_INT_LN_LENGTH                                     14

/* REGISTER tmg_dsi_reg_24 */
#define TMG_DSI_TMG_FORMAT_OFFSET                                     0x60
#define TMG_DSI_TMG_FORMAT_SHIFT                                      0
#define TMG_DSI_TMG_FORMAT_LENGTH                                     2
#define TMG_DSI_LN_OF_240_NUM_SHIFT                                   16
#define TMG_DSI_LN_OF_240_NUM_LENGTH                                  15

/* REGISTER tmg_dsi_reg_25 */
#define TMG_DSI_SPLIT_EN_BUF_OFFSET                                   0x64
#define TMG_DSI_SPLIT_EN_BUF_SHIFT                                    0
#define TMG_DSI_SPLIT_EN_BUF_LENGTH                                   1
#define TMG_DSI_CORE_URGENT_VLD_EN_BUF_SHIFT                          1
#define TMG_DSI_CORE_URGENT_VLD_EN_BUF_LENGTH                         1
#define TMG_DSI_CORE_DFC_L_THRE_BUF_SHIFT                             16
#define TMG_DSI_CORE_DFC_L_THRE_BUF_LENGTH                            13

/* REGISTER tmg_dsi_reg_26 */
#define TMG_DSI_CORE_DFC_H_THRE_BUF_OFFSET                            0x68
#define TMG_DSI_CORE_DFC_H_THRE_BUF_SHIFT                             0
#define TMG_DSI_CORE_DFC_H_THRE_BUF_LENGTH                            13
#define TMG_DSI_CORE_URGENT_L_THRE_BUF_SHIFT                          16
#define TMG_DSI_CORE_URGENT_L_THRE_BUF_LENGTH                         13

/* REGISTER tmg_dsi_reg_27 */
#define TMG_DSI_CORE_URGENT_H_THRE_BUF_OFFSET                         0x6c
#define TMG_DSI_CORE_URGENT_H_THRE_BUF_SHIFT                          0
#define TMG_DSI_CORE_URGENT_H_THRE_BUF_LENGTH                         13
#define TMG_DSI_CORE_DPU_DVFS_L_THRE_BUF_SHIFT                        16
#define TMG_DSI_CORE_DPU_DVFS_L_THRE_BUF_LENGTH                       13

/* REGISTER tmg_dsi_reg_28 */
#define TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_OFFSET                       0x70
#define TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_SHIFT                        0
#define TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_LENGTH                       13
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF0_SHIFT                         16
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF0_LENGTH                        16

/* REGISTER tmg_dsi_reg_29 */
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_OFFSET                        0x74
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_SHIFT                         0
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_LENGTH                        16
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF2_SHIFT                         16
#define TMG_DSI_CORE_OBUF_LVL_THRE_BUF2_LENGTH                        16

/* REGISTER tmg_dsi_reg_30 */
#define TMG_DSI_CORE_OBUF_TH_BUF_OFFSET                               0x78
#define TMG_DSI_CORE_OBUF_TH_BUF_SHIFT                                0
#define TMG_DSI_CORE_OBUF_TH_BUF_LENGTH                               16

/* REGISTER tmg_dsi_reg_31 */
#define CORE_FM_TIMING_EN_OFFSET                                      0x7c
#define TMG_DSI_CORE_FM_TIMING_EN_SHIFT                               0
#define TMG_DSI_CORE_FM_TIMING_EN_LENGTH                              1
#define TMG_DSI_OBUF0_EN_SHIFT                                        1
#define TMG_DSI_OBUF0_EN_LENGTH                                       1
#define TMG_DSI_OBUF1_EN_SHIFT                                        2
#define TMG_DSI_OBUF1_EN_LENGTH                                       1
#define TMG_DSI_OBUF2_EN_SHIFT                                        3
#define TMG_DSI_OBUF2_EN_LENGTH                                       1
#define TMG_DSI_OBUF3_EN_SHIFT                                        4
#define TMG_DSI_OBUF3_EN_LENGTH                                       1
#define TMG_DSI_CORE_MEM_LP_AUTO_EN_BUF_SHIFT                         5
#define TMG_DSI_CORE_MEM_LP_AUTO_EN_BUF_LENGTH                        1
#define TMG_DSI_OBUF_NEAR_EMPTY_SHIFT                                 16
#define TMG_DSI_OBUF_NEAR_EMPTY_LENGTH                                16

/* REGISTER tmg_dsi_reg_32 */
#define OVERLAP_OFFSET                                                0x80
#define OVERLAP_SHIFT                                                 0
#define OVERLAP_LENGTH                                                10
#define SBS_HACT_DIV2_SHIFT                                           10
#define SBS_HACT_DIV2_LENGTH                                          1
#define SBS_DLY_EN_SHIFT                                              11
#define SBS_DLY_EN_LENGTH                                             1
#define SBS_DLY_NUM_SHIFT                                             16
#define SBS_DLY_NUM_LENGTH                                            14

/* REGISTER tmg_dsi_reg_33 */
#define CMD_WAIT_DATA_EN_OFFSET                                       0x84
#define CMD_WAIT_DATA_EN_SHIFT                                        0
#define CMD_WAIT_DATA_EN_LENGTH                                       1

/* REGISTER tmg_dsi_reg_34 */
#define CMD_WAIT_DATA_TOUT_OFFSET                                     0x88
#define CMD_WAIT_DATA_TOUT_SHIFT                                      0
#define CMD_WAIT_DATA_TOUT_LENGTH                                     25
#define CMD_TIME_AUTO_CAL_SHIFT                                       25
#define CMD_TIME_AUTO_CAL_LENGTH                                      1
#define CMD_TE_SEL_SHIFT                                              26
#define CMD_TE_SEL_LENGTH                                             1

/* REGISTER tmg_dsi_reg_35 */
#define VRR_EN_OFFSET                                                 0x8c
#define VRR_EN_SHIFT                                                  0
#define VRR_EN_LENGTH                                                 1
#define CMD_AUTO_EN_SHIFT                                             1
#define CMD_AUTO_EN_LENGTH                                            1
#define CMD_MAN_MODE_SHIFT                                            2
#define CMD_MAN_MODE_LENGTH                                           1
#define CMD_ICG_OPT_SHIFT                                             3
#define CMD_ICG_OPT_LENGTH                                            1
#define CMD_SKIP_TE_NEXT_SHIFT                                        16
#define CMD_SKIP_TE_NEXT_LENGTH                                       10

/* REGISTER tmg_dsi_reg_36 */
#define VIDEO_SYNC_OPT_OFFSET                                         0x90
#define VIDEO_SYNC_OPT_SHIFT                                          0
#define VIDEO_SYNC_OPT_LENGTH                                         1
#define VIDEO_EM_UNIT_NUM_SHIFT                                       16
#define VIDEO_EM_UNIT_NUM_LENGTH                                      12

/* REGISTER tmg_dsi_reg_37 */
#define VIDEO_FRAME_EM_NUM_OFFSET                                     0x94
#define VIDEO_FRAME_EM_NUM_SHIFT                                      0
#define VIDEO_FRAME_EM_NUM_LENGTH                                     8
#define VIDEO_SKIP_FRAME_NUM_SHIFT                                    8
#define VIDEO_SKIP_FRAME_NUM_LENGTH                                   8
#define VIDEO_EM_NEXT_NUM_SHIFT                                       16
#define VIDEO_EM_NEXT_NUM_LENGTH                                      12
#define VIDEO_DET_TE_EN_SHIFT                                         28
#define VIDEO_DET_TE_EN_LENGTH                                        1

/* REGISTER tmg_dsi_reg_38 */
#define CMD_ULPS_EN_OFFSET                                            0x98
#define CMD_ULPS_EN_SHIFT                                             0
#define CMD_ULPS_EN_LENGTH                                            1

/* REGISTER tmg_dsi_reg_39 */
#define ULPS_LN_ENTER_OFFSET                                          0x9c
#define ULPS_LN_ENTER_SHIFT                                           0
#define ULPS_LN_ENTER_LENGTH                                          20
#define ULPS_DLY_FM_SHIFT                                             24
#define ULPS_DLY_FM_LENGTH                                            8

/* REGISTER tmg_dsi_reg_40 */
#define ULPS_LN_EXIT_OFFSET                                           0xa0
#define ULPS_LN_EXIT_SHIFT                                            0
#define ULPS_LN_EXIT_LENGTH                                           20
#define ULPS_PRE_FM_SHIFT                                             24
#define ULPS_PRE_FM_LENGTH                                            8

/* REGISTER tmg_dsi_reg_41 */
#define PWDW_EN_OFFSET                                                0xa4
#define PWDW_EN_SHIFT                                                 0
#define PWDW_EN_LENGTH                                                1

/* REGISTER tmg_dsi_reg_42 */
#define PWDW_LN_TRIG_OFFSET                                           0xa8
#define PWDW_LN_TRIG_SHIFT                                            0
#define PWDW_LN_TRIG_LENGTH                                           20
#define PWDW_DLY_FM_SHIFT                                             24
#define PWDW_DLY_FM_LENGTH                                            8

/* REGISTER tmg_dsi_reg_43 */
#define PWUP_EN_OFFSET                                                0xac
#define PWUP_EN_SHIFT                                                 0
#define PWUP_EN_LENGTH                                                1

/* REGISTER tmg_dsi_reg_44 */
#define PWUP_LN_TRIG_OFFSET                                           0xb0
#define PWUP_LN_TRIG_SHIFT                                            0
#define PWUP_LN_TRIG_LENGTH                                           20
#define PWUP_PRE_FM_SHIFT                                             24
#define PWUP_PRE_FM_LENGTH                                            8

/* REGISTER tmg_dsi_reg_45 */
#define TMG_DSI_CMDLIST_LN_START_OFFSET                               0xb4
#define TMG_DSI_CMDLIST_LN_START_SHIFT                                0
#define TMG_DSI_CMDLIST_LN_START_LENGTH                               20

/* REGISTER tmg_dsi_reg_46 */
#define TMG_DSI_CMDLIST_LN_END_OFFSET                                 0xb8
#define TMG_DSI_CMDLIST_LN_END_SHIFT                                  0
#define TMG_DSI_CMDLIST_LN_END_LENGTH                                 20
#define DOZE_ENTER_EN_SHIFT                                           20
#define DOZE_ENTER_EN_LENGTH                                          1

/* REGISTER tmg_dsi_reg_47 */
#define DOZE_LN_START_OFFSET                                          0xbc
#define DOZE_LN_START_SHIFT                                           0
#define DOZE_LN_START_LENGTH                                          20
#define DOZE_DLY_FM_SHIFT                                             24
#define DOZE_DLY_FM_LENGTH                                            8

/* REGISTER tmg_dsi_reg_48 */
#define DOZE_LN_END_OFFSET                                            0xc0
#define DOZE_LN_END_SHIFT                                             0
#define DOZE_LN_END_LENGTH                                            20
#define DOZE_PRE_FM_SHIFT                                             24
#define DOZE_PRE_FM_LENGTH                                            8

/* REGISTER tmg_dsi_reg_49 */
#define DDR_ACT_DVFS_LN_START_OFFSET                                  0xc4
#define DDR_ACT_DVFS_LN_START_SHIFT                                   0
#define DDR_ACT_DVFS_LN_START_LENGTH                                  14
#define DDR_ACT_DVFS_LN_END_SHIFT                                     16
#define DDR_ACT_DVFS_LN_END_LENGTH                                    14

/* REGISTER tmg_dsi_reg_50 */
#define DDR_IDLE_DVFS_LN_END_OFFSET                                   0xc8
#define DDR_IDLE_DVFS_LN_END_SHIFT                                    0
#define DDR_IDLE_DVFS_LN_END_LENGTH                                   20

/* REGISTER tmg_dsi_reg_51 */
#define TMG_DSI_DDR_DVFS_OK_LN_TRIG_OFFSET                            0xcc
#define TMG_DSI_DDR_DVFS_OK_LN_TRIG_SHIFT                             0
#define TMG_DSI_DDR_DVFS_OK_LN_TRIG_LENGTH                            20

/* REGISTER tmg_dsi_reg_52 */
#define DPU_IDLE_DVFS_LN_TRIG_OFFSET                                  0xd0
#define DPU_IDLE_DVFS_LN_TRIG_SHIFT                                   0
#define DPU_IDLE_DVFS_LN_TRIG_LENGTH                                  20

/* REGISTER tmg_dsi_reg_53 */
#define VRR_ACK_AT_ONCE_OFFSET                                        0xd4
#define VRR_ACK_AT_ONCE_SHIFT                                         0
#define VRR_ACK_AT_ONCE_LENGTH                                        1

/* REGISTER tmg_dsi_reg_54 */
#define TMG_DSI_IPI_COLOR_DEPTH_OFFSET                                0xd8
#define TMG_DSI_IPI_COLOR_DEPTH_SHIFT                                 0
#define TMG_DSI_IPI_COLOR_DEPTH_LENGTH                                4
#define TMG_DSI_IPI_FORMAT_SHIFT                                      4
#define TMG_DSI_IPI_FORMAT_LENGTH                                     4

/* REGISTER tmg_dsi_reg_55 */
#define IPI_SHUTD_EN_OFFSET                                           0xdc
#define IPI_SHUTD_EN_SHIFT                                            0
#define IPI_SHUTD_EN_LENGTH                                           1
#define IPI_COLORM_EN_SHIFT                                           1
#define IPI_COLORM_EN_LENGTH                                          1
#define IPI_TEAR_EN_SHIFT                                             2
#define IPI_TEAR_EN_LENGTH                                            1
#define IPI_TEAR_REQ_RISING_SEL_SHIFT                                 3
#define IPI_TEAR_REQ_RISING_SEL_LENGTH                                1
#define IPI_HIBERNATE_EN_SHIFT                                        4
#define IPI_HIBERNATE_EN_LENGTH                                       1

/* REGISTER tmg_dsi_reg_56 */
#define TMG_DSI_AUTO_SELFTEST_EN_OFFSET                               0xe0
#define TMG_DSI_AUTO_SELFTEST_EN_SHIFT                                0
#define TMG_DSI_AUTO_SELFTEST_EN_LENGTH                               1
#define TMG_DSI_AUTO_SELFTEST_MODE_SHIFT                              1
#define TMG_DSI_AUTO_SELFTEST_MODE_LENGTH                             2

/* REGISTER tmg_dsi_reg_57 */
#define TMG_DSI_AUTO_SELFTEST_MAN_RGB_OFFSET                          0xe4
#define TMG_DSI_AUTO_SELFTEST_MAN_RGB_SHIFT                           0
#define TMG_DSI_AUTO_SELFTEST_MAN_RGB_LENGTH                          24

/* REGISTER tmg_dsi_reg_58 */
#define PARTIAL_MODE_EN_OFFSET                                        0xe8
#define PARTIAL_MODE_EN_SHIFT                                         0
#define PARTIAL_MODE_EN_LENGTH                                        1
#define HACT_PAR_SHIFT                                                16
#define HACT_PAR_LENGTH                                               14

/* REGISTER tmg_dsi_reg_59 */
#define HACT_FSM_PAR_OFFSET                                           0xec
#define HACT_FSM_PAR_SHIFT                                            0
#define HACT_FSM_PAR_LENGTH                                           14
#define VACT_PAR_SHIFT                                                16
#define VACT_PAR_LENGTH                                               14

/* REGISTER tmg_dsi_reg_60 */
#define SBS_INT_INDEX_OFFSET                                          0xf0
#define SBS_INT_INDEX_SHIFT                                           0
#define SBS_INT_INDEX_LENGTH                                          1

/* REGISTER tmg_dsi_reg_61 */
#define TMG_DSI_SYS_TIMSTAMP_GRAY_VSYNC0_OFFSET                       0xf4

/* REGISTER tmg_dsi_reg_62 */
#define TMG_DSI_SYS_TIMSTAMP_GRAY_VSYNC1_OFFSET                       0xf8

/* REGISTER tmg_dsi_reg_63 */
#define TMG_DSI_UNDERFLOW_CNT_EN_OFFSET                               0xfc
#define TMG_DSI_UNDERFLOW_CNT_EN_SHIFT                                0
#define TMG_DSI_UNDERFLOW_CNT_EN_LENGTH                               1

/* REGISTER tmg_dsi_reg_64 */
#define TMG_DSI_UNDERFLOW_CNT_CLR_OFFSET                              0x100
#define TMG_DSI_UNDERFLOW_CNT_CLR_SHIFT                               0
#define TMG_DSI_UNDERFLOW_CNT_CLR_LENGTH                              1

/* REGISTER tmg_dsi_reg_65 */
#define TMG_DSI_UNDERFLOW_EACH_FM_CNT_OFFSET                          0x104
#define TMG_DSI_UNDERFLOW_EACH_FM_CNT_SHIFT                           0
#define TMG_DSI_UNDERFLOW_EACH_FM_CNT_LENGTH                          14
#define TMG_DSI_UNDERFLOW_FM_CNT_SHIFT                                16
#define TMG_DSI_UNDERFLOW_FM_CNT_LENGTH                               14

/* REGISTER tmg_dsi_reg_66 */
#define TMG_DSI_DPU_TOP_CG_GAP_NUM_OFFSET                             0x108
#define TMG_DSI_DPU_TOP_CG_GAP_NUM_SHIFT                              0
#define TMG_DSI_DPU_TOP_CG_GAP_NUM_LENGTH                             8

/* REGISTER tmg_dsi_reg_67 */
#define HACT_CURR_OFFSET                                              0x10c
#define HACT_CURR_SHIFT                                               0
#define HACT_CURR_LENGTH                                              14
#define HACT_FSM_CURR_SHIFT                                           16
#define HACT_FSM_CURR_LENGTH                                          14

/* REGISTER tmg_dsi_reg_68 */
#define TMG_DSI_DEBUG_RD_OFFSET                                       0x110

/* REGISTER tmg_dsi_reg_69 */
#define DPU_CTRL_PWUP_THRE_OFFSET                                     0x114
#define DPU_CTRL_PWUP_THRE_SHIFT                                      0
#define DPU_CTRL_PWUP_THRE_LENGTH                                     12

/* REGISTER tmg_dsi_reg_70 */
#define TMG_DSI_MCLK_CG2_AUTO_EN_OFFSET                               0x118
#define TMG_DSI_MCLK_CG2_AUTO_EN_SHIFT                                0
#define TMG_DSI_MCLK_CG2_AUTO_EN_LENGTH                               1
#define TMG_DSI_AFIFO_FUNC_RESET_DISABLE_SHIFT                        1
#define TMG_DSI_AFIFO_FUNC_RESET_DISABLE_LENGTH                       1
#define TMG_DSI_UNDERFLOW_DRIFT_EN_SHIFT                              2
#define TMG_DSI_UNDERFLOW_DRIFT_EN_LENGTH                             1

/* REGISTER tmg_dsi_reg_71 */
#define DDIC_HSYNC_START_OFFSET                                       0x11c
#define DDIC_HSYNC_START_SHIFT                                        0
#define DDIC_HSYNC_START_LENGTH                                       14
#define DDIC_HSYNC_ST_SEL_SHIFT                                       14
#define DDIC_HSYNC_ST_SEL_LENGTH                                      2
#define DDIC_HSYNC_WIDTH_SHIFT                                        16
#define DDIC_HSYNC_WIDTH_LENGTH                                       12
#define DDIC_HSYNC_MASK_SEL_SHIFT                                     28
#define DDIC_HSYNC_MASK_SEL_LENGTH                                    2
#define DDIC_HSYNC_POL_SHIFT                                          30
#define DDIC_HSYNC_POL_LENGTH                                         1

/* REGISTER tmg_dsi_reg_72 */
#define TMG_DSI_TMG_EN_SW_SDW_EN_OFFSET                               0x120
#define TMG_DSI_TMG_EN_SW_SDW_EN_SHIFT                                0
#define TMG_DSI_TMG_EN_SW_SDW_EN_LENGTH                               1
#define TMG_DSI_TMG_RSVD_SHIFT                                        16
#define TMG_DSI_TMG_RSVD_LENGTH                                       16

/* REGISTER tmg_dsi_reg_73 */
#define TMG_DSI_SEC_EN_OFFSET                                         0x124
#define TMG_DSI_SEC_EN_SHIFT                                          0
#define TMG_DSI_SEC_EN_LENGTH                                         1

/* REGISTER tmg_dsi_reg_74 */
#define TMG_DSI_FORCE_UPDATE_EN_OFFSET                                0x128
#define TMG_DSI_FORCE_UPDATE_EN_SHIFT                                 0
#define TMG_DSI_FORCE_UPDATE_EN_LENGTH                                1
#define TMG_DSI_VSYNC_UPDATE_EN_SHIFT                                 1
#define TMG_DSI_VSYNC_UPDATE_EN_LENGTH                                1
#define TMG_DSI_SHADOW_READ_EN_SHIFT                                  2
#define TMG_DSI_SHADOW_READ_EN_LENGTH                                 1

/* REGISTER tmg_dsi_reg_75 */
#define TMG_DSI_FORCE_UPDATE_PULSE_OFFSET                             0x12c
#define TMG_DSI_FORCE_UPDATE_PULSE_SHIFT                              0
#define TMG_DSI_FORCE_UPDATE_PULSE_LENGTH                             1

/* REGISTER tmg_dsi_reg_76 */
#define TMG_DSI_FORCE_UPDATE_EN_SE_OFFSET                             0x130
#define TMG_DSI_FORCE_UPDATE_EN_SE_SHIFT                              0
#define TMG_DSI_FORCE_UPDATE_EN_SE_LENGTH                             1
#define TMG_DSI_VSYNC_UPDATE_EN_SE_SHIFT                              1
#define TMG_DSI_VSYNC_UPDATE_EN_SE_LENGTH                             1
#define TMG_DSI_SHADOW_READ_EN_SE_SHIFT                               2
#define TMG_DSI_SHADOW_READ_EN_SE_LENGTH                              1

/* REGISTER tmg_dsi_reg_77 */
#define TMG_DSI_FORCE_UPDATE_PULSE_SE_OFFSET                          0x134
#define TMG_DSI_FORCE_UPDATE_PULSE_SE_SHIFT                           0
#define TMG_DSI_FORCE_UPDATE_PULSE_SE_LENGTH                          1

/* REGISTER tmg_dsi_reg_78 */
#define TMG_DSI_ICG_OVERRIDE_OFFSET                                   0x138
#define TMG_DSI_ICG_OVERRIDE_SHIFT                                    0
#define TMG_DSI_ICG_OVERRIDE_LENGTH                                   1

/* REGISTER tmg_dsi_reg_79 */
#define TMG_DSI_TRIGGER_OFFSET                                        0x13c
#define TMG_DSI_TRIGGER_SHIFT                                         0
#define TMG_DSI_TRIGGER_LENGTH                                        1

/* REGISTER tmg_dsi_reg_80 */
#define TMG_DSI_TRIGGER2_OFFSET                                       0x140
#define TMG_DSI_TRIGGER2_SHIFT                                        0
#define TMG_DSI_TRIGGER2_LENGTH                                       1

#endif /* _DSI_TMG_REG_H_ */
