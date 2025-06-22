// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rpmsg_ping", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rpmsg_ping", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include "xrisp_debug.h"
#include "xrisp_rproc.h"
#include "xrisp_log.h"


#define RPMSG_PING_EPT_NAME		"rpmsg-ping"
#define RPMSG_PING_SEND			1
#define RPMSG_PING_SEND_CHECK		2
#define RPMSG_PING_SEND_NOACK		3
#define RPMSG_PING_SEND_ACK		4
#define RPMSG_PING_ACK			5
#define RPMSG_PING_CHECK_DATA		0xee

struct rpmsg_ping_ept {
	struct rpmsg_endpoint *ept;
	struct completion ping_complete;
	struct dentry *dentry;
};

struct rpmsg_ping_msg_s {
	uint32_t cmd;
	uint32_t len;
	uint64_t cookie;
	uint8_t data[];
};

struct rpmsg_ping_s {
	int times;
	int len;
	int ack;
	int sleep; /* unit: ms */
};

static int rpmsg_ping_ept_cb(struct rpmsg_device *rpdev, void *data, int len,
			     void *priv, u32 src)
{
	struct rpmsg_ping_msg_s *msg = data;
	struct rpmsg_ping_ept *ping_ept = dev_get_drvdata(&rpdev->dev);

	if (msg->cmd == RPMSG_PING_SEND) {
		msg->cmd = RPMSG_PING_ACK;
		rpmsg_send(rpdev->ept, msg, len);
	} else if (msg->cmd == RPMSG_PING_ACK) {
		complete(&ping_ept->ping_complete);
	} else if (msg->cmd == RPMSG_PING_SEND_CHECK) {
		size_t data_len;
		size_t i;

		data_len = msg->len - sizeof(struct rpmsg_ping_msg_s);
		for (i = 0; i < data_len; i++) {
			if (msg->data[i] != RPMSG_PING_CHECK_DATA) {
				XRISP_PR_ERROR("rptun ping remote receive data error!\n");
				return -EINVAL;
			}

			msg->data[i] = 0;
		}

		msg->cmd = RPMSG_PING_ACK;
		rpmsg_send(rpdev->ept, msg, len);
	} else if (msg->cmd == RPMSG_PING_SEND_ACK) {
		msg->cmd = RPMSG_PING_ACK;
		rpmsg_send(rpdev->ept, msg, sizeof(*msg));
	}

	return 0;
}

static int rpmsg_ping_once(struct rpmsg_endpoint *ept,
			   int len, int ack)
{
	int ret = 0;
	struct rpmsg_ping_msg_s *msg;
	int data_len;
	struct rpmsg_ping_ept *ping_ept = dev_get_drvdata(&ept->rpdev->dev);

	data_len = len > sizeof(*msg) ? len - sizeof(*msg) : 1;
	msg = kzalloc(sizeof(*msg) + data_len, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	msg->len = sizeof(*msg) + data_len;

	if (ack) {
		if (ack == 1)
			msg->cmd = RPMSG_PING_SEND;
		else if (ack == 2)
			msg->cmd = RPMSG_PING_SEND_CHECK;
		else
			msg->cmd = RPMSG_PING_SEND_ACK;

		if (msg->cmd == RPMSG_PING_SEND_CHECK) {
			memset(msg->data, RPMSG_PING_CHECK_DATA,
			       len - sizeof(struct rpmsg_ping_msg_s) + 1);
		}

		reinit_completion(&ping_ept->ping_complete);

		if (msg->cmd == RPMSG_PING_SEND_CHECK) {
			memset(msg->data, RPMSG_PING_CHECK_DATA,
			       sizeof(struct rpmsg_ping_msg_s) + data_len);
		}

		ret = rpmsg_send(ept, msg, len);
		if (ret)
			goto free_msg;

		ret = wait_for_completion_timeout(&ping_ept->ping_complete, msecs_to_jiffies(3000));
		if (ret) {
			ret = 0;
		} else {
			XRISP_PR_ERROR("wait ack timeout");
			ret = -ETIMEDOUT;
		}
	} else {
		msg->cmd = RPMSG_PING_SEND_NOACK;
		ret = rpmsg_send(ept, msg, len);
		if (ret)
			goto free_msg;
	}

free_msg:
	kfree(msg);
	return ret;
}

static void rpmsg_ping_logout(const char *s, unsigned long value)
{
	struct timespec64 ts;

	ts = ns_to_timespec64(value);
	XRISP_PR_INFO("%s: s %lld , ns %ld", s, ts.tv_sec, ts.tv_nsec);

}

int rpmsg_ping(struct rpmsg_endpoint *ept, const struct rpmsg_ping_s *ping)
{
	int ret = 0;
	unsigned long tm = 0;
	unsigned long min = ULONG_MAX;
	unsigned long max = 0;
	uint64_t total = 0;
	int i;

	if (!ping || ping->times <= 0)
		return -EINVAL;

	for (i = 0; i < ping->times; i++) {
		tm = ktime_get();

		ret = rpmsg_ping_once(ept, ping->len, ping->ack);
		if (ret < 0)
			return ret;

		tm = ktime_get() - tm;
		min = min(min, tm);
		max = max(max, tm);
		total += tm;

		msleep(ping->sleep);
	}

	XRISP_PR_INFO("ping times: %d\n", ping->times);

	rpmsg_ping_logout("avg", total / ping->times);
	rpmsg_ping_logout("min", min);
	rpmsg_ping_logout("max", max);

	return 0;
}


int rpmsg_ping_show(struct seq_file *file, void *data)
{

	seq_puts(file, "usage: echo times > rpmsg_ping\n");
	return 0;
}

static int rpmsg_ping_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;
	single_open(filp, rpmsg_ping_show, inode->i_private);
	return 0;
}

