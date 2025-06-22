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
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/delay.h>


#include "fk-acore-process.h"
#include "fk-acore.h"
#include "fk-acore-utils.h"

#include "../common/fk-audif-dev.h"

#define AUDIF_FIFO_EMPTY_VALUE			(0)
#define AUDIF_FIFO_FULL_WATERMARK		(384)
#define AUDIF_FIFO_FULL_VALUE			(512)

/************************************************************************
 * function: fk_acore_process_interleaver
 * description: data interweave. just support dual channels.
 * parameter:
 *		input : input buffer pointer. left channel data local in upper
 *				half. right channel data local on low half.
 *		size : avalid data length of input buffer.
 *		bit_width : bit width info.
 *		output : output buffer pointer.
 * return:
 ************************************************************************/
void fk_acore_process_interleaver(uint8_t *input, uint32_t size,
	uint8_t bit_width, uint8_t *output)
{
	uint8_t bytes = 0;
	uint32_t samples = 0;
	uint8_t *tmp_input1 = NULL;
	uint8_t *tmp_input2 = NULL;
	uint8_t *tmp_output = output;

	int i, j;

	/* calc bytes per channel one sample*/
	bytes = bit_width / 8;
	/* calc samples with size. dual channels default */
	samples = size / (bytes * 2);

	tmp_input1 = input;
	tmp_input2 = input + (size / 2);

	for (i = 0; i < samples; i++) {
		for (j = 0; j < bytes; j++) {
			*tmp_output = *tmp_input1++;
			*(tmp_output + bytes) = *tmp_input2++;
		}
		tmp_output += (bytes * 2);
	}
}


/************************************************************************
 * function: fk_acore_process_deinterleaver
 * description: data deinterleaver. just support dual channels.
 * parameter:
 *		input : input buffer pointer
 *		size : avalid data length of input buffer
 *		bit_width : bit width info
 *		output : output buffer pointer. eft channel data local in upper
 *				half. right channel data local on low half.
 * return:
 ************************************************************************/
void fk_acore_process_deinterleaver(uint8_t *input, uint32_t size,
	uint8_t bit_width, uint8_t *output)
{
	uint8_t bytes = 0;
	uint32_t samples = 0;
	uint8_t *tmp_input = input;
	uint8_t *tmp_output1 = NULL;
	uint8_t *tmp_output2 = NULL;

	int i, j;

	/* calc bytes per channel one sample*/
	bytes = bit_width / 8;
	/* calc samples with size. dual channels default */
	samples = size / (bytes * 2);

	tmp_output1 = output;
	tmp_output2 = output + (size / 2);

	for (i = 0; i < samples; i++) {
		for (j = 0; j < bytes; j++) {
			*tmp_output1++ = *tmp_input;
			*tmp_output2++ = *(tmp_input+bytes);
			tmp_input++;
		}
		tmp_input += (bytes * 2);
	}
}

/************************************************************************
 * function: fk_acore_data_type_fill
 * description: fill data type info.
 * parameter:
 *		ac_strm : stream instance
 *		type : stream data type info
 * return:
 ************************************************************************/
void fk_acore_data_type_fill(struct acore_stream *ac_strm, struct data_type *type)
{
	/* fill sample rate info */
	switch (ac_strm->fmt.samples) {
	case SAMPLERATE_8K:
		type->data_sample = SAMPLE_8000;
		break;
	case SAMPLERATE_12K:
		type->data_sample = SAMPLE_12000;
		break;
	case SAMPLERATE_16K:
		type->data_sample = SAMPLE_16000;
		break;
	case SAMPLERATE_24K:
		type->data_sample = SAMPLE_24000;
		break;
	case SAMPLERATE_32K:
		type->data_sample = SAMPLE_32000;
		break;
	case SAMPLERATE_48K:
		type->data_sample = SAMPLE_48000;
		break;
	default:
		type->data_sample = SAMPLE_48000;
		AUD_LOG_INFO_LIM(AUD_CORE, "unsupport sample rate:%d ",
			ac_strm->fmt.samples);
		break;
	}

	/* fill format info */
	switch (ac_strm->fmt.bit_width) {
	case PCM_FORMAT_8BIT:
		type->data_format = FORMAT_8BIT;
		break;
	case PCM_FORMAT_16BIT:
		type->data_format = FORMAT_16BIT;
		break;
	case PCM_FORMAT_24BIT:
		type->data_format = FORMAT_24BIT;
		break;
	case PCM_FORMAT_32BIT:
		type->data_format = FORMAT_32BIT;
		break;
	default:
		type->data_format = FORMAT_16BIT;
		AUD_LOG_INFO_LIM(AUD_CORE, "unsupport format:%d ",
			ac_strm->fmt.bit_width);
		break;
	}

	/* fill channel info */
	switch (ac_strm->fmt.channels) {
	case CHANNEL_MONO:
		type->data_channel = CHANNEL_1;
		break;
	case CHANNEL_STEREO:
		type->data_channel = CHANNEL_2;
		break;
	default:
		type->data_channel = CHANNEL_2;
		AUD_LOG_INFO_LIM(AUD_CORE, "unsupport channel:%d ",
			ac_strm->fmt.channels);
		break;
	}
}

