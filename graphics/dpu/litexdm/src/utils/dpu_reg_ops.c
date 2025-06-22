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

#include "osal.h"
#include "dpu_log.h"
#include "dpu_hw_common.h"

void dpu_reg_write(struct dpu_hw_blk *hw, uint32_t reg_offset, uint32_t val)
{
	DPU_IOMEM addr = hw->iomem_base + hw->blk_offset + reg_offset;

	dpu_write_reg(addr, val);
}

void dpu_bitmask_write(struct dpu_hw_blk *hw, uint32_t reg_offset,
		uint32_t val, uint32_t bit_start, uint32_t bit_mask)
{
	DPU_IOMEM addr = hw->iomem_base + hw->blk_offset + reg_offset;
	uint32_t reg_val;

	reg_val = dpu_read_reg(addr);
	reg_val = MERGE_MASK_BITS(reg_val, val, bit_start, bit_mask);

	dpu_write_reg(addr, reg_val);
}

uint32_t dpu_reg_read(struct dpu_hw_blk *hw, uint32_t reg_offset)
{
	DPU_IOMEM addr = hw->iomem_base + hw->blk_offset + reg_offset;
	uint32_t val;

	val = dpu_read_reg(addr);
	dpu_pr_debug("[R] addr:0x%x value:0x%x\n", addr, val);
	return val;
}

uint32_t dpu_bit_read(struct dpu_hw_blk *hw, uint32_t reg_offset,
		uint32_t bit_start, uint32_t bit_mask)
{
	DPU_IOMEM addr = hw->iomem_base + hw->blk_offset + reg_offset;
	uint32_t val;

	val = dpu_read_reg(addr);
	val = (val & bit_mask) >> bit_start;
	return val;
}

void dpu_hw_blk_reg_dump(struct dpu_hw_blk *hw)
{
	uint32_t base;
	uint32_t len;
	uint32_t offset;

	if (!hw) {
		dpu_pr_err("hw is null pointer\n");
		return;
	}

	base = hw->iomem_base + hw->blk_offset;
	len = hw->blk_len;
	dpu_pr_info("dpu_hw_blk_reg_dump base = 0x%x, len = 0x%x\n", base, len);

	for (offset = 0; offset <= len; offset += sizeof(uint32_t))
		dpu_pr_info("addr: 0x%x value: 0x%x\n",
			base + offset, dpu_reg_read(hw, offset));
}

void dpu_sec_rdma(uint8_t id, uint8_t status)
{
	/* reserved uefi BL2 sec rdma interface */
}

void dpu_sec_wb(uint8_t id, uint8_t status)
{
	/* reserved uefi BL2 sec wb interface */
}