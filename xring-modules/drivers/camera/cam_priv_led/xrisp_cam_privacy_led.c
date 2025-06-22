// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "privacy_led", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "privacy_led", __func__, __LINE__

#include <linux/device.h>
#include <linux/string.h>
#include <linux/of_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/kdev_t.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include "xrisp_cam_ctrl.h"
#include "cam_cdm/xrisp_cdm.h"
#include "xrisp_cam_privacy_led.h"
#include "xrisp_log.h"
#include "cam_sysfs/xrisp_sysfs.h"

static struct privacy_led_dev *g_privacy_led;

/*
 * config pinctrl
 */
static int xrisp_priv_led_pinctrl_state_set(struct pinctrl *pinctrl, char *pinctrl_name)
{
	struct pinctrl_state *state;
	int ret;

	if (!pinctrl_name) {
		XRISP_PR_ERROR("pinctrl_name is NULL\n");
		return -EINVAL;
	}

	if (IS_ERR(pinctrl)) {
		XRISP_PR_ERROR("%s: privacy led pinctrl is invalid", __func__);
		return -EINVAL;
	}

	state = pinctrl_lookup_state(pinctrl, pinctrl_name);
	if (IS_ERR(state)) {
		devm_pinctrl_put(pinctrl);
		return -EINVAL;
	}

	ret = pinctrl_select_state(pinctrl, state);
	if (ret < 0) {
		devm_pinctrl_put(pinctrl);
		return -EINVAL;
	}

	return 0;
}

int xrisp_privacy_reminder_led_enable(void)
{
	int ret = 0;

	if (!g_privacy_led)
		return -EINVAL;

	if (IS_ERR(g_privacy_led->pinctrl)) {
		XRISP_PR_ERROR("%s: privacy led pinctrl is invalid", __func__);
		return -EINVAL;
	}

	XRISP_PR_INFO("Enable reminder privacy led");

	mutex_lock(&g_privacy_led->mlock);
	if (atomic_read(&g_privacy_led->power_cnt)) {
		atomic_inc(&g_privacy_led->power_cnt);
		XRISP_PR_INFO("privacy led already powerup count:%d.", atomic_read(&g_privacy_led->power_cnt));
		mutex_unlock(&g_privacy_led->mlock);
		return 0;
	}

	ret = xrisp_priv_led_pinctrl_state_set(g_privacy_led->pinctrl, "default");
	if (ret)
		XRISP_PR_ERROR("pinctrl set default state failed\n");

	atomic_inc(&g_privacy_led->power_cnt);
	mutex_unlock(&g_privacy_led->mlock);

	XRISP_PR_INFO("privacy reminder led enable success.\n");
	return 0;
}

int xrisp_privacy_reminder_led_disable(void)
{
	int ret = 0;

	if (!g_privacy_led)
		return -EINVAL;

	if (IS_ERR(g_privacy_led->pinctrl)) {
		XRISP_PR_ERROR("%s: privacy led pinctrl is invalid", __func__);
		return -EINVAL;
	}


	XRISP_PR_INFO("Disable reminder privacy led");

	mutex_lock(&g_privacy_led->mlock);
	if (atomic_dec_return(&g_privacy_led->power_cnt)) {
		if (atomic_read(&g_privacy_led->power_cnt) < 0)
			atomic_set(&g_privacy_led->power_cnt, 0);

		XRISP_PR_INFO("privacy led power count:%d.", atomic_read(&g_privacy_led->power_cnt));
		mutex_unlock(&g_privacy_led->mlock);
		return 0;
	}

	ret = xrisp_priv_led_pinctrl_state_set(g_privacy_led->pinctrl, "sleep");
	if (ret)
		XRISP_PR_ERROR("pinctrl set sleep state failed\n");

	mutex_unlock(&g_privacy_led->mlock);

	XRISP_PR_INFO("privacy led power down.\n");
	return ret;
}

static ssize_t store_priv_led_en_enable(struct kobject *kobj,
							struct kobj_attribute *attr, const char *buf,
							size_t len)
{
	int led_en;
	ssize_t ret;

	if (!g_privacy_led) {
		XRISP_PR_ERROR("%s: g_privacy_led is null", __func__);
		return -EINVAL;
	}

	if (IS_ERR(g_privacy_led->pinctrl)) {
		XRISP_PR_ERROR("%s: privacy led pinctrl is invalid", __func__);
		return -EINVAL;
	}

	ret = kstrtoint(buf, 10, &led_en);

	if (led_en > 0) {
		ret = xrisp_priv_led_pinctrl_state_set(g_privacy_led->pinctrl, "default");
		if (ret < 0)
			XRISP_PR_ERROR("pinctrl set default state failed\n");

	} else {
		ret = xrisp_priv_led_pinctrl_state_set(g_privacy_led->pinctrl, "sleep");
		if (ret < 0)
			XRISP_PR_ERROR("pinctrl set sleep state failed\n");
	}
	return len;
}

