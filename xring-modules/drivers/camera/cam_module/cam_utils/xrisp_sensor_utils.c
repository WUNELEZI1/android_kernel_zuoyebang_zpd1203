// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sensor_utils", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sensor_utils", __func__, __LINE__

#include <linux/device.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/pinctrl/consumer.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include "xrisp_sensor_utils.h"
#include "xrisp_log.h"

extern uint32_t get_hw_id_value(void);

void xtisp_sensor_util_msleep(uint32_t ms)
{
	if (ms > 20)
		msleep(ms);
	else if (ms)
		usleep_range(ms * 1000,
			(ms * 1000) + 1000);
}

int xrisp_sensor_util_get_dt_regulator_info(struct device *dev, struct cam_hw_info *hw_info)
{
	struct device_node  *of_node;
	int i, count;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}

	if (of_property_read_bool(of_node, "rgltr-nosupport")) {
		XRISP_PR_INFO("No support regulator control");
		hw_info->rgltr_ctrl_support |= RGLTR_BYPASS;
		return 0;
	}
	count = of_property_count_strings(of_node, "volt-names");
	if (count <= 0) {
		if (count == (-EINVAL)) {
			XRISP_PR_ERROR("no support rgltr");
			hw_info->num_rgltr = 0;
			return -EINVAL;
		}
		XRISP_PR_ERROR("read volt-names failed.");
		count = 0;
		return -EINVAL;
	}
	hw_info->num_rgltr = count;
	for (i = 0; i < count; i++) {
		if (of_property_read_string_index(of_node,
			"volt-names", i,
			&hw_info->rgltr_nd_name[i])) {
			XRISP_PR_ERROR("no rgltr_nd_name at cnt=%d", i);
			return -ENODEV;
		}
		if (of_property_read_string(of_node,
			hw_info->rgltr_nd_name[i],
			&hw_info->rgltr_name[i])) {
			XRISP_PR_ERROR("no rgltr_name at cnt=%d", i);
			return -ENODEV;
		}
	}

	if (of_property_read_u32_array(of_node, "rgltr-min-voltage",
		hw_info->rgltr_min_volt,
		hw_info->num_rgltr)) {
		XRISP_PR_ERROR("No minimum volatage value found.");
		return -EINVAL;
	}
	if (of_property_read_u32_array(of_node, "rgltr-max-voltage",
		hw_info->rgltr_max_volt,
		hw_info->num_rgltr)) {
		XRISP_PR_ERROR("No maximum volatage value found");
		return -EINVAL;
	}
	if (of_property_read_u32_array(of_node, "rgltr-load-current",
		hw_info->rgltr_op_mode,
		hw_info->num_rgltr)) {
		XRISP_PR_ERROR("No Load curent found");
		return -EINVAL;
	}
	if (!of_property_read_bool(of_node, "rgltr-cntrl-support")) {
		XRISP_PR_INFO("rgltr volt or current cntrl no support");
		hw_info->rgltr_ctrl_support |= RGLTR_NO_CTRL;
	} else
		hw_info->rgltr_ctrl_support |= RGLTR_CTRL;
	if (!of_property_read_bool(of_node, "is-aoc-cam"))
		hw_info->is_aoc_cam = false;
	else {
		hw_info->is_aoc_cam = true;
		XRISP_PR_INFO("this is aoc_cam");
	}
	if (!of_property_read_bool(of_node, "has-aoc-cam"))
		hw_info->has_aoc = false;
	else {
		hw_info->has_aoc = true;
		XRISP_PR_INFO("this platform has aoc_cam");
	}
	return 0;
}

