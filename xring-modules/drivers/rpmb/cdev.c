// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright(c) 2015 - 2019 Intel Corporation.
 * Copyright(c) 2021 - 2022 Linaro Ltd.
 * Copyright(c) 2023 - 2024 X-Ring technologies Inc.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <linux/slab.h>
#include <linux/capability.h>

#include <soc/xring/rpmb.h>

#include "rpmb-cdev.h"

static dev_t rpmb_devt;
#define RPMB_MAX_DEVS  MINORMASK

#define RPMB_DEV_OPEN    0  /** single open bit (position) */

/* Currently the size is limited based on ufs normal rpmb */
#define RPMB_CMD_MAX_LEN 32768

#define uptr64(val) ((void __user *)(uintptr_t)(val))

/**
 * rpmb_open - the open function
 *
 * @inode: pointer to inode structure
 * @fp: pointer to file structure
 *
 * Return: 0 on success, <0 on error
 */
static int rpmb_open(struct inode *inode, struct file *fp)
{
	struct rpmb_dev *rdev =
		container_of(inode->i_cdev, struct rpmb_dev, cdev);

	/* the rpmb is single open! */
	if (test_and_set_bit(RPMB_DEV_OPEN, &rdev->status))
		return -EBUSY;

	fp->private_data = rdev;

	return nonseekable_open(inode, fp);
}

/**
 * rpmb_release - the cdev release function
 *
 * @inode: pointer to inode structure
 * @fp: pointer to file structure
 *
 * Return: 0 always.
 */
static int rpmb_release(struct inode *inode, struct file *fp)
{
	struct rpmb_dev *rdev = fp->private_data;

	clear_bit(RPMB_DEV_OPEN, &rdev->status);

	return 0;
}

static long rpmb_ioctl_cap_cmd(struct rpmb_dev *rdev,
			       struct rpmb_ioc_cap_cmd __user *ptr)
{
	struct rpmb_ioc_cap_cmd cap;
	int capacity;

	if (copy_from_user(&cap, ptr, sizeof(struct rpmb_ioc_cap_cmd)))
		return -EFAULT;

	capacity = rpmb_get_capacity(rdev, cap.region);
	if (capacity < 0)
		return capacity;

	cap.block_size  = rdev->cap.block_size;
	cap.wr_cnt_max  = rdev->cap.wr_cnt_max;
	cap.rd_cnt_max  = rdev->cap.rd_cnt_max;
	cap.capacity    = capacity;
	cap.auth_method = rdev->cap.auth_method;
	cap.storage_type = rdev->cap.storage_type;
	cap.reserved = 0;

	return copy_to_user(ptr, &cap, sizeof(cap)) ? -EFAULT : 0;
}

static long rpmb_ioctl_pkey_cmd(struct rpmb_dev *rdev,
				struct rpmb_ioc_reqresp_cmd __user *ptr)
{
	struct rpmb_ioc_reqresp_cmd cmd;
	u8 *req, *resp;
	long ret;

	if (copy_from_user(&cmd, ptr, sizeof(struct rpmb_ioc_reqresp_cmd)))
		return -EFAULT;

	if (!cmd.len || !cmd.rlen || !cmd.req || !cmd.resp)
		return -EINVAL;

	if (cmd.len > RPMB_CMD_MAX_LEN || cmd.rlen > RPMB_CMD_MAX_LEN)
		return -EINVAL;

	req = kmalloc(cmd.len, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	resp = kmalloc(cmd.rlen, GFP_KERNEL);
	if (!resp) {
		kfree(req);
		return -ENOMEM;
	}

	if (copy_from_user(req, uptr64(cmd.req), cmd.len))
		ret = -EFAULT;
	else
		ret = rpmb_program_key(rdev, cmd.region, cmd.len, req, cmd.rlen, resp);

	if (copy_to_user(uptr64(cmd.resp), resp, cmd.rlen))
		ret = -EFAULT;

	kfree(req);
	kfree(resp);

	return ret;
}

static long rpmb_ioctl_counter_cmd(struct rpmb_dev *rdev,
				   struct rpmb_ioc_reqresp_cmd __user *ptr)
{
	struct rpmb_ioc_reqresp_cmd cmd;
	u8 *req, *resp;
	long ret;

	if (copy_from_user(&cmd, ptr, sizeof(struct rpmb_ioc_reqresp_cmd)))
		return -EFAULT;

	if (!cmd.len || !cmd.rlen || !cmd.req || !cmd.resp)
		return -EINVAL;

	if (cmd.len > RPMB_CMD_MAX_LEN || cmd.rlen > RPMB_CMD_MAX_LEN)
		return -EINVAL;

	req = kmalloc(cmd.len, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	resp = kmalloc(cmd.rlen, GFP_KERNEL);
	if (!resp) {
		kfree(req);
		return -ENOMEM;
	}

	if (copy_from_user(req, uptr64(cmd.req), cmd.len))
		ret = -EFAULT;
	else
		ret = rpmb_get_write_counter(rdev, cmd.region, cmd.len, req, cmd.rlen, resp);

	if (copy_to_user(uptr64(cmd.resp), resp, cmd.rlen))
		ret = -EFAULT;

	kfree(req);
	kfree(resp);

	return ret;
}

static long rpmb_ioctl_wblocks_cmd(struct rpmb_dev *rdev,
				   struct rpmb_ioc_reqresp_cmd __user *ptr)
{
	struct rpmb_ioc_reqresp_cmd cmd;
	u8 *req, *resp;
	long ret;

