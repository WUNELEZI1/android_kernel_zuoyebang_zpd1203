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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/ratelimit.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/delay.h>

#include "../asoc/fk-dai-be.h"
#include "../common/fk-audio-cma.h"
#include "../common/fk-audio-timer.h"

#include "fk-acore-api.h"
#include "fk-acore.h"
#include "fk-acore-process.h"
#include "fk-acore-state.h"
#include "fk-acore-utils.h"

#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#endif

#include "../common/audio-trace.h"

#define ALLOC_MEMORY_MIN XR_BL2_MEM_BASE_LOW
#define ALLOC_MEMORY_MAX (XR_BL2_MEM_BASE_LOW + XR_CMA_MEM_SIZE)

/************************************************************************
 * function: fk_acore_smem_alloc
 * description: alloc share memory between ap and adsp
 * parameter:
 *		phy_addr : physical address of share memory
 * return:
 *		char * : virtual address of share memory
 ************************************************************************/
char *fk_acore_smem_alloc(struct device *dev, dma_addr_t *phy_addr, size_t size)
{
	void *vir_addr = NULL;

	/*data buffer virtual address*/
	if ((size + SMEM_HEAD_SIZE) <= SMEM_MIN_SIZE) {
		/*data buffer virtual address*/
		vir_addr = fk_audio_buffer_alloc(dev,
			SMEM_MIN_SIZE + SMEM_HEAD_SIZE, phy_addr);
	} else {
		/*data buffer virtual address*/
		vir_addr = fk_audio_buffer_alloc(dev,
			size + SMEM_HEAD_SIZE, phy_addr);
	}

	if (!vir_addr)
		return NULL;

	if ((uint64_t)*phy_addr < ALLOC_MEMORY_MIN || (uint64_t)*phy_addr > ALLOC_MEMORY_MAX) {
		AUD_LOG_ERR(AUD_CORE, "unavailable memory p_addr=0x%llx", (uint64_t)*phy_addr);
		return NULL;
	}

	AUD_LOG_DBG(AUD_CORE, "v_addr:0x%llx p_addr=0x%llx size = %zu",
		(uint64_t)vir_addr, (uint64_t)*phy_addr, size);

	*phy_addr += SMEM_HEAD_SIZE;
	vir_addr += SMEM_HEAD_SIZE;

	return vir_addr;
}
EXPORT_SYMBOL(fk_acore_smem_alloc);

/************************************************************************
 * function: fk_acore_smem_free
 * description: free share memory between ap and adsp
 * parameter:
 *		phy_addr : physical address of share memory
 *		vaddr : virtual address of share memory
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
void fk_acore_smem_free(struct device *dev, size_t size, void *vaddr, dma_addr_t phy_addr)
{
	void *vir_addr = vaddr;
	dma_addr_t pa = phy_addr;

	if (pa) {
		vir_addr = vaddr - SMEM_HEAD_SIZE;
		pa = phy_addr - SMEM_HEAD_SIZE;
		AUD_LOG_DBG(AUD_CORE, "v_addr:0x%llx p_addr=0x%llx size = %zu",
			(uint64_t)vir_addr, pa, size);
		if ((size + SMEM_HEAD_SIZE) <= SMEM_MIN_SIZE)
			size = SMEM_MIN_SIZE;

		if (vir_addr == NULL || pa == 0)
			return;

		fk_audio_buffer_free(dev, size + SMEM_HEAD_SIZE, vir_addr, pa);
	}
}
EXPORT_SYMBOL(fk_acore_smem_free);

/************************************************************************
 * function: fk_acore_register
 * description: register stream to audio core when stream open. Each
 *				stream will correspond to an instance. In the voice
 *				scenario, the uplink and downlink share one instance,
 *				just register one time.
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		cb : callback function
 *		priv : private parameters of callback function
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_register(uint16_t session_id, fk_acore_cb cb, void *priv)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;

	AUD_LOG_INFO(AUD_CORE, "session(0x%x)", session_id);
	get_adsp_time();
	get_utc_time();

#ifdef ATRACE_MACRO
	AUD_TRACE("enter");
#endif

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		/* alloc audio core stream if not already register */
		ac_strm = fk_acore_stream_alloc(session_id);
		if (!ac_strm) {
			AUD_LOG_ERR(AUD_CORE, "register 0x%x fail.", session_id);
			goto FUNC_ERR;
		}
	}

	if (atomic_read(&ac_strm->init_flag) == 0)
		fk_acore_stream_prepare(ac_strm);

	fk_adsp_vote_register(ac_strm->sess_id);

	AUD_LOG_DBG(AUD_CORE, "session id 0x%x init flag %d.",
		session_id, atomic_read(&ac_strm->init_flag));

	/* update callback function and private parameters */
	ac_strm->cb = cb;
	ac_strm->priv = priv;

	/*
	 *  if stream type is ultra, create queue and process thread.
	 *  unregister audio core stream when create resource fail.
	 */
	if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {

		/* create data queue */
		ret = fk_acore_create_data_proc(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "sess_id(0x%x) create data process fail.",
				session_id);
			goto FUNC_ERR;
		}

		/* set bypass dsp flag when create data process resource success */
		ac_strm->bypass_dsp = true;
	}

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return ret;

