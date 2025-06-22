// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "log", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "log", __func__, __LINE___

#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/kstrtox.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_log.h"

static struct xrisp_log_data xrisp_log_priv = {
	.log_level = XRISP_LOG_ERROR,
};

int cam_log_set_level(int level)
{
	xrisp_log_priv.log_level = level;
	XRISP_PR_INFO("set xrisp log level = %d", xrisp_log_priv.log_level);
	return xrisp_log_priv.log_level;
}

int cam_log_get_level(void)
{
	return xrisp_log_priv.log_level;
}
EXPORT_SYMBOL_GPL(cam_log_get_level);

static ssize_t xrisp_log_level_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;

	len = snprintf(buf, PAGE_SIZE, "%d\n", xrisp_log_priv.log_level);
	return len;
}

static ssize_t xrisp_log_level_store(struct device *dev,
					struct device_attribute *attr, const
					char *buf, size_t count)
{
	if (kstrtos32(buf, 0, &xrisp_log_priv.log_level)) {
		XRISP_PR_ERROR("inherit enable store: parse enable failed");
		return -EINVAL;
	}
	XRISP_PR_INFO("write data %d", xrisp_log_priv.log_level);
	cam_log_set_level(xrisp_log_priv.log_level);

	return count;
}

static DEVICE_ATTR_RW(xrisp_log_level);

static struct attribute *xrisp_log_attrs[] = {
	&dev_attr_xrisp_log_level.attr,
	NULL,
};

static const struct attribute_group xrisp_log_attr_group = {
	.attrs = xrisp_log_attrs,
};

int cam_log_init(struct device *dev)
{
	int ret = 0;
	struct kobject *xrisp_kobj = &dev->kobj;

	xrisp_log_priv.dev = dev;

	ret = sysfs_create_group(xrisp_kobj, &xrisp_log_attr_group);
	if (ret)
		return ret;

	XRISP_PR_INFO("%s, create attribute success", __func__);

	return 0;
}

void cam_log_exit(void)
{
	if (xrisp_log_priv.dev)
		sysfs_remove_group(&xrisp_log_priv.dev->kobj, &xrisp_log_attr_group);

	xrisp_log_priv.dev = NULL;
}
