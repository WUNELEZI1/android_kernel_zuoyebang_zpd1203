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
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/rtc.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/timer.h>

#include "fk-acore.h"
#include "fk-acore-process.h"
#include "fk-acore-state.h"
#include "fk-acore-utils.h"

#include "../asoc/fk-dai-be.h"
#include "../common/fk-audio-usb.h"
#include "../common/fk-common-ctrl.h"
#include "../common/fk-common-debugfs.h"
#include "../common/fk-audio-tool-core.h"
#include "../common/fk-audio-memlayout.h"

#include "../adsp/adsp_ssr_driver.h"
#include "../common/audio-trace.h"

#if IS_ENABLED(CONFIG_MIEV)
#include <miev/mievent.h>
#endif
/*UNLOAD & SSR wait_event return val*/
#define VT_RET_VAL			0xFFFFFFFF

#define FRAME_PRINT_FREQ			(100)

static struct acore_client aclient;

#ifdef SIMULATE_DEBUG
static int fk_acore_simulate_write(struct acore_stream *ac_strm);
static int fk_acore_simulate_read(struct acore_stream *ac_strm);
#endif

#ifdef SIMULATE_COMMON
#include "../common/fk-audio-simulate.h"
#endif

static struct wakeup_source *event_ws;

/************************************************************************
 * function: fk_acore_aud_ctrl_handle
 * description: response message process of audio control
 * parameter:
 *		sc_msg : response message content
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
static void fk_acore_aud_ctrl_handle(struct xring_aud_msg *sc_msg,
	struct acore_stream *ac_strm)
{
	enum adsp_ctrl_type func_id = 0;
	struct rsp_payload pay_load = {0};
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;
	uint32_t *cur_timestamp = NULL;
	uint32_t *sample_rate = NULL;

	int cb_flag = 0;

	func_id = (enum adsp_ctrl_type)sc_msg->header.func_id;
	switch (func_id) {
	case ADSP_AUD_CTRL_WRITE:
		if (ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) {
			/* trigger write done callback driectly when scene is ultra */
			AUD_LOG_DBG_LIM(AUD_CORE, "adsp scene(%d) write done unexpected.",
				ac_strm->adsp_sc_t);
		} else {
			struct buf_blk *bufinfo = (struct buf_blk *)&pay_load.payload.buf_info;
			struct adsp_data_buf_info *rsp_data =
				(struct adsp_data_buf_info *)&sc_msg->data[0];

			pay_load.opCode = OP_PLAYBACK_COPY;
			bufinfo->phy_addr = (phys_addr_t)(rsp_data->phy_addr & 0xFFFFFFFF);
			bufinfo->size = rsp_data->len;
			bufinfo->result = sc_msg->header.result;

			if (ac_strm->smem.vir_addr) {
				smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);

				if ((atomic_read(&ac_strm->rel_block_no) % 50) == 0)
					AUD_LOG_DBG(AUD_CORE, "write_done: id(%x) off(%d)(%d) rel_f_no(%d)",
						ac_strm->sess_id, smem_head_ptr->w_off, smem_head_ptr->r_off,
						atomic_read(&ac_strm->rel_block_no));
			}

			/* support return mutil-block at once */
			if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) {
				int blks = 0;
				int rel_blks, put_blks;

				AUD_LOG_DBG(AUD_CORE, "rsp_data->len=%d", rsp_data->len);

				/* Note: the unit of periods_size is bytes in offload scenario*/
				if (rsp_data->len <= ac_strm->smem.periods_size)
					blks = 1;
				else
					blks = rsp_data->len / ac_strm->smem.periods_size;

				rel_blks = atomic_read(&ac_strm->rel_block_no);
				put_blks = atomic_read(&ac_strm->put_block_no);

				if ((rel_blks + blks) > put_blks) {
					AUD_LOG_INFO_LIM(AUD_CORE,
						"under run. blks(%d) rel(%d) put(%d)",
						blks, atomic_read(&ac_strm->rel_block_no),
						atomic_read(&ac_strm->put_block_no));

					if (put_blks > rel_blks) {
						blks = put_blks - rel_blks;
						AUD_LOG_INFO_LIM(AUD_CORE, "put blks(%d)", blks);

						bufinfo->size = ac_strm->smem.periods_size * blks;
						atomic_set(&ac_strm->rel_block_no, put_blks);
						cb_flag = 1;
					}
				} else {
					atomic_add(blks, &ac_strm->rel_block_no);
					cb_flag = 1;
				}
			} else {
				if (atomic_read(&ac_strm->rel_block_no) >=
					atomic_read(&ac_strm->put_block_no)) {
					AUD_LOG_INFO_LIM(AUD_CORE, "under run.rel(%d) put(%d)",
						atomic_read(&ac_strm->rel_block_no),
						atomic_read(&ac_strm->put_block_no));
				} else {
					/* update callback flag and release frame number */
					atomic_inc(&ac_strm->rel_block_no);
					cb_flag = 1;
				}
			}
		}
		break;
	case ADSP_AUD_CTRL_READ:
		if (ac_strm->adsp_sc_t != ADSP_SCENE_ULTRA_FAST_RECORD) {
			struct buf_blk *bufinfo = (struct buf_blk *)&pay_load.payload.buf_info;
			struct adsp_data_buf_info *rsp_data =
				(struct adsp_data_buf_info *)&sc_msg->data[0];

			pay_load.opCode = OP_CAPTURE_COPY;
			bufinfo->phy_addr = (phys_addr_t)(rsp_data->phy_addr & 0xFFFFFFFF);
			bufinfo->result = sc_msg->header.result;
			if (bufinfo->result != RSP_MSG_OK) {
				bufinfo->size = 0;
				AUD_LOG_DBG_LIM(AUD_CORE, "sess(%d) read fail(%d).",
					ac_strm->adsp_sc_t, rsp_data->len);
			} else {
				if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
					if (rsp_data->len > ac_strm->smem.periods_size) {
						AUD_LOG_ERR_LIM(AUD_CORE, "data len %d over",
							rsp_data->len);
						bufinfo->size = ac_strm->smem.periods_size;
						ac_strm->total_len += ac_strm->smem.periods_size;
						ac_strm->cur_timestamp = rsp_data->frame_no;
					} else {
						bufinfo->size = rsp_data->len;

						/* update total length info */
						ac_strm->total_len += rsp_data->len;
						ac_strm->cur_timestamp = rsp_data->frame_no;
					}
				} else {
					bufinfo->size = rsp_data->len;

					/* update total length info */
					ac_strm->total_len += rsp_data->len;
				}

				if ((atomic_read(&ac_strm->put_block_no) % 10) == 0)
					AUD_LOG_DBG(AUD_CORE,
						"read done:id(0x%x) len(%d) blk(%d)(%d)",
						ac_strm->sess_id, rsp_data->len,
						atomic_read(&ac_strm->rel_block_no),
						atomic_read(&ac_strm->put_block_no));
			}

			/* update frame no when overrun */
			if (atomic_read(&ac_strm->rel_block_no) >=
				atomic_read(&ac_strm->put_block_no)) {
				AUD_LOG_INFO_LIM(AUD_CORE, "over run. block:(%d) (%d)",
					atomic_read(&ac_strm->rel_block_no),
					atomic_read(&ac_strm->put_block_no));
			} else {
				/* update callback flag */
				cb_flag = 1;
				atomic_inc(&ac_strm->rel_block_no);
			}
		} else {
			/* trigger read done callback driectly when scene is ultra */
			AUD_LOG_DBG_LIM(AUD_CORE, "adsp scene(%d) read done unexpected.",
				ac_strm->adsp_sc_t);
		}
		break;
	case ADSP_AUD_CTRL_START:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_DBG_LIM(AUD_CORE, "sess(0x%x) start fail.", ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PREPARE;

			pay_load.payload.result = 0xFF;
		} else
			pay_load.payload.result = RSP_MSG_OK;

		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) START RSP.", ac_strm->sess_id);

		pay_load.opCode = OP_START;
		atomic_set(&ac_strm->start_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		cb_flag = 1;
		break;
	case ADSP_AUD_CTRL_STOP:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_INFO(AUD_CORE, "sess(0x%x) stop fail.",
				ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PREPARE;

			pay_load.payload.result = 0xFF;
		}

		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) STOP RSP.", ac_strm->sess_id);

		atomic_set(&ac_strm->stop_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		break;
	case ADSP_AUD_CTRL_PAUSE:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_INFO(AUD_CORE, "sess(0x%x) pause fail.",
				ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PREPARE;
		}

		/* pause operation trigger by stop */
		if (atomic_read(&ac_strm->stop_flag)) {
			pay_load.opCode = OP_STOP;
			cb_flag = 1;
			atomic_set(&ac_strm->stop_flag, 0);
		}

		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) pause ack.", ac_strm->sess_id);
		break;
	case ADSP_AUD_CTRL_RESUME:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_INFO(AUD_CORE, "sess(0x%x) pause fail.",
				ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PAUSE;
		}

		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) resume ack.", ac_strm->sess_id);
		break;
	case ADSP_AUD_CTRL_FLUSH:
		if (sc_msg->header.result != RSP_MSG_OK)
			AUD_LOG_INFO(AUD_CORE, "sess(0x%x) flush fail.", ac_strm->sess_id);

		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) flush ack.", ac_strm->sess_id);

		/* reset share memory head info */
		if (ac_strm->smem.vir_addr) {
			smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
			smem_head_ptr->w_off = 0;
			smem_head_ptr->r_off = 0;
			AUD_LOG_DBG(AUD_CORE, "w_off(%d) r_off(%d)", smem_head_ptr->w_off,
				smem_head_ptr->r_off);
		}

		/* reset put&release block NO */
		atomic_set(&ac_strm->put_block_no, 0);
		atomic_set(&ac_strm->rel_block_no, 0);

		atomic_set(&ac_strm->flush_flag, 0);
		ac_strm->cur_len = 0;

		break;
	case ADSP_AUD_CTRL_SEND_TIMESTAMP:
		cur_timestamp = (uint32_t *)&sc_msg->data[0];
		ac_strm->cur_timestamp = *cur_timestamp;
		sample_rate = (uint32_t *)&sc_msg->data[4];
		ac_strm->fmt.samples = *sample_rate;

		if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY)
			pay_load.payload.sfmt.samples = ac_strm->fmt.samples;

		pay_load.opCode = OP_GET_PARAMS;
		cb_flag = 1;
		/* wakeup timestamp get operation */
		atomic_set(&ac_strm->timestamp_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);
		break;
	case ADSP_AUD_CTRL_LAST_FRAME:
		if (ac_strm->smem.vir_addr) {
			smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
			AUD_LOG_DBG(AUD_CORE, "offset:read(%d) write(%d) flag(%d) len(%d)",
				smem_head_ptr->r_off, smem_head_ptr->w_off,
				smem_head_ptr->flag, smem_head_ptr->len);
		}

		pay_load.opCode = OP_DRAIN;
		cb_flag = 1;

		atomic_set(&ac_strm->last_frame, 1);
		wake_up(&ac_strm->cmd_wait_que);
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "unexpect func_id(%d).", func_id);
		break;
	}

	if (cb_flag && ac_strm->cb)
		ac_strm->cb(ac_strm->sess_id, ac_strm->priv, pay_load);
}

/************************************************************************
 * function: fk_acore_voice_ctrl_handle
 * description: response message process of voice control
 * parameter:
 *		sc_msg : response message content
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
static void fk_acore_voice_ctrl_handle(struct xring_aud_msg *sc_msg,
	struct acore_stream *ac_strm)
{
	enum adsp_ctrl_type func_id = 0;
	struct rsp_payload payload = {0};

	int cb_flag = 0;

	func_id = (enum adsp_ctrl_type)sc_msg->header.func_id;
	switch (func_id) {
	case ADSP_VOICE_CTRL_RECORD:
		AUD_LOG_DBG_LIM(AUD_CORE, "voice record");
		payload.opCode = OP_VOICE_RECORD;

		cb_flag = 1;
		break;
	case ADSP_VOICE_CTRL_START:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_DBG_LIM(AUD_CORE, "sess(0x%x) start fail.", ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PREPARE;

			payload.payload.result = 0xFF;
		}

		payload.opCode = OP_START;
		atomic_set(&ac_strm->start_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		AUD_LOG_DBG(AUD_CORE, "START RSP. wait=%d.",
			atomic_read(&ac_strm->start_wait));

		cb_flag = 1;
		break;
	case ADSP_VOICE_CTRL_STOP:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_INFO(AUD_CORE, "sess(0x%x) stop fail.",
				ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PREPARE;

			payload.payload.result = 0xFF;
		}

		atomic_set(&ac_strm->stop_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		AUD_LOG_DBG(AUD_CORE, "STOP RSP. wait=%d.",
			atomic_read(&ac_strm->stop_wait));

		break;
	default:
		AUD_LOG_DBG_LIM(AUD_CORE, "unexpect func_id(%d).", func_id);
		break;
	}

	if (cb_flag && ac_strm->cb)
		ac_strm->cb(ac_strm->sess_id, ac_strm->priv, payload);
}

/************************************************************************
 * function: fk_acore_param_ctrl_handle
 * description: response message process of voice control
 * parameter:
 *		sc_msg : response message content
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
static void fk_acore_param_ctrl_handle(struct xring_aud_msg *sc_msg,
	struct acore_stream *ac_strm)
{
	enum adsp_ctrl_type func_id = 0;

	func_id = (enum adsp_ctrl_type)sc_msg->header.func_id;
	AUD_LOG_DBG_LIM(AUD_CORE, "func_id(%d).", func_id);
	switch (func_id) {
	case ADSP_PARAM_CTRL_FREE:
		fk_audio_tool_smem_free(ac_strm, sc_msg);
		break;
	default:
		AUD_LOG_DBG_LIM(AUD_CORE, "unexpect func_id(%d).", func_id);
		break;
	}
}

/************************************************************************
 * function: fk_acore_kws_ctrl_handle
 * description: response message process of voice control
 * parameter:
 *		sc_msg : response message content
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
static void fk_acore_kws_ctrl_handle(struct xring_aud_msg *sc_msg,
	struct acore_stream *ac_strm)
{
	enum adsp_ctrl_type func_id = 0;
	struct rsp_payload pay_load = {0};

	int cb_flag = 0;

	func_id = (enum adsp_ctrl_type)sc_msg->header.func_id;
	switch (func_id) {
	case ADSP_KWS_CTRL_START_RECOGNITION:

		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) START RSP.", ac_strm->sess_id);

		pay_load.opCode = OP_START;
		atomic_set(&ac_strm->start_rec_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		cb_flag = 1;
		/*vote adsp poweroff*/
		fk_adsp_vote_unregister(ac_strm->sess_id);
		break;
	case ADSP_KWS_CTRL_STOP_RECOGNITION:
		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) STOP RSP.", ac_strm->sess_id);

		atomic_set(&ac_strm->stop_rec_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);
		break;
	case ADSP_KWS_CTRL_EVT_REPORT:
		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) EVENT RSP.", ac_strm->sess_id);

		if (ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD) {
			struct adsp_data_buf_info *rsp_data = (struct adsp_data_buf_info *)&sc_msg->data[0];

			if (sc_msg->header.result != RSP_MSG_OK) {
				AUD_LOG_DBG_LIM(AUD_CORE, "sess(0x%x) event report fail.", ac_strm->sess_id);
				/* update stream state */
				ac_strm->state = ACORE_STATE_PREPARE;

				pay_load.payload.result = 0xFF;
			}

	/*wakeup 200ms*/
			__pm_wakeup_event(event_ws, EVENT_WAKEUP_TIMEOUT);
			AUD_LOG_DBG(AUD_CORE, "wakeup event 200ms");
			ac_strm->det_event = rsp_data->len;
			pay_load.opCode = OP_START;
			atomic_set(&ac_strm->event_wait, 0);
			wake_up(&ac_strm->cmd_wait_que);

			AUD_LOG_DBG(AUD_CORE, "kws_event RSP. wait=%d.",
				atomic_read(&ac_strm->event_wait));
		}
		break;
	case ADSP_KWS_CTRL_READ:
		if (ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD) {
			struct buf_blk *bufinfo = (struct buf_blk *)&pay_load.payload.buf_info;
			struct adsp_data_buf_info *rsp_data =
				(struct adsp_data_buf_info *)&sc_msg->data[0];

			pay_load.opCode = OP_CAPTURE_COPY;
			bufinfo->phy_addr = (phys_addr_t)(rsp_data->phy_addr & 0xFFFFFFFF);
			bufinfo->result = sc_msg->header.result;
			if (bufinfo->result != RSP_MSG_OK) {
				bufinfo->size = 0;
				AUD_LOG_DBG_LIM(AUD_CORE, "sess(%d) read fail(%d).",
					ac_strm->adsp_sc_t, rsp_data->len);
			} else {
				bufinfo->size = rsp_data->len;

				/* update total length info */
				ac_strm->total_len += rsp_data->len;
			}
			AUD_LOG_DBG_LIM(AUD_CORE,
					"read done:id(0x%x) len(%d) blk(%d)(%d)",
					ac_strm->sess_id, rsp_data->len,
					atomic_read(&ac_strm->rel_block_no),
					atomic_read(&ac_strm->put_block_no));
				/* update callback flag */
				cb_flag = 1;
				atomic_inc(&ac_strm->rel_block_no);
		} else
			/* trigger read done callback driectly when scene is ultra */
			AUD_LOG_DBG_LIM(AUD_CORE, "adsp scene(%d) read done unexpected.",
				ac_strm->adsp_sc_t);
		break;
	case ADSP_KWS_CTRL_UNLOAD_MODEL:
		break;
	case ADSP_KWS_CTRL_VT_FCA_FLAG:
		AUD_LOG_INFO(AUD_CORE, "sess(0x%x) SET FLAG RSP.", ac_strm->sess_id);

		atomic_set(&ac_strm->vt_fca_flag_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);
		break;
	default:
		AUD_LOG_DBG_LIM(AUD_CORE, "unexpect func_id(%d).", func_id);
		break;
	}

	if (cb_flag && ac_strm->cb)
		ac_strm->cb(ac_strm->sess_id, ac_strm->priv, pay_load);

	AUD_LOG_DBG_LIM(AUD_CORE, "exit");
}

