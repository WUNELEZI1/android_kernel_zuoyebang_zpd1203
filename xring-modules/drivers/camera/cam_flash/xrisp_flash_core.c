// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "flash_core", __func__, __LINE__

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
#include "xrisp_flash_common.h"
#include "xrisp_flash_core.h"
#include "cam_sysfs/xrisp_sysfs.h"
#include "xrisp_flash_soc.h"

/* debug */
#define DEBUG_SINGLE_LED        2
#define DEBUG_DOUBLE_LED        4
#define DEBUG_LED1_FLAG         1
#define DEBUG_LED2_FLAG         2
#define FLASH_WAIT_EN_TIMEOUT   3000  // ms

enum flash_type {
	PMIC_DRIVER_MODE = 0,
	I2C_DRIVER_MODE,
};

static struct xrisp_flash_core *g_flash_core;

static struct xrisp_flash_core *xrisp_get_flash_core(void)
{
	return g_flash_core;
}

/*
 * sys class debug
 */
static ssize_t show_flash_led_curr(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int led1_curr = 0;
	int led2_curr = 0;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();
	struct device *dev;

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev))
		return -EINVAL;
	dev = flash_core->dev;
	if (flash_core->ops->debug_leds_determ_led_num(dev) < 0)
		return -EINVAL;
	led1_curr = flash_core->ops->debug_leds_get_curr(dev, XRISP_FLASH_MODE, FLASH_LED1_CURR);
	if (led1_curr < 0)
		return -EINVAL;
	led2_curr = flash_core->ops->debug_leds_get_curr(dev, XRISP_FLASH_MODE, FLASH_LED2_CURR);
	if (led2_curr < 0)
		return -EINVAL;
	XRISP_PR_DEBUG("flash current: led1 = %d mA, led2 = %d mA\n", led1_curr, led2_curr);

	return sprintf(buf, "led1 %d, led2 %d", led1_curr, led2_curr);
}

static int _store_flash_led_curr(int val, enum flash_current_opration_flag current_opration_flag)
{
	int ret = 0;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();
	struct device *dev;

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev)) {
		XRISP_PR_ERROR("invalid value");
		return -ENAVAIL;
	}
	dev = flash_core->dev;
	ret = flash_core->ops->debug_leds_set_curr(dev, val, XRISP_FLASH_MODE, current_opration_flag);
	if (ret < 0)
		return -EINVAL;

	return ret;
}

static ssize_t store_flash_led_curr(struct kobject *kobj, struct kobj_attribute *attr,
				       const char *buf, size_t count)
{
	char *p_args = NULL;
	char *orig = NULL;
	char *token = NULL;
	int arry[DEBUG_DOUBLE_LED] = {0};
	int id = 0;
	int temp;
	ssize_t ret = 0;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev))
		return -EINVAL;
	dev = flash_core->dev;
	if (flash_core->ops->debug_leds_determ_led_num(dev) < 0)
		return -EINVAL;

	p_args = kstrndup(buf, count, GFP_KERNEL);
	if (!p_args)
		return -ENOMEM;

	/* Removes leading and trailing whitespace from param */
	orig = strim(p_args);

	/* Split string using space char, param format: led1 curr led2 curr*/
	while ((token = strsep(&orig, " ")) != NULL) {
		ret = kstrtoint(token, 10, &temp);
		if (ret < 0) {
			XRISP_PR_ERROR("error parsing args %zd\n", ret);
			ret = -ENAVAIL;
			goto free_m;
		}

		if (id < DEBUG_DOUBLE_LED)
			arry[id++] = temp;
		else
			break;
	}

	if (id < DEBUG_SINGLE_LED) {
		XRISP_PR_ERROR("invalide input parameter");
		ret = -ENAVAIL;
		goto free_m;
	}

	switch (id) {
	case DEBUG_SINGLE_LED: /* parameters number:  single led*/
		if (arry[0] == DEBUG_LED1_FLAG &&
		    flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED1))
			_store_flash_led_curr(arry[1], FLASH_LED1_CURR);
		else if (arry[0] == DEBUG_LED2_FLAG &&
			 flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED2))
			_store_flash_led_curr(arry[1], FLASH_LED2_CURR);

		break;

	case DEBUG_DOUBLE_LED: /* parameters number: double led */
		if (arry[0] == DEBUG_LED1_FLAG && flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED1))
			_store_flash_led_curr(arry[1], FLASH_LED1_CURR);

		if (arry[2] == DEBUG_LED2_FLAG && flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED2))
			_store_flash_led_curr(arry[3], FLASH_LED2_CURR);

		break;
	default:
		break;
	}

