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
#include <linux/version.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/timer.h>
#include <asm/dma.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>

#include <sound/hwdep.h>
#include <linux/of_device.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>

#include "../common/fk-audio-log.h"
#include "fk-lsm.h"
#define DRV_NAME "fk-lsm"
static int kws_status;
static struct fk_audio_lsm lsm_info = {0};

static struct snd_pcm_hardware fk_lsm_hardware_listen = {
	.info =	(SNDRV_PCM_INFO_BLOCK_TRANSFER |
		 SNDRV_PCM_INFO_MMAP_VALID |
		 SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_PAUSE |
		 SNDRV_PCM_INFO_RESUME),
	.formats = (SNDRV_PCM_FMTBIT_S16_LE |
		    SNDRV_PCM_FMTBIT_S24_LE |
		    SNDRV_PCM_FMTBIT_S32_LE),
	.rates = (SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_48000 |
		  SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_384000),
	.rate_min = 8000,
	.rate_max = 384000,
	.channels_min =	1,
	.channels_max =	8,
	.buffer_bytes_max = LISTEN_MAX_NUM_PERIODS *
			    LISTEN_MAX_PERIOD_SIZE,
	.period_bytes_min = LISTEN_MIN_PERIOD_SIZE,
	.period_bytes_max = LISTEN_MAX_PERIOD_SIZE,
	.periods_min = LISTEN_MIN_NUM_PERIODS,
	.periods_max = LISTEN_MAX_NUM_PERIODS,
	.fifo_size = 0,
};

static void lsm_event_handler(uint16_t session_id, void *priv, struct rsp_payload payload)
{
	struct fk_audio_lsm *lsm_prtd = priv;
	struct snd_pcm_substream *substream = lsm_prtd->substream;
	struct rsp_payload ptrmem = payload;

	AUD_LOG_DBG_LIM(AUD_SOC, "opcode=%d,buf_info.size=%d,buf_info pa=%llu",
		ptrmem.opCode, ptrmem.payload.buf_info.size, ptrmem.payload.buf_info.phy_addr);
	if (payload.opCode == OP_CAPTURE_COPY) {
		lsm_prtd->pcm_irq_pos += ptrmem.payload.buf_info.size;
		lsm_prtd->pa = ptrmem.payload.buf_info.phy_addr;
		lsm_prtd->size = ptrmem.payload.buf_info.size;
		snd_pcm_period_elapsed(substream);
	}
}

static int fk_lsm_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = &lsm_info;

	substream->wait_time = PCM_WAIT_TIME;
	lsm_prtd->substream = substream;
	runtime->hw = fk_lsm_hardware_listen;
	runtime->private_data = lsm_prtd;
	mutex_init(&lsm_prtd->lsm_lock);

	return ret;
}

static int fk_lsm_prepare(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	int ret = 0;
	uint16_t bits_per_sample = 16;
	struct stream_format st_format = {0};
	struct share_mem smem = {0};
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;
	struct buf_blk p_buf;

	lsm_prtd->pcm_size = snd_pcm_lib_buffer_bytes(substream);
	lsm_prtd->pcm_count = snd_pcm_lib_period_bytes(substream);
	lsm_prtd->pcm_irq_pos = 0;
	/* rate and channels are sent to audio driver */
	lsm_prtd->samp_rate = runtime->rate;
	lsm_prtd->channel_mode = runtime->channels;
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
	smem.vir_addr = runtime->dma_area;
	smem.p_addr = runtime->dma_addr;
	smem.periods_size = runtime->period_size;
	smem.periods_num = runtime->periods;
	ret = fk_acore_startup(lsm_prtd->session_id, st_format, smem);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore startup failed %d", ret);
		return ret;
	}

	AUD_LOG_DBG(AUD_SOC, "dma_addr = 0x%llx, dma_area=0x%llx",
		runtime->dma_addr, (uint64_t)runtime->dma_area);
	p_buf.vir_addr = runtime->dma_area;
	p_buf.phy_addr = runtime->dma_addr;
	p_buf.result = 0;
	p_buf.size = runtime->period_size * runtime->periods *
		runtime->channels * bits_per_sample / 8;
	ret = fk_acore_capture_copy(lsm_prtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore capture copy failed %d", ret);
		return ret;
	}
	return ret;

}

