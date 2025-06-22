// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024-2024, X-Ring technologies Inc., All rights reserved.
 */

#include "../xrse_internal.h"
#include <linux/types.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <linux/errno.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/xrse_platform.h>
#include <linux/android_debug_symbols.h>
#include <asm/memory.h>
#include <linux/module.h>

#define RIP_STATUS_UNPACK_STEP            4
#define BIT_PER_BYTES                     8
#define RIP_STATUS_MASK                   0xFF

void xrse_rip_unpack_status(struct rip_measure_status_str *rip_sta, u64 *reg_arr)
{
	u32 i = 0;
	u8 *my_arr = (u8 *)reg_arr;

	for (i  = 0; i < RIP_MAX; i++)
		rip_sta->measure_status[i] = my_arr[i];
}

int xrse_rip_base_request(u32 module_id, u64 addr, u32 len, struct rip_measure_strategy_str *strategy)
{
	struct arm_smccc_1_2_regs res = {0};

	if (strategy == NULL) {
		xrse_err("rip strategy is null\n");
		return -EINVAL;
	}

	struct arm_smccc_1_2_regs msg = {
		.a0 = FID_BL31_XRSE_RIP_BASE_REQUEST,
		.a1 = (u64)(module_id),
		.a2 = (u64)(addr),
		.a3 = (u64)(len),
		.a4 = (u64)(strategy->alarm_time),
	};

	arm_smccc_1_2_smc(&msg, &res);

	return (u32)res.a0;
}


int xrse_rip_realtime_request(u32 module_id, struct rip_measure_status_str *rip_sta, u32 rip_sta_size)
{
	int ret = -EINVAL;
	struct arm_smccc_1_2_regs res = {0};
	struct arm_smccc_1_2_regs msg = {
		.a0 = FID_BL31_XRSE_RIP_REALTIME_REQUEST,
		.a1 = (u64)(module_id),
	};

	if (rip_sta_size < sizeof(struct rip_measure_status_str)) {
		xrse_err("invalid rip sta size:%d\n", rip_sta_size);
		return -EINVAL;
	}

	if (rip_sta == NULL) {
		xrse_err("rip_sta is null when realtime measure\n");
		return -EINVAL;
	}

	arm_smccc_1_2_smc(&msg, &res);
	ret = (u32)res.a0;
	if (ret) {
		xrse_err("xrse_rip:realtime request fail. ret:%d\n", ret);
		return ret;
	}

	u64 reg_arr[] = {res.a1, res.a2, res.a3, res.a4, res.a5, res.a6, res.a7};

	xrse_rip_unpack_status(rip_sta, reg_arr);

	return 0;
}

int xrse_rip_get_status(struct rip_measure_status_str *rip_sta, u32 rip_sta_size)
{
	int ret = -EINVAL;
	struct arm_smccc_1_2_regs res = {0};
	struct arm_smccc_1_2_regs msg = {
		.a0 = FID_BL31_XRSE_RIP_GET_STATUS_REQUEST,
		.a1 = (u64)RIP_MAGIC_NUMBER,
	};

	if (rip_sta_size < sizeof(struct rip_measure_status_str)) {
		xrse_err("invalid rip sta size:%d\n", rip_sta_size);
		return -EINVAL;
	}

	if (rip_sta == NULL) {
		xrse_err("rip_sta is null when get status\n");
		return -EINVAL;
	}

	arm_smccc_1_2_smc(&msg, &res);
	ret = (u32)res.a0;
	if (ret) {
		xrse_err("xrse_rip: get status fail. ret:%d\n", ret);
		return ret;
	}

	u64 reg_arr[] = {res.a1, res.a2, res.a3, res.a4, res.a5, res.a6, res.a7};

	xrse_rip_unpack_status(rip_sta, reg_arr);

	return 0;
}

int kernel_rodata_rip_request(void)
{
	int ret;
	struct rip_measure_strategy_str strategy = {0};
	void *start_ro, *end_ro;

	start_ro = android_debug_symbol(ADS_START_RO_AFTER_INIT);
	end_ro = android_debug_symbol(ADS_END_RO_AFTER_INIT);
	ret = xrse_rip_base_request((u32)RIP_KERNEL, (u64)virt_to_phys(start_ro), (u32)(end_ro - start_ro), &strategy);
	if (ret)
		xrse_err("rodata rip fail. ret:%d\n", ret);

	return ret;
}

MODULE_IMPORT_NS(MINIDUMP);
