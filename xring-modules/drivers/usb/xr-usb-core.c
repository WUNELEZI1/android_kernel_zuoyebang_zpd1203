// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Inspired by dwc3-of-simple.c
 */

#include "xr-usb-core.h"
#include "xr-usb-debugfs.h"
#include "xr-usb-generic-phy.h"
#include "xr-usb-plat.h"

#include <asm-generic/errno-base.h>
#include <soc/xring/xr_usbdp_event.h>
#include <soc/xring/doze.h>
#include <soc/xring/flowctrl.h>
#include <dt-bindings/xring/platform-specific/pm/include/sys_doze_plat.h>
#include <dt-bindings/xring/platform-specific/common/pm/include/sys_doze.h>
#include <dwc3/core.h>

#include <linux/export.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/extcon-provider.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/phy/phy.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/usb/ch9.h>
#include <linux/usb/role.h>
#include <linux/workqueue.h>
#include <mca_hardware_ic/redriver/ps5169/redriver.h>

#define USB_STATE_WORK_DELAY (HZ)
#define USB_INIT_STATE_DELAY (5 * HZ)

static unsigned int xr_dwc3_readl(void __iomem *base, unsigned int offset)
{
	return readl(base + offset - DWC3_GLOBALS_REGS_START);
}

static void xr_dwc3_writel(void __iomem *base, unsigned int offset,
			   unsigned int value)
{
	writel(value, base + offset - DWC3_GLOBALS_REGS_START);
}

static void xr_usb_combophy_eye_set(struct xring_usb *xr_usb,
				    unsigned int *eye_para, bool orientation)
{
	unsigned int group;
	int ret;

	for (group = 0; group < COMBOPHY_EYE_PARAM_MAX; group++) {
		ret = xr_usb_combophy_set_eye_param(xr_usb->usb3_phy, group,
						    &eye_para[group]);
		if (ret)
			dev_err(xr_usb->dev,
				"Combo phy eye group[%d] set fail\n", group);
	}
}

static void xr_usb_hsphy_eye_set(struct xring_usb *xr_usb,
				 unsigned int *eye_para)
{
	unsigned int group;
	int ret;

	for (group = 0; group < HSPHY_EYE_PARAM_MAX; group++) {
		ret = xr_usb_hsphy_set_eye_param(xr_usb->usb2_phy, group,
						 &eye_para[group]);
		if (ret)
			dev_err(xr_usb->dev, "HS phy eye group[%d] set fail\n",
				group);
	}
}

int combophy_initial(struct xring_usb *xr_usb, enum phy_mode mode, int submode)
{
	int ret;

	ret = xr_usb_combophy_power_on(xr_usb->usb3_phy);
	if (ret) {
		dev_err(xr_usb->dev, "combophy powerup fail\n");
		goto combophy_powerup_err;
	}

	ret = xr_usb_combophy_role_set_mode_ext(xr_usb->usb3_phy, mode,
						submode);
	if (ret) {
		dev_err(xr_usb->dev, "hsphy_role_set_mode fail\n");
		goto combophy_role_set_fail;
	}

	ret = xr_usb_combophy_init(xr_usb->usb3_phy);
	if (ret) {
		dev_err(xr_usb->dev, "combophy_init fail\n");
		goto combophy_init_err;
	}

	return 0;

combophy_init_err:
	xr_usb_combophy_role_set_mode_ext(xr_usb->usb3_phy, PHY_MODE_INVALID,
					  0);
combophy_role_set_fail:
	xr_usb_combophy_power_off(xr_usb->usb3_phy);
combophy_powerup_err:
	return ret;
}

void combophy_exit(struct xring_usb *xr_usb)
{
	int ret;

	ret = xr_usb_combophy_exit(xr_usb->usb3_phy);
	if (ret)
		dev_err(xr_usb->dev, "combophy exit fail\n");

	ret = xr_usb_combophy_power_off(xr_usb->usb3_phy);
	if (ret)
		dev_err(xr_usb->dev, "combophy_poweroff fail\n");
}

int hsphy_initial(struct xring_usb *xr_usb, enum phy_mode mode)
{
	int ret;
	bool eye_set;
	unsigned int *eye_para;

	switch (mode) {
	case PHY_MODE_USB_HOST:
		eye_set = xr_usb->u2phy_h_eye_set;
		eye_para = &xr_usb->host_u2eye[0];
		break;
	case PHY_MODE_USB_DEVICE:
		eye_set = xr_usb->u2phy_d_eye_set;
		eye_para = &xr_usb->device_u2eye[0];
		break;
	default:
		dev_err(xr_usb->dev, "unknown phy mode\n");
		return -EINVAL;
	}

	ret = xr_usb_hsphy_power_on(xr_usb->usb2_phy);
	if (ret) {
		dev_err(xr_usb->dev, "hsphy powerup fail\n");
		goto hsphy_powerup_fail;
	}

	if (eye_set)
		xr_usb_hsphy_eye_set(xr_usb, eye_para);

	ret = xr_usb_hsphy_role_set_mode(xr_usb->usb2_phy, mode);
	if (ret) {
		dev_err(xr_usb->dev, "hsphy_role_set_mode fail\n");
		goto hsphy_role_set_fail;
	}

	ret = xr_usb_hsphy_init(xr_usb->usb2_phy);
	if (ret) {
		dev_err(xr_usb->dev, "hsphy_init fail\n");
		goto hsphy_init_err;
	}

	return 0;

hsphy_init_err:
	xr_usb_hsphy_role_set_mode(xr_usb->usb2_phy, PHY_MODE_INVALID);
hsphy_role_set_fail:
	xr_usb_hsphy_power_off(xr_usb->usb2_phy);
hsphy_powerup_fail:
	return ret;
}

void hsphy_exit(struct xring_usb *xr_usb)
{
	int ret;

	ret = xr_usb_hsphy_role_set_mode(xr_usb->usb2_phy, PHY_MODE_INVALID);
	if (ret)
		dev_err(xr_usb->dev, "hsphy_role_set_mode exit fail\n");

	ret = xr_usb_hsphy_exit(xr_usb->usb2_phy);
	if (ret)
		dev_err(xr_usb->dev, "hsphy exit fail\n");

	ret = xr_usb_hsphy_power_off(xr_usb->usb2_phy);
	if (ret)
		dev_err(xr_usb->dev, "hsphy_poweroff fail\n");
}

static int xr_usb_phy_init(struct xring_usb *xr_usb, enum phy_mode mode,
			   int submode)
{
	int ret;

	if (!xr_usb->usb2_phy || !xr_usb->usb3_phy)
		return -EINVAL;

	ret = hsphy_initial(xr_usb, mode);
	if (ret) {
		dev_err(xr_usb->dev, "hsphy_initial fail\n");
		goto hsphy_initial_err;
	}

	ret = combophy_initial(xr_usb, mode, submode);
	if (ret) {
		dev_err(xr_usb->dev, "combophy_initial fail\n");
		goto combophy_initial_err;
	}

	return 0;

combophy_initial_err:
	hsphy_exit(xr_usb);
hsphy_initial_err:
	return ret;
}

static void xr_usb_phy_exit(struct xring_usb *xr_usb)
{
	combophy_exit(xr_usb);
	hsphy_exit(xr_usb);
}

int xr_usb_queue_event(struct xring_usb *xr_usb,
		       union xr_usbdp_event event_data)
{
	int ret = -ESHUTDOWN;

	spin_lock(&xr_usb->lock);
	if (xr_usb->removing)
		goto unlock;

	/* last event can only be off event */
	if (kfifo_avail(&xr_usb->input_event_fifo) == 1 &&
	    event_data.data != 0) {
		ret = -EAGAIN;
		goto unlock;
	}

	if (!kfifo_in(&xr_usb->input_event_fifo, &event_data, 1)) {
		ret = -EAGAIN;
		goto unlock;
	}

	dev_info(xr_usb->dev, "queue event 0x%x\n", event_data.data);

	if (xr_usb->suspending || xr_usb->changing_ops)
		goto unlock;

