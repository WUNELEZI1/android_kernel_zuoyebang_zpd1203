// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <backend/gpu/mali_kbase_pm_internal.h>
#include <csf/ipa_control/mali_kbase_csf_ipa_control.h>
#include <mali_kbase_xr_query_info.h>

#define GPU_ACTIVE_CNT_IDX (4)
#define FRAG_ACTIVE_CNT_IDX (4)
#define COMPUTE_ACTIVE_CNT_IDX (22)
#define EXEC_CORE_ACTIVE_CNT_IDX (26)
#define TILER_ACTIVE_CNT_IDX (4)

int kbase_xr_query_fence_count(void *data, pid_t query_pid, u64 *fence_count)
{
	struct device *dev = NULL;
	struct kbase_device *kbdev = NULL;

	if (!data) {
		pr_err("Invalid data parameter");
		return -EINVAL;
	}

	dev = (struct device *)data;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -EINVAL;

	if (!fence_count) {
		dev_err(kbdev->dev, "fence_count is NULL,invalid parameter!");
		return -EINVAL;
	}

	if (kbdev->gpu_dev_data.query_pid != query_pid) {
		kbdev->gpu_dev_data.fence_count = 0;
		kbdev->gpu_dev_data.query_pid = query_pid;
	}

	*fence_count = kbdev->gpu_dev_data.fence_count;

	return 0;
}

int kbase_xr_ai_control_register(struct kbase_device *kbdev)
{
	int ret = 0;

	const struct kbase_ipa_control_perf_counter perf_counter[XR_NUM_PERF_COUNTERS] = {
		{ 1, false, KBASE_IPA_CORE_TYPE_CSHW, GPU_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_SHADER, FRAG_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_SHADER, COMPUTE_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_SHADER, EXEC_CORE_ACTIVE_CNT_IDX },
		{ 1, false, KBASE_IPA_CORE_TYPE_TILER, TILER_ACTIVE_CNT_IDX },
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

static u64 kbase_xr_read_value_cnt(struct kbase_device *kbdev, u8 type, int select_idx)
{
	switch (type) {
	case KBASE_IPA_CORE_TYPE_CSHW:
		return kbase_reg_read64(kbdev, IPA_VALUE_CSHW_OFFSET(select_idx));

	case KBASE_IPA_CORE_TYPE_MEMSYS:
		return kbase_reg_read64(kbdev, IPA_VALUE_MEMSYS_OFFSET(select_idx));

	case KBASE_IPA_CORE_TYPE_TILER:
		return kbase_reg_read64(kbdev, IPA_VALUE_TILER_OFFSET(select_idx));

	case KBASE_IPA_CORE_TYPE_SHADER:
		return kbase_reg_read64(kbdev, IPA_VALUE_SHADER_OFFSET(select_idx));

	default:
		WARN(1, "Unknown core type: %u\n", type);
		return 0;
	}
}

static inline u64 kbase_xr_get_raw_value(struct kbase_device *kbdev,
				     struct kbase_ipa_control_prfcnt *prfcnt,
				     bool gpu_ready)
{
	u64 raw_value = 0;

	if (unlikely(kbdev == NULL || prfcnt == NULL)) {
		pr_err("Wrong input for get raw value");
		return 0;
	}

	if (gpu_ready)
		raw_value = kbase_xr_read_value_cnt(kbdev, (u8)prfcnt->type,
					   prfcnt->select_idx);
	else
		raw_value = prfcnt->latest_raw_value;

	prfcnt->latest_raw_value = raw_value;

	return raw_value;
}

static int kbase_xr_ai_control_query(struct kbase_device *kbdev, const void *client,
			    u64 *values, size_t num_values)
{
	struct kbase_ipa_control *ipa_ctrl = NULL;
	struct kbase_ipa_control_session *session = NULL;
	struct kbase_ipa_control_prfcnt *prfcnt = NULL;
	size_t i = 0;
	unsigned long flags = 0;
	bool gpu_ready = false;

	if (unlikely(kbdev == NULL)) {
		pr_err("AI query: wrong input kbdev");
		return -ENODEV;
	}

	if (!client || values == NULL) {
		dev_err(kbdev->dev, "AI query: wrong input arguments");
		return -EINVAL;
	}

	ipa_ctrl = &kbdev->csf.ipa_control;
	session = (struct kbase_ipa_control_session *)client;

	if (!session->active) {
		dev_err(kbdev->dev,
			"AI query: attempt to query inactive session");
		return -EINVAL;
	}

	if (num_values < session->num_prfcnts) {
		dev_err(kbdev->dev,
			"Not enough space (%zu) to return all counter values (%zu)",
			num_values, session->num_prfcnts);
		return -EINVAL;
	}

	spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
	gpu_ready = kbdev->pm.backend.gpu_ready;

	for (i = 0; i < session->num_prfcnts; i++) {
		prfcnt = &session->prfcnts[i];
		values[i] = kbase_xr_get_raw_value(kbdev, prfcnt, gpu_ready);
	}

	spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

	for (i = session->num_prfcnts; i < num_values; i++)
		values[i] = 0;

	return 0;
}

int kbase_xr_query_hwcounter(void *data, u64 *values, size_t num_values)
{
	int ret = 0;
	struct device *dev = (struct device *)data;
	struct kbase_device *const kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	ret = kbase_xr_ai_control_query(kbdev, kbdev->pm.backend.metrics.ai_control_client,
			values, num_values);

	return ret;
}
