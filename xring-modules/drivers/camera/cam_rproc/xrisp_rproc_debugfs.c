// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rproc_debugfs", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rproc_debugfs", __func__, __LINE__

#include <linux/fs.h>
#include <linux/io.h>
#include <linux/remoteproc.h>
#include <linux/atomic/atomic-instrumented.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>
#include "cam_ctrl/xrisp_cam_ctrl.h"
#include "xrisp_rproc_api.h"
#include "xrisp_rproc.h"
#include "xrisp_debug.h"
#include "xrisp_ipcm_api.h"
#include "xrisp_pm_api.h"
#include "xrisp_log.h"

#ifdef CAM_RPROC_DEBUG

static uint64_t saddr;
static uint64_t eaddr;
uint64_t boot_timeout_ms = 1000;
bool dumpmsg;

int rproc_show(struct seq_file *file, void *data)
{
	struct cam_rproc *xrisp_rproc = file->private;

	seq_printf(file, "rproc status: %s\n", RPROC_STATUS(xrisp_rproc->rproc->state));
	seq_printf(file, "rproc power: %d\n", atomic_read(&xrisp_rproc->rproc->power));
	seq_puts(file, "usage: echo cmd > rproc\n");
	seq_puts(file, "cmds: start    --boot the remote proc--\n");
	seq_puts(file, "      stop     --shutdown the remote proc--\n");
	seq_puts(file, "      secboot  --sec boot the remote proc--\n");
	seq_puts(file, "      secdown  --sec down the remote proc--\n");
	return 0;
}

static int xrisp_rproc_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;
	// filp->private_data = inode->i_private;
	single_open(filp, rproc_show, inode->i_private);
	return 0;
}

static int xrisp_rproc_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	single_release(inode, filp);
	// filp->private_data = NULL;
	return 0;
}

static ssize_t xrisp_rproc_read(struct file *file, char __user *buf,
			size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	// struct cam_rproc *xrisp_rproc = file->private_data;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;
	ret = seq_read(file, buf, len, ppos);

	debugfs_file_put(dentry);
	return ret;
}

static ssize_t xrisp_rproc_write(struct file *file, const char __user *buf,
			 size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	char cmd[10];
	struct seq_file *sfile = file->private_data;
	struct cam_rproc *xrisp_rproc = sfile->private;
	unsigned int old_state = 0;
	u8 rgltr_need_set[PM_RGLTR_NUM_MAX] = { true, true, true, true, true };
	int old_power = 0;

	if (!len || len > sizeof(cmd))
		return -EINVAL;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;

	old_state = xrisp_rproc->rproc->state;
	old_power = atomic_read(&xrisp_rproc->rproc->power);

	ret = copy_from_user(cmd, buf, len);
	if (ret) {
		ret = -EFAULT;
		goto out;
	}

	if (!strncmp(cmd, "start", len - 1)) {
		xrisp_store_pipergltr_set(rgltr_need_set, MAX_XISP_DOMAIN_NUM);
		ret = xrisp_rproc_boot(XRISP_NORMAL_BOOT);
		if (ret) {
			XRISP_PR_ERROR("isp power on failed");
			goto out;
		}
		xrisp_rpmsg_register();
		XRISP_PR_INFO("isp power_on finish");
	} else if (!strncmp(cmd, "stop", len - 1)) {
		ret = xrisp_rproc_shutdown();
		XRISP_PR_INFO("isp power_off finish");
	} else if (!strncmp(cmd, "secboot", len - 1)) {
		xrisp_store_pipergltr_set(rgltr_need_set, MAX_XISP_DOMAIN_NUM);
		ret = xrisp_rproc_boot(XRISP_SAFE_FACE_BOOT);
		if (ret) {
			XRISP_PR_ERROR("isp secboot failed");
			goto out;
		}
		xrisp_rpmsg_register();
		XRISP_PR_INFO("isp secboot finish");
	} else if (!strncmp(cmd, "secdown", len - 1)) {
		if (xrisp_rproc->rproc->state != RPROC_ATTACHED) {
			ret = -EINVAL;
			goto out;
		}
		ret = xrisp_rproc_shutdown();
		XRISP_PR_INFO("isp secdown finish");
	} else {
		XRISP_PR_INFO("Unrecognized option\n");
		ret = -EINVAL;
	}

out:
	XRISP_PR_INFO("rproc status: %s --> %s\n", RPROC_STATUS(old_state),
		 RPROC_STATUS(xrisp_rproc->rproc->state));
	XRISP_PR_INFO("rproc power: %d --> %d\n", old_power,
		 atomic_read(&xrisp_rproc->rproc->power));

	debugfs_file_put(dentry);
	return ret < 0 ? ret : len;
}