static int xrisp_sensor_util_get_level_from_name(const char *name, enum cam_vote_level *level)
{
	if (XRISP_CHECK_NULL_RETURN_INT(level)) {
		XRISP_PR_ERROR("level is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(name)) {
		XRISP_PR_ERROR("name is null");
		return -ENOMEM;
	}

	if (!strcmp(name, "suspend")) {
		*level = CAM_SUSPEND_VOTE;
	} else if (!strcmp(name, "low")) {
		*level = CAM_LOW_VOTE;
	} else if (!strcmp(name, "normal")) {
		*level = CAM_NORMAL_VOTE;
	} else if (!strcmp(name, "turbo")) {
		*level = CAM_TURBO_VOTE;
	} else {
		XRISP_PR_ERROR("Invalid level name:%s", name);
		return -EINVAL;
	}

	return 0;
}

int xrisp_sensor_util_get_dt_clk_info(struct device *dev, struct cam_hw_info *hw_info)
{
	int ret;
	struct device_node    *of_node;
	int i, j, count;
	int num_clk_rates, num_clk_levels;
	int num_clk_level_strings;
	const char *clk_cntl_lvl_string;
	enum cam_vote_level level;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}

	count = of_property_count_strings(of_node, "clock-names");
	if (count < 0) {
		if (count == (-EINVAL)) {
			XRISP_PR_INFO("no clock-names found.");
			hw_info->num_clk = 0;
			return 0;
		}
		XRISP_PR_ERROR("read clock-names failed, ret:%d.", count);
		return count;
	}

	hw_info->num_clk = count;
	for (i = 0; i < count; i++) {
		if (of_property_read_string_index(of_node,
			"clock-names", i,
			&hw_info->clk_name[i])) {
			XRISP_PR_ERROR("no clock_name at cnt=%d", i);
			return -ENODEV;
		}
	}

	num_clk_rates = of_property_count_u32_elems(of_node, "clock-rates");
	if (num_clk_rates <= 0) {
		XRISP_PR_ERROR("reading clock-rates count failed");
	return -EINVAL;
	}

	if ((num_clk_rates % hw_info->num_clk) != 0) {
		XRISP_PR_ERROR("mismatch clk/rates, number of clocks=%d, number of rates=%d",
			hw_info->num_clk, num_clk_rates);
		return -EINVAL;
	}

	num_clk_levels = (num_clk_rates / hw_info->num_clk);
	num_clk_level_strings = of_property_count_strings(of_node, "clock-cntl-level");
	if (num_clk_level_strings != num_clk_levels) {
		XRISP_PR_ERROR("Mismatch number of levels=%d, number of level string=%d",
			num_clk_levels, num_clk_level_strings);
		return -EINVAL;
	}
	for (i = 0; i < num_clk_levels; i++) {
		ret = of_property_read_string_index(of_node,
			"clock-cntl-level", i, &clk_cntl_lvl_string);
		if (ret) {
			XRISP_PR_ERROR("reading clock-cntl-level failed, ret:%d", ret);
			return ret;
		}

		ret = xrisp_sensor_util_get_level_from_name(clk_cntl_lvl_string, &level);
		if (ret) {
			XRISP_PR_ERROR("get clock level by string failed.");
			return ret;
		}

		hw_info->clk_level_valid[level] = true;
		for (j = 0; j < hw_info->num_clk; j++) {
			ret = of_property_read_u32_index(of_node, "clock-rates",
				((i * hw_info->num_clk) + j),
				&hw_info->clk_rate[level][j]);
			if (ret) {
				XRISP_PR_ERROR("reading clock-rates failed, ret:%d", ret);
				return ret;
			}

			//XRISP_PR_INFO("hw_info->clk_rate[%d][%d]:%d", level, j,
			//hw_info->clk_rate[level][j]);
		}
	}

	return 0;
}

