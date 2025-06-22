// SPDX-License-Identifier: GPL-2.0
/*
 * hcd.c - Proxy HCD Driver of USB Audio Offload.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "core.h"
#include "hcd.h"
#include "ipc.h"
#include "trace.h"
#include "xhci-helper.h"
#include "xr-usb-adsp.h"

#include <asm/unaligned.h>
#include <host/xhci.h>
#include <linux/atomic/atomic-instrumented.h>
#include <linux/compiler.h>
#include <linux/dma-map-ops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/irqreturn.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/list.h>
#include <linux/minmax.h>
#include <linux/mutex.h>
#include <linux/pm_runtime.h>
#include <linux/usb/ch9.h>
#include <linux/usb/ch11.h>
#include <linux/usb/hcd.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <soc/xring/securelib/securec.h>

#define USB_ADSP_WAIT_HCD_IDLE_TIMEOUT 20 /* 20 * 20ms = 400ms */
#define USB_ADSP_WAIT_STATE_TIMEOPUT   5000

static inline struct xr_usb_adsp_hcd *hcd_to_adsp_hcd(struct usb_hcd *hcd)
{
	return container_of(hcd->driver, struct xr_usb_adsp_hcd,
			    driver_overrides);
}

static inline bool use_origin_hc_driver(struct xr_usb_adsp_hcd *adsp_hcd)
{
	return (xr_usb_adsp_hcd_get_state(adsp_hcd) ==
		USB_ADSP_HCD_STATE_ORIGIN);
}

static int adsp_hcd_queue_hc_request(struct xr_usb_adsp_hcd *adsp_hcd,
				     struct usb_adsp_hc_request *req)
{
	if (!kfifo_in_spinlocked(&adsp_hcd->hc_request_fifo, req, 1,
				 &adsp_hcd->hc_request_lock)) {
		pr_err("failed to push hc request fifo\n");
		return -EAGAIN;
	}

	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ADSP)
		return 0;

	if (!queue_work(system_wq, &adsp_hcd->hc_work))
		pr_warn("work already on a queue\n");

	return 0;
}

static irqreturn_t adsp_xhci_irq(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);
	irqreturn_t ret = IRQ_HANDLED;

	atomic_inc(&adsp_hcd->use_count);
	if (likely(use_origin_hc_driver(adsp_hcd)))
		ret = adsp_hcd->driver_origin->irq(hcd);
	atomic_dec(&adsp_hcd->use_count);

	return IRQ_HANDLED;
}

static int adsp_xhci_reset(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->reset(hcd);

	return -ENOENT;
}

static int adsp_xhci_start(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->start(hcd);

	return -ENOENT;
}

static void adsp_xhci_stop(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->stop(hcd);
}

static void adsp_xhci_shutdown(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->shutdown(hcd);
}

static int adsp_xhci_map_urb_for_dma(struct usb_hcd *hcd, struct urb *urb,
				     gfp_t mem_flags)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);
	int state = xr_usb_adsp_hcd_get_state(adsp_hcd);
	int ret = 0;

	if (state == USB_ADSP_HCD_STATE_ORIGIN)
		ret = adsp_hcd->driver_origin->map_urb_for_dma(hcd, urb,
							       mem_flags);
	else if (state == USB_ADSP_HCD_STATE_SWITCH_ORIGIN)
		ret = -ENOENT;

	return ret;
}

static void adsp_xhci_unmap_urb_for_dma(struct usb_hcd *hcd, struct urb *urb)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (use_origin_hc_driver(adsp_hcd))
		adsp_hcd->driver_origin->unmap_urb_for_dma(hcd, urb);
}

/**
 * Return 1 for not local control, 0 for local control,
 * negative errno for error
 */
static int usb_adsp_handle_req_feature(struct usb_hcd *hcd, struct urb *urb,
				       struct usb_ctrlrequest *setup)
{
	if (setup->wValue != USB_DEVICE_REMOTE_WAKEUP)
		return 1;

	usb_hcd_unlink_urb_from_ep(hcd, urb);
	usb_hcd_giveback_urb(hcd, urb, 0);

	return 0;
}

static int usb_adsp_handle_get_status(struct usb_hcd *hcd, struct urb *urb,
				      struct usb_ctrlrequest *setup)
{
	if (setup->wValue != USB_STATUS_TYPE_STANDARD)
		return 1;

	if (!(setup->bRequestType & USB_DIR_IN) ||
	    setup->wLength != sizeof(u16))
		return -EINVAL;

	put_unaligned_le16(0, urb->transfer_buffer);
	urb->actual_length = sizeof(u16);
	usb_hcd_unlink_urb_from_ep(hcd, urb);
	usb_hcd_giveback_urb(hcd, urb, 0);

	return 0;
}

static int usb_adsp_handle_intf_req(struct xr_usb_adsp_hcd *adsp_hcd,
				    struct usb_hcd *hcd, struct urb *urb,
				    struct usb_ctrlrequest *setup)
{
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;
	u32 len;

	if (!usb_adsp->suspended && !usb_adsp->suspending)
		return 1;

	len = min_t(u32, urb->transfer_buffer_length, setup->wLength);
	if (setup->bRequestType & USB_DIR_IN)
		memset(urb->transfer_buffer, 0, len);

	urb->actual_length = len;
	usb_hcd_unlink_urb_from_ep(hcd, urb);
	usb_hcd_giveback_urb(hcd, urb, 0);

	return 0;
}

static int usb_adsp_handle_local_control(struct xr_usb_adsp_hcd *adsp_hcd,
					 struct usb_hcd *hcd, struct urb *urb)
{
	int ret = 1;
	struct usb_ctrlrequest *setup =
		(struct usb_ctrlrequest *)urb->setup_packet;

	if (!usb_endpoint_xfer_control(&urb->ep->desc))
		return 1;

	pr_info("control type%02x req%02x v%04x i%04x l%u\n",
		setup->bRequestType, setup->bRequest, setup->wValue,
		setup->wIndex, setup->wLength);

	switch (setup->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		switch (setup->bRequest) {
		case USB_REQ_SET_FEATURE:
		case USB_REQ_CLEAR_FEATURE:
			ret = usb_adsp_handle_req_feature(hcd, urb, setup);
			break;
		case USB_REQ_GET_STATUS:
			ret = usb_adsp_handle_get_status(hcd, urb, setup);
			break;
		default:
			break;
		}
		break;
	case USB_RECIP_INTERFACE:
		ret = usb_adsp_handle_intf_req(adsp_hcd, hcd, urb, setup);
		break;
	default:
		break;
	}

