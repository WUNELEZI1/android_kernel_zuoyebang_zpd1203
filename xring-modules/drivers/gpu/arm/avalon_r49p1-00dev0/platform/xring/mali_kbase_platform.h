/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _MALI_KBASE_PLATFORM_H_
#define _MALI_KBASE_PLATFORM_H_

#include <mali_kbase_xr_dvfs.h>
#include <mali_kbase_xr_virtual_freq.h>
/**
 * struct kbase_gpu_device_data - all gpu platform data in device level.
 * callbacks: The callbacks gpu implements.
 * gpu_subsys: gpu regulator.
 * regulator_refcount: refcount for regulator, should be '1' after call regulator_enable()
 *                     and be '0' after call regulator_disable().
 * @query_pid: The pid to query fence.
 * @fence_count: The number of times the fence has been triggered since the first query.
 * @mdr_type: The error type to be reported to mdr.
 * @gpufreq_ops: Operation functions to be registered to DVFS.
 * @dvfs_type: The gpu dvfs debug type.
 * @gpufreq_ready: The ops register status.
 * @gpufreq_refcount: represent gpufreq suspend/resume status, 1 for resume, 0 for suspend.
 * @opp_lowest_freq_khz: The lowest gpu freq in dts opp table.
 * @cl_accel_flag: Indentify wheather excute opencl work.
 * @enabled_hotplug: Flag identifying core is scaling by IPA. Ture if coremask
 *                         is different from original coremask. Other policy set coremask
 *                         operation would not work when this flag is true.
 */
struct kbase_gpu_device_data {
	struct kbase_gpu_callbacks *callbacks;
#if defined(CONFIG_REGULATOR)
	struct regulator *gpu_subsys;
#endif
	void __iomem *ctrlreg;			/* GPU_CTRL */
	void __iomem *crgreg;			/* GPU_CRG */
	void __iomem *dvfsreg;			/* GPU_DVFS */
	void __iomem *lpctrlreg;		/* GPU_LPCTRL */
	void __iomem *pericrgreg;		/* PERI_CRG */
	void __iomem *lmscrgreg;		/* LMS_CRG */
	void __iomem *xctrlcpucfgreg;		/* XCTRLCPU_CFG */
	void __iomem *gpusubreg;		/* GPU_SUBSYS */
	void __iomem *apbpctrlreg;		/* APB_INNER_PCTRL*/
	void __iomem *lpisreg;			/* LPIS_ACTRL_REG */
	/* refcount for regulator,
	 * should be '1' after call regulator_enable()
	 * and be '0' after call regulator_disable().
	 */
	atomic_t regulator_refcount;
	ktime_t time_period_start;

	pid_t query_pid;
	u64 fence_count;
#if (IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ) || IS_ENABLED(CONFIG_MALI_XR_HOTPLUG))
	struct xr_gpu_coremask gpu_coremask_data;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
	u8 mdr_type;
#endif
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	struct xr_gpufreq_ops gpufreq_ops;
	u8 dvfs_type;
	atomic_t gpufreq_ready;
	atomic_t gpufreq_refcount;
	u64 opp_lowest_freq_khz;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
	struct xr_gpu_vfreq gpu_vfreq_data;
	struct work_struct set_coremask_work;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_HOTPLUG)
	bool enabled_hotplug;
#endif
#if IS_ENABLED(CONFIG_MALI_XRING_CL_DVFS)
	bool cl_accel_flag;
#endif
};

#endif /* MALI_KBASE_CONFIG_PLATFORM_H */
