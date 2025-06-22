// SPDX-License-Identifier: GPL-2.0
/*
 * core.c - Core Driver for XRing USB ADSP.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#include <soc/xring/xr_usbdp_event.h>

#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__

struct xr_usb_adsp_stub {
	struct xring_usb *xr_usb;
	struct xr_usb_adsp_ops ops;
};

static int xr_usb_adsp_stub_start(void *context, struct platform_device *xhci)
{
	return -ENOENT;
}

int xr_usb_adsp_stub_stop(void *context)
{
	return -ENOENT;
}

static int xr_usb_adsp_stub_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct xr_usb_adsp_stub *usb_adsp = NULL;

	usb_adsp = devm_kzalloc(dev, sizeof(*usb_adsp), GFP_KERNEL);
	if (!usb_adsp)
		return -ENOMEM;

	usb_adsp->ops.start = xr_usb_adsp_stub_start;
	usb_adsp->ops.stop = xr_usb_adsp_stub_stop;
	usb_adsp->ops.context = usb_adsp;
	pm_runtime_no_callbacks(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0)
		goto err_pm_put;

	pm_runtime_forbid(dev);

	usb_adsp->xr_usb = xr_usb_port_of_get(dev->of_node);
	if (IS_ERR_OR_NULL(usb_adsp->xr_usb)) {
		dev_err(dev, "failed to get xr_usb\n");
		usb_adsp->xr_usb = NULL;
		ret = -EPROBE_DEFER;
		goto err_pm_put;
	}

	ret = xr_usb_adsp_ops_register(usb_adsp->xr_usb, &usb_adsp->ops);
	if (ret) {
		dev_err(dev, "failed to register ops\n");
		goto err_xr_usb_put;
	}

	platform_set_drvdata(pdev, usb_adsp);
	pm_runtime_put(dev);
	dev_info(dev, "probe finished\n");

	return 0;

err_xr_usb_put:
	xr_usb_port_of_put(usb_adsp->xr_usb);
	usb_adsp->xr_usb = NULL;
err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);
	devm_kfree(dev, usb_adsp);
	usb_adsp = NULL;
	return ret;
}

static int xr_usb_adsp_stub_remove(struct platform_device *pdev)
{
	struct xr_usb_adsp_stub *usb_adsp = platform_get_drvdata(pdev);

	pm_runtime_get_sync(&pdev->dev);

	xr_usb_adsp_ops_unregister(usb_adsp->xr_usb);
	xr_usb_port_of_put(usb_adsp->xr_usb);
	usb_adsp->xr_usb = NULL;

	pm_runtime_disable(&pdev->dev);
	pm_runtime_put_noidle(&pdev->dev);
	pm_runtime_set_suspended(&pdev->dev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, usb_adsp);

	return 0;
}

static const struct of_device_id xr_usb_adsp_of_match[] = {
	{ .compatible = "xring,usb-adsp" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_usb_adsp_of_match);

static struct platform_driver xr_usb_adsp_driver = {
	.probe	= xr_usb_adsp_stub_probe,
	.remove	= xr_usb_adsp_stub_remove,
	.driver	= {
		.name = "xring_usb_adsp_stub",
		.of_match_table = xr_usb_adsp_of_match,
	},
};
module_platform_driver(xr_usb_adsp_driver);

MODULE_SOFTDEP("pre: xring_usb");
MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("USB ADSP Driver For Recovery Mode");
