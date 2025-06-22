// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023-2023, X-Ring technologies Inc., All rights reserved.
 *
 * Description: ras driver
 */

#define pr_fmt(fmt)				 "RAS: " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/arm-smccc.h>
#include <linux/version.h>
#include <trace/hooks/traps.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_pub.h>
#include <dt-bindings/xring/platform-specific/common/mdr/include/mdr_public_if.h>
#include "./ras.h"

static struct mdr_exception_info_s g_ras_excep[] = {
	{
		.e_modid = (u32)MODID_XR_AP_RAS_UNCORRECTED,
		.e_modid_end = (u32)MODID_XR_AP_RAS_UNCORRECTED,
		.e_process_priority = MDR_ERR,
		.e_reboot_priority = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_AP,
		.e_reset_core_mask = MDR_AP,
		.e_from_core = MDR_AP,
		.e_reentrant = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type = XR_AP_RAS,
		.e_exce_subtype = AP_RAS_UNCORRECTED_ERR,
		.e_upload_flag = (u32)MDR_UPLOAD_YES,
		.e_from_module = "ras",
		.e_desc = "ap ras uncorrected error",
	},
};

static inline void ras_fetch_mdr_arg(struct atf_mdr_info *mi, u64 *arg)
{
	if (mi->arg_num <= 0)
		return;
	*arg = mi->args[--mi->arg_num];
}

static void ras_fetch_mdr(struct atf_mdr_ras_info *info, struct atf_mdr_info *mi)
{
	int arg_num = mi->arg_num;

	ras_fetch_mdr_arg(mi, &info->misc1);
	ras_fetch_mdr_arg(mi, &info->misc0);
	ras_fetch_mdr_arg(mi, &info->addr);
	ras_fetch_mdr_arg(mi, &info->err_status);
	ras_fetch_mdr_arg(mi, &info->node_id);
	mi->arg_num = arg_num;
}

static void ras_err_status_print(struct atf_mdr_ras_info *info)
{
	ras_err_status_reg_t err_status;

	err_status.raw = info->err_status;
	pr_err("NodeId %llu:ERR_STATUS 0x%llx:ue %d ce %d de %d er %d ci %d.\n",
		info->node_id, err_status.raw,
		err_status.ue, err_status.ce, err_status.de, err_status.er, err_status.ci);
	if (err_status.ue)
		pr_err("uet 0x%x.\n", err_status.uet);
	pr_err("ADDR :0x%llx\n", info->addr);
	pr_err("MISC0:0x%llx\n", info->misc0);
	pr_err("MISC1:0x%llx\n", info->misc1);
}

void ras_mdr_notify(struct atf_mdr_info *mi)
{
	struct atf_mdr_ras_info info = {0};

	if (mi == NULL || mi->arg_num > ATF_MDR_ARGS_MAX)
		return;

	switch (mi->e_modid) {
	case MODID_XR_AP_RAS_UNCORRECTED:
		ras_fetch_mdr(&info, mi);
		ras_err_status_print(&info);
		mi->e_modid = 0;//ensure e_modid set to 0 before mdr_syserr_process_for_ap
		dsb(sy);
		mdr_syserr_process_for_ap(MODID_XR_AP_RAS_UNCORRECTED, mi->args[0], mi->args[1]);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(ras_mdr_notify);

static void __trace_android_rvh_arm64_serror_panic(
		void *unused, struct pt_regs *regs, unsigned long esr)
{
	struct arm_smccc_res res = {0};

	/* smc call into atf */
	arm_smccc_smc(FID_BL31_RAS_EA, esr, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 != 0)
		pr_err("detect RAS error from ATF.\n");
	else
		pr_err("detect no RAS error from ATF.\n");
}

static int __init ras_driver_init(void)
{
	int ret = 0, i;

	ret = register_trace_android_rvh_arm64_serror_panic(
		&__trace_android_rvh_arm64_serror_panic, NULL);
	if (ret) {
		pr_err("register_trace_android_rvh_arm64_serror_panic failed (ret=%d)\n", ret);
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(g_ras_excep); i++) {
		ret = mdr_register_exception(&g_ras_excep[i]);
		if (ret == 0) {
			pr_err("ras exception %d register fail, ret %d.\n", i, ret);
			return -EFAULT;
		}
	}

	pr_info("ras module init success\n");
	return 0;
}

static void __exit ras_driver_exit(void)
{
	pr_info("ras module exit\n");
}

MODULE_DESCRIPTION("XRing ras driver");
MODULE_LICENSE("GPL v2");
module_init(ras_driver_init);
module_exit(ras_driver_exit);
