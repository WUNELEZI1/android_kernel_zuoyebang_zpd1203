// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include "mali_kbase_xr_hotplug.h"
#include <mali_kbase_hwaccess_pm.h>
#include "mali_kbase_config_platform.h"
#include "mali_kbase_xr_virtual_freq.h"

#if IS_ENABLED(CONFIG_MALI_XR_HOTPLUG)
static u64 kbase_xr_hotplug_get_coremask(struct kbase_device *kbdev, int index){
	u64 core_mask;
	const int level_count = 4;
	static int thermal_level_table[] = {
		COREMASK_THERMAL_LEVEL_NONE,
		COREMASK_THERMAL_LEVEL_LOW,
		COREMASK_THERMAL_LEVEL_MED,
		COREMASK_THERMAL_LEVEL_HIGH,
	};

	if (index < 0 || index >= level_count) {
		dev_warn(kbdev->dev,
			     "[hotplug] Invalid index set %d\n", index);
		return kbdev->pm.debug_core_mask;
	}

	kbase_xr_core_num_to_mask(kbdev, thermal_level_table[index], &core_mask);

	return core_mask;
}
#endif /* CONFIG_MALI_XR_HOTPLUG */

#if IS_ENABLED(CONFIG_MALI_XR_HOTPLUG)
int kbase_xr_hotplug_set_coremask(struct kbase_device *kbdev, int index)
{
	int err = 0;
	u64 core_mask;
	u64 shader_present;
	unsigned long flags;

	/* index convert to coremask */
	core_mask = kbase_xr_hotplug_get_coremask(kbdev, index);

	mutex_lock(&kbdev->pm.lock);
	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	shader_present = kbdev->gpu_props.shader_present;

	kbdev->kbase_gpu_device_data.enabled_hotplug = (core_mask != shader_present);

	if (kbdev->pm.debug_core_mask != core_mask) {
		kbase_pm_set_debug_core_mask(kbdev, core_mask);
	}

	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
	mutex_unlock(&kbdev->pm.lock);
	return err;
}
EXPORT_SYMBOL(kbase_xr_hotplug_set_coremask);
#else
int kbase_xr_hotplug_set_coremask(int index)
{
	pr_debug("[mali]CONFIG_MALI_XR_HOTPLUG is disabled.\n");
	return 0;
}
EXPORT_SYMBOL(kbase_xr_hotplug_set_coremask);
#endif /* CONFIG_MALI_XR_HOTPLUG */