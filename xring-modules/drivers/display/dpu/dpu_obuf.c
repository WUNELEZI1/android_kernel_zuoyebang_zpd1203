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

#include <linux/spinlock.h>
#include "dpu_obuf.h"
#include "dpu_log.h"

#define APPLY_OBUFEN(status, obufen) ((status) | (obufen))
#define RELEASE_OBUFEN(status, obufen) ((status) & ~(obufen))

/**
 * obufen_scene_table - store obufen value of each client in different scene
 *
 * obufen_scene_table[i][j] indicates the obufen value of client j in scene i.
 * bit i of obufen value indicates the possession of obuf i.
 *
 * eg: obufen_scene_table[1][DPU_OBUF_CLIENT_DP] store the obufen of CLIENT_DP
 * in scene of dsi single port and dp plug in. The value of 0b1100 indicates
 * DP should obtain obuf2 and obuf3.
 */
static const u32 obufen_scene_table[][DPU_OBUF_CLIENT_MAX] = {
	{0b1111, 0b0000, 0b0000}, /* single port, dp_plug_out */
	{0b0011, 0b0000, 0b1100}, /* single port, dp_plug_in */
	{0b0101, 0b1010, 0b0000}, /* dual port, dp_plug_out */
	{0b0001, 0b0010, 0b1100}, /* dual port, dp_plug_in */
};

static const char * const client_str[] = {
	"DPU_OBUF_CLIENT_DSI_0",
	"DPU_OBUF_CLIENT_DSI_1",
	"DPU_OBUF_CLIENT_DP",
	"DPU_OBUF_CLIENT_MAX",
};

/* record current obufen scene, default to be single port, dp_plug_out */
static const u32 *obufen_scene = &obufen_scene_table[0][0];

static enum dpu_obuf_scene_dsi_port cur_dsi_port = DPU_OBUF_SCENE_DSI_PORT_SINGLE;
static enum dpu_obuf_secne_dp_plug cur_dp_plug = DPU_OBUF_SCENE_DP_PLUG_OUT;

/* initial obufen status, set to be 0b0000, no client hold obuf */
static u32 obufen_status = 0b0000;
static DEFINE_SPINLOCK(obufen_status_lock);
static u32 obufen_client_hold[DPU_OBUF_CLIENT_MAX] = {0b0000, 0b0000, 0b0000};
static bool obufen_client_updated[DPU_OBUF_CLIENT_MAX] = {0};

/**
 * __dpu_obuf_check_apply - check if apply_obufen can be applied to current obufen_status
 * @client: obuf client
 * @apply_obufen: obufen try to be applied
 */
static inline bool __dpu_obuf_check_apply(enum dpu_obuf_client client, u32 apply_obufen)
{
	/* release holded obufen */
	u32 obufen_status_tmp = RELEASE_OBUFEN(obufen_status, obufen_client_hold[client]);

	/* if applied obufen is available in obufen_status_tmp */
	return ((obufen_status_tmp & apply_obufen) == 0);
}

/**
 * dpu_obuf_check_obufen - check if obufen of specified client need to be undated
 * @client: obuf client
 * @obufen_updated: if obufen is needed to be updated
 *
 * If obufen need not to be changed, function will return current obufen of client.
 * If obufen need to be changed, function will return updated obufen of client.
 */
u32 dpu_obuf_check_obufen(enum dpu_obuf_client client, bool *obufen_updated)
{
	u32 apply_obufen = obufen_scene[client];

	spin_lock(&obufen_status_lock);
	/* If obufen is already satisfied, it`s no needed to update obufen. */
	*obufen_updated = (apply_obufen == obufen_client_hold[client]) ?
			false : __dpu_obuf_check_apply(client, apply_obufen);
	spin_unlock(&obufen_status_lock);

	return *obufen_updated ? apply_obufen : obufen_client_hold[client];
}

/**
 * dpu_obuf_get_obufen - get hardware status of obufen of @client
 * @client: dpu obuf client
 *
 * obufen_client_hold status:
 *
 * timing:          ---- pre commit ------------- cfg clr ------------
 *
 *                       check_obufen()
 *                       tmg_obufen_config()
 * hardware status: ---- change     ------------- take effect --------
 *
 *                       update_obufen_cmt()      update_obufen_done()
 * software status: ----------------------------- change  ------------
 */
u32 dpu_obuf_get_obufen(enum dpu_obuf_client client)
{
	if (obufen_client_updated[client])
		return obufen_scene[client];
	else
		return obufen_client_hold[client];
}

void dpu_obuf_update_obufen_cmt(enum dpu_obuf_client client, bool updated)
{
	obufen_client_updated[client] = updated;
	if (updated) {
		spin_lock(&obufen_status_lock);
		DPU_DEBUG("obufen of %s can be updated: 0x%x, obufen status:0x%x\n",
				client_str[client], obufen_scene[client], obufen_status);
		spin_unlock(&obufen_status_lock);
	}
}

void dpu_obuf_update_obufen_done(enum dpu_obuf_client client)
{
	if (obufen_client_updated[client]) {
		spin_lock(&obufen_status_lock);

		/* release old obufen */
		obufen_status = RELEASE_OBUFEN(obufen_status, obufen_client_hold[client]);
		/* apply new obufen */
		obufen_status = APPLY_OBUFEN(obufen_status, obufen_scene[client]);

		DPU_DEBUG("obufen of %s is updated: 0x%x, obufen status:0x%0x\n",
				client_str[client], obufen_scene[client], obufen_status);

		spin_unlock(&obufen_status_lock);

		/* update obufen_client_hold */
		obufen_client_hold[client] = obufen_scene[client];
		/* reset obufen_updated */
		obufen_client_updated[client] = false;
	}
}

/**
 * dpu_obuf_scene_trigger_dsi_port - change obufen scene according to dsi port
 * @dsi_port: dsi port
 */
void dpu_obuf_scene_trigger_dsi_port(enum dpu_obuf_scene_dsi_port dsi_port)
{
	int i;

	obufen_scene = &obufen_scene_table[dsi_port * DPU_OBUF_SCENE_DP_PLUG_MAX + cur_dp_plug][0];
	cur_dsi_port = dsi_port;

	DPU_DEBUG("obufen scene is changed, dsi_port:%s\n",
			dsi_port == 0 ? "single" : "dual");
	for (i = 0; i < DPU_OBUF_CLIENT_MAX; i++)
		DPU_DEBUG("client: %s, expected obufen: 0x%x\n", client_str[i], obufen_scene[i]);
}

/**
 * dpu_obuf_scene_trigger_dp_plug - change obufen scene according to dp plug
 * @dp_plug: dp plug
 */
void dpu_obuf_scene_trigger_dp_plug(enum dpu_obuf_secne_dp_plug dp_plug)
{
	int i;

	obufen_scene = &obufen_scene_table[cur_dsi_port * DPU_OBUF_SCENE_DP_PLUG_MAX + dp_plug][0];
	cur_dp_plug = dp_plug;

	DPU_DEBUG("obufen scene is changed, dp_plug: %s\n",
			dp_plug == 0 ? "plug out" : "plug in");
	for (i = 0; i < DPU_OBUF_CLIENT_MAX; i++)
		DPU_DEBUG("client: %s, expected obufen: 0x%x\n", client_str[i], obufen_scene[i]);
}
