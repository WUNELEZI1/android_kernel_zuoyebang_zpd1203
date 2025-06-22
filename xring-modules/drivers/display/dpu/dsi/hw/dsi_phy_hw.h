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

#ifndef _DSI_PHY_HW_H_
#define _DSI_PHY_HW_H_

#include <linux/platform_device.h>

#include "dsi_common.h"
#include "dsi_hw_phy_ops.h"
#include "dpu_reg_ops.h"
#include "dpu_hw_dsi.h"

/**
 * enum dsi_phy_version - DSI PHY version enumeration
 * @DSI_PHY_VERSION_UNKNOWN:    Unknown version.
 * @DSI_PHY_VERSION_1_2:        version 1_2
 * @DSI_PHY_VERSION_MAX:
 */
enum dsi_phy_version {
	DSI_PHY_VERSION_UNKNOWN = 0,
	DSI_PHY_VERSION_1_2,
	DSI_PHY_VERSION_MAX
};

/**
 * struct dsi_phy_hw_ops - Operations for DSI PHY hardware.
 */
struct dsi_phy_hw_ops {
	int (*init)(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
	void (*deinit)(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
	int (*wait_ready)(struct dpu_hw_blk *hw, struct dsi_phy_cfg *cfg);
};

/**
 * struct dsi_phy_hw - DSI phy hardware object specific to an instance

 * @hw_blk: dsi_phy hw blk object
 * @hw_platform: hardware platform info
 * @version: version info of phy
 * @ops: Function pointer to PHY operations.
 */
struct dsi_phy_hw {
	struct dpu_hw_blk hw_blk;

	enum phy_select hw_platform;
	enum dsi_phy_version version;
	enum dsi_phy_ppi_width ppi_width;

	struct dsi_phy_hw_ops ops;
};

/**
 * dsi_phy_hw_parse - parse dt and initialize the dsi phy hw object
 * @pdev: the parent platform device for dsi phy
 * @phy_hw: the pointer of phy hw object
 *
 * Return: zero on success, -ERRNO on failure
 */
int dsi_phy_hw_parse(struct platform_device *pdev,
		struct dsi_phy_hw *phy_hw);

#endif
