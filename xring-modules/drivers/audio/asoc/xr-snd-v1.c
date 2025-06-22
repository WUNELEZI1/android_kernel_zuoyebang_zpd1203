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

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/info.h>
#include <sound/soc-dai.h>

#include "fk-pcm-routing.h"
#include "fk_dailink.h"

#include "../common/fk-audio-log.h"

#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#endif

#define DRV_NAME "asoc-snd"
#define __CHIPSET__ "xr"
#define FK_DAILINK_NAME(name) (__CHIPSET__#name)

struct fk_asoc_machine_data {
	u32 codec_fmt;
};

static struct snd_soc_card xr_snd_card;

static int fk_be_hw_params_fixup(struct snd_soc_pcm_runtime *rtd,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_dai_link *dai_link = rtd->dai_link;
	struct snd_interval *rate = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_RATE);
	struct snd_interval *channels = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_CHANNELS);
	struct snd_mask *fmt = hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);
	struct dai_format *aif_cfg = NULL;

	AUD_LOG_INFO(AUD_SOC, "dai_id= %d, format = %d, rate = %d",
		dai_link->id, params_format(params), params_rate(params));

	aif_cfg = fk_aif_dev_cfg_get(dai_link->id);
	if (aif_cfg == NULL) {
		AUD_LOG_INFO(AUD_SOC, "dai id (%d), use default value", dai_link->id);
		rate->min = rate->max = PORT_RATE_48KHZ;
		channels->min = channels->max = PORT_CHANNELS_2;
		snd_mask_set_format(fmt, SNDRV_PCM_FORMAT_S16_LE);
	} else {
		rate->min = rate->max = aif_cfg->samples;
		channels->min = channels->max = aif_cfg->channels;
		if (aif_cfg->bit_width == PORT_BITS_16)
			snd_mask_set_format(fmt, SNDRV_PCM_FORMAT_S16_LE);
		else if (aif_cfg->bit_width == PORT_BITS_24)
			snd_mask_set_format(fmt, SNDRV_PCM_FORMAT_S24_LE);
		else if (aif_cfg->bit_width == PORT_BITS_32)
			snd_mask_set_format(fmt, SNDRV_PCM_FORMAT_S32_LE);
	}

	return 0;
}

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link fk_common_dai_links[] = {
	/* FrontEnd DAI Links */
	/*ultra fast*/
	{/* hw:x,0 */
		.name = FK_DAILINK_NAME(Media0),
		.stream_name = "XR_media0",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA0,
		SND_SOC_DAILINK_REG(xr_media0),
	},
	/*fast*/
	{
		.name = FK_DAILINK_NAME(Media1),
		.stream_name = "XR_media1",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA1,
		SND_SOC_DAILINK_REG(xr_media1),
	},
	/*normal*/
	{
		.name = FK_DAILINK_NAME(Media2),
		.stream_name = "XR_media2",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA2,
		SND_SOC_DAILINK_REG(xr_media2),
	},
	{
		.name = FK_DAILINK_NAME(Media3),
		.stream_name = "XR_media3",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA3,
		SND_SOC_DAILINK_REG(xr_media3),
	},
	/*compress capture*/
	{
		.name = FK_DAILINK_NAME(compress_capture),
		.stream_name = "compress",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
				SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA4,
		SND_SOC_DAILINK_REG(xr_media4),
	},
	/*compress playback*/
	{
		.name = FK_DAILINK_NAME(compress_playback),
		.stream_name = "compress1",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
				SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA5,
		SND_SOC_DAILINK_REG(xr_media5),
	},

	/* Voice Call */
	{
		.name = FK_DAILINK_NAME(Voice),
		.stream_name = "XR_voice",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_VOICE,
		SND_SOC_DAILINK_REG(xr_voice),
	},
	/* DSP Loopback */
	{
		.name = FK_DAILINK_NAME(Loopback),
		.stream_name = "XR_Loopback",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_LOOPBACK,
		SND_SOC_DAILINK_REG(xr_loopback),
	},

	/*voip*/
	{
		.name = FK_DAILINK_NAME(Media6),
		.stream_name = "XR_media6",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA6,
		SND_SOC_DAILINK_REG(xr_media6),
	},

	/*haptic*/
	{
		.name = FK_DAILINK_NAME(Haptic),
		.stream_name = "XR_media7",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MEDIA7,
		SND_SOC_DAILINK_REG(xr_media7),
	},

	/*voice trigger*/
	{
		.name = FK_DAILINK_NAME(voice_trigger),
		.stream_name = "XR_lsm",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
				SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_LSM,
		SND_SOC_DAILINK_REG(xr_lsm),
	},

	/*DIRECT*/
	{
		.name = FK_DAILINK_NAME(multichannels),
		.stream_name = "XR_direct",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MULTICHS,
		SND_SOC_DAILINK_REG(xr_multichs),
	},

	/*SPATIAL*/
	{
		.name = FK_DAILINK_NAME(Spatial),
		.stream_name = "XR_spatial",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MULTICHS,
		SND_SOC_DAILINK_REG(xr_spatial),
	},

	/*MMAP*/
	{
		.name = FK_DAILINK_NAME(mmap),
		.stream_name = "XR_mmap",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_MMAP,
		SND_SOC_DAILINK_REG(xr_mmap),
	},

	/*Karaoke*/
	{
		.name = FK_DAILINK_NAME(Karaoke),
		.stream_name = "XR_Karaoke",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_capture = 1,
		.dpcm_playback = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_KARAOKE,
		SND_SOC_DAILINK_REG(xr_karaoke),
	},

	/*INCALL*/
	{
		.name = FK_DAILINK_NAME(incall),
		.stream_name = "XR_incall",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_INCALL,
		SND_SOC_DAILINK_REG(xr_incall),
	},
	/*HF*/
	{
		.name = FK_DAILINK_NAME(hf),
		.stream_name = "XR_hf",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_HF,
		SND_SOC_DAILINK_REG(xr_hf),
	},
	/*ASR*/
	{
		.name = FK_DAILINK_NAME(asr),
		.stream_name = "XR_asr",
		.dynamic = 1,
		.nonatomic = 1,
		.dpcm_capture = 1,
		.trigger = {SND_SOC_DPCM_TRIGGER_POST,
				SND_SOC_DPCM_TRIGGER_POST},
		.ignore_suspend = 1,
		/* this dainlink has playback support */
		.ignore_pmdown_time = 1,
		.id = FK_FRONTEND_DAI_XR_ASR,
		SND_SOC_DAILINK_REG(xr_asr),
	},
};

