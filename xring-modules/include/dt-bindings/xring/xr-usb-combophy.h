/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * xr-usb-hsphy.h - Constant define for XRing USB HS Phy.
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __DT_BINDINGS_XR_USB_COMBOPHY_H__
#define __DT_BINDINGS_XR_USB_COMBOPHY_H__

#define USB_COMBOPHY_IDX_DWC3 0
#define USB_COMBOPHY_IDX_XR_USB 1
#define USB_COMBOPHY_IDX_MAX 2

#define XR_USB_COMBOPHY_CELL_ID 0
#define XR_USB_COMBOPHY_CELL_SIZE 1

#define SSC_PEAK_IDX 0
#define SSC_STEPSIZE_IDX 1
#define EYE_CURSOR_IDX 2
#define EYE_BOOST_IDX 3
#define COMBOPHY_EYE_PARAM_MAX 4

#endif /* __DT_BINDINGS_XR_USB_HSPHY_H__ */
