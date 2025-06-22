// SPDX-License-Identifier: GPL-2.0
/*
 * switch.c - Switch Control of USB Audio Offload.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "core.h"
#include "hcd.h"
#include "ipc.h"
#include "xhci-helper.h"

#include <core/usb.h>
#include <host/xhci.h>
#include <linux/bits.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/hid.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mod_devicetable.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/usb/ch9.h>

#define USB_ADSP_WAIT_SWITCH_ACK_TIMEOUT     5000
#define USB_ADSP_WAIT_EVT_RING_EMPTY_TIMEOUT 20 /* 20 * 20ms = 400ms */

#define XHCI_ERST_PTR_MASK    (~0xFULL)
#define XHCI_CYCLE_STATE_MASK 1
#define CMD_RING_SEG_NUM      1
#define MAX_TRANS_RING_SEG    4

static int usb_adsp_check_xhci_status(struct xhci_hcd *xhci)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&xhci->lock, flags);
	if (xhci->xhc_state &
	    (XHCI_STATE_DYING | XHCI_STATE_HALTED | XHCI_STATE_REMOVING)) {
		ret = -ESHUTDOWN;
		goto err_unlock;
	}

	if (!list_empty(&xhci->cmd_list)) {
		ret = -EBUSY;
		goto err_unlock;
	}

	spin_unlock_irqrestore(&xhci->lock, flags);

	return 0;

err_unlock:
	spin_unlock_irqrestore(&xhci->lock, flags);
	pr_err("check xhci status error - %d\n", ret);
	return ret;
}

static inline int usb_adsp_get_dma_addr(dma_addr_t dma, u32 *out)
{
	pr_debug("get dam value 0x%pad\n", &dma);
	if (!dma || upper_32_bits(dma))
		return -EINVAL;

	*out = lower_32_bits(dma);

	return 0;
}

static int usb_adsp_get_xhci_cmd_ring_info(struct xhci_hcd *xhci,
					   struct adsp_xhci_info *info)
{
	int ret;

	if (!xhci->cmd_ring)
		return -ENOMEM;

	if (xhci->cmd_ring->num_segs != CMD_RING_SEG_NUM) {
		pr_err("invalid cmd ring segment number %u\n",
		       xhci->cmd_ring->num_segs);
		return -EINVAL;
	}

	if (xhci->cmd_ring->enqueue != xhci->cmd_ring->dequeue) {
		pr_err("cmd ring is busy\n");
		return -EBUSY;
	}

	pr_debug("cmd ring dma 0x%pad\n", &xhci->cmd_ring->first_seg->dma);
	ret = usb_adsp_get_dma_addr(xhci->cmd_ring->first_seg->dma,
				    &info->cmd_ring_base);
	if (ret) {
		pr_err("invalid cmd ring address\n");
		return ret;
	}

	info->cmd_ring_base |= xhci->cmd_ring->cycle_state;

	ret = usb_adsp_get_dma_addr(
		xhci_trb_virt_to_dma(xhci->cmd_ring->enq_seg,
				     xhci->cmd_ring->enqueue),
		&info->cmd_ring_enqueue);
	if (ret) {
		pr_err("invalid cmd ring enqueue\n");
		return ret;
	}

	return 0;
}

static int usb_adsp_wait_xhci_event_handled(struct xhci_hcd *xhci)
{
	unsigned long flags;
	int timeout;
	bool empty;
	u64 temp_64;
	dma_addr_t deq;
	struct xhci_intr_reg __iomem *ir_set = NULL;
	struct xhci_ring *evt_ring = NULL;

	ir_set = usb_adsp_get_xhci_ir_set(xhci);
	if (!ir_set)
		return -EINVAL;

	evt_ring = usb_adsp_get_xhci_evt_ring(xhci);
	if (!evt_ring)
		return -EINVAL;

	/* wait event ring empty */
	timeout = USB_ADSP_WAIT_EVT_RING_EMPTY_TIMEOUT;
	while (timeout--) {
		spin_lock_irqsave(&xhci->lock, flags);
		deq = xhci_trb_virt_to_dma(evt_ring->deq_seg,
					   evt_ring->dequeue);
		temp_64 = xhci_read_64(xhci, &ir_set->erst_dequeue);
		temp_64 &= (u64)XHCI_ERST_PTR_MASK;
		empty = (temp_64 == deq);
		spin_unlock_irqrestore(&xhci->lock, flags);

		if (empty)
			return 0;

		msleep(20);
	}

	return -EBUSY;
}

