/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#ifndef _DPU_CHRDEV_H_
#define _DPU_CHRDEV_H_

#include <linux/cdev.h>

/* dpu char device create interface */
struct dksm_chrdev {
	const char *name;
	dev_t devno;
	struct device *chr_dev;
	const struct file_operations *fops;
	struct cdev cdev;
	void *drv_data;
};

/**
 * dksm_create_chrdev - create chrdev
 * when first chrdev is created, device_class will be created automatically.
 *
 * @chrdev: char device
 * Return 0 for success, else for failure
 */
int dksm_create_chrdev(struct dksm_chrdev *chrdev);

/**
 * dksm_destroy_chrdev - destroy chrdev
 * when last chrdev destroy, device_class will be destroy automatically.
 *
 * @chrdev: char device
 */
void dksm_destroy_chrdev(struct dksm_chrdev *chrdev);

#endif