/************************************************************************
 * function: fk_acore_hw_module_ctrl_handle
 * description: response message process of voice control
 * parameter:
 *		sc_msg : response message content
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
static void fk_acore_hw_module_ctrl_handle(struct xring_aud_msg *sc_msg,
	struct acore_stream *ac_strm)
{
	enum adsp_ctrl_type func_id = 0;

	func_id = (enum adsp_ctrl_type)sc_msg->header.func_id;
	switch (func_id) {
	case ADSP_SET_DEV_PATH:
		/*for kws switch dev*/
		if (ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD) {
			/*vote adsp poweroff*/
			fk_adsp_vote_unregister(ac_strm->sess_id);
		}
		break;
	case ADSP_SET_USB_CONNECT:
	case ADSP_SET_USB_DISCONNECT:
		audio_usb_status_notify_complete(func_id);
		break;
	default:
		AUD_LOG_DBG_LIM(AUD_CORE, "unexpect func_id(%d).", func_id);
		break;

	}
}

/************************************************************************
 * function: fk_acore_sys_ctrl_handle
 * description: response message process of voice control
 * parameter:
 *		sc_msg : response message content
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
static void fk_acore_sys_ctrl_handle(struct xring_aud_msg *sc_msg,
	struct acore_stream *ac_strm)
{
	enum adsp_ctrl_type func_id = 0;

	func_id = (enum adsp_ctrl_type)sc_msg->header.func_id;
	switch (func_id) {
	case ADSP_SYS_CTRL_AP_WAKEUP:
	/*for adsp wakeup ap*/
	if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY)
		AUD_LOG_DBG(AUD_CORE, "offload play scene adsp wakeup ap.");
		break;
	default:
		AUD_LOG_DBG_LIM(AUD_CORE, "unexpect func_id(%d).", func_id);
		break;
	}
}


/************************************************************************
 * function: fk_acore_get_rtc_time
 * description: get rtc time and send this msg to adsp
 * parameter:
 *		sc_msg : aud msg form adsp
 * return:
 ************************************************************************/
static void fk_acore_get_rtc_time(struct xring_aud_msg *sc_msg)
{
	struct xring_aud_msg scene_get_rtc_msg = {0};
	struct rtc_time_s *cur_time;
	struct rtc_device *rtc;
	ssize_t retval;
	struct rtc_time tm;
	int ret = 0;

	/* fill scene get rtc time message */
	scene_get_rtc_msg.header.scene_id = sc_msg->header.scene_id;
	scene_get_rtc_msg.header.ins_id = sc_msg->header.ins_id;
	scene_get_rtc_msg.header.cmd_id = ADSP_SEND_RTC_TIME;
	scene_get_rtc_msg.header.result = 0;
	cur_time = (struct rtc_time_s *)&scene_get_rtc_msg.data[0];
	scene_get_rtc_msg.header.len = sizeof(struct xring_aud_msg_header) +
				sizeof(struct rtc_time_s);

	rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	if (rtc == NULL) {
		AUD_LOG_INFO_LIM(AUD_CORE, "failed to open rtc device %s\n",
					CONFIG_RTC_HCTOSYS_DEVICE);
		return;
	}

	retval = rtc_read_time(rtc, &tm);
	if (retval)
		AUD_LOG_INFO_LIM(AUD_CORE, "read rtc time failed");

	cur_time->rtc_year = tm.tm_year;
	cur_time->rtc_month = tm.tm_mon;
	cur_time->rtc_day = tm.tm_mday;
	cur_time->rtc_hour = tm.tm_hour;
	cur_time->rtc_min = tm.tm_min;
	cur_time->rtc_sec = tm.tm_sec;
	AUD_LOG_DBG_LIM(AUD_CORE,
		"cur_time, year=%d, month=%d, day=%d, hour=%d, min=%d, sec=%d",
		cur_time->rtc_year, cur_time->rtc_month,
		cur_time->rtc_day, cur_time->rtc_hour,
		cur_time->rtc_min, cur_time->rtc_sec);
	rtc_class_close(rtc);

	ret = fk_acore_message_send((struct xring_aud_msg *)&scene_get_rtc_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send scene_get_rtc_msg failure.");
		return;
	}

}

/************************************************************************
 * function: fk_acore_adsp_panic_handle
 * description: adsp panic callback
 * parameter:
 *		sc_msg : aud msg form adsp
 * return:
 ************************************************************************/
static void fk_acore_adsp_panic_handle(struct xring_aud_msg *sc_msg)
{
	struct dsp_assert_info *adsp_assert_info = NULL;

	if ((sc_msg->header.cmd_id == ADSP_DEBUG_CTRL) &&
		(sc_msg->header.func_id == ADSP_DEBUG_CTRL_DSP_PANIC)) {
		adsp_assert_info = (struct dsp_assert_info *)&sc_msg->data[0];
		AUD_LOG_ERR(AUD_CORE, "tcb_name:      [%s]", adsp_assert_info->tcb_name);
		AUD_LOG_ERR(AUD_CORE, "int_status:    [0x%x]", adsp_assert_info->int_status);
		AUD_LOG_ERR(AUD_CORE, "scene_flag:    [0x%x]", adsp_assert_info->scene_flag);
		AUD_LOG_ERR(AUD_CORE, "stackoverflow: [0x%x]", adsp_assert_info->stackoverflow);
		AUD_LOG_ERR(AUD_CORE, "version:       [0x%x]", adsp_assert_info->version);
		AUD_LOG_ERR(AUD_CORE, "exccause:      [0x%x]", adsp_assert_info->exccause);
		AUD_LOG_ERR(AUD_CORE, "excvaddr:      [0x%x]", adsp_assert_info->excvaddr);
		AUD_LOG_ERR(AUD_CORE, "excpc:         [0x%x]", adsp_assert_info->excpc);
		AUD_LOG_ERR(AUD_CORE, "reserve0:      [0x%x]", adsp_assert_info->reserve0);
		AUD_LOG_ERR(AUD_CORE, "reserve1:      [0x%x]", adsp_assert_info->reserve1);
		AUD_LOG_ERR(AUD_CORE, "reserve2:      [0x%x]", adsp_assert_info->reserve2);

		adsp_reset(adsp_assert_info->exccause);
	}
}

/************************************************************************
 * function: fk_acore_set_pcie_bar_addr
 * description: set pcie bar addr to adsp in voice call
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_acore_set_pcie_bar_addr(struct acore_stream *ac_strm, enum voice_type voice_type)
{
	int ret = 0;
	struct xring_aud_msg pcie_bar_msg = {0};
	struct voice_pcie_bar_info *pcie_bar_info;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_UL)) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	/* fill message */
	pcie_bar_msg.header.scene_id = ADSP_SCENE_NONE;
	pcie_bar_msg.header.cmd_id = ADSP_VOICE_CTRL;
	pcie_bar_msg.header.func_id = ADSP_VOICE_CTRL_PCIE_BAR;
	pcie_bar_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(struct voice_pcie_bar_info);
	pcie_bar_msg.header.result = 0;

	/* update pcie bar addr */
	pcie_bar_info = (struct voice_pcie_bar_info *)&pcie_bar_msg.data[0];
	pcie_bar_info->BAR0_1_addr = ac_strm->bar_info.BAR0_1_addr;
	pcie_bar_info->BAR2_3_addr = ac_strm->bar_info.BAR2_3_addr;
	pcie_bar_info->voice_type = voice_type;

	AUD_LOG_INFO(AUD_CORE, "set pcie bar, BAR0_1_addr %llu, BAR2_3_addr %llu\n",
		pcie_bar_info->BAR0_1_addr, pcie_bar_info->BAR2_3_addr);
	/* send pause message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&pcie_bar_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send pcie bar info message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_voice_speech_on
 * description: set voice call speech on (off)
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_acore_voice_speech_on_off(struct acore_stream *ac_strm, bool speech_status)
{
	int ret = 0;
	struct xring_aud_msg voice_speech_msg = {0};

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_UL)) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	/* fill message */
	voice_speech_msg.header.scene_id = ADSP_SCENE_NONE;
	voice_speech_msg.header.cmd_id = ADSP_VOICE_CTRL;
	if (speech_status == true)
		voice_speech_msg.header.func_id = ADSP_VOICE_CTRL_SPEECH_ON;
	else
		voice_speech_msg.header.func_id = ADSP_VOICE_CTRL_SPEECH_OFF;

	voice_speech_msg.header.len = sizeof(struct xring_aud_msg_header);
	voice_speech_msg.header.result = 0;

	/* send pause message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&voice_speech_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send voice speech on or off message failure.");
		return ret;
	}

	ac_strm->speech_status = speech_status;

	return ret;
}

/************************************************************************
 * function: fk_acore_msg_handle
 * description: response message process
 * parameter:
 *		msg : response message content
 *		len : length of message
 *		priv : private parameters of ipc callback
 * return:
 ************************************************************************/
