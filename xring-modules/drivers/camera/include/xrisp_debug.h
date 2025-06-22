/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_DEBUG_H_
#define _XRISP_DEBUG_H_

#include <linux/debugfs.h>
#include <linux/ktime.h>
#include "xrisp_log.h"

#define XRISP_DEBUG
#define XRISP_DEBUGFS_ROOT_NAME "xr-isp"
#define XRISP_TIME_DEBUG

#ifdef XRISP_DEBUG
/* define module debug macro*/
#define CAM_BUF_DEBUG
#define CAM_RPROC_DEBUG
#define CAM_CLK_DEBUG
#endif /* XRISP_DEBUG */

/* mbox debug config */
#define XRISP_MBOX_DEBUG_CONFIG

/* XRISP_TIMEUSE_DEBUG */
#define ISP_POWER_ON_TIME_MAX	 (600000)
#define ISP_RPROC_ALLOC_TIME_MAX (40000)
#define ISP_IOCTL_TIMEMS_MAX	 (200)

#ifdef XRISP_TIME_DEBUG
#define KTIME_DEBUG_DEFINE_START()                                                       \
	ktime_t start, duration_us;                                                      \
	start = ktime_get()

#define KTIME_DEBUG_TIMEOUT_CHECK(timeout_us)                                            \
	do {                                                                             \
		duration_us = ktime_us_delta(ktime_get(), start);                        \
		if (duration_us >= timeout_us)                                           \
			XRISP_PR_WARN("timeout, use %lld us\n", duration_us);           \
	} while (0)
#else
#define KTIME_DEBUG_DEFINE_START()
#define KTIME_DEBUG_TIMEOUT_CHECK(timeout_us)
#endif

extern uint64_t trigger_edr_report;

struct dentry *xrisp_debugfs_get_root(void);
int xring_regdump_init(void);
void xring_regdump_exit(void);

int xrisp_mbox_debug_init(void);
void xrisp_mbox_debug_exit(void);

#endif /* _XRISP_DEBUG_H_ */