	if (mod_delayed_work(system_wq, &xr_usb->state_work, 0))
		ret = -EBUSY;
	else
		ret = 0;

unlock:
	spin_unlock(&xr_usb->lock);
	if (ret == -EBUSY) {
		dev_warn(xr_usb->dev, "state_work already on a queue\n");
		ret = 0;
	} else if (ret < 0) {
		dev_err(xr_usb->dev, "failed to queue event - %d\n", ret);
	}
	return ret;
}
EXPORT_SYMBOL(xr_usb_queue_event);

static void xr_usb_adaptation_dwc3(struct xring_usb *xr_usb)
{
	struct device *dev = xr_usb->dev;
	struct dwc3 *dwc = NULL;
	unsigned int reg;

	dwc = platform_get_drvdata(xr_usb->dwc3);

	/* adaptation for FPGA ram_clk less than 60 MHz */
	if (xr_usb->usbtrdtim != 0) {
		reg = xr_dwc3_readl(dwc->regs, DWC3_GUSB2PHYCFG(0));
		reg &= ~DWC3_GUSB2PHYCFG_USBTRDTIM_MASK;
		reg |= DWC3_GUSB2PHYCFG_USBTRDTIM(xr_usb->usbtrdtim);
		xr_dwc3_writel(dwc->regs, DWC3_GUSB2PHYCFG(0), reg);
		dev_info(dev, "GUSB2PHYCFG(0x%x)\n", reg);
	}
}

static int xr_usb_populate_dwc3(struct xring_usb *xr_usb, enum usb_role role)
{
	struct device *dev = xr_usb->dev;
	struct device_node *np = dev->of_node;
	struct device_node *dwc3_np = NULL;
	struct dwc3 *dwc = NULL;
	int ret;

	dev_info(xr_usb->dev, "set dwc3 to %s\n",
		 role == USB_ROLE_HOST ? "HOST" : "DEVICE");
	if (xr_usb->dwc3_edev) {
		bool host_attach = (role == USB_ROLE_HOST);

		(void)extcon_set_state(xr_usb->dwc3_edev, EXTCON_USB,
				       !host_attach);
		(void)extcon_set_state(xr_usb->dwc3_edev, EXTCON_USB_HOST,
				       host_attach);
	}

	if (!xr_usb->dwc3) {
		dwc3_np = of_get_compatible_child(np, "snps,dwc3");
		if (!dwc3_np) {
			dev_err(dev, "failed to find dwc3 core child\n");
			return -ENODEV;
		}

		/* convert its child node to platform_device and register to the bus */
		ret = of_platform_populate(np, NULL, NULL, dev);
		if (ret) {
			dev_err(dev, "failed to register dwc3 core - %d\n",
				ret);
			goto node_put;
		}

		xr_usb->dwc3 = of_find_device_by_node(dwc3_np);
		if (!xr_usb->dwc3) {
			ret = -ENODEV;
			dev_err(dev, "failed to get dwc3 platform device\n");
			goto of_depopulate;
		}
		of_node_put(dwc3_np);
	} else {
		get_device(&xr_usb->dwc3->dev);
		ret = device_attach(&xr_usb->dwc3->dev);
		if (ret <= 0) {
			device_release_driver(&xr_usb->dwc3->dev);
			dev_err(dev, "failed to attach dwc3\n");
			ret = -ENODEV;
			goto of_dwc3_put;
		}
	}

	dwc = platform_get_drvdata(xr_usb->dwc3);
	if (!dwc) {
		ret = -ENODEV;
		dev_err(dev, "failed to get dwc3 platform data\n");
		goto of_dwc3_put;
	}

	xr_usb_adaptation_dwc3(xr_usb);

	if (xr_usb->use_role_switch) {
		struct usb_role_switch *sw = usb_role_switch_get(dev);

		if (IS_ERR_OR_NULL(sw)) {
			ret = -ENODEV;
			dev_err(dev, "failed to get role switch\n");
			goto of_dwc3_put;
		}

		ret = usb_role_switch_set_role(sw, role);
		usb_role_switch_put(sw);
		if (ret) {
			dev_err(dev, "failed to set role switch\n");
			goto of_dwc3_put;
		}
	}

	/* pm wakeup lock */
	device_init_wakeup(dev, true);
	pm_stay_awake(dev);

	return 0;

of_dwc3_put:
	put_device(&xr_usb->dwc3->dev);

of_depopulate:
	if (!xr_usb->dwc3) {
		of_platform_depopulate(dev);
		xr_usb->dwc3 = NULL;
	}
node_put:
	if (dwc3_np)
		of_node_put(dwc3_np);
	return ret;
}

static void xr_usb_depopulate_dwc3(struct xring_usb *xr_usb)
{
	put_device(&xr_usb->dwc3->dev);
	device_release_driver(&xr_usb->dwc3->dev);
	/* pm wakeup unlock */
	pm_relax(xr_usb->dev);
	device_init_wakeup(xr_usb->dev, false);
}

static int xr_usb_pm_notifier(struct notifier_block *notify_block,
			      unsigned long mode, void *_unused)
{
	struct xring_usb *xr_usb =
		container_of(notify_block, struct xring_usb, pm_notify);
	int ret = NOTIFY_DONE;
	bool do_restore = false;
	bool force_queue = false;

	if (mode == PM_SUSPEND_PREPARE) {
		spin_lock(&xr_usb->lock);
		xr_usb->suspending = true;
		spin_unlock(&xr_usb->lock);
		flush_delayed_work(&xr_usb->state_work);

		if (xr_usb->usb_state > USB_STATE_IDLE &&
		    xr_usb->usb_state != USB_STATE_ADSP_HOST &&
		    xr_usb->usb_state != USB_STATE_DYNAMIC_DIS) {
			dev_info(xr_usb->dev, "%s mode still active\n",
				 xr_usb_state_to_name(xr_usb->usb_state));
			ret = NOTIFY_BAD;
			do_restore = true;
		}
	} else if (mode == PM_POST_SUSPEND) {
		force_queue = true;
		dev_info(xr_usb->dev, "usb post suspend notify\n");
	}

	if (do_restore || force_queue) {
		spin_lock(&xr_usb->lock);
		xr_usb->suspending = false;
		if (!kfifo_is_empty(&xr_usb->input_event_fifo) || force_queue) {
			dev_info(xr_usb->dev, "resume state_work, flag:%d\n", force_queue);
			WARN_ON(!queue_delayed_work(system_wq,
						    &xr_usb->state_work, 0));
		}
		spin_unlock(&xr_usb->lock);
	}

	return ret;
}

static int xr_usb_release(struct xring_usb *xr_usb)
{
	int ret;

	ret = reset_control_deassert(xr_usb->vbus_valid);
	if (ret) {
		dev_err(xr_usb->dev, "failed to set vbus_valid\n");
		return ret;
	}

	ret = reset_control_deassert(xr_usb->controller_reset);
	if (ret) {
		dev_err(xr_usb->dev, "failed to deaseert controller_reset\n");
		reset_control_assert(xr_usb->vbus_valid);
	}

	return ret;
}

static void xr_usb_reset(struct xring_usb *xr_usb)
{
	int ret;

	ret = reset_control_assert(xr_usb->controller_reset);
	if (ret)
		dev_err(xr_usb->dev, "failed to aseert controller_reset\n");

	ret = reset_control_assert(xr_usb->vbus_valid);
	if (ret)
		dev_err(xr_usb->dev, "failed to clear vbus_valid\n");
}

static void combophy_init_eye_param(struct xring_usb *xr_usb,
				    enum phy_mode mode, bool orientation)
{
	bool eye_set = false;
	unsigned int *eye_para;

	switch (mode) {
	case PHY_MODE_USB_HOST:
		eye_set = xr_usb->u3phy_h_eye_set;
		eye_para = &xr_usb->host_u3eye[0];
		break;
	case PHY_MODE_USB_DEVICE:
		eye_set = xr_usb->u3phy_d_eye_set;
		eye_para = &xr_usb->device_u3eye[0];
		break;
	default:
		dev_err(xr_usb->dev, "unknown phy mode\n");
	}

