// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#ifdef __KERNEL__
#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#else /* UEFI */
#include "flowctrl_cfg_define.h"
#endif

#include "dpu_hw_tpc_module_ops.h"
#include "dpu_hw_power_ops.h"

// MED1_DPU_TPC: GDMA0~5
#define TPC_MED1_DPU_GDMA0_NORMAL_CRG_CONFIG0_OFFSET 0x0012A8
#define TPC_MED1_DPU_GDMA1_NORMAL_CRG_CONFIG0_OFFSET 0x00B2A8
#define TPC_MED1_DPU_GDMA2_NORMAL_CRG_CONFIG0_OFFSET 0x00D2A8
#define TPC_MED1_DPU_GDMA3_NORMAL_CRG_CONFIG0_OFFSET 0x00E2A8
#define TPC_MED1_DPU_GDMA4_NORMAL_CRG_CONFIG0_OFFSET 0x0102A8
#define TPC_MED1_DPU_GDMA5_NORMAL_CRG_CONFIG0_OFFSET 0x0132A8

// MED1_DPU_TPC: VDMA0~3
#define TPC_MED1_DPU_VDMA0_NORMAL_CRG_CONFIG0_OFFSET 0x0022A8
#define TPC_MED1_DPU_VDMA1_NORMAL_CRG_CONFIG0_OFFSET 0x00C2A8
#define TPC_MED1_DPU_VDMA2_NORMAL_CRG_CONFIG0_OFFSET 0x0112A8
#define TPC_MED1_DPU_VDMA3_NORMAL_CRG_CONFIG0_OFFSET 0x0122A8

// MED1_DPU_TPC: WB0_1
#define TPC_MED1_DPU_WB_NORMAL_CRG_CONFIG0_OFFSET 0x05012C

// MED1_DPU_TPC: DSC_RD0~1
#define TPC_MED1_DPU_DSC_RD0_NORMAL_CRG_CONFIG0_OFFSET 0x051130
#define TPC_MED1_DPU_DSC_RD1_NORMAL_CRG_CONFIG0_OFFSET 0x054130

// MED1_DPU_TPC: DSC_WR0~1
#define TPC_MED1_DPU_DSC_WR0_NORMAL_CRG_CONFIG0_OFFSET 0x029420
#define TPC_MED1_DPU_DSC_WR1_NORMAL_CRG_CONFIG0_OFFSET 0x029620

// MED1_DPU_TPC: VEU_RD
#define TPC_MED1_DPU_VEU_RD_NORMAL_CRG_CONFIG0_OFFSET 0x200528

// MED1_DPU_TPC: VEU_WR
#define TPC_MED1_DPU_VEU_WR_NORMAL_CRG_CONFIG0_OFFSET 0x200E68

// MED1_DPU: VDMA0~3 GDMA0~5 WB0~1
#define TPC_MED1_DPU_GDMA0_CRG_CONFIG0_OFFSET 0x0012C8
#define TPC_MED1_DPU_VDMA0_CRG_CONFIG0_OFFSET 0x0022C8
#define TPC_MED1_DPU_GDMA1_CRG_CONFIG0_OFFSET 0x00B2C8
#define TPC_MED1_DPU_VDMA1_CRG_CONFIG0_OFFSET 0x00C2C8
#define TPC_MED1_DPU_GDMA2_CRG_CONFIG0_OFFSET 0x00D2C8
#define TPC_MED1_DPU_GDMA3_CRG_CONFIG0_OFFSET 0x00E2C8
#define TPC_MED1_DPU_GDMA4_CRG_CONFIG0_OFFSET 0x0102C8
#define TPC_MED1_DPU_VDMA2_CRG_CONFIG0_OFFSET 0x0112C8
#define TPC_MED1_DPU_VDMA3_CRG_CONFIG0_OFFSET 0x0122C8
#define TPC_MED1_DPU_GDMA5_CRG_CONFIG0_OFFSET 0x0132C8
#define TPC_MED1_DPU_WB0_CRG_CONFIG0_OFFSET 0x0050154
#define TPC_MED1_DPU_WB1_CRG_CONFIG0_OFFSET 0x0050174

