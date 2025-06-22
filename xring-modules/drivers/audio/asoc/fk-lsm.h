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

#include "fk-pcm.h"
#include "../acore/fk-acore-api.h"
#include "../acore/fk-acore-define.h"

#define LISTEN_MIN_NUM_PERIODS    1
#define LISTEN_MAX_NUM_PERIODS    8
#define LISTEN_MAX_PERIOD_SIZE    1228800
#define LISTEN_MIN_PERIOD_SIZE    128

#define SNDRV_VA_LOAD_MODULE 0x1
#define SNDRV_VA_START_RECOGNITION 0x3
#define SNDRV_VA_STOP_RECOGNITION 0x4
#define SNDRV_VA_UNLOAD_MODULE 0x5
#define SNDRV_VA_WAIT_EVENT 0x6

enum {
	VT_CMD_INVALID = 0,
	VT_CMD_PORT_START,
	VT_CMD_PORT_SUSPEND,
	VT_CMD_PORT_RESUME,
	VT_CMD_PORT_STOP,
};

struct fk_audio_lsm {
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
	phys_addr_t pa;
	uint32_t size;

	uint8_t vt_flag;
	uint8_t fca_flag;
	uint8_t fca_event;
	uint8_t start_stop_rec;
	unsigned long count;
	struct mutex lsm_lock;
};