loff_t xrisp_rproc_llseek(struct file *file, loff_t offset, int whence)
{
	XRISP_PR_INFO("");
	return -ESPIPE;
}

__maybe_unused
static const struct file_operations xrisp_rproc_rw_fops = {
	.owner	 = THIS_MODULE,
	.open	 = xrisp_rproc_open,
	.release = xrisp_rproc_release,
	.read	 = xrisp_rproc_read,
	.write	 = xrisp_rproc_write,
	.llseek	 = xrisp_rproc_llseek,
};


int rproc_mode_show(struct seq_file *file, void *data)
{
	struct cam_rproc *xrisp_rproc = file->private;

	seq_printf(file, "rproc status: %s\n", RPROC_STATUS(xrisp_rproc->rproc->state));
	seq_printf(file, "rproc power: %d\n", atomic_read(&xrisp_rproc->rproc->power));
	seq_puts(file, "usage: echo ops > rproc\n");
	seq_puts(file, "opss: Y    --set rproc to attach mode will load frimware onece--\n");
	seq_puts(file, "      N    --set rproc to normal mode will load frimware ervery time--\n");
	seq_puts(file, "notice: change rproc boot mode when rproc in detach or offline status\n");
	return 0;
}

static int rproc_mode_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;
	// filp->private_data = inode->i_private;
	single_open(filp, rproc_mode_show, inode->i_private);
	return 0;
}

static int rproc_mode_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	single_release(inode, filp);
	// filp->private_data = NULL;
	return 0;
}

static ssize_t rproc_mode_read(struct file *file, char __user *buf,
			size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	// struct cam_rproc *xrisp_rproc = file->private_data;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;
	ret = seq_read(file, buf, len, ppos);

	debugfs_file_put(dentry);
	return ret;
}

static ssize_t rproc_mode_write(struct file *file, const char __user *buf,
			 size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	ssize_t ret = 0;
	char cmd[10];
	struct seq_file *sfile = file->private_data;
	struct cam_rproc *xrisp_rproc = sfile->private;
	unsigned int old_state = 0;
	int old_power = 0;

	if (!len || len > sizeof(cmd))
		return -EINVAL;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;

	old_state = xrisp_rproc->rproc->state;
	old_power = atomic_read(&xrisp_rproc->rproc->power);

	ret = copy_from_user(cmd, buf, len);
	if (ret) {
		ret = -EFAULT;
		goto out;
	}

	if (!strncmp(cmd, "Y", len - 1)) {
		if (xrisp_rproc->rproc->state == RPROC_RUNNING ||
		    xrisp_rproc->rproc->state == RPROC_ATTACHED) {
			ret = -EBUSY;
			goto out;
		}

		XRISP_PR_INFO("set to attach mode");
		ret = xrisp_rproc_set_attach_mode(true);
	} else if (!strncmp(cmd, "N", len - 1)) {
		if (xrisp_rproc->rproc->state != RPROC_RUNNING &&
		    xrisp_rproc->rproc->state != RPROC_ATTACHED) {
			ret = -EINVAL;
			goto out;
		}
		XRISP_PR_INFO("set to normal mode");
		ret = xrisp_rproc_set_attach_mode(false);
	} else {
		XRISP_PR_INFO("Unrecognized option\n");
		ret = -EINVAL;
	}

out:
	XRISP_PR_INFO("rproc status: %s --> %s\n", RPROC_STATUS(old_state),
		 RPROC_STATUS(xrisp_rproc->rproc->state));
	XRISP_PR_INFO("rproc power: %d --> %d\n", old_power,
		 atomic_read(&xrisp_rproc->rproc->power));

	debugfs_file_put(dentry);
	return ret < 0 ? ret : len;
}

loff_t rproc_mode_llseek(struct file *file, loff_t offset, int whence)
{
	XRISP_PR_INFO("");
	return -ESPIPE;
}