static int xrisp_sensor_util_get_dt_gpio_req_tbl(struct device *dev,
	struct cam_gpio_info *gpio_info, uint16_t *gpio_array,
	uint16_t gpio_array_size)
{
	int32_t ret = 0, i = 0;
	uint32_t count = 0;
	uint32_t *val_array = NULL;
	struct device_node *of_node;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_info)) {
		XRISP_PR_ERROR("gpio_info is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(gpio_array)) {
		XRISP_PR_ERROR("gpio_array is null");
		return -ENOMEM;
	}

	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}
	if (!of_get_property(of_node, "gpio-req-tab-idx", &count))
		return 0;

	count /= sizeof(uint32_t);
	if (!count) {
		XRISP_PR_ERROR("gpio-req-tab-idx 0");
		return 0;
	}

	val_array = kcalloc(count, sizeof(uint32_t), GFP_KERNEL);
	if (!val_array)
		return -ENOMEM;

	gpio_info->gpio_req_tbl = devm_kcalloc(dev, count, sizeof(struct gpio),
		GFP_KERNEL);
	if (!gpio_info->gpio_req_tbl) {
		ret = -ENOMEM;
		goto free_val_array;
	}
	gpio_info->gpio_req_tbl_size = count;

	ret = of_property_read_u32_array(of_node, "gpio-req-tab-idx",
		val_array, count);
	if (ret) {
		XRISP_PR_ERROR("get gpio-req-tab-idx failed, ret:%d", ret);
		goto free_gpio_req_tbl;
	}

	for (i = 0; i < count; i++) {
		if (val_array[i] >= gpio_array_size) {
			XRISP_PR_ERROR("gpio req table index %d invalid", val_array[i]);
			goto free_gpio_req_tbl;
		}
		gpio_info->gpio_req_tbl[i].gpio = gpio_array[val_array[i]];
	}

	ret = of_property_read_u32_array(of_node, "gpio-req-tab-flags", val_array, count);
	if (ret) {
		XRISP_PR_ERROR("get gpio-req-tab-flags failed ret:%d", ret);
		goto free_gpio_req_tbl;
	}

	for (i = 0; i < count; i++)
		gpio_info->gpio_req_tbl[i].flags = val_array[i];


	for (i = 0; i < count; i++) {
		ret = of_property_read_string_index(of_node,
			"gpio-req-tab-label", i,
			&gpio_info->gpio_req_tbl[i].label);
		if (ret) {
			XRISP_PR_ERROR("get gpio-req-tab-label failed ret:%d", ret);
			goto free_gpio_req_tbl;
		}
	}

	kfree(val_array);

	return ret;

free_gpio_req_tbl:
	devm_kfree(dev, gpio_info->gpio_req_tbl);
free_val_array:
	kfree(val_array);
	gpio_info->gpio_req_tbl_size = 0;

	return ret;
}

int xrisp_sensor_util_get_dt_gpio_info(struct device *dev, struct cam_hw_info *hw_info)
{
	struct device_node    *of_node;
	struct cam_gpio_info  *gpio_info;
	int32_t ret, i;
	uint16_t *gpio_array;
	int16_t gpio_array_size;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}

	gpio_array_size = of_count_phandle_with_args(of_node, "gpios", "#gpio-cells");
	if (gpio_array_size <= 0) {
		XRISP_PR_INFO("no gpios found");
		return 0;
	}

	gpio_array = kcalloc(gpio_array_size, sizeof(uint16_t), GFP_KERNEL);
	if (!gpio_array)
		return -ENOMEM;

	for (i = 0; i < gpio_array_size; i++)
		gpio_array[i] = of_get_named_gpio(of_node, "gpios", i);

	gpio_info = &hw_info->gpio_info;
	ret = xrisp_sensor_util_get_dt_gpio_req_tbl(dev, gpio_info, gpio_array,
		gpio_array_size);
	if (ret) {
		XRISP_PR_ERROR("get gpio request table failed.");
		goto free_gpio_array;
	}

	gpio_info->gpio_tbl = devm_kcalloc(dev, gpio_array_size,
		sizeof(struct gpio), GFP_KERNEL);
	if (!gpio_info->gpio_tbl) {
		ret = -ENOMEM;
		goto free_gpio_array;
	}

	for (i = 0; i < gpio_array_size; i++)
		gpio_info->gpio_tbl[i].gpio = gpio_array[i];

	gpio_info->gpio_tbl_size = gpio_array_size;
	kfree(gpio_array);

	return ret;

free_gpio_array:
	kfree(gpio_array);

	return ret;
}

int xrisp_sensor_util_get_clk_resource(struct device *dev, struct cam_hw_info *hw_info)
{
	int i;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	/* Initialize clk */
	for (i = 0; i < hw_info->num_clk; i++) {
		hw_info->clk[i] = devm_clk_get(dev,
					hw_info->clk_name[i]);
		if (IS_ERR_OR_NULL(hw_info->clk[i])) {
			XRISP_PR_ERROR("get clk failed for %s.", hw_info->clk_name[i]);
			return -EINVAL;
		}
	}

	return 0;
}