FUNC_ERR:
	if (ac_strm) {
		AUD_LOG_INFO(AUD_CORE, "idle acore");
		fk_acore_stream_idle(ac_strm);
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_register);

/************************************************************************
 * function: fk_acore_unregister
 * description: unregister stream to audio core when stream close.
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_unregister(uint16_t session_id)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
#endif

	AUD_LOG_INFO(AUD_CORE, "session_id(0x%x)", session_id);

#ifdef ATRACE_MACRO
	AUD_TRACE("enter");
#endif

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		//AC_LOG_DBG("audio core stream 0x%x not found.", session_id);
		return ret;
	}

	/* trigger stop before shutdown */
	ret = fk_acore_stream_trigger_stop(ac_strm);
	if (ret < 0)
		AUD_LOG_DBG(AUD_CORE, "no trigger stop");

	/* destroy data list in ultra scene */
	if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
		/*
		 *  if stream type is ultra, destroy queue and process thread.
		 */
		ret = fk_acore_destroy_data_proc(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "release resource fail. sess_id(0x%x)",
				session_id);
		}

		/* set bypass dsp flag */
		ac_strm->bypass_dsp = false;
	}

	/* shutdown stream.
	 * Note: ac_strm maybe already free in ack. Can not operation ac_strm
	 *		after shutdown.
	 */
	ret = fk_acore_stream_shutdown(ac_strm);
	if (ret < 0) {
		/* idle stream instance after unregister */
		fk_acore_stream_idle(ac_strm);

		AUD_LOG_ERR(AUD_CORE, "stream(0x%x) shutdown failure", session_id);
#if IS_ENABLED(CONFIG_MIEV)
				mievent  = cdev_tevent_alloc(906001155);
				cdev_tevent_add_str(mievent, "Keyword", "pcm close fail");
				cdev_tevent_write(mievent);
				cdev_tevent_destroy(mievent);
#endif
		return -EINVAL;
	}

	fk_adsp_vote_unregister(ac_strm->sess_id);

	/* idle stream instance after unregister */
	fk_acore_stream_idle(ac_strm);

	AUD_LOG_DBG(AUD_CORE, "completed");

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return ret;
}
EXPORT_SYMBOL(fk_acore_unregister);

