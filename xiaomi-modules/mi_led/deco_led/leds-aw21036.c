// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * leds-aw21036.c
 *
 * Copyright (c) 2020 Shanghai Awinic Technology Co., Ltd. All Rights Reserved
 *
 *  Author: Awinic
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/gpio/consumer.h>
#include <linux/uaccess.h>
#include <linux/leds.h>
#include "leds-aw21036.h"
#include "leds-aw21036-reg.h"
/******************************************************
 *
 * Marco
 *
 ******************************************************/
#define AW21036_I2C_NAME "aw21036_led"
#define AW21036_DRIVER_VERSION "V1.2.0"
#define AW_I2C_RETRIES 2
#define AW_I2C_RETRY_DELAY 1
#define AW_READ_CHIPID_RETRIES 2
#define AW_READ_CHIPID_RETRY_DELAY 1

/******************************************************
 *
 * aw21036 led parameter
 *
 ******************************************************/
#define AW21036_CFG_NAME_MAX	64

struct aw21036_cfg aw21036_cfg_array[] = {
	{aw21036_cfg_led_off, sizeof(aw21036_cfg_led_off)},
	{aw21036_led_all_on, sizeof(aw21036_led_all_on)},
	{aw21036_led_red_on, sizeof(aw21036_led_red_on)},
	{aw21036_led_green_on, sizeof(aw21036_led_green_on)},
	{aw21036_led_blue_on, sizeof(aw21036_led_blue_on)},
	{aw21036_led_breath_forever, sizeof(aw21036_led_breath_forever)},
	/* O2S */
	{aw21036_led_1_5_on, sizeof(aw21036_led_1_5_on)},
	{aw21036_led_6_17_on, sizeof(aw21036_led_6_17_on)},
	{aw21036_led_18_21_on, sizeof(aw21036_led_18_21_on)},
	{aw21036_led_22_36_on, sizeof(aw21036_led_22_36_on)},
	{aw21036_led_1_5_off, sizeof(aw21036_led_1_5_off)},
	{aw21036_led_6_17_off, sizeof(aw21036_led_6_17_off)},
	{aw21036_led_18_21_off, sizeof(aw21036_led_18_21_off)},
	{aw21036_led_22_36_off, sizeof(aw21036_led_22_36_off)},
	{aw21036_led_1_5_breath_forever, sizeof(aw21036_led_1_5_breath_forever)},
	{aw21036_led_6_17_breath_forever, sizeof(aw21036_led_6_17_breath_forever)},
	{aw21036_led_18_21_breath_forever, sizeof(aw21036_led_18_21_breath_forever)},
	{aw21036_led_22_36_breath_forever, sizeof(aw21036_led_22_36_breath_forever)},
};

/******************************************************
 *
 * aw21036 i2c write/read
 *
 ******************************************************/
static int
aw21036_i2c_write(struct aw21036 *aw21036, unsigned char reg_addr, unsigned char reg_data)
{
	int ret = -1;
	unsigned char cnt = 0;

	while (cnt < AW_I2C_RETRIES) {
		ret = i2c_smbus_write_byte_data(aw21036->i2c, reg_addr, reg_data);
		if (ret < 0)
			pr_err("%s: i2c_write cnt=%d error=%d\n", __func__, cnt, ret);
		else
			break;

		cnt++;
		usleep_range(AW_I2C_RETRY_DELAY * 1000, AW_I2C_RETRY_DELAY * 1000 + 500);
	}

	return ret;
}

static int
aw21036_i2c_read(struct aw21036 *aw21036, unsigned char reg_addr, unsigned char *reg_data)
{
	int ret = -1;
	unsigned char cnt = 0;

	while (cnt < AW_I2C_RETRIES) {
		ret = i2c_smbus_read_byte_data(aw21036->i2c, reg_addr);
		if (ret < 0) {
			pr_err("%s: i2c_read cnt=%d error=%d\n", __func__, cnt, ret);
		} else {
			*reg_data = ret;
			break;
		}
		cnt++;
		usleep_range(AW_I2C_RETRY_DELAY * 1000, AW_I2C_RETRY_DELAY * 1000 + 500);
	}

	return ret;
}

