// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */



#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "reg", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "reg", __func__, __LINE__

#include <linux/poll.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/remoteproc.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/sched/signal.h>
#include <soc/xring/xrisp/xrisp.h>
#include "remoteproc_internal.h"
#include "xrisp_rproc.h"
#include "xrisp_log.h"

#define subdev_to_xrisp_reg(subdev)	\
		container_of(subdev, struct xrisp_rproc_reg, subdev)
#define filp_to_xrisp_reg(filp)		\
		container_of(filp->private_data, struct xrisp_rproc_reg, regdev)

static int xrisp_rproc_regdev_do_start(struct rproc_subdev *subdev)
{
	struct xrisp_rproc_reg *regdev;


	regdev = subdev_to_xrisp_reg(subdev);

	if (!regdev->avail)
		return 0;

	XRISP_PR_INFO("reg start");

	if (!regdev->logmem) {
		XRISP_PR_ERROR("can not find %s regmem", regdev->name);
		return -EINVAL;
	}

	if (regdev->priv && regdev->regdev_start)
		regdev->regdev_start(regdev->priv);

	wake_up(&regdev->wait_q);

	return 0;
}

static void xrisp_rproc_regdev_do_stop(struct rproc_subdev *subdev, bool crashed)
{
	struct xrisp_rproc_reg *regdev;

	regdev = subdev_to_xrisp_reg(subdev);

	if (!regdev->avail)
		return;

	XRISP_PR_INFO("reg stop");

	if (regdev->priv && regdev->regdev_stop)
		regdev->regdev_stop(regdev->priv, crashed);

	if (regdev->user_task) {
		task_lock(regdev->user_task);
		send_sig(SIGUSR1, regdev->user_task, 0);
		task_unlock(regdev->user_task);
		wait_for_completion(&regdev->sig_kill);
		XRISP_PR_INFO("wait sig kill complet");
	}

	regdev->logmem = NULL;
	regdev->avail = false;

}

static int misc_reg_open(struct inode *inode, struct file *file)
{
	struct xrisp_rproc_reg *regdev = filp_to_xrisp_reg(file);

	if (regdev->user_task && regdev->user_task != current)
		return -EBUSY;

	regdev->user_task = current;
	reinit_completion(&regdev->sig_kill);

	return 0;
}

static int misc_reg_release(struct inode *inode, struct file *file)
{
	struct xrisp_rproc_reg *reg = filp_to_xrisp_reg(file);

	reg->user_task = NULL;
	complete(&reg->sig_kill);
	return 0;
}

static int misc_reg_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret = 0;
	struct cam_rproc_mem *mem = NULL;
	struct cam_rproc *xrisp_rproc = NULL;
	size_t size = vma->vm_end - vma->vm_start;
	struct xrisp_rproc_reg *regdev = filp_to_xrisp_reg(file);

	if (!xrisp_rproc_regdev_avail())
		return -ENODEV;
	if (regdev->user_task && regdev->user_task != current)
		return -EBUSY;

	if (!regdev->logmem)
		return -ENOMEM;

	mem = regdev->logmem->priv;
	xrisp_rproc = regdev->rproc->priv;

	XRISP_PR_INFO("mmap addr 0x%lx size 0x%zx", vma->vm_pgoff << PAGE_SHIFT, size);

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (vma->vm_pgoff == 0) {
		if (size > mem->da.size)
			return -E2BIG;

		ret = dma_mmap_attrs(&regdev->rproc->dev, vma, mem->va, mem->dma_handle,
			       size, 0);
		if (ret) {
			XRISP_PR_ERROR("mmap fail for logmem ret = %d", ret);
			return -EAGAIN;
		}
		return 0;
	}

	//if (io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
	//		       size, vma->vm_page_prot)) {

	//	XRISP_PR_WARN("address 0x%lx mmap error",
	//		 vma->vm_pgoff << PAGE_SHIFT);
	//	return -EAGAIN;
	//}
	return 0;
}

static __poll_t misc_reg_poll(struct file *file, poll_table *wait)
{

	__poll_t ret = 0;
	struct xrisp_rproc_reg *regdev = filp_to_xrisp_reg(file);

	if (regdev->user_task && regdev->user_task != current)
		return -EBUSY;

	poll_wait(file, &regdev->wait_q, wait);

	if (xrisp_rproc_regdev_avail())
		ret = POLLIN | POLLRDNORM | POLLERR | POLLPRI;

	return ret;
}

