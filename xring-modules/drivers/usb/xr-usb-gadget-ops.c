// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 */

#include "xr-usb-core.h"

#include <dwc3/core.h>
#include <gadget/function/rndis.h>
#include <gadget/function/u_ether.h>
#include <gadget/function/u_ncm.h>
#include <gadget/function/u_rndis.h>

#include <linux/configfs.h>
#include <linux/hrtimer.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock_types.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/gadget_configfs.h>
#include <linux/usb/webusb.h>
#include <linux/workqueue.h>

enum find_ether_type {
	FOUND_ETHER_NONE,
	FOUND_ETHER_NCM,
	FOUND_ETHER_RNDIS,
};

/* Attention: gadget_info should same as defined in configfs.c */
#define MAX_USB_STRING_LANGS 2
struct gadget_info {
	struct config_group group;
	struct config_group functions_group;
	struct config_group configs_group;
	struct config_group strings_group;
	struct config_group os_desc_group;
	struct config_group webusb_group;

	struct mutex lock;
	struct usb_gadget_strings *gstrings[MAX_USB_STRING_LANGS + 1];
	struct list_head string_list;
	struct list_head available_func;

	struct usb_composite_driver composite;
	struct usb_composite_dev cdev;
	bool use_os_desc;
	char b_vendor_code;
	char qw_sign[OS_STRING_QW_SIGN_LEN];
	bool use_webusb;
	u16 bcd_webusb_version;
	u8 b_webusb_vendor_code;
	char landing_page[WEBUSB_URL_RAW_MAX_LENGTH];

	spinlock_t spinlock;
	bool unbind;
};

struct config_usb_cfg {
	struct config_group group;
	struct config_group strings_group;
	struct list_head string_list;
	struct usb_configuration c;
	struct list_head func_list;
	struct usb_gadget_strings *gstrings[MAX_USB_STRING_LANGS + 1];
};

/* Attention: usb_udc should same as defined in udc/core.c */
struct usb_udc {
	struct usb_gadget_driver	*driver;
	struct usb_gadget		*gadget;
	struct device			dev;
	struct list_head		list;
	bool				vbus;
	bool				started;
	bool				allow_connect;
	struct work_struct		vbus_work;
	struct mutex			connect_lock;
};

/* Attention: f_ncm should same as defined in f_ncm.c */
struct ndp_parser_opts {
	u32		nth_sign;
	u32		ndp_sign;
	unsigned	nth_size;
	unsigned	ndp_size;
	unsigned	dpe_size;
	unsigned	ndplen_align;
	/* sizes in u16 units */
	unsigned	dgram_item_len; /* index or length */
	unsigned	block_length;
	unsigned	ndp_index;
	unsigned	reserved1;
	unsigned	reserved2;
	unsigned	next_ndp_index;
};

struct f_ncm {
	struct gether			port;
	u8				ctrl_id, data_id;
	char				ethaddr[14];
	struct usb_ep			*notify;
	struct usb_request		*notify_req;
	u8				notify_state;
	atomic_t			notify_count;
	bool				is_open;
	const struct ndp_parser_opts	*parser_opts;
	bool				is_crc;
	u32				ndp_sign;
	/*
	 * for notification, it is accessed from both
	 * callback and ethernet open/close
	 */
	spinlock_t			lock;
	struct net_device		*netdev;
	/* For multi-frame NDP TX */
	struct sk_buff			*skb_tx_data;
	struct sk_buff			*skb_tx_ndp;
	u16				ndp_dgram_count;
	struct hrtimer			task_timer;
};

/* Attention: eth_dev should same as defined in u_ether.c */
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

/* Attention: f_rndis should same as defined in f_rndis.c */
struct f_rndis {
	struct gether			port;
	u8				ctrl_id, data_id;
	u8				ethaddr[ETH_ALEN];
	u32				vendorID;
	const char			*manufacturer;
	struct rndis_params		*params;

	struct usb_ep			*notify;
	struct usb_request		*notify_req;
	atomic_t			notify_count;
};

static inline struct f_rndis *func_to_rndis(struct usb_function *f)
{
	return container_of(f, struct f_rndis, port.func);
}

static inline struct f_ncm *func_to_ncm(struct usb_function *f)
{
	return container_of(f, struct f_ncm, port.func);
}

static int ncm_err_bind(struct usb_configuration *c, struct usb_function *f)
{
	return -ENODEV;
}

static void error_replace_ncm_bind(struct usb_function *f)
{
	f->bind = ncm_err_bind;
}

static enum find_ether_type config_find_ether_inst(struct gadget_info *gi,
						   struct usb_function **found)
{
	struct usb_configuration *c = NULL;

	*found = NULL;

