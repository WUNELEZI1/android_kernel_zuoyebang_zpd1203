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

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <drm/drm_print.h>
#include "dpu_log.h"

u32 g_dpu_msg_level = DPU_LOG_LEVEL_INFO;
u32 g_dpu_msg_category = DPU_BASE|DPU_CPU_REG|DPU_CMDLIST|
		DPU_SYNC|DPU_COLOR|DPU_DSI|DPU_DP|
		DPU_PERF|DPU_DFX|DPU_PROPERTY|DPU_CLK|
		DPU_POWER|DPU_IDLE|DPU_LAYER|DPU_RES|DPU_IOMMU|DPU_EVENT|DPU_COMPOSER;

u32 g_dpu_tpg_ctrl;
u32 g_dpu_tpg_param;
u32 g_cmd_wait_data_en = 1;
u32 g_dpu_postpq_debug;
u32 g_dpu_mdr_en;
u64 g_dpu_frame_no;
u32 g_debug_fence_timeline;
u32 g_enable_basic_dfx;

static const struct dpu_log_map dpu_msg_level_map[] = {
	{"E",           DPU_LOG_LEVEL_ERROR},
	{"W",           DPU_LOG_LEVEL_WARNING},
	{"I",           DPU_LOG_LEVEL_INFO},
	{"D",           DPU_LOG_LEVEL_DEBUG},
};

static const struct dpu_log_map dpu_msg_category_map[] = {
	{"CORE",        DPU_BASE},
	{"REG",         DPU_CPU_REG},
	{"CMDLIST",     DPU_CMDLIST},
	{"ISR",         DPU_ISR},
	{"SYNC",        DPU_SYNC},
	{"COLOR",       DPU_COLOR},
	{"DSI",         DPU_DSI},
	{"PROPERTY",    DPU_PROPERTY},
	{"DP",          DPU_DP},
	{"PERF",        DPU_PERF},
	{"DFX",         DPU_DFX},
	{"CLK",         DPU_CLK},
	{"POWER",       DPU_POWER},
	{"IDLE",        DPU_IDLE},
	{"FLOW",        DPU_FLOWCTRL},
	{"IOMMU",       DPU_IOMMU},
	{"LAYER",       DPU_LAYER},
	{"RES",         DPU_RES},
	{"EVENT",       DPU_EVENT},
	{"COMPOSER",    DPU_COMPOSER},
	{"ALL",         DPU_BASE|DPU_CPU_REG|DPU_CMDLIST|DPU_ISR|
			DPU_SYNC|DPU_COLOR|DPU_DSI|DPU_DP|
			DPU_PERF|DPU_DFX|DPU_PROPERTY|DPU_CLK|
			DPU_POWER|DPU_IDLE|DPU_FLOWCTRL|DPU_IOMMU|DPU_LAYER|DPU_RES|DPU_EVENT|DPU_COMPOSER},
};

u32 dpu_msg_value_get(enum dpu_log_cfg_type cfg_type, u8 *log_cfg)
{
	const struct dpu_log_map *log_map;
	u32 count;
	u32 value = 0;
	int i;

	switch (cfg_type) {
	case LOG_LEVEL:
		count = ARRAY_SIZE(dpu_msg_level_map);
		log_map = dpu_msg_level_map;
		break;
	case LOG_CATEGORY:
		count = ARRAY_SIZE(dpu_msg_category_map);
		log_map = dpu_msg_category_map;
		break;
	default:
		return value;
	}

	for (i = 0; i < count; ++i) {
		if (!strncmp(log_cfg, log_map[i].user_cfg, strlen(log_cfg) - 1)) {
			value = log_map[i].value;
			break;
		}
	}

	return value;
}

u8 *dpu_category_string_get(u32 category)
{
	u8 *string = NULL;
	u32 count;
	int i;

	count = ARRAY_SIZE(dpu_msg_category_map);

	for (i = 0; i < count; ++i) {
		if (category != dpu_msg_category_map[i].value)
			continue;
		else
			string = dpu_msg_category_map[i].user_cfg;
	}

	return string;
}

int dpu_msg_categorys_get(u8 *log_categories_string)
{
	u32 log_categories = 0;
	u8 *log_category_after_split;
	char *cur = kstrdup(log_categories_string, GFP_KERNEL);

	log_category_after_split = strsep(&cur, "|");

	while (log_category_after_split) {
		log_categories |= dpu_msg_value_get(LOG_CATEGORY, log_category_after_split);
		log_category_after_split = strsep(&cur, "|");
	}
	kfree(cur);
	return log_categories;
}