static int aw21036_i2c_write_bits(struct aw21036 *aw21036,
				  unsigned char reg_addr, unsigned int mask,
				  unsigned char reg_data)
{
	unsigned char reg_val;

	aw21036_i2c_read(aw21036, reg_addr, &reg_val);
	reg_val &= mask;
	reg_val |= (reg_data & (~mask));
	aw21036_i2c_write(aw21036, reg_addr, reg_val);

	return 0;
}

static int aw21036_chip_enable(struct aw21036 *aw21036, bool flag)
{
	if (flag)
		aw21036_i2c_write_bits(aw21036, AW21036_REG_GCR,
					AW21036_BIT_GCR_CHIPEN_MASK,
					AW21036_BIT_GCR_CHIPEN_ENABLE);
	else
		aw21036_i2c_write_bits(aw21036, AW21036_REG_GCR,
					AW21036_BIT_GCR_CHIPEN_MASK,
					AW21036_BIT_GCR_CHIPEN_DISABLE);
	return 0;
}

static int aw21036_pwm_freq_cfg(struct aw21036 *aw21036, unsigned int mode)
{
	switch (mode) {
	case AW21036_PWM_FREQ_31K:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_31K);
		break;
	case AW21036_PWM_FREQ_62K:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_62K);
		break;
	case AW21036_PWM_FREQ_125K:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_125K);
		break;
	case AW21036_PWM_FREQ_256K:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_256K);
		break;
	case AW21036_PWM_FREQ_512K:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_512K);
		break;
	case AW21036_PWM_FREQ_1M:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_1M);
		break;
	case AW21036_PWM_FREQ_8M:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_8M);
		break;
	case AW21036_PWM_FREQ_16M:
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_CLKFRQ_MASK,
				       AW21036_BIT_GCR_CLKFRQ_16M);
		break;
	default:
		break;
	}
	return 0;
}

static int aw21036_apse_set(struct aw21036 *aw21036, bool mode)
{
	if (mode) {
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_APSE_MASK,
				       AW21036_BIT_GCR_APSE_ENABLE);
	} else {
		aw21036_i2c_write_bits(aw21036,
				       AW21036_REG_GCR,
				       AW21036_BIT_GCR_APSE_MASK,
				       AW21036_BIT_GCR_APSE_DISABLE);
	}

	return 0;
}

static int aw21036_led_update(struct aw21036 *aw21036)
{
	aw21036_i2c_write(aw21036, AW21036_REG_UPDATE, 0x00);
	return 0;
}

/*****************************************************
 *
 * firmware/cfg update
 *
 *****************************************************/
static void aw21036_update_cfg_array(struct aw21036 *aw21036,
				     unsigned char *p_cfg_data,
				     unsigned int cfg_size)
{
	unsigned int i = 0;

	for (i = 0; i < cfg_size; i += 2)
		aw21036_i2c_write(aw21036, p_cfg_data[i], p_cfg_data[i + 1]);
}

static int aw21036_cfg_update(struct aw21036 *aw21036)
{
	aw21036_update_cfg_array(aw21036,
				 (aw21036_cfg_array[aw21036->effect].p),
				 aw21036_cfg_array[aw21036->effect].count);
	return 0;
}

static int aw21036_sw_reset(struct aw21036 *aw21036)
{
	aw21036_i2c_write(aw21036, AW21036_REG_RESET, 0x00);
	usleep_range(2000, 2500);

	return 0;
}

static int aw21036_hw_reset(struct aw21036 *aw21036)
{
	if (aw21036 && gpio_is_valid(aw21036->reset_gpio)) {
		gpio_set_value_cansleep(aw21036->reset_gpio, 0);
		usleep_range(1000, 1500);
		gpio_set_value_cansleep(aw21036->reset_gpio, 1);
		usleep_range(2000, 2500);
	} else {
		pr_err("%s failed\n", __func__);
	}
	return 0;
}

static int aw21036_hw_off(struct aw21036 *aw21036)
{
	if (aw21036 && gpio_is_valid(aw21036->reset_gpio)) {
		gpio_set_value_cansleep(aw21036->reset_gpio, 0);
		usleep_range(1000, 1500);
	} else {
		pr_err("%s failed\n", __func__);
	}
	return 0;
}


