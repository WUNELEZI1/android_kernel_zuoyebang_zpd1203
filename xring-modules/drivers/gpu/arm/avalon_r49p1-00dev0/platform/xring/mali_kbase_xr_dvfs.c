// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_config.h>
#include <backend/gpu/mali_kbase_pm_internal.h>
#include <mali_kbase_defs.h>

#include <csf/ipa_control/mali_kbase_csf_ipa_control.h>
#include <platform/xring/mali_kbase_xr_dvfs.h>
#include <platform/xring/mali_kbase_xr_virtual_freq.h>
#include <platform/xring/mali_kbase_config_platform.h>

#ifdef CONFIG_MALI_MIDGARD_DVFS
#if MALI_USE_CSF
int kbase_platform_dvfs_event(struct kbase_device *kbdev, u32 utilisation)
{
	return 1;
}
#else   // !MALI_USE_CSF
int kbase_platform_dvfs_event(struct kbase_device *kbdev, u32 utilisation,
	u32 util_gl_share, u32 util_cl_share[2])
{
	return 1;
}
#endif
#endif /* CONFIG_MALI_MIDGARD_DVFS */

/* xr platform gpu dvfs interface for gpufreq */
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
u32 kbase_xr_get_utilisation(void *data)
{
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;
	u32 utilisation = 0;

	if (!data) {
		pr_err("Invalid input data parameter!");
		return -EINVAL;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	KBASE_DEBUG_ASSERT(kbdev != NULL);
	if (kbdev->pm.backend.metrics.kbdev != kbdev) {
		dev_err(kbdev->dev, "%s pm backend metrics not initialized\n", __func__);
		return -EINVAL;
	}

	kbase_pm_get_dvfs_action(kbdev);
	utilisation = kbdev->pm.backend.metrics.utilisation;

	return utilisation;
}

void kbase_xr_notify_change(void *data, u64 freq)
{
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;

	if (!data) {
		pr_err("Invalid data parameter!");
		return;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return;

	kbase_xr_ipa_control_rate_change_notify(kbdev, freq);
}

#if IS_ENABLED(CONFIG_MALI_XRING_CL_DVFS)
int kbase_xr_get_cl_accel(void *data)
{
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;
	bool cl_accel;

	if (!data) {
		pr_err("Invalid input data parameter!");
		return -EINVAL;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	cl_accel = kbdev->gpu_dev_data.cl_accel_flag;

	return cl_accel;
}
#endif /* CONFIG_MALI_XRING_CL_DVFS */

int kbase_xr_register_dvfs_op(struct kbase_device *kbdev)
{
	int ret = 0;
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;

	gpufreq_ops->get_utilization = kbase_xr_get_utilisation;
	gpufreq_ops->notify_change = kbase_xr_notify_change;
#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
	gpufreq_ops->update_coremask = kbase_xr_vfreq_set_core;
	gpufreq_ops->get_vfreq_status = kbase_xr_vfreq_get_status;
#endif
#if IS_ENABLED(CONFIG_MALI_XRING_CL_DVFS)
	gpufreq_ops->get_cl_accel = kbase_xr_get_cl_accel;
#endif
#if MALI_XRING_DVFS_STUB
	gpufreq_ops->gpufreq_suspend = xr_gpufreq_suspend;
	gpufreq_ops->gpufreq_resume  = xr_gpufreq_resume;
	gpufreq_ops->gpufreq_get_freq = xr_gpufreq_get_freq;
	gpufreq_ops->gpufreq_set_freq = xr_gpufreq_set_freq;
#endif
	gpufreq_ops->data = (void *)kbdev->dev;
	ret = xr_gpufreq_ops_register(gpufreq_ops);
	if (ret)
		dev_err(kbdev->dev, "GPUFreq ops register failed!");
	else
		atomic_inc(&kbdev->gpu_dev_data.gpufreq_ready);

	return ret;
}

void kbase_xr_unregister_dvfs_op(struct kbase_device *kbdev)
{
	struct xr_gpufreq_ops *gpufreq_ops = &kbdev->gpu_dev_data.gpufreq_ops;

	atomic_dec(&kbdev->gpu_dev_data.gpufreq_ready);
	xr_gpufreq_ops_term(gpufreq_ops);
	gpufreq_ops->get_utilization = NULL;
	gpufreq_ops->notify_change = NULL;
	gpufreq_ops->gpufreq_suspend = NULL;
	gpufreq_ops->gpufreq_resume  = NULL;
	gpufreq_ops->gpufreq_get_freq = NULL;
	gpufreq_ops->gpufreq_set_freq = NULL;
	gpufreq_ops->data = NULL;
}

#if MALI_XRING_DVFS_STUB
int xr_gpufreq_ops_register(struct xr_gpufreq_ops *ops)
{
	return 0;
}

void xr_gpufreq_ops_term(struct xr_gpufreq_ops *ops) {}

u64 xr_gpufreq_get_freq(void)
{
	return 10000000;
}

int xr_gpufreq_set_freq(u64 freq)
{
	return 0;
}

void xr_gpufreq_suspend(void) {}

void xr_gpufreq_resume(void) {}
#endif
#endif