	return ret;
}

static int adsp_hcd_defer_urb_enqueue(struct xr_usb_adsp_hcd *adsp_hcd,
				      struct usb_hcd *hcd, struct urb *urb,
				      gfp_t mem_flags)
{
	int ret;
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;
	struct usb_adsp_hc_request req = { 0 };

	xr_usb_adsp_dbg_trace(trace_usb_adsp_hc_driver, "%s +", __func__);
	if (!urb)
		return -EINVAL;
	if ((urb->dev != usb_adsp->audio_udev) || !urb->ep)
		return -EINVAL;

	if (hcd != adsp_hcd->hcd) {
		pr_warn("submit USB3 urb to adsp\n");
		return -ESHUTDOWN;
	}

	if (usb_endpoint_xfer_isoc(&urb->ep->desc) ||
	    usb_endpoint_xfer_bulk(&urb->ep->desc)) {
		pr_warn("do not submit isoc or bulk to adsp\n");
		return -ESHUTDOWN;
	} else if (usb_endpoint_is_int_out(&urb->ep->desc)) {
		pr_warn("do not submit int out to adsp\n");
		return -ESHUTDOWN;
	} else if (usb_endpoint_xfer_control(&urb->ep->desc)) {
		if (!urb->setup_packet)
			return -EINVAL;
	}

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (ret) {
		pr_err("failed to link urb to ep\n");
		return ret;
	}

	req.id = USB_ADSP_HC_REQ_SUBMIT_URB;
	req.urb_data.urb = urb;

	return adsp_hcd_queue_hc_request(adsp_hcd, &req);
}

static int adsp_xhci_urb_enqueue(struct usb_hcd *hcd, struct urb *urb,
				 gfp_t mem_flags)
{
	int ret;
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	atomic_inc(&adsp_hcd->use_count);
	if (likely(use_origin_hc_driver(adsp_hcd)))
		ret = adsp_hcd->driver_origin->urb_enqueue(hcd, urb, mem_flags);
	else
		ret = adsp_hcd_defer_urb_enqueue(adsp_hcd, hcd, urb, mem_flags);
	atomic_dec(&adsp_hcd->use_count);

	return ret;
}

static inline void adsp_hcd_wait_state_origin(struct xr_usb_adsp_hcd *adsp_hcd)
{
	long wait_ret;

	wait_ret = wait_event_timeout(
		adsp_hcd->usb_adsp->state_wait, use_origin_hc_driver(adsp_hcd),
		msecs_to_jiffies(USB_ADSP_WAIT_STATE_TIMEOPUT));
	WARN(wait_ret == 0, "wait state timeout");
}

static int adsp_hcd_defer_urb_dequeue(struct xr_usb_adsp_hcd *adsp_hcd,
				      struct xr_usb_adsp_urb *urb_deq,
				      struct usb_hcd *hcd, struct urb *urb,
				      int status)
{
	unsigned long flags;
	int ret;
	struct usb_adsp_hc_request req = { 0 };
	struct xhci_hcd *xhci = NULL;

	xhci = hcd_to_xhci(hcd);
	spin_lock_irqsave(&xhci->lock, flags);
	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	spin_unlock_irqrestore(&xhci->lock, flags);
	if (ret) {
		pr_err("urb check unlink failed\n");
		if (!urb_deq)
			return ret;

		pr_warn("urb on list do dequeue\n");
	}

	pr_info("queue urb dequeue req\n");
	req.id = USB_ADSP_HC_REQ_DEQUEUE_URB;
	req.deq_urb.urb = urb;
	req.deq_urb.status = status;

	return adsp_hcd_queue_hc_request(adsp_hcd, &req);
}

static inline struct xr_usb_adsp_urb *
usb_adsp_move_deq_urb(struct xr_usb_adsp_hcd *adsp_hcd, struct urb *urb)
{
	struct xr_usb_adsp_urb *urb_deq = NULL;
	struct xr_usb_adsp_urb *urb_tmp = NULL;
	struct xr_usb_adsp_urb *urb_next = NULL;

	spin_lock(&adsp_hcd->urb_lock);
	if (unlikely(!list_empty(&adsp_hcd->urb_list))) {
		list_for_each_entry_safe(urb_tmp, urb_next,
					 &adsp_hcd->urb_list, list) {
			if (urb_tmp->urb == urb) {
				list_move_tail(&urb_tmp->list,
					       &adsp_hcd->urb_deq_list);
				urb_deq = urb_tmp;
				break;
			}
		}
	}
	spin_unlock(&adsp_hcd->urb_lock);

	return urb_deq;
}

static int adsp_xhci_urb_dequeue(struct usb_hcd *hcd, struct urb *urb,
				 int status)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);
	struct xr_usb_adsp_urb *urb_deq = NULL;
	int ret = -ENOENT;

	urb_deq = usb_adsp_move_deq_urb(adsp_hcd, urb);

	atomic_inc(&adsp_hcd->use_count);
	if (likely(use_origin_hc_driver(adsp_hcd) && !urb_deq))
		ret = adsp_hcd->driver_origin->urb_dequeue(hcd, urb, status);
	else
		ret = adsp_hcd_defer_urb_dequeue(adsp_hcd, urb_deq, hcd, urb,
						 status);
	atomic_dec(&adsp_hcd->use_count);

	return ret;
}

static int adsp_xhci_alloc_dev(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->alloc_dev(hcd, udev);

	return -ENOENT;
}

static void adsp_xhci_free_dev(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->free_dev(hcd, udev);
}

static int adsp_xhci_alloc_streams(struct usb_hcd *hcd, struct usb_device *udev,
				   struct usb_host_endpoint **eps,
				   unsigned int num_eps,
				   unsigned int num_streams, gfp_t mem_flags)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->alloc_streams(
			hcd, udev, eps, num_eps, num_streams, mem_flags);

	return -ENOENT;
}

static int adsp_xhci_free_streams(struct usb_hcd *hcd, struct usb_device *udev,
				  struct usb_host_endpoint **eps,
				  unsigned int num_eps, gfp_t mem_flags)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->free_streams(
			hcd, udev, eps, num_eps, mem_flags);

	return -ENOENT;
}

