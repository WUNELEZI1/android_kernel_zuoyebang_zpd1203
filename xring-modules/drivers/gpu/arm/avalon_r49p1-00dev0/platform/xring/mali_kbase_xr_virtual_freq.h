// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_XR_VIRTUAL_FREQ_H_
#define _MALI_KBASE_XR_VIRTUAL_FREQ_H_

/**
 * struct xr_gpu_vfreq - Date store gpu virtual devfreq info
 *
 * @lock:             muntex lock for count and coremask in virtual devfreq.
 * @opp_count:       the number of virtual devfreq opp, the property of "opp-type" is
 *                    "gpu-virtual-freq" in each virtual devfreq.
 * @desired_coremask: bit mask of shader core, this mask will be set when call
 *                    set_coremask_work.
 * @set_coremask_work:set desired_coremask to shader core mask by queue work.
 * @vfreq_status:       true if virtual devfreq is enabled, otherwise false. When user want
 *                    to use core_mask sysfile, "echo 0 > vfreq_status" firstly to
 *                    make vfreq_status disabled.
 */
struct xr_gpu_vfreq {
	int opp_count;
	bool vfreq_status;
	u64 desired_coremask;
	struct mutex lock;
	struct work_struct set_coremask_work;
};

/**
 * struct xr_gpu_coremask - Date store gpu all coremask data
 *
 * @xr_coremask_table lookup table for converting between number of cores and
 *                       core mask.
 * @max_core_num:     the max number of cores.
 * @all_coremask_table: array to store coremask, index use number of cores;
 */
struct xr_gpu_coremask {
	int max_core_num;
	u64 *all_coremask_table;
};

/**
 * kbase_xr_vfreq_init - virtual devfreq data initialization.
 * @kbdev:     Kbase device where virtual devfreq is used.
 *
 * This function is called when virtual devfreq initialization.
 *
 * Return: 0 if success, or an error code on failure.
 */
int kbase_xr_vfreq_init(struct kbase_device *kbdev);

/**
 * kbase_xr_vfreq_set_core - Set core mask.
 * @data:      pointer to kbdev->dev.
 * @core_num:  new number of cores will be set.
 *
 * Return: 0 if success, or an error code on failure.
 */
int kbase_xr_vfreq_set_core(void *data, u32 core_num);

/**
 * kbase_xr_vfreq_get_status - get virtual frequency status.
 * @data:      pointer to kbdev->dev.
 *
 * Return: true if virtual frequency is enabled, otherwise virtual frequency is disabled.
 */
int kbase_xr_vfreq_get_status(void *data);

/**
 * kbase_xr_vfreq_set_status - Set virtual frequency is available.
 * @flag:      ture if virtual devfreq function is enabled, other wise false.
 * @kbdev:     Kbase device.
 *
 * This function is called when write filesys "vfreq_status"
 *
 * Return: 0 if success, or an error code on failure.
 */
void kbase_xr_vfreq_set_status(struct kbase_device *kbdev, bool flag);

#endif /* _MALI_KBASE_XR_VIRTUAL_FREQ_H_ */

/**
 * kbase_xr_core_mask_to_num - Convert core mask to number of cores.
 * @core_mask: Shader core mask
 * @core_num:  Point to staore number of cores
 */
int kbase_xr_core_mask_to_num(struct kbase_device *kbdev, u64 core_mask, u32 *core_num);

/**
 * kbase_xr_core_num_to_mask - Convert number of cores to core mask.
 * @core_num: Shader number of cores
 * @core_mask: Point to store shader coremask
 */
int kbase_xr_core_num_to_mask(struct kbase_device *kbdev, u32 core_num, u64 *core_mask);

/**
 * kbase_xr_init_all_coremask_table - Generate a conversion table for mask and core.
 * @kbdev:     Kbase device.
 */
int kbase_xr_init_all_coremask_table(struct kbase_device *kbdev);

/**
 * kbase_xr_term_all_coremask_table - Free allocated memory.
 * @kbdev:     Kbase device.
 */
void kbase_xr_term_all_coremask_table(struct kbase_device *kbdev);