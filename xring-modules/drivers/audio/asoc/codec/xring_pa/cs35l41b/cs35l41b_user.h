/* SPDX-License-Identifier: GPL-2.0-only */
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

#define CS35L41B_NUM_SUPPLIES 2

struct cs35l41b_platform_data {
	bool sclk_frc;
	bool lrclk_frc;
};

struct cs35l41b_private {
	struct cs35l41b_platform_data pdata;
	struct device *dev;
	struct regmap *regmap;
	unsigned int reset_gpio;
	struct regulator_bulk_data supplies[CS35L41B_NUM_SUPPLIES];
	int num_supplies;
	int irq;
	bool bus_spi;
};

enum cs35l41b_enable_type {
	CS35L41B_DISABLE = 0,
	CS35L41B_ENABLE,
};

int cs35l41b_probe(struct cs35l41b_private *cs35l41b, struct cs35l41b_platform_data *pdata);
int cs35l41b_i2c_probe(struct i2c_client *client,
				const struct i2c_device_id *id);
int cs35l41b_i2c_remove(struct i2c_client *client);
