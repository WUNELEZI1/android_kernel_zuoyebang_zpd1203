// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_xr_mdr.h>
#if IS_ENABLED(CONFIG_MALI_XR_GPU_DEBUG)
#include <mali_kbase_xr_gpu_debug.h>
#endif
#include "xrisp_common.h"

static struct mdr_exception_info_s g_gpu_exception[MDR_GPU_TYPES_COUNT] = {
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
	[11] = {
		.e_modid            = (u32)MODID_GPU_FENCE_TIMEOUT,
		.e_modid_end        = (u32)MODID_GPU_FENCE_TIMEOUT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_FENCE_TIMEOUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU FENCE TIMEOUT",
	},
	[12] = {
		.e_modid            = (u32)MODID_GPU_INTERRUPT_TIMEOUT,
		.e_modid_end        = (u32)MODID_GPU_INTERRUPT_TIMEOUT,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_INTERRUPT_TIMEOUT,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU INTERRUPT TIMEOUT",
	},
	[13] = {
		.e_modid            = (u32)MODID_GPU_CS_FATAL,
		.e_modid_end        = (u32)MODID_GPU_CS_FATAL,
		.e_process_priority = MDR_OTHER,
		.e_reboot_priority  = MDR_REBOOT_NO,
		.e_notify_core_mask = MDR_GPU,
		.e_reset_core_mask  = MDR_GPU,
		.e_from_core        = MDR_GPU,
		.e_reentrant        = (u32)MDR_REENTRANT_DISALLOW,
		.e_exce_type        = GPU_S_EXCEPTION,
		.e_exce_subtype     = GPU_S_CS_FATAL,
		.e_upload_flag      = (u32)MDR_UPLOAD_YES,
		.e_from_module      = "GPU",
		.e_desc             = "GPU CS FATAL",
	},
};

