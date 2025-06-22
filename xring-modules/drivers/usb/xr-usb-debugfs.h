/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * xr-usb-debugfs.h - Debugfs for XRing USB Module
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_DEBUGFS_H
#define _XR_USB_DEBUGFS_H

#include <linux/seq_file.h>

struct dentry;
struct xring_usb;

#if IS_ENABLED(CONFIG_XRING_USB_DEBUG)
int xr_usb_debugfs_init(struct xring_usb *xr_usb);
void xr_usb_debugfs_exit(struct dentry **root);
#else
static inline int xr_usb_debugfs_init(struct xring_usb *xr_usb)
{
	return 0;
}

static inline void xr_usb_debugfs_exit(struct dentry **root)
{
}
#endif

#endif /* _XR_USB_DEBUGFS_H */
