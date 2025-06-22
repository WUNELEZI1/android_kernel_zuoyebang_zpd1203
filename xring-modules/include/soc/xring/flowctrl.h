/* SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (C) 2022-2023, X-Ring technologies Inc., All rights reserved.
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
#ifndef __FRANKLIN_SYS_FLOWCTRL__
#define __FRANKLIN_SYS_FLOWCTRL__

enum flowctrl_switch_sel {
	SYS_QOS_ALLOW_DEBUG_EN = 0,
	SYS_FLOW_CTRL_EN,
	DDR_CH3_QOS_ALLOW_R_EN,
	DDR_CH2_QOS_ALLOW_R_EN,
	DDR_CH1_QOS_ALLOW_R_EN,
	DDR_CH0_QOS_ALLOW_R_EN,
	DDR_CH3_QOS_ALLOW_W_EN,
	DDR_CH2_QOS_ALLOW_W_EN,
	DDR_CH1_QOS_ALLOW_W_EN,
	DDR_CH0_QOS_ALLOW_W_EN,
	DDR_CH3_DVFS_GOING_QOS_ALLOW_R_EN,
	DDR_CH2_DVFS_GOING_QOS_ALLOW_R_EN,
	DDR_CH1_DVFS_GOING_QOS_ALLOW_R_EN,
	DDR_CH0_DVFS_GOING_QOS_ALLOW_R_EN,
	DDR_CH3_DVFS_GOING_QOS_ALLOW_W_EN,
	DDR_CH2_DVFS_GOING_QOS_ALLOW_W_EN,
	DDR_CH1_DVFS_GOING_QOS_ALLOW_W_EN,
	DDR_CH0_DVFS_GOING_QOS_ALLOW_W_EN,
	ISP_FLUX_REQ_R_EN,
	ISP_FLUX_REQ_W_EN,
	DPU_FLUX_REQ_R_EN,
	DPU_FLUX_REQ_W_EN,
	FLOWCTLR_SWITCH_SEL_CNT,
};

enum flowctrl_debug_id {
	FLUX_REQ_RO_ID = 0,
	DDR0_QOS_ALLOW_RO_ID,
	DDR1_QOS_ALLOW_RO_ID,
	DDR2_QOS_ALLOW_RO_ID,
	DDR3_QOS_ALLOW_RO_ID,
	SYS_QOS_ALLOW_RO_ID,
	SYS_RD_QOS_ALLOW0_RO_ID,
	SYS_RD_QOS_ALLOW1_RO_ID,
	SYS_RD_QOS_ALLOW2_RO_ID,
	SYS_RD_QOS_ALLOW3_RO_ID,
	SYS_RD_QOS_ALLOW4_RO_ID,
	SYS_RD_QOS_ALLOW5_RO_ID,
	SYS_RD_QOS_ALLOW6_RO_ID,
	SYS_RD_QOS_ALLOW7_RO_ID,
	SYS_WR_QOS_ALLOW0_RO_ID,
	SYS_WR_QOS_ALLOW1_RO_ID,
	SYS_WR_QOS_ALLOW2_RO_ID,
	SYS_WR_QOS_ALLOW3_RO_ID,
	SYS_WR_QOS_ALLOW4_RO_ID,
	SYS_WR_QOS_ALLOW5_RO_ID,
	SYS_WR_QOS_ALLOW6_RO_ID,
	SYS_WR_QOS_ALLOW7_RO_ID,

	FLOWCTRL_DEBUG_RO_MAX
};

// public API
#if IS_ENABLED(CONFIG_XRING_SYS_FLOWCTRL)
int xring_flowctrl_regulator_cfg(u32 power_id);
int xring_flowctrl_usb_qos_cfg(bool usb_audio_mode);
void xring_flowctrl_switch(enum flowctrl_switch_sel field_name, bool enable);
u32 xring_flowctrl_debug_query(enum flowctrl_debug_id id);
void xring_flowctrl_dpu0_cfg(void);
void xring_flowctrl_dpu1_cfg(void);
void xring_flowctrl_dpu2_cfg(void);
void xring_flowctrl_veu_top_subsys_cfg(void);
#else
static inline int xring_flowctrl_regulator_cfg(u32 power_id) {
	return 0;
}
static inline int xring_flowctrl_usb_qos_cfg(bool usb_audio_mode) {
	return 0;
}
static inline void xring_flowctrl_switch(enum flowctrl_switch_sel field_name, bool enable) {}
static inline u32 xring_flowctrl_debug_query(enum flowctrl_debug_id id) {
	return 0;
}
static inline void xring_flowctrl_dpu0_cfg(void) {}
static inline void xring_flowctrl_dpu1_cfg(void) {}
static inline void xring_flowctrl_dpu2_cfg(void) {}
static inline void xring_flowctrl_veu_top_subsys_cfg(void) {}
#endif

#endif
