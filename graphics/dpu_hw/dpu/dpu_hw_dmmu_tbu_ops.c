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

#include "dpu_hw_mmu_tbu_x_reg.h"
#include "dpu_hw_dmmu_tbu_ops.h"

#define TBU_QOS                 4

#define TBU_OFFSET_OF_DSCW      0x100
#define TBU_OFFSET_OF_DSCR      0x80
#define TBU_OFFSET_OF_WB        0x200
#define TBU_OFFSET_OF_LEFT_RCH  0x100
#define TBU_OFFSET_OF_RIGHT_RCH 0x180

static void dpu_mmu_tbu_fb_offset_config(struct dpu_hw_blk *hw,
		struct framebuffer_info *fb_info, u32 tbu_offset)
{
	IOMMU_DEBUG("tbu0 [tlb_pa, iova]: 0x%llx 0x%llx\n", fb_info->tlb_pa,
			fb_info->iova >> PAGE_SHIFT);
	DPU_REG_WRITE(hw, TBU_BASE_ADDR0_LOW_OFFSET + tbu_offset,
			fb_info->tlb_pa, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, TBU_VA0_OFFSET + tbu_offset,
			(fb_info->iova >> PAGE_SHIFT), CMDLIST_WRITE);
	DPU_REG_WRITE(hw, TBU_SIZE0_OFFSET + tbu_offset, 0xFFFF, CMDLIST_WRITE);

	if (fb_info->is_afbc) {
		IOMMU_DEBUG("tbu1 [tlb_pa, iova]: is afbc, config tbu1 same to tbu0.\n");
		DPU_REG_WRITE(hw, TBU_BASE_ADDR1_LOW_OFFSET + tbu_offset,
				fb_info->tlb_pa, CMDLIST_WRITE);
		DPU_REG_WRITE(hw, TBU_VA1_OFFSET + tbu_offset,
				(fb_info->iova >> PAGE_SHIFT), CMDLIST_WRITE);
		DPU_REG_WRITE(hw, TBU_SIZE1_OFFSET + tbu_offset, 0xFFFF, CMDLIST_WRITE);
		return ;
	}

	if (fb_info->plane_num >= 2) {
		IOMMU_DEBUG("tbu1 [tlb_pa, iova]: 0x%llx 0x%llx\n",
				fb_info->tlb_pa + fb_info->tlb_offsets[1],
				(fb_info->iova + fb_info->iova_offsets[1]) >> PAGE_SHIFT);
		DPU_REG_WRITE(hw, TBU_BASE_ADDR1_LOW_OFFSET + tbu_offset,
				fb_info->tlb_pa + fb_info->tlb_offsets[1], CMDLIST_WRITE);
		DPU_REG_WRITE(hw, TBU_VA1_OFFSET + tbu_offset,
				((fb_info->iova + fb_info->iova_offsets[1]) >> PAGE_SHIFT),
				CMDLIST_WRITE);
		DPU_REG_WRITE(hw, TBU_SIZE1_OFFSET + tbu_offset, 0xFFFF, CMDLIST_WRITE);
	}

	if (fb_info->plane_num >= 3) {
		IOMMU_DEBUG("tbu2 [tlb_pa, iova]: 0x%llx 0x%llx\n",
				fb_info->tlb_pa + fb_info->tlb_offsets[2],
				(fb_info->iova + fb_info->iova_offsets[2]) >> PAGE_SHIFT);
		DPU_REG_WRITE(hw, TBU_BASE_ADDR2_LOW_OFFSET + tbu_offset,
				fb_info->tlb_pa + fb_info->tlb_offsets[2], CMDLIST_WRITE);
		DPU_REG_WRITE(hw, TBU_VA2_OFFSET + tbu_offset,
				((fb_info->iova + fb_info->iova_offsets[2]) >> PAGE_SHIFT),
				CMDLIST_WRITE);
		DPU_REG_WRITE(hw, TBU_SIZE2_OFFSET + tbu_offset, 0xFFFF, CMDLIST_WRITE);
	}
}

void dpu_mmu_tbu_config(struct dpu_hw_blk *hw,
		struct framebuffer_info *fb_info, enum module_type type)
{
	u32 tbu_offset;
	u32 val;

	if (!hw || !fb_info) {
		DPU_ERROR("invalid parameters, %pK, %pK\n", hw, fb_info);
		return;
	}

	if (fb_info->iova == 0)
		return;

	switch (type) {
	case MODULE_WDMA:
		tbu_offset = TBU_OFFSET_OF_WB;
		break;
	case MODULE_LEFT_RDMA:
		tbu_offset = TBU_OFFSET_OF_LEFT_RCH;
		break;
	case MODULE_RIGHT_RDMA:
		tbu_offset = TBU_OFFSET_OF_RIGHT_RCH;
		break;
	case MODULE_DSC_WDMA:
		tbu_offset = TBU_OFFSET_OF_DSCW;
		break;
	case MODULE_DSC_RDMA:
		tbu_offset = TBU_OFFSET_OF_DSCR;
		break;
	default:
		DPU_ERROR("invalid channel type: %d", type);
		return;
	}

	DPU_REG_WRITE(hw, MMU_TBU_X_REGBANK_RESET_OFFSET + tbu_offset,
			REG_BANK_RESET_VAL, CMDLIST_WRITE);

	IOMMU_DEBUG("afbc=%d plane_num=%d\n", fb_info->is_afbc, fb_info->plane_num);

	dpu_mmu_tbu_fb_offset_config(hw, fb_info, tbu_offset);

	/* Write tbu enable, plane number, fbc mode */
	val = 0x0;
	val = MERGE_BITS(val, 1, TBU_EN_SHIFT, TBU_EN_LENGTH);
	val = MERGE_BITS(val, fb_info->is_afbc, TBU_FBC_MODE_SHIFT, TBU_FBC_MODE_LENGTH);
	val = MERGE_BITS(val, fb_info->plane_num - 1, TBU_PLANE_NUM_SHIFT, TBU_PLANE_NUM_LENGTH);
	val = MERGE_BITS(val, fb_info->qos, TBU_QOS_SHIFT, TBU_QOS_LENGTH);
	DPU_REG_WRITE(hw, TBU_EN_OFFSET + tbu_offset, val, CMDLIST_WRITE);
}

