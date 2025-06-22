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

#ifndef _DSI_HW_TMG_H_
#define _DSI_HW_TMG_H_

#include "dpu_osal.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsi.h"

#define TMG_VIDEO_MODE                  0x1
#define TMG_COMMAND_MODE                0x0

#define TMG_VRR_ENABLE                  0x1
#define TMG_VRR_DISABLE                 0x0

#define TMG_ENABLE                      0x1
#define TMG_DISABLE                     0x0

#define TMG_DOZE_ENABLE                 0x1
#define TMG_DOZE_DISABLE                0x0

#define TMG_CMD_WAIT_50MS               50000000
#define TMG_CORE_INIT_LN_CFG_NUM        100
#define TMG_SOF_CFG_SELECT_VACT_START   0
#define TMG_SOF_CFG_SELECT_VSYNC_RISING 1
#define TMG_SOF_CFG_LN_NUM              80

#define TMG_SELECT_TE_FROM_GPIO         0x0
#define TMG_SELECT_TE_FROM_DSI          0x1

#define TMG_CMD_TIME_AUTO_CAL_EN        0x0

#define TMG_BASE_0                      0x0
#define TMG_BASE_1                      0x1
#define TMG_BASE_2                      0x2
#define TMG_BASE_3                      0x3

#define TMG_CMDLIST_LINE_CNT            200
#define TMG_DVFS_LINE_CNT               100

#define TMG_T_LINE_NS_DEFAULTE          300
#define TMG_T_DOZE_END_NS               300000
#define TMG_T_PWUP_THRE_NS              4000000
#define TMG_T_ULPS_EXIT_NS              2500000

#define TMG_BLOCK_0                     0x0
#define TMG_BLOCK_1                     0x1

#define TMG_MIN_VFP                     7

/* vsync tmiestamp clk(2.73Mhz) cycle: 366.3ns * 10 */
#define T_VSYNC_TIMESTAMP_CLK           36625

enum ipi_format {
	TMG_FMT_RGB = 0x0,
	TMG_FMT_DSC = 0xB,
	TMG_FMT_MAX,
};

enum ipi_depth {
	TMG_IPI_DEPTH_666    = 0x3,
	/* DSC use this item */
	TMG_IPI_DEPTH_888    = 0x5,
	TMG_IPI_DEPTH_101010 = 0x6,
	TMG_IPI_DEPTH_MAX,
};

enum tmg_format {
	TMG_FORMAT_RGB666 = 0x0,
	TMG_FORMAT_RGB888,
	TMG_FORMAT_RGB101010,
	TMG_FORMAT_DSC,
	TMG_FORMAT_MAX,
};

struct side_by_side {
	/* side by side function delay circule : pixel clk(ipi_clk) */
	u32 sbs_dly_num;
	/* enable side by side delay:0-off, 1-on */
	u32 sbs_dly_en;
	/* side by side hact: = hact / 2 */
	u32 sbs_hact_div2;
	/* side by side overlap */
	u32 overlap;
	/* side by side interrupt enable: 0- off, 1- on */
	u8 sbs_int_index;
};

struct ulps_parms {
	u8 ulps_dly_fm;
	u8 ulps_ln_enter;
	u8 ulps_pre_fm;
	u8 ulps_ln_next;
};

struct tmg_irq_parms {
	u8 onl0_dvfs_fail_int;
	u8 onl0_ddr_dvfs_req_int;
	u8 onl0_tmg_vact_start_int;
	u8 onl0_tmg_hof_int;
	u8 onl0_tmg_partition_pwup_trig_int;
	u8 onl0_tmg_ulps_entry_req_int;
	u8 onl0_tmg_ulps_exit_req_int;
	u8 onl0_tmg_pwdn_trig_int;
	u8 onl0_skip_video_te_init;
};

struct common_parms {
	/* tmg auto calculate htotal */
	u16 core_init_ln_cfg_num;
	u32 half_frame_sel;
	u32 sof_cfg_ln_num;
	u32 cfg_int_ln;
};

#endif /* _DSI_HW_TMG_H_ */