static int fk_lsm_copy(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream, int channel,
			 unsigned long pos, struct iov_iter *iter,
			 unsigned long bytes)
{
	int ret = 0;
	void __user *buf = iter->ubuf;
	struct buf_blk p_buf;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;

	if (!lsm_info.start_stop_rec) {
		pos = lsm_info.count;
		lsm_info.count += bytes;
		if (lsm_info.count >= substream->dma_buffer.bytes)
			lsm_info.count = 0;
	}

//	AUD_LOG_DBG(AUD_SOC, "channel=%d,pos=%lu,bytes=%lu",
//			channel, pos, bytes);
	p_buf.vir_addr = runtime->dma_area + pos;
	p_buf.phy_addr = runtime->dma_addr + pos;
	p_buf.size = bytes;
	p_buf.result = 0;

	AUD_LOG_DBG_LIM(AUD_SOC, "read %lu bytes of buffer from adsp 0x%llx ", bytes, (uint64_t)p_buf.vir_addr);
	if (copy_to_user(buf, p_buf.vir_addr, bytes)) {
		ret = -EFAULT;
		AUD_LOG_ERR(AUD_SOC, "copy_from_user failed");
		return ret;
	}
	ret = fk_acore_capture_copy(lsm_prtd->session_id, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore capture copy failed %d", ret);
		return ret;
	}
	return ret;
}

static snd_pcm_uframes_t fk_lsm_pointer(struct snd_soc_component *component,
				     struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;

	if (lsm_prtd->pcm_irq_pos >= lsm_prtd->pcm_size)
		lsm_prtd->pcm_irq_pos = 0;

	return bytes_to_frames(runtime, (lsm_prtd->pcm_irq_pos));
}

static int fk_lsm_hw_params(struct snd_soc_component *component,
			 struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;
	struct snd_dma_buffer *dma_buf = &substream->dma_buffer;

	lsm_prtd->session_id = get_pcm_session_id(substream);/*get session id*/
	dma_buf->bytes = params_buffer_bytes(params);/*valid data size*/
	dma_buf->area = fk_acore_smem_alloc(component->dev, &dma_buf->addr, dma_buf->bytes);
	if (dma_buf->area == NULL) {
		AUD_LOG_ERR(AUD_SOC, "v_addr=0x%llx p_addr=0x%llx",
			(uint64_t)dma_buf->area, dma_buf->addr);
		return -ENOMEM;
	}
	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	AUD_LOG_DBG(AUD_SOC, "sess_id = 0x%x,bytes=%lu, addr= 0x%llx,area = 0x%llx\n",
			lsm_prtd->session_id, runtime->dma_bytes,
			runtime->dma_addr, (uint64_t)runtime->dma_area);
	ret = fk_acore_register(lsm_prtd->session_id, (fk_acore_cb)lsm_event_handler, lsm_prtd);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_SOC, "acore register failed %d", ret);
		return ret;
	}
	return ret;
}

static int fk_lsm_trigger(struct snd_soc_component *component,
		       struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;
	int vt_cmd = VT_CMD_INVALID;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		if (!kws_status) {
			ret = fk_acore_trigger(lsm_prtd->session_id, OP_START);
			if (ret < 0)
				AUD_LOG_ERR(AUD_SOC, "trigger start failed");
		}
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		vt_cmd = VT_CMD_PORT_SUSPEND;
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		vt_cmd = VT_CMD_PORT_RESUME;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		if (!kws_status) {
			ret = fk_acore_trigger(lsm_prtd->session_id, OP_STOP);
			if (ret < 0)
				AUD_LOG_ERR(AUD_SOC, "trigger stop failed");
		}
		kws_status = 0;
		break;
	default:
		vt_cmd = VT_CMD_INVALID;
		AUD_LOG_ERR(AUD_SOC,
			"unhandled trigger cmd %d\n", cmd);
		break;
	}

	if (vt_cmd != VT_CMD_INVALID) {
		/*record/vioce etc ,vt suspend or resume*/
		/*TODO*/
	}
	return ret;
}