int adsp_xhci_add_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
			   struct usb_host_endpoint *ep)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->add_endpoint(hcd, udev, ep);

	return -ENOENT;
}

int adsp_xhci_drop_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
			    struct usb_host_endpoint *ep)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);
	int ret;

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->drop_endpoint(hcd, udev, ep);

	ret = xr_usb_adsp_stop(adsp_hcd->usb_adsp);
	if (ret)
		return ret;

	return adsp_hcd->driver_origin->drop_endpoint(hcd, udev, ep);
}

static void adsp_xhci_endpoint_disable(struct usb_hcd *hcd,
				       struct usb_host_endpoint *host_ep)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->endpoint_disable(hcd, host_ep);
}

static void adsp_xhci_endpoint_reset(struct usb_hcd *hcd,
				     struct usb_host_endpoint *host_ep)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->endpoint_reset(hcd, host_ep);
}

static int adsp_xhci_check_bandwidth(struct usb_hcd *hcd,
				     struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);
	int ret;

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->check_bandwidth(hcd, udev);

	ret = xr_usb_adsp_stop(adsp_hcd->usb_adsp);
	if (ret)
		return ret;

	return adsp_hcd->driver_origin->check_bandwidth(hcd, udev);
}

static void adsp_xhci_reset_bandwidth(struct usb_hcd *hcd,
				      struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->reset_bandwidth(hcd, udev);
}

static int adsp_xhci_address_device(struct usb_hcd *hcd,
				    struct usb_device *udev,
				    unsigned int timeout_ms)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->address_device(hcd, udev,
							       timeout_ms);

	return -ENOENT;
}

static int adsp_xhci_enable_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->enable_device(hcd, udev);

	return -ENOENT;
}

static int adsp_xhci_update_hub_device(struct usb_hcd *hcd,
				       struct usb_device *hdev,
				       struct usb_tt *tt, gfp_t mem_flags)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->update_hub_device(hcd, hdev, tt,
								  mem_flags);

	return -ENOENT;
}

static int adsp_xhci_reset_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);
	int ret = -ENOENT;

	atomic_inc(&adsp_hcd->use_count);
	if (use_origin_hc_driver(adsp_hcd))
		ret = adsp_hcd->driver_origin->reset_device(hcd, udev);
	atomic_dec(&adsp_hcd->use_count);

	return ret;
}

static int adsp_xhci_get_frame_number(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->get_frame_number(hcd);

	return 0;
}

static int adsp_hcd_hub_control(struct xr_usb_adsp_hcd *adsp_hcd,
				struct usb_hcd *hcd, u16 typeReq, u16 wValue,
				u16 wIndex, char *buf, u16 wLength)
{
	int ret = -ENOTSUPP;

	pr_info("control typeReq%04x v%04x i%04x l%u\n", typeReq, wValue,
		wIndex, wLength);

	switch (typeReq) {
	case GetPortStatus:
		if (wLength != sizeof(u32))
			return -EINVAL;

		put_unaligned(
			cpu_to_le32(xr_usb_adsp_hcd_get_port_state(adsp_hcd)),
			(__le32 *)buf);
		ret = 0;
		break;
	case SetPortFeature:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			atomic_or(USB_PORT_STAT_SUSPEND, &adsp_hcd->port_state);
			ret = 0;
			break;
		case USB_PORT_FEAT_RESET:
			WARN(true, "port reset");
			adsp_hcd_wait_state_origin(adsp_hcd);
			ret = adsp_hcd->driver_origin->hub_control(
				hcd, typeReq, wValue, wIndex, buf, wLength);
			break;
		default:
			break;
		}
		break;
	case ClearPortFeature:
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			atomic_andnot(USB_PORT_STAT_SUSPEND,
				      &adsp_hcd->port_state);
			ret = 0;
			break;
		case USB_PORT_FEAT_C_SUSPEND:
			ret = 0;
			break;
		case USB_PORT_FEAT_ENABLE:
			WARN(true, "port disable");
			adsp_hcd_wait_state_origin(adsp_hcd);
			ret = adsp_hcd->driver_origin->hub_control(
				hcd, typeReq, wValue, wIndex, buf, wLength);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return ret;
}

static int adsp_xhci_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
				 u16 wIndex, char *buf, u16 wLength)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (use_origin_hc_driver(adsp_hcd) || !usb_hcd_is_primary_hcd(hcd))
		return adsp_hcd->driver_origin->hub_control(
			hcd, typeReq, wValue, wIndex, buf, wLength);
	else
		return adsp_hcd_hub_control(adsp_hcd, hcd, typeReq, wValue,
					    wIndex, buf, wLength);
}

static int adsp_xhci_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (use_origin_hc_driver(adsp_hcd))
		return adsp_hcd->driver_origin->hub_status_data(hcd, buf);

	return -ENOENT;
}

static int adsp_xhci_bus_suspend(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->bus_suspend(hcd);

	return 0;
}

static int adsp_xhci_bus_resume(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->bus_resume(hcd);

	return 0;
}

static unsigned long adsp_xhci_get_resuming_ports(struct usb_hcd *hcd)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->get_resuming_ports(hcd);

	return 0;
}

static int adsp_xhci_update_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->update_device(hcd, udev);

	return 0;
}

static int adsp_xhci_set_usb2_hw_lpm(struct usb_hcd *hcd,
				     struct usb_device *udev, int enable)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->set_usb2_hw_lpm(hcd, udev,
								enable);

	return 0;
}

static int adsp_xhci_enable_usb3_lpm_timeout(struct usb_hcd *hcd,
					     struct usb_device *udev,
					     enum usb3_link_state state)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->enable_usb3_lpm_timeout(
			hcd, udev, state);

	return 0;
}

static int adsp_xhci_disable_usb3_lpm_timeout(struct usb_hcd *hcd,
					      struct usb_device *udev,
					      enum usb3_link_state state)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->disable_usb3_lpm_timeout(
			hcd, udev, state);

	return 0;
}

static int adsp_xhci_find_raw_port_number(struct usb_hcd *hcd, int port1)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		return adsp_hcd->driver_origin->find_raw_port_number(hcd,
								     port1);
	return 0;
}

static void adsp_xhci_clear_tt_buffer_complete(struct usb_hcd *hcd,
					       struct usb_host_endpoint *ep)
{
	struct xr_usb_adsp_hcd *adsp_hcd = hcd_to_adsp_hcd(hcd);

