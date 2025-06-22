// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)	"[xr_dfx][bootup]:%s:%d " fmt, __func__, __LINE__

#include <linux/string.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <soc/xring/dfx_bootup.h>

#define BOOTUP_PROC_NAME "bootup_keypoint"
#define BOOTUP_LEN_MAX 4

static ssize_t bootup_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *f_pos);
static int bootup_stage1_init(void);
static void bootup_stage1_exit(void);

static ssize_t bootup_proc_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *f_pos)
{
	char tmp[BOOTUP_LEN_MAX] = {0};
	u32 data;
	int ret;

	if (count > BOOTUP_LEN_MAX) {
		pr_err("count: %lu too large", count);
		return -EINVAL;
	}

	if (copy_from_user(tmp, buffer, count)) {
		pr_err("Fail to copy from user to kernel");
		return -EFAULT;
	}

	if (kstrtouint(tmp, 10, &data) < 0) {
		pr_err("Fail to sscanf str");
		return -EFAULT;
	}

	ret = set_bootup_status(data);
	if (ret) {
		pr_err("[%u] Fail, err %d", data, ret);
		return ret;
	}
	return count;
}


static struct proc_ops bootup_proc_fops = {
	.proc_write	= bootup_proc_write,
};


static int boot_up_proc_init(void)
{
	struct proc_dir_entry *file;

	file = proc_create(BOOTUP_PROC_NAME, 0640, NULL, &bootup_proc_fops);
	if (!file) {
		pr_err("proc_create failed!\n");
		return -ENOMEM;
	}
	pr_info(" ==> success\n");
	return 0;
}

static void bootup_proc_exit(void)
{
	remove_proc_entry(BOOTUP_PROC_NAME, NULL);
	pr_info("--> success");
}

static int __init bootup_stage1_init(void)
{
	int ret;

	ret = bootup_keypoint_addr_init();
	if (ret) {
		pr_err("init keypoint addr failed!\n");
		return -1;
	}
	ret = set_bootup_status(STAGE_KERNEL_STAGE_ONE_START);
	if (ret) {
		pr_err("set_bootup_status failed!\n");
		return ret;
	}
	ret = boot_up_proc_init();
	if (ret) {
		pr_err("init proc failed!\n");
		return -1;
	}

	return 0;
}

static void __exit bootup_stage1_exit(void)
{
	bootup_proc_exit();
	pr_info("remove bootup_stage1.ko");
}

module_init(bootup_stage1_init);
module_exit(bootup_stage1_exit);

MODULE_LICENSE("GPL");