static int rpmsg_ping_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	single_release(inode, filp);
	return 0;
}

static ssize_t rpmsg_ping_read(struct file *file, char __user *buf,
			size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;
	ret = seq_read(file, buf, len, ppos);

	debugfs_file_put(dentry);
	return ret;
}

static ssize_t rpmsg_ping_write(struct file *file, const char __user *buf,
			 size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	char cmd[10];
	struct seq_file *sfile = file->private_data;
	struct rpmsg_ping_ept *ping_ept = sfile->private;
	struct rpmsg_ping_s ping;

	XRISP_PR_INFO("rpmsg ping");

	if (!len || len > sizeof(cmd))
		return -EINVAL;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;

	ret = copy_from_user(cmd, buf, len);
	if (ret)
		goto out;

	ret = kstrtoint(cmd, 0, &ping.times);
	if (ret)
		goto out;
	if (ping.times < 0) {
		ret = -EINVAL;
		goto out;
	}
	ping.len = 32;
	ping.ack = 1;
	ping.sleep = 30;
	ret = rpmsg_ping(ping_ept->ept, &ping);
	if (ret)
		XRISP_PR_ERROR("rpmsg ping fail");
out:
	debugfs_file_put(dentry);
	return ret ? ret : len;
}

static loff_t rpmsg_ping_llseek(struct file *file, loff_t offset, int whence)
{
	XRISP_PR_INFO("");
	return -ESPIPE;
}

static const struct file_operations rpmsg_ping_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rpmsg_ping_open,
	.release = rpmsg_ping_release,
	.read	 = rpmsg_ping_read,
	.write	 = rpmsg_ping_write,
	.llseek	 = rpmsg_ping_llseek,
};

static int rpmsg_ping_probe(struct rpmsg_device *rpdev)
{
	struct rpmsg_ping_ept *ping_ept;
	struct dentry *root;

	XRISP_DEV_INFO(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
					rpdev->src, rpdev->dst);

	ping_ept = devm_kzalloc(&rpdev->dev, sizeof(*ping_ept), GFP_KERNEL);
	if (!ping_ept)
		return -ENOMEM;

	init_completion(&ping_ept->ping_complete);

	rpdev->announce = true;
	ping_ept->ept = rpdev->ept;
	dev_set_drvdata(&rpdev->dev, ping_ept);

	root = rpmsg_debugfs_get_root();
	if (!root)
		return 0;
	ping_ept->dentry = debugfs_create_dir(rpdev->id.name, root);
	debugfs_create_file("ping", 0600, ping_ept->dentry, ping_ept,
			    &rpmsg_ping_fops);

	return 0;
}

static void rpmsg_ping_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_ping_ept *ping_ept = dev_get_drvdata(&rpdev->dev);

	XRISP_DEV_INFO(&rpdev->dev, "rpmsg ping driver is removed\n");

	debugfs_remove(ping_ept->dentry);
	ping_ept->dentry = NULL;
}

static struct rpmsg_device_id rpmsg_driver_ping_id_table[] = {
	{ .name	= "rpmsg-ping" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_ping_id_table);

static struct rpmsg_driver rpmsg_ping_client = {
	.drv.name	= "rpmsg-ping",
	.id_table	= rpmsg_driver_ping_id_table,
	.probe		= rpmsg_ping_probe,
	.callback	= rpmsg_ping_ept_cb,
	.remove		= rpmsg_ping_remove,
};

int xrisp_rpmsg_ping_init(void)
{
	int ret = 0;

	ret = register_rpmsg_driver(&rpmsg_ping_client);
	if (ret)
		XRISP_PR_ERROR("register driver failed");

	return ret;
}

void xrisp_rpmsg_ping_exit(void)
{
	unregister_rpmsg_driver(&rpmsg_ping_client);
}

MODULE_AUTHOR("Zhen Bin<zhenbin@xiaomi.com>");
MODULE_DESCRIPTION("rpmsg ping driver");
MODULE_LICENSE("GPL v2");
