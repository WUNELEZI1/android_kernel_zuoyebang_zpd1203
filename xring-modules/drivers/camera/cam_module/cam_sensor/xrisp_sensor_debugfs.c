// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sensor_debugfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sensor_debugfs", __func__, __LINE__

#include <linux/of.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include "xrisp_sensor_ctrl.h"
#include "xrisp_sensor_dev.h"
#include "xrisp_log.h"

#define XRISP_SENSOR_DEBUG

#ifdef XRISP_SENSOR_DEBUG

struct cam_power_name {
	const char *name;
	int        id;
	uint32_t   type;
};

static int xrisp_power_get(struct cam_pm_dev *cam_pm,
	enum cam_power_type type, uint64_t *val)
{
	struct cam_power_ctrl_record *power_info;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(val)) {
		XRISP_PR_ERROR("val is null");
		return -ENOMEM;
	}

	power_info = &cam_pm->power_info.power_record[type];
	*val = atomic_read(&power_info->power_cnt);

	return 0;
}

static int xrisp_power_set(struct cam_pm_dev *cam_pm,
	enum cam_power_type type, uint64_t val)
{
	struct xrisp_sensor_power_ctrl *ctrl;
	struct cam_hw_info *hw_info;
	struct cam_power_ctrl_tab *power_tab;
	struct cam_power_ctrl *power_info;
	int power_setting_count;
	int ret = 0;
	int i;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	hw_info = cam_pm->hw_info;
	if (XRISP_CHECK_NULL_RETURN_INT(hw_info)) {
		XRISP_PR_ERROR("hw_info is null");
		return -ENOMEM;
	}
	power_tab = hw_info->power_tab;
	if (XRISP_CHECK_NULL_RETURN_INT(power_tab)) {
		XRISP_PR_ERROR("power_tab is null");
		return -ENOMEM;
	}

	if ((type < POWER_TYPE_SENSOR) || (type >= POWER_TYPE_MAX)) {
		XRISP_PR_ERROR("Invalid power type:%d.", type);
		return -EINVAL;
	}

	if (val)
		power_info = &power_tab->pu_settings[type];
	else
		power_info = &power_tab->pd_settings[type];

	if (!power_info->count) {
		XRISP_PR_ERROR("type:%d power setting is not exist.", type);
		return -EINVAL;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(power_info->power_settings)) {
		XRISP_PR_ERROR("power_info->power_settings is null");
		return -ENOMEM;
	}
	power_setting_count = power_info->count;
	ctrl = kzalloc((sizeof(struct xrisp_sensor_power_ctrl) +
		(sizeof(struct xrisp_sensor_power_setting) * power_setting_count)),
		GFP_KERNEL);
	if (!ctrl) {
		XRISP_PR_ERROR("camid:%d alloc memory failed.", cam_pm->id);
		return -ENOMEM;
	}
	ctrl->sensor_id = cam_pm->id;
	ctrl->power_type = type;
	ctrl->count = power_setting_count;
	for (i = 0; i < power_setting_count; i++) {
		ctrl->power_settings[i].power_seq_type =
			(char)power_info->power_settings[i].seq_type;
		ctrl->power_settings[i].delay_milli_seconds =
			(short int)power_info->power_settings[i].seq_delay;
		ctrl->power_settings[i].power_seq_value =
			(uint64_t)power_info->power_settings[i].seq_val;
	}
	if (val) {
		ret = xrisp_sensor_power_up(ctrl);
		if (ret) {
			XRISP_PR_ERROR("cam:%d powerup failed.", cam_pm->id);
			goto free_ctrl;
		}
	} else {
		ret = xrisp_sensor_power_down(ctrl);
		if (ret) {
			XRISP_PR_ERROR("cam:%d powerdown failed.", cam_pm->id);
			goto free_ctrl;
		}
	}

	kfree(ctrl);
	return 0;

free_ctrl:
	kfree(ctrl);
	return ret;
}

static int xrisp_sensor_power_get(void *data, uint64_t *val)
{
	int ret;
	struct cam_pm_dev *cam_pm = (struct cam_pm_dev *)data;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	ret = xrisp_power_get(cam_pm, POWER_TYPE_SENSOR, val);
	if (ret) {
		XRISP_PR_ERROR("cam:%d get sensor power count failed.", cam_pm->id);
		return ret;
	}

	return ret;
}

static int xrisp_sensor_power_set(void *data, uint64_t val)
{
	int ret;
	struct cam_pm_dev *cam_pm = (struct cam_pm_dev *)data;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	ret = xrisp_power_set(cam_pm, POWER_TYPE_SENSOR, val);
	if (ret) {
		XRISP_PR_ERROR("cam:%d sensor power:%d failed.",
			cam_pm->id, (unsigned int)val);
		return ret;
	}

	return ret;
}

DEFINE_DEBUGFS_ATTRIBUTE(xrisp_sensor_power_fops, xrisp_sensor_power_get,
	xrisp_sensor_power_set, "%llu\n");

static int xrisp_af_power_get(void *data, uint64_t *val)
{
	int ret;
	struct cam_pm_dev *cam_pm = (struct cam_pm_dev *)data;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	ret = xrisp_power_get(cam_pm, POWER_TYPE_AF, val);
	if (ret) {
		XRISP_PR_ERROR("cam:%d get sensor power count failed.", cam_pm->id);
		return ret;
	}

	return ret;
}

static int xrisp_af_power_set(void *data, uint64_t val)
{
	int ret;
	struct cam_pm_dev *cam_pm = (struct cam_pm_dev *)data;

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	ret = xrisp_power_set(cam_pm, POWER_TYPE_AF, val);
	if (ret) {
		XRISP_PR_ERROR("cam:%d sensor power:%d failed.",
			cam_pm->id, (unsigned int)val);
		return ret;
	}

	return ret;
}

