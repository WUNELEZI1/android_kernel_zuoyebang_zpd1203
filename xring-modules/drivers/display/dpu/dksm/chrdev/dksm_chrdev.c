// SPDX-License-Identifier: GPL-2.0
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

#include <linux/module.h>
#include <linux/kref.h>

#include "dpu_log.h"
#include "dksm_chrdev.h"

#define DPU_DEV_CLASS_NAME "graphic"

struct dpu_chr_class_ctx {
	struct class *chr_class;
	struct kref refcount;
};

static void dpu_chr_class_ctx_release(struct kref *refcount)
{
	struct dpu_chr_class_ctx *ctx = container_of(refcount,
			struct dpu_chr_class_ctx, refcount);

	if (ctx->chr_class) {
		class_destroy(ctx->chr_class);
		ctx->chr_class = NULL;
	}

	kfree(ctx);
}

static void dpu_chr_class_get(struct kref *refcount)
{
	DPU_INFO("refcount:%u", refcount_read(&refcount->refcount));
	kref_get(refcount);
}

static void dpu_chr_class_put(struct kref *refcount)
{
	DPU_INFO("refcount:%u", refcount_read(&refcount->refcount));
	kref_put(refcount, dpu_chr_class_ctx_release);
}

static struct dpu_chr_class_ctx *dpu_chr_class_ctx_get(void)
{
	static struct dpu_chr_class_ctx *dpu_chr_class;

	if (dpu_chr_class) {
		DPU_INFO("has inited\n");
		return dpu_chr_class;
	}

	dpu_chr_class = kzalloc(sizeof(*dpu_chr_class), GFP_KERNEL);
	if (!dpu_chr_class) {
		DPU_ERROR("alloc dpu_chr_class failed\n");
		return ERR_PTR(-ENOMEM);
	}

	dpu_chr_class->chr_class = class_create(DPU_DEV_CLASS_NAME);
	if (IS_ERR_OR_NULL(dpu_chr_class->chr_class)) {
		DPU_ERROR("class_create fail!, name=%s\n", DPU_DEV_CLASS_NAME);
		kfree(dpu_chr_class);
		return ERR_PTR(-ENOENT);
	}

	kref_init(&dpu_chr_class->refcount);

	return dpu_chr_class;
}

int dksm_create_chrdev(struct dksm_chrdev *chrdev)
{
	struct dpu_chr_class_ctx *dpu_chr_class = dpu_chr_class_ctx_get();
	int ret;

	if (!dpu_chr_class)
		return -EINVAL;

	if (!chrdev)
		return -EINVAL;

	if (alloc_chrdev_region(&chrdev->devno, 0, 1, chrdev->name)) {
		DPU_ERROR("alloc_chrdev_region fail!, name=%s", chrdev->name);
		return -EINVAL;
	}

	cdev_init(&chrdev->cdev, chrdev->fops);
	chrdev->cdev.owner = THIS_MODULE;
	if (cdev_add(&chrdev->cdev, chrdev->devno, 1)) {
		DPU_ERROR("cdev_add fail!, name=%s", chrdev->name);
		ret = -ENOENT;
		goto err_cdev_add;
	}

	chrdev->chr_dev = device_create(dpu_chr_class->chr_class, NULL,
			chrdev->devno, chrdev->drv_data, chrdev->name);
	if (IS_ERR_OR_NULL(chrdev->chr_dev)) {
		DPU_ERROR("device_create fail!, name=%s", chrdev->name);
		ret = PTR_ERR(chrdev->chr_dev);
		goto err_create_device;
	}

	dpu_chr_class_get(&dpu_chr_class->refcount);

	DPU_INFO("succ! major=%d, minor=%d, name=%s",
		MAJOR(chrdev->devno), MINOR(chrdev->devno), chrdev->name);

	return 0;

err_create_device:
	cdev_del(&chrdev->cdev);
err_cdev_add:
	unregister_chrdev_region(chrdev->devno, 1);

	return ret;
}

void dksm_destroy_chrdev(struct dksm_chrdev *chrdev)
{
	struct dpu_chr_class_ctx *dpu_chr_class = dpu_chr_class_ctx_get();

	if (!dpu_chr_class)
		return;

	if (!chrdev)
		return;

	device_destroy(dpu_chr_class->chr_class, chrdev->devno);
	cdev_del(&chrdev->cdev);
	unregister_chrdev_region(chrdev->devno, 1);
	dpu_chr_class_put(&dpu_chr_class->refcount);
}