	if (eye_set)
		xr_usb_combophy_eye_set(xr_usb, eye_para, orientation);
}

int xr_usb_enter_standby(struct xring_usb *xr_usb)
{
	int ret;

	dev_info(xr_usb->dev, "lpstate:%d\n", xr_usb->lpstate);

	if (xr_usb->lpstate != LP_P4PG)
		return 0;

	ret = xr_usb_plat_powerup(xr_usb->plat, xr_usb);
	if (ret) {
		dev_err(xr_usb->dev, "failed to exit powerdown\n");
		goto exit;
	}

	ret = clk_bulk_prepare_enable(xr_usb->num_clks, xr_usb->clks);
	if (ret) {
		dev_err(xr_usb->dev, "failed to enable clks\n");
		goto err_clk_enable;
	}

	ret = reset_control_deassert(xr_usb->sys_reset);
	if (ret) {
		dev_err(xr_usb->dev, "failed to deassert resets\n");
		goto err_sys_reset;
	}

	ret = xr_usb_plat_enter_standby(xr_usb->plat, xr_usb);
	if (ret) {
		dev_err(xr_usb->dev, "failed to enter standby\n");
		xr_usb->is_standby = false;
		goto err_enter_standby;
	}

	xr_usb->is_standby = true;

	clk_bulk_disable_unprepare(xr_usb->num_clks, xr_usb->clks);
	ret = reset_control_assert(xr_usb->sys_reset);
	if (ret)
		dev_err(xr_usb->dev, "failed to assert resets\n");

	return ret;

err_enter_standby:
	reset_control_assert(xr_usb->sys_reset);
err_sys_reset:
	clk_bulk_disable_unprepare(xr_usb->num_clks, xr_usb->clks);
err_clk_enable:
	xr_usb_plat_powerdown(xr_usb->plat, xr_usb);
exit:
	return ret;
}

int xr_usb_exit_standby(struct xring_usb *xr_usb)
{
	int ret;

	if (!xr_usb->is_standby) {
		dev_info(xr_usb->dev, "not use to exit combophy standby\n");
		return 0;
	}

	ret = xr_usb_plat_exit_standby(xr_usb->plat, xr_usb);
	if (ret)
		dev_err(xr_usb->dev, "failed to exit standby\n");

	xr_usb->is_standby = false;

	return ret;
}

static int xr_usb_hardware_init(struct xring_usb *xr_usb, enum phy_mode mode,
				struct usb_event_info event)
{
	int ret;
	unsigned int submode;

	sys_state_doz2nor_vote(VOTER_DOZAP_USB);

	ret = xr_usb_exit_standby(xr_usb);
	if (ret) {
		dev_err(xr_usb->dev, "failed to exit standby\n");
		return ret;
	}

	ret = xr_usb_plat_powerup(xr_usb->plat, xr_usb);
	if (ret)
		dev_err(xr_usb->dev, "failed to exit powerdown\n");

	ret = clk_bulk_prepare_enable(xr_usb->num_clks, xr_usb->clks);
	if (ret) {
		dev_err(xr_usb->dev, "failed to enable clks\n");
		goto exit;
	}

	ret = reset_control_deassert(xr_usb->sys_reset);
	if (ret) {
		dev_err(xr_usb->dev, "failed to deassert resets\n");
		goto err_sys_reset;
	}

	ret = xr_usb_plat_exit_bus_idle(xr_usb->plat, xr_usb, true);
	if (ret) {
		dev_err(xr_usb->dev, "failed to do platform sys_init\n");
		goto err_plat_exit_bus_idle;
	}

	ret = xr_usb_plat_sys_init(xr_usb->plat, xr_usb);
	if (ret) {
		dev_err(xr_usb->dev, "failed to do platform sys_init\n");
		goto err_plat_sys_init;
	}

	submode = PHY_FLIP_INVERT;
	if (event.typec_orig)
		submode |= PHY_ORIENT_FLIP;
	ret = xr_usb_phy_init(xr_usb, mode, submode);
	if (ret) {
		dev_err(xr_usb->dev, "phy init failed\n");
		goto err_usb_phy_init;
	}

	ret = xr_usb_release(xr_usb);
	if (ret) {
		dev_err(xr_usb->dev, "usb release failed\n");
		goto err_usb_release;
	}

	if (event.usbdp_mode >= DP_MODE) {
		ret = xr_usb_dp_poweron(xr_usb, true, event.typec_orig,
					event.usbdp_mode);
		if (ret) {
			dev_err(xr_usb->dev, "dp power on failed\n");
			goto err_dp_poweron;
		}

		if (event.usbdp_mode == USBDP_MODE)
			usb_redriver_release_lanes(xr_usb->redriver,
						   USB_LANES_TWO);
		else if (event.usbdp_mode == DP_MODE)
			usb_redriver_release_lanes(xr_usb->redriver,
						   USB_LANES_FOUR);
	}

	ret = xr_usb_combophy_mode_toggle(xr_usb->usb3_phy, event.usbdp_mode);
	if (ret) {
		dev_err(xr_usb->dev, "combophy_mode_toggle fail\n");
		goto err_combophy_mode_toggle;
	}

	if (event.usbdp_mode != DP_MODE)
		combophy_init_eye_param(xr_usb, mode, event.typec_orig);

	return 0;

err_combophy_mode_toggle:
	if (event.usbdp_mode >= DP_MODE) {
		xr_usb_dp_poweron(xr_usb, false, event.typec_orig, NC_MODE);
		usb_redriver_notify_disconnect(xr_usb->redriver);
	}
err_dp_poweron:
	xr_usb_reset(xr_usb);
err_usb_release:
	xr_usb_phy_exit(xr_usb);
err_usb_phy_init:
	xr_usb_plat_sys_exit(xr_usb->plat, xr_usb);
err_plat_sys_init:
	xr_usb_plat_exit_bus_idle(xr_usb->plat, xr_usb, false);
err_plat_exit_bus_idle:
	reset_control_assert(xr_usb->sys_reset);
err_sys_reset:
	clk_bulk_disable_unprepare(xr_usb->num_clks, xr_usb->clks);
exit:
	xr_usb_plat_powerdown(xr_usb->plat, xr_usb);
	sys_state_doz2nor_unvote(VOTER_DOZAP_USB);

	return ret;
}

static void xr_usb_hardware_exit(struct xring_usb *xr_usb)
{
	int ret;

	if (xr_usb->current_evt.usbdp_mode >= DP_MODE) {
		ret = xr_usb_dp_poweron(
			xr_usb, false, xr_usb->current_evt.typec_orig, NC_MODE);
		if (ret)
			dev_err(xr_usb->dev, "dp power off failed\n");
	}

	xr_usb_reset(xr_usb);

	xr_usb_plat_exit_bus_idle(xr_usb->plat, xr_usb, false);

	xr_usb_phy_exit(xr_usb);

	xr_usb_plat_powerdown(xr_usb->plat, xr_usb);

	clk_bulk_disable_unprepare(xr_usb->num_clks, xr_usb->clks);

	ret = reset_control_assert(xr_usb->sys_reset);
	if (ret)
		dev_err(xr_usb->dev, "failed to assert resets\n");

	ret = xr_usb_enter_standby(xr_usb);
	if (ret)
		dev_err(xr_usb->dev, "failed to init standby\n");

	sys_state_doz2nor_unvote(VOTER_DOZAP_USB);
}

static int xr_usb_start_peripheral(struct xring_usb *xr_usb, bool on,
				   struct usb_event_info event)
{
	int ret;

