// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include "linux/kstrtox.h"
#include "vdec_mdr.h"
#include "subsys.h"
#include "vdec_log.h"
#include "xrisp_common.h"

#define MDR_VDEC_EXP_TYPE_MAX 4
static struct mdr_exception_info_s g_vdec_exception[MDR_VDEC_EXP_TYPE_MAX] = {
	[0] = {
		.e_modid            = (u32)MODID_VPU_DEC_WDT_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_DEC_WDT_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_VDEC,
		.e_reset_core_mask  = MDR_VDEC,
		.e_from_core        = MDR_VDEC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_DEC_WDT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VDEC",
		.e_desc             = "vpu decoding watchdog exception",
	},
	[1] = {
		.e_modid            = (u32)MODID_VPU_DEC_NOC_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_DEC_NOC_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_VDEC,
		.e_reset_core_mask  = MDR_VDEC,
		.e_from_core        = MDR_VDEC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_DEC_NOC,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VDEC",
		.e_desc             = "vpu decoding noc exception",
	},
	[2] = {
		.e_modid            = (u32)MODID_VPU_DEC_HANG_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_DEC_HANG_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_VDEC,
		.e_reset_core_mask  = MDR_VDEC,
		.e_from_core        = MDR_VDEC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_DEC_HANG_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VDEC",
		.e_desc             = "vpu decoding hang, not working",
	},
	[3] = {
		.e_modid            = (u32)MODID_VPU_DEC_EXTERNAL_TIMEOUT_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_DEC_EXTERNAL_TIMEOUT_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_VDEC,
		.e_reset_core_mask  = MDR_VDEC,
		.e_from_core        = MDR_VDEC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_DEC_EXTERNAL_TIMEOUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VDEC",
		.e_desc             = "vpu decoding external timeout exception",
	}
};

static void mdr_vdec_dump(u32 modid, u32 etype,
		u64 coreid, char *log_path, pfn_cb_dump_done pfn_cb, void *data)
{
	struct device *dev = NULL;
	int ret;

	if (!data) {
		vdec_mdr_klog(LOGLVL_ERROR, "Invalid input data parameter");
		return;
	}

	vcmd_klog(LOGLVL_ERROR, "===>moid=%u!!!\n", modid);

	switch (modid) {
	case MODID_VPU_DEC_HANG_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_DEC, VPU_DEC_HANG_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	case MODID_VPU_DEC_WDT_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_DEC, VPU_DEC_WDT_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	case MODID_VPU_DEC_NOC_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_DEC, VPU_DEC_NOC_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	case MODID_VPU_DEC_EXTERNAL_TIMEOUT_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_DEC, VPU_DEC_TIMEOUT_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	default:
		vcmd_klog(LOGLVL_ERROR, "===>modid=%d !!!\n", modid);
		return;
	}

	vcmd_klog(LOGLVL_ERROR, "===>edr_drv_submit_api_sync %s !!!\n", (ret < 0) ? "fail" : "success");

	dev = (struct device *)data;
	vdec_mdr_klog(LOGLVL_ERROR, "modid[%x], etype[%x], coreid[%llx], log_path[%s]\n",
		modid, etype, coreid, log_path);
}

static void mdr_vdec_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	struct device *dev = NULL;
	int ret = 0;

	if (!data) {
		vdec_mdr_klog(LOGLVL_ERROR, "Invalid input data parameter");
		return;
	}

	dev = (struct device *)data;
	vdec_mdr_klog(LOGLVL_WARNING, "modid[%x], etype[%x], coreid[%llx]\n", modid, etype, coreid);
}

int mdr_vdec_exception_register(struct device *dev)
{
	int ret = 0;
	int i = 0;
	struct mdr_register_module_result info;
	struct mdr_module_ops vdec_ops = {
		.ops_dump = mdr_vdec_dump,
		.ops_reset = mdr_vdec_reset,
		.ops_data = dev,
	};

	for (i = 0; i < MDR_VDEC_EXP_TYPE_MAX; i++) {
		/* this function is implemented by mdr and return 0 when error */
		ret = mdr_register_exception(&g_vdec_exception[i]);
		if (ret == 0) {
			vdec_mdr_klog(LOGLVL_ERROR, "VPU exception %d register fail, ret %d.\n", i, ret);
			return -EINVAL;
		}
	}

	/* this function is implemented by mdr and return negative value when error */
	ret = mdr_register_module_ops(MDR_VDEC, &vdec_ops, &info);
	if (ret < 0) {
		vdec_mdr_klog(LOGLVL_ERROR, "VPU module register fail");
		return ret;
	}
	return 0;
}

int mdr_vdec_exception_unregister(struct device *dev)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < MDR_VDEC_EXP_TYPE_MAX; i++) {
		/* this function is implemented by mdr and return 0 when error */
		ret = mdr_unregister_exception(g_vdec_exception[i].e_modid);
		if (ret) {
			vdec_mdr_klog(LOGLVL_ERROR, "VPU exception %d unregister fail, ret %d.\n", i, ret);
			return -EINVAL;
		}
	}

	ret = mdr_unregister_module_ops(MDR_VDEC);
	if (ret) {
		vdec_mdr_klog(LOGLVL_ERROR, "VPU module unregister fail");
		return ret;
	}
	return 0;
}

#ifdef CONFIG_DEC_MDR_TEST
int mdr_type = -1;

static int mdr_vdec_param_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	ret = kstrtoint(val, 10, &mdr_type);
	if (ret) {
		vdec_mdr_klog(LOGLVL_ERROR, "Invalid parameter value:%s\n", val);
		return -EINVAL;
	}

	vdec_mdr_klog(LOGLVL_INFO, "mdr vdec set type %d\n", mdr_type);
	switch (mdr_type) {
	case 0:
		mdr_system_error(MODID_VPU_DEC_WDT_EXCEPTION, 0, 0);
		break;
	case 1:
		mdr_system_error(MODID_VPU_DEC_NOC_EXCEPTION, 0, 0);
		break;
	case 2:
		mdr_system_error(MODID_VPU_DEC_HANG_EXCEPTION, 0, 0);
		break;
	case 3:
		mdr_system_error(MODID_VPU_DEC_EXTERNAL_TIMEOUT_EXCEPTION, 0, 0);
		break;
	default:
		vdec_mdr_klog(LOGLVL_ERROR, "Unexpected mdr_type");
		return -EINVAL;
	}
	return ret;
}

static struct kernel_param_ops mdr_param_cb = {
	.set = mdr_vdec_param_set,
	.get = param_get_int
};

module_param_cb(mdr_test, &mdr_param_cb, &mdr_type, 0644);
#endif
