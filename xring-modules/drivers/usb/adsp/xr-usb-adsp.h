/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_ADSP_H
#define _XR_USB_ADSP_H

#include <linux/err.h>
#include <linux/types.h>

struct device_node;
struct xring_usb_adsp;

#if IS_ENABLED(CONFIG_XRING_USB_ADSP)
struct xring_usb_adsp *of_xr_usb_adsp_get(const struct device_node *node);
void of_xr_usb_adsp_put(struct xring_usb_adsp *usb_adsp);
#else
static inline struct xring_usb_adsp *
of_xr_usb_adsp_get(struct device_node *node)
{
	return NULL;
}

static inline void of_xr_usb_adsp_put(struct xring_usb_adsp *usb_adsp)
{
}

#endif

#endif /* _XR_USB_ADSP_H */
