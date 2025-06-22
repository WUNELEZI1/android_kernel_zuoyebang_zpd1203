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

int fk_fe_dai_init(void);
int fk_be_dai_init(void);
int fk_soc_routing_platform_init(void);
int fk_pcm_dsp_init(void);
int fk_voice_dsp_init(void);
int fk_loopback_dsp_init(void);
int fk_compress_dsp_init(void);
int fk_voip_dsp_init(void);
int fk_haptic_init(void);
int fk_spat_init(void);
int fk_karaoke_init(void);
int fk_hf_dsp_init(void);
int fk_lsm_init(void);
int fk_mmap_dsp_init(void);
int fk_incall_dsp_init(void);
int fk_audio_usb_init(void);
int fk_asoc_machine_init(void);
int audio_platform_init(void);

void fk_fe_dai_exit(void);
void fk_be_dai_exit(void);
void fk_soc_routing_platform_exit(void);
void fk_pcm_dsp_exit(void);
void fk_voice_dsp_exit(void);
void fk_loopback_dsp_exit(void);
void fk_compress_dsp_exit(void);
void fk_voip_dsp_exit(void);
void fk_haptic_exit(void);
void fk_spat_exit(void);
void fk_karaoke_exit(void);
void fk_hf_dsp_exit(void);
void fk_lsm_exit(void);
void fk_mmap_dsp_exit(void);
void fk_incall_dsp_exit(void);
void fk_audio_usb_exit(void);
void fk_asoc_machine_exit(void);
void audio_platform_exit(void);

