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

#include <linux/of.h>

#include "dsi_ctrl_hw.h"
#include "dsi_common.h"
#include "dpu_log.h"

static int dsi_ctrl_hw_blk_parse_sub(struct platform_device *pdev,
		struct dpu_hw_blk *hw_blk, const char *name, bool need_remap)
{
	void __iomem *iomem_ptr;
	struct resource *res;
	u32 index;
	u32 size;

	if (of_property_read_u32(pdev->dev.of_node, "cell-index", &index))
		index = 0;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		DSI_ERROR("failed to get memory resource: %s\n", name);
		return -EINVAL;
	}

	size = resource_size(res);

	hw_blk->blk_len = size;
	hw_blk->blk_id = index;
	hw_blk->base_addr = MEM_DPU_BASE_ADDR;
	hw_blk->blk_offset = res->start - MEM_DPU_BASE_ADDR;

	if (need_remap) {
		iomem_ptr = ioremap(res->start, size);
		if (!iomem_ptr) {
			DSI_ERROR("failed to remap blk: %s\n", name);
			return -ENOMEM;
		}
		hw_blk->iomem_base = iomem_ptr;
		hw_blk->base_addr = res->start;
		hw_blk->blk_offset = 0;
	}

	DSI_DEBUG("%s base_addr: 0x%x, len:%d, offset:0x%x\n",
			name, hw_blk->base_addr, hw_blk->blk_len,
			hw_blk->blk_offset);
	return 0;
}

static int dsi_ctrl_hw_blk_parse(struct platform_device *pdev,
		struct dsi_ctrl_hw *ctrl_hw)
{
	static const char *ctrl_base_name = "ctrl_base";
	static const char *sctrl_base_name = "sctrl_base";
	static const char *dctrl_base_name = "dctrl_base";
	static const char *dsctrl_base_name = "dsctrl_base";

	int ret = 0;

	/* TODO: avoid remap the ctrl/sctrl twice */
	ret = dsi_ctrl_hw_blk_parse_sub(pdev,
			&ctrl_hw->ctrl_blk, ctrl_base_name, true);
	if (ret) {
		DSI_ERROR("parse ctrl hw blk failed\n");
		return -EINVAL;
	}

	/* TODO: avoid remap the ctrl/sctrl twice */
	ret = dsi_ctrl_hw_blk_parse_sub(pdev,
			&ctrl_hw->sctrl_blk, sctrl_base_name, true);
	if (ret) {
		DSI_ERROR("parse sctrl hw blk failed\n");
		return -EINVAL;
	}

	ret = dsi_ctrl_hw_blk_parse_sub(pdev,
			&ctrl_hw->dctrl_blk, dctrl_base_name, false);
	if (ret) {
		DSI_ERROR("parse dctrl hw blk failed\n");
		return -EINVAL;
	}

	ret = dsi_ctrl_hw_blk_parse_sub(pdev,
			&ctrl_hw->dsctrl_blk, dsctrl_base_name, false);
	if (ret) {
		DSI_ERROR("parse dsctrl hw blk failed\n");
		return -EINVAL;
	}

	return ret;
}

int dsi_ctrl_hw_parse(struct platform_device *pdev,
		struct dsi_ctrl_hw *ctrl_hw)
{
	const char *ctrl_hw_version;

	dsi_ctrl_hw_blk_parse(pdev, ctrl_hw);

	ctrl_hw->ctrl_blks.ctrl_blk = &ctrl_hw->ctrl_blk;
	ctrl_hw->ctrl_blks.sctrl_blk = &ctrl_hw->sctrl_blk;

	ctrl_hw->ctrl_blks.dctrl_blk = &ctrl_hw->dctrl_blk;
	ctrl_hw->ctrl_blks.dsctrl_blk = &ctrl_hw->dsctrl_blk;

	ctrl_hw_version = of_get_property(pdev->dev.of_node, "vesrion", NULL);
	if (!ctrl_hw_version) {
		ctrl_hw->version = DSI_CTRL_VERSION_UNKNOWN;
		DSI_ERROR("unable to get ctrl version dts property\n");
	} else {
		if (!strcmp(ctrl_hw_version, "v2.0"))
			ctrl_hw->version = DSI_CTRL_VERSION_2_0;
	}

	DSI_DEBUG("get ctrl version :%d\n", ctrl_hw->version);

	return 0;
}