	if (on) {
		if (xr_usb->redriver != NULL)
			xr_usb->redriver->usb_mode = false;

		usb_redriver_notify_connect(xr_usb->redriver, event.typec_orig);

		ret = xr_usb_hardware_init(xr_usb, PHY_MODE_USB_DEVICE, event);
		if (ret) {
			dev_err(xr_usb->dev, "hardware init failed\n");
			goto err_hardware_init;
		}

		xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_AP, true);
		ret = xr_usb_populate_dwc3(xr_usb, USB_ROLE_DEVICE);
		if (ret) {
			dev_err(xr_usb->dev, "populate dwc3 failed\n");
			xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_AP,
					     false);
			goto err_populate_dwc3;
		}

		xr_usb_try_attach_gadget(xr_usb);
		xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_AP, false);
		xring_flowctrl_usb_qos_cfg(false);
	} else {
		usb_redriver_notify_disconnect(xr_usb->redriver);
		xr_usb_depopulate_dwc3(xr_usb);
		xr_usb_hardware_exit(xr_usb);
	}

	return 0;

err_populate_dwc3:
	xr_usb_hardware_exit(xr_usb);
err_hardware_init:
	return ret;
}

static int xr_hpd_notifier(struct xring_usb *xr_usb,	unsigned int dp_hpd)
{
	if(NULL != xr_usb->redriver)
		usb_redriver_notify_hpd(xr_usb->redriver, dp_hpd);

	return xr_hpd_evt_notifier(xr_usb, dp_hpd);
}

static int xr_usb_start_host(struct xring_usb *xr_usb, bool on,
			     struct usb_event_info event)
{
	int ret;

	if (on) {
		if (xr_usb->redriver != NULL)
			xr_usb->redriver->usb_mode = true;

		usb_redriver_notify_connect(xr_usb->redriver, event.typec_orig);

		ret = xr_usb_hardware_init(xr_usb, PHY_MODE_USB_HOST, event);
		if (ret) {
			dev_err(xr_usb->dev, "hardware init failed\n");
			goto err_hardware_init;
		}

		ret = xr_usb_populate_dwc3(xr_usb, USB_ROLE_HOST);
		if (ret) {
			dev_err(xr_usb->dev, "populate dwc3 failed\n");
			goto err_populate_dwc3;
		}

		if (event.usbdp_mode >= DP_MODE) {
			ret = xr_hpd_notifier(xr_usb, event.dp_hpd);
			if (ret) {
				dev_err(xr_usb->dev,
					"dp process hpd_evt:%d failed\n",
					event.dp_hpd);
				goto err_dp_hpd;
			}
		}

		ret = xr_usb_override_pm_ops(xr_usb);
		if (ret) {
			dev_err(xr_usb->dev, "overried pm ops failed\n");
			goto err_dp_hpd;
		}
		xring_flowctrl_usb_qos_cfg(false);
	} else {
		if (xr_usb->current_evt.usbdp_mode >= DP_MODE) {
			ret = xr_hpd_notifier(xr_usb, event.dp_hpd);
			if (ret)
				dev_err(xr_usb->dev,
					"dp process hpd_evt:%d failed\n",
					event.dp_hpd);
		}

		usb_redriver_notify_disconnect(xr_usb->redriver);

		xr_usb_recover_pm_ops(xr_usb);
		xr_usb_depopulate_dwc3(xr_usb);
		xr_usb_hardware_exit(xr_usb);
	}

	return 0;

err_dp_hpd:
	xr_usb_depopulate_dwc3(xr_usb);
err_populate_dwc3:
	xr_usb_hardware_exit(xr_usb);
err_hardware_init:
	return ret;
}

static int xr_usb_adsp_start(struct xring_usb *xr_usb,
			     struct platform_device *xhci)
{
	if (xr_usb->usb_adsp_ops->start)
		return xr_usb->usb_adsp_ops->start(
			xr_usb->usb_adsp_ops->context, xhci);

	return -EINVAL;
}

static int xr_usb_adsp_stop(struct xring_usb *xr_usb)
{
	if (xr_usb->usb_adsp_ops->stop)
		return xr_usb->usb_adsp_ops->stop(
			xr_usb->usb_adsp_ops->context);

	return -EINVAL;
}

static void xr_usb_adsp_state_update_notify(struct xring_usb *xr_usb,
	unsigned int mode)
{
	if (!xr_usb->usb_adsp_ops)
		return;

	if (xr_usb->usb_adsp_ops->state_update_notify && mode == USB_MODE_UNPLUG)
		xr_usb->usb_adsp_ops->state_update_notify(
			xr_usb->usb_adsp_ops->context);

}

static int xr_usb_start_adsp_host(struct xring_usb *xr_usb, bool on)
{
	int ret;
	struct dwc3 *dwc = NULL;

	dev_info(xr_usb->dev, "%s adsp host\n", on ? "start" : "stop");

	if (!xr_usb->dwc3 || !xr_usb->usb_adsp_ops)
		return -ENOENT;

	dwc = platform_get_drvdata(xr_usb->dwc3);
	if (!dwc) {
		dev_err(xr_usb->dev, "failed to get dwc3 platform data\n");
		return -ENODEV;
	}

	if (on) {
		xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_AP, true);
		xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_ADSP,
				     false);
		ret = xr_usb_adsp_start(xr_usb, dwc->xhci);
		if (ret) {
			dev_err(xr_usb->dev, "failed to start usb adsp - %d\n",
				ret);
			xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_AP,
					     false);
			xr_usb_plat_mask_irq(xr_usb->plat, xr_usb,
					     USB_INTR_ADSP, true);
			return ret;
		}
		xr_usb_usb3_enter_standby(xr_usb->plat, xr_usb);
		xring_flowctrl_usb_qos_cfg(true);
		pm_relax(xr_usb->dev);
	} else {
		pm_stay_awake(xr_usb->dev);
		xr_usb_usb3_exit_standby(xr_usb->plat, xr_usb);
		ret = xr_usb_adsp_stop(xr_usb);
		if (ret)
			dev_err(xr_usb->dev, "failed to stop usb adsp - %d\n",
				ret);
		xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_AP, false);
		xr_usb_plat_mask_irq(xr_usb->plat, xr_usb, USB_INTR_ADSP, true);
		xring_flowctrl_usb_qos_cfg(false);
	}

	return 0;
}

static int usbdp_event_process(struct xring_usb *xr_usb,
			       struct usb_event_info usb_event)
{
	enum xr_usb_state next_state = USB_STATE_UNDEFINED;
	int ret = -EINVAL;

	switch (xr_usb->usb_state) {
	case USB_STATE_IDLE:
		if (usb_event.mode == USB_MODE_HOST) {
			ret = xr_usb_start_host(xr_usb, true, usb_event);
			next_state = USB_STATE_HOST;
		} else if (usb_event.mode == USB_MODE_PERIPEHRAL) {
			ret = xr_usb_start_peripheral(xr_usb, true, usb_event);
			next_state = USB_STATE_PERIPHERAL;
		}
		break;
	case USB_STATE_PERIPHERAL:
		next_state = USB_STATE_IDLE;
		if (usb_event.mode == USB_MODE_UNPLUG)
			ret = xr_usb_start_peripheral(xr_usb, false, usb_event);
		break;
	case USB_STATE_HOST:
		if (usb_event.mode == USB_MODE_UNPLUG) {
			ret = xr_usb_start_host(xr_usb, false, usb_event);
			next_state = USB_STATE_IDLE;
		}
		break;
	case USB_STATE_ADSP_HOST:
		if (usb_event.mode == USB_MODE_UNPLUG) {
			ret = xr_usb_start_adsp_host(xr_usb, false);
			if (ret)
				dev_err(xr_usb->dev,
					"failed to stop adsp host\n");
			ret = xr_usb_start_host(xr_usb, false, usb_event);
			next_state = USB_STATE_IDLE;
		}
		break;
	case USB_STATE_DYNAMIC_DIS:
		next_state = USB_STATE_DYNAMIC_DIS;
		ret = 0;
		break;
	default:
		dev_err(xr_usb->dev, "do nothing in state undefined\n");
		break;
	}

	xr_usb_adsp_state_update_notify(xr_usb, usb_event.mode);

	if (ret == 0) {
		xr_usb->usb_state = next_state;
		xr_usb->current_evt = usb_event;
	} else {
		dev_err(xr_usb->dev,
			"failed to handle usbevent at state %s ret - %d\n",
			xr_usb_state_to_name(xr_usb->usb_state), ret);
	}

	return ret;
}

