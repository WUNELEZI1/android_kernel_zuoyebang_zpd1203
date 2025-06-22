// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/ktime.h>
#include <linux/sched/clock.h>

#include "xrisp_common.h"
#include "xrisp_ipc.h"
#include "xrisp_mdr.h"
#include "xrisp_rproc.h"
#include "xrisp_ramlog.h"

#define DEVICE_NAME  "edr_device"
#define CLASS_NAME   "edr_class"
#define DEVICE_MAJOR 0
#define DEVICE_MINOR 0
#define DEVICE_COUNT 1

#define MDR_BUFFER_SIZE XRISP_RPROC_DFXMEM_SIZE

DECLARE_WAIT_QUEUE_HEAD(wq);

DECLARE_WAIT_QUEUE_HEAD(sync_wq);
static atomic_t sync_done = ATOMIC_INIT(0);

static struct edr_node *edr_data;
static dev_t edr_dev;
static struct cdev edr_cdev;
static struct class *edr_class;
static struct device *edr_device;

static bool g_edr_event_inited;

static uint64_t edr_stamp;
struct tm edr_time;
static u8 *mdr_buffer;
bool mdr_submit_control = true;

static int edr_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int edr_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t edr_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t ret = 0;

	while (edr_data->head == NULL) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(wq, edr_data->head != NULL))
			return -ERESTARTSYS;
	}

	if (copy_to_user(buf, edr_data->head, sizeof(struct edr_event)))
		ret = -EFAULT;
	else
		ret = sizeof(struct edr_event);

	edr_data->head = NULL;

	return ret;
}

struct edr_event new_event = { 0 };

static void submit_api(struct edr_event *event)
{
	if (event == NULL) {
		edr_err("event is null");
		return;
	}
	new_event.type = event->type;
	new_event.sub_type = event->sub_type;
	new_event.log_info = event->log_info;
	new_event.timestamp = event->timestamp;
	new_event.reset = event->reset;
	new_event.frame_id = event->frame_id;

	memset(new_event.logpath, '\0', sizeof(new_event.logpath));

#if EDR_DEBUG
	edr_debug("edr submit: ");
	edr_debug("type[%d]", new_event.type);
	edr_debug("subtype[%d]", new_event.sub_type);
	edr_debug("loginfo[%d]", new_event.log_info);
	edr_debug("timestamp[%llu]", new_event.timestamp);
	edr_debug("reset[%d]", new_event.reset);
	edr_debug("frame_id[%llu]", new_event.frame_id);
	edr_debug("logpath[%s][%lu]", new_event.logpath, sizeof(struct edr_event));
#endif
	spin_lock(&edr_data->lock);
	edr_data->head = &new_event;
	spin_unlock(&edr_data->lock);

	if (mdr_submit_control) {
		if (new_event.type == EDR_DVFS || new_event.type == EDR_FAULT_INJECT_ACK || new_event.type == EDR_POWER_ON || new_event.type == EDR_EXIT) {
			wake_up_interruptible(&wq);
			return;
		}
		//type/subtype translate to modid
		uint32_t modid;

		modid = xrisp_exception_to_mdr(new_event.type, new_event.sub_type);
		if (modid > MODID_ISP_EEPROM_CAM_PROBE_ERROR)
			return;
		//submit modid exception to mdr
		mdr_system_error(modid, 0, 0);
	} else {
		wake_up_interruptible(&wq);
		return;
	}
}

//for other modules use
void edr_drv_submit_api(uint16_t type, uint16_t subtype, uint32_t loginfo, uint8_t reset,
			uint64_t frameid)
{
	struct edr_event notify_data = { 0 };

	edr_stamp = local_clock() / 1000;

	notify_data.type = type;
	notify_data.sub_type = subtype;
	notify_data.log_info = loginfo;
	notify_data.timestamp = edr_stamp;
	notify_data.reset = reset;
	notify_data.frame_id = frameid;
	submit_api(&notify_data);
}
EXPORT_SYMBOL_GPL(edr_drv_submit_api);

