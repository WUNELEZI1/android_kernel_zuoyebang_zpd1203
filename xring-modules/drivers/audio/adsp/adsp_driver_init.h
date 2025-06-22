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
int adsp_boot_init(void);
int adsp_ssr_init(void);
int xring_adsp_driver_init(void);

void adsp_boot_exit(void);
void adsp_ssr_exit(void);
void xring_adsp_driver_exit(void);

