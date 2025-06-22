// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _KBASE_GPU_XR_DEBUGFS_H_
#define _KBASE_GPU_XR_DEBUGFS_H_
/**
 * kbasep_xr_debugfs_init - Init xr debugfs.
 *
 * @kbdev:       The kbase device structure for the device (must be a valid pointer)
 */
void kbasep_xr_debugfs_init(struct kbase_device *kbdev);
#endif