__maybe_unused
static const struct file_operations rproc_mode_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rproc_mode_open,
	.release = rproc_mode_release,
	.read	 = rproc_mode_read,
	.write	 = rproc_mode_write,
	.llseek	 = rproc_mode_llseek,
};

static int rproc_mem_open(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	if (!inode->i_private)
		return -EINVAL;
	filp->private_data = inode->i_private;
	return 0;
}

static int rproc_mem_release(struct inode *inode, struct file *filp)
{
	XRISP_PR_INFO("");
	filp->private_data = NULL;
	return 0;
}

static ssize_t rproc_mem_read(struct file *file, char __user *buf,
			      size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	struct cam_rproc_mem *mem = file->private_data;
	size_t offset = 0;
	size_t roffset = 0;
	ssize_t ret = 0;


	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;
	XRISP_PR_INFO("0x%llx - 0x%llx , target 0x%llx - 0x%llx",
		 mem->da.base, mem->da.base + mem->da.size, saddr, eaddr);
	if (mem->da.region != RPROC_MEM_REGION_FWBUF) {
		ret = simple_read_from_buffer(buf, len, ppos,
					(void *)mem->va,
					mem->da.size);
		goto out;
	}

	if (eaddr < saddr || saddr < mem->da.base ||
		saddr > mem->da.base + mem->da.size ||
		eaddr < mem->da.base ||
		eaddr > mem->da.base + mem->da.size) {
		XRISP_PR_ERROR("out of mem 0x%llx - 0x%llx , target 0x%llx - 0x%llx",
			mem->da.base, mem->da.base + mem->da.size, saddr, eaddr);
		ret = -ENOMEM;
		goto out;
	}

	offset = saddr - mem->da.base;
	roffset = mem->da.base + mem->da.size - eaddr;


	ret = simple_read_from_buffer(buf, len, ppos,
				(void *)mem->va + offset,
				mem->da.size - offset - roffset);

out:
	debugfs_file_put(dentry);
	return ret;
}

static ssize_t rproc_mem_write(struct file *file, const char __user *buf,
			       size_t len, loff_t *ppos)
{
	struct dentry *dentry = file->f_path.dentry;
	struct cam_rproc_mem *mem = file->private_data;
	size_t offset = 0;
	size_t roffset = 0;
	ssize_t ret = 0;

	ret = debugfs_file_get(dentry);
	if (unlikely(ret))
		return ret;
	XRISP_PR_INFO("0x%llx - 0x%llx , target 0x%llx - 0x%llx",
		 mem->da.base, mem->da.base + mem->da.size, saddr, eaddr);
	if (eaddr < saddr || saddr < mem->da.base ||
		saddr > mem->da.base + mem->da.size ||
		eaddr < mem->da.base ||
		eaddr > mem->da.base + mem->da.size) {
		XRISP_PR_ERROR("out of mem");
		ret = -ENOMEM;
		goto out;
	}

	if (*ppos >= eaddr - saddr) {
		XRISP_PR_ERROR("no more space");
		ret = -ENOSPC;
		goto out;
	}

	offset = saddr - mem->da.base;
	roffset = mem->da.base + mem->da.size - eaddr;

	ret = simple_write_to_buffer((void *)mem->va + offset,
					mem->da.size - offset - roffset,
					ppos, buf, len);

out:
	debugfs_file_put(dentry);
	return ret;
}

loff_t rproc_mem_llseek(struct file *file, loff_t offset, int whence)
{
	XRISP_PR_INFO("");
	return -ESPIPE;
}

static const struct file_operations rproc_mem_r_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rproc_mem_open,
	.release = rproc_mem_release,
	.read	 = rproc_mem_read,
	.write	 = rproc_mem_write,
	.llseek	 = rproc_mem_llseek,
};

int rproc_mem_debugfs_install(struct cam_rproc *xrisp_rproc,
			     enum cam_rproc_mem_region region)
{
	struct cam_rproc_mem *mem = &xrisp_rproc->mem[region];

	if (mem->va)
		mem->dentry = debugfs_create_file(mem->name, 0600, xrisp_rproc->dentry,
						mem, &rproc_mem_r_fops);
	return 0;
}

void rproc_mem_debugfs_uninstall(struct cam_rproc *xrisp_rproc, enum cam_rproc_mem_region region)
{
	struct cam_rproc_mem *mem = &xrisp_rproc->mem[region];

	debugfs_remove(mem->dentry);
}

