// SPDX-License-Identifier: GPL-2.0
/*
 * core.c - Core Driver for XRing USB ADSP.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "core.h"
#include "debugfs.h"
#include "hcd.h"
#include "ipc.h"
#include "xhci-helper.h"
#include "xr-usb-adsp.h"

#include <common/fk-audio-memlayout.h>
#include <common/fk-audio-usb.h>
#include <core/usb.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/gfp.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/usb/ch9.h>

#define XR_USB_ADSP_DEFAULT_AUTOSUSPEND_DELAY 5000 /* ms */
#define XR_USB_ADSP_SHARED_BUF_TOTAL_SIZE     USB_MAX_SIZE
#define XR_USB_ADSP_SHARED_BUF_SIZE                                            \
	(XR_USB_ADSP_SHARED_BUF_TOTAL_SIZE / XR_USB_ADSP_SHARED_BUF_NUM)

#define XR_USB_NON_OFFLOAD "0"
#define XR_USB_OFFLOAD "1"
#define XR_USB_UNPLUG "2"

static DEFINE_MUTEX(g_usb_adsp_dev_mutex);
static struct usb_device_id g_quirk_device = {
	.idVendor = 0x00,
	.idProduct = 0x00,
};

void xr_usb_adsp_dbg_trace(void (*trace)(struct va_format *), const char *fmt,
			   ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;
	trace(&vaf);
	va_end(args);
}

static void xr_usb_adsp_input_quirk_device(struct xring_usb_adsp *usb_adsp)
{
	if (!usb_adsp->audio_udev) {
		pr_err("input quirk device fail. audio udev is null\n");
		return;
	}

	g_quirk_device.idVendor =
		le16_to_cpu(usb_adsp->audio_udev->descriptor.idVendor);
	g_quirk_device.idProduct =
		le16_to_cpu(usb_adsp->audio_udev->descriptor.idProduct);
	pr_info("input quirk device 0x%04x:0x%04x\n",
		g_quirk_device.idVendor, g_quirk_device.idProduct);
}

static void xr_usb_adsp_input_quirk_device_release(void)
{
	g_quirk_device.idVendor = 0x00;
	g_quirk_device.idProduct = 0x00;
}

static bool xr_usb_adsp_match_quirk_device(struct usb_device *udev)
{
	if (le16_to_cpu(udev->descriptor.idVendor) != g_quirk_device.idVendor)
		return false;

	if (le16_to_cpu(udev->descriptor.idProduct) != g_quirk_device.idProduct)
		return false;

	return true;
}

static int xr_usb_adsp_start(void *context, struct platform_device *xhci)
{
	int ret;
	struct xr_usb_adsp_hcd *adsp_hcd = NULL;
	struct xring_usb_adsp *usb_adsp = (struct xring_usb_adsp *)context;

	pr_info("enter\n");
	if (!usb_adsp || !xhci)
		return -EINVAL;

	mutex_lock(&usb_adsp->mutex);
	pm_runtime_get_sync(usb_adsp->dev);
	adsp_hcd = usb_adsp->adsp_hcd;
	if (!xr_usb_adsp_hcd_valid(adsp_hcd) || !usb_adsp->audio_udev) {
		ret = -ENODEV;
		goto out;
	}

	if (usb_adsp->audio_udev->state == USB_STATE_NOTATTACHED ||
	    !usb_adsp_xhci_port_connect(xhci)) {
		pr_err("device gone\n");
		ret = -ENODEV;
		goto out;
	}

	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ORIGIN) {
		ret = -EBUSY;
		goto out;
	}

	xr_usb_adsp_hcd_set_state(adsp_hcd, USB_ADSP_HCD_STATE_SWITCH_ADSP);
	ret = xr_usb_adsp_hcd_wait_idle(adsp_hcd);
	if (ret) {
		pr_err("failed to wait hcd idle - %d\n", ret);
		xr_usb_adsp_hcd_set_state(adsp_hcd, USB_ADSP_HCD_STATE_ORIGIN);
		goto out_hcd_flush;
	}

	ret = xr_usb_adsp_switch_on(usb_adsp, xhci);
	if (ret) {
		pr_err("failed to switch to adsp - %d\n", ret);
		xr_usb_adsp_hcd_set_state(adsp_hcd, USB_ADSP_HCD_STATE_ORIGIN);
		xr_usb_adsp_input_quirk_device(usb_adsp);
		goto out_hcd_flush;
	}

	xr_usb_adsp_hcd_start(adsp_hcd);
	xr_usb_adsp_hcd_set_state(adsp_hcd, USB_ADSP_HCD_STATE_ADSP);
	WARN_ON(!try_module_get(THIS_MODULE));

out_hcd_flush:
	/* There may urb enqueue during switch, flush hc_work */
	xr_usb_adsp_hcd_flush_work(adsp_hcd);
out:
	pm_runtime_mark_last_busy(usb_adsp->dev);
	pm_runtime_put_autosuspend(usb_adsp->dev);
	mutex_unlock(&usb_adsp->mutex);
	pr_info("exit\n");
	return ret;
}

