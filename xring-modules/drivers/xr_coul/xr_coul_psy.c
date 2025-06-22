// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * xr coul driver for Xring coulomb meter
 *
 * Copyright (c) 2024, XRing Technologies Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#include <linux/power_supply.h>
#include <soc/xring/xr_coul_interface.h>
#include <xr_coul.h>

#define coul_psy_debug(fmt, args...) \
	pr_debug("[coul_psy] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_psy_info(fmt, args...) \
	pr_info("[coul_psy] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_psy_warn(fmt, args...) \
	pr_warn("[coul_psy] [%s:%d]" fmt, __func__, __LINE__, ## args)
#define coul_psy_err(fmt, args...) \
	pr_err("[coul_psy] [%s:%d]" fmt, __func__, __LINE__, ## args)

#define PERMILLAGE              1000
#define BATT_MISSING_SOC        50
#define BATT_MISSING_TEMP       250

struct coul_psy_dev_info {
	struct device *dev;
	unsigned int batt_board_id;
	struct power_supply *batt_psy;
};

static enum power_supply_property xr_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_MODEL_NAME,
};

static int coul_battery_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *pval)
{
	struct coul_psy_dev_info *pi = power_supply_get_drvdata(psy);

	if (!pi) {
		coul_psy_err("NULL point of *pi\n");
		return -EINVAL;
	}

	coul_psy_debug("prop:%d\n", psp);
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		pval->intval = coul_intf_get_chg_status();
		if (pval->intval == CHARGING_STATE_FULL)
			pval->intval = POWER_SUPPLY_STATUS_FULL;
		else if (pval->intval == CHARGING_STATE_START)
			pval->intval = POWER_SUPPLY_STATUS_CHARGING;
		else
			pval->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		if (pi->batt_board_id == BAT_BOARD_FPGA
			|| pi->batt_board_id == BAT_BOARD_UDP)
			pval->intval = 1;
		else
			pval->intval = coul_intf_is_batt_exist();
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		pval->intval = POWER_SUPPLY_TECHNOLOGY_LIPO; /* Default technology is "Li-poly" */
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		pval->intval = coul_intf_read_chg_cycle();
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		pval->intval = coul_intf_read_batt_vol_uv();
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		pval->intval = coul_intf_read_batt_curr();
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		pval->intval = coul_intf_read_batt_fcc_design();
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		pval->intval = coul_intf_read_batt_fcc();
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (pi->batt_board_id == BAT_BOARD_FPGA
			|| pi->batt_board_id == BAT_BOARD_UDP)
			pval->intval = BATT_MISSING_SOC;
		else {
			pval->intval = coul_intf_read_batt_cap();
		}
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (pi->batt_board_id == BAT_BOARD_FPGA
			|| pi->batt_board_id == BAT_BOARD_UDP)
			pval->intval = BATT_MISSING_TEMP;
		else
			pval->intval = coul_get_tbatt(); /* 0.1 degrees */
		break;
	case POWER_SUPPLY_PROP_MODEL_NAME:
		pval->strval = "ATL";
		break;
	default:
		coul_psy_err("get prop %d is not supported in coul battery\n", psp);
		return -EINVAL;
	}

	return 0;
}

static const struct power_supply_desc xr_battery_desc = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = xr_battery_props,
	.num_properties = ARRAY_SIZE(xr_battery_props),
	.get_property = coul_battery_get_property,
};

static int coul_psy_register(struct platform_device *pdev,
					struct coul_psy_dev_info *pi)
{
	struct power_supply_config coul_battery_cfg = {};

	coul_battery_cfg.drv_data = pi;
	pi->batt_psy = devm_power_supply_register(&pdev->dev,
							&xr_battery_desc,
							&coul_battery_cfg);
	if (IS_ERR(pi->batt_psy)) {
		coul_psy_err("Couldn't register battery power supply\n");
		return PTR_ERR(pi->batt_psy);
	}

	return 0;
}

static int coul_psy_parse_dts(struct coul_psy_dev_info *pi)
{
	int ret = 0;
	struct device_node *np = NULL;

	np = pi->dev->of_node;
	if (!np) {
		coul_psy_err("NULL point of of_node\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "battery_board_type", &pi->batt_board_id);
	if (ret) {
		pi->batt_board_id = BAT_BOARD_FPGA;
		coul_psy_err("get board type fail\n");
	}

	return 0;
}

static int coul_psy_probe(struct platform_device *pdev)
{
	int ret;
	struct coul_psy_dev_info *pi = NULL;

	pi = devm_kzalloc(&pdev->dev, sizeof(*pi), GFP_KERNEL);
	if (!pi)
		return -ENOMEM;

	pi->dev = &pdev->dev;

	ret = coul_psy_parse_dts(pi);
	if (ret)
		goto fail0;

	platform_set_drvdata(pdev, pi);

	ret = coul_psy_register(pdev, pi);
	if (ret) {
		coul_psy_err("coul psy register fail\n");
		goto psy_register_fail;
	}

	coul_psy_debug("coul psy probe success!!!\n");
	return ret;

psy_register_fail:
	platform_set_drvdata(pdev, NULL);
fail0:
	return ret;
}

static int coul_psy_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id coul_psy_match_table[] = {
	{
		.compatible = "xring,coul_psy",
	},
	{
		/* end */
	},
};

static struct platform_driver coul_psy_driver = {
	.probe = coul_psy_probe,
	.remove = coul_psy_remove,
	.driver = {
		.name = "xr_coul_psy",
		.owner = THIS_MODULE,
		.of_match_table = coul_psy_match_table,
	},
};

module_platform_driver(coul_psy_driver);

MODULE_SOFTDEP("pre: xr_coul");
MODULE_AUTHOR("XRing Technologies Co., Ltd");
MODULE_DESCRIPTION("xring coul power supply driver");
MODULE_LICENSE("GPL");
