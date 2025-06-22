/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_ADSP_CORE_H
#define _XR_USB_ADSP_CORE_H

#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/spinlock_types.h>
#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/wait.h>

#include <adsp/adsp_ssr_driver.h>
#include <soc/xring/xr_usbdp_event.h>

#undef pr_fmt
#undef dev_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#define dev_fmt(fmt) ": %s:%d: " fmt, __func__, __LINE__

#define XR_USB_TRACE_MSG_MAX 500

struct usb_hcd;
struct xhci_hcd;
struct xr_usb_adsp_hcd;
struct xring_usb;

struct xr_usb_adsp_shared_buf {
	u32 addr;
	u32 len;
	u32 index;
	u32 merge_num;
	u8 *virt_addr;
};

#define XR_USB_ADSP_SHARED_BUF_NUM 10
struct xr_usb_adsp_shared_buf_pool {
	u8 *virt_addr;
	spinlock_t lock;
	unsigned long usage_map;
	struct xr_usb_adsp_shared_buf bufs[XR_USB_ADSP_SHARED_BUF_NUM];
};

struct xring_usb_adsp {
	struct device *dev;
	struct notifier_block udev_nb;
	struct xr_usb_adsp_ops ops;
	struct xring_usb *xr_usb;
	struct platform_device *xhci;
	struct xr_usb_adsp_hcd *adsp_hcd;
	struct xr_usb_adsp_ipc *ipc;
	struct usb_device *audio_udev;
	struct mutex mutex;
	wait_queue_head_t state_wait;
	struct xr_usb_adsp_shared_buf_pool buf_pool;
	struct dentry *debugfs_root;

#define USB_AUDIO_DEV_QUIRK_SR_CHECK_HID_INPUT BIT(0)
#define USB_AUDIO_DEV_QUIRK_SR_DISABLE BIT(1)
#define USB_AUDIO_DEV_QUIRK_RESUME_LONG BIT(2)
	u32 quirks;
	/* status */
	unsigned pending_usb_start : 1;
	unsigned suspending : 1;
	unsigned suspended : 1;
	unsigned handling_adsp_exception : 1;
	unsigned udev_state : 1;

	struct recovery_ops adsp_recovery_ops;
	struct adsp_ssr_action *adsp_recovery_action;
};

/* switch.c */
int xr_usb_adsp_switch_on(struct xring_usb_adsp *usb_adsp, struct platform_device *xhci);
int xr_usb_adsp_switch_off(struct xring_usb_adsp *usb_adsp);
void xr_usb_adsp_disable_xhci_irq(struct xhci_hcd *xhci, bool disable);
/* core.c */
int xr_usb_adsp_stop(void *context);
const struct xr_usb_adsp_shared_buf *
xr_usb_adsp_get_shared_buf(struct xring_usb_adsp *usb_adsp, u32 len);
void xr_usb_adsp_put_shared_buf(struct xring_usb_adsp *usb_adsp,
				const struct xr_usb_adsp_shared_buf *buf);
void xr_usb_adsp_switch_notify(struct xring_usb_adsp *usb_adsp,
			       unsigned long action);
int xr_usb_adsp_handle_start(struct xring_usb_adsp *usb_adsp);
int xr_usb_adsp_first_hid_in(struct xring_usb_adsp *usb_adsp, u8 ep_index,
			     u32 actual_len, int status);
int xr_usb_adsp_port_change(struct xring_usb_adsp *usb_adsp, u32 portsc);
int xr_usb_adsp_urb_giveback(struct xring_usb_adsp *usb_adsp, u32 base,
			     u32 len);
void xr_usb_adsp_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
			   ...);
bool xr_usb_adsp_check_intf_has_eps(struct usb_interface *intf);
#endif /* _XR_USB_ADSP_CORE_H */