int xr_usb_adsp_stop(void *context)
{
	int ret = 0;
	int port_status;
	struct xr_usb_adsp_hcd *adsp_hcd = NULL;
	struct xring_usb_adsp *usb_adsp = (struct xring_usb_adsp *)context;

	pr_info("enter\n");
	if (!usb_adsp)
		return -EINVAL;

	mutex_lock(&usb_adsp->mutex);
	pm_runtime_get_noresume(usb_adsp->dev);

	adsp_hcd = usb_adsp->adsp_hcd;
	if (!xr_usb_adsp_hcd_valid(adsp_hcd)) {
		pr_err("invalid adsp hcd\n");
		ret = -ENODEV;
		goto out;
	}

	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ADSP) {
		pr_info("not in adsp state\n");
		ret = 0;
		goto out;
	}

	xr_usb_adsp_hcd_set_state(adsp_hcd, USB_ADSP_HCD_STATE_SWITCH_ORIGIN);

	if (!usb_adsp->handling_adsp_exception) {
		ret = xr_usb_adsp_switch_off(usb_adsp);
	} else {
		pr_info("handling adsp exception\n");
		usb_adsp->handling_adsp_exception = false;
		xr_usb_adsp_hcd_died(adsp_hcd);
	}

	xr_usb_adsp_hcd_flush_work(adsp_hcd);
	xr_usb_adsp_hcd_stop(adsp_hcd);
	xr_usb_adsp_hcd_set_state(adsp_hcd, USB_ADSP_HCD_STATE_ORIGIN);

	if (ret) {
		pr_err("failed to switch to adsp off - %d\n", ret);
		xr_usb_adsp_hcd_died(adsp_hcd);
		ret = 0;
		goto out;
	}

	/* Notify port disconnect */
	port_status = xr_usb_adsp_hcd_get_port_state(adsp_hcd);
	if (!(port_status & USB_PORT_STAT_CONNECTION) &&
	    (port_status &
	     (USB_PORT_STAT_C_CONNECTION << USB_PORT_STATE_CHANGE_SHIFT)))
		usb_hcd_poll_rh_status(adsp_hcd->hcd);

	module_put(THIS_MODULE);

	wake_up(&usb_adsp->state_wait);
out:
	pm_runtime_put_sync_suspend(usb_adsp->dev);
	usb_adsp->handling_adsp_exception = false;
	mutex_unlock(&usb_adsp->mutex);

	pr_info("exit\n");
	return ret;
}

static void xr_usb_adsp_state_update_notify(void *context)
{
	struct xring_usb_adsp *usb_adsp = (struct xring_usb_adsp *)context;

	pr_info("enter\n");

	if (!usb_adsp) {
		pr_err("state update invalid context\n");
		return;
	}

	mutex_lock(&usb_adsp->mutex);
	xr_usb_adsp_input_quirk_device_release();
	mutex_unlock(&usb_adsp->mutex);

	pr_info("exit\n");
}

static int xr_usb_adsp_is_running(struct xring_usb_adsp *usb_adsp)
{
	int state = xr_usb_adsp_hcd_get_state(usb_adsp->adsp_hcd);

	if (!xr_usb_adsp_hcd_valid(usb_adsp->adsp_hcd)) {
		pr_err("invalid adsp hcd\n");
		return -ENODEV;
	}

	if (state == USB_ADSP_HCD_STATE_ORIGIN) {
		pr_err("invalid state %d\n", state);
		return -ENOENT;
	}

	if (!usb_adsp->audio_udev) {
		pr_err("no audio device\n");
		return -ENOENT;
	}

	return 0;
}

int xr_usb_adsp_urb_giveback(struct xring_usb_adsp *usb_adsp, u32 base, u32 len)
{
	int ret;

	mutex_lock(&usb_adsp->mutex);
	ret = xr_usb_adsp_is_running(usb_adsp);
	if (ret)
		goto out;

	ret = xr_usb_adsp_hcd_urb_giveback(usb_adsp->adsp_hcd, base, len);
out:
	mutex_unlock(&usb_adsp->mutex);
	return ret;
}

int xr_usb_adsp_handle_start(struct xring_usb_adsp *usb_adsp)
{
	int ret = 0;

	mutex_lock(&usb_adsp->mutex);
	if (!usb_adsp->audio_udev) {
		usb_adsp->pending_usb_start = true;
		pr_info("usb start pending\n");
		goto out;
	}

	ret = xr_usb_queue_adsp_on(usb_adsp->xr_usb, true);
	if (ret)
		pr_err("failed to queue adsp on\n");
out:
	mutex_unlock(&usb_adsp->mutex);
	return ret;
}

int xr_usb_adsp_first_hid_in(struct xring_usb_adsp *usb_adsp, u8 ep_index,
			     u32 actual_len, int status)
{
	int ret;

	mutex_lock(&usb_adsp->mutex);
	pm_runtime_get_sync(usb_adsp->dev);
	ret = xr_usb_adsp_is_running(usb_adsp);
	if (ret)
		goto out;

	ret = xr_usb_adsp_hcd_first_hid_in(usb_adsp->adsp_hcd, ep_index,
					   actual_len, status);
out:
	pm_runtime_mark_last_busy(usb_adsp->dev);
	pm_runtime_put_autosuspend(usb_adsp->dev);
	mutex_unlock(&usb_adsp->mutex);
	return ret;
}