static int fk_lsm_close(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;
	struct stream_params s_param;

	AUD_LOG_DBG(AUD_SOC, "sess_id = 0x%x,bytes=%lu, addr= 0x%llx,area = 0x%llx\n",
			lsm_prtd->session_id, runtime->dma_bytes,
			runtime->dma_addr, (uint64_t)runtime->dma_area);
	memset(&s_param, 0x0, sizeof(struct stream_params));
	if (!lsm_info.start_stop_rec) {
		s_param.type = PARAMS_KWS_STOP_RECOGNITON;
		ret = fk_acore_set_params(lsm_prtd->session_id, &s_param);
		if (ret < 0)
			AUD_LOG_ERR(AUD_SOC, "stop rec failed");
	}
	ret = fk_acore_unregister(lsm_prtd->session_id);
	fk_acore_smem_free(component->dev, runtime->dma_bytes,
							runtime->dma_area, runtime->dma_addr);
//	kfree(lsm_prtd);
	return ret;
}

static int fk_lsm_hwdep_ioctl(struct snd_hwdep *hw, struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	uint32_t det_status = 0;
	struct stream_params s_param;

	if (!hw) {
		AUD_LOG_ERR(AUD_SOC, "HW is NULL");
		return -1;
	}
	struct snd_pcm *pcm = hw->private_data;

	if (!pcm || !&pcm->streams[SNDRV_PCM_STREAM_CAPTURE] || !pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream) {
		AUD_LOG_ERR(AUD_SOC, "pcm substream is NULL");
		return -1;
	}

	struct snd_pcm_substream *substream = pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
	struct snd_pcm_runtime *runtime = substream->runtime;

	if (!runtime || !runtime->private_data) {
		AUD_LOG_ERR(AUD_SOC, "runtime is NULL");
		return -1;
	}

	struct fk_audio_lsm *lsm_prtd = runtime->private_data;

	memset(&s_param, 0x0, sizeof(struct stream_params));

	switch (cmd) {
	case SNDRV_VA_LOAD_MODULE:{
			struct kws_module_info kws_module_info = {0};

			if (copy_from_user(&kws_module_info, (void *)arg,
				   sizeof(kws_module_info))) {
				AUD_LOG_ERR(AUD_SOC, "copy_from_user failed\n");
				ret = -EINVAL;
				return ret;
			}
			s_param.type = PARAMS_SET_KWS_MODULE_INFO;
			s_param.params.module_info = kws_module_info;
			ret = fk_acore_set_params(lsm_prtd->session_id, &s_param);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_SOC, "fk_acore_set_params failed.");
				return ret;
			}
		}
			break;
	case SNDRV_VA_UNLOAD_MODULE:{
			struct kws_module_info kws_module_info = {0};

			AUD_LOG_DBG(AUD_SOC, "SNDRV_VA_UNLOAD_MODULE ");
			if (copy_from_user(&kws_module_info, (void *)arg,
				   sizeof(kws_module_info))) {
				AUD_LOG_ERR(AUD_SOC, "copy_from_user failed.");
				ret = -EINVAL;
				return ret;
			}
			s_param.type = PARAMS_DEL_KWS_MODULE_INFO;
			s_param.params.module_info = kws_module_info;
			ret = fk_acore_set_params(lsm_prtd->session_id, &s_param);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_SOC, "fk_acore_set_params failed.");
				return ret;
			}
		}
			break;
	case SNDRV_VA_START_RECOGNITION:{
		/*TODO */
			kws_status = 1;
			lsm_info.start_stop_rec = 0;
			lsm_info.count = 0;
			s_param.type = PARAMS_KWS_START_RECOGNITON;

			ret = fk_acore_set_params(lsm_prtd->session_id, &s_param);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_SOC, "start rec failed");
				return ret;
			}
		}
			break;
	case SNDRV_VA_STOP_RECOGNITION:{
		/*TODO */
			lsm_info.start_stop_rec = 1;
			s_param.type = PARAMS_KWS_STOP_RECOGNITON;

			ret = fk_acore_set_params(lsm_prtd->session_id, &s_param);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_SOC, "stop rec failed");
				return ret;
			}
		}
		break;
	case SNDRV_VA_WAIT_EVENT:
			s_param.type = PARAMS_KWS_EVENT_STATUS;

			ret = fk_acore_get_params(lsm_prtd->session_id, &s_param);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_SOC, "wait event failed");
				return ret;
			}
			det_status = s_param.params.value;
			AUD_LOG_DBG(AUD_SOC, "det_status=%d", det_status);
			put_user(det_status, (uint32_t __user *)arg);
			break;
	default:
			AUD_LOG_INFO(AUD_SOC, "unsupport cmd");
			break;
	}
	return ret;
}

