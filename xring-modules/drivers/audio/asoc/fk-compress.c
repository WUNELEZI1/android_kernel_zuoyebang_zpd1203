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
#include <linux/math64.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/timer.h>
#include <sound/tlv.h>
#include <sound/compress_params.h>
#include <sound/compress_offload.h>
#include <sound/compress_driver.h>

#include "../common/fk-audio-cma.h"
#include "../common/fk-audio-log.h"
#include "fk-compress.h"
#define DRV_NAME "fk-compress"

static struct fk_compr_audio offload_info;

static struct support_format g_compress_capture_support_format[] = {
	{SND_AUDIOCODEC_AMR, 1, 2, {8000, 16000}},
	{SND_AUDIOCODEC_AMRWB, 1, 2, {8000, 16000}},
	{SND_AUDIOCODEC_AAC, 2, 4, {8000, 16000, 44100, 48000}},
};

static void compr_event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	struct fk_compr_audio *cprtd = priv;
	struct snd_compr_stream *stream = cprtd->stream;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct rsp_payload ptrmem = payload;
	struct block_info *bk_info = NULL;
	uint32_t total;

	AUD_LOG_DBG_LIM(AUD_SOC, "buffer_size=%d",
		ptrmem.payload.buf_info.size);

	if (payload.opCode == OP_PLAYBACK_COPY) {
		total = ptrmem.payload.buf_info.size;
		cprtd->copied_total += total;
		snd_compr_fragment_elapsed(stream);
	} else if (payload.opCode == OP_DRAIN) {
		AUD_LOG_DBG(AUD_SOC, "stream->runtime->state=%d",
			stream->runtime->state);
		/*TODO for UDP */
		if (stream->runtime->state != SNDRV_PCM_STATE_DRAINING)
			msleep(20);
		/*end*/
		snd_compr_drain_notify(stream);
		stream->runtime->state = SNDRV_PCM_STATE_RUNNING;
	} else if (payload.opCode == OP_CAPTURE_COPY) {
		cprtd->bk_write += 1;
		if (cprtd->bk_write >= runtime->fragments)
			cprtd->bk_write -= runtime->fragments;
		bk_info = &cprtd->cap_block_info[cprtd->bk_write];
		bk_info->size = ptrmem.payload.buf_info.size;
		bk_info->byte_avail = ptrmem.payload.buf_info.size;
		bk_info->byte_copied = 0;
		cprtd->received_total += ptrmem.payload.buf_info.size;
		snd_compr_fragment_elapsed(stream);
	} else if (payload.opCode == OP_GET_PARAMS && stream->direction == SND_COMPRESS_PLAYBACK) {
		cprtd->samp_rate = ptrmem.payload.sfmt.samples;
		AUD_LOG_DBG(AUD_SOC, "cprtd->samp_rate=%u", cprtd->samp_rate);
	}

}

static void compress_capture_pre_ssr(void *priv)
{
	struct fk_compr_audio *cprtd = priv;
	struct snd_compr_stream *stream = cprtd->stream;

	AUD_LOG_INFO(AUD_SOC, "compress capture pre ssr enter");
	stream->runtime->state = SNDRV_PCM_STATE_XRUN;
	snd_compr_fragment_elapsed(stream);
	AUD_LOG_INFO(AUD_SOC, "compress capture pre ssr exit");
}

static struct recovery_ops compress_capture_ssr_ops = {
	.pre_recovery = compress_capture_pre_ssr,
	.post_recovery = NULL,
};

static void populate_record_codec_list(struct fk_compr_audio *prtd)
{
	prtd->compr_cap.direction = SND_COMPRESS_CAPTURE;
	prtd->compr_cap.min_fragment_size =
			COMPR_RECORD_MIN_FRAGMENT_SIZE;
	prtd->compr_cap.max_fragment_size =
			COMPR_RECORD_MAX_FRAGMENT_SIZE;
	prtd->compr_cap.min_fragments =
			COMPR_RECORD_MIN_NUM_FRAGMENTS;
	prtd->compr_cap.max_fragments =
			COMPR_RECORD_MAX_NUM_FRAGMENTS;
	prtd->compr_cap.num_codecs = 3;
	prtd->compr_cap.codecs[0] = SND_AUDIOCODEC_AMR;
	prtd->compr_cap.codecs[1] = SND_AUDIOCODEC_AMRWB;
	prtd->compr_cap.codecs[2] = SND_AUDIOCODEC_AAC;
}

