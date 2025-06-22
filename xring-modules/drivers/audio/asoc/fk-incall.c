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
#include "fk-incall.h"

#define DRV_NAME "fk-incall"

#define PLAYBACK_MIN_NUM_PERIODS    2
#define PLAYBACK_MAX_NUM_PERIODS    8
#define PLAYBACK_MAX_PERIOD_SIZE    122880
#define PLAYBACK_MIN_PERIOD_SIZE    16
#define CAPTURE_MIN_NUM_PERIODS     2
#define CAPTURE_MAX_NUM_PERIODS     8
#define CAPTURE_MAX_PERIOD_SIZE     122880
#define CAPTURE_MIN_PERIOD_SIZE     16
static struct snd_pcm_hardware fk_incall_hardware_capture = {
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
static struct snd_pcm_hardware fk_incall_hardware_playback = {
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
static void incall_event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	struct fk_incall *prtd = priv;
	struct snd_pcm_substream *substream = prtd->substream;
	struct rsp_payload ptrmem = payload;

	AUD_LOG_DBG_LIM(AUD_SOC, "ptrmem.opcode=%d,ptrmem.payload.result=%d,ptrmem.payload.buf_info.result=%d",
		ptrmem.opCode, ptrmem.payload.result, ptrmem.payload.buf_info.result);
	if ((payload.opCode == OP_PLAYBACK_COPY) || (payload.opCode == OP_CAPTURE_COPY)) {
		prtd->pcm_irq_pos += prtd->pcm_count;
		snd_pcm_period_elapsed(substream);
	}
}
static int fk_music_incall_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

	prtd->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	prtd->pcm_count = snd_pcm_lib_period_bytes(substream);
	prtd->pcm_irq_pos = 0;
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
	smem.p_addr = runtime->dma_addr;
	smem.vir_addr = runtime->dma_area;
	smem.periods_size = runtime->period_size;
	smem.periods_num = runtime->periods;
	ret = fk_acore_startup(prtd->session_id, st_format, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
	return ret;
}
static int fk_voice_call_record_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

	prtd->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	prtd->pcm_count = snd_pcm_lib_period_bytes(substream);
	prtd->pcm_irq_pos = 0;
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
	smem.vir_addr = runtime->dma_area;
	smem.p_addr = runtime->dma_addr;
	smem.periods_size = runtime->period_size;
	smem.periods_num = runtime->periods;
	ret = fk_acore_startup(prtd->session_id, st_format, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
/*simulate test*/
{
	struct buf_blk p_buf;
	struct snd_pcm_runtime *runtime = substream->runtime;

	AUD_LOG_INFO(AUD_SOC, "dma_addr = 0x%llx, dma_area=0x%llx",
		runtime->dma_addr, (uint64_t)runtime->dma_area);
	p_buf.vir_addr = runtime->dma_area;
	p_buf.phy_addr = runtime->dma_addr;
	p_buf.result = 0;
	p_buf.size = runtime->period_size * runtime->periods *
		runtime->channels * bits_per_sample / 8;
	ret = fk_acore_capture_copy(prtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore capture copy failed %d", ret);
		return ret;
	}
}
/*simulate test end*/
	return ret;
}
static int fk_music_incall_copy(struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, void __user *buf,
			 unsigned long bytes)
{
	int ret = 0;
	struct buf_blk p_buf;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

	AUD_LOG_DBG_LIM(AUD_SOC, "channel = %d, pos = %lu, bytes = %lu", channel, pos, bytes);
	p_buf.vir_addr = runtime->dma_area + pos;
	p_buf.phy_addr = runtime->dma_addr + pos;
	p_buf.size = bytes;
	p_buf.result = 0;
	if (copy_from_user(p_buf.vir_addr, buf, bytes)) {
		ret = -EFAULT;
		AUD_LOG_ERR(AUD_SOC, "copy_from_user failed");
		return ret;
	}
	ret = fk_acore_playback_copy(prtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore playback copy failed %d", ret);
		return ret;
	}
	return ret;
}
static int fk_voice_call_record_copy(struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, void __user *buf,
			 unsigned long bytes)
{
	int ret = 0;
	struct buf_blk p_buf;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

	AUD_LOG_DBG_LIM(AUD_SOC, "channel = %d, pos = %lu, bytes = %lu", channel, pos, bytes);
	p_buf.vir_addr = runtime->dma_area + pos;
	p_buf.phy_addr = runtime->dma_addr + pos;
	p_buf.size = bytes;
	p_buf.result = 0;
	if (copy_to_user(buf, p_buf.vir_addr, bytes)) {
		ret = -EFAULT;
		AUD_LOG_ERR(AUD_SOC, "copy_from_user failed");
		return ret;
	}
	ret = fk_acore_capture_copy(prtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore capture copy failed %d", ret);
		return ret;
	}
	return ret;
}
static int fk_incall_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = NULL;

