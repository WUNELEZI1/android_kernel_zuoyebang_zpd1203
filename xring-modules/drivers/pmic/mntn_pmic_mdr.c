// SPDX-License-Identifier: GPL-2.0-only
/*
 * MPMIC MDR(Maintenance Data Record)
 *
 * Copyright (c) 2024 XRing Technologies Co., Ltd.
 *
 */
#define pr_fmt(fmt)     "MNTN_MPMIC_MDR: " fmt
#include "mntn_pmic_mdr.h"
#include <soc/xring/dfx_switch.h>

struct pmic_mdr_exception_info_s {
	const char *irq_name;
	struct mdr_exception_info_s info;
};


int ocp_reset_system = 1;

#define MPMIC_EXCEPTION_INFO(_irq_name, _e_type) \
{\
	.irq_name = _irq_name,                                      \
	.info = {                                                   \
		.e_modid            = (u32)MODID_XR_AP_MPMIC_##_e_type, \
		.e_modid_end        = (u32)MODID_XR_AP_MPMIC_##_e_type, \
		.e_process_priority = MDR_OTHER,                        \
		.e_reboot_priority  = MDR_REBOOT_NO,                    \
		.e_notify_core_mask = MDR_AP,                           \
		.e_reset_core_mask  = 0,                                \
		.e_from_core        = MDR_AP,                           \
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,      \
		.e_exce_type        = XR_AP_PMU,                        \
		.e_exce_subtype     = XR_AP_PMU_MPMIC_##_e_type,        \
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,              \
		.e_from_module      = "XR_MPMIC",                       \
		.e_desc             = #_e_type,                         \
	},                                                          \
}

static struct pmic_mdr_exception_info_s g_mpmic_ocp_exceptions[] = {
	MPMIC_EXCEPTION_INFO("ocp-ldo_adc",  OCP_LDOADC),
	MPMIC_EXCEPTION_INFO("ocp-ldo_coul", OCP_LDOCOUL),
	MPMIC_EXCEPTION_INFO("ocp-ldo_pmu",  OCP_LDOPMU),
	MPMIC_EXCEPTION_INFO("ocp-ulldo5",   OCP_ULLDO5),
	MPMIC_EXCEPTION_INFO("ocp-lldo5",    OCP_LLDO5),
	MPMIC_EXCEPTION_INFO("ocp-mldo4",    OCP_MLDO4),
	MPMIC_EXCEPTION_INFO("ocp-mldo5",    OCP_MLDO5),
	MPMIC_EXCEPTION_INFO("ocp-mldo6",    OCP_MLDO6),
	MPMIC_EXCEPTION_INFO("ocp-mldo7",    OCP_MLDO7),
	MPMIC_EXCEPTION_INFO("ocp-mldo8",    OCP_MLDO8),
	MPMIC_EXCEPTION_INFO("ocp-hldo5",    OCP_HLDO5),
	MPMIC_EXCEPTION_INFO("ocp-hldo6",    OCP_HLDO6),
	MPMIC_EXCEPTION_INFO("ocp-hldo7",    OCP_HLDO7),
	MPMIC_EXCEPTION_INFO("ocp-hldo8",    OCP_HLDO8),
	MPMIC_EXCEPTION_INFO("ocp-hldo9",    OCP_HLDO9),
	MPMIC_EXCEPTION_INFO("ocp-hldo10",   OCP_HLDO10),
	MPMIC_EXCEPTION_INFO("ocp-ulldo1",   OCP_ULLDO1),
	MPMIC_EXCEPTION_INFO("ocp-ulldo2",   OCP_ULLDO2),
	MPMIC_EXCEPTION_INFO("ocp-lldo3",    OCP_LLDO3),
	MPMIC_EXCEPTION_INFO("ocp-lldo4",    OCP_LLDO4),
	MPMIC_EXCEPTION_INFO("ocp-mldo1",    OCP_MLDO1),
	MPMIC_EXCEPTION_INFO("ocp-mldo2",    OCP_MLDO2),
	MPMIC_EXCEPTION_INFO("ocp-hldo2",    OCP_HLDO2),
	MPMIC_EXCEPTION_INFO("ocp-hldo3",    OCP_HLDO3),
};

/*
 * return 0 on fail
 * return the MDR modid on success
 */
u32 mpmic_mdr_get_e_modid_by_ocp_irq_name(const char *irq_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(g_mpmic_ocp_exceptions); i++) {
		struct pmic_mdr_exception_info_s *exception_info = &g_mpmic_ocp_exceptions[i];

		if (strcmp(exception_info->irq_name, irq_name) == 0)
			return exception_info->info.e_modid;
	}

	return 0;
}

int mpmic_mdr_exception_register(void)
{
	u32 mdr_ocp_registered_id[ARRAY_SIZE(g_mpmic_ocp_exceptions)] = {0};
	size_t mdr_ocp_registered_cnt = 0;
	int i;
	int ret;

	ocp_reset_system = is_switch_on(SWITCH_PMIC_OCP_SYS_RESET);

	for (i = 0; i < ARRAY_SIZE(g_mpmic_ocp_exceptions); i++) {
		struct mdr_exception_info_s *exception_info = &g_mpmic_ocp_exceptions[i].info;

		if (ocp_reset_system) {
			exception_info->e_reset_core_mask = MDR_AP;
			exception_info->e_reboot_priority = MDR_REBOOT_NOW;
		}
		ret = mdr_register_exception(exception_info);
		if (ret == 0) {
			pr_err("register exception %s failed\n", exception_info->e_desc);
			ret = -EINVAL;
			goto fail_mdr_register;
		}
		mdr_ocp_registered_id[mdr_ocp_registered_cnt] = exception_info->e_modid;
		mdr_ocp_registered_cnt++;
	}

	return 0;

fail_mdr_register:
	for (i = 0; i < mdr_ocp_registered_cnt; i++) {
		if (mdr_ocp_registered_id[i])
			mdr_unregister_exception(mdr_ocp_registered_id[i]);
	}
	return ret;
}

void mpmic_mdr_exception_unregister(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(g_mpmic_ocp_exceptions); i++) {
		struct mdr_exception_info_s *exception_info = &g_mpmic_ocp_exceptions[i].info;

		mdr_unregister_exception(exception_info->e_modid);
	}
}
