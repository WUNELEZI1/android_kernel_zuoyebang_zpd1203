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
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved. */

#include "dpu_hw_init_module_ops.h"
#include "dpu_hw_power_ops.h"
#include "dpu_hw_top_reg.h"
#include "dpu_hw_tpc_module_ops.h"

#define DPU_CLK_AUTO_CG_ENABLE_CFG_VAL                        0xFFFFFFFF
#define DPU_CLK_AUTO_CG_DISABLE_CFG_VAL                       0x00000000

#define DPU_TOP_CLK_AUTO_CG_OFFSET                            0x00000110
#define DPU_TOP_CLK_AUTO_CG_ENABLE_CFG_VAL                    0x00ff0000
#define DPU_TOP_CLK_AUTO_CG_DISABLE_CFG_VAL                   0x00ff00ff

#define DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL5_OFFSET              0x00000330
#define DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL5_VALUE               0x0000003f

#define DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL6_OFFSET              0x00000340
#define DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL6_VALUE               0x0000003f

#define MEDIA1_CRG_ADDRESS                                    0xE7A00000
#define MEDIA1_CRG_SIZE                                       0x00001000

#define DPU_TMG0_MCLK_AUTO_EN_SHIFT                           5
#define DPU_TMG1_MCLK_AUTO_EN_SHIFT                           6
#define DPU_TMG2_MCLK_AUTO_EN_SHIFT                           7

#define DPU_CMDLIST_CHANNEL_NUM                               13
#define DPU_CMDLIST_OUTSTANDING_VALUE                         4

static DPU_IOMEM g_media1_crg_base;
static DPU_IOMEM g_dpu_base;

struct module_partition_info {
	u32 part_id;
	u32 offset;
};

struct module_partition_info g_prepipe_info[] = {
	{DPU_PARTITION_1, 0x1400},  // G0 prepipe
	{DPU_PARTITION_1, 0xB400},  // G1 prepipe
	{DPU_PARTITION_1, 0xD400},  // G2 prepipe
	{DPU_PARTITION_1, 0xE400},  // G3 prepipe
	{DPU_PARTITION_2, 0x10400}, // G4 prepipe
	{DPU_PARTITION_2, 0x13400}, // G5 prepipe
};

struct module_partition_info v_prepipe_info[] = {
	{DPU_PARTITION_1, 0x2400},  // V0 prepipe
	{DPU_PARTITION_1, 0xC400},  // V1 prepipe
	{DPU_PARTITION_2, 0x11400}, // V2 prepipe
	{DPU_PARTITION_2, 0x12400}, // V3 prepipe
};

void dpu_hw_auto_cg_reset(u32 part_id)
{
	switch (part_id) {
	case DPU_PARTITION_1:
		// dpu_top_reg_72; bit[8:0] = 0
		DPU_REG_WRITE_BARE(g_dpu_base + 0x000120, 0x00000000);
		// dpu_top_reg_74; bit[12:0] = 0
		DPU_REG_WRITE_BARE(g_dpu_base + 0x000128, 0x00000000);
		// dpu_top_reg_88; bit[8:0] = 0
		DPU_REG_WRITE_BARE(g_dpu_base + 0x000160, 0x00000000);
		// dpu_top_reg_95; bit[5:0] = 0
		DPU_REG_WRITE_BARE(g_dpu_base + 0x00017C, 0x00000000);

		break;
	case DPU_PARTITION_2:
		// dpu_top_reg_81; bit[6:0] = 0
		DPU_REG_WRITE_BARE(g_dpu_base + 0x000144, 0x00000000);
		// dpu_top_reg_83; bit[12:0] = 0
		DPU_REG_WRITE_BARE(g_dpu_base + 0x00014C, 0x00000000);

		break;
	default:
		break;
	}
}

