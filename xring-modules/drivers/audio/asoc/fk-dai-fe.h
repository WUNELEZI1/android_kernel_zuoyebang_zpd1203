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
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>

#define XR_MEDIA0 0x0 /*ULTRA FAST*/
#define XR_MEDIA1 0x1 /*FAST*/
#define XR_MEDIA2 0x2 /*NORMAL*/
#define XR_MEDIA3 0X3 /*NORMAL-1*/
#define XR_MEDIA4 0x4 /*COMPRESS CAPTURE*/
#define XR_MEDIA5 0x5 /*COMPRESS PLAYBACK*/
#define XR_MEDIA6 0x6 /*VOIP*/
#define XR_MEDIA7 0x7 /*HAPTIC*/
#define XR_VOICE  0x8 /*VOICE*/
#define XR_LSM    0xA /*VOICETRIGGER*/
#define XR_DIRECT 0xB /*DIRECT*/
#define XR_MMAP 0xC /*MUlIMEDIA*/
#define XR_SPATIAL  0xD /*SPATIAL*/
#define XR_KARAOKE  0xE /*KARAOKE*/
#define XR_INCALL 0xF /*INCALL*/
#define XR_HF 0x10 /*HF*/
#define XR_ASR   0x11 /*ASR*/

int get_pcm_session_id(struct snd_pcm_substream *substream);
int get_voip_session_id(struct snd_pcm_substream *substream);
int get_compr_session_id(struct snd_compr_stream *cstream);
int get_voice_session_id(struct snd_pcm_substream *substream);
int get_haptic_session_id(struct snd_pcm_substream *substream);
int get_spat_session_id(struct snd_pcm_substream *substream);
int get_mmap_session_id(struct snd_pcm_substream *substream);
int get_karaoke_session_id(struct snd_pcm_substream *substream);
int get_incall_session_id(struct snd_pcm_substream *substream);
int get_hf_session_id(struct snd_pcm_substream *substream);

int get_session_id(void *strm);