#ifdef CONFIG_COMPAT
static int fk_lsm_hwdep_compat_ioctl(struct snd_hwdep *hw,
				      struct file *file,
				      unsigned int cmd,
				      unsigned long arg)
{
	/* we only support mmap fd. Handling is common in both modes */
	return fk_lsm_hwdep_ioctl(hw, file, cmd, arg);
}
#else
static int fk_lsm_hwdep_compat_ioctl(struct snd_hwdep *hw,
				      struct file *file,
				      unsigned int cmd,
				      unsigned long arg)
{
	return -EINVAL;
}
#endif

static int fk_lsm_add_hwdep_dev(struct snd_soc_pcm_runtime *runtime)
{
	struct snd_hwdep *hwdep;
	int rc;
	char id[] = "LSM_NN";

	rc = snd_hwdep_new(runtime->card->snd_card,
			   &id[0],
			   runtime->pcm->device,
			   &hwdep);
	if (!hwdep || rc < 0) {
		AUD_LOG_ERR(AUD_SOC, "hwdep intf failed to create %s - hwdep\n", id);
		return rc;
	}

	// hwdep->iface = SNDRV_HWDEP_IFACE_AUDIO_BE; /* for lack of a FE iface */
	hwdep->private_data = runtime->pcm; /* of type struct snd_pcm */
	hwdep->ops.ioctl = fk_lsm_hwdep_ioctl;
	hwdep->ops.ioctl_compat = fk_lsm_hwdep_compat_ioctl;
	return 0;
}

int fk_lsm_new(struct snd_soc_component *component,
			     struct snd_soc_pcm_runtime *rtd)
{
	// struct snd_card *card = rtd->card->snd_card;
	// struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

	ret = fk_lsm_add_hwdep_dev(rtd);
	if (ret) {
		AUD_LOG_DBG(AUD_SOC, "Could not add hw dep node\n");
		return ret;
	}
	return ret;
}

void fk_lsm_set_params(int flag)
{
	struct stream_params strm_param = {0};
	int ret = 0;

	strm_param.params.value = lsm_info.vt_flag|lsm_info.fca_flag;
	strm_param.type = PARAMS_SET_VT_FCA_FLAG;
	ret = fk_acore_set_params(lsm_info.session_id, &strm_param);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "fk acore set vt flag failed");
}

static int fk_lsm_vt_flag_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lsm_info.vt_flag;
	return 0;
}

static int fk_lsm_vt_flag_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int vt_flag = 0;

	vt_flag = ucontrol->value.integer.value[0];
	AUD_LOG_DBG(AUD_COMM, "vt_flag=%d", vt_flag);

	if (vt_flag == true) {
		lsm_info.vt_flag |= vt_flag << 0;
		fk_lsm_set_params(lsm_info.vt_flag);
	} else if (vt_flag == false) {
		lsm_info.vt_flag ^= !vt_flag << 0;
		fk_lsm_set_params(lsm_info.vt_flag);
	} else {
		AUD_LOG_ERR(AUD_COMM, "vt flag value invalid, flag %d", vt_flag);
	}

	return ret;
}

static int fk_lsm_fca_flag_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lsm_info.fca_flag;
	return 0;
}