void fk_acore_msg_handle(void *msg, int len, void *priv)
{
	struct xring_aud_msg *sc_msg = (struct xring_aud_msg *)msg;
	struct acore_stream *ac_strm = NULL;

	enum adsp_scene_ctrl_type sc_ctrl_type = 0;
	struct rsp_payload pay_load = {0};

	if (len != sc_msg->header.len)
		AUD_LOG_INFO_LIM(AUD_CORE, "len unmatch.(%d) (%d)", len, sc_msg->header.len);

	sc_ctrl_type = (enum adsp_scene_ctrl_type)sc_msg->header.cmd_id;
	/* adsp get rtc time */
	if (sc_ctrl_type == ADSP_GET_RTC_TIME) {
		fk_acore_get_rtc_time(sc_msg);
		return;
	}
	if (sc_ctrl_type == ADSP_SEND_RTC_TIME)
		return;

	if (sc_ctrl_type == ADSP_DEBUG_CTRL)
		fk_acore_adsp_panic_handle(sc_msg);

	/* get audio core stream by session id */
	ac_strm = fk_acore_stream_get_by_scene(sc_msg->header.scene_id, sc_msg->header.ins_id);
	if (!ac_strm || !atomic_read(&ac_strm->init_flag)) {
		AUD_LOG_INFO_LIM(AUD_CORE, "scene_id(0x%x) ins_id(0x%x) idle.",
			sc_msg->header.scene_id, sc_msg->header.ins_id);
		return;
	}

	switch (sc_ctrl_type) {
	case ADSP_AUD_SCENE_STARTUP:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_INFO_LIM(AUD_CORE, "sess(0x%x) startup fail.", ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_IDLE;
			pay_load.payload.result = 0xFF;
		} else {
			pay_load.payload.result = RSP_MSG_OK;
		}

		AUD_LOG_INFO(AUD_CORE, "sess(%x) STARTUP RSP", ac_strm->sess_id);

		pay_load.opCode = OP_STARTUP;
		atomic_set(&ac_strm->startup_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		if (ac_strm->cb)
			ac_strm->cb(ac_strm->sess_id, ac_strm->priv, pay_load);
		break;
	case ADSP_AUD_SCENE_SHUTDOWN:
		if (sc_msg->header.result != RSP_MSG_OK) {
			AUD_LOG_INFO_LIM(AUD_CORE, "sess(0x%x) start fail.", ac_strm->sess_id);
			/* update stream state */
			ac_strm->state = ACORE_STATE_PREPARE;
			pay_load.payload.result = 0xFF;
		} else {
			pay_load.payload.result = RSP_MSG_OK;
		}

		if (ac_strm->cb)
			ac_strm->cb(ac_strm->sess_id, ac_strm->priv, pay_load);

		AUD_LOG_INFO(AUD_CORE, "sess(%x) SHUTDOWN RSP", ac_strm->sess_id);

		pay_load.opCode = OP_UNREGISTER;
		atomic_set(&ac_strm->shutdown_wait, 0);
		wake_up(&ac_strm->cmd_wait_que);

		break;
	case ADSP_AUD_CTRL:
		fk_acore_aud_ctrl_handle(sc_msg, ac_strm);
		break;
	case ADSP_VOICE_CTRL:
		fk_acore_voice_ctrl_handle(sc_msg, ac_strm);
		break;
	case ADSP_PARAM_CTRL:
		fk_acore_param_ctrl_handle(sc_msg, ac_strm);
		break;
	case ADSP_KWS_CTRL:
		fk_acore_kws_ctrl_handle(sc_msg, ac_strm);
		break;
	case ADSP_HW_MODULE_CTRL:
		fk_acore_hw_module_ctrl_handle(sc_msg, ac_strm);
		break;
	case ADSP_SYS_CTRL:
		fk_acore_sys_ctrl_handle(sc_msg, ac_strm);
		break;
	default:
		break;
	}
}

/************************************************************************
 * function: fk_acore_get_scene_id
 * description: get scene id by session id
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 * return:
 *		0 : success
 *		< 0 : failure
 ************************************************************************/
int fk_acore_get_scene_id(uint16_t session_id)
{
	int ret = -EINVAL;
	uint8_t scene_type = 0;
	uint8_t stream_id = 0;

	/* session_id:
	 *     bit[15:8] : scene type
	 *     bit[7:0] : stream id, use to support multi-instance.
	 */
	scene_type = (session_id & 0xFF00) >> 8;
	stream_id = session_id & 0xFF;

	AUD_LOG_DBG_LIM(AUD_CORE, "scene_type(0x%x) stream_id(0x%x).",
		scene_type, stream_id);

	switch (scene_type) {
	case SCENE_PCM_PLAYBACK:
		switch (stream_id) {
		case PCM_PLAYBACK_ULTRA_LOWLATENCY:
			ret = ADSP_SCENE_ULTRA_FAST_PLAY;
			break;
		case PCM_PLAYBACK_LOWLATENCY:
			ret = ADSP_SCENE_FAST_PLAY;
			break;
		case PCM_PLAYBACK_NORMAL:
		case PCM_PLAYBACK_NORMAL_1:
			ret = ADSP_SCENE_NORMAL_PLAY;
			break;
		case PCM_PLAYBACK_DIRECT:
			ret = ADSP_SCENE_DIRECT_PLAY;
			break;
		default:
			AUD_LOG_INFO_LIM(AUD_CORE, "pcm playback stream(0x%x) unexpect.",
				stream_id);
			ret = -EINVAL;
			break;
		}
		break;
	case SCENE_PCM_CAPTURE:
		switch (stream_id) {
		case PCM_CAPTURE_ULTRA_LOWLATENCY:
			ret = ADSP_SCENE_ULTRA_FAST_RECORD;
			break;
		case PCM_CAPTURE_LOWLATENCY:
			ret = ADSP_SCENE_FAST_RECORD;
			break;
		case PCM_CAPTURE_NORMAL:
		case PCM_CAPTURE_NORMAL_1:
			ret = ADSP_SCENE_NORMAL_RECORD;
			break;
		case PCM_CAPTURE_VOICE_TRIGGER:
		case PCM_CAPTURE_ASR:
			ret = ADSP_SCENE_KWS_RECORD;
			break;
		default:
			AUD_LOG_INFO_LIM(AUD_CORE, "pcm capture stream(0x%x) unexpect.",
				stream_id);
			ret = -EINVAL;
			break;
		}
		break;
	case SCENE_COMPR_CAPTURE:
		ret = ADSP_SCENE_OFFLOAD_RECORD;
		break;
	case SCENE_COMPR_PLAYBACK:
		ret = ADSP_SCENE_OFFLOAD_PLAY;
		break;
	case SCENE_VOIP_CAPTURE:
		ret = ADSP_SCENE_VOIP_UL;
		break;
	case SCENE_VOIP_PLAYBACK:
		ret = ADSP_SCENE_VOIP_DL;
		break;
	case SCENE_VOICE_CAPTURE:
		ret = ADSP_SCENE_VOICE_CALL_UL;
		break;
	case SCENE_VOICE_PLAYBACK:
		ret = ADSP_SCENE_VOICE_CALL_DL;
		break;
	case SCENE_HPF:
		ret = ADSP_SCENE_HFP;
		break;
	case SCENE_LOOPBACK:
		ret = ADSP_SCENE_LOOPBACK;
		break;
	case SCENE_HAPTIC:
		ret = ADSP_SCENE_HAPTIC;
		break;
	case SCENE_MMAP_PLAYBACK:
			ret = ADSP_SCENE_MMAP_PLAY;
			break;
	case SCENE_MMAP_CAPTURE:
			ret = ADSP_SCENE_MMAP_RECORD;
			break;
	case SCENE_SPATIAL:
		ret = ADSP_SCENE_SPATIAL_PLAY;
		break;
	case SCENE_KARAOKE_PLAYBACK:
		ret = ADSP_SCENE_KARAOKE_DL;
		break;
	case SCENE_KARAOKE_CAPTURE:
		ret = ADSP_SCENE_KARAOKE_UL;
		break;
	case SCENE_INCALL_PLAYBACK:
		ret = ADSP_SCENE_MUSIC_INCALL_PLAY;
		break;
	case SCENE_INCALL_CAPTURE:
		ret = ADSP_SCENE_VOICE_CALL_RECORD;
		break;
	case SCENE_HF_PLAYBACK:
			ret = ADSP_SCENE_HF_DL;
			break;
	case SCENE_HF_CAPTURE:
			ret = ADSP_SCENE_HF_UL;
			break;
	default:
		AUD_LOG_INFO_LIM(AUD_CORE, "session id(0x%x) unexpect.", session_id);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_get_ins_id
 * description: get instance id by session id
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 * return:
 *		>= 0 : index of instance
 *		< 0 : failure
 ************************************************************************/
int fk_acore_get_ins_id(uint16_t session_id)
{
	int ret = EOK;
	uint8_t scene_type = 0;
	uint8_t stream_id = 0;

	AUD_LOG_DBG_LIM(AUD_CORE, "session id is 0x%x.", session_id);

	/* session_id:
	 *     bit[15:8] : scene type
	 *     bit[7:0] : stream id, use to support multi-instance.
	 */
	scene_type = (session_id & 0xFF00) >> 8;
	stream_id = session_id & 0xFF;

	switch (scene_type) {
	case SCENE_PCM_PLAYBACK:
		switch (stream_id) {
		/* TODO: will supplement the definition of multiple instances in normal scene */
		case PCM_PLAYBACK_NORMAL:
		case PCM_PLAYBACK_LOWLATENCY:
		case PCM_PLAYBACK_ULTRA_LOWLATENCY:
		case PCM_PLAYBACK_DIRECT:
			ret = STRM_INS_1;
			break;
		case PCM_PLAYBACK_NORMAL_1:
			ret = STRM_INS_2;
			break;
		default:
			AUD_LOG_INFO_LIM(AUD_CORE, "pcm playback stream(0x%x) unexpect.",
				stream_id);
			ret = -EINVAL;
			break;
		}
		break;
	case SCENE_PCM_CAPTURE:
		switch (stream_id) {
		case PCM_CAPTURE_ULTRA_LOWLATENCY:
		case PCM_CAPTURE_LOWLATENCY:
		case PCM_CAPTURE_NORMAL:
		case PCM_CAPTURE_VOICE_TRIGGER:
			ret = STRM_INS_1;
			break;
		case PCM_CAPTURE_NORMAL_1:
		case PCM_CAPTURE_ASR:
			ret = STRM_INS_2;
			break;
		default:
			AUD_LOG_INFO_LIM(AUD_CORE, "pcm capture stream(0x%x) unexpect.",
				stream_id);
			ret = -EINVAL;
			break;
		}
		break;
	case SCENE_COMPR_CAPTURE:
	case SCENE_COMPR_PLAYBACK:
	case SCENE_VOIP_CAPTURE:
	case SCENE_VOIP_PLAYBACK:
	case SCENE_VOICE_CAPTURE:
	case SCENE_VOICE_PLAYBACK:
	case SCENE_HPF:
	case SCENE_LOOPBACK:
	case SCENE_HAPTIC:
	case SCENE_MMAP_PLAYBACK:
	case SCENE_MMAP_CAPTURE:
	case SCENE_SPATIAL:
	case SCENE_KARAOKE_CAPTURE:
	case SCENE_KARAOKE_PLAYBACK:
	case SCENE_INCALL_PLAYBACK:
	case SCENE_INCALL_CAPTURE:
	case SCENE_HF_PLAYBACK:
	case SCENE_HF_CAPTURE:
		ret = STRM_INS_1;
		break;
	default:
		AUD_LOG_INFO_LIM(AUD_CORE, "session id(0x%x) unexpect.",
			session_id);
		ret = 0;
		break;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_get_stream_dir
 * description: get direct of stream by session id
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 * return:
 *		<= 0: failure
 *		STRM_DIR_PLAYBACK: playback
 *		STRM_DIR_CAPTURE: capture
 ************************************************************************/
int fk_acore_get_stream_dir(uint16_t session_id)
{
	int ret = STRM_DIR_MAX;
	uint8_t scene_type = 0;

	AUD_LOG_DBG_LIM(AUD_CORE, "session id is 0x%x.", session_id);

	scene_type = (session_id & 0xFF00) >> 8;
	switch (scene_type) {
	case SCENE_PCM_CAPTURE:
	case SCENE_COMPR_CAPTURE:
	case SCENE_VOIP_CAPTURE:
	case SCENE_VOICE_CAPTURE:
	case SCENE_MMAP_CAPTURE:
	case SCENE_KARAOKE_CAPTURE:
	case SCENE_INCALL_CAPTURE:
	case SCENE_HF_CAPTURE:
		ret = STRM_DIR_CAPTURE;
		break;
	case SCENE_PCM_PLAYBACK:
	case SCENE_COMPR_PLAYBACK:
	case SCENE_VOIP_PLAYBACK:
	case SCENE_VOICE_PLAYBACK:
	case SCENE_HAPTIC:
	case SCENE_MMAP_PLAYBACK:
	case SCENE_SPATIAL:
	case SCENE_KARAOKE_PLAYBACK:
	case SCENE_INCALL_PLAYBACK:
	case SCENE_HF_PLAYBACK:
		ret = STRM_DIR_PLAYBACK;
		break;
	case SCENE_LOOPBACK:
	case SCENE_HPF:
		ret = STRM_DIR_DUAL;
		break;
	default:
		ret = -1;
		AUD_LOG_INFO_LIM(AUD_CORE, "session id(0x%x) unexpect.", session_id);
		break;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_get
 * description: get stream instance by session id
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 * return:
 *		NULL: failure
 *		other: success
 ************************************************************************/
struct acore_stream *fk_acore_stream_get(uint16_t session_id)
{
	struct acore_stream *acStrm = NULL;

	if (!aclient.init_flag) {
		AUD_LOG_INFO_LIM(AUD_CORE, "acore uninit");
		return NULL;
	}

	mutex_lock(&aclient.list_lock);

	list_for_each_entry(acStrm, &aclient.ac_strm_list, strm_list) {
		if (acStrm->sess_id == session_id) {
			mutex_unlock(&aclient.list_lock);
			//AC_LOG_DBG_LIM("get session(0x%x).", acStrm->sess_id);
			return acStrm;
		}
	}

	mutex_unlock(&aclient.list_lock);

	return NULL;
}

/************************************************************************
 * function: fk_acore_stream_get_by_scene
 * description: get stream instance by scene id and instance id
 * parameter:
 *		scene_id: adsp scene type id.		ins_id: instance id
 * return:
 *		NULL: failure
 *		other: success
 * Note:
 *		need remove audio client lock when in irq env
 ************************************************************************/
struct acore_stream *fk_acore_stream_get_by_scene(uint8_t scene_id, uint8_t ins_id)
{
	struct acore_stream *acStrm = NULL;

	//AUD_LOG_DBG_LIM(AUD_CORE, "scene_id(0x%x) ins_id(0x%x).", scene_id, ins_id);
	mutex_lock(&aclient.list_lock);

	list_for_each_entry(acStrm, &aclient.ac_strm_list, strm_list) {
		if ((acStrm->adsp_sc_t == scene_id) && (acStrm->adsp_ins_id == ins_id)) {
			mutex_unlock(&aclient.list_lock);
			return acStrm;
		}
	}

	mutex_unlock(&aclient.list_lock);

	return NULL;
}

/************************************************************************
 * function: fk_acore_stream_alloc
 * description: alloc stream instance, and bind by session id
 * parameter:
 *		session_id : identify stream
 *			bit[15:8]: scene id.		bit[7:0]: stream id
 * return:
 *		NULL: failure
 *		other: success
 ************************************************************************/
struct acore_stream *fk_acore_stream_alloc(uint16_t session_id)
{
	struct acore_stream *acStrm = NULL;
	int strm_dir = 0;
	int sc_type = 0;
	int ins_id = 0;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", session_id);

	if (atomic_read(&aclient.ssr_flag)) {
		AUD_LOG_INFO(AUD_CORE, "adsp reset");
		return NULL;
	}

	/* update stream direction */
	strm_dir = fk_acore_get_stream_dir(session_id);
	if (strm_dir < 0) {
		AUD_LOG_ERR(AUD_CORE, "get stream dir fail(0x%x).", session_id);
		return NULL;
	}

	/* update stream scene type by session_id */
	sc_type = fk_acore_get_scene_id(session_id);
	if (sc_type < 0) {
		AUD_LOG_ERR(AUD_CORE, "session_id(0x%x) get scene fail.", session_id);
		return NULL;
	}

	/* update scene instance id by session_id */
	ins_id = fk_acore_get_ins_id(session_id);
	if (ins_id < 0) {
		AUD_LOG_ERR(AUD_CORE, "session_id(0x%x) get ins fail.", session_id);
		return NULL;
	}

	acStrm = kzalloc(sizeof(struct acore_stream), GFP_KERNEL);
	if (!acStrm) {
		AUD_LOG_ERR(AUD_CORE, "alloc struct fail: session id(0x%x).", session_id);
		return NULL;
	}

	spin_lock_init(&acStrm->strm_lock);

	/* init audio core stream */
	acStrm->fe_flag = 0;
	acStrm->be_flag = 0;
	acStrm->cb = NULL;
	acStrm->bypass_dsp = 0;
	acStrm->state = ACORE_STATE_IDLE;

	atomic_set(&acStrm->put_block_no, 0);
	atomic_set(&acStrm->rel_block_no, 0);
	acStrm->cur_len = 0;
	acStrm->total_len = 0;
	acStrm->cur_timestamp = 0;

	atomic_set(&acStrm->write_flag, 0);
	atomic_set(&acStrm->start_flag, 0);
	atomic_set(&acStrm->stop_flag, 0);
	atomic_set(&acStrm->resume_flag, 0);
	atomic_set(&acStrm->flush_flag, 0);
	atomic_set(&acStrm->last_frame, 0);

	/* init wait condition */
	init_waitqueue_head(&acStrm->cmd_wait_que);
	atomic_set(&acStrm->startup_wait, 0);
	atomic_set(&acStrm->start_wait, 0);
	atomic_set(&acStrm->stop_wait, 0);
	atomic_set(&acStrm->shutdown_wait, 0);
	atomic_set(&acStrm->timestamp_wait, 0);
	atomic_set(&acStrm->event_wait, 0);

	/* init stream mutex */
	mutex_init(&acStrm->cap_lock);
	mutex_init(&acStrm->play_lock);

	mutex_lock(&aclient.list_lock);
	list_add_tail(&acStrm->strm_list, &aclient.ac_strm_list);
	mutex_unlock(&aclient.list_lock);

	atomic_inc(&aclient.ac_num);
	/* update session id */
	acStrm->sess_id = session_id;

	/* update stream direction info */
	acStrm->strm_dir = (enum stream_dir)strm_dir;

	/* update scene type */
	acStrm->adsp_sc_t = (enum adsp_scene_type)sc_type;
	if (sc_type == ADSP_SCENE_OFFLOAD_PLAY) {
		/* init use gapless state */
		acStrm->use_gapless = 0;
		acStrm->initial_samples = 0;
		acStrm->trailing_samples = 0;
	}

	if ((sc_type == ADSP_SCENE_MMAP_PLAY) ||
		(sc_type == ADSP_SCENE_MMAP_RECORD)) {
		acStrm->mmap_offset_vaddr = fk_get_audio_static_mem(MMAP_OFFSET, &acStrm->mmap_offset_paddr, MMAP_OFFSET_SIZE);
		AUD_LOG_DBG(AUD_CORE, "acStrm->mmap_offset_paddr 0x%x.", acStrm->mmap_offset_paddr);
	}

	/* update instance id */
	acStrm->adsp_ins_id = (uint8_t)ins_id;

	acStrm->speech_status = false;

	atomic_set(&acStrm->init_flag, 1);

	return acStrm;
}

/************************************************************************
 * function: fk_acore_stream_prepare
 * description: prepare stream but not need alloc stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		EOK: success
 ************************************************************************/
int fk_acore_stream_prepare(struct acore_stream *ac_strm)
{
	int ret = EOK;
	unsigned long flags;

	AUD_LOG_INFO(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (atomic_read(&aclient.ssr_flag)) {
		AUD_LOG_INFO(AUD_CORE, "adsp reset");
		return -1;
	}

	spin_lock_irqsave(&ac_strm->strm_lock, flags);

	ac_strm->state = ACORE_STATE_IDLE;

	atomic_inc(&aclient.ac_num);

	atomic_set(&ac_strm->init_flag, 1);

	spin_unlock_irqrestore(&ac_strm->strm_lock, flags);

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_idle
 * description: free stream but not destroy stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		EOK: success
 ************************************************************************/
void fk_acore_stream_idle(struct acore_stream *ac_strm)
{
	unsigned long flags;

	AUD_LOG_INFO(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	spin_lock_irqsave(&ac_strm->strm_lock, flags);

	atomic_set(&ac_strm->init_flag, 0);

	atomic_dec(&aclient.ac_num);

	/* init audio core stream */
	ac_strm->fe_flag = 0;
	ac_strm->be_flag = 0;
	ac_strm->cb = NULL;
	ac_strm->bypass_dsp = 0;
	ac_strm->state = ACORE_STATE_IDLE;

	atomic_set(&ac_strm->put_block_no, 0);
	atomic_set(&ac_strm->rel_block_no, 0);
	ac_strm->cur_len = 0;
	ac_strm->total_len = 0;
	ac_strm->cur_timestamp = 0;

	atomic_set(&ac_strm->write_flag, 0);
	atomic_set(&ac_strm->start_flag, 0);
	atomic_set(&ac_strm->stop_flag, 0);
	atomic_set(&ac_strm->resume_flag, 0);
	atomic_set(&ac_strm->flush_flag, 0);
	atomic_set(&ac_strm->last_frame, 0);

	atomic_set(&ac_strm->startup_wait, 0);
	atomic_set(&ac_strm->start_wait, 0);
	atomic_set(&ac_strm->stop_wait, 0);
	atomic_set(&ac_strm->shutdown_wait, 0);
	atomic_set(&ac_strm->timestamp_wait, 0);
	atomic_set(&ac_strm->event_wait, 0);
	atomic_set(&ac_strm->start_rec_wait, 0);
	atomic_set(&ac_strm->stop_rec_wait, 0);

	ac_strm->speech_status = false;

	/* reset smem info */
	ac_strm->smem.vir_addr = NULL;
	ac_strm->smem.p_addr = 0;

	spin_unlock_irqrestore(&ac_strm->strm_lock, flags);
}

/************************************************************************
 * function: fk_acore_stream_free
 * description: free stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		NULL: failure
 *		other: success
 ************************************************************************/
int fk_acore_stream_free(struct acore_stream *ac_strm)
{
	int ret = EOK;

	AUD_LOG_DBG_LIM(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	mutex_lock(&aclient.list_lock);
	list_del(&ac_strm->strm_list);
	mutex_unlock(&aclient.list_lock);

	atomic_dec(&aclient.ac_num);

	kfree(ac_strm);

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_free_all
 * description: free all stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		NULL: failure
 *		other: success
 ************************************************************************/
void fk_acore_stream_free_all(void)
{
	struct acore_stream *acStrm = NULL;
	struct acore_stream *acStrm_temp;

	AUD_LOG_INFO(AUD_CORE, "begin.");

	if (!aclient.init_flag) {
		AUD_LOG_INFO(AUD_CORE, "acore uninit");
		return;
	}

	mutex_lock(&aclient.list_lock);

	list_for_each_entry_safe(acStrm, acStrm_temp, &aclient.ac_strm_list, strm_list) {
		list_del(&acStrm->strm_list);
		kfree(acStrm);
	}

	mutex_unlock(&aclient.list_lock);
}

/************************************************************************
 * function: fk_acore_stream_restartup
 * description: operation startup by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_restartup(struct acore_stream *ac_strm)
{
	int ret = EOK;
	int cnt = 0;

	AUD_LOG_DBG(AUD_CORE, "restart session(0x%x)", ac_strm->sess_id);

	while (atomic_read(&ac_strm->flush_flag)) {
		AUD_LOG_INFO_LIM(AUD_CORE, "wait for flush completed");
		/* TODO: modify later */
		msleep(100);
		if (cnt++ > 10)
			break;
	}

	/* trigger stop before shutdown */
	ret = fk_acore_stream_trigger_stop(ac_strm);
	if (ret < 0)
		AUD_LOG_ERR(AUD_CORE, "no trigger stop");

	/* shutdown stream */
	ret = fk_acore_stream_shutdown(ac_strm);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_CORE, "stream shutdown failure");
		return -EINVAL;
	}

	atomic_set(&ac_strm->put_block_no, 0);
	atomic_set(&ac_strm->rel_block_no, 0);

	ret = fk_acore_stream_startup(ac_strm);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_CORE, "stream startup failure");
		return -EINVAL;
	}

	AUD_LOG_DBG(AUD_CORE, "complete.");

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_startup
 * description: operation startup by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_startup(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;
#if IS_ENABLED(CONFIG_MIEV)
	struct misight_mievent *mievent;
#endif

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (atomic_read(&aclient.ssr_flag)) {
		AUD_LOG_INFO(AUD_CORE, "adsp reset");
		return -1;
	}

	if (ac_strm->adsp_sc_t == ADSP_SCENE_LOOPBACK) {
		enum loopback_type lp_type = LP_TYPE_PCM_ALGO;

		lp_type = fk_loopback_type_query();
		if (lp_type >= LP_TYPE_MAX) {
			AUD_LOG_INFO(AUD_CORE, "loopback type(%d) unexpect.", lp_type);
			lp_type = LP_TYPE_PCM_ALGO;
		}
		ac_strm->lp_type = lp_type;
	}

	/* get operation function by stream state */
	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
		return ret;
	}

	if (ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD)
		fk_audio_tool_stream_set_cal(ac_strm);

	if (ops->startup) {
		/* init share memory head info */
		if (ac_strm->smem.vir_addr) {
			smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
			smem_head_ptr->w_off = 0;
			smem_head_ptr->r_off = 0;
			smem_head_ptr->flag = 0;
			smem_head_ptr->len = 0;
			AUD_LOG_DBG(AUD_CORE, "w_off(%d) r_off(%d) flag(%d) len(%d)",
				smem_head_ptr->w_off, smem_head_ptr->r_off,
				smem_head_ptr->flag, smem_head_ptr->len);
		}

		if (ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_UL) {
			enum voice_type voice_type = VOICE_TYPE_NORMAL;

			voice_type = fk_voice_type_query();
			fk_acore_set_pcie_bar_addr(ac_strm, voice_type);
		}

		atomic_set(&ac_strm->startup_wait, 1);
		ret = ops->startup(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "%s: startup failure", __func__);
			return ret;
		}

		/* update audio core stream state */
		ac_strm->state = ACORE_STATE_PREPARE;
		/* wait for dsp startup ack */
		ret = wait_event_timeout(ac_strm->cmd_wait_que,
			!atomic_read(&ac_strm->startup_wait), WAIT_TIMEOUT);
		if (!ret && atomic_read(&ac_strm->startup_wait)) {
			ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "audio ipc timeout startup timeout. state=%d wait=%d",
					ac_strm->state, atomic_read(&ac_strm->startup_wait));
				fk_audio_dump_ipc();
				fk_audio_adsp_uncache_log_dump();
				/* TODO: reset adsp */
#if IS_ENABLED(CONFIG_MIEV)
				mievent  = cdev_tevent_alloc(906001052);
				cdev_tevent_add_str(mievent, "Keyword", "adsp timeout");
				cdev_tevent_write(mievent);
				cdev_tevent_destroy(mievent);
#endif
		} else if (ac_strm->state == ACORE_STATE_IDLE) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "startup fail. state=%d wait=%d",
				ac_strm->state, atomic_read(&ac_strm->startup_wait));
#if IS_ENABLED(CONFIG_MIEV)
			mievent  = cdev_tevent_alloc(906001301);
			cdev_tevent_add_str(mievent, "Keyword", "adsp ack fail");
			cdev_tevent_write(mievent);
			cdev_tevent_destroy(mievent);
#endif
		} else {
			if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
				ret = fk_acore_set_frames_info(ac_strm);
				if (ret < 0) {
					AUD_LOG_ERR(AUD_CORE, "set frames info failure");
					return -EINVAL;
				}
			} else if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) {
				/* TODO: set gapless info if gapless mode is true */
				AUD_LOG_INFO(AUD_CORE, "gapless mode=%d",
					ac_strm->use_gapless);
				/* TODO: clear gaplss use flag */
			} else if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
				fk_acore_process_startup(ac_strm);
			}
			ret = EOK;
		}
	} else {
		AUD_LOG_INFO(AUD_CORE,
			"current state(%d) unsupport startup operation",
			(uint8_t)ac_strm->state);
	}

	if (ac_strm->adsp_sc_t != ADSP_SCENE_KWS_RECORD)
		fk_audio_tool_stream_set_cal(ac_strm);

