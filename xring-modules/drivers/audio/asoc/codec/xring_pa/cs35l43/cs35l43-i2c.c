// SPDX-License-Identifier: GPL-2.0

/*
 * cs35l43-i2c.c -- CS35l41 I2C driver
 *
 * Copyright 2020 Cirrus Logic, Inc.
 *
 * Author:	David Rhodes	<david.rhodes@cirrus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#define DEBUG
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

#include "wm_adsp.h"
#include "cs35l43.h"
#include "cs35l43-user.h"
// #include <sound/cs35l43.h>

static struct regmap_config cs35l43_regmap_i2c = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.reg_format_endian = REGMAP_ENDIAN_BIG,
	.val_format_endian = REGMAP_ENDIAN_BIG,
	.max_register = CS35L43_DSP1_PMEM_5114,
	.reg_defaults = cs35l43_reg,
	.num_reg_defaults = CS35L43_NUM_DEFAULTS,
	.volatile_reg = cs35l43_volatile_reg,
	.readable_reg = cs35l43_readable_reg,
	.precious_reg = cs35l43_precious_reg,
	.cache_type = REGCACHE_RBTREE,
};

static const struct i2c_device_id cs35l43_id_i2c[] = {
	{"cs35l43", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cs35l43_id_i2c);

struct debugfs_spk {
	unsigned int reg;
	unsigned int val;
	struct regmap *regmap;
};

struct debugfs_spk debugfs_data;

int spk_reg_debugs_write(unsigned int reg, unsigned int val)
{
	unsigned int ret, value;

	ret = regmap_write(debugfs_data.regmap, reg, val);
	if (ret)
		return -EINVAL;
	regmap_read(debugfs_data.regmap, reg, &value);
	pr_info("%s: value = 0x%X\n", __func__, value);

	return ret;
}
EXPORT_SYMBOL_GPL(spk_reg_debugs_write);

int spk_reg_debugs_read(unsigned int reg)
{
	unsigned int value;

	regmap_read(debugfs_data.regmap, reg, &value);
	pr_info("%s: value = 0x%X\n", __func__, value);

	return 0;
}
EXPORT_SYMBOL_GPL(spk_reg_debugs_read);

int cs35l43_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct cs35l43_private *cs35l43;
	struct device *dev = &client->dev;
	struct cs35l43_platform_data *pdata = dev_get_platdata(dev);
	const struct regmap_config *regmap_config = &cs35l43_regmap_i2c;
	int ret;

	cs35l43 = devm_kzalloc(dev, sizeof(struct cs35l43_private), GFP_KERNEL);

	if (cs35l43 == NULL)
		return -ENOMEM;

	cs35l43->dev = dev;
	cs35l43->irq = client->irq;

	i2c_set_clientdata(client, cs35l43);
	cs35l43->regmap = devm_regmap_init_i2c(client, regmap_config);
	if (IS_ERR(cs35l43->regmap)) {
		ret = PTR_ERR(cs35l43->regmap);
		dev_err(cs35l43->dev, "Failed to allocate register map: %d\n",
			ret);
	#if IS_ENABLED(CONFIG_MIEV)
		mievent_report(906001353,"PA i2c exception",cs35l43->dev);
	#endif
		return ret;
	}

	debugfs_data.regmap = cs35l43->regmap;

	return cs35l43_probe(cs35l43, pdata);
}
EXPORT_SYMBOL_GPL(cs35l43_i2c_probe);

int cs35l43_i2c_remove(struct i2c_client *client)
{
	int ret = 0;

	struct cs35l43_private *cs35l43 = i2c_get_clientdata(client);

	cs35l43_remove(cs35l43);

	return ret;
}
EXPORT_SYMBOL_GPL(cs35l43_i2c_remove);

MODULE_DESCRIPTION("I2C CS35L43 driver");
MODULE_AUTHOR("David Rhodes, Cirrus Logic Inc, <david.rhodes@cirrus.com>");
