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

#define ISP_CRG_RESET_RATE 104450000

static const int xrisp_output_clk_source[CLK_ISP_CRG_OUT_MAX] = {
	[CLK_ISP2CSI_FE] = CLK_ISP_FUNC1,
	[CLK_ISP_CVE] = CLK_ISP_FUNC3,
	[CLK_ISP_PE] = CLK_ISP_FUNC2,
	[CLK_ISP_BE] = CLK_ISP_FUNC2,
	[CLK_ISP_FE_CORE2] = CLK_ISP_FUNC1,
	[CLK_ISP_FE_CORE1] = CLK_ISP_FUNC1,
	[CLK_ISP_FE_CORE0] = CLK_ISP_FUNC1,
	[CLK_ISP_FE_ROUTER] = CLK_ISP_FUNC1,
};

const char *xrisp_rate_prop_name[XRISP_CLK_RATE_MAX] = {
	[XRISP_CLK_RATE_075V] = "isp-clock-rate-075",
	[XRISP_CLK_RATE_065V] = "isp-clock-rate-065",
	[XRISP_CLK_RATE_060V] = "isp-clock-rate-060",
	[XRISP_CLK_RATE_AON] = "isp-clock-rate-aon",
};

static void xrisp_crg_clk_release_devs(struct xrisp_clk_data *priv)
{
	xrisp_clk_release_devs(priv);
}

static int xrisp_crg_clk_enable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int ret = 0;
	int i;

	for (i = 0; i < CLK_ISP_CRG_OUT_MAX; i++) {
		if ((1 << i) & mask) {
			int in_clk_id = xrisp_output_clk_source[i];

			ret = xrisp_clk_enable(priv, 1 << in_clk_id);
			if (ret) {
				XRISP_PR_ERROR("xrisp crg enable clk fail, clk_id = %d", in_clk_id);
				return ret;
			}
		}
	}

	return 0;
}

static void xrisp_crg_clk_disable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int i;

	for (i = 0; i < CLK_ISP_CRG_OUT_MAX; i++) {
		if ((1 << i) & mask) {
			int in_clk_id = xrisp_output_clk_source[i];
			unsigned int enable_mask = 0;

			enable_mask = xrisp_clk_disable(priv, 1 << in_clk_id);
			if ((enable_mask & (1 << in_clk_id)) == 0)
				xrisp_clk_deinit_clk(priv, in_clk_id, ISP_CRG_RESET_RATE);
		}
	}
}

static unsigned int xrisp_crg_clk_get_stats(struct xrisp_clk_data *priv)
{
	unsigned int ret = 0;

	ret = xrisp_clk_get_stats(priv);
	return ret;
}

static int xrisp_crg_clk_set_rate_by_mask(struct xrisp_clk_data *priv, unsigned int mask,
								   int volt)
{
	int ret = 0;
	int i;
	unsigned int source_mask = 0;

	if (volt < 0 || volt >= XRISP_CLK_RATE_MAX) {
		XRISP_PR_ERROR("xrisp clk volt err, volt = %d\n", volt);
		return -EINVAL;
	}

	for (i = 0; i < CLK_ISP_CRG_OUT_MAX; i++) {
		if ((1 << i) & mask) {
			int in_clk_id = xrisp_output_clk_source[i];

			source_mask |= 1 << in_clk_id;
		}
	}

	ret = xrisp_clk_set_rate_by_mask(priv, source_mask, volt);
	if (ret) {
		XRISP_PR_ERROR("xrisp crg set clk rate fail, source_mask = %d!!!", source_mask);
		return ret;
	}

	return ret;
}

static int xrisp_crg_clk_enable_init_rate(struct xrisp_clk_data *priv, unsigned int mask,
								   int volt)
{
	int ret = 0;
	int i;

	if (volt < 0 || volt >= XRISP_CLK_RATE_MAX) {
		XRISP_PR_ERROR("xrisp clk volt err, volt = %d\n", volt);
		return -EINVAL;
	}

	for (i = 0; i < CLK_ISP_CRG_OUT_MAX; i++) {
		if ((1 << i) & mask) {
			int in_clk_id = xrisp_output_clk_source[i];

			ret = xrisp_clk_enable_init_rate(priv, 1 << in_clk_id, volt);
			if (ret) {
				XRISP_PR_ERROR("xrisp crg init clk fail, clk id = %d!!!", in_clk_id);
				return ret;
			}
		}
	}

	return ret;
}

static int xrisp_crg_clk_set_rate(struct xrisp_clk_data *priv, int ch, unsigned int rate)
{
	int ret = 0;
	int source_ch;

	if (ch < 0 || ch >= CLK_ISP_CRG_OUT_MAX) {
		ret = -ECHRNG;
		return ret;
	}
	source_ch = xrisp_output_clk_source[ch];
	ret = xrisp_clk_set_rate(priv, source_ch, rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp crg set rate fail, ch = %d", ch);
		return ret;
	}
	return ret;
}

static unsigned int xrisp_crg_clk_get_rate(struct xrisp_clk_data *priv, int ch)
{
	int ret = 0;
	int source_ch;

	if (ch < 0 || ch >= CLK_ISP_CRG_OUT_MAX) {
		ret = -ECHRNG;
		return ret;
	}
	source_ch = xrisp_output_clk_source[ch];
	ret = xrisp_clk_get_rate(priv, source_ch);
#ifdef XRISP_CLK_RATE_DEBUG
	XRISP_PR_INFO("xrisp crg get rate: ch = %d, rate = %u", ch, ret);
#endif
	return ret;
}

static void xrisp_crg_clk_deinit_clk(struct xrisp_clk_data *priv)
{
	int i;

	for (i = 0; i < CLK_ISP_CRG_OUT_MAX; i++) {
		int in_clk_id = xrisp_output_clk_source[i];

		xrisp_clk_disable(priv, 1 << in_clk_id);
	}

	xrisp_clk_deinit_clk(priv, CLK_ISP_FUNC1, ISP_CRG_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_FUNC2, ISP_CRG_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_FUNC3, ISP_CRG_RESET_RATE);
}

static struct xrisp_clk_ops_t xrisp_clk_ops = {
	.enable = xrisp_crg_clk_enable,
	.disable = xrisp_crg_clk_disable,
	.get_stats = xrisp_crg_clk_get_stats,
	.set_rate_by_mask = xrisp_crg_clk_set_rate_by_mask,
	.enable_init_rate = xrisp_crg_clk_enable_init_rate,
	.set_rate = xrisp_crg_clk_set_rate,
	.get_rate = xrisp_crg_clk_get_rate,
	.release = xrisp_crg_clk_release_devs,
	.deinit = xrisp_crg_clk_deinit_clk,
};

int xrisp_crg_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
					   const char *prop_name)
{
	int ret;

	ret = xrisp_clk_init_priv_data(priv, dev, clk_num, XRISP_CLK_RATE_MAX,
			&xrisp_rate_prop_name[0], &xrisp_clk_ops, prop_name);
	if (ret)
		goto release;
	XRISP_PR_INFO("%s succeed !!!", __func__);
	return ret;
release:
	xrisp_clk_release_devs(priv);
	return ret;
}