/************************************************************************
 * function: fk_acore_process_startup
 * description: config audio if to start data transfer.
 * parameter:
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
void fk_acore_process_startup(struct acore_stream *ac_strm)
{
	struct data_type type = {0};
	int ap_fifo_watermark = 0;

	ac_strm->audif_clk_status = audif_clk_status_read();
	if (ac_strm->audif_clk_status)
		return;

	AUD_LOG_INFO(AUD_CORE, "put process start cond ");
	if (ac_strm->strm_dir == STRM_DIR_PLAYBACK) {
		/* fill data info and register to audif driver */
		fk_acore_data_type_fill(ac_strm, (struct data_type *)&type);
		audif_ap_fifo_write_register(fk_acore_playback_data_trigger, ac_strm,
			(struct data_type *)&type);

		/* set AP fifo empty watermark */
		ap_fifo_watermark = AUDIF_FIFO_FULL_WATERMARK;
		audif_fifo_empty_watermark_ap_set(ap_fifo_watermark);
		/* flush ap fifo */
		audif_fifo_clr_ap_enable();

	} else {
		/* fill data info and register to audif driver */
		fk_acore_data_type_fill(ac_strm, (struct data_type *)&type);
		audif_ar_fifo_read_register(fk_acore_capture_data_trigger, ac_strm,
			(struct data_type *)&type);

		/* set AR fifo full watermark */
		if ((ac_strm->smem.periods_size > AUDIF_FIFO_EMPTY_VALUE) &&
			(ac_strm->smem.periods_size < AUDIF_FIFO_FULL_VALUE)) {
			audif_fifo_empty_watermark_ap_set(ac_strm->smem.periods_size);
		} else {
			audif_fifo_empty_watermark_ap_set(AUDIF_FIFO_FULL_WATERMARK);
		}

		/* enable ar fifo */
		audif_ad_fifo_enable_ar(true);
	}
}

/************************************************************************
 * function: fk_acore_process_start
 * description: config audio if to start data transfer.
 * parameter:
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
void fk_acore_process_start(struct acore_stream *ac_strm)
{
#ifndef SIMULATE_DEBUG
	ac_strm->audif_clk_status = audif_clk_status_read();
	if (ac_strm->audif_clk_status)
		return;

	AUD_LOG_INFO(AUD_CORE, "put process start cond ");
	if (ac_strm->strm_dir == STRM_DIR_PLAYBACK) {
		/* unmask ap interrupt */
		//audif_aempty_ap_mask_enable(true);
		ac_strm->work_on = true;
		audif_ap_schedule_work();
	} else {
		/* unmask ar interrupt */
		//audif_afull_ar_mask_enable(true);
		ac_strm->work_on = true;
		audif_ar_schedule_work();
	}
#endif
}

/************************************************************************
 * function: fk_acore_process_stop
 * description: config audio if to stop data transfer.
 * parameter:
 *		ac_strm : stream instance
 * return:
 ************************************************************************/
void fk_acore_process_stop(struct acore_stream *ac_strm)
{
	ac_strm->audif_clk_status = audif_clk_status_read();
	if (ac_strm->audif_clk_status)
		return;

	AUD_LOG_INFO(AUD_CORE, "put process stop cond ");
#ifndef SIMULATE_DEBUG
	if (ac_strm->strm_dir == STRM_DIR_PLAYBACK) {
		ac_strm->work_on = false;
		/* disable audio if ap fifo interrupt */
		audif_aempty_ap_mask_enable(false);

		/* flush ap fifo */
		audif_fifo_clr_ap_enable();
		audif_ap_fifo_write_unregister();

		fk_acore_playback_data_flush(ac_strm);
	} else {
		ac_strm->work_on = false;
		/* disable ar fifo interrupt and stop data thread */
		audif_afull_ar_mask_enable(false);

		/* disable ar fifo */
		audif_ad_fifo_enable_ar(false);
		audif_ar_fifo_read_unregister();

		fk_acore_capture_data_flush(ac_strm);
	}
#endif
}

