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
#include "fk-karaoke.h"

#define DRV_NAME "fk-karaoke"

#define PLAYBACK_MIN_NUM_PERIODS    2
#define PLAYBACK_MAX_NUM_PERIODS    8
#define PLAYBACK_MAX_PERIOD_SIZE    122880
#define PLAYBACK_MIN_PERIOD_SIZE    16
#define CAPTURE_MIN_NUM_PERIODS     2
#define CAPTURE_MAX_NUM_PERIODS     8
#define CAPTURE_MAX_PERIOD_SIZE     122880
#define CAPTURE_MIN_PERIOD_SIZE     16

static struct snd_pcm_hardware fk_karaoke_hardware_capture = {
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
	.rate_max =             96000,
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

static struct snd_pcm_hardware fk_karaoke_hardware_playback = {
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
	.rate_max =             96000,
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

static struct fk_karaoke karaoke_info;

static void karaoke_event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	if (payload.opCode == OP_CAPTURE_COPY)
		AUD_LOG_INFO(AUD_SOC, "fk karaoke capture");
}

static int fk_karaoke_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = &karaoke_info;

	AUD_LOG_INFO(AUD_SOC, "stream is %d", substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		runtime->hw = fk_karaoke_hardware_capture;
		prtd->playback_substream = substream;
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		runtime->hw = fk_karaoke_hardware_playback;
		prtd->capture_substream = substream;
	} else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d", substream->stream);
		return -EINVAL;
	}

	runtime->private_data = prtd;

	return 0;
}

static int fk_karaoke_trigger_start(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;

	AUD_LOG_INFO(AUD_SOC, "stream is %d", substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		ret = fk_acore_trigger(prtd->playback_session_id, OP_START);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed %d", ret);
			return ret;
		}

		prtd->playback_start = 1;
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ret = fk_acore_trigger(prtd->capture_session_id, OP_START);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed %d", ret);
			return ret;
		}

		prtd->capture_start = 1;
	}

	return ret;
}

static int fk_karaoke_trigger_stop(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;

	AUD_LOG_INFO(AUD_SOC, "stream is %d", substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		prtd->playback_start = 0;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		prtd->capture_start = 0;

	return ret;
}

static int fk_karaoke_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;

	AUD_LOG_ERR(AUD_SOC, "cmd %d", cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		ret = fk_karaoke_trigger_start(substream);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "start failed %d", ret);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		ret = fk_karaoke_trigger_stop(substream);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "stop failed %d", ret);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int fk_karaoke_playback_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format strm_fmt = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;

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
	prtd->play_samples = strm_fmt.samples = runtime->rate;
	prtd->play_channels = strm_fmt.channels = runtime->channels;
	prtd->play_bits = strm_fmt.bit_width = bits_per_sample;
	strm_fmt.codec_type = 0;

	AUD_LOG_INFO(AUD_SOC, "rate 0x%x, channel 0x%x, bit-width 0x%x",
		strm_fmt.samples, strm_fmt.channels, strm_fmt.bit_width);

	ret = fk_acore_startup(prtd->playback_session_id, strm_fmt, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);

	prtd->playback_prepare = 1;

	/* karaoke playback without write operation, active trigger start */
	fk_karaoke_trigger_start(substream);

	AUD_LOG_INFO(AUD_SOC, "completed");

	return 0;
}

static int fk_karaoke_capture_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format strm_fmt = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;

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
	prtd->cap_samples = strm_fmt.samples = runtime->rate;
	prtd->cap_channels = strm_fmt.channels = runtime->channels;
	prtd->cap_bits = strm_fmt.bit_width = bits_per_sample;
	strm_fmt.codec_type = 0;

	AUD_LOG_INFO(AUD_SOC, "rate 0x%x, channel 0x%x, bit-width 0x%x",
		strm_fmt.samples, strm_fmt.channels, strm_fmt.bit_width);
	ret = fk_acore_startup(prtd->capture_session_id, strm_fmt, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);

	prtd->capture_prepare = 1;

	AUD_LOG_INFO(AUD_SOC, "completed");

	return 0;
}

static int fk_karaoke_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;
	int ret = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		ret = fk_karaoke_playback_prepare(substream);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "playback failed %d", ret);
			return ret;
		}
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ret = fk_karaoke_capture_prepare(substream);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "capture failed %d", ret);
			return ret;
		}
	}

	if ((prtd->playback_prepare == 1) && (prtd->capture_prepare == 1)) {
		if ((prtd->play_samples != prtd->cap_samples) ||
			(prtd->play_channels != prtd->cap_channels) ||
			(prtd->play_bits != prtd->cap_bits)) {
			AUD_LOG_ERR(AUD_SOC, "capture format:samples(%d) channels(%d) bits(%d)",
				prtd->cap_samples, prtd->cap_channels, prtd->cap_bits);
			AUD_LOG_ERR(AUD_SOC, "play format:samples(%d) channels(%d) bits(%d)",
				prtd->play_samples, prtd->play_channels, prtd->play_bits);
			return -1;
		}
	}

	return ret;
}

static int fk_karaoke_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	uint16_t session_id;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;

	session_id = get_karaoke_session_id(substream);/*get session id*/
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		prtd->playback_session_id = session_id;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		prtd->capture_session_id = session_id;
	else {
		AUD_LOG_INFO(AUD_SOC, "invaild stream(%d)", substream->stream);
		return -1;
	}

	AUD_LOG_DBG(AUD_SOC, "session_id 0x%x", session_id);

	ret = fk_acore_register(session_id, (fk_acore_cb)karaoke_event_handler, prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_karaoke_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t session_id = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_karaoke *prtd = runtime->private_data;

	AUD_LOG_INFO(AUD_SOC, "stream is %d", substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		session_id = prtd->playback_session_id;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		session_id = prtd->capture_session_id;
	else {
		AUD_LOG_INFO(AUD_SOC, "invaild stream(%d)", substream->stream);
		return -1;
	}

	ret = fk_acore_unregister(session_id);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "unregister failed %d", ret);

	return ret;
}

const struct snd_soc_component_driver fk_karaoke_soc_component = {
	.name		= DRV_NAME,
	.open		= fk_karaoke_open,
	.prepare    = fk_karaoke_prepare,
	.hw_params	= fk_karaoke_hw_params,
	.trigger	= fk_karaoke_trigger,
	.close      = fk_karaoke_close,
};

static int fk_karaoke_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev,
					&fk_karaoke_soc_component,
					NULL, 0);
}
static int fk_karaoke_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_karaoke_dt_match[] = {
	{.compatible = "xring,fk-karaoke"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_karaoke_dt_match);

static struct platform_driver fk_karaoke_driver = {
	.driver = {
		.name = "fk-karaoke",
		.owner = THIS_MODULE,
		.of_match_table = fk_karaoke_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_karaoke_probe,
	.remove = fk_karaoke_remove,
};

int __init fk_karaoke_init(void)
{
	return platform_driver_register(&fk_karaoke_driver);
}

void fk_karaoke_exit(void)
{
	platform_driver_unregister(&fk_karaoke_driver);
}

MODULE_DESCRIPTION("Karaoke PCM module platform driver");
MODULE_LICENSE("Dual BSD/GPL");