int fk_compr_new(struct snd_soc_pcm_runtime *rtd, int num)
{
	int ret = 0;

	if (rtd == NULL) {
		AUD_LOG_ERR(AUD_SOC, "RTD is NULL");
		return 0;
	}
	ret = snd_soc_new_compress(rtd, num);
	if (ret)
		AUD_LOG_ERR(AUD_SOC, "Fail to create pcm for compress");

	return ret;
}
EXPORT_SYMBOL(fk_compr_new);

static int fk_compr_playback_open(struct snd_compr_stream *stream)
{
	int ret = 0;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct snd_soc_pcm_runtime *rtd = stream->private_data;
	struct snd_soc_component *component = NULL;
	struct fk_compr_audio *cprtd = &offload_info;

	AUD_LOG_INFO(AUD_SOC, "buffer_size=%llu", runtime->buffer_size);
	component = snd_soc_rtdcom_lookup(rtd, DRV_NAME);
	if (!component) {
		AUD_LOG_ERR(AUD_SOC, "component is NULL");
		return -EINVAL;
	}

	cprtd->stream = stream;

	cprtd->codec = SND_AUDIOCODEC_MP3;
	cprtd->pos = 0;
	cprtd->index = 0;
	cprtd->samp_rate = 48000;
	cprtd->bits = 16;
	cprtd->copied_total = 0;
	cprtd->num_channels = 2;
	cprtd->first_buffer = 1;
	cprtd->last_buffer = 0;
	cprtd->marker_timestamp = 0;
	//cprtd->session_id = get_session_id(stream);
	cprtd->session_id = get_compr_session_id(stream);
	runtime->private_data = cprtd;
	spin_lock_init(&cprtd->lock);
	AUD_LOG_INFO(AUD_SOC, "sess_id = 0x%x", cprtd->session_id);
	ret = fk_acore_register(cprtd->session_id, (fk_acore_cb)compr_event_handler, cprtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_compr_capture_open(struct snd_compr_stream *stream)
{
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = &offload_info;
	int ret = 0;

	cprtd->stream = stream;
	//cprtd->session_id = get_session_id(stream);
	cprtd->session_id = get_compr_session_id(stream);
	cprtd->bk_read = 0;
	cprtd->bk_write = 0;
	cprtd->received_total = 0;
	cprtd->bytes_copied = 0;
	cprtd->bits = 16;
	runtime->private_data = cprtd;
	spin_lock_init(&cprtd->lock);

	populate_record_codec_list(cprtd);

	ret = fk_acore_register(cprtd->session_id, (fk_acore_cb)compr_event_handler, cprtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}

	cprtd->capture_ssr = fk_adsp_ssr_register(&compress_capture_ssr_ops, cprtd);

	return 0;
}

static int fk_compr_open(struct snd_soc_component *component,
		    struct snd_compr_stream *stream)
{
	int ret = 0;

	if (stream->direction == SND_COMPRESS_PLAYBACK)
		ret = fk_compr_playback_open(stream);
	else if (stream->direction == SND_COMPRESS_CAPTURE)
		ret = fk_compr_capture_open(stream);

	return ret;
}

static int fk_compr_free(struct snd_soc_component *component,
		    struct snd_compr_stream *stream)
{
	int ret = 0;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;

	AUD_LOG_INFO(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr=0x%llx,area=0x%llx",
				cprtd->session_id, runtime->dma_bytes,
				runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_unregister(cprtd->session_id);
	fk_acore_smem_free(component->dev, runtime->dma_bytes,
							runtime->dma_area, runtime->dma_addr);
	if (stream->direction == SND_COMPRESS_CAPTURE)
		fk_adsp_ssr_unregister(cprtd->capture_ssr);

	return ret;
}

static int fk_compr_trigger(struct snd_soc_component *component,
		       struct snd_compr_stream *stream, int cmd)
{
	int ret = 0;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		cprtd->first_buffer = 0;
		ret = fk_acore_trigger(cprtd->session_id, OP_START);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "trigger start failed");
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		if (stream->direction == SND_COMPRESS_PLAYBACK) {
			ret = fk_acore_trigger(cprtd->session_id, OP_PAUSE);
			if (ret < 0)
				AUD_LOG_ERR(AUD_SOC, "trigger pause failed");
			ret = fk_acore_trigger(cprtd->session_id, OP_FLUSH);
			offload_info.copied_total = 0;
			offload_info.index = 0;
			if (ret < 0)
				AUD_LOG_ERR(AUD_SOC, "trigger flush failed");
		} else if (stream->direction == SND_COMPRESS_CAPTURE) {
			ret = fk_acore_trigger(cprtd->session_id, OP_STOP);
			if (ret < 0)
				AUD_LOG_INFO(AUD_SOC, "trigger stop failed");
		}
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		ret = fk_acore_trigger(cprtd->session_id, OP_PAUSE);
		if (ret < 0)
			AUD_LOG_INFO(AUD_SOC, "trigger pause push failed");
		break;
	case SND_COMPR_TRIGGER_DRAIN:
		snd_compr_use_pause_in_draining(stream);
		ret = fk_acore_trigger(cprtd->session_id, OP_DRAIN);
		if (ret < 0)
			AUD_LOG_INFO(AUD_SOC, "trigger drain failed");
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int fk_compr_pointer(struct snd_soc_component *component,
		       struct snd_compr_stream *stream,
		       struct snd_compr_tstamp *tstamp)
{
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;
	struct snd_compr_tstamp compr_tstamp;
	struct stream_params s_param;
	unsigned long flags;
	int ret = 0;

	memset(&compr_tstamp, 0x0, sizeof(struct snd_compr_tstamp));
	memset(&s_param, 0x0, sizeof(struct stream_params));

	spin_lock_irqsave(&cprtd->lock, flags);
	compr_tstamp.sampling_rate = cprtd->samp_rate;

	if (stream->direction == SND_COMPRESS_PLAYBACK) {
		compr_tstamp.copied_total = cprtd->copied_total;
		AUD_LOG_INFO_LIM(AUD_SOC, "ctstamp->copied_total=%d",
			compr_tstamp.copied_total);
		if (!cprtd->first_buffer) {
			spin_unlock_irqrestore(&cprtd->lock, flags);
			s_param.type = PARAMS_GET_TIMESTAMP;
			fk_acore_get_params(cprtd->session_id, &s_param);
			/* DSP returns timestamp in ms */
			cprtd->marker_timestamp = s_param.params.value * cprtd->samp_rate;
			compr_tstamp.pcm_io_frames =
				(snd_pcm_uframes_t)div64_u64(cprtd->marker_timestamp, 1000);
			spin_lock_irqsave(&cprtd->lock, flags);
		}
	} else if (stream->direction == SND_COMPRESS_CAPTURE) {
		runtime->total_bytes_available = cprtd->received_total;
		compr_tstamp.copied_total = cprtd->received_total;
		spin_unlock_irqrestore(&cprtd->lock, flags);
		s_param.type = PARAMS_GET_TIMESTAMP;
		fk_acore_get_params(cprtd->session_id, &s_param);
		/* DSP returns timestamp in ms */
		cprtd->marker_timestamp = s_param.params.value * cprtd->samp_rate;
		compr_tstamp.pcm_io_frames =
			(snd_pcm_uframes_t)div64_u64(cprtd->marker_timestamp, 1000);
		AUD_LOG_DBG(AUD_SOC, "pcm_io_frames 0x%x", compr_tstamp.pcm_io_frames);
		spin_lock_irqsave(&cprtd->lock, flags);
	}

	spin_unlock_irqrestore(&cprtd->lock, flags);

	memcpy(tstamp, &compr_tstamp, sizeof(struct snd_compr_tstamp));

	return ret;

}

static int fk_compr_configure_dsp_for_playback(struct snd_compr_stream *stream)
{
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	int ret = 0;

	st_format.samples = cprtd->samp_rate;
	st_format.channels = cprtd->num_channels;
	st_format.bit_width = cprtd->bits;
	st_format.codec_type = cprtd->codec;
	smem.p_addr = runtime->dma_addr;
	smem.vir_addr = runtime->dma_area;
	smem.periods_size = runtime->fragment_size;
	smem.periods_num = runtime->fragments;
	ret = fk_acore_startup(cprtd->session_id, st_format, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
	return ret;

}

static int fk_compr_configure_dsp_for_capture(struct snd_compr_stream *stream)
{
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct block_info *capture_bk_info = NULL;
	struct buf_blk p_buf;
	int ret = 0, index = 0;

	for (; index < runtime->fragments; index++) {
		capture_bk_info = &cprtd->cap_block_info[index];
		capture_bk_info->bk_num = index;
		capture_bk_info->bk_p_addr = runtime->dma_addr +
			capture_bk_info->bk_num * runtime->fragment_size;
		capture_bk_info->bk_vir_addr = runtime->dma_area +
			capture_bk_info->bk_num * runtime->fragment_size;
		capture_bk_info->size = 0;
		capture_bk_info->byte_avail = 0;
		capture_bk_info->byte_copied = 0;
	}

	st_format.samples = cprtd->samp_rate;
	st_format.channels = cprtd->num_channels;
	st_format.bit_width = cprtd->bits;
	st_format.codec_type = cprtd->codec;
	smem.p_addr = runtime->dma_addr;
	smem.vir_addr = runtime->dma_area;
	smem.periods_size = runtime->fragment_size;
	smem.periods_num = runtime->fragments;

	ret = fk_acore_startup(cprtd->session_id, st_format, smem);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);

	AUD_LOG_INFO(AUD_SOC, "dma_addr=0x%llx,dma_area=0x%llx, session_id=0x%x",
		runtime->dma_addr, (uint64_t)runtime->dma_area, cprtd->session_id);
	p_buf.vir_addr = runtime->dma_area;
	p_buf.phy_addr = runtime->dma_addr;
	p_buf.result = 0;
	p_buf.size = runtime->fragment_size * runtime->fragments;
	ret = fk_acore_capture_copy(cprtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore capture copy failed %d", ret);
		return ret;
	}

	return ret;
}

static int fk_compr_capture_param_valid(struct snd_compr_params *params)
{
	int i = 0, index = 0;
	struct support_format *format;

	for (; i < sizeof(g_compress_capture_support_format) / sizeof(struct support_format); i++) {
		format = &g_compress_capture_support_format[i];
		if (params->codec.id == format->codec_id) {
			if (params->codec.ch_in > format->ch) {
				AUD_LOG_ERR(AUD_SOC, "codec channel invalid");
				return -EINVAL;
			}

			for (; index < format->num_sample_rate; index++) {
				if (params->codec.sample_rate == format->sample_rate[index])
					break;
			}
			if (index == format->num_sample_rate) {
				AUD_LOG_ERR(AUD_SOC, "codec samplerate invalid");
				return -EINVAL;
			}
			break;
		}
	}

	if (i == sizeof(g_compress_capture_support_format) / sizeof(struct support_format)) {
		AUD_LOG_ERR(AUD_SOC, "codec type is not support");
		return -EINVAL;
	}

	return 0;
}

static int fk_compr_set_params(struct snd_soc_component *component,
			  struct snd_compr_stream *stream,
			  struct snd_compr_params *params)
{
	int ret = 0;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;
	struct snd_dma_buffer *dma_buffer = &stream->dma_buffer;

	if (stream->direction == SND_COMPRESS_CAPTURE) {
		ret = fk_compr_capture_param_valid(params);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_SOC, "check compress capture failed");
			return ret;
		}
	}

	memcpy(&cprtd->codec_param, params, sizeof(struct snd_compr_params));
	cprtd->bk_write = runtime->fragments - 1;
	cprtd->codec = cprtd->codec_param.codec.id;
	cprtd->num_channels = cprtd->codec_param.codec.ch_in;
	cprtd->samp_rate = cprtd->codec_param.codec.sample_rate;
	AUD_LOG_INFO(AUD_SOC, "sample_rate = %d", cprtd->samp_rate);
	AUD_LOG_INFO(AUD_SOC, "codec_id = %d", cprtd->codec_param.codec.id);

	AUD_LOG_INFO(AUD_SOC, "fragment_size=%d,fragments=%d,buffer_size=%llu",
		runtime->fragment_size, runtime->fragments, runtime->buffer_size);
	dma_buffer->bytes = runtime->buffer_size;
	dma_buffer->area = fk_acore_smem_alloc(component->dev,
							&dma_buffer->addr, dma_buffer->bytes);
	if (dma_buffer->area == NULL) {
		AUD_LOG_ERR(AUD_SOC, "v_addr=0x%llx p_addr=0x%llx",
			(uint64_t)dma_buffer->area, dma_buffer->addr);
		return -ENOMEM;
	}
	snd_compr_set_runtime_buffer(stream, &stream->dma_buffer);
	AUD_LOG_INFO(AUD_SOC, "sess_id = 0x%x,bytes=%zu, addr= 0x%llx,area = 0x%llx\n",
			cprtd->session_id, runtime->dma_bytes,
			runtime->dma_addr, (uint64_t)runtime->dma_area);

	if (stream->direction == SND_COMPRESS_PLAYBACK)
		ret = fk_compr_configure_dsp_for_playback(stream);
	else if (stream->direction == SND_COMPRESS_CAPTURE)
		ret = fk_compr_configure_dsp_for_capture(stream);

	return ret;
}

static int fk_compr_playback_copy(struct snd_compr_stream *stream, char __user *buf,
		    size_t count)
{
	int ret = 0;
	struct buf_blk p_buf;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *cprtd = runtime->private_data;

	if (count == runtime->buffer_size) {
		cprtd->pos = 0;
	} else {
		cprtd->pos = cprtd->index * runtime->fragment_size;
		cprtd->index++;
		if (cprtd->index >= runtime->fragments)
			cprtd->index = 0;
	}

	p_buf.vir_addr = runtime->dma_area + cprtd->pos;
	p_buf.phy_addr = runtime->dma_addr + cprtd->pos;
	p_buf.size = count;
	p_buf.result = 0;

	AUD_LOG_INFO(AUD_SOC, "write %zu bytes of buffer to adsp 0x%llx",
		count, (uint64_t)p_buf.vir_addr);
	if (copy_from_user(p_buf.vir_addr, buf, count)) {
		ret = -EFAULT;
		AUD_LOG_ERR(AUD_SOC, "copy_from_user failed");
		return ret;
	}
	ret = fk_acore_playback_copy(cprtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore playback copy failed %d", ret);
		return ret;
	}
	return count;
}

static int fk_compr_capture_copy(struct snd_compr_stream *stream, char __user *buf,
		    size_t count)
{
	int ret = 0;
	struct snd_compr_runtime *runtime = stream->runtime;
	struct fk_compr_audio *prtd = runtime->private_data;
	uint32_t to_read;
	struct block_info *bk_info;
	struct buf_blk p_buf = {0};

	if (count > (prtd->received_total - prtd->bytes_copied)) {
		/* update read buffer */
		return 0;
	}

	bk_info = &prtd->cap_block_info[prtd->bk_read];
	AUD_LOG_DBG_LIM(AUD_SOC,
		"read %d, vir 0x%llx, num %d, size 0x%x, avail 0x%x, copied 0x%x, count %lu",
		prtd->bk_read, (uint64_t)bk_info->bk_vir_addr, bk_info->bk_num,
		bk_info->size, bk_info->byte_avail, bk_info->byte_copied, count);

	to_read = (count >= bk_info->byte_avail) ? bk_info->byte_avail : count;

	ret = copy_to_user(buf, bk_info->bk_vir_addr + bk_info->byte_copied, to_read);
	if (ret) {
		ret = -EFAULT;
		AUD_LOG_ERR(AUD_SOC, "usr_addr 0x%llx,vir_addr 0x%llx,read_num %d",
			(uint64_t)buf, (uint64_t)(bk_info->bk_vir_addr +
			bk_info->byte_copied), to_read);
		return ret;
	}

	if (count >= bk_info->byte_avail) {
		p_buf.phy_addr = bk_info->bk_p_addr;
		p_buf.vir_addr = bk_info->bk_vir_addr;
		p_buf.size = runtime->fragment_size;
		ret = fk_acore_capture_copy(prtd->session_id, p_buf);

		prtd->bk_read += 1;
		if (prtd->bk_read == runtime->fragments)
			prtd->bk_read = 0;
	}

	bk_info->byte_avail -= to_read;
	bk_info->byte_copied += to_read;
	prtd->bytes_copied += to_read;

	return to_read;
}

static int fk_compr_copy(struct snd_soc_component *component,
		    struct snd_compr_stream *stream, char __user *buf,
		    size_t count)
{
	int ret = 0;

	if (stream->direction == SND_COMPRESS_PLAYBACK)
		ret = fk_compr_playback_copy(stream, buf, count);
	else if (stream->direction == SND_COMPRESS_CAPTURE)
		ret = fk_compr_capture_copy(stream, buf, count);
	return ret;
}

static int fk_compr_get_caps(struct snd_soc_component *component,
			struct snd_compr_stream *cstream, struct snd_compr_caps *arg)
{
	struct snd_compr_runtime *runtime = cstream->runtime;
	struct fk_compr_audio *prtd = runtime->private_data;
	int ret = 0;

	if ((arg != NULL) && (prtd != NULL)) {
		memcpy(arg, &prtd->compr_cap, sizeof(struct snd_compr_caps));
	} else {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_SOC, "arg (0x%pK), prtd (0x%pK)", arg, prtd);
	}

	return ret;
}