int xr_usb_adsp_port_change(struct xring_usb_adsp *usb_adsp, u32 portsc)
{
	int ret;

	mutex_lock(&usb_adsp->mutex);
	pm_runtime_get_sync(usb_adsp->dev);
	ret = xr_usb_adsp_is_running(usb_adsp);
	if (ret)
		goto out;

	/* Port change return 1 for disconnect */
	ret = xr_usb_adsp_hcd_port_change(usb_adsp->adsp_hcd, portsc);
	if (ret > 0) {
		pr_info("usb port disconnect\n");
		ret = xr_usb_queue_adsp_on(usb_adsp->xr_usb, false);
	} else if (ret < 0) {
		pr_err("usb port status fault err - %d\n", ret);
		(void)xr_usb_queue_adsp_exception(usb_adsp->xr_usb);
	}
out:
	pm_runtime_mark_last_busy(usb_adsp->dev);
	pm_runtime_put_autosuspend(usb_adsp->dev);
	mutex_unlock(&usb_adsp->mutex);
	return ret;
}

struct xring_usb_adsp *of_xr_usb_adsp_get(const struct device_node *node)
{
	struct device_node *usb_adsp_node = NULL;
	struct platform_device *usb_adsp_dev = NULL;
	struct xring_usb_adsp *usb_adsp = NULL;

	if (!node)
		return ERR_PTR(-EINVAL);

	usb_adsp_node = of_parse_phandle(node, "usb-adsp", 0);
	if (!usb_adsp_node) {
		pr_err("falied to get usb_adsp node\n");
		return ERR_PTR(-ENODEV);
	}

	usb_adsp_dev = of_find_device_by_node(usb_adsp_node);
	if (!usb_adsp_dev) {
		pr_err("falied to get usb_adsp device\n");
		of_node_put(usb_adsp_node);
		return ERR_PTR(-ENODEV);
	}

	mutex_lock(&g_usb_adsp_dev_mutex);
	usb_adsp = platform_get_drvdata(usb_adsp_dev);
	if (!usb_adsp) {
		pr_err("falied to get usb_adsp\n");
		put_device(&usb_adsp_dev->dev);
	} else {
		WARN_ON(!try_module_get(THIS_MODULE));
	}
	mutex_unlock(&g_usb_adsp_dev_mutex);
	of_node_put(usb_adsp_node);

	return usb_adsp;
}
EXPORT_SYMBOL_GPL(of_xr_usb_adsp_get);

void of_xr_usb_adsp_put(struct xring_usb_adsp *usb_adsp)
{
	if (!usb_adsp)
		return;

	mutex_lock(&g_usb_adsp_dev_mutex);
	put_device(usb_adsp->dev);
	module_put(THIS_MODULE);
	mutex_unlock(&g_usb_adsp_dev_mutex);
}
EXPORT_SYMBOL_GPL(of_xr_usb_adsp_put);

enum xr_usb_adsp_usb_id_type {
	XR_USB_ADSP_USB_ID_HID = 0,
	XR_USB_ADSP_USB_ID_AUDIO_CTL,
	XR_USB_ADSP_USB_ID_AUDIO_DATA,
	XR_USB_ADSP_USB_ID_MAX,
};

static const struct usb_device_id xr_usb_adsp_usb_ids[] = {
	{ .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
	  .bInterfaceClass = USB_INTERFACE_CLASS_HID },
	{ .match_flags = (USB_DEVICE_ID_MATCH_INT_CLASS |
			  USB_DEVICE_ID_MATCH_INT_SUBCLASS),
	  .bInterfaceClass = USB_CLASS_AUDIO,
	  .bInterfaceSubClass = USB_SUBCLASS_AUDIOCONTROL },
	{ .match_flags = (USB_DEVICE_ID_MATCH_INT_CLASS |
			  USB_DEVICE_ID_MATCH_INT_SUBCLASS),
	  .bInterfaceClass = USB_CLASS_AUDIO,
	  .bInterfaceSubClass = USB_SUBCLASS_AUDIOSTREAMING },
	{} /* Terminating entry */
};

static bool xr_usb_adsp_check_valid_hid(struct usb_interface *intf)
{
	const int valid_hid_alt_num = 1;
	struct usb_host_interface *curr_alt = NULL;

	if (intf->num_altsetting != valid_hid_alt_num)
		return false;

	curr_alt = intf->cur_altsetting;
	if (!curr_alt)
		return false;

	if (curr_alt->desc.bNumEndpoints > USB_ADSP_HID_EP_MAX)
		return false;

	if (!curr_alt->endpoint)
		return false;

	return true;
}

static int xr_usb_adsp_hid_int_ep_count(struct usb_interface *intf)
{
	struct usb_host_interface *curr_alt = NULL;
	struct usb_host_endpoint *ep = NULL;
	u8 i;

	curr_alt = intf->cur_altsetting;
	if (!curr_alt)
		return -EINVAL;

	if (curr_alt->desc.bNumEndpoints > USB_ADSP_HID_EP_MAX) {
		pr_warn("adsp hid interface has epnum:%u\n",
			curr_alt->desc.bNumEndpoints);
		return -EINVAL;
	}

	for (i = 0; i < curr_alt->desc.bNumEndpoints; i++) {
		ep = &curr_alt->endpoint[i];
		if (!usb_endpoint_xfer_int(&ep->desc)) {
			pr_err("check hid interface has invalid ep\n");
			return -EINVAL;
		}
	}

	return curr_alt->desc.bNumEndpoints;
}

