// SPDX-License-Identifier: GPL-2.0
/*
 * xhci-helper.c - xHCI helper functions for Proxy HCD Driver.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "core.h"
#include "xhci-helper.h"

#include <linux/dma-mapping.h>
#include <linux/completion.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
dma_addr_t xhci_trb_virt_to_dma(struct xhci_segment *seg, union xhci_trb *trb)
{
	unsigned long segment_offset;

	if (!seg || !trb || trb < seg->trbs)
		return 0;
	/* offset in TRBs */
	segment_offset = trb - seg->trbs;
	if (segment_offset >= TRBS_PER_SEGMENT)
		return 0;
	return seg->dma + (segment_offset * sizeof(*trb));
}
#endif

struct xhci_virt_device *usb_adsp_get_xhci_dev(struct xhci_hcd *xhci,
					       int slot_id)
{
	if (slot_id <= 0 || slot_id >= MAX_HC_SLOTS) {
		pr_err("invalid slot id %d\n", slot_id);
		return ERR_PTR(-EINVAL);
	}

	if (!xhci->devs[slot_id]) {
		pr_err("virt dev is null for slot id %d\n", slot_id);
		return ERR_PTR(-ENODEV);
	}

	return xhci->devs[slot_id];
}

void xhci_inc_deq(struct xhci_hcd *xhci, struct xhci_ring *ring)
{
	unsigned int link_trb_count = 0;

	/* event ring doesn't have link trbs, check for last trb */
	if (ring->type == TYPE_EVENT) {
		if (!xhci_last_trb_on_seg(ring->deq_seg, ring->dequeue)) {
			ring->dequeue++;
			return;
		}
		if (xhci_last_trb_on_ring(ring, ring->deq_seg, ring->dequeue))
			ring->cycle_state ^= 1;
		ring->deq_seg = ring->deq_seg->next;
		ring->dequeue = ring->deq_seg->trbs;
		return;
	}

	/* All other rings have link trbs */
	if (!xhci_trb_is_link(ring->dequeue)) {
		if (xhci_last_trb_on_seg(ring->deq_seg, ring->dequeue)) {
			pr_warn("Missing link TRB at end of segment\n");
		} else {
			ring->dequeue++;
			ring->num_trbs_free++;
		}
	}

	while (xhci_trb_is_link(ring->dequeue)) {
		ring->deq_seg = ring->deq_seg->next;
		ring->dequeue = ring->deq_seg->trbs;

		if (link_trb_count++ > ring->num_segs) {
			pr_warn("Ring is an endless link TRB loop\n");
			break;
		}
	}
}

struct xhci_segment *xhci_trb_in_td(struct xhci_hcd *xhci,
				    struct xhci_segment *start_seg,
				    union xhci_trb *start_trb,
				    union xhci_trb *end_trb,
				    dma_addr_t suspect_dma)
{
	dma_addr_t start_dma;
	dma_addr_t end_seg_dma;
	dma_addr_t end_trb_dma;
	struct xhci_segment *cur_seg;

	start_dma = xhci_trb_virt_to_dma(start_seg, start_trb);
	cur_seg = start_seg;

	do {
		if (start_dma == 0)
			return NULL;
		/* We may get an event for a Link TRB in the middle of a TD */
		end_seg_dma = xhci_trb_virt_to_dma(
			cur_seg, &cur_seg->trbs[TRBS_PER_SEGMENT - 1]);
		/* If the end TRB isn't in this segment, this is set to 0 */
		end_trb_dma = xhci_trb_virt_to_dma(cur_seg, end_trb);

		if (end_trb_dma > 0) {
			/* The end TRB is in this segment, so suspect should be here */
			if (start_dma <= end_trb_dma) {
				if (suspect_dma >= start_dma &&
				    suspect_dma <= end_trb_dma)
					return cur_seg;
			} else {
				/* Case for one segment with
				 * a TD wrapped around to the top
				 */
				if ((suspect_dma >= start_dma &&
				     suspect_dma <= end_seg_dma) ||
				    (suspect_dma >= cur_seg->dma &&
				     suspect_dma <= end_trb_dma))
					return cur_seg;
			}
			return NULL;
		}

		/* Might still be somewhere in this segment */
		if (suspect_dma >= start_dma &&
			suspect_dma <= end_seg_dma)
			return cur_seg;

		cur_seg = cur_seg->next;
		start_dma = xhci_trb_virt_to_dma(cur_seg, &cur_seg->trbs[0]);
	} while (cur_seg != start_seg);

	return NULL;
}

static void xhci_unmap_td_bounce_buffer(struct xhci_hcd *xhci,
					struct xhci_ring *ring,
					struct xhci_td *td)
{
	struct device *dev = xhci_to_hcd(xhci)->self.controller;
	struct xhci_segment *seg = td->bounce_seg;
	struct urb *urb = td->urb;
	size_t len;

	if (!ring || !seg || !urb)
		return;

	if (usb_urb_dir_out(urb)) {
		dma_unmap_single(dev, seg->bounce_dma, ring->bounce_buf_len,
				 DMA_TO_DEVICE);
		return;
	}

