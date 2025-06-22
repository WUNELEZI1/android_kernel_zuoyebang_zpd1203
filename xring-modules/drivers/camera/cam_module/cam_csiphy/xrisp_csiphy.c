// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sensor_csiphy", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sensor_csiphy", __func__, __LINE__

#include <linux/device.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include "xrisp_csiphy.h"
#include "xrisp_log.h"
#include "xrisp_sensor_dev.h"

static struct csiphy_dev *g_csiphy;

struct csiphy_dev *xrisp_csiphy_get_context(void)
{
	return g_csiphy;
}

bool xrisp_sensor_get_has_aoc_status(void)
{
	struct csiphy_dev *csiphy;
	struct cam_hw_info *hw_info;

	csiphy = xrisp_csiphy_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}
	hw_info = csiphy->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	return csiphy->hw_info->has_aoc;
}

int xrisp_csiphy_enable(void)
{
	int ret = 0;
	struct csiphy_dev *csiphy;
	struct cam_hw_info *hw_info;

	csiphy = xrisp_csiphy_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}
	hw_info = csiphy->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	mutex_lock(&csiphy->mlock);
	if (atomic_read(&csiphy->power_cnt)) {
		atomic_inc(&csiphy->power_cnt);
		XRISP_PR_INFO("csiphy already powerup count:%d.", atomic_read(&csiphy->power_cnt));
		mutex_unlock(&csiphy->mlock);
		return 0;
	}

	ret = xrisp_sensor_util_regulator_set_by_name(hw_info, "csiphy_vdd", 1);
	if (ret) {
		XRISP_PR_ERROR("csiphy enable regulator failed, ret:%d", ret);
		goto csiphy_unlock;
	}
	udelay(200);
	ret = xrisp_sensor_util_clk_set(hw_info, "clk_csi_sys", 1, CAM_NORMAL_VOTE);
	if (ret) {
		XRISP_PR_ERROR("csiphy enable clock failed, ret:%d", ret);
		goto regulator_disable;
	}

	atomic_inc(&csiphy->power_cnt);
	mutex_unlock(&csiphy->mlock);

	XRISP_PR_DEBUG("csiphy power up success.\n");
	return ret;

regulator_disable:
	xrisp_sensor_util_regulator_set_by_name(hw_info, "csiphy_vdd", 0);

csiphy_unlock:
	mutex_unlock(&csiphy->mlock);

	return ret;
}

int xrisp_csiphy_disable(void)
{
	int ret = 0;
	struct csiphy_dev *csiphy;
	struct cam_hw_info *hw_info;

	csiphy = xrisp_csiphy_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}
	hw_info = csiphy->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	mutex_lock(&csiphy->mlock);
	if (atomic_dec_return(&csiphy->power_cnt)) {
		if (atomic_read(&csiphy->power_cnt) < 0)
			atomic_set(&csiphy->power_cnt, 0);

		XRISP_PR_INFO("csiphy power count:%d.", atomic_read(&csiphy->power_cnt));
		mutex_unlock(&csiphy->mlock);
		return 0;
	}

	ret = xrisp_sensor_util_clk_set(hw_info, "clk_csi_sys", 0, CAM_LOW_VOTE);
	if (ret)
		XRISP_PR_ERROR("csiphy disable clock failed, ret:%d", ret);

	udelay(200);
	ret = xrisp_sensor_util_regulator_set_by_name(hw_info, "csiphy_vdd", 0);
	if (ret)
		XRISP_PR_ERROR("csiphy disable regulator failed, ret:%d", ret);


	mutex_unlock(&csiphy->mlock);

	XRISP_PR_DEBUG("csiphy power down.\n");
	return ret;
}

int xrisp_csiphy_disable_force(void)
{
	int ret = 0;
	struct csiphy_dev *csiphy;
	struct cam_hw_info *hw_info;

	csiphy = xrisp_csiphy_get_context();
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}
	hw_info = csiphy->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	mutex_lock(&csiphy->mlock);
	if (atomic_read(&csiphy->power_cnt) > 0) {
		ret = xrisp_sensor_util_clk_set(hw_info, "clk_csi_sys", 0, CAM_LOW_VOTE);
		if (ret)
			XRISP_PR_ERROR("csiphy disable clock failed, ret:%d", ret);

		udelay(200);
		ret = xrisp_sensor_util_regulator_set_by_name(hw_info, "csiphy_vdd", 0);
		if (ret)
			XRISP_PR_ERROR("csiphy disable regulator failed, ret:%d", ret);

		XRISP_PR_INFO("csiphy force power down count:%d.", atomic_read(&csiphy->power_cnt));
		atomic_set(&csiphy->power_cnt, 0);
	}
	mutex_unlock(&csiphy->mlock);
	return 0;
}

