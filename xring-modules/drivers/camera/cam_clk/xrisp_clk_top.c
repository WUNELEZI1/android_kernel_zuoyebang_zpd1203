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
#include <linux/platform_device.h>
#include <linux/err.h>
#include "xrisp_clk.h"
#include "xrisp_log.h"

struct xrisp_clk_data *xrisp_clk_priv_p;

static struct xrisp_clk_init_data clk_init_data[] = {
	{XRISP_CRG_CLK, "xrisp_crg_clk", CLK_ISP_CRG_IN_MAX, xrisp_crg_clk_init},
	{XRISP_MCU_CLK, "xrisp_mcu_clk", ISP_MCU_OUT_MAX, xrisp_mcu_clk_init},
	{XRISP_DEBUG_CLK, "xrisp_debug_clk", ISP_DEBUG_OUT_MAX, xrisp_debug_clk_init},
	{XRISP_PERF_CLK, "xrisp_perf_clk", ISP_PERF_OUT_MAX, xrisp_perf_clk_init},
};

static int xrisp_clk_probe(struct platform_device *pdev)
{
	int ret = 0;
	int i;
	int clk_type_num;

	clk_type_num = ARRAY_SIZE(clk_init_data);

	XRISP_PR_INFO("enter xrisp_clk probe");
	xrisp_clk_priv_p =
		devm_kzalloc(&pdev->dev, sizeof(struct xrisp_clk_data) * clk_type_num, GFP_KERNEL);
	if (xrisp_clk_priv_p == NULL) {
		XRISP_PR_ERROR("%s Out of memory", __func__);
		return -ENOMEM;
	}
	memset(xrisp_clk_priv_p, 0, sizeof(struct xrisp_clk_data) * clk_type_num);
	if (CLK_ISP_CRG_IN_MAX + ISP_MCU_OUT_MAX
		> of_property_count_strings(pdev->dev.of_node, "clock-names")) {
		XRISP_PR_ERROR("xrisp clk name number error in dts");
		return -EINVAL;
	}

	for (i = 0; i < clk_type_num; i++) {
		enum xrisp_clk_e clk_id = clk_init_data[i].clk_id;
		const char *clk_prop_name = clk_init_data[i].prop_name;
		int clk_num = clk_init_data[i].clk_num;
		struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk_id];

		clk_init_data[i].clk_init_func(priv, &pdev->dev, clk_num, clk_prop_name);
	}

	platform_set_drvdata(pdev, xrisp_clk_priv_p);

	XRISP_PR_INFO("enter xrisp_clk debug init");
	for (i = 0; i < clk_type_num; i++) {
		enum xrisp_clk_e clk_id = clk_init_data[i].clk_id;
		struct xrisp_clk_data *priv = &xrisp_clk_priv_p[clk_id];

		clk_debugfs_init(priv);
	}

	return ret;
}

static int xrisp_clk_remove(struct platform_device *pdev)
{
	int i;

	clk_debugfs_exit();

	if (xrisp_clk_priv_p) {
		for (i = 0; i < XRISP_CLK_MAX; i++) {
			if (xrisp_clk_priv_p[i].ops && xrisp_clk_priv_p[i].ops->release)
				xrisp_clk_priv_p[i].ops->release(&xrisp_clk_priv_p[i]);
		}
		xrisp_clk_priv_p = NULL;
	}

	return 0;
}

static const struct of_device_id xrisp_clk_of_match[] = {
	{
		.compatible = "xring,cam-device-clk",
	},
	{},
};

static struct platform_driver xrisp_clk_driver = {
	.probe = xrisp_clk_probe,
	.remove = xrisp_clk_remove,
	.driver = {
		.name = "cam-device-clk",
		.of_match_table = xrisp_clk_of_match,
	},
};

int xrisp_clk_init(void)
{
	return platform_driver_register(&xrisp_clk_driver);
}

void xrisp_clk_exit(void)
{
	platform_driver_unregister(&xrisp_clk_driver);
}

MODULE_AUTHOR("dongjunbo <dongjunbo@xiaomi.com>");
MODULE_DESCRIPTION("Clk Driver for ISP");
MODULE_LICENSE("GPL v2");
