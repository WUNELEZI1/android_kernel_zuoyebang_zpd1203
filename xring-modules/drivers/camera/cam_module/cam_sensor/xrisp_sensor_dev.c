// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sensor_dev", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sensor_dev", __func__, __LINE__

#include <linux/device.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include "xrisp_sensor_dev.h"
#include "xrisp_log.h"

static struct cam_pm_dev *g_cam_sensor_tab[CAM_SENSOR_NUM_MAX];

struct cam_pm_dev *xrisp_sensor_get_context(uint32_t idx)
{
	if (idx < CAM_SENSOR_NUM_MAX)
		return g_cam_sensor_tab[idx];

	return NULL;
}

static int xrisp_sensor_gpio_pin_init_one(struct device *dev,
	const char *name, enum xr_camera_power_seq_type type,
	struct cam_gpio_info *gpio_info, struct cam_power_ctrl_gpio *gpio_seq)
{
	int32_t  ret;
	int32_t  val;
	struct device_node *of_node;
	uint32_t gpio_array_size;
	struct gpio  *gpio_tbl;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(name)) {
		XRISP_PR_ERROR("name is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_info)) {
		XRISP_PR_ERROR("gpio_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_seq)) {
		XRISP_PR_ERROR("gpio_seq is null");
		return -ENOMEM;
	}
	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}
	gpio_tbl = gpio_info->gpio_tbl;
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_tbl)) {
		XRISP_PR_ERROR("gpio_tbl is null");
		return -ENOMEM;
	}

	if (type < SENSOR_DVDD || type >= SENSOR_SEQ_TYPE_MAX) {
		XRISP_PR_ERROR("Invalid seq type:%d.", type);
		return -EINVAL;
	}
	gpio_array_size = gpio_info->gpio_tbl_size;

	ret = of_property_read_u32(of_node, name, &val);
	if (ret != -EINVAL) {
		if (ret < 0) {
			XRISP_PR_ERROR("get %s failed ret:%d.", name, ret);
			return ret;
		} else if (val >= gpio_array_size) {
			XRISP_PR_ERROR("%s invalid val:%d.", name, val);
			return -EINVAL;
		}
		gpio_seq->gpio_num[type] = gpio_tbl[val].gpio;
		gpio_seq->valid[type] = 1;
	}

	return 0;
}

static int xrisp_sensor_gpio_pin_init(struct cam_pm_dev *cam_pm)
{
	int32_t  ret = 0;
	int i;
	struct device         *dev;
	struct cam_hw_info    *hw_info;
	struct cam_gpio_info  *gpio_info;
	struct cam_power_ctrl_gpio *gpio_seq;
	uint32_t gpio_array_size;
	struct cam_seq_name gpio_tab[] = {
		{"gpio-dvdd", SENSOR_DVDD},
		{"gpio-vana", SENSOR_AVDD},
		{"gpio-bob1", SENSOR_BOB1},
		{"gpio-af", SENSOR_AF},
		{"gpio-vois", SENSOR_OIS},
		{"gpio-vio", SENSOR_CAMIO},
		{"gpio-bob2", SENSOR_BOB2},
		{"gpio-rst", SENSOR_RESET},
		{"gpio-mclk", SENSOR_MCLK},
		{"gpio-standby", SENSOR_STANDBY},
		{"gpio-apert", SENSOR_APERT},
		{"gpio_custom1", SENSOR_CUSTOM_GPIO1},
		{"gpio_custom2", SENSOR_CUSTOM_GPIO2},
	};

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	hw_info = cam_pm->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	gpio_info = &hw_info->gpio_info;
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_info)) {
		XRISP_PR_ERROR("gpio_info is null");
		return -ENOMEM;
	}
	dev = cam_pm->dev;
	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}

	gpio_array_size = gpio_info->gpio_tbl_size;
	if (!gpio_array_size) {
		XRISP_PR_ERROR("Invalid size of gpio table");
		return -EINVAL;
	}

	gpio_seq = &cam_pm->power_info.gpio_seq;
	memset(gpio_seq, 0x0, sizeof(struct cam_power_ctrl_gpio));

	for (i = 0; i < ARRAY_SIZE(gpio_tab); i++) {
		ret = xrisp_sensor_gpio_pin_init_one(dev,
			gpio_tab[i].name, gpio_tab[i].seq_type,
			gpio_info, gpio_seq);
		if (ret) {
			memset(gpio_seq, 0x0, sizeof(struct cam_power_ctrl_gpio));
			return ret;
		}
	}

	return ret;
}

