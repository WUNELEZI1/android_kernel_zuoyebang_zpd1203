// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_PM_H_
#define _MALI_KBASE_PM_H_
/**
 * kbase_pm_config_memory_repair - config gpu memory repair from sequence to concurrent.
 *
 * @kbdev: The kbase device structure for the device (must be a valid pointer)
 *
 */
void kbase_xr_config_memory_repair(struct kbase_device *kbdev);
#endif /* _MALI_KBASE_PM_H_ */
