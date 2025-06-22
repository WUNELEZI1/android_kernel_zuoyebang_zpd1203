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

#ifndef _DPU_HW_TOP_REG_H_
#define _DPU_HW_TOP_REG_H_

/* REGISTER saturn_dpu_top_reg_0 */
#define MINOR_NUMBER_OFFSET                                           0x00
#define MINOR_NUMBER_SHIFT                                            0
#define MINOR_NUMBER_LENGTH                                           8
#define MAJOR_NUMBER_SHIFT                                            8
#define MAJOR_NUMBER_LENGTH                                           8
#define PRODUCT_ID_SHIFT                                              16
#define PRODUCT_ID_LENGTH                                             16

/* REGISTER saturn_dpu_top_reg_4 */
#define DSC_RD_MEM_LP_AUTO_EN_OFFSET                                  0x10
#define DSC_RD_MEM_LP_AUTO_EN_SHIFT                                   0
#define DSC_RD_MEM_LP_AUTO_EN_LENGTH                                  4

/* REGISTER saturn_dpu_top_reg_5 */
#define RD_SDW_REG_EN_OFFSET                                          0x14
#define RD_SDW_REG_EN_SHIFT                                           0
#define RD_SDW_REG_EN_LENGTH                                          1

/* REGISTER saturn_dpu_top_reg_7 */
#define SHUT_DOWN_NUM_OFFSET                                          0x1c
#define SHUT_DOWN_NUM_SHIFT                                           0
#define SHUT_DOWN_NUM_LENGTH                                          8

/* REGISTER saturn_dpu_top_reg_8 */
#define DEEP_SLEP_NUM_OFFSET                                          0x20
#define DEEP_SLEP_NUM_SHIFT                                           0
#define DEEP_SLEP_NUM_LENGTH                                          8

/* REGISTER saturn_dpu_top_reg_9 */
#define DPU_TOP_PCLK_CG_EN_OFFSET                                     0x24
#define DPU_TOP_PCLK_CG_EN_SHIFT                                      0
#define DPU_TOP_PCLK_CG_EN_LENGTH                                     1

/* REGISTER saturn_dpu_top_reg_10 */
#define DPU_CTL_PCLK_CG_EN_OFFSET                                     0x28
#define DPU_CTL_PCLK_CG_EN_SHIFT                                      0
#define DPU_CTL_PCLK_CG_EN_LENGTH                                     1
#define DPU_CMD_PCLK_CG_EN_SHIFT                                      1
#define DPU_CMD_PCLK_CG_EN_LENGTH                                     1

/* REGISTER saturn_dpu_top_reg_11 */
#define DSC_MST0_PCLK_CG_EN_OFFSET                                    0x2c
#define DSC_MST0_PCLK_CG_EN_SHIFT                                     0
#define DSC_MST0_PCLK_CG_EN_LENGTH                                    1

/* REGISTER saturn_dpu_top_reg_12 */
#define DSC_MST1_PCLK_CG_EN_OFFSET                                    0x30
#define DSC_MST1_PCLK_CG_EN_SHIFT                                     0
#define DSC_MST1_PCLK_CG_EN_LENGTH                                    1

/* REGISTER saturn_dpu_top_reg_13 */
#define DVFS_PCLK_CG_EN_OFFSET                                        0x34
#define DVFS_PCLK_CG_EN_SHIFT                                         0
#define DVFS_PCLK_CG_EN_LENGTH                                        1

