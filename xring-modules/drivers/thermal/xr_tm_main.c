// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include "xr_tm_main.h"

static int __init xr_thermal_init(void)
{
	xr_soc_tm_init();
	xr_virt_tm_init();
	xr_cpu_hotplug_cdev_init();
	xr_cpu_pause_cdev_init();
	xr_cpufreq_cdev_init();
	xr_board_tm_init();
	walt_thermal_init();
	return 0;
}

static void __exit xr_thermal_exit(void)
{
	xr_virt_tm_exit();
	xr_soc_tm_exit();
	xr_board_tm_exit();
	xr_cpu_hotplug_cdev_exit();
	xr_cpu_pause_cdev_exit();
	xr_cpufreq_cdev_exit();
}

module_init(xr_thermal_init);
module_exit(xr_thermal_exit);

MODULE_SOFTDEP("pre: xr_pmic xr_cpufreq dse");
MODULE_AUTHOR("Chongchong Ding <dingchongchong@xiaomi.com>");
MODULE_DESCRIPTION("XRing thermal driver");
MODULE_LICENSE("GPL v2");
