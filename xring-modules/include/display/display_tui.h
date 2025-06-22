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

#ifndef __DISPLAY_TUI_H_
#define __DISPLAY_TUI_H_

/**
 * REE notifies TEE of power-off callback interfaceIf the REE side
 * shows that it has been powered down and the TEE side has not yet exited back,
 * the REE display notifies the TEE to do the exit process.
 */
typedef void (*ree_power_off_notify)(void);

/**
 * display_enter_tui - TEE notification REE display into TUI.
 * if ree panel power off return -EINVAL
 */
int display_enter_tui(void);

/**
 * display_exit_tui - TEE notification REE display exit TUI.
 */
void display_exit_tui(void);

/**
 * kernel is not friendly to floating-point arithmetic,
 * the dpi returned here is multiplied by 1000,
 * the caller needs to divide by 1000 when using it.
 *
 * display_get_frame_resolution - Returns display resolution and dpi parameters.
 * @width: width of pixel.
 * @height: height of pixel.
 * @dpi_x: dpi width, need to divide by 1000 when using.
 * @dpi_y: dpi height, need to divide by 1000 when using.
 */
void display_get_frame_resolution(uint32_t *width, uint32_t *height,
								  uint32_t *dpi_x, uint32_t *dpi_y);

/**
 * Callback Registration Interface for Notifying TEE of Exit.
 *
 * @callback - Callback interface function pointer for power off
 */
void display_callback_regist(ree_power_off_notify callback);

#endif
