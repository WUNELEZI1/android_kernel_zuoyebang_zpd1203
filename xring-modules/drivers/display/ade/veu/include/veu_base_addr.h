/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#ifndef _VEU_BASE_ADDR_H_
#define _VEU_BASE_ADDR_H_

#define VEU_TOP    0x0
#define VEU_TOP_MEM_SD_EXIT_NUM 0x18
#define VEU_INT    0x80

// veu ctrl
#define VEU_SCENE_CTL0     0x100
#define VEU_CTL_TOP        0x1c0
#define VEU_WB0_SEL_ID     0x38

//veu cmdlist
#define VEU_CMDLIST        0x300

// veu rdma mmu
#define RDMA_TOP     0x500
#define RDMA_PATH    0x580
#define MMU_TOP      0x680
#define RDMA_TBU     0x700
#define WDMA_TBU     0x780

// veu pq
#define PIPE_TOP        0x800

// veu wb
#define WB_CORE       0xE00

// veu scl2d
#define VEU_2DSCL_BASE      0xF00

#endif // _VEU_BASE_ADDR_H_
