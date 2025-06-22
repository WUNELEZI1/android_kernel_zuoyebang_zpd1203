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

#define ISP_MCU_RESET_RATE 104450000
#define ISP_REF_RESET_RATE 19200000
#define ISP_TIMER_RESET_RATE 33000
#define ISP_WTD_RESET_RATE 33000

const char *xrisp_mcu_rate_prop_name[XRISP_MCU_CLK_RATE_MAX] = {
	[XRISP_MCU_CLK_RATE_1200M] = "mcu-clock-rate-1200M",
	[XRISP_MCU_CLK_RATE_557M] = "mcu-clock-rate-557M",
};

static void xrisp_mcu_clk_release_devs(struct xrisp_clk_data *priv)
{
	xrisp_clk_release_devs(priv);
}

static int xrisp_mcu_clk_enable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int ret = 0;

	ret = xrisp_clk_enable(priv, mask);
	if (ret) {
		XRISP_PR_ERROR("xrisp mcu enable clk fail, mask = %u", mask);
		return ret;
	}
	return 0;
}

static void xrisp_mcu_clk_disable(struct xrisp_clk_data *priv, unsigned int mask)
{
	int i;
	unsigned int enable_mask = 0;

	enable_mask = xrisp_clk_disable(priv, mask);
	for (i = 0; i < ISP_MCU_OUT_MAX; i++) {
		if ((mask & (1 << i)) && ((enable_mask & (1 << i)) == 0)) {
			if (i == CLK_ISP_MCU || i == CLK_ISP_I2C || i == CLK_ISP_I3C)
				xrisp_clk_deinit_clk(priv, i, ISP_MCU_RESET_RATE);
			else if (i == CLK_ISP_REF)
				xrisp_clk_deinit_clk(priv, i, ISP_REF_RESET_RATE);
			else if (i == CLK_ISP_TIMER)
				xrisp_clk_deinit_clk(priv, i, ISP_TIMER_RESET_RATE);
			else if (i == CLK_ISP_WTD)
				xrisp_clk_deinit_clk(priv, i, ISP_WTD_RESET_RATE);
		}
	}
}

static unsigned int xrisp_mcu_clk_get_stats(struct xrisp_clk_data *priv)
{
	unsigned int ret = 0;

	ret = xrisp_clk_get_stats(priv);
	return ret;
}

static int xrisp_mcu_clk_set_rate_by_mask(struct xrisp_clk_data *priv, unsigned int mask,
			int mcu_rate)
{
	int ret = 0;

	if (mcu_rate < 0 || mcu_rate >= XRISP_MCU_CLK_RATE_MAX) {
		XRISP_PR_ERROR("xrisp mcu clk rate_index err, rate_index = %d\n", mcu_rate);
		return -EINVAL;
	}

	ret = xrisp_clk_set_rate_by_mask(priv, mask, mcu_rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp mcu clk set rate by mask fail!!!");
		return ret;
	}
	return ret;
}

static int xrisp_mcu_clk_enable_init_rate(struct xrisp_clk_data *priv, unsigned int mask,
			int mcu_rate)
{
	int ret = 0;

	if (mcu_rate < 0 || mcu_rate >= XRISP_MCU_CLK_RATE_MAX) {
		XRISP_PR_ERROR("xrisp mcu clk rate_index err, rate_index = %d\n", mcu_rate);
		return -EINVAL;
	}

	ret = xrisp_clk_enable_init_rate(priv, mask, mcu_rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp mcu init clk fail!!!");
		return ret;
	}
	return ret;
}

static int xrisp_mcu_clk_set_rate(struct xrisp_clk_data *priv, int ch, unsigned int rate)
{
	int ret = 0;

	ret = xrisp_clk_set_rate(priv, ch, rate);
	if (ret) {
		XRISP_PR_ERROR("xrisp mcu set rate fail, ch = %d", ch);
		return ret;
	}
	return ret;
}

static unsigned int xrisp_mcu_clk_get_rate(struct xrisp_clk_data *priv, int ch)
{
	unsigned int ret = 0;

	ret = xrisp_clk_get_rate(priv, ch);
#ifdef XRISP_CLK_RATE_DEBUG
	XRISP_PR_INFO("xrisp mcu get rate: ch = %d, rate = %u", ch, ret);
#endif
	return ret;
}

static void xrisp_mcu_clk_deinit_clk(struct xrisp_clk_data *priv)
{
	xrisp_clk_disable(priv, (1 << ISP_MCU_OUT_MAX) - 1);
	xrisp_clk_deinit_clk(priv, CLK_ISP_MCU, ISP_MCU_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_I2C, ISP_MCU_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_I3C, ISP_MCU_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_REF, ISP_REF_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_TIMER, ISP_TIMER_RESET_RATE);
	xrisp_clk_deinit_clk(priv, CLK_ISP_WTD, ISP_WTD_RESET_RATE);
}

static struct xrisp_clk_ops_t xrisp_mcu_clk_ops = {
	.enable = xrisp_mcu_clk_enable,
	.disable = xrisp_mcu_clk_disable,
	.get_stats = xrisp_mcu_clk_get_stats,
	.set_rate_by_mask = xrisp_mcu_clk_set_rate_by_mask,
	.enable_init_rate = xrisp_mcu_clk_enable_init_rate,
	.set_rate = xrisp_mcu_clk_set_rate,
	.get_rate = xrisp_mcu_clk_get_rate,
	.release = xrisp_mcu_clk_release_devs,
	.deinit = xrisp_mcu_clk_deinit_clk,
};

int xrisp_mcu_clk_init(struct xrisp_clk_data *priv, struct device *dev, int clk_num,
					   const char *prop_name)
{
	int ret;

	ret = xrisp_clk_init_priv_data(priv, dev, clk_num, XRISP_MCU_CLK_RATE_MAX,
		&xrisp_mcu_rate_prop_name[0], &xrisp_mcu_clk_ops, prop_name);
	if (ret)
		goto release;
	XRISP_PR_INFO("%s succeed !!!", __func__);
	return ret;
release:
	xrisp_mcu_clk_release_devs(priv);
	return ret;
}
