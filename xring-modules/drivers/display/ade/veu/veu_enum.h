/* SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _VEU_ENUM_H_
#define _VEU_ENUM_H_

enum PIPE_EN {
	PREALPHA_EN = 1 << 0,
	NONPREALPHA_EN = 1 << 1,
	CSC_EN = 1 << 2,
	DITHER_EN = 1 << 3,
	Y2R_EN = 1 << 4,
	R2Y_EN = 1 << 5,
	CROP1_EN = 1 << 8,
	CROP2_EN = 1 << 9,
};

enum VEU_MODULE_TYPE {
	MODULE_RDMA = 0,
	MODULE_WDMA,
	MODULE_NUM = 2,
};

enum VEU_PROFILE_LEVEL {
	PROFILE_LVL_INVALID = -1,
	PROFILE_LVL_0 = 0,
	PROFILE_LVL_1 = 1,
	PROFILE_LVL_2 = 2,
	PROFILE_LVL_MAX = PROFILE_LVL_2,
};

enum VEU_POWER_MODE {
	POWER_MODE_INVALID = -1,
	POWER_MODE_OFF = 0,
	POWER_MODE_ON = 1,
	POWER_MODE_MAX = POWER_MODE_ON,
};

#endif // _VEU_ENUM_H_