DEFINE_DEBUGFS_ATTRIBUTE(xrisp_af_power_fops, xrisp_af_power_get,
	xrisp_af_power_set, "%llu\n");

int xrisp_parse_power_settings_from_dts(struct device *dev,
	struct device_node *of_node, struct cam_power_ctrl *power_info)
{
	int count;
	struct cam_hw_power_setting *power_settings;
	int i;

	if (XRISP_CHECK_NULL_RETURN_INT(of_node)) {
		XRISP_PR_ERROR("of_node is null");
		return -ENOMEM;
	}
	if (XRISP_CHECK_NULL_RETURN_INT(power_info)) {
		XRISP_PR_ERROR("power_info is null");
		return -ENOMEM;
	}
	count = of_property_count_strings(of_node, "seq-names");
	if (count <= 0) {
		XRISP_PR_ERROR("no seq-names found.");
		count = 0;
		return -EINVAL;
	}

	power_info->count = count;
	power_settings = devm_kcalloc(dev, count,
		sizeof(struct cam_hw_power_setting), GFP_KERNEL);
	if (!power_settings)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		if (of_property_read_u32_index(of_node,
			"seq-table", i, &power_settings[i].seq_type)) {
			XRISP_PR_ERROR("get seq table:%d failed.", i);
			return -EINVAL;
		}

		if (of_property_read_u32_index(of_node,
			"seq-delay", i, &power_settings[i].seq_delay)) {
			XRISP_PR_ERROR("get seq delay:%d failed.", i);
			return -EINVAL;
		}

		if (of_property_read_u32_index(of_node,
			"seq-val", i, &power_settings[i].seq_val)) {
			XRISP_PR_ERROR("get seq val:%d failed.", i);
			return -EINVAL;
		}
	}

	power_info->power_settings = power_settings;
	return 0;
}

int xrisp_parse_power_table(struct cam_pm_dev *cam_pm)
{
	struct device *dev;
	struct device_node *of_node;
	struct device_node *power_node;
	struct cam_hw_info *hw_info;
	struct cam_power_ctrl_tab *power_tab;
	struct cam_power_ctrl *power_info;
	int i, m;
	int ret;
	struct cam_power_name power_type_tab[] = {
			{"sensor-pu", POWER_TYPE_SENSOR, 1},
			{"af-pu", POWER_TYPE_AF, 1},
			{"ois-pu", POWER_TYPE_OIS, 1},
			{"eeprom-pu", POWER_TYPE_EEPROM, 1},
			{"sensor-pd", POWER_TYPE_SENSOR, 0},
			{"af-pd", POWER_TYPE_AF, 0},
			{"ois-pd", POWER_TYPE_OIS, 0},
			{"eeprom-pd", POWER_TYPE_EEPROM, 0},
		};

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	dev = cam_pm->dev;
	hw_info = cam_pm->hw_info;
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

	power_tab = devm_kzalloc(dev, sizeof(struct cam_pm_dev), GFP_KERNEL);
	if (XRISP_CHECK_NULL_RETURN_INT(power_tab)) {
		XRISP_PR_ERROR("power_tab is null");
		return -ENOMEM;
	}
	hw_info->power_tab = power_tab;

	for (i = 0; i < ARRAY_SIZE(power_type_tab); i++) {
		power_node = of_parse_phandle(of_node, power_type_tab[i].name, 0);
		if (!power_node) {
			XRISP_PR_DEBUG("no find %s node", power_type_tab[i].name);
			continue;
		}
		m = power_type_tab[i].id;
		if (m >= MAX_POWER_TYPE) {
			XRISP_PR_ERROR("Invalid power type:%d.", m);
			continue;
		}
		if (power_type_tab[i].type)
			power_info = &power_tab->pu_settings[m];
		else
			power_info = &power_tab->pd_settings[m];

		ret = xrisp_parse_power_settings_from_dts(dev,
			power_node, power_info);
		if (ret) {
			XRISP_PR_ERROR("parse %s failed.", power_type_tab[i].name);
			power_info->count = 0;
			continue;
		}
	}

	return 0;
}

int xrisp_sensor_debugfs_init(struct cam_pm_dev *cam_pm)
{
	int ret = 0;
	int id;
	char name[32];

	if (XRISP_CHECK_NULL_RETURN_INT(cam_pm)) {
		XRISP_PR_ERROR("cam_pm is null");
		return -ENOMEM;
	}
	id = cam_pm->id;
	sprintf(name, "xrisp_cam%d", id);
	XRISP_PR_DEBUG("create debugfs:%s", name);
	cam_pm->dentry = debugfs_create_dir(name, NULL);
	if (IS_ERR_OR_NULL(cam_pm->dentry)) {
		XRISP_DEV_ERROR(cam_pm->dev, "create debugfs failed.");
		return -EINVAL;
	}

	debugfs_create_file("power", 0600, cam_pm->dentry, cam_pm, &xrisp_sensor_power_fops);
	debugfs_create_file("af_power", 0600, cam_pm->dentry, cam_pm, &xrisp_af_power_fops);

	// XRISP_PR_INFO("create debugfs.");

	xrisp_parse_power_table(cam_pm);
	return ret;
}

void xrisp_sensor_debugfs_exit(struct cam_pm_dev *cam_pm)
{
	debugfs_remove(cam_pm->dentry);
}

#else /* XRISP_SENSOR_DEBUG */

int xrisp_sensor_debugfs_init(struct cam_pm_dev *cam_pm)
{
	return 0;
}

void xrisp_sensor_debugfs_exit(struct cam_pm_dev *cam_pm)
{}

#endif /* XRISP_SENSOR_DEBUG */
