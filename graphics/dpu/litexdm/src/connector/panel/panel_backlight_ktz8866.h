// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _PANEL_BACKLIGHT_KTZ8866_H_
#define _PANEL_BACKLIGHT_KTZ8866_H_

#define KTZ8866_backlight_EN_MASK         0x40
#define KTZ8866_backlight_EN_SHIFT        6
#define KTZ8866_backlight_DISABLE         0
#define KTZ8866_backlight_ENABLE          1
#define KTZ8866_LCD_BIAS_ENP              66       //GPIO for Enable pin for positive power (OUTP)
#define KTZ8866_LCD_BIAS_ENN              53       //GPIO for Enable pin for negative power (OUTN)
#define KTZ8866_LCD_DRV_HW_EN             6       //GPIO for Active high hardware enable pin
#define KTZ8866_LCD_DRV_I2C_SCL           5       //Clock of the I 2 C interface.
#define KTZ8866_LCD_DRV_I2C_SDA           4       //Bi-directional data pin of the I 2 C interface.
#define KTZ8866_DISP_ID                   0x01
#define KTZ8866_DISP_BC1                  0x02
#define KTZ8866_DISP_BC2                  0x03
#define KTZ8866_DISP_BB_LSB               0x04
#define KTZ8866_DISP_BB_MSB               0x05
#define KTZ8866_DISP_BL_ENABLE            0x08
#define KTZ8866_DISP_BIAS_CONF1           0x09
#define KTZ8866_DISP_BIAS_CONF2           0x0a
#define KTZ8866_DISP_BIAS_CONF3           0x0b
#define KTZ8866_DISP_BIAS_BOOST           0x0c
#define KTZ8866_DISP_BIAS_VPOS            0x0d
#define KTZ8866_DISP_BIAS_VNEG            0x0e
#define KTZ8866_DISP_FLAGS                0x0f
#define KTZ8866_DISP_OPTION1              0x10
#define KTZ8866_DISP_OPTION2              0x11
#define KTZ8866_DISP_PTD_LSB              0x12
#define KTZ8866_DISP_PTD_MSB              0x13
#define KTZ8866_DISP_DIMMING              0x14
#define KTZ8866_DISP_FULL_CURRENT         0x15

void ktz8866_power_on(void);

void ktz8866_power_off(void);

void ktz8866_set_backlight(unsigned int backlight_level);

#endif