	list_for_each_entry(c, &gi->cdev.configs, list) {
		struct usb_function *f;
		struct config_usb_cfg *cfg;

		cfg = container_of(c, struct config_usb_cfg, c);

		list_for_each_entry(f, &c->functions, list) {
			if (!f->name)
				continue;

			if (!strcmp(f->name, "cdc_network")) {
				pr_info("found ncm function\n");
				*found = f;
				return FOUND_ETHER_NCM;
			} else if (!strcmp(f->name, "rndis")) {
				pr_info("found rndis function\n");
				*found = f;
				return FOUND_ETHER_RNDIS;
			}
		}
	}

	return FOUND_ETHER_NONE;
}

static void xr_usb_reset_ncm(struct xring_usb *xr_usb, struct usb_function *f)
{
	struct f_ncm *ncm = NULL;
	struct f_ncm_opts *ncm_opts = NULL;
	int status;

	dev_warn(xr_usb->dev, "clean ncm net device\n");

	ncm_opts = container_of(f->fi, struct f_ncm_opts, func_inst);
	mutex_lock(&ncm_opts->lock);
	if (!ncm_opts->bound) {
		mutex_unlock(&ncm_opts->lock);
		return;
	}

	gether_cleanup(netdev_priv(ncm_opts->net));
	ncm_opts->net = gether_setup_default();
	if (WARN_ON(IS_ERR(ncm_opts->net))) {
		dev_err(xr_usb->dev, "failed to create new net\n");
		ncm_opts->net = NULL;
		goto err_out;
	}

	ncm_opts->bound = false;

	ncm = func_to_ncm(f);
	status = gether_get_host_addr_cdc(ncm_opts->net, ncm->ethaddr,
					  sizeof(ncm->ethaddr));
	if (status < 12) { /* strlen("01234567890a") */
		dev_err(xr_usb->dev, "failed to set ethaddr\n");
		goto err_out;
	}

	ncm->port.ioport = netdev_priv(ncm_opts->net);
	mutex_unlock(&ncm_opts->lock);
	return;

err_out:
	mutex_unlock(&ncm_opts->lock);
	error_replace_ncm_bind(f);
}

static void xr_usb_reset_rndis(struct xring_usb *xr_usb, struct usb_function *f)
{
	struct f_rndis *rndis = NULL;
	struct f_rndis_opts *rndis_opts = NULL;

	dev_warn(xr_usb->dev, "clean rndis net device\n");

	rndis_opts = container_of(f->fi, struct f_rndis_opts, func_inst);
	mutex_lock(&rndis_opts->lock);
	if (rndis_opts->borrowed_net || !rndis_opts->bound) {
		mutex_unlock(&rndis_opts->lock);
		return;
	}

	gether_cleanup(netdev_priv(rndis_opts->net));
	rndis_opts->net = NULL;
	rndis_opts->bound = false;

	rndis = func_to_rndis(f);
	rndis->port.ioport = NULL;
	mutex_unlock(&rndis_opts->lock);
}

static void xr_usb_reset_ether(struct xring_usb *xr_usb, struct usb_function *f,
			     enum find_ether_type ether_type)
{
	if (ether_type == FOUND_ETHER_NCM)
		xr_usb_reset_ncm(xr_usb, f);
	else if (ether_type == FOUND_ETHER_RNDIS)
		xr_usb_reset_rndis(xr_usb, f);
}

static void xr_usb_composite_unbind(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);
	struct usb_function *f = NULL;
	enum find_ether_type ether_type;

	dev_info(xr_usb->dev, "enter\n");
	if (WARN_ON(!xr_usb->gadget_driver.unbind || !xr_usb->gi))
		return;

	ether_type = config_find_ether_inst(xr_usb->gi, &f);

	xr_usb->gadget_driver.unbind(gadget);

	if (f)
		xr_usb_reset_ether(xr_usb, f, ether_type);

	dev_info(xr_usb->dev, "exit\n");
}

static void xr_usb_composite_reset(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	dev_info(xr_usb->dev, "enter\n");
	if (WARN_ON(!xr_usb->gadget_driver.reset || !xr_usb->gi))
		return;

	xr_usb->dev_link_connect = false;
	xr_usb->gadget_driver.reset(gadget);
	dev_info(xr_usb->dev, "exit\n");
}

static void xr_usb_composite_disconnect(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	dev_info(xr_usb->dev, "enter\n");
	if (WARN_ON(!xr_usb->gadget_driver.disconnect || !xr_usb->gi))
		return;

	dev_info(xr_usb->dev, "last gadget state %s speed %s\n",
		 usb_state_string(gadget->state),
		 usb_speed_string(gadget->speed));
	xr_usb->dev_link_connect = false;
	xr_usb->gadget_driver.disconnect(gadget);
	dev_info(xr_usb->dev, "exit\n");
}

