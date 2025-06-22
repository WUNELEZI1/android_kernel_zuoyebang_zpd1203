// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/genalloc.h>
#include <linux/slab.h>
#include "soc/xring/sensorhub/sensor_power/shub_sensor_power.h"
#include "soc/xring/sensorhub/ipc/shub_ipc_drv.h"

#define TYPE 0x7C
#define POWER_ON _IO(TYPE, 1)
#define POWER_OFF _IO(TYPE, 2)
#define DEVICE_NAME "shub_sensor_power_manager"

static int shub_sensor_power_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int shub_sensor_power_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t shub_sensor_power_read(struct file *fp, char __user *buffer,
				      size_t st, loff_t *pos)
{
	int err;
	ssize_t size;
	const char *power_names = sensor_power_get_names();

	size = strlen(power_names);
	if (st <= size)
		return -ENOBUFS;

	err = copy_to_user(buffer, power_names, size);
	if (err)
		return err;

	return size;
}

static long shub_sensor_power_ioctl(struct file *filp, unsigned int cmd,
				    unsigned long arg)
{
	int err = 0;
	int index;

	if (!arg)
		return -EINVAL;
	err = copy_from_user(&index, (void *)arg, sizeof(index));
	if (err)
		return err;

	pr_info("sensor power operation %d", index);
	if (index < 0)
		return -EINVAL;

	switch (cmd) {
	case POWER_ON:
		err = sensor_power_on(index);
		break;
	case POWER_OFF:
		err = sensor_power_off(index);
		break;
	default:
		break;
	}
	return err;
}

static const struct file_operations sensor_power_fops = {
	.owner = THIS_MODULE,
	.open = shub_sensor_power_open,
	.read = shub_sensor_power_read,
	.release = shub_sensor_power_release,
	.unlocked_ioctl = shub_sensor_power_ioctl,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &sensor_power_fops,
};

static int __init shub_sensor_power_init(void)
{
	int ret;

	ret = misc_register(&misc);
	if (ret == 0) {
		pr_info("shub_sensor_power register success\n");
		return 0;
	}
	pr_info("shub_sensor_power register err %d\n", ret);
	return ret;
}

static void __exit shub_sensor_power_exit(void)
{
	misc_deregister(&misc);
}
module_init(shub_sensor_power_init);
module_exit(shub_sensor_power_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("XRing Sensor Power Manager");
MODULE_LICENSE("GPL v2");
