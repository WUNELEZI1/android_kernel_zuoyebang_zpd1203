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

#ifndef _DP_AUDIO_H_
#define _DP_AUDIO_H_

#include "dp_hw_ctrl.h"

/**
 * struct dp_audio - to save information of audio driver
 * @cb: callback function of audio driver
 * @priv_data: private data of audio driver
 * @is_audio_playing: if audio is playing
 */
struct dp_audio {
	struct dp_audio_cb *cb;
	void *priv_data;

	bool is_audio_playing;
};

#endif /* _DP_AUDIO_H_ */