static void dpu_p0_auto_cg_config(bool enable)
{
	u32 value;
	u32 reg_val;

	if (enable)
		reg_val = 0xFFFFFFFF;
	else
		reg_val = 0x0;

	/*
	 * dpu_top_reg_68; bit[9:5] for mclk, [4:0] for aclk
	 * Attention chip constraint: bit[7:5] tmg mclk auto cg must be disabled
	 */
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000110, reg_val & 0xFFFFFF1F);

	// mclk config:
	// dpu_top_reg_69; bit[1] = 1
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000114, reg_val);
	// dpu_top_reg_71; bit[2:0] = 1
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00011C, reg_val);
	// dpu_top_reg_96; bit[3:0] = 1
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000180, reg_val);

	if (enable) {
		// TMG0_DSI0 tmg_dsi_reg_70; bit[0] MCLK_CG2_AUTO_EN = 1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x051318);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x051318, value | 0x1);

		// TMG1_DSI1 tmg_dsi_reg_70; bit[0] MCLK_CG2_AUTO_EN = 1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x054318);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x054318, value | 0x1);

		// TMG2_DP tmg_dp_reg_31; bit[0] MCLK_CG2_AUTO_EN = 1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x05707C);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x05707C, value | 0x1);
	} else {
		// TMG0_DSI0 tmg_dsi_reg_70; bit[0] MCLK_CG2_AUTO_EN = 0
		value = DPU_REG_READ_BARE(g_dpu_base + 0x051318);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x051318, value & 0xFFFFFFFE);

		// TMG1_DSI1 tmg_dsi_reg_70; bit[0] MCLK_CG2_AUTO_EN = 0
		value = DPU_REG_READ_BARE(g_dpu_base + 0x054318);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x054318, value & 0xFFFFFFFE);

		// TMG2_DP tmg_dp_reg_31; bit[0] MCLK_CG2_AUTO_EN = 0
		value = DPU_REG_READ_BARE(g_dpu_base + 0x05707C);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x05707C, value & 0xFFFFFFFE);
	}

	// aclk config
	// hardware constraint workaround:
	// must has waiting times that more than three times register R/W before this
	// to makesure rch top config takes effect
	// dpu_top_reg_70; bit[2:1] = 0x3
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000118, reg_val);
	// dpu_top_reg_97; bit[4:0] = 0x1F
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000184, reg_val);
}

static void dpu_p1_auto_cg_config(bool enable)
{
	u32 value;
	u32 reg_val;

	if (enable)
		reg_val = 0xFFFFFFFF;
	else
		reg_val = 0x0;

	// mclk config:
	// dpu_top_reg_73; bit[5:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000124, reg_val);
	// dpu_top_reg_75; bit[3:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00012C, reg_val);
	// dpu_top_reg_89; bit[19:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000164, reg_val);
	// dpu_top_reg_91; bit[2:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00016C, reg_val);
	// dpu_top_reg_92; bit[22:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000170, reg_val);
	// dpu_top_reg_93; bit[22:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000174, reg_val);
	// prepqfull_pipe1 dpu_2dscl_reg_295; bit[2:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x041C9C, reg_val);
	// prepqfull_pipe3 dpu_2dscl_reg_295; bit[2:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x043C9C, reg_val);
	// postpqfull dpu_2dscl_reg_295; bit[2:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x029C9C, reg_val);

	if (enable) {
		// postpipe_full_reg_16; bit[0] DSC0_CLK_AUTO_EN = 0
		value = DPU_REG_READ_BARE(g_dpu_base + 0x018340);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x018340, value | 0x1);
		// postpipe_lite_reg_16; bit[0] DSC1_CLK_AUTO_EN = 0
		value = DPU_REG_READ_BARE(g_dpu_base + 0x030340);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x030340, value | 0x1);
	} else {
		value = DPU_REG_READ_BARE(g_dpu_base + 0x018340);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x018340, value & 0xFFFFFFFE);
		value = DPU_REG_READ_BARE(g_dpu_base + 0x018340);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x030340, value & 0xFFFFFFFE);
	}

	// prepqfull_pipe0 (G0) g_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x001458, reg_val);
	// prepqfull_pipe0 (G0) g_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x001490, reg_val);
	// prepqfull_pipe1 (V0) v_prepipe_reg_17; bit[0] DPU_PREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x002444, reg_val);
	// prepqfull_pipe1 (V0) v_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x002458, reg_val);
	// prepqfull_pipe1 (V0) v_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x002490, reg_val);
	// prepqfull_pipe2 (G1) g_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00B458, reg_val);
	// prepqfull_pipe2 (G1) g_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00B490, reg_val);
	// prepqfull_pipe3 (V1) v_prepipe_reg_17; bit[0] DPU_PREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00C444, reg_val);
	// prepqfull_pipe3 (V1) v_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00C458, reg_val);
	// prepqfull_pipe3 (V1) v_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00C490, reg_val);
	// prepqfull_pipe4 (G2) g_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00D458, reg_val);
	// prepqfull_pipe4 (G2) g_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00D490, reg_val);
	// prepqfull_pipe5 (G3) g_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00E458, reg_val);
	// prepqfull_pipe5 (G3) g_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00E490, reg_val);

	// aclk config
	// hardware constraint workaround:
	// must has waiting times that more than three times register R/W before this
	// to makesure rch top config takes effect
	// dpu_top_reg_72; bit[8:0] = 0x1FF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000120, reg_val);
	// dpu_top_reg_74; bit[12:0] = 0x1FFF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000128, reg_val);
	// dpu_top_reg_95; bit[5:0] = 0x3F
	// 0x17C must be configured before 0x160
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00017C, reg_val);
	// dpu_top_reg_88; bit[8:0] = 0x1FF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000160, reg_val);
}

