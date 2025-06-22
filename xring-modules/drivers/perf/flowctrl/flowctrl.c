// SPDX-License-Identifier: GPL-2.0
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
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <dt-bindings/xring/platform-specific/flowctrl_cfg_define.h>
#include <dt-bindings/xring/platform-specific/ip_regulator_define.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/arm-smccc.h>
#include <dt-bindings/xring/platform-specific/common/smc_id/bl31_smc_id.h>
#include <soc/xring/flowctrl.h>
#include "flowctrl_reg.h"

#define SYS_QOS_CFG_ID 1
#undef pr_fmt
#define pr_fmt(fmt) "[flowctrl]:%s:%d " fmt, __func__, __LINE__

struct tpc_common_cfg_map *gp_tpc_common_cfg_table;
struct tpc_cfg_map *gp_tpc_cfg_table;

enum base_reg_id {
	ACPU_PCTRL_ID = 0,
	ACPU_DPU_TOP_ID,
	ACPU_MEDIA1_TPC_ID,
	ACPU_GPU_CTRL_ID,
	ACPU_M2_SC_TPC_ID,
	ACPU_M2_CRG_ID,
	ACPU_USB_SUBSYS_XCTRL_ID,
	BASE_REG_NUM
};
const u32 base_reg_arr[BASE_REG_NUM] = {
	ACPU_PCTRL, ACPU_DPU_TOP, ACPU_MEDIA1_TPC, ACPU_GPU_CTRL,
	ACPU_M2_SC_TPC, ACPU_M2_CRG, ACPU_USB_SUBSYS_XCTRL
};

struct reg_addr_ioremap {
	u32 pa;
	void __iomem *va;
};
struct reg_addr_ioremap g_pa_va_remap[BASE_REG_NUM];

static void xring_flowctrl_media1_dpu_common_dp1_tpc_cfg(void)
{
#define DPU1_COMMON_CHANNEL_NUM 6
#define DPU1_COMMON_CHANNEL_NA89_NUM 3
	const u32 dpu_reg_ch_offset_arr[DPU1_COMMON_CHANNEL_NUM] = {
		TPC_MED1_DPU_GDMA0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA1_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA1_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA2_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA3_NORMAL_CRG_CONFIG0_OFFSET
	};
	const u32 dpu_reg_ch_na89_arr[DPU1_COMMON_CHANNEL_NA89_NUM] = {
		TPC_MED1_DPU_WB_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_WR0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_WR1_NORMAL_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	for (i = 0; i < DPU1_COMMON_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va, dpu_reg_ch_offset_arr[i],
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x10,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x14,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x18,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x1C,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}

	for (i = 0; i < DPU1_COMMON_CHANNEL_NA89_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i],
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0x4,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0x8,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0xC,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0x10,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0x14,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0x18,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[6]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_na89_arr[i] + 0x1C,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[7]);
	}
}

static void xring_flowctrl_media1_dpu_common_dp2_tpc_cfg(void)
{
#define DPU2_COMMON_CHANNEL_NUM 4
	const u32 dpu_reg_ch_offset_arr[DPU2_COMMON_CHANNEL_NUM] = {
		TPC_MED1_DPU_GDMA4_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA2_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA3_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA5_NORMAL_CRG_CONFIG0_OFFSET};
	u32 i = 0;

	for (i = 0; i < DPU2_COMMON_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x10,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x14,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x18,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x1C,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}
}

static void xring_flowctrl_media1_dpu_common_dp0_tpc_cfg(void)
{
#define DPU0_COMMON_CHANNEL_NUM 2
	const u32 dpu_reg_ch_offset_arr[DPU0_COMMON_CHANNEL_NUM] = {
		TPC_MED1_DPU_DSC_RD0_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_RD1_NORMAL_CRG_CONFIG0_OFFSET
	};

	u32 i = 0;

	for (i = 0; i < DPU0_COMMON_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x10,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x14,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x20,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x24,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}
}

static void xring_flowctrl_media1_ctrl_common_tpc_cfg(void)
{
	// 2. MED1_TPC: MEDIA1_CTRL_COMMON
	u32 i = 0;

	for (i = 0; i < TPC_COMMON_CFG_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va,
			TPC_MED1_CTRL_NORMAL_CRG_CONFIG0_OFFSET + (i * 0x4),
			gp_tpc_common_cfg_table[M1_CTRL_COMMON_ID].val[i]);
	}
}