static struct kobj_attribute dev_attr_priv_led_en =
	__ATTR(priv_led_en, 0664, NULL, store_priv_led_en_enable);

static struct attribute *priv_led_attrs[] = {
	&dev_attr_priv_led_en.attr,
	NULL
};

static const struct attribute_group priv_led_attr_group = {
	.name = NULL,
	.attrs = priv_led_attrs,
};

static int privacy_led_sys_node_init(struct privacy_led_dev *led_dev)
{
	int ret = 0;

	XRISP_PR_INFO("Enter init");

	if (!led_dev) {
		XRISP_PR_ERROR("led_dev is null");
		return -EINVAL;
	}

	if (IS_ERR(led_dev->pinctrl)) {
		XRISP_PR_ERROR("pinctrl is invalid");
		return -EINVAL;
	}

	ret = xrisp_priv_led_pinctrl_state_set(led_dev->pinctrl, "sleep");
	if (ret) {
		XRISP_PR_ERROR("pinctrl set sleep state failed");
		goto destory_dev;
	}

	ret = xrisp_wait_sysfs_init_done();
	if (ret)
		goto destory_dev;

	g_privacy_led->node.isp_kobj = xrisp_get_isp_kobj();
	if (!g_privacy_led->node.isp_kobj) {
		XRISP_PR_ERROR("%s: get isp kobj failed", __func__);
		goto destory_dev;
	}

	g_privacy_led->node.priv_led_kobj = kobject_create_and_add("xrisp_priv_led", g_privacy_led->node.isp_kobj);
	if (!g_privacy_led->node.isp_kobj)
		goto destory_isp_kobj;

	ret = sysfs_create_group(g_privacy_led->node.priv_led_kobj, &priv_led_attr_group);
	if (ret)
		goto destory_kobj;

	XRISP_PR_INFO("%s finish\n", __func__);
	return ret;

destory_kobj:
	kobject_put(g_privacy_led->node.priv_led_kobj);

destory_isp_kobj:
	xrisp_put_isp_kobj();
	g_privacy_led->node.isp_kobj = NULL;

destory_dev:
	return -EINVAL;
}

void privacy_led_sys_node_exit(void)
{

	if (!g_privacy_led)
		return;

	sysfs_remove_group(g_privacy_led->node.priv_led_kobj, &priv_led_attr_group);
	kobject_put(g_privacy_led->node.priv_led_kobj);
	xrisp_put_isp_kobj();
}

static int xrisp_privacy_led_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct privacy_led_dev *priv;

	XRISP_PR_INFO("cam privacy led probe start");

	priv = devm_kzalloc(&pdev->dev, sizeof(struct privacy_led_dev), GFP_KERNEL);
	if (!priv) {
		XRISP_PR_ERROR("privacy led mem alloc failed");
		return -ENOMEM;
	}

	priv->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(priv->pinctrl)) {
		XRISP_PR_INFO("privacy led pinctrl not set");
		return -EINVAL;
	}

	mutex_init(&priv->mlock);

	platform_set_drvdata(pdev, priv);
	g_privacy_led = priv;

	ret = privacy_led_sys_node_init(priv);
	if (ret)
		XRISP_PR_ERROR("privacy_led_sysfs init failed");

	XRISP_PR_INFO("cam privacy led probe success");

	return ret;
}

static int xrisp_privacy_led_remove(struct platform_device *pdev)
{
	privacy_led_sys_node_exit();
	return 0;
}

static const struct of_device_id xrisp_privacy_led_of_match[] = {
	{
		.compatible = "xrisp,privacy-led-device",
	},
	{},
};

static struct platform_driver xrisp_privacy_led_driver = {
	.probe	= xrisp_privacy_led_probe,
	.remove	= xrisp_privacy_led_remove,
	.driver	= {
		.name			= "xrisp,privacy-led-device",
		.of_match_table	= xrisp_privacy_led_of_match,
	},
};

int xrisp_privacy_led_init(void)
{
	return platform_driver_register(&xrisp_privacy_led_driver);
}

void xrisp_privacy_led_exit(void)
{
	platform_driver_unregister(&xrisp_privacy_led_driver);
}

MODULE_AUTHOR("yangshuyong <yangshuyong@xiaomi.com>");
MODULE_DESCRIPTION("xrisp cam privacy led");
MODULE_LICENSE("GPL v2");
