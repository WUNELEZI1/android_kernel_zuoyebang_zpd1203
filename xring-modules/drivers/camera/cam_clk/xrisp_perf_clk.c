// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "clk", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "clk", __func__, __LINE__

#include <linux/io.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/err.h>
#include "xrisp_clk.h"
#include "xrisp_log.h"

static void xrisp_perf_clk_release_devs(struct xrisp_clk_data *priv)
{
	xrisp_clk_release_devs(priv);
}

static int xrisp_perf_clk_enable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int ret = 0;

	ret = xrisp_clk_enable(priv, mask);
	if (ret) {
		XRISP_PR_ERROR("xrisp perf enable clk fail, mask = %u", mask);
		return ret;
	}
	return 0;
}

static void xrisp_perf_clk_disable(struct xrisp_clk_data *priv, unsigned int mask)
{
	xrisp_clk_disable(priv, mask);
}

static unsigned int xrisp_perf_clk_get_stats(struct xrisp_clk_data *priv)
{
	unsigned int ret = 0;

	ret = xrisp_clk_get_stats(priv);
	return ret;
}

static int xrisp_perf_clk_set_rate_by_mask(struct xrisp_clk_data *priv,
	unsigned int mask, int perf_rate)
{
	return 0;
}

static int xrisp_perf_clk_enable_init_rate(struct xrisp_clk_data *priv,
	unsigned int mask, int perf_rate)
{
	int ret = 0;

	ret = xrisp_clk_enable_init_rate(priv, mask, 0);
	if (ret) {
		XRISP_PR_ERROR("xrisp perf init clk fail!!!");
		return ret;
	}
	return ret;
}

static int xrisp_perf_clk_set_rate(struct xrisp_clk_data *priv, int ch, unsigned int rate)
{
	int ret = 0;

	ret = xrisp_clk_set_rate(priv, ch, rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp perf set rate fail, ch = %d", ch);
		return ret;
	}
	return ret;
}

static unsigned int xrisp_perf_clk_get_rate(struct xrisp_clk_data *priv, int ch)
{
	unsigned int ret = 0;

	ret = xrisp_clk_get_rate(priv, ch);
	XRISP_PR_INFO("xrisp perf get rate: ch = %d, rate = %u", ch, ret);
	return ret;
}

static void xrisp_perf_clk_deinit_clk(struct xrisp_clk_data *priv)
{
	xrisp_clk_disable(priv, (1 << ISP_PERF_OUT_MAX) - 1);
}

static struct xrisp_clk_ops_t xrisp_perf_clk_ops = {
	.enable = xrisp_perf_clk_enable,
	.disable = xrisp_perf_clk_disable,
	.get_stats = xrisp_perf_clk_get_stats,
	.set_rate_by_mask = xrisp_perf_clk_set_rate_by_mask,
	.enable_init_rate = xrisp_perf_clk_enable_init_rate,
	.set_rate = xrisp_perf_clk_set_rate,
	.get_rate = xrisp_perf_clk_get_rate,
	.release = xrisp_perf_clk_release_devs,
	.deinit = xrisp_perf_clk_deinit_clk,
};

int xrisp_perf_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
						const char *prop_name)
{
	int ret;

	ret = xrisp_clk_init_priv_data(priv, dev, clk_num, 0, NULL, &xrisp_perf_clk_ops, prop_name);
	if (ret)
		goto release;
	XRISP_PR_INFO("%s succeed !!!", __func__);
	return ret;
release:
	xrisp_perf_clk_release_devs(priv);
	return ret;
}
