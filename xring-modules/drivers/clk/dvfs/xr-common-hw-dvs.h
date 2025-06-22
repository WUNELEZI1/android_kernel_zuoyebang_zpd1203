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
#ifndef __XR_COMMON_HW_DVS_H__
#define __XR_COMMON_HW_DVS_H__

#include <linux/types.h>
#include <linux/of.h>

#define MAX_DVS_PROT_STRLEN   64

struct dvs_channel {
	unsigned int      channel_id;
	void __iomem      *reg_base;
	unsigned int      volt_level;
	unsigned int      record_reg_offset;
	unsigned int      record_mask;
	unsigned int      record_shift;
	struct list_head  chan_list;
};

struct dvs_config {
	void __iomem      *reg_base;
	unsigned int      lowtemp_offset;
	unsigned int      lowtemp_mask;
	unsigned int      lowtemp_shift;
	unsigned int      dvs_timeout_irq;
};

struct dvs_mntn {
	void __iomem      *reg_base;
	unsigned int      volt_level_fixed;
	unsigned int      fixed_volt_offset;
	unsigned int      fixed_volt_mask;
	unsigned int      fixed_volt_shift;
};

int dvs_is_low_temperature(const struct dvs_config *const dvs_cfg);
void dvs_per_vote_record(const struct dvs_channel *const dvs_channel,
			const struct dvs_config *const dvs_cfg);
int dvs_get_base(const struct device_node *const np, void __iomem **base);

#endif /* __XR_COMMON_HW_DVS_H__ */