/*****************************************************
 *
 * check chip id and version
 *
 *****************************************************/
static int aw21036_read_chipid(struct aw21036 *aw21036)
{
	int ret = -1;
	unsigned char cnt = 0;
	unsigned char reg_val = 0;

	/* hardware reset */
	aw21036_hw_reset(aw21036);
	usleep_range(2000, 2500);

	while (cnt < AW_READ_CHIPID_RETRIES) {
		ret = aw21036_i2c_read(aw21036, AW21036_REG_RESET, &reg_val);
		if (ret < 0) {
			dev_err(aw21036->dev, "%s failed to read chipid: %d\n", __func__, ret);
		} else {
			if (reg_val == AW21036_CHIPID) {
				pr_info("%s chipid=0x%02X, match aw21036!\n", __func__, reg_val);
				return 0;
			}
		}
		cnt++;
		usleep_range(AW_READ_CHIPID_RETRY_DELAY * 1000,
			     AW_READ_CHIPID_RETRY_DELAY * 1000 + 500);
	}
	pr_info("%s unsupported chip: 0x%x\n", __func__, reg_val);
	return -EINVAL;
}

static int aw21036_read_version(struct aw21036 *aw21036)
{
	int ret = -1;
	unsigned char reg_val = 0;

	ret = aw21036_i2c_read(aw21036, AW21036_REG_VER, &reg_val);
	if (ret < 0)
		dev_err(aw21036->dev, "%s:failed to read version: %d\n", __func__, ret);
	else
		pr_info("%s chip version=0x%02X\n", __func__, reg_val);

	return ret;
}

/*****************************************************
 *
 * aw21036 led cfg
 *
 *****************************************************/
static void aw21036_brightness_work(struct work_struct *work)
{
	struct aw21036 *aw21036 = container_of(work, struct aw21036, brightness_work);

	if (aw21036->cdev.brightness > aw21036->cdev.max_brightness)
		aw21036->cdev.brightness = aw21036->cdev.max_brightness;

	pr_info("%s breath_status = %d, set brightness = %d\n", __func__,
		aw21036->breath_status, aw21036->cdev.brightness);
	if (aw21036->breath_status && aw21036->cdev.brightness == 0) {
		pr_info("%s breath_status is ture, skip set brightness = %d\n", __func__, aw21036->cdev.brightness);
			aw21036->breath_status = false;
		return;
	}
	aw21036_i2c_write(aw21036, AW21036_REG_GCCR, aw21036->cdev.brightness);
}

static void aw21036_set_brightness(struct led_classdev *cdev, enum led_brightness brightness)
{
	struct aw21036 *aw21036 = container_of(cdev, struct aw21036, cdev);

	aw21036->cdev.brightness = brightness;

	schedule_work(&aw21036->brightness_work);
}

static void aw21036_breath_work(struct work_struct *work)
{
	struct aw21036 *aw21036 = container_of(work, struct aw21036, breath_work);

	pr_info("%s working\n", __func__);
	while (aw21036->breath_status) {
		aw21036->effect = 12;  //aw21036_cfg_led_off
		aw21036_cfg_update(aw21036);
		usleep_range(1000000, 1000000+100);
		aw21036->effect = 8;  //aw21036_led_all_on
		aw21036_cfg_update(aw21036);
		usleep_range(1000000, 1000000+100);
	}
	if (!aw21036->breath_status) {
		pr_info("%s exit working 12\n", __func__);
		aw21036->effect = 12;  //aw21036_cfg_led_off
		aw21036_cfg_update(aw21036);
	}
}