void xr_usb_adsp_disable_xhci_irq(struct xhci_hcd *xhci, bool disable)
{
	unsigned long flags;
	u32 iman;
	struct xhci_intr_reg __iomem *ir_set = NULL;

	ir_set = usb_adsp_get_xhci_ir_set(xhci);
	if (!ir_set) {
		pr_err("failed to get xhci ir_set\n");
		return;
	}

	spin_lock_irqsave(&xhci->lock, flags);
	iman = readl(&ir_set->irq_pending);
	if (disable)
		iman &= ~IMAN_IE;
	else
		iman |= IMAN_IE;
	writel(iman, &ir_set->irq_pending);
	spin_unlock_irqrestore(&xhci->lock, flags);
}

static int usb_adsp_get_xhci_evt_ring_info(struct xhci_hcd *xhci,
					   struct adsp_xhci_info *info)
{
	int ret;
	unsigned int i;
	struct xhci_segment *seg = NULL;
	struct xhci_ring *evt_ring = NULL;

	evt_ring = usb_adsp_get_xhci_evt_ring(xhci);
	if (!evt_ring)
		return -EINVAL;

	if (!evt_ring->num_segs)
		return -EINVAL;

	ret = usb_adsp_wait_xhci_event_handled(xhci);
	if (ret) {
		pr_err("failed to wait xhci event ring empty\n");
		return ret;
	}

	xr_usb_adsp_disable_xhci_irq(xhci, true);

	seg = evt_ring->first_seg;
	for (i = 0; i < evt_ring->num_segs; i++) {
		if (!seg->dma || upper_32_bits(seg->dma)) {
			pr_err("invalid event ring seg dma\n");
			return -EINVAL;
		}
		pr_debug("event ring%u dma 0x%pad\n", i, &seg->dma);
		seg = seg->next;
	}
	info->evt_ring_cycle = evt_ring->cycle_state;

	return 0;
}

static int usb_adsp_get_ep_ring_info(struct xhci_virt_ep *xhci_ep,
				     struct adsp_xhci_ep_info *ep_info)
{
	int ret;

	if (xhci_ep->ring->num_segs > MAX_TRANS_RING_SEG) {
		pr_err("invalid ep ring segment number %u\n",
		       xhci_ep->ring->num_segs);
		return -EINVAL;
	}
	ep_info->ep_index = xhci_ep->ep_index;
	ep_info->ring_type = xhci_ep->ring->type;
	ep_info->ring_seg_num = xhci_ep->ring->num_segs;
	pr_debug("ep%u ring dma 0x%pad\n", xhci_ep->ep_index,
		 &xhci_ep->ring->first_seg->dma);
	ret = usb_adsp_get_dma_addr(xhci_ep->ring->first_seg->dma,
				    &ep_info->ring_base);
	if (ret) {
		pr_err("invalid ep ring base\n");
		return ret;
	}

	ep_info->ring_base |= xhci_ep->ring->cycle_state;

	ret = usb_adsp_get_dma_addr(
		xhci_trb_virt_to_dma(xhci_ep->ring->enq_seg,
				     xhci_ep->ring->enqueue),
		&ep_info->ring_enqueue);
	if (ret) {
		pr_err("invalid ep ring enqueue\n");
		return ret;
	}

	ret = usb_adsp_get_dma_addr(
		xhci_trb_virt_to_dma(xhci_ep->ring->deq_seg,
				     xhci_ep->ring->dequeue),
		&ep_info->ring_dequeue);
	if (ret) {
		pr_err("invalid ep ring dequeue\n");
		return ret;
	}

	return 0;
}

static const struct usb_device_id audio_ctrl_usb_id = {
	.match_flags = (USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS),
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_SUBCLASS_AUDIOCONTROL
};

static const struct usb_device_id audio_data_usb_id = {
	.match_flags = (USB_DEVICE_ID_MATCH_INT_CLASS |
			USB_DEVICE_ID_MATCH_INT_SUBCLASS),
	.bInterfaceClass = USB_CLASS_AUDIO,
	.bInterfaceSubClass = USB_SUBCLASS_AUDIOSTREAMING
};

static const struct usb_device_id hid_usb_id = {
	.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
	.bInterfaceClass = USB_INTERFACE_CLASS_HID
};

