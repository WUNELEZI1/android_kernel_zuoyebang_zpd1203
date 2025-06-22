/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * xr-usb-reset-plat.h - Platform code for Reset control.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_RESET_PLAT_H
#define _XR_USB_RESET_PLAT_H

#include <dt-bindings/xring/xr-usb-reset.h>
#include <linux/bitops.h>
#include <linux/types.h>

enum xr_usb_reg_type {
	XR_USB_REG_TYPE_SCR = 0,
	XR_USB_REG_TYPE_RW,
	XR_USB_REG_TYPE_BMRW,
};

struct xring_usb_reset_data {
	int base_idx;
	enum xr_usb_reg_type type;
	bool active_low;
	unsigned int offset;
	unsigned int mask;
	unsigned int value;
};

struct xring_usb_reset_plat {
	int num_reg_base;
	struct xring_usb_reset_data resets_data[USB_RESET_IDX_MAX];
};

/* O1 platform data for xr-usb-reset */
#if IS_ENABLED(CONFIG_XRING_USB_O1)
#include <dt-bindings/xring/platform-specific/hss1_crg.h>
#include <dt-bindings/xring/platform-specific/usb_sctrl.h>

#define USBSYS_APB_N                                                           \
	(BIT(HSS1_CRG_RST0_W1S_IP_PRST_USB_EUSB_APB_N_SHIFT) |                 \
	 BIT(HSS1_CRG_RST0_W1S_IP_PRST_USB_COMBOPHY_TCA_N_SHIFT) |             \
	 BIT(HSS1_CRG_RST0_W1S_IP_PRST_USB_COMBOPHY_APB1_N_SHIFT) |            \
	 BIT(HSS1_CRG_RST0_W1S_IP_PRST_USB_COMBOPHY_APB0_N_SHIFT))
#define USB_VBUS_VALID BIT(USB_CC_SEL_USB_VBUS_VALID_CC_SHIFT)
#define USB_CONTROLLER_REG_N BIT(USB_IP_W1S_IP_RST_CONTROLLER_REG_N_SHIFT)

static struct xring_usb_reset_plat xr_usb_reset_o1_plat = {
	.num_reg_base = 2,
	.resets_data[USB_RESET_IDX_USBSYS_APB] = { 0, XR_USB_REG_TYPE_SCR,
							true,
							HSS1_CRG_RST0_W1S,
							USBSYS_APB_N,
							USBSYS_APB_N },
	.resets_data[USB_IDX_VBUS_VALID] = { 1, XR_USB_REG_TYPE_RW,
							true,
							USB_USB_VBUS_VALID_CC_SEL,
							USB_VBUS_VALID,
							USB_VBUS_VALID },

	.resets_data[USB_RESET_IDX_USB_CONTROLLER] = { 1, XR_USB_REG_TYPE_SCR,
							true,
							USB_IP_RST_CONTROLLER_REG_N_W1S,
							USB_CONTROLLER_REG_N,
							USB_CONTROLLER_REG_N },
};

#define SET_XR_USB_RESET_O1_DATA() .data = &xr_usb_reset_o1_plat,
#else
#define SET_XR_USB_RESET_O1_DATA()
#endif

#endif /* _XR_USB_RESET_PLAT_H */
