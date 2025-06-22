// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include "soc/xring/sensorhub/sensor_power/shub_sensor_power.h"
#define BUFFER_SIZE 1024

struct sensor_power_instance_s {
	struct regulator *sensor_regulator;
	bool power_on;
};

static struct device *g_dev;
static struct device_node *g_sensor_power_node;
static struct sensor_power_instance_s *sensor_power_instance;
static int g_sensor_power_num;
static char buffer[BUFFER_SIZE];

static int get_regulator(const char *power_name,
			 struct regulator **dst_regulator)
{
	struct regulator *sensor_regulator = NULL;

	if (!g_sensor_power_node)
		return -EINVAL;

	sensor_regulator = regulator_get(g_dev, power_name);
	if (IS_ERR_OR_NULL(sensor_regulator)) {
		pr_err("get regulator for %s fail, ret %d\n", power_name,
		       PTR_ERR_OR_ZERO(sensor_regulator));
		return PTR_ERR_OR_ZERO(sensor_regulator);
	}

	(*dst_regulator) = sensor_regulator;
	return 0;
}

static int power_operation(int index, bool enable)
{
	int ret;

	if (!sensor_power_instance) {
		pr_err("sensor power not init\n");
		return -ENXIO;
	}
	if (index >= g_sensor_power_num ||
	    !sensor_power_instance[index].sensor_regulator) {
		pr_err("sensor power regulator for %d not find\n", index);
		return -EINVAL;
	}

	if (enable) {
		if (sensor_power_instance[index].power_on) {
			pr_warn("sensor power %d has power on\n", index);
			return -EINVAL;
		}
		ret = regulator_enable(
			sensor_power_instance[index].sensor_regulator);
	} else {
		if (!sensor_power_instance[index].power_on) {
			pr_warn("sensor power %d not power on\n", index);
			return -EINVAL;
		}
		ret = regulator_disable(
			sensor_power_instance[index].sensor_regulator);
	}
	if (ret) {
		pr_err("modify regulator for %d fail, ret %d\n", index, ret);
		return ret;
	}
	sensor_power_instance[index].power_on = enable;

	return 0;
}

static struct device_node *get_sensor_power_node(void)
{
	struct device_node *sh_node = NULL;
	struct device_node *sensor_power_root = NULL;

	if (g_sensor_power_node)
		return g_sensor_power_node;

	sh_node = of_find_node_by_name(NULL, "sensorhub");
	if (!sh_node) {
		pr_err("sensorhub root node is null!\n");
		return NULL;
	}

	if (!of_device_is_available(sh_node)) {
		pr_err("%s,status property is not okay\n", __func__);
		return NULL;
	}

	sensor_power_root = of_find_node_by_name(sh_node, "sensor_power");
	if (!sensor_power_root) {
		pr_err("sensor power root node is null!\n");
		return NULL;
	}

	if (!of_device_is_available(sensor_power_root)) {
		pr_warn("sensor power status property is not okay\n");
		return NULL;
	}

	of_node_put(sh_node);
	g_sensor_power_node = sensor_power_root;
	pr_info("success find sensor power node\n");
	return g_sensor_power_node;
}

static void power_operation_all(bool enable)
{
	int index = 0;

	for (index = 0; index < g_sensor_power_num; index++)
		power_operation(index, enable);
}

void sensor_power_on_all(void)
{
	power_operation_all(true);
}
EXPORT_SYMBOL(sensor_power_on_all);

void sensor_power_off_all(void)
{
	power_operation_all(false);
}
EXPORT_SYMBOL(sensor_power_off_all);

void sensor_power_init(void)
{
	struct device_node *sensor_power_root = NULL;
	struct of_phandle_iterator it;
	const char *power_name;
	int size = 0;
	int index;
	int err;
	int buffer_pos = 0;

	sensor_power_root = get_sensor_power_node();
	if (!sensor_power_root)
		return;

	size = of_count_phandle_with_args(sensor_power_root, "power-supply",
					  NULL);
	pr_info("get sensor power count ret %d\n", size);
	if (size <= 0)
		return;
	g_sensor_power_num = size;
	sensor_power_instance = kcalloc(
		size, sizeof(struct sensor_power_instance_s), GFP_KERNEL);
	if (!sensor_power_instance)
		return;

	index = 0;
	of_for_each_phandle(&it, err, sensor_power_root, "power-supply", NULL,
			    0) {
		err = of_property_read_string(it.node, "regulator-name",
					      &power_name);
		if (err)
			pr_err("find sensor power[%d] name err %d", index, err);
		else {
			get_regulator(power_name,
				      &sensor_power_instance[index++]
					       .sensor_regulator);
			pr_info("find sensor power [%s] success", power_name);
			if ((sizeof(buffer) - buffer_pos) > strlen(power_name))
				buffer_pos +=
					snprintf(&buffer[buffer_pos],
						 sizeof(buffer) - buffer_pos,
						 "%s\n", power_name);
			else
				pr_warn("sensor power name buffer no space all:%lu use:%d\n",
					sizeof(buffer), buffer_pos);
			of_node_put(it.node);
		}
		err = 0;
	}
}
EXPORT_SYMBOL(sensor_power_init);

void sensor_power_deinit(void)
{
	int index;

	if (!sensor_power_instance)
		return;

	for (index = 0; index < g_sensor_power_num; index++) {
		if (!sensor_power_instance[index].sensor_regulator)
			continue;
		if (sensor_power_instance[index].power_on)
			regulator_disable(
				sensor_power_instance[index].sensor_regulator);

		regulator_put(sensor_power_instance[index].sensor_regulator);
	}

	g_sensor_power_num = 0;
	if (g_sensor_power_node)
		of_node_put(g_sensor_power_node);
	kfree(sensor_power_instance);
	sensor_power_instance = NULL;
}
EXPORT_SYMBOL(sensor_power_deinit);

void sensor_power_set_dev(struct device *dev)
{
	g_dev = dev;
}
EXPORT_SYMBOL(sensor_power_set_dev);

const char *sensor_power_get_names(void)
{
	return buffer;
}
EXPORT_SYMBOL(sensor_power_get_names);

int sensor_power_on(int index)
{
	return power_operation(index, true);
}
EXPORT_SYMBOL(sensor_power_on);

int sensor_power_off(int index)
{
	return power_operation(index, false);
}
EXPORT_SYMBOL(sensor_power_off);

MODULE_LICENSE("Dual BSD/GPL");
