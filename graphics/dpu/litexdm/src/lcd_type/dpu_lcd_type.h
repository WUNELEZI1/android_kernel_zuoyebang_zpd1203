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

#ifndef _DPU_LCD_TYPE_H_
#define _DPU_LCD_TYPE_H_

/**
 * get_lcd_type - Get the lcd type object
 *
 * @return 0 on success, -1 on failure
 */
int32_t dpu_get_lcd_type(void);

int32_t dpu_get_scene_id_from_lcd(void);
/**
 * dpu_get_fake_lcd_type - Get the fake lcd type object
 *
 * @return 0 on success, -1 on failure
 */
int32_t dpu_get_fake_lcd_type(void);
#endif