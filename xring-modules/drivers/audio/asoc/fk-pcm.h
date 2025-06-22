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

#define PLAYBACK_MIN_NUM_PERIODS    2
#define PLAYBACK_MAX_NUM_PERIODS    16
#define PLAYBACK_MAX_PERIOD_SIZE    122880
#define PLAYBACK_MIN_PERIOD_SIZE    16
#define CAPTURE_MIN_NUM_PERIODS     2
#define CAPTURE_MAX_NUM_PERIODS     16
#define CAPTURE_MAX_PERIOD_SIZE     122880
#define CAPTURE_MIN_PERIOD_SIZE     16

// #define ASOC_LOG_ERR(fmt, args...)		pr_err("[asoc]:%s:" fmt "\n", __func__, ##args)
// #define ASOC_LOG_INFO(fmt, args...)		pr_info("[asoc]:%s:" fmt "\n", __func__, ##args)
// #define ASOC_LOG_DBG(fmt, args...)		pr_info("[asoc]:%s:" fmt "\n", __func__, ##args)
enum {
	NORMAL_PCM_MODE = 0,
	FAST_PCM_MODE,
	ULTRA_FAST_PCM_MODE,
};

struct fk_audio {
	struct snd_pcm_substream *substream;
	unsigned int pcm_size;
	unsigned int pcm_count;
	unsigned int pcm_irq_pos;       /* IRQ position */
	uint16_t source; /* Encoding source bit mask */

//	uint8_t fe_dai_id;/*pcm fe dai id*/
//	uint8_t pcm_scene_type;/*scene type id*/
	uint16_t session_id;/*pcm session id*/
	uint32_t samp_rate;
	uint32_t channel_mode;
	struct audio_memory_dma_buf *memory_priv;
};

struct fk_plat_data {
	int pcm_mode;
	struct snd_pcm *pcm;
	struct mutex lock;
};
