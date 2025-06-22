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
/* File generation time: 2023-06-07 15:37:17. */

#ifndef _VEU_HW_CMDLIST_TOP_REG_H_
#define _VEU_HW_CMDLIST_TOP_REG_H_

#define VEU_CMDLIST_ADDR 0x300
#define VEU_CTRL_TOP_ADDR 0x1c0

/* REGISTER cmdlist_reg_0 */
#define CMDLIST_CH_START_ADDRL_OFFSET                                 0x00
#define CMDLIST_CH_START_ADDRL_SHIFT                                  4
#define CMDLIST_CH_START_ADDRL_LENGTH                                 28

/* REGISTER cmdlist_reg_16 */
#define CMDLIST_CH_START_ADDRH_OFFSET                                 0x40
#define CMDLIST_CH_START_ADDRH_SHIFT                                  0
#define CMDLIST_CH_START_ADDRH_LENGTH                                 7

/* REGISTER cmdlist_reg_32 */
#define CMDLIST_CH_SECU_EN_OFFSET                                     0x80
#define CMDLIST_CH_SECU_EN_SHIFT                                      0
#define CMDLIST_CH_SECU_EN_LENGTH                                     1
#define CMDLIST_CH_USER_ID_SHIFT                                      1
#define CMDLIST_CH_USER_ID_LENGTH                                     15

/* REGISTER cmdlist_reg_45 */
#define SECU_CFG_ICG_OVERRIDE_OFFSET                                  0xb4
#define CMDLIST_TOP_SECU_CFG_ICG_OVERRIDE_SHIFT                       0
#define CMDLIST_TOP_SECU_CFG_ICG_OVERRIDE_LENGTH                      1

/* REGISTER cmdlist_reg_48 */
#define CMDLIST_CH_ARCACHE_OFFSET                                     0xc0
#define CMDLIST_CH_ARCACHE_SHIFT                                      0
#define CMDLIST_CH_ARCACHE_LENGTH                                     4
#define CMDLIST_CH_ARQOS_SHIFT                                        4
#define CMDLIST_CH_ARQOS_LENGTH                                       4
#define CMDLIST_CH_ARREGION_SHIFT                                     8
#define CMDLIST_CH_ARREGION_LENGTH                                    4
#define CMDLIST_CH_Y_FIRST_SHIFT                                      12
#define CMDLIST_CH_Y_FIRST_LENGTH                                     13
#define CMDLIST_CH_OSD_DEP_SHIFT                                      25
#define CMDLIST_CH_OSD_DEP_LENGTH                                     4

/* REGISTER cmdlist_reg_64 */
#define CMDLIST_CH_CONT_ADDRL_OFFSET                                  0x100
#define CMDLIST_CH_CONT_ADDRL_SHIFT                                   4
#define CMDLIST_CH_CONT_ADDRL_LENGTH                                  28

/* REGISTER cmdlist_reg_80 */
#define CMDLIST_CH_CONT_ADDRH_OFFSET                                  0x140
#define CMDLIST_CH_CONT_ADDRH_SHIFT                                   0
#define CMDLIST_CH_CONT_ADDRH_LENGTH                                  7

/* REGISTER cmdlist_reg_96 */
#define CMDLIST_BURST_LEN_OFFSET                                      0x180
#define CMDLIST_BURST_LEN_SHIFT                                       0
#define CMDLIST_BURST_LEN_LENGTH                                      5
#define CMDLIST_TOP_AXI_PORT_SEL_SHIFT                                5
#define CMDLIST_TOP_AXI_PORT_SEL_LENGTH                               2
#define ONL_ARB_RATIO_SHIFT                                           7
#define ONL_ARB_RATIO_LENGTH                                          3
#define REORDER_BURST_SPLIT_EN_SHIFT                                  10
#define REORDER_BURST_SPLIT_EN_LENGTH                                 1
#define REORDER_BURST_SPLIT_MODE_SHIFT                                11
#define REORDER_BURST_SPLIT_MODE_LENGTH                               1
#define REORDER_OSD_NUM_SHIFT                                         12
#define REORDER_OSD_NUM_LENGTH                                        6

/* REGISTER cmdlist_reg_98 */
#define CMDLIST_CH_Y_OTHER_OFFSET                                     0x188
#define CMDLIST_CH_Y_OTHER_SHIFT                                      0
#define CMDLIST_CH_Y_OTHER_LENGTH                                     13

/* REGISTER cmdlist_reg_114 */
#define CMDLIST_CLR_TIMEOUT_TH_OFFSET                                 0x1c8
#define CMDLIST_CLR_TIMEOUT_TH_SHIFT                                  16
#define CMDLIST_CLR_TIMEOUT_TH_LENGTH                                 16

/* REGISTER cmdlist_reg_115 */
#define CMDLIST_PSLVERR_INT_MSK_OFFSET                                0x1cc
#define CMDLIST_PSLVERR_INT_MSK_SHIFT                                 0
#define CMDLIST_PSLVERR_INT_MSK_LENGTH                                1
#define CMDLIST_CH_CLR_TIMEOUT_INT_MSK_SHIFT                          16
#define CMDLIST_CH_CLR_TIMEOUT_INT_MSK_LENGTH                         13

/* REGISTER cmdlist_reg_116 */
#define CMDLIST_PSLVERR_INTS_OFFSET                                   0x1d0
#define CMDLIST_PSLVERR_INTS_SHIFT                                    0
#define CMDLIST_PSLVERR_INTS_LENGTH                                   1
#define CMDLIST_CH_CLR_TIMEOUT_INTS_SHIFT                             16
#define CMDLIST_CH_CLR_TIMEOUT_INTS_LENGTH                            13

/* REGISTER cmdlist_reg_117 */
#define CMDLIST_PSLVERR_INT_RAW_OFFSET                                0x1d4
#define CMDLIST_PSLVERR_INT_RAW_SHIFT                                 0
#define CMDLIST_PSLVERR_INT_RAW_LENGTH                                1
#define CMDLIST_CH_CLR_TIMEOUT_INT_RAW_SHIFT                          16
#define CMDLIST_CH_CLR_TIMEOUT_INT_RAW_LENGTH                         13

/* REGISTER cmdlist_reg_118 */
#define CMDLIST_CH_DBG0_OFFSET                                        0x1d8
#define CMDLIST_CH_DBG0_SHIFT                                         0
#define CMDLIST_CH_DBG0_LENGTH                                        11
#define CMDLIST_CH_DBG1_SHIFT                                         16
#define CMDLIST_CH_DBG1_LENGTH                                        11

/* REGISTER cmdlist_reg_126 */
#define CMDLIST_DBG_OFFSET                                            0x1f8
#define CMDLIST_DBG_SHIFT                                             0
#define CMDLIST_DBG_LENGTH                                            6

/* REGISTER cmdlist_reg_127 */
#define CMDLIST_TOP_MEM_LP_AUTO_EN_OFFSET                             0x1fc
#define CMDLIST_TOP_MEM_LP_AUTO_EN_SHIFT                              0
#define CMDLIST_TOP_MEM_LP_AUTO_EN_LENGTH                             3

#endif