	prtd = kzalloc(sizeof(struct fk_incall), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;
	substream->wait_time = PCM_WAIT_TIME;
	prtd->substream = substream;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		runtime->hw = fk_incall_hardware_playback;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		runtime->hw = fk_incall_hardware_capture;

	runtime->private_data = prtd;
	return 0;
}
static int fk_incall_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	int ret = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		ret = fk_music_incall_prepare(substream);
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		ret = fk_voice_call_record_prepare(substream);
	return ret;
}
static int fk_incall_copy(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, struct iov_iter *iter,
			 unsigned long bytes)
{
	int ret = 0;
	void __user *buf = iter->ubuf;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		ret = fk_music_incall_copy(substream, channel, pos, buf, bytes);
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		ret = fk_voice_call_record_copy(substream, channel, pos, buf, bytes);
	return ret;
}
static int fk_incall_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;

	prtd->session_id = get_incall_session_id(substream);
	dma_buf->bytes = params_buffer_bytes(params);
	dma_buf->area = fk_acore_smem_alloc(component->dev, &dma_buf->addr, dma_buf->bytes);
	if (dma_buf->area == NULL) {
		AUD_LOG_ERR(AUD_SOC, "v_addr=0x%llx p_addr=0x%llx",
			(uint64_t)dma_buf->area, dma_buf->addr);
		return -ENOMEM;
	}
	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	AUD_LOG_INFO(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr= 0x%llx,area=0x%llx",
		prtd->session_id, runtime->dma_bytes, runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_register(prtd->session_id, (fk_acore_cb)incall_event_handler, prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}
	return ret;
}
int fk_incall_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

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
static snd_pcm_uframes_t fk_incall_pointer(struct snd_soc_component *component,
				     struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

	if (prtd->pcm_irq_pos >= prtd->pcm_size)
		prtd->pcm_irq_pos = 0;
	return bytes_to_frames(runtime, (prtd->pcm_irq_pos));
}
static int fk_incall_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_incall *prtd = runtime->private_data;

	AUD_LOG_INFO(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr=0x%llx,area=0x%llx",
		prtd->session_id, runtime->dma_bytes,
		runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_unregister(prtd->session_id);
	/*free audio share memory*/
	fk_acore_smem_free(component->dev, runtime->dma_bytes,
						runtime->dma_area, runtime->dma_addr);
	kfree(prtd);
	return ret;
}
static int fk_asoc_incall_new(struct snd_soc_component *component,
			     struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}
const struct snd_soc_component_driver fk_soc_incall_component = {
	.name			= DRV_NAME,
	.open			= fk_incall_open,
	.prepare		= fk_incall_prepare,
	.copy			= fk_incall_copy,
	.pointer		= fk_incall_pointer,
	.hw_params		= fk_incall_hw_params,
	.trigger		= fk_incall_trigger,
	.close			= fk_incall_close,
	.pcm_construct	= fk_asoc_incall_new,
};
static int fk_incall_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev,
					&fk_soc_incall_component,
					NULL, 0);
}
static int fk_incall_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}
static const struct of_device_id fk_incall_dt_match[] = {
	{.compatible = "xring,fk-incall-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_incall_dt_match);
static struct platform_driver fk_incall_driver = {
	.driver = {
		.name = "fk-incall-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_incall_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_incall_probe,
	.remove = fk_incall_remove,
};
int __init fk_incall_dsp_init(void)
{
	return platform_driver_register(&fk_incall_driver);
}
void fk_incall_dsp_exit(void)
{
	platform_driver_unregister(&fk_incall_driver);
}
MODULE_DESCRIPTION("INCALL module platform driver");
MODULE_LICENSE("Dual BSD/GPL");