static int usb_adsp_get_xhci_ep_info(struct xhci_virt_device *dev,
				     struct usb_device *udev,
				     struct usb_adsp_start_info *info)
{
	int ret;
	struct adsp_xhci_info *xhci_info = NULL;

	xhci_info = &info->xhci_info;
	for (int i = 0; i < ADSP_XHCI_EP_MAX; i++) {
		if (!dev->eps[i].ring)
			continue;

		xhci_info->ep_mask |= BIT(i);
		ret = usb_adsp_get_ep_ring_info(&dev->eps[i],
						&xhci_info->eps[i]);
		if (ret) {
			pr_err("failed to get ep%d info\n", i);
			return ret;
		}

		/* Check hid data pending */
		if (xhci_info->eps[i].ring_enqueue !=
		    xhci_info->eps[i].ring_dequeue) {
			pr_info("ep%d data pending\n", i);
			info->udev_info.ep_data_pending_mask |= BIT(i);
		}
	}

	return 0;
}

static int usb_adsp_get_xhci_udev_info(struct xhci_hcd *xhci,
				       struct usb_device *udev,
				       struct usb_adsp_start_info *info)
{
	int ret;
	struct xhci_virt_device *dev = NULL;

	dev = usb_adsp_get_xhci_dev(xhci, udev->slot_id);
	if (IS_ERR_OR_NULL(dev)) {
		pr_err("failed to get xhci dev error - %ld\n", PTR_ERR(dev));
		return -ENODEV;
	}
	info->xhci_info.slot_id = udev->slot_id;

	ret = usb_adsp_get_dma_addr(dev->out_ctx->dma,
				    &info->xhci_info.out_ctx);
	if (ret) {
		pr_err("invalid dev out ctx\n");
		return ret;
	}

	ret = usb_adsp_get_dma_addr(dev->in_ctx->dma, &info->xhci_info.in_ctx);
	if (ret) {
		pr_err("invalid dev in ctx\n");
		return ret;
	}

	ret = usb_adsp_get_xhci_ep_info(dev, udev, info);
	if (ret) {
		pr_err("failed to get ep res - %d\n", ret);
		return ret;
	}
	return 0;
}

static int usb_adsp_get_xhci_info(struct xhci_hcd *xhci,
				  struct usb_device *udev,
				  struct usb_adsp_start_info *info)
{
	int ret;

	if (!xhci->dcbaa)
		return -ENOMEM;

	/* Get command ring info */
	ret = usb_adsp_get_xhci_cmd_ring_info(xhci, &info->xhci_info);
	if (ret)
		return ret;

	ret = usb_adsp_get_xhci_evt_ring_info(xhci, &info->xhci_info);
	if (ret)
		return ret;

	ret = usb_adsp_get_xhci_udev_info(xhci, udev, info);
	if (ret)
		return ret;

	return 0;
}

static int usb_adsp_get_audio_data_info(struct usb_device *udev,
					struct usb_interface *intf,
					u8 **extra_start, u32 *extra_max)
{
	int i, j;
	struct usb_host_interface *intf_alt = NULL;
	struct adsp_udev_intf_info *intf_info =
		(struct adsp_udev_intf_info *)(*extra_start);

	if (*extra_max < sizeof(*intf_info))
		return -ENOMEM;

	intf_info->intf_num = intf->cur_altsetting->desc.bInterfaceNumber;
	intf_info->alt_max = intf->num_altsetting;
	*extra_max -= sizeof(*intf_info);
	*extra_start += sizeof(*intf_info);

	if (!intf->altsetting)
		return -ENODEV;

	for (i = 0; i < intf->num_altsetting; i++) {
		struct adsp_udev_intf_alt_info *tmp_intf_alt = NULL;

		if (*extra_max < sizeof(*tmp_intf_alt))
			return -ENOMEM;

		intf_alt = intf->altsetting + i;
		tmp_intf_alt = (struct adsp_udev_intf_alt_info *)(*extra_start);
		tmp_intf_alt->alt_num = intf_alt->desc.bAlternateSetting;
		tmp_intf_alt->ep_max = intf_alt->desc.bNumEndpoints;
		*extra_start += sizeof(*tmp_intf_alt);
		*extra_max -= sizeof(*tmp_intf_alt);

		if (intf_alt->desc.bNumEndpoints != 0 && !intf_alt->endpoint)
			return -ENOENT;

		for (j = 0; j < intf_alt->desc.bNumEndpoints; j++) {
			struct usb_host_endpoint *ep = NULL;
			struct adsp_udev_ep_info *tmp_ep = NULL;

			if (*extra_max < sizeof(*tmp_ep))
				return -ENOMEM;

			ep = intf_alt->endpoint + j;
			tmp_ep = (struct adsp_udev_ep_info *)(*extra_start);
			if (usb_endpoint_dir_in(&ep->desc))
				tmp_ep->pipe = usb_rcvisocpipe(
					udev, ep->desc.bEndpointAddress);
			else
				tmp_ep->pipe = usb_sndisocpipe(
					udev, ep->desc.bEndpointAddress);
			tmp_ep->max_packet_size =
				usb_endpoint_maxp(&ep->desc) *
				usb_endpoint_maxp_mult(&ep->desc);
			tmp_ep->interval = ep->desc.bInterval;
			*extra_start += sizeof(*tmp_ep);
			*extra_max -= sizeof(*tmp_ep);
		}
	}

	return 0;
}

