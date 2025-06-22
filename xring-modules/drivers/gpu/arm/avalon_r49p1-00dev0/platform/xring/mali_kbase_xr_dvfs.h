// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef __MALI_KBASE_XR_DVFS__
#define __MALI_KBASE_XR_DVFS__
#include <mali_kbase.h>

#define MALI_XRING_DVFS_STUB 0

#if MALI_XRING_DVFS_STUB
struct xr_gpufreq_ops {
	void *data;
	u32 (*get_utilization)(void *data);
	void (*notify_change)(void *data, u64 freq);
	void (*gpufreq_suspend)(void);
	void (*gpufreq_resume)(void);
	u64 (*gpufreq_get_freq)(void);
	int (*gpufreq_set_freq)(u64 freq);
	int (*update_coremask)(void *data, u32 core_num);
	int (*get_cl_accel)(void *data);
	int (*get_vfreq_status)(void *data);
};

u64 xr_gpufreq_get_freq(void);
int xr_gpufreq_set_freq(u64 freq);
int xr_gpufreq_ops_register(struct xr_gpufreq_ops *ops);
void xr_gpufreq_ops_term(struct xr_gpufreq_ops *ops);
void xr_gpufreq_suspend(void);
void xr_gpufreq_resume(void);
#else
#include <soc/xring/xr_gpufreq.h>
#endif

int kbase_xr_register_dvfs_op(struct kbase_device *kbdev);
u32 kbase_xr_get_utilisation(void *data);
void kbase_xr_notify_change(void *data, u64 freq);
void kbase_xr_unregister_dvfs_op(struct kbase_device *kbdev);
int kbase_xr_get_cl_accel(void *data);
#endif /* __MALI_KBASE_XR_DVFS__ */
