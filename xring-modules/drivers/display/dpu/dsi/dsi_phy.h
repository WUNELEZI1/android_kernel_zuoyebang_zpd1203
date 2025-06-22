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

#ifndef _DSI_PHY_H_
#define _DSI_PHY_H_

#include "hw/dsi_phy_hw.h"
#include "dsi_common.h"

/**
 * phy_engine_state - define engine status for dsi phy.
 * @DSI_PHY_ENGINE_OFF:  Engine is turned off.
 * @DSI_PHY_ENGINE_ON:   Engine is turned on.
 * @DSI_PHY_ENGINE_MAX:  Maximum value.
 */
enum phy_engine_state {
	DSI_PHY_ENGINE_OFF = 0,
	DSI_PHY_ENGINE_ON,
	DSI_PHY_ENGINE_MAX,
};

/**
 * dsi_phy_hw_features - features supported by DSI PHY hardware
 * @DSI_PHY_DPHY:        Supports DPHY
 * @DSI_PHY_CPHY:        Supports CPHY
 * @DSI_PHY_SPLIT_LINK:  Supports Split Link
 * @DSI_PHY_MAX_FEATURES:
 */
enum dsi_phy_hw_features {
	DSI_PHY_DPHY,
	DSI_PHY_CPHY,
	DSI_PHY_SPLIT_LINK,
	DSI_PHY_MAX_FEATURES
};

/**
 * dsi_phy - dsi phy structure
 * @dev: parent device of dsi phy
 * @hw_phy: phy hardware instance
 * @ver_info: phy version spec info
 * @list: phy entry in registry
 */
struct dsi_phy {
	struct device *dev;

	struct dsi_phy_hw phy_hw;

	struct list_head list;
};

/**
 * dsi_phy_id_get - get the phy hardware id
 * @phy: the dsi phy phandle
 *
 * Return: hardware id or a negative number on failure.
 */
int dsi_phy_id_get(struct dsi_phy *phy);

/**
 * dsi_phy_add - add the dsi phy to the phy list
 * @phy: the dsi phy phandle
 */
void dsi_phy_add(struct dsi_phy *phy);

/**
 * dsi_phy_remove - remove the dsi phy from the phy list
 * @phy: the dsi phy phandle
 */
void dsi_phy_remove(struct dsi_phy *phy);

/**
 * dsi_phy_init - init the phy hw
 * @phy: the dsi phy phandle
 * @cfg: the pointer of dsi phy configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_phy_init(struct dsi_phy *phy, struct dsi_phy_cfg *cfg);

/**
 * dsi_phy_wait_ready - wait phy hw ready
 * @phy: the dsi phy phandle
 * @cfg: the pointer of dsi phy configure
 *
 * Return: 0 on ready or a negative number on busy.
 */
int dsi_phy_wait_ready(struct dsi_phy *phy, struct dsi_phy_cfg *cfg);

#if defined(CONFIG_OF)
/**
 * of_find_dsi_phy - find the dsi phy by device node from the phy list
 * @np: the device node pointer
 *
 * Return: dsi phy pointer on success, error pointer on failure
 */
struct dsi_phy *of_find_dsi_phy(const struct device_node *np);
#else
struct dsi_phy *of_find_dsi_phy(const struct device_node *np)
{
	return ERR_PTR(-ENODEV);
}
#endif

#endif /* _DSI_PHY_H_ */
