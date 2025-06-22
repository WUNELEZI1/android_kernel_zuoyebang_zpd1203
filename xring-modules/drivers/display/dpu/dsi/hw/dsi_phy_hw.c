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

#include "dpu_dt_util.h"
#include "dsi_phy_hw.h"
#include "dsi_common.h"
#include "dpu_log.h"

static int dsi_phy_hw_blk_parse(struct platform_device *pdev,
		struct dpu_hw_blk *hw_phy)
{
	static const char *name = "phy_base";
	void __iomem *iomem_ptr;
	struct resource *res;
	u32 index;
	u32 size;
	int ret = 0;

	ret = of_property_read_u32(pdev->dev.of_node, "cell-index", &index);
	if (ret) {
		DSI_DEBUG("set default dsi phy index to 0\n");
		index = 0;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		DSI_ERROR("failed to get memory resource: %s\n", name);
		return -EINVAL;
	}

	size = resource_size(res);

	iomem_ptr = ioremap(res->start, size);
	if (!iomem_ptr) {
		DSI_ERROR("failed to remap blk: %s\n", name);
		return -ENOMEM;
	}

	hw_phy->iomem_base = iomem_ptr;
	hw_phy->blk_len = size;
	hw_phy->blk_id = index;
	hw_phy->base_addr = res->start;
	DSI_DEBUG("%s base_addr: 0x%x, len: %d, offset: 0x%x\n", name, hw_phy->base_addr, hw_phy->blk_len,
			hw_phy->blk_offset);
	return 0;
}

static void dsi_phy_version_parse(struct device_node *np,
		struct dsi_phy_hw *phy_hw)
{
	const char *version;

	version = of_get_property(np, "vesrion", NULL);
	if (!version) {
		phy_hw->version = DSI_PHY_VERSION_UNKNOWN;
		DSI_ERROR("unable to get phy version dts property\n");
	} else {
		if (!strcmp(version, "v1.2")) {
			phy_hw->version = DSI_PHY_VERSION_1_2;
		} else {
			phy_hw->version = DSI_PHY_VERSION_UNKNOWN;
			DSI_ERROR("get unknown phy version\n");
		}
	}

	DSI_DEBUG("get phy version :%d\n", phy_hw->version);
}

static void dsi_phy_ppi_width_parse(struct device_node *np,
		struct dsi_phy_hw *phy_hw)
{
	int ppi_width;
	int ret = 0;

	ret = of_property_read_u32(np, "ppi-width", &ppi_width);
	if (ret) {
		DSI_DEBUG("read dsi phy ppi_width failed, set default to 8\n");
		ppi_width = 8;
	}

	switch (ppi_width) {
	case (8):
		phy_hw->ppi_width = PHY_PPI_WIDTH_8BIT;
		break;
	case (16):
		phy_hw->ppi_width = PHY_PPI_WIDTH_16BIT;
		break;
	case (32):
		phy_hw->ppi_width = PHY_PPI_WIDTH_32BIT;
		break;
	default:
		DSI_DEBUG("get invalid ppi width, set default as 8");
		phy_hw->ppi_width = PHY_PPI_WIDTH_8BIT;
		break;
	}

	DSI_DEBUG("get phy ppi width enum:%d(0 == 8BIT)\n", phy_hw->ppi_width);
}

static int dsi_phy_hw_platform_parse(struct device_node *np,
		struct dsi_phy_hw *phy_hw)
{
	const char *hw_platform;
	int ret = 0;

	hw_platform = of_get_property(np, "hw-platform", NULL);
	if (!hw_platform) {
		phy_hw->hw_platform = XILINX_DPHY;
		DSI_INFO("set default phy select as xilinx dphy\n");
	} else {
		if (!strcmp(hw_platform, "snps_asic")) {
			phy_hw->hw_platform = SNPS_CDPHY_ASIC;
		} else if (!strcmp(hw_platform, "snps_fpga")) {
			phy_hw->hw_platform = SNPS_CDPHY_FPGA;
		} else if (!strcmp(hw_platform, "xilinx")) {
			phy_hw->hw_platform = XILINX_DPHY;
		} else {
			DSI_ERROR("get invalid phy version\n");
			ret = -EINVAL;
			goto error;
		}
	}

	DSI_DEBUG("get phy hw platform: %s\n", hw_platform);

error:
	return ret;
}

int dsi_phy_hw_parse(struct platform_device *pdev,
		struct dsi_phy_hw *phy_hw)
{
	struct device_node *np;
	int ret = 0;

	if (!pdev || !phy_hw) {
		DSI_ERROR("invalid parameters\n");
		ret = -EINVAL;
		goto error;
	}

	np = pdev->dev.of_node;
	ret = dsi_phy_hw_platform_parse(np, phy_hw);
	if (ret) {
		DSI_ERROR("failed to parse phy hw platform\n");
		goto error;
	}

	switch (phy_hw->hw_platform) {
	case SNPS_CDPHY_ASIC:
	case SNPS_CDPHY_FPGA:
		ret = dsi_phy_hw_blk_parse(pdev, &phy_hw->hw_blk);
		if (ret) {
			DSI_ERROR("parse phy hw blk failed\n");
			goto error;
		};
		break;
	case XILINX_DPHY:
		dsi_phy_hw_blk_parse(pdev, &phy_hw->hw_blk);
		DSI_DEBUG("hw address for xilinx phy is useless\n");
		break;
	default:
		DSI_ERROR("error phy select type:%d\n", phy_hw->hw_platform);
		goto error;
	}

	dsi_phy_version_parse(np, phy_hw);
	DSI_DEBUG("get phy hw_platform:%d, phy version:%d\n",
			phy_hw->hw_platform, phy_hw->version);

	dsi_phy_ppi_width_parse(np, phy_hw);

error:
	return ret;
}
