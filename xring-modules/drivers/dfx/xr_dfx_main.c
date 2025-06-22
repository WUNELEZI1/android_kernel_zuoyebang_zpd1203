// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: xr dfx driver
 */

#include <linux/module.h>
#include "xr_dfx_main.h"

static int __init dfx_init(void)
{
	int ret;

	ret = dfx_switch_driver_init();
	if (ret) {
		pr_err("dfx_switch_driver_init fail, ret=%d\n", ret);
		return ret;
	}
	pr_info("dfx_switch init succ\n");

	ret = trace_hook_init();
	if (ret) {
		pr_err("trace_hook_init fail, ret=%d\n", ret);
		return ret;
	}
	pr_info("trace_hook init succ\n");

	ret = mdr_init();
	if (ret) {
		pr_err("mdr_init fail, ret=%d\n", ret);
		return ret;
	}
	pr_info("mdr init succ\n");

	/* put module depend on mdr after mdr_init */
	ret = reboot_reason_init();
	if (ret) {
		pr_err("reboot_reason_init fail, ret=%d\n", ret);
		return ret;
	}
	pr_info("reboot_reason init succ\n");

	/* put logsave module below there */
	ret = fastbootlog_init();
	if (ret) {
		pr_err("fastbootlog_init fail, ret=%d\n", ret);
		return ret;
	}
	pr_info("fastbootlog init succ\n");

	ret = memdump_init();
	if (ret) {
		pr_err("memdump_init fail, ret=%d\n", ret);
		return ret;
	}
	pr_info("memdump init succ\n");

	pr_info("xr_dfx module init succ\n");
	return 0;
}

static void __exit dfx_exit(void)
{
	memdump_exit();
	fastbootlog_exit();
	reboot_reason_exit();
	dfx_switch_driver_exit();
	pr_info("dfx module exit succ\n");
}

MODULE_DESCRIPTION("XRing dfx driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre:dw_apb_rtc");
module_init(dfx_init);
module_exit(dfx_exit);
