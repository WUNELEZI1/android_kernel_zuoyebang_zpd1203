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

#include "fk-dai-fe.h"
#include "../acore/fk-acore-api.h"
#include "../common/fk-audio-log.h"

#define DRV_NAME "fk-loopback"

#define PLAYBACK_MIN_NUM_PERIODS	2
#define PLAYBACK_MAX_NUM_PERIODS	8
#define PLAYBACK_MAX_PERIOD_SIZE	122880
#define PLAYBACK_MIN_PERIOD_SIZE	128
#define CAPTURE_MIN_NUM_PERIODS		2
#define CAPTURE_MAX_NUM_PERIODS		8
#define CAPTURE_MAX_PERIOD_SIZE		122880
#define CAPTURE_MIN_PERIOD_SIZE		320

struct fk_loopback {
	struct snd_pcm_substream *cap_substream;
	struct snd_pcm_substream *play_substream;
	uint8_t cap_open;
	uint8_t play_open;
	uint8_t cap_startup;
	uint8_t play_startup;

	uint8_t loop_on;

	uint32_t session_id;
};

static struct fk_loopback fk_lp_priv;

static struct snd_pcm_hardware fk_loopback_hardware_capture = {
	.info = (SNDRV_PCM_INFO_MMAP |
			SNDRV_PCM_INFO_BLOCK_TRANSFER |
			SNDRV_PCM_INFO_MMAP_VALID |
			SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE |
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

static struct snd_pcm_hardware fk_loopback_hardware_playback = {
	.info = (SNDRV_PCM_INFO_MMAP |
			SNDRV_PCM_INFO_BLOCK_TRANSFER |
			SNDRV_PCM_INFO_MMAP_VALID |
			SNDRV_PCM_INFO_INTERLEAVED |
			SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE |
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

static int fk_loopback_open(struct snd_soc_component *component,
	struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	if (!component) {
		AUD_LOG_ERR(AUD_SOC, "component is NULL");
		return -EINVAL;
	}
	AUD_LOG_INFO(AUD_SOC, "stream=%d", substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		runtime->hw = fk_loopback_hardware_playback;

		fk_lp_priv.play_substream = substream;
		fk_lp_priv.play_open = 1;
	/* Capture path */
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		runtime->hw = fk_loopback_hardware_capture;

		fk_lp_priv.cap_substream = substream;
		fk_lp_priv.cap_open = 1;
	} else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d", substream->stream);
		return -EINVAL;
	}

	fk_lp_priv.session_id = SCENE_LOOPBACK << 8;

	runtime->private_data = &fk_lp_priv;

	return 0;
}

static int fk_loopback_hw_params(struct snd_soc_component *component,
			struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_loopback *fk_lp;

	fk_lp = runtime->private_data;
	if (!fk_lp) {
		AUD_LOG_ERR(AUD_SOC, "private is NULL");
		return -EINVAL;
	}

	AUD_LOG_INFO(AUD_SOC, "number=%d, session_id=0x%x",
		substream->number, fk_lp->session_id);

	if (fk_lp->cap_open && fk_lp->play_open) {
		ret = fk_acore_register(fk_lp->session_id, NULL, fk_lp);
		if (ret) {
			AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
			return ret;
		}

		/* update loop on flag */
		fk_lp->loop_on = 1;
	}

	return 0;
}

static int fk_loopback_prepare(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_loopback *fk_lp = NULL;
	uint8_t *startup_flag = NULL;

	uint16_t bits_per_sample = 16;
	struct stream_format lp_format = {0};
	struct share_mem smem = {0};

	fk_lp = runtime->private_data;
	if (!fk_lp) {
		AUD_LOG_ERR(AUD_SOC, "private is NULL");
		return -EINVAL;
	}

	AUD_LOG_INFO(AUD_SOC, "stream=%d", substream->stream);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		startup_flag = (uint8_t *)&fk_lp->play_startup;
	/* Capture path */
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		startup_flag = (uint8_t *)&fk_lp->cap_startup;
	} else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d",
			substream->stream);
		return -EINVAL;
	}
	/* set startup flag */
	*startup_flag = 1;