static struct edr_event value = { 0 };

static void submit_api_sync(struct edr_event *event)
{
	if (event == NULL) {
		edr_err("event is null");
		return;
	}
	value.type = event->type;
	value.sub_type = event->sub_type;
	value.log_info = event->log_info;
	value.timestamp = event->timestamp;
	value.reset = event->reset;
	value.frame_id = event->frame_id;

	strscpy(value.logpath, event->logpath, sizeof(value.logpath) - 1);
	value.logpath[sizeof(value.logpath) - 1] = '\0';

	edr_info("type[%d]", value.type);
	edr_info("subtype[%d]", value.sub_type);
	edr_info("loginfo[%d]", value.log_info);
	edr_info("logpath[%s]", value.logpath);

	spin_lock(&edr_data->lock);
	edr_data->head = &value;
	spin_unlock(&edr_data->lock);

	wake_up_interruptible(&wq);
}
int edr_drv_submit_api_sync(uint16_t type, uint16_t subtype, uint32_t loginfo, char *path)
{
	struct edr_event data = { 0 };
	char sub_path[256] = { 0 };

	edr_stamp = local_clock() / 1000;
	data.type = type;
	data.sub_type = subtype;
	data.log_info = loginfo;
	data.timestamp = edr_stamp;
	data.frame_id = 0;
	data.reset = 0;

	if (path == NULL)
		return -1;

	strscpy(sub_path, path, sizeof(sub_path) - 1);
	int len = strlen(path);

	if (path[len - 1] == '/')
		path[len - 1] = '\0';

	char *last_slash = strrchr(path, '/');

	if (last_slash != NULL) {
		strscpy(data.logpath, last_slash + 1, sizeof(data.logpath) - 1);
		data.logpath[sizeof(data.logpath) - 1] = '\0';
	} else {
		strscpy(data.logpath, path, sizeof(data.logpath) - 1);
		data.logpath[sizeof(data.logpath) - 1] = '\0';
	}

	submit_api_sync(&data);
	// wait_event_interruptible_timeout wait for ack
	// if timeout continue, no block
	if (wait_event_interruptible_timeout(sync_wq, atomic_read(&sync_done) == 1,
					     msecs_to_jiffies(2000)) > 0)
		edr_info("log catch complete");
	else {
		//timeout
		edr_info("Timeout waiting for log catch");
		return -1;
	}
	atomic_set(&sync_done, 0);
	return 0;
}
EXPORT_SYMBOL_GPL(edr_drv_submit_api_sync);

#if EDR_DEBUG
static ssize_t edr_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct edr_event *event;
	struct EdrIpcAp2Isp data = { 0 };
	int ret;
	char message[20];

	if (copy_from_user(message, buf, count))
		return -EFAULT;

	edr_info("edr write");
	event = kmalloc(sizeof(struct edr_event), GFP_KERNEL);
	if (event == NULL)
		return -ENOMEM;

	edr_stamp = local_clock() / 1000;
	//edr_info("edr_stamp:%llu", edr_stamp);

	event->type = 2;
	event->sub_type = 3;
	event->log_info = message[0];
	//close mdr submit, echo 1 > /dev/edr_device
	if (event->log_info == 49)
		mdr_submit_control = false;
	//open mdr submit, echo 0 > /dev/edr_device
	if (event->log_info == 48)
		mdr_submit_control = true;
	event->timestamp = edr_stamp;
	event->reset = 0;

	submit_api(event);
	kfree(event);
	//test ap send msg to isp
	data.msgid = 1;
	data.opcode = 2;
	data.buf_info.size = 16384;
	data.buf_info.hwaddr = 0xffff00;

	ret = edr_ipc_send(&data, sizeof(struct EdrIpcAp2Isp));
	if (ret != 0)
		edr_err("edr ipc send err");
	return count;
}
#endif

