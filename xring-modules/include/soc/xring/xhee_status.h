/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024-2025, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XHEE_STATUS_H__
#define __XHEE_STATUS_H__
#include <linux/printk.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>

#define XHEE_ENABLED  0xa5
#define XHEE_DISABLED 0x6b

#ifdef CONFIG_XRING_DEBUG
static inline bool xhee_status_enabled(void)
{
	struct arm_smccc_res res = { 0 };

	arm_smccc_smc(FID_XHEE_ENABLED_STATUS, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 == XHEE_ENABLED)
		return true;
	else if (res.a0 == XHEE_DISABLED)
		return false;
	pr_err("invalid xhee status value, return default!\n");
	return true;
}
#else
static inline bool xhee_status_enabled(void)
{
	return true;
}
#endif

#endif /* __XHEE_STATUS_H__ */