static int xr_usb_composite_setup(struct usb_gadget *gadget,
				  const struct usb_ctrlrequest *ctrl)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	if (WARN_ON(!xr_usb->gadget_driver.setup || !xr_usb->gi))
		return -ENODEV;

	if (!xr_usb->dev_link_connect) {
		xr_usb->dev_link_connect = true;
		dev_info(xr_usb->dev, "link speed %s\n",
			 usb_speed_string(gadget->speed));
	}
	return xr_usb->gadget_driver.setup(gadget, ctrl);
}

static int xr_usb_gadget_get_frame(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->get_frame(gadget);
}

static int xr_usb_gadget_wakeup(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->wakeup(gadget);
}

static int xr_usb_gadget_func_wakeup(struct usb_gadget *gadget, int intf_id)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->func_wakeup(gadget, intf_id);
}

static int xr_usb_gadget_set_remote_wakeup(struct usb_gadget *gadget, int set)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->set_remote_wakeup(gadget, set);
}

static int xr_usb_gadget_set_selfpowered(struct usb_gadget *gadget,
					 int is_selfpowered)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->set_selfpowered(gadget, is_selfpowered);
}

static int xr_usb_gadget_pullup(struct usb_gadget *gadget, int is_on)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->pullup(gadget, is_on);
}

static int xr_usb_gadget_start(struct usb_gadget *gadget,
			       struct usb_gadget_driver *gadget_driver)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);
	int ret;

	xr_usb->dev_link_connect = false;
	ret = xr_usb->dwc3_ops->udc_start(gadget, gadget_driver);
	if (ret) {
		dev_err(xr_usb->dev, "failed to start udc\n");
		if (gadget_driver->udc_name) {
			dev_warn(xr_usb->dev, "reset udc_name\n");
			kfree(gadget_driver->udc_name);
			gadget_driver->udc_name = NULL;
		}
	}
	return ret;
}

static int xr_usb_gadget_stop(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);
	int ret;

	ret = xr_usb->dwc3_ops->udc_stop(gadget);
	if (ret)
		dev_err(xr_usb->dev, "failed to stop udc\n");

	if (!xr_usb->gi)
		return ret;

	xr_usb->dev_link_connect = false;
	xr_usb->gi->composite.gadget_driver.unbind =
		xr_usb->gadget_driver.unbind;
	xr_usb->gi->composite.gadget_driver.reset =
		xr_usb->gadget_driver.reset;
	xr_usb->gi->composite.gadget_driver.disconnect =
		xr_usb->gadget_driver.disconnect;
	xr_usb->gi->composite.gadget_driver.setup =
		xr_usb->gadget_driver.setup;
	xr_usb->gi = NULL;
	memset(&xr_usb->gadget_driver, 0, sizeof(xr_usb->gadget_driver));

	return ret;
}

static void xr_usb_gadget_set_speed(struct usb_gadget *gadget,
				    enum usb_device_speed speed)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	xr_usb->dwc3_ops->udc_set_speed(gadget, speed);
}

static void xr_usb_gadget_set_ssp_rate(struct usb_gadget *gadget,
				       enum usb_ssp_rate rate)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	xr_usb->dwc3_ops->udc_set_ssp_rate(gadget, rate);
}

static void xr_usb_gadget_config_params(struct usb_gadget *gadget,
					struct usb_dcd_config_params *params)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	xr_usb->dwc3_ops->get_config_params(gadget, params);
}

static int xr_usb_gadget_vbus_draw(struct usb_gadget *gadget, unsigned mA)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->vbus_draw(gadget, mA);
}

static void xr_usb_gadget_async_callbacks(struct usb_gadget *gadget, bool enable)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);
	struct usb_composite_driver *c = NULL;
	struct usb_gadget_driver *gadget_driver = NULL;

	xr_usb->dwc3_ops->udc_async_callbacks(gadget, enable);

	if (WARN_ON(!gadget->udc))
		return;

	if (WARN_ON(!gadget->udc->driver))
		return;

	dev_info(xr_usb->dev, "replace gadget driver ops\n");
	gadget_driver = gadget->udc->driver;
	c = container_of(gadget_driver, struct usb_composite_driver,
			 gadget_driver);
	xr_usb->gi = container_of(c, struct gadget_info, composite);
	/* Save once */
	if (gadget_driver->unbind != xr_usb_composite_unbind)
		xr_usb->gadget_driver = *gadget_driver;

	gadget_driver->unbind = xr_usb_composite_unbind;
	gadget_driver->reset = xr_usb_composite_reset;
	gadget_driver->disconnect = xr_usb_composite_disconnect;
	gadget_driver->setup = xr_usb_composite_setup;
}

