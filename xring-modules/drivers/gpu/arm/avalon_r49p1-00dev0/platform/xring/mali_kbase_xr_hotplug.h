// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */
#ifndef _MALI_KBASE_XR_HOTPLUG_H_
#define _MALI_KBASE_XR_HOTPLUG_H_

/* The value of COREMASK_THERMAL_LEVEL is number of cores */
#define COREMASK_THERMAL_LEVEL_NONE 16
#define COREMASK_THERMAL_LEVEL_LOW  12
#define COREMASK_THERMAL_LEVEL_MED  8
#define COREMASK_THERMAL_LEVEL_HIGH 4

/**
 * kbase_xr_hotplug_set_coremask - update coremask via IPA thermal index.
 * @index:	0 1 2 3, The higher the number, the higher the temperature
 *          control level.
 *
 * This function is called when IPA detect temperature control level changes
 * and requires adjusting the number of cores.
 *
 * Return: 0 if success, or an error code on failure.
 */
int kbase_xr_hotplug_set_coremask(int index);

#endif