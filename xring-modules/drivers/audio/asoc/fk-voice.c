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

#include <linux/init.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/timer.h>
#include <asm/dma.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>

#include "../common/fk-audio-log.h"
#include "fk-voice.h"

#define DRV_NAME "fk-voice"

#define PLAYBACK_MIN_NUM_PERIODS    2
#define PLAYBACK_MAX_NUM_PERIODS    8
#define PLAYBACK_MAX_PERIOD_SIZE    122880
#define PLAYBACK_MIN_PERIOD_SIZE    128
#define CAPTURE_MIN_NUM_PERIODS     2
#define CAPTURE_MAX_NUM_PERIODS     8
#define CAPTURE_MAX_PERIOD_SIZE     122880
#define CAPTURE_MIN_PERIOD_SIZE     320

static struct snd_pcm_hardware fk_voice_hardware_capture = {
	.info =                 (SNDRV_PCM_INFO_MMAP |
				SNDRV_PCM_INFO_BLOCK_TRANSFER |
				SNDRV_PCM_INFO_MMAP_VALID |
				SNDRV_PCM_INFO_INTERLEAVED |
				SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats =              (SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE |
				SNDRV_PCM_FMTBIT_S32_LE),
	.rates =                SNDRV_PCM_RATE_8000_384000,
	.rate_min =             8000,
	.rate_max =             384000,
	.channels_min =         1,
	.channels_max =         4,
	.buffer_bytes_max =     CAPTURE_MAX_NUM_PERIODS *
				CAPTURE_MAX_PERIOD_SIZE,
	.period_bytes_min =	CAPTURE_MIN_PERIOD_SIZE,
	.period_bytes_max =     CAPTURE_MAX_PERIOD_SIZE,
	.periods_min =          CAPTURE_MIN_NUM_PERIODS,
	.periods_max =          CAPTURE_MAX_NUM_PERIODS,
	.fifo_size =            0,
};

static struct snd_pcm_hardware fk_voice_hardware_playback = {
	.info =                 (SNDRV_PCM_INFO_MMAP |
				SNDRV_PCM_INFO_BLOCK_TRANSFER |
				SNDRV_PCM_INFO_MMAP_VALID |
				SNDRV_PCM_INFO_INTERLEAVED |
				SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats =              (SNDRV_PCM_FMTBIT_S16_LE |
				SNDRV_PCM_FMTBIT_S24_LE |
				SNDRV_PCM_FMTBIT_S24_3LE |
				SNDRV_PCM_FMTBIT_S32_LE),
	.rates =                SNDRV_PCM_RATE_8000_384000,
	.rate_min =             8000,
	.rate_max =             384000,
	.channels_min =         1,
	.channels_max =         8,
	.buffer_bytes_max =     PLAYBACK_MAX_NUM_PERIODS *
				PLAYBACK_MAX_PERIOD_SIZE,
	.period_bytes_min =	PLAYBACK_MIN_PERIOD_SIZE,
	.period_bytes_max =     PLAYBACK_MAX_PERIOD_SIZE,
	.periods_min =          PLAYBACK_MIN_NUM_PERIODS,
	.periods_max =          PLAYBACK_MAX_NUM_PERIODS,
	.fifo_size =            0,
};

static struct fk_voice voice_info;

static int fk_voice_band_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_info.band;

	return 0;
}

static int fk_voice_band_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int band_value = NUM_SPEECH_BAND;
	struct stream_params strm_param = {0};

	band_value = ucontrol->value.integer.value[0];

	if ((band_value >= SPEECH_NB) && (band_value < NUM_SPEECH_BAND)) {
		strm_param.type = PARAMS_SET_VOICE_BANDWIDTH;
		strm_param.params.value = band_value;
		ret = fk_acore_set_params(voice_info.playback_session_id, &strm_param);
		if (ret < 0)
			AUD_LOG_ERR(AUD_COMM, "set param failed, voice band set failed");
		else
			voice_info.band = band_value;
	} else {
		AUD_LOG_INFO(AUD_COMM, "voice band value invalid, band %d", band_value);
	}

	AUD_LOG_INFO(AUD_COMM, "current voice band = %d", voice_info.band);

	return 1;
}

static int fk_voice_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_info.volume;

	return 0;
}

static int fk_voice_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int volume_index = 0;

	volume_index = ucontrol->value.integer.value[0];
	ret = fk_acore_set_vol(voice_info.playback_session_id, volume_index);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "set voice vol failed");
	else
		voice_info.volume = volume_index;

	AUD_LOG_INFO(AUD_COMM, "current voice volume = %d", voice_info.volume);
	return 1;
}

static int fk_voice_ul_mute_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_info.ul_mute;

	return 0;
}

