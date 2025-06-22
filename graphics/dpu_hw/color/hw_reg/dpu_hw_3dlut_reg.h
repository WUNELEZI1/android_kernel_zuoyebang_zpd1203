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

#ifndef _DPU_HW_LUT_3D_REG_H_
#define _DPU_HW_LUT_3D_REG_H_

/* REGISTER Lut_3d_reg_0 */
#define LUT_3D_EN_OFFSET                                              0x00
#define LUT_3D_EN_SHIFT                                               0
#define LUT_3D_EN_LENGTH                                              1
#define LUT_3D_MEM_LP_AUTO_EN_SHIFT                                   1
#define LUT_3D_MEM_LP_AUTO_EN_LENGTH                                  1

/* REGISTER Lut_3d_reg_1 */
#define LUT_3D_CFG_DONE_OFFSET                                        0x04
#define LUT_3D_CFG_DONE_SHIFT                                         0
#define LUT_3D_CFG_DONE_LENGTH                                        1

/* REGISTER Lut_3d_reg_2 */
#define DEBUG_MODE_OFFSET                                             0x08
#define DEBUG_MODE_SHIFT                                              0
#define DEBUG_MODE_LENGTH                                             1

/* REGISTER Lut_3d_reg_3 */
#define M_PLUT0_R_OFFSET                                              0x0c
#define M_PLUT0_R_SHIFT                                               0
#define M_PLUT0_R_LENGTH                                              13
#define M_PLUT0_G_SHIFT                                               16
#define M_PLUT0_G_LENGTH                                              13

/* REGISTER Lut_3d_reg_4 */
#define M_PLUT0_B_OFFSET                                              0x10
#define M_PLUT0_B_SHIFT                                               0
#define M_PLUT0_B_LENGTH                                              13
#define M_PLUT1_R_SHIFT                                               16
#define M_PLUT1_R_LENGTH                                              13

/* REGISTER Lut_3d_reg_5 */
#define M_PLUT1_G_OFFSET                                              0x14
#define M_PLUT1_G_SHIFT                                               0
#define M_PLUT1_G_LENGTH                                              13
#define M_PLUT1_B_SHIFT                                               16
#define M_PLUT1_B_LENGTH                                              13

/* REGISTER lut_3d_reg_6to7805 */
#define M_PLUT2_R_OFFSET                                              0x18
#define M_PLUT2_R_SHIFT                                               0
#define M_PLUT2_R_LENGTH                                              13
#define M_PLUT2_G_SHIFT                                               16
#define M_PLUT2_G_LENGTH                                              13

#endif