static const struct file_operations misc_regdev_fops = {
	.owner = THIS_MODULE,
	.open = misc_reg_open,
	.release = misc_reg_release,
	.mmap = misc_reg_mmap,
	.poll = misc_reg_poll,
};

int xrisp_rproc_add_reg_subdev(struct cam_rproc *xrisp_rproc)
{
	int ret = 0;
	struct xrisp_rproc_reg *regdev;

	regdev = &xrisp_rproc->regdev;
	regdev->rproc = xrisp_rproc->rproc;
	regdev->logmem = NULL;
	regdev->avail = false;
	regdev->rsc_offset = -1;
	regdev->subdev.start = xrisp_rproc_regdev_do_start;
	regdev->subdev.stop = xrisp_rproc_regdev_do_stop;
	regdev->priv = NULL;
	regdev->regdev_start = NULL;
	regdev->regdev_stop = NULL;
	init_completion(&regdev->sig_kill);
	memcpy(regdev->name, "xrisp-reg", sizeof("xrisp-reg"));
	regdev->user_task = NULL;
	init_waitqueue_head(&regdev->wait_q);

	regdev->regdev.parent = &xrisp_rproc->rproc->dev;
	regdev->regdev.name = regdev->name;
	regdev->regdev.minor = MISC_DYNAMIC_MINOR;
	regdev->regdev.fops = &misc_regdev_fops;

	ret = misc_register(&regdev->regdev);

	if (ret) {
		XRISP_PR_ERROR("reg misc device register fail ret = %d", ret);
		return ret;
	}

	rproc_add_subdev(regdev->rproc, &regdev->subdev);

	return 0;
}

void xrisp_rproc_remove_reg_subdev(struct cam_rproc *xrisp_rproc)
{
	struct xrisp_rproc_reg *regdev;

	regdev = &xrisp_rproc->regdev;

	misc_deregister(&regdev->regdev);

	regdev->logmem = NULL;
	regdev->avail = false;
	regdev->rsc_offset = -1;
	regdev->priv = NULL;
	regdev->regdev_start = NULL;
	regdev->regdev_stop = NULL;
	rproc_remove_subdev(regdev->rproc, &regdev->subdev);
}

bool xrisp_rproc_regdev_avail(void)
{
	if (!g_xrproc)
		return false;
	return g_xrproc->regdev.avail;
}
EXPORT_SYMBOL(xrisp_rproc_regdev_avail);

void *xrisp_rproc_regdev_mem_addr(void)
{
	if (!g_xrproc)
		return NULL;
	if (!g_xrproc->regdev.avail || !g_xrproc->regdev.logmem)
		return NULL;

	return g_xrproc->regdev.logmem->va;
}
EXPORT_SYMBOL(xrisp_rproc_regdev_mem_addr);

size_t xrisp_rproc_regdev_mem_size(void)
{
	if (!g_xrproc)
		return 0;
	if (!g_xrproc->regdev.avail || !g_xrproc->regdev.logmem)
		return 0;

	return g_xrproc->regdev.logmem->len;
}
EXPORT_SYMBOL(xrisp_rproc_regdev_mem_size);

int xrisp_register_regdev_ops(void *priv, int (*regdev_start)(void *priv),
			      void (*regdev_stop)(void *priv, bool crashed))
{
	if (!priv || !regdev_start || !regdev_stop)
		return -EINVAL;

	if (!g_xrproc)
		return -ENODEV;
	g_xrproc->regdev.priv = priv;
	g_xrproc->regdev.regdev_start = regdev_start;
	g_xrproc->regdev.regdev_stop = regdev_stop;
	return 0;
}
EXPORT_SYMBOL(xrisp_register_regdev_ops);

int xrisp_unregister_regdev_ops(void)
{
	if (!g_xrproc)
		return -ENODEV;

	g_xrproc->regdev.priv = NULL;
	g_xrproc->regdev.regdev_start = NULL;
	g_xrproc->regdev.regdev_stop = NULL;

	return 0;
}
EXPORT_SYMBOL(xrisp_unregister_regdev_ops);
