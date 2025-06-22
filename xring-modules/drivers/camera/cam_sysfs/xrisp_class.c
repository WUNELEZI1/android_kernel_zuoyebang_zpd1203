// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sysfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sysfs", __func__, __LINE___

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kobject.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/kstrtox.h>
#include <linux/major.h>
#include <linux/list.h>

#include "xrisp_sysfs.h"
#include "xrisp_log.h"

#define XRISP_MINORS_MAX (100)
static DECLARE_BITMAP(xrisp_minors_bitmap, XRISP_MINORS_MAX);

static LIST_HEAD(xrisp_misc_list);
static DEFINE_MUTEX(xrisp_dev_mtx);

static struct class *xrisp_class;
static int xrisp_major;

int xrisp_class_dev_register(struct xrisp_miscdev_entry *misc)
{
	dev_t dev;
	int err = 0;
	int i = 0;

	if (!xrisp_class || xrisp_major < 0)
		return -EINVAL;

	INIT_LIST_HEAD(&misc->list);

	mutex_lock(&xrisp_dev_mtx);

	i = find_first_zero_bit(xrisp_minors_bitmap, XRISP_MINORS_MAX);

	if (i >= XRISP_MINORS_MAX) {
		err = -EBUSY;
		goto out;
	}

	misc->minor = i;
	set_bit(i, xrisp_minors_bitmap);
	dev = MKDEV(xrisp_major, misc->minor);

	misc->this_device = device_create_with_groups(xrisp_class, misc->parent, dev, misc,
						      misc->groups, "%s", misc->name);
	if (IS_ERR(misc->this_device)) {
		if (i < XRISP_MINORS_MAX)
			clear_bit(i, xrisp_minors_bitmap);

		err = PTR_ERR(misc->this_device);
		goto out;
	}

	list_add(&misc->list, &xrisp_misc_list);
	XRISP_PR_INFO("xrisp_class device %s register success", misc->name);

out:
	mutex_unlock(&xrisp_dev_mtx);
	return err;
}

void xrisp_class_dev_unregister(struct xrisp_miscdev_entry *misc)
{
	int i = misc->minor;

	if (WARN_ON(list_empty(&misc->list)))
		return;

	mutex_lock(&xrisp_dev_mtx);

	list_del(&misc->list);
	device_destroy(xrisp_class, MKDEV(xrisp_major, misc->minor));
	if (i < XRISP_MINORS_MAX)
		clear_bit(i, xrisp_minors_bitmap);

	mutex_unlock(&xrisp_dev_mtx);
}

static char *xrisp_devnode(const struct device *dev, umode_t *mode)
{
	struct xrisp_miscdev_entry *c = dev_get_drvdata(dev);

	if (mode && c->mode)
		*mode = c->mode;
	if (c->nodename)
		return kstrdup(c->nodename, GFP_KERNEL);

	return NULL;
}

static int xrisp_class_dev_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct xrisp_miscdev_entry *c;
	const struct file_operations *new_fops = NULL;
	int ret = 0;

	mutex_lock(&xrisp_dev_mtx);

	list_for_each_entry(c, &xrisp_misc_list, list) {
		if (c->minor == minor) {
			new_fops = fops_get(c->fops);
			break;
		}
	}

	if (!new_fops) {
		XRISP_PR_ERROR("can't find xrisp class device, minor=%d\n", minor);
		ret = -ENODEV;
		goto fail;
	}

	file->private_data = c;

	replace_fops(file, new_fops);
	if (file->f_op->open)
		ret = file->f_op->open(inode, file);
fail:
	mutex_unlock(&xrisp_dev_mtx);
	return ret;
}

static const struct file_operations xrisp_class_fops = {
	.owner = THIS_MODULE,
	.open = xrisp_class_dev_open,
	.llseek = noop_llseek,
};

int xrisp_class_init(void)
{
	struct class *class;

	class = class_create("xrisp_class");
	if (IS_ERR(class)) {
		XRISP_PR_ERROR("xrisp_class create failed");
		return PTR_ERR(class);
	}

	xrisp_major = register_chrdev(0, "xrisp", &xrisp_class_fops);
	if (xrisp_major < 0) {
		XRISP_PR_ERROR("xrisp major register failed");
		goto destroy_class;
	}

	class->devnode = xrisp_devnode;
	xrisp_class = class;

	XRISP_PR_INFO("xrisp class init success");
	return 0;

destroy_class:
	class_destroy(class);
	xrisp_class = NULL;
	return -ENODEV;
}

void xrisp_class_exit(void)
{
	struct xrisp_miscdev_entry *node;

	if (!xrisp_class)
		return;

	list_for_each_entry(node, &xrisp_misc_list, list)
		xrisp_class_dev_unregister(node);

	class_destroy(xrisp_class);
	unregister_chrdev(xrisp_major, "xrisp");
	xrisp_class = NULL;
	xrisp_major = 0;

	XRISP_PR_INFO("xrisp class exit finished");
}
