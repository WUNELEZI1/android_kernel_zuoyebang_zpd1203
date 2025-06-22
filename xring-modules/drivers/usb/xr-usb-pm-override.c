// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-core.h"

#include <dwc3/core.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>

static int dwc3_suspend(struct device *dev)
{
	dev_info(dev, "enter");
	return 0;
}

static int dwc3_resume(struct device *dev)
{
	dev_info(dev, "enter");
	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	return 0;
}

static void dwc3_complete(struct device *dev)
{
}

static int dwc3_runtime_suspend(struct device *dev)
{
	dev_info(dev, "enter");
	return 0;
}

static int dwc3_runtime_resume(struct device *dev)
{
	dev_info(dev, "enter");
	pm_runtime_mark_last_busy(dev);

	return 0;
}

static int dwc3_runtime_idle(struct device *dev)
{
	dev_info(dev, "enter");
	pm_runtime_mark_last_busy(dev);

	return -EBUSY;
}

static const struct dev_pm_ops xr_dwc3_pm_ops_overrides = {
	.suspend = dwc3_suspend,
	.resume = dwc3_resume,
	.complete = dwc3_complete,
	.runtime_suspend = dwc3_runtime_suspend,
	.runtime_resume = dwc3_runtime_resume,
	.runtime_idle = dwc3_runtime_idle,
};

static int xhci_suspend(struct device *dev)
{
	dev_info(dev, "enter");
	return 0;
}

static int xhci_resume(struct device *dev)
{
	dev_info(dev, "enter");
	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	return 0;
}

static int xhci_restore(struct device *dev)
{
	dev_info(dev, "enter");
	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	return 0;
}

static int __maybe_unused xhci_runtime_suspend(struct device *dev)
{
	dev_info(dev, "enter");
	return 0;
}

static int __maybe_unused xhci_runtime_resume(struct device *dev)
{
	dev_info(dev, "enter");
	return 0;
}

static const struct dev_pm_ops xr_xhci_pm_ops_overrides = {
	.suspend = pm_sleep_ptr(xhci_suspend),
	.resume = pm_sleep_ptr(xhci_resume),
	.freeze = pm_sleep_ptr(xhci_suspend),
	.thaw = pm_sleep_ptr(xhci_resume),
	.poweroff = pm_sleep_ptr(xhci_suspend),
	.restore = pm_sleep_ptr(xhci_restore),

	SET_RUNTIME_PM_OPS(xhci_runtime_suspend,
			   xhci_runtime_resume,
			   NULL)
};

static void
xr_usb_override_dev_pm_ops(struct device *dev,
			   const struct dev_pm_ops *pm_ops_overrides,
			   const struct dev_pm_ops **pm_ops_origin)
{
	if (!dev->driver || !dev->driver->pm) {
		dev_err(dev, "can't override pm_ops\n");
		return;
	}

	*pm_ops_origin = dev->driver->pm;
	dev->driver->pm = pm_ops_overrides;
}

static void xr_usb_recover_dev_pm_ops(struct device *dev,
				      const struct dev_pm_ops **pm_ops_origin)
{
	if (!dev->driver || !dev->driver->pm) {
		dev_err(dev, "can't recover pm_ops\n");
		return;
	}

	dev->driver->pm = *pm_ops_origin;
	*pm_ops_origin = NULL;
}

int xr_usb_override_pm_ops(struct xring_usb *xr_usb)
{
	struct dwc3 *dwc = NULL;

	dwc = platform_get_drvdata(xr_usb->dwc3);
	if (!dwc) {
		dev_err(xr_usb->dev, "Failed to get dwc3 device\n");
		return -ENODEV;
	}
	flush_work(&dwc->drd_work);
	if (!dwc->xhci) {
		dev_err(xr_usb->dev, "Failed to get xhci device\n");
		return -ENODEV;
	}
	xr_usb_override_dev_pm_ops(dwc->dev, &xr_dwc3_pm_ops_overrides,
				   &xr_usb->dwc3_pm_ops_origin);
	xr_usb_override_dev_pm_ops(&dwc->xhci->dev, &xr_xhci_pm_ops_overrides,
				   &xr_usb->xhci_pm_ops_origin);

	return 0;
}

void xr_usb_recover_pm_ops(struct xring_usb *xr_usb)
{
	struct dwc3 *dwc = NULL;

	dwc = platform_get_drvdata(xr_usb->dwc3);
	if (dwc) {
		xr_usb_recover_dev_pm_ops(dwc->dev, &xr_usb->dwc3_pm_ops_origin);
		if (dwc->xhci)
			xr_usb_recover_dev_pm_ops(&dwc->xhci->dev,
						  &xr_usb->xhci_pm_ops_origin);
	} else {
		dev_err(xr_usb->dev, "Failed to get dwc3 device\n");
	}
}
