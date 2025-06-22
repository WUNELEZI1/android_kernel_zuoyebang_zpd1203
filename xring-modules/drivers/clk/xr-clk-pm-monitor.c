// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/syscore_ops.h>
#include "xr-clk-common.h"


static HLIST_HEAD(clocks_pm);

struct pmclk_node {
	const char *name;
	struct clk *clk;
	struct hlist_node node;
};

static int xr_clk_suspend(void)
{
	struct pmclk_node *pmclk = NULL;

	clkinfo("kernel clock check status!\n");
	hlist_for_each_entry(pmclk, &clocks_pm, node) {
		if (__clk_is_enabled(pmclk->clk)) {
			clkwarn("[warning]Sus_Clock_Check: [%s] enabled!\n", pmclk->name);
			if (IS_ENABLED(CONFIG_XRING_CLK_PM_RESET_SYS))
				panic("clock not disable before suspend!\n");
		}
	}

	return 0;
}

static struct syscore_ops xr_clk_syscore_ops = {
	.suspend = xr_clk_suspend,
};

static int clk_pm_monitor_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct pmclk_node *pm_clk = NULL;
	unsigned int index;
	int ret;
	int num;

	num = of_property_count_strings(np, "clock-names");
	if (num < 0) {
		clkerr("find clock-names attribute number failed,ret=%d!\n", num);
		ret = num;
		goto err;
	}

	for (index = 0; index < num; index++) {
		pm_clk = devm_kzalloc(dev, sizeof(*pm_clk), GFP_KERNEL);
		if (IS_ERR_OR_NULL(pm_clk)) {
			clkerr("fail to alloc pm_clk node!\n");
			ret = -ENOMEM;
			goto err;
		}
		ret = of_property_read_string_index(np, "clock-names", index, &(pm_clk->name));
		if (ret) {
			clkerr("Failed to get clock-names attributes, index=%d\n", index);
			goto err;
		}
		pm_clk->clk = devm_clk_get(dev, pm_clk->name);
		if (IS_ERR_OR_NULL(pm_clk->clk)) {
			clkerr("%s get clk handle failed!\n", pm_clk->name);
			ret = -ENODEV;
			goto err;
		}
		/* add clk handle to list */
		hlist_add_head(&pm_clk->node, &clocks_pm);
	}

	/* register clk suspend ops */
	register_syscore_ops(&xr_clk_syscore_ops);
err:
	return ret;
}

static int clk_pm_monitor_remove(struct platform_device *pdev)
{
	clkinfo("clk pm monitor removed!\n");
	return 0;
}


static const struct of_device_id clk_pm_monitor_of_match[] = {
	{ .compatible = "xring,clk-pm-monitor" },
	{},
};
MODULE_DEVICE_TABLE(of, clk_pm_monitor_of_match);

static struct platform_driver clk_pm_monitor_driver = {
	.probe          = clk_pm_monitor_probe,
	.remove         = clk_pm_monitor_remove,
	.driver         = {
		.name   = "clk-monitor",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(clk_pm_monitor_of_match),
	},
};

int xr_clk_pm_monitor_init(void)
{
	return platform_driver_register(&clk_pm_monitor_driver);
}
EXPORT_SYMBOL_GPL(xr_clk_pm_monitor_init);

void xr_clk_pm_monitor_exit(void)
{
	platform_driver_unregister(&clk_pm_monitor_driver);
	clkinfo("clk pm monitor exit!\n");
}
EXPORT_SYMBOL_GPL(xr_clk_pm_monitor_exit);