static int power_set(void *data, u64 val)
{
	struct cam_rproc *xrisp_rproc = data;

	if (val == 0)
		xrisp_power_off(xrisp_rproc);
	else
		xrisp_power_on(xrisp_rproc);

	return 0;
}

static int power_get(void *data, u64 *val)
{
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(rproc_power_fops, power_get, power_set, "%llx\n");

static int rss_show(struct seq_file *m, void *v)
{
	XRISP_PR_INFO("");
	if (provider_task) {
		seq_printf(m, "[%s][pid:%d]provider: total_vm:%ld, rss:%ld\n",
			 provider_task->comm, provider_task->pid, provider_task->mm->total_vm,
			 get_mm_rss(provider_task->mm));
		XRISP_PR_INFO("[%s][pid:%d]provider: total_vm:%ld, rss:%ld\n",
			 provider_task->comm, provider_task->pid, provider_task->mm->total_vm,
			 get_mm_rss(provider_task->mm));
	}
	if (current_task) {
		seq_printf(m, "[%s][pid:%d]current: total_vm:%ld, rss:%ld\n",
			 current_task->comm, current_task->pid, current_task->mm->total_vm,
			 get_mm_rss(current_task->mm));
		XRISP_PR_INFO("[%s][pid:%d]current: total_vm:%ld, rss:%ld\n",
			 current_task->comm, current_task->pid, current_task->mm->total_vm,
			 get_mm_rss(current_task->mm));
	}
	return 0;
}

static int rss_open(struct inode *inode, struct file *file)
{
	XRISP_PR_INFO("");
	return single_open(file, rss_show, NULL);
}

static ssize_t rss_write(struct file *file,
			 const char __user *buf,
			 size_t count, loff_t *ppos)
{
	return 0;
}

static const struct file_operations rproc_rss_fops = {
	.open = rss_open,
	.release = single_release,
	.read = seq_read,
	.write = rss_write,
};

int rproc_debugfs_init(struct cam_rproc *xrisp_rproc)
{
	struct dentry *root;
	int ret = 0;

	root = xrisp_debugfs_get_root();

	xrisp_rproc->dentry = debugfs_create_dir("rproc", root);
	if (IS_ERR_OR_NULL(xrisp_rproc->dentry))
		return -EINVAL;
	xrisp_rproc->rpmsg_dentry = debugfs_create_dir("rpmsg", root);
	if (IS_ERR_OR_NULL(xrisp_rproc->rpmsg_dentry))
		return -EINVAL;
	debugfs_create_bool("dumpmsg", 0600, xrisp_rproc->rpmsg_dentry, &dumpmsg);
	debugfs_create_file("rproc", 0600, xrisp_rproc->dentry, xrisp_rproc, &xrisp_rproc_rw_fops);
	debugfs_create_file("attach_mode", 0600, xrisp_rproc->dentry, xrisp_rproc,
			     &rproc_mode_fops);
	debugfs_create_u64("saddr", 0600, xrisp_rproc->dentry, &saddr);
	debugfs_create_u64("eaddr", 0600, xrisp_rproc->dentry, &eaddr);
	debugfs_create_u64("boot_timeout_ms", 0600, xrisp_rproc->dentry, &boot_timeout_ms);
	debugfs_create_file("power", 0600, xrisp_rproc->dentry, xrisp_rproc,
			       &rproc_power_fops);
	debugfs_create_file("rss", 0600, xrisp_rproc->dentry, xrisp_rproc,
			       &rproc_rss_fops);

	return ret;
}

void rproc_debugfs_exit(struct cam_rproc *xrisp_rproc)
{
	debugfs_remove(xrisp_rproc->dentry);
	debugfs_remove(xrisp_rproc->rpmsg_dentry);
}

struct dentry *rpmsg_debugfs_get_root(void)
{
	if (!g_xrproc)
		return NULL;
	return g_xrproc->rpmsg_dentry;
}

#else /* CAM_IPCM_DEBUG */

struct dentry *rpmsg_debugfs_get_root(void)
{
	return NULL;
}

int rproc_debugfs_init(struct cam_rproc *xrisp_rproc)
{
	return 0;
}

void rproc_debugfs_exit(struct cam_rproc *xrisp_rproc) {}

#endif /* CAM_IPCM_DEBUG */