/* REGISTER saturn_dpu_top_reg_14 */
#define RCH0_PREPIPE0_PCLK_CG_EN_OFFSET                               0x38
#define RCH0_PREPIPE0_PCLK_CG_EN_SHIFT                                0
#define RCH0_PREPIPE0_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_15 */
#define RCH1_PREPIPE1_PCLK_CG_EN_OFFSET                               0x3c
#define RCH1_PREPIPE1_PCLK_CG_EN_SHIFT                                0
#define RCH1_PREPIPE1_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_16 */
#define RCH2_PREPIPE2_PCLK_CG_EN_OFFSET                               0x40
#define RCH2_PREPIPE2_PCLK_CG_EN_SHIFT                                0
#define RCH2_PREPIPE2_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_17 */
#define RCH3_PREPIPE3_PCLK_CG_EN_OFFSET                               0x44
#define RCH3_PREPIPE3_PCLK_CG_EN_SHIFT                                0
#define RCH3_PREPIPE3_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_18 */
#define RCH4_PREPIPE4_PCLK_CG_EN_OFFSET                               0x48
#define RCH4_PREPIPE4_PCLK_CG_EN_SHIFT                                0
#define RCH4_PREPIPE4_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_19 */
#define RCH5_PREPIPE5_PCLK_CG_EN_OFFSET                               0x4c
#define RCH5_PREPIPE5_PCLK_CG_EN_SHIFT                                0
#define RCH5_PREPIPE5_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_20 */
#define RCH6_PREPIPE6_PCLK_CG_EN_OFFSET                               0x50
#define RCH6_PREPIPE6_PCLK_CG_EN_SHIFT                                0
#define RCH6_PREPIPE6_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_21 */
#define RCH7_PREPIPE7_PCLK_CG_EN_OFFSET                               0x54
#define RCH7_PREPIPE7_PCLK_CG_EN_SHIFT                                0
#define RCH7_PREPIPE7_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_22 */
#define RCH8_PREPIPE8_PCLK_CG_EN_OFFSET                               0x58
#define RCH8_PREPIPE8_PCLK_CG_EN_SHIFT                                0
#define RCH8_PREPIPE8_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_23 */
#define RCH9_PREPIPE9_PCLK_CG_EN_OFFSET                               0x5c
#define RCH9_PREPIPE9_PCLK_CG_EN_SHIFT                                0
#define RCH9_PREPIPE9_PCLK_CG_EN_LENGTH                               1

/* REGISTER saturn_dpu_top_reg_24 */
#define P1_H0_MMU_PCLK_CG_EN_OFFSET                                   0x60
#define P1_H0_MMU_PCLK_CG_EN_SHIFT                                    0
#define P1_H0_MMU_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_25 */
#define P2_MMU_PCLK_CG_EN_OFFSET                                      0x64
#define P2_MMU_PCLK_CG_EN_SHIFT                                       0
#define P2_MMU_PCLK_CG_EN_LENGTH                                      1

/* REGISTER saturn_dpu_top_reg_26 */
#define P1_H0_RDMA_PCLK_CG_EN_OFFSET                                  0x68
#define P1_H0_RDMA_PCLK_CG_EN_SHIFT                                   0
#define P1_H0_RDMA_PCLK_CG_EN_LENGTH                                  5

/* REGISTER saturn_dpu_top_reg_32 */
#define PART2_RDMA_PCLK_CG_EN_OFFSET                                  0x80
#define PART2_RDMA_PCLK_CG_EN_SHIFT                                   0
#define PART2_RDMA_PCLK_CG_EN_LENGTH                                  5

/* REGISTER saturn_dpu_top_reg_36 */
#define P1_H0_PREPQ_PCLK_CG_EN_OFFSET                                 0x90
#define P1_H0_PREPQ_PCLK_CG_EN_SHIFT                                  0
#define P1_H0_PREPQ_PCLK_CG_EN_LENGTH                                 8

/* REGISTER saturn_dpu_top_reg_42 */
#define PART2_PREPQ_PCLK_CG_EN_OFFSET                                 0xa8
#define PART2_PREPQ_PCLK_CG_EN_SHIFT                                  0
#define PART2_PREPQ_PCLK_CG_EN_LENGTH                                 5

/* REGISTER saturn_dpu_top_reg_46 */
#define POSTPIPE0_PCLK_CG_EN_OFFSET                                   0xb8
#define POSTPIPE0_PCLK_CG_EN_SHIFT                                    0
#define POSTPIPE0_PCLK_CG_EN_LENGTH                                   1
#define CMPS0_PCLK_CG_EN_SHIFT                                        1
#define CMPS0_PCLK_CG_EN_LENGTH                                       1
#define POST_TPG0_PCLK_CG_EN_SHIFT                                    2
#define POST_TPG0_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_47 */
#define POSTPIPE1_PCLK_CG_EN_OFFSET                                   0xbc
#define POSTPIPE1_PCLK_CG_EN_SHIFT                                    0
#define POSTPIPE1_PCLK_CG_EN_LENGTH                                   1
#define CMPS1_PCLK_CG_EN_SHIFT                                        1
#define CMPS1_PCLK_CG_EN_LENGTH                                       1
#define POST_TPG1_PCLK_CG_EN_SHIFT                                    2
#define POST_TPG1_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_48 */
#define POSTPIPE2_PCLK_CG_EN_OFFSET                                   0xc0
#define POSTPIPE2_PCLK_CG_EN_SHIFT                                    0
#define POSTPIPE2_PCLK_CG_EN_LENGTH                                   1
#define CMPS2_PCLK_CG_EN_SHIFT                                        1
#define CMPS2_PCLK_CG_EN_LENGTH                                       1
#define POST_TPG2_PCLK_CG_EN_SHIFT                                    2
#define POST_TPG2_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_49 */
#define P1_H1_WB0_PCLK_CG_EN_OFFSET                                   0xc4
#define P1_H1_WB0_PCLK_CG_EN_SHIFT                                    0
#define P1_H1_WB0_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_50 */
#define P1_H1_WB1_PCLK_CG_EN_OFFSET                                   0xc8
#define P1_H1_WB1_PCLK_CG_EN_SHIFT                                    0
#define P1_H1_WB1_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_51 */
#define P1_H1_DSCW_PCLK_CG_EN_OFFSET                                  0xcc
#define P1_H1_DSCW_PCLK_CG_EN_SHIFT                                   0
#define P1_H1_DSCW_PCLK_CG_EN_LENGTH                                  1