static int xrisp_csiphy_dts_parse(struct csiphy_dev *csiphy)
{
	int ret;
	struct device         *dev;

	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}

	dev = csiphy->dev;
	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}

	csiphy->hw_info = devm_kzalloc(dev, sizeof(struct cam_hw_info), GFP_KERNEL);
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy->hw_info)) {
		XRISP_PR_ERROR("csiphy->hw_info is null");
		return -ENOMEM;
	}

	ret = xrisp_sensor_util_get_dt_regulator_info(dev, csiphy->hw_info);
	if (ret) {
		XRISP_PR_ERROR("csiphy get regulator info failed.");
		return ret;
	}

	ret = xrisp_sensor_util_get_dt_clk_info(dev, csiphy->hw_info);
	if (ret) {
		XRISP_PR_ERROR("csiphy get clock info failed.");
		return ret;
	}

	XRISP_PR_INFO("csiphy dts parse ok!");
	return ret;

}

static int xrisp_csiphy_probe(struct platform_device *pdev)
{
	int ret;
	struct csiphy_dev *csiphy;

	csiphy = devm_kzalloc(&pdev->dev, sizeof(struct csiphy_dev), GFP_KERNEL);
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}

	csiphy->dev = &pdev->dev;

	ret = xrisp_csiphy_dts_parse(csiphy);
	if (ret) {
		XRISP_PR_ERROR("csiphy dts parse failed!");
		return -EINVAL;
	}

	ret = xrisp_sensor_util_get_clk_resource(&pdev->dev, csiphy->hw_info);
	if (ret) {
		XRISP_PR_ERROR("csiphy get clk resource failed!");
		return -EINVAL;
	}

	ret = xrisp_sensor_util_get_regulator_resource(&pdev->dev, csiphy->hw_info);
	if (ret) {
		XRISP_PR_ERROR("csiphy get regulator resource failed!");
		return -EINVAL;
	}

	mutex_init(&csiphy->mlock);
	atomic_set(&csiphy->power_cnt, 0);

	if (csiphy->hw_info != NULL) {
		if (csiphy->hw_info->has_aoc)
			xrisp_power_sync_init(csiphy->dev);
	}
	platform_set_drvdata(pdev, csiphy);
	g_csiphy = csiphy;

	XRISP_PR_INFO("csiphy probe success!");
	return ret;
}

static int xrisp_csiphy_remove(struct platform_device *pdev)
{
	struct csiphy_dev *csiphy;

	csiphy = platform_get_drvdata(pdev);
	if (XRISP_CHECK_NULL_RETURN_INT(csiphy)) {
		XRISP_PR_ERROR("csiphy is null");
		return -ENOMEM;
	}

	g_csiphy = NULL;
	XRISP_PR_INFO("xrisp csiphy remove success!\n");
	return 0;
}

static const struct of_device_id xrisp_csiphy_dt_match[] = {
	{.compatible = "xring,csi-phy"},
	{ }
};

MODULE_DEVICE_TABLE(of, xrisp_csiphy_dt_match);

static struct platform_driver xrisp_csiphy_driver = {
	.probe = xrisp_csiphy_probe,
	.remove = xrisp_csiphy_remove,
	.driver = {
		.name = "xring,csi-phy",
		.owner = THIS_MODULE,
		.of_match_table = xrisp_csiphy_dt_match,
	},
};

int xrisp_csiphy_init(void)
{
	int32_t ret = 0;

	ret = platform_driver_register(&xrisp_csiphy_driver);
	if (ret < 0)
		XRISP_PR_ERROR("platform_driver_register failed, ret =%d", ret);
	return ret;
}

void xrisp_csiphy_exit(void)
{
	platform_driver_unregister(&xrisp_csiphy_driver);
}

MODULE_DESCRIPTION("xrisp csiphy driver");
MODULE_LICENSE("GPL");
