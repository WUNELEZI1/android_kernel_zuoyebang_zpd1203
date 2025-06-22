// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sgm37864_flash_dev", __func__, __LINE__

#include <linux/module.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <soc/xring/xr_timestamp.h>
#include <linux/math64.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include <linux/errno.h>
#include <linux/pinctrl/consumer.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include "xrisp_log.h"
#include "xrisp_flash_core.h"
#include "xrisp_flash_soc.h"
#include "xrisp_flash_common.h"

/* registers definitions */
#define FLASH_ENABLE_ADDR                  0x01
#define LED1_FLASH_BRIGHTNESS              0x03
#define LED2_FLASH_BRIGHTNESS              0x04
#define LED1_TORCH_BRIGHTNESS              0x05
#define LED2_TORCH_BRIGHTNESS              0x06
#define TIMING_CONFIGURATION               0x08

/*
 * flash brightness
 * min 3.91 mA, step 7.83 mA, max 2000 mA
 */
#define FLASH_MAX_CURRENT                  2000  // mA
#define FLASH_MIN_CURRENT                  4     // mA
#define FLASH_CURR_STRP                    7830  // (7.83 * 1000) uA
#define SGM37864_QUERY_FLASH_MIN_CURRENT   3910  //(3.91 * 1000)uA

/*
 * torch brightness: it is real for chip
 * min 0.98 mA, step 1.96 mA, max 500 mA
 */
#define TORCH_MAX_CURRENT                  500   // mA
#define TORCH_MIN_CURRENT                  1     // mA
#define TORCH_CURR_STRP                    1960  //(1.96 * 1000)
#define SGM37864_QUERY_TORCH_MIN_CURRENT   980   //(0.98  * 1000)uA

/*
 * flash timeout duration
 * min 40 ms, step 40 ms, max 1600 ms
 * exceeding 400 ms, the internal thermal shutdown circuit may trip before
 * reaching the desired flash timeout value
 */
#define FLASH_MIN_DURATION                  40   // ms
#define FLASH_MAX_DURATION                  1600 // ms
#define FLASH_TIMEOUT_SAFE_MAX_DURATION_REG 9    // 400ms
#define FLASH_TIMEOUT_MIN_DURATION_REG      0    // 40ms
#define FLASH_TIMEOUT_MAX_DURATION_REG     15    //1600ms

#define MICROAMPS_TO_MILLIAMPS             1000
#define MILLIAMPS_TO_MICROAMPS             1000

/* valid reg val */
#define FLASH_REG_MIN_VALID    0
#define FLASH_REG_MAN_VALID    255

#define LED1_ENABLE    BIT(0)
#define LED2_ENABLE    BIT(1)
#define MODE_MASK      GENMASK(3, 2)
#define TORCH_MODE     2
#define FLASH_MODE     3

static struct regmap_config xrisp_i2c_regmap_cfg = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0xFF,
};

static int flash_reg_read(struct device *dev, int addr, int *val)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	if ((!val) || (!dev)) {
		XRISP_PR_ERROR("flash_data or val is null\n");
		return -EINVAL;
	}
	flash_data = dev_get_drvdata(dev);
	if (!flash_data) {
		XRISP_PR_ERROR("flash_data is null\n");
		return -EINVAL;
	}
	ret = regmap_read(flash_data->regm, addr, val);
	if (ret < 0) {
		XRISP_PR_ERROR("read reg fail\n");
		return -EINVAL;
	}

	return ret;
}

static int flash_reg_write(struct device *dev, int addr, int val)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	if (!dev)
		return -EINVAL;
	flash_data = dev_get_drvdata(dev);
	if (!flash_data) {
		XRISP_PR_ERROR("flash_data is null\n");
		return -EINVAL;
	}
	ret = regmap_write(flash_data->regm, addr, val);
	if (ret < 0) {
		XRISP_PR_ERROR("write reg fail\n");
		return -EINVAL;
	}

	return ret;
}

