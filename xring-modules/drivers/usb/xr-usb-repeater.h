/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_REPEATER_H
#define _XR_USB_REPEATER_H

#include <linux/errno.h>
#include <linux/device.h>
#include <linux/phy/phy.h>
#include <linux/types.h>

struct usb_repeater  {
	struct device		*dev;

	int	(*reset)(struct usb_repeater *x, bool set);
	int	(*init)(struct usb_repeater *x, enum phy_mode mode);
	int	(*suspend)(struct usb_repeater *r, int suspend);
};

#if IS_ENABLED(CONFIG_XRING_USB_REPEATER)
struct usb_repeater *usb_get_repeater_by_phandle(struct device *dev,
		const char *phandle, u8 index);
void usb_put_repeater(struct usb_repeater *r);
#else
static inline struct usb_repeater *
usb_get_repeater_by_phandle(struct device *dev,
				 const char *phandle, u8 index)
{
	return NULL;
}

static inline void usb_put_repeater(struct usb_repeater *r)
{
}
#endif

static inline int usb_repeater_reset(struct usb_repeater *r,
				bool set)
{
	if (r && r->reset != NULL)
		return r->reset(r, set);
	else
		return 0;
}

static inline int usb_repeater_init(struct usb_repeater *r, enum phy_mode mode)
{
	if (r && r->init != NULL)
		return r->init(r, mode);
	else
		return 0;
}
#endif /* _XR_USB_REPEATER */