#ifdef SIMULATE_COMMON
	//set vir addr to simulate module
	if (ac_strm->strm_dir == STRM_DIR_PLAYBACK)
		fk_simulate_set_vir_addr(ac_strm->smem.vir_addr, 0);
	else
		fk_simulate_set_vir_addr(ac_strm->smem.vir_addr, 1);
#endif

	AUD_LOG_DBG(AUD_CORE, "sess id(0x%x) exit.", ac_strm->sess_id);

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_shutdown
 * description: operation shutdown by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_shutdown(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (ac_strm->state == ACORE_STATE_IDLE) {
		ac_strm->fe_flag = 0;
		AUD_LOG_DBG(AUD_CORE, "session(0x%x) already shutdown.", ac_strm->sess_id);
		return ret;
	}

	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		AUD_LOG_INFO(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
		return ret;
	}

	if (ops->shutdown) {
		/* update fe flag */
		ac_strm->fe_flag = 0;

		/* update audio core stream state */
		atomic_set(&ac_strm->shutdown_wait, 1);

		ret = ops->shutdown(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "shutdown failure");
			/* resume fe flag */
			ac_strm->fe_flag = 1;

			return ret;
		}

		ac_strm->state = ACORE_STATE_IDLE;
		if (atomic_read(&aclient.ssr_flag)) {
			AUD_LOG_INFO(AUD_CORE, "adsp reset");
			return ret;
		}

		/* wait for dsp shutdown ack */
		ret = wait_event_timeout(ac_strm->cmd_wait_que,
			!atomic_read(&ac_strm->shutdown_wait), WAIT_TIMEOUT);
		if (!ret && atomic_read(&ac_strm->shutdown_wait)) {
			//ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "audio ipc timeout shutdown timeout. state=%d wait=%d",
				ac_strm->state, atomic_read(&ac_strm->shutdown_wait));
			fk_audio_dump_ipc();
			fk_audio_adsp_uncache_log_dump();
		} else {
			ret = EOK;
			AUD_LOG_DBG(AUD_CORE, "shutdown success");
		}
	} else {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "current state(%d) unsupport shutdown operation",
			(uint8_t)ac_strm->state);
	}

	AUD_LOG_DBG(AUD_CORE, "sess id(%d) completed.", ac_strm->sess_id);

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_start
 * description: operation start by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_start(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_INFO(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (atomic_read(&aclient.ssr_flag)) {
		AUD_LOG_INFO(AUD_CORE, "adsp reset");
		return -1;
	}

	if (((ac_strm->adsp_sc_t == ADSP_SCENE_NORMAL_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_VOIP_DL) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_MUSIC_INCALL_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_DIRECT_PLAY)) &&
		(atomic_read(&ac_strm->write_flag) == 0)) {
		/* trigger start after write to avoid data unready
		 * when dsp get data from share memory.
		 * just set start flag here.
		 */
		atomic_set(&ac_strm->start_flag, 1);
		AUD_LOG_DBG(AUD_CORE, "set session(0x%x) start flag.", ac_strm->sess_id);
	} else {
		ops = fk_stream_ops_get(ac_strm);
		if (!ops) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
			return ret;
		}

		if (ops->start) {
			atomic_set(&ac_strm->start_wait, 1);

			ret = ops->start(ac_strm);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_CORE, "start failure");
				return ret;
			}

			AUD_LOG_DBG(AUD_CORE, "wait for start ack. wait=%d",
				atomic_read(&ac_strm->start_wait));
			/* wait for dsp start ack */
				ret = wait_event_timeout(ac_strm->cmd_wait_que,
					!atomic_read(&ac_strm->start_wait), WAIT_TIMEOUT);
			if (!ret && atomic_read(&ac_strm->start_wait)) {
				ret = -EINVAL;
				AUD_LOG_ERR(AUD_CORE, "audio ipc timeout start timeout. state=%d wait=%d",
					ac_strm->state, atomic_read(&ac_strm->start_wait));
				fk_audio_dump_ipc();
				fk_audio_adsp_uncache_log_dump();
				/* update audio core stream state */
				ac_strm->state = ACORE_STATE_PREPARE;

				return ret;
			}

			ret = EOK;

			/* update audio core stream state */
			ac_strm->state = ACORE_STATE_RUN;

			if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
				AUD_LOG_DBG(AUD_CORE, "session(%d) process start", ac_strm->sess_id);
				/* trigger process after start success */
				fk_acore_process_start(ac_strm);
			}

			/*current share memory head info*/
			if (ac_strm->smem.vir_addr) {
				struct adsp_pcm_smem_head *smem_head_ptr = NULL;

				smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
				AUD_LOG_DBG(AUD_CORE,
					"w_off(%d) r_off(%d). put(%d) release(%d)",
					smem_head_ptr->w_off, smem_head_ptr->r_off,
					atomic_read(&ac_strm->put_block_no),
					atomic_read(&ac_strm->rel_block_no));
			}

			if (ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_UL)
				fk_acore_voice_speech_on_off(ac_strm, true);

		} else {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "current state(%d) unsupport start operation",
				(uint8_t)ac_strm->state);
		}
	}
	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_stop
 * description: operation stop by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_stop(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (ac_strm->state == ACORE_STATE_IDLE) {
		atomic_set(&ac_strm->start_flag, 0);
		AUD_LOG_INFO(AUD_CORE, "clear session(0x%x) start flag.",
			ac_strm->sess_id);
		return ret;
	}

	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "get ops failure by state(%d)",
			ac_strm->state);
		return ret;
	}

	if (ops->stop) {
		if (ac_strm->speech_status == true)
			fk_acore_voice_speech_on_off(ac_strm, false);

		if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD))
			/* trigger process thread after stop success */
			fk_acore_process_stop(ac_strm);

		atomic_set(&ac_strm->stop_wait, 1);

		ret = ops->stop(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "stop failure");
			return ret;
		}

		if (atomic_read(&aclient.ssr_flag)) {
			AUD_LOG_INFO(AUD_CORE, "adsp reset");
			return ret;
		}

		AUD_LOG_DBG_LIM(AUD_CORE, "wait for stop ack. wait=%d",
			atomic_read(&ac_strm->stop_wait));
		/* wait for dsp stop ack */
		ret = wait_event_timeout(ac_strm->cmd_wait_que,
				!atomic_read(&ac_strm->stop_wait), WAIT_TIMEOUT);
		if (!ret && atomic_read(&ac_strm->stop_wait)) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "audio ipc timeout stop timeout. state=%d wait=%d",
				ac_strm->state, atomic_read(&ac_strm->stop_wait));
			fk_audio_dump_ipc();
			fk_audio_adsp_uncache_log_dump();
			return ret;
		}

		ret = EOK;

		/* update audio core stream state */
		ac_strm->state = ACORE_STATE_PREPARE;

		if ((ac_strm->adsp_sc_t != ADSP_SCENE_ULTRA_FAST_PLAY) &&
			(ac_strm->adsp_sc_t != ADSP_SCENE_ULTRA_FAST_RECORD)) {
			/* reset share memory head */
			if (ac_strm->smem.vir_addr) {
				struct adsp_pcm_smem_head *smem_head_ptr = NULL;

				smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
				smem_head_ptr->w_off = 0;
				smem_head_ptr->r_off = 0;
				AUD_LOG_DBG(AUD_CORE, "w_off(%d) r_off(%d).", smem_head_ptr->w_off,
					smem_head_ptr->r_off);
			}
			/* reset frame count */
			atomic_set(&ac_strm->rel_block_no, 0);
			atomic_set(&ac_strm->put_block_no, 0);
		}
	} else {
		ret = EOK;
		AUD_LOG_INFO(AUD_CORE, "current state(%d)", (uint8_t)ac_strm->state);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_pause
 * description: operation pause by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_pause(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
		fk_acore_process_stop(ac_strm);

		/* update audio core stream state */
		ac_strm->state = ACORE_STATE_PAUSE;
		return ret;
	}

	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		ret = -EINVAL;
		AUD_LOG_INFO(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
		return ret;
	}

	if (ops->pause) {
		ret = ops->pause(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "pause failure");
			return ret;
		}

		/* update audio core stream state */
		ac_strm->state = ACORE_STATE_PAUSE;
	} else {
		//ret = -EINVAL;
		AUD_LOG_INFO(AUD_CORE, "current state(%d) unsupport pause operation",
			(uint8_t)ac_strm->state);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_resume
 * description: operation resume by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_resume(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
		fk_acore_process_start(ac_strm);
		/* update audio core stream state */
		ac_strm->state = ACORE_STATE_RUN;
		return ret;
	}

	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
		return ret;
	}

	if (ops->resume) {
		ret = ops->resume(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "resume failure");
			return ret;
		}

		/* update audio core stream state */
		ac_strm->state = ACORE_STATE_RUN;
	} else {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "current state(%d) unsupport resume operation",
			(uint8_t)ac_strm->state);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_flush
 * description: operation flush by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_flush(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
		AUD_LOG_INFO(AUD_CORE, "flush operation in scene(%d).",
			ac_strm->adsp_sc_t);
		return ret;
	}

	if (ac_strm->state == ACORE_STATE_RUN) {
		/* pause stream and will be resume after stream flush */
		AUD_LOG_DBG_LIM(AUD_CORE, "flush stream.");

		ret = fk_acore_stream_trigger_pause(ac_strm);
		if (ret < 0)
			AUD_LOG_INFO_LIM(AUD_CORE, "pause stream before flush fail.");
		else
			atomic_set(&ac_strm->resume_flag, 1);
	}

	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
		return ret;
	}

	if (ops->flush) {
		atomic_set(&ac_strm->flush_flag, 1);
		ret = ops->flush(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "flush failure");
			return ret;
		}
	} else {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "current state(%d) unsupport flush operation",
			(uint8_t)ac_strm->state);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_drain
 * description: operation drain by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_drain(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_PLAY) {
		AUD_LOG_INFO(AUD_CORE, "just support drain on compress playback scene");
		return ret;
	}

	// ops = fk_stream_ops_get(ac_strm);
	// if (!ops) {
	// ret = -EINVAL;
	// AC_LOG_ERR("get ops failure by state(%d)", ac_strm->state);
	// return ret;
	// }

	atomic_set(&ac_strm->last_frame, 0);
	smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
	smem_head_ptr->flag = 1;
	/* update last buffer info */
#ifdef LAST_FRAME_UNUSED
	if (ac_strm->cur_len) {
		smem_head_ptr->len = ac_strm->cur_len;
		atomic_inc(&ac_strm->put_block_no);
		/* update write offset info to share memory head */
		smem_head_ptr->w_off = atomic_read(&ac_strm->put_block_no);
	} else
		smem_head_ptr->len = ac_strm->smem.periods_size;
#endif
	AUD_LOG_INFO(AUD_CORE, "last buffer len(%d)", smem_head_ptr->len);

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_trigger_next_track
 * description: operation next track by stream instance
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_trigger_next_track(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct acore_stream_ops *ops = NULL;

	AUD_LOG_DBG(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	if (ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_PLAY) {
		AUD_LOG_INFO(AUD_CORE, "just support next on compress playback scene");
		return ret;
	}

	/* TODO: send new gapless info if gapless update */

	/* wait for dsp drain complete ack */
	ret = wait_event_timeout(ac_strm->cmd_wait_que,
		!atomic_read(&ac_strm->last_frame), WAIT_TIMEOUT);
	if (!ret) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "startup fail. state=%d", ac_strm->state);
		return ret;
	}

	ops = fk_stream_ops_get(ac_strm);
	if (!ops) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "get ops failure by state(%d)", ac_strm->state);
		return ret;
	}

	if (ops->next_track) {
		ret = ops->next_track(ac_strm);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "next track failure");
			return ret;
		}
	} else {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "current state(%d) unsupport next track operation",
			(uint8_t)ac_strm->state);
	}

	return ret;
}


/************************************************************************
 * function: fk_acore_stream_data_send
 * description: send playback data
 * parameter:
 *		ac_strm : stream instance
 *		p_buf : data buffer info
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_data_send(struct acore_stream *ac_strm, struct buf_blk p_buf)
{
	int ret = EOK;
	int bytes_per_blk = 0;
	//void *buf_seek = NULL;
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;
	int blk_num = p_buf.size / (ac_strm->smem.periods_size *
				fk_acore_get_bytes_per_sample(ac_strm));
	int cnt = 0;

	ac_strm->total_len += p_buf.size;

	if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY)
		bytes_per_blk = ac_strm->smem.periods_size;
	else
		bytes_per_blk = (ac_strm->smem.periods_size *
			fk_acore_get_bytes_per_sample(ac_strm));

	/* calculate block number with remaining data */
	if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) {
		if (p_buf.size < bytes_per_blk)
			bytes_per_blk = p_buf.size;
	}

	blk_num = (p_buf.size + ac_strm->cur_len) /
		bytes_per_blk;
	/* calculate remaining data length */
	ac_strm->cur_len = (p_buf.size + ac_strm->cur_len) %
		bytes_per_blk;

	while (atomic_read(&ac_strm->flush_flag)) {
		AUD_LOG_INFO_LIM(AUD_CORE, "wait for flush completed");
		msleep(20);
		if (cnt++ > 10)
			break;
	}

	switch (ac_strm->adsp_sc_t) {
	case ADSP_SCENE_NORMAL_PLAY:
	case ADSP_SCENE_FAST_PLAY:
	case ADSP_SCENE_VOIP_DL:
	case ADSP_SCENE_HAPTIC:
	case ADSP_SCENE_SPATIAL_PLAY:
	case ADSP_SCENE_KARAOKE_DL:
	case ADSP_SCENE_MUSIC_INCALL_PLAY:
	case ADSP_SCENE_DIRECT_PLAY:
	case ADSP_SCENE_OFFLOAD_PLAY:
		if (!blk_num) {
			AUD_LOG_INFO_LIM(AUD_CORE,
				"Incomplete data block. b_size(%d) p_size(%d) c_len(%d)",
				p_buf.size, ac_strm->smem.periods_size, ac_strm->cur_len);
			return ret;
		}

		atomic_add(blk_num, &ac_strm->put_block_no);
		smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
		/* update write offset info to share memory head */
		smem_head_ptr->w_off = atomic_read(&ac_strm->put_block_no);
		smem_head_ptr->len = bytes_per_blk;

		/* correction put frame number when over boundary.
		 *  note: During the write operation, if interrupted
		 *        by the stop operation. So the write operation
		 *        at this time is invalid and cannot be included
		 *        in the frame count.
		 */
		if (atomic_read(&ac_strm->put_block_no) >
			(atomic_read(&ac_strm->rel_block_no)
			+ ac_strm->smem.periods_num)) {
			AUD_LOG_INFO_LIM(AUD_CORE, "over-boundary: put(%d) release(%d)",
				atomic_read(&ac_strm->put_block_no),
				atomic_read(&ac_strm->rel_block_no));
		}

		AUD_LOG_DBG_LIM(AUD_CORE, "put(%d) release(%d)",
			atomic_read(&ac_strm->put_block_no),
				atomic_read(&ac_strm->rel_block_no));
		AUD_LOG_DBG_LIM(AUD_CORE, "len(%d)", smem_head_ptr->len);
#ifdef SIMULATE_DEBUG
		AUD_LOG_DBG_LIM(AUD_CORE, "r_off(%d) w_off(%d)",
			smem_head_ptr->r_off, smem_head_ptr->w_off);
#endif

		break;
	case ADSP_SCENE_ULTRA_FAST_PLAY:
		ret = fk_acore_playback_buf_put(ac_strm, p_buf);
		if (ret < 0) {
			AUD_LOG_INFO_LIM(AUD_CORE, "scene(%d) send failure.",
			ac_strm->adsp_sc_t);
		}
		break;
	default:
		break;
	}

	/* trigger resume when resume flag set */
	if ((ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) &&
		atomic_read(&ac_strm->resume_flag)) {
		ret = fk_acore_stream_trigger_resume(ac_strm);
		if (ret < 0)
			AUD_LOG_INFO_LIM(AUD_CORE, "resume stream after flush fail.");
		else
			atomic_set(&ac_strm->resume_flag, 0);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_stream_data_recv
 * description: receive capture data
 * parameter:
 *		ac_strm : stream instance
 *		p_buf : data buffer info
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_stream_data_recv(struct acore_stream *ac_strm, struct buf_blk p_buf)
{
	int ret = EOK;
	int blk_num = 0;

	struct adsp_pcm_smem_head *smem_head_ptr = NULL;

	/* print debug info */
	if ((atomic_read(&ac_strm->put_block_no) % FRAME_PRINT_FREQ) == 0) {
		AUD_LOG_DBG_LIM(AUD_CORE, "sess(0x%x): frame no(%d) total len(%d)",
			ac_strm->sess_id, atomic_read(&ac_strm->put_block_no),
			ac_strm->total_len);
	}

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_NORMAL_RECORD) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_VOIP_UL) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_FAST_RECORD) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_RECORD)) {
		blk_num = p_buf.size / (ac_strm->smem.periods_size *
				fk_acore_get_bytes_per_sample(ac_strm));
	} else if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
		blk_num = p_buf.size / ac_strm->smem.periods_size;
	}

	switch (ac_strm->adsp_sc_t) {
	case ADSP_SCENE_NORMAL_RECORD:
	case ADSP_SCENE_FAST_RECORD:
	case ADSP_SCENE_OFFLOAD_RECORD:
	case ADSP_SCENE_VOIP_UL:
	case ADSP_SCENE_VOICE_CALL_RECORD:
		smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);

		atomic_add(blk_num, &ac_strm->put_block_no);
		ac_strm->total_len += p_buf.size;

		/* update read offset info to share memory head */
		smem_head_ptr->r_off = atomic_read(&ac_strm->put_block_no);

