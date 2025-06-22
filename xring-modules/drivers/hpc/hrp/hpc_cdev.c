// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/miscdevice.h>
#include <linux/compat.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <soc/xring/xr_tsens.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/atomic.h>

#include "hpc_internal.h"

struct hpc_tsens_info {
	int id;
	int temp;
};

#define HDEV_BOOT _IO('X', 0)

#define HDEV_QUREY_BOOT_STATUS _IO('X', 1)

#define HDEV_QUREY_TEMP _IOWR('X', 2, struct hpc_tsens_info)
#define TIMER_INTERVAL_MS	600000

#if IS_ENABLED(CONFIG_XRING_DEBUG)
int hpc_log_level = HPC_LOG_LEVEL_INFO;
module_param(hpc_log_level, int, 0644);
MODULE_PARM_DESC(hpc_log_level, "Control log level (0=none, 1=info, ...)");
#endif

struct hpc_cdev {
	struct platform_device *pdev;
	struct hpc_device *hdev;
	struct miscdevice miscdev;
};
struct proc_info {
	pid_t tgid;
	pid_t pid;
	atomic_t count;
	struct list_head list;
};

static struct {
	struct timer_list timer;
	atomic_t open_count;
	struct list_head proc_list;
	struct mutex list_mutex;
	dev_t dev_num;
} dev_ctx = {
	.open_count = ATOMIC_INIT(0),
	.proc_list = LIST_HEAD_INIT(dev_ctx.proc_list),
	.list_mutex = __MUTEX_INITIALIZER(dev_ctx.list_mutex),
};

static void timer_callback(struct timer_list *t)
{
	struct proc_info *entry;

	hpcerr("=== Timer report at %lld ns ===\n", ktime_get_real_ns());

	mutex_lock(&dev_ctx.list_mutex);
	list_for_each_entry(entry, &dev_ctx.proc_list, list) {
		hpcerr("Active process: tgid=%d, pid=%d\n", entry->tgid, entry->pid);
	}

	if (atomic_read(&dev_ctx.open_count) > 0)
		mod_timer(&dev_ctx.timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));

	mutex_unlock(&dev_ctx.list_mutex);
}

static int hcdev_open(struct inode *inode, struct file *filp)
{
	struct hpc_cdev *hcdev = container_of(filp->private_data, struct hpc_cdev, miscdev);
	int ret = 0;
	struct proc_info *info, *entry;
	bool exists = false;

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->tgid = current->tgid;
	info->pid = current->pid;
	atomic_set(&info->count, 1);

	mutex_lock(&dev_ctx.list_mutex);
	list_for_each_entry(entry, &dev_ctx.proc_list, list) {
		if (entry->tgid == info->tgid) {
			exists = true;
			break;
		}
	}

	if (!exists)
		list_add(&info->list, &dev_ctx.proc_list);
	else {
		kfree(info);
		info = NULL;
		atomic_inc(&entry->count);
	}

	if (atomic_inc_return(&dev_ctx.open_count) == 1) {
		timer_setup(&dev_ctx.timer, timer_callback, 0);
		mod_timer(&dev_ctx.timer, jiffies + msecs_to_jiffies(TIMER_INTERVAL_MS));
	}

	mutex_unlock(&dev_ctx.list_mutex);

	if (info)
		hpcdbg("New record: tgid=%d, pid=%d\n", info->tgid, info->pid);
	else
		hpcdbg("Duplicate record: tgid=%d, pid=%d\n", current->tgid, current->pid);

	filp->private_data = hcdev;
	return ret;
}

static int hcdev_get_tsens_info(void __user *argp)
{
	struct hpc_tsens_info tsens_info = {0};
	int ret;

	ret = copy_from_user(&tsens_info, argp, sizeof(struct hpc_tsens_info));
	if (ret != 0) {
		hpcerr("copy from user for tsens info failed: %d\n", ret);
		return ret;
	}

	ret = xr_tsens_read_temp(tsens_info.id, &tsens_info.temp);
	if (ret != 0) {
		hpcerr("call xr_tens_read_temp failed: %d\n", ret);
		return ret;
	}

	ret = copy_to_user(argp, &tsens_info, sizeof(struct hpc_tsens_info));
	if (ret != 0) {
		hpcerr("copy to user for tsens info failed: %d\n", ret);
		return ret;
	}

	return ret;
}