static void xring_flowctrl_media1_dpu_dsc_rd_tpc_cfg(void)
{
#define DPU0_DSC_RD_CHANNEL_NUM 2
	const u32 dpu_reg_ch_offset_arr[DPU0_DSC_RD_CHANNEL_NUM] = {
		TPC_MED1_DPU_DSC_RD0_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_DSC_RD1_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	// cfg0 bit15 tpc_qos_remap_en set 1 now, so need cfg 2nd times
	for (i = 0; i < DPU0_DSC_RD_CHANNEL_NUM; i++) {
		// first cfg: cfg0 bit17 set 0
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[1]); // CFG1
		// cfg0 bit17 set 0, then CFG2-1st cfg
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i], 1, 17, 17);
		// CFG2-1st
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[4]);
		// finnal cfg3
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[5]);
	}
}

static void xring_flowctrl_media1_dpu_axi_tpc_cfg(void)
{
#define DPU0_AXI_CHANNEL_NUM 6
	const u32 dpu_reg_ch_offset_arr[DPU0_AXI_CHANNEL_NUM] = {
		TPC_MED1_DPU_AXI0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_AXI1_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_AXI2_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_AXI3_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_AXI4_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_AXI5_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	for (i = 0; i < DPU0_AXI_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_cfg_table[M1_DPU_AXI_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_cfg_table[M1_DPU_AXI_ID].val[1]);
			// attention：diffent with other dpu mst,
			// CONFIG2(+0xC) is tpc qos0~7/qos8~15 remap qos val
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_AXI_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_AXI_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_AXI_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_AXI_ID].val[5]);
	}
}

static void xring_flowctrl_media1_tcu_tpc_cfg(void)
{
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_TCU_CRG_CONFIG0_OFFSET,
		gp_tpc_cfg_table[M1_TCU_ID].val[0]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_TCU_CRG_CONFIG1_OFFSET,
		gp_tpc_cfg_table[M1_TCU_ID].val[1]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_TCU_CRG_CONFIG2_OFFSET,
		gp_tpc_cfg_table[M1_TCU_ID].val[2]); // media1_tcu cfg2-1st
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_TCU_CRG_CONFIG2_OFFSET,
		gp_tpc_cfg_table[M1_TCU_ID].val[3]); // media1_tcu cfg2-2nd
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_TCU_CRG_CONFIG2_OFFSET,
		gp_tpc_cfg_table[M1_TCU_ID].val[4]); // media1_tcu cfg2-3rd
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_TCU_CRG_CONFIG3_OFFSET,
		gp_tpc_cfg_table[M1_TCU_ID].val[5]);
}

///////////////////////////////////// KERNEL RESUME CFG
static void xring_flowctrl_fc_qos_cfg(void)
{
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va, FC_QOS_OFFSET, FLOWCTRL_FC_QOS_VAL);
}

//////////////////////////////////  regulator cfg
static void xring_flowctrl_media1_subsys_cfg(void)
{
	xring_flowctrl_media1_ctrl_common_tpc_cfg();
	xring_flowctrl_media1_tcu_tpc_cfg();
}

void xring_flowctrl_dpu0_cfg(void)
{
	xring_flowctrl_media1_dpu_common_dp0_tpc_cfg();
	xring_flowctrl_media1_dpu_dsc_rd_tpc_cfg();
	xring_flowctrl_media1_dpu_axi_tpc_cfg();
}
EXPORT_SYMBOL(xring_flowctrl_dpu0_cfg);

