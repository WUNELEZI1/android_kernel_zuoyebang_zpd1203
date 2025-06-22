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

#ifndef __DP_AUDIO_INTERFACE_H__
#define __DP_AUDIO_INTERFACE_H__

#include <linux/types.h>

#define CTA_AUDIO_DATA_BLOCK_SIZE                                 32
#define CTA_SPEAKER_ALLOCATION_DATA_BLOCK_SIZE                    4

struct dp_audio_cta_blk {
	/* if sink supports Basic Audio. */
	bool basic_audio_supported;

	/* if audio_data_blk is valid. */
	bool audio_data_blk_valid;

	/**
	 * audio data block in EDID CTA extension block, data is composed following
	 * CTA-861-G. Table 53 General Format of "CTA Data Block Collection" and
	 * chapter 7.5.2 Audio Data Block.
	 *
	 * audio_data_blk[0] - CTA audio data block header
	 *     bit 7:5 = audio tag code
	 *     bit 4:0 = length of audio data block payload data
	 * audio_data_blk[1, 2, ...] - CTA audio data block payload data
	 */
	u8 audio_data_blk[CTA_AUDIO_DATA_BLOCK_SIZE];

	/* if spk_alloc_data_blk is valid. */
	bool spk_alloc_data_blk_valid;

	/**
	 * speaker allocation data block in EDID CTA extension block, data is
	 * composed following CTA-861-G. Table 53 General Format of "CTA Data Block
	 * Collection" and chapter 7.5.3 Speaker Allocation Data Block.
	 *
	 * spk_alloc_data_blk[0] - CTA speaker allocation data block header
	 *     bit 7:5 = audio tag code
	 *     bit 4:0 = length of speaker allocation data block payload data
	 * audio_data_blk[1, 2, 3] - CTA speaker allocation data block payload data
	 */
	u8 spk_alloc_data_blk[CTA_SPEAKER_ALLOCATION_DATA_BLOCK_SIZE];
};

struct dp_audio_cb {
	int (*audio_notify)(void *priv_data, bool plugged);
};

/**
 * struct dp_audio_fmt_info - audio format information
 * @samp_rate: sample rate
 * @bit_width: bit width
 * @ch_num: channel number
 */
struct dp_audio_fmt_info {
	u32 samp_rate;
	u16 bit_width;
	u16 ch_num;
};

/**
 * dp_get_audio_cta_blk - get audio data block in EDID CTA extension block
 * @audio_blk: audio data block
 */
int dp_get_audio_cta_blk(struct dp_audio_cta_blk *audio_blk);

/**
 * dp_enable_audio - enable audio transfer
 * @info: audio format information, could be NULL if @enable is false
 * @enable: true or false
 */
int dp_enable_audio(struct dp_audio_fmt_info *info, bool enable);

/**
 * dp_register_audio_cb - register callback of audio driver
 * @cb: callback function of audio driver
 * @priv_data: private data of audio driver, will be passed to callback function
 */
int dp_register_audio_cb(struct dp_audio_cb *cb, void *priv_data);

#endif /* __DP_AUDIO_INTERFACE_H__ */
