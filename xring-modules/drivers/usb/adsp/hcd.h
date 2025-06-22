/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_ADSP_HCD_H
#define _XR_USB_ADSP_HCD_H

#include <linux/atomic.h>
#include <linux/kfifo.h>
#include <linux/platform_device.h>
#include <linux/spinlock_types.h>
#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/ch11.h>
#include <linux/usb/hcd.h>
#include <linux/workqueue.h>

#define USB_PORT_STATE_CHANGE_SHIFT 16

struct usb_hcd;
struct xring_usb_adsp;
struct xr_usb_adsp_shared_buf;

enum usb_adsp_hc_request_id {
	USB_ADSP_HC_REQ_SUBMIT_URB = 0,
	USB_ADSP_HC_REQ_DEQUEUE_URB,
	USB_ADSP_HC_REQ_MAX,
};

struct xr_usb_adsp_urb {
	struct list_head list;
	struct urb *urb;
	const struct xr_usb_adsp_shared_buf *shared_buf;
	int ep_type;
};

struct usb_adsp_hc_submit_urb_data {
	struct urb *urb;
};

struct usb_adsp_hc_deq_urb_data {
	int status;
	struct urb *urb;
};

#define USB_ADSP_REQ_LEN_MAX 20
struct usb_adsp_hc_request {
	u8 id;
	u8 reserved[3];
	union {
		u8 data[USB_ADSP_REQ_LEN_MAX];
		struct usb_adsp_hc_submit_urb_data urb_data;
		struct usb_adsp_hc_deq_urb_data deq_urb;
	};
} __aligned(sizeof(long));

struct xr_usb_adsp_hcd {
	const struct hc_driver *driver_origin;
	atomic_t use_count;
#define USB_ADSP_HCD_STATE_ORIGIN 0
#define USB_ADSP_HCD_STATE_SWITCH_ADSP 1
#define USB_ADSP_HCD_STATE_ADSP 2
#define USB_ADSP_HCD_STATE_SWITCH_ORIGIN 3
	atomic_t state;
	struct hc_driver driver_overrides;

	struct xring_usb_adsp *usb_adsp;
	struct usb_hcd *hcd; /* usb2 hcd */
	atomic_t port_state; /* port change and port status */
	struct list_head urb_list;
	struct list_head urb_deq_list;
	spinlock_t urb_lock;
	struct work_struct hc_work;
	spinlock_t hc_request_lock;
#define USB_ADSP_HC_REQ_FIFO_SIZE 16
	DECLARE_KFIFO(hc_request_fifo, struct usb_adsp_hc_request,
		      USB_ADSP_HC_REQ_FIFO_SIZE);
};

static inline bool xr_usb_adsp_hcd_valid(struct xr_usb_adsp_hcd *adsp_hcd)
{
	return (adsp_hcd != NULL) && (adsp_hcd->driver_origin != NULL);
}

static inline void xr_usb_adsp_hcd_set_state(struct xr_usb_adsp_hcd *adsp_hcd,
					     int state)
{
	atomic_set(&adsp_hcd->state, state);
}

static inline int xr_usb_adsp_hcd_get_state(struct xr_usb_adsp_hcd *adsp_hcd)
{
	return atomic_read(&adsp_hcd->state);
}

static inline int xr_usb_adsp_hcd_get_port_state(struct xr_usb_adsp_hcd *adsp_hcd)
{
	return atomic_read(&adsp_hcd->port_state);
}

static inline void xr_usb_adsp_hcd_enable_port(struct xr_usb_adsp_hcd *adsp_hcd)
{
	unsigned int init_state = USB_PORT_STAT_CONNECTION |
				  USB_PORT_STAT_ENABLE | USB_PORT_STAT_POWER;
	atomic_set(&adsp_hcd->port_state, init_state);
}

struct xr_usb_adsp_hcd *xr_usb_adsp_hcd_create(struct device *,
					       struct xring_usb_adsp *);
void xr_usb_adsp_hcd_destroy(struct device *dev, struct xr_usb_adsp_hcd *adsp_hcd);
void xr_usb_adsp_hcd_add(struct xr_usb_adsp_hcd *adsp_hcd, struct usb_hcd *hcd);
void xr_usb_adsp_hcd_remove(struct xr_usb_adsp_hcd *adsp_hcd, struct usb_hcd *hcd);
int xr_usb_adsp_hcd_wait_idle(struct xr_usb_adsp_hcd *adsp_hcd);
void xr_usb_adsp_hcd_flush_work(struct xr_usb_adsp_hcd *adsp_hcd);
void xr_usb_adsp_hcd_start(struct xr_usb_adsp_hcd *adsp_hcd);
void xr_usb_adsp_hcd_stop(struct xr_usb_adsp_hcd *adsp_hcd);
int xr_usb_adsp_hcd_first_hid_in(struct xr_usb_adsp_hcd *adsp_hcd, u8 ep_index,
				 u32 actual_len, int status);
int xr_usb_adsp_hcd_port_change(struct xr_usb_adsp_hcd *adsp_hcd, u32 portsc);
int xr_usb_adsp_hcd_urb_giveback(struct xr_usb_adsp_hcd *adsp_hcd, u32 base,
				 u32 len);
void xr_usb_adsp_hcd_died(struct xr_usb_adsp_hcd *adsp_hcd);

#endif /* _XR_USB_ADSP_HCD_H */