static int xrisp_flash_power_up(struct device *dev)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	if (!dev)
		return -EINVAL;

	flash_data = dev_get_drvdata(dev);
	if (!flash_data)
		return -EINVAL;
	if (atomic_inc_return(&flash_data->power_cnt) == 1) {
		ret = xrisp_pinctrl_state_set(flash_data->pinctrl, "default");
		if (ret < 0) {
			XRISP_PR_ERROR("pinctrl set default state failed\n");
			return -EINVAL;
		}
		XRISP_PR_INFO("power up success");
	} else {
		XRISP_PR_INFO("flash already power up, power_cnt = %d", atomic_read(&flash_data->power_cnt));
	}

	return ret;
}

static int xrisp_flash_power_down(struct device *dev, bool force_power_down_flag)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	if (!dev)
		return -EINVAL;

	flash_data = dev_get_drvdata(dev);
	if (!flash_data)
		return -EINVAL;

	if (atomic_dec_return(&flash_data->power_cnt) == 0 || force_power_down_flag) {
		ret = xrisp_pinctrl_state_set(flash_data->pinctrl, "sleep");
		if (ret < 0) {
			XRISP_PR_ERROR("pinctrl set sleep state failed\n");
			return -EINVAL;
		}
		XRISP_PR_INFO("power down success");
		if (force_power_down_flag)
			atomic_set(&flash_data->power_cnt, 0);
	} else {
		XRISP_PR_INFO("There's still business to be done with the flash, power_cnt = %d",
			atomic_read(&flash_data->power_cnt));
	}

	if (atomic_read(&flash_data->power_cnt) < 0)
		atomic_set(&flash_data->power_cnt, 0);

	return ret;
}

static int flash_get_enable_reg(struct device *dev, int *val)
{
	int ret = 0;

	if ((!dev) || (!val))
		return -EINVAL;
	ret = flash_reg_read(dev, FLASH_ENABLE_ADDR, val);
	if (ret < 0)
		return -EINVAL;

	return ret;
}

static int xrisp_get_use_led_num(struct device *dev, enum flash_led_flag led_num)
{
	struct xrisp_flash_data *flash_data;

	if (!dev)
		return -EINVAL;

	flash_data = dev_get_drvdata(dev);
	if (!flash_data)
		return -EINVAL;

	if (led_num == XRISP_FLASH_LED1)
		return flash_data->flash_dev_id[0];
	else if (led_num == XRISP_FLASH_LED2)
		return flash_data->flash_dev_id[1];
	else
		return -EINVAL;
}

static int flash_reg_valid(int reg_val)
{
	if ((reg_val < FLASH_REG_MIN_VALID) || (reg_val > FLASH_REG_MAN_VALID))
		return -EINVAL;
	return 0;
}

static int flash_curr_valid(uint32_t led_current)
{
	if (led_current > FLASH_MAX_CURRENT
		|| led_current < FLASH_MIN_CURRENT) {
		XRISP_PR_ERROR("flash led current exceeds limit, led_current = %d\n",
			led_current);
		return -EINVAL;
	}
	return 0;
}

static int cal_curr_fla(uint32_t led_current)
{
	int led_reg_val = 0;

	if (flash_curr_valid(led_current) < 0)
		return -EINVAL;

	led_current *= MILLIAMPS_TO_MICROAMPS;
	led_reg_val = (int)((led_current - SGM37864_QUERY_FLASH_MIN_CURRENT) / FLASH_CURR_STRP);
	if (flash_reg_valid(led_reg_val) < 0)
		return -EINVAL;

	return led_reg_val;
}

static int torch_curr_valid(uint32_t led_current)
{
	if (led_current > TORCH_MAX_CURRENT
		|| led_current < TORCH_MIN_CURRENT) {
		XRISP_PR_ERROR("torch led current exceeds limit, ledCur = %d\n",
			led_current);
		return -EINVAL;
	}
	return 0;
}

static int cal_curr_torch(uint32_t led_current)
{
	int led_reg_val = 0;

	if (torch_curr_valid(led_current) < 0)
		return -EINVAL;
	led_current *= MILLIAMPS_TO_MICROAMPS;
	led_reg_val = (int)((led_current - SGM37864_QUERY_TORCH_MIN_CURRENT) / TORCH_CURR_STRP);
	if (flash_reg_valid(led_reg_val) < 0)
		return -EINVAL;

	return led_reg_val;
}

