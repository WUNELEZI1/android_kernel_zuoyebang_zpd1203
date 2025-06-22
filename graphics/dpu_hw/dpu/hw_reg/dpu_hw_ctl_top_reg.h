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

#ifndef _DPU_HW_CTL_TOP_REG_H_
#define _DPU_HW_CTL_TOP_REG_H_

/* REGISTER dpu_ctl_top_reg_0 */
#define CMDLIST_RCH_EN_OFFSET                                         0x0
#define CMDLIST_RCH_EN_SHIFT                                          0
#define CMDLIST_RCH_EN_LENGTH                                         1

/* REGISTER dpu_ctl_top_reg_10 */
#define CMDLIST_CMPS_OTHER_EN_OFFSET                                  0x28
#define CMDLIST_CMPS_OTHER_EN_SHIFT                                   0
#define CMDLIST_CMPS_OTHER_EN_LENGTH                                  1
#define CMDLIST_CMPS_TOP_EN_SHIFT                                     1
#define CMDLIST_CMPS_TOP_EN_LENGTH                                    1

/* REGISTER dpu_ctl_top_reg_13 */
#define CMDLIST_RDMA_CFG_RDY_OFFSET                                   0x34
#define CMDLIST_RDMA_CFG_RDY_SHIFT                                    0
#define CMDLIST_RDMA_CFG_RDY_LENGTH                                   10
#define CMDLIST_PREPQ_CFG_RDY_SHIFT                                   10
#define CMDLIST_PREPQ_CFG_RDY_LENGTH                                  10
#define CMDLIST_CMPS_CFG_RDY_SHIFT                                    20
#define CMDLIST_CMPS_CFG_RDY_LENGTH                                   3
#define CMDLIST_WB_CFG_RDY_SHIFT                                      23
#define CMDLIST_WB_CFG_RDY_LENGTH                                     2

/* REGISTER dpu_ctl_top_reg_14 */
#define WB0_SEL_ID_OFFSET                                             0x38
#define WB0_SEL_ID_SHIFT                                              0
#define WB0_SEL_ID_LENGTH                                             5

/* REGISTER dpu_ctl_top_reg_15 */
#define WB1_SEL_ID_OFFSET                                             0x3c
#define WB1_SEL_ID_SHIFT                                              0
#define WB1_SEL_ID_LENGTH                                             5

/* REGISTER dpu_ctl_top_reg_16 */
#define REUSE_RDMA_ACT_OFFSET                                         0x40
#define REUSE_RDMA_ACT_SHIFT                                          0
#define REUSE_RDMA_ACT_LENGTH                                         10
#define REUSE_PREPQ_ACT_SHIFT                                         10
#define REUSE_PREPQ_ACT_LENGTH                                        10
#define REUSE_CMPS_ACT_SHIFT                                          20
#define REUSE_CMPS_ACT_LENGTH                                         3
#define REUSE_WB_ACT_SHIFT                                            23
#define REUSE_WB_ACT_LENGTH                                           2

/* REGISTER dpu_ctl_top_reg_17 */
#define RCH_CONFLICT_INTS_OFFSET                                      0x44
#define RCH_CONFLICT_INTS_SHIFT                                       0
#define RCH_CONFLICT_INTS_LENGTH                                      10
#define ACAD_TIMEOUT_INTS_SHIFT                                       10
#define ACAD_TIMEOUT_INTS_LENGTH                                      1
#define WB_TIMEOUT_INTS_SHIFT                                         11
#define WB_TIMEOUT_INTS_LENGTH                                        2
#define CMDLIST_RDMA_CFG_TIMEOUT_INTS_SHIFT                           13
#define CMDLIST_RDMA_CFG_TIMEOUT_INTS_LENGTH                          10
#define CMDLIST_PREPQ_CFG_TIMEOUT_INTS_SHIFT                          23
#define CMDLIST_PREPQ_CFG_TIMEOUT_INTS_LENGTH                         4
#define PSLVERR_INTS_SHIFT                                            27
#define PSLVERR_INTS_LENGTH                                           1

