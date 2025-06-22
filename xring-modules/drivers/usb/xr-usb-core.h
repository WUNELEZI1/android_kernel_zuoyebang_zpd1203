/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#include <soc/xring/xr_usbdp_event.h>
#include <dt-bindings/xring/xr-usb-combophy.h>
#include <dt-bindings/xring/xr-usb-hsphy.h>

#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/kfifo.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/role.h>
#include <linux/workqueue.h>

#ifndef _XR_USB_CORE_H
#define _XR_USB_CORE_H

#undef pr_fmt
#undef dev_fmt
#define pr_fmt(fmt) "[%s]: %s:%d: " fmt, KBUILD_MODNAME, __func__, __LINE__
#define dev_fmt(fmt) ": %s:%d: " fmt, __func__, __LINE__

struct gadget_info;
struct phy;
struct xring_usb_plat;
struct xring_usb_adsp;
struct usb_redriver;
struct extcon_dev;

enum xr_usb_state {
	USB_STATE_UNDEFINED = 0,
	USB_STATE_POWERDOWN,
	USB_STATE_IDLE,
	USB_STATE_PERIPHERAL,
	USB_STATE_HOST,
	USB_STATE_ADSP_HOST,
	USB_STATE_DYNAMIC_DIS,
	USB_STATE_MAX,
};

enum xr_usb_dynamic_mode {
	DYNAMIC_DISABLED = 0,
	DYNAMIC_ENABLED,
};

struct typec_usb_info {
	unsigned int orig : 1;
	unsigned int usbdp_mode : 2;
	unsigned int hpd : 2;
};

enum xr_usb_lpstate {
	LP_NORMAL = 0,
	LP_POWERDOWN,
	LP_P4PG,
};

struct xring_usb {
	struct device *dev;
	struct clk_bulk_data *clks;
	int num_clks;
	struct reset_control *sys_reset;
	struct reset_control *vbus_valid;
	struct reset_control *controller_reset;
	struct xr_usb_hsphy *usb2_phy;
	struct xr_usb_combophy *usb3_phy;
	unsigned int usbtrdtim;
	struct usb_redriver *redriver;
	struct extcon_dev *dwc3_edev;

	spinlock_t lock;

	/* properties */
	unsigned int is_fpga : 1;
	unsigned int use_role_switch : 1;
	unsigned int u2phy_d_eye_set : 1;
	unsigned int u2phy_h_eye_set : 1;
	unsigned int u3phy_d_eye_set : 1;
	unsigned int u3phy_h_eye_set : 1;
	unsigned int is_standby : 1;
	unsigned int support_usb_adsp : 1;
	unsigned int has_usb_redriver : 1;
	unsigned int device_u2eye[HSPHY_EYE_PARAM_MAX];
	unsigned int host_u2eye[HSPHY_EYE_PARAM_MAX];
	unsigned int device_u3eye[COMBOPHY_EYE_PARAM_MAX];
	unsigned int host_u3eye[COMBOPHY_EYE_PARAM_MAX];

	/* status */
	unsigned int removing : 1;
	unsigned int suspending : 1;
	unsigned int restore : 1;
	unsigned int changing_ops : 1;
	unsigned int dev_link_connect: 1;

	/* dp operation */
	const struct xr_dp_ops *dp_ops;

	struct notifier_block pm_notify;

	const struct xring_usb_plat *plat;
	struct platform_device *dwc3;
	/* gadget operations */
	const struct bus_type *gadget_bus;
	struct notifier_block gadget_bus_nb;
	struct usb_gadget_ops gadget_ops;
	struct usb_gadget_driver gadget_driver;
	const struct usb_gadget_ops *dwc3_ops;
	struct gadget_info *gi;

	const struct xr_usb_adsp_ops *usb_adsp_ops;

	enum xr_usb_state usb_state;
	enum xr_usb_lpstate lpstate;
	struct usb_event_info current_evt;
	struct typec_usb_info dbg_set_evt;
	struct delayed_work state_work;
#define INPUT_EVENT_FIFO_SIZE 8
	DECLARE_KFIFO(input_event_fifo, union xr_usbdp_event,
				INPUT_EVENT_FIFO_SIZE);

	enum usb_role force_role;
	atomic_t dynamic_mode;
	enum usb_device_speed otg_speed;
	struct notifier_block udev_nb;
	struct dentry *debugfs_root;

	const struct dev_pm_ops *dwc3_pm_ops_origin;
	const struct dev_pm_ops *xhci_pm_ops_origin;

	/* The platform private data is stored at the end of this structure. */
	unsigned long plat_priv[] __aligned(sizeof(s64));
};

const char *xr_usb_state_to_name(enum xr_usb_state state);

static inline int xr_usb_dp_poweron(struct xring_usb *xr_usb, bool on,
		unsigned int orientation, unsigned int usbdp_mode)
{
	if (!xr_usb)
		return -ENODEV;

	if (!xr_usb->dp_ops)
		return 0;

	if (xr_usb->dp_ops->manage_power)
		return xr_usb->dp_ops->manage_power(on, orientation, usbdp_mode);

	dev_warn(xr_usb->dev, "dp not init power on intf\n");
	return -EINVAL;
}

static inline int xr_hpd_evt_notifier(struct xring_usb *xr_usb,
		unsigned int dp_hpd)
{
	if (!xr_usb)
		return -ENODEV;

	if (!xr_usb->dp_ops)
		return 0;

	if (xr_usb->dp_ops->hpd_evt_notifier)
		return xr_usb->dp_ops->hpd_evt_notifier(dp_hpd);

	dev_warn(xr_usb->dev, "hpd evt notifier not register\n");
	return -EINVAL;
}

/* xr-usb-gadget-ops */
void xr_usb_init_gadget_ops(struct xring_usb *xr_usb);
void xr_usb_exit_gadget_ops(struct xring_usb *xr_usb);
void xr_usb_try_attach_gadget(struct xring_usb *xr_usb);
/* xr-usb-pm-override */
int xr_usb_override_pm_ops(struct xring_usb *xr_usb);
void xr_usb_recover_pm_ops(struct xring_usb *xr_usb);

#endif /* _XR_USB_CORE_H */
