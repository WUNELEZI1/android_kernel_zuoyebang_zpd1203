// SPDX-License-Identifier: BSD-3-Clause-Clear
/*
 * Copyright (c) 2022, MediaTek Inc.
 */

#include <linux/uaccess.h>
#include "mtk_debugfs.h"

#define MTK_DBGFS_NAME "mtk_wwan"

static struct dentry *mtk_drv_dentry;

struct dentry *mtk_get_drv_dentry(void)
{
	return mtk_drv_dentry;
}

void mtk_drv_dbgfs_init(void)
{
	mtk_drv_dentry = debugfs_create_dir(MTK_DBGFS_NAME, NULL);
}

void mtk_drv_dbgfs_exit(void)
{
	mtk_dbgfs_remove(mtk_drv_dentry);
}

/**
 * mtk_dbgfs_create_dir() -Create a directory in the debugfs filesystem.
 *
 * @parent: A pointer to the parent dentry for this directory, must not be NULL.
 * @name: The name of new directory.
 *
 * Return: If it fails, return a NULL pointer or an error pointer, the others indicate success.
 */
struct dentry *mtk_dbgfs_create_dir(struct dentry *parent, const char *name)
{
	if (IS_ERR_OR_NULL(parent) || !name)
		return NULL;

	return debugfs_create_dir(name, parent);
}

/**
 * mtk_dbgfs_create_file() -Create a file in the debugfs filesystem.
 *
 * @parent: A pointer to the parent dentry for this file, must not be NULL.
 * @dbgfs: Please check MTK_DBGFS macro for more details.
 * @data: Private data for this file.
 *
 * Return: If it fails, return a NULL pointer or an error pointer, the others indicate success.
 */
struct dentry *mtk_dbgfs_create_file(struct dentry *parent,
				     const struct mtk_debugfs *dbgfs, void *data)
{
	if (IS_ERR_OR_NULL(parent) || !dbgfs)
		return NULL;

	return debugfs_create_file(dbgfs->name, dbgfs->mode, parent, data, &dbgfs->fops);
}

ssize_t __mtk_dbgfs_fops_read(const struct mtk_debugfs *dbgfs, struct file *file,
			      char __user *user, size_t cnt, loff_t *ppos)
{
	char *buf;
	ssize_t ret;

	if (!dbgfs->read)
		return -EPERM;
	if (*ppos)
		return 0; /* For calling 'read callback' only once. */
	if (cnt > MTK_DBGFS_BUF_MAX)
		cnt = MTK_DBGFS_BUF_MAX;

	buf = kzalloc(MTK_DBGFS_BUF_MAX, GFP_KERNEL);
	ret = dbgfs->read(file->private_data, buf, cnt);
	if (ret < 0) {
		kfree(buf);
		return ret;
	}

	ret = simple_read_from_buffer(user, cnt, ppos, buf, ret);

	kfree(buf);
	return ret;
}

ssize_t __mtk_dbgfs_fops_write(const struct mtk_debugfs *dbgfs, struct file *file,
			       const char __user *user, size_t cnt, loff_t *loff)
{
	char *buf;
	ssize_t ret, i;
	char t;

	if (!dbgfs->write)
		return -EPERM;
	if (cnt >= MTK_DBGFS_BUF_MAX)
		return -EINVAL;

	buf = kzalloc(MTK_DBGFS_BUF_MAX, GFP_KERNEL);
	ret = copy_from_user(buf, user, cnt);
	if (ret) {
		kfree(buf);
		return -EIO;
	}
	i = cnt;
	while (i > 0) {
		t = buf[i - 1];
		if (t == '\n' || t == '\r')
			i--;
		else
			break;
	}
	buf[i] = '\0';
	ret = dbgfs->write(file->private_data, buf, i);

	kfree(buf);
	return ret == i ? cnt : -EIO;
}

