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

#ifndef _DPU_DBG_H_
#define _DPU_DBG_H_

extern u32 g_dpu_msg_level;
extern u64 g_dpu_frame_no;
extern u32 g_dpu_msg_category;
extern u32 g_dpu_lowpower_en;

/*
 * bit[16] - tpg enable
 * bit[15:8] - tpg position in enum tpg_position
 * bit[7:0] - tpg mode in enum tpg_mode_index
 */
extern u32 g_dpu_tpg_ctrl;
extern u32 g_dpu_tpg_param;
extern u32 g_cmd_wait_data_en;
extern u32 g_dpu_postpq_debug;
extern u32 g_dpu_mdr_en;
extern u32 g_debug_fence_timeline;
extern u32 g_enable_basic_dfx;

#define GET_TPG_ENABLE(cfg) \
		(((cfg) & 0x10000) >> 16)
#define GET_TPG_POSITION(cfg) \
		(((cfg) & 0xFF00) >> 8)
#define GET_TPG_MODE(cfg) \
		((cfg) & 0xFF)

struct dpu_log_map {
	u8 *user_cfg;
	u32 value;
};

/* the supported modules for this log config */
enum dpu_log_cfg_type {
	LOG_LEVEL = 0,
	LOG_CATEGORY,
};

u32 dpu_msg_value_get(enum dpu_log_cfg_type cfg_type, u8 *log_cfg);
int dpu_msg_categorys_get(u8 *log_categories_string);
u8 *dpu_category_string_get(u32 category);

#endif /* _DPU_DBG_H_ */
