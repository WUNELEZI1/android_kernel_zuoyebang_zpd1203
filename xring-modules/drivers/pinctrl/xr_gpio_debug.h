/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#ifndef __XR_GPIO_DEBUG_H__
#define __XR_GPIO_DEBUG_H__

#include "pinctrl-xr.h"

#if IS_ENABLED(CONFIG_XRING_GPIO_DEBUG)
int xr_gpio_debug_init(struct xr_pinctrl *pctrl);
void xr_gpio_debug_exit(void);
#else
static inline int xr_gpio_debug_init(struct xr_pinctrl *pctrl) { return 0; };
static inline void xr_gpio_debug_exit(void) { };
#endif

#endif