static struct snd_soc_dai_link fk_i2s_be_dai_links[] = {
	{
		.name = "XR_I2S0_RX",
		.stream_name = "XR_I2S0 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S0_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s0_rx),
	},
	{
		.name = "XR_I2S0_TX",
		.stream_name = "XR_I2S0 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S0_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s0_tx),
	},
	{
		.name = "XR_I2S1_RX",
		.stream_name = "XR_I2S1 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S1_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s1_rx),
	},
	{
		.name = "XR_I2S1_TX",
		.stream_name = "XR_I2S1 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S1_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s1_tx),
	},
	{
		.name = "XR_I2S2_RX",
		.stream_name = "XR_I2S2 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S2_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 5,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s2_rx),
	},
	{
		.name = "XR_I2S2_TX",
		.stream_name = "XR_I2S2 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S2_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 3,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s2_tx),
	},
	{
		.name = "XR_I2S3_RX",
		.stream_name = "XR_I2S3 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S3_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s3_rx),
	},
	{
		.name = "XR_I2S3_TX",
		.stream_name = "XR_I2S3 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S3_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s3_tx),
	},
	{
		.name = "XR_I2S4_RX",
		.stream_name = "XR_I2S4 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S4_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 1,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s4_rx),
	},
	{
		.name = "XR_I2S4_TX",
		.stream_name = "XR_I2S4 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S4_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 1,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s4_tx),
	},
	{
		.name = "XR_I2S9_TX",
		.stream_name = "XR_I2S9 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S9_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s9_tx),
	},
#ifdef AUDIO_ROUTES_RESERVE
	{
		.name = "XR_I2S5_RX",
		.stream_name = "XR_I2S5 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S5_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s5_rx),
	},
	{
		.name = "XR_I2S5_TX",
		.stream_name = "XR_I2S5 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S5_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s5_tx),
	},
	{
		.name = "XR_I2S6_RX",
		.stream_name = "XR_I2S6 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S6_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s6_rx),
	},
	{
		.name = "XR_I2S6_TX",
		.stream_name = "XR_I2S6 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S6_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s6_tx),
	},
	{
		.name = "XR_I2S7_RX",
		.stream_name = "XR_I2S7 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S7_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s7_rx),
	},
	{
		.name = "XR_I2S7_TX",
		.stream_name = "XR_I2S7 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S7_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s7_tx),
	},
	{
		.name = "XR_I2S8_RX",
		.stream_name = "XR_I2S8 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S8_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s8_rx),
	},
	{
		.name = "XR_I2S8_TX",
		.stream_name = "XR_I2S8 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_I2S8_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_i2s8_tx),
	},
	{
		.name = "XR_I2S9_RX",
		.stream_name = "XR_I2S9 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_I2S9_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_i2s9_rx),
	},
#endif
};

