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

#include <linux/module.h>
#include <linux/moduleparam.h>

#include "veu_drv.h"
#include "veu_defs.h"
#include "veu_uapi.h"
#include "include/veu_hw_scene_ctl_reg.h"
#include "include/veu_base_addr.h"
#include "veu_utils.h"

void veu_hw_init(struct veu_data *veu_dev)
{
	volatile u32 value;

	// veu_top_reg_6; bit[7:0] mem_sd_exit_num = 0x1
	outp32(veu_dev->base + VEU_TOP + 0x00018, 0x00000001);

	// scene_ctl
	// VEU dpu_ctl_reg_3; bit[13:8] timing_inter0 = 40
	value = inp32(veu_dev->base + VEU_SCENE_CTL0 + 0x0000C);
	outp32(veu_dev->base + VEU_SCENE_CTL0 + 0x0000C, value | 0x00002800);

	// cmdlist
	// VEU cmdlist_reg_48; bit[28:25] cmdlist_ch_osd_dep = 4
	value = inp32(veu_dev->base + VEU_CMDLIST + 0x000C0);
	outp32(veu_dev->base + VEU_CMDLIST + 0x000C0, value | 0x08000000);

	// rdma_top has one instance in partion3
	// VEU rdma_top_reg_1; bit[3:1] dmac0_burst_len = 2, bit[15:8] dmac0_osd_num = 64
	outp32(veu_dev->base + RDMA_TOP + 0x00004, 0x00002815);

	// rdma_path has one instance in partion3
	// VEU rdma_path_reg_0; bit[21:16] rdma_burst_len = 31
	value = inp32(veu_dev->base + RDMA_PATH + 0x00000);
	outp32(veu_dev->base + RDMA_PATH + 0x00000, value | 0x001F0000);

	// mmu
	// veu (MMU4) mmu_top_reg_2; bit[18:16] dmac0_burst_length = 0
	outp32(veu_dev->base + MMU_TOP + 0x00008, 0x00000010);

	// hardware constraint workaround:
	//    read 0x688 three times to makesure rch top config takes effect,
	//    then config aclk augo cg
	value = inp32(veu_dev->base + MMU_TOP + 0x00008);
	value = inp32(veu_dev->base + MMU_TOP + 0x00008);
	value = inp32(veu_dev->base + MMU_TOP + 0x00008);

	// veu_top_reg_1; bit[5:0] = 0x3F, bit[8] = 0x1
	outp32(veu_dev->base + VEU_TOP + 0x00004, 0xFFFFFFFF);
	// veu_top_reg_4; bit[6:5] = 0x3, bit[8] = 0x1
	outp32(veu_dev->base + VEU_TOP + 0x00010, 0xFFFFFFFF);
	// veu_top_reg_5; bit[13:0] = 0x3FFF
	outp32(veu_dev->base + VEU_TOP + 0x00014, 0xFFFFFFFF);

	outp32(veu_dev->base + RDMA_TBU + 0x0, 0xd00);
	outp32(veu_dev->base + WDMA_TBU + 0x0, 0xd00);
	outp32(veu_dev->base + VEU_CMDLIST + 0xc0, 0x40000e0);
}

void veu_ctrl_set_reg(struct veu_data *veu_dev)
{
	veu_check_and_void_return(!veu_dev, "veu_dev is null");
	/* select prepq0 output */
	outp32(veu_dev->base + VEU_CTL_TOP + VEU_WB0_SEL_ID, 0x1);
	/* mount rch0 wb0 to ctl */
	outp32(veu_dev->base + VEU_SCENE_CTL0 +  NML_RCH_EN_OFFSET, BIT(16) | BIT(0));
	/* 1 for offline mode */
	outp32(veu_dev->base + VEU_SCENE_CTL0 + NML_CMD_UPDT_EN_OFFSET, 0x1);
	outp32(veu_dev->base + VEU_SCENE_CTL0 + BOTH_CFG_RDY_OFFSET, 0x1);
}