static int hpd_event_process(struct xring_usb *xr_usb,
			     struct dp_event_info hpd_event)
{
	int ret;

	if (xr_usb->usb_state != USB_STATE_HOST) {
		dev_err(xr_usb->dev, "receive hpd evt but usb is not host\n");
		return -EINVAL;
	}

	xr_usb->current_evt.dp_hpd = hpd_event.dp_hpd;

	ret = xr_hpd_notifier(xr_usb, hpd_event.dp_hpd);
	if (ret)
		dev_err(xr_usb->dev, "dp process hpd_evt:%d failed\n",
			hpd_event.dp_hpd);

	return ret;
}

static int dynamic_event_process(struct xring_usb *xr_usb,
				 struct dynamic_event_info event)
{
	enum xr_usb_state next_state = USB_STATE_UNDEFINED;
	struct usb_event_info off_event = { 0 };
	int ret = -EINVAL;

	off_event.type = XR_USB_EVENT_TYPE_USBDP;
	off_event.mode = USB_MODE_UNPLUG;

	switch (xr_usb->usb_state) {
	case USB_STATE_IDLE:
		if (event.enable)
			break;

		ret = 0;
		next_state = USB_STATE_DYNAMIC_DIS;
		break;
	case USB_STATE_PERIPHERAL:
		next_state = USB_STATE_DYNAMIC_DIS;
		if (!event.enable)
			ret = xr_usb_start_peripheral(xr_usb, false, off_event);
		break;
	case USB_STATE_HOST:
		next_state = USB_STATE_DYNAMIC_DIS;
		if (!event.enable)
			ret = xr_usb_start_host(xr_usb, false, off_event);
		break;
	case USB_STATE_ADSP_HOST:
		next_state = USB_STATE_DYNAMIC_DIS;
		if (!event.enable) {
			ret = xr_usb_start_adsp_host(xr_usb, false);
			if (ret)
				dev_err(xr_usb->dev,
					"failed to stop adsp host\n");
			ret = xr_usb_start_host(xr_usb, false, off_event);
		}
		break;
	case USB_STATE_DYNAMIC_DIS:
		if (!event.enable)
			break;

		if (xr_usb->current_evt.mode == USB_MODE_UNPLUG) {
			ret = 0;
			next_state = USB_STATE_IDLE;
		} else if (xr_usb->current_evt.mode == USB_MODE_PERIPEHRAL) {
			ret = xr_usb_start_peripheral(xr_usb, true,
						      xr_usb->current_evt);
			next_state = USB_STATE_PERIPHERAL;
		} else if (xr_usb->current_evt.mode == USB_MODE_HOST) {
			ret = xr_usb_start_host(xr_usb, true,
						xr_usb->current_evt);
			next_state = USB_STATE_HOST;
		}
		break;
	default:
		dev_err(xr_usb->dev, "do nothing in state %d\n",
			xr_usb->usb_state);
		break;
	}

	if (ret == 0) {
		xr_usb->usb_state = next_state;
		if (xr_usb->usb_state == USB_STATE_DYNAMIC_DIS)
			atomic_set(&xr_usb->dynamic_mode, DYNAMIC_DISABLED);
		else
			atomic_set(&xr_usb->dynamic_mode, DYNAMIC_ENABLED);
	} else {
		dev_err(xr_usb->dev,
			"failed to handle usbevent at state %s ret - %d\n",
			xr_usb_state_to_name(xr_usb->usb_state), ret);
	}

	return ret;
}

static int adsp_event_process(struct xring_usb *xr_usb,
			      struct adsp_event_info adsp_event)
{
	enum xr_usb_state next_state = USB_STATE_UNDEFINED;
	struct usb_event_info off_event = { 0 };
	bool host_offon = false;
	int ret = -EINVAL;

	switch (xr_usb->usb_state) {
	case USB_STATE_HOST:
		if (adsp_event.start) {
			ret = xr_usb_start_adsp_host(xr_usb, true);
			next_state = USB_STATE_ADSP_HOST;
			if (!ret)
				break;

			host_offon = true;
		}
		break;
	case USB_STATE_ADSP_HOST:
		if (!adsp_event.start || adsp_event.exception) {
			ret = xr_usb_start_adsp_host(xr_usb, false);
			if (ret)
				dev_err(xr_usb->dev,
					"failed to stop adsp host\n");

			host_offon = true;
		}
		break;
	default:
		dev_err(xr_usb->dev, "do nothing in state %d\n",
			xr_usb->usb_state);
		break;
	}

	if (host_offon) {
		off_event.type = XR_USB_EVENT_TYPE_USBDP;
		off_event.mode = USB_MODE_UNPLUG;

		/* Start On/Off for switch failed */
		next_state = USB_STATE_HOST;
		ret = xr_usb_start_host(xr_usb, false, off_event);
		if (ret) {
			dev_err(xr_usb->dev, "failed to stop host\n");
			ret = 0;
		} else {
			next_state = USB_STATE_IDLE;
			ret = xr_usb_start_host(xr_usb, true, xr_usb->current_evt);
			if (ret) {
				dev_err(xr_usb->dev, "failed to start host\n");
				ret = 0;
			} else {
				next_state = USB_STATE_HOST;
			}
		}
	}

	if (ret == 0)
		xr_usb->usb_state = next_state;
	else
		dev_err(xr_usb->dev,
			"failed to handle usbevent at state %s ret - %d\n",
			xr_usb_state_to_name(xr_usb->usb_state), ret);

	return ret;
}

static int xr_usb_check_force_role(struct xring_usb *xr_usb,
				   union xr_usbdp_event input_event)
{
	if (xr_usb->force_role == USB_ROLE_NONE)
		return 0;

	if (xr_usb->usb_state != USB_STATE_IDLE)
		return -ENOENT;

	if (USBDP_EVENT_TYPE(input_event.data) != XR_USB_EVENT_TYPE_USBDP)
		return -EINVAL;

	if (input_event.usb.mode == USB_MODE_PERIPEHRAL &&
	    xr_usb->force_role == USB_ROLE_DEVICE)
		return 0;

	if (input_event.usb.mode == USB_MODE_HOST &&
	    xr_usb->force_role == USB_ROLE_HOST)
		return 0;

	return -EINVAL;
}

static void xr_usb_get_redriver(struct xring_usb *xr_usb)
{
	int retry_max = 5;

	while (retry_max--) {
		xr_usb->redriver =
			usb_get_redriver_by_phandle(xr_usb->dev->of_node,
						    "ssusb_redriver", 0);
		if (!IS_ERR(xr_usb->redriver))
			break;

		xr_usb->redriver = NULL;
	}

	if (!xr_usb->redriver)
		dev_err(xr_usb->dev, "failed to get redriver\n");
}

static void xr_usb_init_enter_lpmode(struct xring_usb *xr_usb)
{
	int ret;

	if (xr_usb->lpstate == LP_P4PG) {
		ret = xr_usb_enter_standby(xr_usb);
		if (ret) {
			dev_err(xr_usb->dev, "init enter standby fail\n");
			xr_usb_plat_powerdown(xr_usb->plat, xr_usb);
		}
	} else if (xr_usb->lpstate == LP_POWERDOWN) {
		xr_usb_plat_powerdown(xr_usb->plat, xr_usb);
	}
}

