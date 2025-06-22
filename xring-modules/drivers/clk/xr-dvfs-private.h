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
#ifndef __XR_DVFS_PRIVATE_H__
#define __XR_DVFS_PRIVATE_H__

enum {
	MEDIA_DVFS_VOTE,
	PERI_DVFS_VOTE,
	MAX_DVFS_VOTE
};

enum {
	MEDIA_VOLT_0 = 0,
	MEDIA_VOLT_1,
	MEDIA_VOLT_2,
	MEDIA_VOLT_MAX,
};

enum {
	PERI_VOLT_0 = 0,
	PERI_VOLT_1,
	PERI_VOLT_2,
	PERI_VOLT_3,
	PERI_VOLT_MAX,
};

enum {
	VOLT_LEVEL_0 = 0,
	VOLT_LEVEL_1,
	VOLT_LEVEL_2,
	VOLT_LEVEL_3,
	VOLT_LEVEL_MAX,
};

#define MAX_CHANNEL_NUM                 8
#define DVFS_MAX_FREQ_NUM               4
#define DVFS_MAX_VOLT_NUM               4

#define MEDIA_DVFS_VOLT_LEVEL           3
#define PERI_DVFS_VOLT_LEVEL            4

#define FREQ_CONVERSION_COEFF           1000

#define MAX_PROPERTY_STRLEN             64
#define MNTN_REG_MASK                   0xffff

#define max_value(v1, v2)               ((v1) > (v2) ? (v1) : (v2))

#endif /* __XR_DVFS_PRIVATE_H__ */