static int flash_timeout_set(struct device *dev, int dura)
{
	int ret = 0;

	if (!dev)
		return -EINVAL;

	if (dura > FLASH_TIMEOUT_MAX_DURATION_REG || dura < FLASH_TIMEOUT_MIN_DURATION_REG) {
		XRISP_PR_ERROR("timeout err, durq = %d", dura);
		return -EINVAL;
	}
	ret = flash_reg_write(dev, TIMING_CONFIGURATION, dura);
	if (ret < 0) {
		XRISP_PR_ERROR("set timeout write reg fail\n");
		return -EINVAL;
	}
	XRISP_PR_DEBUG("set timeout %d ms success\n", ((dura + 1) * 40));

	return ret;
}

static int flash_disable(struct device *dev)
{
	int val = 0;
	int ret = 0;

	if (!dev)
		return -EINVAL;

	ret = flash_reg_write(dev, FLASH_ENABLE_ADDR, val);
	if (ret < 0) {
		XRISP_PR_ERROR("disable write reg fail\n");
		return -EINVAL;
	}
	XRISP_PR_INFO("flash disabale\n");

	return ret;
}

static int xrisp_determ_led_num(struct device *dev)
{
	if (!dev) {
		XRISP_PR_ERROR("dev is null\n");
		return -EINVAL;
	}

	if ((!xrisp_get_use_led_num(dev, XRISP_FLASH_LED1))
		&& (!xrisp_get_use_led_num(dev, XRISP_FLASH_LED2))) {
		XRISP_PR_ERROR("no lights available\n");
		return -EINVAL;
	}

	return 0;
}

static int flash_enable(struct device *dev, uint32_t led1Cur, uint32_t led2Cur)
{
	int ret = 0;
	int led1_val;
	int led2_val;
	int val = 0;
	bool led1_flag = false;
	bool led2_flag = false;

	if (!dev)
		return -EINVAL;
	if (xrisp_determ_led_num(dev) < 0)
		return -EINVAL;
	ret = flash_get_enable_reg(dev, &val);
	if (ret < 0)
		return -EINVAL;

	if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED1)) {
		led1_val = cal_curr_fla(led1Cur);
		if (led1_val < 0)
			return -EINVAL;
		ret = flash_reg_write(dev, LED1_FLASH_BRIGHTNESS, led1_val);
		if (ret < 0) {
			XRISP_PR_ERROR("flash led1 current write reg fail\n");
			return -EINVAL;
		}
		led1_flag = true;
		XRISP_PR_DEBUG("flash: led1Cur = %d mA\n", led1Cur);
	}

	if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED2)) {
		led2_val = cal_curr_fla(led2Cur);
		if (led2_val < 0)
			return -EINVAL;
		ret = flash_reg_write(dev, LED2_FLASH_BRIGHTNESS, led2_val);
		if (ret < 0) {
			XRISP_PR_ERROR("flash led2 current write reg fail\n");
			return -EINVAL;
		}
		led2_flag = true;
		XRISP_PR_DEBUG("flash: led2Cur = %d mA\n", led2Cur);
	}

	ret = flash_timeout_set(dev, FLASH_TIMEOUT_SAFE_MAX_DURATION_REG);
	if (ret < 0) {
		XRISP_PR_ERROR("flash set timeout err\n");
		return -EINVAL;
	}

	if (led1_flag)
		val |= FIELD_PREP(LED1_ENABLE, 1);
	if (led2_flag)
		val |= FIELD_PREP(LED2_ENABLE, 1);
	val |= FIELD_PREP(MODE_MASK, FLASH_MODE);
	XRISP_PR_DEBUG("flash enable reg val is %d\n", val);

	ret = flash_reg_write(dev, FLASH_ENABLE_ADDR, val);
	if (ret < 0) {
		XRISP_PR_ERROR("led enable write reg fail\n");
		return -EINVAL;
	}

	return ret;
}