static int fk_offload_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = offload_info.volume;

	return 0;
}

static int fk_offload_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int volume_index = 0;

	volume_index = ucontrol->value.integer.value[0];
	ret = fk_acore_set_vol(offload_info.session_id, volume_index);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "set offload play vol failed");
	else
		offload_info.volume = volume_index;

	AUD_LOG_INFO(AUD_COMM, "current offload play volume = %d", offload_info.volume);
	return 1;
}


static const struct snd_kcontrol_new fk_audio_offload_controls[] = {
	SOC_SINGLE_EXT("offload playback volume", SND_SOC_NOPM, 0, VOLUME_VALUE_MAX, 0,
		fk_offload_volume_get, fk_offload_volume_put),
};

static int fk_compr_probe(struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component, fk_audio_offload_controls,
		ARRAY_SIZE(fk_audio_offload_controls));

	return 0;
}

static struct snd_compress_ops fk_compress_ops = {
	.open			= fk_compr_open,
	.free			= fk_compr_free,
	.trigger		= fk_compr_trigger,
	.pointer		= fk_compr_pointer,
	.set_params		= fk_compr_set_params,
	.copy			= fk_compr_copy,
	.get_caps		= fk_compr_get_caps,
};

const struct snd_soc_component_driver fk_soc_compr_component = {
	.name			= DRV_NAME,
	.probe			= fk_compr_probe,
	.compress_ops	= &fk_compress_ops,
};

static int fk_compress_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev,
					&fk_soc_compr_component,
					NULL, 0);
}

static int fk_compress_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_compress_dt_match[] = {
	{.compatible = "xring,fk-compress-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_compress_dt_match);

static struct platform_driver fk_compress_driver = {
	.driver = {
		.name = "fk-compress-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_compress_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_compress_probe,
	.remove = fk_compress_remove,
};
int __init fk_compress_dsp_init(void)
{

	return platform_driver_register(&fk_compress_driver);
}

void fk_compress_dsp_exit(void)
{
	platform_driver_unregister(&fk_compress_driver);
}

MODULE_DESCRIPTION("compress module platform driver");
MODULE_LICENSE("Dual BSD/GPL");