	if (likely(use_origin_hc_driver(adsp_hcd)))
		adsp_hcd->driver_origin->clear_tt_buffer_complete(hcd, ep);
}

/* hc driver overrides */
static const struct hc_driver xr_usb_adsp_hc_driver = {
	.description = "xhci-hcd",
	.product_desc = "xHCI Host Controller",
	.hcd_priv_size = sizeof(struct xhci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = adsp_xhci_irq,
	.flags = HCD_MEMORY | HCD_DMA | HCD_USB3 | HCD_SHARED | HCD_BH,

	/*
	 * basic lifecycle operations
	 */
	.reset = adsp_xhci_reset, /* set in xhci_init_driver() */
	.start = adsp_xhci_start,
	.stop = adsp_xhci_stop,
	.shutdown = adsp_xhci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.map_urb_for_dma = adsp_xhci_map_urb_for_dma,
	.unmap_urb_for_dma = adsp_xhci_unmap_urb_for_dma,
	.urb_enqueue = adsp_xhci_urb_enqueue,
	.urb_dequeue = adsp_xhci_urb_dequeue,
	.alloc_dev = adsp_xhci_alloc_dev,
	.free_dev = adsp_xhci_free_dev,
	.alloc_streams = adsp_xhci_alloc_streams,
	.free_streams = adsp_xhci_free_streams,
	.add_endpoint = adsp_xhci_add_endpoint,
	.drop_endpoint = adsp_xhci_drop_endpoint,
	.endpoint_disable = adsp_xhci_endpoint_disable,
	.endpoint_reset = adsp_xhci_endpoint_reset,
	.check_bandwidth = adsp_xhci_check_bandwidth,
	.reset_bandwidth = adsp_xhci_reset_bandwidth,
	.address_device = adsp_xhci_address_device,
	.enable_device = adsp_xhci_enable_device,
	.update_hub_device = adsp_xhci_update_hub_device,
	.reset_device = adsp_xhci_reset_device,

	/*
	 * scheduling support
	 */
	.get_frame_number = adsp_xhci_get_frame_number,

	/*
	 * root hub support
	 */
	.hub_control = adsp_xhci_hub_control,
	.hub_status_data = adsp_xhci_hub_status_data,
	.bus_suspend = adsp_xhci_bus_suspend,
	.bus_resume = adsp_xhci_bus_resume,
	.get_resuming_ports = adsp_xhci_get_resuming_ports,

	/*
	 * call back when device connected and addressed
	 */
	.update_device = adsp_xhci_update_device,
	.set_usb2_hw_lpm = adsp_xhci_set_usb2_hw_lpm,
	.enable_usb3_lpm_timeout = adsp_xhci_enable_usb3_lpm_timeout,
	.disable_usb3_lpm_timeout = adsp_xhci_disable_usb3_lpm_timeout,
	.find_raw_port_number = adsp_xhci_find_raw_port_number,
	.clear_tt_buffer_complete = adsp_xhci_clear_tt_buffer_complete,
};

static inline struct xr_usb_adsp_urb *
usb_adsp_find_urb_giveback(struct xr_usb_adsp_hcd *adsp_hcd, u32 base)
{
	struct xr_usb_adsp_urb *urb = NULL;
	struct xr_usb_adsp_urb *urb_tmp = NULL;

	spin_lock(&adsp_hcd->urb_lock);
	list_for_each_entry(urb_tmp, &adsp_hcd->urb_deq_list, list) {
		if (urb_tmp->shared_buf->addr == base) {
			urb = urb_tmp;
			break;
		}
	}

	if (!urb) {
		list_for_each_entry(urb_tmp, &adsp_hcd->urb_list, list) {
			if (urb_tmp->shared_buf->addr == base) {
				urb = urb_tmp;
				break;
			}
		}
	}
	if (urb)
		list_del_init(&urb->list);
	spin_unlock(&adsp_hcd->urb_lock);

	return urb;
}

int xr_usb_adsp_hcd_urb_giveback(struct xr_usb_adsp_hcd *adsp_hcd, u32 base,
				 u32 len)
{
	int ret = 0;
	int status = 0;
	u32 shared_buf_len;
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;
	struct xr_usb_adsp_urb *urb = NULL;
	struct usb_adsp_urb_data *adsp_urb_data = NULL;
	bool is_in;

	urb = usb_adsp_find_urb_giveback(adsp_hcd, base);
	if (!urb) {
		pr_err("can not found urb in list\n");
		return -EINVAL;
	}

	if (!urb->urb) {
		pr_info("urb has been giveback\n");
		goto free_urb;
	}

	shared_buf_len = urb->shared_buf->len * urb->shared_buf->merge_num;
	if (len < sizeof(*adsp_urb_data) || len != shared_buf_len) {
		pr_err("invalid len of shared buf\n");
		ret = -EINVAL;
		status = -ENOENT;
		goto giveback_urb;
	}

	adsp_urb_data =
		(struct usb_adsp_urb_data *)(urb->shared_buf->virt_addr);
	if (adsp_urb_data->status) {
		pr_err("urb error status %d\n", adsp_urb_data->status);
		status = adsp_urb_data->status;
		urb->urb->actual_length = 0;
		goto giveback_urb;
	}

	urb->urb->actual_length = adsp_urb_data->actual_len;
	if (urb->urb->actual_length > shared_buf_len - sizeof(*adsp_urb_data)) {
		pr_warn("actual length over buffer max\n");
		urb->urb->actual_length = 0;
	}

	if (urb->urb->actual_length == 0)
		goto giveback_urb;

	if (usb_endpoint_type(&urb->urb->ep->desc) == USB_ENDPOINT_XFER_CONTROL)
		is_in = !!(adsp_urb_data->ctrl.bRequestType & USB_DIR_IN);
	else
		is_in = usb_endpoint_dir_in(&urb->urb->ep->desc);

	if (is_in) {
		if (memcpy_s(urb->urb->transfer_buffer,
			     urb->urb->transfer_buffer_length,
			     adsp_urb_data->data,
			     adsp_urb_data->actual_len) != EOK) {
			pr_warn("URB req %u and actual %u transfer length over\n",
				urb->urb->transfer_buffer_length,
				urb->urb->actual_length);
			urb->urb->actual_length = 0;
		}
	}

giveback_urb:
	usb_hcd_unlink_urb_from_ep(adsp_hcd->hcd, urb->urb);
	usb_hcd_giveback_urb(adsp_hcd->hcd, urb->urb, status);
free_urb:
	if (urb->ep_type == USB_ENDPOINT_XFER_INT) {
		pm_runtime_mark_last_busy(usb_adsp->dev);
	} else {
		pm_runtime_mark_last_busy(usb_adsp->dev);
		pm_runtime_put_autosuspend(usb_adsp->dev);
	}

	xr_usb_adsp_put_shared_buf(usb_adsp, urb->shared_buf);
	kfree(urb);

	return ret;
}

static int
hc_work_submit_urb_to_adsp(struct xring_usb_adsp *usb_adsp,
			   struct usb_adsp_hc_submit_urb_data *urb_data)
{
	int ret;
	u32 shared_buf_len;
	struct urb *urb = urb_data->urb;
	struct xr_usb_adsp_urb *adsp_urb = NULL;
	struct xr_usb_adsp_hcd *adsp_hcd = usb_adsp->adsp_hcd;
	struct usb_adsp_urb_data *adsp_urb_data = NULL;
	bool is_out = false;

	if (!urb)
		return -EINVAL;

	ret = usb_adsp_handle_local_control(adsp_hcd, adsp_hcd->hcd, urb);
	if (ret <= 0) {
		pr_info("local urb handled ret - %d\n", ret);
		return ret;
	}

	pm_runtime_get_sync(usb_adsp->dev);
	if (urb->transfer_buffer_length > USB_ADSP_URB_LEN_MAX) {
		pr_err("transfer length %u over max %u\n",
		       urb->transfer_buffer_length, USB_ADSP_URB_LEN_MAX);
		ret = -EINVAL;
		goto err_giveback_urb;
	}

	adsp_urb = kzalloc(sizeof(*adsp_urb), GFP_KERNEL);
	if (!adsp_urb) {
		ret = -ENOMEM;
		goto err_giveback_urb;
	}

	adsp_urb->shared_buf = xr_usb_adsp_get_shared_buf(
		usb_adsp, urb->transfer_buffer_length + sizeof(*adsp_urb_data));
	if (!adsp_urb->shared_buf) {
		ret = -ENOMEM;
		goto err_free_urb;
	}

	shared_buf_len =
		adsp_urb->shared_buf->len * adsp_urb->shared_buf->merge_num;
	memset(adsp_urb->shared_buf->virt_addr, 0, shared_buf_len);

	INIT_LIST_HEAD(&adsp_urb->list);
	adsp_urb->urb = urb;
	adsp_urb->ep_type = usb_endpoint_type(&urb->ep->desc);

	adsp_urb_data =
		(struct usb_adsp_urb_data *)adsp_urb->shared_buf->virt_addr;
	adsp_urb_data->pipe = urb->pipe;
	adsp_urb_data->len = urb->transfer_buffer_length;
	adsp_urb_data->status = -EINPROGRESS;

	if (usb_endpoint_xfer_control(&urb->ep->desc)) {
		adsp_urb_data->ctrl =
			*((struct usb_ctrlrequest *)urb->setup_packet);
		is_out = !(adsp_urb_data->ctrl.bRequestType & USB_DIR_IN);
	}

	if (is_out && urb->transfer_buffer_length) {
		if (memcpy_s(adsp_urb_data->data,
			     shared_buf_len - sizeof(*adsp_urb_data),
			     urb->transfer_buffer,
			     urb->transfer_buffer_length) != EOK) {
			pr_err("failed to copy transfer data\n");
			ret = -ESHUTDOWN;
			goto err_free_buf;
		}
	}

	ret = usb_adsp_send_urb_msg(adsp_urb->shared_buf->addr, shared_buf_len);
	if (ret) {
		pr_err("failed to send urb msg\n");
		ret = -ESHUTDOWN;
		goto err_free_buf;
	}

	spin_lock(&adsp_hcd->urb_lock);
	list_add_tail(&adsp_urb->list, &adsp_hcd->urb_list);
	spin_unlock(&adsp_hcd->urb_lock);

	if (usb_endpoint_xfer_int(&urb->ep->desc)) {
		pm_runtime_mark_last_busy(usb_adsp->dev);
		pm_runtime_put_autosuspend(usb_adsp->dev);
	}

	return 0;

err_free_buf:
	xr_usb_adsp_put_shared_buf(usb_adsp, adsp_urb->shared_buf);
err_free_urb:
	kfree(adsp_urb);
err_giveback_urb:
	usb_hcd_unlink_urb_from_ep(adsp_hcd->hcd, urb);
	usb_hcd_giveback_urb(adsp_hcd->hcd, urb, ret);
	pm_runtime_mark_last_busy(usb_adsp->dev);
	pm_runtime_put_autosuspend(usb_adsp->dev);

	return ret;
}

static int hc_work_cancel_urb(struct xring_usb_adsp *usb_adsp,
			      struct usb_adsp_hc_submit_urb_data *urb_data)
{
	struct xr_usb_adsp_hcd *adsp_hcd = usb_adsp->adsp_hcd;
	struct urb *urb = urb_data->urb;

	if (!urb)
		return -EINVAL;

	pr_warn("submit in state %d\n", xr_usb_adsp_hcd_get_state(adsp_hcd));
	usb_hcd_unlink_urb_from_ep(adsp_hcd->hcd, urb);
	usb_hcd_giveback_urb(adsp_hcd->hcd, urb, -ENOENT);

	return 0;
}

static int hc_work_submit_urb(struct xring_usb_adsp *usb_adsp,
			      struct usb_adsp_hc_submit_urb_data *urb_data)
{
	struct xr_usb_adsp_hcd *adsp_hcd = usb_adsp->adsp_hcd;
	int state = xr_usb_adsp_hcd_get_state(adsp_hcd);

	if (state == USB_ADSP_HCD_STATE_ADSP)
		return hc_work_submit_urb_to_adsp(usb_adsp, urb_data);
	else
		return hc_work_cancel_urb(usb_adsp, urb_data);
}

static int hc_work_dequeue_urb(struct xring_usb_adsp *usb_adsp,
			       struct usb_adsp_hc_deq_urb_data *deq_urb)
{
	int ret;
	unsigned long flags;
	struct xr_usb_adsp_hcd *adsp_hcd = usb_adsp->adsp_hcd;
	struct urb *urb = deq_urb->urb;
	struct xr_usb_adsp_urb *adsp_urb = NULL;
	struct xr_usb_adsp_urb *urb_tmp = NULL;
	struct xhci_hcd *xhci = NULL;
	u32 shared_buf_addr = 0;
	u32 shared_buf_len = 0;

	pr_info("dequeue urb\n");
	spin_lock(&adsp_hcd->urb_lock);
	list_for_each_entry(urb_tmp, &adsp_hcd->urb_deq_list, list) {
		if (urb_tmp->urb == urb) {
			adsp_urb = urb_tmp;
			adsp_urb->urb = NULL;
			break;
		}
	}
	spin_unlock(&adsp_hcd->urb_lock);

	if (adsp_urb) {
		shared_buf_addr = adsp_urb->shared_buf->addr;
		shared_buf_len = adsp_urb->shared_buf->len *
				 adsp_urb->shared_buf->merge_num;
	} else if (usb_endpoint_xfer_int(&urb->ep->desc)) {
		pr_info("dequeue first hid urb\n");
	} else {
		pr_err("failed to find dequeue urb\n");
		xhci = hcd_to_xhci(adsp_hcd->hcd);
		spin_lock_irqsave(&xhci->lock, flags);
		ret = usb_hcd_check_unlink_urb(adsp_hcd->hcd, urb,
					       deq_urb->status);
		spin_unlock_irqrestore(&xhci->lock, flags);
		if (ret == -EIDRM) {
			pr_err("urb not in ep list\n");
			return ret;
		}

		pr_err("dequeue urb in ep's list\n");
		return -ENOENT;
	}

	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ADSP) {
		pr_err("state invalid while dequeue urb\n");
		ret = -ENODEV;
		goto giveback_urb;
	}

	ret = usb_adsp_send_deq_urb_msg(shared_buf_addr, shared_buf_len,
					!shared_buf_addr);
	if (ret)
		pr_err("failed to send dequeue msg\n");

	/*
	 * for urb not found in urb_deq_list, it maybe first hid urb
	 * and should wait dequeue finish on adsp
	 */
	if (!adsp_urb)
		return ret;

giveback_urb:
	usb_hcd_unlink_urb_from_ep(adsp_hcd->hcd, urb);
	usb_hcd_giveback_urb(adsp_hcd->hcd, urb, deq_urb->status);
	return ret;
}

