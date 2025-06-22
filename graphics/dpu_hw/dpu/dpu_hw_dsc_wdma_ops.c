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

#include "dpu_hw_dsc_wdma_ops.h"
#include "dpu_hw_dsc_wdma_reg.h"

void dsc_wdma_layer_config(struct dpu_hw_blk *hw, struct dsc_wr_layer_cfg *layer_cfg)
{
	u32 hsize = DSC_WB_HSIZE(layer_cfg->width, layer_cfg->format);

	DPU_BIT_WRITE(hw, DSC_WDMA_HSIZE_OFFSET, hsize,
		DSC_WDMA_HSIZE_SHIFT, DSC_WDMA_HSIZE_LENGTH, DIRECT_WRITE);
	DPU_BIT_WRITE(hw, DSC_WDMA_HSIZE_OFFSET, layer_cfg->height,
		DSC_WDMA_VSIZE_SHIFT, DSC_WDMA_VSIZE_LENGTH, DIRECT_WRITE);
	DPU_BIT_WRITE(hw, DSC_WDMA_STRIDE_OFFSET, hsize,
		DSC_WDMA_STRIDE_SHIFT, DSC_WDMA_STRIDE_LENGTH, DIRECT_WRITE);
	DPU_REG_WRITE(hw, DSC_WDMA_BASE_ADDR_LOW_OFFSET, GET_LOW32_ADDR(layer_cfg->wdma_base_addr), DIRECT_WRITE);
	DPU_BIT_WRITE(hw, DSC_WDMA_BASE_ADDR_HIGH_OFFSET, GET_HIGH32_ADDR(layer_cfg->wdma_base_addr),
		DSC_WDMA_BASE_ADDR_HIGH_SHIFT, DSC_WDMA_BASE_ADDR_HIGH_LENGTH, DIRECT_WRITE);
}

void dsc_wdma_axi_cfg(struct dpu_hw_blk *hw, struct dsc_wr_axi_cfg *axi_cfg)
{
	u32 val;

	DPU_BIT_WRITE(hw, DSC_WDMA_OUTSTANDING_NUM_OFFSET, axi_cfg->boundary_en,
		DSC_WDMA_BOUNDARY_EN_SHIFT, DSC_WDMA_BOUNDARY_EN_LENGTH, DIRECT_WRITE);
	DPU_BIT_WRITE(hw, DSC_WDMA_OUTSTANDING_NUM_OFFSET, axi_cfg->burst_boundary,
		DSC_WDMA_BURST_BOUNDARY_SHIFT, DSC_WDMA_BURST_BOUNDARY_LENGTH, DIRECT_WRITE);
	DPU_BIT_WRITE(hw, DSC_WDMA_OUTSTANDING_NUM_OFFSET, axi_cfg->osd,
		DSC_WDMA_OUTSTANDING_NUM_SHIFT, DSC_WDMA_OUTSTANDING_NUM_LENGTH, DIRECT_WRITE);

	val = 0x0;
	val = MERGE_BITS(val, axi_cfg->qos, DSC_WDMA_QOS_SHIFT, DSC_WDMA_QOS_LENGTH);
	DPU_REG_WRITE(hw, DSC_WDMA_QOS_OFFSET, val, DIRECT_WRITE);
}

void dsc_wdma_mem_lp_ctl(struct dpu_hw_blk *hw, u8 auto_en)
{
	DPU_BIT_WRITE(hw, DSC_WDMA_MEM_LP_AUTO_EN_OFFSET, auto_en,
		DSC_WDMA_MEM_LP_AUTO_EN_SHIFT, DSC_WDMA_MEM_LP_AUTO_EN_LENGTH, DIRECT_WRITE);
}

void dsc_wdma_secure_mode(struct dpu_hw_blk *hw, u8 secure_en)
{
	DPU_BIT_WRITE(hw, DSC_WDMA_NSAID_OFFSET, secure_en,
		DSCW_AXI_SECU_EN_SHIFT, DSCW_AXI_SECU_EN_LENGTH, DIRECT_WRITE);
}