free_m:
	kfree(p_args);

	return ret ? ret : count;
}


static ssize_t store_flash_timeout(struct kobject *kobj, struct kobj_attribute *attr,
				       const char *buf, size_t count)
{
	int ret;
	int dura;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();
	struct device *dev;

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev))
		return -EINVAL;
	dev = flash_core->dev;
	if (flash_core->ops->debug_leds_determ_led_num(dev) < 0)
		return -EINVAL;
	ret = kstrtou32(buf, 0, &dura);
	if (ret) {
		XRISP_PR_ERROR("invalid value\n");
		return -EINVAL;
	}
	XRISP_PR_DEBUG("timeout set ops %u.\n", dura);

	ret = flash_core->ops->debug_leds_timeout(dev, dura);
	if (ret < 0) {
		XRISP_PR_ERROR("flash set timeout err\n");
		return -EINVAL;
	}
	return count;
}

static ssize_t store_torch_curr(struct kobject *kobj, struct kobj_attribute *attr,
				       const char *buf, size_t count)
{
	int ret;
	uint32_t val;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev))
		return -EINVAL;
	dev = flash_core->dev;
	if (flash_core->ops->debug_leds_determ_led_num(dev) < 0)
		return -EINVAL;

	ret = kstrtou32(buf, 10, &val);
	if (ret) {
		XRISP_PR_ERROR("invalid value\n");
		return -EINVAL;
	}

	ret = flash_core->ops->debug_leds_set_curr(dev, val, XRISP_TORCH_MODE, TORCH_LED1_CURR);
	if (ret < 0)
		return -EINVAL;
	ret = flash_core->ops->debug_leds_set_curr(dev, val, XRISP_TORCH_MODE, TORCH_LED2_CURR);
	if (ret < 0)
		return -EINVAL;

	XRISP_PR_DEBUG("torch: led_curr = %u\n", val);
	return count;
}

static int _store_flash_led_enable(const int en, enum flash_led_flag led_flag)
{
	int ret = 0;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev)) {
		XRISP_PR_ERROR("flash_core is null");
		return -EINVAL;
	}
	dev = flash_core->dev;
	ret = flash_core->ops->debug_leds_flash_enable(dev, en, led_flag);
	if (ret < 0) {
		XRISP_PR_ERROR("led-%d enable write reg fail\n", led_flag);
		return -EINVAL;
	}
	return ret;
}

static ssize_t store_flash_led_enable(struct kobject *kobj, struct kobj_attribute *attr,
				       const char *buf, size_t count)
{
	char *p_args = NULL;
	char *orig = NULL;
	char *token = NULL;
	int arry[DEBUG_DOUBLE_LED] = {0};
	int id = 0;
	int temp;
	ssize_t ret = 0;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_core) || (!flash_core->dev)) {
		XRISP_PR_ERROR("flash_core is null");
		return -EINVAL;
	}
	dev = flash_core->dev;
	if (flash_core->ops->debug_leds_determ_led_num(dev) < 0)
		return -EINVAL;

	p_args = kstrndup(buf, count, GFP_KERNEL);
	if (!p_args)
		return -ENOMEM;

	orig = strim(p_args);

	/* Split string using space char, param format: led1 curr led2 curr */
	while ((token = strsep(&orig, " ")) != NULL) {
		ret = kstrtoint(token, 10, &temp);
		if (ret < 0) {
			XRISP_PR_ERROR("error parsing args %zd\n", ret);
			ret = -ENAVAIL;
			goto free_m;
		}

		if (id < DEBUG_DOUBLE_LED)
			arry[id++] = temp;
		else
			break;
	}

	if (id < DEBUG_SINGLE_LED) {
		XRISP_PR_ERROR("invalid input parameters");
		ret = -ENAVAIL;
		goto free_m;
	}

	switch (id) {
	case DEBUG_SINGLE_LED: /* parameters number:  single led*/
		if (arry[0] == DEBUG_LED1_FLAG)
			_store_flash_led_enable(arry[1], XRISP_FLASH_LED1);
		else if (arry[0] == DEBUG_LED2_FLAG)
			_store_flash_led_enable(arry[1], XRISP_FLASH_LED2);

		break;
	case DEBUG_DOUBLE_LED: /* parameters number: double led */
		_store_flash_led_enable(arry[1], XRISP_FLASH_LED);
		break;
	default:
		break;
	}
