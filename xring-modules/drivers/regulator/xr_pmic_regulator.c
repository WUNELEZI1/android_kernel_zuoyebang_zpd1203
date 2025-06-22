// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */

#include <linux/arm-smccc.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/of_address.h>
#include <linux/pm.h>
#include "soc/xring/xr_timestamp.h"
#include "xr_regulator_internal.h"
#include "regulator/internal.h"

#define XR_SPMI_READ(hdl, addr, pdata)  regmap_read(hdl, addr, (unsigned int *)pdata)
#define XR_SPMI_WRITE(hdl, addr, data)  regmap_write(hdl, addr, data)
#define XR_SPMI_UDELAY(delay_us)        udelay(delay_us)

#include <dt-bindings/xring/platform-specific/pmic/pmic_regulator_op.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#define UV_PER_MV 1000

static int of_regulator_properties(struct xring_regulator_pmic *xpr,
		struct platform_device *pdev)
{
	int ret;
	int id = 0;
	struct device *dev = NULL;
	struct device_node *np = NULL;

	if (!xpr || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_property_read_u32(np, "regulator-id", &id);
	if (ret) {
		dev_err(dev, "%s: get regulator-id failed\n", xpr->rdesc.name);
		return ret;
	}
	xpr->rdesc.id = id;

	ret = of_property_read_u32(np, "step-uV", &xpr->step_uV);
	if (ret) {
		dev_err(dev, "%s: get step-uV failed\n", xpr->rdesc.name);
		return ret;
	}

	if (xpr->step_uV == 0) {
		dev_err(dev, "%s: step-uV invalid\n", xpr->rdesc.name);
		return ret;
	}

	// this param is optional, do not return error if not exist
	// for converity check, add the ret assign
	ret = of_property_read_u32(np, "xring,sub_regulator_on_delay",
			&xpr->sub_regulator_on_delay);

	return 0;
}

static int dt_parse(struct xring_regulator_pmic *xpr,
		struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!xpr || !pdev) {
		pr_err("[%s]regulator get  dt para is err!\n", __func__);
		return -EINVAL;
	}
	dev = &pdev->dev;
	np = dev->of_node;

	ret = of_regulator_properties(xpr, pdev);

	return ret;
}


/*
 * helper function to ensure when it returns it is at least 'delay_us'
 * after 'since'.
 */
static void ensured_time_after(u64 since_us, u32 delay_us)
{
	u64 now_us;
	u64 elapsed_us;
	u32 actual_us32 = 0;

	now_us = xr_timestamp_gettime();
	do_div(now_us, NSEC_PER_USEC);
	elapsed_us = now_us - since_us;
	if (delay_us > elapsed_us) {
		actual_us32 = (u32)(delay_us - elapsed_us);
		if (actual_us32 >= 1000) {
			msleep(actual_us32 / 1000);
			udelay(actual_us32 % 1000);
		} else if (actual_us32 > 0) {
			udelay(actual_us32);
		}
	}
	pr_debug("PMIC: since:%llu, now:%llu, delay %d us",
			since_us, now_us, actual_us32);
}

static int pmic_sub_reg_enabling(struct xr_regulator_manager *mng,
			struct regulator_dev *sub_rdev)
{
	struct xring_regulator_pmic *xpr;

	xpr = container_of(mng, struct xring_regulator_pmic, regulator_mng);
	if (xpr->sub_regulator_on_delay) {
		mutex_lock(&mng->lock);

		ensured_time_after(mng->last_enable_time, xpr->sub_regulator_on_delay);

		mng->last_enable_time = xr_timestamp_gettime();
		do_div(mng->last_enable_time, NSEC_PER_USEC);
		mutex_unlock(&mng->lock);
	}

	return 0;
}

