/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * debugfs.h - Debugfs for XRing USB ADSP Module
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_ADSP_DEBUGFS_H
#define _XR_USB_ADSP_DEBUGFS_H

struct xring_usb_adsp;
struct dentry;

#if IS_ENABLED(CONFIG_XRING_USB_DEBUG)
int xr_usb_adsp_debugfs_init(struct xring_usb_adsp *usb_adsp);
void xr_usb_adsp_debugfs_exit(struct xring_usb_adsp *usb_adsp);
#else
static inline int xr_usb_adsp_debugfs_init(struct xring_usb_adsp *usb_adsp)
{
	return 0;
}

static inline void xr_usb_adsp_debugfs_exit(struct xring_usb_adsp *usb_adsp)
{
}
#endif

#endif /* _XR_USB_ADSP_DEBUGFS_H */
