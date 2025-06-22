/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_UTIL_H
#define _XR_USB_UTIL_H

#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/types.h>

static inline void __iomem *
devm_ioremap_resource_byname_no_req_region(struct platform_device *pdev,
					   const char *name)
{
	struct resource *res = NULL;

	if (!pdev || !name)
		return NULL;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		dev_err(&pdev->dev, "failed to get \"%s\" memory property\n",
			name);
		return NULL;
	}

	return devm_ioremap(&pdev->dev, res->start, resource_size(res));
}
#endif /* _XR_USB_UTIL_H */
