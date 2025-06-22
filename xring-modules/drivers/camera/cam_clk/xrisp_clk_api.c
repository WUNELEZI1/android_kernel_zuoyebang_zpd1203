// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "clk", __func__, __LINE__
#define dev_fmt(fmt) "[XRISP_DRV][%s] %s(%d): " fmt, "clk", __func__, __LINE__

#include <linux/module.h>
#include <linux/err.h>
#include "xrisp_clk.h"
#include "xrisp_log.h"

int xrisp_clk_api_enable(enum xrisp_clk_e clk, unsigned int mask)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];
	int ret = 0;

	if (priv->ops && priv->ops->enable)
		ret = priv->ops->enable(priv, mask);
	return ret;
}

void xrisp_clk_api_disable(enum xrisp_clk_e clk, unsigned int mask)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];

	if (priv->ops && priv->ops->disable)
		priv->ops->disable(priv, mask);
}
unsigned int xrisp_clk_api_get_stats(enum xrisp_clk_e clk)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];
	unsigned int ret = 0;

	if (priv->ops && priv->ops->get_stats)
		ret = priv->ops->get_stats(priv);
	return ret;
}

int xrisp_clk_api_set_rate_by_mask(enum xrisp_clk_e clk, unsigned int mask, int rate_index)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];
	int ret = 0;

	if (priv->ops && priv->ops->set_rate_by_mask)
		ret = priv->ops->set_rate_by_mask(priv, mask, rate_index);
	return ret;
}

int xrisp_clk_api_enable_init_rate(enum xrisp_clk_e clk, unsigned int mask, int rate_index)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];
	int ret = 0;

	if (priv->ops && priv->ops->enable_init_rate)
		ret = priv->ops->enable_init_rate(priv, mask, rate_index);
	return ret;
}

int xrisp_clk_api_set_rate(enum xrisp_clk_e clk, int ch, unsigned int rate)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];
	int ret = 0;

	if (priv->ops && priv->ops->set_rate)
		ret = priv->ops->set_rate(priv, ch, rate);
	return ret;
}

unsigned int xrisp_clk_api_get_rate(enum xrisp_clk_e clk, int ch)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];
	unsigned int ret = 0;

	if (priv->ops && priv->ops->get_rate)
		ret = priv->ops->get_rate(priv, ch);
	return ret;
}

void xrisp_clk_api_deinit_clk(enum xrisp_clk_e clk)
{
	struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk];

	if (priv->ops && priv->ops->deinit)
		priv->ops->deinit(priv);
}