#ifdef SIMULATE_DEBUG
		AUD_LOG_INFO_LIM(AUD_CORE, "r_off(%d) w_off(%d).", smem_head_ptr->r_off,
			smem_head_ptr->w_off);
#endif

		break;
	case ADSP_SCENE_ULTRA_FAST_RECORD:
		ret = fk_acore_capture_buf_put(ac_strm, p_buf);
		if (ret < 0)
			AUD_LOG_ERR_LIM(AUD_CORE, "scene(%d) send failure.", ac_strm->adsp_sc_t);
		break;
	default:
		break;
	}

	return ret;
}

/*************************************************************************
 * function: fk_acore_peri_slot_set
 * description: set slot info on audio interface.
 * parameter:
 *		ac_strm : stream instance
 *		uint8_t *peri : slot info of audio interface.
 * return:
 *		0: success
 *		other: failure
 *************************************************************************/
int fk_acore_peri_slot_set(struct acore_stream *ac_strm,
	void *peri)
{
	int ret = EOK;
	uint32_t channel_num = ac_strm->dai_fmt.channels;
	uint16_t *slot;

	/*
	 *    I2S: bit[0] left channel
	 *         bit[1] right channel
	 *    TDM: bit[0~7] channel 0~7
	 *    Note: for the audif, slots must be configured in pairs.
	 *        eg: {bit[1], bit[0]} {bit[3], bit[2]} {bit[5], bit[4]}
	 */
	slot = (uint16_t *)peri;
	switch (channel_num) {
	case 1:
	case 2:
		*slot = 0x3;
		break;
	case 3:
	case 4:
		*slot = 0xF;
		break;
	case 5:
	case 6:
		*slot = 0x3F;
		break;
	case 7:
	case 8:
		*slot = 0xFF;
		break;
	default:
		*slot = 0x3;
		AUD_LOG_INFO(AUD_CORE, "unsupport channels(%d)", channel_num);
		break;
	}
	AUD_LOG_DBG(AUD_CORE, "slot: 0x%p", slot);

	return ret;
}

/*************************************************************************
 * function: fk_acore_peri_ul_set
 * description: set uplink info of audio interface.
 * parameter:
 *		ac_strm : stream instance
 *		peri_sel_ul_path : slot info of audio interface on uplink path.
 * return:
 *		0: success
 *		other: failure
 *************************************************************************/
int fk_acore_peri_ul_set(struct acore_stream *ac_strm,
	struct peri_sel_ul_path *ul_peri_sel)
{
	int ret = EOK;
	int port_id;

	if (ac_strm->strm_dir == STRM_DIR_DUAL)
		port_id = ac_strm->in_port_id;
	else
		port_id = ac_strm->port_id;

	AUD_LOG_DBG_LIM(AUD_CORE, "in_port_id: %x", port_id);

	/* AUDIF:
	 *     l_i2s_idx:
	 *     r_i2s_idx: i2s index
	 *                eg: 0-i2s0 1-i2s1 2-i2s2 3-i2s3 4-i2s4
	 *     l_i2s_chn:
	 *     r_i2s_chn:
	 *               IIS: 0-left 1-right
	 *               TDM: bit[0~7] channel 0~7
	 *
	 */
	switch (port_id) {
	case FK_XR_I2S0_TX:
	case FK_XR_TDM0_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_AUDIFIIS0;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_AUDIFIIS0;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S1_TX:
	case FK_XR_TDM1_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_AUDIFIIS1;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_AUDIFIIS1;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S2_TX:
	case FK_XR_TDM2_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_AUDIFIIS2;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_AUDIFIIS2;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S3_TX:
	case FK_XR_TDM3_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_AUDIFIIS3;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_AUDIFIIS3;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S4_TX:
	case FK_XR_TDM4_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_AUDIFIIS4;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_AUDIFIIS4;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_VAD_I2S_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_VAD; //vad i2s
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_VAD; //vad i2s
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S5_TX:
	case FK_XR_TDM5_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_IIS5;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_IIS5;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S6_TX:
	case FK_XR_TDM6_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_IIS6;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_IIS6;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S7_TX:
	case FK_XR_TDM7_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_IIS7;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_IIS7;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S8_TX:
	case FK_XR_TDM8_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_IIS8;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_IIS8;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_I2S9_TX:
	case FK_XR_TDM9_TX:
		ul_peri_sel->l_i2s_idx = PORT_ID_IIS9;
		ul_peri_sel->l_i2s_chn = 0;
		ul_peri_sel->r_i2s_idx = PORT_ID_IIS9;
		ul_peri_sel->r_i2s_chn = 1;
		break;
	case FK_XR_TDM7_8_TX:
	case FK_XR_AUDIO_USB_TX:
		break;
	default:
		break;
	}
	AUD_LOG_DBG(AUD_CORE, "ul path info: %x %x", ul_peri_sel->l_i2s_idx,
			ul_peri_sel->l_i2s_chn);

	return ret;
}

/*************************************************************************
 * function: fk_acore_peri_dl_set
 * description: set downlink info of audio interface.
 * parameter:
 *		ac_strm : stream instance
 *		peri_sel_dl_path : slot info of audio interface on downlink path.
 * return:
 *		0: success
 *		other: failure
 *************************************************************************/
int fk_acore_peri_dl_set(struct acore_stream *ac_strm,
	struct peri_sel_dl_path *dl_peri_sel)
{
	int ret = EOK;
	int port_id;

	if (ac_strm->strm_dir == STRM_DIR_DUAL)
		port_id = ac_strm->out_port_id;
	else
		port_id = ac_strm->port_id;

	AUD_LOG_DBG_LIM(AUD_CORE, "out_port_id: 0x%x", port_id);

	switch (port_id) {
	case FK_XR_I2S0_RX:
	case FK_XR_TDM0_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s0_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s0_slot);
		break;
	case FK_XR_I2S1_RX:
	case FK_XR_TDM1_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s1_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s1_slot);
		break;
	case FK_XR_I2S2_RX:
	case FK_XR_TDM2_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s2_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s2_slot);
		break;
	case FK_XR_I2S3_RX:
	case FK_XR_TDM3_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s3_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s3_slot);
		break;
	case FK_XR_I2S4_RX:
	case FK_XR_TDM4_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s4_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s4_slot);
		break;
	case FK_XR_I2S5_RX:
	case FK_XR_TDM5_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s5_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s5_slot);
		break;
	case FK_XR_I2S6_RX:
	case FK_XR_TDM6_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s6_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s6_slot);
		break;
	case FK_XR_I2S7_RX:
	case FK_XR_TDM7_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s7_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s7_slot);
		break;
	case FK_XR_I2S8_RX:
	case FK_XR_TDM8_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s8_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s8_slot);
		break;
	case FK_XR_I2S9_RX:
	case FK_XR_TDM9_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s9_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s9_slot);
		break;
	case FK_XR_TDM7_8_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s7_slot);
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->i2s8_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s7_slot);
		AUD_LOG_DBG(AUD_CORE, "dl path info: %x", dl_peri_sel->i2s8_slot);
		break;
	case FK_XR_AUDIO_USB_RX:
		break;
	case FK_XR_MI2S0_RX:
		fk_acore_peri_slot_set(ac_strm, &dl_peri_sel->mi2s0_slot);
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "unsupport port id(%d)", port_id);
		break;
	}

	dl_peri_sel->ch_map_info0 = ac_strm->chan_map_info0;
	dl_peri_sel->ch_map_info1 = ac_strm->chan_map_info1;

	AUD_LOG_INFO(AUD_CORE, "info0(0x%x) info1(0x%x).",
		dl_peri_sel->ch_map_info0, dl_peri_sel->ch_map_info1);
	return ret;
}

#ifdef STARTUP_INFO_VER_02
/************************************************************************
 * function: fk_acore_set_bt_fmt
 * description: set bt codec info.
 * parameter:
 *		port_info : audio interface port info
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_acore_set_bt_fmt(struct adsp_dev_port_info *port_info)
{
	int ret = EOK;
	struct bt_codec_config bt_config = {0};

	fk_bt_config_query(&bt_config);

	memcpy(&port_info->bt_config, &bt_config, sizeof(struct bt_codec_config));

	return ret;
}

/************************************************************************
 * function: fk_acore_audif_port_fmt
 * description: set audio interface port DAI format.
 * parameter:
 *		port_info : audio interface port info
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_acore_audif_port_fmt(struct adsp_dev_port_info *port_info)
{
	int ret = EOK;

	/* Note: Currently using a fixed data format,
	 * adjustments will be done as needed.
	 */
	port_info->dai_info.codec_type = fk_codec_type_query();
	port_info->dai_info.sample_rate = 48000;
	port_info->dai_info.channels = fk_audif_fmtchs_query();
	port_info->dai_info.bit_width = fk_audif_fmtbit_query();

	return ret;
}

/************************************************************************
 * function: fk_acore_tdm_port_fmt
 * description: set audio interface port DAI format.
 * parameter:
 *		port_info : audio interface port info
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_acore_tdm_port_fmt(struct adsp_dev_port_info *port_info)
{
	int ret = EOK;

	/* Note: Currently using a fixed data format,
	 * adjustments will be done as needed.
	 */
	port_info->dai_info.codec_type = fk_codec_type_query();
	port_info->dai_info.sample_rate = 48000;
	port_info->dai_info.channels = fk_audio_tdm_fmtchs_query();
	port_info->dai_info.bit_width = fk_audif_fmtbit_query();

	return ret;
}

/************************************************************************
 * function: fk_acore_port_id_config
 * description: set audio interface id info.
 * parameter:
 *		port_info : audio interface port info
 *		port_id : audio interface port id
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_acore_port_id_config(struct adsp_dev_port_info *port_info,
	int port_id)
{
	int ret = EOK;

	switch (port_id) {
	case FK_XR_I2S0_TX:
	case FK_XR_I2S0_RX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS0;
		fk_acore_audif_port_fmt(port_info);
		break;
	case FK_XR_I2S1_TX:
	case FK_XR_I2S1_RX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS1;
		fk_acore_audif_port_fmt(port_info);
		fk_acore_set_bt_fmt(port_info);
		AUD_LOG_INFO(AUD_CORE, "pid %d, type %d, rate %d, chs %d, bits %d",
			port_info->port_id, port_info->bt_config.port_info.codec_type,
			port_info->bt_config.port_info.sample_rate,
			port_info->bt_config.port_info.channels,
			port_info->bt_config.port_info.bit_width);
		break;
	case FK_XR_I2S2_TX:
	case FK_XR_I2S2_RX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS2;
		fk_acore_audif_port_fmt(port_info);
		break;
	case FK_XR_I2S3_TX:
	case FK_XR_I2S3_RX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS3;
		fk_acore_audif_port_fmt(port_info);
		break;
	case FK_XR_I2S4_TX:
	case FK_XR_I2S4_RX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS4;
		fk_acore_audif_port_fmt(port_info);
		break;
	case FK_XR_TDM0_RX:
	case FK_XR_TDM0_TX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS0;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM1_RX:
	case FK_XR_TDM1_TX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS1;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM2_RX:
	case FK_XR_TDM2_TX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS2;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM3_RX:
	case FK_XR_TDM3_TX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS3;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM4_RX:
	case FK_XR_TDM4_TX:
		port_info->port_id |= 1 << PORT_ID_AUDIFIIS4;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM5_RX:
	case FK_XR_TDM5_TX:
		port_info->port_id |= 1 << PORT_ID_IIS5;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM6_RX:
	case FK_XR_TDM6_TX:
		port_info->port_id |= 1 << PORT_ID_IIS6;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM7_RX:
	case FK_XR_TDM7_TX:
		port_info->port_id |= 1 << PORT_ID_IIS7;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM8_RX:
	case FK_XR_TDM8_TX:
		port_info->port_id |= 1 << PORT_ID_IIS8;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_TDM9_RX:
	case FK_XR_TDM9_TX:
		port_info->port_id |= 1 << PORT_ID_IIS9;
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_AUDIO_USB_TX:
	case FK_XR_AUDIO_USB_RX:
		port_info->port_id |= 1 << PORT_ID_USB;
		break;
	case FK_XR_TDM7_8_RX:
	case FK_XR_TDM7_8_TX:
		port_info->port_id |= (1 << PORT_ID_IIS7) | (1 << PORT_ID_IIS8);
		fk_acore_tdm_port_fmt(port_info);
		break;
	case FK_XR_MI2S0_RX:
		port_info->port_id |= 1 << PORT_ID_MIIS0;
		fk_acore_audif_port_fmt(port_info);
		break;
	case FK_XR_VAD_I2S_TX:
		port_info->port_id |= 1 << PORT_ID_VAD;
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "unsupport port(%d)", port_id);
		break;
	}
	return ret;
}

/************************************************************************
 * function: fk_acore_port_id_get
 * description: set audio interface id info.
 * parameter:
 *		port_id : audio interface port id
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
uint32_t fk_acore_port_id_get(int port_id)
{
	uint32_t adsp_port_id = 0;

	switch (port_id) {
	case FK_XR_I2S0_TX:
	case FK_XR_I2S0_RX:
	case FK_XR_TDM0_RX:
	case FK_XR_TDM0_TX:
		adsp_port_id |= 1 << PORT_ID_AUDIFIIS0;
		break;
	case FK_XR_I2S1_TX:
	case FK_XR_I2S1_RX:
	case FK_XR_TDM1_RX:
	case FK_XR_TDM1_TX:
		adsp_port_id |= 1 << PORT_ID_AUDIFIIS1;
		break;
	case FK_XR_I2S2_TX:
	case FK_XR_I2S2_RX:
	case FK_XR_TDM2_RX:
	case FK_XR_TDM2_TX:
		adsp_port_id |= 1 << PORT_ID_AUDIFIIS2;
		break;
	case FK_XR_I2S3_TX:
	case FK_XR_I2S3_RX:
	case FK_XR_TDM3_RX:
	case FK_XR_TDM3_TX:
		adsp_port_id |= 1 << PORT_ID_AUDIFIIS3;
		break;
	case FK_XR_I2S4_TX:
	case FK_XR_I2S4_RX:
	case FK_XR_TDM4_RX:
	case FK_XR_TDM4_TX:
		adsp_port_id |= 1 << PORT_ID_AUDIFIIS4;
		break;
	case FK_XR_TDM5_RX:
	case FK_XR_TDM5_TX:
		adsp_port_id |= 1 << PORT_ID_IIS5;
		break;
	case FK_XR_TDM6_RX:
	case FK_XR_TDM6_TX:
		adsp_port_id |= 1 << PORT_ID_IIS6;
		break;
	case FK_XR_TDM7_RX:
	case FK_XR_TDM7_TX:
		adsp_port_id |= 1 << PORT_ID_IIS7;
		break;
	case FK_XR_TDM8_RX:
	case FK_XR_TDM8_TX:
		adsp_port_id |= 1 << PORT_ID_IIS8;
		break;
	case FK_XR_TDM9_RX:
	case FK_XR_TDM9_TX:
		adsp_port_id |= 1 << PORT_ID_IIS9;
		break;
	case FK_XR_AUDIO_USB_TX:
	case FK_XR_AUDIO_USB_RX:
		adsp_port_id |= 1 << PORT_ID_USB;
		break;
	case FK_XR_MI2S0_RX:
		adsp_port_id |= 1 << PORT_ID_MIIS0;
		break;
	case FK_XR_VAD_I2S_TX:
		adsp_port_id |= 1 << PORT_ID_VAD;
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "unsupport port(%d)", port_id);
		break;
	}
	return adsp_port_id;
}

/************************************************************************
 * function: fk_acore_port_set
 * description: set audio interface id info.
 * parameter:
 *		ac_strm : stream instance
 *		info : info ptr
 *		en : 1 - open port
 *			   0 - close port
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_port_set(struct acore_stream *ac_strm,
	void *info, bool en)
{
	int ret = EOK;
	int portId;

	/* config port id */
	if (ac_strm->strm_dir == STRM_DIR_DUAL) {
		struct adsp_scene_lp_startup_info *portInfo = info;
		/* used this branch currently */
		portId = ac_strm->in_port_id;
		portInfo->ul_port_id = fk_acore_port_id_get(portId);
		/* update port format info */
		portInfo->ul_dai_info.sample_rate = ac_strm->in_dai_fmt.samples;
		portInfo->ul_dai_info.bit_width = ac_strm->in_dai_fmt.bit_width;
		portInfo->ul_dai_info.channels = ac_strm->in_dai_fmt.channels;
		AUD_LOG_DBG(AUD_CORE, "ul_dai_info:port(%u) rate(%u) bits(%u) channels(%u)",
			portInfo->ul_port_id,
			portInfo->ul_dai_info.sample_rate,
			portInfo->ul_dai_info.bit_width,
			portInfo->ul_dai_info.channels);

		portId = ac_strm->out_port_id;
		portInfo->dl_port_id = fk_acore_port_id_get(portId);
		/* update port format info */
		portInfo->dl_dai_info.sample_rate = ac_strm->out_dai_fmt.samples;
		portInfo->dl_dai_info.bit_width = ac_strm->out_dai_fmt.bit_width;
		portInfo->dl_dai_info.channels = ac_strm->out_dai_fmt.channels;
		AUD_LOG_DBG(AUD_CORE, "dl_dai_info:port(%u) rate(%u) bits(%u) channels(%u)",
			portInfo->dl_port_id,
			portInfo->dl_dai_info.sample_rate,
			portInfo->dl_dai_info.bit_width,
			portInfo->dl_dai_info.channels);
	} else {
		struct adsp_dev_port_info *portInfo = info;

		portId = ac_strm->port_id;
		ret = fk_acore_port_id_config(portInfo, portId);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "config port(%d) fail", portId);
			goto done;
		}

		/* update port format info */
		portInfo->dai_info.sample_rate = ac_strm->dai_fmt.samples;
		portInfo->dai_info.bit_width = ac_strm->dai_fmt.bit_width;
		portInfo->dai_info.channels = ac_strm->dai_fmt.channels;

		AUD_LOG_INFO(AUD_CORE, "port(%d): rate(%u) bits(%u) channels(%u) codec_type(%u)",
			ac_strm->port_id,
			portInfo->dai_info.sample_rate,
			portInfo->dai_info.bit_width,
			portInfo->dai_info.channels,
			portInfo->dai_info.codec_type);

		/* config port enable status */
		if (en) {
			portInfo->enable_flag = 1;

			if ((ac_strm->port_id == FK_XR_AUDIO_USB_RX) ||
				(ac_strm->port_id == FK_XR_AUDIO_USB_TX)) {
				struct aud_usb_offload_info info;
				int try_cnt = 10;

				while (1) {
					ret = audio_usb_offload_param_get((struct aud_usb_offload_info *)&info,
						ac_strm->strm_dir, ac_strm->dai_fmt);
					if ((ret < 0) && (try_cnt--)) {
						AUD_LOG_INFO(AUD_CORE, "retry usb get. try(%d)", try_cnt);
						mdelay(200);
						continue;
					} else if ((ret < 0) && (try_cnt == 0)) {
						AUD_LOG_ERR(AUD_CORE, "usb parameter get fail");
						goto done;
					}

					break;
				}

				/* config usb interface info */
				portInfo->usb_info.intf_num = info.intf_num;
				portInfo->usb_info.alt_num = info.alt_setting;
				portInfo->usb_info.idle_intf_num = info.idle_intf_num;
				portInfo->usb_info.idle_alt_num = info.idle_alt_setting;
				portInfo->usb_info.pipe = info.pipe;
				portInfo->usb_info.datainterval = info.datainterval;
				portInfo->usb_info.clock_id = info.clock_id;
				portInfo->usb_info.protocol = info.protocol;
				portInfo->usb_info.ctrl_intf = info.ctrl_intf;
				portInfo->usb_info.sync_pipe = info.sync_pipe;
				portInfo->usb_info.sync_interval = info.sync_interval;
				portInfo->usb_info.attributes = info.attributes;
				portInfo->usb_info.dev_type = info.dev_type;
				/* config usb dai info */
				portInfo->usb_info.format = ac_strm->dai_fmt.pcm_fmt;
				portInfo->usb_info.bits = ac_strm->dai_fmt.bit_width;
				AUD_LOG_INFO(AUD_CORE, "usb dai info. format(%d) bits(%d) type(%d)",
					portInfo->usb_info.format, portInfo->usb_info.bits,
					portInfo->usb_info.dev_type);
			}
		} else
			portInfo->enable_flag = 0;

		AUD_LOG_DBG(AUD_CORE, "port(%d) enable flag(%d)", portId,
			portInfo->enable_flag);
	}