static long hcdev_ioctl(struct file *filp, unsigned int ioctl,
		unsigned long arg)
{
	struct hpc_cdev *hcdev = filp->private_data;
	struct hpc_device *hdev = hcdev->hdev;
	int ret = 0;
	void __user *argp = (void __user *)arg;

	switch (ioctl) {
	case HDEV_BOOT:
		hpcdbg("hpc boot start\n");
		ret = hdev_boot(hdev);
		if (ret != 0)
			hpcerr("hdev boot failed: %d\n", ret);

		hpcdbg("hpc boot end\n");
		break;
	case HDEV_QUREY_BOOT_STATUS:
		ret = atomic_read(&hdev->power);
		hpcdbg("hpc boot status is %d\n", ret);
		break;
	case HDEV_QUREY_TEMP:
		ret = hcdev_get_tsens_info(argp);
		hpcdbg("hpc get tsens status is %d\n", ret);
		break;
	default:
		hpcerr("Unsupported ioctl\n");
		ret = -EINVAL;
	}

	return ret;
}

static int hcdev_release(struct inode *inode, struct file *filp)
{
	int ret = 0;
	struct hpc_cdev *hcdev = filp->private_data;
	struct hpc_device *hdev = hcdev->hdev;
	struct proc_info *entry, *tmp;

	mutex_lock(&dev_ctx.list_mutex);
	if (atomic_dec_return(&dev_ctx.open_count) == 0) {
		list_for_each_entry_safe(entry, tmp, &dev_ctx.proc_list, list) {
			list_del(&entry->list);
			kfree(entry);
		}

		del_timer_sync(&dev_ctx.timer);
		hpcdbg("Timer stopped and list cleared\n");
	} else {
		list_for_each_entry_safe(entry, tmp, &dev_ctx.proc_list, list) {
			if (entry->tgid == current->tgid) {
				if (atomic_dec_return(&entry->count) == 0) {
					list_del(&entry->list);
					kfree(entry);
				}
				break;
			}
		}
	}
	mutex_unlock(&dev_ctx.list_mutex);

	hpcdbg("Release count: %d (tgid=%d, pid=%d)\n",
	      atomic_read(&dev_ctx.open_count), current->tgid, current->pid);

	hpcdbg("entry\n");
	ret = hdev_shutdown(hdev);
	if (ret != 0)
		hpcerr("hdev shutdown failed: %d\n", ret);

	hpcdbg("end\n");

	return ret;
}

static const struct file_operations hcdev_fops = {
	.open = hcdev_open,
	.unlocked_ioctl = hcdev_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
	.release = hcdev_release,
};

static int hcdev_miscdev_init(struct hpc_cdev *hcdev)
{
	struct miscdevice *miscdev = &hcdev->miscdev;
	int ret;

	miscdev->parent = &hcdev->pdev->dev;
	miscdev->minor = MISC_DYNAMIC_MINOR;
	miscdev->name = "hpc-cdev";
	miscdev->fops = &hcdev_fops;
	ret = misc_register(miscdev);
	if (ret != 0)
		hpcerr("register hpc cdev failed: %d\n", ret);

	return ret;
}

static void hcdev_miscdev_destroy(struct hpc_cdev *hcdev)
{
	misc_deregister(&hcdev->miscdev);
}

static int hcdev_probe(struct platform_device *pdev)
{
	struct hpc_cdev *hcdev = NULL;
	int ret;

	hpcinfo("entry\n");
	hcdev = devm_kzalloc(&pdev->dev, sizeof(*hcdev), GFP_KERNEL);
	if (!hcdev) {
		hpcerr("hpc cdev zalloc failed\n");
		return -ENOMEM;
	}

	hcdev->pdev = pdev;
	hcdev->hdev = dev_get_drvdata(pdev->dev.parent);
	init_waitqueue_head(&hcdev->hdev->cancel_all_wait);
	hcdev->hdev->cancel_all_flag = false;
	ret = hcdev_miscdev_init(hcdev);
	if (ret != 0)
		return ret;

	platform_set_drvdata(pdev, hcdev);

	hpcinfo("end\n");

	return 0;
}

static int hcdev_remove(struct platform_device *pdev)
{
	struct hpc_cdev *hcdev = platform_get_drvdata(pdev);
	struct proc_info *entry, *tmp;

	mutex_lock(&dev_ctx.list_mutex);
	list_for_each_entry_safe(entry, tmp, &dev_ctx.proc_list, list) {
		list_del(&entry->list);
		kfree(entry);
	}
	mutex_unlock(&dev_ctx.list_mutex);

	hpcinfo("entry\n");
	hcdev_miscdev_destroy(hcdev);
	hpcinfo("end\n");

	return 0;
}

static struct platform_driver hcdev_driver = {
	.probe = hcdev_probe,
	.remove = hcdev_remove,
	.driver = {
		.name = "hpc-cdev",
	},
};
module_platform_driver(hcdev_driver);

MODULE_AUTHOR("High Performance Computing Group");
MODULE_DESCRIPTION("HPC CDEV Driver");
MODULE_LICENSE("GPL v2");
