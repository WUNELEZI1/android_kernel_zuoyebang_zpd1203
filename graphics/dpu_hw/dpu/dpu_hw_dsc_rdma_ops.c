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

#include "dpu_hw_dsc_rdma_ops.h"
#include "dpu_hw_dsc_wdma_ops.h"
#include "dpu_hw_dsc_rdma_reg.h"
#include "dpu_hw_dsc_rdma_top_reg.h"

#define GET_DSC_RDMA_BASE(offset) (0x100 + (offset))
#define GET_DSC_RDMA_TOP_BASE(offset) (0 + offset)

void dsc_rdma_layer_config(struct dpu_hw_blk *hw, struct dsc_rd_layer_cfg *layer_cfg)
{
	u32 data_width = DSC_WB_HSIZE(layer_cfg->width, layer_cfg->format);

	DPU_BIT_WRITE(hw, GET_DSC_RDMA_BASE(RD_DATA_WIDTH_OFFSET), data_width,
		RD_DATA_WIDTH_SHIFT, RD_DATA_WIDTH_LENGTH, DIRECT_WRITE);
	DPU_BIT_WRITE(hw, GET_DSC_RDMA_BASE(RD_DATA_WIDTH_OFFSET), layer_cfg->height,
		RD_DATA_HEIGHT_SHIFT, RD_DATA_HEIGHT_LENGTH, DIRECT_WRITE);
	DPU_BIT_WRITE(hw, GET_DSC_RDMA_BASE(RD_DATA_STRIDE_OFFSET), data_width,
		RD_DATA_STRIDE_SHIFT, RD_DATA_STRIDE_LENGTH, DIRECT_WRITE);

	DPU_REG_WRITE(hw, GET_DSC_RDMA_BASE(RD_BASE_LOW_ADDR_OFFSET),
		GET_LOW32_ADDR(layer_cfg->rdma_base_addr), DIRECT_WRITE);
	DPU_BIT_WRITE(hw, GET_DSC_RDMA_BASE(RD_BASE_HEIGHT_ADDR_OFFSET),
		GET_HIGH32_ADDR(layer_cfg->rdma_base_addr),
		RD_BASE_HEIGHT_ADDR_SHIFT, RD_BASE_HEIGHT_ADDR_LENGTH, DIRECT_WRITE);
}

void dsc_rdma_axi_cfg(struct dpu_hw_blk *hw, struct dsc_rd_axi_cfg *axi_cfg)
{
	u32 val;

	DPU_BIT_WRITE(hw, GET_DSC_RDMA_BASE(OUTSTANDING_NUM_OFFSET), axi_cfg->burst_split_en,
		DSC_RDMA_BURST_SPLIT_EN_SHIFT, DSC_RDMA_BURST_SPLIT_EN_LENGTH, DIRECT_WRITE);

	val = 0x0;
	val = MERGE_BITS(val, axi_cfg->qos, AXI_RQOS_SHIFT, AXI_RQOS_LENGTH);
	DPU_REG_WRITE(hw, GET_DSC_RDMA_BASE(DSC_RDMA_AXI_RCACHE_OFFSET), val, DIRECT_WRITE);
}

void dsc_rdma_secure_mode(struct dpu_hw_blk *hw, u8 secure_en)
{
	DPU_BIT_WRITE(hw, RDMA_SECURITY_OFFSET, secure_en,
		RDMA_SECURITY_SHIFT, RDMA_SECURITY_LENGTH, DIRECT_WRITE);
}

void dump_dsc_rdma_top_status(struct dpu_hw_blk *hw)
{
	DPU_DUMP_REG("irq raw", hw->blk_id, DSC_RDMA_TOP_IRQ_RAW_OFFSET);
	DPU_DUMP_REG("irq status", hw->blk_id, DSC_RDMA_TOP_IRQ_STATUS_OFFSET);
}

/* enalbe clk_sysbus_data_media1_rs_core */
void ocm_data_pipeline_ctrl(bool enable)
{
	DPU_IOMEM peri_crg_base;

	REG_MAP(peri_crg_base, 0xec001000, 0x1000);
	if (enable)
		DPU_REG_WRITE_BARE(peri_crg_base + 0x60, 0x10);
	else
		DPU_REG_WRITE_BARE(peri_crg_base + 0x60, 0x0);
	REG_UNMAP(peri_crg_base);
}
