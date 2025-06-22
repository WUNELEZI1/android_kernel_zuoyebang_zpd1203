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
#ifndef __XR_CLK_DVFS_H__
#define __XR_CLK_DVFS_H__

#include <linux/clk-provider.h>

#include "xr-dvfs-private.h"
#include "xr-dvfs-vote.h"

struct xr_dvfs_clk {
	struct clk_hw      hw;
	struct clk_hw      **friend_hw;
	unsigned int	   vote_id;
	unsigned int	   vote_type;
	unsigned int	   sensitive_level;
	unsigned int	   sensitive_volt[DVFS_MAX_VOLT_NUM];
	unsigned long	   sensitive_freq[DVFS_MAX_FREQ_NUM];
	unsigned long	   lowtemp_max_freq;
	unsigned int	   lowtemp_property;
};

struct dvfs_cfg {
	unsigned long	sensitive_freq[DVFS_MAX_FREQ_NUM];
	unsigned int	sensitive_volt[DVFS_MAX_VOLT_NUM];
	unsigned int	sensitive_level;
	unsigned long	lowtemp_max_freq;
	unsigned int	lowtemp_property;
};

struct clk_hw *devm_xr_clk_hw_dvfs_clock(struct device *dev, const char *name,
		struct clk_hw **friend_hw, unsigned int vote_id,
		unsigned int vote_type, const struct dvfs_cfg *dvs_cfg);
#endif /* __XR_CLK_DVFS_H__ */
