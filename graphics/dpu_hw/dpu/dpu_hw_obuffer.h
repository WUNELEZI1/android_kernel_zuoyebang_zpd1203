/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_HW_OBUFFER_H_
#define _DPU_HW_OBUFFER_H_

#include <linux/types.h>

#define OBUF_MEMORY0_1_SIZE 1194
#define OBUF_MEMORY2_3_SIZE 1638

/* obuffer_parms - obuffer paramters for configure */
struct obuffer_parms {
	/* obuffer low level */
	u16 core_dfc_l_thre_buf;
	/* obuffer high level */
	u16 core_dfc_h_thre_buf;
	/* urgent level enable : 0- off, 1-on */
	u8 core_urgent_vld_en_buf;
	/* urgent low level */
	u16 core_urgent_l_thre_buf;
	/* urgent high level */
	u16 core_urgent_h_thre_buf;
	/* obuffer dpu dvfs low level */
	u16 core_dpu_dvfs_l_thre_buf;
	/* obuffer dpu dvfs high level */
	u16 core_dpu_dvfs_h_thre_buf;
	u16 core_obuf_lvl_thre_buf0;
	u16 core_obuf_lvl_thre_buf1;
	u16 core_obuf_lvl_thre_buf2;
	u16 core_obuf_th_buf;
	/* underflow = h, if obuffer low level < this value */
	u16 obuf_near_empty;
	u8 split_en_buf;
	/* mem sd: 0- by hardware, 1- by software */
	u8 core_mem_lp_auto_en_buf;
	/* bit i means mem i is used or not used */
	u32 obufen;
	/* obuffer enable : 0- Disable, 1- Enable */
	u8 core_fm_timing_en;
};

#endif /* _DPU_HW_OBUFFER_H_ */