/************************************************************************
 * function: fk_acore_playback_done
 * description: trigger callback function when one block buffer playback
 *		completed.
 * parameter:
 *		ac_strm : stream instance
 *		p_buf : data buffer info
 * return:
 ************************************************************************/
static void fk_acore_playback_done(struct acore_stream *ac_strm, struct buf_blk p_buf)
{
	struct rsp_payload payload = {0};
	struct buf_blk *bufinfo = (struct buf_blk *)&payload.payload.buf_info;

	payload.opCode = OP_PLAYBACK_COPY;
	bufinfo->phy_addr = (uint64_t)p_buf.phy_addr;
	bufinfo->size = p_buf.size;
	bufinfo->result = RSP_MSG_OK;

	/* update block number */
	atomic_inc(&ac_strm->rel_block_no);

	if (ac_strm->cb)
		ac_strm->cb(ac_strm->sess_id, ac_strm->priv, payload);
}

/************************************************************************
 * function: fk_acore_playback_buf_put
 * description: put buffer to data list
 * parameter:
 *		ac_strm : stream instance
 *		p_buf : data buffer info
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_playback_buf_put(struct acore_stream *ac_strm, struct buf_blk p_buf)
{
	int ret = EOK;
	struct data_list_node *data_node = NULL;
	int blk_num = 0;
	int blk_buf_bytes = 0;
	int i = 0;

	if (ac_strm->strm_dir != STRM_DIR_PLAYBACK) {
		AUD_LOG_ERR(AUD_CORE, "dir(%d) unexpect.", ac_strm->strm_dir);
		return -EINVAL;
	}

	blk_buf_bytes = ac_strm->smem.periods_size *
		fk_acore_get_bytes_per_sample(ac_strm);
	blk_num = p_buf.size / blk_buf_bytes;

	for (i = 0; i < blk_num; i++) {
		data_node = kzalloc(sizeof(struct data_list_node), GFP_KERNEL);
		if (!data_node) {
			AUD_LOG_ERR(AUD_CORE, "alloc data node failure.");
			return -ENOMEM;
		}

		/* fill data node info */
		data_node->p_buf.vir_addr = p_buf.vir_addr + (i * blk_buf_bytes);
		data_node->p_buf.phy_addr = p_buf.phy_addr + (i * blk_buf_bytes);
		data_node->p_buf.size = blk_buf_bytes;
		data_node->p_buf.result = 0;

		/* add data node to tail of list */
		list_add_tail(&data_node->data_list, &ac_strm->strm_data_list);

		/* update put block number */
		atomic_inc(&ac_strm->put_block_no);

		if ((atomic_read(&ac_strm->put_block_no) % 50) == 1)
			AUD_LOG_DBG(AUD_CORE, "blk_num(%d) size(%d) count(%d)",
				blk_num, p_buf.size, atomic_read(&ac_strm->put_block_no));
	}

	if (ac_strm->state == ACORE_STATE_PREPARE)
		fk_acore_playback_data_trigger(ac_strm);

	return ret;
}

/************************************************************************
 * function: fk_acore_playback_data_flush
 * description: flush buffer which in data list
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_playback_data_flush(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct list_head *ptr = NULL;
	struct list_head *next = NULL;
	struct data_list_node *data_node = NULL;

	AUD_LOG_INFO(AUD_CORE, "enter");

	/* release the remain data node */
	list_for_each_safe(ptr, next, &ac_strm->strm_data_list) {
		data_node = list_entry(ptr, struct data_list_node, data_list);
		if (data_node) {
			AUD_LOG_DBG(AUD_CORE, "free node:%llx size:%d",
				(uint64_t)data_node, data_node->p_buf.size);

			list_del(&data_node->data_list);
			kfree(data_node);
		}
	}

	AUD_LOG_DBG(AUD_CORE, "exit");

	return ret;
}