static int aw21036_led_blink_set(struct led_classdev *cdev,
	unsigned long *delay_on, unsigned long *delay_off)
{
	struct aw21036 *aw21036 = container_of(cdev, struct aw21036, cdev);
	int ret = 0;

	pr_info("%s delay_on = 0x%lx， delay_off = 0x%lx\n", __func__, *delay_on, *delay_off);
	if (*delay_on == 0 && *delay_off == 0) {
		aw21036->breath_status = true;
		pr_info("%s blank mode enter \n", __func__);
	}
	if (*delay_on == 0xFFFF && *delay_off == 0xFFFF) {
		pr_info("%s blank mode exit \n", __func__);
		aw21036->breath_status = false;
	}
	if (*delay_on != 0 && *delay_on != 0xFFFF) {
		schedule_work(&aw21036->breath_work);
	} else if (*delay_off == 1) {
		
	} else {
		//aw21036->effect = 5;  //aw21036_led_breath_forever
		//aw21036_cfg_update(aw21036);
	}

	return ret;
}
/*****************************************************
 *
 * device tree
 *
 *****************************************************/
static int aw21036_parse_dt(struct device *dev, struct aw21036 *aw21036, struct device_node *np)
{
	unsigned int ret = 0;

	aw21036->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);

	ret = of_property_read_u32(np, "led_current", &aw21036->led_current);
	if (ret) {
		aw21036->led_current = 255;
		dev_err(dev, "%s dts led_current not found\n", __func__);
	} else {
		dev_info(dev, "%s read dts led_current=%d\n", __func__, aw21036->led_current);
	}
	ret = of_property_read_u32(np, "pwm_freq", &aw21036->pwm_freq);
	if (ret) {
		aw21036->pwm_freq = 0;
		dev_err(dev, "%s dts pwm_freq not found\n", __func__);
	} else {
		dev_info(dev, "%s read dts pwm_freq%d\n", __func__, aw21036->pwm_freq);
	}
	aw21036->apse_mode = of_property_read_bool(np, "apse_mode");
	if (aw21036->apse_mode)
		dev_info(dev, "%s driver use apse_mode\n", __func__);
	else
		dev_info(dev, "%s driver use general_mode\n", __func__);

	ret = of_property_read_u32(np, "imax", &aw21036->imax);
	if (ret) {
		dev_err(dev, "%s dts imax not found\n", __func__);
		return ret;
	}
	ret = of_property_read_u32(np, "brightness", &aw21036->cdev.brightness);
	if (ret) {
		dev_err(dev, "%s dts brightness not found\n", __func__);
		return ret;
	}
	ret = of_property_read_u32(np, "max_brightness", &aw21036->cdev.max_brightness);
	if (ret) {
		dev_err(dev, "%s dts max_brightness not found\n", __func__);
		aw21036->cdev.max_brightness = 255;
		return ret;
	}

	return 0;
}

/******************************************************
 *
 * led class dev
 ******************************************************/
static int aw21036_led_init(struct aw21036 *aw21036)
{
	aw21036_chip_enable(aw21036, true);
	usleep_range(200, 300);
	aw21036_pwm_freq_cfg(aw21036, aw21036->pwm_freq);
	aw21036_apse_set(aw21036, aw21036->apse_mode);
	aw21036_i2c_write(aw21036, AW21036_REG_GCCR, aw21036->led_current);
	return 0;
}

/******************************************************
 *
 * sys group attribute: reg
 *
 ******************************************************/
static ssize_t
reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	unsigned int databuf[2] = { 0, 0 };

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2)
		aw21036_i2c_write(aw21036, databuf[0], databuf[1]);
	pr_info("%s databuf[0] = 0x%02x, &databuf[1] = 0x%02x\n", __func__, databuf[0], databuf[1]);
	return len;
}

static ssize_t
reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	ssize_t len = 0;
	unsigned int i = 0;
	unsigned char reg_val = 0;

	for (i = 0; i < AW21036_REG_MAX; i++) {
		if (!(aw21036_reg_access[i] & REG_RD_ACCESS))
			continue;
		aw21036_i2c_read(aw21036, i, &reg_val);
		len += snprintf(buf + len, PAGE_SIZE - len, "reg:0x%02x=0x%02x\n", i, reg_val);
	}
	return len;
}

static ssize_t
hwen_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	int rc;
	unsigned int val = 0;

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	pr_info("%s val = %d\n", __func__, val);
	if (val > 0)
		aw21036_hw_reset(aw21036);
	else
		aw21036_hw_off(aw21036);
	return len;
}

static ssize_t hwen_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	ssize_t len = 0;

	len +=
	snprintf(buf + len, PAGE_SIZE - len, "hwen=%d\n", gpio_get_value(aw21036->reset_gpio));
	return len;
}