static int xrisp_sensor_dts_parse(struct cam_pm_dev *cam_pm)
{
	int ret;
	struct device         *dev;
	struct device_node    *of_node;
	struct cam_hw_info    *hw_info;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}

	dev = cam_pm->dev;
	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}

	hw_info = devm_kzalloc(dev, sizeof(struct cam_hw_info), GFP_KERNEL);
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	cam_pm->hw_info = hw_info;

	ret = of_property_read_u32(of_node, "cell-index", &hw_info->cell_idx);
	if (ret) {
		XRISP_PR_ERROR("read cell-index failed");
		return ret;
	}

	if (hw_info->cell_idx > CAM_SENSOR_NUM_MAX) {
		XRISP_PR_ERROR("Invalid cell_idx:%d max:%d.",
			hw_info->cell_idx, CAM_SENSOR_NUM_MAX);
		return -EINVAL;
	}
	cam_pm->id = hw_info->cell_idx;

	ret = xrisp_sensor_util_get_dt_regulator_info(dev, hw_info);
	if (ret)
		return ret;

	ret = xrisp_sensor_util_get_dt_clk_info(dev, hw_info);
	if (ret)
		return ret;

	ret = xrisp_sensor_util_get_dt_gpio_info(dev, hw_info);
	if (ret)
		return ret;

	ret = xrisp_sensor_gpio_pin_init(cam_pm);
	if (ret)
		return ret;

	XRISP_PR_DEBUG("cam sensor pm dts parse ok!");
	return ret;
}

static int xrisp_sensor_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct cam_pm_dev *cam_pm;

	XRISP_PR_INFO("start!\n");
	cam_pm = devm_kzalloc(&pdev->dev, sizeof(struct cam_pm_dev), GFP_KERNEL);
	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}

	cam_pm->pdev = pdev;
	cam_pm->dev = &pdev->dev;

	ret = xrisp_sensor_dts_parse(cam_pm);
	if (ret) {
		XRISP_PR_ERROR("dts parse failed!");
		return -EINVAL;
	}

	ret = xrisp_sensor_util_get_clk_resource(&pdev->dev, cam_pm->hw_info);
	if (ret) {
		XRISP_PR_ERROR("get clk resource failed!");
		return -EINVAL;
	}
	ret = xrisp_sensor_util_get_regulator_resource(&pdev->dev, cam_pm->hw_info);
	if (ret) {
		XRISP_PR_ERROR("get regulator resource failed!");
		return -EINVAL;
	}
	ret = xrisp_sensor_util_get_pinctrl_resource(&pdev->dev, cam_pm->hw_info);
	if (ret) {
		XRISP_PR_ERROR("get pinctrl resource failed!");
		return -EINVAL;
	}

	xrisp_sensor_ois_init(cam_pm);
	xrisp_sensor_debugfs_init(cam_pm);
	mutex_init(&cam_pm->mlock);
	xrisp_parklens_init(cam_pm);
	platform_set_drvdata(pdev, cam_pm);
	g_cam_sensor_tab[cam_pm->id] = cam_pm;
	XRISP_PR_INFO("camid:%d xrisp sensor porbe success!\n", cam_pm->id);

	return ret;
}

static int xrisp_sensor_remove(struct platform_device *pdev)
{
	struct cam_pm_dev *cam_pm;
	int i;

	cam_pm = platform_get_drvdata(pdev);
	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	xrisp_parklens_exit(cam_pm);
	xrisp_power_sync_exit();
	xrisp_sensor_debugfs_exit(cam_pm);

	for (i = 0; i < CAM_SENSOR_NUM_MAX; i++) {
		if (g_cam_sensor_tab[i] == cam_pm) {
			g_cam_sensor_tab[i] = NULL;
			break;
		}
	}

	XRISP_PR_ERROR("camid:%d xrisp_sensor_pm_remove success!\n", cam_pm->id);
	return 0;
}

static const struct of_device_id xrisp_sensor_dt_match[] = {
	{.compatible = "xring,cam-sensor"},
	{ }
};

MODULE_DEVICE_TABLE(of, xrisp_sensor_dt_match);

static struct platform_driver xrisp_sensor_driver = {
	.probe = xrisp_sensor_probe,
	.remove = xrisp_sensor_remove,
	.driver = {
		.name = "xring,camera_sensor",
		.owner = THIS_MODULE,
		.of_match_table = xrisp_sensor_dt_match,
	},
};

int xrisp_sensor_init(void)
{
	int32_t ret = 0;

	ret = platform_driver_register(&xrisp_sensor_driver);
	if (ret < 0)
		XRISP_PR_ERROR("register platform failed, ret:%d", ret);
	return ret;
}

void xrisp_sensor_exit(void)
{
	platform_driver_unregister(&xrisp_sensor_driver);
}

MODULE_DESCRIPTION("xrisp sensor driver");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: xrisp_wl28681c");
MODULE_SOFTDEP("pre: xrisp_wl28661d");