void xring_flowctrl_dpu1_cfg(void)
{
#define CHANNEL_NUM 8
#define CHANNEL_DSC_WR_NUM 2
	const u32 dpu_reg_ch_offset_arr[CHANNEL_NUM] = {
		TPC_MED1_DPU_GDMA0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VDMA0_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA1_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VDMA1_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_GDMA2_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_GDMA3_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_WB0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_WB1_CRG_CONFIG0_OFFSET
	};

	const u32 dpu_dsc_wr_ch_offset_arr[CHANNEL_DSC_WR_NUM] = {
		TPC_MED1_DPU_DSC_WR0_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_DSC_WR1_CRG_CONFIG0_OFFSET
	};
	u32 i = 0;

	xring_flowctrl_media1_dpu_common_dp1_tpc_cfg();
	// DPU1 channel: G0, V0, G1, V1, G2, G3， WB0~1, DSC_WR0~1
	// cfg0 bit15 tpc_qos_remap_en set 1 now, so need cfg 2nd times
	for (i = 0; i < CHANNEL_NUM; i++) {
		// first cfg: cfg0 bit17 set 0
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[1]); // CFG1
		// cfg0 bit17 set 0, then CFG2-1st cfg
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i], 1, 17, 17);
		// CFG2-1st
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[4]);
		// finnal cfg3
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[5]);
	}

	for (i = 0; i < CHANNEL_DSC_WR_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_dsc_wr_ch_offset_arr[i],
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_dsc_wr_ch_offset_arr[i] + 0x4,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_dsc_wr_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[2]);
		// 2nd cfg
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_dsc_wr_ch_offset_arr[i], 1, 17, 17);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_dsc_wr_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_dsc_wr_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_DSC_ID].val[5]);
	}

}
EXPORT_SYMBOL(xring_flowctrl_dpu1_cfg);

void xring_flowctrl_dpu2_cfg(void)
{
	// DPU2里对应的是G4,V2,V3,G5, 配置方法同xring_flowctrl_dpu1_cfg
#define DPU2_CHANNEL_NUM 4
	const u32 dpu_reg_ch_offset_arr[DPU2_CHANNEL_NUM] = {
		TPC_MED1_DPU_GDMA4_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VDMA2_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VDMA3_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_GDMA5_CRG_CONFIG0_OFFSET};
	u32 i = 0;

	xring_flowctrl_media1_dpu_common_dp2_tpc_cfg();

	for (i = 0; i < DPU2_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i], 1, 17, 17);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_cfg_table[M1_DPU_VGDMAX_ID].val[5]);
	}
}
EXPORT_SYMBOL(xring_flowctrl_dpu2_cfg);

void xring_flowctrl_veu_top_subsys_cfg(void)
{
#define DPU_VEU_TOP_CHANNEL_NUM 2  // 2: MED1_DPU: VEU_RDMA/VEU_WDMA
	u32 i = 0;

	const u32 dpu_veu_reg_ch_offset_arr[DPU_VEU_TOP_CHANNEL_NUM] = {
		TPC_MED1_DPU_VEU_RD_NORMAL_CRG_CONFIG0_OFFSET,
		TPC_MED1_DPU_VEU_WR_NORMAL_CRG_CONFIG0_OFFSET
	};
	const u32 dpu_reg_ch_offset_arr[DPU_VEU_TOP_CHANNEL_NUM] = {
		TPC_MED1_DPU_VEU_RDMA_CRG_CONFIG0_OFFSET, TPC_MED1_DPU_VEU_WDMA_CRG_CONFIG0_OFFSET
	};

	// 1.16 MED1_DPU_TPC: DPU_COMMON: VEU_RD && VEU_WR
	for (i = 0; i < DPU_VEU_TOP_CHANNEL_NUM; i++) {
		// 1.16 MED1_DPU_TPC: DPU_COMMON: VEU_RD
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i],
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0xC,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0x10,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0x14,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[5]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0x18,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[8]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_veu_reg_ch_offset_arr[i] + 0x1C,
			gp_tpc_common_cfg_table[DPU_COMMON_ID].val[9]);
	}

	for (i = 0; i < DPU_VEU_TOP_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i],
			gp_tpc_cfg_table[M1_DPU_VEU_ID].val[0]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x4,
			gp_tpc_cfg_table[M1_DPU_VEU_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8,
			gp_tpc_cfg_table[M1_DPU_VEU_ID].val[2]);
		// cfg0 bit17 set 1, then cfg2-2nd cfg
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i], 1, 17, 17);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0x8, gp_tpc_cfg_table[M1_DPU_VEU_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_DPU_TOP_ID].va,
			dpu_reg_ch_offset_arr[i] + 0xC, gp_tpc_cfg_table[M1_DPU_VEU_ID].val[5]);
	}
}
EXPORT_SYMBOL(xring_flowctrl_veu_top_subsys_cfg);

