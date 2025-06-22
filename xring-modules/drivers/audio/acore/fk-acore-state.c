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

#include "fk-acore-define.h"
#include "fk-acore-state.h"
#include "fk-acore.h"
#include "fk-acore-utils.h"
#include "../common/fk-audio-memlayout.h"

/************************** function define begin ***************************/
static int acore_stream_startup(struct acore_stream *ac_strm);
static int acore_stream_shutdown(struct acore_stream *ac_strm);
static int acore_stream_start(struct acore_stream *ac_strm);
static int acore_stream_pause(struct acore_stream *ac_strm);
static int acore_stream_flush(struct acore_stream *ac_strm);
static int acore_stream_resume(struct acore_stream *ac_strm);
static int acore_stream_drain(struct acore_stream *ac_strm);
static int acore_stream_next_track(struct acore_stream *ac_strm);
static int acore_stream_stop(struct acore_stream *ac_strm);
/************************** function define end *****************************/


static struct acore_stream_ops a_strm_ops[ACORE_STATE_MAX] = {
	/* ACORE_STATE_IDLE */
	{
		.startup = acore_stream_startup,
	},
	/* ACORE_STATE_PREPARE */
	{
		.shutdown = acore_stream_shutdown,
		.start = acore_stream_start,
		.pause = acore_stream_pause,
		.flush = acore_stream_flush,
		.stop = acore_stream_stop,
	},
	/* ACORE_STATE_PAUSE */
	{
		.shutdown = acore_stream_shutdown,
		.start = acore_stream_start,
		.flush = acore_stream_flush,
		.resume = acore_stream_resume,
		.stop = acore_stream_stop,
		.drain = acore_stream_drain,
		.next_track = acore_stream_next_track,
	},
	/* ACORE_STATE_RUN */
	{
		.shutdown = acore_stream_shutdown,
		.pause = acore_stream_pause,
		.flush = acore_stream_flush,
		.stop = acore_stream_stop,
		.drain = acore_stream_drain,
		.next_track = acore_stream_next_track,
	},
};

/************************************************************************
 * function: acore_stream_startup
 * description: startup stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_startup(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct xring_aud_msg scene_startup_msg = {0};
	struct adsp_scene_startup_info *startup_info;
	struct adsp_scene_lp_startup_info *lp_startup_info;

	/* fill scene startup message */
	scene_startup_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_startup_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_startup_msg.header.cmd_id = ADSP_AUD_SCENE_STARTUP;
	scene_startup_msg.header.result = 0;

	if (ac_strm->adsp_sc_t == ADSP_SCENE_LOOPBACK) {
		scene_startup_msg.header.len = sizeof(struct xring_aud_msg_header) +
			sizeof(struct adsp_scene_lp_startup_info);

		lp_startup_info = (struct adsp_scene_lp_startup_info *)&scene_startup_msg.data[0];
#ifdef STARTUP_INFO_VER_02
		/* fill loopback type info */
		lp_startup_info->lp_info.loopback_mode = 0;
		lp_startup_info->lp_info.loopback_enc_mode = 0;

		AUD_LOG_DBG(AUD_CORE, "lp_type: %d, lp_mode %d, lp_enc_mode %d",
			lp_startup_info->lp_type,
			lp_startup_info->lp_info.loopback_mode,
			lp_startup_info->lp_info.loopback_enc_mode);

		ret = fk_acore_port_set(ac_strm, (void *)lp_startup_info, 1);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "port set failure.");
			return ret;
		}

		ret = fk_acore_peri_sel_set(ac_strm, lp_startup_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "lp audioIf iobuf or reg set failure.");
			return ret;
		}
		AUD_LOG_DBG(AUD_CORE, "lp path info: %x %x", lp_startup_info->ul_peri_sel.l_i2s_idx,
			lp_startup_info->dl_peri_sel.i2s0_slot);