static ssize_t
sw_reset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	int rc;
	unsigned int val = 0;

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	pr_info("%s val = %d\n", __func__, val);
	if (val > 0) {
		aw21036_sw_reset(aw21036);
		aw21036_led_init(aw21036);
	}

	return len;
}

static ssize_t effect_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	ssize_t len = 0;
	unsigned int i;

	for (i = 0; i < sizeof(aw21036_cfg_array) / sizeof(struct aw21036_cfg); i++) {
		len += snprintf(buf + len, PAGE_SIZE - len, "effect[%d]: %ps\n",
				i, aw21036_cfg_array[i].p);
	}
	len += snprintf(buf + len, PAGE_SIZE - len, "current effect[%d]: %ps\n",
			aw21036->effect, aw21036_cfg_array[aw21036->effect].p);
	return len;
}

static ssize_t
effect_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);
	int rc;
	unsigned int val = 0;

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;
	pr_info("%s val = %d\n", __func__, val);
	if (val < (sizeof(aw21036_cfg_array) / sizeof(struct aw21036_cfg) + 1)) {
		aw21036->effect = val;
		aw21036_cfg_update(aw21036);
	}
	return len;
}

static ssize_t
rgbcolor_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	unsigned int databuf[2] = { 0, 0 };
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw21036 *aw21036 = container_of(led_cdev, struct aw21036, cdev);

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2) {
		aw21036_i2c_write(aw21036, AW21036_REG_GCFG0, 0x00);/*GEn = 0 */
		aw21036_i2c_write(aw21036, AW21036_REG_GCFG1, 0x10);/*GCOLDIS = 1 GEn = 0*/
		aw21036_i2c_write(aw21036, AW21036_REG_GCR2, 0x01);/*RGBMD = 1*/
		aw21036_i2c_write(aw21036, AW21036_REG_BR0 + databuf[0], 0xFF);
		aw21036_i2c_write(aw21036, AW21036_REG_GCCR, 0x0F);

		aw21036->rgbcolor = (databuf[1] & 0x00ff0000) >> 16;
		aw21036_i2c_write(aw21036, AW21036_REG_COL0 + databuf[0] * 3, aw21036->rgbcolor);

		aw21036->rgbcolor = (databuf[1] & 0x0000ff00) >> 8;
		aw21036_i2c_write(aw21036, AW21036_REG_COL0 + databuf[0] * 3 + 1,
				  aw21036->rgbcolor);

		aw21036->rgbcolor = (databuf[1] & 0x000000ff);
		aw21036_i2c_write(aw21036, AW21036_REG_COL0 + databuf[0] * 3 + 2,
				  aw21036->rgbcolor);

		aw21036_led_update(aw21036);
	}
	return len;
}

static DEVICE_ATTR_RW(reg);
static DEVICE_ATTR_RW(hwen);
static DEVICE_ATTR_WO(sw_reset);
static DEVICE_ATTR_RW(effect);
static DEVICE_ATTR_WO(rgbcolor);


static struct attribute *aw21036_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_hwen.attr,
	&dev_attr_sw_reset.attr,
	&dev_attr_effect.attr,
	&dev_attr_rgbcolor.attr,
	NULL
};

static struct attribute_group aw21036_attribute_group = {
	.attrs = aw21036_attributes
};

/******************************************************
 *
 * i2c driver
 *
 ******************************************************/
