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
#ifndef __XR_MEDIA_HW_DVS_H__
#define __XR_MEDIA_HW_DVS_H__

#include "clk/xr-dvfs-private.h"
#include <linux/types.h>

#define MEDIA_LINK_CLK_NUM  3

enum mediabus_type {
	MEDIA1_BUS = 0,
	MEDIA2_BUS = 1,
	MEDIABUS_MAX,
};

enum {
	MEDIA_FIXED_VOLT_NONE = 0,
	MEDIA_FIXED_VOLT_0    = 1, /* 0.6V */
	MEDIA_FIXED_VOLT_1    = 2, /* 0.65V */
	MEDIA_FIXED_VOLT_2    = 3, /* 0.75V */
	MEDIA_FIXED_VOLT_MAX,
};

struct clock_mediabus {
	enum mediabus_type  bus_type;
	unsigned int        link_clk_num;
	struct clk          *bus_clk[MEDIA_LINK_CLK_NUM];
	unsigned int        volt_level;
	unsigned int        lowtemp_volt_level;
	unsigned long       sensitive_freq[MEDIA_LINK_CLK_NUM][MEDIA_DVFS_VOLT_LEVEL];
	struct list_head    mediabus_list;
};

#endif /* __XR_MEDIA_HW_DVS_H__ */