#else
		/* fill stream data format info. just for init. dsp unuse */
		lp_startup_info->codecInfo.codec_type = CODEC_NONE;

		/* fill loopback type info */
		lp_startup_info->lpType = ac_strm->lp_type;

		ret = fk_acore_peri_sel_set(ac_strm, lp_startup_info);
		AUD_LOG_DBG(AUD_CORE, "lp path info: %x %x", lp_startup_info->ulPeriSel.l_i2s_idx,
			lp_startup_info->dlPeriSel.i2s0_slot);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "lp audioIf iobuf or reg set failure.");
			return ret;
		}

#endif
	} else {
		scene_startup_msg.header.len = sizeof(struct xring_aud_msg_header) +
			sizeof(struct adsp_scene_startup_info);

		startup_info = (struct adsp_scene_startup_info *)&scene_startup_msg.data[0];

#ifdef STARTUP_INFO_VER_02
		/* fill stream data format info */
		if ((ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY)) {
			ret = fk_acore_adsp_codec_caps_check(ac_strm);
			if (ret < 0) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "scene(%d) unsupport codec type(%d).",
					ac_strm->adsp_sc_t, ac_strm->fmt.codec_type);
				return ret;
			}

			startup_info->strm_info.codec_type =
				fk_acore_get_adsp_codec_type(ac_strm->fmt.codec_type);
		} else {
			startup_info->strm_info.codec_type = CODEC_NONE;
		}
		startup_info->strm_info.sample_rate = ac_strm->fmt.samples;
		startup_info->strm_info.channels = (uint8_t)(ac_strm->fmt.channels & 0xFF);
		startup_info->strm_info.bit_width = (uint8_t)(ac_strm->fmt.bit_width & 0xFF);
		AUD_LOG_DBG(AUD_CORE, "stream: session(%d) sample(%d) channels(%d) bit width(%d).",
			ac_strm->sess_id, ac_strm->fmt.samples, ac_strm->fmt.channels,
			ac_strm->fmt.bit_width);

		/* fill share memory info */
		if ((ac_strm->adsp_sc_t == ADSP_SCENE_MMAP_PLAY) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_MMAP_RECORD)) {
			startup_info->smem.p_addr = ac_strm->smem.p_addr - SMEM_HEAD_SIZE;
			startup_info->smem.p_addr_head = ac_strm->mmap_offset_paddr;
		} else {
			startup_info->smem.p_addr = (uint32_t)(ac_strm->smem.p_addr & 0xFFFFFFFF);
			startup_info->smem.p_addr_head = ac_strm->smem.p_addr - SMEM_HEAD_SIZE;
		}
		startup_info->smem.blk_buf_size = ac_strm->smem.periods_size *
			(startup_info->strm_info.channels * startup_info->strm_info.bit_width / 8);
		startup_info->smem.blk_buf_num = (uint16_t)(ac_strm->smem.periods_num & 0xFFFF);
#else
		/* fill stream data format info */
		if ((ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY)) {
			ret = fk_acore_adsp_codec_caps_check(ac_strm);
			if (ret < 0) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "scene(%d) unsupport codec type(%d).",
					ac_strm->adsp_sc_t, ac_strm->fmt.codec_type);
				return ret;
			}

			startup_info->codecInfo.codec_type =
				fk_acore_get_adsp_codec_type(ac_strm->fmt.codec_type);
		} else {
			startup_info->codecInfo.codec_type = CODEC_NONE;
		}
		startup_info->codecInfo.sample_rate = ac_strm->fmt.samples;
		startup_info->codecInfo.channels = (uint8_t)(ac_strm->fmt.channels & 0xFF);
		startup_info->codecInfo.bit_width = (uint8_t)(ac_strm->fmt.bit_width & 0xFF);
		AUD_LOG_DBG(AUD_CORE, "sample(%d) channels(%d) bit width(%d).",
			ac_strm->fmt.samples, ac_strm->fmt.channels,
			ac_strm->fmt.bit_width);

		/* fill share memory info */
		if ((ac_strm->adsp_sc_t == ADSP_SCENE_MMAP_PLAY) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_MMAP_RECORD)) {
			startup_info->smem.p_addr = ac_strm->smem.p_addr - SMEM_HEAD_SIZE;
			startup_info->smem.p_addr_head = ac_strm->mmap_offset_paddr;
		} else {
			startup_info->smem.p_addr = (uint32_t)(ac_strm->smem.p_addr & 0xFFFFFFFF);
			startup_info->smem.p_addr_head = ac_strm->smem.p_addr - SMEM_HEAD_SIZE;
		}
		startup_info->smem.blk_buf_size = ac_strm->smem.periods_size *
			(startup_info->codecInfo.channels * startup_info->codecInfo.bit_width / 8);
		startup_info->smem.blk_buf_num = (uint16_t)(ac_strm->smem.periods_num & 0xFFFF);

