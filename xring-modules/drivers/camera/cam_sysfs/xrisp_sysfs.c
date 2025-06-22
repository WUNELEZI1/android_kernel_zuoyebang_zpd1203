// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "sysfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "sysfs", __func__, __LINE___

#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/kstrtox.h>
#include <media/v4l2-event.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_sysfs.h"
#include "xrisp_log.h"

DECLARE_COMPLETION(sysfs_init_done);
static struct xrisp_sysfs_data *xrisp_sysfs_priv_p;

static void xrisp_sys_send_sig(void)
{
	mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);
	if (xrisp_sysfs_priv_p->user_task)
		send_sig(SIGIO, xrisp_sysfs_priv_p->user_task, 0);
	mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);
}

static int xrisp_sysfs_send_sig_th(void *data)
{
	bool need_send_sig = false;

	while (!kthread_should_stop()) {
		mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);
		if (PROP_INDEX(xrisp_sysfs_priv_p->write_idx) !=
		    PROP_INDEX(xrisp_sysfs_priv_p->read_idx)) {
			need_send_sig = true;
		} else {
			need_send_sig = false;
			set_current_state(TASK_INTERRUPTIBLE);
			XRISP_PR_DEBUG("prop ringbuf empty");
		}
		mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);

		if (need_send_sig) {
			xrisp_sys_send_sig();
			XRISP_PR_DEBUG("need read prop");
			msleep(2000);
		} else {
			schedule();
		}
	}

	xrisp_sysfs_priv_p->send_sig_th = NULL;
	return 0;
}

static ssize_t xrisp_prop_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	u64 val;

	mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);

	if (PROP_INDEX(xrisp_sysfs_priv_p->write_idx) == PROP_INDEX(xrisp_sysfs_priv_p->read_idx)) {
		XRISP_PR_DEBUG("prop ringbuf empty");
		val = 0;
	} else {
		val = xrisp_sysfs_priv_p->prop_buf[xrisp_sysfs_priv_p->read_idx];
		xrisp_sysfs_priv_p->read_idx = PROP_INDEX(xrisp_sysfs_priv_p->read_idx + 1);
	}

	mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);

	len = snprintf(buf, sizeof(val), "%llu\n", val);
	XRISP_PR_DEBUG("read data %llu", val);

	return len;
}

static ssize_t xrisp_prop_store(struct device *dev, struct device_attribute *attr,
				     const char *buf, size_t count)
{
	u64 val;
	int full_try = 0;

	if (kstrtou64(buf, 0, &val)) {
		XRISP_PR_ERROR("get write val failed");
		return -EINVAL;
	}

	XRISP_PR_DEBUG("write data %llu", val);

	mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);

	while (PROP_INDEX(xrisp_sysfs_priv_p->write_idx + 1) ==
	       PROP_INDEX(xrisp_sysfs_priv_p->read_idx)) {
		XRISP_PR_INFO("prop ringbuf full");
		mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);

		if (full_try++ > 3)
			return -EINVAL;
		wake_up_process(xrisp_sysfs_priv_p->send_sig_th);
		msleep(1000);

		mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);
	}

	xrisp_sysfs_priv_p->prop_buf[xrisp_sysfs_priv_p->write_idx] = val;
	xrisp_sysfs_priv_p->write_idx = PROP_INDEX(xrisp_sysfs_priv_p->write_idx + 1);

	mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);
	wake_up_process(xrisp_sysfs_priv_p->send_sig_th);

	return count;
}

static DEVICE_ATTR_RW(xrisp_prop);

static struct attribute *xrisp_attrs[] = {
	&dev_attr_xrisp_prop.attr,
	NULL,
};

static const struct attribute_group xrisp_attr_group = {
	.attrs = xrisp_attrs,
};

void xrisp_sys_set_user_task(void)
{
	mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);
	xrisp_sysfs_priv_p->user_task = current;
	mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);
	XRISP_PR_DEBUG("user task set %s", current->comm);
}

void xrisp_sys_clear_user_task(void)
{
	mutex_lock(&xrisp_sysfs_priv_p->prop_mtx);
	xrisp_sysfs_priv_p->user_task = NULL;
	mutex_unlock(&xrisp_sysfs_priv_p->prop_mtx);
}