static int pmic_regulator_is_enabled(struct regulator_dev *rdev)
{
	unsigned long data;
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	if (xr_regulator_is_sec(rdev->desc->id)) {
		ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_PMIC_REGULATOR_IS_EN,
				rdev->desc->id, 0, &data);
		if (ret)
			return ret;
		ret = data;
	} else {
		ret = xr_regulator_is_enabled(rdev->regmap, rdev->desc->id);
		if (ret < 0) {
			dev_err(&rdev->dev, "pmic op fail (ret=%d)\n", ret);
			return ret;
		}
	}

	if (ret)
		dev_dbg(&rdev->dev, "pmic is power on\n");
	else
		dev_dbg(&rdev->dev, "pmic is power off\n");

	return ret;
}

static int pmic_regulator_enable(struct regulator_dev *rdev)
{
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	/*
	 * regulator hook need this event, regulator core do not support this event,
	 * so send this event in driver
	 */
	blocking_notifier_call_chain(&rdev->notifier,
			REGULATOR_EVENT_PRE_ENABLE, NULL);

	/* PMIC constraint: enable delay for regulators under one power plane */
	if (rdev->supply) {
		struct xr_regulator_manager *supply_mng;

		supply_mng = regulator_get_drvdata(rdev->supply);
		if (supply_mng &&
				supply_mng->on_sub_regulator_enabling) {
			supply_mng->on_sub_regulator_enabling(supply_mng, rdev);
		}
	}

	if (xr_regulator_is_sec(rdev->desc->id)) {
		ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_PMIC_REGULATOR_ENABLE,
				rdev->desc->id, 0, NULL);
		if (ret)
			return ret;
	} else {
		ret = xr_regulator_enable(rdev->regmap, rdev->desc->id, 1);
		if (ret < 0) {
			dev_err(&rdev->dev, "power on fail (ret=%d)\n", ret);
			return ret;
		}
	}

	dev_dbg(&rdev->dev, "power on success\n");

	return ret;
}

static int pmic_regulator_disable(struct regulator_dev *rdev)
{
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	if (xr_regulator_is_sec(rdev->desc->id)) {
		ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_PMIC_REGULATOR_DISABLE,
				rdev->desc->id, 0, NULL);
		if (ret)
			return ret;
	} else {
		ret = xr_regulator_enable(rdev->regmap, rdev->desc->id, 0);
		if (ret < 0) {
			dev_err(&rdev->dev, "power off fail (ret=%d)\n", ret);
			return ret;
		}
	}

	dev_dbg(&rdev->dev, "power off success\n");

	return ret;
}

static int pmic_regulator_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV,
		unsigned int *selector)
{
	int ret = 0;
	int vsel;
	int uV = 0;
	int lim_min_uV, lim_max_uV;
	bool is_range;

	if (!rdev)
		return -ENODEV;

	if (min_uV > max_uV) {
		dev_err(&rdev->dev, "request v=[%d, %d] invalid\n", min_uV, max_uV);
		return -EINVAL;
	}

	is_range = min_uV != max_uV;

	lim_min_uV = rdev->desc->volt_table[0];
	lim_max_uV = rdev->desc->volt_table[rdev->desc->n_voltages - 1];
	if (max_uV < lim_min_uV || min_uV > lim_max_uV) {
		dev_err(&rdev->dev, "request v=[%d, %d] is outside possible v=[%d, %d]\n",
				min_uV, max_uV, lim_min_uV, lim_max_uV);
		return -EINVAL;
	}

	for (vsel = 0; vsel < rdev->desc->n_voltages; vsel++) {
		uV = rdev->desc->volt_table[vsel];

		if (is_range) {
			if ((min_uV <= uV) && (uV <= max_uV)) {
				*selector = vsel;
				break;
			}
		} else {
			/* If min_uV==max_uV, select a voltage not bigger than max_uV */
			if (uV == max_uV) {
				*selector = vsel;
				break;
			} else if (uV > max_uV) {
				if ((vsel > 0) && (rdev->desc->volt_table[vsel - 1] < max_uV)) {
					*selector = vsel - 1;
					break;
				}
			}
		}
	}

	if (vsel == rdev->desc->n_voltages) {
		dev_err(&rdev->dev, "request v=[%d, %d], no suitable voltage found\n",
				min_uV, max_uV);
		return -EINVAL;
	}

	uV = rdev->desc->volt_table[*selector];

	if (xr_regulator_is_sec(rdev->desc->id)) {
		ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_PMIC_REGULATOR_SET_MV,
				rdev->desc->id, uV / UV_PER_MV, NULL);
		if (ret)
			return ret;
	} else {
		ret = xr_regulator_set_voltage(rdev->regmap, rdev->desc->id, uV / UV_PER_MV);
		if (ret < 0) {
			dev_err(&rdev->dev, "set_voltage fail (ret=%d)\n", ret);
			return ret;
		}
	}

	dev_dbg(&rdev->dev, "set_voltage to %dmV\n", uV / UV_PER_MV);

	return ret;
}

