// SPDX-License-Identifier: GPL-2.0
/*
 * mca_bmd.c
 *
 * battery missing detection driver
 *
 * Copyright (c) 2023-2023 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/err.h>
#include <linux/iio/consumer.h>
#include <linux/platform_device.h>
#include <linux/thermal.h>
#include <mca/common/mca_event.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_parse_dts.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "inc/mca_bmd.h"
#include <mca/common/mca_sysfs.h>
#include <mca/strategy/strategy_fg_class.h>
#include <mca/platform/platform_fg_ic_ops.h>
#include "../mca_hardware_ic/subpmic/xm_subpmic/subpmic_sc6601/inc/subpmic.h"
#include <mca/common/mca_charge_mievent.h>
#include <mca/strategy/strategy_class.h>
#include <mca/platform/platform_buckchg_class.h>
#include <linux/pinctrl/consumer.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_bmd"
#endif

#define MAX_STRING_LEN 20

#define BTB_OPEN_MAX_VOL 1900
#define BTB_OPEN_MIN_VOL 1700

#define FAST_HEARTBEAT_TIMER_MS 2000
#define SLOW_HEARTBEAT_TIMER_MS 600000

struct bmd_scheme_data {
	int scheme;
	struct iio_channel *channel;
	int gpio;
};

struct mca_bmd_dev {
	struct device *dev;
	struct bmd_scheme_data bmd_scheme[MAX_BTB];
	struct delayed_work monitor_bmd_work;
	bool btb_online[MAX_BTB];
	bool batt_missing;
	bool fake_batt;
};

enum bmd_scheme {
	ADC_SCHEME,
	GPIO_SCHEME,
	INT_SCHEME,
	IIC_SCHEME,
	MAX_SCHEME,
};

static bool mca_bmd_get_btb_status(struct mca_bmd_dev *info, int index);

static int mca_bmd_set_gpio_status(struct device *dev, char *status)
{
	struct pinctrl *pinctrl;
	struct pinctrl_state *s;
	int ret;

	pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrl)) {
		mca_log_err("Failed to get pinctrl\n");
		return -ENODEV;
	}

	s = pinctrl_lookup_state(pinctrl, status);
	if (IS_ERR(s)) {
		devm_pinctrl_put(pinctrl);
		mca_log_err("Failed to lookup pinctrl state: %s\n", status);
		return -ENOENT;
	}

	ret = pinctrl_select_state(pinctrl, s);
	if (ret < 0) {
		devm_pinctrl_put(pinctrl);
		mca_log_err("Failed to select pinctrl state: %s\n", status);
		return ret;
	}
	return ret;
}

static void mca_bmd_monitor_workfunc(struct work_struct *work)
{
	struct mca_bmd_dev *info = container_of(work, struct mca_bmd_dev, monitor_bmd_work.work);
	bool batt_missing = false;
	bool fake_batt = false;

	for (int i = 0; i < MAX_BTB; i++)
		info->btb_online[i] = mca_bmd_get_btb_status(info, i);

	mca_log_info("btb_online[0]: %d, btb_online[1]: %d\n",
		info->btb_online[MASTER_BTB], info->btb_online[SLAVE_BTB]);

	if (!info->btb_online[MASTER_BTB] || !info->btb_online[SLAVE_BTB])
		batt_missing = true;
	else
		batt_missing = false;

	if (info->batt_missing != batt_missing) {
		info->batt_missing = batt_missing;
		// NOTICE: enable in P01 stage to avoid cannot charge
		// info->batt_missing = 0;
		mca_event_block_notify(MCA_EVENT_TYPE_HW_INFO, MCA_EVENT_BATT_BTB_CHANGE, &info->batt_missing);
		if (batt_missing)
			mca_charge_mievent_report(CHARGE_DFX_BATTERY_MISSING, NULL, 0);
		else
			mca_charge_mievent_set_state(MIEVENT_STATE_END, CHARGE_DFX_BATTERY_MISSING);
	}

	/*detect fake battery*/
	if (!info->btb_online[MASTER_BTB] && !info->btb_online[SLAVE_BTB])
		fake_batt = true;
	else
		fake_batt = false;

	if (info->fake_batt != fake_batt) {
		info->fake_batt = fake_batt;
		mca_log_err("fake_batt = %d\n", fake_batt);
		mca_event_block_notify(MCA_EVENT_TYPE_BATTERY_INFO, MCA_EVENT_BATTERY_FAKE_POWER, &info->fake_batt);
	}

	if (batt_missing) {
		mca_log_err("BTB disconnect\n");
		subpmic_dev_notify(SC6601_SUBPMIC_EVENT_BTB_CHANGE, false);
	}

	schedule_delayed_work(&info->monitor_bmd_work, msecs_to_jiffies(FAST_HEARTBEAT_TIMER_MS));
}