/* REGISTER saturn_dpu_top_reg_52 */
#define P1_H1_OUTCTRL_PCLK_CG_EN_OFFSET                               0xd0
#define P1_H1_OUTCTRL_PCLK_CG_EN_SHIFT                                0
#define P1_H1_OUTCTRL_PCLK_CG_EN_LENGTH                               11

/* REGISTER saturn_dpu_top_reg_54 */
#define WB0_PCLK_CG_EN_OFFSET                                         0xd8
#define WB0_PCLK_CG_EN_SHIFT                                          0
#define WB0_PCLK_CG_EN_LENGTH                                         2

/* REGISTER saturn_dpu_top_reg_55 */
#define WB1_PCLK_CG_EN_OFFSET                                         0xdc
#define WB1_PCLK_CG_EN_SHIFT                                          0
#define WB1_PCLK_CG_EN_LENGTH                                         2

/* REGISTER saturn_dpu_top_reg_56 */
#define DSC_WB0_PCLK_CG_EN_OFFSET                                     0xe0
#define DSC_WB0_PCLK_CG_EN_SHIFT                                      0
#define DSC_WB0_PCLK_CG_EN_LENGTH                                     2

/* REGISTER saturn_dpu_top_reg_57 */
#define DSC_WB1_PCLK_CG_EN_OFFSET                                     0xe4
#define DSC_WB1_PCLK_CG_EN_SHIFT                                      0
#define DSC_WB1_PCLK_CG_EN_LENGTH                                     2

/* REGISTER saturn_dpu_top_reg_58 */
#define POST_WDMA_PCLK_CG_EN_OFFSET                                   0xe8
#define POST_WDMA_PCLK_CG_EN_SHIFT                                    0
#define POST_WDMA_PCLK_CG_EN_LENGTH                                   1

/* REGISTER saturn_dpu_top_reg_59 */
#define DSC_RD0_PCLK_CG_EN_OFFSET                                     0xec
#define DSC_RD0_PCLK_CG_EN_SHIFT                                      0
#define DSC_RD0_PCLK_CG_EN_LENGTH                                     1

/* REGISTER saturn_dpu_top_reg_60 */
#define DSC_RD1_PCLK_CG_EN_OFFSET                                     0xf0
#define DSC_RD1_PCLK_CG_EN_SHIFT                                      0
#define DSC_RD1_PCLK_CG_EN_LENGTH                                     1

/* REGISTER saturn_dpu_top_reg_61 */
#define DSC_RD_MMU_PCLK_CG_EN_OFFSET                                  0xf4
#define DSC_RD_MMU_PCLK_CG_EN_SHIFT                                   0
#define DSC_RD_MMU_PCLK_CG_EN_LENGTH                                  1

/* REGISTER saturn_dpu_top_reg_62 */
#define TMG0_PCLK_CG_EN_OFFSET                                        0xf8
#define TMG0_PCLK_CG_EN_SHIFT                                         0
#define TMG0_PCLK_CG_EN_LENGTH                                        1

/* REGISTER saturn_dpu_top_reg_63 */
#define TMG1_PCLK_CG_EN_OFFSET                                        0xfc
#define TMG1_PCLK_CG_EN_SHIFT                                         0
#define TMG1_PCLK_CG_EN_LENGTH                                        1

/* REGISTER saturn_dpu_top_reg_64 */
#define TMG2_PCLK_CG_EN_OFFSET                                        0x100
#define TMG2_PCLK_CG_EN_SHIFT                                         0
#define TMG2_PCLK_CG_EN_LENGTH                                        1

