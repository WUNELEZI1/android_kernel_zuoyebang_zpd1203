// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include "app_router_multiplex/inc/shub_app_router_multiplex.h"
#include <dt-bindings/xring/platform-specific/ipc_resource.h>
#include <dt-bindings/xring/platform-specific/ipc_tags_sh_ap.h>
#include <soc/xring/sensorhub/shub_notifier.h>
#include <soc/xring/sensorhub/shub_boot_prepare.h>
#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include "soc/xring/sensorhub/sensor_power/shub_sensor_power.h"

#define MERGE_VC_ID_TAG(VC_ID, TAG_ID) (((VC_ID) << 16) | (TAG_ID))
#define CAMERA_INDEX (1)
#define TYPE 0x7A
#define GET_CLIENT_LEFT _IO(TYPE, 1)
#define GET_RX_BUF_SIZE _IO(TYPE, 2)
#define GET_TX_BUF_SIZE _IO(TYPE, 3)
#define GET_PORT_MAX_LEN _IO(TYPE, 4)
#define APT_ACUQIRE _IO(TYPE, 5)

struct app_router_s {
	struct app_router_multiplex *service;
	struct cdev cdev;
};

struct out_warpper {
	char *buffer;
	size_t st;
	uint32_t payload;
};

static struct class *dev_class;
static int major;
static int minor;
static dev_t dev;
struct device *device;
static const char name[] = "app_router_hal";
static struct app_router_s *app_router;
static DEFINE_MUTEX(g_app_router_lock);

static int sh_shub_app_router_cb(struct notifier_block *nb,
				 unsigned long action, void *data)
{
	if (action == ACTION_SH_RESET) {
		pr_info("%s reset receive\n", __func__);
		atomic_set(&app_router->service->sensor_framework_states,
			   STATES_NOT_READY);
#if IS_ENABLED(CONFIG_XRING_SH_APP_ROUTER_RESET_SENSOR_POWER)
		sensor_power_off_all();
		sensor_power_on_all();
#endif
	} else
		pr_warn("%s unknown action[%lu]\n", __func__, action);
	return NOTIFY_DONE;
}

static struct notifier_block sh_reset_notifier = {
	.notifier_call = sh_shub_app_router_cb,
};

static int app_router_open(struct inode *inode, struct file *fp)
{
	pr_info("shub app router open\n");
	fp->private_data = NULL;
	return 0;
}

static int app_router_open_lock(struct inode *inode, struct file *fp)
{
	int res;

	mutex_lock(&g_app_router_lock);
	res = app_router_open(inode, fp);
	mutex_unlock(&g_app_router_lock);

	return res;
}

static int app_router_acquire(struct file *fp, uint32_t index)
{
	int err;

	fp->private_data = kmalloc(sizeof(uint32_t), GFP_KERNEL);
	if (!fp->private_data)
		return -ENOMEM;

	err = app_router_multiplex_reg_hal(app_router->service, index);
	if (err) {
		kfree(fp->private_data);
		fp->private_data = NULL;
		return err;
	}
	*(uint32_t *)fp->private_data = index;
	pr_info("open index:%u\n", index);
	return 0;
}

static uint32_t copy_data(uint8_t *first_clip, uint32_t first_len,
			  uint8_t *second_clip, uint32_t second_len, void *arg)
{
	uint32_t all_len = 0;
	uint32_t i;
	uint32_t tmp;
	uint8_t *r_current = first_clip;
	uint32_t len = first_len;

	struct out_warpper *warpper = (struct out_warpper *)arg;
	struct adt_packet *packet;
	int clip;
	int err = 0;

	for (clip = 0; clip < 2; clip++) {
		for (i = 0; i < len;) {
			packet = (struct adt_packet *)r_current;
			if (packet->type == DISCARD_PACKET)
				break;
			tmp = sizeof(struct adt_packet) + packet->data_len;
			all_len += tmp;
			i += tmp;
			r_current += tmp;
		}
		if (all_len) {
			err = copy_to_user(warpper->buffer + warpper->payload,
					   r_current - all_len, all_len);
			if (err && clip)
				return first_len;
			else if (err)
				return 0;
		}
		warpper->payload += all_len;
		len = second_len;
		r_current = second_clip;
		all_len = 0;
	}
	return first_len + second_len;
}