free_m:
	kfree(p_args);

	return ret ? ret : count;
}

static ssize_t store_torch_enable(struct kobject *kobj, struct kobj_attribute *attr,
					const char *buf, size_t count)
{
	char *orig = NULL;
	char *token = NULL;
	char *p_args = NULL;
	int arry[DEBUG_DOUBLE_LED] = {0};
	int id = 0;
	int reg = 0;
	int temp;
	int val = 0;
	ssize_t ret = 0;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev)) {
		XRISP_PR_ERROR("flash_core is null");
		return -EINVAL;
	}
	dev = flash_core->dev;
	if (flash_core->ops->debug_leds_determ_led_num(dev) < 0)
		return -EINVAL;
	if (flash_core->ops->debug_leds_read(dev, &val) < 0)
		return -EINVAL;

	p_args = kstrndup(buf, count, GFP_KERNEL);
	if (!p_args)
		return -ENOMEM;

	orig = strim(p_args);

	while ((token = strsep(&orig, " ")) != NULL) {
		if (kstrtoint(token, 10, &temp) < 0) {
			XRISP_PR_ERROR("error parsing args\n");
			ret = -EINVAL;
			goto free_m;
		}

		if (id < DEBUG_DOUBLE_LED)
			arry[id++] = temp;
		else
			break;
	}

	if (id < DEBUG_SINGLE_LED) {
		XRISP_PR_ERROR("invalid input parameters");
		ret = -EINVAL;
		goto free_m;
	}

	switch (id) {
	case DEBUG_SINGLE_LED:
		if (arry[0] == DEBUG_LED1_FLAG
			&& flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED1)) {
			reg = flash_core->ops->debug_get_leds_torch_enable_num(arry[1], val, XRISP_FLASH_LED1);
		} else if (arry[0] == DEBUG_LED2_FLAG
			&& flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED2)) {
			reg = flash_core->ops->debug_get_leds_torch_enable_num(arry[1], val, XRISP_FLASH_LED2);
		}
		break;
	case DEBUG_DOUBLE_LED:
		if (arry[0] == DEBUG_LED1_FLAG
			&& flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED1)) {
			reg = flash_core->ops->debug_get_leds_torch_enable_num(arry[1], val, XRISP_FLASH_LED1);
		}
		if (arry[2] == DEBUG_LED2_FLAG
			&& flash_core->ops->debug_leds_get_use_num(dev, XRISP_FLASH_LED2)) {
			reg = flash_core->ops->debug_get_leds_torch_enable_num(arry[3], reg, XRISP_FLASH_LED2);
		}
		break;
	default:
		return -EINVAL;
	}
	if (reg < 0)
		return -EINVAL;
	if (flash_core->ops->debug_leds_torch_enable(dev, reg) < 0) {
		XRISP_PR_ERROR("torch enable fail\n");
		ret = -EINVAL;
	}

free_m:
	kfree(p_args);
	return ret ? ret : count;
}

