// SPDX-License-Identifier: GPL-2.0-only
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

#ifndef _DPU_REG_OPS_H_
#define _DPU_REG_OPS_H_

#include "osal.h"

#define CMDLIST_WRITE false
#define DIRECT_WRITE true

/**
 * dpu_hw_blk - the hardware block base class
 * @iomem_base: the mapped dpu base address
 * @base_addr: the base address of dpu
 * @blk_id: the block id of the same type block
 * @blk_offset: the block relative offset
 * @blk_len: the block length
 * @features: features of this block
 * @priv: the private filed for storing private data
 */
struct dpu_hw_blk {
	uint32_t iomem_base;
	uint32_t base_addr;

	int32_t blk_id;
	uint32_t blk_offset;
	uint32_t blk_len;

	unsigned long features;
	void *priv;
};

/* NOTE: please don't use below APIs directly, use the macro version instead */
void dpu_reg_write(struct dpu_hw_blk *hw, uint32_t reg_offset, uint32_t val);
void dpu_bitmask_write(struct dpu_hw_blk *hw, uint32_t reg_offset, uint32_t val,
		uint32_t bit_start, uint32_t bit_mask);
uint32_t dpu_reg_read(struct dpu_hw_blk *hw, uint32_t reg_offset);
uint32_t dpu_bit_read(struct dpu_hw_blk *hw, uint32_t reg_offset,
		uint32_t bit_start, uint32_t bit_mask);

void dpu_sec_rdma(uint8_t id, uint8_t status);
void dpu_sec_wb(uint8_t id, uint8_t status);
/**
 * the macro definition of hardware register operations
 * offset is the relative offset for every module
 */
#define DPU_REG_WRITE(hw, offset, val, force_cpu_write) dpu_reg_write(hw, offset, val)
#define DPU_REG_WRITE_NO_LOG(hw, offset, val) dpu_reg_write(hw, offset, val)
#define DPU_BIT_WRITE(hw, offset, val, bit_start, bit_num, force_cpu_write) \
	dpu_bitmask_write(hw, offset, val, bit_start, BITS_MASK(bit_start, bit_num))
#define DPU_BIT_WRITE_NO_LOG(hw, offset, val, bit_start, bit_num) \
	dpu_bitmask_write(hw, offset, val, bit_start, BITS_MASK(bit_start, bit_num))
#define DPU_BITMASK_WRITE(hw, offset, val, bit_start, bit_mask, force_cpu_write) \
	dpu_bitmask_write(hw, offset, val, bit_start, bit_mask)
#define DPU_REG_READ(hw, offset) dpu_reg_read(hw, offset)
#define DPU_REG_READ_NO_LOG(hw, offset) \
	*(volatile uint32_t *)(uintptr_t)((hw)->iomem_base + (hw)->blk_offset + (offset))
#define DPU_BIT_READ(hw, offset, bit_start, bit_num) \
	dpu_bit_read(hw, offset, bit_start, BITS_MASK(bit_start, bit_num))

#define DPU_REPEAT_WRITE(hw, offset, val, force_cpu_write) \
	dpu_reg_write(hw, offset, val)
#define DPU_REG_WRITE_WITH_NODE_ID(hw, node_id, offset, val) \
	dpu_reg_write(hw, offset, val)

/**
 * used to resolve compile error in UEFI
 * cause low_level code is shared by Android kernel
 * and UEFI
 */
#define DPU_STRIPE_REG_WRITE(hw, node_id, offset, val) \
	dpu_reg_write(hw, offset, 0)
#define DPU_STRIPE_BIT_WRITE(hw, node_id, offset, val, bit_start, bit_num) \
	dpu_bitmask_write(hw, offset, 0, 0, 0)

#define wait_reg_timeout(addr, val, cond, delay_time, timeout) \
({ \
	(val) = dpu_read_reg((addr)); \
	uint32_t _times = 0; \
	while (!(cond)) { \
		dpu_udelay((delay_time)); \
		_times += (delay_time); \
		if (_times >= (timeout)) { \
			break; \
		} \
		(val) = dpu_read_reg((addr)); \
	} \
	(cond) ? _times : -1;  \
})

/* the macro definition of read_poll_timeout functions */
#define DPU_READ_POLL_TIMEOUT(hw, offset, val, cond, delay_us, timeout_us) \
	wait_reg_timeout((hw)->iomem_base + (hw)->blk_offset + (offset), \
		(val), (cond), (delay_us), (timeout_us))

#define DPU_READ_POLL_TIMEOUT_ATOMIC(hw, offset, val, cond, delay_us, timeout_us) \
	wait_reg_timeout((hw)->iomem_base + (hw)->blk_offset + (offset), \
		(val), (cond), (delay_us), (timeout_us))

#define DPU_USLEEP(us) dpu_udelay((us))
#define DPU_MSLEEP(ms) dpu_udelay(1000 * (ms))
#define DPU_UDELAY(us) dpu_udelay(us)
#define DPU_MDELAY(ms) dpu_udelay(1000 * (ms))

#define DPU_DFX_INFO(blk_name, id, str, fmt, ...) \
do { \
	sprintf(str, fmt, ##__VA_ARGS__); \
	DPU_INFO("[DPU_DFX][%s%d] %s", blk_name, id, str); \
} while (0)

#define DPU_DFX_ERROR(blk_name, id, str, fmt, ...) \
do { \
	sprintf(str, fmt, ##__VA_ARGS__); \
	DPU_ERROR("[DPU_DFX][%s%d] %s", blk_name, id, str); \
} while (0)

#define DPU_DUMP_REG(blk_name, id, offset) \
	DPU_INFO("[DPU_DFX][%s%d] 0x%x: 0x%x\n", blk_name, id, offset, \
		DPU_REG_READ(hw, offset))

#define DPU_SEC_RDMA(id, status) \
	dpu_sec_rdma((id), (status))

#define DPU_SEC_WB(id, status) \
	dpu_sec_wb((id), (status))

void dpu_hw_blk_reg_dump(struct dpu_hw_blk *hw);
#endif