int xrisp_prop_ringbuf_init(void)
{
	memset(xrisp_sysfs_priv_p->prop_buf, 0, sizeof(u64) * PROP_RINGBUF_SIZE);
	mutex_init(&xrisp_sysfs_priv_p->prop_mtx);
	xrisp_sysfs_priv_p->read_idx = 0;
	xrisp_sysfs_priv_p->write_idx = 0;

	xrisp_sysfs_priv_p->send_sig_th =
		kthread_run(xrisp_sysfs_send_sig_th, NULL, "xrisp_prop_sig");
	if (IS_ERR(xrisp_sysfs_priv_p->send_sig_th)) {
		XRISP_PR_ERROR("create send_sig thread fail");
		mutex_destroy(&xrisp_sysfs_priv_p->prop_mtx);
		return -EINVAL;
	}
	return 0;
}

void xrisp_prop_ringbuf_deinit(void)
{
	kthread_stop(xrisp_sysfs_priv_p->send_sig_th);
	mutex_destroy(&xrisp_sysfs_priv_p->prop_mtx);
}

struct kobject *xrisp_get_isp_kobj(void)
{
	if (xrisp_sysfs_priv_p->kobj) {
		kobject_get(xrisp_sysfs_priv_p->kobj);
		return xrisp_sysfs_priv_p->kobj;
	} else
		return NULL;
}

void xrisp_put_isp_kobj(void)
{
	if (xrisp_sysfs_priv_p->kobj)
		kobject_put(xrisp_sysfs_priv_p->kobj);
}

int xrisp_wait_sysfs_init_done(void)
{
	unsigned long timeout;

	timeout = wait_for_completion_timeout(&sysfs_init_done, msecs_to_jiffies(500));
	if (timeout)
		return 0;

	XRISP_PR_ERROR("xrisp sysfs init timeout");
	return -ETIMEDOUT;
}

static void xrisp_sysfs_init_done(void)
{
	complete_all(&sysfs_init_done);
}

int cam_sysfs_init(struct device *dev, struct video_device *vdev)
{
	int ret = 0;

	xrisp_sysfs_priv_p = devm_kzalloc(dev, sizeof(struct xrisp_sysfs_data), GFP_KERNEL);
	if (!xrisp_sysfs_priv_p)
		return -ENOMEM;

	xrisp_sysfs_priv_p->dev = dev;
	xrisp_sysfs_priv_p->vdev = vdev;
	xrisp_sysfs_priv_p->kobj = &dev->kobj;
	ret = sysfs_create_link(kernel_kobj, xrisp_sysfs_priv_p->kobj, "xrisp");
	if (ret) {
		XRISP_PR_ERROR("sysfs_create_link fail");
		goto kfree_mem;
	}
	ret = sysfs_create_group(xrisp_sysfs_priv_p->kobj, &xrisp_attr_group);
	if (ret) {
		XRISP_PR_ERROR("sysfs_create_group fail");
		goto remove_link;
	}

	ret = xrisp_prop_ringbuf_init();
	if (ret)
		goto remove_group;

	ret = xrisp_class_init();
	if (ret) {
		XRISP_PR_ERROR("xrisp class init fail");
		goto deinit_prop;
	}
	xrisp_sysfs_init_done();
	XRISP_PR_INFO("xrisp sysfs init done");

	return 0;

deinit_prop:
	xrisp_prop_ringbuf_deinit();

remove_group:
	sysfs_remove_group(xrisp_sysfs_priv_p->kobj, &xrisp_attr_group);

remove_link:
	sysfs_remove_link(kernel_kobj, "xrisp");

kfree_mem:
	devm_kfree(dev, xrisp_sysfs_priv_p);
	xrisp_sysfs_priv_p = NULL;

	return ret;
}

void cam_sysfs_exit(void)
{
	struct kobject *xrisp_kobj;

	if (!xrisp_sysfs_priv_p)
		return;

	xrisp_prop_ringbuf_deinit();
	xrisp_class_exit();

	if (xrisp_sysfs_priv_p->kobj) {
		xrisp_kobj = xrisp_sysfs_priv_p->kobj;
		sysfs_remove_group(xrisp_kobj, &xrisp_attr_group);
		sysfs_remove_link(kernel_kobj, "xrisp");
	}
}
