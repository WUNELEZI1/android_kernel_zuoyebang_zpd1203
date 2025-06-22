// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <backend/gpu/mali_kbase_pm_internal.h>
#include <csf/ipa_control/mali_kbase_csf_ipa_control.h>
#include <mali_kbase_xr_smart_gaming.h>
#include <soc/xring/perf_actuator.h>
#if IS_ENABLED(CONFIG_MALI_XR_CORE_HOTPLUG)
#include <mali_kbase_xr_core_hotplug.h>
#endif

#define GPU_ACTIVE_CNT_IDX (4)
#define FRAG_ACTIVE_CNT_IDX (4)
#define COMPUTE_ACTIVE_CNT_IDX (22)
#define SHADER_CORE_ACTIVE_CNT_IDX (53)

#define CMD_GET_GPU_FENCE	PERF_RW_CMD(GET_GPU_FENCE, struct kbase_fence_info)
#define CMD_GET_GPU_COUNTER	PERF_RW_CMD(GET_GPU_COUNTER, struct kbase_counter_info)
#define CMD_SET_GPU_CORENUM	PERF_RW_CMD(SET_GPU_CORENUM, u32)

int kbase_xr_query_fence_count(pid_t query_pid, u64 *fence_count)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;

	if (!fence_count) {
		pr_err("mali fence_count is NULL,invalid parameter!");
		return -EINVAL;
	}

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		if (kbdev->gpu_dev_data.query_pid != query_pid) {
			kbdev->gpu_dev_data.fence_count = 0;
			kbdev->gpu_dev_data.query_pid = query_pid;
		}
		*fence_count = kbdev->gpu_dev_data.fence_count;
	}
	kbase_device_put_list(kbdev_list);

	return 0;
}

int kbase_xr_ai_control_register(struct kbase_device *kbdev)
{
	int ret = 0;

	const struct kbase_ipa_control_perf_counter perf_counter[XR_NUM_PERF_COUNTERS] = {
		{ 1, false, KBASE_IPA_CORE_TYPE_CSHW, GPU_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_SHADER, FRAG_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_SHADER, COMPUTE_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_SHADER, SHADER_CORE_ACTIVE_CNT_IDX },
	};

	ret = kbase_ipa_control_register(
			kbdev, perf_counter, XR_NUM_PERF_COUNTERS,
			&kbdev->pm.backend.metrics.ai_control_client);

	if (ret) {
		dev_err(kbdev->dev,
			"Failed to register xr IPA with kbase_ipa_control: ret=%d",
			ret);
		return -EINVAL;
	}

	return 0;
}

int kbase_xr_query_hwcounter(u64 *values, size_t num_values)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	int ret = 0;

	if (!values)
		return -EINVAL;

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		ret = kbase_ipa_control_query(kbdev, kbdev->pm.backend.metrics.ai_control_client,
				values, num_values, NULL, true);
	}
	kbase_device_put_list(kbdev_list);
	return ret;
}

int kbase_xr_get_fence_info(void __user *uarg)
{
	struct kbase_fence_info fence_info;
	int query_pid = 0;
	u64 fence_count = 0;
	int ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&fence_info, uarg, sizeof(struct kbase_fence_info))) {
		return -EFAULT;
	}

	query_pid = fence_info.pid;
	if (query_pid < 0) {
		pr_err("gpu fence get wrong pid!");
		return -EFAULT;
	}
	ret = kbase_xr_query_fence_count(query_pid, &fence_count);
	if (ret != 0) {
		pr_err("gpu fence get unexcepted result: %d!", ret);
		return -EFAULT;
	}
	fence_info.fence_count = fence_count;

	if (copy_to_user(uarg, &fence_info, sizeof(struct kbase_fence_info))) {
		return -EIO;
	}

	return ret;
}

int kbase_xr_get_counter_info(void __user *uarg)
{
	struct kbase_counter_info counter_info;
	size_t num = 0;
	int ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&counter_info, uarg, sizeof(struct kbase_counter_info))) {
		return -EFAULT;
	}

	num = counter_info.num_value;
	if (num <= 0 || num > XR_NUM_PERF_COUNTERS) {
		pr_err("gpu count num value err!");
		return -EFAULT;
	}
	ret = kbase_xr_query_hwcounter(&counter_info.counter_value[0], num);
	if (ret != 0) {
		pr_err("gpu count get unexpected result: %d!", ret);
		return -EFAULT;
	}

	if (copy_to_user(uarg, &counter_info, sizeof(struct kbase_counter_info))) {
		return -EIO;
	}

	return ret;
}

int kbase_xr_smart_gaming_core_set(void __user *uarg)
{
	struct list_head *entry = NULL;
	const struct list_head *kbdev_list = NULL;
	u32 core_num = 0;
	int ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&core_num, uarg, sizeof(core_num))) {
		return -EFAULT;
	}

	kbdev_list = kbase_device_get_list();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;

		kbdev = list_entry(entry, struct kbase_device, entry);
		ret = kbase_xr_hotplug_core_set_num(kbdev, HOTPLUG_POLICY_SMART_GAMING, core_num);
	}
	kbase_device_put_list(kbdev_list);

	return ret;
}

void kbase_gpu_query_register(void)
{
	register_perf_actuator(CMD_GET_GPU_FENCE, kbase_xr_get_fence_info);
	register_perf_actuator(CMD_GET_GPU_COUNTER, kbase_xr_get_counter_info);
	register_perf_actuator(CMD_SET_GPU_CORENUM, kbase_xr_smart_gaming_core_set);
}

void kbase_gpu_query_unregister(void)
{
	unregister_perf_actuator(CMD_GET_GPU_FENCE);
	unregister_perf_actuator(CMD_GET_GPU_COUNTER);
	unregister_perf_actuator(CMD_SET_GPU_CORENUM);
}