static void handle_hc_request(struct xr_usb_adsp_hcd *adsp_hcd)
{
	int ret;
	struct usb_adsp_hc_request req = { 0 };
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;

	while (kfifo_out_spinlocked(&adsp_hcd->hc_request_fifo, &req, 1,
				    &adsp_hcd->hc_request_lock)) {
		ret = -EINVAL;

		switch (req.id) {
		case USB_ADSP_HC_REQ_SUBMIT_URB:
			ret = hc_work_submit_urb(usb_adsp, &req.urb_data);
			break;
		case USB_ADSP_HC_REQ_DEQUEUE_URB:
			ret = hc_work_dequeue_urb(usb_adsp, &req.deq_urb);
			break;
		default:
			break;
		}

		if (ret)
			pr_err("failed to handle hc req %u - %d\n", req.id,
			       ret);
	}
}

static void handle_hc_work(struct work_struct *work)
{
	struct xr_usb_adsp_hcd *adsp_hcd =
		container_of(work, struct xr_usb_adsp_hcd, hc_work);
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;

	if (!usb_adsp) {
		pr_err("no valid usb_adsp\n");
		return;
	}

	mutex_lock(&usb_adsp->mutex);
	if (xr_usb_adsp_hcd_get_state(adsp_hcd) != USB_ADSP_HCD_STATE_ADSP) {
		pr_warn("exit hc_work while not in state adsp\n");
		goto unlock;
	}

	handle_hc_request(adsp_hcd);

unlock:
	mutex_unlock(&usb_adsp->mutex);
}

