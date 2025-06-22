/* SPDX-License-Identifier: GPL-2.0-only
 *
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
int __init madera_dev_module_init(void);
void madera_dev_module_exit(void);

int __init madera_i2c_module_init(void);
void madera_i2c_module_exit(void);

int __init madera_pin_driver_init(void);
void madera_pin_driver_exit(void);

int __init madera_irq_driver_init(void);
void madera_irq_driver_exit(void);

int __init madera_gpio_driver_init(void);
void madera_gpio_driver_exit(void);

int __init arizona_ldo1_init(void);
void __exit madera_ldo1_exit(void);

int __init arizona_micsupp_init(void);
void __exit arizona_micsupp_exit(void);

int __init madera_extcon_driver_init(void);
void madera_extcon_driver_exit(void);

int __init cs47l92_codec_driver_init(void);
void cs47l92_codec_driver_exit(void);

