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
#include <linux/mm.h>
#include <linux/dma-direction.h>

#include "../acore/fk-acore-api.h"
#include "../common/fk-audio-cma.h"
#include "../common/fk-audio-log.h"

#include "fk-mmap.h"
#define DRV_NAME "fk-mmap"

static struct snd_pcm_hardware fk_mmap_hardware_capture = {
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
	.buffer_bytes_max =     MMAP_CAPTURE_MAX_NUM_PERIODS *
				MMAP_CAPTURE_MAX_PERIOD_SIZE,
	.period_bytes_min =	MMAP_CAPTURE_MIN_PERIOD_SIZE,
	.period_bytes_max =     MMAP_CAPTURE_MAX_PERIOD_SIZE,
	.periods_min =          MMAP_CAPTURE_MIN_NUM_PERIODS,
	.periods_max =          MMAP_CAPTURE_MAX_NUM_PERIODS,
	.fifo_size =            0,
};

static struct snd_pcm_hardware fk_mmap_hardware_playback = {
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
	.buffer_bytes_max =     MMAP_PLAYBACK_MAX_NUM_PERIODS *
				MMAP_PLAYBACK_MAX_PERIOD_SIZE,
	.period_bytes_min =	MMAP_PLAYBACK_MIN_PERIOD_SIZE,
	.period_bytes_max =     MMAP_PLAYBACK_MAX_PERIOD_SIZE,
	.periods_min =          MMAP_PLAYBACK_MIN_NUM_PERIODS,
	.periods_max =          MMAP_PLAYBACK_MAX_NUM_PERIODS,
	.fifo_size =            0,
};

static struct fk_mmap mmap_info[2];

static int fk_mmap_playback_shm_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = MMAP_MEMORY_ELEMENT_NUM;

	return 0;
}

static int fk_mmap_playback_shm_get(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = mmap_info[0].mem_info.fd;
	ucontrol->value.integer.value[1] = mmap_info[0].mem_info.size;

	AUD_LOG_DBG(AUD_SOC, "mmap_data_blk:%d, %d",
			mmap_info[0].mem_info.fd, mmap_info[0].mem_info.size);
	return 0;
}

static int fk_mmap_record_shm_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = MMAP_MEMORY_ELEMENT_NUM;

	return 0;
}

static int fk_mmap_record_shm_get(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = mmap_info[1].mem_info.fd;
	ucontrol->value.integer.value[1] = mmap_info[1].mem_info.size;

	AUD_LOG_DBG(AUD_SOC, "mmap_data_blk:%d, %d",
			mmap_info[1].mem_info.fd, mmap_info[1].mem_info.size);
	return 0;
}

static int fk_mmap_dl_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = mmap_info[0].volume;

	return 0;
}

static int fk_mmap_dl_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int volume_index = 0;

	volume_index = ucontrol->value.integer.value[0];
	ret = fk_acore_set_vol(mmap_info[0].session_id, volume_index);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "set mmap playback vol failed");
	else
		mmap_info[0].volume = volume_index;

	AUD_LOG_INFO(AUD_COMM, "current mmap playback volume = %d", mmap_info[0].volume);
	return 1;
}

static const struct snd_kcontrol_new fk_mmap_controls[] = {
	{
		.access = (SNDRV_CTL_ELEM_ACCESS_READWRITE |
			   SNDRV_CTL_ELEM_ACCESS_INACTIVE),
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "XR Mmap Playback",
		.info = fk_mmap_playback_shm_info,
		.get = fk_mmap_playback_shm_get,
	},
	{
		.access = (SNDRV_CTL_ELEM_ACCESS_READWRITE |
			   SNDRV_CTL_ELEM_ACCESS_INACTIVE),
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "XR Mmap Record",
		.info = fk_mmap_record_shm_info,
		.get = fk_mmap_record_shm_get,
	},
	SOC_SINGLE_EXT("MMAP PLAYBACK VOLUME", SND_SOC_NOPM, 0, VOLUME_VALUE_MAX, 0,
		fk_mmap_dl_volume_get, fk_mmap_dl_volume_put),
};