static bool xr_usb_adsp_check_valid_audio(struct usb_interface *intf)
{
	int i;
	u8 subclass;

	if (intf->num_altsetting == 0)
		return false;

	for (i = 0; i < intf->num_altsetting; i++) {
		if (intf->altsetting[i].desc.bInterfaceClass != USB_CLASS_AUDIO)
			return false;

		subclass = intf->altsetting[i].desc.bInterfaceSubClass;
		if (subclass != USB_SUBCLASS_AUDIOCONTROL &&
		    subclass != USB_SUBCLASS_AUDIOSTREAMING)
			return false;
	}
	return true;
}

bool xr_usb_adsp_check_intf_has_eps(struct usb_interface *intf)
{
	int i;
	struct usb_host_interface *intf_alt = NULL;

	if (!intf->altsetting) {
		pr_err("intf has no altsetting\n");
		return false;
	}

	for (i = 0; i < intf->num_altsetting; i++) {
		intf_alt = intf->altsetting + i;
		if (intf_alt->desc.bNumEndpoints != 0) {
			pr_debug("intf[%d] alt[%d] has eps\n", intf_alt->desc.bInterfaceNumber,
					intf_alt->desc.bAlternateSetting);
			return true;
		}
	}

	return false;
}

static const struct usb_device_id xr_usb_adsp_quirk_support_ids[] = {
	/* Shanling UA1s */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x20b1,
		.idProduct = 0x301f,
	},
	/* moma wireless mic */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x3547,
		.idProduct = 0x0001,
	},
	/* boya wireless mic */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x4c4a,
		.idProduct = 0x4155,
	},
	/* Sonata BHD */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x22e1,
		.idProduct = 0xe205,
	},
	{} /* Terminating entry */
};

static const struct usb_device_id xr_usb_adsp_quirk_non_offload[] = {
	/* Realtek */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x2f6e,
		.idProduct = 0x4ba0,
	},
	{} /* Terminating entry */
};

static bool xr_usb_adsp_check_quirk_dev(struct usb_device *udev,
					struct usb_interface *usb_intf)
{
	const struct usb_device_id *match_id = NULL;

	/* Match quirk by device info */
	match_id = usb_device_match_id(udev, xr_usb_adsp_quirk_support_ids);
	if (!match_id)
		return false;

	/* Check driver bound of quirk interface  */
	return usb_intf->condition == USB_INTERFACE_UNBOUND;
}

static bool xr_usb_adsp_check_valid_udev(struct usb_device *udev)
{
	int i, j, found_type;
	const int valid_hid_intf_num = 3;
	int hid_intf_count = 0;
	int hid_ep_count = 0;
	int audio_ctrl_intf_count = 0;
	int audio_data_intf_count = 0;
	int ret;

	/* is root hub */
	if (!udev->parent)
		return false;

	/* is behind hub */
	if (udev->parent->parent)
		return false;

	if (!udev->actconfig) {
		pr_err("udev has no active configuration\n");
		return false;
	}

	if (udev->speed != USB_SPEED_HIGH && udev->speed != USB_SPEED_FULL)
		return false;

	if (xr_usb_adsp_match_quirk_device(udev)) {
		pr_info("quirk audio device force non offload\n");
		return false;
	}

	if (usb_device_match_id(udev, xr_usb_adsp_quirk_non_offload)) {
		pr_info("force non offload audio device\n");
		return false;
	}

	for (i = 0; i < udev->actconfig->desc.bNumInterfaces; i++) {
		struct usb_interface *usb_intf = udev->actconfig->interface[i];

		if (!usb_intf) {
			pr_debug("udev has invalid interface\n");
			return false;
		}

		if (!usb_intf->cur_altsetting) {
			pr_debug("interface has no invalid altsetting\n");
			return false;
		}

		found_type = -1;
		for (j = 0; j < XR_USB_ADSP_USB_ID_MAX; j++) {
			if (usb_match_one_id(usb_intf,
					     &xr_usb_adsp_usb_ids[j])) {
				found_type = j;
				break;
			}
		}

		pr_debug("usb_intf check dev intf:%d found_type:%d\n", i, found_type);

		if (found_type == XR_USB_ADSP_USB_ID_HID) {
			if (xr_usb_adsp_check_valid_hid(usb_intf)) {
				ret = xr_usb_adsp_hid_int_ep_count(usb_intf);
				if (ret < 0) {
					pr_info("adsp hid intf:%d ep invalid\n", i);
					return false;
				}
				hid_ep_count += ret;
				hid_intf_count++;
			} else {
				pr_info("adsp hid check fail\n");
				return false;
			}
		} else if (found_type == XR_USB_ADSP_USB_ID_AUDIO_CTL) {
			if (xr_usb_adsp_check_valid_audio(usb_intf)) {
				audio_ctrl_intf_count++;
			} else {
				pr_info("adsp ctl check fail\n");
				return false;
			}
		} else if (found_type == XR_USB_ADSP_USB_ID_AUDIO_DATA) {
			if (xr_usb_adsp_check_valid_audio(usb_intf)) {
				audio_data_intf_count++;
			} else {
				pr_info("adsp data check fail\n");
				return false;
			}
		} else {
			if (!xr_usb_adsp_check_quirk_dev(udev, usb_intf) &&
				xr_usb_adsp_check_intf_has_eps(usb_intf)) {
				pr_debug("adsp quirk check fail\n");
				return false;
			}
		}
	}

	if (hid_intf_count > valid_hid_intf_num ||
	    hid_ep_count > USB_ADSP_HID_EP_MAX ||
	    audio_ctrl_intf_count > USB_ADSP_AUDIO_CTRL_NUM ||
	    audio_data_intf_count > USB_ADSP_MAX_AUDIO_DATA_NUM) {
		pr_err("udev intf count err. hid_intf:%d hid_ep:%d ctrl:%d data:%d\n",
			hid_intf_count, hid_ep_count, hid_intf_count,
			audio_data_intf_count);
		return false;
	}

	pr_info("found valid audio device\n");
	return true;
}