int xrisp_sensor_util_get_regulator_resource(struct device *dev, struct cam_hw_info *hw_info)
{
	int i;

	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	/* Initialize regulators */
	for (i = 0; i < hw_info->num_rgltr; i++) {
		hw_info->rgltr[i] = devm_regulator_get(dev,
					hw_info->rgltr_name[i]);
		if (IS_ERR_OR_NULL(hw_info->rgltr[i])) {
			XRISP_PR_ERROR("get regulator failed for %s.", hw_info->rgltr_name[i]);
			return -EINVAL;
		}
	}

	return 0;
}

int xrisp_sensor_util_get_pinctrl_resource(struct device *dev, struct cam_hw_info *hw_info)
{
	struct cam_pinctrl_info  *pinctrl_info;
	struct device_node *of_node    = NULL;
	int ret;
	char *cci_active               = SENSOR_PINCTRL_CCI_STATE_ACTIVE;
	char *cci_sleep                = SENSOR_PINCTRL_CCI_STATE_SLEEP;
	uint32_t distinct_hwid         = 0;
	uint32_t pcb_hwid              = 0;

	of_node = dev->of_node;
	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(dev)) {
		XRISP_PR_ERROR("dev is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}

	pinctrl_info = &hw_info->pinctrl_info;
	pinctrl_info->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(pinctrl_info->pinctrl)) {
		XRISP_PR_ERROR("get pinctrl failed");
		return -EINVAL;
	}

	pinctrl_info->gpio_state_active =
		pinctrl_lookup_state(pinctrl_info->pinctrl, SENSOR_PINCTRL_STATE_ACTIVE);
	if (IS_ERR_OR_NULL(pinctrl_info->gpio_state_active)) {
		XRISP_PR_ERROR("get pinctrl active state failed.");
		return -EINVAL;
	}
	pinctrl_info->gpio_state_suspend =
		pinctrl_lookup_state(pinctrl_info->pinctrl, SENSOR_PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(pinctrl_info->gpio_state_suspend)) {
		XRISP_PR_ERROR("get pinctrl sleep state failed.");
		return -EINVAL;
	}

	if (of_find_property(of_node, "distinct_hwid", NULL)) {
		ret = of_property_read_u32(of_node, "distinct_hwid", &distinct_hwid);
		if (ret) {
			XRISP_PR_ERROR("[PCB_DISTINCTION] get distinct_hwid failed, ret:%d", ret);
		} else {
			pcb_hwid = get_hw_id_value();
			XRISP_PR_INFO("[PCB_DISTINCTION] pcb_hwid:0x%x, distinct_hwid:0x%x", pcb_hwid, distinct_hwid);
			if (pcb_hwid > distinct_hwid) {
				cci_active = SENSOR_PINCTRL_CCI_STATE_ACTIVE_1;
				cci_sleep  = SENSOR_PINCTRL_CCI_STATE_SLEEP_1;
				XRISP_PR_INFO("[PCB_DISTINCTION] Use cci_1, when pc_hwid is larger than distinct_hwid.");
			}
		}
	}

	pinctrl_info->gpio_state_cci_active =
		pinctrl_lookup_state(pinctrl_info->pinctrl, cci_active);
	if (IS_ERR_OR_NULL(pinctrl_info->gpio_state_cci_active)) {
		pr_err("get pinctrl cci active state failed.");
		return -EINVAL;
	}
	pinctrl_info->gpio_state_cci_suspend =
		pinctrl_lookup_state(pinctrl_info->pinctrl, cci_sleep);
	if (IS_ERR_OR_NULL(pinctrl_info->gpio_state_cci_suspend)) {
		pr_err("get pinctrl cci sleep state failed.");
		return -EINVAL;
	}

	return 0;
}

