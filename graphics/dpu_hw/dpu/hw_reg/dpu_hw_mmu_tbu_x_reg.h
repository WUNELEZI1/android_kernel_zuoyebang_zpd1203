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

#ifndef _DPU_HW_MMU_TBU_X_REG_H_
#define _DPU_HW_MMU_TBU_X_REG_H_

/* REGISTER mmu_tbu_reg_0 */
#define TBU_EN_OFFSET                                                 0x00
#define TBU_EN_SHIFT                                                  0
#define TBU_EN_LENGTH                                                 1
#define TBU_FBC_MODE_SHIFT                                            1
#define TBU_FBC_MODE_LENGTH                                           1
#define TBU_PLANE_NUM_SHIFT                                           2
#define TBU_PLANE_NUM_LENGTH                                          2
#define TBU_BURST_LIMIT_EN_SHIFT                                      4
#define TBU_BURST_LIMIT_EN_LENGTH                                     1
#define TLB_FETCH_ACTIVE_EN_SHIFT                                     5
#define TLB_FETCH_ACTIVE_EN_LENGTH                                    1
#define TBU_QOS_SHIFT                                                 8
#define TBU_QOS_LENGTH                                                4

/* REGISTER mmu_tbu_reg_1 */
#define TBU_BASE_ADDR0_LOW_OFFSET                                     0x04

/* REGISTER mmu_tbu_reg_2 */
#define TBU_BASE_ADDR0_HIGH_OFFSET                                    0x08
#define TBU_BASE_ADDR0_HIGH_SHIFT                                     0
#define TBU_BASE_ADDR0_HIGH_LENGTH                                    12

/* REGISTER mmu_tbu_reg_3 */
#define TBU_BASE_ADDR1_LOW_OFFSET                                     0x0c

/* REGISTER mmu_tbu_reg_4 */
#define TBU_BASE_ADDR1_HIGH_OFFSET                                    0x10
#define TBU_BASE_ADDR1_HIGH_SHIFT                                     0
#define TBU_BASE_ADDR1_HIGH_LENGTH                                    12

/* REGISTER mmu_tbu_reg_5 */
#define TBU_BASE_ADDR2_LOW_OFFSET                                     0x14

/* REGISTER mmu_tbu_reg_6 */
#define TBU_BASE_ADDR2_HIGH_OFFSET                                    0x18
#define TBU_BASE_ADDR2_HIGH_SHIFT                                     0
#define TBU_BASE_ADDR2_HIGH_LENGTH                                    12

/* REGISTER mmu_tbu_reg_7 */
#define TBU_VA0_OFFSET                                                0x1c

/* REGISTER mmu_tbu_reg_8 */
#define TBU_VA1_OFFSET                                                0x20

/* REGISTER mmu_tbu_reg_9 */
#define TBU_VA2_OFFSET                                                0x24

/* REGISTER mmu_tbu_reg_10 */
#define TBU_SIZE0_OFFSET                                              0x28
#define TBU_SIZE0_SHIFT                                               0
#define TBU_SIZE0_LENGTH                                              16

/* REGISTER mmu_tbu_reg_11 */
#define TBU_SIZE1_OFFSET                                              0x2c
#define TBU_SIZE1_SHIFT                                               0
#define TBU_SIZE1_LENGTH                                              16

/* REGISTER mmu_tbu_reg_12 */
#define TBU_SIZE2_OFFSET                                              0x30
#define TBU_SIZE2_SHIFT                                               0
#define TBU_SIZE2_LENGTH                                              16

/* REGISTER mmu_tbu_reg_13 */
#define MMU_TBU_X_FORCE_UPDATE_EN_OFFSET                              0x34
#define MMU_TBU_X_FORCE_UPDATE_EN_SHIFT                               0
#define MMU_TBU_X_FORCE_UPDATE_EN_LENGTH                              1
#define MMU_TBU_X_VSYNC_UPDATE_EN_SHIFT                               1
#define MMU_TBU_X_VSYNC_UPDATE_EN_LENGTH                              1
#define MMU_TBU_X_SHADOW_READ_EN_SHIFT                                2
#define MMU_TBU_X_SHADOW_READ_EN_LENGTH                               1

/* REGISTER mmu_tbu_reg_14 */
#define MMU_TBU_X_FORCE_UPDATE_PULSE_OFFSET                           0x38
#define MMU_TBU_X_FORCE_UPDATE_PULSE_SHIFT                            0
#define MMU_TBU_X_FORCE_UPDATE_PULSE_LENGTH                           1

/* REGISTER mmu_tbu_reg_15 */
#define MMU_TBU_X_FORCE_UPDATE_EN_SE_OFFSET                           0x3c
#define MMU_TBU_X_FORCE_UPDATE_EN_SE_SHIFT                            0
#define MMU_TBU_X_FORCE_UPDATE_EN_SE_LENGTH                           1
#define MMU_TBU_X_VSYNC_UPDATE_EN_SE_SHIFT                            1
#define MMU_TBU_X_VSYNC_UPDATE_EN_SE_LENGTH                           1
#define MMU_TBU_X_SHADOW_READ_EN_SE_SHIFT                             2
#define MMU_TBU_X_SHADOW_READ_EN_SE_LENGTH                            1

/* REGISTER mmu_tbu_reg_16 */
#define MMU_TBU_X_FORCE_UPDATE_PULSE_SE_OFFSET                        0x40
#define MMU_TBU_X_FORCE_UPDATE_PULSE_SE_SHIFT                         0
#define MMU_TBU_X_FORCE_UPDATE_PULSE_SE_LENGTH                        1

/* REGISTER mmu_tbu_reg_17 */
#define MMU_TBU_X_ICG_OVERRIDE_OFFSET                                 0x44
#define MMU_TBU_X_ICG_OVERRIDE_SHIFT                                  0
#define MMU_TBU_X_ICG_OVERRIDE_LENGTH                                 1

/* REGISTER mmu_tbu_reg_18 */
#define MMU_TBU_X_REGBANK_RESET_OFFSET                                0x48
#define MMU_TBU_X_REGBANK_RESET_SHIFT                                 0
#define MMU_TBU_X_REGBANK_RESET_LENGTH                                1

/* REGISTER mmu_tbu_reg_19 */
#define MMU_TBU_X_REGBANK_RESET2_OFFSET                               0x4c
#define MMU_TBU_X_REGBANK_RESET2_SHIFT                                0
#define MMU_TBU_X_REGBANK_RESET2_LENGTH                               1

/* REGISTER mmu_tbu_reg_20 */
#define TBU_NSAID_OFFSET                                              0x50
#define TBU_NSAID_SHIFT                                               0
#define TBU_NSAID_LENGTH                                              16

#endif
