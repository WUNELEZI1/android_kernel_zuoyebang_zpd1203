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

static const char *xrisp_debug_rate_prop_name[XRISP_CLK_PERI_VOLT_MAX] = {
	[XRISP_CLK_PERI_VOLT_080V] = "debug-clock-rate-080",
	[XRISP_CLK_PERI_VOLT_070V] = "debug-clock-rate-070",
	[XRISP_CLK_PERI_VOLT_065V] = "debug-clock-rate-065",
	[XRISP_CLK_PERI_VOLT_060V] = "debug-clock-rate-060",
};

static void xrisp_debug_clk_release_devs(struct xrisp_clk_data *priv)
{
	xrisp_clk_release_devs(priv);
}

static int xrisp_debug_clk_enable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int ret = 0;

	ret = xrisp_clk_enable(priv, mask);
	if (ret) {
		XRISP_PR_ERROR("xrisp debug enable clk fail, mask = %u", mask);
		return ret;
	}
	return 0;
}

static void xrisp_debug_clk_disable(struct xrisp_clk_data *priv, unsigned int mask)
{
	xrisp_clk_disable(priv, mask);
}

static unsigned int xrisp_debug_clk_get_stats(struct xrisp_clk_data *priv)
{

	unsigned int ret = 0;

	ret = xrisp_clk_get_stats(priv);
	return ret;
}

static int xrisp_debug_clk_set_rate_by_mask(struct xrisp_clk_data *priv, unsigned int mask,
				int debug_rate)
{
	int ret = 0;

	if (debug_rate < 0 || debug_rate >= XRISP_CLK_PERI_VOLT_MAX) {
		XRISP_PR_ERROR("xrisp debug clk rate_index err, rate_index = %d\n", debug_rate);
		return -EINVAL;
	}

	ret = xrisp_clk_set_rate_by_mask(priv, mask, debug_rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp debug set rate by mask fail!!!");
		return ret;
	}
	return ret;
}

static int xrisp_debug_clk_enable_init_rate(struct xrisp_clk_data *priv, unsigned int mask,
				int debug_rate)
{
	int ret = 0;

	if (debug_rate < 0 || debug_rate >= XRISP_CLK_PERI_VOLT_MAX) {
		XRISP_PR_ERROR("xrisp debug clk rate_index err, rate_index = %d\n", debug_rate);
		return -EINVAL;
	}

	ret = xrisp_clk_enable_init_rate(priv, mask, debug_rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp debug init clk fail!!!");
		return ret;
	}
	return ret;
}

static int xrisp_debug_clk_set_rate(struct xrisp_clk_data *priv, int ch, unsigned int rate)
{
	int ret = 0;

	ret = xrisp_clk_set_rate(priv, ch, rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp debug set rate fail, ch = %d", ch);
		return ret;
	}
	return ret;
}

static unsigned int xrisp_debug_clk_get_rate(struct xrisp_clk_data *priv, int ch)
{
	unsigned int ret = 0;

	ret = xrisp_clk_get_rate(priv, ch);
	XRISP_PR_INFO("xrisp debug get rate: ch = %d, rate = %u", ch, ret);
	return ret;
}

static void xrisp_debug_clk_deinit_clk(struct xrisp_clk_data *priv)
{
	xrisp_clk_disable(priv, (1 << ISP_DEBUG_OUT_MAX) - 1);
}

static struct xrisp_clk_ops_t xrisp_debug_clk_ops = {
	.enable = xrisp_debug_clk_enable,
	.disable = xrisp_debug_clk_disable,
	.get_stats = xrisp_debug_clk_get_stats,
	.set_rate_by_mask = xrisp_debug_clk_set_rate_by_mask,
	.enable_init_rate = xrisp_debug_clk_enable_init_rate,
	.set_rate = xrisp_debug_clk_set_rate,
	.get_rate = xrisp_debug_clk_get_rate,
	.release = xrisp_debug_clk_release_devs,
	.deinit = xrisp_debug_clk_deinit_clk,
};

int xrisp_debug_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
						 const char *prop_name)
{
	int ret;

	ret = xrisp_clk_init_priv_data(priv, dev, clk_num, XRISP_CLK_PERI_VOLT_MAX,
			&xrisp_debug_rate_prop_name[0], &xrisp_debug_clk_ops, prop_name);
	if (ret)
		goto release;
	XRISP_PR_INFO("%s succeed !!!", __func__);
	return ret;
release:
	xrisp_debug_clk_release_devs(priv);
	return ret;
}
