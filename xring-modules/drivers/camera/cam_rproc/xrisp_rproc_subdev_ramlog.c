// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rpmsg_ramlog", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rpmsg_ramlog", __func__, __LINE__

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/remoteproc.h>
#include <soc/xring/xrisp/xrisp.h>
#include "remoteproc_internal.h"
#include "xrisp_rproc.h"
#include "xrisp_log.h"

#define to_xrisp_ramlog(subdev)	container_of(subdev, struct xrisp_rproc_ramlog, subdev)

static int xrisp_rproc_ramlog_do_start(struct rproc_subdev *subdev)
{
	struct xrisp_rproc_ramlog *ramlog;


	ramlog = to_xrisp_ramlog(subdev);

	if (!ramlog->avail)
		return 0;

	XRISP_PR_INFO("ramlog start");

	if (ramlog->priv && ramlog->ramlog_start)
		ramlog->ramlog_start(ramlog->priv);

	return 0;
}

static void xrisp_rproc_ramlog_do_stop(struct rproc_subdev *subdev, bool crashed)
{
	struct xrisp_rproc_ramlog *ramlog;

	atomic_set(&rproc_stop, 1);
	smp_mb(); //set ramlog stop flag early

	ramlog = to_xrisp_ramlog(subdev);

	if (!ramlog->avail)
		return;

	XRISP_PR_INFO("ramlog stop");

	if (ramlog->priv && ramlog->ramlog_stop)
		ramlog->ramlog_stop(ramlog->priv, crashed);

	ramlog->logmem = NULL;
	ramlog->avail = false;

}

int xrisp_rproc_handle_ramlog(struct rproc *rproc, void *ptr,
			      int offset, int avail)
{
	struct cam_rproc *xrisp_rproc = rproc->priv;
	struct fw_rsc_trace *rsc = ptr;
	struct xrisp_rproc_ramlog *ramlog;
	struct xrisp_rproc_reg *regdev;

	ramlog = &xrisp_rproc->ramlog;

	if (sizeof(*rsc) > avail) {
		XRISP_PR_ERROR("rsc is truncated");
		return -EINVAL;
	}

	if (rsc->da != FW_RSC_ADDR_ANY || rsc->len != 0) {
		XRISP_PR_ERROR("not support initialized rsc");
		XRISP_PR_ERROR("initialized rsc: name %s, da 0x%x, len 0x%x",
			rsc->name, rsc->da, rsc->len);
		return -EINVAL;
	}

	if (rsc->reserved) {
		XRISP_PR_ERROR("rsc has non zero reserved bytes");
		return -EINVAL;
	}

	memcpy(ramlog->name, rsc->name, XRISP_MAX_NAME_LEN);
	ramlog->rsc_offset = offset;
	ramlog->logmem = xrisp_rproc_find_carveout_by_name(rproc, rsc->name);
	if (!ramlog->logmem) {
		XRISP_PR_ERROR("can not find %s logmem", ramlog->name);
		return -EINVAL;
	}

	/* update ramlog rsc to firmware */
	rsc->da = ramlog->logmem->da;
	rsc->len = ramlog->logmem->len;
	ramlog->avail = true;

	regdev = &xrisp_rproc->regdev;
	regdev->logmem = ramlog->logmem;
	regdev->avail = true;

	return RSC_HANDLED;
}

void xrisp_rproc_add_ramlog_subdev(struct cam_rproc *xrisp_rproc)
{
	struct xrisp_rproc_ramlog *ramlog;

	ramlog = &xrisp_rproc->ramlog;
	ramlog->rproc = xrisp_rproc->rproc;
	ramlog->logmem = NULL;
	ramlog->avail = false;
	ramlog->rsc_offset = -1;
	ramlog->subdev.start = xrisp_rproc_ramlog_do_start;
	ramlog->subdev.stop = xrisp_rproc_ramlog_do_stop;
	ramlog->priv = NULL;
	ramlog->ramlog_start = NULL;
	ramlog->ramlog_stop = NULL;

	rproc_add_subdev(ramlog->rproc, &ramlog->subdev);
}

void xrisp_rproc_remove_ramlog_subdev(struct cam_rproc *xrisp_rproc)
{
	struct xrisp_rproc_ramlog *ramlog;

	ramlog = &xrisp_rproc->ramlog;
	ramlog->logmem = NULL;
	ramlog->avail = false;
	ramlog->rsc_offset = -1;
	ramlog->priv = NULL;
	ramlog->ramlog_start = NULL;
	ramlog->ramlog_stop = NULL;
	rproc_remove_subdev(ramlog->rproc, &ramlog->subdev);
}

bool xrisp_rproc_ramlog_avail(void)
{
	if (!g_xrproc)
		return false;
	return g_xrproc->ramlog.avail;
}
EXPORT_SYMBOL(xrisp_rproc_ramlog_avail);

void *xrisp_rproc_ramlog_addr(void)
{
	if (!g_xrproc)
		return NULL;
	if (!g_xrproc->ramlog.avail || !g_xrproc->ramlog.logmem)
		return NULL;

	return g_xrproc->ramlog.logmem->va;
}
EXPORT_SYMBOL(xrisp_rproc_ramlog_addr);

#if (0)
phys_addr_t xrisp_rproc_ramlog_pa(void)
{
	struct cam_rproc_mem *ramlog_mem = g_xrproc->ramlog.logmem->priv;

	if (!g_xrproc)
		return 0;
	if (!g_xrproc->ramlog.avail || !g_xrproc->ramlog.logmem)
		return 0;

	return ramlog_mem->pa;
}
EXPORT_SYMBOL(xrisp_rproc_ramlog_pa);
#endif

size_t xrisp_rproc_ramlog_size(void)
{
	if (!g_xrproc)
		return 0;
	if (!g_xrproc->ramlog.avail || !g_xrproc->ramlog.logmem)
		return 0;

	return g_xrproc->ramlog.logmem->len;
}
EXPORT_SYMBOL(xrisp_rproc_ramlog_size);

int xrisp_register_ramlog_ops(void *priv, int (*ramlog_start)(void *priv),
			      void (*ramlog_stop)(void *priv, bool crashed))
{
	if (!priv || !ramlog_start || !ramlog_stop)
		return -EINVAL;

	if (!g_xrproc)
		return -ENODEV;
	g_xrproc->ramlog.priv = priv;
	g_xrproc->ramlog.ramlog_start = ramlog_start;
	g_xrproc->ramlog.ramlog_stop = ramlog_stop;
	return 0;
}
EXPORT_SYMBOL(xrisp_register_ramlog_ops);

int xrisp_unregister_ramlog_ops(void)
{
	if (!g_xrproc)
		return -ENODEV;

	g_xrproc->ramlog.priv = NULL;
	g_xrproc->ramlog.ramlog_start = NULL;
	g_xrproc->ramlog.ramlog_stop = NULL;

	return 0;
}
EXPORT_SYMBOL(xrisp_unregister_ramlog_ops);
