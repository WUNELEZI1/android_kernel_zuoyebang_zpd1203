// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "mbox_debug", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/kfifo.h>
#include <soc/xring/ipc/xr_rproc.h>

#include "xrisp_log.h"
#include "xrisp_debug.h"
#include "cam_ctrl/xrisp_camctrl_ipcm.h"
#include "cam_ctrl/xrisp_cam_ctrl.h"

#define MBOX_MSG_VALID_SIZE (59)
#define BUF_SIZE	    (2048)

struct cam_debug_ack_msg {
	int32_t result;
	uint8_t read_data[50];
} __packed;

struct xrisp_mbox_uft_dev {
	struct kfifo fifo;
	struct task_struct *user_task;
	bool is_open;
};

DEFINE_MUTEX(mbox_mtx);
struct dentry *mbox_dfs_dir;
struct xrisp_mbox_uft_dev mbox_priv;

static int mbox_debug_buffer_fifo_init(void)
{
	int ret;

	ret = kfifo_alloc(&mbox_priv.fifo, BUF_SIZE, GFP_KERNEL);
	if (ret) {
		XRISP_PR_ERROR("kfifo alloc failed");
		return -ENOMEM;
	}

	return 0;
}

static void mbox_debug_buffer_fifo_deinit(void)
{
	kfifo_free(&mbox_priv.fifo);
}

static int mbox_debug_buffer_in(void *data)
{
	if (kfifo_avail(&mbox_priv.fifo) >= MBOX_MSG_VALID_SIZE)
		return kfifo_in(&mbox_priv.fifo, data, MBOX_MSG_VALID_SIZE);
	else
		return -EAGAIN;
}

static int mbox_debug_buffer_out(void *data)
{
	if (kfifo_len(&mbox_priv.fifo) >= MBOX_MSG_VALID_SIZE)
		return kfifo_out(&mbox_priv.fifo, data, MBOX_MSG_VALID_SIZE);
	else
		return -EAGAIN;
}

int mbox_debug_cb(void *data, unsigned int len)
{
	int ret;

	ret = mbox_debug_buffer_in(data);
	if (ret < 0) {
		XRISP_PR_ERROR("msg buf full, recv failed");
		goto sig;
	}

	XRISP_PR_DEBUG("msg recv success, free msg cnt=%d",
		       kfifo_avail(&mbox_priv.fifo) / MBOX_MSG_VALID_SIZE);

sig:
	if (mbox_priv.user_task)
		send_sig(SIGUSR1, mbox_priv.user_task, 0);
	return 0;
}

int mbox_debug_topic_register(void)
{
	return xrisp_topic_handle_register(TOPIC_MBOX_DEBUG, mbox_debug_cb, IPCM_ORDERED_HANDLE);
}

void mbox_debug_topic_unregister(void)
{
	xrisp_topic_handle_unregister(TOPIC_MBOX_DEBUG);
}

ssize_t mbox_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	u8 msg[MBOX_MSG_VALID_SIZE] = { 0 };
	int ret;

	if (count > MBOX_MSG_VALID_SIZE) {
		XRISP_PR_ERROR("msg size over limit, count=%zu", count);
		return -EINVAL;
	}

	ret = mbox_debug_buffer_out(msg);
	if (ret < 0) {
		XRISP_PR_ERROR("msg buf empty, last=%d", kfifo_len(&mbox_priv.fifo));
		return -EAGAIN;
	}

	XRISP_PR_DEBUG("read count %zu", count);

	if (copy_to_user(buf, msg, count)) {
		XRISP_PR_ERROR("copy_to_user failed");
		return -EFAULT;
	}

	XRISP_PR_DEBUG("msg read success, use msg cnt=%d",
		       kfifo_len(&mbox_priv.fifo) / MBOX_MSG_VALID_SIZE);

	return count;
}

static ssize_t mbox_send(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
	u8 msg[MBOX_MSG_VALID_SIZE + 1] = { 0 };

	if (count > MBOX_MSG_VALID_SIZE) {
		XRISP_PR_ERROR("msg size over limit");
		return -EINVAL;
	}

	if (copy_from_user(msg + 1, buf, count)) {
		XRISP_PR_ERROR("copy_from_user failed");
		return -EFAULT;
	}

	msg[0] = TOPIC_MBOX_DEBUG;

	if (xr_rproc_send_sync(CTRL_MBOX_CH, (unsigned int *)msg, MBOX_MSG_VALID_SIZE + 1)) {
		XRISP_PR_ERROR("send msg failed");
		return -EFAULT;
	}

	XRISP_PR_DEBUG("send msg to isp sucess");

	return count;
}

int mbox_open(struct inode *inode, struct file *file)
{
	mutex_lock(&mbox_mtx);

	if (mbox_priv.is_open) {
		mutex_unlock(&mbox_mtx);
		return -EBUSY;
	}
	mbox_priv.is_open = true;
	mbox_priv.user_task = current;

	mutex_unlock(&mbox_mtx);
	return 0;
}

int mbox_release(struct inode *inode, struct file *file)
{
	mutex_lock(&mbox_mtx);
	mbox_priv.is_open = false;
	mbox_priv.user_task = NULL;
	mutex_unlock(&mbox_mtx);
	return 0;
}

static const struct file_operations mbox_fops = {
	.open = mbox_open,
	.read = mbox_read,
	.write = mbox_send,
	.release = mbox_release,
};

int xrisp_mbox_debug_init(void)
{
	struct dentry *mbox_file;
	int ret;

	ret = mbox_debug_buffer_fifo_init();
	if (ret)
		return ret;

	ret = mbox_debug_topic_register();
	if (ret) {
		XRISP_PR_ERROR("failed to register mbox_debug topic");
		goto free_fifo;
	}

	mbox_dfs_dir = debugfs_create_dir("isp_mbox", xrisp_debugfs_get_root());
	if (IS_ERR_OR_NULL(mbox_dfs_dir)) {
		XRISP_PR_ERROR("failed to create mbox_debug dir");
		ret = -ENOMEM;
		goto unregister_topic;
	}

	mbox_file = debugfs_create_file("mbox", 0600, mbox_dfs_dir, NULL, &mbox_fops);
	if (IS_ERR_OR_NULL(mbox_file)) {
		XRISP_PR_ERROR("failed to create mbox_debug file");
		ret = -ENOMEM;
		goto remove_dir;
	}

	XRISP_PR_INFO("xrisp mbox_debug init success");
	return 0;

remove_dir:
	debugfs_remove(mbox_dfs_dir);
	mbox_dfs_dir = NULL;
unregister_topic:
	mbox_debug_topic_unregister();
free_fifo:
	mbox_debug_buffer_fifo_deinit();
	return ret;
}

void xrisp_mbox_debug_exit(void)
{
	if (!mbox_dfs_dir)
		return;

	debugfs_remove(mbox_dfs_dir);
	mbox_debug_topic_unregister();
	mbox_debug_buffer_fifo_deinit();
}