static struct snd_soc_dai_link fk_vad_i2s_be_dai_links[] = {
	{
		.name = "XR_VAD_I2S_TX",
		.stream_name = "XR_VAD_I2S Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.num_codecs = 1,
		.id = FK_XR_VAD_I2S_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_vad_i2s_tx),
	},
};

static struct snd_soc_dai_link fk_mi2s0_be_dai_links[] = {
	{
		.name = "XR_MI2S0_RX",
		.stream_name = "XR_MI2S0 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_MI2S0_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_mi2s0_rx),
	},
};

static struct snd_soc_dai_link fk_tdm_be_dai_links[] = {
#ifdef AUDIO_ROUTES_RESERVE
	{
		.name = "XR_TDM0_RX",
		.stream_name = "XR_TDM0 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM0_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm0_rx),
	},
	{
		.name = "XR_TDM0_TX",
		.stream_name = "XR_TDM0 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM0_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm0_tx),
	},
	{
		.name = "XR_TDM1_RX",
		.stream_name = "XR_TDM1 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM1_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm1_rx),
	},
	{
		.name = "XR_TDM1_TX",
		.stream_name = "XR_TDM1 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM1_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm1_tx),
	},

	{
		.name = "XR_TDM2_RX",
		.stream_name = "XR_TDM2 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM2_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 3,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm2_rx),
	},
	{
		.name = "XR_TDM2_TX",
		.stream_name = "XR_TDM2 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM2_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 3,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm2_tx),
	},
	{
		.name = "XR_TDM3_RX",
		.stream_name = "XR_TDM3 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM3_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm3_rx),
	},
	{
		.name = "XR_TDM3_TX",
		.stream_name = "XR_TDM3 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM3_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm3_tx),
	},
	{
		.name = "XR_TDM4_RX",
		.stream_name = "XR_TDM4 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM4_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm4_rx),
	},
	{
		.name = "XR_TDM4_TX",
		.stream_name = "XR_TDM4 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM4_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm4_tx),
	},
	{
		.name = "XR_TDM5_RX",
		.stream_name = "XR_TDM5 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM5_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm5_rx),
	},
	{
		.name = "XR_TDM5_TX",
		.stream_name = "XR_TDM5 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM5_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm5_tx),
	},
	{
		.name = "XR_TDM6_RX",
		.stream_name = "XR_TDM6 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM6_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm6_rx),
	},
	{
		.name = "XR_TDM6_TX",
		.stream_name = "XR_TDM6 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM6_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm6_tx),
	},
#endif
	{
		.name = "XR_TDM7_RX",
		.stream_name = "XR_TDM7 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM7_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 14,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm7_rx),
	},
	{
		.name = "XR_TDM7_TX",
		.stream_name = "XR_TDM7 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM7_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm7_tx),
	},
	{
		.name = "XR_TDM8_RX",
		.stream_name = "XR_TDM8 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM8_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm8_rx),
	},
	{
		.name = "XR_TDM8_TX",
		.stream_name = "XR_TDM8 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM8_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm8_tx),
	},
	{
		.name = "XR_TDM9_RX",
		.stream_name = "XR_TDM9 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM9_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm9_rx),
	},
	{
		.name = "XR_TDM9_TX",
		.stream_name = "XR_TDM9 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM9_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 0,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm9_tx),
	},
	{
		.name = "XR_TDM7_8_RX",
		.stream_name = "XR_TDM7_8 Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_TDM7_8_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 10,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_tdm7_8_rx),
	},
	{
		.name = "XR_TDM7_8_TX",
		.stream_name = "XR_TDM7_8 Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_TDM7_8_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.num_codecs = 10,
		.ignore_suspend = 1,
		SND_SOC_DAILINK_REG(xr_tdm7_8_tx),
	},
};