static int torch_enable(struct device *dev, uint32_t led1Cur, uint32_t led2Cur)
{
	int ret = 0;
	int led1_val;
	int led2_val;
	int val = 0;
	bool led1_flag = false;
	bool led2_flag = false;

	if (!dev)
		return -EINVAL;
	if (xrisp_determ_led_num(dev) < 0)
		return -EINVAL;
	ret = flash_get_enable_reg(dev, &val);
	if (ret < 0)
		return -EINVAL;

	if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED1)) {
		led1_val = cal_curr_torch(led1Cur);
		if (led1_val < 0)
			return -EINVAL;
		ret = flash_reg_write(dev, LED1_TORCH_BRIGHTNESS, led1_val);
		if (ret < 0) {
			XRISP_PR_ERROR("torch led1 current write reg fail\n");
			return -EINVAL;
		}
		led1_flag = true;
		XRISP_PR_DEBUG("torch: led1Cur = %d mA\n", led1Cur);
	}

	if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED2)) {
		led2_val = cal_curr_torch(led2Cur);
		if (led2_val < 0)
			return -EINVAL;
		ret = flash_reg_write(dev, LED2_TORCH_BRIGHTNESS, led2_val);
		if (ret < 0) {
			XRISP_PR_ERROR("torch led2 current write reg fail\n");
			return -EINVAL;
		}
		led2_flag = true;
		XRISP_PR_DEBUG("torch: led2Cur = %d mA\n", led2Cur);
	}

	if (led1_flag)
		val |= FIELD_PREP(LED1_ENABLE, 1);
	if (led2_flag)
		val |= FIELD_PREP(LED2_ENABLE, 1);
	val |= FIELD_PREP(MODE_MASK, TORCH_MODE);
	XRISP_PR_DEBUG("torch enable reg val = %d\n", val);

	ret = flash_reg_write(dev, FLASH_ENABLE_ADDR, val);
	if (ret < 0) {
		XRISP_PR_ERROR("led enable write reg fail\n");
		return -EINVAL;
	}

	return ret;
}

static int flash_init(struct device *dev)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	if (!dev)
		return -EINVAL;
	flash_data = dev_get_drvdata(dev);
	if (!flash_data)
		return -EINVAL;

	ret = xrisp_flash_power_up(flash_data->dev);
	if (ret < 0) {
		XRISP_PR_ERROR("pinctrl set default state failed\n");
		return -EINVAL;
	}

	return ret;
}

static int flash_deinit(struct device *dev)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	if (!dev)
		return -EINVAL;
	flash_data = dev_get_drvdata(dev);
	if (!flash_data)
		return -EINVAL;

	ret = xrisp_flash_power_down(flash_data->dev, false);
	if (ret < 0) {
		XRISP_PR_ERROR("pinctrl set sleep state failed\n");
		return -EINVAL;
	}

	return ret;
}

static int flash_regval_to_curr(bool flash_flag, int reg_val)
{
	if (flash_reg_valid(reg_val) < 0)
		return -EINVAL;

	if (flash_flag)
		return ((reg_val * FLASH_CURR_STRP + SGM37864_QUERY_FLASH_MIN_CURRENT)
			/ MICROAMPS_TO_MILLIAMPS);
	else
		return ((reg_val * TORCH_CURR_STRP + SGM37864_QUERY_TORCH_MIN_CURRENT))
			/ MICROAMPS_TO_MILLIAMPS;
}

static int flash_query_current(struct device *dev,
			struct xrisp_flash_querry_current_cmd *query_current)
{
	int ret = 0;
	int val = 0;

	if (!dev)
		return -EINVAL;

