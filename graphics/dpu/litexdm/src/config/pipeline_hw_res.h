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

#ifndef PIPELINE_HW_RES_H
#define PIPELINE_HW_RES_H

#include "osal.h"
#include "litexdm.h"

#define MAX_HW_RES_NUM 10

enum {
	CONNECTOR_NONE = -1,
	CONNECTOR_DSI0,
	CONNECTOR_DSI1,
	CONNECTOR_WB0,
	CONNECTOR_WB1,
	CONNECTOR_MAX,
};

enum {
	MIXER0,
	MIXER1,
	MIXER2,
	MIXER_CNT,
};

enum {
	ONLINE_SCENE,
	OFFLINE_SCENE,
};

struct pipeline_hw_res {
	uint32_t comp_id;
	uint32_t conn_id;
	uint32_t scene_type;
};

struct hw_list {
	uint32_t res_num;
	uint32_t res_list[MAX_HW_RES_NUM];
};

/**
 * get_support_conn_res - get all connector hw res
 *
 * @return struct hw_list
 */
struct hw_list get_support_conn_res(void);

/**
 * get_support_comp_res - get all composer hw res
 *
 * @return struct hw_list
 */
struct hw_list get_support_comp_res(void);

/**
 * is_offline_scene - is offline scene
 *
 * @return bool
 */
bool is_offline_scene(uint32_t scene_id);

/**
 * get_conn_id_from_scene_id - get connector id (logical id)
 *
 * @return uint32_t
 */
uint32_t get_conn_id_from_scene_id(uint32_t scene_id);

/**
 * is_dsi_connector - is dsi connector
 *
 * @return bool
 */
bool is_dsi_connector(uint32_t scene_id);

/**
 * is_wb_connector - is wb connector
 *
 * @return bool
 */
bool is_wb_connector(uint32_t scene_id);

/**
 * get_comp_id_from_scene_id - get composer id (logical id)
 *
 * @return uint32_t
 */
uint32_t get_comp_id_from_scene_id(uint32_t scene_id);

#endif