static int pmic_regulator_get_voltage(struct regulator_dev *rdev)
{
	unsigned long data;
	int ret = 0;
	uint16_t mv;

	if (!rdev)
		return -ENODEV;

	if (xr_regulator_is_sec(rdev->desc->id)) {
		ret = xr_regulator_ffa_direct_message(&rdev->dev, FID_BL31_PMIC_REGULATOR_GET_MV,
				rdev->desc->id, 0, &data);
		if (ret)
			return ret;
		mv = data;
	} else {
		ret = xr_regulator_get_voltage(rdev->regmap, rdev->desc->id, &mv);
		if (ret < 0) {
			dev_err(&rdev->dev, "get_voltage fail (ret=%d)\n", ret);
			return ret;
		}
	}

	dev_dbg(&rdev->dev, "get_voltage: %dmV\n", mv);

	return mv * UV_PER_MV;
}

static unsigned int mode_xr_to_kernel(unsigned int mode)
{
	switch (mode) {
	case XR_REGULATOR_MODE_NORMAL:
		return REGULATOR_MODE_NORMAL;

	case XR_REGULATOR_MODE_LP:
		return REGULATOR_MODE_IDLE;

	case XR_REGULATOR_MODE_INVALID:
	default:
		return REGULATOR_MODE_INVALID;
	}
}

static unsigned int mode_kernel_to_xr(unsigned int mode)
{
	switch (mode) {
	case REGULATOR_MODE_FAST:
	case REGULATOR_MODE_NORMAL:
		return XR_REGULATOR_MODE_NORMAL;

	case REGULATOR_MODE_IDLE:
	case REGULATOR_MODE_STANDBY:
		return XR_REGULATOR_MODE_LP;

	case REGULATOR_MODE_INVALID:
	default:
		return XR_REGULATOR_MODE_INVALID;
	}
}

static uint32_t pmic_regulator_get_mode(struct regulator_dev *rdev)
{
	int ret = 0;
	uint16_t mode = REGULATOR_MODE_INVALID;

	if (!rdev)
		return REGULATOR_MODE_INVALID;

	if (xr_regulator_is_sec(rdev->desc->id)) {
		/* TODO: smc call for secure regulator */
		dev_info(&rdev->dev, "secure regulator not implement yet\n");
		return 0;
	}

	ret = xr_regulator_get_mode(rdev->regmap, rdev->desc->id, &mode);
	if (ret < 0) {
		dev_err(&rdev->dev, "get_mode fail (ret=%d)\n", ret);
		return REGULATOR_MODE_INVALID;
	}

	dev_dbg(&rdev->dev, "get_mode: %d\n", mode);

	return mode_xr_to_kernel(mode);
}

