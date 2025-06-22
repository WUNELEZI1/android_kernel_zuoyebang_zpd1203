// SPDX-License-Identifier: GPL-2.0
/*
 *mca_subpmic_sysfs.c
 *
 * mca O1 platform sysfs driver
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
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/power_supply.h>
#include <mca/common/mca_log.h>
#include <mca/common/mca_sysfs.h>
#include <mca/strategy/strategy_fg_class.h>
#include <mca/strategy/strategy_class.h>
#include <mca/strategy/strategy_wireless_class.h>
#include <mca/protocol/protocol_class.h>
#include <mca/platform/platform_buckchg_class.h>
#include <mca/platform/platform_wireless_class.h>
#include <mca/protocol/protocol_pd_class.h>
#include <linux/power_supply.h>
#include <mca/common/mca_charge_interface.h>
#include <mca/platform/platform_sc6601a_cid_class.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_subpmic_sysfs"
#endif

#define QTI_POWER_SUPPLY_TYPE_USB_HVDCP         20
#define QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3       21
#define QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5     22
#define QTI_POWER_SUPPLY_TYPE_USB_FLOAT         23

#define POWER_SUPPLY_TYPE_USB_HVDCP         QTI_POWER_SUPPLY_TYPE_USB_HVDCP
#define POWER_SUPPLY_TYPE_USB_HVDCP_3       QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3
#define POWER_SUPPLY_TYPE_USB_HVDCP_3P5     QTI_POWER_SUPPLY_TYPE_USB_HVDCP_3P5
#define POWER_SUPPLY_TYPE_USB_FLOAT         QTI_POWER_SUPPLY_TYPE_USB_FLOAT

struct mca_subpmic_sysfs {
	struct device *dev;
	struct class mca_subpmic_class;
	bool support_multi_typec;
};

/* Standard usb_type definitions similar to power_supply_sysfs.c */
static const char * const power_supply_usb_type_text[] = {
	"Unknown", "SDP", "CDP", "DCP", "USB_FLOAT", "USB_HVDCP",
	"USB_HVDCP_3", "HVDCP_3_B", "USB_HVDCP_3P5", "C", "USB_PD", "PD_PPS", "PD_PPS",
};

/* Custom usb_type definitions */
static const char * const qc_power_supply_usb_type_text[] = {
	"HVDCP", "HVDCP_3", "HVDCP_3P5", "USB_FLOAT"
};

static const char * const power_supply_ex_pd_usb_type_text[] = {
	"Unknown", "PD", "PD_PPS",
};

static const char * const power_supply_wls_type_text[] = {
	"Unknown", "BPP", "EPP", "HPP"
};

static const char *get_usb_type_name(u32 usb_type)
{
    u32 i;

    pr_err("%s: usb_type=%d\n", __func__, usb_type);

    for (i = 0; i < ARRAY_SIZE(power_supply_usb_type_text); i++) {
        if (i == usb_type)
            return power_supply_usb_type_text[i];
    }
    return "Unknown";
}

static ssize_t real_type_show(const struct class *c,
        const struct class_attribute *attr, char *buf)
{
    int rc;
    int real_type = 0;
    int bc12_chg_type = 0;
    int pd_active = 0;

    (void)protocol_class_pd_get_pd_active(TYPEC_PORT_0, &pd_active);
    rc = protocol_class_get_adapter_type(ADAPTER_PROTOCOL_BC12, &bc12_chg_type);
    if (rc < 0)
        return rc;

    if (pd_active) {
        if (pd_active == QTI_POWER_SUPPLY_PD_ACTIVE)
            real_type = XM_CHARGER_TYPE_PD;
        else if (pd_active == QTI_POWER_SUPPLY_PD_PPS_ACTIVE)
            real_type = XM_CHARGER_TYPE_PD_VERIFY;
    } else {
        real_type = bc12_chg_type;
    }
    mca_log_info("pd_active = %d, real type = %s\n", pd_active, get_usb_type_name(real_type));
    return snprintf(buf, PAGE_SIZE, "%s\n", get_usb_type_name(real_type));
}
static CLASS_ATTR_RO(real_type);

static ssize_t adapter_online_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int rc;
    int online = 0;

    rc = platform_class_buckchg_ops_get_online(MAIN_BUCK_CHARGER, &online);
	if (rc < 0)
		return rc;

	mca_log_info("online = %d\n", online);

	return snprintf(buf, PAGE_SIZE, "%d\n", online);
}
static CLASS_ATTR_RO(adapter_online);

