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
#include "xr-common-hw-dvs.h"
#include "clk/clk.h"
#include "clk/xr-clk-common.h"
#include "clk/xr-dvfs-private.h"
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <soc/xring/xr-clk-provider.h>

int dvs_is_low_temperature(const struct dvs_config *const dvs_cfg)
{
	unsigned int temperature;
	int ret = 0;

	if (unlikely(!dvs_cfg))
		return 0;

	temperature = readl(dvs_cfg->lowtemp_offset + dvs_cfg->reg_base);
	temperature &= dvs_cfg->lowtemp_mask;
	temperature >>= dvs_cfg->lowtemp_shift;
	if (temperature != NORMAL_TEMPRATURE)
		ret = LOW_TEMPRATURE;

	return ret;
}
EXPORT_SYMBOL_GPL(dvs_is_low_temperature);

void dvs_per_vote_record(const struct dvs_channel *const dvs_channel,
			 const struct dvs_config *const dvs_cfg)
{
	unsigned int reg_value;
	unsigned int record_mask, record_shift;

	if (unlikely(!dvs_channel) || unlikely(!dvs_cfg))
		return;

	record_mask = dvs_channel->record_mask;
	if (unlikely(!record_mask))
		return;
	record_shift = dvs_channel->record_shift;

	reg_value = ((dvs_channel->volt_level << record_shift) & record_mask);
	reg_value |= (record_mask << HIMASK_OFFSET);
	writel(reg_value, dvs_cfg->reg_base + dvs_channel->record_reg_offset);
}
EXPORT_SYMBOL_GPL(dvs_per_vote_record);

int dvs_get_base(const struct device_node *const np, void __iomem **base)
{
	unsigned int clk_base_id;

	if (unlikely(!np) || unlikely(!base))
		return -EINVAL;

	if (of_property_read_u32(np, "xring,base-addr", &clk_base_id)) {
		clkerr("%s node doesn't have %s\n", np->name, "xring,base-addr");
		return -EINVAL;
	}
	if (clk_base_id >= XR_CLK_MAX_BASECRG) {
		clkerr("%s clk base id illegal %u\n", np->name, clk_base_id);
		return -EINVAL;
	}
	*base = get_xr_clk_base(clk_base_id);

	return 0;
}
EXPORT_SYMBOL_GPL(dvs_get_base);