static ssize_t app_router_read(struct file *fp, char __user *buffer, size_t st,
			       loff_t *pos)
{
	uint32_t index;
	struct out_warpper warpper = { .buffer = buffer,
				       .st = st,
				       .payload = 0 };

	if (!fp->private_data)
		return -EFAULT;
	if (st < app_router->service->service->rx->size)
		return -EINVAL;
	index = *(uint32_t *)fp->private_data;

	if (buffer_empty(app_router->service->buffers[index]))
		return 0;
	buffer_read(app_router->service->buffers[index], copy_data,
		    (void *)&warpper);
	return warpper.payload;
}

static ssize_t app_router_read_lock(struct file *fp, char __user *buffer,
				    size_t st, loff_t *pos)
{
	ssize_t res;

	mutex_lock(&g_app_router_lock);
	res = app_router_read(fp, buffer, st, pos);
	mutex_unlock(&g_app_router_lock);

	return res;
}

static ssize_t app_router_write(struct file *fp, const char __user *buffer,
				size_t write_size, loff_t *pos)
{
	struct msg_meta meta;
	int err;
	uint32_t index;
	struct private_data_s *private_data;
	int written_size;

	if (!is_shub_boot_done())
		return -EAGAIN;
	if (!fp->private_data)
		return -EFAULT;
	if (write_size != sizeof(meta))
		return -EINVAL;
	if (!buffer)
		return -EINVAL;

	if (atomic_read(&app_router->service->sensor_framework_states) !=
	    STATES_READY)
		return -EAGAIN;

	index = *(uint32_t *)fp->private_data;
	private_data = app_router->service->private_datas[index];
	err = copy_from_user(&meta, buffer, sizeof(meta));
	if (err)
		return err;
	if (!meta.msg)
		return -EINVAL;

	if (meta.msg_len + sizeof(struct adt_packet) >
	    app_router->service->service->tx->size)
		return -EINVAL;

	err = copy_from_user(private_data->buffer, meta.msg, meta.msg_len);
	if (err)
		return err;
	meta.msg = private_data->buffer;

	written_size = app_router_multiplex_send(app_router->service, &meta);

	return written_size <= 0 ? written_size : sizeof(meta);
}

static ssize_t app_router_write_lock(struct file *fp, const char __user *buffer,
				     size_t write_size, loff_t *pos)
{
	ssize_t res;

	mutex_lock(&g_app_router_lock);
	res = app_router_write(fp, buffer, write_size, pos);
	mutex_unlock(&g_app_router_lock);

	return res;
}

static int app_router_release(struct inode *inode, struct file *fp)
{
	uint32_t index;

	pr_info("shub app router release\n");
	if (!fp->private_data)
		return 0;

	index = *(uint32_t *)fp->private_data;
	if (index != CAMERA_INDEX)
		app_router_multiplex_notify_exit(app_router->service, index);
	app_router_multiplex_unreg_hal(app_router->service, index);
	kfree(fp->private_data);
	fp->private_data = NULL;
	pr_info("shub app router release done\n");
	return 0;
}

static int app_router_release_lock(struct inode *inode, struct file *fp)
{
	int res;

	mutex_lock(&g_app_router_lock);
	res = app_router_release(inode, fp);
	mutex_unlock(&g_app_router_lock);

	return res;
}

int app_router_notify_exit(unsigned int index)
{
	return app_router_multiplex_notify_exit(app_router->service, index);
}
EXPORT_SYMBOL(app_router_notify_exit);

static long app_router_ioctl(struct file *fp, unsigned int cmd,
			     unsigned long arg)
{
	int left = 0;
	int err = 0;
	uint32_t len;
	uint32_t index;

	if (!arg)
		return -EINVAL;

	switch (cmd) {
	case GET_CLIENT_LEFT:
		left = app_router_multiplex_client_left(app_router->service);
		err = copy_to_user((void *)arg, &left, sizeof(left));
		break;
	case GET_RX_BUF_SIZE:
		err = copy_to_user((void *)arg,
				   &app_router->service->service->rx->size,
				   sizeof(uint32_t));
		break;
	case GET_TX_BUF_SIZE:
		len = app_router->service->service->tx->size -
		      sizeof(struct adt_packet);
		err = copy_to_user((void *)arg, &len, sizeof(uint32_t));
		break;
	case GET_PORT_MAX_LEN:
		err = copy_to_user((void *)arg,
				   &app_router->service->service->port->max_len,
				   sizeof(uint32_t));
		break;
	case APT_ACUQIRE:
		err = copy_from_user(&index, (void *)arg, sizeof(uint32_t));
		if (err)
			break;
		err = app_router_acquire(fp, index);
		break;
	default:
		break;
	}
	return err;
}