static void xr_usb_adsp_udev_add(struct xring_usb_adsp *usb_adsp,
				 struct usb_device *udev)
{
	mutex_lock(&usb_adsp->mutex);
	/* only udev connnect to roothub */
	if (udev->parent && !udev->parent->parent)
		usb_adsp->udev_state = true;

	if (!xr_usb_adsp_check_valid_udev(udev)) {
		mutex_unlock(&usb_adsp->mutex);
		return;
	}

	usb_adsp->audio_udev = udev;
	if (usb_adsp->pending_usb_start) {
		if (xr_usb_queue_adsp_on(usb_adsp->xr_usb, true))
			pr_err("failed to queue adsp on\n");
		usb_adsp->pending_usb_start = false;
	}
	mutex_unlock(&usb_adsp->mutex);
}

static void xr_usb_adsp_udev_remove(struct xring_usb_adsp *usb_adsp,
				    struct usb_device *udev)
{
	mutex_lock(&usb_adsp->mutex);
	/* only udev connnect to roothub */
	if (udev->parent && !udev->parent->parent)
		usb_adsp->udev_state = false;

	if (usb_adsp->audio_udev == udev) {
		usb_adsp->audio_udev = NULL;
		usb_adsp->quirks = 0;
		usb_adsp->pending_usb_start = false;
	}
	mutex_unlock(&usb_adsp->mutex);
}

static void xr_usb_adsp_ubus_add(struct xring_usb_adsp *usb_adsp,
				 struct usb_bus *ubus)
{
	struct usb_hcd *hcd = bus_to_hcd(ubus);

	mutex_lock(&usb_adsp->mutex);
	xr_usb_adsp_hcd_add(usb_adsp->adsp_hcd, hcd);
	mutex_unlock(&usb_adsp->mutex);
}

static void xr_usb_adsp_ubus_remove(struct xring_usb_adsp *usb_adsp,
				    struct usb_bus *ubus)
{
	struct usb_hcd *hcd = bus_to_hcd(ubus);

	xr_usb_adsp_stop(usb_adsp);
	mutex_lock(&usb_adsp->mutex);
	xr_usb_adsp_hcd_remove(usb_adsp->adsp_hcd, hcd);
	usb_adsp->xhci = NULL;
	mutex_unlock(&usb_adsp->mutex);
}

