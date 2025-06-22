// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: dfx-switch driver
 */

#define pr_fmt(fmt)	"[xr_dfx][dfx_switch]:%s:%d " fmt, __func__, __LINE__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/string.h>
#include <soc/xring/dfx_switch.h>

#define DFX_SWITCH_LEN		255

const char *dfxswitch_cmd = "dfxswitch=";
static char dfxswitch[DFX_SWITCH_LEN + 1] = {'\0'};

bool is_switch_on(u32 switch_name)
{
	bool ret = false;

	BUILD_BUG_ON(SWITCH_MAX > DFX_SWITCH_LEN);

	if (switch_name >= SWITCH_MAX)
		return false;

	if (dfxswitch[switch_name] == '1')
		ret = true;

	return ret;
}
EXPORT_SYMBOL(is_switch_on);

int dfx_switch_driver_init(void)
{
	int ret = 0;
	struct device_node *np = NULL;
	const char *bootargs = NULL;
	char *cmdline = NULL;
	char *cmdline_start = NULL;
	char *dfxswitch_start = dfxswitch;
	u32 switch_cnt = 0;

	pr_info("dfx_switch module init\n");

	np = of_find_node_by_path("/chosen");
	if (!np) {
		pr_err("find chosen node fail\n");
		return -ENODEV;
	}

	ret = of_property_read_string(np, "bootargs", &bootargs);
	if (ret) {
		pr_err("read bootargs fail\n");
		return ret;
	}

	cmdline = kstrdup(bootargs, GFP_KERNEL);
	if (!cmdline)
		return -ENOMEM;

	pr_info("start handle cmdline\n");
	cmdline_start = strstr(cmdline, dfxswitch_cmd);
	if (!cmdline_start) {
		pr_err("find dfxswitch cmdline not exist\n");
		return -EINVAL;
	}

	cmdline_start += strlen(dfxswitch_cmd);
	while ((*cmdline_start == '1') || (*cmdline_start == '0')) {
		*dfxswitch_start++ = *cmdline_start++;
		switch_cnt++;
		if (switch_cnt >= DFX_SWITCH_LEN)
			break;
	}

	pr_info("dfx_switch state:%s\n", dfxswitch);

	kfree(cmdline);

	return ret;
}

void dfx_switch_driver_exit(void)
{
	pr_info("dfx_switch module exit\n");
}
