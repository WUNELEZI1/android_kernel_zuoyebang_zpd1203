// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rpmsg_epts", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rpmsg_epts", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include "xrisp_debug.h"
#include "xrisp_rproc.h"
#include "xrisp_rproc_api.h"
#include "xrisp_log.h"

extern bool dumpmsg;

struct rpmsg_ept {
	struct rpmsg_endpoint *ept;
	struct dentry *dentry;
	void *priv;
	void (*recv_cb)(void *priv, void *data, int len);
};

static struct rpmsg_ept g_rpepts;

int xrisp_rpmsg_register_recv_cb(void *priv,
	void (*recv_cb)(void *priv, void *data, int len))
{
	if (!priv || !recv_cb)
		return -EINVAL;
	g_rpepts.priv = priv;
	g_rpepts.recv_cb = recv_cb;
	return 0;
}

int xrisp_rpmsg_send(void *data, int len)
{
	if (!g_rpepts.ept)
		return -ENODEV;
	if (dumpmsg)
		print_hex_dump_debug("rpmsg send ", DUMP_PREFIX_OFFSET, 16, 4,
				      data, len, true);
	return rpmsg_send(g_rpepts.ept, data, len);
}

static int rpmsg_epts_cb(struct rpmsg_device *rpdev, void *data, int len,
			     void *priv, u32 src)
{
	struct rpmsg_ept *rpept = dev_get_drvdata(&rpdev->dev);

	if (dumpmsg)
		print_hex_dump_debug("rpmsg revc ", DUMP_PREFIX_OFFSET, 16, 4,
				      data, len, true);

	if (rpept->priv && rpept->recv_cb)
		rpept->recv_cb(rpept->priv, data, len);

	return 0;
}

static int rpmsg_epts_show(struct seq_file *file, void *data)
{

	seq_puts(file, "usage:echo data > send\n");
	return 0;
}

static int rpmsg_epts_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;
	single_open(filp, rpmsg_epts_show, inode->i_private);
	return 0;
}

static int rpmsg_epts_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	single_release(inode, filp);
	return 0;
}

static ssize_t rpmsg_epts_read(struct file *file, char __user *buf,
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

static ssize_t rpmsg_epts_write(struct file *file, const char __user *buf,
				size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	//struct seq_file *sfile = file->private_data;
	//struct rpmsg_epts *rpept = sfile->private;
	void *data = NULL;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;

	data = kzalloc(len, GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto out;
	}

	ret = copy_from_user(data, buf, len);
	if (ret) {
		ret = -EFAULT;
		goto free;
	}

	ret = xrisp_rpmsg_send(data, len);
free:
	kfree(data);
out:
	debugfs_file_put(dentry);
	return ret < 0 ? ret : len;
}

static loff_t rpmsg_epts_llseek(struct file *file, loff_t offset, int whence)
{
	XRISP_PR_INFO("");
	return -ESPIPE;
}

static const struct file_operations rpmsg_epts_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rpmsg_epts_open,
	.release = rpmsg_epts_release,
	.read	 = rpmsg_epts_read,
	.write	 = rpmsg_epts_write,
	.llseek	 = rpmsg_epts_llseek,
};

static int rpmsg_epts_probe(struct rpmsg_device *rpdev)
{

	// struct dentry *root;

	XRISP_DEV_INFO(&rpdev->dev, "new channel: 0x%x -> 0x%x!",
		 rpdev->src, rpdev->dst);

	rpdev->announce = true;
	dev_set_drvdata(&rpdev->dev, &g_rpepts);
	g_rpepts.ept = rpdev->ept;
	g_rpepts.dentry = NULL;

	xrisp_rproc_rpmsg_complete();
	XRISP_DEV_INFO(&rpdev->dev, "probe complete!");

	// root = rpmsg_debugfs_get_root();
	// if (!root)
	//	return 0;
	// g_rpepts.dentry = debugfs_create_dir(rpdev->id.name, root);
	// debugfs_create_file("send", 0600, g_rpepts.dentry, &g_rpepts,
	//		    &rpmsg_epts_fops);

	return 0;
}

static void rpmsg_epts_remove(struct rpmsg_device *rpdev)
{
	struct rpmsg_ept *rpept = dev_get_drvdata(&rpdev->dev);

	// debugfs_remove(rpept->dentry);
	XRISP_DEV_INFO(&rpdev->dev, "rpmsg client driver is removed");
	rpept->ept = NULL;
	rpept->dentry = NULL;
}

static struct rpmsg_device_id rpmsg_driver_epts_id_table[] = {
	{ .name	= "rpmsg-isp" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_epts_id_table);

static struct rpmsg_driver rpmsg_epts_client = {
	.drv.name	= "rpmsg-epts",
	.id_table	= rpmsg_driver_epts_id_table,
	.probe		= rpmsg_epts_probe,
	.callback	= rpmsg_epts_cb,
	.remove		= rpmsg_epts_remove,
};

int xrisp_rpmsg_epts_init(void)
{
	int ret = 0;

	ret = register_rpmsg_driver(&rpmsg_epts_client);
	if (ret)
		XRISP_PR_ERROR("register driver failed");

	return ret;
}

void xrisp_rpmsg_epts_exit(void)
{
	unregister_rpmsg_driver(&rpmsg_epts_client);
}

MODULE_AUTHOR("Zhen Bin<zhenbin@xiaomi.com>");
MODULE_DESCRIPTION("rpmsg epts driver");
MODULE_LICENSE("GPL v2");