static int fk_voice_ul_mute_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	uint8_t mute = 0;

	mute = ucontrol->value.integer.value[0];
	if ((mute == false) || (mute == true)) {
		ret = fk_acore_set_mute(voice_info.capture_session_id, mute);
		if (ret < 0)
			AUD_LOG_ERR(AUD_COMM, "fk acore set ul mute failed");
		else
			voice_info.ul_mute = mute;
	} else {
		AUD_LOG_ERR(AUD_COMM, "voice ul mute value invalid, mute %d", mute);
	}

	AUD_LOG_INFO(AUD_COMM, "current voice ul mute = %d", voice_info.ul_mute);
	return 1;
}

static int fk_voice_dl_mute_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_info.dl_mute;

	return 0;
}

static int fk_voice_dl_mute_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	uint8_t mute = 0;

	mute = ucontrol->value.integer.value[0];
	if ((mute == false) || (mute == true)) {
		ret = fk_acore_set_mute(voice_info.playback_session_id, mute);
		if (ret < 0)
			AUD_LOG_ERR(AUD_COMM, "fk acore set dl mute failed");
		else
			voice_info.dl_mute = mute;
	} else {
		AUD_LOG_ERR(AUD_COMM, "voice dl mute value invalid, mute %d", mute);
	}

	AUD_LOG_INFO(AUD_COMM, "current voice dl mute = %d", voice_info.dl_mute);
	return 1;
}

static const struct snd_kcontrol_new fk_audio_voice_controls[] = {
	SOC_SINGLE_EXT("VOICE BAND", SND_SOC_NOPM, 0, NUM_SPEECH_BAND, 0,
		fk_voice_band_get, fk_voice_band_put),
	SOC_SINGLE_EXT("VOICE VOLUME", SND_SOC_NOPM, 0, VOLUME_VALUE_MAX, 0,
		fk_voice_volume_get, fk_voice_volume_put),
	SOC_SINGLE_EXT("VOICE UL MUTE", SND_SOC_NOPM, 0, 1, 0,
		fk_voice_ul_mute_get, fk_voice_ul_mute_put),
	SOC_SINGLE_EXT("VOICE DL MUTE", SND_SOC_NOPM, 0, 1, 0,
		fk_voice_dl_mute_get, fk_voice_dl_mute_put),
};

static int fk_voice_probe(struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component, fk_audio_voice_controls,
		ARRAY_SIZE(fk_audio_voice_controls));

	return 0;
}

static void voice_event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	struct rsp_payload ptrmem = payload;

	AUD_LOG_DBG_LIM(AUD_SOC, "ptrmem.opcode=%d,ptrmem.payload.result=%d,ptrmem.payload.buf_info.result=%d",
		ptrmem.opCode, ptrmem.payload.result, ptrmem.payload.buf_info.result);
}

static int fk_voice_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *voice = &voice_info;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		runtime->hw = fk_voice_hardware_capture;
		voice->playback_substream = substream;
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		runtime->hw = fk_voice_hardware_playback;
		voice->capture_substream = substream;
	} else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d", substream->stream);
		return -EINVAL;
	}

	/* voice using I2S, pcie backup */
	voice->bar_info.BAR0_1_addr = PCIE_BAR0_1_addr;
	voice->bar_info.BAR2_3_addr = PCIE_BAR2_3_addr;
	voice->band = NUM_SPEECH_BAND;

	runtime->private_data = voice;

	AUD_LOG_INFO(AUD_SOC, "fk voice exit");
	return 0;
}

static int fk_voice_trigger_start(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *prtd = runtime->private_data;

	if ((prtd->playback_prepare == 1) && (prtd->capture_prepare == 1)) {
		ret = fk_acore_trigger(prtd->playback_session_id, OP_START);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed %d", ret);
			return ret;
		}

		ret = fk_acore_trigger(prtd->capture_session_id, OP_START);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed %d", ret);
			return ret;
		}
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		prtd->playback_start = 1;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		prtd->capture_start = 1;

	AUD_LOG_INFO(AUD_SOC, "fk voice trigger start exit");

	return ret;
}

static int fk_voice_trigger_stop(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *prtd = runtime->private_data;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		prtd->playback_start = 0;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		prtd->capture_start = 0;

	AUD_LOG_INFO(AUD_SOC, "fk voice trigger stop exit");

	return ret;
}

