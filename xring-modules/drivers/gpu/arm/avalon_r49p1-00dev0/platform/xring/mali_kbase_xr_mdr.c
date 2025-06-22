// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#include <mdr_pub.h>
#include <mali_kbase.h>

#define MDR_GPU_EXP_TYPE_MAX 11

static struct mdr_exception_info_s g_gpu_exception[MDR_GPU_EXP_TYPE_MAX] = {
	[0] = {
		.e_modid            = (u32)MODID_GPU_IRQ_FAULT,
		.e_modid_end        = (u32)MODID_GPU_IRQ_FAULT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_IRQ_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU IRQ FAULT",
	},
	[1] = {
		.e_modid            = (u32)MODID_GPU_BUS_FAULT,
		.e_modid_end        = (u32)MODID_GPU_BUS_FAULT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_BUS_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU BUS FAULT",
	},
	[2] = {
		.e_modid            = (u32)MODID_GPU_FAULT,
		.e_modid_end        = (u32)MODID_GPU_FAULT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_GPU_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU FAULT",
	},
	[3] = {
		.e_modid            = (u32)MODID_GPU_CS_FAULT,
		.e_modid_end        = (u32)MODID_GPU_CS_FAULT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_CS_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU CS FAULT",
	},
	[4] = {
		.e_modid            = (u32)MODID_GPU_UNHANDLE_PAGE_FAULT,
		.e_modid_end        = (u32)MODID_GPU_UNHANDLE_PAGE_FAULT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_UNHANDLE_PAGE_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU UNHANDLE PAGE FAULT",
	},
	[5] = {
		.e_modid            = (u32)MODID_MCU_UNHANDLE_PAGE_FAULT,
		.e_modid_end        = (u32)MODID_MCU_UNHANDLE_PAGE_FAULT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_MCU_UNHANDLE_PAGE_FAULT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU MCU UNHANDLE PAGE FAULT",
	},
	[6] = {
		.e_modid            = (u32)MODID_GPU_SOFT_RESET_TIME_OUT,
		.e_modid_end        = (u32)MODID_GPU_SOFT_RESET_TIME_OUT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_SOFT_RESET_TIME_OUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU SOFT RESET TIME OUT",
	},
	[7] = {
		.e_modid            = (u32)MODID_GPU_HARD_RESET_TIME_OUT,
		.e_modid_end        = (u32)MODID_GPU_HARD_RESET_TIME_OUT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_HARD_RESET_TIME_OUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU HARD RESET TIME OUT",
	},
	[8] = {
		.e_modid            = (u32)MODID_GPU_BIT_STUCK,
		.e_modid_end        = (u32)MODID_GPU_BIT_STUCK,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_BIT_STUCK,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU BIT STUCK",
	},
	[9] = {
		.e_modid            = (u32)MODID_GPU_REGULATOR_ON_FAIL,
		.e_modid_end        = (u32)MODID_GPU_REGULATOR_ON_FAIL,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_REGULATOR_ON_FAIL,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU REGULATOR ON FAIL",
	},
	[10] = {
		.e_modid            = (u32)MODID_GPU_REGULATOR_OFF_FAIL,
		.e_modid_end        = (u32)MODID_GPU_REGULATOR_OFF_FAIL,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_REGULATOR_OFF_FAIL,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU REGULATOR OFF FAIL",
	},
};

static void kbase_xr_mdr_gpu_dump(u32 modid, u32 etype,
			u64 coreid, char *log_path, pfn_cb_dump_done pfn_cb, void *data)
{
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;

	if (!data) {
		pr_err("Invalid input data parameter");
		return;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return;

	dev_err(kbdev->dev, "%s modid[%x], etype[%x], coreid[%llx], log_path[%s]\n",
		__func__, modid, etype, coreid, log_path);
}

static void kbase_xr_mdr_gpu_reset(u32 modid, u32 etype, u64 coreid, void *data) {}

int kbase_xr_mdr_exception_register(struct kbase_device *kbdev)
{
	int ret = 0;
	int i = 0;
	struct mdr_register_module_result info;
	struct mdr_module_ops gpu_ops = {
		.ops_dump = kbase_xr_mdr_gpu_dump,
		.ops_reset = kbase_xr_mdr_gpu_reset,
		.ops_data = kbdev->dev,
	};

	for (i = 0; i < MDR_GPU_EXP_TYPE_MAX; i++) {
		/* this function is implemented by mdr and return 0 when error */
		ret = mdr_register_exception(&g_gpu_exception[i]);
		if (ret == 0) {
			dev_err(kbdev->dev, "GPU exception %d register fail, ret %d.\n", i, ret);
			return -EINVAL;
		}
	}

	/* this function is implemented by mdr and return negative value when error */
	ret = mdr_register_module_ops(MDR_GPU, &gpu_ops, &info);
	if (ret < 0) {
		dev_err(kbdev->dev, "GPU module register fail");
		return ret;
	}
	return 0;
}

int kbase_xr_mdr_exception_unregister(struct kbase_device *kbdev)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < MDR_GPU_EXP_TYPE_MAX; i++) {
		ret = mdr_unregister_exception(g_gpu_exception[i].e_modid);
		if (ret) {
			dev_err(kbdev->dev, "GPU exception %d unregister fail, ret %d.\n", i, ret);
			return -EINVAL;
		}
	}

	ret = mdr_unregister_module_ops(MDR_GPU);
	if (ret) {
		dev_err(kbdev->dev, "GPU module unregister fail");
		return ret;
	}
	return 0;
}