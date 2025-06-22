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

#include "dpu_log.h"
#include "dpu_hw_common.h"
#include "dpu_reg_ops.h"

static inline void dpu_reg_hw_info_dump(const char *prefix,
		struct dpu_hw_blk *hw, u32 reg_offset, u32 val)
{
	DPU_REG_DEBUG("[%s] addr: 0x%08x, val: 0x%08x\n", prefix,
			hw->base_addr + hw->blk_offset + reg_offset, val);
}

void cpu_write(struct dpu_hw_blk *hw, u32 reg_offset, u32 val)
{
	void *addr = hw->iomem_base + hw->blk_offset + reg_offset;

	dpu_reg_hw_info_dump("W", hw, reg_offset, val);

	writel(val, addr);
}


void cpu_bitmask_write(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask)
{
	void *addr = hw->iomem_base + hw->blk_offset + reg_offset;
	u32 reg_val;

	reg_val = readl(addr);

	reg_val = MERGE_MASK_BITS(reg_val, val, bit_start, bit_mask);

	dpu_reg_hw_info_dump("W", hw, reg_offset, reg_val);

	writel(reg_val, addr);
}

void dpu_bitmask_write_no_log(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 val, u32 bit_start, u32 bit_mask)
{
	void *addr = hw->iomem_base + hw->blk_offset + reg_offset;
	u32 reg_val;

	reg_val = readl(addr);

	reg_val = MERGE_MASK_BITS(reg_val, val, bit_start, bit_mask);

	writel(reg_val, addr);
}

u32 dpu_reg_read(struct dpu_hw_blk *hw, u32 reg_offset)
{
	void *addr = hw->iomem_base + hw->blk_offset + reg_offset;
	u32 val;

	val = readl(addr);
	dpu_reg_hw_info_dump("R", hw, reg_offset, val);

	return val;
}

u32 dpu_bit_read(struct dpu_hw_blk *hw, u32 reg_offset,
		u32 bit_start, u32 bit_mask)
{
	void *addr = hw->iomem_base + hw->blk_offset + reg_offset;
	u32 val = 0;

	val = readl(addr);
	val = (val & bit_mask) >> bit_start;

	return val;
}

void dpu_sec_rdma(u8 id, u8 status)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_DPU_RCH_MID_ENABLE, id, status, 0, 0, 0, 0, 0, &res);
}

void dpu_sec_wb(u8 id, u8 status)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_DPU_WB_MID_ENABLE, id, status, 0, 0, 0, 0, 0, &res);
}