	if (copy_from_user(&cmd, ptr, sizeof(struct rpmb_ioc_reqresp_cmd)))
		return -EFAULT;

	if (!cmd.len || !cmd.rlen || !cmd.req || !cmd.resp)
		return -EINVAL;

	if (cmd.len > RPMB_CMD_MAX_LEN || cmd.rlen > RPMB_CMD_MAX_LEN)
		return -EINVAL;

	req = kmalloc(cmd.len, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	resp = kmalloc(cmd.rlen, GFP_KERNEL);
	if (!resp) {
		kfree(req);
		return -ENOMEM;
	}

	if (copy_from_user(req, uptr64(cmd.req), cmd.len))
		ret = -EFAULT;
	else
		ret = rpmb_write_blocks(rdev, cmd.region, cmd.len, req, cmd.rlen, resp);

	if (copy_to_user(uptr64(cmd.resp), resp, cmd.rlen))
		ret = -EFAULT;

	kfree(req);
	kfree(resp);

	return ret;
}

static long rpmb_ioctl_rblocks_cmd(struct rpmb_dev *rdev,
				   struct rpmb_ioc_reqresp_cmd __user *ptr)
{
	struct rpmb_ioc_reqresp_cmd cmd;
	u8 *req, *resp;

	long ret;

	if (copy_from_user(&cmd, ptr, sizeof(struct rpmb_ioc_reqresp_cmd)))
		return -EFAULT;

	if (!cmd.len || !cmd.rlen || !cmd.req || !cmd.resp)
		return -EINVAL;

	if (cmd.len > RPMB_CMD_MAX_LEN || cmd.rlen > RPMB_CMD_MAX_LEN)
		return -EINVAL;

	req = kmalloc(cmd.len, GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	resp = kmalloc(cmd.rlen, GFP_KERNEL);
	if (!resp) {
		kfree(req);
		return -ENOMEM;
	}

	if (copy_from_user(req, uptr64(cmd.req), cmd.len))
		ret = -EFAULT;
	else
		ret = rpmb_read_blocks(rdev, cmd.region, cmd.len, req, cmd.rlen, resp);

	if (copy_to_user(uptr64(cmd.resp), resp, cmd.rlen))
		ret = -EFAULT;

	kfree(req);
	kfree(resp);

	return ret;
}

/**
 * rpmb_ioctl - rpmb ioctl dispatcher
 *
 * @fp: a file pointer
 * @cmd: ioctl command RPMB_IOC_SEQ_CMD RPMB_IOC_CAP_CMD
 * @arg: ioctl data: rpmb_ioc_ver_cmd rpmb_ioc_cap_cmd pmb_ioc_seq_cmd
 *
 * Return: 0 on success; < 0 on error
 */
static long rpmb_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	struct rpmb_dev *rdev = fp->private_data;
	void __user *ptr = (void __user *)arg;

	switch (cmd) {
	case RPMB_IOC_CAP_CMD:
		return rpmb_ioctl_cap_cmd(rdev, ptr);
	case RPMB_IOC_PKEY_CMD:
		return rpmb_ioctl_pkey_cmd(rdev, ptr);
	case RPMB_IOC_COUNTER_CMD:
		return rpmb_ioctl_counter_cmd(rdev, ptr);
	case RPMB_IOC_WBLOCKS_CMD:
		return rpmb_ioctl_wblocks_cmd(rdev, ptr);
	case RPMB_IOC_RBLOCKS_CMD:
		return rpmb_ioctl_rblocks_cmd(rdev, ptr);
	default:
		dev_err(&rdev->dev, "unsupported ioctl 0x%x.\n", cmd);
		return -ENOIOCTLCMD;
	}
}

static const struct file_operations rpmb_fops = {
	.open           = rpmb_open,
	.release        = rpmb_release,
	.unlocked_ioctl = rpmb_ioctl,
	.owner          = THIS_MODULE,
	.llseek         = noop_llseek,
};

void rpmb_cdev_prepare(struct rpmb_dev *rdev)
{
	rdev->dev.devt = MKDEV(MAJOR(rpmb_devt), rdev->id);
	rdev->cdev.owner = THIS_MODULE;
	cdev_init(&rdev->cdev, &rpmb_fops);
}

int rpmb_cdev_add(struct rpmb_dev *rdev)
{
	return cdev_add(&rdev->cdev, rdev->dev.devt, 1);
}

void rpmb_cdev_del(struct rpmb_dev *rdev)
{
	if (rdev->dev.devt)
		cdev_del(&rdev->cdev);
}

int __init rpmb_cdev_init(void)
{
	int ret;

	ret = alloc_chrdev_region(&rpmb_devt, 0, RPMB_MAX_DEVS, "rpmb");
	if (ret < 0)
		pr_err("unable to allocate char dev region\n");

	return ret;
}

void __exit rpmb_cdev_exit(void)
{
	unregister_chrdev_region(rpmb_devt, RPMB_MAX_DEVS);
}