static int fk_lsm_fca_flag_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int fca_flag = 0;

	fca_flag = ucontrol->value.integer.value[0];

	if (fca_flag == true) {
		lsm_info.fca_flag |= fca_flag << 1;
		fk_lsm_set_params(lsm_info.fca_flag);
	} else if (fca_flag == false) {
		lsm_info.fca_flag ^= !fca_flag << 1;
		fk_lsm_set_params(lsm_info.fca_flag);
	} else {
		AUD_LOG_ERR(AUD_COMM, "fca flag value invalid, flag %d", fca_flag);
	}

	return ret;
}

static int fk_lsm_fca_event_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lsm_info.fca_event;
	return 0;
}

static int fk_lsm_fca_event_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct stream_params strm_param = {0};

	lsm_info.fca_event = ucontrol->value.integer.value[0];
	if (lsm_info.fca_event) {
		strm_param.type = PARAMS_SET_FCA_EVENT;
		ret = fk_acore_set_params(lsm_info.session_id, &strm_param);
		if (ret < 0)
			AUD_LOG_ERR(AUD_COMM, "fk acore set vt flag failed");
	}

	return ret;
}

static const struct snd_kcontrol_new fk_audio_lsm_lsm_controls[] = {
	SOC_SINGLE_EXT("enable VT flag", SND_SOC_NOPM, 0, 1, 0,
		fk_lsm_vt_flag_get, fk_lsm_vt_flag_put),
	SOC_SINGLE_EXT("enable fca flag", SND_SOC_NOPM, 0, 1, 0,
		fk_lsm_fca_flag_get, fk_lsm_fca_flag_put),
	SOC_SINGLE_EXT("fca event", SND_SOC_NOPM, 0, 1, 0,
		fk_lsm_fca_event_get, fk_lsm_fca_event_put),
};

static int fk_lsm_probe(struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component, fk_audio_lsm_lsm_controls,
		ARRAY_SIZE(fk_audio_lsm_lsm_controls));
	return 0;
}

int fk_lsm_ioctl(struct snd_soc_component *component,
		     struct snd_pcm_substream *substream,
		     unsigned int cmd, void *arg)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fk_audio_lsm *lsm_prtd = runtime->private_data;

	mutex_lock(&lsm_prtd->lsm_lock);
	switch (cmd) {
	case SNDRV_PCM_IOCTL1_RESET:
		lsm_prtd->pcm_irq_pos = 0;
		runtime->control->appl_ptr = 0;
		runtime->status->hw_ptr = 0;
	}
	mutex_unlock(&lsm_prtd->lsm_lock);

	return 0;
}

const struct snd_soc_component_driver fk_soc_lsm_component = {
	.name			= DRV_NAME,
	.probe			= fk_lsm_probe,
	.open			= fk_lsm_open,
	.prepare		= fk_lsm_prepare,
	.copy			= fk_lsm_copy,
	.pointer		= fk_lsm_pointer,
	.hw_params		= fk_lsm_hw_params,
	.trigger		= fk_lsm_trigger,
	.close			= fk_lsm_close,
	.ioctl			= fk_lsm_ioctl,
	.pcm_construct	= fk_lsm_new,
};

static int fk_dev_lsm_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev,
					&fk_soc_lsm_component,
					NULL, 0);
}

static int fk_dev_lsm_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_lsm_dt_match[] = {
	{.compatible = "xring,fk-lsm-dsp"},
	{}
};
MODULE_DEVICE_TABLE(of, fk_lsm_dt_match);

static struct platform_driver fk_lsm_driver = {
	.driver = {
		.name = "fk-lsm-dsp",
		.owner = THIS_MODULE,
		.of_match_table = fk_lsm_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = fk_dev_lsm_probe,
	.remove = fk_dev_lsm_remove,
};

int __init fk_lsm_init(void)
{

	return platform_driver_register(&fk_lsm_driver);
}

void fk_lsm_exit(void)
{
	platform_driver_unregister(&fk_lsm_driver);
}

MODULE_DESCRIPTION("lsm module platform driver");
MODULE_LICENSE("Dual BSD/GPL");