/************************************************************************
 * function: fk_acore_playback_data_trigger
 * description: get data buffer from data list. playback data by audio if.
 * parameter:
 *		priv : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
void fk_acore_playback_data_trigger(void *priv)
{
	struct acore_stream *ac_strm = (struct acore_stream *)priv;
	struct data_list_node *data_node = NULL;
	int ap_fifo_size = 0;
	int ap_put_num = 0;

	if (ac_strm == NULL) {
		AUD_LOG_INFO(AUD_CORE, "ac_strm is NULL");
		return;
	}

	while (ac_strm->work_on == true) {
		ap_fifo_size = AUDIF_FIFO_FULL_VALUE - audif_fifo_status_ap_l_get();
		AUD_LOG_DBG(AUD_CORE, "ap_fifo_size is %d", ap_fifo_size);
		ap_put_num = ap_fifo_size / ac_strm->smem.periods_size;

		while (ap_put_num > 0) {
			/* playback mute data if list empty */
			if (list_empty(&ac_strm->strm_data_list)) {
				AUD_LOG_INFO(AUD_CORE, "under run.");
				mdelay(1);
				/* TODO: send mute data by audio IF api.
				 * mute data no need trigger playback done.
				 * and exit when stop trigger.
				 */
			} else {
				data_node = list_first_entry(&ac_strm->strm_data_list,
					struct data_list_node, data_list);

				/* update current buffer info */
				ac_strm->cur_buf.phy_addr = data_node->p_buf.phy_addr;
				ac_strm->cur_buf.vir_addr = data_node->p_buf.vir_addr;
				ac_strm->cur_buf.size = data_node->p_buf.size;
				ac_strm->cur_buf.result = 0;

				/* send data by audio IF api */
				audif_ap_fifo_write_data(ac_strm->cur_buf.vir_addr,
					(uint64_t)ac_strm->cur_buf.phy_addr,
					ac_strm->cur_buf.size);

				list_del(&data_node->data_list);

				/* free data node resource */
				kfree(data_node);

				/* When the length is 0, it is considered as the start of the
				 * first block of data or an illegal data block, and there is
				 * no need to trigger playback done callback.
				 */
				if (ac_strm->cur_buf.size) {
					/* trigger write done to cb handle */
					fk_acore_playback_done(ac_strm, ac_strm->cur_buf);
				}
			}
			ap_put_num--;
		}
	}
}

/************************************************************************
 * function: fk_acore_capture_done
 * description: trigger callback function when one block buffer capture
 *		completed.
 * parameter:
 *		ac_strm : stream instance
 *		p_buf : buffer info
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
static void fk_acore_capture_done(struct acore_stream *ac_strm, struct buf_blk p_buf)
{
	struct rsp_payload payload = {0};
	struct buf_blk *bufinfo = (struct buf_blk *)&payload.payload.buf_info;

	payload.opCode = OP_CAPTURE_COPY;
	bufinfo->phy_addr = (uint64_t)p_buf.phy_addr;
	bufinfo->size = p_buf.size;
	bufinfo->result = RSP_MSG_OK;

	if (ac_strm->cb)
		ac_strm->cb(ac_strm->sess_id, ac_strm->priv, payload);

	atomic_inc(&ac_strm->rel_block_no);
	ac_strm->total_len += p_buf.size;
}

/************************************************************************
 * function: fk_acore_capture_buf_put
 * description: put empty block buffer to data list.
 * parameter:
 *		ac_strm : stream instance
 *		p_buf : buffer info
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_capture_buf_put(struct acore_stream *ac_strm, struct buf_blk p_buf)
{
	int ret = EOK;
	struct data_list_node *data_node = NULL;
	int blk_num = 0;
	int blk_buf_bytes = 0;
	int i = 0;

	if (ac_strm->strm_dir != STRM_DIR_CAPTURE) {
		AUD_LOG_ERR(AUD_CORE, "dir(%d) unexpect.",
			ac_strm->strm_dir);
		return -EINVAL;
	}

	blk_buf_bytes = ac_strm->smem.periods_size *
		fk_acore_get_bytes_per_sample(ac_strm);
	blk_num = p_buf.size / blk_buf_bytes;
	AUD_LOG_DBG_LIM(AUD_CORE, "blk_num(%d) size(%d) p_size(%d)",
		blk_num, p_buf.size, ac_strm->smem.periods_size);

	for (i = 0; i  < blk_num; i++) {
		data_node = kzalloc(sizeof(struct data_list_node), GFP_KERNEL);
		if (!data_node) {
			AUD_LOG_ERR(AUD_CORE, "alloc data node failure.");
			return -ENOMEM;
		}

		/* fill data node info */
		data_node->p_buf.vir_addr = p_buf.vir_addr + (i * blk_buf_bytes);
		data_node->p_buf.phy_addr = p_buf.phy_addr + (i * blk_buf_bytes);
		data_node->p_buf.size = blk_buf_bytes;
		data_node->p_buf.result = 0;

		/* add data node to tail of list */
		list_add_tail(&data_node->data_list, &ac_strm->strm_data_list);

		/* update put block number */
		atomic_inc(&ac_strm->put_block_no);
	}

	return ret;
}

