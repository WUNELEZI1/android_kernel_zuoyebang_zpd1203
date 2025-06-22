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

#include <linux/types.h>
#include "dpu_log.h"
#include "dp_display.h"
#include "dp_audio.h"

int dp_get_audio_cta_blk(struct dp_audio_cta_blk *audio_blk)
{
	struct dp_display *display;

	display = dp_display_get(DP_DEVICE_ID_0);
	if (display == NULL) {
		DP_ERROR("dp device is not prepared yet\n");
		return -EBUSY;
	}

	if (audio_blk == NULL) {
		DP_ERROR("invalid parameter of audio_blk: NULL\n");
		return -EINVAL;
	}

	if (display->status.hpd != DP_HPD_EVENT_PLUG_IN) {
		DP_INFO("there is no displayport device connected\n");
		return -ENODEV;
	}

	memcpy(audio_blk, &display->panel->sink_caps.audio_blk,
			sizeof(struct dp_audio_cta_blk));

	return 0;
}
EXPORT_SYMBOL(dp_get_audio_cta_blk);

int dp_enable_audio(struct dp_audio_fmt_info *ainfo, bool enable)
{
	struct dp_display *display;

	display = dp_display_get(DP_DEVICE_ID_0);
	if (display == NULL) {
		DP_ERROR("dp device is not prepared yet\n");
		return -EBUSY;
	}

	if (enable && ainfo == NULL) {
		DP_ERROR("invalid parameter of ainfo: NULL\n");
		return -EINVAL;
	}

	if (!display->status.is_training_done) {
		DP_INFO("there is no displayport device prepared\n");
		return -ENODEV;
	}

	display->ctrl->funcs->enable_audio(display->ctrl, ainfo, enable);
	display->audio.is_audio_playing = enable;
	DP_INFO("dp audio info transfer is %s\n", enable ? "on" : "off");

	return 0;
}
EXPORT_SYMBOL(dp_enable_audio);

int dp_register_audio_cb(struct dp_audio_cb *cb, void *priv_data)
{
	struct dp_display *display;

	display = dp_display_get(DP_DEVICE_ID_0);
	if (display == NULL) {
		DP_ERROR("dp device is not prepared yet\n");
		return -EBUSY;
	}

	display->audio.cb = cb;
	display->audio.priv_data = priv_data;

	return 0;
}
EXPORT_SYMBOL(dp_register_audio_cb);
