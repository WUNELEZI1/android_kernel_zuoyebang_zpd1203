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
#include <linux/io.h>
#include <linux/of_device.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>
#include "../common/fk-audio-log.h"
#include "../common/fk-audio-cma.h"
#include "../acore/fk-acore-api.h"
#include "fk-hf.h"

#define DRV_NAME "fk-hf"

#define PLAYBACK_MIN_NUM_PERIODS    0
#define PLAYBACK_MAX_NUM_PERIODS    8
#define PLAYBACK_MAX_PERIOD_SIZE    122880
#define PLAYBACK_MIN_PERIOD_SIZE    0
#define CAPTURE_MIN_NUM_PERIODS     2
#define CAPTURE_MAX_NUM_PERIODS     8
#define CAPTURE_MAX_PERIOD_SIZE     122880
#define CAPTURE_MIN_PERIOD_SIZE     16
static struct snd_pcm_hardware fk_hf_hardware_capture = {
	.info =             (SNDRV_PCM_INFO_MMAP |
						SNDRV_PCM_INFO_BLOCK_TRANSFER |
						SNDRV_PCM_INFO_MMAP_VALID |
						SNDRV_PCM_INFO_INTERLEAVED |
						SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats =			(SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
	.rates =			SNDRV_PCM_RATE_8000_384000,
	.rate_min =			8000,
	.rate_max =			384000,
	.channels_min =		1,
	.channels_max =		4,
	.buffer_bytes_max = CAPTURE_MAX_NUM_PERIODS * CAPTURE_MAX_PERIOD_SIZE,
	.period_bytes_min = CAPTURE_MIN_PERIOD_SIZE,
	.period_bytes_max = CAPTURE_MAX_PERIOD_SIZE,
	.periods_min =		CAPTURE_MIN_NUM_PERIODS,
	.periods_max =		CAPTURE_MAX_NUM_PERIODS,
	.fifo_size =		0,
};
static struct snd_pcm_hardware fk_hf_hardware_playback = {
	.info =             (SNDRV_PCM_INFO_MMAP |
						SNDRV_PCM_INFO_BLOCK_TRANSFER |
						SNDRV_PCM_INFO_MMAP_VALID |
						SNDRV_PCM_INFO_INTERLEAVED |
						SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	.formats =          (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
	.rates =			SNDRV_PCM_RATE_8000_384000,
	.rate_min =			8000,
	.rate_max =			384000,
	.channels_min =		1,
	.channels_max =		8,
	.buffer_bytes_max = PLAYBACK_MAX_NUM_PERIODS * PLAYBACK_MAX_PERIOD_SIZE,
	.period_bytes_min =	PLAYBACK_MIN_PERIOD_SIZE,
	.period_bytes_max = PLAYBACK_MAX_PERIOD_SIZE,
	.periods_min =		PLAYBACK_MIN_NUM_PERIODS,
	.periods_max =		PLAYBACK_MAX_NUM_PERIODS,
	.fifo_size =		0,
};

static int fk_hf_playback_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_hf *prtd = runtime->private_data;

	/* rate and channels are sent to audio driver */
	prtd->sample_rate = runtime->rate;
	prtd->channel_mode = runtime->channels;
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S32_LE:
		bits_per_sample = 32;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		bits_per_sample = 32;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		bits_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
	default:
		bits_per_sample = 16;
		break;
	}
	prtd->bit_width = bits_per_sample;
	st_format.samples = runtime->rate;
	st_format.channels = runtime->channels;
	st_format.bit_width = bits_per_sample;
	st_format.codec_type = 0;

	ret = fk_acore_startup(prtd->session_id, st_format, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
	return ret;
}
static int fk_hf_capture_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_hf *prtd = runtime->private_data;

	/* rate and channels are sent to audio driver */
	prtd->sample_rate = runtime->rate;
	prtd->channel_mode = runtime->channels;
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S32_LE:
		bits_per_sample = 32;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		bits_per_sample = 32;
		break;
	case SNDRV_PCM_FORMAT_S24_3LE:
		bits_per_sample = 24;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
	default:
		bits_per_sample = 16;
		break;
	}
	prtd->bit_width = bits_per_sample;
	st_format.samples = runtime->rate;
	st_format.channels = runtime->channels;
	st_format.bit_width = bits_per_sample;
	st_format.codec_type = 0;

	ret = fk_acore_startup(prtd->session_id, st_format, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);

	return ret;
}

static int fk_hf_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_hf *prtd = NULL;

	prtd = kzalloc(sizeof(struct fk_hf), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	prtd->substream = substream;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		runtime->hw = fk_hf_hardware_playback;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		runtime->hw = fk_hf_hardware_capture;

	runtime->private_data = prtd;
	return 0;
}

static int fk_hf_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_hf *prtd = runtime->private_data;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		ret = fk_acore_trigger(prtd->session_id, OP_START);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed");
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		ret = fk_acore_trigger(prtd->session_id, OP_STOP);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed");
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		ret = fk_acore_trigger(prtd->session_id, OP_PAUSE);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "acore trigger failed");
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int fk_hf_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	int ret = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		ret = fk_hf_playback_prepare(substream);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "hf playback prepare failed %d", ret);
			return ret;
		}
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		ret = fk_hf_capture_prepare(substream);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "hf capture prepare failed %d", ret);
			return ret;
		}
	}

	ret = fk_hf_trigger(component, substream, SNDRV_PCM_TRIGGER_START);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "hf trigger start failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_hf_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_hf *prtd = runtime->private_data;

	prtd->session_id = get_hf_session_id(substream);

	ret = fk_acore_register(prtd->session_id, NULL, prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}
	return ret;
}

static int fk_hf_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_hf *prtd = runtime->private_data;

	AUD_LOG_INFO(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr=0x%llx,area=0x%llx\n",
		prtd->session_id, runtime->dma_bytes, runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_unregister(prtd->session_id);
	/*free audio share memory*/
	fk_acore_smem_free(component->dev, runtime->dma_bytes,
						runtime->dma_area, runtime->dma_addr);
	kfree(prtd);
	return ret;
}

static int fk_asoc_hf_new(struct snd_soc_component *component,
			     struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

const struct snd_soc_component_driver fk_soc_hf_component = {
	.name			= DRV_NAME,
	.open			= fk_hf_open,
	.prepare		= fk_hf_prepare,
	.hw_params		= fk_hf_hw_params,
	.trigger		= fk_hf_trigger,
	.close			= fk_hf_close,
	.pcm_construct	= fk_asoc_hf_new,
};
static int fk_hf_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev,
					&fk_soc_hf_component,
					NULL, 0);
}
static int fk_hf_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}
static const struct of_device_id fk_hf_dt_match[] = {
	{.compatible = "xring,fk-hf-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_hf_dt_match);
static struct platform_driver fk_hf_driver = {
	.driver = {
		.name = "fk-hf-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_hf_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_hf_probe,
	.remove = fk_hf_remove,
};
int __init fk_hf_dsp_init(void)
{
	return platform_driver_register(&fk_hf_driver);
}
void fk_hf_dsp_exit(void)
{
	platform_driver_unregister(&fk_hf_driver);
}
MODULE_DESCRIPTION("HF module platform driver");
MODULE_LICENSE("Dual BSD/GPL");

