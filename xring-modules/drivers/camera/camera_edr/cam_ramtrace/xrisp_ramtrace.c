// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/sched/clock.h>
#include <soc/xring/xrisp/xrisp.h>
#include "cam_sysfs/xrisp_sysfs.h"

#include "xrisp_common.h"
#include "xrisp_ramtrace.h"
#include "xrisp_ipc.h"

/*
 * use left space of ramlog
 * 0x100B gap between ramlog and ram trace
 */
#define RAM_TRACE_BUF_OFFSET    0x600000
#define RAM_TRACE_BUF_SIZE      0x400000

#define RAM_TRACE_STOP	0
#define RAM_TRACE_START	1

void *ramtrace_buf;

static ssize_t xrisp_ramtrace_read(struct file *file, char __user *buf,
		size_t count, loff_t *ppos)
{
	return 0;
}

static int xrisp_ramtrace_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int xrisp_ramtrace_release(struct inode *inode, struct file *file)
{
	return 0;
}

const struct file_operations xrisp_ramtrace_fops = {
	.open = xrisp_ramtrace_open,
	.read = xrisp_ramtrace_read,
	.release = xrisp_ramtrace_release,
};

static int proc_ramtrace_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", (char *)ramtrace_buf);
	return 0;
}

static int proc_ramtrace_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_ramtrace_show, NULL);
}

static const struct proc_ops ramtrace_proc_ops = {
	.proc_flags	= PROC_ENTRY_PERMANENT,
	.proc_read	= seq_read,
	.proc_open	= proc_ramtrace_open,
	.proc_release	= single_release,
	.proc_lseek	= seq_lseek,
};

static int __init proc_ramtrace_init(void)
{
	proc_create("xrisp_ramtrace", 0400, NULL, &ramtrace_proc_ops);
	return 0;
}

static int proc_ramtrace_exit(void)
{
	remove_proc_entry("xrisp_ramtrace", NULL);
	return 0;
}

static int ramtrace_ctl;

static void ramtrace_ctl_handler(int ctl)
{
	struct EdrIpcAp2Isp msg = {0};

	if (ctl < 0)
		ramtrace_err("invalid ramtrace_ctl value\n");
	else if (ctl == 0) {
		/* send stop info to FW */
		ramtrace_info("ramtrace stop, send cmd to FW\n");
		msg.msgid = 1;
		msg.opcode = EDR_MSG_OPCODE_RAMTRACE_STOP;
		edr_ipc_send((void *)&msg, sizeof(struct edr_event));

	} else {
		/* send start info to FW */
		ramtrace_info("ramtrace start, send cmd to FW\n");
		msg.msgid = 1;
		msg.opcode = EDR_MSG_OPCODE_RAMTRACE_START;
		edr_ipc_send((void *)&msg, sizeof(struct edr_event));
	}
}

static ssize_t ramtrace_ctl_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%d\n", ramtrace_ctl);
}

static ssize_t ramtrace_ctl_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	int err, value;

	err = kstrtoint(buf, 0, &value);
	if (err)
		return err;

	if (value < 0)
		return -EINVAL;

	ramtrace_ctl = value;
	ramtrace_ctl_handler(ramtrace_ctl);

	return count;
}

static struct device_attribute xrisp_ramtrace_ctl_attr = {
	.show = ramtrace_ctl_show,
	.store = ramtrace_ctl_store,
	.attr.mode = 0644,
	.attr.name = "xrisp_ramtrace_ctl",
};

static struct xrisp_miscdev_entry ramtrace_misc = {
	.name = "xrisp_ramtrace",
	.fops = &xrisp_ramtrace_fops,
};

static struct kobject *isp_kobj;
int xring_edr_ramtrace_init(void)
{
	int ret = 0;

	ramtrace_info("ramtrace init");

	/*
	 * ramtrace_buf init by ramlog driver
	 * this is temp solution
	 */
	ramtrace_info("ramtrace addr:0x%pK, size:0x%x\n", ramtrace_buf,
		      RAM_TRACE_BUF_SIZE);
	ret = xrisp_wait_sysfs_init_done();
	if (ret)
		return ret;

	ret = xrisp_class_dev_register(&ramtrace_misc);
	if (ret) {
		ramtrace_err("register misc device fail\n");
		return -EINVAL;
	}

	isp_kobj = xrisp_get_isp_kobj();
	if (!isp_kobj) {
		ramtrace_err("can't get isp kobj\n");
		ret = -EINVAL;
		goto unregister_chrdev;
	}

	ret = sysfs_create_file(isp_kobj, &(xrisp_ramtrace_ctl_attr.attr));
	if (ret)
		goto put_isp_kobj;

	proc_ramtrace_init();
	ramtrace_info("ramtrace init ok");
	return 0;

put_isp_kobj:
	xrisp_put_isp_kobj();
	isp_kobj = NULL;
unregister_chrdev:
	xrisp_class_dev_unregister(&ramtrace_misc);
	return ret;
}

void xring_edr_ramtrace_exit(void)
{
	if (!isp_kobj)
		return;

	proc_ramtrace_exit();
	sysfs_remove_file(isp_kobj, &(xrisp_ramtrace_ctl_attr.attr));
	xrisp_put_isp_kobj();
	isp_kobj = NULL;
	xrisp_class_dev_unregister(&ramtrace_misc);

	ramtrace_info("ramtrace exit");
}

MODULE_AUTHOR("Qiang Wu <wuqiang9@xiaomi.com>");
MODULE_DESCRIPTION("x-ring isp ramtrace");
MODULE_LICENSE("GPL v2");