static void xring_flowctrl_vdec_subsys_cfg(void)
{
	pr_debug("vpu m1 vdec cfg:\n");
	// MED1_TPC: MED1_VDECqq
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_VDEC_CRG_CONFIG0_OFFSET,
		gp_tpc_cfg_table[M1_VDEC_ID].val[0]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_VDEC_CRG_CONFIG1_OFFSET,
		gp_tpc_cfg_table[M1_VDEC_ID].val[1]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_VDEC_CRG_CONFIG2_OFFSET,
		gp_tpc_cfg_table[M1_VDEC_ID].val[2]); // media1_vdec cfg2-1st
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_VDEC_CRG_CONFIG2_OFFSET,
		gp_tpc_cfg_table[M1_VDEC_ID].val[3]); // media1_vdec cfg2-2nd
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_VDEC_CRG_CONFIG2_OFFSET,
		gp_tpc_cfg_table[M1_VDEC_ID].val[4]); // media1_vdec cfg2-3rd
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_MEDIA1_TPC_ID].va, TPC_MED1_VDEC_CRG_CONFIG3_OFFSET,
		gp_tpc_cfg_table[M1_VDEC_ID].val[5]);
}

static void xring_flowctrl_venc_subsys_cfg(void)
{
	pr_debug("vpu m2 venc cfg:\n");
	// before med2_venc cfg, reset is needed.
	FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_M2_CRG_ID].va,
		M2_VENC_TPC_CRG_OFFSET2, 0x1, 9, 9);
	FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_M2_CRG_ID].va,
		M2_VENC_TPC_CRG_OFFSET1, 0x1, 12, 12);

	// MED2_TPC: MED2_CTRL: MED2_VENC
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_VENC_CRG_CONFIG0_OFFSET, gp_tpc_cfg_table[M2_VENC_ID].val[0]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_VENC_CRG_CONFIG1_OFFSET, gp_tpc_cfg_table[M2_VENC_ID].val[1]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_VENC_CRG_CONFIG2_OFFSET, gp_tpc_cfg_table[M2_VENC_ID].val[2]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_VENC_CRG_CONFIG2_OFFSET, gp_tpc_cfg_table[M2_VENC_ID].val[3]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_VENC_CRG_CONFIG2_OFFSET, gp_tpc_cfg_table[M2_VENC_ID].val[4]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_VENC_CRG_CONFIG3_OFFSET, gp_tpc_cfg_table[M2_VENC_ID].val[5]);
}

static void xring_flowctrl_gpu_subsys_cfg(void)
{
#define GPU_CHANNEL_NUM 4
	// main_TPC: MAIN_GPU: COMMON && GPU_P0~3
	u32 i = 0;
	const u32 gpu_offset_arr[GPU_CHANNEL_NUM] = {
		TPC_MAIN_GPU0_CRG_CONFIG0_OFFSET, TPC_MAIN_GPU1_CRG_CONFIG0_OFFSET,
		TPC_MAIN_GPU2_CRG_CONFIG0_OFFSET, TPC_MAIN_GPU3_CRG_CONFIG0_OFFSET
	};

	for (i = 0; i < TPC_COMMON_CFG_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			TPC_MAIN_GPU_NORMAL_CRG_CONFIG0_OFFSET + (i * 0x4),
			gp_tpc_common_cfg_table[MAIN_GPU_COMMON_ID].val[i]);
	}

	// MAIN_GPU: GPU_P0~3
	for (i = 0; i < GPU_CHANNEL_NUM; i++) {
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			gpu_offset_arr[i], gp_tpc_cfg_table[MAIN_GPU_ID].val[0] & 0xFF, 0, 7);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			gpu_offset_arr[i] + 0x4, gp_tpc_cfg_table[MAIN_GPU_ID].val[1]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			gpu_offset_arr[i] + 0x8, gp_tpc_cfg_table[MAIN_GPU_ID].val[2]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			gpu_offset_arr[i] + 0x8, gp_tpc_cfg_table[MAIN_GPU_ID].val[3]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			gpu_offset_arr[i] + 0x8, gp_tpc_cfg_table[MAIN_GPU_ID].val[4]);
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_GPU_CTRL_ID].va,
			gpu_offset_arr[i] + 0xC, gp_tpc_cfg_table[MAIN_GPU_ID].val[5]);
	}
}