static int pmic_regulator_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	unsigned int xr_mode;
	int ret = 0;

	if (!rdev)
		return -ENODEV;

	xr_mode = mode_kernel_to_xr(mode);

	if (xr_regulator_is_sec(rdev->desc->id)) {
		/* TODO: smc call for secure regulator */
		dev_info(&rdev->dev, "secure regulator not implement yet\n");
		return 0;
	}

	ret = xr_regulator_set_mode(rdev->regmap, rdev->desc->id, xr_mode);
	if (ret < 0) {
		dev_err(&rdev->dev, "set_mode fail (ret=%d)\n", ret);
		return ret;
	}

	/*
	 * regulator hook need this event, regulator core do not support this event,
	 * so send this event in driver
	 */
	blocking_notifier_call_chain(&rdev->notifier,
			REGULATOR_EVENT_SET_MODE, (void *)(uintptr_t)mode);

	dev_dbg(&rdev->dev, "set_mode to %d\n", xr_mode);

	return 0;
}

const struct regulator_ops xring_regulator_pmic_ops = {
	.is_enabled  = pmic_regulator_is_enabled,
	.enable      = pmic_regulator_enable,
	.disable     = pmic_regulator_disable,
	.set_voltage = pmic_regulator_set_voltage,
	.get_voltage = pmic_regulator_get_voltage,
	.get_mode    = pmic_regulator_get_mode,
	.set_mode    = pmic_regulator_set_mode,
};

static const struct xring_regulator_pmic xr_pmic_regl = {
	.rdesc = {
		.ops = &xring_regulator_pmic_ops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = dt_parse,
};

static const struct of_device_id xring_of_match_tbl[] = {
	{
		.compatible = "xring,pmic-regulator",
		.data = &xr_pmic_regl,
	},
	{ /* end */ }
};

static int of_xring_regulator_pmic(struct device *dev,
		struct xring_regulator_pmic **xpm)
{
	const struct of_device_id *match = NULL;
	const struct xring_regulator_pmic *temp = NULL;

	/* to check which type of regulator this is */
	match = of_match_device(xring_of_match_tbl, dev);
	if (!match) {
		dev_err(dev, "get xring pmic regulator fail!\n\r");
		return -EINVAL;
	}

	temp = match->data;

	*xpm = kmemdup(temp, sizeof(struct xring_regulator_pmic), GFP_KERNEL);
	if (!(*xpm))
		return -ENOMEM;

	return 0;
}

static int pmic_regulator_suspend(struct device *dev)
{
	struct xring_regulator_pmic *xpr = dev_get_drvdata(dev);
	struct regulator_dev *rdev = xpr->rdev;
	struct regulator *consumer;
	const char *consumer_name;
	const struct regulator_state *rstate;

	dev_dbg(dev, "%s ++\n", __func__);

	if (!rdev->constraints->always_on && rdev->desc->ops->is_enabled(rdev)) {
		rstate = &rdev->constraints->state_mem;
		if (rstate->enabled == ENABLE_IN_SUSPEND) {
			dev_dbg(dev, "%s -- ENABLE_IN_SUSPEND\n", __func__);
			return 0;
		}

		dev_err(dev, "ERROR: %s is not disabled, rdev use_count:%d\n", rdev->constraints->name, rdev->use_count);

		list_for_each_entry(consumer, &rdev->consumer_list, list) {
			if (consumer->enable_count) {
				consumer_name = consumer->supply_name ? consumer->supply_name :
					consumer->dev ? dev_name(consumer->dev) : "deviceless";
				dev_err(dev, "consumer %s is not disabled, enable_count:%d\n",
						consumer_name, consumer->enable_count);
			}
		}
	}

	dev_dbg(dev, "%s --\n", __func__);

	return 0;
}

static int pmic_regulator_resume(struct device *dev)
{
	dev_dbg(dev, "%s ++\n", __func__);

	dev_dbg(dev, "%s --\n", __func__);

	return 0;
}

static const struct dev_pm_ops xring_regulator_pmic_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(pmic_regulator_suspend, pmic_regulator_resume)
};

