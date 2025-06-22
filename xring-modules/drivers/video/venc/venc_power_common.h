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
#ifndef _VENC_POWER_COMMON_H_
#define _VENC_POWER_COMMON_H_

#include <linux/regulator/consumer.h>
#include "venc_clk_manager.h"

#define VENC_POWER_MAX_NUM 2
#define VENC_CLK_NAME_LEN 128

typedef enum {
	POWER_VENC_SUBSYS,
	POWER_MEDIA2_SUBSYS,
	POWER_DOMAIN_MAX,
} venc_power_domain_e;

typedef struct {
	struct clk *clk;
	char clk_name[VENC_CLK_NAME_LEN];
	unsigned long work_rate;
	unsigned long default_rate;
} venc_clock;

typedef struct {
	struct device *dev;
	venc_clock *venc_clk;
	atomic_t power_count[VENC_POWER_MAX_NUM];
	struct mutex pm_lock;
	struct regulator *venc_rg;
	struct regulator *media2_rg;
	bool always_on_curr_state;
	bool is_suspended;
	bool is_poweroff;
} venc_power_mgr;

struct hantroenc_dev {
	struct device *dev;
	void *priv_data;
	venc_power_mgr *pm;
	venc_clk_mgr *cm;
};

#endif