static long app_router_ioctl_lock(struct file *fp, unsigned int cmd,
				  unsigned long arg)
{
	long res;

	mutex_lock(&g_app_router_lock);
	res = app_router_ioctl(fp, cmd, arg);
	mutex_unlock(&g_app_router_lock);

	return res;
}

static unsigned int app_router_poll(struct file *fp, poll_table *wait)
{
	int mask = 0;
	uint32_t index;
	struct private_data_s *private_data;

	if (unlikely(!fp->private_data))
		return -EFAULT;
	index = *(uint32_t *)fp->private_data;
	private_data = app_router->service->private_datas[index];
	if (unlikely(!private_data))
		return -EFAULT;

	poll_wait(fp, &private_data->ready, wait);
	if (unlikely(private_data->sensor_reset)) {
		mask |= POLLPRI;
		private_data->sensor_reset = false;
	}
	if (!buffer_empty(app_router->service->buffers[index]))
		mask |= POLLIN | POLLRDNORM;
	return mask;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = app_router_open_lock,
	.read = app_router_read_lock,
	.write = app_router_write_lock,
	.release = app_router_release_lock,
	.unlocked_ioctl = app_router_ioctl_lock,
	.poll = app_router_poll,
};

static int shub_app_router_init(void)
{
	int err = 0;
	struct port_ops *ipc_port = get_ipc_port();
	const uint32_t rx_buf_size = ipc_port->max_len * 2;
	const uint32_t tx_buf_size = ipc_port->max_len;
	const int ipc_channel = MERGE_VC_ID_TAG(IPC_VC_AO_NS_SH_AP_DATA_TRANS,
						TAG_SH_SENSOR_FRAMEWORK);

	err = alloc_chrdev_region(&dev, minor, 1, name);
	major = MAJOR(dev);

	if (err < 0) {
		pr_err("register dev error!\n");
		return err;
	}

	app_router = kzalloc(sizeof(struct app_router_s), GFP_KERNEL);
	if (!app_router) {
		err = -ENOMEM;
		goto fail;
	}

	err = app_router_multiplex_init(&app_router->service, ipc_port,
					tx_buf_size, rx_buf_size, ipc_channel);
	if (err)
		goto fail;
	err = app_router_multiplex_add_monitor(app_router->service,
					       ipc_channel);
	if (err)
		goto add_monitor_fail;
	err = sh_reset_notifier_register(&sh_reset_notifier);
	if (err)
		goto add_monitor_fail;

	cdev_init(&app_router->cdev, &fops);
	app_router->cdev.owner = THIS_MODULE;
	app_router->cdev.ops = &fops;

	err = cdev_add(&app_router->cdev, dev, 1);
	if (err) {
		pr_err("ERROR: cdev_add\n");
		goto dev_add_fail;
	}
	dev_class = class_create(name);

	if (IS_ERR(dev_class)) {
		pr_err("Unable to class_create\n");
		goto class_create_fail;
	}
	device = device_create(dev_class, NULL, dev, NULL, name);
	if (IS_ERR(device)) {
		pr_err("Unable to device_create\n");
		goto device_create_fail;
	}

	pr_info("%s init, major = %d\n", name, major);
	return err;

device_create_fail:
	class_destroy(dev_class);

class_create_fail:
	cdev_del(&app_router->cdev);

dev_add_fail:
	sh_reset_notifier_unregister(&sh_reset_notifier);

add_monitor_fail:
	app_router_multiplex_deinit(app_router->service);

fail:
	kfree(app_router);
	unregister_chrdev_region(dev, 1);
	return err;
}

static void shub_app_router_exit(void)
{
	const int ipc_channel = MERGE_VC_ID_TAG(IPC_VC_AO_NS_SH_AP_DATA_TRANS,
						TAG_SH_SENSOR_FRAMEWORK);

	sh_reset_notifier_unregister(&sh_reset_notifier);
	app_router_multiplex_del_monitor(app_router->service, ipc_channel);
	app_router_multiplex_deinit(app_router->service);
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&app_router->cdev);
	kfree(app_router);
	app_router = NULL;
	unregister_chrdev_region(dev, 1);

	pr_info("%s exit!\n", name);
}

module_init(shub_app_router_init);
module_exit(shub_app_router_exit);

MODULE_AUTHOR("X-Ring technologies Inc");
MODULE_DESCRIPTION("XRing App Router Hal Entry");
MODULE_LICENSE("GPL v2");
