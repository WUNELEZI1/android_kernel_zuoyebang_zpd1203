// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_XR_VIRTUAL_FREQ_H_
#define _MALI_KBASE_XR_VIRTUAL_FREQ_H_

/**
 * kbase_xr_vfreq_get_status - Get virtual frequency status.
 * @data:      The pointer to kbdev->dev.
 *
 * Return: True if virtual frequency is enabled, otherwise virtual frequency is disabled.
 */
int kbase_xr_vfreq_get_status(void *data);

/**
 * kbase_xr_vfreq_core_set - Virtual freq set coremask.
 * @data:      The pointer to kbdev->dev.
 * @core_num:  The num of cores to set.
 * Return: True if set coremask successfully.
 */
int kbase_xr_vfreq_core_set(void *data, u32 core_num);
#endif