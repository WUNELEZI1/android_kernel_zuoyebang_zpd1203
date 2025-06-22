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

#ifndef DPU_REG_OPS_H
#define DPU_REG_OPS_H

#include <linux/iopoll.h>
#include <linux/delay.h>
#include <asm/io.h>

/**
 * dpu_hw_blk - the hardware block base class
 * @iomem_base: the mapped dpu base address
 * @base_addr: the base address of dpu
 * @blk_id: the block id of the same type block
 * @blk_offset: the block relative offset
 * @blk_len: the block length
 * @features: features of this block
 * @priv: the private filed for storing private data
 * @cmd_node: the node_id pointer for crtc or plane
 */
struct dpu_hw_blk {
	void __iomem *iomem_base;
	u32 base_addr;

	int blk_id;
	u32 blk_offset;
	u32 blk_len;

	unsigned long features;
	void *priv;
	s64 *cmd_node;
};

/* NOTE: please don't use below APIs directly, use the macro version instead */
void cpu_write(struct dpu_hw_blk *hw, u32 reg_offset, u32 val);
void cpu_bitmask_write(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask);
void dpu_bitmask_write_no_log(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask);
u32 dpu_reg_read(struct dpu_hw_blk *hw, u32 reg_offset);
u32 dpu_bit_read(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 bit_start, u32 bit_mask);

void cmdlist_write(struct dpu_hw_blk *hw, u32 reg_offset, u32 val);
void cmdlist_bitmask_write(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask);
void cmdlist_write_with_node_id(struct dpu_hw_blk *hw, s64 node_id, u32 reg_offset, u32 val);
void stripe_cmdlist_bitmask_write(struct dpu_hw_blk *hw, s64 node_id, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask);
void cmdlist_repeat_write(struct dpu_hw_blk *hw, u32 reg_offset, u32 val);
void cmdlist_repeat_write_with_node_id(struct dpu_hw_blk *hw, s64 node_id, u32 reg_offset, u32 val);

void dpu_sec_rdma(u8 id, u8 status);
void dpu_sec_wb(u8 id, u8 status);
/**
 * the macro definition of hardware register operations
 * offset is the relative offset for every module
 */
#define CMDLIST_WRITE false
#define DIRECT_WRITE true

#define DPU_REG_WRITE(hw, offset, val, force_cpu_write) \
do { \
	if (force_cpu_write || !(hw)->cmd_node) \
		cpu_write((hw), (offset), (val)); \
	else \
		cmdlist_write((hw), (offset), (val)); \
} while (0)

#define DPU_STRIPE_REG_WRITE(hw, node_id, offset, val) \
		cmdlist_write_with_node_id((hw), (node_id), (offset), (val))

#define DPU_REG_WRITE_WITH_NODE_ID(hw, node_id, offset, val) \
		cmdlist_repeat_write_with_node_id((hw), (node_id), (offset), (val))

#define DPU_BITMASK_WRITE(hw, offset, val, bit_start, bit_mask, force_cpu_write) \
do { \
	if (force_cpu_write || !(hw)->cmd_node) \
		cpu_bitmask_write((hw), (offset), (val), (bit_start), (bit_mask)); \
	else \
		cmdlist_bitmask_write((hw), (offset), (val), (bit_start), (bit_mask)); \
} while (0)

#define DPU_BIT_WRITE(hw, offset, val, bit_start, bit_num, force_cpu_write) \
		DPU_BITMASK_WRITE(hw, offset, val, bit_start, \
				BITS_MASK(bit_start, bit_num), force_cpu_write)

#define DPU_REPEAT_WRITE(hw, offset, val, force_cpu_write) \
do { \
	if (force_cpu_write) \
		cpu_write((hw), (offset), (val)); \
	else \
		cmdlist_repeat_write((hw), (offset), (val)); \
} while (0)

#define DPU_REG_WRITE_NO_LOG(hw, offset, val) \
	writel((val), (hw)->iomem_base + (hw)->blk_offset + (offset))

#define DPU_BIT_WRITE_NO_LOG(hw, offset, val, bit_start, bit_num) \
	dpu_bitmask_write_no_log((hw), (offset), (val), (bit_start), \
		BITS_MASK((bit_start), (bit_num)))

#define DPU_REG_READ(hw, offset) \
	dpu_reg_read((hw), (offset))

#define DPU_REG_READ_NO_LOG(hw, offset) \
	readl((void *)(hw->iomem_base + hw->blk_offset + (offset)))

#define DPU_BIT_READ(hw, offset, bit_start, bit_num) \
	dpu_bit_read((hw), (offset), (bit_start), BITS_MASK((bit_start), (bit_num)))

/* the macro definition of read_poll_timeout functions */
#define DPU_READ_POLL_TIMEOUT(hw, offset, val, cond, delay_us, timeout_us) \
	readl_poll_timeout((hw)->iomem_base + (hw)->blk_offset + (offset), \
		(val), (cond), (delay_us), (timeout_us))

#define DPU_READ_POLL_TIMEOUT_ATOMIC(hw, offset, val, cond, \
		delay_us, timeout_us) \
	readl_poll_timeout_atomic( \
		(hw)->iomem_base + (hw)->blk_offset + (offset), \
		(val), (cond), (delay_us), (timeout_us))

#define DPU_USLEEP(us) usleep_range((us), (us) + (us))
#define DPU_MSLEEP(ms) usleep_range(1000 * (ms), 1000 * (ms) + 1000 * (ms))
#define DPU_UDELAY(us) udelay(us)
#define DPU_MDELAY(ms) mdelay(ms)

#define DPU_DFX_INFO(blk_name, id, str, fmt, ...) \
do { \
	sprintf(str, fmt, ##__VA_ARGS__); \
	DPU_LOGI_IF(DPU_DFX, "[%s%d] %s", blk_name, id, str); \
} while (0)

#define DPU_DFX_ERROR(blk_name, id, str, fmt, ...) \
do { \
	sprintf(str, fmt, ##__VA_ARGS__); \
	DPU_ERROR("[%s%d] %s", blk_name, id, str); \
} while (0)

#define DPU_DUMP_REG(blk_name, id, offset) \
	DPU_LOGI_IF(DPU_DFX, "[%s%d] 0x%x: 0x%x\n", blk_name, id, offset, \
		DPU_REG_READ(hw, offset))

#define DPU_SEC_RDMA(id, status) \
	dpu_sec_rdma((id), (status))

#define DPU_SEC_WB(id, status) \
	dpu_sec_wb((id), (status))

#endif