static void dpu_p2_auto_cg_config(bool enable)
{
	u32 reg_val;

	if (enable)
		reg_val = 0xFFFFFFFF;
	else
		reg_val = 0x0;

	// dpu_top_reg_82; bit[3:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000148, reg_val);
	// dpu_top_reg_84; bit[3:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000150, reg_val);
	// prepqlite_pipe1 dpu_2dscl_reg_295; bit[2:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x04949C, reg_val);
	// prepqlite_pipe2 dpu_2dscl_reg_295; bit[2:0] = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x04B49C, reg_val);
	// prepqlite_pipe0 (G4) g_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x010458, reg_val);
	// prepqlite_pipe0 (G4) g_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x010490, reg_val);
	// prepqlite_pipe1 (V2) v_prepipe_reg_17; bit[0] DPU_PREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x011444, reg_val);
	// prepqlite_pipe1 (V2) v_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x011458, reg_val);
	// prepqlite_pipe1 (V2) prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x011490, reg_val);
	// prepqlite_pipe2 (V3) v_prepipe_reg_17; bit[0] DPU_PREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x012444, reg_val);
	// prepqlite_pipe2 (V3) v_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x012458, reg_val);
	// prepqlite_pipe2 (V3) v_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x012490, reg_val);
	// prepqlite_pipe3 (G5) g_prepipe_reg_22; bit[0] DPU_NONPREALPHA_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x013458, reg_val);
	// prepqlite_pipe3 (G5) g_prepipe_reg_36; bit[0] DITHER_CG2_AUTO_EN = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x013490, reg_val);

	// aclk config
	// hardware constraint workaround:
	// must has waiting times that more than three times register R/W before this
	// to makesure rch top config takes effect
	// dpu_top_reg_81; bit[6:0] = 0x7F
	DPU_REG_WRITE_BARE(g_dpu_base + 0x000144, reg_val);
	// dpu_top_reg_83; bit[12:0] = 0x1FFF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00014C, reg_val);
}

void dpu_hw_auto_cg_disable(u32 part_id)
{
	switch (part_id) {
	case DPU_PARTITION_0:
		dpu_p0_auto_cg_config(false);
		break;
	case DPU_PARTITION_1:
		dpu_p1_auto_cg_config(false);
		break;
	case DPU_PARTITION_2:
		dpu_p2_auto_cg_config(false);
		break;
	default:
		PERF_ERROR("invalid partition id:%u\n", part_id);
		break;
	}
}

void dpu_hw_auto_cg_enable(u32 part_id)
{
	switch (part_id) {
	case DPU_PARTITION_0:
		dpu_p0_auto_cg_config(true);
		break;
	case DPU_PARTITION_1:
		dpu_p1_auto_cg_config(true);
		break;
	case DPU_PARTITION_2:
		dpu_p2_auto_cg_config(true);
		break;
	default:
		PERF_ERROR("invalid partition id:%u\n", part_id);
		break;
	}
}

void dpu_hw_top_clk_auto_cg_cfg(u32 op)
{
	u32 value;

	value = (op == ENABLE_AUTO_CG) ? DPU_TOP_CLK_AUTO_CG_ENABLE_CFG_VAL :
			DPU_TOP_CLK_AUTO_CG_DISABLE_CFG_VAL;

	if (!g_media1_crg_base) {
		PERF_ERROR("g_media1_crg_base uninitialized\n");
		return;
	}

	DPU_REG_WRITE_BARE(g_media1_crg_base + DPU_TOP_CLK_AUTO_CG_OFFSET, value);
}