static int xring_regulator_pmic_probe(struct platform_device *pdev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	struct regulator_desc *rdesc = NULL;
	struct regulator_dev *rdev = NULL;
	struct xring_regulator_pmic *xpr = NULL;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config config = {};
	struct regmap *regmap = NULL;
	unsigned int *volt_table = NULL;
	int ret = 0;
	int index;

	dev = &pdev->dev;
	np = dev->of_node;

	dev_dbg(dev, "xirng pmic regulator probe start\n");

	regmap = dev_get_regmap(dev->parent, NULL);
	if (!regmap)
		return -ENODEV;

	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (!initdata) {
		dev_err(dev, "get regulator init data error !\n");
		return -EINVAL;
	}

	ret = of_xring_regulator_pmic(dev, &xpr);
	if (ret) {
		dev_err(dev, "allocate xring_regulator_pmic fail!\n");
		goto xring_regulator_pmic_probe_end;
	}

	mutex_init(&xpr->regulator_mng.lock);
	xpr->regulator_mng.on_sub_regulator_enabling = pmic_sub_reg_enabling;

	rdesc = &xpr->rdesc;
	rdesc->name = initdata->constraints.name;

	if (of_property_present(np, "vin-supply"))
		initdata->supply_regulator = "vin";

	/* to parse device tree data for regulator specific */
	ret = xpr->dt_parse(xpr, pdev);
	if (ret) {
		dev_err(dev, "device tree parameter parse error!\n");
		goto xring_regulator_pmic_probe_end;
	}

	xpr->min_uV = initdata->constraints.min_uV;
	xpr->max_uV = initdata->constraints.max_uV;
	if (xpr->min_uV > xpr->max_uV) {
		dev_err(dev, "min/max microvolt invalid!\n");
		ret = -EINVAL;
		goto xring_regulator_pmic_probe_end;
	}

	xpr->rdesc.n_voltages = xpr->max_uV - xpr->min_uV;
	if (xpr->rdesc.n_voltages % xpr->step_uV)
		dev_warn(dev, "vol range is not multiple of step\n");

	xpr->rdesc.n_voltages = xpr->rdesc.n_voltages / xpr->step_uV + 1;

	volt_table = devm_kzalloc(dev, sizeof(unsigned int) * xpr->rdesc.n_voltages, GFP_KERNEL);
	if (!volt_table)
		return -ENOMEM;

	for (index = 0; index < xpr->rdesc.n_voltages; index++)
		volt_table[index] = xpr->min_uV + xpr->step_uV * index;

	xpr->rdesc.volt_table = volt_table;

	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = &xpr->regulator_mng;
	config.of_node = pdev->dev.of_node;
	xpr->np = np;

	/* register regulator */
	rdev = regulator_register(&pdev->dev, rdesc, &config);
	if (IS_ERR(rdev)) {
		dev_err(dev, "failed to register %s\n", rdesc->name);
		ret = PTR_ERR(rdev);
		goto xring_regulator_pmic_probe_end;
	}

	rdev->regmap = regmap;
	xpr->rdev = rdev;
	platform_set_drvdata(pdev, xpr);

	dev_info(dev, "xring pmic regulator probe end\n");

	return 0;

xring_regulator_pmic_probe_end:
	kfree(xpr);
	return ret;
}

static int xring_regulator_pmic_remove(struct platform_device *pdev)
{
	struct xring_regulator_pmic *xpr = NULL;

	if (!pdev)
		return -ENODEV;

	xpr = platform_get_drvdata(pdev);
	if (!xpr)
		return -ENODEV;

	regulator_unregister(xpr->rdev);

	kfree(xpr);
	return 0;
}

static struct platform_driver xring_regulator_pmic_driver = {
	.driver = {
		.name = "xring_regulator_pmic",
		.owner = THIS_MODULE,
		.of_match_table = xring_of_match_tbl,
		.pm = &xring_regulator_pmic_dev_pm_ops,
	},
	.probe = xring_regulator_pmic_probe,
	.remove = xring_regulator_pmic_remove,
};

int xring_regulator_pmic_init(void)
{
	return platform_driver_register(&xring_regulator_pmic_driver);
}

void xring_regulator_pmic_exit(void)
{
	platform_driver_unregister(&xring_regulator_pmic_driver);
}