#endif

		/* check if share memory valid */
		if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
			int expect_frames = 0;
			int actual_frames = 0;

			/* reuse periods_size in compress scene */
			startup_info->smem.blk_buf_size = ac_strm->smem.periods_size;

			expect_frames = fk_compr_frames_perblk_get(ac_strm,
				startup_info->smem.blk_buf_size);
			actual_frames = fk_acore_get_frames_info();
			if (expect_frames < actual_frames) {
				AUD_LOG_ERR(AUD_CORE, "share memory block size too small(%d).",
					startup_info->smem.blk_buf_size);
			}
		} else if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) {
			startup_info->smem.blk_buf_size = ac_strm->smem.periods_size;
		}

#ifdef STARTUP_INFO_VER_02
		ret = fk_acore_port_set(ac_strm, (void *)&startup_info->port_info, true);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "port set failure.");
			return ret;
		}

		ret = fk_acore_peri_sel_set(ac_strm, (void *)&startup_info->port_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "port slot set failure.");
			return ret;
		}
#else
		ret = fk_acore_port_set(ac_strm, startup_info, 1);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "port set failure.");
			return ret;
		}

		ret = fk_acore_peri_sel_set(ac_strm, startup_info);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "port slot set failure.");
			return ret;
		}
#endif
	}

	/* send starup message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_startup_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send startup message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: acore_stream_shutdown
 * description: shutdown stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_shutdown(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct xring_aud_msg scene_shutdown_msg = {0};

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	/* fill scene trigger message */
	scene_shutdown_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_shutdown_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_shutdown_msg.header.cmd_id = ADSP_AUD_SCENE_SHUTDOWN;
	scene_shutdown_msg.header.len = sizeof(struct xring_aud_msg_header);
	scene_shutdown_msg.header.result = 0;

	/* send starup message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_shutdown_msg);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_CORE, "send shutdown message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: acore_stream_start
 * description: start stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_start(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct xring_aud_msg scene_trigger_msg = {0};

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	/* fill scene startup message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	if ((ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_DL) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_UL)) {
		scene_trigger_msg.header.cmd_id = ADSP_VOICE_CTRL;
		scene_trigger_msg.header.func_id = ADSP_VOICE_CTRL_START;
	} else {
		scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
		scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_START;
	}
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header);
	scene_trigger_msg.header.result = 0;

	/* send starup message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send start message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: acore_stream_pause
 * description: pause stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_pause(struct acore_stream *ac_strm)
{
	int ret = EOK;

	struct xring_aud_msg scene_trigger_msg = {0};

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	/* fill scene pause message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
	scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_PAUSE;
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header);
	scene_trigger_msg.header.result = 0;

	/* send pause message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send pause message failure.");
		return ret;
	}

	/* update stream state */
	ac_strm->state = ACORE_STATE_PAUSE;

	return ret;
}

