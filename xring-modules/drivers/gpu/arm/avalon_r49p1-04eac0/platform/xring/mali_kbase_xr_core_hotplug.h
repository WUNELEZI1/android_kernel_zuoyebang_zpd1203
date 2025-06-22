// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2024, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _KBASE_GPU_CORE_HOTPLUG_H_
#define _KBASE_GPU_CORE_HOTPLUG_H_

/* all hot plug policies, priority from high to low */
enum kbase_hotplug_policy {
	HOTPLUG_POLICY_MANUAL_CONTROL,
	HOTPLUG_POLICY_THERMAL_CONTROL,
	HOTPLUG_POLICY_SMART_GAMING,
	HOTPLUG_POLICY_VIRTUAL_FREQ,
	HOTPLUG_POLICY_NONE
};

struct kbase_core_mask_info {
	u64 *core_mask_table;
	enum kbase_hotplug_policy current_policy;
	struct mutex hotplug_policy_lock;
	struct work_struct set_coremask_work;
	u64 desired_coremask;
};

/**
 * kbase_xr_hotplug_core_init - Init core hotplug.
 *
 * @kbdev:      The kbdev.
 */
int kbase_xr_hotplug_core_init(struct kbase_device *kbdev);

/**
 * kbase_xr_hotplug_core_term - Term core hotplug.
 *
 * @kbdev:      The kbdev.
 */
void kbase_xr_hotplug_core_term(struct kbase_device *kbdev);

/**
 * kbase_xr_hotplug_core_set_num - Set coremask with corenum parameter.
 *
 * @kbdev:         The kbdev.
 * @target_policy: Which policy need to set coremask.
 * @core_num:      The num of cores to set.
 * Return: True if set coremask successfully.
 */
int kbase_xr_hotplug_core_set_num(struct kbase_device *kbdev,
				enum kbase_hotplug_policy target_policy, u32 core_num);

/**
 * kbase_xr_hotplug_core_set_mask - Set coremask with coremask parameter.
 *
 * @kbdev:         The kbdev.
 * @target_policy: Which policy need to set coremask.
 * @core_mask:     The coremask to set.
 * Return: True if set coremask successfully.
 */
int kbase_xr_hotplug_core_set_mask(struct kbase_device *kbdev,
				enum kbase_hotplug_policy target_policy, u64 core_mask);
#endif