	dma_unmap_single(dev, seg->bounce_dma, ring->bounce_buf_len,
			 DMA_FROM_DEVICE);
	/* for in tranfers we need to copy the data from bounce to sg */
	if (urb->num_sgs) {
		len = sg_pcopy_from_buffer(urb->sg, urb->num_sgs,
					   seg->bounce_buf, seg->bounce_len,
					   seg->bounce_offs);
		if (len != seg->bounce_len)
			pr_warn("WARN Wrong bounce buffer read length: %zu != %d\n",
				len, seg->bounce_len);
	} else {
		memcpy(urb->transfer_buffer + seg->bounce_offs, seg->bounce_buf,
		       seg->bounce_len);
	}
	seg->bounce_len = 0;
	seg->bounce_offs = 0;
}

static void xhci_giveback_urb_in_irq(struct xhci_hcd *xhci,
				     struct xhci_td *cur_td, int status)
{
	struct urb *urb = cur_td->urb;
	struct urb_priv *urb_priv = urb->hcpriv;
	struct usb_hcd *hcd = bus_to_hcd(urb->dev->bus);

	kfree(urb_priv);
	usb_hcd_unlink_urb_from_ep(hcd, urb);
	usb_hcd_giveback_urb(hcd, urb, status);
}

int xhci_td_cleanup(struct xhci_hcd *xhci, struct xhci_td *td,
		    struct xhci_ring *ep_ring, int status)
{
	struct urb *urb = NULL;

	/* Clean up the endpoint's TD list */
	urb = td->urb;

	/* if a bounce buffer was used to align this td then unmap it */
	xhci_unmap_td_bounce_buffer(xhci, ep_ring, td);

	/* Do one last check of the actual transfer length.
	 * If the host controller said we transferred more data than the buffer
	 * length, urb->actual_length will be a very big number (since it's
	 * unsigned).  Play it safe and say we didn't transfer anything.
	 */
	if (urb->actual_length > urb->transfer_buffer_length) {
		pr_warn("URB req %u and actual %u transfer length mismatch\n",
			urb->transfer_buffer_length, urb->actual_length);
		urb->actual_length = 0;
		status = 0;
	}
	/* TD might be removed from td_list if we are giving back a cancelled URB */
	if (!list_empty(&td->td_list))
		list_del_init(&td->td_list);
	/* Giving back a cancelled URB, or if a slated TD completed anyway */
	if (!list_empty(&td->cancelled_td_list))
		list_del_init(&td->cancelled_td_list);

	xhci_inc_td_cnt(urb);
	/* Giveback the urb when all the tds are completed */
	if (xhci_last_td_in_urb(td)) {
		if ((urb->actual_length != urb->transfer_buffer_length &&
		     (urb->transfer_flags & URB_SHORT_NOT_OK)) ||
		    (status != 0 && !usb_endpoint_xfer_isoc(&urb->ep->desc)))
			pr_warn(
				"Giveback URB %pK, len = %d, expected = %d, status = %d\n",
				urb, urb->actual_length,
				urb->transfer_buffer_length, status);

		/* set isoc urb status to 0 just as EHCI, UHCI, and OHCI */
		if (usb_pipetype(urb->pipe) == PIPE_ISOCHRONOUS)
			status = 0;
		xhci_giveback_urb_in_irq(xhci, td, status);
	}

	return 0;
}

void xhci_quiesce(struct xhci_hcd *xhci)
{
	u32 halted;
	u32 cmd;
	u32 mask;

	pr_err("%s\n", __func__);

	mask = ~(XHCI_IRQS);
	halted = readl(&xhci->op_regs->status) & STS_HALT;
	if (!halted)
		mask &= ~CMD_RUN;

	cmd = readl(&xhci->op_regs->command);
	cmd &= mask;
	writel(cmd, &xhci->op_regs->command);
}

static struct xhci_virt_ep *xhci_get_virt_ep(struct xhci_hcd *xhci,
					     unsigned int slot_id,
					     unsigned int ep_index)
{
	if (slot_id == 0 || slot_id >= MAX_HC_SLOTS) {
		pr_warn("Invalid slot_id %u\n", slot_id);
		return NULL;
	}
	if (ep_index >= EP_CTX_PER_DEV) {
		pr_warn("Invalid endpoint index %u\n", ep_index);
		return NULL;
	}
	if (!xhci->devs[slot_id]) {
		pr_warn("No xhci virt device for slot_id %u\n", slot_id);
		return NULL;
	}

	return &xhci->devs[slot_id]->eps[ep_index];
}

static void xhci_kill_ring_urbs(struct xhci_hcd *xhci, struct xhci_ring *ring)
{
	struct xhci_td *cur_td;
	struct xhci_td *tmp;

	list_for_each_entry_safe(cur_td, tmp, &ring->td_list, td_list) {
		list_del_init(&cur_td->td_list);

		if (!list_empty(&cur_td->cancelled_td_list))
			list_del_init(&cur_td->cancelled_td_list);

		xhci_unmap_td_bounce_buffer(xhci, ring, cur_td);

		xhci_inc_td_cnt(cur_td->urb);
		if (xhci_last_td_in_urb(cur_td))
			xhci_giveback_urb_in_irq(xhci, cur_td, -ESHUTDOWN);
	}
}

