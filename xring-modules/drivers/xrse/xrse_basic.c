// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 * Description: xrse driver
 * Modify time: 2024-04-03
 */

#include "dfx/xrse_mdr.h"
#include "xrse_cmd.h"
#include "xrse_internal.h"
#include "secboot/secboot_status_check.h"
#include "soc/xring/xrse/xrse.h"
#include "soc/xring/xrse/secboot_verify.h"
#include <linux/arm-smccc.h>
#include <linux/preempt.h>
#include <linux/memory.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/arm-smccc.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/irqreturn.h>
#include <linux/version.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/lpis_actrl.h>

#define MODULE_NAME (256)
#define XRSE_READY_STATUS_REG      (ACPU_LPIS_ACTRL + LPIS_ACTRL_SC_RSV_NS_1)
#define XRSE_READY_STATUS_REG_SIZE (4) /* 4Bytes */
#define USRRW_GRPRW_MODE           (0660)

static void __iomem *g_xrse_status_reg;
static struct platform_device *g_xrse_dev;

static ssize_t xrse_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;

	xrse_debug("enter xrse show\n");

	ret = xrse_cmd_res_show(buf);
	if (ret < 0) {
		xrse_err("result show fail. ret:%d\n", ret);
		return ret;
	}

	xrse_debug("result show success\n");
	return ret;
}

static ssize_t xrse_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;

	xrse_debug("enter xrse store, buf:%s, count:%zu", buf, count);

	/* reading out xrse show cmd input */
	ret = xrse_parse_cmd_store(buf, count);
	if (ret) {
		xrse_err("failed to parse and exec cmd input, ret:%d\n", ret);
		return ret;
	}

	xrse_debug("store success\n");

	return count;
}

static DEVICE_ATTR(xrse, USRRW_GRPRW_MODE, xrse_show, xrse_store);

static struct attribute *g_xrse_attrs[] = {
	&dev_attr_xrse.attr,
	NULL,
};

static struct attribute_group g_xrse_attr_group = {
	.attrs = g_xrse_attrs,
};

bool is_xrse_ready(void)
{
	return (readl(g_xrse_status_reg) & (1 << XRSE_READY_STATUS_SHIFT)) ? true : false;
}

static void xrse_register_sub_pdev(void)
{
	int ret;

	ret = xrse_dfx_init();
	if (ret != 0) {
		pr_err("fail to register xrse dfx\n");
		goto xrse_dfx_fail;
	}

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	ret = secboot_verify_ca_init();
	if (ret != 0) {
		pr_err("fail to register xrse secboot\n");
		goto xrse_secboot_fail;
	}
#endif

	ret = kernel_rodata_rip_request();
	if (ret != 0)
		pr_err("fail to request kernel rodata rip\n");

	return;

#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
xrse_secboot_fail:
	secboot_verify_ca_exit();
#endif
xrse_dfx_fail:
	xrse_dfx_exit();
}

static void xrse_unregister_sub_pdev(void)
{
	xrse_dfx_exit();
#if IS_ENABLED(CONFIG_XRING_MITEE_SUPPORT)
	secboot_verify_ca_exit();
#endif
}

static int xrse_dev_probe(struct platform_device *pdev)
{
	int ret;

	pr_info("xrse dev probe\n");

	ret = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (ret)
		pr_info("xrse add child nodes failed, ret=%d\n", ret);
	else
		pr_info("xrse add child nodes success\n");

	ret = sysfs_create_group(&pdev->dev.kobj, &g_xrse_attr_group);
	if (ret) {
		pr_err("xrse failed to create sysfs group!\n");
		return ret;
	}

	g_xrse_dev = pdev;

	return ret;
}

static int xrse_dev_remove(struct platform_device *pdev)
{
	pr_info("xrse dev remove\n");
	sysfs_remove_file(&pdev->dev.kobj, &(dev_attr_xrse.attr));
	return 0;
}

static const struct of_device_id xrse_of_match[] = {
	{ .compatible = "xring,xrse" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xrse_of_match);

static struct platform_driver xrse_driver = {
	.probe = xrse_dev_probe,
	.remove = xrse_dev_remove,
	.driver = {
		.name = "xrse-dev",
		.of_match_table = of_match_ptr(xrse_of_match),
	},
};

static int xrse_basic_init(void)
{
	return platform_driver_register(&xrse_driver);
}

static void xrse_basic_exit(void)
{
	platform_driver_unregister(&xrse_driver);
}

static int __init xrse_init(void)
{
	pr_info("xrse driver init++\n");

	g_xrse_status_reg = ioremap(XRSE_READY_STATUS_REG, XRSE_READY_STATUS_REG_SIZE);
	if (!g_xrse_status_reg) {
		pr_err("xrse status reg ioremap failed\n");
		return -ENOMEM;
	}

	xrse_register_sub_pdev();
	xrse_basic_init();

	pr_info("xrse driver init--\n");

	return 0;
}

static void __exit xrse_exit(void)
{
	if (g_xrse_status_reg != NULL) {
		iounmap(g_xrse_status_reg);
		g_xrse_status_reg = NULL;
	}

	xrse_basic_exit();
	xrse_unregister_sub_pdev();
}

struct platform_device *xrse_get_device(void)
{
	return g_xrse_dev;
}

MODULE_DESCRIPTION("XRing xrse driver");
MODULE_LICENSE("GPL v2");
module_init(xrse_init);
module_exit(xrse_exit);