/* REGISTER dpu_ctl_top_reg_18 */
#define RCH_CONFLICT_INT_EN_OFFSET                                    0x48
#define RCH_CONFLICT_INT_EN_SHIFT                                     0
#define RCH_CONFLICT_INT_EN_LENGTH                                    10
#define ACAD_TIMEOUT_INT_EN_SHIFT                                     10
#define ACAD_TIMEOUT_INT_EN_LENGTH                                    1
#define WB_TIMEOUT_INT_EN_SHIFT                                       11
#define WB_TIMEOUT_INT_EN_LENGTH                                      2
#define CMDLIST_RDMA_CFG_TIMEOUT_INT_EN_SHIFT                         13
#define CMDLIST_RDMA_CFG_TIMEOUT_INT_EN_LENGTH                        10
#define CMDLIST_PREPQ_CFG_TIMEOUT_INT_EN_SHIFT                        23
#define CMDLIST_PREPQ_CFG_TIMEOUT_INT_EN_LENGTH                       4
#define PSLVERR_INT_EN_SHIFT                                          27
#define PSLVERR_INT_EN_LENGTH                                         1

/* REGISTER dpu_ctl_top_reg_19 */
#define RCH_CONFLICT_INT_RAW_OFFSET                                   0x4c
#define RCH_CONFLICT_INT_RAW_SHIFT                                    0
#define RCH_CONFLICT_INT_RAW_LENGTH                                   10
#define ACAD_TIMEOUT_INT_RAW_SHIFT                                    10
#define ACAD_TIMEOUT_INT_RAW_LENGTH                                   1
#define WB_TIMEOUT_INT_RAW_SHIFT                                      11
#define WB_TIMEOUT_INT_RAW_LENGTH                                     2
#define CMDLIST_RDMA_CFG_TIMEOUT_INT_RAW_SHIFT                        13
#define CMDLIST_RDMA_CFG_TIMEOUT_INT_RAW_LENGTH                       10
#define CMDLIST_PREPQ_CFG_TIMEOUT_INT_RAW_SHIFT                       23
#define CMDLIST_PREPQ_CFG_TIMEOUT_INT_RAW_LENGTH                      4
#define PSLVERR_INT_RAW_SHIFT                                         27
#define PSLVERR_INT_RAW_LENGTH                                        1

/* REGISTER dpu_ctl_top_reg_20 */
#define WB_SEL_SECU_OFFSET                                            0x50
#define WB_SEL_SECU_SHIFT                                             0
#define WB_SEL_SECU_LENGTH                                            2
#define CTL_TOP_SECU_CFG_ICG_OVERRIDE_SHIFT                           2
#define CTL_TOP_SECU_CFG_ICG_OVERRIDE_LENGTH                          1

/* REGISTER dpu_ctl_top_reg_21 */
#define PSLVERR_ADDR_OFFSET                                           0x54
#define PSLVERR_ADDR_SHIFT                                            0
#define PSLVERR_ADDR_LENGTH                                           8

/* REGISTER dpu_ctl_top_reg_22 */
#define CMDLIST_CH_SW_EVENT_OFFSET                                    0x58
#define CMDLIST_CH_SW_EVENT_SHIFT                                     0
#define CMDLIST_CH_SW_EVENT_LENGTH                                    4

/* REGISTER dpu_ctl_top_reg_35 */
#define CTL_RD_SHADOW_OFFSET                                          0x8c
#define CTL_RD_SHADOW_SHIFT                                           0
#define CTL_RD_SHADOW_LENGTH                                          1

/* REGISTER dpu_ctl_top_reg_36 */
#define RCH_START_CMPS_Y_OFFSET                                       0x90
#define RCH_START_CMPS_Y_SHIFT                                        0
#define RCH_START_CMPS_Y_LENGTH                                       16

/* REGISTER dpu_ctl_top_reg_46 */
#define WB0_SLICE_CNT_OFFSET                                          0xb8
#define WB0_SLICE_CNT_SHIFT                                           0
#define WB0_SLICE_CNT_LENGTH                                          5
#define WB1_SLICE_CNT_SHIFT                                           5
#define WB1_SLICE_CNT_LENGTH                                          5
#define WB_BUSY_SHIFT                                                 10
#define WB_BUSY_LENGTH                                                2
#define ACAD_BUSY_SHIFT                                               12
#define ACAD_BUSY_LENGTH                                              2
#define DSCW_BUSY_SHIFT                                               14
#define DSCW_BUSY_LENGTH                                              2

#endif
