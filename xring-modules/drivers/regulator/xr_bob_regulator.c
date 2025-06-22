// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * buckboost regulator
 *
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#define pr_fmt(fmt)     "BOB_REGULATOR: " fmt
#define dev_fmt(fmt)    pr_fmt(fmt)

#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/delay.h>
#include <linux/time.h>
#include "soc/xring/xr_timestamp.h"
#include "soc/xring/xr_regulator_mng.h"
#include "xr_bob_regulator.h"
#include "soc/xring/xr_pmic.h"

#define BUCKBOOST_REG_CONTROL 0x1
#define BUCKBOOST_REG_STATUS  0x2
#define BUCKBOOST_REG_DEVID   0x3
#define BUCKBOOST_REG_MAX     BUCKBOOST_REG_DEVID

#define BUCKBOOST_REG_CONTROL_FPWM  (1 << 3)

struct xring_regulator_bob_data {
	struct i2c_client           *client;
	struct regmap               *reg;
	struct regulator_desc       desc;
	struct regulator_init_data  *init_data;
	struct regulator_dev        *rdev;
	struct xr_regulator_manager regulator_mng;
	u32                         sub_regulator_on_delay; // unit: us
};

int xr_bob_fpwm_set_enable(struct regulator *bob_regulator, bool en)
{
	struct xr_regulator_manager *mng;
	struct xring_regulator_bob_data *drvdata;
	struct i2c_client *client;
	unsigned int reg_val = 0;
	unsigned int reg_wr_val = 0;
	int ret;

	if (bob_regulator == NULL) {
		pr_err("bob_regulator null_ptr!\n");
		return -EINVAL;
	}

	mng = regulator_get_drvdata(bob_regulator);
	drvdata = container_of(mng, struct xring_regulator_bob_data, regulator_mng);
	client = drvdata->client;

	ret = regmap_read(drvdata->reg, BUCKBOOST_REG_CONTROL, &reg_val);
	if (ret < 0) {
		dev_err(&client->dev, "read BUCKBOOST_REG_CONTROL fail, ret=%d!\n", ret);
		return ret;
	}

	if (en)
		reg_wr_val = reg_val | BUCKBOOST_REG_CONTROL_FPWM;
	else
		reg_wr_val = reg_val & (~BUCKBOOST_REG_CONTROL_FPWM);
	dev_info(&client->dev, "xring_bob_set_fpwm %s: reg_val=0x%x, reg_wr_val=0x%x\n",
			(en ? "enable" : "disable"), reg_val, reg_wr_val);

	ret = regmap_write(drvdata->reg, BUCKBOOST_REG_CONTROL, reg_wr_val);
	if (ret < 0)
		dev_err(&client->dev, "write BUCKBOOST_REG_CONTROL fail, ret=%d!\n", ret);

	return ret;
}
EXPORT_SYMBOL(xr_bob_fpwm_set_enable);

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

static int bob_sub_reg_enabling(struct xr_regulator_manager *mng,
			struct regulator_dev *sub_rdev)
{
	struct xring_regulator_bob_data *drvdata;

	drvdata = container_of(mng, struct xring_regulator_bob_data, regulator_mng);
	if (drvdata->sub_regulator_on_delay) {
		mutex_lock(&mng->lock);

		ensured_time_after(mng->last_enable_time, drvdata->sub_regulator_on_delay);

		mng->last_enable_time = xr_timestamp_gettime();
		do_div(mng->last_enable_time, NSEC_PER_USEC);
		mutex_unlock(&mng->lock);
	}

	return 0;
}

static int of_get_regulator_bob_config(struct device *dev,
				struct xring_regulator_bob_data *drvdata)
{
	struct device_node *np = dev->of_node;
	struct regulator_desc *desc = &drvdata->desc;
	struct regulator_init_data *init_data;
	int ret;

	init_data = of_get_regulator_init_data(dev, dev->of_node, desc);
	if (!init_data) {
		dev_err(dev, "Fail to get BOB regulator_init_data\n");
		return -EINVAL;
	}
	drvdata->init_data = init_data;

	init_data->constraints.apply_uV = 0;

	desc->name = init_data->constraints.name;

	if (init_data->constraints.min_uV == init_data->constraints.max_uV) {
		desc->fixed_uV = init_data->constraints.min_uV;
		desc->n_voltages = 1;
	} else {
		dev_err(dev, "BOB regulator specified with variable voltages\n");
		return -EINVAL;
	}

	// this param is optional, do not return error if not exist
	// for converity check, add the ret assign
	ret = of_property_read_u32(np, "xring,sub_regulator_on_delay",
			&drvdata->sub_regulator_on_delay);

	return 0;
}

static const struct regulator_ops regulator_bob_ops = {
};

static const struct regmap_config buckboost_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = BUCKBOOST_REG_MAX,
};

static int xring_regulator_bob_probe(struct i2c_client *client)
{
	struct xring_regulator_bob_data *drvdata;
	struct regulator_config config = { };
	int ret;

	drvdata = devm_kzalloc(&client->dev, sizeof(struct xring_regulator_bob_data),
							GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	drvdata->client = client;
	drvdata->reg = devm_regmap_init_i2c(client, &buckboost_regmap_config);
	if (IS_ERR(drvdata->reg))
		return PTR_ERR(drvdata->reg);

	mutex_init(&drvdata->regulator_mng.lock);
	drvdata->regulator_mng.on_sub_regulator_enabling = bob_sub_reg_enabling;

	ret = of_get_regulator_bob_config(&client->dev, drvdata);
	if (ret < 0)
		return ret;

	drvdata->desc.type = REGULATOR_VOLTAGE;
	drvdata->desc.owner = THIS_MODULE;
	drvdata->desc.ops = &regulator_bob_ops;

	config.dev = &client->dev;
	config.init_data = drvdata->init_data;
	config.driver_data = &drvdata->regulator_mng;
	config.of_node = client->dev.of_node;

	drvdata->rdev = devm_regulator_register(&client->dev, &drvdata->desc, &config);
	if (IS_ERR(drvdata->rdev)) {
		ret = dev_err_probe(&client->dev, PTR_ERR(drvdata->rdev),
				    "Failed to register regulator: %ld\n",
				    PTR_ERR(drvdata->rdev));
		return ret;
	}

	i2c_set_clientdata(client, drvdata);

	dev_info(&client->dev, "xring bob regulator %s probe end supplying %duV\n",
			drvdata->desc.name, drvdata->desc.fixed_uV);

	return 0;
}

static void xring_regulator_bob_remove(struct i2c_client *client)
{
	struct xring_regulator_bob_data *drvdata;

	drvdata = i2c_get_clientdata(client);

	mutex_destroy(&drvdata->regulator_mng.lock);
}

static const struct of_device_id bob_of_match[] = {
	{
		.compatible = "xring,buckboost-regulator",
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, bob_of_match);

static struct i2c_driver xring_regulator_bob_driver = {
	.driver		= {
		.name		= "xring_regulator_bob",
		.of_match_table = of_match_ptr(bob_of_match),
	},
	.probe = xring_regulator_bob_probe,
	.remove = xring_regulator_bob_remove,
};

int xring_regulator_bob_init(void)
{
	return i2c_add_driver(&xring_regulator_bob_driver);
}

void xring_regulator_bob_exit(void)
{
	i2c_del_driver(&xring_regulator_bob_driver);
}
