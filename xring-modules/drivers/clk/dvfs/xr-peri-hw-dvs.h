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
#ifndef __XR_PERI_HW_DVS_H__
#define __XR_PERI_HW_DVS_H__

enum {
	PERI_FIXED_VOLT_NONE = 0,
	PERI_FIXED_VOLT_0 = 1, /* 0.6v */
	PERI_FIXED_VOLT_1 = 2, /* 0.65v */
	PERI_FIXED_VOLT_2 = 3, /* 0.7v */
	PERI_FIXED_VOLT_3 = 4, /* 0.8v */
	PERI_FIXED_VOLT_MAX,
};

#endif /* __XR_PERI_HW_DVS_H__ */