static ssize_t store_power_up(struct kobject *kobj, struct kobj_attribute *attr,
				       const char *buf, size_t count)
{
	int power_up;
	int ret;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();
	struct device *dev;

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev))
		return -EINVAL;
	dev = flash_core->dev;

	ret = kstrtoint(buf, 10, &power_up);
	if (ret)
		return ret;

	if (power_up < 0)
		return -EINVAL;

	XRISP_PR_DEBUG("power_up %d\n", power_up);

	if (power_up > 0) {
		ret = flash_core->ops->debug_leds_power_up(dev);
		if (ret < 0) {
			XRISP_PR_ERROR("pinctrl set default state failed\n");
			return -EINVAL;
		}
	} else {
		ret = flash_core->ops->debug_leds_power_down(dev, false);
		if (ret < 0) {
			XRISP_PR_ERROR("pinctrl set sleep state failed\n");
			return -EINVAL;
		}
	}

	return ret ? ret : count;
}

static struct kobj_attribute kobj_attr_xrisp_flash_power =
	__ATTR(xrisp_flash_power, 0664, NULL, store_power_up);

static struct kobj_attribute kobj_attr_xrisp_flash_torch_en =
	__ATTR(xrisp_flash_torch_en, 0664, NULL, store_torch_enable);

static struct kobj_attribute kobj_attr_xrisp_flash_leds_en =
	__ATTR(xrisp_flash_leds_en, 0664, NULL, store_flash_led_enable);

static struct kobj_attribute kobj_attr_xrisp_flash_torch_curr =
	__ATTR(xrisp_flash_torch_curr, 0664, NULL, store_torch_curr);

static struct kobj_attribute kobj_attr_xrisp_flash_timeout =
	__ATTR(xrisp_flash_timeout, 0664, NULL, store_flash_timeout);

static struct kobj_attribute kobj_attr_xrisp_flash_leds_curr =
	__ATTR(xrisp_flash_leds_curr, 0664, show_flash_led_curr, store_flash_led_curr);

struct attribute *flash_attrs[] = {
	&kobj_attr_xrisp_flash_timeout.attr,
	&kobj_attr_xrisp_flash_leds_curr.attr,
	&kobj_attr_xrisp_flash_leds_en.attr,
	&kobj_attr_xrisp_flash_torch_curr.attr,
	&kobj_attr_xrisp_flash_torch_en.attr,
	&kobj_attr_xrisp_flash_power.attr,
	NULL,
};

static const struct attribute_group flash_attr_group = {
	.attrs = flash_attrs,
};

int flash_sys_node_init(void)
{
	int ret = 0;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (!flash_core)
		return -EINVAL;

	flash_core->isp_kobj = xrisp_get_isp_kobj();
	if (!flash_core->isp_kobj) {
		kobject_put(flash_core->isp_kobj);
		return -EINVAL;
	}

	flash_core->flash_kobj = kobject_create_and_add("xrisp_flash", flash_core->isp_kobj);
	ret = sysfs_create_group(flash_core->flash_kobj, &flash_attr_group);
	if (ret)
		goto destory_kobj;

	return ret;

destory_kobj:
	kobject_put(flash_core->flash_kobj);
	kobject_put(flash_core->isp_kobj);
	return -EINVAL;
}

void flash_sys_node_exit(void)
{
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (flash_core) {
		sysfs_remove_group(flash_core->flash_kobj, &flash_attr_group);
		kobject_put(flash_core->flash_kobj);
		kobject_put(flash_core->isp_kobj);
	}
}

/*
 * parse flash operation instructions
 */
static enum hrtimer_restart flash_hrtimer_callback(struct hrtimer *hrtimer)
{
	struct xrisp_flash_core *flash_core;

	flash_core = container_of(hrtimer, struct xrisp_flash_core, xrisp_flash_hrtimer);
	if (flash_core->flash_flag_finish)
		complete(&flash_core->flash_flag);

	return HRTIMER_NORESTART;
}

