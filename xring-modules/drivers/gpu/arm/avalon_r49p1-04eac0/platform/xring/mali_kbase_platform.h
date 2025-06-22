/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _MALI_KBASE_PLATFORM_H_
#define _MALI_KBASE_PLATFORM_H_

#if IS_ENABLED(CONFIG_MALI_XR_MDR)
#include <mali_kbase_xr_mdr.h>
#endif
#include <mali_kbase_xr_dvfs.h>
#if IS_ENABLED(CONFIG_MALI_XR_CORE_HOTPLUG)
#include <mali_kbase_xr_core_hotplug.h>
#endif
#if IS_ENABLED(CONFIG_MALI_XR_THERMAL_CONTROL)
#include <dt-bindings/xring/xr_thermal_cdev.h>
#endif
#include <mali_kbase_xr_pm.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <mali_kbase_xr_pm.h>

/**
 * struct kbase_gpu_device_data - all gpu platform data in device level.
 * @callbacks: The callbacks gpu implements.
 * @vdd_gpu: Gpu buck regulator.
 * @gpu_subsys: Mtcmos regulator.
 * @power_state: Current state of gpu power.
 * @power_off_hrtimer: Timer to do control power off.
 * @power_off_timer_workq: Work queue to do power off.
 * @power_off_timer_work: Work to do power off.
 * @power_update_lock: Protect power update.
 * @regulator_refcount: Refcount for regulator, should be '1' after call regulator_enable()
 *                     and be '0' after call regulator_disable().
 * @query_pid: The pid to query fence.
 * @fence_count: The number of times the fence has been triggered since the first query.
 * @mdr_type: The error type to be reported to mdr.
 * @mdr_threshold: The threshold for mdr.
 * @mdr_count: The count for mdr.
 * @mdr_lock: The mutex lock for mdr.
 * @gpu_mdr_workq: Work queue to report mdr.
 * @gpu_mdr_work: Work to report mdr.
 * @modid: The modid for mdr.
 * @kctx: The kbase context of each process to report mdr.
 * @arg1: The parameter 1 for mdr report funciton.
 * @arg2: The parameter 2 for mdr report function.
 * @gpufreq_ops: Operation functions to be registered to DVFS.
 * @dvfs_type: The gpu dvfs debug type.
 * @gpufreq_ready: The ops register status.
 * @gpufreq_refcount: represent gpufreq suspend/resume status, 1 for resume, 0 for suspend.
 * @opp_lowest_freq_khz: The lowest gpu freq in dts opp table.
 * @cl_accel_flag: Indentify whether excute opencl work.
 * @core_info: The structure used to set coremask.
 * @vfreq_status: Whether virtual freq is enabled.
 */
struct kbase_gpu_device_data {
	struct kbase_gpu_callbacks *callbacks;
#if defined(CONFIG_REGULATOR)
	struct regulator *vdd_gpu;
	struct regulator *gpu_subsys;
	enum kbase_xr_power_state power_state;
#endif
	struct hrtimer power_off_hrtimer;
	struct workqueue_struct *power_off_timer_workq;
	struct work_struct power_off_timer_work;
	struct mutex power_update_lock;
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
#if IS_ENABLED(CONFIG_MALI_XR_SMART_GAMING)
	pid_t query_pid;
	u64 fence_count;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_MDR)
	u8 mdr_type;
	u8 mdr_threshold[MDR_GPU_TYPES_COUNT];
	u8 mdr_count[MDR_GPU_TYPES_COUNT];
	struct mutex mdr_lock;
	struct workqueue_struct *gpu_mdr_workq;
	struct work_struct gpu_mdr_work;
	u32 modid;
	struct kbase_context *kctx;
	u32 arg1;
	u32 arg2;
#endif
#if IS_ENABLED(CONFIG_MALI_MIDGARD_DVFS)
	struct xr_gpufreq_ops gpufreq_ops;
	u8 dvfs_type;
	atomic_t gpufreq_ready;
	atomic_t gpufreq_refcount;
	u64 opp_lowest_freq_khz;
#endif
#if IS_ENABLED(CONFIG_MALI_XRING_CL_DVFS)
	bool cl_high_accel_flag;
	bool cl_med_accel_flag;
	bool cl_low_accel_flag;
	bool cl_no_accel_flag;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_CORE_HOTPLUG)
	struct kbase_core_mask_info core_info;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
	atomic_t vfreq_status;
#endif
#if IS_ENABLED(CONFIG_MALI_XR_THERMAL_CONTROL)
	struct xr_gpuhp_cdev_ops thermal_ops;
#endif
};

#endif /* MALI_KBASE_CONFIG_PLATFORM_H */