// MED1_DPU: DSC_RD0~1 DSC_WR0~1
#define TPC_MED1_DPU_DSC_RD0_CRG_CONFIG0_OFFSET 0x051158
#define TPC_MED1_DPU_DSC_RD1_CRG_CONFIG0_OFFSET 0x054158
#define TPC_MED1_DPU_DSC_WR0_CRG_CONFIG0_OFFSET 0x029448
#define TPC_MED1_DPU_DSC_WR1_CRG_CONFIG0_OFFSET 0x029648

// MED1_DPU: VEU_RDMA/VEU_WDMA
#define TPC_MED1_DPU_VEU_RDMA_CRG_CONFIG0_OFFSET 0x200548
#define TPC_MED1_DPU_VEU_WDMA_CRG_CONFIG0_OFFSET 0x200E90

// MED1_DPU: DPU_AXI~5 TPC-2 grade
#define TPC_MED1_DPU_AXI0_CRG_CONFIG0_OFFSET 0x05805C
#define TPC_MED1_DPU_AXI1_CRG_CONFIG0_OFFSET 0x058074
#define TPC_MED1_DPU_AXI2_CRG_CONFIG0_OFFSET 0x05808C
#define TPC_MED1_DPU_AXI3_CRG_CONFIG0_OFFSET 0x0580A4
#define TPC_MED1_DPU_AXI4_CRG_CONFIG0_OFFSET 0x0580BC
#define TPC_MED1_DPU_AXI5_CRG_CONFIG0_OFFSET 0x0580D4

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

static DPU_IOMEM g_dpu_base;
static struct tpc_common_cfg_map *g_flow_tpc_common_cfg_table = NULL;
static struct tpc_cfg_map *g_flow_tpc_cfg_table = NULL;

static void dpu_media1_dpu_common_p1_tpc_cfg(void)
{
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_GDMA0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA1_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA1_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA2_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA3_NORMAL_CRG_CONFIG0_OFFSET
	};
	const u32 dpu_reg_ch_na89_arr[] = {
		TPC_MED1_DPU_WB_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_WR0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_WR1_NORMAL_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base + dpu_reg_ch_offset_arr[i],
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x10,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x14,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x18,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x1C,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}

	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_na89_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i],
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0x4,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0x8,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0xC,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0x10,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0x14,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0x18,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[6]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_na89_arr[i] + 0x1C,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[7]);
	}
}

static void dpu_media1_dpu_common_p2_tpc_cfg(void)
{
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_GDMA4_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA2_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA3_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA5_NORMAL_CRG_CONFIG0_OFFSET};
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x10,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x14,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x18,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x1C,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}
}

static void dpu_media1_dpu_common_p0_tpc_cfg(void)
{
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_DSC_RD0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_RD1_NORMAL_CRG_CONFIG0_OFFSET
	};

	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x10,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x14,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x20,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x24,
			g_flow_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}
}

static void dpu_media1_dpu_dsc_rd_tpc_cfg(void)
{
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_DSC_RD0_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_RD1_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	// cfg0 bit15 tpc_qos_remap_en set 1 now, so need cfg 2nd times
	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		// first cfg: cfg0 bit17 set 0
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[1]); // CFG1
		// cfg0 bit17 set 0, then CFG2-1st cfg
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			(g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[0] | BIT(17)));
		// CFG2-1st
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[4]);
		// finnal cfg3
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[5]);
	}
}

