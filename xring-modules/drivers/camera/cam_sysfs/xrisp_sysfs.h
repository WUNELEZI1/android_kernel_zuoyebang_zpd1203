/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_ISP_SYSFS_H_
#define _XR_ISP_SYSFS_H_

#include <linux/device.h>

#define PROP_RINGBUF_SIZE (1 << 5)
#define PROP_COUNT_MASK	  (PROP_RINGBUF_SIZE - 1)
#define PROP_INDEX(idx)	  (idx & PROP_COUNT_MASK)

struct xrisp_sysfs_data {
	u64 prop_buf[PROP_RINGBUF_SIZE];
	u64 read_idx;
	u64 write_idx;
	struct mutex prop_mtx;
	struct task_struct *send_sig_th;
	struct task_struct *user_task;
	struct device *dev;
	struct video_device *vdev;
	struct kobject *kobj;
};

struct xrisp_miscdev_entry {
	int minor;
	const char *name;
	const struct file_operations *fops;
	struct list_head list;
	struct device *parent;
	struct device *this_device;
	const struct attribute_group **groups;
	const char *nodename;
	umode_t mode;
};

void xrisp_sys_set_user_task(void);
void xrisp_sys_clear_user_task(void);

int cam_sysfs_init(struct device *dev, struct video_device *vdev);
void cam_sysfs_exit(void);

struct kobject *xrisp_get_isp_kobj(void);
void xrisp_put_isp_kobj(void);
int xrisp_wait_sysfs_init_done(void);

int xrisp_class_dev_register(struct xrisp_miscdev_entry *misc);
void xrisp_class_dev_unregister(struct xrisp_miscdev_entry *misc);

int xrisp_class_init(void);
void xrisp_class_exit(void);

#endif /* _XR_ISP_SYSFS_H_ */
