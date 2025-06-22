// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifdef CONFIG_DYNAMIC_DEBUG
#undef CONFIG_DYNAMIC_DEBUG
#endif
#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "rpmsg_mpu", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "rpmsg_mpu", __func__, __LINE__

#include <linux/mutex.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/remoteproc.h>
#include <soc/xring/xrisp/xrisp.h>
#include "remoteproc_internal.h"
#include "xrisp_rproc.h"
#include "xrisp_rproc_api.h"
#include "xrisp_log.h"

#define to_xrisp_rproc_mpu(subdev)	container_of(subdev, struct xrisp_rproc_mpu, subdev)

static int xrisp_rproc_mpu_do_start(struct rproc_subdev *subdev)
{
	struct xrisp_rproc_mpu *rproc_mpu;

	rproc_mpu = to_xrisp_rproc_mpu(subdev);

	XRISP_PR_INFO("rproc_mpu start");

	xrisp_rproc_mpu_lock();
	rproc_mpu->avail = true;
	xrisp_rproc_mpu_unlock();

	if (rproc_mpu->priv && rproc_mpu->rproc_mpu_start)
		rproc_mpu->rproc_mpu_start(rproc_mpu->priv);

	return 0;
}

static void xrisp_rproc_mpu_do_stop(struct rproc_subdev *subdev, bool crashed)
{
	struct xrisp_rproc_mpu *rproc_mpu;

	rproc_mpu = to_xrisp_rproc_mpu(subdev);

	if (!rproc_mpu->avail)
		return;

	XRISP_PR_INFO("rproc_mpu stop");

	if (rproc_mpu->priv && rproc_mpu->rproc_mpu_stop)
		rproc_mpu->rproc_mpu_stop(rproc_mpu->priv, crashed);

	xrisp_rproc_mpu_lock();
	rproc_mpu->avail = false;
	xrisp_rproc_mpu_unlock();

}

void xrisp_rproc_add_mpu_subdev(struct cam_rproc *xrisp_rproc)
{
	struct xrisp_rproc_mpu *rproc_mpu;

	rproc_mpu = &xrisp_rproc->rproc_mpu;
	rproc_mpu->rproc = xrisp_rproc->rproc;
	rproc_mpu->avail = false;
	mutex_init(&rproc_mpu->lock);
	rproc_mpu->rsc_offset = -1;
	rproc_mpu->subdev.start = xrisp_rproc_mpu_do_start;
	rproc_mpu->subdev.stop = xrisp_rproc_mpu_do_stop;
	rproc_mpu->priv = NULL;
	rproc_mpu->rproc_mpu_start = NULL;
	rproc_mpu->rproc_mpu_stop = NULL;

	rproc_add_subdev(rproc_mpu->rproc, &rproc_mpu->subdev);
}

void xrisp_rproc_remove_mpu_subdev(struct cam_rproc *xrisp_rproc)
{
	struct xrisp_rproc_mpu *rproc_mpu;

	rproc_mpu = &xrisp_rproc->rproc_mpu;
	rproc_mpu->avail = false;
	mutex_destroy(&rproc_mpu->lock);
	rproc_mpu->rsc_offset = -1;
	rproc_mpu->priv = NULL;
	rproc_mpu->rproc_mpu_start = NULL;
	rproc_mpu->rproc_mpu_stop = NULL;
	rproc_remove_subdev(rproc_mpu->rproc, &rproc_mpu->subdev);
}

bool xrisp_rproc_mpu_avail(void)
{
	if (!g_xrproc || !xrisp_rproc_is_running())
		return false;
	return g_xrproc->rproc_mpu.avail;
}

void xrisp_rproc_mpu_lock(void)
{
	if (!g_xrproc)
		return;
	mutex_lock(&g_xrproc->rproc_mpu.lock);
}

void xrisp_rproc_mpu_unlock(void)
{
	if (!g_xrproc)
		return;
	mutex_unlock(&g_xrproc->rproc_mpu.lock);
}

int xrisp_register_rproc_mpu_ops(void *priv, int (*start)(void *priv),
			      void (*stop)(void *priv, bool crashed))
{
	if (!priv || !start || !stop)
		return -EINVAL;

	if (!g_xrproc)
		return -ENODEV;
	g_xrproc->rproc_mpu.priv = priv;
	g_xrproc->rproc_mpu.rproc_mpu_start = start;
	g_xrproc->rproc_mpu.rproc_mpu_stop = stop;
	return 0;
}

int xrisp_unregister_rproc_mpu_ops(void)
{
	if (!g_xrproc)
		return -ENODEV;

	g_xrproc->rproc_mpu.priv = NULL;
	g_xrproc->rproc_mpu.rproc_mpu_start = NULL;
	g_xrproc->rproc_mpu.rproc_mpu_start = NULL;
	g_xrproc->rproc_mpu.avail = false;

	return 0;
}
