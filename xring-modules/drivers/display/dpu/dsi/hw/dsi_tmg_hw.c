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

#include <linux/of_address.h>
#include <linux/of.h>

#include "dsi_tmg_hw.h"
#include "dsi_common.h"
#include "dpu_log.h"

static int dsi_tmg_hw_parse_hw_blk(struct platform_device *pdev,
		struct device_node *np, struct dpu_hw_blk *hw_blk)
{
	struct resource res;
	u32 index;
	u32 size;
	int ret;

	if (of_property_read_u32(np, "cell-index", &index))
		index = 0;

	ret = of_address_to_resource(np, 0, &res);
	if (ret != 0) {
		DSI_ERROR("failed to get memory resource\n");
		return -EINVAL;
	}

	size = resource_size(&res);
	hw_blk->blk_len = size;
	hw_blk->blk_id = index;
	hw_blk->base_addr = MEM_DPU_BASE_ADDR;
	hw_blk->blk_offset = res.start - MEM_DPU_BASE_ADDR;
	DSI_DEBUG("tmg base: base addr:0x%x, len:%d, offset:0x%x\n",
			hw_blk->base_addr, hw_blk->blk_len,
			hw_blk->blk_offset);

	return 0;
}

int dsi_tmg_hw_parse(struct platform_device *pdev,
		struct device_node *np, struct dsi_tmg_hw *tmg_hw)
{
	int ret = 0;

	ret = dsi_tmg_hw_parse_hw_blk(pdev, np, &tmg_hw->hw_blk);
	if (ret) {
		DSI_ERROR("parse tmg hw blk failed\n");
		return ret;
	}

	return 0;
}
