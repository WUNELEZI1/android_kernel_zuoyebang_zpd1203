// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt) "[XRISP_DRV][%14s] %s(%d): " fmt, "top", __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include "xrisp_top.h"
#include "xrisp_debug.h"

static const struct xrisp_driver_submodule xrisp_driver_modules[] = {
	{ "cam-device-manager", xrisp_cdm_init, xrisp_cdm_exit },
	{ "cam-ipcm", xrisp_ipcm_init, xrisp_ipcm_exit },
	{ "cam-device-clk", xrisp_clk_init, xrisp_clk_exit },
	{ "cam-isp-rgltr", xrisp_regulator_init, xrisp_regulator_exit },
	{ "cam-remote-proc", xrisp_rproc_init, xrisp_rproc_exit },
	{ "cam-rpmsg-ping", xrisp_rpmsg_ping_init, xrisp_rpmsg_ping_exit },
	{ "cam-rpmsg-client", xrisp_rpmsg_epts_init, xrisp_rpmsg_epts_exit },
	{ "cam-buffer-manager", xrisp_cbm_init, xrisp_cbm_exit },
	{ "cam_regdump", xring_regdump_init, xring_regdump_exit },
	{ "cam-sensor", xrisp_sensor_init, xrisp_sensor_exit },
	{ "cam_privacy_led", xrisp_privacy_led_init, xrisp_privacy_led_exit},
	{ "cam-csiphy", xrisp_csiphy_init, xrisp_csiphy_exit },
	{ "cam-i2c-flash", xrisp_flash_init, xrisp_flash_exit },
	{ "cam-i2c-tof", xrisp_tof_init, xrisp_tof_exit },
	{ "cam-spi-tof", xrisp_multi_tof_init, xrisp_multi_tof_exit },
	{ "cam-ca-tee", xrisp_ca_init, xrisp_ca_exit },
	{ "edr-event", xring_edr_event_init, xring_edr_event_exit},
	{ "edr-wdt", xring_edr_wdt_init, xring_edr_wdt_exit},
	{ "edr-ramlog", xring_edr_ramlog_init, xring_edr_ramlog_exit},
#ifdef CONFIG_XRING_ISP_EDR_TRACE
	{ "edr-ramtrace", xring_edr_ramtrace_init, xring_edr_ramtrace_exit},
#endif
	{ "edr-mdr", xring_mdr_init, xring_mdr_exit},
	//{ "edr_thermal", xrisp_thermal_init, xrisp_thermal_exit},
};

static int __init xrisp_init(void)
{
	int ret = 0;
	int i = 0;

	pr_info("xrisp init");

	ret = xrisp_debugfs_init();
	if (ret) {
		pr_err("xrisp debugds init fail ret = %d", ret);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(xrisp_driver_modules); i++) {
		ret = xrisp_driver_modules[i].init();
		if (ret) {
			pr_err("xrisp module %s init fail",
				xrisp_driver_modules[i].submodule_name);
			goto modules_exit;
		}
		pr_info("register %s driver start",
			xrisp_driver_modules[i].submodule_name);
	}
	pr_info("xrisp init finish");
	return 0;

modules_exit:
	for (--i; i >= 0; i--)
		xrisp_driver_modules[i].exit();

	xrisp_debugfs_exit();

	return ret;
}

static void __exit xrisp_exit(void)
{
	int i = 0;

	pr_info("xrisp exit");

	for (i = ARRAY_SIZE(xrisp_driver_modules) - 1; i >= 0; i--)
		xrisp_driver_modules[i].exit();

	xrisp_debugfs_exit();
	pr_info("xrisp exit finish");
}

module_init(xrisp_init);
module_exit(xrisp_exit);

MODULE_IMPORT_NS(DMA_BUF);
MODULE_DESCRIPTION("x-ring isp driver");
MODULE_LICENSE("GPL v2");