static int xr_usb_adsp_udev_notify(struct notifier_block *self,
				   unsigned long action, void *dev)
{
	struct xring_usb_adsp *usb_adsp =
		container_of(self, struct xring_usb_adsp, udev_nb);

	switch (action) {
	case USB_DEVICE_ADD:
		xr_usb_adsp_udev_add(usb_adsp, dev);
		break;
	case USB_DEVICE_REMOVE:
		xr_usb_adsp_udev_remove(usb_adsp, dev);
		break;
	case USB_BUS_ADD:
		xr_usb_adsp_ubus_add(usb_adsp, dev);
		break;
	case USB_BUS_REMOVE:
		xr_usb_adsp_ubus_remove(usb_adsp, dev);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static int xr_usb_adsp_shared_buf_init(struct xr_usb_adsp_shared_buf_pool *pool)
{
	unsigned int i;
	uint32_t phy_addr = 0;

	pool->virt_addr = (u8 *)fk_get_audio_static_mem(
		USB_AUD_MEM, &phy_addr, XR_USB_ADSP_SHARED_BUF_TOTAL_SIZE);
	if (!pool->virt_addr) {
		pr_err("failed to ioremap shared buffer\n");
		return -ENOMEM;
	}
	spin_lock_init(&pool->lock);
	pool->usage_map = 0;
	for (i = 0; i < XR_USB_ADSP_SHARED_BUF_NUM; ++i) {
		pool->bufs[i].addr = phy_addr + i * XR_USB_ADSP_SHARED_BUF_SIZE;
		pool->bufs[i].len = XR_USB_ADSP_SHARED_BUF_SIZE;
		pool->bufs[i].virt_addr =
			pool->virt_addr + i * XR_USB_ADSP_SHARED_BUF_SIZE;
		pool->bufs[i].index = i;
		pool->bufs[i].merge_num = 1;
	}

	return 0;
}

const struct xr_usb_adsp_shared_buf *
xr_usb_adsp_get_shared_buf(struct xring_usb_adsp *usb_adsp, u32 len)
{
	unsigned long free_bit;
	u32 merge_num;
	u32 merge_mask;
	struct xr_usb_adsp_shared_buf *buf = NULL;
	bool found = false;

	if (!usb_adsp || len > XR_USB_ADSP_SHARED_BUF_TOTAL_SIZE)
		return NULL;
	if (!usb_adsp->buf_pool.virt_addr)
		return NULL;

	merge_num = (len + XR_USB_ADSP_SHARED_BUF_SIZE - 1) /
		    XR_USB_ADSP_SHARED_BUF_SIZE;
	if (merge_num > XR_USB_ADSP_SHARED_BUF_NUM || merge_num == 0)
		return NULL;

	merge_mask = BIT(merge_num) - 1;

	spin_lock(&usb_adsp->buf_pool.lock);
	do {
		free_bit = find_first_zero_bit(&usb_adsp->buf_pool.usage_map,
					       XR_USB_ADSP_SHARED_BUF_NUM);
		if (free_bit + merge_num <= XR_USB_ADSP_SHARED_BUF_NUM &&
		    !((usb_adsp->buf_pool.usage_map >> free_bit) & merge_mask))
			found = true;
	} while (!found && free_bit < XR_USB_ADSP_SHARED_BUF_NUM);

	if (found) {
		usb_adsp->buf_pool.usage_map |= merge_mask << free_bit;
		buf = &usb_adsp->buf_pool.bufs[free_bit];
		buf->merge_num = merge_num;
		buf = buf->virt_addr ? buf : NULL;
	}
	spin_unlock(&usb_adsp->buf_pool.lock);

	return buf;
}

void xr_usb_adsp_put_shared_buf(struct xring_usb_adsp *usb_adsp,
				const struct xr_usb_adsp_shared_buf *buf)
{
	u32 merge_mask;

	if (!usb_adsp || !buf)
		return;
	if (!usb_adsp->buf_pool.virt_addr)
		return;
	if (buf->index >= XR_USB_ADSP_SHARED_BUF_NUM)
		return;

	spin_lock(&usb_adsp->buf_pool.lock);
	merge_mask = BIT(buf->merge_num) - 1;
	usb_adsp->buf_pool.usage_map &= ~(merge_mask << buf->index);
	spin_unlock(&usb_adsp->buf_pool.lock);
}

static void
xr_usb_adsp_shared_buf_exit(struct xr_usb_adsp_shared_buf_pool *pool)
{
	if (pool->virt_addr) {
		iounmap(pool->virt_addr);
		pool->virt_addr = NULL;
	}
}

static void xr_usb_adsp_exception_pre_handler(void *priv)
{
	struct xring_usb_adsp *usb_adsp = (struct xring_usb_adsp *)priv;

	if (!usb_adsp) {
		pr_err("priv is null\n");
		return;
	}

	mutex_lock(&usb_adsp->mutex);
	if (xr_usb_adsp_hcd_get_state(usb_adsp->adsp_hcd) !=
	    USB_ADSP_HCD_STATE_ADSP) {
		pr_info("not in usb adsp\n");
		mutex_unlock(&usb_adsp->mutex);
		return;
	}

	pr_info("handle adsp exception\n");

	if (usb_adsp->adsp_hcd->hcd) {
		struct xhci_hcd *xhci = hcd_to_xhci(usb_adsp->adsp_hcd->hcd);

		xr_usb_adsp_disable_xhci_irq(xhci, true);
		xhci_quiesce(xhci);
	}
	usb_adsp->handling_adsp_exception = true;
	mutex_unlock(&usb_adsp->mutex);
}

static void xr_usb_adsp_exception_post_handler(void *priv)
{
	struct xring_usb_adsp *usb_adsp = (struct xring_usb_adsp *)priv;

	if (!usb_adsp) {
		pr_err("priv is null\n");
		return;
	}

	pr_info("adsp exception post handler\n");

	mutex_lock(&usb_adsp->mutex);
	xr_usb_queue_adsp_exception(usb_adsp->xr_usb);
	mutex_unlock(&usb_adsp->mutex);
}

bool udev_supp_adsp(void *priv)
{
	struct usb_device *udev = (struct usb_device *)priv;

	if (!udev) {
		pr_err("priv is null\n");
		return false;
	}

	return xr_usb_adsp_check_valid_udev(udev);
}

static void xr_usb_adsp_register_audio_func(struct xring_usb_adsp *usb_adsp)
{
	struct recovery_ops *ops = NULL;

	ops = &usb_adsp->adsp_recovery_ops;
	ops->pre_recovery = xr_usb_adsp_exception_pre_handler;
	ops->post_recovery = xr_usb_adsp_exception_post_handler;
	usb_adsp->adsp_recovery_action = fk_adsp_ssr_register(ops, usb_adsp);
	if (usb_adsp->adsp_recovery_action == NULL)
		pr_err("failed to register adsp ssr action\n");

	usb2aud_cb_register(udev_supp_adsp, usb_adsp);
}

static void xr_usb_adsp_unregister_audio_func(struct xring_usb_adsp *usb_adsp)
{
	if (usb_adsp->adsp_recovery_action != NULL) {
		fk_adsp_ssr_unregister(usb_adsp->adsp_recovery_action);
		usb_adsp->adsp_recovery_action = NULL;
	}

	usb2aud_cb_unregister();
}

static ssize_t supp_adsp_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct xring_usb_adsp *usb_adsp = dev_get_drvdata(dev);
	const char *supp_adsp = NULL;

	mutex_lock(&usb_adsp->mutex);
	if (usb_adsp->audio_udev)
		supp_adsp = XR_USB_OFFLOAD;
	else if (usb_adsp->udev_state)
		supp_adsp = XR_USB_NON_OFFLOAD;
	else
		supp_adsp = XR_USB_UNPLUG;

	mutex_unlock(&usb_adsp->mutex);

	/* "0":non_offload "1":offload "2":unplug */
	return snprintf(buf, PAGE_SIZE, "%s\n", supp_adsp);
}
static DEVICE_ATTR_RO(supp_adsp);

static void xr_usb_adsp_sysfs_init(struct device *dev)
{
	int ret;

	ret = device_create_file(dev, &dev_attr_supp_adsp);
	if (ret)
		dev_err(dev, "sysfs init failed\n");
}

static void xr_usb_adsp_sysfs_exit(struct device *dev)
{
	device_remove_file(dev, &dev_attr_supp_adsp);
}

static int xr_usb_adsp_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct xring_usb_adsp *usb_adsp = NULL;

	usb_adsp = devm_kzalloc(dev, sizeof(*usb_adsp), GFP_KERNEL);
	if (!usb_adsp)
		return -ENOMEM;

	usb_adsp->dev = dev;
	usb_adsp->ops.start = xr_usb_adsp_start;
	usb_adsp->ops.stop = xr_usb_adsp_stop;
	usb_adsp->ops.state_update_notify = xr_usb_adsp_state_update_notify;
	usb_adsp->ops.context = usb_adsp;

	pm_runtime_get_noresume(dev);
	pm_runtime_set_active(dev);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_set_autosuspend_delay(dev,
					 XR_USB_ADSP_DEFAULT_AUTOSUSPEND_DELAY);
	pm_runtime_enable(dev);

	pm_runtime_forbid(dev);

	usb_adsp->xr_usb = xr_usb_port_of_get(dev->of_node);
	if (IS_ERR_OR_NULL(usb_adsp->xr_usb)) {
		dev_err(dev, "failed to get xr_usb\n");
		usb_adsp->xr_usb = NULL;
		ret = -EPROBE_DEFER;
		goto err_pm_put;
	}

	ret = xr_usb_adsp_ops_register(usb_adsp->xr_usb, &usb_adsp->ops);
	if (ret) {
		dev_err(dev, "failed to register ops\n");
		goto err_xr_usb_put;
	}

	usb_adsp->ipc = xr_usb_adsp_ipc_create(dev, usb_adsp);
	if (IS_ERR(usb_adsp->ipc)) {
		dev_err(dev, "failed to init ipc\n");
		ret = PTR_ERR(usb_adsp->ipc);
		goto err_remove_ops;
	}

	usb_adsp->adsp_hcd = xr_usb_adsp_hcd_create(dev, usb_adsp);
	if (IS_ERR(usb_adsp->adsp_hcd)) {
		dev_err(dev, "failed to init adsp_hcd\n");
		ret = PTR_ERR(usb_adsp->adsp_hcd);
		goto err_put_ipc;
	}

	ret = xr_usb_adsp_shared_buf_init(&usb_adsp->buf_pool);
	if (ret) {
		dev_err(dev, "failed to init shared buffer pool\n");
		goto err_put_hcd;
	}

	ret = xr_usb_adsp_debugfs_init(usb_adsp);
	if (ret) {
		dev_err(dev, "debugfs init failed\n");
		goto err_put_hcd;
	}

	xr_usb_adsp_register_audio_func(usb_adsp);

	mutex_init(&usb_adsp->mutex);
	init_waitqueue_head(&usb_adsp->state_wait);
	usb_adsp->udev_nb.notifier_call = xr_usb_adsp_udev_notify;
	usb_register_notify(&usb_adsp->udev_nb);

	mutex_lock(&g_usb_adsp_dev_mutex);
	platform_set_drvdata(pdev, usb_adsp);
	mutex_unlock(&g_usb_adsp_dev_mutex);

	xr_usb_adsp_sysfs_init(dev);

	pm_runtime_allow(dev);
	pm_runtime_put_sync_suspend(dev);
	dev_info(dev, "probe finished\n");

	return 0;

err_put_hcd:
	xr_usb_adsp_hcd_destroy(dev, usb_adsp->adsp_hcd);
	usb_adsp->adsp_hcd = NULL;
err_put_ipc:
	xr_usb_adsp_ipc_destroy(dev, usb_adsp->ipc);
	usb_adsp->ipc = NULL;
err_remove_ops:
	xr_usb_adsp_ops_unregister(usb_adsp->xr_usb);
err_xr_usb_put:
	xr_usb_port_of_put(usb_adsp->xr_usb);
	usb_adsp->xr_usb = NULL;
err_pm_put:
	pm_runtime_allow(dev);
	pm_runtime_disable(dev);
	pm_runtime_dont_use_autosuspend(dev);
	pm_runtime_set_suspended(dev);
	pm_runtime_put_noidle(dev);

	devm_kfree(dev, usb_adsp);
	usb_adsp = NULL;
	return ret;
}