	ret = flash_reg_read(dev, LED1_FLASH_BRIGHTNESS, &val);
	if (ret < 0)
		return -EINVAL;
	query_current->currentMilliampere[0] = flash_regval_to_curr(true, val);
	if (flash_curr_valid(query_current->currentMilliampere[0]) < 0) {
		XRISP_PR_ERROR("query led1 curr is invalid\n");
		return -EINVAL;
	}
	XRISP_PR_INFO("flash led1 current query is %d\n",
		query_current->currentMilliampere[0]);
	ret = flash_reg_read(dev, LED2_FLASH_BRIGHTNESS, &val);
	if (ret < 0)
		return -EINVAL;
	query_current->currentMilliampere[1] = flash_regval_to_curr(true, val);
	if (flash_curr_valid(query_current->currentMilliampere[1]) < 0) {
		XRISP_PR_ERROR("query led2 curr is invalid\n");
		return -EINVAL;
	}
	XRISP_PR_INFO("flash led2 current query is %d\n",
		query_current->currentMilliampere[1]);

	return ret;
}

static int xrisp_flash_pinctrl_init(struct xrisp_flash_data *flash_data)
{
	int ret = 0;

	if (!flash_data)
		return -EINVAL;

	flash_data->pinctrl = devm_pinctrl_get(flash_data->dev);
	if (IS_ERR(flash_data->pinctrl))
		return -EINVAL;

	ret = xrisp_pinctrl_state_set(flash_data->pinctrl, "sleep");
	if (ret < 0) {
		XRISP_PR_ERROR("pinctrl set sleep state failed\n");
		return -EINVAL;
	}

	return ret;
}

static int flash_duration_valid(int flash_time)
{
	if (flash_time > FLASH_MAX_DURATION
		|| flash_time < FLASH_MIN_DURATION) {
		XRISP_PR_ERROR("flashDuration exceeds limit\n");
		return -EINVAL;
	}
	return 0;
}

#define FLASH_SAFE_MAX_DURATION 400   // ms

static int flash_dura_to_regval(int duration)
{
	int val;

	/* Here, limited the max timeout values for protecting led */
	if (duration <= FLASH_SAFE_MAX_DURATION) {
		if (!duration)
			val = FLASH_TIMEOUT_MIN_DURATION_REG;
		else
			val = (duration - 1) / FLASH_MIN_DURATION;
	} else {
		val = FLASH_TIMEOUT_SAFE_MAX_DURATION_REG;
	}

	return val;
}

static int debug_led_flash_enable(struct device *dev, const int en, enum flash_led_flag led_flag)
{
	int ret = 0;
	int val = 0;

	if (!dev)
		return -EINVAL;

	ret = flash_get_enable_reg(dev, &val);
	if (ret < 0)
		return -EINVAL;

	if (en > 0) {
		switch (led_flag) {
		case XRISP_FLASH_LED1:
			val |= FIELD_PREP(LED1_ENABLE, 1);
			val &= ~LED2_ENABLE;
			break;
		case XRISP_FLASH_LED2:
			val |= FIELD_PREP(LED2_ENABLE, 1);
			val &= ~LED1_ENABLE;
			break;
		case XRISP_FLASH_LED:
			val |= FIELD_PREP(LED1_ENABLE, 1);
			val |= FIELD_PREP(LED2_ENABLE, 1);
			break;
		default:
			break;
		}
		val |= FIELD_PREP(MODE_MASK, FLASH_MODE);
	} else {
		val &= ~LED1_ENABLE;
		val &= ~LED2_ENABLE;
		val |= FIELD_PREP(MODE_MASK, 0);
	}

	ret = flash_reg_write(dev, FLASH_ENABLE_ADDR, val);
	if (ret < 0)
		return -EINVAL;

	return ret;
}

