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
#include <linux/module.h>
#include "madera-codec-init.h"

static int __init madera_codec_init(void)
{
	madera_dev_module_init();
	madera_i2c_module_init();
	madera_pin_driver_init();
	madera_irq_driver_init();
	madera_gpio_driver_init();
	arizona_ldo1_init();
	arizona_micsupp_init();
	cs47l92_codec_driver_init();
	madera_extcon_driver_init();
	return 0;
}

static void madera_codec_exit(void)
{
	madera_dev_module_exit();
	madera_i2c_module_exit();
	madera_pin_driver_exit();
	madera_irq_driver_exit();
	madera_gpio_driver_exit();
	madera_ldo1_exit();
	arizona_micsupp_exit();
	cs47l92_codec_driver_exit();
	madera_extcon_driver_exit();
}

module_init(madera_codec_init);
module_exit(madera_codec_exit);

