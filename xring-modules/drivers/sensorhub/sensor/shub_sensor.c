// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dts.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>

uint64_t g_sensor_dts_addr_offset;

typedef int (*function_pointer)(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);
static int parse_common_info(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);
static int parse_amg_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);
static int parse_mag_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);
static int parse_ois_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);
static int parse_hall_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);
static int parse_touch_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size);


struct parse_func_map {
	enum sensor_index index;
	const char *name;
	function_pointer parse_function;
	unsigned long size;
};

struct parse_func_map functionMap[] = {
	{SH_SENSOR_AMG_INDEX, "amg", parse_amg_sensor, sizeof(struct sh_dts_amg_info)},
	{SH_SENSOR_MAG_INDEX, "mag", parse_mag_sensor, sizeof(struct sh_dts_mag_info)},
	{SH_SENSOR_ALS_INDEX, "als", parse_common_info, sizeof(struct sh_dts_sensor_common_info)},
	{SH_SENSOR_FLICKER_INDEX, "flicker", parse_common_info, sizeof(struct sh_dts_sensor_common_info)},
	{SH_SENSOR_BAR_INDEX, "bar", parse_common_info, sizeof(struct sh_dts_sensor_common_info)},
	{SH_SENSOR_SAR_INDEX, "sar", parse_common_info, sizeof(struct sh_dts_sensor_common_info)},
	{SH_SENSOR_TOF_INDEX, "tof", parse_common_info, sizeof(struct sh_dts_sensor_common_info)},
	{SH_SENSOR_OIS_INDEX, "ois", parse_ois_sensor, sizeof(struct sh_dts_ois_info)},
	{SH_SENSOR_HALL_INDEX, "hall", parse_hall_sensor, sizeof(struct sh_dts_hall_info)},
	{SH_SENSOR_TOUCH_INDEX, "touch", parse_touch_sensor, sizeof(struct sh_dts_touch_info)}};

static int get_parse_func_num(const char *name)
{
	int numFunctions = sizeof(functionMap) / sizeof(struct parse_func_map);

	for (int i = 0; i < numFunctions; i++) {
		if (strcmp(name, functionMap[i].name) == 0)
			return i;
	}
	pr_err("Function not found: %s\n", name);
	return -1;
}