static void xring_flowctrl_media2_ctrl_common_tpc_cfg(void)
{
	u32 i = 0;

	for (i = 0; i < TPC_COMMON_CFG_NUM; i++) {
		FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
			TPC_MED2_CTRL_NORMAL_CRG_CONFIG0_OFFSET + (i * 0x4),
			gp_tpc_common_cfg_table[M2_CTRL_COMMON_ID].val[i]);
	}
}

static void xring_flowctrl_media2_tcu_tpc_cfg(void)
{
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_TCU_CRG_CONFIG0_OFFSET, gp_tpc_cfg_table[M2_TCU_ID].val[0]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_TCU_CRG_CONFIG1_OFFSET, gp_tpc_cfg_table[M2_TCU_ID].val[1]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_TCU_CRG_CONFIG2_OFFSET, gp_tpc_cfg_table[M2_TCU_ID].val[2]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_TCU_CRG_CONFIG2_OFFSET, gp_tpc_cfg_table[M2_TCU_ID].val[3]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_TCU_CRG_CONFIG2_OFFSET, gp_tpc_cfg_table[M2_TCU_ID].val[4]);
	FLOWCTRL_REG_WRITE(g_pa_va_remap[ACPU_M2_SC_TPC_ID].va,
		TPC_MED2_CTRL_TCU_CRG_CONFIG3_OFFSET, gp_tpc_cfg_table[M2_TCU_ID].val[5]);
}

static void xring_flowctrl_med2_subsys_cfg(void)
{
	// before med2_venc cfg, reset and enable clk are needed.
	FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_M2_CRG_ID].va,
		M2_VENC_TPC_CRG_OFFSET2, 0x1, 9, 9);
	FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_M2_CRG_ID].va,
		M2_VENC_TPC_CRG_OFFSET1, 0x1, 12, 12);
	// MED2_CTRL tpc cfg
	xring_flowctrl_media2_ctrl_common_tpc_cfg();
	xring_flowctrl_media2_tcu_tpc_cfg();
	pr_debug("first tcp reset and clk, then cfg done\n");
}

// **************************PUBLIC API:*****************************************
int xring_flowctrl_regulator_cfg(u32 power_id)
{
	struct arm_smccc_res res = {0xff};

	pr_debug("power_id %d to cfg flowctrl reg.\n", power_id);

	switch (power_id) {
	case MEDIA1_SUBSYS_ID:
		xring_flowctrl_media1_subsys_cfg();
		break;
	case VDEC_SUBSYS_ID:
		xring_flowctrl_vdec_subsys_cfg();
		break;
	case MEDIA2_SUBSYS_ID:
		xring_flowctrl_med2_subsys_cfg();
		break;
	case ISP_SUBSYS_ID:
		arm_smccc_smc(FID_BL31_PERF_FLOWCTRL_CFG, ISP_SUBSYS_ID, 0, 0, 0, 0, 0, 0, &res);
		break;
	case ISP_MCU_TOP_ID:
		arm_smccc_smc(FID_BL31_PERF_FLOWCTRL_CFG, ISP_MCU_TOP_ID, 0, 0, 0, 0, 0, 0, &res);
		break;
	case VENC_SUBSYS_ID:
		xring_flowctrl_venc_subsys_cfg();
		break;
	case GPU_SUBSYS_ID:
		xring_flowctrl_gpu_subsys_cfg();
		break;
	case NPU_SUBSYS_ID:
		arm_smccc_smc(FID_BL31_PERF_FLOWCTRL_CFG, NPU_SUBSYS_ID, 0, 0, 0, 0, 0, 0, &res);
		break;
	case NPU_VDSP_ID:
		arm_smccc_smc(FID_BL31_PERF_FLOWCTRL_CFG, NPU_VDSP_ID, 0, 0, 0, 0, 0, 0, &res);
		break;
	default:
		break;
	}
	return 0;
}
EXPORT_SYMBOL(xring_flowctrl_regulator_cfg);