/* REGISTER saturn_dpu_top_reg_68 */
#define TOP_CLK_AUTO_EN_OFFSET                                        0x110
#define TOP_CLK_AUTO_EN_SHIFT                                         0
#define TOP_CLK_AUTO_EN_LENGTH                                        11

/* REGISTER saturn_dpu_top_reg_69 */
#define CTL_CLK_AUTO_EN_OFFSET                                        0x114
#define CTL_CLK_AUTO_EN_SHIFT                                         0
#define CTL_CLK_AUTO_EN_LENGTH                                        2

/* REGISTER saturn_dpu_top_reg_70 */
#define CMDLIST_CLK_AUTO_EN_OFFSET                                    0x118
#define CMDLIST_CLK_AUTO_EN_SHIFT                                     0
#define CMDLIST_CLK_AUTO_EN_LENGTH                                    3

/* REGISTER saturn_dpu_top_reg_71 */
#define PXL_CLK_AUTO_EN_OFFSET                                        0x11c
#define PXL_CLK_AUTO_EN_SHIFT                                         0
#define PXL_CLK_AUTO_EN_LENGTH                                        3

/* REGISTER saturn_dpu_top_reg_72 */
#define P1_H0_ACLK_AUTO_EN_OFFSET                                     0x120
#define P1_H0_ACLK_AUTO_EN_SHIFT                                      0
#define P1_H0_ACLK_AUTO_EN_LENGTH                                     9

/* REGISTER saturn_dpu_top_reg_73 */
#define P1_H0_MCLK_AUTO_EN_OFFSET                                     0x124
#define P1_H0_MCLK_AUTO_EN_SHIFT                                      0
#define P1_H0_MCLK_AUTO_EN_LENGTH                                     6

/* REGISTER saturn_dpu_top_reg_74 */
#define P1_H0_RDMA_CLK_AUTO_EN_OFFSET                                 0x128
#define P1_H0_RDMA_CLK_AUTO_EN_SHIFT                                  0
#define P1_H0_RDMA_CLK_AUTO_EN_LENGTH                                 16

/* REGISTER saturn_dpu_top_reg_75 */
#define P1_H0_PREPQ_CLK_AUTO_EN_OFFSET                                0x12c
#define P1_H0_PREPQ_CLK_AUTO_EN_SHIFT                                 0
#define P1_H0_PREPQ_CLK_AUTO_EN_LENGTH                                8

/* REGISTER saturn_dpu_top_reg_81 */
#define PART2_ACLK_AUTO_EN_OFFSET                                     0x144
#define PART2_ACLK_AUTO_EN_SHIFT                                      0
#define PART2_ACLK_AUTO_EN_LENGTH                                     7

/* REGISTER saturn_dpu_top_reg_82 */
#define PART2_MCLK_AUTO_EN_OFFSET                                     0x148
#define PART2_MCLK_AUTO_EN_SHIFT                                      0
#define PART2_MCLK_AUTO_EN_LENGTH                                     4

/* REGISTER saturn_dpu_top_reg_83 */
#define PART2_RDMA_CLK_AUTO_EN_OFFSET                                 0x14c
#define PART2_RDMA_CLK_AUTO_EN_SHIFT                                  0
#define PART2_RDMA_CLK_AUTO_EN_LENGTH                                 16

/* REGISTER saturn_dpu_top_reg_84 */
#define PART2_PREPQ_CLK_AUTO_EN_OFFSET                                0x150
#define PART2_PREPQ_CLK_AUTO_EN_SHIFT                                 0
#define PART2_PREPQ_CLK_AUTO_EN_LENGTH                                8

/* REGISTER saturn_dpu_top_reg_88 */
#define P1_H1_ACLK_AUTO_EN_OFFSET                                     0x160
#define P1_H1_ACLK_AUTO_EN_SHIFT                                      0
#define P1_H1_ACLK_AUTO_EN_LENGTH                                     9

/* REGISTER saturn_dpu_top_reg_89 */
#define P1_H1_MCLK_AUTO_EN_OFFSET                                     0x164
#define P1_H1_MCLK_AUTO_EN_SHIFT                                      0
#define P1_H1_MCLK_AUTO_EN_LENGTH                                     20

/* REGISTER saturn_dpu_top_reg_90 */
#define P1_H1_DSCCLK0_AUTO_EN_OFFSET                                  0x168
#define P1_H1_DSCCLK0_AUTO_EN_SHIFT                                   0
#define P1_H1_DSCCLK0_AUTO_EN_LENGTH                                  1
#define P1_H1_DSCCLK1_AUTO_EN_SHIFT                                   1
#define P1_H1_DSCCLK1_AUTO_EN_LENGTH                                  1