static int usb_adsp_get_audio_ctrl_info(struct usb_device *udev,
					struct usb_interface *intf,
					u8 **extra_start, u32 *extra_max)
{
	struct adsp_udev_intf_info *intf_info =
		(struct adsp_udev_intf_info *)(*extra_start);
	struct usb_host_interface *intf_alt = NULL;
	struct adsp_udev_intf_alt_info *tmp_intf_alt = NULL;

	if (intf->condition != USB_INTERFACE_BOUND) {
		pr_err("audio interface has not probed\n");
		return -EINVAL;
	}

	if (!intf->cur_altsetting || intf->num_altsetting != 1)
		return -EINVAL;

	if (intf->cur_altsetting->desc.bNumEndpoints > 1)
		return -EINVAL;

	if (*extra_max < sizeof(*intf_info))
		return -ENOMEM;

	intf_info->intf_num = intf->cur_altsetting->desc.bInterfaceNumber;
	intf_info->alt_max = intf->num_altsetting;
	*extra_max -= sizeof(*intf_info);
	*extra_start += sizeof(*intf_info);
	if (*extra_max < sizeof(*tmp_intf_alt))
		return -ENOMEM;

	intf_alt = &intf->altsetting[0];
	if (intf_alt->desc.bNumEndpoints != 0 && !intf_alt->endpoint)
		return -ENOENT;

	tmp_intf_alt = (struct adsp_udev_intf_alt_info *)(*extra_start);
	tmp_intf_alt->alt_num = intf_alt->desc.bAlternateSetting;
	tmp_intf_alt->ep_max = intf_alt->desc.bNumEndpoints;
	*extra_start += sizeof(*tmp_intf_alt);
	*extra_max -= sizeof(*tmp_intf_alt);

	if (tmp_intf_alt->ep_max) {
		struct usb_host_endpoint *ep = NULL;
		struct adsp_udev_ep_info *tmp_ep = NULL;

		if (*extra_max < sizeof(*tmp_ep))
			return -ENOMEM;

		ep = &intf_alt->endpoint[0];
		if (!usb_endpoint_dir_in(&ep->desc) ||
		    !usb_endpoint_xfer_int(&ep->desc))
			return -EINVAL;

		tmp_ep = (struct adsp_udev_ep_info *)(*extra_start);
		tmp_ep->pipe =
			usb_rcvintpipe(udev, ep->desc.bEndpointAddress);
		tmp_ep->max_packet_size = usb_endpoint_maxp(&ep->desc) *
					  usb_endpoint_maxp_mult(&ep->desc);
		tmp_ep->interval = ep->desc.bInterval;
		*extra_start += sizeof(*tmp_ep);
		*extra_max -= sizeof(*tmp_ep);
	}

	return 0;
}

static const struct usb_device_id usb_adsp_hid_ep_quirk_devs[] = {
	/*
	 * Set the valid HID EP number through driver_info.
	 */
	/* SAMSUNG USBC Headset*/
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x04e8,
		.idProduct = 0xa051,
		.driver_info = USB_AUDIO_DEV_QUIRK_USE_HID1,
	},
	{}, /* Terminating entry */
};

static void usb_adsp_detect_hid_ep_quirks(struct usb_device *udev,
	struct audio_udev_info *info)
{
	const struct usb_device_id *match_id = NULL;

	if (info->hid.ep_num > USB_ADSP_HID_EP_MAX) {
		pr_info("detect hid ep number over max size:%d\n",
			info->hid.ep_num);
		return;
	}

