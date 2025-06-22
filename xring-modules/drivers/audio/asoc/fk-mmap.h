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

#define MMAP_PLAYBACK_MIN_NUM_PERIODS    2
#define MMAP_PLAYBACK_MAX_NUM_PERIODS    1024
#define MMAP_PLAYBACK_MAX_PERIOD_SIZE    122880
#define MMAP_PLAYBACK_MIN_PERIOD_SIZE    16
#define MMAP_CAPTURE_MIN_NUM_PERIODS     2
#define MMAP_CAPTURE_MAX_NUM_PERIODS     1024
#define MMAP_CAPTURE_MAX_PERIOD_SIZE     122880
#define MMAP_CAPTURE_MIN_PERIOD_SIZE     16

#define MMAP_MEMORY_ELEMENT_NUM 2

struct share_mem_info {
	uint32_t fd;
	uint32_t size;
};

struct fk_mmap {
	struct snd_pcm_substream *substream;
	unsigned int pcm_size;
	unsigned int pcm_count;
	unsigned int pcm_irq_pos;
	uint16_t source;

	uint16_t session_id;
	uint32_t samp_rate;
	uint32_t channel_mode;
	struct audio_memory_dma_buf *memory_priv;

	int volume;
	struct share_mem_info mem_info;
	dma_addr_t addr;

	bool open_status;
};

