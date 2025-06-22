// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */
#include "../xrse_cmd.h"
#include "../xrse_internal.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <soc/xring/efuse.h>
#include "soc/xring/xrse/xrse.h"
#include "soc/xring/xrse/secboot_verify.h"
#include <dt-bindings/xring/platform-specific/xrse_platform.h>

#define OEM_SECBOOT_ENABLE                            (1)
#define SECDEBUG_ENABLE                               (0)
#define SECBOOT_CHECK_CMD_NUM_MAX                     (1)

int xrse_secboot_status_check(int cmd)
{
	int ret = 0;
	u32 data = 0;
	u32 field_id = 0;
	u8 enable_flag = 0;

	switch (cmd) {
	case XRSE_SHOW_SECDEBUG_ENABLE:
		xrse_info("get secdebug enable status\n");
		field_id = EFUSE_SECDEBUG_ENABLE;
		enable_flag = SECDEBUG_ENABLE;
		break;

	case XRSE_SHOW_OEM_SECBOOT_ENABLE:
		xrse_info("get OEM secboot enable status\n");
		field_id = EFUSE_OEM_SECBOOT_ENABLE;
		enable_flag = OEM_SECBOOT_ENABLE;
		break;

	default:
		xrse_err("invalid cmd:%d\n", cmd);
		return -EINVAL;
	}

	ret = bl31_efuse_read(field_id, sizeof(data), (u8 *)&data);
	if (ret) {
		xrse_err("bl31_efuse_read fail, ret:0x%x\n", ret);
		return -EINVAL;
	}

	return data == enable_flag ? 1 : 0;
}

static int xrse_secboot_status_check_show(int cmd)
{
	int ret = 0;
	int enable = 0;
	char buf[XRSE_RES_BUF_LENGTH_MAX] = {0};

	enable = xrse_secboot_status_check(cmd);
	if (enable < 0) {
		xrse_err("xrse_secboot_status_check fail, ret:%d\n", ret);
		return enable;
	}

	switch (cmd) {
	case XRSE_SHOW_SECDEBUG_ENABLE:
		ret = snprintf(buf, XRSE_RES_BUF_LENGTH_MAX, "secdebug is %s", enable == 1 ? "enable" : "disable");
		if (ret < 0) {
			xrse_err("snprintf res fail, cmd:%d, ret:0x%x\n", cmd, ret);
			return ret;
		}

		return xrse_cmd_res_store(buf, ret);

	case XRSE_SHOW_OEM_SECBOOT_ENABLE:
		ret = snprintf(buf, XRSE_RES_BUF_LENGTH_MAX, "oem secboot is %s",
						enable == 1 ? "enable" : "disable");
		if (ret < 0) {
			xrse_err("snprintf res fail, cmd:%d, ret:0x%x\n", cmd, ret);
			return ret;
		}

		return xrse_cmd_res_store(buf, ret);

	default:
		xrse_err("invalid cmd:%d\n", cmd);
		ret = -EINVAL;
	}

	return ret;
}

int xr_secboot_status_store(int argc, char *argv[])
{
	int ret = -EINVAL;
	unsigned long cmd = 0;

	if (argc < SECBOOT_CHECK_CMD_NUM_MAX) {
		xrse_err("invalid cmd num:%d\n", argc);
		return ret;
	}

	ret = kstrtoul(argv[0], 0, &cmd);
	if (ret) {
		xrse_err("secboot check get cmd fail. cmd:%s\n", argv[0]);
		return ret;
	}

	if (cmd >= XRSE_SHOW_CMD_MAX) {
		xrse_err("invalid cmd:%ld\n", cmd);
		return -EINVAL;
	}

	ret = xrse_secboot_status_check_show((int)cmd);
	if (ret) {
		xrse_err("status check fail. ret:%d\n", ret);
		return ret;
	}

	xrse_info("status check success. cmd:%lu\n", cmd);

	return 0;
}
