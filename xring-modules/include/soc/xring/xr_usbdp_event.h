/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 */

#ifndef __XR_USBDP_EVENT_H__
#define __XR_USBDP_EVENT_H__

#include <linux/of.h>
#include <linux/platform_device.h>

#define USB_MODE_UNPLUG	    0u
#define USB_MODE_PERIPEHRAL 1u
#define USB_MODE_HOST	    2u

#define XR_USB_EVENT_TYPE_MASK	0x03
#define USBDP_EVENT_TYPE(n)	((u32)(n)&XR_USB_EVENT_TYPE_MASK)
#define XR_USB_EVENT_TYPE_USBDP		0u
#define XR_USB_EVENT_TYPE_DP		1u
#define XR_USB_EVENT_TYPE_DYNAMIC	2u
#define XR_USB_EVENT_TYPE_ADSP		3u

#define ORIG_NORMAL 0u
#define ORIG_FLIP   1u

#define NC_MODE		0u
#define USB_MODE	1u
#define DP_MODE		2u
#define USBDP_MODE	3u
#define USBDP_MODE_MASK 0x03

/*
 * struct usb_event_info - usbdp event
 * @type:               For usb event, type must set to XR_USB_EVENT_TYPE_USBDP(0u)
 * @mode:               USB event mode
 * @typec_orig:         typec direction(ORIG_NORMAL/ORIG_FLIP)
 * @usbdp_mode:         usbdp mode
 * @dp_hpd:             dp_hpd feature set
 * @reserved31_9:       wait for extend
 */
struct usb_event_info {
	unsigned int type : 2;
	unsigned int mode : 2; /* USB_MODE:UNPLUG/PERIPEHRAL/HOST/ADSP_HOST */
	/* property */
	unsigned int typec_orig : 1; /* normal side or flip side*/
	unsigned int usbdp_mode : 2;
	unsigned int dp_hpd : 2;

	unsigned int reserved31_9 : 23;
} __packed;

/*
 * struct dp_event_info - dp only event
 * @type:               For dp event, type must set to XR_USB_EVENT_TYPE_DP(1u)
 * @dp_hpd:             dp_hpd feature set
 * @reserved31_4:       wait for extend
 */
struct dp_event_info {
	unsigned type : 2;
	unsigned dp_hpd : 2;

	unsigned int reserved31_4 : 28;
} __packed;

enum dp_hpd_event {
	DP_HPD_EVENT_PLUG_OUT = 0,
	DP_HPD_EVENT_PLUG_IN,
	DP_HPD_EVENT_IRQ,
	DP_HPD_EVENT_MAX,
};

enum dp_orientation {
	DP_ORIENTATION_OBSERVE = 0,
	DP_ORIENTATION_RESERVE,
	DP_ORIENTATION_MAX,
};

enum dp_combo_mode {
	DP_COMBO_MODE_DP2,
	DP_COMBO_MODE_DP4,
};

/*
 * struct dynamic_event_info - dynamic event
 * @type:               For dynamic event, type must set to XR_USB_EVENT_TYPE_DYNAMIC(2u)
 * @enable:             0 or 1
 * @reserved31_3:       wait for extend
 */
struct dynamic_event_info {
	unsigned type : 2;
	unsigned enable : 1;

	unsigned int reserved31_3 : 29;
} __packed;

/*
 * struct adsp_event_info - usb adsp event
 * @type:               For adsp event, type must set to XR_USB_EVENT_TYPE_ADSP(3u)
 * @start:             0 or 1
 * @reserved31_3:       wait for extend
 */
struct adsp_event_info {
	unsigned type : 2;
	unsigned start : 1;
	unsigned exception : 1;

	unsigned int reserved31_3 : 29;
} __packed;

/**
 * struct xr_usbdp_event - event to usb_dp setting
 * @data:               Data specific to the event
 * @field:              Event structure for usb event or usbdp event
 * @dp_field:           Event structure for only dp event
 *
 * @description:        Event for a system to invoke usbdp_function
 *                      always use bit[0:1] to descript event type
 */
