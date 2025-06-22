/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_THERMAL_H__
#define __XRING_THERMAL_H__

#if IS_ENABLED(CONFIG_XRING_SOC_THERMAL)
int xr_soc_tm_init(void);
void xr_soc_tm_exit(void);
#else
static int xr_soc_tm_init(void) { return 0; }
static void xr_soc_tm_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_BOARD_THERMAL)
int xr_board_tm_init(void);
void xr_board_tm_exit(void);
#else
static int xr_board_tm_init(void) { return 0; }
static void xr_board_tm_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_VIRTUAL_THERMAL)
int xr_virt_tm_init(void);
void xr_virt_tm_exit(void);
#else
static int xr_virt_tm_init(void) { return 0; }
static void xr_virt_tm_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_CPU_HOTPLUG_CDEV)
int xr_cpu_hotplug_cdev_init(void);
void xr_cpu_hotplug_cdev_exit(void);
#else
static int xr_cpu_hotplug_cdev_init(void) { return 0; }
static void xr_cpu_hotplug_cdev_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_CPU_PAUSE_CDEV)
int xr_cpu_pause_cdev_init(void);
void xr_cpu_pause_cdev_exit(void);
#else
static int xr_cpu_pause_cdev_init(void) { return 0; }
static void xr_cpu_pause_cdev_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_CPUFREQ_CDEV)
int xr_cpufreq_cdev_init(void);
void xr_cpufreq_cdev_exit(void);
#else
static int xr_cpufreq_cdev_init(void) { return 0; }
static void xr_cpufreq_cdev_exit(void) { }
#endif

#if IS_ENABLED(CONFIG_XRING_TASK_PP)
int walt_thermal_init(void);
void walt_thermal_exit(void);
#else
static inline int walt_thermal_init(void) { return 0; }
static inline void walt_thermal_exit(void) { }
#endif

#endif
