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

#include "osal.h"
#include "pipeline_hw_res.h"
#include "dpu_log.h"

extern struct pipeline_hw_res g_pipeline_hw_res[SCENE_ID_MAX];

static bool is_hw_res_existed(struct hw_list hw_list_t, uint32_t hw_id)
{
	uint32_t i;

	for (i = 0; i < hw_list_t.res_num; i++) {
		if (hw_id == hw_list_t.res_list[i])
			return true;
	}
	return false;
}

struct hw_list get_support_conn_res(void)
{
	struct hw_list conn_hw_list = {0};
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(g_pipeline_hw_res); i++) {
		struct pipeline_hw_res hw_res = g_pipeline_hw_res[i];
		if (is_hw_res_existed(conn_hw_list, hw_res.conn_id))
			continue;
		conn_hw_list.res_list[conn_hw_list.res_num++] = hw_res.conn_id;
	}
	return conn_hw_list;
}

struct hw_list get_support_comp_res(void)
{
	struct hw_list comp_hw_list = {0};
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(g_pipeline_hw_res); i++) {
		struct pipeline_hw_res hw_res = g_pipeline_hw_res[i];
		if (is_hw_res_existed(comp_hw_list, hw_res.comp_id))
			continue;
		comp_hw_list.res_list[comp_hw_list.res_num++] = hw_res.comp_id;
	}
	return comp_hw_list;
}

uint32_t get_conn_id_from_scene_id(uint32_t scene_id)
{
	return g_pipeline_hw_res[scene_id].conn_id;
}

uint32_t get_comp_id_from_scene_id(uint32_t scene_id)
{
	return g_pipeline_hw_res[scene_id].comp_id;
}

bool is_dsi_connector(uint32_t scene_id)
{
	uint32_t conn_id = g_pipeline_hw_res[scene_id].conn_id;
	return ((conn_id == CONNECTOR_DSI0) || (conn_id == CONNECTOR_DSI1)) ? true : false;
}

bool is_wb_connector(uint32_t scene_id)
{
	uint32_t conn_id = g_pipeline_hw_res[scene_id].conn_id;
	return ((conn_id == CONNECTOR_WB0) || (conn_id == CONNECTOR_WB1)) ? true : false;
}

bool is_offline_scene(uint32_t scene_id)
{
	return g_pipeline_hw_res[scene_id].scene_type == OFFLINE_SCENE;
}
