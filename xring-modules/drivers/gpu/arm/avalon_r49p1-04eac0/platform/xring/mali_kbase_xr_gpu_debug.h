// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#ifndef _KBASE_XR_GPU_DEBUG_H_
#define _KBASE_XR_GPU_DEBUG_H_
#include <mali_kbase.h>
#include <csf/mali_kbase_csf_util.h>
#include <csf/mali_kbase_csf_csg.h>
#include <csf/mali_kbase_csf_sync.h>
#include <csf/mali_kbase_csf_cpu_queue.h>
#include <csf/mali_kbase_csf_firmware_log.h>

#define CSHW_IT_COMP_REG(r) (CSHW_BASE + 0x1000 + r)
#define CSHW_IT_FRAG_REG(r) (CSHW_BASE + 0x2000 + r)
#define CSHW_IT_TILER_REG(r) (CSHW_BASE + 0x3000 + r)

#define CSHW_BASE 0x0030000
#define CSHW_CSHWIF_0 0x4000 /* () CSHWIF 0 registers */
#define CSHWIF(n) (CSHW_BASE + CSHW_CSHWIF_0 + (n)*256)
#define CSHWIF_REG(n, r) (CSHWIF(n) + r)
#define CSHW_FEATURES 0x0004

#define CSHW_FEATURES_NUM_CSHWIF_SHIFT GPU_U(0)
#define CSHW_FEATURES_NUM_CSHWIF_MASK (GPU_U(0x3F) << CSHW_FEATURES_NUM_CSHWIF_SHIFT)
#define CSHW_FEATURES_NUM_CSHWIF_GET(reg_val) \
	(((reg_val)&CSHW_FEATURES_NUM_CSHWIF_MASK) >> CSHW_FEATURES_NUM_CSHWIF_SHIFT)

#define CSHWIF_CONFIG_JASID_SHIFT GPU_U(0)
#define CSHWIF_CONFIG_JASID_MASK (GPU_U(0xF) << CSHWIF_CONFIG_JASID_SHIFT)
#define CSHWIF_CONFIG_JASID_GET(reg_val) \
	(((reg_val)&CSHWIF_CONFIG_JASID_MASK) >> CSHWIF_CONFIG_JASID_SHIFT)

#define CSHWIF_CB_STACK_CURRENT_SHIFT GPU_U(0)
#define CSHWIF_CB_STACK_CURRENT_MASK (GPU_U(0xFF) << CSHWIF_CB_STACK_CURRENT_SHIFT)
#define CSHWIF_CB_STACK_CURRENT_GET(reg_val) \
	(((reg_val)&CSHWIF_CB_STACK_CURRENT_MASK) >> CSHWIF_CB_STACK_CURRENT_SHIFT)

void kbase_csf_dump_kctx_debug_info(struct kbase_context *kctx);

#endif
