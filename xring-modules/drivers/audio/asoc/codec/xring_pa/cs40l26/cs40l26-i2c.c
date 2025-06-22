// SPDX-License-Identifier: GPL-2.0
//
// cs40l26-i2c.c -- CS40L26 I2C Driver
//
// Copyright 2022 Cirrus Logic, Inc.
//
// Author: Fred Treven <fred.treven@cirrus.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "include/cs40l26.h"

int cs40l26_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct cs40l26_private *cs40l26;
	int ret;

	cs40l26 = devm_kzalloc(&client->dev, sizeof(struct cs40l26_private), GFP_KERNEL);
	if (!cs40l26)
		return -ENOMEM;

	i2c_set_clientdata(client, cs40l26);

	cs40l26->regmap = devm_regmap_init_i2c(client, &cs40l26_regmap);
	if (IS_ERR(cs40l26->regmap)) {
		ret = PTR_ERR(cs40l26->regmap);
		dev_err(&client->dev, "Failed to allocate register map: %d\n", ret);
		return ret;
	}

	cs40l26->dev = &client->dev;
	cs40l26->irq = client->irq;

	return cs40l26_probe(cs40l26);
}
EXPORT_SYMBOL_GPL(cs40l26_i2c_probe);

void cs40l26_i2c_remove(struct i2c_client *client)
{
	struct cs40l26_private *cs40l26 = i2c_get_clientdata(client);

	cs40l26_remove(cs40l26);
}
EXPORT_SYMBOL_GPL(cs40l26_i2c_remove);

MODULE_DESCRIPTION("CS40L26 I2C Driver");
MODULE_AUTHOR("Fred Treven, Cirrus Logic Inc. <fred.treven@cirrus.com>");
MODULE_LICENSE("GPL");
