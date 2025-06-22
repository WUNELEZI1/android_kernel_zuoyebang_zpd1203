// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _KBASE_GPU_MDR_H_
#define _KBASE_GPU_MDR_H_
/**
 * kbase_xr_mdr_exception_register - Register gpu exception to mdr.
 *
 * @kbdev:       The kbase device structure for the device (must be a valid pointer)
 */
int kbase_xr_mdr_exception_register(struct kbase_device *kbdev);
/**
 * kbase_xr_mdr_exception_unregister - Unregister gpu exception to mdr.
 *
 * @kbdev:       The kbase device structure for the device (must be a valid pointer)
 */
int kbase_xr_mdr_exception_unregister(struct kbase_device *kbdev);
#endif
