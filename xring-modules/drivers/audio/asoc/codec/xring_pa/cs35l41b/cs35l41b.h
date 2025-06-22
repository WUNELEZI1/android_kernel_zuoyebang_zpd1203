/* SPDX-License-Identifier: GPL-2.0-only */
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

#define CS35L41_INT1_MASK_DEFAULT                         (0x7ffd7e3f)

#define CS35L41_CTRL_KEYS_TEST_KEY_CTRL_REG               (0x40)
#define CS35L41_TEST_KEY_CTRL_UNLOCK_1                    (0x00000055)
#define CS35L41_TEST_KEY_CTRL_UNLOCK_2                    (0x000000aa)
#define CS35L41_TEST_KEY_CTRL_LOCK_1                      (0x000000cc)
#define CS35L41_TEST_KEY_CTRL_LOCK_2                      (0x00000033)

#define IRQ1_IRQ1_MASK_1_REG                              (0x10110)
#define PAD_INTF_GPIO_PAD_CONTROL_REG                     (0x242c)

#define XM_UNPACKED24_DSP1_CCM_CORE_CONTROL_REG           (0x2bc1000)
#define CS35L41B_GLOBAL_SYNC_REG                          0x00002024
#define CS35L41B_GLOBAL_AMP_MUTE_SHIFT                    4
#define CS35L41B_GLOBAL_AMP_MUTE                          (1 << CS35L41B_GLOBAL_AMP_MUTE_SHIFT)
#define CS35L41B_GLOBAL_AMP_UNMUTE                        (0 << CS35L41B_GLOBAL_AMP_MUTE_SHIFT)

#define CS35L41B_REFCLK_INPUT_REG                         0x00002c04
#define CS35L41B_ASP_CONTROL1_REG                         0x00004804
#define CS35L41B_GLOBAL_SAMPLE_RATE                       (0x00002c0c)
#define CS35L41B_ASP_CONTROL2_REG                         (0x00004808)
#define CS35L41B_ASP_ENABLES1_REG                         0x00004800
#define CS35L41B_ASP_REG_VALUE_ADDR                       0x00002D10
#define MSM_GLOBAL_ENABLES_REG                            (0x00002014)
#define CS35L41B_ASP_BLOCK_ENABLES                        0x00002018