	/* Match quirk by device info */
	match_id = usb_device_match_id(udev, usb_adsp_hid_ep_quirk_devs);
	if (match_id) {
		pr_info("Match quirk hid device %x:%x\n",
			match_id->idVendor, match_id->idProduct);

		info->hid.valid_hid = (u8)match_id->driver_info;
	}
}

static int usb_adsp_get_hid_info(struct usb_device *udev,
				 struct usb_interface *intf,
				 struct audio_udev_info *info)
{
	struct usb_host_endpoint *ep = NULL;
	u8 intf_epnum;
	u8 hid_epn;
	u8 i;

	if (intf->condition != USB_INTERFACE_BOUND) {
		pr_err("hid interface has not probed\n");
		return -EINVAL;
	}

	hid_epn = info->hid.ep_num;
	intf_epnum = intf->cur_altsetting->desc.bNumEndpoints;
	info->hid.ep_num += intf_epnum;
	if (!intf->cur_altsetting->endpoint || intf_epnum == 0 ||
	    info->hid.ep_num > USB_ADSP_HID_EP_MAX) {
		pr_err("hid interface has no valid hid. cur_alt epnumber:%u, hid epsum:%u\n",
		       intf->cur_altsetting->desc.bNumEndpoints, info->hid.ep_num);
		return -EINVAL;
	}

	for (i = 0; i < intf_epnum; i++) {
		ep = &intf->cur_altsetting->endpoint[i];
		if (!usb_endpoint_xfer_int(&ep->desc)) {
			pr_err("hid interface has invalid endpoint desc\n");
			return -EINVAL;
		}
		if (usb_endpoint_dir_in(&ep->desc)) {
			if (!info->hid.valid_hid)
				info->hid.valid_hid |= BIT(hid_epn + i);
			info->hid.eps[hid_epn + i].pipe = usb_rcvintpipe(
				udev, ep->desc.bEndpointAddress);
		} else {
			info->hid.eps[hid_epn + i].pipe = usb_sndintpipe(
				udev, ep->desc.bEndpointAddress);
		}
		pr_info("hid_epn:%d, i:%d\n", hid_epn, i);
		info->hid.eps[hid_epn + i].max_packet_size = usb_endpoint_maxp(&ep->desc);
		info->hid.eps[hid_epn + i].interval = ep->desc.bInterval;
	}

	info->flags |= AUDIO_UDEV_FLGA_HAS_HID;

	return 0;
}

static const struct usb_device_id usb_adsp_quirk_devs[] = {
	/* HUAWEI USB-C HEADSET */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x12D1,
		.idProduct = 0x3A07,
		.driver_info = USB_AUDIO_DEV_QUIRK_SR_CHECK_HID_INPUT |
			       USB_AUDIO_DEV_QUIRK_RESUME_LONG,
	},
	/* TX 384KB Hifi Type_C Audio */
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR |
			       USB_DEVICE_ID_MATCH_PRODUCT,
		.idVendor = 0x0bda,
		.idProduct = 0x492f,
		.driver_info = USB_AUDIO_DEV_QUIRK_SR_DISABLE,
	},
	{}, /* Terminating entry */
};

static void usb_adsp_detect_quirks(struct usb_device *udev,
				   struct audio_udev_info *info)
{
	const struct usb_device_id *match_id = NULL;

	/* Match quirk by device info */
	match_id = usb_device_match_id(udev, usb_adsp_quirk_devs);
	if (match_id) {
		pr_info("Match quirk device %x:%x\n",
			match_id->idVendor, match_id->idProduct);
		info->quirks |= (u32)match_id->driver_info;
	}
}

static int usb_adsp_get_udev_info(struct usb_device *udev,
				  struct audio_udev_info *info, u32 extra_max)
{
	int i;
	u8 *extra_start = (u8 *)(info + 1);
	struct usb_host_config *config = udev->actconfig;
	int ret = -ENODEV;

	if (udev->speed == USB_SPEED_HIGH)
		info->flags = AUDIO_UDEV_FLGA_HS;
	else if (udev->speed == USB_SPEED_FULL)
		info->flags = 0;
	else
		return -ENOENT;

	if (!config)
		return -ENODEV;

	if (config->desc.bmAttributes & USB_CONFIG_ATT_WAKEUP)
		info->flags |= AUDIO_UDEV_FLGA_REMOTE_WAKEUP;