/************************************************************************
 * function: fk_acore_startup
 * description: startup stream. The audio pipeline is composed of
 *				fe(stream) and be(port). The audio pipeline will
 *				start when the fe and be are both enabled.
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		fmt : data format of stream
 *		smem : share memory of stream between ap and dsp.
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_startup(uint16_t session_id, struct stream_format fmt, struct share_mem smem)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
#endif

	AUD_LOG_DBG(AUD_CORE, "session id(0x%x)", session_id);

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_ERR(AUD_CORE, "audio core stream 0x%x not found.", session_id);
		return -EINVAL;
	}

	/* update stream format info */
	ac_strm->fmt.samples = fmt.samples;
	ac_strm->fmt.channels = fmt.channels;
	ac_strm->fmt.bit_width = fmt.bit_width;
	ac_strm->fmt.codec_type = fmt.codec_type;
	AUD_LOG_DBG(AUD_CORE, "samples(%d) channels(%d) bit_width(%d) codec(%x)",
		fmt.samples, fmt.channels, fmt.bit_width, fmt.codec_type);

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_UL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_KARAOKE_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_KARAOKE_UL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_HF_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_HF_UL)) {
		if ((smem.p_addr >> 32) & 0xFFFFFFFF) {
			AUD_LOG_ERR(AUD_CORE, "invalid physical addr 0x%llx.", smem.p_addr);
			return -EINVAL;
		}
		/* update share memory info */
		ac_strm->smem.vir_addr = smem.vir_addr;
		ac_strm->smem.p_addr = smem.p_addr;
		ac_strm->smem.periods_size = smem.periods_size;
		ac_strm->smem.periods_num = smem.periods_num;
		AUD_LOG_DBG(AUD_CORE, "vaddr(0x%llx) paddr(0x%llx) period size(%d) period num(%d)",
			(uint64_t)smem.vir_addr, smem.p_addr,
			smem.periods_size, smem.periods_num);
	}

	/* set fe flag */
	ac_strm->fe_flag = 1;

	/* Proactively querying port information to avoid scenarios
	 * where ports are not closed after scene shutdown.
	 */
	if (!ac_strm->be_flag) {
		if (ac_strm->strm_dir == STRM_DIR_DUAL) {
			ac_strm->in_port_id =
				fk_port_id_get_by_sess_dir(ac_strm->sess_id, FK_BE_PORT_TX);
			if (ac_strm->in_port_id >= 0) {
				struct dai_format *in_dai_fmt = fk_aif_dev_cfg_get(ac_strm->in_port_id);

				if (in_dai_fmt != NULL) {
					/* update dai format info */
					ac_strm->in_dai_fmt.samples = in_dai_fmt->samples;
					ac_strm->in_dai_fmt.channels = in_dai_fmt->channels;
					ac_strm->in_dai_fmt.bit_width = in_dai_fmt->bit_width;
					AUD_LOG_INFO(AUD_CORE, "in port(%d): rate(%d) channels(%d) bits(%d)",
						ac_strm->in_port_id, in_dai_fmt->samples,
						in_dai_fmt->channels, in_dai_fmt->bit_width);
				}
			}

			ac_strm->out_port_id =
				fk_port_id_get_by_sess_dir(ac_strm->sess_id, FK_BE_PORT_RX);
			if (ac_strm->out_port_id >= 0) {
				struct dai_format *out_dai_fmt = fk_aif_dev_cfg_get(ac_strm->out_port_id);

				if (out_dai_fmt != NULL) {
					/* update dai format info */
					ac_strm->out_dai_fmt.samples = out_dai_fmt->samples;
					ac_strm->out_dai_fmt.channels = out_dai_fmt->channels;
					ac_strm->out_dai_fmt.bit_width = out_dai_fmt->bit_width;
					AUD_LOG_INFO(AUD_CORE, "out port(%d): rate(%d) channels(%d) bits(%d)",
						ac_strm->out_port_id, out_dai_fmt->samples,
						out_dai_fmt->channels, out_dai_fmt->bit_width);
				}
			}

			if ((ac_strm->in_port_id >= 0) && (ac_strm->out_port_id >= 0))
				ac_strm->be_flag = 1;

			AUD_LOG_DBG(AUD_CORE, "in_port_id(%d) out_port_id(%d)",
				ac_strm->in_port_id, ac_strm->out_port_id);
		} else {
			fk_stream_port_info_update(ac_strm);
			if (ac_strm->port_num)
				ac_strm->be_flag = 1;

			AUD_LOG_DBG(AUD_CORE, "port_num(%d)", ac_strm->port_num);
		}
	}

	/* trigger startup when fe&be set complete */
	if (ac_strm->fe_flag && ac_strm->be_flag) {
		struct dai_format *dai_fmt = NULL;

		ac_strm->port_id = ac_strm->port_info[0];
		AUD_LOG_INFO(AUD_CORE, "pord id:%d", ac_strm->port_id);

		dai_fmt = fk_aif_dev_cfg_get(ac_strm->port_id);
		if (dai_fmt != NULL) {
			/* update dai format info */
			ac_strm->dai_fmt.samples = dai_fmt->samples;
			ac_strm->dai_fmt.channels = dai_fmt->channels;
			ac_strm->dai_fmt.bit_width = dai_fmt->bit_width;
			AUD_LOG_INFO(AUD_CORE, "port(%d): rate(%d) channels(%d) bits(%d)",
				ac_strm->port_id, dai_fmt->samples,
				dai_fmt->channels, dai_fmt->bit_width);
		}

#ifdef ATRACE_MACRO
		AUD_TRACE("trigger");
#endif

		ret = fk_acore_stream_startup(ac_strm);
		if (ret < 0) {
#if IS_ENABLED(CONFIG_MIEV)
			mievent  = cdev_tevent_alloc(906001153);
			cdev_tevent_add_str(mievent, "Keyword", "pcm_prepare fail");
			cdev_tevent_write(mievent);
			cdev_tevent_destroy(mievent);
#endif
			AUD_LOG_ERR(AUD_CORE, "stream(0x%x) failure", session_id);
			return -EINVAL;
		}

#ifdef ATRACE_MACRO
		AUD_TRACE("return");
#endif

		/* config additional devices */
		if (ac_strm->port_num > 1)
			fk_acore_add_path_config(ac_strm);
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_startup);

