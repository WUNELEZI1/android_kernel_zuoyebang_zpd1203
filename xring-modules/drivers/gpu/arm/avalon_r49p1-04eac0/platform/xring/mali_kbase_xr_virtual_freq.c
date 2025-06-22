// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#if IS_ENABLED(CONFIG_MALI_XR_CORE_HOTPLUG)
#include <mali_kbase_xr_core_hotplug.h>
#endif

int kbase_xr_vfreq_get_status(void *data)
{
	bool vfreq_status;
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;

	if (!data) {
		pr_err("Invalid input data parameter!");
		return -EINVAL;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	vfreq_status = (atomic_read(&kbdev->gpu_dev_data.vfreq_status) == 1);
	return vfreq_status;
}

int kbase_xr_vfreq_core_set(void *data, u32 core_num)
{
	int ret = 0;
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;

	if (!data) {
		pr_err("[mali gpu] Invalid input data parameter from virtual freq!");
		return -EINVAL;
	}
	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);
	if (!kbdev)
		return -ENODEV;

	ret = kbase_xr_hotplug_core_set_num(kbdev, HOTPLUG_POLICY_VIRTUAL_FREQ, core_num);

	return ret;
}