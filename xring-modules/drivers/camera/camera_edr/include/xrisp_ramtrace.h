/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#ifndef _XRISP_RAMTRACE_H_
#define _XRISP_RAMTRACE_H_

#include "xrisp_log.h"

#define ramtrace_err(fmt, ...) \
	XRISP_PR_ERROR("[XRISP_DRV][edr][ramtrace] %s: "fmt, __func__, ##__VA_ARGS__)

#define ramtrace_warn(fmt, ...) \
	XRISP_PR_WARN("[XRISP_DRV][edr][ramtrace] %s: "fmt, __func__, ##__VA_ARGS__)

#define ramtrace_info(fmt, ...) \
	XRISP_PR_INFO("[XRISP_DRV][edr][ramtrace] %s: "fmt, __func__, ##__VA_ARGS__)

#define ramtrace_debug(fmt, ...) \
	XRISP_PR_DEBUG("[XRISP_DRV][edr][ramtrace] %s: "fmt, __func__, ##__VA_ARGS__)

extern int edr_ipc_send(void *data, unsigned int len);
#endif