static void mmap_event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	struct fk_mmap *prtd = priv;
	struct snd_pcm_substream *substream = prtd->substream;
	struct rsp_payload ptrmem = payload;

	AUD_LOG_DBG_LIM(AUD_SOC, "opcode=%d,payload result=%d,buf_info result=%d",
		ptrmem.opCode, ptrmem.payload.result, ptrmem.payload.buf_info.result);
	if ((payload.opCode == OP_PLAYBACK_COPY) || (payload.opCode == OP_CAPTURE_COPY)) {
		prtd->pcm_irq_pos += prtd->pcm_count;
		snd_pcm_period_elapsed(substream);
	}
}

static snd_pcm_uframes_t fk_mmap_pointer(struct snd_soc_component *component,
				     struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = runtime->private_data;
	struct stream_params s_param;
	uint32_t offset = 0;

	memset(&s_param, 0x0, sizeof(struct stream_params));

	/*get hw_ptr*/
	s_param.type = PARAMS_MMAP_HW_PRT;

	offset = fk_acore_get_params(prtd->session_id, &s_param);
	offset %= runtime->periods;
	prtd->pcm_irq_pos = offset * prtd->pcm_count;

	AUD_LOG_DBG_LIM(AUD_SOC, "offset %d, runtime->periods %d , pcm_irq_pos=%d", offset, runtime->periods, prtd->pcm_irq_pos);

	return bytes_to_frames(runtime, (prtd->pcm_irq_pos));
}


static int fk_mmap_playback_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = runtime->private_data;

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

static int fk_mmap_capture_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = runtime->private_data;

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

	return ret;
}

static int fk_mmap_probe(struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component, fk_mmap_controls, ARRAY_SIZE(fk_mmap_controls));

	return 0;
}

static int fk_mmap_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = NULL;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		runtime->hw = fk_mmap_hardware_playback;
		prtd = &mmap_info[0];
	}
	/* Capture path */
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		runtime->hw = fk_mmap_hardware_capture;
		prtd = &mmap_info[1];
	} else {
		AUD_LOG_ERR(AUD_SOC, "Invalid Stream type %d", substream->stream);
		return -EINVAL;
	}

	prtd->substream = substream;
	runtime->private_data = prtd;
	return ret;
}

static int fk_mmap_prepare(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream)
{
	int ret = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		ret = fk_mmap_playback_prepare(substream);
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		ret = fk_mmap_capture_prepare(substream);
	return ret;
}

static int fk_mmap_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = runtime->private_data;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;
	size_t pcm_byte = 0;

	prtd->session_id = get_mmap_session_id(substream);
	/* mmap mem alloc */
	pcm_byte = params_buffer_bytes(params);
	dma_buf->bytes = PAGE_ALIGN(pcm_byte);

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
	ret = fk_acore_register(prtd->session_id, (fk_acore_cb)mmap_event_handler, prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}
	prtd->addr = runtime->dma_addr;
	prtd->open_status = true;
	return ret;
}

int fk_mmap_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = runtime->private_data;

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
			AUD_LOG_ERR(AUD_SOC, "trigger start failed");
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

static int fk_mmap_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_mmap *prtd = runtime->private_data;

	if (!prtd->open_status) {
		AUD_LOG_ERR(AUD_SOC, "mmap stream not start but close");
		return 0;
	}

	AUD_LOG_DBG(AUD_SOC, "sess_id=0x%x,bytes=%zu,addr=0x%llx,area=0x%llx",
		prtd->session_id, runtime->dma_bytes,
		runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_unregister(prtd->session_id);
	/*free audio share memory*/
	fk_acore_smem_free(component->dev, runtime->dma_bytes,
						runtime->dma_area, runtime->dma_addr);
	prtd->open_status = false;
	return ret;
}

int mmap_plaback_dmabuf(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	return remap_pfn_range(vma, vma->vm_start, mmap_info[0].addr >> PAGE_SHIFT,
		dmabuf->size, vma->vm_page_prot);
}

int mmap_capture_dmabuf(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	return remap_pfn_range(vma, vma->vm_start, mmap_info[1].addr >> PAGE_SHIFT,
		dmabuf->size, vma->vm_page_prot);
}

int vmap_dmabuf(struct dma_buf *dmabuf, struct iosys_map *map)
{
	AUD_LOG_DBG(AUD_SOC, "ENTER");
	return 0;
}

