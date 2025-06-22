// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _KBASE_GPU_MDR_H_
#define _KBASE_GPU_MDR_H_
#include <mali_kbase.h>
#include <mdr_pub.h>

enum kbase_xr_mdr_types {
    MDR_GPU_IRQ_FAULT,
    MDR_GPU_BUS_FAULT,
    MDR_GPU_FAULT,
    MDR_GPU_CS_FAULT,
    MDR_GPU_UNHANDLE_PAGE_FAULT,
    MDR_MCU_UNHANDLE_PAGE_FAULT,
    MDR_GPU_SOFT_RESET_TIME_OUT,
    MDR_GPU_HARD_RESET_TIME_OUT,
    MDR_GPU_BIT_STUCK,
    MDR_GPU_REGULATOR_ON_FAIL,
    MDR_GPU_REGULATOR_OFF_FAIL,
    MDR_GPU_FENCE_TIMEOUT,
    MDR_GPU_INTERRUPT_TIMEOUT,
    MDR_GPU_CS_FATAL,
    MDR_GPU_TYPES_COUNT
};

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

/**
 * kbase_xr_mdr_system_error - Report gpu exception to mdr.
 *
 * @kbdev:       The kbase device structure for the device (must be a valid pointer)
 * @kctx:        The kbase context structure
 * @modid:       The modid.
 * @arg1:        The reserved parameter, not used.
 * @arg2:        The reserved parameter, not used.
 */
void kbase_xr_mdr_system_error(struct kbase_device *kbdev, struct kbase_context *kctx, u32 modid, u32 arg1, u32 arg2);

/**
 * kbase_xr_mdr_init - Init mdr.
 * @kbdev:       The kbase device structure for the device (must be a valid pointer)
 */
void kbase_xr_mdr_init(struct kbase_device *kbdev);
#endif