static int debug_get_torch_enable_num(int en, int val, enum flash_led_flag led_flag)
{
	if (en > 0) {
		switch (led_flag) {
		case XRISP_FLASH_LED1:
			val |= LED1_ENABLE;
			break;
		case XRISP_FLASH_LED2:
			val |= LED2_ENABLE;
			break;
		default:
			return -EINVAL;
		}
	} else if (en == 0) {
		switch (led_flag) {
		case XRISP_FLASH_LED1:
			val &= ~LED1_ENABLE;
			break;
		case XRISP_FLASH_LED2:
			val &= ~LED2_ENABLE;
			break;
		default:
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}
	if (flash_reg_valid(val) < 0)
		return -EINVAL;

	return val;
}

static int debug_torch_enable(struct device *dev, int val)
{
	int ret = 0;

	if (!dev)
		return -EINVAL;
	val |= FIELD_PREP(MODE_MASK, TORCH_MODE);
	ret = flash_reg_write(dev, FLASH_ENABLE_ADDR, val);
	if (ret < 0)
		return -EINVAL;

	return ret;
}

static int flash_get_curr(struct device *dev, enum flash_mode_flag mode_flag, enum flash_current_opration_flag current_opration_flag)
{
	int ret = 0;
	int led_reg = 0;
	int curr = 0;

	if (!dev)
		return -EINVAL;

	switch (current_opration_flag) {
	case FLASH_LED1_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED1))
			ret = flash_reg_read(dev, LED1_FLASH_BRIGHTNESS, &led_reg);
		break;
	case FLASH_LED2_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED2))
			ret = flash_reg_read(dev, LED2_FLASH_BRIGHTNESS, &led_reg);
		break;
	case TORCH_LED1_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED1))
			ret = flash_reg_read(dev, LED1_TORCH_BRIGHTNESS, &led_reg);
		break;
	case TORCH_LED2_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED2))
			ret = flash_reg_read(dev, LED2_TORCH_BRIGHTNESS, &led_reg);
		break;
	default:
		return -EINVAL;
	}
	if (ret < 0 || flash_reg_valid(led_reg) < 0) {
		XRISP_PR_ERROR("flash led1 read reg fail\n");
		return ret;
	}

	if (mode_flag == XRISP_FLASH_MODE)
		curr = flash_regval_to_curr(true, led_reg);
	else if (mode_flag == XRISP_TORCH_MODE)
		curr = flash_regval_to_curr(false, led_reg);
	else
		XRISP_PR_ERROR("flash mode err\n");

	return curr;
}

static int flash_set_curr(struct device *dev, int curr, enum flash_mode_flag mode_flag,
		enum flash_current_opration_flag current_opration_flag)
{
	int curr_reg = 0;
	int ret = 0;

	if (!dev)
		return -EINVAL;

	if (mode_flag == XRISP_FLASH_MODE) {
		curr_reg = cal_curr_fla(curr);
		if (curr_reg < 0)
			return -EINVAL;
	} else if (mode_flag == XRISP_TORCH_MODE) {
		curr_reg = cal_curr_torch(curr);
		if (curr_reg < 0)
			return -EINVAL;
	} else {
		return -EINVAL;
	}

	switch (current_opration_flag) {
	case FLASH_LED1_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED1))
			ret = flash_reg_write(dev, LED1_FLASH_BRIGHTNESS, curr_reg);
		break;
	case FLASH_LED2_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED2))
			ret = flash_reg_write(dev, LED2_FLASH_BRIGHTNESS, curr_reg);
		break;
	case TORCH_LED1_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED1))
			ret = flash_reg_write(dev, LED1_TORCH_BRIGHTNESS, curr_reg);
		break;
	case TORCH_LED2_CURR:
		if (xrisp_get_use_led_num(dev, XRISP_FLASH_LED2))
			ret = flash_reg_write(dev, LED2_TORCH_BRIGHTNESS, curr_reg);
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

static int flash_timeout(struct device *dev, int dura)
{
	int timeout_reg = 0;
	int ret = 0;

	if (!dev)
		return -EINVAL;
	timeout_reg = flash_dura_to_regval(dura);
	ret = flash_timeout_set(dev, timeout_reg);
	if (ret < 0)
		return -EINVAL;
	return ret;
}