static int flash_bring_rer_to_enable(void)
{
	uint32_t cmdType;
	struct device *dev;
	int ret;
	uint32_t led1Cur;
	uint32_t led2Cur;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();
	struct xrisp_flash_bring_cmd FlashBringCmd;
	struct xrisp_flash_rercmd   FlashRERCmd;

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev) || (!flash_core->flash_connect_cmd))
		return -EINVAL;
	dev = flash_core->dev;
	cmdType = flash_core->flash_connect_cmd->header.cmdType;
	if (cmdType == FlashCmdTypeBring) {
		FlashBringCmd = flash_core->flash_connect_cmd->upayload.flash_bring;
		led1Cur = FlashBringCmd.currentMilliampere[0];
		led2Cur = FlashBringCmd.currentMilliampere[1];
	} else if (cmdType == FlashCmdTypeRER) {
		FlashRERCmd = flash_core->flash_connect_cmd->upayload.flash_rercmd;
		led1Cur = FlashRERCmd.currentMilliampere[0];
		led2Cur = FlashRERCmd.currentMilliampere[1];
	} else {
		XRISP_PR_ERROR("cmd type is err");
		return -EINVAL;
	}
	ret = flash_core->ops->leds_enable(dev, led1Cur, led2Cur);
	if (ret < 0) {
		XRISP_PR_ERROR("fail to open flash.\n");
		return -EINVAL;
	}
	flash_core->flash_flag_finish = false;

	return ret;
}

static int flash_timer(uint64_t flash_time)
{
	ktime_t kt;
	__u64 time_ns_start;
	__u64 time_ns_end;
	__u64 time_ns;
	int ret;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (!flash_core)
		return -EINVAL;

	kt = ms_to_ktime(flash_time);
	time_ns_start = xr_timestamp_gettime();
	hrtimer_start(&flash_core->xrisp_flash_hrtimer, kt,
		HRTIMER_MODE_REL_PINNED);

	ret = wait_for_completion_timeout(&flash_core->flash_flag,
			msecs_to_jiffies(FLASH_WAIT_EN_TIMEOUT));
	if (!ret) {
		XRISP_PR_ERROR("wait ack timeout, ret = %d\n", ret);
		return -ETIMEDOUT;
	}

	time_ns_end = xr_timestamp_gettime();
	time_ns = time_ns_end - time_ns_start;
	XRISP_PR_DEBUG("time_ns = %llu\n", time_ns);

	return ret;
}

static int flash_wait_time_to_corres_opration(enum flash_led_opration_flag led_opration_flag,
			uint64_t flash_time)
{
	int ret = 0;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_core) || (!flash_core->ops) || (!flash_core->dev))
		return -EINVAL;
	dev = flash_core->dev;

	if (led_opration_flag == XRISP_FLASH_DISABLE) {
		if (flash_core->ops->leds_duration_valid(flash_time) < 0) {
			XRISP_PR_ERROR("flashDuration exceeds limit\n");
			return -EINVAL;
		}
		XRISP_PR_DEBUG("flash_duration = %llu ms, hrtimer start\n", flash_time);
		ret = flash_timer(flash_time);
		if (ret < 0)
			return -EINVAL;

		ret = flash_core->ops->leds_disable(dev);
		if (ret < 0) {
			XRISP_PR_ERROR("flash disable fail\n");
			return -EINVAL;
		}
		flash_core->flash_flag_finish = false;
	} else if (led_opration_flag == XRISP_FLASH_ENABLE) {
		XRISP_PR_DEBUG("flash_waittime = %llu ms,  hrtimer start\n", flash_time);
		ret = flash_timer(flash_time);
		if (ret < 0)
			return -EINVAL;
		ret = flash_bring_rer_to_enable();
		if (ret < 0)
			return -EINVAL;
	} else {
		return -EINVAL;
	}

	return ret;
}