static int parse_common_info(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size)
{
	int ret = 0;
	uint32_t val = 0;
	const char *strings[8];
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_sensor_common_info *common_dts = (struct sh_dts_sensor_common_info *)addr;

	if (sizeof(struct sh_bsp_dts) + sizeof(struct sh_app_dts) + g_sensor_dts_addr_offset + size > SHUB_NS_DDR_SHM_DTS_SIZE) {
		pr_err("sensorhub dts size too big!\n");
		return -ENOMEM;
	}

	ret = of_property_read_u32(node, "bus_type", &val);
	if (ret < 0) {
		pr_err("get bus_type failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &common_dts->bus_type);
	pr_info("get bus_type %d!\n", val);

	ret = of_property_read_u32(node, "bus_num", &val);
	if (ret < 0) {
		pr_err("get bus_num failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &common_dts->bus_num);
	pr_info("get bus_num %d!\n", val);

	ret = of_property_read_u32(node, "slave_address", &val);
	if (ret < 0) {
		pr_err("get slave_address failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &common_dts->slave_address);
	pr_info("get slave_address %d!\n", val);

	ret = of_property_read_u32(node, "bus_speed_khz", &val);
	if (ret < 0) {
		pr_err("get bus_speed_khz failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writew((uint16_t)val, &common_dts->bus_speed_khz);
	pr_info("get bus_speed_khz %d!\n", val);

	ret = of_property_read_string(node, "driver_id", &strings[0]);
	if (ret < 0) {
		pr_err("get driver_id failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	memset_io(common_dts->driver_id, 0, sizeof(common_dts->driver_id));
	memcpy_toio(common_dts->driver_id, strings[0], strlen(strings[0]));
	pr_info("get driver_id %s!\n", strings[0]);

	ret = of_property_read_u32(node, "chip_id_addr", &val);
	if (ret < 0) {
		pr_err("get chip_id_addr failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writew((uint16_t)val, &common_dts->chip_id_addr);
	pr_info("get chip_id_addr %d!\n", val);

	ret = of_property_read_u32(node, "chip_id_value", &val);
	if (ret < 0) {
		pr_err("get chip_id_value failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &common_dts->chip_id_val);
	pr_info("get chip_id_value %d!\n", val);

	val = index;
	writeb((uint8_t)val, &common_dts->index);
	pr_info("sensor index %d!\n", val);

	g_sensor_dts_addr_offset += size;
	pr_info("line %d get g_sensor_dts_addr_offset %llu!\n", __LINE__, g_sensor_dts_addr_offset);

	if (ret >= 0)
		ret = 0;

_err_node:
	return ret;
}

static int parse_amg_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size)
{
	int ret = 0;
	uint32_t val = 0;
	uint64_t val_u64 = 0;
	uint8_t val_u8 = 0;
	uint8_t orientation[3] = {0};
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_amg_info *amg_dts = (struct sh_dts_amg_info *)addr;

	pr_info("begin parse amg info %llu\n", (uint64_t)amg_dts);
	ret = parse_common_info(node, dts_head, index, size);
	if (ret < 0) {
		pr_err("get common info failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	if (amg_dts->common_info.bus_type == SH_BUS_I3C) {
		ret = of_property_read_u32(node, "i3c_device_id_flags", &val);
		if (ret < 0) {
			pr_err("get i3c_device_id_flags failed, ret[%d]!\n", ret);
			goto _err_node;
		}
		writeb((uint8_t)val, &amg_dts->i3c_device_id_flags);
		pr_info("get i3c_device_id_flags %d!\n", val);

		ret = of_property_read_u32(node, "i3c_device_id_dcr", &val);
		if (ret < 0) {
			pr_err("get i3c_device_id_dcr failed, ret[%d]!\n", ret);
			goto _err_node;
		}
		writeb((uint8_t)val, &amg_dts->i3c_device_id_dcr);
		pr_info("get i3c_device_id_dcr %d!\n", val);

		ret = of_property_read_u64(node, "i3c_device_id_pid", &val_u64);
		if (ret < 0) {
			pr_err("get i3c_device_id_pid failed, ret[%d]!\n", ret);
			goto _err_node;
		}
		memcpy_toio(&amg_dts->i3c_device_id_pid, &val_u64, sizeof(amg_dts->i3c_device_id_pid));
		pr_info("get i3c_device_id_pid %llu!\n", val_u64);
	}

	ret = of_property_read_u8_array(node, "orientation", orientation, 3);
	if (ret < 0) {
		pr_err("get orientation failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	for (int i = 0; i < 3; i++) {
		writeb(orientation[i], &amg_dts->orientation[i]);
		pr_info("get orientation %d!\n", orientation[i]);
	}

	ret = of_property_read_u8(node, "irq_num", &val_u8);
	if (ret < 0) {
		pr_err("get irq_num failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb(val_u8, &amg_dts->irq_num);
	pr_info("get irq_num %d!\n", val_u8);

	if (ret >= 0)
		ret = 0;

_err_node:
	pr_info("end parse amg info\n");
	return ret;
}

static int parse_mag_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size)
{
	int ret = 0;
	uint8_t orientation[3] = {0};
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_mag_info *mag_dts = (struct sh_dts_mag_info *)addr;

	pr_info("begin parse mag info\n");
	ret = parse_common_info(node, dts_head, index, size);
	if (ret < 0) {
		pr_err("get common info failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	ret = of_property_read_u8_array(node, "orientation", orientation, 3);
	if (ret < 0) {
		pr_err("get orientation failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	for (int i = 0; i < 3; i++) {
		writeb(orientation[i], &mag_dts->orientation[i]);
		pr_info("get orientation %d!\n", orientation[i]);
	}
	if (ret >= 0)
		ret = 0;

_err_node:
	pr_info("end parse mag info\n");
	return ret;
}

static int parse_ois_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size)
{
	int ret = 0;
	uint32_t val = 0;
	uint8_t orientation[3] = {0};
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_ois_info *ois_dts = (struct sh_dts_ois_info *)addr;

	pr_info("begin parse ois info\n");
	ret = parse_common_info(node, dts_head, index, size);
	if (ret < 0) {
		pr_err("get common info failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	ret = of_property_read_u32(node, "bus_type_x", &val);
	if (ret < 0) {
		pr_err("get bus_type_x failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &ois_dts->bus_type_x);
	pr_info("get bus_type_x %d!\n", val);

	ret = of_property_read_u32(node, "bus_num_x", &val);
	if (ret < 0) {
		pr_err("get bus_num_x failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &ois_dts->bus_num_x);
	pr_info("get bus_num_x %d!\n", val);

	ret = of_property_read_u32(node, "slave_address_x", &val);
	if (ret < 0) {
		pr_err("get slave_address_x failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &ois_dts->slave_address_x);
	pr_info("get slave_address_x %d!\n", val);

	ret = of_property_read_u32(node, "bus_speed_khz_x", &val);
	if (ret < 0) {
		pr_err("get bus_speed_khz_x failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writew((uint16_t)val, &ois_dts->bus_speed_khz_x);
	pr_info("get bus_speed_khz_x %d!\n", val);

	ret = of_property_read_u32(node, "bus_type_y", &val);
	if (ret < 0) {
		pr_err("get bus_type_y failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &ois_dts->bus_type_y);
	pr_info("get bus_type_y %d!\n", val);

	ret = of_property_read_u32(node, "bus_num_y", &val);
	if (ret < 0) {
		pr_err("get bus_num_y failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &ois_dts->bus_num_y);
	pr_info("get bus_num_y %d!\n", val);

	ret = of_property_read_u32(node, "slave_address_y", &val);
	if (ret < 0) {
		pr_err("get slave_address_y failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &ois_dts->slave_address_y);
	pr_info("get slave_address_y %d!\n", val);

	ret = of_property_read_u32(node, "bus_speed_khz_y", &val);
	if (ret < 0) {
		pr_err("get bus_speed_khz_y failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writew((uint16_t)val, &ois_dts->bus_speed_khz_y);
	pr_info("get bus_speed_khz_y %d!\n", val);
	ret = of_property_read_u8_array(node, "orientation", orientation, 3);
	if (ret < 0) {
		pr_err("get orientation failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	for (int i = 0; i < 3; i++) {
		writeb(orientation[i], &ois_dts->orientation[i]);
		pr_info("get orientation %d!\n", orientation[i]);
	}

	if (ret >= 0)
		ret = 0;

_err_node:
	pr_info("end parse ois info\n");
	return ret;
}

static int parse_hall_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size)
{
	int ret = 0;
	uint32_t val = 0;
	const char *strings[8];
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_hall_info *hall_dts = (struct sh_dts_hall_info *)addr;

	if (sizeof(struct sh_bsp_dts) + sizeof(struct sh_app_dts) + g_sensor_dts_addr_offset + size > SHUB_NS_DDR_SHM_DTS_SIZE) {
		pr_err("sensorhub dts size too big!\n");
		return -ENOMEM;
	}

	pr_info("begin parse hall info\n");
	val = index;
	writeb((uint8_t)val, &hall_dts->common_info.index);
	pr_info("sensor index %d!\n", val);
	ret = of_property_read_string(node, "driver_id", &strings[0]);
	if (ret < 0) {
		pr_err("get driver_id failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	memset_io(hall_dts->common_info.driver_id, 0, sizeof(hall_dts->common_info.driver_id));
	memcpy_toio(hall_dts->common_info.driver_id, strings[0], strlen(strings[0]));
	pr_info("get driver_id %s!\n", strings[0]);
	ret = of_property_read_u32(node, "gpio_n", &val);
	if (ret < 0) {
		pr_err("get gpio_n failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &hall_dts->gpio_n);
	pr_info("get gpio_n %d!\n", val);

	ret = of_property_read_u32(node, "gpio_s", &val);
	if (ret < 0) {
		pr_err("get gpio_s failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &hall_dts->gpio_s);
	pr_info("get gpio_s %d!\n", val);
	g_sensor_dts_addr_offset += size;
	pr_info("line %d get g_sensor_dts_addr_offset %llu!\n", __LINE__, g_sensor_dts_addr_offset);
	if (ret >= 0)
		ret = 0;

_err_node:
	pr_info("end parse hall info\n");
	return ret;
}

static int parse_touch_sensor(struct device_node *node, struct sh_app_dts *dts_head, enum sensor_index index, unsigned long size)
{
	int ret = 0;
	uint32_t val = 0;
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_touch_info *touch_dts = (struct sh_dts_touch_info *)addr;

	pr_info("begin parse touch info\n");

	ret = parse_common_info(node, dts_head, index, size);
	if (ret < 0) {
		pr_err("get common info failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	ret = of_property_read_u32(node, "irq_gpio", &val);
	if (ret < 0) {
		pr_err("get gpio_n failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &touch_dts->irq_gpio);
	pr_info("get gpio_n %d!\n", val);

	ret = of_property_read_u32(node, "cs_gpio", &val);
	if (ret < 0) {
		pr_err("get gpio_n failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &touch_dts->cs_gpio);
	pr_info("get gpio_n %d!\n", val);

	ret = of_property_read_u32(node, "spi,mode", &val);
	if (ret < 0) {
		pr_err("get gpio_n failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &touch_dts->spi_mode);
	pr_info("get gpio_n %d!\n", val);

	ret = of_property_read_u32(node, "spi,bits_per_word", &val);
	if (ret < 0) {
		pr_err("get gpio_n failed, ret[%d]!\n", ret);
		goto _err_node;
	}
	writeb((uint8_t)val, &touch_dts->spi_bits_per_word);
	pr_info("get gpio_n %d!\n", val);
	if (ret >= 0)
		ret = 0;

_err_node:
	pr_info("end parse touch info\n");
	return ret;
}

static void add_parse_end_info(struct sh_app_dts *dts_head)
{
	uint32_t val = 0;
	uint64_t addr = (uint64_t)dts_head + g_sensor_dts_addr_offset;
	struct sh_dts_sensor_common_info *common_dts = (struct sh_dts_sensor_common_info *)addr;

	if (sizeof(struct sh_bsp_dts) + sizeof(struct sh_app_dts) + g_sensor_dts_addr_offset + sizeof(struct sh_dts_sensor_common_info) > SHUB_NS_DDR_SHM_DTS_SIZE) {
		pr_err("sensorhub dts size too big!\n");
		return;
	}

	val = SH_SENSOR_MAX_INDEX;
	writeb((uint8_t)val, &common_dts->index);
	pr_info("sensor index %d!\n", val);
}

static int parse_sensor_info(struct device_node *parent, struct sh_app_dts *sensor_dts, const char *name)
{
	int ret = 0;
	struct device_node *node = NULL;
	int func_idx = get_parse_func_num(name);

	if (func_idx < 0) {
		pr_err("get parse function failed, ret[%d]!\n", func_idx);
		return -1;
	}
	enum sensor_index idx = functionMap[func_idx].index;
	unsigned long size = functionMap[func_idx].size;

	while (1) {
		node = of_get_next_available_child(parent, node);
		if (node == NULL) {
			pr_info("l%d, this is last node\n", __LINE__);
			break;
		}
		functionMap[func_idx].parse_function(node, sensor_dts, idx, size);
	}
	if (ret >= 0)
		ret = 0;

	return ret;
}

static int parse_sensor_dts(void)
{
	int ret = 0;
	struct sh_app_dts *sensor_dts = shub_get_app_dts();
	struct device_node *root = NULL;
	struct device_node *sensor_node = NULL;
	struct device_node *node = NULL;
	int sensor_num = sizeof(functionMap) / sizeof(struct parse_func_map);

	g_sensor_dts_addr_offset += sizeof(struct sh_app_dts);

	pr_info("sensor_dts %llu %llu!\n", (uint64_t)sensor_dts, g_sensor_dts_addr_offset);
	pr_info("begin parse sensor info\n");
	root = of_find_node_by_name(NULL, "sensorhub");
	if (!root) {
		ret = -EOPNOTSUPP;
		pr_err("sensorhub root node is null!\n");
		goto _err_node;
	}

	if (!of_device_is_available(root)) {
		pr_err("%s,status property is not okay\n", __func__);
		ret = -EOPNOTSUPP;
		goto _err_node;
	}

	sensor_node = of_find_node_by_name(root, "sensordriver");
	if (!sensor_node) {
		ret = -EOPNOTSUPP;
		pr_err("sensordrvier is null!\n");
		goto _err_node;
	}

	if (!of_device_is_available(sensor_node)) {
		pr_err("%s,status property is not okay\n", __func__);
		ret = -EOPNOTSUPP;
		goto _err_node;
	}
	pr_info("support %d sensors!\n", sensor_num);
	for (int i = 0; i < sensor_num; i++) {
		node = of_find_node_by_name(sensor_node, functionMap[i].name);
		if (!node) {
			pr_info("l%d, no %s node\n", __LINE__, functionMap[i].name);
			continue;
		}

		if (!of_device_is_available(node)) {
			pr_err("%s,status property is not okay\n", __func__);
			continue;
		}
		ret = parse_sensor_info(node, sensor_dts, functionMap[i].name);
		if (ret < 0) {
			pr_err("parse sensor %s info failed!\n", functionMap[i].name);
			continue;
		}
	}
	add_parse_end_info(sensor_dts);

_err_node:
	return ret;
}

static int parse_virtualsensor_dts(void)
{
	int ret = 0;
	struct sh_app_dts *sensor_dts = shub_get_app_dts();
	struct device_node *root = NULL;
	struct device_node *virtualsensor_node = NULL;

	memset_io(sensor_dts->virtualsensor_info.excluded_types, 0, sizeof(sensor_dts->virtualsensor_info.excluded_types));
	root = of_find_node_by_name(NULL, "sensorhub");
	if (!root) {
		ret = -EOPNOTSUPP;
		pr_err("sensorhub root node is null!\n");
		goto _err_node;
	}

	if (!of_device_is_available(root)) {
		pr_err("%s,status property is not okay\n", __func__);
		ret = -EOPNOTSUPP;
		goto _err_node;
	}

	virtualsensor_node = of_find_node_by_name(root, "virtualsensor");
	if (!virtualsensor_node) {
		ret = -EOPNOTSUPP;
		pr_err("virtualsensor is null!\n");
		goto _err_node;
	}

	if (!of_device_is_available(virtualsensor_node)) {
		pr_err("%s,status property is not okay\n", __func__);
		goto _err_node;
	}

	int num = 0;

	num = of_property_count_elems_of_size(virtualsensor_node, "excluded_types", sizeof(u32));
	if (num > 0 && num <= SH_EXCLUDED_TYPE_NUM) {
		uint32_t *excluded_types = kmalloc(sizeof(uint32_t) * num, GFP_KERNEL);

		if (!excluded_types) {
			ret = -ENOMEM;
			goto _err_node;
		}
		ret = of_property_read_u32_array(virtualsensor_node, "excluded_types", excluded_types, num);
		if (ret < 0) {
			pr_err("of_property_read_u32_array failed!\n");
			kfree(excluded_types);
			goto _err_node;
		}
		memset_io(sensor_dts->virtualsensor_info.excluded_types, 0, sizeof(sensor_dts->virtualsensor_info.excluded_types));
		for (int i = 0; i < num; i++) {
			writel(excluded_types[i], &sensor_dts->virtualsensor_info.excluded_types[i]);
			pr_info("excluded_types %d!\n", excluded_types[i]);
		}
		kfree(excluded_types);
	} else {
		pr_err("excluded_types num(%d) is invalid!\n", num);
		ret = -1;
	}

_err_node:
	return ret;
}

static int shub_sensor_init(void)
{
	int ret = 0;

	ret = parse_sensor_dts();
	if (ret < 0) {
		pr_err("parse_sensor_dts fail,ret=%d!\n", ret);
		return ret;
	}

	ret = parse_virtualsensor_dts();
	if (ret < 0) {
		pr_err("parse_virtualsensor_dts fail,ret=%d!\n", ret);
		return ret;
	}

	pr_info("parse shub_sensor dts done!\n");

	return ret;
}

static void shub_sensor_exit(void)
{
	pr_info("shub_sensor exit\n");
}

module_init(shub_sensor_init);
module_exit(shub_sensor_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_SOFTDEP("post: shub_boot");
