// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "clk_debugfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "clk_debugfs", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "xrisp_clk.h"
#include "xrisp_debug.h"
#include "xrisp_log.h"

#ifdef CAM_CLK_DEBUG

static struct dentry *clk_debug_dentry;

static int clk_show(struct seq_file *file, void *data)
{
	struct xrisp_clk_data *priv = file->private;
	unsigned int gate_stats = 0;
	int i;

	seq_puts(file, "usage: echo cmd > clk\n");
	seq_puts(file, "cmds: enable [mask]         --enable clk gate by mask--\n");
	seq_puts(file, "      disable [mask]        --disable clk gate by mask--\n");
	seq_puts(file, "      enable_init_rate [mask] [id]"
		       " --enable and init clk rate by mask, rate_id = id--\n");
	seq_puts(file, "      set_rate_by_mask [mask] [id]"
		       " --set clk rate by mask, rate_id = id--\n");
	seq_puts(file, "      set_rate [ch] [rate]  --set channel rate--\n");
	seq_puts(file, "      get_rate [ch]         --get channel rate--\n");
	seq_puts(file, "      deinit_clk            --deinit clk--\n");

	gate_stats = priv->ops->get_stats(priv);
	seq_printf(file, "xrisp_clk clk_stats = %u\n", gate_stats);

	for (i = 0; i < priv->clk_num; i++) {
		struct xrisp_clk_info_t *clk_info_p = &priv->clk_info[i];

		seq_printf(file, "xrisp clk name = %s, clk_ptr = %p, enable = %d, rate = %u\n",
				   clk_info_p->clk_name, clk_info_p->clk_p, atomic_read(&clk_info_p->clk_enable),
				   clk_info_p->clk_rate);
	}
	return 0;
}

static int xrisp_clk_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;

	single_open(filp, clk_show, inode->i_private);
	return 0;
}

static int xrisp_clk_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	single_release(inode, filp);
	return 0;
}


static ssize_t xrisp_clk_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
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

static ssize_t xrisp_clk_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	struct xrisp_clk_data *priv = ((struct seq_file *)file->private_data)->private;
	int ret = 0;
	char clk_buf[32];
	char cmd[32];
	char ch_str[32];
	unsigned int ch = 0;
	unsigned long arg = 0;

	if (len == 0 || len > sizeof(cmd))
		return -EINVAL;

	ret = copy_from_user(clk_buf, buf, len);

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;

	if (sscanf(clk_buf, "%20s %4s %lu", cmd, ch_str, &arg) < 2) {
		ret = -EFAULT;
		goto out;
	}

	if (strncmp(ch_str, "0x", 2) == 0 || strncmp(ch_str, "0X", 2) == 0) {
		if (kstrtouint(ch_str + 2, 16, &ch) < 0)
			goto out;
	} else {
		if (kstrtouint(ch_str, 10, &ch) < 0)
			goto out;
	}
	if (strcmp(cmd, "enable") == 0) {
		XRISP_PR_INFO("clk_debugfs enable");
		ret = priv->ops->enable(priv, ch);
	} else if (strcmp(cmd, "disable") == 0) {
		XRISP_PR_INFO("clk_debugfs disable");
		priv->ops->disable(priv, ch);
	} else if (strcmp(cmd, "enable_init_rate") == 0) {
		XRISP_PR_INFO("clk_debugfs enable_init_rate");
		ret = priv->ops->enable_init_rate(priv, ch, arg);
	} else if (strcmp(cmd, "set_rate_by_mask") == 0) {
		XRISP_PR_INFO("clk_debugfs set_rate_by_mask");
		ret = priv->ops->set_rate_by_mask(priv, ch, arg);
	} else if (strcmp(cmd, "set_rate") == 0) {
		XRISP_PR_INFO("clk_debugfs set rate");
		ret = priv->ops->set_rate(priv, ch, arg);
	} else if (strcmp(cmd, "get_rate") == 0) {
		XRISP_PR_INFO("clk_debugfs get rate");
		arg = priv->ops->get_rate(priv, ch);
		XRISP_PR_INFO("xrisp_clk ch = %u, rate = %lu", ch, arg);
	} else if (strcmp(cmd, "deinit_clk") == 0) {
		XRISP_PR_INFO("clk_debugfs deinit clk");
		priv->ops->deinit(priv);
	} else {
		XRISP_PR_INFO("Unrecognized option\n");
		ret = -EINVAL;
	}

out:
	debugfs_file_put(dentry);
	return ret < 0 ? ret : len;
}

__maybe_unused static const struct file_operations xrisp_clk_rw_fops = {
	.owner = THIS_MODULE,
	.open = xrisp_clk_open,
	.release = xrisp_clk_release,
	.read = xrisp_clk_read,
	.write = xrisp_clk_write,
};

int clk_debugfs_init(struct xrisp_clk_data *priv)
{
	int ret = 0;

	if (clk_debug_dentry == NULL) {
		struct dentry *root;

		root = xrisp_debugfs_get_root();
		clk_debug_dentry = debugfs_create_dir("clk", root);
	}

	debugfs_create_file(priv->name + sizeof("xrisp_") - 1, 0600, clk_debug_dentry, priv,
						&xrisp_clk_rw_fops);

	return ret;
}

void clk_debugfs_exit(void)
{
	if (clk_debug_dentry != NULL) {
		debugfs_remove(clk_debug_dentry);
		clk_debug_dentry = NULL;
	}
}

#else /* CAM_CLK_DEBUG */


int clk_debugfs_init(struct xrisp_clk_data *priv)
{
	return 0;
}
void clk_debugfs_exit(void)
{}

#endif /* CAM_CLK_DEBUG */