static int flash_precision_enable(uint64_t flashOnWaitTime, uint64_t flashDuration)
{
	int ret = 0;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (!flash_core)
		return -EINVAL;

	flash_core->flash_flag_finish = true;
	ret = flash_wait_time_to_corres_opration(XRISP_FLASH_ENABLE, flashOnWaitTime);
	if (ret < 0) {
		XRISP_PR_ERROR("set flash wait time fail\n");
		return -EINVAL;
	}
	XRISP_PR_DEBUG("flash wait time is %llu ms, wait time end\n",
			flashOnWaitTime);
	init_completion(&flash_core->flash_flag);
	flash_core->flash_flag_finish = true;
	ret = flash_wait_time_to_corres_opration(XRISP_FLASH_DISABLE, flashDuration);
	if (ret < 0) {
		XRISP_PR_ERROR("set flash duration time fail\n");
		return -EINVAL;
	}
	XRISP_PR_DEBUG("flash dura time is %llu ms, dura time end, ret = %d\n",
		flashDuration, ret);

	return ret;
}

static int xrisp_flash_timer_init(void)
{
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (!flash_core)
		return -EINVAL;

	init_completion(&flash_core->flash_flag);
	hrtimer_init(&flash_core->xrisp_flash_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	flash_core->xrisp_flash_hrtimer.function = flash_hrtimer_callback;

	return 0;
}

static int flash_bring(struct xrisp_flash_bring_cmd *bring_cmd)
{
	uint8_t  opstatus;
	int      ret = 0;
	uint32_t led1_current;
	uint32_t led2_current;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!bring_cmd) || (!flash_core) || (!flash_core->ops) || (!flash_core->dev)) {
		XRISP_PR_ERROR("flash bring cmd or flash_core is null\n");
		return -EINVAL;
	}
	dev = flash_core->dev;
	led1_current = bring_cmd->currentMilliampere[0];
	led2_current = bring_cmd->currentMilliampere[1];
	opstatus = bring_cmd->opstatus;

	if (opstatus == FlashOpstatusOff) {
		ret = flash_core->ops->leds_disable(dev);
		if (ret < 0) {
			XRISP_PR_ERROR("flash disable fail\n");
			return -EINVAL;
		}
	} else if (opstatus == FlashOpstatusBringHigh
			|| opstatus == FlashOpstatusBringLow) {
		ret = flash_core->ops->leds_enable(dev, led1_current, led2_current);
		if (ret < 0) {
			XRISP_PR_ERROR("fail to open flash.\n");
			return -EINVAL;
		}
		XRISP_PR_INFO("flash enable immediately success\n");
	} else if (opstatus == FlashOpstatusBringPrecision) {
		ret = flash_precision_enable(bring_cmd->flashOnWaitTime, bring_cmd->flashDuration);
		if (ret < 0)
			return -EINVAL;
	} else {
		XRISP_PR_ERROR("flash opstatus error\n");
		return -EINVAL;
	}

	return ret;
}

static int flash_rer(struct xrisp_flash_rercmd *flash_rer)
{
	int ret = 0;
	int i;
	uint16_t itrCount;

	if (!flash_rer) {
		XRISP_PR_ERROR("ref cmd is null\n");
		return -EINVAL;
	}

	itrCount = flash_rer->numberOfIteration;
	for (i = 0; i < itrCount; i++) {
		ret = flash_precision_enable(flash_rer->flashOnDelayMillisecond,
				flash_rer->flashOffDelayMillisecond);
		if (ret < 0)
			return -EINVAL;
	}
	return ret;
}

static int torch_bring(struct xrisp_flash_bring_cmd *bring_cmd)
{
	int      ret;
	uint32_t led1_current;
	uint32_t led2_current;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!bring_cmd) || (!flash_core) || (!flash_core->ops) || (!flash_core->dev)) {
		XRISP_PR_ERROR("torch bring cmd is null\n");
		return -EINVAL;
	}
	dev = flash_core->dev;
	led1_current = bring_cmd->currentMilliampere[0];
	led2_current = bring_cmd->currentMilliampere[1];

	ret = flash_core->ops->leds_torch_enable(dev, led1_current, led2_current);
	if (ret < 0) {
		XRISP_PR_ERROR("fail to open torch.\n");
		return -EINVAL;
	}
	XRISP_PR_INFO("torch enable success!!!\n");

	return ret;
}

