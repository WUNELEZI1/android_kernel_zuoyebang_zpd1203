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

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include "adsp_boot_driver.h"
#include "adsp_ssr_driver.h"
#include "adsp_driver_init.h"

int __init xring_adsp_driver_init(void)
{
	adsp_boot_init();
	adsp_ssr_init();

	return 0;
}

void xring_adsp_driver_exit(void)
{
	adsp_boot_exit();
	adsp_ssr_exit();
}

