// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */


#ifndef __ASIC_MEMORY_LAYOUT_H__
#define __ASIC_MEMORY_LAYOUT_H__

#define XR_OFFSET                       32
#define XR_APPEND(high, low)            ( ((unsigned long long)(high) << XR_OFFSET) | (low))

#define XR_ADSP_RUN_MEM_BASE_HIGH       0x0
#define XR_ADSP_RUN_MEM_BASE_LOW        0x47D80000

#define XR_ADSP_RUN_MEM_BASE            XR_APPEND(XR_ADSP_RUN_MEM_BASE_HIGH, XR_ADSP_RUN_MEM_BASE_LOW)

#define XR_ADSP_SHARE_MEM_BASE_LOW      0x49180000

#define XR_BL2_MEM_BASE_LOW             0x5f800000

#define XR_CMA_MEM_SIZE                 0x6C00000
#endif