int xrisp_flash_parse_command(struct xrisp_flash_connect_cmd *flash_cmd)
{
	int ret = 0;
	uint32_t cmdType;
	uint32_t flashType;
	struct xrisp_flash_bring_cmd flash_bring_cmd;
	struct xrisp_flash_querry_current_cmd *query_current;
	struct xrisp_flash_rercmd flash_rer_cmd;
	struct device *dev;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if ((!flash_cmd) || (!flash_core) || (!flash_core->ops) || (!flash_core->dev)) {
		XRISP_PR_ERROR("flash cmd error or flash core is NULL\n");
		return -EINVAL;
	}
	dev = flash_core->dev;
	memcpy(flash_core->flash_connect_cmd, flash_cmd,
		sizeof(struct xrisp_flash_connect_cmd));
	if (!(flash_core->flash_connect_cmd)) {
		XRISP_PR_ERROR("flash_core->flash_connect_cmd is null pointer\n");
		return -EINVAL;
	}

	cmdType = flash_cmd->header.cmdType;
	switch (cmdType) {
	case FlashCmdTypeInit:
		flashType = flash_cmd->upayload.flash_info.flashType;
		if (flashType == I2C_DRIVER_MODE) {
			ret = flash_core->ops->leds_init(dev);
		} else {
			XRISP_PR_ERROR("flashType is err, flashType = %d", flashType);
			ret = -EINVAL;
		}
		break;
	case FlashCmdTypeBring:
		flash_bring_cmd = flash_cmd->upayload.flash_bring;
		ret = flash_bring(&flash_bring_cmd);
		break;

	case FlashCmdTypeWidget:
		flash_bring_cmd = flash_cmd->upayload.flash_bring;
		ret = torch_bring(&flash_bring_cmd);
		break;

	case FlashCmdTypeRER:
		flash_rer_cmd = flash_cmd->upayload.flash_rercmd;
		ret = flash_rer(&flash_rer_cmd);
		break;

	case FlashCmdTypeQueryCurrent:
		query_current = &(flash_cmd->upayload.querry_current);
		ret = flash_core->ops->leds_query_current(dev, query_current);
		break;

	case FlashCmdTypeDeInit:
		ret = flash_core->ops->leds_deinit(dev);
		break;

	default:
		XRISP_PR_ERROR("invalid cmdType: %d\n", cmdType);
		return -EINVAL;
	}

	return ret;
}

int flash_core_init(struct device *dev, struct xrisp_flash_core_ops *flash_core_ops)
{
	int ret = 0;

	if (!dev || !flash_core_ops)
		return -EINVAL;

	g_flash_core = devm_kzalloc(dev,
		sizeof(struct xrisp_flash_core),
		GFP_KERNEL);
	if (!(g_flash_core)) {
		XRISP_PR_ERROR("flash_core is NULL\n");
		return -EINVAL;
	}
	g_flash_core->flash_connect_cmd = devm_kzalloc(dev,
		sizeof(struct xrisp_flash_connect_cmd),
		GFP_KERNEL);
	if (!(g_flash_core->flash_connect_cmd)) {
		XRISP_PR_ERROR("flash_core->flash_connect_cmd is NULL\n");
		return -EINVAL;
	}

	g_flash_core->ops = flash_core_ops;
	g_flash_core->dev = dev;

	ret = xrisp_flash_timer_init();
	if (ret < 0)
		goto err_free;

	return ret;

err_free:
	hrtimer_cancel(&g_flash_core->xrisp_flash_hrtimer);
	g_flash_core = NULL;
	return -EINVAL;
}

void flash_core_deinit(void)
{
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (flash_core) {
		hrtimer_cancel(&flash_core->xrisp_flash_hrtimer);
		flash_core = NULL;
	}
}

void xrisp_flash_close(void)
{
	int ret = 0;
	struct xrisp_flash_core *flash_core = xrisp_get_flash_core();

	if (flash_core) {
		hrtimer_cancel(&flash_core->xrisp_flash_hrtimer);
		if (flash_core->ops && flash_core->dev) {
			ret = flash_core->ops->debug_leds_power_down(flash_core->dev, true);
			if (ret < 0)
				XRISP_PR_ERROR("flash power down fail!\n");
		}
		flash_core = NULL;
	}
}