/************************************************************************
 * function: fk_acore_capture_data_flush
 * description: flush buffer node which in data list
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_capture_data_flush(struct acore_stream *ac_strm)
{
	int ret = EOK;
	struct list_head *ptr = NULL;
	struct list_head *next = NULL;
	struct data_list_node *data_node = NULL;

	AUD_LOG_INFO_LIM(AUD_CORE, "enter");

	/* release the remain data node */
	list_for_each_safe(ptr, next, &ac_strm->strm_data_list) {
		data_node = list_entry(ptr, struct data_list_node, data_list);
		if (data_node) {
			list_del(&data_node->data_list);
			/* default len is 0 */
			data_node->p_buf.size = 0;

			AUD_LOG_INFO_LIM(AUD_CORE, "free node(%llx",
				(uint64_t)data_node);
			kfree(data_node);
		}
	}

	AUD_LOG_INFO_LIM(AUD_CORE, "exit");

	return ret;
}

/************************************************************************
 * function: fk_acore_capture_data_trigger
 * description: get data buffer from data list. capture data by audio if.
 * parameter:
 *		priv : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
void fk_acore_capture_data_trigger(void *priv)
{
	struct acore_stream *ac_strm = (struct acore_stream *)priv;
	struct data_list_node *data_node = NULL;
	struct buf_blk p_buf;
	int read_size = 0;
	int ap_fifo_size = 0;
	int ap_put_num = 0;

	while (ac_strm->work_on) {
		ap_fifo_size = audif_fifo_status_ar_l_get();
		AUD_LOG_DBG(AUD_CORE, "ar_fifo_size is %d", ap_fifo_size);
		ap_put_num = ap_fifo_size / ac_strm->smem.periods_size;
		while (ap_put_num) {
			if (list_empty(&ac_strm->strm_data_list)) {
				AUD_LOG_INFO_LIM(AUD_CORE, "over run. size(%d)", read_size);
				mdelay(1);
				break;
				/* TODO: flush audio IF fifo. */
			} else {
				/* get data node */
				data_node = list_first_entry(&ac_strm->strm_data_list,
						struct data_list_node, data_list);

				/* read data from audio IF fifo */
				read_size = audif_ar_fifo_read_data(data_node->p_buf.vir_addr,
					(uint64_t)data_node->p_buf.phy_addr, data_node->p_buf.size);
				if (read_size <= 0) {
					data_node->p_buf.size = 0;
					AUD_LOG_DBG_LIM(AUD_CORE, "read fail. size(%d)", read_size);
				} else {
					list_del(&data_node->data_list);
					data_node->p_buf.size = read_size;

					p_buf.phy_addr = data_node->p_buf.phy_addr;
					p_buf.vir_addr = data_node->p_buf.vir_addr;
					p_buf.size = data_node->p_buf.size;
					p_buf.result = 0;

					AUD_LOG_DBG_LIM(AUD_CORE, "free node(%llx)", (uint64_t)data_node);
					/* free data node resource */
					kfree(data_node);

					/* trigger read done to cb handle */
					fk_acore_capture_done(ac_strm, p_buf);
				}
				ap_put_num--;
			}
		}
	}
}

/************************************************************************
 * function: fk_acore_create_data_proc
 * description: create resource for data process
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_create_data_proc(struct acore_stream *ac_strm)
{
	int ret = EOK;

	AUD_LOG_INFO(AUD_CORE, "enter");

	INIT_LIST_HEAD(&ac_strm->strm_data_list);

	ac_strm->cur_buf.phy_addr = 0xFFFFFFFF;
	ac_strm->cur_buf.vir_addr = NULL;
	ac_strm->cur_buf.size = 0;

	return ret;
}

/************************************************************************
 * function: fk_acore_destroy_data_proc
 * description: destroy resource for data process
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
int fk_acore_destroy_data_proc(struct acore_stream *ac_strm)
{
	int ret = EOK;

	AUD_LOG_INFO(AUD_CORE, "enter");

	return ret;
}

