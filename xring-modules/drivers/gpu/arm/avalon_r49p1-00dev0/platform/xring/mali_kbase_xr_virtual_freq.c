// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <platform/xring/mali_kbase_config_platform.h>
#include <backend/gpu/mali_kbase_pm_internal.h>

#if (IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ) || IS_ENABLED(CONFIG_MALI_XR_HOTPLUG))
int kbase_xr_core_mask_to_num(struct kbase_device *kbdev, u64 core_mask, u32 *core_num) {
	u64 prev_core_mask = core_mask;
	*core_num = 0;

	while (core_mask != 0) {
		*core_num += 1;
		core_mask = (core_mask & (core_mask - 1));
	}

	dev_dbg(kbdev->dev, "input core_mask:0x%llx, output core_num:%d", prev_core_mask, *core_num);
	return 0;
}

int kbase_xr_core_num_to_mask(struct kbase_device *kbdev, u32 core_num, u64 *core_mask) {
	struct xr_gpu_coremask *gpu_coremask_data = &kbdev->gpu_dev_data.gpu_coremask_data;

	if (core_num > gpu_coremask_data->max_core_num) {
		dev_warn(kbdev->dev, "input core number:%d over gpu max core number:%d",
			 core_num, gpu_coremask_data->max_core_num);
		return 0;
	}

	*core_mask = gpu_coremask_data->all_coremask_table[core_num];
	dev_dbg(kbdev->dev, "input core_num:%d, output core_mask:0x%llx", core_num, *core_mask);

	return 0;
}

int kbase_xr_init_all_coremask_table(struct kbase_device *kbdev) {
	int err;
	u64 core_mask;
	int core_count;
	int i = 0;
	u64 remaining_core_mask = kbdev->gpu_props.shader_present;
	struct xr_gpu_coremask *gpu_coremask_data = &kbdev->gpu_dev_data.gpu_coremask_data;
#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
	struct xr_gpu_vfreq *gpu_vfreq_data = &kbdev->gpu_dev_data.gpu_vfreq_data;
#endif
	/* calculate cores count according to available shader core mask */
	kbase_xr_core_mask_to_num(kbdev, remaining_core_mask, &core_count);

	if (!core_count) {
		err= -EINVAL;
		goto failed_get_count;
	}

	gpu_coremask_data->max_core_num = core_count;

	gpu_coremask_data->all_coremask_table =
		kmalloc_array((size_t)(core_count + 1), sizeof(u64), GFP_KERNEL);

        if (!gpu_coremask_data->all_coremask_table) {
		err = -ENOMEM;
		goto failed_alloc_mem;
	}

	/* generate coremask table */
	gpu_coremask_data->all_coremask_table[0] = remaining_core_mask;
	for (u32 core_num = 1; core_num <= core_count; core_num++) {
		/* calculate coremask */
		core_mask = 0;
		remaining_core_mask = kbdev->gpu_props.shader_present;
		for (i = core_num; i > 0; i--) {
			int core = ffs(remaining_core_mask);

			if (!core) {
				dev_warn(kbdev->dev, "Over GPU max cores, core\n");
				break;
			}

			core_mask |= (1ull << (core - 1));
			remaining_core_mask &= ~(1ull << (core - 1));
		}

		if (!core_mask) {
			dev_warn(kbdev->dev, "Invalid core number\n");
			break;
		}

		gpu_coremask_data->all_coremask_table[core_num] = core_mask;

		dev_dbg(kbdev->dev, "output core_num=%d, core_mask=0x%llx\n", core_num, core_mask);
	}

	dev_info(kbdev->dev, "success to init coremask table, core_num:%d", gpu_coremask_data->max_core_num);

	return 0;

failed_alloc_mem:
	kfree(gpu_coremask_data->all_coremask_table);
	gpu_coremask_data->all_coremask_table = NULL;
failed_get_count:
#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
	gpu_vfreq_data->vfreq_status = false;
#endif
	return err;
}

void kbase_xr_term_all_coremask_table(struct kbase_device *kbdev) {
	struct xr_gpu_coremask *gpu_coremask_data = &kbdev->gpu_dev_data.gpu_coremask_data;
	kfree(gpu_coremask_data->all_coremask_table);
	gpu_coremask_data->all_coremask_table = NULL;
}
#endif

