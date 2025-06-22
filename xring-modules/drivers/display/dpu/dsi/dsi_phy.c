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

#include <linux/of_device.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "hw/dsi_version_ctrl.h"
#include "dpu_module_drv.h"
#include "dpu_log.h"
#include "dsi_phy.h"

static LIST_HEAD(dsi_phy_list);
static DEFINE_MUTEX(dsi_phy_lock);

int dsi_phy_id_get(struct dsi_phy *phy)
{
	if (!phy) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	return phy->phy_hw.hw_blk.blk_id;
}

int dsi_phy_init(struct dsi_phy *phy, struct dsi_phy_cfg *cfg)
{
	struct dsi_phy_hw *phy_hw;
	int ret = 0;

	if (!phy || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", phy, cfg);
		return -EINVAL;
	}

	phy_hw = &phy->phy_hw;

	if (phy_hw->ops.init) {
		ret = phy_hw->ops.init(&phy_hw->hw_blk, cfg);
		if (ret) {
			DSI_ERROR("failed to init phy hw\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no phy operation: init\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_phy_wait_ready(struct dsi_phy *phy, struct dsi_phy_cfg *cfg)
{
	struct dsi_phy_hw *phy_hw;
	int ret = 0;

	if (!phy || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", phy, cfg);
		return -EINVAL;
	}

	phy_hw = &phy->phy_hw;

	if (phy_hw->ops.wait_ready) {
		ret = phy_hw->ops.wait_ready(&phy_hw->hw_blk, cfg);
		if (ret) {
			DSI_ERROR("failed to init phy hw\n");
			return -EINVAL;
		}
	} else {
		DSI_ERROR("no phy operation: wait_ready\n");
		return -EINVAL;
	}

	return 0;
}

void dsi_phy_add(struct dsi_phy *phy)
{
	mutex_lock(&dsi_phy_lock);
	list_add_tail(&phy->list, &dsi_phy_list);
	mutex_unlock(&dsi_phy_lock);
}

void dsi_phy_remove(struct dsi_phy *phy)
{
	mutex_lock(&dsi_phy_lock);
	list_del_init(&phy->list);
	mutex_unlock(&dsi_phy_lock);
}

#ifdef CONFIG_OF
struct dsi_phy *of_find_dsi_phy(const struct device_node *np)
{
	struct dsi_phy *phy = NULL;
	bool found = false;

	if (!of_device_is_available(np))
		return ERR_PTR(-ENODEV);

	mutex_lock(&dsi_phy_lock);
	list_for_each_entry(phy, &dsi_phy_list, list) {
		if (phy->dev->of_node == np) {
			found = true;
			break;
		}
	}
	mutex_unlock(&dsi_phy_lock);

	if (!found)
		phy = ERR_PTR(-ENODEV);

	return phy;
}
#endif

static int dsi_phy_pwr_init(struct platform_device *pdev,
		struct dsi_phy *phy)
{
	/* TODO: implement power supplies here */
	int ret = 0;

	DSI_DEBUG("power supplies init\n");
	return ret;
}

static int dsi_phy_settings_init(struct platform_device *pdev,
				 struct dsi_phy *phy)
{
	int ret = 0;

	DSI_DEBUG("phy_settings init\n");
	return ret;
}

static int dsi_phy_obj_init(struct dsi_phy *phy, struct platform_device *pdev)
{
	int ret = 0;

	if (!of_device_is_available(pdev->dev.of_node)) {
		DSI_ERROR("no device of node\n");
		return -ENODEV;
	}

	if (dsi_phy_hw_parse(pdev, &phy->phy_hw)) {
		DSI_ERROR("failed to parse phy hardware info\n");
		return -EINVAL;
	}

	/* regist hw callback func */
	if (dsi_phy_version_ctrl(&phy->phy_hw)) {
		DSI_ERROR("phy hw version ctrl failed\n");
		return -EINVAL;
	}

	ret = dsi_phy_pwr_init(pdev, phy);
	if (ret) {
		DSI_ERROR("failed to parse phy voltage supplies\n");
		return -EINVAL;
	}

	ret = dsi_phy_settings_init(pdev, phy);
	if (ret) {
		DSI_ERROR("Failed to parse phy setting\n");
		return -EINVAL;
	}

	INIT_LIST_HEAD(&phy->list);

	phy->dev = &pdev->dev;
	return 0;
}

static int dsi_phy_pdev_probe(struct platform_device *pdev)
{
	struct dsi_phy *phy;
	int ret;

	if (!pdev || !pdev->dev.of_node) {
		DSI_ERROR("phy pdev not found\n");
		return -ENODEV;
	}

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	ret = dsi_phy_obj_init(phy, pdev);
	if (ret) {
		DSI_ERROR("failed to init dsi phy\n");
		goto error;
	}

	dsi_phy_add(phy);

	platform_set_drvdata(pdev, phy);
	return 0;

error:
	kfree(phy);
	return ret;
}

static int dsi_phy_pdev_remove(struct platform_device *pdev)
{
	struct dsi_phy *phy;

	phy = platform_get_drvdata(pdev);

	dsi_phy_remove(phy);

	kfree(phy);

	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id dsi_phy_dt_match[] = {
	{ .compatible = "xring,dsi-phy" },
	{},
};

static struct platform_driver dsi_phy_platform_driver = {
	.probe = dsi_phy_pdev_probe,
	.remove = dsi_phy_pdev_remove,
	.driver = {
		.name = "xring_dsi_phy",
		.of_match_table = dsi_phy_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init dsi_phy_drv_register(void)
{
	return platform_driver_register(&dsi_phy_platform_driver);
}

void __exit dsi_phy_drv_unregister(void)
{
	platform_driver_unregister(&dsi_phy_platform_driver);
}