static struct snd_soc_dai_link fk_usb_be_dai_links[] = {
	{
		.name = "XR_AUDIO_USB_TX",
		.stream_name = "XR_AUDIO_USB Capture",
		.no_pcm = 1,
		.dpcm_capture = 1,
		.id = FK_XR_AUDIO_USB_TX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_audio_usb_tx),
	},

	{
		.name = "XR_AUDIO_USB_RX",
		.stream_name = "XR_AUDIO_USB Playback",
		.no_pcm = 1,
		.dpcm_playback = 1,
		.id = FK_XR_AUDIO_USB_RX,
		.be_hw_params_fixup = fk_be_hw_params_fixup,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		SND_SOC_DAILINK_REG(xr_audio_usb_rx),
	},
};

static struct snd_soc_dai_link xr_snd_dai_links[
			ARRAY_SIZE(fk_common_dai_links) +
			ARRAY_SIZE(fk_i2s_be_dai_links) +
			ARRAY_SIZE(fk_vad_i2s_be_dai_links) +
			ARRAY_SIZE(fk_mi2s0_be_dai_links) +
			ARRAY_SIZE(fk_tdm_be_dai_links) +
			ARRAY_SIZE(fk_usb_be_dai_links)];

static int fk_populate_dai_link_component_of_node(
					struct snd_soc_card *card)
{
	int i, j, index, ret = 0;
	struct device *cdev = card->dev;
	struct snd_soc_dai_link *dai_link = card->dai_link;
	struct device_node *np = NULL;
	int codecs_enabled = 0;
	struct snd_soc_dai_link_component *codecs_comp = NULL;

	if (!cdev) {
		AUD_LOG_ERR(AUD_SOC, "Sound card device memory NULL");
		return -ENODEV;
	}

	for (i = 0; i < card->num_links; i++) {
		if (dai_link[i].platforms->of_node && dai_link[i].cpus->of_node)
			continue;

		/* populate platform_of_node for snd card dai links */
		if (dai_link[i].platforms->name &&
		    !dai_link[i].platforms->of_node) {
			index = of_property_match_string(cdev->of_node,
						"asoc-platform-names",
						dai_link[i].platforms->name);
			if (index < 0) {
				AUD_LOG_ERR(AUD_SOC, "No match found for platform name: %s",
					dai_link[i].platforms->name);
				ret = index;
				goto err;
			}
			np = of_parse_phandle(cdev->of_node, "asoc-platform",
					      index);
			if (!np) {
				AUD_LOG_ERR(AUD_SOC,
					"retrieving phandle for platform %s, index %d failed",
					dai_link[i].platforms->name, index);
				ret = -ENODEV;
				goto err;
			}
			dai_link[i].platforms->of_node = np;
			dai_link[i].platforms->name = NULL;
		}

		/* populate cpu_of_node for snd card dai links */
		if (dai_link[i].cpus->dai_name && !dai_link[i].cpus->of_node) {
			index = of_property_match_string(cdev->of_node,
						 "asoc-cpu-names",
						 dai_link[i].cpus->dai_name);
			if (index >= 0) {
				np = of_parse_phandle(cdev->of_node, "asoc-cpu",
						index);
				if (!np) {
					AUD_LOG_ERR(AUD_SOC,
						"retrieving phandle for cpu dai %s failed",
						dai_link[i].cpus->dai_name);
					ret = -ENODEV;
					goto err;
				}
				dai_link[i].cpus->of_node = np;
				dai_link[i].cpus->dai_name = NULL;
			}
		}

		/* populate codec_of_node for snd card dai links */
		if (dai_link[i].num_codecs > 0) {
			for (j = 0; j < dai_link[i].num_codecs; j++) {
				if (dai_link[i].codecs[j].of_node ||
						!dai_link[i].codecs[j].name)
					continue;

				index = of_property_match_string(cdev->of_node,
						"asoc-codec-names",
						dai_link[i].codecs[j].name);
				if (index < 0)
					continue;
				np = of_parse_phandle(cdev->of_node,
						      "asoc-codec",
						      index);
				if (!np) {
					AUD_LOG_ERR(AUD_SOC,
						"retrieving phandle for codec %s failed",
						dai_link[i].codecs[j].name);
					ret = -ENODEV;
					goto err;
				}
				dai_link[i].codecs[j].of_node = np;
				dai_link[i].codecs[j].name = NULL;
			}
		}
	}