static void xr_usb_state_work(struct work_struct *work)
{
	struct xring_usb *xr_usb =
		container_of(work, struct xring_usb, state_work.work);
	union xr_usbdp_event input_event;
	int ret;

	if (xr_usb->support_usb_adsp && !xr_usb->usb_adsp_ops) {
		dev_info(xr_usb->dev, "delay state work for usb adsp\n");
		if (!queue_delayed_work(system_wq,
					&xr_usb->state_work,
					USB_STATE_WORK_DELAY)) {
			dev_warn(xr_usb->dev, "state work already on queue\n");
		}
		return;
	}

	if (xr_usb->usb_state == USB_STATE_POWERDOWN) {
		xr_usb_init_enter_lpmode(xr_usb);
		xr_usb->usb_state = USB_STATE_IDLE;
		dev_info(xr_usb->dev, "usb enter lpmode\n");
	}

	if (xr_usb->has_usb_redriver && !xr_usb->redriver)
		xr_usb_get_redriver(xr_usb);

	while (kfifo_out_spinlocked(&xr_usb->input_event_fifo, &input_event, 1,
				    &xr_usb->lock)) {
		ret = -EINVAL;

		dev_info(xr_usb->dev, "handle event data:%x\n",
			 input_event.data);

		if (xr_usb_check_force_role(xr_usb, input_event)) {
			dev_warn(xr_usb->dev, "force filter event:%x, ret:%d\n",
				 input_event.data, ret);
			return;
		}

		switch (USBDP_EVENT_TYPE(input_event.data)) {
		case XR_USB_EVENT_TYPE_USBDP:
			ret = usbdp_event_process(xr_usb, input_event.usb);
			break;
		case XR_USB_EVENT_TYPE_DP:
			ret = hpd_event_process(xr_usb, input_event.dp);
			break;
		case XR_USB_EVENT_TYPE_DYNAMIC:
			ret = dynamic_event_process(xr_usb,
						    input_event.dynamic);
			break;
		case XR_USB_EVENT_TYPE_ADSP:
			ret = adsp_event_process(xr_usb, input_event.adsp);
			break;
		default:
			break;
		}
		if (ret)
			dev_err(xr_usb->dev, "fail event data:%x, ret:%d\n",
				input_event.data, ret);
	}
}

static const char *const xr_usb_state_names[] = {
	[USB_STATE_UNDEFINED] = "undefined",
	[USB_STATE_POWERDOWN] = "powerdown",
	[USB_STATE_IDLE] = "idle",
	[USB_STATE_PERIPHERAL] = "peripheral",
	[USB_STATE_HOST] = "host",
	[USB_STATE_ADSP_HOST] = "adsp host",
	[USB_STATE_DYNAMIC_DIS] = "dynamic disabled",
};

const char *xr_usb_state_to_name(enum xr_usb_state state)
{
	if (state < 0 || state >= USB_STATE_MAX)
		return "invalid";

	return xr_usb_state_names[state];
}

static void xr_usb_state_init(struct xring_usb *xr_usb)
{
	const char *state = NULL;
	enum xr_usb_state init_state = USB_STATE_IDLE;
	int ret;

	xr_usb->usb_state = USB_STATE_POWERDOWN;
	atomic_set(&xr_usb->dynamic_mode, DYNAMIC_ENABLED);

	ret = device_property_read_string(xr_usb->dev, "init-state", &state);
	if (ret < 0) {
		dev_info(xr_usb->dev, "queue work for enter lpmode\n");
		WARN_ON(!queue_delayed_work(system_wq, &xr_usb->state_work,
					    USB_INIT_STATE_DELAY));
		return;
	}

	ret = match_string(xr_usb_state_names, ARRAY_SIZE(xr_usb_state_names),
			   state);
	if (ret < 0)
		return;

	init_state = ret;
	if (init_state == USB_STATE_PERIPHERAL)
		ret = xr_usb_queue_device_on(xr_usb, ORIG_NORMAL, USB_MODE,
					     false);
	else if (init_state == USB_STATE_HOST)
		ret = xr_usb_queue_host_on(xr_usb, ORIG_NORMAL, USB_MODE,
					   false);
	else
		ret = -EINVAL;

	/* Init debug state */
	if (ret == 0) {
		xr_usb->dbg_set_evt.usbdp_mode = USB_MODE;
		xr_usb->dbg_set_evt.orig = ORIG_NORMAL;
		xr_usb->dbg_set_evt.hpd = 0;
	} else {
		dev_err(xr_usb->dev, "failed to queue init state event\n");
	}
}

static void xr_usb_get_properties(struct xring_usb *xr_usb)
{
	int ret;
	unsigned int usbtrdtim = 0;

	ret = device_property_read_u32(xr_usb->dev, "dwc3,usbtrdtim_quirk",
				       &usbtrdtim);
	if (!ret) {
		dev_info(xr_usb->dev, "set usbtrdtim quirk\n");
		xr_usb->usbtrdtim = usbtrdtim;
	}

	xr_usb->is_fpga = device_property_read_bool(xr_usb->dev, "fpga");
	xr_usb->use_role_switch =
		device_property_read_bool(xr_usb->dev, "usb-role-switch");
	xr_usb->support_usb_adsp =
		device_property_read_bool(xr_usb->dev, "support-usb-adsp");
	xr_usb->has_usb_redriver =
		device_property_read_bool(xr_usb->dev, "ssusb_redriver");
	if (xr_usb->has_usb_redriver)
		dev_info(xr_usb->dev, "has usb redriver\n");
}

static void xr_usb_update_eye_from_property(struct xring_usb *xr_usb)
{
	int ret;

	ret = device_property_read_u32_array(xr_usb->dev, "u2-eye-param-device",
					     xr_usb->device_u2eye,
					     HSPHY_EYE_PARAM_MAX);
	if (ret < 0) {
		dev_err(xr_usb->dev,
			"failed to get u2 device phy para_value!\n");
		xr_usb->u2phy_d_eye_set = false;
	} else {
		xr_usb->u2phy_d_eye_set = true;
	}

	ret = device_property_read_u32_array(xr_usb->dev, "u2-eye-param-host",
					     xr_usb->host_u2eye,
					     HSPHY_EYE_PARAM_MAX);
	if (ret < 0) {
		dev_err(xr_usb->dev, "failed to get u2 host phy para_value!\n");
		xr_usb->u2phy_h_eye_set = false;
	} else {
		xr_usb->u2phy_h_eye_set = true;
	}

	ret = device_property_read_u32_array(xr_usb->dev, "u3-eye-param-device",
					     xr_usb->device_u3eye,
					     COMBOPHY_EYE_PARAM_MAX);
	if (ret < 0) {
		dev_err(xr_usb->dev,
			"failed to get u3 device phy para_value!\n");
		xr_usb->u3phy_d_eye_set = false;
	} else {
		xr_usb->u3phy_d_eye_set = true;
	}

	ret = device_property_read_u32_array(xr_usb->dev, "u3-eye-param-host",
					     xr_usb->host_u3eye,
					     COMBOPHY_EYE_PARAM_MAX);
	if (ret < 0) {
		dev_err(xr_usb->dev, "failed to get u3 host phy para_value!\n");
		xr_usb->u3phy_h_eye_set = false;
	} else {
		xr_usb->u3phy_h_eye_set = true;
	}
}

struct xring_usb *xr_usb_port_of_get(const struct device_node *node)
{
	struct device_node *np = NULL;
	struct platform_device *pdev = NULL;

	np = of_parse_phandle(node, "xr-usb", 0);
	if (!np) {
		pr_err("xr-usb phandle not found\n");
		return NULL;
	}
	pdev = of_find_device_by_node(np);
	if (!pdev) {
		pr_err("xr-usb node not found\n");
		return NULL;
	}

	of_node_put(np);

	if (!try_module_get(THIS_MODULE)) {
		put_device(&pdev->dev);
		return NULL;
	}

	return platform_get_drvdata(pdev);
}
EXPORT_SYMBOL_GPL(xr_usb_port_of_get);

void xr_usb_port_of_put(struct xring_usb *xr_usb)
{
	if (!IS_ERR_OR_NULL(xr_usb)) {
		module_put(THIS_MODULE);
		put_device(xr_usb->dev);
	}
}
EXPORT_SYMBOL_GPL(xr_usb_port_of_put);

int xr_usbdp_ops_register(struct xring_usb *xr_usb, const struct xr_dp_ops *ops)
{
	if (IS_ERR_OR_NULL(xr_usb)) {
		pr_err("usbdp_ops_register no dev\n");
		return -ENODEV;
	}

	if (!ops) {
		pr_err("invalid xr_dp_ops\n");
		return -EINVAL;
	}

	xr_usb->dp_ops = ops;

	return 0;
}
EXPORT_SYMBOL_GPL(xr_usbdp_ops_register);

