// SPDX-License-Identifier: GPL-2.0
/*
 *mca_xring_usb.c
 *
 *xring usb driver
 *
 * Copyright (c) 2024-2024 Xiaomi Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/of.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb/typec.h>
#include <linux/extcon-provider.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/power_supply.h>
#include <linux/of_gpio.h>

#include <mca/platform/platform_buckchg_class.h>
#include <mca/protocol/protocol_class.h>
#include <mca/protocol/protocol_pd_class.h>
#include <mca/common/mca_charge_interface.h>
#include <soc/xring/xr_usbdp_event.h>
#include <mca/common/mca_parse_dts.h>
#include <mca/platform/platform_cp_class.h>
#include <mca/common/mca_log.h>

#ifndef MCA_LOG_TAG
#define MCA_LOG_TAG "mca_xr_usb"
#endif

enum dr {
	DR_IDLE,
	DR_DEVICE,
	DR_HOST,
	DR_DEVICE_TO_HOST,
	DR_HOST_TO_DEVICE,
	DR_MAX,
};

struct mca_xring_usb {
	struct device *dev;
	enum dr usb_dr;
	struct xring_usb *xr_usb;
	struct delayed_work xr_usb_dwork;
};

struct mca_xring_usb *ppm_global = NULL;

static int get_cc_orientation(struct mca_xring_usb *ppm)
{
	int ret;
	int cc_orientation = 0;

	ret = protocol_class_pd_get_typec_cc_orientation(TYPEC_PORT_0,
							 &cc_orientation);
	if (cc_orientation != 0) {
		cc_orientation--;
		mca_log_info("%s:cc_orientation = %d\n", __func__,
			 cc_orientation);
	}
	mca_log_info("%s cc_orientation = %d !\n", __func__,
		 cc_orientation);
	if (ret)
		mca_log_err("%s:failed to get cc orientation\n",
			__func__);

	return cc_orientation;
}

static void xr_usb_dwork_handler(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct mca_xring_usb *ppm =
		container_of(dwork, struct mca_xring_usb, xr_usb_dwork);
	enum dr usb_dr = ppm->usb_dr;

	mca_log_info("%s !\n", __func__);

	switch (usb_dr) {
	case DR_HOST:
		if (xr_usb_queue_host_on(ppm->xr_usb, get_cc_orientation(ppm),
					 USB_MODE, DP_HPD_EVENT_PLUG_OUT))
			mca_log_err("%s failed to queue host on\n", __func__);
		break;
	default:
		break;
	}
}

int mca_xring_usb_host_on(enum dr dr)
{
	struct mca_xring_usb *ppm = ppm_global;

	ppm->usb_dr = dr;

	cancel_delayed_work_sync(&ppm->xr_usb_dwork);
	schedule_delayed_work(&ppm->xr_usb_dwork, 0);

	return 0;
}
EXPORT_SYMBOL_GPL(mca_xring_usb_host_on);

int mca_xring_usb_host_off(enum dr dr)
{
	struct mca_xring_usb *ppm = ppm_global;

	ppm->usb_dr = dr;

	cancel_delayed_work_sync(&ppm->xr_usb_dwork);
	if (xr_usb_queue_off(ppm->xr_usb))
		mca_log_err("%s failed to queue off\n", __func__);

	return 0;
}
EXPORT_SYMBOL_GPL(mca_xring_usb_host_off);

int mca_xr_usb_queue_host_on(uint8_t polarity, unsigned int dp_mode, enum dp_hpd_event hpd_event)
{
	struct mca_xring_usb *ppm = ppm_global;

	cancel_delayed_work_sync(&ppm->xr_usb_dwork);

	if (xr_usb_queue_off(ppm->xr_usb))
		mca_log_err("%s failed to queue off\n", __func__);

	if (xr_usb_queue_host_on(ppm->xr_usb, polarity, dp_mode, hpd_event)) {
		mca_log_err("%s failed to queue host on\n", __func__);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(mca_xr_usb_queue_host_on);

int mca_xr_usb_queue_dp_hpd(enum dp_hpd_event hpd_event)
{
	struct mca_xring_usb *ppm = ppm_global;

	if (xr_usb_queue_dp_hpd(ppm->xr_usb, hpd_event)) {
		mca_log_err("%s failed to queue hpd\n", __func__);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(mca_xr_usb_queue_dp_hpd);

int mca_xr_usb_data_role_to_device(void)
{
	struct mca_xring_usb *ppm = ppm_global;

	cancel_delayed_work_sync(&ppm->xr_usb_dwork);
	if (xr_usb_queue_off(ppm->xr_usb))
		mca_log_err("%s failed to queue off\n", __func__);
	if (xr_usb_queue_device_on(ppm->xr_usb, get_cc_orientation(ppm),
					USB_MODE, DP_HPD_EVENT_PLUG_OUT))
		mca_log_err("%s failed to queue device on\n", __func__);

	return 0;
}
EXPORT_SYMBOL_GPL(mca_xr_usb_data_role_to_device);

int mca_xr_usb_data_role_to_host(void)
{
	struct mca_xring_usb *ppm = ppm_global;

	cancel_delayed_work_sync(&ppm->xr_usb_dwork);
	if (xr_usb_queue_off(ppm->xr_usb))
		mca_log_err("%s failed to queue off\n", __func__);
	if (xr_usb_queue_host_on(ppm->xr_usb, get_cc_orientation(ppm),
				USB_MODE, DP_HPD_EVENT_PLUG_OUT))
		mca_log_err("%s failed to queue host on\n", __func__);

	return 0;
}
EXPORT_SYMBOL_GPL(mca_xr_usb_data_role_to_host);

static int mca_xring_usb_probe(struct platform_device *pdev)
{
	int ret = 0;
	static int probe_cnt = 0;
	struct mca_xring_usb *ppm = NULL;

	mca_log_err("%s probe_cnt = %d\n", __func__, ++probe_cnt);

	ppm = devm_kzalloc(&pdev->dev, sizeof(*ppm), GFP_KERNEL);
	if (!ppm)
		return -ENOMEM;

	ppm->dev = &pdev->dev;

	ppm->xr_usb = xr_usb_port_of_get(pdev->dev.of_node);
	if (!ppm->xr_usb) {
		mca_log_err("%s failed to get xr_usb\n", __func__);
		ret = -EPROBE_DEFER;
		return ret;
	} else {
		mca_log_info("%s get xr_usb successful !\n", __func__);
	}

	INIT_DELAYED_WORK(&ppm->xr_usb_dwork, xr_usb_dwork_handler);

	platform_set_drvdata(pdev, ppm);

	ppm_global = ppm;

	mca_log_err("%s %s!!\n", __func__, ret == -EPROBE_DEFER ?
			    "Over probe cnt max" : "OK");
	return 0;
}

static int mca_xring_usb_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct mca_xring_usb *ppm = platform_get_drvdata(pdev);

	if (!ppm)
		return -EINVAL;

	if (ppm->xr_usb) {
		xr_usb_port_of_put(ppm->xr_usb);
		ppm->xr_usb = NULL;
	}

	return ret;
}

static void mca_xring_usb_shutdown(struct platform_device *pdev)
{
	struct mca_xring_usb *ppm = platform_get_drvdata(pdev);

	mca_log_err("%s mca_xring_usb_shutdown 11\n",
				   __func__);
	if (!ppm)
		return;

	//ppm->shutdown_flag = true;

	return;
}

static const struct of_device_id mca_xring_usb_of_match[] = {
	{ .compatible = "mca,mca_xring_usb" },
	{ }
};
MODULE_DEVICE_TABLE(of, mca_xring_usb_of_match);

static struct platform_driver mca_xring_usb_driver = {
	.driver = {
		.name = "mca_xring_usb",
		.of_match_table = of_match_ptr(mca_xring_usb_of_match),
	},
	.probe = mca_xring_usb_probe,
	.remove = mca_xring_usb_remove,
	.shutdown = mca_xring_usb_shutdown,
};

static int __init mca_xring_usb_init(void)
{
	return platform_driver_register(&mca_xring_usb_driver);
}
late_initcall(mca_xring_usb_init);

static void __exit mca_xring_usb_exit(void)
{
	platform_driver_unregister(&mca_xring_usb_driver);
}
module_exit(mca_xring_usb_exit);

MODULE_DESCRIPTION("mca xring usb");
MODULE_AUTHOR("luguohong@xiaomi.com");
MODULE_LICENSE("GPL v2");