static int xr_usb_adsp_remove(struct platform_device *pdev)
{
	struct xring_usb_adsp *usb_adsp = platform_get_drvdata(pdev);

	xr_usb_adsp_sysfs_exit(&pdev->dev);
	mutex_lock(&g_usb_adsp_dev_mutex);
	platform_set_drvdata(pdev, NULL);
	mutex_unlock(&g_usb_adsp_dev_mutex);

	pm_runtime_get_sync(&pdev->dev);

	xr_usb_adsp_unregister_audio_func(usb_adsp);
	xr_usb_adsp_ops_unregister(usb_adsp->xr_usb);
	xr_usb_port_of_put(usb_adsp->xr_usb);
	usb_adsp->xr_usb = NULL;

	xr_usb_adsp_debugfs_exit(usb_adsp);
	usb_unregister_notify(&usb_adsp->udev_nb);
	xr_usb_adsp_hcd_destroy(&pdev->dev, usb_adsp->adsp_hcd);
	usb_adsp->adsp_hcd = NULL;
	xr_usb_adsp_ipc_destroy(&pdev->dev, usb_adsp->ipc);
	usb_adsp->ipc = NULL;
	xr_usb_adsp_shared_buf_exit(&usb_adsp->buf_pool);

	pm_runtime_allow(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_put_noidle(&pdev->dev);

	pm_runtime_set_suspended(&pdev->dev);

	devm_kfree(&pdev->dev, usb_adsp);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int xr_usb_adsp_suspend(struct device *dev)
{
	struct xring_usb_adsp *usb_adsp = dev_get_drvdata(dev);
	struct xr_usb_adsp_hcd *adsp_hcd = NULL;
	int ret = 0;

	dev_info(dev, "enter");
	mutex_lock(&usb_adsp->mutex);
	adsp_hcd = usb_adsp->adsp_hcd;
	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ORIGIN &&
	    xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ADSP) {
		pr_err("usb adsp busy\n");
		ret = -EBUSY;
	}
	mutex_unlock(&usb_adsp->mutex);

	dev_info(dev, "exit");

	return ret;
}

static int xr_usb_adsp_resume(struct device *dev)
{
	dev_info(dev, "enter");

	return 0;
}

static int xr_usb_adsp_suspend_late(struct device *dev)
{
	struct xring_usb_adsp *usb_adsp = dev_get_drvdata(dev);
	struct xr_usb_adsp_hcd *adsp_hcd = NULL;
	int ret = 0;

	dev_info(dev, "enter");
	mutex_lock(&usb_adsp->mutex);
	adsp_hcd = usb_adsp->adsp_hcd;
	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ORIGIN &&
	    xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ADSP) {
		pr_err("usb adsp busy\n");
		ret = -EBUSY;
	}
	usb_adsp->suspending = false;
	usb_adsp->suspended = true;
	mutex_unlock(&usb_adsp->mutex);

	xr_usb_adsp_ipc_suspend(usb_adsp->ipc);

	dev_info(dev, "exit");

	return ret;
}