struct sg_table *map_dma_buf(struct dma_buf_attachment *attach,
					 enum dma_data_direction direction)
{
	return 0;
}


void unmap_dma_buf(struct dma_buf_attachment *attach,
			      struct sg_table *sg_table,
			      enum dma_data_direction direction)
{
	AUD_LOG_DBG(AUD_SOC, "ENTER");
}

int mmap_pin(struct dma_buf_attachment *attach)
{
	AUD_LOG_DBG(AUD_SOC, "ENTER");
	return 0;
}
void mmap_unpin(struct dma_buf_attachment *attach)
{
	AUD_LOG_DBG(AUD_SOC, "ENTER");
}

void release_dmabuf(struct dma_buf *dmabuf)
{
	AUD_LOG_DBG(AUD_SOC, "ENTER");
}

static const struct dma_buf_ops mmap_playback_dmabuf_ops = {
	.cache_sgt_mapping = true,
	.map_dma_buf	   = map_dma_buf,
	.unmap_dma_buf	   = unmap_dma_buf,
	.release	   = release_dmabuf,
	.mmap		   = mmap_plaback_dmabuf,
	.vmap		   = vmap_dmabuf,
};

static const struct dma_buf_ops mmap_capture_dmabuf_ops = {
	.cache_sgt_mapping = true,
	.map_dma_buf	   = map_dma_buf,
	.unmap_dma_buf	   = unmap_dma_buf,
	.release	   = release_dmabuf,
	.mmap		   = mmap_capture_dmabuf,
	.vmap		   = vmap_dmabuf,
};

static int fk_mmap_memory_alloc(struct snd_soc_component *component,
		    struct snd_pcm_substream *substream,
		    struct vm_area_struct *vma)
{
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;
	struct dma_buf *dmabuf;
	int ret = 0, fd = 0;

	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	/* create the dmabuf */
	exp_info.size = dma_buf->bytes;
	exp_info.priv = dma_buf->area;
	exp_info.flags = O_RDWR;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		exp_info.ops  = &mmap_playback_dmabuf_ops;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		exp_info.ops  = &mmap_capture_dmabuf_ops;

	dmabuf = dma_buf_export(&exp_info);
	fd = dma_buf_fd(dmabuf, O_CLOEXEC);
	if (fd < 0) {
		AUD_LOG_ERR(AUD_SOC, "dma_buf_fd failed, fd:%d\n", fd);
		ret = -EINVAL;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		mmap_info[0].mem_info.size = dma_buf->bytes;
		mmap_info[0].mem_info.fd = fd;
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		mmap_info[1].mem_info.size = dma_buf->bytes;
		mmap_info[1].mem_info.fd = fd;
	}
	AUD_LOG_DBG(AUD_SOC, "fd=%d, size=%d", fd, (uint32_t)dma_buf->bytes);
	return ret;
}

const struct snd_soc_component_driver fk_soc_mmap_component = {
	.name			= DRV_NAME,
	.probe			= fk_mmap_probe,
	.open			= fk_mmap_open,
	.prepare		= fk_mmap_prepare,
	.hw_params		= fk_mmap_hw_params,
	.trigger		= fk_mmap_trigger,
	.close			= fk_mmap_close,
	.mmap			= fk_mmap_memory_alloc,
	.pointer		= fk_mmap_pointer,
};

static int fk_pcm_mmap_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev,
					&fk_soc_mmap_component,
					NULL, 0);
}
static int fk_pcm_mmap_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_mmap_dt_match[] = {
	{.compatible = "xring,fk-mmap-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_mmap_dt_match);

static struct platform_driver fk_mmap_driver = {
	.driver = {
		.name = "fk-mmap-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_mmap_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_pcm_mmap_probe,
	.remove = fk_pcm_mmap_remove,
};

int __init fk_mmap_dsp_init(void)
{

	return platform_driver_register(&fk_mmap_driver);
}

void fk_mmap_dsp_exit(void)
{
	platform_driver_unregister(&fk_mmap_driver);
}

MODULE_DESCRIPTION("MMAP module platform driver");
MODULE_LICENSE("Dual BSD/GPL");