static bool mca_bmd_get_btb_status(struct mca_bmd_dev *info, int index)
{
	int ret;
	int data;
	bool bmd_online = false;

	if (!info) {
		mca_log_err(" %snull pointer\n", __func__);
		return -1;
	}
	mca_log_info("bmd_scheme = %d, index = %d\n", info->bmd_scheme[index].scheme, index);

	switch (info->bmd_scheme[index].scheme) {
	case ADC_SCHEME:
		ret = iio_read_channel_processed((struct iio_channel *)info->bmd_scheme[index].channel, &data);
		if (ret < 0) {
			mca_log_err("Error in reading btb_adc_voltage channel\n");
			return 0;
		}
		if (data > BTB_OPEN_MIN_VOL && data < BTB_OPEN_MAX_VOL)
			bmd_online = false;
		else
			bmd_online = true;
		break;
	case GPIO_SCHEME:
		data = gpio_get_value(info->bmd_scheme[index].gpio);
		bmd_online = !data;
		break;
	case INT_SCHEME:
		break;
	case IIC_SCHEME:
		data = strategy_class_fg_is_chip_ok();
		if (data <= 0)
			bmd_online = false;
		else
			bmd_online = true;
		break;
	default:
		break;
	}
	return bmd_online;
}

static int mca_bmd_process_event(int event, int value, void *data)
{
	struct mca_bmd_dev *info = data;

	if (!info)
		return -1;

	switch (event) {
	case MCA_EVENT_USB_CONNECT:
	case MCA_EVENT_WIRELESS_CONNECT:
		cancel_delayed_work_sync(&info->monitor_bmd_work);
		schedule_delayed_work(&info->monitor_bmd_work, 0);
		mca_bmd_set_gpio_status(info->dev, "bmd_work");
		break;
	case MCA_EVENT_USB_DISCONNECT:
	case MCA_EVENT_WIRELESS_DISCONNECT:
		mca_bmd_set_gpio_status(info->dev, "bmd_idle");
		cancel_delayed_work_sync(&info->monitor_bmd_work);
		break;
	default:
		break;
	}

	return 0;
}

static int mca_bmd_parse_dt(struct mca_bmd_dev *info)
{
	struct device_node *node = info->dev->of_node;
	int ret = 0;
	int val[MAX_BTB] = {0};

	if (!node) {
		mca_log_err("device tree info missing\n");
		return -1;
	}

	ret |= mca_parse_dts_u32_array(node, "btb_bmd_scheme", val, MAX_BTB);
	for (int i = 0; i < MAX_BTB; i++) {
		info->bmd_scheme[i].scheme = val[i];
		switch (info->bmd_scheme[i].scheme) {
		case ADC_SCHEME:
			info->bmd_scheme[i].channel = devm_iio_channel_get(info->dev, "btb_adc");
			break;
		case GPIO_SCHEME:
			info->bmd_scheme[i].gpio = of_get_named_gpio(node, "btb_gpio", 0);
			if (!gpio_is_valid(info->bmd_scheme[i].gpio)) {
				mca_log_err("failed to parse irq_gpio\n");
				return -1;
			}
			ret = gpio_request(info->bmd_scheme[i].gpio, "btb_gpio");
			if (ret)
				mca_log_err("unable to request btb_gpio gpio [%d]\n", info->bmd_scheme[i].gpio);
			else {
				ret = gpio_direction_input(info->bmd_scheme[i].gpio);
				if (ret)
					mca_log_err("unable to set direction btb_gpio [%d]\n", info->bmd_scheme[i].gpio);
			}
			break;
		case INT_SCHEME:
			break;
		case IIC_SCHEME:
			break;
		default:
			break;
		}
	}
	return ret;
}

enum btb_attr_list {
	BTB_PROP_MASTER,
	BTB_PROP_SLAVE,
	BTB_PROP_MISSING,
};

static ssize_t btb_sysfs_show(struct device *dev,
		struct device_attribute *attr, char *buf);

