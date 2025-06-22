/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
 */

#ifndef __XRSE_SECBOOT_DRV_H__
#define __XRSE_SECBOOT_DRV_H__

#include <linux/types.h>

enum xrse_show_cmd {
	XRSE_SHOW_SECDEBUG_ENABLE,
	XRSE_SHOW_OEM_SECBOOT_ENABLE,
	XRSE_SHOW_CMD_MAX
};

struct sb_addr_info {
	u64 addr;
	u32 size;
	u32 in_ddr;
};

/*
 * Description: Check the status of secure boot.
 * Input: cmd: the specific status to check,
 * Return: < 0 failed; 1: enable; others: disable;
 */
int xrse_secboot_status_check(int cmd);

int xrse_secboot_verify(u8 cmd, u32 image_id, u32 subimage_id,
				const struct sb_addr_info *src, const struct sb_addr_info *dst);
int secboot_verify_ca_init(void);
void secboot_verify_ca_exit(void);

#endif