	for (i = 0; i < config->desc.bNumInterfaces && i < USB_MAXINTERFACES;
	     ++i) {
		struct usb_interface *intf = config->interface[i];

		if (!intf) {
			ret = -ENODEV;
			break;
		}

		if (usb_match_one_id(intf, &audio_ctrl_usb_id)) {
			ret = usb_adsp_get_audio_ctrl_info(
				udev, intf, &extra_start, &extra_max);
			if (ret)
				break;
			info->audio.ctrl_intf_max++;
		} else if (usb_match_one_id(intf, &audio_data_usb_id)) {
			ret = usb_adsp_get_audio_data_info(
				udev, intf, &extra_start, &extra_max);
			if (ret)
				break;
			info->audio.data_intf_max++;
		} else if (usb_match_one_id(intf, &hid_usb_id)) {
			ret = usb_adsp_get_hid_info(udev, intf, info);
			if (ret)
				break;
		} else {
			if (intf->condition != USB_INTERFACE_UNBOUND &&
				xr_usb_adsp_check_intf_has_eps(intf)) {
				pr_err("not support non standand interface\n");
				ret = -EINVAL;
				break;
			}
		}

		if (info->audio.ctrl_intf_max > USB_ADSP_AUDIO_CTRL_NUM ||
		    info->audio.data_intf_max > USB_ADSP_MAX_AUDIO_DATA_NUM) {
			ret = -EINVAL;
			break;
		}
	}

	if (!ret) {
		usb_adsp_detect_hid_ep_quirks(udev, info);
		usb_adsp_detect_quirks(udev, info);
	}

	/* The audio device does not operate SR when
	 * 1. The device has hid function and does not support remote wakeup attribute.
	 * 2. The device has no hid function but supoorts remote wakeup attribute.
	 */
	if ((info->flags & (AUDIO_UDEV_FLGA_REMOTE_WAKEUP |
		AUDIO_UDEV_FLGA_HAS_HID)) == AUDIO_UDEV_FLGA_REMOTE_WAKEUP ||
		(info->flags & (AUDIO_UDEV_FLGA_REMOTE_WAKEUP |
		AUDIO_UDEV_FLGA_HAS_HID)) == AUDIO_UDEV_FLGA_HAS_HID)
		info->quirks |= USB_AUDIO_DEV_QUIRK_SR_DISABLE;

	pr_info("audio device flag_info:0x%x, quirks_info:0x%x\n",
		info->flags, info->quirks);

	return ret;
}

static int usb_adsp_get_start_info(struct xhci_hcd *xhci,
				   struct usb_device *udev,
				   struct usb_adsp_start_info *info,
				   u32 extra_max)
{
	int ret;

	ret = usb_adsp_get_udev_info(udev, &info->udev_info, extra_max);
	if (ret) {
		pr_err("falied to get udev info - %d\n", ret);
		return ret;
	}

	return usb_adsp_get_xhci_info(xhci, udev, info);
}

/* Must called within usb_adsp->mutex */
int xr_usb_adsp_switch_on(struct xring_usb_adsp *usb_adsp,
			  struct platform_device *xhci)
{
	struct usb_device *udev = usb_adsp->audio_udev;
	struct usb_hcd *primary_hcd = dev_get_drvdata(&xhci->dev);
	struct usb_hcd *shared_hcd = NULL;
	struct xhci_hcd *xhci_hcd = NULL;
	const struct xr_usb_adsp_shared_buf *shared_buf = NULL;
	struct usb_adsp_start_info *info = NULL;
	u32 shared_buf_len;
	int ret;

	if (usb_adsp->xhci)
		return -EBUSY;

	if (!udev || !primary_hcd)
		return -ENODEV;

	shared_hcd = primary_hcd->shared_hcd;
	if (!shared_hcd)
		return -ENODEV;

	xhci_hcd = hcd_to_xhci(primary_hcd);
	if (!xhci_hcd)
		return -ENODEV;

	ret = usb_adsp_check_xhci_status(xhci_hcd);
	if (ret) {
		pr_err("failed to check xhci software status - %d\n", ret);
		return ret;
	}

	shared_buf = xr_usb_adsp_get_shared_buf(usb_adsp, sizeof(*info));
	if (!shared_buf) {
		pr_err("failed to get shared buf\n");
		return -ENOMEM;
	}

	shared_buf_len = shared_buf->len * shared_buf->merge_num;
	memset(shared_buf->virt_addr, 0, shared_buf_len);

