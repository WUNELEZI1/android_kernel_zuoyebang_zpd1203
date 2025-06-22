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
#ifndef __DT_BINDINGS_XR_THERMAL_H__
#define __DT_BINDINGS_XR_THERMAL_H__

#define GPU_CORE_MASK_INDEX_0	16
#define GPU_CORE_MASK_INDEX_1	12
#define GPU_CORE_MASK_INDEX_2	8
#define GPU_CORE_MASK_INDEX_3	4
#define GPU_CORE_MASK_INDEX_4	2

#define GPU_CORE_HOT_LEVEL_0    0
#define GPU_CORE_HOT_LEVEL_1    1
#define GPU_CORE_HOT_LEVEL_2    2
#define GPU_CORE_HOT_LEVEL_3    3
#define GPU_CORE_HOT_LEVEL_4    4
#define GPU_CORE_HOT_LEVEL_MIN  GPU_CORE_HOT_LEVEL_0
#define GPU_CORE_HOT_LEVEL_MAX  GPU_CORE_HOT_LEVEL_4

#endif