	if (fk_lp->play_startup && fk_lp->cap_startup) {
		struct snd_pcm_runtime *cap_runtime = fk_lp->cap_substream->runtime;
		struct snd_pcm_runtime *play_runtime = fk_lp->play_substream->runtime;

		/* check rate */
		if (cap_runtime->rate != play_runtime->rate) {
			AUD_LOG_ERR(AUD_SOC, "rate unmatch: cap(%d) play(%d)",
				cap_runtime->rate, play_runtime->rate);
			return -EINVAL;
		}

		/* check channel */
		if (cap_runtime->channels != play_runtime->channels) {
			AUD_LOG_ERR(AUD_SOC, "channels unmatch: cap(%d) play(%d)",
				cap_runtime->channels, play_runtime->channels);
			return -EINVAL;
		}

		/* check format */
		if (cap_runtime->format != play_runtime->format) {
			AUD_LOG_ERR(AUD_SOC, "format unmatch: cap(%d) play(%d)",
				cap_runtime->format, play_runtime->format);
			return -EINVAL;
		}

		/* fill loopback stream format info */
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
		lp_format.samples = runtime->rate;
		lp_format.channels = runtime->channels;
		lp_format.bit_width = bits_per_sample;
		lp_format.codec_type = 0;

		/* data stream bypass ap in loopback usecase */
		smem.p_addr = 0;
		smem.vir_addr = NULL;

		/* startup loopback usecase */
		ret = fk_acore_startup(fk_lp->session_id, lp_format, smem);
		/* resume startup flag */
		if (ret < 0) {
			*startup_flag = 0;
			AUD_LOG_ERR(AUD_SOC, "startup failed %d", ret);
		}
	}

	return ret;
}

static int fk_loopback_close(struct snd_soc_component *component,
			struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_loopback *fk_lp = NULL;

	fk_lp = runtime->private_data;
	if (!fk_lp) {
		AUD_LOG_ERR(AUD_SOC, "lp private is NULL");
		return -EINVAL;
	}

	AUD_LOG_INFO(AUD_SOC, "number=%d", substream->number);

	if (fk_lp->loop_on) {
		ret = fk_acore_unregister(fk_lp->session_id);
		if (!ret) {
			fk_lp->loop_on = 0;
		} else {
			AUD_LOG_ERR(AUD_SOC, "lp unregister fail %d",
				fk_lp->session_id);
		}
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		fk_lp->play_open = 0;
		fk_lp->play_startup = 0;
		fk_lp->play_substream = NULL;
	/* Capture path */
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		fk_lp->cap_open = 0;
		fk_lp->cap_startup = 0;
		fk_lp->cap_substream = NULL;
	} else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d",
			substream->stream);
		return -EINVAL;
	}

	return 0;
}

static int fk_loopback_trigger(struct snd_soc_component *component,
		struct snd_pcm_substream *substream, int cmd)
{
	AUD_LOG_INFO(AUD_SOC, "number=%d, cmd=%d", substream->number, cmd);
	return 0;
}

const struct snd_soc_component_driver fk_loopback_soc_component = {
	.name		= DRV_NAME,
	.open		= fk_loopback_open,
	.trigger	= fk_loopback_trigger,
	.hw_params	= fk_loopback_hw_params,
	.prepare	= fk_loopback_prepare,
	.close		= fk_loopback_close,
};

static int fk_loopback_probe(struct platform_device *pdev)
{
	fk_lp_priv.loop_on = 0;

	fk_lp_priv.cap_open = 0;
	fk_lp_priv.play_open = 0;
	fk_lp_priv.cap_startup = 0;
	fk_lp_priv.play_startup = 0;

	fk_lp_priv.session_id = 0;

	fk_lp_priv.cap_substream = NULL;
	fk_lp_priv.play_substream = NULL;

	AUD_LOG_INFO(AUD_SOC, "loopback device register");

	return snd_soc_register_component(&pdev->dev,
					&fk_loopback_soc_component,
					NULL, 0);
}
static int fk_loopback_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_loopback_dt_match[] = {
	{.compatible = "xring,fk-loopback-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_loopback_dt_match);

static struct platform_driver fk_loopback_driver = {
	.driver = {
		.name = "fk-loopback-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_loopback_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_loopback_probe,
	.remove = fk_loopback_remove,
};

int __init fk_loopback_dsp_init(void)
{
	return platform_driver_register(&fk_loopback_driver);
}

void fk_loopback_dsp_exit(void)
{
	platform_driver_unregister(&fk_loopback_driver);
}

MODULE_DESCRIPTION("Loopback PCM module platform driver");
MODULE_LICENSE("Dual BSD/GPL");