done:
	return ret;
}

/************************************************************************
 * function: fk_acore_peri_sel_set
 * description: set audio interface slot info.
 * parameter:
 *		ac_strm : stream instance
 *		void *info : info ptr
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_peri_sel_set(struct acore_stream *ac_strm,
	void *info)
{
	int ret = EOK;
	struct adsp_dev_port_info *portInfo = NULL;
	struct adsp_scene_lp_startup_info *lpStartUpInfo = NULL;
	struct peri_sel_ul_path *ulPeriSel = NULL;
	struct peri_sel_dl_path *dlPeriSel = NULL;

	switch (ac_strm->adsp_sc_t) {
	case ADSP_SCENE_ULTRA_FAST_RECORD:
	case ADSP_SCENE_NORMAL_RECORD:
	case ADSP_SCENE_OFFLOAD_RECORD:
	case ADSP_SCENE_VOIP_UL:
	case ADSP_SCENE_FAST_RECORD:
	case ADSP_SCENE_VOICE_CALL_UL:
	case ADSP_SCENE_KWS_RECORD:
	case ADSP_SCENE_MMAP_RECORD:
	case ADSP_SCENE_KARAOKE_UL:
	case ADSP_SCENE_VOICE_CALL_RECORD:
	case ADSP_SCENE_HF_UL:
		portInfo = (struct adsp_dev_port_info *)info;
		ulPeriSel = (struct peri_sel_ul_path *)(&portInfo->ul_peri_sel);
		fk_acore_peri_ul_set(ac_strm, ulPeriSel);
		break;
	case ADSP_SCENE_NORMAL_PLAY:
	case ADSP_SCENE_FAST_PLAY:
	case ADSP_SCENE_ULTRA_FAST_PLAY:
	case ADSP_SCENE_OFFLOAD_PLAY:
	case ADSP_SCENE_VOIP_DL:
	case ADSP_SCENE_VOICE_CALL_DL:
	case ADSP_SCENE_MMAP_PLAY:
	case ADSP_SCENE_SPATIAL_PLAY:
	case ADSP_SCENE_KARAOKE_DL:
	case ADSP_SCENE_MUSIC_INCALL_PLAY:
	case ADSP_SCENE_DIRECT_PLAY:
	case ADSP_SCENE_HF_DL:
		portInfo = (struct adsp_dev_port_info *)info;
		dlPeriSel = (struct peri_sel_dl_path *)(&portInfo->dl_peri_sel);
		fk_acore_peri_dl_set(ac_strm, dlPeriSel);
		break;
	case ADSP_SCENE_HAPTIC:
		portInfo = (struct adsp_dev_port_info *)info;
		dlPeriSel = (struct peri_sel_dl_path *)(&portInfo->dl_peri_sel);
		/* Note: slot 0/1 use for phone, slot 0/1/2/3 use for pad */
		if (ac_strm->port_id == FK_XR_I2S2_RX)
			dlPeriSel->i2s2_slot = 0xC;
		else if (ac_strm->port_id == FK_XR_I2S7_RX)
			dlPeriSel->i2s7_slot = 0x30;
		else
			AUD_LOG_INFO(AUD_CORE, "Haptic port(%d)", ac_strm->port_id);
		break;
	case ADSP_SCENE_LOOPBACK:
		lpStartUpInfo = (struct adsp_scene_lp_startup_info *)info;
		/* config uplink path info */
		ulPeriSel = (struct peri_sel_ul_path *)(&lpStartUpInfo->ul_peri_sel);
		fk_acore_peri_ul_set(ac_strm, ulPeriSel);
		/* config downlink path info */
		dlPeriSel = (struct peri_sel_dl_path *)(&lpStartUpInfo->dl_peri_sel);
		fk_acore_peri_dl_set(ac_strm, dlPeriSel);
		break;
	default:
		AUD_LOG_ERR(AUD_CORE, "unsupport scene(%d)", ac_strm->adsp_sc_t);
		ret = -EINVAL;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_add_path_config
 * description: Configure additional path
 * parameter:
 *		ac_strm : stream instance
 *		en :
 *			true : enable path
 *			false : disable path
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_add_path_config(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct xring_aud_msg pathUpdateMsg = {0};
	struct adsp_dev_port_info *portInfo;
	struct dai_format *dai_fmt = NULL;
	int port_idx = 0;

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_LOOPBACK) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
		AUD_LOG_INFO_LIM(AUD_CORE, "scene(%d)", ac_strm->adsp_sc_t);
		return ret;
	}

	for (port_idx = 1; port_idx < ac_strm->port_num; port_idx++) {
		/* update pord id */
		ac_strm->port_id = ac_strm->port_info[port_idx];

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

		portInfo = (struct adsp_dev_port_info *)&pathUpdateMsg.data[0];
		ret = fk_acore_port_set(ac_strm, (void *)portInfo, true);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_CORE, "set port failure.");
			return ret;
		}

		ret = fk_acore_peri_sel_set(ac_strm, (void *)portInfo);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "port slot set failure.");
			return ret;
		}

		/* fill path update message */
		pathUpdateMsg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
		pathUpdateMsg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
		pathUpdateMsg.header.cmd_id = ADSP_HW_MODULE_CTRL;
		pathUpdateMsg.header.func_id = ADSP_SET_DEV_PATH;
		pathUpdateMsg.header.result = 0;
		pathUpdateMsg.header.len = sizeof(struct xring_aud_msg_header) +
			sizeof(struct adsp_dev_port_info);

		AUD_LOG_DBG_LIM(AUD_CORE, "message info 0: %x %x %x %x",
			(uint32_t)pathUpdateMsg.data[0],
			(uint32_t)pathUpdateMsg.data[4],
			(uint32_t)pathUpdateMsg.data[8],
			(uint32_t)pathUpdateMsg.data[12]);
		AUD_LOG_DBG_LIM(AUD_CORE, "message info 1: %x %x %x %x",
			(uint32_t)pathUpdateMsg.data[16],
			(uint32_t)pathUpdateMsg.data[20],
			(uint32_t)pathUpdateMsg.data[24],
			(uint32_t)pathUpdateMsg.data[28]);

		/* send starup message */
		ret = fk_acore_message_send((struct xring_aud_msg *)&pathUpdateMsg);
		if (ret < 0) {
			ret = -EINVAL;
			AUD_LOG_ERR(AUD_CORE, "send path update message failure.");
			return ret;
		}
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_path_update
 * description: set path status
 * parameter:
 *		ac_strm : stream instance
 *		en :
 *			true : enable path
 *			false : disable path
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_path_update(struct acore_stream *ac_strm, bool en)
{
	int ret = EOK;
	struct xring_aud_msg pathUpdateMsg = {0};
	struct adsp_dev_port_info *portInfo;

	AUD_LOG_INFO(AUD_CORE, "port(%d) status(%d) update.", ac_strm->port_id, en);

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_LOOPBACK) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_PLAY) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_ULTRA_FAST_RECORD)) {
		AUD_LOG_INFO_LIM(AUD_CORE, "scene(%d)", ac_strm->adsp_sc_t);
		return ret;
	}

	/*for kws switch dev*/
	if (ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD) {
	/*vote adsp poweron*/
		ret = fk_adsp_vote_register(ac_strm->sess_id);
		if (ret < 0)
			AUD_LOG_ERR(AUD_CORE, "vt vote adsp power on failed.");
	}

	portInfo = (struct adsp_dev_port_info *)&pathUpdateMsg.data[0];
	ret = fk_acore_port_set(ac_strm, (void *)portInfo, en);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_CORE, "set port failure.");
		return ret;
	}

	ret = fk_acore_peri_sel_set(ac_strm, (void *)portInfo);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "port slot set failure.");
		return ret;
	}

	/* fill path update message */
	pathUpdateMsg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	pathUpdateMsg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	pathUpdateMsg.header.cmd_id = ADSP_HW_MODULE_CTRL;
	pathUpdateMsg.header.func_id = ADSP_SET_DEV_PATH;
	pathUpdateMsg.header.result = 0;
	pathUpdateMsg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct adsp_dev_port_info);

	AUD_LOG_DBG_LIM(AUD_CORE, "message info 0: %x %x %x %x",
		(uint32_t)pathUpdateMsg.data[0],
		(uint32_t)pathUpdateMsg.data[4],
		(uint32_t)pathUpdateMsg.data[8],
		(uint32_t)pathUpdateMsg.data[12]);
	AUD_LOG_DBG_LIM(AUD_CORE, "message info 1: %x %x %x %x",
		(uint32_t)pathUpdateMsg.data[16],
		(uint32_t)pathUpdateMsg.data[20],
		(uint32_t)pathUpdateMsg.data[24],
		(uint32_t)pathUpdateMsg.data[28]);

	/* send starup message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&pathUpdateMsg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send path update message failure.");
		return ret;
	}

	return ret;
}

#else

/************************************************************************
 * function: fk_acore_peri_sel_set
 * description: set audio interface id and slot info.
 * parameter:
 *		ac_strm : stream instance
 *		void *startup_info : startup info ptr
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_peri_sel_set(struct acore_stream *ac_strm,
	void *startup_info)
{
	int ret = EOK;
	struct adsp_scene_startup_info *startUpInfo;
	struct adsp_scene_lp_startup_info *lpStartUpInfo;
	struct peri_sel_ul_path *ulPeriSel;
	struct peri_sel_dl_path *dlPeriSel;

	switch (ac_strm->adsp_sc_t) {
	case ADSP_SCENE_ULTRA_FAST_RECORD:
	case ADSP_SCENE_NORMAL_RECORD:
	case ADSP_SCENE_FAST_RECORD:
	case ADSP_SCENE_OFFLOAD_RECORD:
	case ADSP_SCENE_VOIP_UL:
	case ADSP_SCENE_VOICE_CALL_UL:
	case ADSP_SCENE_MMAP_RECORD:
	case ADSP_SCENE_KARAOKE_UL:
	case ADSP_SCENE_VOICE_CALL_RECORD:
	case ADSP_SCENE_HF_UL:
		startUpInfo = (struct adsp_scene_startup_info *)startup_info;
		ulPeriSel = (struct peri_sel_ul_path *)(&startUpInfo->peri_sel.ulPeriSel);
		fk_acore_peri_ul_set(ac_strm, ulPeriSel);
		break;
	case ADSP_SCENE_NORMAL_PLAY:
	case ADSP_SCENE_FAST_PLAY:
	case ADSP_SCENE_ULTRA_FAST_PLAY:
	case ADSP_SCENE_OFFLOAD_PLAY:
	case ADSP_SCENE_VOIP_DL:
	case ADSP_SCENE_VOICE_CALL_DL:
	case ADSP_SCENE_MMAP_PLAY:
	case ADSP_SCENE_SPATIAL_PLAY:
	case ADSP_SCENE_KARAOKE_DL:
	case ADSP_SCENE_MUSIC_INCALL_PLAY:
	case ADSP_SCENE_DIRECT_PLAY:
	case ADSP_SCENE_HF_DL:
		startUpInfo = (struct adsp_scene_startup_info *)startup_info;
		dlPeriSel = (struct peri_sel_dl_path *)(&startUpInfo->peri_sel.dlPeriSel);
		fk_acore_peri_dl_set(ac_strm, dlPeriSel);
		break;
	case ADSP_SCENE_LOOPBACK:
		lpStartUpInfo = (struct adsp_scene_lp_startup_info *)startup_info;
		/* config uplink path info */
		ulPeriSel = (struct peri_sel_ul_path *)(&lpStartUpInfo->ulPeriSel);
		fk_acore_peri_ul_set(ac_strm, ulPeriSel);
		/* config downlink path info */
		dlPeriSel = (struct peri_sel_dl_path *)(&lpStartUpInfo->dlPeriSel);
		fk_acore_peri_dl_set(ac_strm, dlPeriSel);
		break;
	default:
		AUD_LOG_ERR(AUD_CORE, "unsupport scene(%d)", ac_strm->adsp_sc_t);
		ret = -EINVAL;
	}

	return ret;
}

#endif

/************************************************************************
 * function: fk_acore_audioIf_iobuf_reg_set
 * description: set io buffer parameters value of audio if.
 * parameter:
 *		ac_strm : stream instance
 *		io_buf : io buffer parameters
 *		audif_reg : io port parameters
 * return:
 *		0: success
 *		other: failure
 * note: io_buf and reg wit be set by dsp policy. TODO.
 ************************************************************************/
int fk_acore_audioIf_iobuf_reg_set(struct acore_stream *ac_strm, struct adsp_io_buffer *io_buf,
	struct audif_reg_devpath *audif_reg)
{
	int ret = EOK;

	/* init audif io&reg info */
	memset(io_buf, 0, sizeof(struct adsp_io_buffer));
	memset(audif_reg, 0, sizeof(struct audif_reg_devpath));

	switch (ac_strm->adsp_sc_t) {
	case ADSP_SCENE_ULTRA_FAST_RECORD:
	case ADSP_SCENE_ULTRA_FAST_PLAY:
		AUD_LOG_DBG_LIM(AUD_CORE, "ultra use ap or ar io");
		break;
	case ADSP_SCENE_NORMAL_RECORD:
	case ADSP_SCENE_VOIP_UL:
	case ADSP_SCENE_FAST_RECORD:
	case ADSP_SCENE_MMAP_RECORD:
	case ADSP_SCENE_KARAOKE_UL:
	case ADSP_SCENE_VOICE_CALL_RECORD:
	case ADSP_SCENE_HF_UL:
		/* set ad fifo
		 * Note: use default now, it will be select by policy define
		 */
		io_buf->ulPathID0 = 1;
		fk_acore_audioIf_reg_set(ac_strm, ADSP_IO_BUF_AD0, audif_reg);
		break;
	case ADSP_SCENE_NORMAL_PLAY:
	case ADSP_SCENE_FAST_PLAY:
	case ADSP_SCENE_VOIP_DL:
	case ADSP_SCENE_MMAP_PLAY:
	case ADSP_SCENE_SPATIAL_PLAY:
	case ADSP_SCENE_KARAOKE_DL:
	case ADSP_SCENE_MUSIC_INCALL_PLAY:
	case ADSP_SCENE_HF_DL:
		io_buf->dlPathID0 = 1;
		fk_acore_audioIf_reg_set(ac_strm, ADSP_IO_BUF_DA0, audif_reg);
		break;
	case ADSP_SCENE_LOOPBACK:
		io_buf->ulPathID0 = 1;
		fk_acore_audioIf_reg_set(ac_strm, ADSP_IO_BUF_AD0, audif_reg);
		io_buf->dlPathID0 = 1;
		fk_acore_audioIf_reg_set(ac_strm, ADSP_IO_BUF_DA0, audif_reg);
		break;
	default:
		AUD_LOG_ERR(AUD_CORE, "unsupport scene(%d)", ac_strm->adsp_sc_t);
		ret = -EINVAL;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_audioIf_reg_set
 * description: set io port parameters value of audio if.
 * parameter:
 *		ac_strm : stream instance
 *		io_buf_t : io buffer index
 *		audif_reg : io port parameters
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_audioIf_reg_set(struct acore_stream *ac_strm, enum adsp_io_buffer_type io_buf_t,
	struct audif_reg_devpath *audif_reg)
{
	int ret = EOK;

	if (ac_strm->strm_dir == STRM_DIR_DUAL) {
		switch (io_buf_t) {
		case ADSP_IO_BUF_AD0:
			switch (ac_strm->in_port_id) {
			case FK_XR_I2S0_TX:
				audif_reg->ad0_audif_iis_sel |= (1 << I2S0_BIT) & 0xFF;
				break;
			default:
				AUD_LOG_INFO(AUD_CORE, "unsupport in port id(%d)",
					ac_strm->in_port_id);
				break;
			}
			break;
		case ADSP_IO_BUF_DA0:
			switch (ac_strm->out_port_id) {
			case FK_XR_I2S0_RX:
				audif_reg->da0_audif_iis_sel |= (1 << I2S0_BIT) & 0xFF;
				break;
			default:
				AUD_LOG_INFO(AUD_CORE, "unsupport out port id(%d)",
					ac_strm->out_port_id);
				break;
			}
			break;
		default:
			AUD_LOG_INFO(AUD_CORE, "unsupport io buf(%d)", io_buf_t);
			break;
		}
	} else {
		switch (ac_strm->port_id) {
		case FK_XR_I2S0_RX:
		case FK_XR_I2S0_TX:
			switch (io_buf_t) {
			case ADSP_IO_BUF_DA0:
				audif_reg->da0_audif_iis_sel |= (1 << I2S0_BIT) & 0xFF;
				break;
			case ADSP_IO_BUF_AD0:
				audif_reg->ad0_audif_iis_sel |= (1 << I2S0_BIT) & 0xFF;
				break;
			default:
				AUD_LOG_INFO(AUD_CORE, "unsupport io buf(%d)", io_buf_t);
				break;
			}
			break;
		default:
			AUD_LOG_INFO(AUD_CORE, "unsupport port id(%d)", ac_strm->port_id);
			break;
		}
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_set_frames_info
 * description: get timestamp from adsp
 * parameter:
 *		ac_strm : stream instance
 *		timestamp : Relative timestamp information from startup.
 *					in milliseconds
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_get_timestamp_info(struct acore_stream *ac_strm,
	uint32_t *timestamp)
{
	int ret = 0;
	struct xring_aud_msg frames_info_msg = {0};

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_RECORD) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_PLAY)) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
		*timestamp = ac_strm->cur_timestamp;
		AUD_LOG_INFO_LIM(AUD_CORE, "timestamp is %d", ac_strm->cur_timestamp);
		return 0;
	}

