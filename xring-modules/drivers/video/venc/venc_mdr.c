// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include "linux/kstrtox.h"
#include "venc_mdr.h"
#include "vcx_vcmd_priv.h"
#include "xrisp_common.h"

#define MDR_VENC_EXP_TYPE_MAX 4
static struct mdr_exception_info_s g_venc_exception[MDR_VENC_EXP_TYPE_MAX] = {
	[0] = {
		.e_modid            = (u32)MODID_VPU_ENC_WDT_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_ENC_WDT_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_VENC,
		.e_reset_core_mask  = MDR_VENC,
		.e_from_core        = MDR_VENC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_ENC_WDT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VENC",
		.e_desc             = "vpu encoding watchdog exception",
	},
	[1] = {
		.e_modid            = (u32)MODID_VPU_ENC_NOC_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_ENC_NOC_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_VENC,
		.e_reset_core_mask  = MDR_VENC,
		.e_from_core        = MDR_VENC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_ENC_NOC,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VENC",
		.e_desc             = "vpu encoding noc exception",
	},
	[2] = {
		.e_modid            = (u32)MODID_VPU_ENC_HANG_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_ENC_HANG_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NOW,
		.e_notify_core_mask = MDR_VENC,
		.e_reset_core_mask  = MDR_VENC,
		.e_from_core        = MDR_VENC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_ENC_HANG_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VENC",
		.e_desc             = "vpu encoding hang, not working",
	},
	[3] = {
		.e_modid            = (u32)MODID_VPU_ENC_EXTERNAL_TIMEOUT_EXCEPTION,
		.e_modid_end        = (u32)MODID_VPU_ENC_EXTERNAL_TIMEOUT_EXCEPTION,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_VENC,
		.e_reset_core_mask  = MDR_VENC,
		.e_from_core        = MDR_VENC,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = VPU_S_EXCEPTION,
		.e_exce_subtype     = VPU_S_ENC_EXTERNAL_TIMEOUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "VENC",
		.e_desc             = "vpu encoding external timeout exception",
	}
};

static void mdr_venc_dump(u32 modid, u32 etype,
			u64 coreid, char *log_path, pfn_cb_dump_done pfn_cb, void *data)
{
	struct device *dev = NULL;
	int ret;

	if (!data) {
		venc_mdr_klog(LOGLVL_ERROR, "Invalid input data parameter");
		return;
	}

	vcmd_klog(LOGLVL_ERROR, "===>moid=%u!!!\n", modid);

	switch (modid) {
	case MODID_VPU_ENC_HANG_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_ENC, VPU_ENC_HANG_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	case MODID_VPU_ENC_WDT_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_ENC, VPU_ENC_WDT_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	case MODID_VPU_ENC_NOC_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_ENC, VPU_ENC_NOC_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	case MODID_VPU_ENC_EXTERNAL_TIMEOUT_EXCEPTION:
		ret = edr_drv_submit_api_sync(XRING_S_VPU_ENC, VPU_ENC_TIMEOUT_EXCEPTION, EDR_LOGCAT, log_path);
		break;
	default:
		vcmd_klog(LOGLVL_ERROR, "===>modid=%u !!!\n", modid);
		return;
	}

	vcmd_klog(LOGLVL_ERROR, "===>edr_drv_submit_api_sync %s !!!\n", (ret < 0) ? "fail" : "success");

	dev = (struct device *)data;

	venc_mdr_klog(LOGLVL_ERROR, "modid[%x], etype[%x], coreid[%llx], log_path[%s]\n",
		modid, etype, coreid, log_path);
}

static void mdr_venc_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	struct device *dev = NULL;
	int ret = 0;

	if (!data) {
		venc_mdr_klog(LOGLVL_ERROR, "Invalid input data parameter");
		return;
	}
	venc_mdr_klog(LOGLVL_WARNING, "modid[%x], etype[%x], coreid[%llx]\n", modid, etype, coreid);

	dev = (struct device *)data;
}

int mdr_venc_exception_register(struct device *dev)
{
	int ret = 0;
	int i = 0;
	struct mdr_register_module_result info;
	struct mdr_module_ops venc_ops = {
		.ops_dump = mdr_venc_dump,
		.ops_reset = mdr_venc_reset,
		.ops_data = dev,
	};

	for (i = 0; i < MDR_VENC_EXP_TYPE_MAX; i++) {
		/* this function is implemented by mdr and return 0 when error */
		ret = mdr_register_exception(&g_venc_exception[i]);
		if (ret == 0) {
			venc_mdr_klog(LOGLVL_ERROR, "VPU exception %d register fail, ret %d.\n", i, ret);
			return -EINVAL;
		}
	}

	/* this function is implemented by mdr and return negative value when error */
	ret = mdr_register_module_ops(MDR_VENC, &venc_ops, &info);
	if (ret < 0) {
		venc_mdr_klog(LOGLVL_ERROR, "VPU module register fail");
		return ret;
	}
	return 0;
}

int mdr_venc_exception_unregister(struct device *dev)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < MDR_VENC_EXP_TYPE_MAX; i++) {
		/* this function is implemented by mdr and return 0 when error */
		ret = mdr_unregister_exception(g_venc_exception[i].e_modid);
		if (ret) {
			venc_mdr_klog(LOGLVL_ERROR, "VPU exception %d unregister fail, ret %d.\n", i, ret);
			return -EINVAL;
		}
	}

	ret = mdr_unregister_module_ops(MDR_VENC);
	if (ret) {
		venc_mdr_klog(LOGLVL_ERROR, "VPU module unregister fail");
		return ret;
	}
	return 0;
}


#ifdef CONFIG_ENC_MDR_TEST
int mdr_type = -1;

static int mdr_venc_param_set(const char *val, const struct kernel_param *kp)
{
	int ret = 0;

	ret = kstrtoint(val, 10, &mdr_type);
	if (ret) {
		venc_mdr_klog(LOGLVL_ERROR, "Invalid parameter value:%s\n", val);
		return -EINVAL;
	}

	venc_mdr_klog(LOGLVL_INFO, "mdr venc set type %d\n", mdr_type);
	switch (mdr_type) {
	case 0:
		mdr_system_error(MODID_VPU_ENC_WDT_EXCEPTION, 0, 0);
		break;
	case 1:
		mdr_system_error(MODID_VPU_ENC_NOC_EXCEPTION, 0, 0);
		break;
	case 2:
		mdr_system_error(MODID_VPU_ENC_HANG_EXCEPTION, 0, 0);
		break;
	case 3:
		mdr_system_error(MODID_VPU_ENC_EXTERNAL_TIMEOUT_EXCEPTION, 0, 0);
		break;
	default:
		venc_mdr_klog(LOGLVL_ERROR, "Unexpected mdr_type");
		return -EINVAL;
	}

	return ret;
}

static struct kernel_param_ops mdr_param_cb = {
	.set = mdr_venc_param_set,
	.get = param_get_int
};

module_param_cb(mdr_test, &mdr_param_cb, &mdr_type, 0644);
#endif