static int fk_voice_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		ret = fk_voice_trigger_start(substream);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed %d", ret);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		ret = fk_voice_trigger_stop(substream);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed %d", ret);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int fk_voice_playback_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *prtd = runtime->private_data;

	/* rate and channels are sent to audio driver */
	prtd->samp_rate = runtime->rate;
	prtd->channel_mode = runtime->channels;
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S32_LE:
		bits_per_sample = 32;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		bits_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		bits_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
	default:
		bits_per_sample = 16;
		break;
	}
	st_format.samples = runtime->rate;
	st_format.channels = runtime->channels;
	st_format.bit_width = bits_per_sample;
	st_format.codec_type = 0;

	AUD_LOG_INFO(AUD_SOC, "playback rate 0x%x, channel 0x%x, bit-width 0x%x",
			st_format.samples, st_format.channels, st_format.bit_width);

	ret = fk_acore_startup(prtd->playback_session_id, st_format, smem);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
		return 0;
	}

	prtd->playback_prepare = 1;
	AUD_LOG_INFO(AUD_SOC, "fk voice playback prepare exit");

	return 0;
}

static int fk_voice_capture_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *prtd = runtime->private_data;

	/* rate and channels are sent to audio driver */
	prtd->samp_rate = runtime->rate;
	prtd->channel_mode = runtime->channels;
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S32_LE:
		bits_per_sample = 32;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		bits_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		bits_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
	default:
		bits_per_sample = 16;
		break;
	}
	st_format.samples = runtime->rate;
	st_format.channels = runtime->channels;
	st_format.bit_width = bits_per_sample;
	st_format.codec_type = 0;

	AUD_LOG_INFO(AUD_SOC, "capture rate 0x%x, channel 0x%x, bit-width 0x%x",
		st_format.samples, st_format.channels, st_format.bit_width);
	ret = fk_acore_startup(prtd->capture_session_id, st_format, smem);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
		return 0;
	}

	prtd->capture_prepare = 1;
	AUD_LOG_INFO(AUD_SOC, "fk voice capture prepare exit");

	return 0;
}

static int fk_voice_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	int ret = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		ret = fk_voice_playback_prepare(substream);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "voice playback prepare failed %d", ret);
			return ret;
		}
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ret = fk_voice_capture_prepare(substream);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "voice capture prepare failed %d", ret);
			return ret;
		}
	}

	ret = fk_voice_trigger(component, substream, SNDRV_PCM_TRIGGER_START);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "voice trigger start failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_voice_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	uint16_t session_id;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *prtd = runtime->private_data;
	struct stream_params strm_param = {0};

	session_id = get_voice_session_id(substream);/*get session id*/
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		prtd->playback_session_id = session_id;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		prtd->capture_session_id = session_id;

	AUD_LOG_DBG(AUD_SOC, "fk voice hw param session_id 0x%x", session_id);

	ret = fk_acore_register(session_id, (fk_acore_cb)voice_event_handler, prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}

	strm_param.type = PARAMS_SET_VOICE_PCIE_BAR;
	strm_param.params.pcie_bar.BAR0_1_addr = prtd->bar_info.BAR0_1_addr;
	strm_param.params.pcie_bar.BAR2_3_addr = prtd->bar_info.BAR2_3_addr;
	ret = fk_acore_set_params(session_id, &strm_param);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore set param failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_voice_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t session_id = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_voice *prtd = runtime->private_data;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		session_id = prtd->playback_session_id;
		prtd->playback_start = 0;
		prtd->playback_prepare = 0;
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		session_id = prtd->capture_session_id;
		prtd->capture_start = 0;
		prtd->capture_prepare = 0;
	}

	ret = fk_acore_unregister(session_id);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "fk acore unregister failed %d", ret);

	AUD_LOG_DBG(AUD_SOC, "fk voice trigger close exit");

	return ret;
}
static int fk_asoc_pcm_new(struct snd_soc_component *component,
			     struct snd_soc_pcm_runtime *rtd)
{
	AUD_LOG_DBG(AUD_SOC, "rtd->num=%d", rtd->num);
	return 0;
}

const struct snd_soc_component_driver fk_voice_soc_component = {
	.name		= DRV_NAME,
	.probe		= fk_voice_probe,
	.open		= fk_voice_open,
	.open       = fk_voice_open,
	.prepare    = fk_voice_prepare,
	.hw_params	= fk_voice_hw_params,
	.trigger	= fk_voice_trigger,
	.close      = fk_voice_close,
	.pcm_construct	= fk_asoc_pcm_new,
};

static int fk_pcm_voice_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev, &fk_voice_soc_component, NULL, 0);
}

static int fk_pcm_voice_remove(struct platform_device *pdev)

{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_voice_dt_match[] = {
	{.compatible = "xring,fk-voice-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_voice_dt_match);

static struct platform_driver fk_voice_driver = {
	.driver = {
		.name = "fk-voice-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_voice_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_pcm_voice_probe,
	.remove = fk_pcm_voice_remove,
};

int __init fk_voice_dsp_init(void)
{
	return platform_driver_register(&fk_voice_driver);
}

void fk_voice_dsp_exit(void)
{
	platform_driver_unregister(&fk_voice_driver);
}

MODULE_DESCRIPTION("Voice PCM module platform driver");
MODULE_LICENSE("Dual BSD/GPL");
