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
#include <linux/io.h>
#include <linux/version.h>
#include <linux/of_device.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>

#include "../acore/fk-acore-api.h"
#include "../common/fk-audio-cma.h"
#include "../common/fk-audio-log.h"

#include "fk-pcm.h"
#include "../common/audio-trace.h"

#define DRV_NAME "fk-pcm"

static struct snd_pcm_hardware fk_pcm_hardware_capture = {
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

static struct snd_pcm_hardware fk_pcm_hardware_playback = {
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

static void event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	struct fk_audio *prtd = priv;
	struct snd_pcm_substream *substream = prtd->substream;
	struct rsp_payload ptrmem = payload;

	AUD_LOG_DBG_LIM(AUD_SOC, "opcode=%d,payload result=%d,buf_info result=%d",
		ptrmem.opCode, ptrmem.payload.result, ptrmem.payload.buf_info.result);
	if ((payload.opCode == OP_PLAYBACK_COPY) || (payload.opCode == OP_CAPTURE_COPY)) {
		prtd->pcm_irq_pos += prtd->pcm_count;
		snd_pcm_period_elapsed(substream);
	}
}

static int fk_pcm_playback_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;

	AUD_LOG_DBG(AUD_SOC, "session_id=0x%x", prtd->session_id);
	prtd->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	prtd->pcm_count = snd_pcm_lib_period_bytes(substream);
	prtd->pcm_irq_pos = 0;
	/* rate and channels are sent to audio driver */
	prtd->samp_rate = runtime->rate;
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

static int fk_pcm_capture_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;
	struct buf_blk p_buf;

	AUD_LOG_DBG(AUD_SOC, "prtd->session_id=0x%x", prtd->session_id);
	prtd->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	prtd->pcm_count = snd_pcm_lib_period_bytes(substream);
	prtd->pcm_irq_pos = 0;
	/* rate and channels are sent to audio driver */
	prtd->samp_rate = runtime->rate;
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
		AUD_LOG_ERR(AUD_SOC, "startup failed %d", ret);

	AUD_LOG_DBG(AUD_SOC, "dma_addr = 0x%llx, dma_area=0x%llx",
		runtime->dma_addr, (uint64_t)runtime->dma_area);
	p_buf.vir_addr = runtime->dma_area;
	p_buf.phy_addr = runtime->dma_addr;
	p_buf.result = 0;
	p_buf.size = runtime->period_size * runtime->periods *
		runtime->channels * bits_per_sample / 8;
	ret = fk_acore_capture_copy(prtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "capture copy failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_pcm_playback_copy(struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, void __user *buf,
			 unsigned long bytes)
{
	int ret = 0;
	struct buf_blk p_buf;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;

	p_buf.vir_addr = runtime->dma_area + pos;
	p_buf.phy_addr = runtime->dma_addr + pos;
	p_buf.size = bytes;
	p_buf.result = 0;

	AUD_LOG_DBG_LIM(AUD_SOC, "write %lu bytes of buffer to adsp 0x%llx ", bytes, (uint64_t)p_buf.vir_addr);
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

static int fk_pcm_capture_copy(struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, void __user *buf,
			 unsigned long bytes)
{
	int ret = 0;
	struct buf_blk p_buf;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;

	AUD_LOG_DBG_LIM(AUD_SOC, "channel=%d,pos=%lu,bytes=%lu",
		channel, pos, bytes);

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

static int fk_pcm_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = NULL;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter");
#endif

	prtd = kzalloc(sizeof(struct fk_audio), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	substream->wait_time = PCM_WAIT_TIME;
	prtd->substream = substream;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		runtime->hw = fk_pcm_hardware_playback;
	/* Capture path */
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		runtime->hw = fk_pcm_hardware_capture;
	else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d", substream->stream);
		kfree(prtd);
		return -EINVAL;
	}

	runtime->private_data = prtd;

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return ret;
}

static int fk_pcm_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	int ret = 0;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter");
#endif

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		ret = fk_pcm_playback_prepare(substream);
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		ret = fk_pcm_capture_prepare(substream);

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return ret;
}

static int fk_pcm_copy(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, struct iov_iter *iter,
			 unsigned long bytes)
{
	int ret = 0;
	void __user *buf = iter->ubuf;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		ret = fk_pcm_playback_copy(substream, channel, pos, buf, bytes);
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		ret = fk_pcm_capture_copy(substream, channel, pos, buf, bytes);
	return ret;
}

static int fk_pcm_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter");
#endif