static void dpu_hw_clk_init(void)
{
	if (!g_media1_crg_base) {
		PERF_ERROR("g_media1_crg_base uninitialized\n");
		return;
	}

	DPU_REG_WRITE_BARE(g_media1_crg_base + DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL5_OFFSET,
			DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL5_VALUE);

	DPU_REG_WRITE_BARE(g_media1_crg_base + DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL6_OFFSET,
			DPU_TOP_CLK_MEDIA_CRG_AUTOGTCTRL6_VALUE);
}

static void dpu_p0_phase0_hw_init(struct dpu_hw_init_cfg *cfg)
{
	u32 i, value;

	if (cfg->auto_cg_cfg == DISABLE_AUTO_CG)
		dpu_p0_auto_cg_config(false);

	// dpu_top_reg_7; bit[7:0] shut_down_num = 0xFF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00001C, 0x000000FF);

	// tmg
	// TMG0_DSI0 tmg_dsi_reg_51; bit[19:0] DDR_DVFS_OK_LN_TRIG = 0xF_FFFF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x0512CC, 0x000FFFFF);
	// TMG1_DSI1 tmg_dsi_reg_51; bit[19:0] DDR_DVFS_OK_LN_TRIG = 0xF_FFFF
	DPU_REG_WRITE_BARE(g_dpu_base + 0x0542CC, 0x000FFFFF);
	// TMG2_DP tmg_dp_reg_21; bit[19:0] DDR_DVFS_OK_LN_TRIG = 0xF_FFFF
	value = DPU_REG_READ_BARE(g_dpu_base + 0x0570CC);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x0570CC, value | 0x000FFFFF);

	// cmdlist
	// DPU cmdlist_reg_48; bit[28:25] cmdlist_ch_osd_dep 4
	for (i = 0; i < DPU_CMDLIST_CHANNEL_NUM; i++) {
		value = DPU_REG_READ_BARE(g_dpu_base + 0x0005C0 + i * 4);
		value = MERGE_BITS(value, DPU_CMDLIST_OUTSTANDING_VALUE,
				25, 4);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x0005C0 + i * 4, value);
	}

	// dsc_rdma
	// dsc_rdma0 dsc_rdma_reg_0; bit[7] burst_split_en = 1
	DPU_REG_WRITE_BARE(g_dpu_base + 0x051100, 0x000000BF);
	// dsc_rdma0 dsc_rdma_reg_4; bit[5:0] rd_burst_len = 15
	DPU_REG_WRITE_BARE(g_dpu_base + 0x051110, 0x0000000F);
	// dsc_rdma1 dsc_rdma_reg_0; bit[7] burst_split_en = 1
	DPU_REG_WRITE_BARE(g_dpu_base + 0x054100, 0x000000BF);
	// dsc_rdma1 dsc_rdma_reg_4; bit[5:0] rd_burst_len = 15
	DPU_REG_WRITE_BARE(g_dpu_base + 0x054110, 0x0000000F);

	// mmu
	// dsc_rd (MMU0) mmu_top_reg_2; bit[18:16] dmac0_burst_length = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x051008, 0x00000010);
}

static void dpu_p0_phase1_hw_init(struct dpu_hw_init_cfg *cfg)
{
	if (cfg->auto_cg_cfg == ENABLE_AUTO_CG)
		dpu_p0_auto_cg_config(true);

	if (cfg->sram_lp_cfg == ENABLE_SRAM_LP) {
		// dpu_top_reg_4; bit[3:0] dsc_rd_mem_lp_auto_en = 0xF
		DPU_REG_WRITE_BARE(g_dpu_base + 0x000010, 0x0000000F);
		// DPU cmdlist_reg_127; bit[2:0] mem_lp_auto_en = 0x7
		DPU_REG_WRITE_BARE(g_dpu_base + 0x0006FC, 0x00000007);
		// TMG0_DSI0 tmg_dsi_reg_31; bit[5] CORE_MEM_LP_AUTO_EN_BUF = 0x1
		DPU_REG_WRITE_BARE(g_dpu_base + 0x05127C, 0x00000020);
		// TMG1_DSI1 tmg_dsi_reg_31; bit[5] CORE_MEM_LP_AUTO_EN_BUF = 0x1
		DPU_REG_WRITE_BARE(g_dpu_base + 0x05427C, 0x00000020);
		// TMG2_DP tmg_dp_reg_16; bit[21] CORE_MEM_LP_AUTO_EN_BUF = 0x1
		DPU_REG_WRITE_BARE(g_dpu_base + 0x057040, 0x00200000);
	}
}