int xring_flowctrl_usb_qos_cfg(bool usb_audio_mode)
{
	if (usb_audio_mode)
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_USB_SUBSYS_XCTRL_ID].va,
			0xE4, 0x66, 0, 7);
	else
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_USB_SUBSYS_XCTRL_ID].va,
			0xE4, 0x22, 0, 7);

	pr_debug("usb_audio_mode %d\n", usb_audio_mode);
	return 0;
}
EXPORT_SYMBOL(xring_flowctrl_usb_qos_cfg);

void xring_flowctrl_switch(enum flowctrl_switch_sel field_name, bool enable)
{
	switch (field_name) {
	case SYS_QOS_ALLOW_DEBUG_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL0_OFFSET, enable, 5, 5);
		break;
	case SYS_FLOW_CTRL_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL0_OFFSET, enable, 4, 4);
		break;
	case DDR_CH3_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL0_OFFSET, enable, 3, 3);
		break;
	case DDR_CH2_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL0_OFFSET, enable, 2, 2);
		break;
	case DDR_CH1_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL0_OFFSET, enable, 1, 1);
		break;
	case DDR_CH0_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL0_OFFSET, enable, 0, 0);
		break;
	case DDR_CH3_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 15, 15);
		break;
	case DDR_CH2_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 14, 14);
		break;
	case DDR_CH1_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 13, 13);
		break;
	case DDR_CH0_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 12, 12);
		break;
	case DDR_CH3_DVFS_GOING_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 11, 11);
		break;
	case DDR_CH2_DVFS_GOING_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 10, 10);
		break;
	case DDR_CH1_DVFS_GOING_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 9, 9);
		break;
	case DDR_CH0_DVFS_GOING_QOS_ALLOW_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 8, 8);
		break;
	case DDR_CH3_DVFS_GOING_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 7, 7);
		break;
	case DDR_CH2_DVFS_GOING_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 6, 6);
		break;
	case DDR_CH1_DVFS_GOING_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 5, 5);
		break;
	case DDR_CH0_DVFS_GOING_QOS_ALLOW_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 4, 4);
		break;
	case ISP_FLUX_REQ_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 3, 3);
		break;
	case ISP_FLUX_REQ_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 2, 2);
		break;
	case DPU_FLUX_REQ_R_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 1, 1);
		break;
	case DPU_FLUX_REQ_W_EN:
		FLOWCTRL_REG_BIT_WRITE(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_QOS_ALLOW_ENABL1_OFFSET, enable, 0, 0);
		break;
	default:
		break;
	}
	pr_info("flowctrl_switch_sel %d, enable %d\n", field_name, enable);
}
EXPORT_SYMBOL(xring_flowctrl_switch);

u32 xring_flowctrl_debug_query(enum flowctrl_debug_id id)
{
	u32 reg_val = 0;

	switch (id) {
	case FLUX_REQ_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_FLUX_REQ_RO_OFFSET);
		break;
	case DDR0_QOS_ALLOW_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_DDR0_QOS_ALLOW_RO_OFFSET);
		break;
	case DDR1_QOS_ALLOW_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_DDR1_QOS_ALLOW_RO_OFFSET);
		break;
	case DDR2_QOS_ALLOW_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_DDR2_QOS_ALLOW_RO_OFFSET);
		break;
	case DDR3_QOS_ALLOW_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_DDR3_QOS_ALLOW_RO_OFFSET);
		break;
	case SYS_QOS_ALLOW_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_QOS_ALLOW_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW0_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW0_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW1_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW1_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW2_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW2_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW3_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW3_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW4_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW4_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW5_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW5_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW6_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW6_RO_OFFSET);
		break;
	case SYS_RD_QOS_ALLOW7_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_RD_QOS_ALLOW7_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW0_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW0_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW1_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW1_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW2_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW2_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW3_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW3_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW4_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW4_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW5_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW5_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW6_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW6_RO_OFFSET);
		break;
	case SYS_WR_QOS_ALLOW7_RO_ID:
		reg_val = FLOWCTRL_REG_READ(g_pa_va_remap[ACPU_PCTRL_ID].va,
			FC_SYS_WR_QOS_ALLOW7_RO_OFFSET);
		break;
	default:
		break;
	}
	pr_info("query id = %d, reg_val = 0x%08x\n", id, reg_val);
	return reg_val;
}
EXPORT_SYMBOL(xring_flowctrl_debug_query);
/**
 * xring_flowctrl_device_suspend - Suspend callback from the OS.
 *
 * @dev:  The device to suspend
 *
 * This is called by Linux when the device should suspend.
 *
 * Return: A standard Linux error code on failure, 0 otherwise.
 */