static int xr_usb_gadget_check_config(struct usb_gadget *gadget)
{
	struct xring_usb *xr_usb =
		container_of(gadget->ops, struct xring_usb, gadget_ops);

	return xr_usb->dwc3_ops->check_config(gadget);
}

static const struct usb_gadget_ops xr_usb_gadget_ops = {
	.get_frame		= xr_usb_gadget_get_frame,
	.wakeup			= xr_usb_gadget_wakeup,
	.func_wakeup		= xr_usb_gadget_func_wakeup,
	.set_remote_wakeup	= xr_usb_gadget_set_remote_wakeup,
	.set_selfpowered	= xr_usb_gadget_set_selfpowered,
	.pullup			= xr_usb_gadget_pullup,
	.udc_start		= xr_usb_gadget_start,
	.udc_stop		= xr_usb_gadget_stop,
	.udc_set_speed		= xr_usb_gadget_set_speed,
	.udc_set_ssp_rate	= xr_usb_gadget_set_ssp_rate,
	.get_config_params	= xr_usb_gadget_config_params,
	.vbus_draw		= xr_usb_gadget_vbus_draw,
	.check_config		= xr_usb_gadget_check_config,
	.udc_async_callbacks	= xr_usb_gadget_async_callbacks,
};

static void xr_usb_replace_gadget_ops(struct xring_usb *xr_usb,
				      struct usb_gadget *gadget)
{
	dev_info(xr_usb->dev, "replace dwc3 gadget ops\n");
	xr_usb->dwc3_ops = gadget->ops;
	gadget->ops = &xr_usb->gadget_ops;
}

static int xr_usb_gadget_bus_notify(struct notifier_block *nb,
				    unsigned long action, void *data)
{
	struct xring_usb *xr_usb =
		container_of(nb, struct xring_usb, gadget_bus_nb);
	struct device *dev = data;
	struct usb_gadget *gadget = container_of(dev, struct usb_gadget, dev);

	switch (action) {
	case BUS_NOTIFY_ADD_DEVICE:
		xr_usb_replace_gadget_ops(xr_usb, gadget);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void xr_usb_gadget_release(struct device *dev)
{
	struct usb_gadget *gadget = container_of(dev, struct usb_gadget, dev);

	kfree(gadget);
}

void xr_usb_init_gadget_ops(struct xring_usb *xr_usb)
{
	struct usb_gadget *gadget = NULL;

	/* Get gadget_bus_type */
	gadget = kzalloc(sizeof(struct usb_gadget), GFP_KERNEL);
	if (!gadget) {
		dev_err(xr_usb->dev, "failed alloc gadget\n");
		return;
	}

	usb_initialize_gadget(NULL, gadget, xr_usb_gadget_release);
	if (!gadget->dev.bus) {
		dev_err(xr_usb->dev, "can't find gadget bus\n");
		usb_put_gadget(gadget);
		return;
	}

	xr_usb->gadget_bus_nb.notifier_call = xr_usb_gadget_bus_notify;
	xr_usb->gadget_bus = gadget->dev.bus;
	if (bus_register_notifier(xr_usb->gadget_bus, &xr_usb->gadget_bus_nb))
		dev_err(xr_usb->dev, "failed to register gadget notifier\n");

	xr_usb->gadget_ops = xr_usb_gadget_ops;
	usb_put_gadget(gadget);
}

void xr_usb_exit_gadget_ops(struct xring_usb *xr_usb)
{
	if (!xr_usb->gadget_bus)
		return;

	if (bus_unregister_notifier(xr_usb->gadget_bus, &xr_usb->gadget_bus_nb))
		dev_err(xr_usb->dev, "failed to register gadget notifier\n");

	xr_usb->gadget_bus = NULL;
}

void xr_usb_try_attach_gadget(struct xring_usb *xr_usb)
{
	struct dwc3 *dwc = NULL;
	struct usb_udc *udc = NULL;

	if (!xr_usb->dwc3)
		return;

	dwc = platform_get_drvdata(xr_usb->dwc3);
	if (!dwc) {
		dev_err(xr_usb->dev, "dwc3 not probed\n");
		return;
	}

	/* Wait mode switch finished */
	flush_work(&dwc->drd_work);
	if (!dwc->gadget) {
		dev_err(xr_usb->dev, "dwc3 not in gadget mode\n");
		return;
	}

	/* Check gadget driver */
	udc = dwc->gadget->udc;
	if (WARN_ON(!udc))
		return;

	if (!udc->driver) {
		dev_warn(xr_usb->dev, "try bind gadget driver\n");
		if (device_attach(&dwc->gadget->dev) <= 0) {
			device_release_driver(&dwc->gadget->dev);
			dev_warn(xr_usb->dev, "failed to attach dwc3 gadget\n");
		}
	}
}
