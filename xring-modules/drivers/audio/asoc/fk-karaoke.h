/* SPDX-License-Identifier: GPL-2.0-only */
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
#ifndef _FK_KARAOKE_
#define _FK_KARAOKE_

#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/timer.h>
#include <asm/dma.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/of_device.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>

#include "fk-dai-fe.h"
#include "../acore/fk-acore-api.h"

struct fk_karaoke {
	struct snd_pcm_substream *playback_substream;
	struct snd_pcm_substream *capture_substream;

	int play_samples;
	int play_channels;
	int play_bits;

	int cap_samples;
	int cap_channels;
	int cap_bits;

	int playback_prepare;
	int capture_prepare;

	int playback_start;
	int capture_start;

	uint16_t playback_session_id;
	uint16_t capture_session_id;
};

#endif //_FK_KARAOKE_
