/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * xr-usb-plat.h - Platform driver for XRing USB Module
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XR_USB_PLAT_H
#define _XR_USB_PLAT_H

#include <linux/errno.h>
#include <linux/phy/phy.h>
#include <linux/seq_file.h>
#include <linux/types.h>

struct xring_usb;

enum usb_intr_target {
	USB_INTR_AP = 0,
	USB_INTR_ADSP,
};

struct xring_usb_plat {
	unsigned int priv_size;
	int (*plat_probe)(struct xring_usb *xr_usb);
	void (*plat_remove)(struct xring_usb *xr_usb);
	void (*set_quirks)(struct xring_usb *xr_usb);
	int (*sys_init)(struct xring_usb *xr_usb);
	void (*sys_exit)(struct xring_usb *xr_usb);
	void (*mask_irq)(struct xring_usb *xr_usb, enum usb_intr_target tgt, bool on);
	int (*exit_bus_idle)(struct xring_usb *xr_usb, bool set);
	int (*powerup)(struct xring_usb *xr_usb);
	void (*powerdown)(struct xring_usb *xr_usb);
	int (*plat_suspend)(struct xring_usb *xr_usb);
	int (*plat_resume)(struct xring_usb *xr_usb);
	int (*enter_standby)(struct xring_usb *xr_usb);
	int (*exit_standby)(struct xring_usb *xr_usb);
	void (*usb3_enter_standby)(struct xring_usb *xr_usb);
	void (*usb3_exit_standby)(struct xring_usb *xr_usb);
};

static inline int xr_usb_plat_probe(const struct xring_usb_plat *plat,
				    struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->plat_probe)
		return -EINVAL;

	return plat->plat_probe(xr_usb);
}

static inline void xr_usb_plat_remove(const struct xring_usb_plat *plat,
				      struct xring_usb *xr_usb)
{
	if (!plat)
		return;
	if (!plat->plat_remove)
		return;

	plat->plat_remove(xr_usb);
}

static inline void xr_usb_plat_set_quirks(const struct xring_usb_plat *plat,
					  struct xring_usb *xr_usb)
{
	if (!plat)
		return;
	if (!plat->set_quirks)
		return;

	plat->set_quirks(xr_usb);
}

static inline int xr_usb_plat_sys_init(const struct xring_usb_plat *plat,
				       struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->sys_init)
		return -EINVAL;

	return plat->sys_init(xr_usb);
}

static inline void xr_usb_plat_sys_exit(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return;
	if (!plat->sys_exit)
		return;

	plat->sys_exit(xr_usb);
}

static inline void xr_usb_plat_mask_irq(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb,
					enum usb_intr_target tgt,
					bool on)
{
	if (!plat)
		return;
	if (!plat->mask_irq)
		return;

	plat->mask_irq(xr_usb, tgt, on);
}

static inline int xr_usb_plat_exit_bus_idle(const struct xring_usb_plat *plat,
			struct xring_usb *xr_usb, bool set)
{
	if (!plat)
		return -EINVAL;
	if (!plat->exit_bus_idle)
		return -EINVAL;

	return plat->exit_bus_idle(xr_usb, set);
}

static inline int xr_usb_plat_powerup(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->powerup)
		return 0;

	return plat->powerup(xr_usb);
}

static inline void xr_usb_plat_powerdown(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return;
	if (!plat->powerdown)
		return;

	plat->powerdown(xr_usb);
}

static inline int xr_usb_plat_suspend(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->plat_suspend)
		return 0;

	return plat->plat_suspend(xr_usb);
}

static inline int xr_usb_plat_resume(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->plat_resume)
		return 0;

	return plat->plat_resume(xr_usb);
}

static inline int xr_usb_plat_enter_standby(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->enter_standby)
		return 0;

	return plat->enter_standby(xr_usb);
}

static inline int xr_usb_plat_exit_standby(const struct xring_usb_plat *plat,
					struct xring_usb *xr_usb)
{
	if (!plat)
		return -EINVAL;
	if (!plat->exit_standby)
		return 0;

	return plat->exit_standby(xr_usb);
}

static inline void xr_usb_usb3_enter_standby(const struct xring_usb_plat *plat,
					     struct xring_usb *xr_usb)
{
	if (!plat)
		return;
	if (!plat->usb3_enter_standby)
		return;

	plat->usb3_enter_standby(xr_usb);
}

static inline void xr_usb_usb3_exit_standby(const struct xring_usb_plat *plat,
					    struct xring_usb *xr_usb)
{
	if (!plat)
		return;
	if (!plat->usb3_exit_standby)
		return;

	plat->usb3_exit_standby(xr_usb);
}

#if IS_ENABLED(CONFIG_XRING_USB_O1)
extern struct xring_usb_plat g_xr_usb_o1_plat;
#define SET_XR_USB_O1_DATA() .data = &g_xr_usb_o1_plat,
#else
#define SET_XR_USB_O1_DATA()
#endif

#endif /* _XR_USB_PLAT_H */
