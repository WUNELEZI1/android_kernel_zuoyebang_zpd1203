/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 */
#ifndef __XR_GPUFREQ_H__
#define __XR_GPUFREQ_H__

struct xr_gpufreq_ops {
	void *data;
	u32 (*get_utilization)(void *data);
	void (*notify_change)(void *data, u64 freq);
	void (*gpufreq_suspend)(void);
	void (*gpufreq_resume)(void);
	u64 (*gpufreq_get_freq)(void);
	int (*gpufreq_set_freq)(u64 freq);
	int (*update_coremask)(void *data, u32 core_num);
	u32 (*get_cl_accel)(void *data);
	int (*get_vfreq_status)(void *data);
};

int xr_gpufreq_ops_register(struct xr_gpufreq_ops *ops);
void xr_gpufreq_ops_term(struct xr_gpufreq_ops *ops);
struct device *xr_get_gpufreq_device(void);

#endif
