/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __WL28661D_H
#define __WL28661D_H

#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/atomic.h>


#define MV_PER_V                     1000
#define WL28661D_ID                  0x33
#define WL28661D_ID_ADDR             0x00
#define WL28661D_EN_ADDR             0x0e
#define WL28661D_DISCHARGE_ADDR      0x02
#define WL28661D_DISCHARGE_LDO_EN    1

struct wl28661d_chip {
	struct device *dev;
	struct i2c_client *client;
	const char *name;
	uint32_t rgltr_num;
	uint32_t *wldo_volt_addr;
	struct regmap *rmap;
	struct dentry *debugfs_dir;
	uint32_t reg_addr;
	uint32_t reg_value;
	const char **rgltr_name_arr;
};

#endif /* __WL28661D_H */