static void kbase_xr_mdr_gpu_dump(u32 modid, u32 etype,
			u64 coreid, char *log_path, pfn_cb_dump_done pfn_cb, void *data)
{
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;
	u32 gpu_subtype;
	u32 edr_subtype;

	if (!data) {
		pr_err("Invalid input data parameter");
		return;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return;

	if (modid <= MODID_GPU_START || modid >= MODID_GPU_END) {
		dev_err(kbdev->dev, "invalid gpu modid[%x]", modid);
		return;
	}

	gpu_subtype = modid - MODID_GPU_START - 1;
	edr_subtype = gpu_subtype + GPU_START;

#if IS_ENABLED(CONFIG_MALI_XR_GPU_DEBUG)
	dev_warn(kbdev->dev, "%s: Dumping data at timestamp 0x%llx",
		g_gpu_exception[gpu_subtype].e_desc,
		kbase_reg_read64_coherent(kbdev, GPU_CONTROL_ENUM(TIMESTAMP)));
	if (kbdev->gpu_dev_data.kctx)
		kbase_csf_dump_kctx_debug_info(kbdev->gpu_dev_data.kctx);
	kbase_csf_firmware_log_dump_buffer(kbdev);
	KBASE_KTRACE_DUMP(kbdev);
#endif

	if (edr_drv_submit_api_sync(XRING_S_GPU, edr_subtype, EDR_LOGCAT, log_path))
		dev_err(kbdev->dev, "edr_subtype[%x] catch error", edr_subtype);

	dev_err(kbdev->dev, "%s modid[%x], etype[%x], coreid[%llx], log_path[%s]",
		__func__, modid, etype, coreid, log_path);
}

static void kbase_xr_mdr_gpu_reset(u32 modid, u32 etype, u64 coreid, void *data)
{
	return;
}

static void kbase_xr_mdr_worker(struct work_struct *work)
{
	struct kbase_device *kbdev =
		container_of(work, struct kbase_device,
			     gpu_dev_data.gpu_mdr_work);
	u8 threshold = 0;
	int i = 0;
	int index = 0;
	u8 *count = kbdev->gpu_dev_data.mdr_count;
	u32 modid = kbdev->gpu_dev_data.modid;
	u32 arg1 = kbdev->gpu_dev_data.arg1;
	u32 arg2 = kbdev->gpu_dev_data.arg2;

	struct kbase_context *kctx = kbdev->gpu_dev_data.kctx;
	if (kctx)
		count = kctx->mdr_count;
	/*
	 * modid and mdr type pair, total MDR_GPU_TYPES_COUNT pairs.
	 */
	u32 modid_map[] = {
		MODID_GPU_IRQ_FAULT, MDR_GPU_IRQ_FAULT,
		MODID_GPU_BUS_FAULT, MDR_GPU_BUS_FAULT,
		MODID_GPU_FAULT, MDR_GPU_FAULT,
		MODID_GPU_CS_FAULT, MDR_GPU_CS_FAULT,
		MODID_GPU_UNHANDLE_PAGE_FAULT, MDR_GPU_UNHANDLE_PAGE_FAULT,
		MODID_MCU_UNHANDLE_PAGE_FAULT, MDR_MCU_UNHANDLE_PAGE_FAULT,
		MODID_GPU_SOFT_RESET_TIME_OUT, MDR_GPU_SOFT_RESET_TIME_OUT,
		MODID_GPU_HARD_RESET_TIME_OUT, MDR_GPU_HARD_RESET_TIME_OUT,
		MODID_GPU_BIT_STUCK, MDR_GPU_BIT_STUCK,
		MODID_GPU_REGULATOR_ON_FAIL, MDR_GPU_REGULATOR_ON_FAIL,
		MODID_GPU_REGULATOR_OFF_FAIL, MDR_GPU_REGULATOR_OFF_FAIL,
		MODID_GPU_FENCE_TIMEOUT, MDR_GPU_FENCE_TIMEOUT,
		MODID_GPU_INTERRUPT_TIMEOUT, MDR_GPU_INTERRUPT_TIMEOUT,
		MODID_GPU_CS_FATAL, MDR_GPU_CS_FATAL
	};
	mutex_lock(&kbdev->gpu_dev_data.mdr_lock);
	for (i = 0; i < MDR_GPU_TYPES_COUNT; i++) {
		if (modid == modid_map[i * 2]) {
			index = modid_map[i * 2 + 1];
			count[index]++;
			threshold = kbdev->gpu_dev_data.mdr_threshold[index];
			dev_warn(kbdev->dev, "%s report mdr: %s [%d:%d]",
				(kctx ? kctx->comm : NULL),
				g_gpu_exception[index].e_desc,
				count[index], threshold);
			if (count[index] > threshold) {
				count[index] = 0;
				mdr_system_error(modid, arg1, arg2);
			}
			break;
		}
	}
	mutex_unlock(&kbdev->gpu_dev_data.mdr_lock);
}

void kbase_xr_mdr_init(struct kbase_device *kbdev)
{
	enum kbase_xr_mdr_types type;
	u8 threshold_buffer[MDR_GPU_TYPES_COUNT] = {
		[MDR_GPU_IRQ_FAULT] = 0,
		[MDR_GPU_BUS_FAULT] = 0,
		[MDR_GPU_FAULT] = 3,
		[MDR_GPU_CS_FAULT] = 30,
		[MDR_GPU_UNHANDLE_PAGE_FAULT] = 30,
		[MDR_MCU_UNHANDLE_PAGE_FAULT] = 30,
		[MDR_GPU_SOFT_RESET_TIME_OUT] = 0,
		[MDR_GPU_HARD_RESET_TIME_OUT] = 3,
		[MDR_GPU_BIT_STUCK] = 3,
		[MDR_GPU_REGULATOR_ON_FAIL] = 0,
		[MDR_GPU_REGULATOR_OFF_FAIL] = 0,
		[MDR_GPU_FENCE_TIMEOUT] = 30,
		[MDR_GPU_INTERRUPT_TIMEOUT] = 30,
		[MDR_GPU_CS_FATAL] = 30,
	};

	for (type = MDR_GPU_IRQ_FAULT; type < MDR_GPU_TYPES_COUNT; type++) {
		kbdev->gpu_dev_data.mdr_threshold[type] = threshold_buffer[type];
		kbdev->gpu_dev_data.mdr_count[type] = 0;
	}
	mutex_init(&kbdev->gpu_dev_data.mdr_lock);

	kbdev->gpu_dev_data.gpu_mdr_workq =
		alloc_workqueue("kbase_gpu_mdr_work_queue", WQ_HIGHPRI | WQ_UNBOUND, 0);
	if (!kbdev->gpu_dev_data.gpu_mdr_workq)
		dev_err(kbdev->dev, "Alloc kbase gpu mdr work queue failed");

	INIT_WORK(&kbdev->gpu_dev_data.gpu_mdr_work, kbase_xr_mdr_worker);
	kbdev->gpu_dev_data.modid = 0;
	kbdev->gpu_dev_data.kctx = NULL;
	kbdev->gpu_dev_data.arg1 = 0;
	kbdev->gpu_dev_data.arg2 = 0;
}

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

	for (i = 0; i < MDR_GPU_TYPES_COUNT; i++) {
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

	for (i = 0; i < MDR_GPU_TYPES_COUNT; i++) {
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
	if (kbdev->gpu_dev_data.gpu_mdr_workq)
		destroy_workqueue(kbdev->gpu_dev_data.gpu_mdr_workq);
	return 0;
}

void kbase_xr_mdr_system_error(struct kbase_device *kbdev, struct kbase_context *kctx, u32 modid, u32 arg1, u32 arg2)
{
	kbdev->gpu_dev_data.modid = modid;
	kbdev->gpu_dev_data.kctx = kctx;
	kbdev->gpu_dev_data.arg1 = arg1;
	kbdev->gpu_dev_data.arg2 = arg2;
	queue_work(kbdev->gpu_dev_data.gpu_mdr_workq, &kbdev->gpu_dev_data.gpu_mdr_work);
}