	info = (struct usb_adsp_start_info *)shared_buf->virt_addr;
	ret = usb_adsp_get_start_info(xhci_hcd, udev, info,
				      shared_buf_len - sizeof(*info));
	if (ret) {
		pr_err("failed to get start info - %d\n", ret);
		goto put_buf;
	}

	mutex_unlock(&usb_adsp->mutex);
	ret = usb_adsp_send_start_msg_wait_ack(
		usb_adsp->ipc, shared_buf->addr, shared_buf_len,
		USB_ADSP_WAIT_SWITCH_ACK_TIMEOUT);

	/* Send stop message if start error */
	if (ret) {
		pr_err("failed to handle startinfo msg - %d\n", ret);
		ret = usb_adsp_send_stop_msg_wait_ack(
			usb_adsp->ipc, shared_buf->addr, shared_buf_len,
			USB_ADSP_WAIT_SWITCH_ACK_TIMEOUT);
		if (ret)
			pr_err("failed to send stop msg - %d\n", ret);

		ret = -EFAULT;
	}
	mutex_lock(&usb_adsp->mutex);
	if (!ret)
		usb_adsp->xhci = xhci;

put_buf:
	xr_usb_adsp_put_shared_buf(usb_adsp, shared_buf);
	xr_usb_adsp_disable_xhci_irq(xhci_hcd, false);
	return ret;
}

static int xr_usb_adsp_ring_update(struct xhci_ring *ring, u32 cycle_state,
				   dma_addr_t dma_addr)
{
	int i;
	u64 offset;
	struct xhci_segment *seg = ring->first_seg;

	ring->cycle_state = cycle_state;

	for (i = 0; i < ring->num_segs; i++) {
		if (dma_addr >= seg->dma &&
		    dma_addr < seg->dma + TRB_SEGMENT_SIZE) {
			offset = (dma_addr - seg->dma) / sizeof(union xhci_trb);
			ring->enqueue = seg->trbs + offset;
			ring->dequeue = ring->enqueue;
			ring->enq_seg = seg;
			ring->deq_seg = seg;
			return 0;
		}
		seg = seg->next;
	}

	return -EINVAL;
}

static int xr_usb_adsp_update_cmd_ring(struct xhci_hcd *xhci_hcd, u32 ring_que)
{
	u32 cycle_state = ring_que & XHCI_CYCLE_STATE_MASK;
	dma_addr_t ring_que_dma = ring_que & ~XHCI_CYCLE_STATE_MASK;

	if (!xhci_hcd->cmd_ring)
		return -ENOENT;

	return xr_usb_adsp_ring_update(xhci_hcd->cmd_ring, cycle_state,
				       ring_que_dma);
}

static int xr_usb_adsp_update_evt_ring(struct xhci_hcd *xhci_hcd, u32 ring_que)
{
	u32 cycle_state = ring_que & XHCI_CYCLE_STATE_MASK;
	dma_addr_t ring_que_dma = ring_que & ~XHCI_CYCLE_STATE_MASK;
	struct xhci_ring *evt_ring = NULL;

	evt_ring = usb_adsp_get_xhci_evt_ring(xhci_hcd);
	if (!evt_ring)
		return -EINVAL;

	return xr_usb_adsp_ring_update(evt_ring, cycle_state, ring_que_dma);
}

static int xr_usb_adsp_update_ep0_ring(struct xhci_hcd *xhci_hcd,
				       struct xhci_virt_device *dev,
				       u32 ring_que)
{
	struct xhci_virt_ep *xhci_ep = NULL;
	u32 cycle_state = ring_que & XHCI_CYCLE_STATE_MASK;
	dma_addr_t ring_que_dma = ring_que & ~XHCI_CYCLE_STATE_MASK;

	xhci_ep = &dev->eps[0];
	if (!xhci_ep->ring) {
		pr_warn("ep0 has no ring\n");
		return -ENOENT;
	}

	return xr_usb_adsp_ring_update(xhci_ep->ring, cycle_state,
				       ring_que_dma);
}

