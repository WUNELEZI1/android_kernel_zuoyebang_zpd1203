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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <sound/soc.h>

#include "cs35l41b_user.h"

static struct regmap_config cs35l41b_regmap_i2c = {
	.reg_bits = 32,
	.val_bits = 32,
};

int cs35l41b_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct cs35l41b_private *cs35l41b;
	struct device *dev = &client->dev;
	struct cs35l41b_platform_data *pdata = dev_get_platdata(dev);
	const struct regmap_config *regmap_config = &cs35l41b_regmap_i2c;
	int ret;

	cs35l41b = devm_kzalloc(dev, sizeof(struct cs35l41b_private), GFP_KERNEL);

	if (cs35l41b == NULL)
		return -ENOMEM;

	cs35l41b->dev = dev;

	if (cs35l41b->dev == NULL)
		return -ENOMEM;

	cs35l41b->irq = client->irq;
	cs35l41b->bus_spi = false;

	i2c_set_clientdata(client, cs35l41b);
	cs35l41b->regmap = devm_regmap_init_i2c(client, regmap_config);
	if (IS_ERR(cs35l41b->regmap)) {
		ret = PTR_ERR(cs35l41b->regmap);
		dev_err(cs35l41b->dev, "Failed regmap\n");
		return ret;
	}

	return cs35l41b_probe(cs35l41b, pdata);
}
EXPORT_SYMBOL_GPL(cs35l41b_i2c_probe);

int cs35l41b_i2c_remove(struct i2c_client *client)
{
	struct cs35l41b_private *cs35l41b = i2c_get_clientdata(client);

	snd_soc_unregister_component(cs35l41b->dev);
	return 0;
}
EXPORT_SYMBOL_GPL(cs35l41b_i2c_remove);

MODULE_DESCRIPTION("I2C CS35L41B driver");
MODULE_LICENSE("GPL");
