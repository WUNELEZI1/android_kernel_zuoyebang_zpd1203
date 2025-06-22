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
#include "../acore/fk-acore-api.h"
#include "../acore/fk-acore-define.h"

#define COMPR_RECORD_MIN_FRAGMENT_SIZE (1024)
#define COMPR_RECORD_MAX_FRAGMENT_SIZE (1024)
#define COMPR_RECORD_MIN_NUM_FRAGMENTS 2
#define COMPR_RECORD_MAX_NUM_FRAGMENTS 10
#define MAX_NUM_SAMPLE_RATE 20

struct support_format {
	uint32_t codec_id;
	uint32_t ch;
	uint32_t num_sample_rate;
	uint32_t sample_rate[MAX_NUM_SAMPLE_RATE];
};

struct block_info {
	/* virtual address of block memory */
	void *bk_vir_addr;
	/* physical address of block memory */
	phys_addr_t bk_p_addr;

	uint16_t bk_num;
	uint32_t size;
	uint32_t byte_avail;
	uint32_t byte_copied;
};

struct fk_compr_audio {
	struct snd_compr_stream *stream;
	struct snd_compr_caps compr_cap;
	struct snd_compr_params codec_param;

	uint32_t codec;
	uint32_t buffer_size;
	int32_t first_buffer;
	int32_t last_buffer;

	uint16_t session_id;/*pcm session id*/
	uint32_t samp_rate;
	uint32_t num_channels;
	uint32_t bits;
	uint32_t copied_total;
	uint32_t fragments;
	uint32_t pos;
	uint32_t index;

	uint64_t marker_timestamp;

	/* parameter for captrue*/
	uint64_t received_total; /* bytes received from DSP */
	uint64_t bytes_copied; /* to userspace */
	uint16_t bk_read;
	uint16_t bk_write;
	struct block_info cap_block_info[COMPR_RECORD_MAX_NUM_FRAGMENTS];

	/*params for playback*/
	uint32_t app_pointer; /* offset */
	void *buffer;

	spinlock_t lock;

	struct audio_memory_dma_buf *memory_priv;

	int volume;

	struct adsp_ssr_action *capture_ssr;
};

int fk_compr_new(struct snd_soc_pcm_runtime *rtd, int num);