/* REGISTER saturn_dpu_top_reg_91 */
#define P1_H1_CMPS_CLK_AUTO_EN_OFFSET                                 0x16c
#define P1_H1_CMPS_CLK_AUTO_EN_SHIFT                                  0
#define P1_H1_CMPS_CLK_AUTO_EN_LENGTH                                 3

/* REGISTER saturn_dpu_top_reg_92 */
#define P1_H1_OUTCTRL_CLK_AUTO_EN_OFFSET                              0x170
#define P1_H1_OUTCTRL_CLK_AUTO_EN_SHIFT                               0
#define P1_H1_OUTCTRL_CLK_AUTO_EN_LENGTH                              23

/* REGISTER saturn_dpu_top_reg_95 */
#define P1_H1_WB_CLK_AUTO_EN_OFFSET                                   0x17c
#define P1_H1_WB_CLK_AUTO_EN_SHIFT                                    0
#define P1_H1_WB_CLK_AUTO_EN_LENGTH                                   6

/* REGISTER saturn_dpu_top_reg_96 */
#define TOP_TMG_CLK_AUTO_EN_OFFSET                                    0x180
#define TOP_TMG_CLK_AUTO_EN_SHIFT                                     0
#define TOP_TMG_CLK_AUTO_EN_LENGTH                                    4

/* REGISTER saturn_dpu_top_reg_97 */
#define TOP_DSC_RD_CLK_AUTO_EN_OFFSET                                 0x184
#define TOP_DSC_RD_CLK_AUTO_EN_SHIFT                                  0
#define TOP_DSC_RD_CLK_AUTO_EN_LENGTH                                 5

/* REGISTER saturn_dpu_top_reg_98 */
#define TOP_CFG_SE_OFFSET                                             0x188
#define TOP_CFG_SE_SHIFT                                              0
#define TOP_CFG_SE_LENGTH                                             1

/* REGISTER saturn_dpu_top_reg_99 */
#define TOP_FORCE_UPDATE_EN_OFFSET                                    0x18c
#define TOP_FORCE_UPDATE_EN_SHIFT                                     0
#define TOP_FORCE_UPDATE_EN_LENGTH                                    1
#define TOP_VSYNC_UPDATE_EN_SHIFT                                     1
#define TOP_VSYNC_UPDATE_EN_LENGTH                                    1
#define TOP_SHADOW_READ_EN_SHIFT                                      2
#define TOP_SHADOW_READ_EN_LENGTH                                     1

/* REGISTER saturn_dpu_top_reg_100 */
#define TOP_FORCE_UPDATE_PULSE_OFFSET                                 0x190
#define TOP_FORCE_UPDATE_PULSE_SHIFT                                  0
#define TOP_FORCE_UPDATE_PULSE_LENGTH                                 1

/* REGISTER saturn_dpu_top_reg_101 */
#define TOP_FORCE_UPDATE_EN_SE_OFFSET                                 0x194
#define TOP_FORCE_UPDATE_EN_SE_SHIFT                                  0
#define TOP_FORCE_UPDATE_EN_SE_LENGTH                                 1
#define TOP_VSYNC_UPDATE_EN_SE_SHIFT                                  1
#define TOP_VSYNC_UPDATE_EN_SE_LENGTH                                 1
#define TOP_SHADOW_READ_EN_SE_SHIFT                                   2
#define TOP_SHADOW_READ_EN_SE_LENGTH                                  1

/* REGISTER saturn_dpu_top_reg_102 */
#define TOP_FORCE_UPDATE_PULSE_SE_OFFSET                              0x198
#define TOP_FORCE_UPDATE_PULSE_SE_SHIFT                               0
#define TOP_FORCE_UPDATE_PULSE_SE_LENGTH                              1

/* REGISTER saturn_dpu_top_reg_103 */
#define TOP_ICG_OVERRIDE_OFFSET                                       0x19c
#define TOP_ICG_OVERRIDE_SHIFT                                        0
#define TOP_ICG_OVERRIDE_LENGTH                                       1

/* REGISTER saturn_dpu_top_reg_104 */
#define TOP_TRIGGER_OFFSET                                            0x1a0
#define TOP_TRIGGER_SHIFT                                             0
#define TOP_TRIGGER_LENGTH                                            1

/* REGISTER saturn_dpu_top_reg_105 */
#define TOP_TRIGGER2_OFFSET                                           0x1a4
#define TOP_TRIGGER2_SHIFT                                            0
#define TOP_TRIGGER2_LENGTH                                           1

#endif
