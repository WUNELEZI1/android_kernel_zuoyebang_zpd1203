// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <dt-bindings/xring/xr_thermal_cdev.h>
#if IS_ENABLED(CONFIG_MALI_XR_CORE_HOTPLUG)
#include <mali_kbase_xr_core_hotplug.h>
#endif

int kbase_xr_thermal_control_core_set(void *data, u32 core_num)
{

	int ret = 0;
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;

	if (!data) {
		pr_err("[mali gpu] Invalid input data parameter from thermal control!");
		return -EINVAL;
	}
	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);
	if (!kbdev)
		return -ENODEV;

	ret = kbase_xr_hotplug_core_set_num(kbdev, HOTPLUG_POLICY_THERMAL_CONTROL, core_num);

	return ret;
}

int kbase_xr_thermal_control_init(struct kbase_device *kbdev)
{
	int ret = 0;
	struct xr_gpuhp_cdev_ops *ops = &kbdev->gpu_dev_data.thermal_ops;
	ops->data = (void *)kbdev->dev;
	ops->update_hotplug_coremask = kbase_xr_thermal_control_core_set;
	ret = xr_gpuhp_cdev_ops_register(ops);
	return ret;
}

void kbase_xr_thermal_control_term(void)
{
	xr_gpuhp_cdev_ops_unregister();
}