static unsigned int edr_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;

	//edr_info("edr poll");
	poll_wait(filp, &wq, wait);

	spin_lock(&edr_data->lock);
	if (edr_data->head != NULL)
		mask |= POLLIN | POLLRDNORM;
	spin_unlock(&edr_data->lock);

	return mask;
}

static long edr_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct EdrIpcAp2Isp cmd_data;

	switch (cmd) {
	case EDR_CMD_COREDUMP:
		if (copy_from_user(&cmd_data, (void *)arg, sizeof(struct EdrIpcAp2Isp)))
			return -EFAULT;
#if EDR_DEBUG
		edr_debug("EdripcAp2Isp cmd data is:");
		edr_debug("msg_id[%llu]", cmd_data.msgid);
		edr_debug("opcode[0x%x]", cmd_data.opcode);
		edr_debug("size[%d]", cmd_data.buf_info.size);
		edr_debug("hwaddr[0x%llx]", cmd_data.buf_info.hwaddr);
#endif
		//send ipc to fw
		edr_ipc_send(&cmd_data, sizeof(struct EdrIpcAp2Isp));
		break;
	case EDR_CMD_TIMEZONE:
		if (copy_from_user(&cmd_data, (void *)arg, sizeof(struct EdrIpcAp2Isp)))
			return -EFAULT;
#if EDR_DEBUG
		edr_debug("EdripcAp2Isp cmd data is:");
		edr_debug("msg_id[%llu]", cmd_data.msgid);
		edr_debug("opcode[0x%x]", cmd_data.opcode);
		edr_debug("month[%d]", cmd_data.edrtimezone.month);
		edr_debug("day[%d]", cmd_data.edrtimezone.day);
		edr_debug("hour[%d]", cmd_data.edrtimezone.hour);
#endif
		edr_time.tm_mon = cmd_data.edrtimezone.month;
		edr_time.tm_mday = cmd_data.edrtimezone.day;
		edr_time.tm_hour = cmd_data.edrtimezone.hour;
		break;
	case EDR_CMD_EXIT:
		edr_drv_submit_api(EDR_EXIT, EDR_EXIT, EDR_EXIT, 0, 0);
		break;
	case EDR_CMD_LOGDONE:
		edr_info("edr log dump complete");
		atomic_set(&sync_done, 1);
		wake_up(&sync_wq);
		break;
	case EDR_CMD_FAULT_INJECT_CANCEL:
		if (copy_from_user(&cmd_data, (void *)arg, sizeof(struct EdrIpcAp2Isp)))
			return -EFAULT;
#if EDR_DEBUG
		edr_info("EdripcAp2Isp cmd data is:");
		edr_info("msg_id[%llu]", cmd_data.msgid);
		edr_info("opcode[0x%x]", cmd_data.opcode);
		edr_info("cmd[%d]", cmd_data.faultinject.cmd);
		edr_info("res[0x%x]", cmd_data.faultinject.res);
		edr_info("sensorid[%d]", cmd_data.faultinject.sensorid);
		edr_info("intefaceid[%d]", cmd_data.faultinject.interfaceid);
		edr_info("param[%s]", cmd_data.faultinject.param);
#endif
		//send ipc to fw
		edr_ipc_send(&cmd_data, sizeof(struct EdrIpcAp2Isp));
		break;
	case EDR_CMD_FW_LOG_LEVEL:
		if (copy_from_user(&cmd_data, (void *)arg, sizeof(struct EdrIpcAp2Isp)))
			return -EFAULT;
		ramlog_loglevel_set(cmd_data.loglevel);
		break;

#if IS_ENABLED(CONFIG_XRING_DEBUG)
	case EDR_CMD_DUMP_MDR_LOG:
	{
		loff_t pos = 0;
		struct file *filp = NULL;
		char *filename = NULL;

		filename = kmalloc(PATH_MAX, GFP_KERNEL);
		if (!filename)
			return -ENOMEM;

		if (copy_from_user(filename, (char __user *)arg, PATH_MAX)) {
			kfree(filename);
			return -EFAULT;
		}

		filp = filp_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		kfree(filename);
		if (IS_ERR(filp))
			return PTR_ERR(filp);

		ret = kernel_write(filp, mdr_buffer, MDR_BUFFER_SIZE, &pos);
		if (ret < 0) {
			filp_close(filp, NULL);
			return ret;
		} else if (ret != MDR_BUFFER_SIZE) {
			ret = -EIO;
		} else {
			ret = 0;
		}
		filp_close(filp, NULL);
		break;
	}
#endif

	default:
		ret = -ENOTTY;
	}
	return ret;
}