static int xr_usb_adsp_update_hid_ring(struct xhci_hcd *xhci_hcd,
				       struct xhci_virt_device *dev,
				       u32 ring_que, u32 ep_index)
{
	struct xhci_virt_ep *xhci_ep = NULL;
	u32 cycle_state = ring_que & XHCI_CYCLE_STATE_MASK;
	dma_addr_t ring_que_dma = ring_que & ~XHCI_CYCLE_STATE_MASK;

	pr_debug("hid info: %u 0x%x\n", ep_index, ring_que);
	if (ep_index < 1 || ep_index >= EP_CTX_PER_DEV)
		return -EINVAL;

	xhci_ep = &dev->eps[ep_index];
	if (!xhci_ep->ring) {
		pr_warn("ep%u has no ring\n", ep_index);
		return -ENOENT;
	}

	return xr_usb_adsp_ring_update(xhci_ep->ring, cycle_state,
				       ring_que_dma);
}

static int xr_usb_adsp_update_xhci(struct xring_usb_adsp *usb_adsp,
				   const struct usb_adsp_stop_info *stopinfo)
{
	int ret;
	struct usb_hcd *primary_hcd = dev_get_drvdata(&usb_adsp->xhci->dev);
	struct xhci_hcd *xhci_hcd = NULL;
	struct xhci_virt_device *dev = NULL;

	if (!primary_hcd)
		return -ENODEV;

	xhci_hcd = hcd_to_xhci(primary_hcd);
	if (!xhci_hcd)
		return -ENODEV;

	ret = xr_usb_adsp_update_cmd_ring(xhci_hcd, stopinfo->cmd_ring_queue);
	if (ret) {
		pr_err("falied to update commnad ring\n");
		return ret;
	}

	ret = xr_usb_adsp_update_evt_ring(xhci_hcd, stopinfo->event_ring_queue);
	if (ret) {
		pr_err("falied to update event ring\n");
		return ret;
	}

	if (!usb_adsp->audio_udev) {
		pr_err("stop while no udev?\n");
		return -ENOENT;
	}

	if (usb_adsp->audio_udev->slot_id <= 0 ||
	    usb_adsp->audio_udev->slot_id >= MAX_HC_SLOTS) {
		pr_err("invalid slot id %d\n", usb_adsp->audio_udev->slot_id);
		return -EINVAL;
	}

	if (!xhci_hcd->devs[usb_adsp->audio_udev->slot_id]) {
		pr_err("virt dev is null for slot id %d\n",
		       usb_adsp->audio_udev->slot_id);
		return -ENODEV;
	}

	dev = xhci_hcd->devs[usb_adsp->audio_udev->slot_id];

	ret = xr_usb_adsp_update_ep0_ring(xhci_hcd, dev,
					  stopinfo->ep0_ring_queue);
	if (ret) {
		pr_err("falied to update ep0 ring\n");
		return ret;
	}

	ret = xr_usb_adsp_update_hid_ring(xhci_hcd, dev,
					  stopinfo->hid_ring_queue,
					  stopinfo->hid_ep_index);
	if (ret) {
		pr_err("falied to update hid ep ring\n");
		return ret;
	}

	xr_usb_adsp_disable_xhci_irq(xhci_hcd, false);
	return 0;
}

/* Must called within usb_adsp->mutex */
int xr_usb_adsp_switch_off(struct xring_usb_adsp *usb_adsp)
{
	int ret;
	const struct xr_usb_adsp_shared_buf *shared_buf = NULL;
	const struct usb_adsp_stop_info *stopinfo = NULL;
	u32 shared_buf_len;

	if (!usb_adsp->xhci) {
		pr_err("has no valid xhci device\n");
		return 0;
	}

	shared_buf = xr_usb_adsp_get_shared_buf(usb_adsp, sizeof(*stopinfo));
	if (!shared_buf)
		return -ENOMEM;

	shared_buf_len = shared_buf->len * shared_buf->merge_num;
	memset(shared_buf->virt_addr, 0, shared_buf_len);
	mutex_unlock(&usb_adsp->mutex);

	ret = usb_adsp_send_stop_msg_wait_ack(usb_adsp->ipc, shared_buf->addr,
					      shared_buf_len,
					      USB_ADSP_WAIT_SWITCH_ACK_TIMEOUT);
	mutex_lock(&usb_adsp->mutex);
	if (ret) {
		pr_err("failed to handle stop msg - %d\n", ret);
		ret = -EFAULT;
		goto out;
	}

	stopinfo = (const struct usb_adsp_stop_info *)shared_buf->virt_addr;
	ret = xr_usb_adsp_update_xhci(usb_adsp, stopinfo);
	if (ret)
		pr_err("failed to update xhci - %d\n", ret);

out:
	usb_adsp->xhci = NULL;
	xr_usb_adsp_put_shared_buf(usb_adsp, shared_buf);
	return ret;
}
