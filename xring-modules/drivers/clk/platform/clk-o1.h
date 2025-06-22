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

#ifndef __XR_CLK_O1_H__
#define __XR_CLK_O1_H__

#include <linux/platform_device.h>

#if IS_ENABLED(CONFIG_XRING_CLK_O1)
int xring_clocks_init(void);
int xring_clocks_exit(void);
#else
static inline int xring_clocks_init(void) { return; }
static inline int xring_clocks_exit(void) { return; }
#endif


#endif /* __XR_CLK_O1_H__ */