	/* In multi-codec scenario, check if codecs are enabled for this platform */
	for (i = 0; i < card->num_links; i++) {
		codecs_enabled = 0;
		if (dai_link[i].num_codecs > 1) {
			for (j = 0; j < dai_link[i].num_codecs; j++) {
				if (!dai_link[i].codecs[j].of_node)
					continue;

				np = dai_link[i].codecs[j].of_node;
				if (!of_device_is_available(np)) {
					dai_link[i].codecs[j].of_node = NULL;
					continue;
				}
				codecs_enabled++;
			}
			if (codecs_enabled > 0 &&
					codecs_enabled < dai_link[i].num_codecs) {
				codecs_comp = devm_kzalloc(cdev,
					sizeof(struct snd_soc_dai_link_component)
					* codecs_enabled, GFP_KERNEL);
				if (!codecs_comp) {
					AUD_LOG_ERR(AUD_SOC,
						"%s dailink codec component alloc failed",
						dai_link[i].name);
					ret = -ENOMEM;
					goto err;
				}
				index = 0;
				for (j = 0; j < dai_link[i].num_codecs; j++) {
					if (dai_link[i].codecs[j].of_node) {
						codecs_comp[index].of_node =
						  dai_link[i].codecs[j].of_node;
						codecs_comp[index].dai_name =
						  dai_link[i].codecs[j].dai_name;
						codecs_comp[index].name = NULL;
						index++;
					}
				}
				dai_link[i].codecs = codecs_comp;
				dai_link[i].num_codecs = codecs_enabled;
			}
		}
	}

err:
	return ret;
}

static const struct of_device_id xr_asoc_snd_machine_of_match[] = {
	{ .compatible = "xring,xr-asoc-snd"},
	{},
};

static struct snd_soc_card *populate_snd_card_dailinks(struct device *dev)
{
	struct snd_soc_card *card = NULL;
	struct snd_soc_dai_link *dailink = NULL;
	int total_links = 0;
	int ret = 0;
	u32 i2s_audio_intf = 0, vad_i2s_intf = 0, tdm_audio_intf = 0;
	u32 audio_usb_intf = 0;
	u32 mi2s0_intf = 0;
	const struct of_device_id *match;

	match = of_match_node(xr_asoc_snd_machine_of_match, dev->of_node);
	if (!match) {
		AUD_LOG_ERR(AUD_SOC, "No DT match found for sound card");
		return NULL;
	}

	card = &xr_snd_card;

	memcpy(xr_snd_dai_links + total_links,
		    fk_common_dai_links,
		    sizeof(fk_common_dai_links));
	total_links += ARRAY_SIZE(fk_common_dai_links);

	ret = of_property_read_u32(dev->of_node, "xring,i2s-audio-intf",
					  &i2s_audio_intf);
	if (ret)
		AUD_LOG_INFO(AUD_SOC, "No DT match i2s audio interface");
	else {
		if (i2s_audio_intf) {
			memcpy(xr_snd_dai_links + total_links,
				fk_i2s_be_dai_links,
				sizeof(fk_i2s_be_dai_links));
			total_links +=
				ARRAY_SIZE(fk_i2s_be_dai_links);
			AUD_LOG_INFO(AUD_SOC, "Using fk_i2s_be_dai_links");
		}
	}

	ret = of_property_read_u32(dev->of_node, "xring,vad-i2s-intf",
					  &vad_i2s_intf);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "%s: No DT match vad_i2s interface\n",
			__func__);
	} else {
		if (vad_i2s_intf) {
			memcpy(xr_snd_dai_links + total_links,
				fk_vad_i2s_be_dai_links,
				sizeof(fk_vad_i2s_be_dai_links));
			total_links +=
				ARRAY_SIZE(fk_vad_i2s_be_dai_links);
			AUD_LOG_INFO(AUD_SOC, "%s: Using fk_vad_i2s_be_dai_links\n", __func__);
		}
	}
	ret = of_property_read_u32(dev->of_node, "xring,tdm-audio-intf",
					  &tdm_audio_intf);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "%s: No DT match tdm interface\n",
			__func__);
	} else {
		if (tdm_audio_intf) {
			memcpy(xr_snd_dai_links + total_links,
				fk_tdm_be_dai_links,
				sizeof(fk_tdm_be_dai_links));
			total_links +=
				ARRAY_SIZE(fk_tdm_be_dai_links);
			AUD_LOG_INFO(AUD_SOC, "%s: Using fk_tdm_be_dai_links\n", __func__);
		}
	}
	ret = of_property_read_u32(dev->of_node, "xring,audio-usb-intf",
					  &audio_usb_intf);
	if (ret)
		AUD_LOG_INFO(AUD_SOC, "No DT match audio usb interface");
	else {
		if (audio_usb_intf) {
			memcpy(xr_snd_dai_links + total_links,
				fk_usb_be_dai_links,
				sizeof(fk_usb_be_dai_links));
			total_links +=
				ARRAY_SIZE(fk_usb_be_dai_links);
			AUD_LOG_DBG(AUD_SOC, "Using fk_usb_be_dai_links");
		}
	}
	ret = of_property_read_u32(dev->of_node, "xring,mi2s0-intf",
					  &mi2s0_intf);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "%s: No DT match mi2s0 interface\n",
			__func__);
	} else {
		if (mi2s0_intf) {
			memcpy(xr_snd_dai_links + total_links,
				fk_mi2s0_be_dai_links,
				sizeof(fk_mi2s0_be_dai_links));
			total_links +=
				ARRAY_SIZE(fk_mi2s0_be_dai_links);
			AUD_LOG_INFO(AUD_SOC, "%s: Using fk_mi2s0_be_dai_links\n", __func__);
		}
	}
	dailink = xr_snd_dai_links;

	if (card) {
		card->dai_link = dailink;
		card->num_links = total_links;
	}

	return card;
}

