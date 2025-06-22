// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023-2024, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)	"[xr_dfx][reboot_reason]:%s:%d " fmt, __func__, __LINE__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include "dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h"

static void reboot_comm(void *cmd)
{
	unsigned int i;
	unsigned int curr_reboot_type = ADB_REBOOT;
	const struct reboot_reason_word *reboot_reason_map = NULL;

	if (cmd == NULL)
		cmd = "ADB_REBOOT";

	pr_info("cmd = %s\n", (char *) cmd ? : "NULL");

	reboot_reason_map = get_reboot_reason_map();
	if (reboot_reason_map == NULL) {
		pr_info("reboot_reason_map is NULL\n");
		return;
	}

	for (i = 0; i < get_reboot_reason_map_size(); i++) {
		if (!strncmp((char *)reboot_reason_map[i].name, cmd,
					sizeof(reboot_reason_map[i].name))) {
			curr_reboot_type = reboot_reason_map[i].num;
			break;
		}
	}

	if (!strncmp(cmd, "boringssl-self-check-failed", MDR_REBOOTREASONWORD_MAXLEN))
		curr_reboot_type = BORINGSSL_FAILED;
	else if (!strncmp(cmd, "apexd-failed", MDR_REBOOTREASONWORD_MAXLEN))
		curr_reboot_type = APEXD_FAILED;
	else if (!strncmp(cmd, "bootloader,bootstrap-apexd-failed", MDR_REBOOTREASONWORD_MAXLEN))
		curr_reboot_type = BOOTSTRAP_APEXD_FAILED;
	else if (!strncmp(cmd, "vold-failed", MDR_REBOOTREASONWORD_MAXLEN))
		curr_reboot_type = VOLD_FAILED;
	else if (!strncmp(cmd, "dm-verity_device_corrupted", MDR_REBOOTREASONWORD_MAXLEN))
		curr_reboot_type = DM_VERITY_FAILED;

	set_reboot_reason(curr_reboot_type);
}

static int xr_reboot_notify(struct notifier_block *nb, unsigned long event, void *cmd)
{
	reboot_comm(cmd);

	return NOTIFY_DONE;
}

static struct notifier_block xr_reboot_nb = {
	.notifier_call = xr_reboot_notify,
	.priority = 130,
};

int reboot_reason_init(void)
{
	int ret = 0;

	pr_info("==>start\n");

	ret = register_restart_handler(&xr_reboot_nb);
	if (ret)
		pr_err(" cannot register xr_reboot_nb!!!!\n");

	return ret;
}

void reboot_reason_exit(void)
{
	pr_info("===>exit\n");
}