int xr_usb_adsp_hcd_first_hid_in(struct xr_usb_adsp_hcd *adsp_hcd, u8 ep_index,
				 u32 actual_len, int status)
{
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;
	struct usb_device *udev = usb_adsp->audio_udev;
	struct usb_hcd *primary_hcd = adsp_hcd->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(primary_hcd);
	struct xhci_virt_ep *ep = NULL;
	struct xhci_td *td = NULL;
	struct xhci_ring *evt_ring = NULL;
	int slot_id = udev->slot_id;
	int ret = -ENODEV;
	u32 requested;

	if (slot_id <= 0 || slot_id >= MAX_HC_SLOTS) {
		pr_err("invalid slot id %d\n", slot_id);
		return -EINVAL;
	}
	if (ep_index <= 1) {
		pr_err("invalid ep_index %u\n", ep_index);
		return -EINVAL;
	}

	/* convert from usb ep index to xhci ep index */
	ep_index--;
	if (ep_index >= EP_CTX_PER_DEV) {
		pr_err("invalid ep_index %u\n", ep_index);
		return -EINVAL;
	}

	spin_lock(&xhci->lock);
	if (!xhci->devs[slot_id]) {
		pr_err("no valid virt device for slot %d\n", slot_id);
		goto err_out;
	}

	ep = &xhci->devs[slot_id]->eps[ep_index];
	if (!ep) {
		pr_err("no valid ep for slot %d ep_index %u\n", slot_id,
		       ep_index);
		goto err_out;
	}
	if (!ep->ring) {
		pr_err("no valid ep ring for slot %d ep_index %u\n", slot_id,
		       ep_index);
		goto err_out;
	}

	if (list_empty(&ep->ring->td_list)) {
		pr_err("no valid td for slot %d ep_index %u\n", slot_id,
		       ep_index);
		ret = -ENOENT;
		goto inc_evt_ring;
	}

	td = list_first_entry(&ep->ring->td_list, struct xhci_td, td_list);
	if (!usb_endpoint_xfer_int(&td->urb->ep->desc)) {
		pr_err("trb complete not fo interrupt endpoint\n");
		goto clean_up;
	}

	if (!status) {
		td->status = 0;
		td->urb->actual_length = actual_len;
	} else {
		pr_warn("first hid in urb return error - %d\n", status);
		td->status = status;
		td->urb->actual_length = 0;
	}

	requested = td->urb->transfer_buffer_length;
	if (actual_len > requested) {
		pr_warn("bad transfer trb length %u in event trb\n",
			actual_len);
		td->urb->actual_length = 0;
	}

clean_up:
	/* Update ring dequeue pointer */
	ep->ring->dequeue = td->last_trb;
	ep->ring->deq_seg = td->last_trb_seg;
	ep->ring->num_trbs_free += td->num_trbs - 1;
	xhci_inc_deq(xhci, ep->ring);

	ret = xhci_td_cleanup(xhci, td, ep->ring, td->status);

inc_evt_ring:
	evt_ring = usb_adsp_get_xhci_evt_ring(xhci);
	if (evt_ring)
		xhci_inc_deq(xhci, evt_ring);
err_out:
	spin_unlock(&xhci->lock);
	return ret;
}

