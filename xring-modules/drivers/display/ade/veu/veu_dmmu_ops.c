// SPDX-License-Identifier: GPL-2.0-only
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

#include "veu_utils.h"
#include "veu_dmmu_ops.h"
#include "veu_defs.h"
#include "veu_enum.h"
#include "veu_hw_mmu_reg.h"
#include "veu_base_addr.h"

#define TBU_QOS            0xd
#define TBU_OFFSET_OF_WB        0x200
#define TBU_OFFSET_OF_RCH       0x100
/* one tlb entry used 4 bytes memory */
#define ENTRY_UNIT_USED_MEM     4

static void veu_mmu_tbu_offset_config(struct veu_data *veu_dev,
		struct buffer_info *info, u32 tbu_offset)
{
	VEU_DBG("tbu0 [tlb_pa, iova]: 0x%llx 0x%llx\n", info->tlb_pa,
			info->iova >> PAGE_SHIFT);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_BASE_ADDR0_LOW_OFFSET + tbu_offset,
			info->tlb_pa);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_VA0_OFFSET + tbu_offset,
			(info->iova >> PAGE_SHIFT));
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_SIZE0_OFFSET + tbu_offset, 0xFFFF);

	if (info->is_afbc) {
		VEU_DBG("tbu1 [tlb_pa, iova]: is afbc, config tbu1 same to tbu0.\n");
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_BASE_ADDR1_LOW_OFFSET + tbu_offset,
				info->tlb_pa);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_VA1_OFFSET + tbu_offset,
				(info->iova >> PAGE_SHIFT));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_SIZE1_OFFSET + tbu_offset, 0xFFFF);
		return;
	}

	if (info->plane_num >= 2) {
		VEU_DBG("tbu1 [tlb_pa, iova]: 0x%llx 0x%llx\n",
				info->tlb_pa + info->tlb_offsets[1],
				(info->iova + info->iova_offsets[1]) >> PAGE_SHIFT);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_BASE_ADDR1_LOW_OFFSET + tbu_offset,
				info->tlb_pa + info->tlb_offsets[1]);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_VA1_OFFSET + tbu_offset,
				((info->iova + info->iova_offsets[1]) >> PAGE_SHIFT));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_SIZE1_OFFSET + tbu_offset, 0xFFFF);
	}

	if (info->plane_num >= 3) {
		VEU_DBG("tbu2 [tlb_pa, iova]: 0x%llx 0x%llx\n",
				info->tlb_pa + info->tlb_offsets[2],
				(info->iova + info->iova_offsets[2]) >> PAGE_SHIFT);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_BASE_ADDR2_LOW_OFFSET + tbu_offset,
				info->tlb_pa + info->tlb_offsets[2]);
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_VA2_OFFSET + tbu_offset,
				((info->iova + info->iova_offsets[2]) >> PAGE_SHIFT));
		veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX, TBU_SIZE2_OFFSET + tbu_offset, 0xFFFF);
	}
}

void veu_mmu_tbu_config(struct veu_data *veu_dev,
		struct buffer_info *info, enum VEU_MODULE_TYPE type)
{
	u32 tbu_ctrl_val;
	u32 tbu_offset;

	if (!veu_dev || !info) {
		VEU_ERR("invalid parameters, %pK, %pK", veu_dev, info);
		return;
	}

	if (info->iova == 0)
		return;

	switch (type) {
	case MODULE_WDMA:
		tbu_offset = WDMA_TBU;
		break;
	case MODULE_RDMA:
		tbu_offset = RDMA_TBU;
		break;
	default:
		VEU_ERR("invalid channel type: %d", type);
		return;
	}

	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			tbu_offset + MMU_TBU_X_REGBANK_RESET_OFFSET, 1);

	VEU_DBG("afbc=%d plane_num=%d\n", info->is_afbc, info->plane_num);
	veu_mmu_tbu_offset_config(veu_dev, info, tbu_offset);

	/* Write tbu enable, plane number, fbc mode */
	tbu_ctrl_val = veu_reg_read(veu_dev, tbu_offset + TBU_EN_OFFSET);
	tbu_ctrl_val = MERGE_BITS(tbu_ctrl_val, 1, TBU_EN_SHIFT, TBU_EN_LENGTH);
	tbu_ctrl_val = MERGE_BITS(tbu_ctrl_val, info->is_afbc,
			TBU_FBC_MODE_SHIFT, TBU_FBC_MODE_LENGTH);
	tbu_ctrl_val = MERGE_BITS(tbu_ctrl_val, info->plane_num - 1, TBU_PLANE_NUM_SHIFT,
			TBU_PLANE_NUM_LENGTH);
	tbu_ctrl_val = MERGE_BITS(tbu_ctrl_val, TBU_QOS, TBU_QOS_SHIFT,
			TBU_QOS_LENGTH);
	veu_dev->set_reg(veu_dev, VEU_BASE_NODE_INDEX,
			tbu_offset + TBU_EN_OFFSET, tbu_ctrl_val);
}
