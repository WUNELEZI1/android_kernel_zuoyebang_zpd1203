/* SPDX-License-Identifier: GPL-2.0 */
/*
 * xhci-helper.h - xHCI helper functions for Proxy HCD Driver.
 *
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_ADSP_XHCI_HELPER_H
#define _XR_USB_ADSP_XHCI_HELPER_H

#include <host/xhci.h>
#include <linux/version.h>

static inline bool xhci_trb_is_link(union xhci_trb *trb)
{
	return TRB_TYPE_LINK_LE32(trb->link.control);
}

static inline bool xhci_last_trb_on_seg(struct xhci_segment *seg,
					union xhci_trb *trb)
{
	return trb == &seg->trbs[TRBS_PER_SEGMENT - 1];
}

static inline bool xhci_last_trb_on_ring(struct xhci_ring *ring,
					 struct xhci_segment *seg,
					 union xhci_trb *trb)
{
	return xhci_last_trb_on_seg(seg, trb) && (seg->next == ring->first_seg);
}

static inline void xhci_inc_td_cnt(struct urb *urb)
{
	struct urb_priv *urb_priv = urb->hcpriv;

	urb_priv->num_tds_done++;
}

static inline bool xhci_last_td_in_urb(struct xhci_td *td)
{
	struct urb_priv *urb_priv = td->urb->hcpriv;

	return urb_priv->num_tds_done == urb_priv->num_tds;
}

static inline struct xhci_ring *
usb_adsp_get_xhci_evt_ring(struct xhci_hcd *xhci)
{
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	return xhci->event_ring;
#else
	if (xhci->max_interrupters == 0 || !xhci->interrupters)
		return NULL;

	if (!xhci->interrupters[0])
		return NULL;

	return xhci->interrupters[0]->event_ring;
#endif
}

static inline struct xhci_intr_reg __iomem *
usb_adsp_get_xhci_ir_set(struct xhci_hcd *xhci)
{
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	return xhci->ir_set;
#else
	if (xhci->max_interrupters == 0 || !xhci->interrupters)
		return NULL;

	if (!xhci->interrupters[0])
		return NULL;

	return xhci->interrupters[0]->ir_set;
#endif
}

struct xhci_virt_device *usb_adsp_get_xhci_dev(struct xhci_hcd *xhci,
					       int slot_id);
void xhci_inc_deq(struct xhci_hcd *xhci, struct xhci_ring *ring);
struct xhci_segment *xhci_trb_in_td(struct xhci_hcd *xhci,
				    struct xhci_segment *start_seg,
				    union xhci_trb *start_trb,
				    union xhci_trb *end_trb,
				    dma_addr_t suspect_dma);
int xhci_td_cleanup(struct xhci_hcd *xhci, struct xhci_td *td,
		    struct xhci_ring *ep_ring, int status);
void usb_adsp_xhci_hc_died(struct xhci_hcd *xhci);
void xhci_quiesce(struct xhci_hcd *xhci);
bool usb_adsp_xhci_port_connect(struct platform_device *pdev);

#endif /* _XR_USB_ADSP_XHCI_HELPER_H */