static void dpu_p1_phase0_hw_init(struct dpu_hw_init_cfg *cfg)
{
	u32 value;

	if (cfg->auto_cg_cfg == DISABLE_AUTO_CG)
		dpu_p1_auto_cg_config(false);

	// prepqfull_pipe1 dpu_2dscl_reg_296; bit[15:0] sd_exit_cnt = 0x0001
	DPU_REG_WRITE_BARE(g_dpu_base + 0x041CA0, 0x00000001);
	// prepqfull_pipe3 dpu_2dscl_reg_296; bit[15:0] sd_exit_cnt = 0x0001
	DPU_REG_WRITE_BARE(g_dpu_base + 0x043CA0, 0x00000001);
	// postpqfull dpu_2dscl_reg_296; bit[15:0] sd_exit_cnt = 0x0001
	DPU_REG_WRITE_BARE(g_dpu_base + 0x029CA0, 0x00000001);
	// postpipe_full_reg_13;
	// bit[31:16] DSC0_MEM_SD_CNT_MAX = 0x00FF, bit[15:0] DSC0_MEM_SD_DLY = 0x0000
	DPU_REG_WRITE_BARE(g_dpu_base + 0x018334, 0x00FF0000);
	// postpipe_lite_reg_13;
	// bit[31:16] DSC1_MEM_SD_CNT_MAX = 0x00FF, bit[15:0] DSC1_MEM_SD_DLY = 0x0000
	DPU_REG_WRITE_BARE(g_dpu_base + 0x030334, 0x00FF0000);

	// Partion1
	// rdma_top G0
	// G0 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x001284, 0x00004015);
	// G0 rdma_top_reg_2; bit[3:1] dmac1_burst_len = 2; bit[15:8] dmac1_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x001288, 0x00004015);
	// V0 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x002284, 0x00004015);
	// V0 rdma_top_reg_2; bit[3:1] dmac1_burst_len = 2; bit[15:8] dmac1_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x002288, 0x00004015);
	// G1 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00B284, 0x00004015);
	// G1 rdma_top_reg_2; bit[3:1] dmac1_burst_len = 2; bit[15:8] dmac1_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00B288, 0x00004015);
	// V1 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00C284, 0x00004015);
	// V1 rdma_top_reg_2; bit[3:1] dmac1_burst_len = 2; bit[15:8] dmac1_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00C288, 0x00004015);
	// G2 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00D284, 0x00004015);
	// G2 rdma_top_reg_2; bit[3:1] dmac1_burst_len = 2; bit[15:8] dmac1_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00D288, 0x00004015);
	// G3 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00E284, 0x00004015);
	// G3 rdma_top_reg_2; bit[3:1] dmac1_burst_len = 2; bit[15:8] dmac1_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00E288, 0x00004015);

	// rdma_path partion1_harden0:
	// G0 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x001000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x001000, value | 0x001F0000);
	// V0 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x002000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x002000, value | 0x001F0000);
	// G1 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x00B000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00B000, value | 0x001F0000);
	// V1 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x00C000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00C000, value | 0x001F0000);
	// G2 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x00D000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00D000, value | 0x001F0000);
	// G3 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x00E000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00E000, value | 0x001F0000);

	// wdma_top partion1_harden1
	// dpu_wdma_top_reg_2; bit[7:0] dmac_wr_outs_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x050108, 0x00000040);

	// wb_core partion1_harden1
	// wb0 dpu_wb_core_reg_13; bit[4:0] wb_wdma_outstanding_num = 31
	DPU_REG_WRITE_BARE(g_dpu_base + 0x050234, 0x00002F1F);
	// wb1 dpu_wb_core_reg_13; bit[4:0] wb_wdma_outstanding_num = 31
	DPU_REG_WRITE_BARE(g_dpu_base + 0x050834, 0x00002F1F);

	// dsc_wdma
	// dsc_wdma0 dsc_wdma_reg_4; bit[5:0] dsc_wdma_outstanding_num = 31
	DPU_REG_WRITE_BARE(g_dpu_base + 0x029410, 0x00002F1F);
	// dsc_wdma1 dsc_wdma_reg_4; bit[5:0] dsc_wdma_outstanding_num = 31
	DPU_REG_WRITE_BARE(g_dpu_base + 0x029610, 0x00002F1F);

	// mmu
	// wdma (MMU1) mmu_top_reg_2; bit[18:16] dmac0_burst_length = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x050008, 0x00000010);
	// rdma (MMU2) mmu_top_reg_2; bit[18:16] dmac0_burst_length = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x001208, 0x00000010);
	// rdma (MMU2) mmu_top_reg_3; bit[18:16] dmac1_burst_length = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x00120C, 0x00000010);
}