static const struct file_operations edr_fops = {
	.owner = THIS_MODULE,
	.open = edr_open,
	.release = edr_release,
	.read = edr_read,
#if EDR_DEBUG
	.write = edr_write,
#endif
	.poll = edr_poll,
	.unlocked_ioctl = edr_ioctl,
};

int xring_edr_event_init(void)
{
	int ret;

	edr_info("event init");
	if (g_edr_event_inited) {
		edr_info("edr event is already initialized! ");
		return 0;
	}

	edr_data = kmalloc(sizeof(struct edr_node), GFP_KERNEL);
	if (edr_data == NULL)
		return -ENOMEM;

	edr_data->head = NULL;
	spin_lock_init(&edr_data->lock);
	ret = alloc_chrdev_region(&edr_dev, DEVICE_MINOR, DEVICE_COUNT, DEVICE_NAME);
	if (ret < 0) {
		kfree(edr_data);
		return ret;
	}
	cdev_init(&edr_cdev, &edr_fops);
	edr_cdev.owner = THIS_MODULE;

	ret = cdev_add(&edr_cdev, edr_dev, DEVICE_COUNT);
	if (ret < 0) {
		unregister_chrdev_region(edr_dev, DEVICE_COUNT);
		kfree(edr_data);
		return ret;
	}

	// creaste device class
	edr_class = class_create(CLASS_NAME);
	if (IS_ERR(edr_class)) {
		edr_err("failed to create device class");
		cdev_del(&edr_cdev);
		unregister_chrdev_region(edr_dev, DEVICE_COUNT);
		kfree(edr_data);
		return PTR_ERR(edr_class);
	}

	// create device node
	edr_device = device_create(edr_class, NULL, edr_dev, NULL, DEVICE_NAME);
	if (IS_ERR(edr_device)) {
		edr_err("failed to create device node");
		class_destroy(edr_class);
		cdev_del(&edr_cdev);
		unregister_chrdev_region(edr_dev, DEVICE_COUNT);
		kfree(edr_data);
		return PTR_ERR(edr_device);
	}
	// edr ipc rcv register
	edr_ipc_rcv();
	// mdr register
	xrisp_mdr_exception_register();
	g_edr_event_inited = true;

	/* mdr remap */
	mdr_buffer = mdr_map(XRISP_RPROC_DFXMEM_ADDR, XRISP_RPROC_DFXMEM_SIZE);
	if (!mdr_buffer) {
		edr_err("ISP DFX mem mdr_map fail");
		return -1;
	}

	return 0;
}
void xring_edr_event_exit(void)
{
	edr_info("event exit");
	edr_ipc_exit();
	device_destroy(edr_class, edr_dev);
	class_destroy(edr_class);
	cdev_del(&edr_cdev);
	unregister_chrdev_region(edr_dev, DEVICE_COUNT);
	xrisp_mdr_exception_unregister();
	kfree(edr_data);
}

MODULE_AUTHOR("fang jun<fangjun3@xiaomi.com>");
MODULE_DESCRIPTION("x-ring edr event");
MODULE_LICENSE("GPL v2");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