/************************************************************************
 * function: fk_acore_set_paths
 * description: startup port. The audio pipeline is composed of
 *				fe(stream) and be(port). The audio pipeline will
 *				start when the fe and be are both enabled.
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		port_id : input or output port id of backend.
 *		fmt : data format of port interface.
 *		en :
 *			true : enable path
 *			false : disable path
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_set_paths(uint16_t session_id, int port_id,
	struct dai_format fmt, bool en)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;

	AUD_LOG_INFO(AUD_CORE, "port(%d) samples(%d) ch(%d) bits(%d)",
		port_id, fmt.samples, fmt.channels, fmt.bit_width);

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		if (en == true) {
			/* alloc audio core stream if not already register */
			ac_strm = fk_acore_stream_alloc(session_id);
			if (!ac_strm) {
				AUD_LOG_ERR(AUD_CORE, "register 0x%x fail.", session_id);
				return -EINVAL;
			}
		} else {
			/* stream instance already free by unregister operation */
			AUD_LOG_DBG(AUD_CORE, "instance already free");
			return ret;
		}
	}

	if (en == true) {
		if (atomic_read(&ac_strm->init_flag) == 0) {
			fk_acore_stream_prepare(ac_strm);
			AUD_LOG_DBG(AUD_CORE, "session id 0x%x prepare.", session_id);
		}
	}

	AUD_LOG_DBG(AUD_CORE, "session id 0x%x init flag %d.",
		session_id, atomic_read(&ac_strm->init_flag));

	if (ac_strm->strm_dir == STRM_DIR_DUAL) {
		switch (port_id) {
		case FK_XR_I2S0_TX:
		case FK_XR_I2S1_TX:
		case FK_XR_I2S2_TX:
		case FK_XR_I2S3_TX:
		case FK_XR_I2S4_TX:
		case FK_XR_I2S5_TX:
		case FK_XR_I2S6_TX:
		case FK_XR_I2S7_TX:
		case FK_XR_I2S8_TX:
		case FK_XR_I2S9_TX:
		case FK_XR_TDM0_TX:
		case FK_XR_TDM1_TX:
		case FK_XR_TDM2_TX:
		case FK_XR_TDM3_TX:
		case FK_XR_TDM4_TX:
		case FK_XR_TDM5_TX:
		case FK_XR_TDM6_TX:
		case FK_XR_TDM7_TX:
		case FK_XR_TDM8_TX:
		case FK_XR_TDM9_TX:
		case FK_XR_TDM7_8_TX:
			ac_strm->in_port_id = port_id;
			if (en)
				ac_strm->in_be_flag = 1;
			else
				ac_strm->in_be_flag = 0;

			/* update in dai format info */
			ac_strm->in_dai_fmt.samples = fmt.samples;
			ac_strm->in_dai_fmt.channels = fmt.channels;
			ac_strm->in_dai_fmt.bit_width = fmt.bit_width;
			break;
		case FK_XR_I2S0_RX:
		case FK_XR_I2S1_RX:
		case FK_XR_I2S2_RX:
		case FK_XR_I2S3_RX:
		case FK_XR_I2S4_RX:
		case FK_XR_I2S5_RX:
		case FK_XR_I2S6_RX:
		case FK_XR_I2S7_RX:
		case FK_XR_I2S8_RX:
		case FK_XR_I2S9_RX:
		case FK_XR_TDM0_RX:
		case FK_XR_TDM1_RX:
		case FK_XR_TDM2_RX:
		case FK_XR_TDM3_RX:
		case FK_XR_TDM4_RX:
		case FK_XR_TDM5_RX:
		case FK_XR_TDM6_RX:
		case FK_XR_TDM7_RX:
		case FK_XR_TDM8_RX:
		case FK_XR_TDM9_RX:
		case FK_XR_TDM7_8_RX:
			ac_strm->out_port_id = port_id;
			if (en)
				ac_strm->out_be_flag = 1;
			else
				ac_strm->out_be_flag = 0;

			/* update out dai format info */
			ac_strm->out_dai_fmt.samples = fmt.samples;
			ac_strm->out_dai_fmt.channels = fmt.channels;
			ac_strm->out_dai_fmt.bit_width = fmt.bit_width;
			break;
		default:
			AUD_LOG_ERR(AUD_CORE, "unknown port %d.", port_id);
			break;
		}

		/* update be flag when in&out be set */
		if (ac_strm->in_be_flag && ac_strm->out_be_flag) {
			if (ac_strm->in_dai_fmt.samples != ac_strm->out_dai_fmt.samples) {
				AUD_LOG_ERR(AUD_CORE, "dai format unmatch");
				return -EINVAL;
			}
			AUD_LOG_DBG(AUD_CORE, "set be flag.");
			ac_strm->be_flag = 1;
		} else {
			AUD_LOG_DBG(AUD_CORE, "clear session(%x) be flag.", ac_strm->sess_id);
			ac_strm->be_flag = 0;
		}
	} else {
		fk_stream_port_info_update(ac_strm);

		/* set be flag */
		if (ac_strm->port_num)
			ac_strm->be_flag = 1;
		else
			ac_strm->be_flag = 0;

		/* update current operation port id */
		ac_strm->port_id = port_id;

		/* update dai format info */
		ac_strm->dai_fmt.samples = fmt.samples;
		ac_strm->dai_fmt.channels = fmt.channels;
		ac_strm->dai_fmt.bit_width = fmt.bit_width;
		ac_strm->dai_fmt.pcm_fmt = fmt.pcm_fmt;
	}

	if (ac_strm->state == ACORE_STATE_IDLE) {
		AUD_LOG_DBG(AUD_CORE, "fe(%d) be(%d)", ac_strm->fe_flag,
			ac_strm->be_flag);
		/* trigger startup when fe&be set complete */
		if (ac_strm->fe_flag && ac_strm->be_flag) {
			ret = fk_acore_stream_startup(ac_strm);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_CORE, "stream(0x%x) startup failure",
					session_id);
				return -EINVAL;
			}
		}
	} else {
		/* trigger ADSP_SET_DEV_PATH to update port path */
		fk_acore_path_update(ac_strm, en);
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_set_paths);

