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

#include "pipeline_hw_res.h"

/*
 * This includes only the master connector, which is still SCENE_ID_0 in the case of dual mipi
 */
struct pipeline_hw_res g_pipeline_hw_res[SCENE_ID_MAX] = {
	{MIXER0, CONNECTOR_DSI0, ONLINE_SCENE},
	{MIXER0, CONNECTOR_DSI1, ONLINE_SCENE},
	{MIXER1, CONNECTOR_DSI0, ONLINE_SCENE},
	{MIXER1, CONNECTOR_DSI1, ONLINE_SCENE},
	{MIXER1, CONNECTOR_WB0, OFFLINE_SCENE},
	{MIXER2, CONNECTOR_WB1, OFFLINE_SCENE},
};
