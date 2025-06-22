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

#ifndef _PANEL_AW37504_H_
#define _PANEL_AW37504_H_

#define AW37504_LCD_BIAS_ENP              214       // GPIO for Enable pin for positive power (OUTP)
#define AW37504_LCD_BIAS_ENN              211       // GPIO for Enable pin for negative power (OUTN)
#define AW37504_LCD_DRV_HW_EN             231       // GPIO for Active high hardware enable pin
#define AW37504_LCD_DRV_I2C_SCL           134       // Clock of the I 2 C interface.
#define AW37504_LCD_DRV_I2C_SDA           135       // Bi-directional data pin of the I 2 C interface.
#define AW37504_VOUTP                     0x00
#define AW37504_VOUTN                     0x01
#define AW37504_APPS                      0x03
#define AW37504_CTRL                      0x04
#define AW37504_WPRTEN                    0x05

void aw37504_config(void);

#endif