/************************************************************************
 * function: fk_acore_set_channel_map
 * description: startup port. The audio pipeline is composed of
 *				fe(stream) and be(port). The audio pipeline will
 *				start when the fe and be are both enabled.
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		port_id : input or output port id of backend.
 *		map_info : channel map info.
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_set_channel_map(uint16_t session_id, int port_id,
	struct channel_map_info *map_info)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;
	uint32_t map_info0 = 0, map_info1 = 0;
	int i;

	AUD_LOG_INFO(AUD_CORE, "port id(%d) num(%d).", port_id, map_info->num);
	if (map_info->num > CHANNEL_NUM_MAX) {
		AUD_LOG_INFO(AUD_SOC, "invalid channel map num");
		return -EINVAL;
	}

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_ERR(AUD_CORE, "unexist session_id 0x%x.", session_id);
		return -EINVAL;
	}

	AUD_LOG_DBG(AUD_CORE, "session id 0x%x init flag %d.",
		session_id, atomic_read(&ac_strm->init_flag));

	switch (port_id) {
	case FK_XR_I2S0_TX:
	case FK_XR_I2S1_TX:
	case FK_XR_I2S2_TX:
	case FK_XR_I2S3_TX:
	case FK_XR_I2S4_TX:
	case FK_XR_I2S5_TX:
	case FK_XR_I2S6_TX:
	case FK_XR_I2S7_TX:
	case FK_XR_I2S8_TX:
	case FK_XR_I2S9_TX:
	case FK_XR_TDM0_TX:
	case FK_XR_TDM1_TX:
	case FK_XR_TDM2_TX:
	case FK_XR_TDM3_TX:
	case FK_XR_TDM4_TX:
	case FK_XR_TDM5_TX:
	case FK_XR_TDM6_TX:
	case FK_XR_TDM7_TX:
	case FK_XR_TDM8_TX:
	case FK_XR_TDM9_TX:
	case FK_XR_I2S0_RX:
	case FK_XR_I2S1_RX:
	case FK_XR_I2S2_RX:
	case FK_XR_I2S3_RX:
	case FK_XR_I2S4_RX:
	case FK_XR_I2S5_RX:
	case FK_XR_I2S6_RX:
	case FK_XR_I2S7_RX:
	case FK_XR_I2S8_RX:
	case FK_XR_I2S9_RX:
	case FK_XR_TDM0_RX:
	case FK_XR_TDM1_RX:
	case FK_XR_TDM2_RX:
	case FK_XR_TDM3_RX:
	case FK_XR_TDM4_RX:
	case FK_XR_TDM5_RX:
	case FK_XR_TDM6_RX:
	case FK_XR_TDM7_RX:
	case FK_XR_TDM8_RX:
	case FK_XR_TDM9_RX:
		/* set channel map info */
		for (i = 0; i < CHAN_MAP_INFO_0_NUM; i++)
			map_info0 = map_info0 | SLOT_VAL(map_info->info[i], i);

		map_info1 = 0;
		break;
	case FK_XR_TDM7_8_RX:
	case FK_XR_TDM7_8_TX:
		/* set channel map info */
		for (i = 0; i < CHAN_MAP_INFO_0_NUM; i++)
			map_info0 = map_info0 | SLOT_VAL(map_info->info[i], i);

		for (i = CHAN_MAP_INFO_0_NUM; i < map_info->num; i++)
			map_info1 = map_info1 |
				SLOT_VAL(map_info->info[i], (i - CHAN_MAP_INFO_0_NUM));

		break;
	default:
		map_info0 = 0;
		map_info1 = 0;
		break;
	}

	AUD_LOG_INFO(AUD_CORE, "info0(0x%x) info1(0x%x).",
		map_info0, map_info1);
	ac_strm->chan_map_info0 = map_info0;
	ac_strm->chan_map_info1 = map_info1;

	return ret;
}
EXPORT_SYMBOL(fk_acore_set_channel_map);

