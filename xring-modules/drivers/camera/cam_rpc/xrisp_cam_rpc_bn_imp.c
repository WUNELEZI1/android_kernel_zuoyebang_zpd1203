// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "xrisp_rpc", __func__, __LINE__

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/bitops.h>
#include <dt-bindings/xring/platform-specific/xrisp.h>

#include "xrisp_pm_api.h"

int xrisp_pipe_power_up(struct xrisp_power_up *powerup_set)
{
	int ret;

	ret = xrisp_pipe_rgltr_enable(powerup_set->domain, MAX_XISP_DOMAIN_NUM);
	if (ret != 0)
		return ret;

	return 0;
}

int xrisp_pipe_power_down(struct xrisp_power_down *powerup_set)
{
	int ret;

	ret = xrisp_pipe_rgltr_disable(powerup_set->domain, MAX_XISP_DOMAIN_NUM);
	if (ret != 0)
		return ret;

	return 0;
}