static int xr_usb_adsp_resume_early(struct device *dev)
{
	dev_info(dev, "enter");

	return 0;
}

static int xr_usb_adsp_pm_prepare(struct device *dev)
{
	struct xring_usb_adsp *usb_adsp = dev_get_drvdata(dev);

	dev_info(dev, "enter");
	mutex_lock(&usb_adsp->mutex);
	usb_adsp->suspending = true;
	mutex_unlock(&usb_adsp->mutex);

	flush_work(&usb_adsp->adsp_hcd->hc_work);

	dev_info(dev, "exit");

	return 0;
}

static void xr_usb_adsp_pm_complete(struct device *dev)
{
	struct xring_usb_adsp *usb_adsp = dev_get_drvdata(dev);

	dev_info(dev, "enter");
	mutex_lock(&usb_adsp->mutex);
	usb_adsp->suspending = false;
	usb_adsp->suspended = false;
	if (xr_usb_adsp_hcd_get_state(usb_adsp->adsp_hcd) ==
	    USB_ADSP_HCD_STATE_ADSP) {
		pm_runtime_get_sync(dev);
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);
	}
	mutex_unlock(&usb_adsp->mutex);
	xr_usb_adsp_ipc_resume(usb_adsp->ipc);
	dev_info(dev, "exit");
}
#endif

#ifdef CONFIG_PM
static int xr_usb_adsp_runtime_suspend(struct device *dev)
{
	dev_info(dev, "enter");
	pm_relax(dev);
	device_init_wakeup(dev, false);

	return 0;
}

static int xr_usb_adsp_runtime_resume(struct device *dev)
{
	dev_info(dev, "enter");
	device_init_wakeup(dev, true);
	pm_stay_awake(dev);
	pm_runtime_mark_last_busy(dev);

	return 0;
}

static int xr_usb_adsp_runtime_idle(struct device *dev)
{
	dev_info(dev, "enter");
	pm_runtime_mark_last_busy(dev);
	pm_runtime_autosuspend(dev);

	return 0;
}
#endif

static const struct dev_pm_ops xr_usb_adsp_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(xr_usb_adsp_suspend, xr_usb_adsp_resume)
	LATE_SYSTEM_SLEEP_PM_OPS(xr_usb_adsp_suspend_late, xr_usb_adsp_resume_early)
	.prepare = pm_sleep_ptr(xr_usb_adsp_pm_prepare),
	.complete = pm_sleep_ptr(xr_usb_adsp_pm_complete),
	SET_RUNTIME_PM_OPS(xr_usb_adsp_runtime_suspend,
			   xr_usb_adsp_runtime_resume, xr_usb_adsp_runtime_idle)
};

static const struct of_device_id xr_usb_adsp_of_match[] = {
	{ .compatible = "xring,usb-adsp" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_usb_adsp_of_match);

static struct platform_driver xr_usb_adsp_driver = {
	.probe	= xr_usb_adsp_probe,
	.remove	= xr_usb_adsp_remove,
	.driver	= {
		.name = "xring_usb_adsp",
		.of_match_table = xr_usb_adsp_of_match,
		.pm = &xr_usb_adsp_dev_pm_ops,
	},
};
module_platform_driver(xr_usb_adsp_driver);

MODULE_SOFTDEP("pre: xring_audio");
MODULE_SOFTDEP("pre: xring_usb");
MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("USB ADSP Driver For XRing USB Audio Offload");