/************************************************************************
 * function: fk_acore_set_vol
 * description: set digital gain of stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		val : index of gain step
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_set_vol(uint16_t session_id, int val)
{
	int ret = 0;
	struct acore_stream *ac_strm = NULL;

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "audio core stream 0x%x not found.", session_id);
		return ret;
	}

	ret = fk_acore_set_volume(ac_strm, val);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "fk acore (session-id = 0x%x) set volume failed.", session_id);
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL(fk_acore_set_vol);

/************************************************************************
 * function: fk_acore_set_mute
 * description: set software mute status of stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		flag : mute status
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_set_mute(uint16_t session_id, uint8_t mute)
{
	int ret = 0;
	struct acore_stream *ac_strm = NULL;

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "audio core stream 0x%x not found.", session_id);
		return ret;
	}

	ret = fk_acore_set_mute_flag(ac_strm, mute);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "fk acore (session-id = 0x%x) set volume failed.", session_id);
		return ret;
	}
	return 0;
}
EXPORT_SYMBOL(fk_acore_set_mute);

/************************************************************************
 * function: fk_acore_set_mute
 * description: set parameters of stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		strm_param :
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_set_params(uint16_t session_id, struct stream_params *strm_param)
{
	int ret = EOK;
	enum speech_band band = 0;
	struct acore_stream *ac_strm = NULL;
	struct stream_format *str_fmt = NULL;
	struct voice_pcie_bar *pcie_bar = NULL;
	struct kws_module_info *m_info = NULL;
	struct ec_ctrl *ec_ctrl = NULL;
	uint8_t flag = 0;

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_ERR(AUD_CORE, "audio core stream 0x%x not found.",
			session_id);
		return -EINVAL;
	}

	AUD_LOG_DBG(AUD_CORE, "type:%d", strm_param->type);
	switch (strm_param->type) {
	case PARAMS_GAPLESS_INIT_SAMPS:
	case PARAMS_GAPLESS_TRAIL_SAMPS:
		break;
	case PARAMS_SET_DEC_INFO:
		str_fmt = (struct stream_format *)&strm_param->params.sfmt;
		/* update stream format info */
		ac_strm->fmt.samples = str_fmt->samples;
		ac_strm->fmt.channels = str_fmt->channels;
		ac_strm->fmt.bit_width = str_fmt->bit_width;
		ac_strm->fmt.codec_type = str_fmt->codec_type;
		AUD_LOG_INFO(AUD_CORE, "samples(%d) channels(%d) bit_width(%d) codec(%x)",
			str_fmt->samples, str_fmt->channels,
			str_fmt->bit_width, str_fmt->codec_type);
		break;
	case PARAMS_SET_VOICE_PCIE_BAR:
		pcie_bar = (struct voice_pcie_bar *)&strm_param->params.pcie_bar;
		ac_strm->bar_info.BAR0_1_addr = pcie_bar->BAR0_1_addr;
		ac_strm->bar_info.BAR2_3_addr = pcie_bar->BAR2_3_addr;
		break;
	case PARAMS_SET_KWS_MODULE_INFO:
		m_info = (struct kws_module_info *)&strm_param->params.module_info;
		ret = fk_acore_kws_load_module(ac_strm, m_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_kws_load_module failed.");
		}
		break;
	case PARAMS_DEL_KWS_MODULE_INFO:
		m_info = (struct kws_module_info *)&strm_param->params.module_info;
		ret = fk_acore_kws_unload_module(ac_strm, m_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_kws_load_module failed.");
		}
		break;
	case PARAMS_KWS_START_RECOGNITON:
		ret = fk_acore_kws_start_rec(ac_strm, m_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_kws_start_recognition failed.");
		}
		break;
	case PARAMS_KWS_STOP_RECOGNITON:
		ret = fk_acore_kws_stop_rec(ac_strm, m_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_kws_stop_recognition failed.");
		}
		break;
	case PARAMS_KWS_EC_REF:
		ec_ctrl = (struct ec_ctrl *)&strm_param->params.kws_ec_ctrl;
		ret = fk_acore_kws_ec_ref(ac_strm, ec_ctrl);
		break;
	case PARAMS_SET_VOICE_BANDWIDTH:
		band = (enum speech_band)strm_param->params.value;
		ret = fk_acore_speech_bandwidth(ac_strm, band);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_speech_bandwidth failed.");
		}
		break;
	case PARAMS_SET_VT_FCA_FLAG:
		flag = (uint8_t)strm_param->params.value;
		ret = fk_acore_set_vt_fca_flag(ac_strm, flag);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_set_vt_fca_flag failed.");
		}
		break;
	case PARAMS_SET_FCA_EVENT:
		ret = fk_acore_set_fca_event(ac_strm);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_set_fca_event failed.");
		}
		break;
	default:
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "parameters type unexpect.");
		break;
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_set_params);

