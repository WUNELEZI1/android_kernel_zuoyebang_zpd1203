/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_FLASH_COMMON_H_
#define _XRISP_FLASH_COMMON_H_

#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/i2c.h>

#define FLASH_MAX_NUM  2
struct xrisp_flash_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regm;
	struct pinctrl *pinctrl;
	uint32_t flash_dev_id[FLASH_MAX_NUM];
	atomic_t power_cnt;
};

#endif