static void dpu_p1_phase1_hw_init(struct dpu_hw_init_cfg *cfg)
{
	volatile u32 value;

	if (cfg->auto_cg_cfg == ENABLE_AUTO_CG)
		dpu_p1_auto_cg_config(true);

	// prepqfull_pipe1 dpu_2dscl_reg_297;
	// bit[0] mem_lp_auto_en = 0x0, bit[1] extra_lp_auto_en = 0x0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x041CA4, 0x00000000);

	// prepqfull_pipe3 dpu_2dscl_reg_297;
	// bit[0] mem_lp_auto_en = 0x0, bit[1] extra_lp_auto_en = 0x0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x043CA4, 0x00000000);

	// postpqfull dpu_2dscl_reg_297;
	// bit[0] mem_lp_auto_en = 0x0, bit[1] extra_lp_auto_en = 0x0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x029CA4, 0x00000000);

	if (cfg->sram_lp_cfg == ENABLE_SRAM_LP) {
		// Partion1
		// G0 rdma_top_reg_4; bit[28:0] mem_lp_auto_en = 0x1FFF_FFFF
		DPU_REG_WRITE_BARE(g_dpu_base + 0x001290, 0x1FFFFFFF);

		// postpqfull acad_reg_0; bit[5] mem_lp_sd_en = 0x1
		// value = DPU_REG_READ_BARE(g_dpu_base + 0x018A00);
		// DPU_REG_WRITE_BARE(g_dpu_base + 0x018A00, value | 0x00000020);

		// postpqfull usr_gma_reg_0; bit[1] mem_lp_auto_en = 0x1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x020900);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x020900, value | 0x00000002);
		// postpq_full_reg_0; bit[6] dither_mem_lp_en = 0x1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x018400);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x018400, value | 0x00000040);
		// postpipe_full_reg_29; bit[0] split_mem_lp_en = 0x1
		DPU_REG_WRITE_BARE(g_dpu_base + 0x018374, 0x00000001);

		// postpipe_full_reg_16; bit[1] DSC0_MEM_SD_EN = 0x1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x018340);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x018340, value | 0x00000002);

		// postpipe_lite_reg_16; bit[1] DSC1_MEM_SD_EN = 0x1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x030340);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x030340, value | 0x00000002);
		// postpq_lite_reg_0; bit[6] dither_mem_lp_en = 0x1
		value = DPU_REG_READ_BARE(g_dpu_base + 0x030400);
		DPU_REG_WRITE_BARE(g_dpu_base + 0x030400, value | 0x00000040);

		// Note: disable wdma_top & wb_core mem_lp for hardware constraint
		// dpu_wdma_top_reg_3; bit[3:0] mem_lp_auto_en = 0xF
		//DPU_REG_WRITE_BARE(g_dpu_base + 0x05010C, 0x0000000F);
		// dpu_wb_core_reg_57; bit[5:0] mem_lp_auto_en = 0x3F
		//DPU_REG_WRITE_BARE(g_dpu_base + 0x0502E4, 0x0000003F);
		// dpu_wb_core_reg_57; bit[5:0] mem_lp_auto_en = 0x3F
		//DPU_REG_WRITE_BARE(g_dpu_base + 0x0508E4, 0x0000003F);

		// dsc_wdma_reg_7; bit[0] mem_lp_auto_en = 0x1
		DPU_REG_WRITE_BARE(g_dpu_base + 0x02941C, 0x00000001);
		// dsc_wdma_reg_7; bit[0] mem_lp_auto_en = 0x1
		DPU_REG_WRITE_BARE(g_dpu_base + 0x02961C, 0x00000001);
	}
}