/************************************************************************
 * function: fk_acore_get_params
 * description: get parameters of stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		strm_param :
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_get_params(uint16_t session_id, struct stream_params *strm_param)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;
	uint32_t tstamp = 0;
	uint32_t det_data_len = 0;
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_ERR(AUD_CORE, "stream 0x%x not found.", session_id);
		return -EINVAL;
	}

	switch (strm_param->type) {
	case PARAMS_GET_TIMESTAMP:
		ret = fk_acore_get_timestamp_info(ac_strm, &tstamp);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_get_timestamp_info failed.");
		} else {
			strm_param->params.value = tstamp;
		}
		break;
	case PARAMS_KWS_EVENT_STATUS:
		ret = fk_acore_kws_event_status(ac_strm, &det_data_len);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "fk_acore_kws_event_status failed.");
		} else
			strm_param->params.value = det_data_len;
		break;
	case PARAMS_MMAP_HW_PRT:
		smem_head_ptr = ac_strm->mmap_offset_vaddr;
		if (ac_strm->adsp_sc_t == ADSP_SCENE_MMAP_PLAY)
			ret = smem_head_ptr->r_off;
		else if (ac_strm->adsp_sc_t == ADSP_SCENE_MMAP_RECORD)
			ret = smem_head_ptr->w_off;
		break;
	default:
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "parameters type(%d) unexpect.", strm_param->type);
		break;
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_get_params);

/************************************************************************
 * function: fk_acore_trigger
 * description: state operation of stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		op : operation code
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_trigger(uint16_t session_id, enum op_code op)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;

	AUD_LOG_INFO(AUD_CORE, "session_id(0x%x) op:%x",
		session_id, (uint32_t)op);

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_ERR(AUD_CORE, "stream 0x%x not found.", session_id);
		return -EINVAL;
	}

	switch (op) {
	case OP_START:
		if (ac_strm->state == ACORE_STATE_PAUSE) {
			ret = fk_acore_stream_trigger_resume(ac_strm);
			if (ret < 0) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "trigger restart failure");
			}
		} else {
			ret = fk_acore_stream_trigger_start(ac_strm);
			if (ret < 0) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "trigger start failure");
			}
		}
		break;
	case OP_STOP:
		/* workaround for x-run */
		atomic_set(&ac_strm->stop_flag, 1);
		if ((ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_DL) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_UL)) {
			ret = fk_acore_stream_trigger_stop(ac_strm);
			if (ret < 0) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "trigger stop failure");
			}
		} else {
			ret = fk_acore_stream_trigger_pause(ac_strm);
			if (ret < 0) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "trigger pause failure");
			} else {
				mdelay(1);
				ret = fk_acore_stream_trigger_flush(ac_strm);
				if (ret < 0) {
					ret = -EINVAL;
					AUD_LOG_DBG(AUD_CORE, "trigger flush failure");
				}
			}
		}
		break;
	case OP_PAUSE:
		ret = fk_acore_stream_trigger_pause(ac_strm);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "trigger pause failure");
		}
		break;
	case OP_RESUME:
		ret = fk_acore_stream_trigger_resume(ac_strm);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "trigger resume failure");
		}
		break;
	case OP_FLUSH:
		ret = fk_acore_stream_trigger_flush(ac_strm);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "trigger flush failure");
		}
		break;
	case OP_DRAIN:
		ret = fk_acore_stream_trigger_drain(ac_strm);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "trigger drain failure");
		}
		break;
	case OP_NEXT_TRACK:
		ret = fk_acore_stream_trigger_next_track(ac_strm);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "trigger next track failure");
		}
		break;
	default:
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "op(%d) code unsupport", op);
		break;
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_trigger);

