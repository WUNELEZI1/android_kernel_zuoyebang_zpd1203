// SPDX-License-Identifier: GPL-2.0+
/*
 * u_ether_override.c -- Ethernet-over-USB link layer utilities for Gadget stack
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 */

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/string_helpers.h>
#include <linux/usb/composite.h>

#include "u_ether_override.h"


struct eth_dev {
	/* lock is held while accessing port_usb
	 */
	spinlock_t		lock;
	struct gether		*port_usb;

	struct net_device	*net;
	struct usb_gadget	*gadget;

	spinlock_t		req_lock;	/* guard {rx,tx}_reqs */
	struct list_head	tx_reqs, rx_reqs;
	atomic_t		tx_qlen;

	struct sk_buff_head	rx_frames;

	unsigned		qmult;

	unsigned		header_len;
	struct sk_buff		*(*wrap)(struct gether *port, struct sk_buff *skb);
	int			(*unwrap)(struct gether *port,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	struct work_struct	work;

	unsigned long		todo;
#define	WORK_RX_MEMORY		0

	bool			zlp;
	bool			no_skb_reserve;
	bool			ifname_set;
	u8			host_mac[ETH_ALEN];
	u8			dev_mac[ETH_ALEN];
};

/**
 * xr_gether_disconnect - override gether_disconnect()
 *
 * This is called to deactivate endpoints and let the network layer know
 * the connection went inactive ("no carrier").
 *
 * On return, the state is as if gether_connect() had never been called.
 * The endpoints are inactive, and accordingly without active USB I/O.
 * Pointers to endpoint descriptors and endpoint private data are nulled.
 */
void xr_gether_disconnect(struct gether *link)
{
	struct eth_dev		*dev = link->ioport;
	struct usb_request	*req;

	WARN_ON(!dev);
	if (!dev)
		return;

	pr_info("%s\n", __func__);

	spin_lock(&dev->lock);
	dev->port_usb = NULL;
	link->is_suspend = false;
	spin_unlock(&dev->lock);

	netif_stop_queue(dev->net);
	netif_carrier_off(dev->net);

	/* disable endpoints, forcing (synchronous) completion
	 * of all pending i/o.  then free the request objects
	 * and forget about the endpoints.
	 */
	usb_ep_disable(link->in_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->tx_reqs)) {
		req = list_first_entry(&dev->tx_reqs, struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
		usb_ep_free_request(link->in_ep, req);
		spin_lock(&dev->req_lock);
	}
	spin_unlock(&dev->req_lock);
	link->in_ep->desc = NULL;

	usb_ep_disable(link->out_ep);
	spin_lock(&dev->req_lock);
	while (!list_empty(&dev->rx_reqs)) {
		req = list_first_entry(&dev->rx_reqs, struct usb_request, list);
		list_del(&req->list);

		spin_unlock(&dev->req_lock);
		usb_ep_free_request(link->out_ep, req);
		spin_lock(&dev->req_lock);
	}
	spin_unlock(&dev->req_lock);
	link->out_ep->desc = NULL;

	/* finish forgetting about this USB link episode */
	dev->header_len = 0;
	dev->unwrap = NULL;
	dev->wrap = NULL;
}
