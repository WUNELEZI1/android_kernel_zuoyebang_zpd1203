/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DMABUF Xring cga heap
 *
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 *
 * Copyright (C) 2011 Google, Inc.
 */

#ifndef _XRING_MEM_ADAPTER_INTERNAL_H_
#define _XRING_MEM_ADAPTER_INTERNAL_H_

#include "abord_mm_opt/xr_mm_abord.h"
#include "kshrink_slabd/xr_kshrink_slabd.h"

#define xrmem_err(fmt, ...) \
	pr_err("[MEM_ADAPTER] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrmem_warn(fmt, ...) \
	pr_warn("[MEM_ADAPTER] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrmem_info(fmt, ...) \
	pr_info("[MEM_ADAPTER] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define xrmem_debug(fmt, ...) \
	pr_debug("[MEM_ADAPTER] %s:%d: "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define XR_MEM_INIT_IDLE		0
#define XR_MEM_INIT_DONE		1

int cma_track_init(void);
int pin_track_init(void);
void cma_track_exit(void);
void pin_track_exit(void);

#endif