struct mca_sysfs_attr_info btb_sysfs_field_tbl[] = {
	mca_sysfs_attr_ro(btb_sysfs, 0440, BTB_PROP_MASTER, btb_master_status),
	mca_sysfs_attr_ro(btb_sysfs, 0440, BTB_PROP_SLAVE, btb_slave_status),
	mca_sysfs_attr_ro(btb_sysfs, 0440, BTB_PROP_MISSING, btb_missing_status),
};

#define BTB_SYSFS_ATTRS_SIZE   ARRAY_SIZE(btb_sysfs_field_tbl)

static struct attribute *btb_sysfs_attrs[BTB_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group btb_sysfs_attr_group = {
	.attrs = btb_sysfs_attrs,
};

static ssize_t btb_sysfs_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mca_sysfs_attr_info *attr_info;
	ssize_t count = 0;
	struct mca_bmd_dev *info = dev_get_drvdata(dev);
	bool btb_online = 0;

	attr_info = mca_sysfs_lookup_attr(attr->attr.name,
		btb_sysfs_field_tbl, BTB_SYSFS_ATTRS_SIZE);
	if (!attr_info)
		return -1;

	switch (attr_info->sysfs_attr_name) {
	case BTB_PROP_MASTER:
		btb_online = mca_bmd_get_btb_status(info, MASTER_BTB);
		count = scnprintf(buf, PAGE_SIZE, "%d\n", btb_online);
		break;
	case BTB_PROP_SLAVE:
		btb_online = mca_bmd_get_btb_status(info, SLAVE_BTB);
		count = scnprintf(buf, PAGE_SIZE, "%d\n", btb_online);
		break;
	case BTB_PROP_MISSING:
		count = scnprintf(buf, PAGE_SIZE, "%d\n", info->batt_missing);
		break;
	default:
		break;
	}

	return count;
}

static int btb_sysfs_create_group(struct device *dev)
{
	mca_sysfs_init_attrs(btb_sysfs_attrs, btb_sysfs_field_tbl,
		BTB_SYSFS_ATTRS_SIZE);
	return mca_sysfs_create_link_group(SYSFS_DEV_5, "bmd",
	 dev, &btb_sysfs_attr_group);
}

static int mca_bmd_probe(struct platform_device *pdev)
{
	struct mca_bmd_dev *info;
	int ret;
	int online = 0;

	mca_log_info("bmd probe begin\n");

	if (strategy_class_fg_ops_is_init_ok() <= 0) {
		mca_log_info("fg is not ready, wait for it\n");
		return -EPROBE_DEFER;
	}

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->dev = &pdev->dev;
	platform_set_drvdata(pdev, info);
	ret = mca_bmd_parse_dt(info);
	if (ret) {
		mca_log_err("parse dt faile\n");
		return -1;
	}

	platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER, &online);
	(void)mca_strategy_ops_register(STRATEGY_FUNC_TYPE_BMD, mca_bmd_process_event, NULL, NULL, info);

	info->batt_missing = 0;
	INIT_DELAYED_WORK(&info->monitor_bmd_work, mca_bmd_monitor_workfunc);
	if (online) {
		cancel_delayed_work_sync(&info->monitor_bmd_work);
		schedule_delayed_work(&info->monitor_bmd_work, msecs_to_jiffies(FAST_HEARTBEAT_TIMER_MS));
	}
	btb_sysfs_create_group(info->dev);
	mca_log_err("%s success\n", __func__);
	return 0;
}

static int mca_bmd_remove(struct platform_device *pdev)
{
	return 0;
}

static void mca_bmd_shutdown(struct platform_device *pdev)
{

}

static const struct of_device_id match_table[] = {
	{.compatible = "mca,bmd"},
	{},
};

static struct platform_driver mca_bmd_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "mca_bmd",
		.of_match_table = match_table,
	},
	.probe = mca_bmd_probe,
	.remove = mca_bmd_remove,
	.shutdown = mca_bmd_shutdown,
};

static int __init mca_bmd_init(void)
{
	return platform_driver_register(&mca_bmd_driver);
}
module_init(mca_bmd_init);

static void __exit mca_bmd_exit(void)
{
	platform_driver_unregister(&mca_bmd_driver);
}
module_exit(mca_bmd_exit);

MODULE_DESCRIPTION("battery missing detection");
MODULE_AUTHOR("lvxiaofeng@xiaomi.com");
MODULE_LICENSE("GPL v2");