static void dpu_p2_phase0_hw_init(struct dpu_hw_init_cfg *cfg)
{
	u32 value;

	if (cfg->auto_cg_cfg == DISABLE_AUTO_CG)
		dpu_p2_auto_cg_config(false);

	// prepqlite_pipe1 dpu_2dscl_reg_296; bit[15:0] sd_exit_cnt = 0x0001
	DPU_REG_WRITE_BARE(g_dpu_base + 0x0494A0, 0x00000001);
	// prepqlite_pipe2 dpu_2dscl_reg_296; bit[15:0] sd_exit_cnt = 0x0001
	DPU_REG_WRITE_BARE(g_dpu_base + 0x04B4A0, 0x00000001);

	// rdma_top partion2:
	// G4 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x010284, 0x00004015);
	// V2 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x011284, 0x00004015);
	// G3 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x012284, 0x00004015);
	// G5 rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	DPU_REG_WRITE_BARE(g_dpu_base + 0x013284, 0x00004015);

	// rdma_path partion2
	// G4 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x010000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x010000, value | 0x001F0000);
	// V2 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x011000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x011000, value | 0x001F0000);
	// V3 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x012000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x012000, value | 0x001F0000);
	// G5 rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = DPU_REG_READ_BARE(g_dpu_base + 0x013000);
	DPU_REG_WRITE_BARE(g_dpu_base + 0x013000, value | 0x001F0000);

	// mmu
	// rdma (MMU3) mmu_top_reg_2; bit[18:16] dmac0_burst_length = 0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x010208, 0x00000010);
}

static void dpu_p2_phase1_hw_init(struct dpu_hw_init_cfg *cfg)
{
	if (cfg->auto_cg_cfg == ENABLE_AUTO_CG)
		dpu_p2_auto_cg_config(true);

	// prepqlite_pipe1 dpu_2dscl_reg_297;
	// bit[0] mem_lp_auto_en = 0x0, bit[1] extra_lp_auto_en = 0x0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x0494A4, 0x00000000);

	// prepqlite_pipe2 dpu_2dscl_reg_297;
	// bit[0] mem_lp_auto_en = 0x0, bit[1] extra_lp_auto_en = 0x0
	DPU_REG_WRITE_BARE(g_dpu_base + 0x04B4A4, 0x00000000);

	if (cfg->sram_lp_cfg == ENABLE_SRAM_LP) {
		// G4 rdma_top_reg_4;
		// bit[15:0] mem_lp_auto_en = 0xFFFF, bit[27:24] mem_lp_auto_en = 0xF
		DPU_REG_WRITE_BARE(g_dpu_base + 0x010290, 0x0F00FFFF);
	}
}

void dpu_hw_do_hw_init(u32 part_id, struct dpu_hw_init_cfg *cfg)
{
	if (!cfg) {
		PERF_ERROR("invalid params\n");
		return;
	}

	switch (part_id) {
	case DPU_PARTITION_0:
		dpu_p0_phase0_hw_init(cfg);
		dpu_p0_phase1_hw_init(cfg);
		dpu_hw_clk_init();
		break;
	case DPU_PARTITION_1:
		dpu_p1_phase0_hw_init(cfg);
		dpu_p1_phase1_hw_init(cfg);
		break;
	case DPU_PARTITION_2:
		dpu_p2_phase0_hw_init(cfg);
		dpu_p2_phase1_hw_init(cfg);
		break;
	default:
		PERF_ERROR("invalid partition id:%u\n", part_id);
		break;
	}
}

void dpu_hw_tmg_mclk_auto_cg_dump(void)
{
	u32 value;

	value = DPU_REG_READ_BARE(g_dpu_base + 0x000110);
	if ((value & BITS_MASK(5, 3)) != 0)
		DFX_ERROR("tmg mclk auto cg should not enable, dpu_top:%#x\n", value);
}

void dpu_hw_init_module_init(DPU_IOMEM dpu_base)
{
	REG_MAP(g_media1_crg_base, MEDIA1_CRG_ADDRESS, MEDIA1_CRG_SIZE);

	dpu_hw_tpc_module_init(dpu_base);

	g_dpu_base = dpu_base;
}

void dpu_hw_init_module_deinit(void)
{
	REG_UNMAP(g_media1_crg_base);
}