	//prtd->session_id = get_session_id(substream);/*get session id*/
	prtd->session_id = get_pcm_session_id(substream);/*get session id*/
	dma_buf->bytes = params_buffer_bytes(params);/*valid data size*/
	dma_buf->area = fk_acore_smem_alloc(component->dev, &dma_buf->addr, dma_buf->bytes);
	if (dma_buf->area == NULL) {
		AUD_LOG_ERR(AUD_SOC, "v_addr=0x%llx p_addr=0x%llx",
			(uint64_t)dma_buf->area, dma_buf->addr);
		return -ENOMEM;
	}
	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	AUD_LOG_DBG(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr= 0x%llx,area=0x%llx",
			prtd->session_id, runtime->dma_bytes,
			runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_register(prtd->session_id, (fk_acore_cb)event_handler, prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return ret;
}

int fk_pcm_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		ret = fk_acore_trigger(prtd->session_id, OP_START);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "trigger start failed");
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		ret = fk_acore_trigger(prtd->session_id, OP_STOP);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "trigger stop failed");
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		ret = fk_acore_trigger(prtd->session_id, OP_PAUSE);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "trigger pause failed");
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static snd_pcm_uframes_t fk_pcm_pointer(struct snd_soc_component *component,
				     struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;

	if (prtd->pcm_irq_pos >= prtd->pcm_size)
		prtd->pcm_irq_pos = 0;
	//AUD_LOG_DBG_LIM(AUD_SOC, "pcm_irq_pos=%d", prtd->pcm_irq_pos);
	return bytes_to_frames(runtime, (prtd->pcm_irq_pos));
}

static int fk_pcm_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio *prtd = runtime->private_data;

	AUD_LOG_DBG(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr=0x%llx,area=0x%llx",
		prtd->session_id, runtime->dma_bytes,
		runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_unregister(prtd->session_id);
	/*free audio share memory*/
	fk_acore_smem_free(component->dev, runtime->dma_bytes,
						runtime->dma_area, runtime->dma_addr);
	kfree(prtd);
	return ret;
}
static int fk_asoc_pcm_new(struct snd_soc_component *component,
			     struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

const struct snd_soc_component_driver fk_soc_component = {
	.name			= DRV_NAME,
	.open			= fk_pcm_open,
	.prepare		= fk_pcm_prepare,
	.copy			= fk_pcm_copy,
	.pointer		= fk_pcm_pointer,
	.hw_params		= fk_pcm_hw_params,
	.trigger		= fk_pcm_trigger,
	.close			= fk_pcm_close,
	.pcm_construct	= fk_asoc_pcm_new,
};

static int fk_pcm_probe(struct platform_device *pdev)
{
	int ret = 0;
	int id;
	struct fk_plat_data *pdata;

	ret = of_property_read_u32(pdev->dev.of_node, "xring,fk-pcm-dsp-id", &id);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "xring,fk-pcm-dsp-id missing in DT node");
		return ret;
	}
	pdata = kmalloc(sizeof(struct fk_plat_data), GFP_KERNEL);
	if (of_property_read_bool(pdev->dev.of_node, "xring,fk-fast-pcm"))
		pdata->pcm_mode = FAST_PCM_MODE;
	else if (of_property_read_bool(pdev->dev.of_node, "xring,fk-ultra-fast-pcm"))
		pdata->pcm_mode = ULTRA_FAST_PCM_MODE;
	else
		pdata->pcm_mode = NORMAL_PCM_MODE;

	mutex_init(&pdata->lock);
	dev_set_drvdata(&pdev->dev, pdata);

	return snd_soc_register_component(&pdev->dev,
					&fk_soc_component,
					NULL, 0);
}
static int fk_pcm_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_pcm_dt_match[] = {
	{.compatible = "xring,fk-pcm-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_pcm_dt_match);

static struct platform_driver fk_pcm_driver = {
	.driver = {
		.name = "fk-pcm-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_pcm_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_pcm_probe,
	.remove = fk_pcm_remove,
};

int __init fk_pcm_dsp_init(void)
{

	return platform_driver_register(&fk_pcm_driver);
}

void fk_pcm_dsp_exit(void)
{
	platform_driver_unregister(&fk_pcm_driver);
}

MODULE_DESCRIPTION("PCM module platform driver");
MODULE_LICENSE("Dual BSD/GPL");
