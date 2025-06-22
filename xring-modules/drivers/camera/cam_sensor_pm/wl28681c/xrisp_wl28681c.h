/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 *  <zhangzhuo6@xiaomi.com>
 */

#ifndef __WL28681C_H
#define __WL28681C_H

#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/atomic.h>


#define MV_PER_V               1000
#define WL28681C_ID            0x0D
#define WL28681C_ID_ADDR       0x00
#define WL28681C_RESET_ADDR    0x11
#define WL28681C_FLT_SD_OFF    0x06
#define WL28681C_FLT_SD_ON     0x07
#define WL28681C_UVP_ADDR      0x15
#define WL28681C_OCP_ADDR      0x16
#define WL28681C_VIN_UVP_ADDR  0x17
#define WL28681C_EN_ADDR       0x03
#define GPIO_LOW               0
#define GPIO_HIGH              1
#define WL28681_PINCTRL_NUM    4

struct wl28681c_chip {
	struct device *dev;
	struct i2c_client *client;
	const char *name;
	uint32_t en_gpio;
	uint32_t rgltr_num;
	int irq;
	uint32_t inter_gpio;
	uint32_t *wldo_volt_addr;
	struct regmap *rmap;
	struct dentry *debugfs_dir;
	uint32_t reg_addr;
	uint32_t reg_value;
	uint32_t delay_after_rst;
	const char **rgltr_name_arr;
	struct pinctrl *pinctrl_t;
	const char *pinctrl_name[WL28681_PINCTRL_NUM];
};

#endif /* __WL28681C_H */