int xrisp_sensor_util_clk_set(struct cam_hw_info *hw_info, const char *name,
		uint32_t enable, int level)
{
	int ret, i;

	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	if (!hw_info->num_clk)
		return 0;

	if (level < CAM_SUSPEND_VOTE || level >= CAM_MAX_VOTE) {
		XRISP_PR_ERROR("Invalid clock level:%d", level);
		return -EINVAL;
	}

	for (i = 0; i < hw_info->num_clk; i++) {
		if (!strcmp(hw_info->clk_name[i], name)) {
			if (enable) {
				ret = clk_set_rate(hw_info->clk[i],
					hw_info->clk_rate[level][i]);
				if (ret) {
					XRISP_PR_ERROR("clk:%s set rate:%d failed.",
						name, hw_info->clk_rate[level][i]);
					return -EINVAL;
				}
				ret = clk_prepare_enable(hw_info->clk[i]);
				if (ret) {
					XRISP_PR_ERROR("clk:%s enable failed.", name);
					return -EINVAL;
				}
			} else {
				ret = clk_set_rate(hw_info->clk[i],
					hw_info->clk_rate[level][i]);
				if (ret) {
					pr_err("clk:%s set rate:%d failed in disable.",
						name, hw_info->clk_rate[level][i]);
				}
				clk_disable_unprepare(hw_info->clk[i]);
			}
			return 0;
		}
	}

	XRISP_PR_ERROR("not found clk:%s.", name);
	return -EINVAL;
}

int xrisp_sensor_util_regulator_set(struct regulator *rgltr,
	const char *rgltr_name,
	uint32_t rgltr_min_volt,
	uint32_t rgltr_max_volt,
	uint32_t rgltr_op_mode,
	uint32_t rgltr_delay_ms,
	uint32_t enable)
{
	int ret;

	if (XRISP_CHECK_NULL_RETURN_INT(rgltr)) {
		XRISP_PR_ERROR("rgltr is null");
		return -ENOMEM;
	}
	if (enable) {
		ret = regulator_set_voltage(rgltr, rgltr_min_volt, rgltr_max_volt);
		if (ret) {
			XRISP_PR_ERROR("%s set regulator voltage failed.", rgltr_name);
			return ret;
		}

		ret = regulator_set_load(rgltr, rgltr_op_mode);
		if (ret) {
			XRISP_PR_ERROR("%s set regulator load current failed.", rgltr_name);
			return ret;
		}

		ret = regulator_enable(rgltr);
		if (ret) {
			XRISP_PR_ERROR("%s regulator enable failed", rgltr_name);
			return ret;
		}

		xtisp_sensor_util_msleep(rgltr_delay_ms);
	} else {
		ret = regulator_disable(rgltr);
		if (ret) {
			XRISP_PR_ERROR("%s regulator disable failed.", rgltr_name);
			return ret;
		}

		xtisp_sensor_util_msleep(rgltr_delay_ms);

		ret = regulator_set_load(rgltr, 0);
		if (ret) {
			XRISP_PR_ERROR("%s set regulator load failed in disable", rgltr_name);
			return ret;
		}

		ret = regulator_set_voltage(rgltr, 0, rgltr_max_volt);
		if (ret) {
			XRISP_PR_ERROR("%s set regulator voltage failed in disable", rgltr_name);
			return ret;
		}
	}

	return ret;
}

int xrisp_sensor_util_regulator_set_by_name(struct cam_hw_info *hw_info,
	const char *name, uint32_t enable)
{
	int ret, i;

	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	if (hw_info->rgltr_ctrl_support & RGLTR_BYPASS) {
		XRISP_PR_INFO("no support regulator:%s.", name);
		return 0;
	}
	if (!hw_info->num_rgltr) {
		XRISP_PR_ERROR("no regulator:%s num.", name);
		return -EINVAL;
	}

	for (i = 0; i < hw_info->num_rgltr; i++) {
		if (!strcmp(hw_info->rgltr_nd_name[i], name)) {
			ret = xrisp_sensor_util_regulator_set(hw_info->rgltr[i],
				hw_info->rgltr_name[i],
				hw_info->rgltr_min_volt[i],
				hw_info->rgltr_max_volt[i],
				hw_info->rgltr_op_mode[i],
				hw_info->rgltr_delay[i],
				enable);
			if (ret) {
				XRISP_PR_ERROR("regulator:%s set failed.", name);
				return ret;
			}
			return 0;
		}
	}

	XRISP_PR_ERROR("not found regulator:%s.", name);
	return -EINVAL;
}
