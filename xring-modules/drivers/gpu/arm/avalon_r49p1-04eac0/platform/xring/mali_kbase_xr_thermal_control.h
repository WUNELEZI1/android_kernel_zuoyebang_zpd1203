// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_XR_THERMAL_H_
#define _MALI_KBASE_XR_THERMAL_H_

int kbase_xr_thermal_control_init(struct kbase_device *kbdev);
void kbase_xr_thermal_control_term(void);
#endif