/************************************************************************
 * function: acore_stream_flush
 * description: flush stream buffer
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_flush(struct acore_stream *ac_strm)
{
	int ret = EOK;

	struct xring_aud_msg scene_trigger_msg = {0};

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	/* fill scene flush message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
	scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_FLUSH;
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header);
	scene_trigger_msg.header.result = 0;

	/* send flush message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send flush message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: acore_stream_resume
 * description: resume stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_resume(struct acore_stream *ac_strm)
{
	int ret = EOK;

	struct xring_aud_msg scene_trigger_msg = {0};

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	/* fill scene resume message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
	scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_RESUME;
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header);
	scene_trigger_msg.header.result = 0;

	/* send resume message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send resume message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: acore_stream_drain
 * description: drain stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_drain(struct acore_stream *ac_strm)
{
	int ret = EOK;

	struct xring_aud_msg scene_trigger_msg = {0};
	struct adsp_last_buf_info *last_buf;

	last_buf = (struct adsp_last_buf_info *)&scene_trigger_msg.data[0];

	/* fill scene drain message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
	scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_LAST_FRAME;
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct adsp_last_buf_info);
	scene_trigger_msg.header.result = 0;

	if (ac_strm->cur_len) {
		last_buf->data_len = ac_strm->cur_len;
		atomic_inc(&ac_strm->put_block_no);
		last_buf->write_offset = atomic_read(&ac_strm->put_block_no);
	} else {
		last_buf->data_len = ac_strm->smem.periods_size;
		last_buf->write_offset = atomic_read(&ac_strm->put_block_no);
	}

	AUD_LOG_DBG(AUD_CORE, "last block: index(%d) len(%d)", last_buf->write_offset,
		last_buf->data_len);

	/* send drain message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_DBG(AUD_CORE, "send drain message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: acore_stream_next_track
 * description: next track stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_next_track(struct acore_stream *ac_strm)
{
	int ret = EOK;

	struct xring_aud_msg scene_trigger_msg = {0};
#ifdef STARTUP_INFO_VER_02
	struct stream_info *decode_info;

	decode_info = (struct stream_info *)&scene_trigger_msg.data[0];
#else
	struct codec_info *decode_info;

	decode_info = (struct codec_info *)&scene_trigger_msg.data[0];
#endif

	/* fill scene drain message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
	scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_SET_NEXT_TRACK;
#ifdef STARTUP_INFO_VER_02
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct stream_info);
#else
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct codec_info);
#endif
	scene_trigger_msg.header.result = 0;

	decode_info->codec_type = ac_strm->fmt.codec_type;
	decode_info->sample_rate = ac_strm->fmt.samples;
	decode_info->channels = ac_strm->fmt.channels;
	decode_info->bit_width = ac_strm->fmt.bit_width;
	AUD_LOG_DBG(AUD_CORE, "new track decode info: type(%d) sample rate(%d)",
		decode_info->codec_type, decode_info->sample_rate);

	/* send drain message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send next track message failure.");
		return ret;
	}

	return ret;
}


/************************************************************************
 * function: acore_stream_stop
 * description: stop stream
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static int acore_stream_stop(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct xring_aud_msg scene_trigger_msg = {0};

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	/* fill scene stop message */
	scene_trigger_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	scene_trigger_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	if ((ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_DL) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_UL)) {
		scene_trigger_msg.header.cmd_id = ADSP_VOICE_CTRL;
		scene_trigger_msg.header.func_id = ADSP_VOICE_CTRL_STOP;
	} else {
		scene_trigger_msg.header.cmd_id = ADSP_AUD_CTRL;
		scene_trigger_msg.header.func_id = ADSP_AUD_CTRL_STOP;
	}
	scene_trigger_msg.header.len = sizeof(struct xring_aud_msg_header);
	scene_trigger_msg.header.result = 0;

	/* send stop message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_trigger_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send stop message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_stream_ops_get
 * description: get stream ops by state
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
struct acore_stream_ops *fk_stream_ops_get(struct acore_stream *ac_strm)
{
	struct acore_stream_ops *ops = NULL;

	if (ac_strm->state >= ACORE_STATE_MAX)
		return NULL;

	ops = (struct acore_stream_ops *)&a_strm_ops[ac_strm->state];
	return ops;
}