#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
void kbase_xr_vfreq_set_core_work(struct work_struct *work)
{
	struct kbase_device *kbdev =
		container_of(work, struct kbase_device,
			     gpu_dev_data.gpu_vfreq_data.set_coremask_work);
	unsigned long flags;
	u64 shader_present;
	u64 desired_coremask;
	shader_present = kbdev->gpu_props.shader_present;
	desired_coremask = kbdev->gpu_dev_data.gpu_vfreq_data.desired_coremask;

	mutex_lock(&kbdev->pm.lock);
	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	if (kbdev->pm.debug_core_mask != desired_coremask)
		kbase_pm_set_debug_core_mask(kbdev, desired_coremask);
	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);
	mutex_unlock(&kbdev->pm.lock);
}

int kbase_xr_vfreq_init(struct kbase_device *kbdev)
{
	struct xr_gpu_vfreq *gpu_vfreq_data = &kbdev->gpu_dev_data.gpu_vfreq_data;
	u64 shader_present = kbdev->gpu_props.shader_present;

	KBASE_DEBUG_ASSERT(gpu_vfreq_data != NULL);

	/* use workqueue to set coremask avoid deadlock */
	INIT_WORK(&kbdev->gpu_dev_data.gpu_vfreq_data.set_coremask_work,
		  kbase_xr_vfreq_set_core_work);

	gpu_vfreq_data->desired_coremask = shader_present;

	gpu_vfreq_data->vfreq_status = true;

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
int kbase_xr_vfreq_set_core(void *data, u32 core_num) {
	u64 coremask;
	u64 debug_core_mask;
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;
	struct xr_gpu_vfreq *gpu_vfreq_data;

	if (!data) {
		pr_err("[mali] Invalid input data parameter!");
		return -EINVAL;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	struct xr_gpu_coremask *gpu_coremask_data = &kbdev->gpu_dev_data.gpu_coremask_data;

#if IS_ENABLED(CONFIG_MALI_XR_HOTPLUG)
	/* don't set coremask when high temperature policy control gpu cores*/
	if (kbdev->kbase_gpu_device_data.enabled_hotplug) {
		dev_dbg(kbdev->dev, "High temperature policy control coremask, can't set coremask!\n");
		return -EINVAL;
	}
#endif /* CONFIG_MALI_XR_HOTPLUG */

	/* When smartgaming running, dvfs can't control cores.*/
	if (kbdev->gpu_dev_data.query_pid > 0) {
		dev_dbg(kbdev->dev,"DVFS Failed to set coremask, smartgaming running.");
		return -ENODEV;
	}

	gpu_vfreq_data = &kbdev->gpu_dev_data.gpu_vfreq_data;

	if (!gpu_vfreq_data->vfreq_status) {
		dev_dbg(kbdev->dev, "virtual freq set core mask function is disable.");
		return -ENODEV;
	}

	if ((core_num > 0) && (core_num <= gpu_coremask_data->max_core_num)) {
		/* virtual freq convert core number to coremask */
		kbase_xr_core_num_to_mask(kbdev, core_num, &coremask);
	} else {
		/* real freq set coremask to shader present */
		coremask = kbdev->gpu_props.shader_present;
	}

	mutex_lock(&gpu_vfreq_data->lock);
	gpu_vfreq_data->desired_coremask = coremask;
	debug_core_mask = kbdev->pm.debug_core_mask;

	if (gpu_vfreq_data->desired_coremask != debug_core_mask) {
		queue_work(system_freezable_power_efficient_wq,
			   &gpu_vfreq_data->set_coremask_work);
	}
	mutex_unlock(&gpu_vfreq_data->lock);

	return 0;
}
#endif /* CONFIG_MALI_XR_VIRTUAL_DEVFREQ */

#if IS_ENABLED(CONFIG_MALI_XR_VIRTUAL_DEVFREQ)
int kbase_xr_vfreq_get_status(void *data) {
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

	vfreq_status = kbdev->gpu_dev_data.gpu_vfreq_data.vfreq_status;
	return vfreq_status;
}

void kbase_xr_vfreq_set_status(struct kbase_device *kbdev, bool flag)
{
	struct xr_gpu_vfreq *gpu_vfreq_data;

	gpu_vfreq_data = &kbdev->gpu_dev_data.gpu_vfreq_data;

	mutex_lock(&gpu_vfreq_data->lock);
	gpu_vfreq_data->vfreq_status = flag;
	mutex_unlock(&gpu_vfreq_data->lock);

	return;
}
#endif