	/* fill message */
	frames_info_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	frames_info_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	frames_info_msg.header.cmd_id = ADSP_AUD_CTRL;
	frames_info_msg.header.func_id = ADSP_AUD_CTRL_GET_TIMESTAMP;
	frames_info_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(uint32_t);
	frames_info_msg.header.result = 0;

	atomic_set(&ac_strm->timestamp_wait, 1);

	/* send pause message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&frames_info_msg);
	if (ret < 0) {
		ret = -EINVAL;
		*timestamp = 0;
		AUD_LOG_ERR(AUD_CORE, "send timestamp info get message failure.");
		return ret;
	}

	/* wait for get timestamp ack */
	ret = wait_event_timeout(ac_strm->cmd_wait_que,
		!atomic_read(&ac_strm->timestamp_wait), WAIT_TIMEOUT);
	if (!ret) {
		ret = -EINVAL;
		*timestamp = 0;
		AUD_LOG_INFO(AUD_CORE, "get timestamp fail. state=%d", ac_strm->state);
	} else {
		*timestamp = ac_strm->cur_timestamp;
		AUD_LOG_INFO_LIM(AUD_CORE, "timestamp is %d", ac_strm->cur_timestamp);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_set_frames_info
 * description: set frames of compress stream one buffer block to adsp
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_set_frames_info(struct acore_stream *ac_strm)
{
	int ret = 0;
	struct xring_aud_msg frames_info_msg = {0};
	uint32_t *frames_per_blk = NULL;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if (ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_RECORD) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	/* fill message */
	frames_info_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	frames_info_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	frames_info_msg.header.cmd_id = ADSP_AUD_CTRL;
	frames_info_msg.header.func_id = ADSP_AUD_CTRL_SET_FRAME_PER_BLK;
	frames_info_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(uint32_t);
	frames_info_msg.header.result = 0;

	/* update frames info */
	frames_per_blk = (uint32_t *)&frames_info_msg.data[0];
	*frames_per_blk = (uint32_t)(aclient.frames_per_blk & 0xFFFF);

	AUD_LOG_DBG(AUD_CORE, "frames is %d", *frames_per_blk);

	/* send pause message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&frames_info_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send frames info message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_get_frames_info
 * description: get frames of compress stream one buffer block
 * parameter:
 *		void
 * return:
 *		>=0: frames number
 *		<0: failure
 ************************************************************************/
int fk_acore_get_frames_info(void)
{
	int frames = aclient.frames_per_blk;

	return frames;
}

/************************************************************************
 * function: fk_acore_set_gapless_info
 * description: set gapless info on compress playback scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_set_gapless_info(struct acore_stream *ac_strm,
	uint32_t type, uint32_t value)
{
	int ret = EOK;

	if (ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_PLAY) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "stream scene(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	if (aclient.gapless_mode == false) {
		AUD_LOG_INFO(AUD_CORE, "gapless mode is false.");
		return ret;
	}

	if (type == PARAMS_GAPLESS_INIT_SAMPS) {
		ac_strm->initial_samples = value;
		AUD_LOG_INFO(AUD_CORE, "initial samples:%d.", ac_strm->initial_samples);
	} else if (type == PARAMS_GAPLESS_TRAIL_SAMPS) {
		ac_strm->trailing_samples = value;
		AUD_LOG_INFO(AUD_CORE, "trailing samples:%d.", ac_strm->trailing_samples);
	}

	ac_strm->use_gapless = 1;

	return ret;
}

/************************************************************************
 * function: fk_acore_kws_event_status
 * description: kws load module on voice trigger scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_kws_event_status(struct acore_stream *ac_strm, uint32_t *event_data)
{
	int ret = 0;

	atomic_set(&ac_strm->event_wait, 1);

	if (atomic_read(&aclient.ssr_flag)) {
		AUD_LOG_INFO(AUD_CORE, "adsp reset");
		return -1;
	}

	ret = wait_event_freezable(ac_strm->cmd_wait_que,
		!atomic_read(&ac_strm->event_wait));
	if (!ret) {
		*event_data = ac_strm->det_event;
		AUD_LOG_INFO_LIM(AUD_CORE, "rec_event is %d", ac_strm->det_event);
	}
	AUD_LOG_DBG(AUD_CORE, "wait_event_freezable = %d", ret);
	return ret;
}

/************************************************************************
 * function: fk_acore_kws_load_module
 * description: kws load module on voice trigger scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_kws_load_module(struct acore_stream *ac_strm, struct kws_module_info *module_info)
{
	int ret = 0;
	struct xring_aud_msg kws_module_info_msg = {0};

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if (ac_strm->adsp_sc_t != ADSP_SCENE_KWS_RECORD) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	/* fill message */
	kws_module_info_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	kws_module_info_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	kws_module_info_msg.header.cmd_id = ADSP_KWS_CTRL;
	kws_module_info_msg.header.func_id = ADSP_KWS_CTRL_LOAD_MODEL;
	kws_module_info_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(struct kws_module_info);
	kws_module_info_msg.header.result = 0;

	module_info = (struct kws_module_info *)&kws_module_info_msg.data[0];

	ret = fk_acore_message_send((struct xring_aud_msg *)&kws_module_info_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send kws_module info failure.");
		return ret;
	}
	return ret;
}

/************************************************************************
 * function: fk_acore_kws_unload_module
 * description: kws load module on voice trigger scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_kws_unload_module(struct acore_stream *ac_strm, struct kws_module_info *module_info)
{
	int ret = 0;
	struct xring_aud_msg kws_module_info_msg = {0};

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if (ac_strm->adsp_sc_t != ADSP_SCENE_KWS_RECORD) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	ac_strm->det_event = VT_RET_VAL;
	atomic_set(&ac_strm->event_wait, 0);
	wake_up(&ac_strm->cmd_wait_que);

	/* fill message */
	kws_module_info_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	kws_module_info_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	kws_module_info_msg.header.cmd_id = ADSP_KWS_CTRL;
	kws_module_info_msg.header.func_id = ADSP_KWS_CTRL_UNLOAD_MODEL;
	kws_module_info_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(struct kws_module_info);
	kws_module_info_msg.header.result = 0;

	module_info = (struct kws_module_info *)&kws_module_info_msg.data[0];

	ret = fk_acore_message_send((struct xring_aud_msg *)&kws_module_info_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send del_kws_module info failure.");
		return ret;
	}
	return ret;
}

/************************************************************************
 * function: fk_acore_kws_start_rec
 * description: kws start rec on voice trigger scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_kws_start_rec(struct acore_stream *ac_strm, struct kws_module_info *module_info)
{
	int ret = EOK;
	struct xring_aud_msg kws_start_rec_msg = {0};

	AUD_LOG_INFO(AUD_CORE, "session id is 0x%x.", ac_strm->sess_id);

	atomic_set(&ac_strm->start_rec_wait, 1);
	atomic_set(&ac_strm->put_block_no, 0);
	atomic_set(&ac_strm->rel_block_no, 0);

	/* fill scene startup message */
	kws_start_rec_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	kws_start_rec_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	kws_start_rec_msg.header.cmd_id = ADSP_KWS_CTRL;
	kws_start_rec_msg.header.func_id = ADSP_KWS_CTRL_START_RECOGNITION;
	kws_start_rec_msg.header.len = sizeof(struct xring_aud_msg_header);
	kws_start_rec_msg.header.result = 0;

	/* send starup message */
	ret = fk_acore_message_send((struct xring_aud_msg *)&kws_start_rec_msg);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_CORE, "send start message failure.");
		ret = -EINVAL;
	}

	ret = wait_event_timeout(ac_strm->cmd_wait_que,
				!atomic_read(&ac_strm->start_rec_wait), WAIT_TIMEOUT);
	if (!ret && atomic_read(&ac_strm->start_rec_wait)) {
		//ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "audio ipc timeout start rec timeout. state=%d wait=%d",
				ac_strm->state, atomic_read(&ac_strm->start_rec_wait));
	} else {
		ret = EOK;
		AUD_LOG_DBG(AUD_CORE, "start rec success");
	}

	/*print ipc send & ack count*/
	fk_audio_dump_ipc();

	return ret;
}

/************************************************************************
 * function: fk_acore_kws_stop_rec
 * description: kws stop rec on voice trigger scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_kws_stop_rec(struct acore_stream *ac_strm, struct kws_module_info *module_info)
{
	int ret = 0;
	struct xring_aud_msg kws_stop_rec_msg = {0};

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if (ac_strm->adsp_sc_t != ADSP_SCENE_KWS_RECORD) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}
	atomic_set(&ac_strm->stop_rec_wait, 1);

	/*vote adsp poweron*/
	ret = fk_adsp_vote_register(ac_strm->sess_id);
	if (ret < 0)
		AUD_LOG_ERR(AUD_CORE, "vt vote adsp power on failed.");

	/* fill message */
	kws_stop_rec_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	kws_stop_rec_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	kws_stop_rec_msg.header.cmd_id = ADSP_KWS_CTRL;
	kws_stop_rec_msg.header.func_id = ADSP_KWS_CTRL_STOP_RECOGNITION;
	kws_stop_rec_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(struct kws_module_info);
	kws_stop_rec_msg.header.result = 0;

	ret = fk_acore_message_send((struct xring_aud_msg *)&kws_stop_rec_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send kws stop rec failure.");
		return ret;
	}
	ret = wait_event_timeout(ac_strm->cmd_wait_que,
				!atomic_read(&ac_strm->stop_rec_wait), WAIT_TIMEOUT);
	if (!ret && atomic_read(&ac_strm->stop_rec_wait)) {
		ret = EOK;
		AUD_LOG_ERR(AUD_CORE, "audio ipc timeout stop rec timeout. state=%d wait=%d",
				ac_strm->state, atomic_read(&ac_strm->stop_rec_wait));
	} else {
		ret = EOK;
		AUD_LOG_DBG(AUD_CORE, "stop rec success");
	}
	return ret;
}

/************************************************************************
 * function: fk_acore_kws_ec_ref
 * description: kws ec ref on voice trigger scene
 * parameter:
 *		ac_strm : stream instance
 *		type	: gapless info type
 *		value	: parameter value
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_kws_ec_ref(struct acore_stream *ac_strm, struct ec_ctrl *kws_ec_ref)
{
	int ret = 0;
	struct xring_aud_msg kws_ec_ref_msg = {0};
	struct ec_ctrl *kws_ec_info;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if (ac_strm->adsp_sc_t != ADSP_SCENE_KWS_RECORD) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}

	kws_ec_ref_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	kws_ec_ref_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	kws_ec_ref_msg.header.cmd_id = ADSP_PARAM_CTRL;
	kws_ec_ref_msg.header.func_id = ADSP_PARAM_CTRL_EC;
	kws_ec_ref_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(struct ec_ctrl);
	kws_ec_ref_msg.header.result = 0;

	kws_ec_info = (struct ec_ctrl *)&kws_ec_ref_msg.data[0];
	kws_ec_info->ec_type = kws_ec_ref->ec_type;
	kws_ec_info->port_id = kws_ec_ref->port_id;
	kws_ec_info->channel = kws_ec_ref->channel;
	kws_ec_info->bit_width = kws_ec_ref->bit_width;
	kws_ec_info->sample_rate = kws_ec_ref->sample_rate;

	ret = fk_acore_message_send((struct xring_aud_msg *)&kws_ec_ref_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send kws ec ref msg failure.");
		return ret;
	}
	return ret;
}

/************************************************************************
 * function: fk_acore_speech_bandwidth
 * description: set voice bandwidth
 * parameter:
 *		ac_strm : stream instance
 *		band	: bacndwidth
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_speech_bandwidth(struct acore_stream *ac_strm, enum speech_band band)
{
	int ret = 0;
	struct xring_aud_msg speech_bandwidth_msg = {0};
	uint32_t *band_info = NULL;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_UL)) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "scene id(%d) unexpect.", ac_strm->adsp_sc_t);
		return ret;
	}


	/* fill message */
	speech_bandwidth_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	speech_bandwidth_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	speech_bandwidth_msg.header.cmd_id = ADSP_VOICE_CTRL;
	speech_bandwidth_msg.header.func_id = ADSP_VOICE_CTRL_SET_BANDWIDTH;
	speech_bandwidth_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(uint32_t);

	band_info = (uint32_t *)&speech_bandwidth_msg.data[0];
	*band_info = (uint32_t)band;

	ret = fk_acore_message_send((struct xring_aud_msg *)&speech_bandwidth_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send speech bandwidth failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_set_volume
 * description: set vol
 * parameter:
 *		ac_strm : stream instance
 *		volume	: vol
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_set_volume(struct acore_stream *ac_strm, int volume)
{
	int ret = 0;
	struct xring_aud_msg volume_msg = {0};
	int *volume_index = NULL;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	/* fill message */
	volume_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	volume_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;

	if ((ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_DL) ||
		(ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_DL)) {
		volume_msg.header.cmd_id = ADSP_VOICE_CTRL;
		volume_msg.header.func_id = ADSP_VOICE_CTRL_VOL;
	} else {
		volume_msg.header.cmd_id = ADSP_AUD_CTRL;
		volume_msg.header.func_id = ADSP_AUD_CTRL_VOL;
	}

	volume_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(int);

	volume_index = (int *)&volume_msg.data[0];
	*volume_index = volume;

	ret = fk_acore_message_send((struct xring_aud_msg *)&volume_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send volume failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_set_mute_flag
 * description: set mute
 * parameter:
 *		ac_strm : stream instance
 *		mute	: mute flag
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_set_mute_flag(struct acore_stream *ac_strm, uint8_t mute)
{
	int ret = 0;
	struct xring_aud_msg mute_msg = {0};
	int *mute_flag = NULL;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	/* fill message */
	mute_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	mute_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;

	if (ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_DL) {
		mute_msg.header.cmd_id = ADSP_VOICE_CTRL;
		mute_msg.header.func_id = ADSP_VOICE_CTRL_DL_MUTE;
	} else if (ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_UL) {
		mute_msg.header.cmd_id = ADSP_VOICE_CTRL;
		mute_msg.header.func_id = ADSP_VOICE_CTRL_UL_MUTE;
	} else {
		mute_msg.header.cmd_id = ADSP_AUD_CTRL;
	}

	mute_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(int);

	mute_flag = (int *)&mute_msg.data[0];
	*mute_flag = (int)mute;

	ret = fk_acore_message_send((struct xring_aud_msg *)&mute_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send mute failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_set_vt_fca_flag
 * description: set vt_fca_flag
 * parameter:
 *		ac_strm : stream instance
 *		flag	:  vt/fca flag
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_set_vt_fca_flag(struct acore_stream *ac_strm, uint8_t flag)
{
	int ret = 0;
	struct xring_aud_msg flag_msg = {0};
	uint8_t *vt_fca_flag = NULL;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}
	AUD_LOG_INFO(AUD_CORE, "flag= %d", flag);

	/*vote power on for fca_flag when vt in low power*/
	fk_adsp_vote_register(ac_strm->sess_id);

	atomic_set(&ac_strm->vt_fca_flag_wait, 1);

	/* fill message */
	flag_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	flag_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;

	if (ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD) {
		flag_msg.header.cmd_id = ADSP_KWS_CTRL;
		flag_msg.header.func_id = ADSP_KWS_CTRL_VT_FCA_FLAG;
	} else {
		flag_msg.header.cmd_id = ADSP_AUD_CTRL;
	}

	flag_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(int);

	vt_fca_flag = &flag_msg.data[0];
	*vt_fca_flag = flag;

	ret = fk_acore_message_send((struct xring_aud_msg *)&flag_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send flag failure.");
		return ret;
	}

	ret = wait_event_timeout(ac_strm->cmd_wait_que,
				!atomic_read(&ac_strm->vt_fca_flag_wait), WAIT_TIMEOUT);
	if (!ret && atomic_read(&ac_strm->vt_fca_flag_wait)) {
		//ret = -EINVAL;
		AUD_LOG_INFO(AUD_CORE, "set vt_fca_flag timeout. state=%d wait=%d",
				ac_strm->state, atomic_read(&ac_strm->vt_fca_flag_wait));
	} else {
		ret = EOK;
		AUD_LOG_DBG(AUD_CORE, "set vt_fca_flag success");
	}
	/*vote power off for fca_flag when vt in low power*/
	fk_adsp_vote_unregister(ac_strm->sess_id);
	return ret;
}

/************************************************************************
 * function: fk_acore_set_fca_event
 * description: set fca_event
 * parameter:
 *		ac_strm : stream instance

 * return:
 *		0: success
 *		other: failure
 ************************************************************************/

int fk_acore_set_fca_event(struct acore_stream *ac_strm)
{
	int ret = 0;
	struct xring_aud_msg event_msg = {0};

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "acore stream ptr is NULL");
		return ret;
	}

	event_msg.header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
	event_msg.header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
	event_msg.header.cmd_id = ADSP_KWS_CTRL;
	event_msg.header.func_id = ADSP_KWS_CTRL_FCA_EVENT;
	event_msg.header.len = sizeof(struct xring_aud_msg_header);
	event_msg.header.result = 0;

	ret = fk_acore_message_send((struct xring_aud_msg *)&event_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send event failure.");
		return ret;
	}

	return ret;
}


/************************************************************************
 * function: fk_acore_message_send
 * description: send message to ipc.
 * parameter:
 *		msg : message content
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_message_send(struct xring_aud_msg *msg)
{
	int ret = EOK;

	AUD_LOG_DBG_LIM(AUD_CORE, "scene(0x%x) ins(0x%x) cmd(0x%x) func(0x%x) ",
		msg->header.scene_id, msg->header.ins_id,
		msg->header.cmd_id, msg->header.func_id);

#ifdef SIMULATE_DEBUG
{
	struct cmd_cb_list_node *cmd_cb_node = NULL;
	struct xring_aud_msg *cmd_cb_msg = NULL;

	cmd_cb_node = kzalloc(sizeof(struct cmd_cb_list_node), GFP_KERNEL);
	if (!cmd_cb_node) {
		AUD_LOG_ERR(AUD_CORE, "alloc cmd node failure.");
		return -ENOMEM;
	}

	cmd_cb_msg = (struct xring_aud_msg *)&cmd_cb_node->msg;
	memcpy(cmd_cb_msg, msg, msg->header.len);

	/* add data node to tail of list */
	list_add_tail(&cmd_cb_node->cmd_list, &aclient.cmd_cb_list);

	AUD_LOG_INFO_LIM(AUD_CORE, "fill cmd msg");
}
#else
	if (atomic_read(&aclient.ssr_flag)) {
		AUD_LOG_INFO(AUD_CORE, "adsp reset");
		return ret;
	}

	/* Ensure the ADSP is powered on before the message send */
	if (get_adsp_power_status() == false)
		AUD_LOG_INFO(AUD_CORE, "adsp power off");

	/* FK-AUDIO_MAILBOX begin */
	msg->header.msg_source = AP_AUDIO_MSG;
	/* FK-AUDIO_MAILBOX end */
	ret = fk_audio_ipc_send_async(AP_AUDIO_MSG, (unsigned int *)msg,
			msg->header.len, NULL, NULL);
#endif
	return ret;
}