/************************************************************************
 * function: fk_acore_playback_copy
 * description: send data of playback stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		p_buf : data buffer info
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_playback_copy(uint16_t session_id, struct buf_blk p_buf)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_INFO_LIM(AUD_CORE, "stream 0x%x not found.", session_id);
		return -EINVAL;
	}

	if ((ac_strm->state >= ACORE_STATE_MAX) || (ac_strm->state == ACORE_STATE_IDLE)) {
		AUD_LOG_INFO_LIM(AUD_CORE, "stream state(%d) unexpected.", ac_strm->state);
		return -EINVAL;
	}

	ret = fk_acore_stream_data_send(ac_strm, p_buf);
	if (ret < 0) {
		AUD_LOG_INFO_LIM(AUD_CORE, "sess(0x%x) send failure.", session_id);
		return -EINVAL;
	}

	/* check if need trigger start */
	if (atomic_read(&ac_strm->write_flag) == 0) {
		AUD_LOG_INFO_LIM(AUD_CORE, "sess(0x%x) set write flag.", session_id);
		atomic_set(&ac_strm->write_flag, 1);

		if (atomic_read(&ac_strm->start_flag)) {
			/* trigger stream start when pending start operation exist */
			ret = fk_acore_stream_trigger_start(ac_strm);
			if (ret < 0) {
				/* clear write flag */
				atomic_set(&ac_strm->write_flag, 0);
				AUD_LOG_INFO_LIM(AUD_CORE, "sess(%x) trigger start failure.",
					session_id);
			} else {
				/* clear start flag */
				atomic_set(&ac_strm->start_flag, 0);
			}
		}
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_playback_copy);

/************************************************************************
 * function: fk_acore_capture_copy
 * description: receive data of capture stream
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 *		p_buf : data buffer info
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_capture_copy(uint16_t session_id, struct buf_blk p_buf)
{
	int ret = EOK;
	struct acore_stream *ac_strm = NULL;

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get(session_id);
	if (!ac_strm) {
		AUD_LOG_ERR_LIM(AUD_CORE, "stream 0x%x not found.", session_id);
		return -EINVAL;
	}

	if ((ac_strm->state >= ACORE_STATE_MAX) ||
		(ac_strm->state == ACORE_STATE_IDLE)) {
		AUD_LOG_ERR_LIM(AUD_CORE, "stream 0x%x unexpected.",
			ac_strm->state);
		return -EINVAL;
	}

	ret = fk_acore_stream_data_recv(ac_strm, p_buf);
	if (ret < 0) {
		AUD_LOG_ERR_LIM(AUD_CORE, "sess(0x%x) send failure.", session_id);
		return -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(fk_acore_capture_copy);

