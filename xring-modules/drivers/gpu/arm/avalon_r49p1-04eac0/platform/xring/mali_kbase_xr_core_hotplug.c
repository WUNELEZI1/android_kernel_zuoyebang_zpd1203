// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <mali_kbase_hwaccess_pm.h>
#include <mali_kbase_xr_core_hotplug.h>

int kbase_xr_set_coremask(struct kbase_device *kbdev, u64 new_core_mask)
{
	unsigned long flags;
	u64 shader_present = kbdev->gpu_props.shader_present;

	mutex_lock(&kbdev->pm.lock);
	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	if ((new_core_mask & shader_present) != new_core_mask) {
		spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
		mutex_unlock(&kbdev->pm.lock);
		dev_err(kbdev->dev,
			"Invalid core mask 0x%llX: Includes non-existent cores (present = 0x%llX)",
			new_core_mask, shader_present);
		return -EINVAL;
	} else if (!(new_core_mask & shader_present & kbdev->pm.backend.ca_cores_enabled)) {
		spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
		mutex_unlock(&kbdev->pm.lock);
		dev_err(kbdev->dev,
			"Invalid core mask 0x%llX: No intersection with currently available cores"
			"(present = 0x%llX, CA enabled = 0x%llX)",
			new_core_mask,
			kbdev->gpu_props.shader_present, kbdev->pm.backend.ca_cores_enabled);
		return -EINVAL;
	}
	if (kbdev->pm.debug_core_mask != new_core_mask)
		kbase_pm_set_debug_core_mask(kbdev, new_core_mask);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
	mutex_unlock(&kbdev->pm.lock);

	return 0;
}

void kbase_xr_set_coremask_worker(struct work_struct *work)
{
	if (!work) {
		pr_err("Mali gpu Invalid work parameter");
		return;
	}
	struct kbase_device *kbdev =
		container_of(work, struct kbase_device,
			     gpu_dev_data.core_info.set_coremask_work);
	kbase_xr_set_coremask(kbdev, kbdev->gpu_dev_data.core_info.desired_coremask);
}

int kbase_xr_hotplug_core_init(struct kbase_device *kbdev)
{
	u64 *core_mask_table = NULL;
	const u32 max_core_num = hweight64(kbdev->gpu_props.shader_present);
	int i = 0;
	int j = 0;
	u64 shader_present = 0;
	u64 core_mask = 0;

	kbdev->gpu_dev_data.core_info.current_policy = HOTPLUG_POLICY_NONE;
	core_mask_table = kmalloc_array(max_core_num, sizeof(u64), GFP_KERNEL);
	if (!core_mask_table) {
		dev_err(kbdev->dev, "Kmalloc core_mask table failed");
		return -ENOMEM;
	}

	/* init core_mask_table */
	for (i = 0; i < max_core_num; i++) {
		shader_present = kbdev->gpu_props.shader_present;
		core_mask = 0;
		for (j = 0; j <= i; j++) {
			core_mask |= (1ull << (ffs(shader_present) - 1));
			shader_present &=  ~(1ull << (ffs(shader_present) - 1));
		}
		core_mask_table[i] = core_mask;
	}
	kbdev->gpu_dev_data.core_info.core_mask_table = core_mask_table;

	/* use workqueue to set coremask avoid deadlock */
	INIT_WORK(&kbdev->gpu_dev_data.core_info.set_coremask_work,
		  kbase_xr_set_coremask_worker);

	return 0;
}

void kbase_xr_hotplug_core_term(struct kbase_device *kbdev)
{
	cancel_work_sync(&kbdev->gpu_dev_data.core_info.set_coremask_work);
	if (kbdev->gpu_dev_data.core_info.core_mask_table) {
		kfree(kbdev->gpu_dev_data.core_info.core_mask_table);
		kbdev->gpu_dev_data.core_info.core_mask_table= NULL;
	}
}

static int kbase_xr_core_num_to_mask(struct kbase_device *kbdev, u32 core_num, u64 *core_mask)
{
	int max_core_num = hweight64(kbdev->gpu_props.shader_present);
	u64 *core_mask_table = kbdev->gpu_dev_data.core_info.core_mask_table;
	if (!core_mask_table) {
		dev_err(kbdev->dev, "Core mask table is null");
		return -EINVAL;
	}
	if (core_num < 1) {
		dev_warn(kbdev->dev, "Invalid core num parameter");
		return -EINVAL;
	}
	if (core_num > max_core_num)
		*core_mask = kbdev->gpu_props.shader_present;
	else
		*core_mask = core_mask_table[core_num - 1];

	return 0;
}

int kbase_xr_hotplug_core_set_num(struct kbase_device *kbdev,
				enum kbase_hotplug_policy target_policy, u32 core_num)
{
	int ret = 0;
	u64 core_mask = 0;
	enum kbase_hotplug_policy current_policy;

	mutex_lock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);
	current_policy = kbdev->gpu_dev_data.core_info.current_policy;
	if (current_policy < target_policy) {
		mutex_unlock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);
		dev_warn(kbdev->dev, "Skip policy %d because there is higer policy %d in use",
			target_policy, current_policy);
		return -EINVAL;
	}

	ret = kbase_xr_core_num_to_mask(kbdev, core_num, &core_mask);
	if (ret) {
		mutex_unlock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);
		dev_warn(kbdev->dev, " Invalid core_num param %d from policy %d",
			core_num, target_policy);
		return ret;
	}
	kbdev->gpu_dev_data.core_info.desired_coremask = core_mask;

	queue_work(system_freezable_power_efficient_wq,
			   &kbdev->gpu_dev_data.core_info.set_coremask_work);

	/* reset current policy when all core are set*/
	if (core_mask == kbdev->gpu_props.shader_present)
		kbdev->gpu_dev_data.core_info.current_policy = HOTPLUG_POLICY_NONE;
	else
		kbdev->gpu_dev_data.core_info.current_policy = target_policy;
	mutex_unlock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);

	return 0;
}

int kbase_xr_hotplug_core_set_mask(struct kbase_device *kbdev,
				enum kbase_hotplug_policy target_policy, u64 core_mask)
{
	enum kbase_hotplug_policy current_policy;

	mutex_lock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);
	current_policy = kbdev->gpu_dev_data.core_info.current_policy;
	if (current_policy < target_policy) {
		mutex_unlock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);
		dev_warn(kbdev->dev, "Skip policy %d because there is higer policy %d in use",
			target_policy, current_policy);
		return -EINVAL;
	}

	kbdev->gpu_dev_data.core_info.desired_coremask = core_mask;
	queue_work(system_freezable_power_efficient_wq,
			   &kbdev->gpu_dev_data.core_info.set_coremask_work);

	/* reset current policy when all core are set*/
	if (core_mask == kbdev->gpu_props.shader_present)
		kbdev->gpu_dev_data.core_info.current_policy = HOTPLUG_POLICY_NONE;
	else
		kbdev->gpu_dev_data.core_info.current_policy = target_policy;
	mutex_unlock(&kbdev->gpu_dev_data.core_info.hotplug_policy_lock);

	return 0;
}