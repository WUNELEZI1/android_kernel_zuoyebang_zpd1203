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

#ifndef _DPU_OBUF_H_
#define _DPU_OBUF_H_

#include <linux/types.h>

enum dpu_obuf_scene_dsi_port {
	DPU_OBUF_SCENE_DSI_PORT_SINGLE = 0,
	DPU_OBUF_SCENE_DSI_PORT_DUAL,
	DPU_OBUF_SCENE_DSI_PORT_MAX,
};

enum dpu_obuf_secne_dp_plug {
	DPU_OBUF_SCENE_DP_PLUG_OUT = 0,
	DPU_OBUF_SCENE_DP_PLUG_IN,
	DPU_OBUF_SCENE_DP_PLUG_MAX,
};

enum dpu_obuf_client {
	DPU_OBUF_CLIENT_DSI_0 = 0,
	DPU_OBUF_CLIENT_DSI_1,
	DPU_OBUF_CLIENT_DP,
	DPU_OBUF_CLIENT_MAX,
};

u32 dpu_obuf_check_obufen(enum dpu_obuf_client client, bool *obufen_updated);
u32 dpu_obuf_get_obufen(enum dpu_obuf_client client);

void dpu_obuf_update_obufen_cmt(enum dpu_obuf_client client, bool updated);
void dpu_obuf_update_obufen_done(enum dpu_obuf_client client);

void dpu_obuf_scene_trigger_dsi_port(enum dpu_obuf_scene_dsi_port scene);
void dpu_obuf_scene_trigger_dp_plug(enum dpu_obuf_secne_dp_plug scene);

#endif /* _DPU_OBUF_H_ */