int xr_usbdp_ops_unregister(struct xring_usb *xr_usb)
{
	if (IS_ERR_OR_NULL(xr_usb)) {
		pr_err("usbdp_ops_unregister no dev\n");
		return -ENODEV;
	}

	xr_usb->dp_ops = NULL;

	return 0;
}
EXPORT_SYMBOL_GPL(xr_usbdp_ops_unregister);

int xr_usb_adsp_ops_register(struct xring_usb *xr_usb,
			     const struct xr_usb_adsp_ops *ops)
{
	if (IS_ERR_OR_NULL(xr_usb)) {
		pr_err("invalid dev\n");
		return -ENODEV;
	}

	if (!ops) {
		pr_err("invalid ops\n");
		return -EINVAL;
	}

	spin_lock(&xr_usb->lock);
	xr_usb->changing_ops = true;
	spin_unlock(&xr_usb->lock);

	flush_delayed_work(&xr_usb->state_work);

	xr_usb->usb_adsp_ops = ops;

	spin_lock(&xr_usb->lock);
	xr_usb->changing_ops = false;
	spin_unlock(&xr_usb->lock);

	return 0;
}
EXPORT_SYMBOL_GPL(xr_usb_adsp_ops_register);

void xr_usb_adsp_ops_unregister(struct xring_usb *xr_usb)
{
	if (IS_ERR_OR_NULL(xr_usb)) {
		pr_err("invalid dev\n");
		return;
	}

	spin_lock(&xr_usb->lock);
	xr_usb->changing_ops = true;
	spin_unlock(&xr_usb->lock);

	flush_delayed_work(&xr_usb->state_work);
	xr_usb->usb_adsp_ops = NULL;

	spin_lock(&xr_usb->lock);
	xr_usb->changing_ops = false;
	spin_unlock(&xr_usb->lock);
}
EXPORT_SYMBOL_GPL(xr_usb_adsp_ops_unregister);

static void xr_usb_udev_add(struct xring_usb *xr_usb, struct usb_device *udev)
{
	/* is root hub */
	if (!udev->parent)
		return;

	/* is behind hub */
	if (udev->parent->parent)
		return;

	if (!udev->actconfig) {
		pr_err("udev has no active configuration\n");
		return;
	}

	if (udev->speed > xr_usb->otg_speed) {
		pr_info("connect to %s udev\n", usb_speed_string(udev->speed));
		xr_usb->otg_speed = udev->speed;
	}
}

static void xr_usb_udev_remove(struct xring_usb *xr_usb,
			       struct usb_device *udev)
{
	/* is root hub */
	if (!udev->parent)
		return;

	/* is behind hub */
	if (udev->parent->parent)
		return;

	xr_usb->otg_speed = USB_SPEED_UNKNOWN;
}

static int xr_usb_udev_notify(struct notifier_block *self, unsigned long action,
			      void *dev)
{
	struct xring_usb *xr_usb =
		container_of(self, struct xring_usb, udev_nb);

	switch (action) {
	case USB_DEVICE_ADD:
		xr_usb_udev_add(xr_usb, dev);
		break;
	case USB_DEVICE_REMOVE:
		xr_usb_udev_remove(xr_usb, dev);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static ssize_t otg_speed_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%s\n",
			usb_speed_string(xr_usb->otg_speed));
}

static ssize_t dynamic_mode_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			atomic_read(&xr_usb->dynamic_mode));
}

static ssize_t dynamic_mode_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);
	int mode = 0;

	if (kstrtoint(buf, 10, &mode))
		return -EINVAL;

	pr_info("queue dynamic event %d\n", mode);
	if (mode != DYNAMIC_ENABLED && mode != DYNAMIC_DISABLED)
		return -EINVAL;

	if (xr_usb_queue_dynamic_event(xr_usb, (mode == DYNAMIC_ENABLED)))
		pr_err("failed to queue dynamic event\n");

	return count;
}

static ssize_t force_role_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);
	static const char *const usb_role_strings[] = {
		"none", "host", "device"
	};

	return snprintf(buf, PAGE_SIZE, "%s\n",
			usb_role_strings[xr_usb->force_role]);
}

static ssize_t force_role_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);
	int role = 0;

	if (kstrtoint(buf, 10, &role))
		return -EINVAL;

	pr_info("force role to %d\n", role);
	if (role < USB_ROLE_NONE || role > USB_ROLE_DEVICE)
		return -EINVAL;

	xr_usb->force_role = role;

	return count;
}

static struct device_attribute xr_usb_attrs[] = {
	__ATTR_RO(otg_speed),
	__ATTR_RW(dynamic_mode),
	__ATTR_RW(force_role),
};

static int xr_usb_sysfs_init(struct device *dev)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(xr_usb_attrs); i++) {
		ret = device_create_file(dev, &xr_usb_attrs[i]);
		if (ret)
			goto err;
	}

	return 0;

err:
	for (i--; i >= 0; i--)
		device_remove_file(dev, &xr_usb_attrs[i]);

	return ret;
}

static void xr_usb_sysfs_exit(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(xr_usb_attrs); i++)
		device_remove_file(dev, &xr_usb_attrs[i]);
}

static const unsigned int xr_dwc3_extcon_cable[] = {
	EXTCON_USB,
	EXTCON_USB_HOST,
	EXTCON_NONE,
};

static int xr_usb_register_dwc3_extcon(struct xring_usb *xr_usb)
{
	int ret;

	if (xr_usb->use_role_switch)
		return 0;

	xr_usb->dwc3_edev =
		devm_extcon_dev_allocate(xr_usb->dev, xr_dwc3_extcon_cable);
	if (IS_ERR(xr_usb->dwc3_edev)) {
		dev_err(xr_usb->dev, "failed to allocate memory for extcon\n");
		ret = PTR_ERR(xr_usb->dwc3_edev);
		xr_usb->dwc3_edev = NULL;
		return ret;
	}

	ret = devm_extcon_dev_register(xr_usb->dev, xr_usb->dwc3_edev);
	if (ret) {
		dev_err(xr_usb->dev, "failed to register extcon device\n");
		return ret;
	}

	return 0;
}