static struct xrisp_flash_core_ops flash_ops = {
	.leds_init = flash_init,
	.leds_query_current = flash_query_current,
	.leds_deinit = flash_deinit,
	.leds_enable = flash_enable,
	.leds_disable = flash_disable,
	.leds_torch_enable = torch_enable,
	.leds_duration_valid = flash_duration_valid,
	.debug_leds_read = flash_get_enable_reg,
	.debug_leds_determ_led_num = xrisp_determ_led_num,
	.debug_leds_power_up = xrisp_flash_power_up,
	.debug_leds_power_down = xrisp_flash_power_down,
	.debug_leds_get_use_num = xrisp_get_use_led_num,
	.debug_leds_flash_enable = debug_led_flash_enable,
	.debug_get_leds_torch_enable_num = debug_get_torch_enable_num,
	.debug_leds_torch_enable = debug_torch_enable,
	.debug_leds_get_curr = flash_get_curr,
	.debug_leds_set_curr = flash_set_curr,
	.debug_leds_timeout = flash_timeout,
};

static void cam_flash_i2c_driver_remove(struct i2c_client *client)
{
	struct xrisp_flash_data *flash_data = i2c_get_clientdata(client);

	if (flash_data) {
		flash_sys_node_exit();
		flash_core_deinit();
		flash_data = NULL;
	}
}

static int cam_flash_i2c_driver_probe(struct i2c_client *client)
{
	int ret = 0;
	struct xrisp_flash_data *flash_data;

	XRISP_PR_INFO("flash probe enter\n");
	flash_data = devm_kzalloc(&client->dev,
		sizeof(struct xrisp_flash_data),
		GFP_KERNEL);
	if (!flash_data)
		return -EINVAL;

	flash_data->regm = devm_regmap_init_i2c(client, &xrisp_i2c_regmap_cfg);
	if (!(flash_data->regm)) {
		XRISP_PR_ERROR("regis i2c regmap fail\n");
		return -EINVAL;
	}

	flash_data->dev = &client->dev;
	flash_data->client = client;

	/*init pinctrl*/
	ret = xrisp_flash_pinctrl_init(flash_data);
	if (ret < 0) {
		XRISP_PR_ERROR("probe: pinctrl set sleep state failed\n");
		return -EINVAL;
	}
	atomic_set(&flash_data->power_cnt, 0);

	memset(flash_data->flash_dev_id, 0, sizeof(flash_data->flash_dev_id));
	ret = xrisp_flash_parse_dts(flash_data);
	if (ret < 0) {
		XRISP_PR_ERROR("leds init failed\n");
		return -EINVAL;
	}

	ret = flash_core_init(flash_data->dev, &flash_ops);
	if (ret < 0) {
		XRISP_PR_ERROR("sys class create fail\n");
		goto err_free;
	}

	ret = flash_sys_node_init();
	if (ret < 0) {
		XRISP_PR_ERROR("sys class create fail\n");
		flash_sys_node_exit();
	}

	i2c_set_clientdata(client, flash_data);
	XRISP_PR_INFO("flash probe finish\n");
	return ret;

err_free:
	flash_core_deinit();
	flash_data = NULL;
	return -EINVAL;
}

static const struct i2c_device_id flash_id_table[] = {
	{"xring,cam-i2c-flash", 0},
	{ } /* NULL terminated */
};
MODULE_DEVICE_TABLE(i2c, flash_id_table);

static const struct of_device_id xrisp_i2c_flash_dt_match[] = {
	{ .compatible = "xring,cam-i2c-flash", },
	{ },
};
MODULE_DEVICE_TABLE(of, xrisp_i2c_flash_dt_match);

static struct i2c_driver cam_flash_i2c_driver = {
	.id_table = flash_id_table,
	.probe  = cam_flash_i2c_driver_probe,
	.remove = cam_flash_i2c_driver_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "xring,cam-i2c-flash",
		.of_match_table = xrisp_i2c_flash_dt_match,
	},
};

int xrisp_flash_init(void)
{
	int rc = 0;

	rc = i2c_add_driver(&cam_flash_i2c_driver);
	if (rc < 0)
		XRISP_PR_ERROR("flash: i2c_add_driver failed rc: %d", rc);
	return rc;
}

void xrisp_flash_exit(void)
{
	i2c_del_driver(&cam_flash_i2c_driver);
}

MODULE_AUTHOR("lailin <lailin@xiaomi.com>");
MODULE_DESCRIPTION("xrisp flash");
MODULE_LICENSE("GPL v2");