static unsigned int xr_usb_xhci_port_speed(unsigned int port_status)
{
	if (DEV_LOWSPEED(port_status))
		return USB_PORT_STAT_LOW_SPEED;
	if (DEV_HIGHSPEED(port_status))
		return USB_PORT_STAT_HIGH_SPEED;

	return 0;
}

static u32 xhci_portsc_to_port_status(u32 portsc)
{
	u32 port_status = 0;
	u32 link_state;

	if (portsc & PORT_CSC)
		port_status |= USB_PORT_STAT_C_CONNECTION
			       << USB_PORT_STATE_CHANGE_SHIFT;
	if (portsc & PORT_PEC)
		port_status |= USB_PORT_STAT_C_ENABLE
			       << USB_PORT_STATE_CHANGE_SHIFT;
	if ((portsc & PORT_OCC))
		port_status |= USB_PORT_STAT_C_OVERCURRENT
			       << USB_PORT_STATE_CHANGE_SHIFT;
	if ((portsc & PORT_RC))
		port_status |= USB_PORT_STAT_C_RESET
			       << USB_PORT_STATE_CHANGE_SHIFT;

	/* common wPortPort_Status bits */
	if (portsc & PORT_CONNECT) {
		port_status |= USB_PORT_STAT_CONNECTION;
		port_status |= xr_usb_xhci_port_speed(portsc);
	}
	if (portsc & PORT_PE)
		port_status |= USB_PORT_STAT_ENABLE;
	if (portsc & PORT_OC)
		port_status |= USB_PORT_STAT_OVERCURRENT;
	if (portsc & PORT_RESET)
		port_status |= USB_PORT_STAT_RESET;

	link_state = portsc & PORT_PLS_MASK;
	if (portsc & PORT_POWER) {
		port_status |= USB_PORT_STAT_POWER;

		/* link state is only valid if port is powered */
		if (link_state == XDEV_U3)
			port_status |= USB_PORT_STAT_SUSPEND;
		if (link_state == XDEV_U2)
			port_status |= USB_PORT_STAT_L1;
	}

	return port_status;
}

/*
 * Return 1 for disconnect and 0 no action
 */
int xr_usb_adsp_hcd_port_change(struct xr_usb_adsp_hcd *adsp_hcd, u32 portsc)
{
	int disconnect = 0;
	union {
		struct usb_hub_status hub_status;
		u32 data;
	} port_status;
	u16 change;
	u16 status;

	if (portsc == ~(u32)0) {
		pr_err("port fault\n");
		return -ENODEV;
	}

	port_status.data = xhci_portsc_to_port_status(portsc);
	change = port_status.hub_status.wHubChange;
	status = port_status.hub_status.wHubStatus;

	if (change & USB_PORT_STAT_C_CONNECTION) {
		pr_info("port connect change\n");

		if (!(status & USB_PORT_STAT_CONNECTION)) {
			atomic_andnot(USB_PORT_STAT_CONNECTION,
				      &adsp_hcd->port_state);
			atomic_or(USB_PORT_STAT_C_CONNECTION
					  << USB_PORT_STATE_CHANGE_SHIFT,
				  &adsp_hcd->port_state);
			disconnect = 1;
		} else {
			pr_warn("port change to connect at adsp?\n");
		}
	}

	if (change & USB_PORT_STAT_C_ENABLE)
		pr_warn("port enable change\n");

	if (change & USB_PORT_STAT_C_OVERCURRENT)
		pr_warn("port over current change\n");

	if (change & USB_PORT_STAT_C_RESET)
		pr_warn("port reset change\n");

	if (change & USB_PORT_STAT_C_LINK_STATE)
		pr_info("port link state change\n");

	return disconnect;
}

void xr_usb_adsp_hcd_died(struct xr_usb_adsp_hcd *adsp_hcd)
{
	pr_err("do hc died\n");
	if (adsp_hcd->hcd) {
		struct xhci_hcd *xhci = hcd_to_xhci(adsp_hcd->hcd);
		unsigned long flags;

		spin_lock_irqsave(&xhci->lock, flags);
		usb_adsp_xhci_hc_died(xhci);
		spin_unlock_irqrestore(&xhci->lock, flags);
	}
}

void xr_usb_adsp_hcd_flush_work(struct xr_usb_adsp_hcd *adsp_hcd)
{
	handle_hc_request(adsp_hcd);
}

void xr_usb_adsp_hcd_start(struct xr_usb_adsp_hcd *adsp_hcd)
{
	xr_usb_adsp_hcd_enable_port(adsp_hcd);
}

void xr_usb_adsp_hcd_stop(struct xr_usb_adsp_hcd *adsp_hcd)
{
	struct xr_usb_adsp_urb *adsp_urb = NULL;
	struct xr_usb_adsp_urb *urb_tmp = NULL;

	/* Giveback all urbs */
	spin_lock(&adsp_hcd->urb_lock);
	list_for_each_entry_safe(adsp_urb, urb_tmp, &adsp_hcd->urb_list, list) {
		list_del_init(&adsp_urb->list);
		spin_unlock(&adsp_hcd->urb_lock);

		usb_hcd_unlink_urb_from_ep(adsp_hcd->hcd, adsp_urb->urb);
		usb_hcd_giveback_urb(adsp_hcd->hcd, adsp_urb->urb, -ESHUTDOWN);
		xr_usb_adsp_put_shared_buf(adsp_hcd->usb_adsp,
					   adsp_urb->shared_buf);
		kfree(adsp_urb);

		spin_lock(&adsp_hcd->urb_lock);
	}

	/* Giveback all dequeued urbs */
	list_for_each_entry_safe(adsp_urb, urb_tmp, &adsp_hcd->urb_deq_list,
				 list) {
		list_del_init(&adsp_urb->list);
		spin_unlock(&adsp_hcd->urb_lock);

		if (adsp_urb->urb) {
			usb_hcd_unlink_urb_from_ep(adsp_hcd->hcd,
						   adsp_urb->urb);
			usb_hcd_giveback_urb(adsp_hcd->hcd, adsp_urb->urb,
					     -ESHUTDOWN);
		}
		xr_usb_adsp_put_shared_buf(adsp_hcd->usb_adsp,
					   adsp_urb->shared_buf);
		kfree(adsp_urb);

		spin_lock(&adsp_hcd->urb_lock);
	}
	spin_unlock(&adsp_hcd->urb_lock);
}