static int xr_usb_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct xring_usb *xr_usb = NULL;
	const struct xring_usb_plat *plat_data = NULL;
	int ret;

	BUILD_BUG_ON(ARRAY_SIZE(xr_usb_state_names) != USB_STATE_MAX);

	if (!node)
		return -ENOENT;

	plat_data = of_device_get_match_data(dev);
	if (!plat_data) {
		dev_err(dev, "plat_data is null\n");
		return -ENOENT;
	}

	xr_usb = devm_kzalloc(dev, sizeof(*xr_usb) + plat_data->priv_size,
			      GFP_KERNEL);
	if (!xr_usb)
		return -ENOMEM;

	xr_usb->dev = dev;
	xr_usb->plat = plat_data;
	spin_lock_init(&xr_usb->lock);
	INIT_DELAYED_WORK(&xr_usb->state_work, xr_usb_state_work);
	INIT_KFIFO(xr_usb->input_event_fifo);

	pm_runtime_no_callbacks(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0)
		goto err_pm_put;

	pm_runtime_forbid(dev);

	xr_usb_get_properties(xr_usb);

	xr_usb->sys_reset =
		devm_reset_control_get_shared(dev, "usbsys_apb_reset");
	if (IS_ERR(xr_usb->sys_reset)) {
		dev_err(dev, "get sysreset failed\n");
		ret = PTR_ERR(xr_usb->sys_reset);
		goto err_pm_allow;
	}

	xr_usb->vbus_valid =
		devm_reset_control_get_shared(dev, "usb_vbus_valid");
	if (IS_ERR(xr_usb->vbus_valid)) {
		dev_err(dev, "get vbus_valid failed\n");
		ret = PTR_ERR(xr_usb->vbus_valid);
		goto err_pm_allow;
	}

	xr_usb->controller_reset =
		devm_reset_control_get_shared(dev, "usb_controller_reset");
	if (IS_ERR(xr_usb->controller_reset)) {
		dev_err(dev, "get controller_reset failed\n");
		ret = PTR_ERR(xr_usb->controller_reset);
		goto err_pm_allow;
	}

	ret = devm_clk_bulk_get_all(dev, &xr_usb->clks);
	if (ret < 0) {
		dev_err(dev, "get clks failed\n");
		goto err_pm_allow;
	}
	xr_usb->num_clks = ret;

	xr_usb->usb2_phy = xr_usb_devm_hsphy_get(dev, "usb2-phy");
	if (IS_ERR(xr_usb->usb2_phy)) {
		dev_err(dev, "no usb2 phy configured\n");
		ret = PTR_ERR(xr_usb->usb2_phy);
		goto err_pm_allow;
	}

	xr_usb->usb3_phy = xr_usb_devm_combophy_get(dev, "usb3-phy");
	if (IS_ERR(xr_usb->usb3_phy)) {
		dev_err(dev, "no usb3 phy configured\n");
		ret = PTR_ERR(xr_usb->usb3_phy);
		goto err_pm_allow;
	}

	ret = xr_usb_plat_probe(plat_data, xr_usb);
	if (ret) {
		dev_err(dev, "platform probe failed\n");
		goto err_pm_allow;
	}

	xr_usb_update_eye_from_property(xr_usb);

	ret = xr_usb_debugfs_init(xr_usb);
	if (ret < 0) {
		dev_err(dev, "debugfs init failed\n");
		goto err_plat_remove;
	}

	ret = xr_usb_sysfs_init(dev);
	if (ret) {
		dev_err(dev, "sysfs init failed\n");
		goto err_remove_debugfs;
	}

	ret = xr_usb_register_dwc3_extcon(xr_usb);
	if (ret) {
		dev_err(dev, "extcon init failed\n");
		goto err_remove_sysfs;
	}

	if (IS_ENABLED(CONFIG_PM))
		xr_usb->pm_notify.notifier_call = xr_usb_pm_notifier;

	platform_set_drvdata(pdev, xr_usb);

	xr_usb_init_gadget_ops(xr_usb);

	xr_usb_state_init(xr_usb);

	ret = register_pm_notifier(&xr_usb->pm_notify);
	if (ret)
		/* Do not fail the driver initialization, as suspend can
		 * be prevented in a later phase if needed
		 */
		dev_err(dev, "register_pm_notifier failed: %d\n", ret);

	xr_usb->udev_nb.notifier_call = xr_usb_udev_notify;
	usb_register_notify(&xr_usb->udev_nb);
	pm_runtime_put(dev);

	dev_info(dev, "probe finished\n");

	return 0;

err_remove_sysfs:
	xr_usb_sysfs_exit(xr_usb->dev);
err_remove_debugfs:
	xr_usb_debugfs_exit(&xr_usb->debugfs_root);
err_plat_remove:
	xr_usb_plat_remove(plat_data, xr_usb);
err_pm_allow:
	pm_runtime_allow(dev);

err_pm_put:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);

	dev_err(dev, "probe failed - %d\n", ret);
	return ret;
}

static void xr_usb_teardown(struct xring_usb *xr_usb)
{
	struct usb_event_info event = { 0 };

	pm_runtime_get_sync(xr_usb->dev);

	xr_usb_sysfs_exit(xr_usb->dev);
	usb_unregister_notify(&xr_usb->udev_nb);
	unregister_pm_notifier(&xr_usb->pm_notify);

	xr_usb_debugfs_exit(&xr_usb->debugfs_root);
	spin_lock(&xr_usb->lock);
	xr_usb->removing = true;
	spin_unlock(&xr_usb->lock);
	cancel_delayed_work_sync(&xr_usb->state_work);

	event.mode = USB_MODE_UNPLUG;
	if (xr_usb->usb_state == USB_STATE_PERIPHERAL) {
		(void)xr_usb_start_peripheral(xr_usb, false, event);
	} else if (xr_usb->usb_state == USB_STATE_HOST) {
		(void)xr_usb_start_host(xr_usb, false, event);
	} else if (xr_usb->usb_state == USB_STATE_ADSP_HOST) {
		(void)xr_usb_start_adsp_host(xr_usb, false);
		(void)xr_usb_start_host(xr_usb, false, event);
	}

	if (xr_usb->dwc3) {
		of_platform_depopulate(xr_usb->dev);
		xr_usb->dwc3 = NULL;
	}
	xr_usb->usb_state = USB_STATE_UNDEFINED;
	xr_usb_exit_gadget_ops(xr_usb);

	xr_usb_plat_remove(xr_usb->plat, xr_usb);

	pm_runtime_disable(xr_usb->dev);
	pm_runtime_put_noidle(xr_usb->dev);
	pm_runtime_set_suspended(xr_usb->dev);
	usb_put_redriver(xr_usb->redriver);
}

static int xr_usb_remove(struct platform_device *pdev)
{
	struct xring_usb *xr_usb = platform_get_drvdata(pdev);

	dev_info(xr_usb->dev, "enter\n");
	xr_usb_teardown(xr_usb);
	dev_info(xr_usb->dev, "remove finished\n");

	return 0;
}

static void xr_usb_shutdown(struct platform_device *pdev)
{
	struct xring_usb *xr_usb = platform_get_drvdata(pdev);

	dev_info(xr_usb->dev, "enter\n");
	xr_usb_teardown(xr_usb);
	dev_info(xr_usb->dev, "shutdown finished\n");
}

#ifdef CONFIG_PM_SLEEP
static int xr_usb_suspend(struct device *dev)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);
	int ret;

	dev_info(dev, "suspend in state:%s\n",
		 xr_usb_state_to_name(xr_usb->usb_state));

	if (xr_usb->usb_state == USB_STATE_ADSP_HOST)
		return 0;

	ret = xr_usb_exit_standby(xr_usb);
	if (ret)
		dev_err(xr_usb->dev, "suspend failed to exit standby\n");

	ret = xr_usb_plat_suspend(xr_usb->plat, xr_usb);
	if (ret)
		dev_err(dev, "plat suspend fail\n");

	xr_usb->usb_state = USB_STATE_POWERDOWN;

	dev_info(dev, "suspend finish\n");

	return 0;
}

static int xr_usb_resume(struct device *dev)
{
	struct xring_usb *xr_usb = dev_get_drvdata(dev);
	int ret;

	dev_info(dev, "resume in state:%s\n",
		 xr_usb_state_to_name(xr_usb->usb_state));

	if (xr_usb->usb_state == USB_STATE_ADSP_HOST)
		return 0;

	ret = xr_usb_plat_resume(xr_usb->plat, xr_usb);
	if (ret)
		dev_err(dev, "plat resume fail\n");

	dev_info(dev, "resume finish\n");

	return 0;
}

static const struct dev_pm_ops xr_usb_pm_ops = { SET_SYSTEM_SLEEP_PM_OPS(
	xr_usb_suspend, xr_usb_resume) };

#define XRUSB_PM_OPS (&xr_usb_pm_ops)
#else
#define XRUSB_PM_OPS NULL
#endif /* CONFIG_PM_SLEEP */

static const struct of_device_id xr_usb_of_match[] = {
	{ .compatible = "xring,o1,usb", SET_XR_USB_O1_DATA() },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, xr_usb_of_match);

static struct platform_driver xr_usb_driver = {
	.probe	= xr_usb_probe,
	.remove	= xr_usb_remove,
	.shutdown = xr_usb_shutdown,
	.driver	= {
		.name	= "xring_usb",
		.pm	= XRUSB_PM_OPS,
		.of_match_table = xr_usb_of_match,
	},
};
module_platform_driver(xr_usb_driver);

MODULE_AUTHOR("Dongliang Wang <wangdongliang6@xiaomi.com>");
MODULE_AUTHOR("LingHan Wei <weilinghan@xiaomi.com>");
MODULE_AUTHOR("Huinan Gu <guhuinan@xiaomi.com>");
MODULE_AUTHOR("Yu Chen <chenyu45@xiaomi.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("DesignWare DWC3 XRING Glue Driver");
MODULE_SOFTDEP("pre: xr_doze");
