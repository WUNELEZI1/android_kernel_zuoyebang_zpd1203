// SPDX-License-Identifier: GPL-2.0-only
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

#include <linux/kernel.h>
#include <linux/module.h>
#include "platform_init.h"

int __init audio_platform_init(void)
{
	fk_fe_dai_init();
	fk_be_dai_init();
	fk_soc_routing_platform_init();
	fk_pcm_dsp_init();
	fk_voice_dsp_init();
	fk_loopback_dsp_init();
	fk_compress_dsp_init();
	fk_voip_dsp_init();
	fk_haptic_init();
	fk_spat_init();
	fk_karaoke_init();
	fk_hf_dsp_init();
	fk_lsm_init();
	fk_mmap_dsp_init();
	fk_incall_dsp_init();
	fk_audio_usb_init();
	fk_asoc_machine_init();

	return 0;
}

void audio_platform_exit(void)
{
	fk_fe_dai_exit();
	fk_be_dai_exit();
	fk_soc_routing_platform_exit();
	fk_pcm_dsp_exit();
	fk_voice_dsp_exit();
	fk_loopback_dsp_exit();
	fk_compress_dsp_exit();
	fk_voip_dsp_exit();
	fk_haptic_exit();
	fk_spat_exit();
	fk_karaoke_exit();
	fk_hf_dsp_exit();
	fk_lsm_exit();
	fk_mmap_dsp_exit();
	fk_incall_dsp_exit();
	fk_audio_usb_exit();
	fk_asoc_machine_exit();
}

