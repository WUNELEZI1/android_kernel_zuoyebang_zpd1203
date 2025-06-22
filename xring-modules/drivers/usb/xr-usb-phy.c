// SPDX-License-Identifier: GPL-2.0
/*
 * xr-usb-phy.c - Phy Driver for XRing USB.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#include "xr-usb-generic-phy.h"

#include <linux/kernel.h>
#include <linux/module.h>

#undef pr_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__

static int __init usb_phy_init(void)
{
	int ret;

	ret = usb_hsphy_init();
	if (ret) {
		pr_err("hsphy_init fail");
		goto out;
	}

	ret = usb_combophy_init();
	if (ret) {
		pr_err("combophy_init fail");
		usb_hsphy_exit();
	}

out:
	return ret;
}

static void usb_phy_exit(void)
{
	usb_hsphy_exit();
	usb_combophy_exit();
}

module_init(usb_phy_init);
module_exit(usb_phy_exit);

MODULE_SOFTDEP("pre: xring_usb_repeater");
MODULE_SOFTDEP("post: xring_usb");
MODULE_AUTHOR("Huinan Gu <guhuinan@xiaomi.com>");
MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PHY Driver For XRing USB Module");