static int xring_flowctrl_device_suspend(struct device *dev)
{
	pr_debug("save perf reg val\n");
	if (!dev)
		return -ENODEV;

	// todo: add pctrl: TPC_ADDR_MASKED0~1/INTL_MODE 寄存器的保存和恢复
	return 0;
}

/**
 * xring_flowctrl_device_resume - Resume callback from the OS.
 *
 * @dev:  The device to resume
 *
 * This is called by Linux when the device should resume from suspension.
 *
 * Return: A standard Linux error code
 */
static int xring_flowctrl_device_resume(struct device *dev)
{
	if (!dev)
		return -ENODEV;

	// reg cfg code start ....
	xring_flowctrl_fc_qos_cfg();

	pr_info("resume cfg done.\n");
	return 0;
}

static int xring_flowctrl_pdev_probe(struct platform_device *pdev)
{
	u32 i = 0;

	for (i = ACPU_PCTRL_ID; i < BASE_REG_NUM; i++) {
		g_pa_va_remap[i].pa = base_reg_arr[i];
		g_pa_va_remap[i].va = ioremap(base_reg_arr[i], 0xFF0000);
	}
	gp_tpc_common_cfg_table =
		(struct tpc_common_cfg_map *)xring_flowctrl_get_cfg_table(MST_TPC_COMMON_TABLE_ID);
	gp_tpc_cfg_table =
		(struct tpc_cfg_map *)xring_flowctrl_get_cfg_table(MST_TPC_TABLE_ID);

	xring_flowctrl_fc_qos_cfg();

	pr_notice("ioremap and get cfg table done! enable flowctrl\n");
	return 0;
}

static int xring_flowctrl_pdev_remove(struct platform_device *pdev)
{
	u32 i = 0;

	for (i = 0; i < BASE_REG_NUM; i++)
		iounmap(g_pa_va_remap[i].va);

	pr_notice("iounmap done!\n");
	return 0;
}

void flowctrl_reg_write(const void *base_addr, const u32 offset, const u32 val)
{
	void *addr = NULL;

	addr = (void *)base_addr + offset;
	writel(val, addr);
}

u32 flowctrl_reg_read(const void *base_addr, const u32 offset)
{
	u32 val = 0;

	val = readl(base_addr + offset);
	return val;
}

void flowctrl_reg_bit_write(const void *base_addr, u32 offset, u32 val, u32 start_bit, u32 end_bit)
{
	u32 mask = BITS_MASK(start_bit, end_bit);
	u32 new_cfg_val = (val << start_bit) & mask;
	u32 reg_val = 0;

	reg_val = readl(base_addr + offset);

	reg_val &= ~mask;
	// set new val
	reg_val |= new_cfg_val;
	writel(reg_val, (void *)base_addr + offset);
}

/* The power management operations for the platform driver.
 */
static const struct dev_pm_ops xring_flowctrl_dev_pm_ops = {
	.suspend = xring_flowctrl_device_suspend,
	.resume = xring_flowctrl_device_resume,
};

static const struct of_device_id flowctrl_of_match[] = {
	{ .compatible = "xring,perf_flowctrl", },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, flowctrl_of_match);

static struct platform_driver xring_flowctrl_platform_driver = {
	.probe = xring_flowctrl_pdev_probe,
	.remove = xring_flowctrl_pdev_remove,
	.driver = {
			.name = "perf_flowctrl",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(flowctrl_of_match),
			.pm = &xring_flowctrl_dev_pm_ops,
		},
};

static int xring_flowctrl_drv_register(void)
{
	return platform_driver_register(&xring_flowctrl_platform_driver);
}
static void xring_flowctrl_drv_unregister(void)
{
	platform_driver_unregister(&xring_flowctrl_platform_driver);
	pr_info("unregister success.\n");
}

module_init(xring_flowctrl_drv_register);
module_exit(xring_flowctrl_drv_unregister);

MODULE_DESCRIPTION("X-RingTek flowctrl driver");
MODULE_LICENSE("GPL v2");
