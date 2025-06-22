// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */

#define pr_fmt(fmt)     "PMIC_GPIO: " fmt

#include <linux/kernel.h>
#include <linux/errno.h>
#include "soc/xring/xr_pmic.h"
#include "dt-bindings/xring/platform-specific/pmic/top_reg.h"
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <soc/xring/xr-pmic-spmi.h>

int xr_pmic_gpio_set_value(int gpio, int value)
{
	unsigned int buf;
	int offset = gpio;
	u8 reg_val;
	int ret;

	if (gpio != 3) {
		pr_err("gpio_direction_output: invalid gpio num %d\n", gpio);
		return -EINVAL;
	}
	/* set gpio output value */
	ret = xr_pmic_reg_read(TOP_REG_INTF_GPIO_CFG3, &buf);
	if (ret) {
		pr_err("read pmic_gpio[%d] io_val failed!\n", gpio);
		return -EIO;
	}
	reg_val = buf & 0xff;

	if (!!value)
		reg_val |= BIT(offset);
	else
		reg_val &= ~BIT(offset);
	ret = xr_pmic_reg_write(TOP_REG_INTF_GPIO_CFG3, reg_val);
	if (ret != 0) {
		pr_err("write pmic_gpio[%d] io_val fail!\n", gpio);
		return -EIO;
	}

	return 0;
}
EXPORT_SYMBOL(xr_pmic_gpio_set_value);

int xr_pmic_gpio_direction_output(int gpio, int value)
{
	unsigned int buf;
	int offset = gpio;
	int ret;
	u8 reg_val;

	ret = xr_pmic_gpio_set_value(gpio, value);
	if (ret != 0) {
		pr_err("gpio set failed, ret = %d\n", ret);
		return ret;
	}

	/* set gpio direction */
	ret = xr_pmic_reg_read(TOP_REG_INTF_GPIO_CFG1, &buf);
	if (ret) {
		pr_err("read pmic_gpio[%d] io_direct failed!\n", gpio);
		return -EIO;
	}
	reg_val = buf & 0xff;

	reg_val |= BIT(offset);
	ret = xr_pmic_reg_write(TOP_REG_INTF_GPIO_CFG1, reg_val);
	if (ret != 0) {
		pr_err("write pmic_gpio[%d] io_direct fail\n", gpio);
		return -EIO;
	}

	return 0;
}
EXPORT_SYMBOL(xr_pmic_gpio_direction_output);