static void dpu_media1_dpu_axi_tpc_cfg(void)
{
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_AXI0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_AXI1_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_AXI2_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_AXI3_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_AXI4_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_AXI5_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			g_flow_tpc_cfg_table[M1_DPU_AXI_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_cfg_table[M1_DPU_AXI_ID].val[1]);
			// attention：diffent with other dpu mst,
			// CONFIG2(+0xC) is tpc qos0~7/qos8~15 remap qos val
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_AXI_ID].val[2]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_AXI_ID].val[3]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_AXI_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_AXI_ID].val[5]);
	}
}

static void dpu_p0_tpc_cfg(void)
{
	dpu_media1_dpu_common_p0_tpc_cfg();
	dpu_media1_dpu_dsc_rd_tpc_cfg();
	dpu_media1_dpu_axi_tpc_cfg();
}

static void dpu_p1_tpc_cfg(void)
{
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_GDMA0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VDMA0_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA1_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VDMA1_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA2_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_GDMA3_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_WB0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_WB1_CRG_CONFIG0_OFFSET
	};

	const u32 dpu_dsc_wr_ch_offset_arr[] = {
		TPC_MED1_DPU_DSC_WR0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_DSC_WR1_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	dpu_media1_dpu_common_p1_tpc_cfg();
	// DPU1 channel: G0, V0, G1, V1, G2, G3， WB0~1, DSC_WR0~1
	// cfg0 bit15 tpc_qos_remap_en set 1 now, so need cfg 2nd times
	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		// first cfg: cfg0 bit17 set 0
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[1]); // CFG1
		// cfg0 bit17 set 0, then CFG2-1st cfg
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			(g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[0] | BIT(17)));
		// CFG2-1st
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[4]);
		// finnal cfg3
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[5]);
	}

	for (i = 0; i < ARRAY_SIZE(dpu_dsc_wr_ch_offset_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_dsc_wr_ch_offset_arr[i],
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_dsc_wr_ch_offset_arr[i] + 0x4,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[1]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_dsc_wr_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[2]);
		// 2nd cfg
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_dsc_wr_ch_offset_arr[i],
			(g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[0] | BIT(17)));
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_dsc_wr_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_dsc_wr_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_DSC_ID].val[5]);
	}
}

static void dpu_p2_tpc_cfg(void)
{
	// DPU2 channel G4,V2,V3,G5,
	const u32 dpu_reg_ch_offset_arr[] = {
		TPC_MED1_DPU_GDMA4_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VDMA2_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA3_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_GDMA5_CRG_CONFIG0_OFFSET};
	u32 i = 0;

	dpu_media1_dpu_common_p2_tpc_cfg();

	for (i = 0; i < ARRAY_SIZE(dpu_reg_ch_offset_arr); i++) {
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[0]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x4,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[1]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i],
			(g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[0] | BIT(17)));
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0x8,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[4]);
		DPU_REG_WRITE_BARE(g_dpu_base +
			dpu_reg_ch_offset_arr[i] + 0xC,
			g_flow_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[5]);
	}
}

void dpu_hw_tpc_module_cfg(u32 part_id)
{
	if (!g_flow_tpc_common_cfg_table || !g_flow_tpc_cfg_table) {
		DPU_ERROR("invalid tpc init parameters\n");
		return;
	}
	switch (part_id) {
	case DPU_PARTITION_0:
		dpu_p0_tpc_cfg();
		break;
	case DPU_PARTITION_1:
		dpu_p1_tpc_cfg();
		break;
	case DPU_PARTITION_2:
		dpu_p2_tpc_cfg();
		break;
	default:
		DPU_ERROR("invalid partition id:%u\n", part_id);
		break;
	}
}

void dpu_hw_tpc_module_init(DPU_IOMEM dpu_base)
{
	g_flow_tpc_common_cfg_table =
		(struct tpc_common_cfg_map *)xring_flowctrl_get_cfg_table(MST_TPC_COMMON_TABLE_ID);
	g_flow_tpc_cfg_table =
		(struct tpc_cfg_map *)xring_flowctrl_get_cfg_table(MST_TPC_TABLE_ID);
	g_dpu_base = dpu_base;
}
