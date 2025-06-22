/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * u_ether.h -- interface to USB gadget "ethernet link" utilities
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 */

#ifndef __U_ETHER_OVERRIDE_H
#define __U_ETHER_OVERRIDE_H

#include <linux/err.h>
#include <linux/if_ether.h>
#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>
#include <linux/netdevice.h>

#include "u_ether.h"

void xr_gether_disconnect(struct gether *);

#endif /* __U_ETHER_OVERRIDE_H */