union xr_usbdp_event {
	unsigned int data;
	struct usb_event_info usb;
	struct dp_event_info dp;
	struct dynamic_event_info dynamic;
	struct adsp_event_info adsp;
};

struct xr_dp_ops {
	int (*manage_power)(bool on, enum dp_orientation orientation,
			    unsigned int usbdp_mode);
	int (*hpd_evt_notifier)(enum dp_hpd_event hpd_event);
};

struct xr_usb_adsp_ops {
	int (*start)(void *context, struct platform_device *xhci);
	int (*stop)(void *context);
	void (*state_update_notify)(void *context);
	void *context;
};

struct xring_usb;
/* find device node by funciton of_parse_phandle */
struct xring_usb *xr_usb_port_of_get(const struct device_node *node);
void xr_usb_port_of_put(struct xring_usb *xr_usb);
int xr_usb_queue_event(struct xring_usb *xr_usb, union xr_usbdp_event event);
int xr_usbdp_ops_register(struct xring_usb *xr_usb,
			  const struct xr_dp_ops *ops);
int xr_usbdp_ops_unregister(struct xring_usb *xr_usb);
int xr_usb_adsp_ops_register(struct xring_usb *xr_usb,
			     const struct xr_usb_adsp_ops *ops);
void xr_usb_adsp_ops_unregister(struct xring_usb *xr_usb);

#define xr_usb_queue_device_on(port, orig, _usbdp_mode, _dp_hpd)               \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event = { .data = 0 };                  \
		__event.usb.type = XR_USB_EVENT_TYPE_USBDP;                    \
		__event.usb.mode = USB_MODE_PERIPEHRAL;                        \
		__event.usb.typec_orig = !!orig;                               \
		__event.usb.usbdp_mode = _usbdp_mode & USBDP_MODE_MASK;        \
		__event.usb.dp_hpd = _dp_hpd;                                  \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#define xr_usb_queue_host_on(port, orig, _usbdp_mode, _dp_hpd)                 \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event;                                  \
		__event.usb.type = XR_USB_EVENT_TYPE_USBDP;                    \
		__event.usb.mode = USB_MODE_HOST;                              \
		__event.usb.typec_orig = !!orig;                               \
		__event.usb.usbdp_mode = _usbdp_mode & USBDP_MODE_MASK;        \
		__event.usb.dp_hpd = _dp_hpd;                                  \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#define xr_usb_queue_off(port)                                                 \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event;                                  \
		__event.usb.type = XR_USB_EVENT_TYPE_USBDP;                    \
		__event.usb.mode = USB_MODE_UNPLUG;                            \
		__event.usb.typec_orig = 0;                                    \
		__event.usb.usbdp_mode = NC_MODE;                              \
		__event.usb.dp_hpd = DP_HPD_EVENT_PLUG_OUT;                    \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#define xr_usb_queue_dp_hpd(port, _dp_hpd)                                     \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event;                                  \
		__event.dp.type = XR_USB_EVENT_TYPE_DP;                        \
		__event.dp.dp_hpd = _dp_hpd;                                   \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#define xr_usb_queue_adsp_on(port, on)                                         \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event = { .data = 0 };                  \
		__event.adsp.type = XR_USB_EVENT_TYPE_ADSP;                    \
		__event.adsp.start = on;                                       \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#define xr_usb_queue_adsp_exception(port)                                      \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event = { .data = 0 };                  \
		__event.adsp.type = XR_USB_EVENT_TYPE_ADSP;                    \
		__event.adsp.start = true;                                     \
		__event.adsp.exception = true;                                 \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#define xr_usb_queue_dynamic_event(port, en)                                   \
	({                                                                     \
		int __ret;                                                     \
		union xr_usbdp_event __event;                                  \
		__event.dynamic.type = XR_USB_EVENT_TYPE_DYNAMIC;              \
		__event.dynamic.enable = en;                                   \
		__ret = xr_usb_queue_event(port, __event);                     \
		__ret;                                                         \
	})

#endif