static int usb_adsp_wait_ep0_idle(struct xr_usb_adsp_hcd *adsp_hcd)
{
	struct xring_usb_adsp *usb_adsp = adsp_hcd->usb_adsp;
	struct usb_device *udev = usb_adsp->audio_udev;
	struct usb_hcd *primary_hcd = adsp_hcd->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(primary_hcd);
	struct xhci_virt_device *dev = NULL;
	struct xhci_virt_ep *ep0 = NULL;
	unsigned long flags;
	int retry_count = 10;
	bool ep0_idle = false;

	dev = usb_adsp_get_xhci_dev(xhci, udev->slot_id);
	if (IS_ERR_OR_NULL(dev)) {
		pr_err("failed to get xhci dev error - %ld\n", PTR_ERR(dev));
		return -ENODEV;
	}

	ep0 = &dev->eps[0];
	while (retry_count--) {
		spin_lock_irqsave(&xhci->lock, flags);
		if (!ep0->ring) {
			spin_unlock_irqrestore(&xhci->lock, flags);
			pr_err("ep0 has no ring\n");
			return -ENOENT;
		}

		if (list_empty(&ep0->ring->td_list))
			ep0_idle = true;

		spin_unlock_irqrestore(&xhci->lock, flags);

		if (ep0_idle)
			break;

		msleep(20);

		/* try process event */
		(void)adsp_hcd->driver_origin->irq(primary_hcd);
	}

	if (!ep0_idle) {
		pr_err("wait ep0 idle timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

int xr_usb_adsp_hcd_wait_idle(struct xr_usb_adsp_hcd *adsp_hcd)
{
	int timeout = USB_ADSP_WAIT_HCD_IDLE_TIMEOUT;

	while (atomic_read(&adsp_hcd->use_count) && timeout--)
		msleep(20);

	if (atomic_read(&adsp_hcd->use_count))
		return -EBUSY;

	return usb_adsp_wait_ep0_idle(adsp_hcd);
}

void xr_usb_adsp_hcd_add(struct xr_usb_adsp_hcd *adsp_hcd, struct usb_hcd *hcd)
{
	if (!hcd->driver)
		return;

	if (!hcd->driver->description)
		return;

	if (strcmp(hcd->driver->description, "xhci-hcd"))
		return;

	if (!adsp_hcd->driver_origin)
		adsp_hcd->driver_origin = hcd->driver;

	if (usb_hcd_is_primary_hcd(hcd)) {
		atomic_set(&adsp_hcd->use_count, 0);
		atomic_set(&adsp_hcd->state, USB_ADSP_HCD_STATE_ORIGIN);
		atomic_set(&adsp_hcd->port_state, 0);
		adsp_hcd->hcd = hcd;
	}

	pr_info("Override xhci hcd driver\n");
	hcd->driver = &adsp_hcd->driver_overrides;

	/* set cma area */
	if (WARN_ON(!hcd->self.sysdev))
		return;

	hcd->self.sysdev->cma_area = dma_contiguous_default_area;
}

void xr_usb_adsp_hcd_remove(struct xr_usb_adsp_hcd *adsp_hcd,
			    struct usb_hcd *hcd)
{
	if (hcd->driver == &adsp_hcd->driver_overrides) {
		pr_info("Restore xhci hcd driver\n");
		if (WARN_ON(!kfifo_is_empty(&adsp_hcd->hc_request_fifo)))
			xr_usb_adsp_hcd_flush_work(adsp_hcd);
		if (WARN_ON(!list_empty(&adsp_hcd->urb_list) ||
			    !list_empty(&adsp_hcd->urb_deq_list)))
			xr_usb_adsp_hcd_stop(adsp_hcd);

		hcd->driver = adsp_hcd->driver_origin;
		/* Primary hcd deleted after shared hcd */
		if (usb_hcd_is_primary_hcd(hcd)) {
			adsp_hcd->driver_origin = NULL;
			adsp_hcd->hcd = NULL;
		}
	}
}

struct xr_usb_adsp_hcd *xr_usb_adsp_hcd_create(struct device *dev,
					       struct xring_usb_adsp *usb_adsp)
{
	struct xr_usb_adsp_hcd *adsp_hcd = NULL;

	adsp_hcd = devm_kzalloc(dev, sizeof(*adsp_hcd), GFP_KERNEL);
	if (!adsp_hcd)
		return ERR_PTR(-ENOMEM);

	adsp_hcd->usb_adsp = usb_adsp;
	INIT_LIST_HEAD(&adsp_hcd->urb_list);
	INIT_LIST_HEAD(&adsp_hcd->urb_deq_list);
	spin_lock_init(&adsp_hcd->urb_lock);
	spin_lock_init(&adsp_hcd->hc_request_lock);
	INIT_WORK(&adsp_hcd->hc_work, handle_hc_work);
	INIT_KFIFO(adsp_hcd->hc_request_fifo);
	if (memcpy_s(&adsp_hcd->driver_overrides,
		     sizeof(adsp_hcd->driver_overrides), &xr_usb_adsp_hc_driver,
		     sizeof(xr_usb_adsp_hc_driver)) != EOK) {
		pr_err("falied to copy hc_driver\n");
		devm_kfree(dev, adsp_hcd);
		return ERR_PTR(-EFAULT);
	}

	return adsp_hcd;
}

void xr_usb_adsp_hcd_destroy(struct device *dev,
			     struct xr_usb_adsp_hcd *adsp_hcd)
{
	flush_work(&adsp_hcd->hc_work);
	if (adsp_hcd->hcd) {
		xr_usb_adsp_hcd_remove(adsp_hcd, adsp_hcd->hcd->shared_hcd);
		xr_usb_adsp_hcd_remove(adsp_hcd, adsp_hcd->hcd);
	}
	devm_kfree(dev, adsp_hcd);
}
