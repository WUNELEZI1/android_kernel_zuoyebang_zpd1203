// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][edr][thermal] %s(%d): " fmt, __func__, __LINE__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/thermal.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/kernfs.h>
#include <linux/power_supply.h>
#include <linux/notifier.h>
#include "xrisp_log.h"
#include "cam_sysfs/xrisp_sysfs.h"

/**
 * This struct import from  driver/base/base.h
 *
 * struct subsys_private - structure to hold the private to the driver core portions of the bus_type/class structure.
 *
 * @subsys - the struct kset that defines this subsystem
 * @devices_kset - the subsystem's 'devices' directory
 * @interfaces - list of subsystem interfaces associated
 * @mutex - protect the devices, and interfaces lists.
 *
 * @drivers_kset - the list of drivers associated
 * @klist_devices - the klist to iterate over the @devices_kset
 * @klist_drivers - the klist to iterate over the @drivers_kset
 * @bus_notifier - the bus notifier list for anything that cares about things
 *                 on this bus.
 * @bus - pointer back to the struct bus_type that this structure is associated
 *        with.
 *
 * @glue_dirs - "glue" directory to put in-between the parent device to
 *              avoid namespace conflicts
 * @class - pointer back to the struct class that this structure is associated
 *          with.
 *
 * This structure is the one that is the actual kobject allowing struct
 * bus_type/class to be statically allocated safely.  Nothing outside of the
 * driver core should ever touch these fields.
 */

struct subsys_private {
	struct kset subsys;
	struct kset *devices_kset;
	struct list_head interfaces;
	struct mutex mutex;

	struct kset *drivers_kset;
	struct klist klist_devices;
	struct klist klist_drivers;
	struct blocking_notifier_head bus_notifier;
	unsigned int drivers_autoprobe : 1;
	const struct bus_type *bus;
	struct device *dev_root;

	struct kset glue_dirs;
	struct class *class;

	struct lock_class_key lock_key;
};

#define to_subsys_private(obj) container_of(obj, struct subsys_private, subsys.kobj)

static int board_sensor_temp;
static int board_sensor_second_temp;

static ssize_t xrisp_thermal_board_sensor_temp_show(struct kobject *kobj,
						    struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", board_sensor_temp);
}

static ssize_t xrisp_thermal_board_sensor_temp_store(struct kobject *kobj,
						     struct kobj_attribute *attr, const char *buf,
						     size_t len)
{
	int val = 0;

	val = kstrtol(buf, 10, NULL);

	board_sensor_temp = val;

	return len;
}

static ssize_t xrisp_thermal_board_sensor_second_temp_show(struct kobject *kobj,
							   struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", board_sensor_second_temp);
}

static ssize_t xrisp_thermal_board_sensor_second_temp_store(struct kobject *kobj,
							    struct kobj_attribute *attr,
							    const char *buf, size_t len)
{
	int val = 0;

	val = kstrtol(buf, 10, NULL);

	board_sensor_second_temp = val;

	return len;
}

static struct kobj_attribute kobj_attr_board_sensor_temp =
	__ATTR(board_sensor_temp, 0664, xrisp_thermal_board_sensor_temp_show,
	       xrisp_thermal_board_sensor_temp_store);
static struct kobj_attribute kobj_attr_board_sensor_second_temp =
	__ATTR(board_sensor_second_temp, 0664, xrisp_thermal_board_sensor_second_temp_show,
	       xrisp_thermal_board_sensor_second_temp_store);

static struct attribute *xrisp_thermal_dev_attr_group[] = {
	&kobj_attr_board_sensor_temp.attr,
	&kobj_attr_board_sensor_second_temp.attr,
	NULL,
};

static const struct attribute_group xrisp_thermal_group = {
	.attrs = xrisp_thermal_dev_attr_group,
};

static struct kobject *isp_kobj;
static struct kobject *thermal_kobj;

static void xrisp_create_thermal_node(void)
{
	int ret = 0;

	ret = xrisp_wait_sysfs_init_done();
	if (ret)
		return;

	isp_kobj = xrisp_get_isp_kobj();
	if (!isp_kobj) {
		XRISP_PR_ERROR("%s: get isp kobj failed\n", __func__);
		return;
	}

	thermal_kobj = kobject_create_and_add("xrisp_thermal", isp_kobj);
	if (!isp_kobj)
		goto destory_isp_kobj;

	ret = sysfs_create_group(thermal_kobj, &xrisp_thermal_group);
	if (ret)
		goto destory_kobj;

	XRISP_PR_INFO("%s finish\n", __func__);
	return;

destory_kobj:
	kobject_put(thermal_kobj);
destory_isp_kobj:
	xrisp_put_isp_kobj();
	isp_kobj = NULL;
}

static void xrisp_destroy_thermal_node(void)
{
	if (!isp_kobj)
		return;

	sysfs_remove_group(thermal_kobj, &xrisp_thermal_group);
	kobject_put(thermal_kobj);
	xrisp_put_isp_kobj();
}

int xrisp_thermal_init(void)
{
	xrisp_create_thermal_node();

	return 0;
}

void xrisp_thermal_exit(void)
{
	xrisp_destroy_thermal_node();
}

MODULE_AUTHOR("XRING ISP");
MODULE_DESCRIPTION("XRING ISP interface for debug");
MODULE_LICENSE("GPL v2");
