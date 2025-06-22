/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2023 XRing Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __XRING_HEAPS_DTS_H__
#define __XRING_HEAPS_DTS_H__

#define CMA_HEAP        0x1
#define CARVEOUT_HEAP   0x2
#define CPA_HEAP        0x3
#define CGA_HEAP        0x4
#define SGA_HEAP        0x5
#define SYSTEM_HEAP     0x6

#define HEAP_NORMAL     0x1
#define HEAP_SECURE     0x10
#define HEAP_PROTECT    0x100
#define HEAP_MIX        0x1000

#define HEAP_PROTECT_DRM        0x1
#define HEAP_SECURE_FACEID      0x2
#define HEAP_SYSTEM_DIO_ALLOC   0x3

#define NORMAL_CASE                     0x0
#define PMPU_PROTECTED_CASE             0x1
#define MREGION_NPU_DYM_CASE            0x5
#define MREGION_ISP_FACEID_CASE         0x6
#define MREGION_TUI_DISPLAY_CASE        0x7
#define MREGION_TUI_FONT_CASE           0x8

#define SUBREGION_SIZE      0x200000 /* subregion size in mregion */
#define GENPOOL_SHIFT       0xc /* each bitmap bit represents 2^12 in genpool */
#define APPEND_SIZE         0x1000000 /* append size each times in cga heap */
#define PER_ALLOC_SIZE      0x200000 /* per-alloc size in sga */
#define CPA_SIZE            0x66800000 /* cpa heap size: 1640M */

#endif
