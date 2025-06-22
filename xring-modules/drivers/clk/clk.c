// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "clk.h"
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include "platform/clk-o1.h"
#include "xr-clk-pm-monitor.h"

static int xring_clk_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret = 0;

	unsigned long type = (unsigned long)of_device_get_match_data(dev);

	if (type >= XR_CLK_MAX_BASECRG) {
		clkerr("invalid xr clk base enum: %lu\n", type);
		return -EINVAL;
	}

	media_power_votemng_init(dev, type);

	/* parse clk reg base address */
	ret = crg_base_address_init(pdev, type);

	return ret;
}

static int xring_clk_remove(struct platform_device *pdev)
{
	media_power_votemng_exit();
	clkinfo("Clk Driver Remove!\n");
	return 0;
}

static const struct of_device_id xring_clk_of_match[] = {
	{ .compatible = "xring,clk_peri_crgctrl",      .data = (void *)XR_CLK_CRGCTRL },
	{ .compatible = "xring,clk_media1_crgctrl",    .data = (void *)XR_CLK_MEDIA1CRG },
	{ .compatible = "xring,clk_media2_crgctrl",    .data = (void *)XR_CLK_MEDIA2CRG },
	{ .compatible = "xring,clk_hss1_crgctrl",      .data = (void *)XR_CLK_HSIF1CRG },
	{ .compatible = "xring,clk_hss2_crgctrl",      .data = (void *)XR_CLK_HSIF2CRG },
	{ .compatible = "xring,clk_lms_crgctrl",       .data = (void *)XR_CLK_LMSCRG },
	{ .compatible = "xring,lpis_crgctrl",          .data = (void *)XR_CLK_LPISCRG },
	{ .compatible = "xring,clk_lpisactrl_crgctrl", .data = (void *)XR_CLK_LPISACTRLCRG },
	{ .compatible = "xring,clk_dvs_peri_crgctrl",  .data = (void *)XR_CLK_DVSPERI },
	{ .compatible = "xring,clk_dvs_media_crgctrl", .data = (void *)XR_CLK_DVSMEDIA },
	{ /* Sentinel */ },
};
MODULE_DEVICE_TABLE(of, xring_clk_of_match);

static struct platform_driver xring_clk_driver = {
	.probe = xring_clk_probe,
	.remove = xring_clk_remove,
	.driver = {
		.name = "xr-clk-base",
		/*
		 * Disable bind attributes: clocks are not removed and
		 * reloading the driver will crash or break devices.
		 */
		.suppress_bind_attrs = true,
		.of_match_table = of_match_ptr(xring_clk_of_match),
	},
};

static int __init xring_clk_init(void)
{
	int ret;

	clk_feature_state_init();

	/* parse crg base address and provide for other c file */
	ret = platform_driver_register(&xring_clk_driver);
	if (ret) {
		pr_err("platform_driver_register failed: %d\n", ret);
		return ret;
	}

	/* register all clock reg-info to kernel */
	ret = xring_clocks_init();
	if (ret) {
		clkerr("xring clocks init,ret=%d\n", ret);
		return ret;
	}

	ret = xr_clk_pm_monitor_init();
	if (ret) {
		clkerr("clk pm monitor init fail,ret=%d\n", ret);
		return ret;
	}

	return ret;
}
subsys_initcall(xring_clk_init);

static void __exit xring_clk_exit(void)
{
	xr_clk_pm_monitor_exit();
	xring_clocks_exit();
	/* unregister clk platform driver */
	platform_driver_unregister(&xring_clk_driver);
}
module_exit(xring_clk_exit);

MODULE_SOFTDEP("pre: xr_vote_mng");
MODULE_SOFTDEP("pre: xr-pmic-spmi");
MODULE_SOFTDEP("pre: xr_doze");
MODULE_AUTHOR("Shaobo Zheng <zhengshaobo1@xiaomi.com>");
MODULE_DESCRIPTION("XRing Clock Driver");
MODULE_LICENSE("GPL v2");