static ssize_t voltage_now_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int rc;
	int voltage_now = 4000;

	rc = platform_class_buckchg_ops_get_batt_volt(MAIN_BUCK_CHARGER, &voltage_now);
	if (rc < 0)
		return rc;

	mca_log_info("voltage_now = %d\n", voltage_now);
	return snprintf(buf, PAGE_SIZE, "%d\n", voltage_now);
}
static CLASS_ATTR_RO(voltage_now);

static ssize_t authentic_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int val = 1;

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t authentic_store(const struct class *c,
		const struct class_attribute *attr,
		const char *buf, size_t count)
{
	return count;
}
static CLASS_ATTR_RW(authentic);

static ssize_t slave_authentic_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int val = 1;

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t slave_authentic_store(const struct class *c,
		const struct class_attribute *attr,
		const char *buf, size_t count)
{
	return count;
}
static CLASS_ATTR_RW(slave_authentic);

static ssize_t pd_verifed_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int rc;
	int val;
	rc = protocol_class_get_adapter_verified(ADAPTER_PROTOCOL_PD, &val);
	if (rc < 0)
		return rc;

	mca_log_info("show pd_verifed = %d\n", val);

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t pd_verifed_store(const struct class *c,
		const struct class_attribute *attr,
		const char *buf, size_t count)
{
	int rc;
	int val;
	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	mca_log_info("store pd_verifed = %d\n", val);
	rc = protocol_class_set_adapter_verified(ADAPTER_PROTOCOL_PD, val);
	if (rc < 0)
		return rc;

	return count;
}
static CLASS_ATTR_RW(pd_verifed);

static ssize_t quick_charge_type_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int value = 0;
	int online = 0;

	(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
		STRATEGY_STATUS_TYPE_QC_TYPE, &value);

	if (!value)
		(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BUCK_CHARGE,
			STRATEGY_STATUS_TYPE_QC_TYPE, &value);

	mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BASIC_WIRELESS,
		STRATEGY_STATUS_TYPE_ONLINE, &online);
	if (online)
		(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BASIC_WIRELESS,
			STRATEGY_STATUS_TYPE_QC_TYPE, &value);

	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}
static CLASS_ATTR_RO(quick_charge_type);

static ssize_t power_max_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int value = 0;
	int online = 0;

	(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_QUICK_CHARGE,
		STRATEGY_STATUS_TYPE_POWER_MAX, &value);

	mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BASIC_WIRELESS,
		STRATEGY_STATUS_TYPE_ONLINE, &online);
	if (online)
		(void)mca_strategy_func_get_status(STRATEGY_FUNC_TYPE_BASIC_WIRELESS,
			STRATEGY_STATUS_TYPE_POWER_MAX, &value);

	return snprintf(buf, PAGE_SIZE, "%d\n", value);
}
static CLASS_ATTR_RO(power_max);

static ssize_t soc_decimal_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int soc_decimal, rate;

	(void)strategy_class_fg_ops_get_soc_decimal(&soc_decimal, &rate);

	return snprintf(buf, PAGE_SIZE, "%d\n", soc_decimal);
}
static CLASS_ATTR_RO(soc_decimal);

static ssize_t soc_decimal_rate_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	int soc_decimal, rate;

	(void)strategy_class_fg_ops_get_soc_decimal(&soc_decimal, &rate);

	return snprintf(buf, PAGE_SIZE, "%d\n", rate);
}
static CLASS_ATTR_RO(soc_decimal_rate);

static ssize_t otg_ui_support_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	bool enable;

	platform_class_buckchg_ops_is_support_cid(MAIN_BUCK_CHARGER, &enable);
	return snprintf(buf, PAGE_SIZE, "%d\n", enable);

}
static CLASS_ATTR_RO(otg_ui_support);

static ssize_t cid_status_show(const struct class *c,
					const struct class_attribute *attr, char *buf)
{
	bool online = false;
	int rc;

	rc = platform_class_buckchg_ops_get_cid_status(MAIN_BUCK_CHARGER, &online);
	if (rc < 0)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n", online);
}
static CLASS_ATTR_RO(cid_status);

