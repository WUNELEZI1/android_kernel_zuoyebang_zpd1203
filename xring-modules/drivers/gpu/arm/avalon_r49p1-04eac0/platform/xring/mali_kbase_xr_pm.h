// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_PM_H_
#define _MALI_KBASE_PM_H_

enum kbase_xr_power_state {
	KBASE_XR_POWER_OFF,
	KBASE_XR_SUBCHIP_ON,
	KBASE_XR_SUBSYS_ON,
	KBASE_XR_POWER_ON,
	KBASE_XR_SUBSYS_OFF,
	KBASE_XR_SUBCHIP_PEND_OFF,
	KBASE_XR_SUBCHIP_OFF
};

/**
 * kbase_xr_power_init - init xr platform power config.
 *
 * @kbdev: The kbase device structure for the device (must be a valid pointer)
 *
 */
int kbase_xr_power_init(struct kbase_device *kbdev);

/**
 * kbase_xr_power_term - term xr platform power config.
 *
 * @kbdev: The kbase device structure for the device (must be a valid pointer)
 *
 */
void kbase_xr_power_term(struct kbase_device *kbdev);

/**
 * kbase_pm_config_memory_repair - config gpu memory repair from sequence to concurrent.
 *
 * @kbdev: The kbase device structure for the device (must be a valid pointer)
 *
 */
void kbase_xr_config_memory_repair(struct kbase_device *kbdev);
#endif /* _MALI_KBASE_PM_H_ */