static void xhci_kill_endpoint_urbs(struct xhci_hcd *xhci,
		int slot_id, int ep_index)
{
	struct xhci_td *cur_td;
	struct xhci_td *tmp;
	struct xhci_virt_ep *ep;
	struct xhci_ring *ring;

	ep = xhci_get_virt_ep(xhci, slot_id, ep_index);
	if (!ep)
		return;

	if ((ep->ep_state & EP_HAS_STREAMS) ||
			(ep->ep_state & EP_GETTING_NO_STREAMS)) {
		int stream_id;

		for (stream_id = 1; stream_id < ep->stream_info->num_streams;
				stream_id++) {
			ring = ep->stream_info->stream_rings[stream_id];
			if (!ring)
				continue;

			xhci_kill_ring_urbs(xhci, ring);
		}
	} else {
		ring = ep->ring;
		if (!ring)
			return;
		xhci_kill_ring_urbs(xhci, ring);
	}

	list_for_each_entry_safe(cur_td, tmp, &ep->cancelled_td_list,
			cancelled_td_list) {
		list_del_init(&cur_td->cancelled_td_list);
		xhci_inc_td_cnt(cur_td->urb);

		if (xhci_last_td_in_urb(cur_td))
			xhci_giveback_urb_in_irq(xhci, cur_td, -ESHUTDOWN);
	}
}

static void xhci_complete_del_and_free_cmd(struct xhci_command *cmd, u32 status)
{
	list_del(&cmd->cmd_list);

	if (cmd->completion) {
		cmd->status = status;
		complete(cmd->completion);
	} else {
		kfree(cmd);
	}
}

static void adsp_xhci_cleanup_command_queue(struct xhci_hcd *xhci)
{
	struct xhci_command *cur_cmd, *tmp_cmd;

	xhci->current_cmd = NULL;
	list_for_each_entry_safe(cur_cmd, tmp_cmd, &xhci->cmd_list, cmd_list)
		xhci_complete_del_and_free_cmd(cur_cmd, COMP_COMMAND_ABORTED);
}

/*
 * host controller died, register read returns 0xffffffff
 * Complete pending commands, mark them ABORTED.
 * URBs need to be given back as usb core might be waiting with device locks
 * held for the URBs to finish during device disconnect, blocking host remove.
 *
 * Call with xhci->lock held.
 * lock is relased and re-acquired while giving back urb.
 */
void usb_adsp_xhci_hc_died(struct xhci_hcd *xhci)
{
	int i, j;

	if (xhci->xhc_state & XHCI_STATE_DYING)
		return;

	pr_err("xHCI host controller not responding, assume dead\n");
	xhci->xhc_state |= XHCI_STATE_DYING;

	adsp_xhci_cleanup_command_queue(xhci);

	/* return any pending urbs, remove may be waiting for them */
	for (i = 0; i <= HCS_MAX_SLOTS(xhci->hcs_params1); i++) {
		if (!xhci->devs[i])
			continue;
		for (j = 0; j < 31; j++)
			xhci_kill_endpoint_urbs(xhci, i, j);
	}

	/* inform usb core hc died if PCI remove isn't already handling it */
	if (!(xhci->xhc_state & XHCI_STATE_REMOVING))
		usb_hc_died(xhci_to_hcd(xhci));
}

bool usb_adsp_xhci_port_connect(struct platform_device *pdev)
{
	unsigned long flags;
	u32 portsc;
	unsigned int max_ports;
	struct usb_hcd *primary_hcd = dev_get_drvdata(&pdev->dev);
	struct usb_hcd *shared_hcd = NULL;
	struct xhci_hcd *xhci = NULL;
	struct xhci_port **ports = NULL;
	struct xhci_port *port = NULL;
	bool ret = true;

	if (!primary_hcd)
		return false;

	shared_hcd = primary_hcd->shared_hcd;
	if (!shared_hcd)
		return false;

	xhci = hcd_to_xhci(primary_hcd);
	if (!xhci)
		return false;

	spin_lock_irqsave(&xhci->lock, flags);
	ports = xhci->usb2_rhub.ports;
	max_ports = xhci->usb2_rhub.num_ports;
	if (max_ports == 0) {
		pr_err("no ports?\n");
		ret = false;
		goto out;
	}

	port = ports[0];
	portsc = readl(port->addr);
	if ((portsc & (PORT_CONNECT | PORT_PE)) != (PORT_CONNECT | PORT_PE)) {
		pr_err("device not enabled\n");
		ret = false;
		goto out;
	}

	if (portsc & (PORT_CSC | PORT_PEC)) {
		pr_err("status changed\n");
		ret = false;
	}

out:
	spin_unlock_irqrestore(&xhci->lock, flags);
	return ret;
}