static int aw21036_i2c_probe(struct i2c_client *i2c)
{
	struct aw21036 *aw21036;
	struct device_node *np = i2c->dev.of_node;
	int ret;

	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		dev_err(&i2c->dev, "check_functionality failed\n");
		return -ENODEV;
	}
	aw21036 = devm_kzalloc(&i2c->dev, sizeof(struct aw21036), GFP_KERNEL);
	if (aw21036 == NULL)
		return -ENOMEM;
	aw21036->dev = &i2c->dev;
	aw21036->i2c = i2c;
	i2c_set_clientdata(i2c, aw21036);
	mutex_init(&aw21036->cfg_lock);

	/* aw21036 parse device tree */
	if (np) {
		ret = aw21036_parse_dt(&i2c->dev, aw21036, np);
		if (ret) {
			dev_err(&i2c->dev, "%s: failed to parse device tree node\n", __func__);
			goto err_parse_dt;
		}
	} else {
		aw21036->reset_gpio = -1;
	}
	if (gpio_is_valid(aw21036->reset_gpio)) {
		ret = gpio_request(aw21036->reset_gpio, "aw21036_rst");
		if (ret) {
			dev_err(&i2c->dev, "%s: rst request failed\n",
				__func__);
			goto err_gpio_request;
		}
	}
	gpio_direction_output(aw21036->reset_gpio, 0);

	/* aw21036 identify */
	ret = aw21036_read_chipid(aw21036);
	if (ret < 0) {
		dev_err(&i2c->dev, "%s: aw21036_read_chipid failed ret=%d\n", __func__, ret);
		goto err_id;
	}
	ret = aw21036_read_version(aw21036);
	if (ret < 0) {
		dev_err(&i2c->dev, "%s: aw21036_read_version failed ret=%d\n", __func__, ret);
		goto err_id;
	}
	dev_set_drvdata(&i2c->dev, aw21036);
	aw21036_led_init(aw21036);
	aw21036->cdev.name = "aw21036_led";
	INIT_WORK(&aw21036->brightness_work, aw21036_brightness_work);
	INIT_WORK(&aw21036->breath_work, aw21036_breath_work);
	aw21036->cdev.brightness_set = aw21036_set_brightness;
	aw21036->cdev.blink_set = aw21036_led_blink_set;
	ret = led_classdev_register(aw21036->dev, &aw21036->cdev);
	if (ret) {
		dev_err(aw21036->dev, "unable to register led ret=%d\n", ret);
		goto err_class;
	}
	ret = sysfs_create_group(&aw21036->cdev.dev->kobj, &aw21036_attribute_group);
	if (ret) {
		dev_err(aw21036->dev, "led sysfs ret: %d\n", ret);
		goto err_sysfs;
	}
	pr_info("%s probe completed!\n", __func__);
	return 0;

err_sysfs:
	led_classdev_unregister(&aw21036->cdev);
err_class:
err_id:
	gpio_free(aw21036->reset_gpio);
err_gpio_request:
err_parse_dt:
	devm_kfree(&i2c->dev, aw21036);
	aw21036 = NULL;
	return ret;
}

static void aw21036_i2c_remove(struct i2c_client *i2c)
{
	struct aw21036 *aw21036 = i2c_get_clientdata(i2c);

	sysfs_remove_group(&aw21036->cdev.dev->kobj, &aw21036_attribute_group);
	led_classdev_unregister(&aw21036->cdev);
	if (gpio_is_valid(aw21036->reset_gpio))
		gpio_free(aw21036->reset_gpio);
	devm_kfree(&i2c->dev, aw21036);
	aw21036 = NULL;
}

static const struct i2c_device_id aw21036_i2c_id[] = {
	{AW21036_I2C_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, aw21036_i2c_id);

static const struct of_device_id aw21036_dt_match[] = {
	{.compatible = "awinic,aw21036_led"},
	{}
};

static struct i2c_driver aw21036_i2c_driver = {
	.driver = {
		.name = AW21036_I2C_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(aw21036_dt_match),
		},
	.probe = aw21036_i2c_probe,
	.remove = aw21036_i2c_remove,
	.id_table = aw21036_i2c_id,
};

static int __init aw21036_i2c_init(void)
{
	int ret = 0;

	pr_info("%s enter, aw21036 driver version %s\n", __func__, AW21036_DRIVER_VERSION);
	ret = i2c_add_driver(&aw21036_i2c_driver);
	if (ret) {
		pr_err("%s failed to register aw21036 driver!\n", __func__);
		return ret;
	}
	return 0;
}
module_init(aw21036_i2c_init);

static void __exit aw21036_i2c_exit(void)
{
	i2c_del_driver(&aw21036_i2c_driver);
}
module_exit(aw21036_i2c_exit);

MODULE_DESCRIPTION("AW21036 LED Driver");
MODULE_LICENSE("GPL v2");
