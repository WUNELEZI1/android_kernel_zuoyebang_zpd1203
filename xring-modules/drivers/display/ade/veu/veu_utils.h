/* SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _VEU_UTILS_H
#define _VEU_UTILS_H

#include <linux/iopoll.h>
#include <linux/ktime.h>
#include "veu_drv.h"
#include "veu_enum.h"

/* align */
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, al)     ((val) & ~((typeof(val))(al)-1))
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, al)       (((val) + ((al)-1)) & ~((typeof(val))(al)-1))
#endif

static inline int32_t ceil(uint32_t a, uint32_t b)
{
	if (b == 0)
		return -1;
	if (a % b != 0)
		return a / b + 1;
	else
		return a / b;
}

#define VEU_READ_POLL_TIMEOUT(veu_dev, offset, val, cond, delay_us, timeout_us) \
	readl_poll_timeout((veu_dev)->base + (offset), \
		(val), (cond), (delay_us), (timeout_us))

/* BIT related operations for operating register */
#define BITS_MASK(bit_start, bit_num) \
	((BIT((bit_num)) - 1) << (bit_start))

#define CLEAR_MASK_BITS(val, bit_mask) \
	((val) & (~(u32)(bit_mask)))

#define MERGE_MASK_BITS(reg_val, bits_val, bit_start, bit_mask) \
	(CLEAR_MASK_BITS(reg_val, bit_mask) | (bits_val) << (bit_start))

#define MERGE_BITS(reg_val, bits_val, bit_start, bit_num) \
	MERGE_MASK_BITS(reg_val, bits_val, bit_start, \
		BITS_MASK(bit_start, bit_num))

#define outp32(addr, val) writel(val, addr)
#define inp32(addr) readl(addr)

int veu_get_iova_by_sharefd(struct veu_data *veu_dev,
		int sharefd, uint64_t buf_size, enum  VEU_MODULE_TYPE module_type);

uint32_t veu_reg_read(struct veu_data *veu_dev, uint32_t offset);

void cpu_bit_write(struct veu_data *veu_dev, uint32_t offset, uint32_t val, uint32_t bit_start, uint32_t bit_width);

// cmdlist config
void cmdlist_write(struct veu_data *veu_dev, uint32_t node_index,
		uint32_t reg_offset, uint32_t val);

bool is_bpp_32(uint32_t format);
bool is_yuv422_rformat(uint32_t format);
bool is_yuv420_rformat(uint32_t format);
bool is_yuv420_wformat(uint32_t format);
bool is_yuv_planar(uint32_t format);
bool is_yuv_semi_planar(uint32_t format);
uint32_t get_bpp_by_format(uint32_t format);

struct veu_rect to_veu_rect(struct veu_rect_ltrb in_rect);
struct veu_rect_ltrb to_veu_rect_ltrb(struct veu_rect in_rect);
struct veu_rect_ltrb stripe_rect_to_veu_rect_ltrb(struct stripe_rect in_rect);

ktime_t veu_get_timestamp(void);
s64 veu_timestamp_diff(ktime_t begin);

void veu_rect_clear(struct veu_rect_ltrb *rect);

#endif /* _VEU_UTILS_H */