static ssize_t cc_toggle_store(const struct class *c,
		const struct class_attribute *attr, const char *buf, size_t count)
{
	int rc;
	int val;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;
	rc = protocol_class_pd_set_cc_toggle(TYPEC_PORT_0, val);
	if (rc < 0)
		return rc;

	return count;

}

static ssize_t cc_toggle_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	bool cc_toggle = false;
	int rc;

	rc = protocol_class_pd_get_cc_toggle(TYPEC_PORT_0, &cc_toggle);
	if (rc < 0)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n", cc_toggle);

}
static CLASS_ATTR_RW(cc_toggle);

static ssize_t has_dp_show(const struct class *c,
		const struct class_attribute *attr, char *buf)
{
	bool has_dp = false;
	int rc;

	rc = protocol_class_pd_get_has_dp(TYPEC_PORT_0, &has_dp);
	if (rc < 0)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n", has_dp);

}
static CLASS_ATTR_RO(has_dp);

static struct attribute *mca_subpmic_sysfs_attrs[] = {
	&class_attr_real_type.attr,
	&class_attr_authentic.attr,
	&class_attr_slave_authentic.attr,
	&class_attr_pd_verifed.attr,
	&class_attr_quick_charge_type.attr,
	&class_attr_power_max.attr,
	&class_attr_soc_decimal.attr,
	&class_attr_soc_decimal_rate.attr,
	&class_attr_otg_ui_support.attr,
	&class_attr_cid_status.attr,
	&class_attr_cc_toggle.attr,
	&class_attr_has_dp.attr,
	&class_attr_adapter_online.attr,
	&class_attr_voltage_now.attr,
	NULL,
};
ATTRIBUTE_GROUPS(mca_subpmic_sysfs);

static int subpmic_sysfs_parse_dt(struct mca_subpmic_sysfs *subpmic_dev)
{
	struct device_node *node = subpmic_dev->dev->of_node;

	subpmic_dev->support_multi_typec = of_property_read_bool(node, "mi,support_multi_typec");
	return 0;
}

static int mca_subpmic_sysfs_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct mca_subpmic_sysfs *subpmic_sysfs;

	mca_log_info("probe begin\n");
	subpmic_sysfs = devm_kzalloc(&pdev->dev, sizeof(*subpmic_sysfs), GFP_KERNEL);
	if (!subpmic_sysfs)
		return -ENOMEM;

	subpmic_sysfs->dev = &pdev->dev;
	rc = subpmic_sysfs_parse_dt(subpmic_sysfs);
	if (rc) {
		mca_log_err("%s parse dt fail\n", __func__);
		return rc;
	}
	platform_set_drvdata(pdev, subpmic_sysfs);
	subpmic_sysfs->mca_subpmic_class.name = "subpmic-battery";
	//subpmic_sysfs->mca_subpmic_class.owner = THIS_MODULE;
	subpmic_sysfs->mca_subpmic_class.class_groups = mca_subpmic_sysfs_groups;
	rc = class_register(&subpmic_sysfs->mca_subpmic_class);
	if (rc < 0) {
		mca_log_err("class reg failed %d\n", rc);
		return rc;
	}

	mca_log_info("probe ok\n");
	return 0;
}

static int mca_subpmic_sysfs_remove(struct platform_device *pdev)
{
	struct mca_subpmic_sysfs *subpmic_sysfs = platform_get_drvdata(pdev);

	class_unregister(&subpmic_sysfs->mca_subpmic_class);

	return 0;
}

static const struct of_device_id match_table[] = {
	{.compatible = "mca,subpmic_sysfs"},
	{},
};

static struct platform_driver mca_subpmic_sysfs_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "mca_subpmic_sysfs",
		.of_match_table = match_table,
	},
	.probe = mca_subpmic_sysfs_probe,
	.remove = mca_subpmic_sysfs_remove,
};

static int __init mca_subpmic_sysfs_init(void)
{
	return platform_driver_register(&mca_subpmic_sysfs_driver);
}
module_init(mca_subpmic_sysfs_init);

static void __exit mca_subpmic_sysfs_exit(void)
{
	platform_driver_unregister(&mca_subpmic_sysfs_driver);
}
module_exit(mca_subpmic_sysfs_exit);

MODULE_DESCRIPTION("mca subpmic sysfs");
MODULE_AUTHOR("tianye9@xiaomi.com");
MODULE_LICENSE("GPL v2");