struct fk_asoc_machine_data *fk_asoc_machine_data_set(struct device *dev)
{
	struct fk_asoc_machine_data *pdata = NULL;
	int ret;

	pdata = devm_kzalloc(dev,
			sizeof(struct fk_asoc_machine_data), GFP_KERNEL);
	if (!pdata)
		return NULL;

	ret = of_property_read_u32(dev->of_node, "xring,codec-fmt",
				  &pdata->codec_fmt);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "No DT match xring,codec-fmt");
		return NULL;
	}

	return pdata;
}

static int fk_asoc_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = NULL;
	struct fk_asoc_machine_data *pdata = NULL;
	int ret = 0;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
	struct timespec64 curTime;
#endif

	AUD_LOG_INFO(AUD_SOC, "Sound card register begin");

	if (!pdev->dev.of_node) {
		AUD_LOG_ERR(AUD_SOC, "No platform supplied from device tree");
		return -EINVAL;
	}

	pdata = fk_asoc_machine_data_set(&pdev->dev);
	if (!pdata) {
		AUD_LOG_ERR(AUD_SOC, "fk asoc machine data set failed");
		ret = -EINVAL;
		goto err;
	}

	card = populate_snd_card_dailinks(&pdev->dev);
	if (!card) {
		AUD_LOG_ERR(AUD_SOC, "Card uninitialized");
		ret = -EINVAL;
		goto err;
	}
	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, pdata);
	// snd_soc_card_set_drvdata(card, pdata);
	ret = snd_soc_of_parse_card_name(card, "xring,model");
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "parse card name failed, err:%d",
			ret);
		goto err;
	}

	ret = fk_populate_dai_link_component_of_node(card);
	if (ret) {
		ret = -EPROBE_DEFER;
		AUD_LOG_ERR(AUD_SOC, "pop dai link component node, err:%d",
			ret);
		goto err;
	}

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret == -EPROBE_DEFER) {
		AUD_LOG_ERR(AUD_SOC, "register card, err:%d", ret);
		ret = -EPROBE_DEFER;
		goto err;
	} else if (ret) {
		AUD_LOG_ERR(AUD_SOC, "snd_soc_register_card failed (%d)", ret);
		ret = -EPROBE_DEFER;
		goto err;
	}
	AUD_LOG_INFO(AUD_SOC, "Sound card %s registered", card->name);

	return 0;
err:
#if IS_ENABLED(CONFIG_MIEV)
	if (ret != -EPROBE_DEFER) {
		AUD_LOG_DBG(AUD_SOC, "<%s><%d>: X, failed.non-DEFER skip sound card registration.\n", __func__, __LINE__);
		ktime_get_real_ts64(&curTime);
		mievent  = cdev_tevent_alloc(906001001);
		cdev_tevent_add_int(mievent, "CurrentTime", curTime.tv_sec);
		cdev_tevent_add_str(mievent, "Keyword", "sound_card_not_registered");
		cdev_tevent_write(mievent);
		cdev_tevent_destroy(mievent);
	}
#endif
	return ret;
}

static int fk_asoc_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver xr_asoc_snd_machine_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = xr_asoc_snd_machine_of_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_asoc_machine_probe,
	.remove = fk_asoc_machine_remove,
};
int __init fk_asoc_machine_init(void)
{
	return platform_driver_register(&xr_asoc_snd_machine_driver);
}

void fk_asoc_machine_exit(void)
{
	platform_driver_unregister(&xr_asoc_snd_machine_driver);
}

MODULE_DESCRIPTION("XR ALSA SoC");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, xr_asoc_snd_machine_of_match);