/************************************************************************
 * function: fk_aocre_common_message_send
 * description: send message to ipc.
 * parameter:
 *		msg : message content
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_aocre_common_message_send(struct xring_aud_msg *msg)
{
	int ret = 0;

	AUD_LOG_DBG_LIM(AUD_COMM, "scene(0x%x) ins(0x%x) cmd(0x%x) func(0x%x) ",
		msg->header.scene_id, msg->header.ins_id,
		msg->header.cmd_id, msg->header.func_id);

	if (fk_adsp_vote_register(AUDIO_COMMON_CONN_SESS) < 0)
		AUD_LOG_DBG(AUD_COMM, "audio common dis power up.");

	/* FK-AUDIO_MAILBOX begin */
	msg->header.msg_source = AP_AUDIO_MSG;
	/* FK-AUDIO_MAILBOX end */
	ret = fk_audio_ipc_send_async(AP_AUDIO_MSG, (unsigned int *)msg,
			msg->header.len, NULL, NULL);

	if (fk_adsp_vote_unregister(AUDIO_COMMON_CONN_SESS) < 0)
		AUD_LOG_DBG(AUD_COMM, "audio common disconnect power down.");

	return ret;
}

/************************************************************************
 * function: fk_audio_set_dsp_temp
 * description: set adsp temperature ctrl cmd
 * parameter:
 *		temp : adsp temperature
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_set_dsp_temp(int temp)
{
	int ret = 0;
	struct xring_aud_msg cmd_msg = {0};
	int *temp_index = NULL;

	/* fill message */
	cmd_msg.header.scene_id = ADSP_SCENE_NONE;
	cmd_msg.header.cmd_id = ADSP_SYS_CTRL;
	cmd_msg.header.func_id = ADSP_SYS_CTRL_TEMP;

	cmd_msg.header.len = sizeof(struct xring_aud_msg_header) + sizeof(int);

	temp_index = (int *)&cmd_msg.data[0];
	*temp_index = temp;

	ret = fk_aocre_common_message_send((struct xring_aud_msg *)&cmd_msg);
	if (ret < 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "send common msg failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_pre_ssr
 * description: pre-process when audio subsystem assert
 * parameter:
 *		void *priv : private parameter
 * return:
 *
 ************************************************************************/
void fk_acore_pre_ssr(void *priv)
{
	struct acore_stream *acStrm = NULL;
	struct acore_stream *acStrm_temp;

	AUD_LOG_INFO(AUD_CORE, "begin");

	atomic_set(&aclient.ssr_flag, 1);

	atomic_set(&aclient.vote_num, 0);

	/* reset mailbox */
	fk_mbx_msg_reset(NULL);

	/* Traverse the currently activated audio stream
	 * and update status information.
	 */
	mutex_lock(&aclient.list_lock);

	list_for_each_entry_safe(acStrm, acStrm_temp, &aclient.ac_strm_list, strm_list) {
		if (!atomic_read(&acStrm->init_flag))
			continue;

		AUD_LOG_INFO(AUD_CORE, "reset session(0x%x)", acStrm->sess_id);

		acStrm->det_event = VT_RET_VAL;
		atomic_set(&acStrm->flush_flag, 0);
		atomic_set(&acStrm->last_frame, 0);

		atomic_set(&acStrm->startup_wait, 0);
		atomic_set(&acStrm->start_wait, 0);
		atomic_set(&acStrm->stop_wait, 0);
		atomic_set(&acStrm->shutdown_wait, 0);
		atomic_set(&acStrm->timestamp_wait, 0);
		atomic_set(&acStrm->event_wait, 0);
		atomic_set(&acStrm->start_rec_wait, 0);
		atomic_set(&acStrm->stop_rec_wait, 0);
		wake_up(&acStrm->cmd_wait_que);

		atomic_set(&acStrm->put_block_no, 0);
		atomic_set(&acStrm->rel_block_no, 0);
	}

	mutex_unlock(&aclient.list_lock);

	fk_adsp_vote_reset();

	AUD_LOG_INFO(AUD_CORE, "end");
}

/************************************************************************
 * function: fk_acore_post_ssr
 * description: post-process when audio subsystem assert
 * parameter:
 *		void *priv : private parameter
 * return:
 ************************************************************************/
void fk_acore_post_ssr(void *priv)
{
	struct acore_stream *acStrm = NULL;
	struct acore_stream *acStrm_temp;

	AUD_LOG_INFO(AUD_CORE, "begin");

	mutex_lock(&aclient.list_lock);

	list_for_each_entry_safe(acStrm, acStrm_temp, &aclient.ac_strm_list, strm_list) {
		if (!atomic_read(&acStrm->init_flag))
			continue;

		AUD_LOG_INFO(AUD_CORE, "session(0x%x) still active", acStrm->sess_id);

		if (acStrm->state != ACORE_STATE_IDLE)
			AUD_LOG_ERR(AUD_CORE, "session(0x%x) state unexpect", acStrm->state);

		/* update stream state */
		atomic_set(&acStrm->init_flag, 0);
		acStrm->fe_flag = 0;
		acStrm->state = ACORE_STATE_IDLE;
	}

	mutex_unlock(&aclient.list_lock);

	atomic_set(&aclient.ssr_flag, 0);

	AUD_LOG_INFO(AUD_CORE, "end");
}

static struct recovery_ops acore_ssr_ops = {
	.pre_recovery = fk_acore_pre_ssr,
	.post_recovery = fk_acore_post_ssr,
};


#ifdef SIMULATE_DEBUG

static int fk_acore_simulate_read(struct acore_stream *ac_strm)
{
	int ret = 0;
	int r_off = 0, w_off = 0;
	uint32_t phy_addr = 0;
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;

	int frames_per_blk = 0;
	int samples_per_frame = 0;
	int compress_rate = 0;

	struct cmd_cb_list_node *cmd_cb_node = NULL;
	struct xring_aud_msg *msg = NULL;
	struct adsp_data_buf_info *data = NULL;

	if (!ac_strm) {
		AUD_LOG_INFO_LIM(AUD_CORE, "ac_strm is NULL.");
		return ret;
	}

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_NORMAL_RECORD) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_RECORD) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOIP_UL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_FAST_RECORD) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOICE_CALL_RECORD)) {
		return 0;
	}

	smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
	r_off = smem_head_ptr->r_off;
	w_off = smem_head_ptr->w_off;

	AUD_LOG_INFO_LIM(AUD_CORE, "r_off(%d) w_off(%d) size(%d)",
		r_off, w_off, ac_strm->smem.periods_size);

	if (r_off > w_off) {
		cmd_cb_node = kzalloc(sizeof(struct cmd_cb_list_node),
			GFP_KERNEL);
		if (!cmd_cb_node) {
			AUD_LOG_ERR(AUD_CORE, "alloc cmd node failure.");
			return -ENOMEM;
		}

		msg = (struct xring_aud_msg *)&cmd_cb_node->msg;
		msg->header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
		msg->header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
		msg->header.cmd_id = ADSP_AUD_CTRL;
		msg->header.func_id = ADSP_AUD_CTRL_READ;
		msg->header.len = sizeof(struct xring_aud_msg_header) +
			sizeof(struct adsp_data_buf_info);
		msg->header.result = 0;

		data = (struct adsp_data_buf_info *)&msg->data[0];

		if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
			frames_per_blk = fk_acore_get_frames_info();
			samples_per_frame = 1024; //SAMPLES_PER_AAC_FRAME
			compress_rate = 15; //COMPRESS_RATE_AAC_DEFAULT
			data->len = frames_per_blk * samples_per_frame / compress_rate;
		} else {
			data->len = ac_strm->smem.periods_num *
				fk_acore_get_bytes_per_sample(ac_strm);
		}

		phy_addr = ac_strm->smem.p_addr + (w_off / ac_strm->smem.periods_num)
				* data->len;
			data->phy_addr = (uint32_t)(phy_addr & 0xFFFFFFFF);

		cmd_cb_node->priv = ac_strm;

		/* update write offset */
		w_off++;
		smem_head_ptr->w_off = w_off;
		AUD_LOG_INFO_LIM(AUD_CORE, "fill read msg");

		/* add data node to tail of list */
		list_add_tail(&cmd_cb_node->cmd_list, &aclient.cmd_cb_list);
	}

	return ret;
}

static int fk_acore_simulate_write(struct acore_stream *ac_strm)
{
	int ret = 0;
	struct adsp_pcm_smem_head *smem_head_ptr = NULL;

	if (!ac_strm) {
		AUD_LOG_INFO_LIM(AUD_CORE, "ac_strm is NULL.");
		return ret;
	}

	AUD_LOG_INFO_LIM(AUD_CORE, "scene(%d) state(%d)",
		ac_strm->adsp_sc_t, ac_strm->state);

	if ((ac_strm->adsp_sc_t != ADSP_SCENE_FAST_PLAY) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_NORMAL_PLAY) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_OFFLOAD_PLAY) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_VOIP_DL) &&
		(ac_strm->adsp_sc_t != ADSP_SCENE_MUSIC_INCALL_PLAY)) {
		return 0;
	}

	if (ac_strm->state == ACORE_STATE_RUN) {
		int r_off = 0, w_off = 0;
		struct cmd_cb_list_node *cmd_cb_node = NULL;
		struct xring_aud_msg *msg = NULL;
		struct adsp_data_buf_info *data = NULL;
		uint32_t phy_addr = 0;

		smem_head_ptr = (ac_strm->smem.vir_addr - SMEM_HEAD_SIZE);
		r_off = smem_head_ptr->r_off;
		w_off = smem_head_ptr->w_off;

		AUD_LOG_INFO_LIM(AUD_CORE, "p_size(%d) r_off(%d) w_off(%d)",
			ac_strm->smem.periods_size, r_off, w_off);

		if (w_off > r_off) {
			cmd_cb_node = kzalloc(sizeof(struct cmd_cb_list_node),
				GFP_KERNEL);
			if (!cmd_cb_node) {
				AC_LOG_ERR_LIM("alloc cmd node failure.");
				return -ENOMEM;
			}

			msg = (struct xring_aud_msg *)&cmd_cb_node->msg;
			msg->header.scene_id = (uint8_t)ac_strm->adsp_sc_t;
			msg->header.ins_id = (uint8_t)ac_strm->adsp_ins_id;
			msg->header.cmd_id = ADSP_AUD_CTRL;
			msg->header.func_id = ADSP_AUD_CTRL_WRITE;
			msg->header.len = sizeof(struct xring_aud_msg_header) +
				sizeof(struct adsp_data_buf_info);
			msg->header.result = 0;

			data = (struct adsp_data_buf_info *)&msg->data[0];

			/* update read offset */
			r_off++;
			smem_head_ptr->r_off = r_off;
			AUD_LOG_INFO_LIM(AUD_CORE, "fill write msg. r_off(%d) w_off(%d)",
				smem_head_ptr->r_off, smem_head_ptr->w_off);

			data->len = ac_strm->smem.periods_size *
				fk_acore_get_bytes_per_sample(ac_strm);
			phy_addr = ac_strm->smem.p_addr + (r_off / ac_strm->smem.periods_num)
				* data->len;
			data->phy_addr = (uint32_t)(phy_addr & 0xFFFFFFFF);
			data->frame_no = r_off;

			cmd_cb_node->priv = ac_strm;

			/* add data node to tail of list */
			list_add_tail(&cmd_cb_node->cmd_list, &aclient.cmd_cb_list);
		}
	}

	return ret;
}


static int fk_acore_simulate_proc_thread(void *priv)
{
	struct cmd_cb_list_node *cmd_cb_node = NULL;
	struct xring_aud_msg *msg = NULL;
	struct acore_stream *ac_strm = NULL;

	AUD_LOG_INFO_LIM(AUD_CORE, "enter");

	for (;;) {
		if (list_empty(&aclient.cmd_cb_list)) {
			msleep(60);
			//continue;
		} else {
			cmd_cb_node = list_first_entry(&aclient.cmd_cb_list,
				struct cmd_cb_list_node, cmd_list);

			msg = (struct xring_aud_msg *)&cmd_cb_node->msg;
			AUD_LOG_INFO_LIM(AUD_CORE, " scene(%d) cmd(0x%x) func(0x%x)",
				msg->header.scene_id,
				msg->header.cmd_id,
				msg->header.func_id);

			ac_strm = fk_acore_stream_get_by_scene(msg->header.scene_id,
				msg->header.ins_id);
			if (!ac_strm) {
				AUD_LOG_INFO_LIM(AUD_CORE, "ac stream is null.");
				list_del(&cmd_cb_node->cmd_list);
				continue;
			}

			if (msg->header.func_id == ADSP_AUD_CTRL_WRITE) {
				if (ac_strm->state == ACORE_STATE_IDLE) {
					list_del(&cmd_cb_node->cmd_list);
					AUD_LOG_INFO_LIM(AUD_CORE, "idle state.");
					msleep(100);
					continue;
				}
			}

			list_del(&cmd_cb_node->cmd_list);

			fk_acore_msg_handle((void *)msg, msg->header.len, NULL);

			kfree(cmd_cb_node);

			msleep(20);
		}

		if (!ac_strm) {
			msleep(20);
			continue;
		}

		if ((ac_strm->adsp_sc_t == ADSP_SCENE_NORMAL_RECORD) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_VOIP_UL) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_FAST_RECORD) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_KWS_RECORD) ||
			(ac_strm->adsp_sc_t == ADSP_SCENE_VOICE_CALL_RECORD)) {
			if (ac_strm->state == ACORE_STATE_RUN)
				fk_acore_simulate_read(ac_strm);
		} else if ((ac_strm->adsp_sc_t == ADSP_SCENE_FAST_PLAY) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_NORMAL_PLAY) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_VOIP_DL) ||
				(ac_strm->adsp_sc_t == ADSP_SCENE_MUSIC_INCALL_PLAY)) {
			if (ac_strm->state == ACORE_STATE_RUN)
				fk_acore_simulate_write(ac_strm);
		}
	}

	return 0;
}
#endif

/************************************************************************
 * function: fk_acore_get_acore_client
 * description:
 * parameter:
 * return:
 *		other: success
 *		NULL: failure
 ************************************************************************/
void *fk_acore_get_acore_client(void)
{
	return (void *)&aclient;
}

int __init fk_acore_init(void)
{
	aclient.init_flag = 1;

	/* initial audio stream list */
	INIT_LIST_HEAD(&aclient.ac_strm_list);
	mutex_init(&aclient.list_lock);

	aclient.route_table = NULL;

	atomic_set(&aclient.ac_num, 0);

	/* init vote status */
	atomic_set(&aclient.vote_num, 0);
	/* initial adsp vote list */
	INIT_LIST_HEAD(&aclient.vote_list);
	mutex_init(&aclient.vote_lock);

#ifdef SIMULATE_DEBUG
	INIT_LIST_HEAD(&aclient.cmd_cb_list);

	aclient.cb_thrd_fn = kthread_run(fk_acore_simulate_proc_thread, NULL, "acthrd%d", 1);
	AUD_LOG_INFO(AUD_CORE, "cb thread run");
#else
	fk_audio_ipc_recv_register(AP_AUDIO_MSG, fk_acore_msg_handle, NULL);
#endif

	/* register audio ssr operation */
	aclient.ssr_act = fk_adsp_ssr_register(&acore_ssr_ops, &aclient);

	audio_debugfs_init((struct acore_client *)&aclient);

	/* unsupport backdoor default */
	aclient.backdoor_boot = false;

	aclient.frames_per_blk = FRAMES_PER_BLOCK_DEFAULT;

	/* unsupport gapless mode default */
	aclient.gapless_mode = false;

	event_ws = wakeup_source_register(NULL, "audio_event_wakeup");

	return 0;
}

void fk_acore_exit(void)
{

#ifndef SIMULATE_DEBUG
	fk_audio_ipc_recv_unregister(AP_AUDIO_MSG);

	audio_debugfs_deinit((struct acore_client *)&aclient);
#endif

	fk_acore_stream_free_all();

	/* unregister audio ssr operation */
	fk_adsp_ssr_unregister(aclient.ssr_act);
}

// module_init(fk_acore_init);
// module_exit(fk_acore_exit);

// /* Module information */
// MODULE_DESCRIPTION("XRING AUDIO CORE driver");
// MODULE_LICENSE("Dual BSD/GPL");

