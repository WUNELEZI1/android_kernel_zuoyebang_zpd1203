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
#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>

#include "fk-audio-tool-core.h"
#include "fk-audio-cma.h"
#include "fk-audio-memlayout.h"
#include "../adsp/adsp_ssr_driver.h"
#include "../acore/fk-acore-utils.h"
#include "../asoc/fk-dai-be.h"
#include "audio-trace.h"

#define LOG_UART_MASK       (1 << (LOG_UART - 1))
#define LOG_USB_P_MASK      (1 << (LOG_USB_P - 1))
#define LOG_USB_C_MASK      (1 << (LOG_USB_C - 1))

struct audio_tool_pdata tool_pdata_g;
struct dump_blk dump_blk_g;

/************************************************************************
 * function: fk_audio_tool_pdata_get
 * description: get audio tool globle pdata
 * parameter:
 *		struct audio_tool_pdata *
 * return:
 *		struct audio_tool_pdata *
 ************************************************************************/
struct audio_tool_pdata *fk_audio_tool_pdata_get(void)
{
	struct audio_tool_pdata *pdata;

	pdata = &tool_pdata_g;
	return pdata;
}

/************************************************************************
 * function: fk_audio_tool_memory_list_check
 * description: check cal memory list & free dup cal memory
 * parameter:
 *		phy_addr : physical address of share memory
 *		size     : size of share memory
 * return:
 *		null
 ************************************************************************/
void fk_audio_tool_memory_list_check(dma_addr_t phy_addr,
	uint16_t size)
{
	struct cal_memory_list_node *memory_node = NULL;
	struct list_head *ptr, *next;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();

	mutex_lock(&pdata->memory_list_lock);
	if (list_empty(&pdata->cal_memory_list)) {
		mutex_unlock(&pdata->memory_list_lock);
		AUD_LOG_DBG(AUD_COMM, "none memory list");
		return;
	}

	/* get data node */
	list_for_each_safe(ptr, next, &pdata->cal_memory_list) {
		memory_node = list_entry(ptr, struct cal_memory_list_node, data_list);
		if ((memory_node->phy_addr == (uint64_t)phy_addr) &&
			(memory_node->size == size)) {
			if (memory_node->vir_addr) {
				AUD_LOG_DBG(AUD_COMM, "matched free memory, v_addr:0x%llx",
					memory_node->vir_addr);
				fk_acore_smem_free(audio_tool_misc.parent, memory_node->size,
					(void *)memory_node->vir_addr, memory_node->phy_addr);
				list_del(&memory_node->data_list);
				/* free data node resource */
				kfree(memory_node);
			}
		}
	}
	mutex_unlock(&pdata->memory_list_lock);
}

/************************************************************************
 * function: fk_audio_tool_smem_free
 * description: free share memory between ap and adsp
 * parameter:
 *		phy_addr : physical address of share memory
 * return:
 *		char * : virtual address of share memory
 ************************************************************************/
void fk_audio_tool_smem_free(struct acore_stream *ac_strm,
	struct xring_aud_msg *sc_msg)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct cal_param_info *param_info;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter.");
#endif
	atomic_set(&pdata->cal_wait, 0);
	wake_up(&pdata->cal_wait_que);
	param_info = (struct cal_param_info *)&sc_msg->data[0];

	AUD_LOG_DBG(AUD_COMM, "v_addr:0x%llx p_addr=0x%llx size %hu",
		(uint64_t)param_info->vir_addr, (uint64_t)param_info->phy_addr,
		param_info->size);

	fk_audio_tool_memory_list_check(param_info->phy_addr,
		param_info->size);
#ifdef ATRACE_MACRO
	AUD_TRACE("exit.");
#endif
}

/************************************************************************
 * function: fk_audio_tool_dump_read
 * description: put empty block buffer to dump data list.
 * parameter:
 *		buf : user buf
 *		len : len
 * return:
 *		size : success
 *		other : failure
 ************************************************************************/
int fk_audio_tool_dump_read(char __user *buf, size_t len)
{
	struct dump_data_list_node *data_node = NULL;
	struct packetheader *packetheader_t;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	int size, ret = 0;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter.");
#endif

	if (!buf) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user buf.");
		return -EFAULT;
	}

	mutex_lock(&pdata->dump_list_lock);
	if (list_empty(&pdata->dump_data_list)) {
		mutex_unlock(&pdata->dump_list_lock);
		return -1;
	}

	/* get data node */
	data_node = list_first_entry(&pdata->dump_data_list,
			struct dump_data_list_node, data_list);
	if (data_node) {
		AUD_LOG_DBG(AUD_COMM, "vir addr:0x%llx, len:%u", (uint64_t)data_node->p_buf.vir_addr,
			data_node->p_buf.size);

		size = data_node->p_buf.size;
		/* Dump data is read in blocks, and the size read by the user must be larger than the block.*/
		if (len < size) {
			AUD_LOG_ERR(AUD_COMM, "Invalid user buf.");
			mutex_unlock(&pdata->dump_list_lock);
			return -EFAULT;
		}

		packetheader_t = (struct packetheader *)data_node->p_buf.vir_addr;
		AUD_LOG_DBG(AUD_COMM, "flag:%d,packet_type:%d,packet_len:%d,packet_wr_num:%d,packet_send_num:%d.",
			packetheader_t->flag, packetheader_t->packet_type, packetheader_t->packet_len,
			packetheader_t->packet_wr_num, packetheader_t->packet_send_num);

		ret = copy_to_user(buf, data_node->p_buf.vir_addr, data_node->p_buf.size);
		if (ret) {
			AUD_LOG_DBG(AUD_COMM, "copy data to user fail.");
			mutex_unlock(&pdata->dump_list_lock);
			return -1;
		}

		list_del(&data_node->data_list);
		AUD_LOG_DBG(AUD_COMM, "free node(0x%llx)", (uint64_t)data_node);

		vfree(data_node->p_buf.vir_addr);
		/* free data node resource */
		kfree(data_node);

		mutex_unlock(&pdata->dump_list_lock);

#ifdef ATRACE_MACRO
		AUD_TRACE("exit.");
#endif

		return size;
	}
	mutex_unlock(&pdata->dump_list_lock);
	return -1;
}


/************************************************************************
 * function: fk_audio_tool_dump_copy_put
 * description: put empty block buffer to dump data list.
 * parameter:
 *		p_buf : dump buffer info
 * return:
 *		0 : success
 *		other : failure
 ************************************************************************/
void fk_audio_tool_dump_copy_put(struct dump_blk p_buf)
{
	struct dump_data_list_node *data_node = NULL;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct tool_drvdata *drvdata = fk_audio_tool_drvdata_get();

#ifdef ATRACE_MACRO
	AUD_TRACE("enter.");
#endif
	data_node = kzalloc(sizeof(struct dump_data_list_node), GFP_KERNEL);
	if (!data_node) {
		AUD_LOG_ERR(AUD_COMM, "alloc dump data node failure.");
		return;
	}

	/* fill dump data node info */
	data_node->p_buf.vir_addr = p_buf.vir_addr;
	data_node->p_buf.phy_addr = p_buf.phy_addr;
	data_node->p_buf.size = p_buf.size;

	/* add data node to tail of list */
	mutex_lock(&pdata->dump_list_lock);
	list_add_tail(&data_node->data_list, &pdata->dump_data_list);
	mutex_unlock(&pdata->dump_list_lock);

	/*todo kill fasync */
	if (drvdata->async_queue)
		kill_fasync(&drvdata->async_queue, SIGIO, POLL_IN);

#ifdef ATRACE_MACRO
	AUD_TRACE("exit.");
#endif
}

/************************************************************************
 * function: fk_audio_tool_message_send
 * description: send message to ipc.
 * parameter:
 *		msg : message content
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_message_send(struct xring_aud_msg *msg)
{
	int ret = 0;

	AUD_LOG_DBG_LIM(AUD_COMM, "scene(0x%x) ins(0x%x) cmd(0x%x) func(0x%x) ",
		msg->header.scene_id, msg->header.ins_id,
		msg->header.cmd_id, msg->header.func_id);

	if (fk_adsp_vote_register(AUDIO_TOOL_CONN_SESS) < 0)
		AUD_LOG_DBG(AUD_COMM, "audio tool dis power up.");

	/* FK-AUDIO_MAILBOX begin */
	msg->header.msg_source = AP_AUDIO_TOOL;
	/* FK-AUDIO_MAILBOX end */
	ret = fk_audio_ipc_send_async(AP_AUDIO_TOOL, (unsigned int *)msg,
			msg->header.len, NULL, NULL);

	return ret;
}

/************************************************************************
 * function: fk_audio_tool_get_stream_id
 * description: get stream id by pcm id.
 * parameter:
 *		pcm_id : pcm id
 * return:
 *		stream_id: stream id
 ************************************************************************/
static int fk_audio_tool_get_stream_id(uint32_t pcm_id)
{
	uint32_t stream_id;

	switch (pcm_id) {
	case 0:
		stream_id = XR_MEDIA0;
		break;
	case 1:
		stream_id = XR_MEDIA1;
		break;
	case 2:
		stream_id = XR_MEDIA2;
		break;
	case 3:
		stream_id = XR_MEDIA3;
		break;
	case 4:
		stream_id = XR_MEDIA4;
		break;
	case 5:
		stream_id = XR_MEDIA5;
		break;
	case 6:
		stream_id = XR_VOICE;
		break;
	case 8:
		stream_id = XR_MEDIA6;
		break;
	case 10:
		stream_id = XR_LSM;
		break;
	case 11:
		stream_id = XR_DIRECT;
		break;
	case 12:
		stream_id = XR_SPATIAL;
		break;
	case 13:
		stream_id = XR_MMAP;
		break;
	case 14:
		stream_id = XR_KARAOKE;
		break;
	case 15:
		stream_id = XR_INCALL;
		break;
	default:
		stream_id = 0xffff;
		AUD_LOG_ERR(AUD_COMM, "false pcm id.");
	}

	AUD_LOG_DBG(AUD_COMM, "pcm id:%d, stream id:%d.", pcm_id, stream_id);
	return stream_id;
}

/************************************************************************
 * function: fk_audio_tool_get_session_id
 * description: get cal session id.
 * parameter:
 *		stream_id : stream id
 *		port_id   : port id
 * return:
 *		session_id: session id
 ************************************************************************/
int fk_audio_tool_get_session_id(uint32_t stream_id, int port_id)
{
	int scene_type_id = 0;
	int session_id = -1;
	int stream = 0;

	switch (port_id) {
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
	case FK_XR_MI2S0_RX:
	case FK_XR_AUDIO_USB_RX:
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
		/* playback */
		stream = SNDRV_PCM_STREAM_PLAYBACK;
		break;
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
	case FK_XR_VAD_I2S_TX:
	case FK_XR_AUDIO_USB_TX:
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
		/* capture */
		stream = SNDRV_PCM_STREAM_CAPTURE;
		break;
	}

	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		switch (stream_id) {
		case XR_MEDIA0:
		case XR_MEDIA1:
		case XR_MEDIA2:
		case XR_MEDIA3:
		case XR_LSM:
			scene_type_id = SCENE_PCM_PLAYBACK;
			break;
		case XR_MEDIA5:
			scene_type_id = SCENE_COMPR_PLAYBACK;
			break;
		case XR_MEDIA6:
			scene_type_id = SCENE_VOIP_PLAYBACK;
			break;
		case XR_MEDIA7:
			scene_type_id = SCENE_HAPTIC;
			break;
		case XR_VOICE:
			scene_type_id = SCENE_VOICE_PLAYBACK;
			break;
		case XR_MMAP:
			scene_type_id = SCENE_MMAP_PLAYBACK;
			break;
		case XR_SPATIAL:
			scene_type_id = SCENE_SPATIAL;
			break;
		case XR_KARAOKE:
			scene_type_id = SCENE_KARAOKE_PLAYBACK;
			break;
		}
	} else if (stream == SNDRV_PCM_STREAM_CAPTURE) {
		switch (stream_id) {
		case XR_MEDIA0:
		case XR_MEDIA1:
		case XR_MEDIA2:
		case XR_MEDIA3:
		case XR_LSM:
			scene_type_id = SCENE_PCM_CAPTURE;
			break;
		case XR_MEDIA4:
			scene_type_id = SCENE_COMPR_CAPTURE;
			break;
		case XR_MEDIA6:
			scene_type_id = SCENE_VOIP_CAPTURE;
			break;
		case XR_VOICE:
			scene_type_id = SCENE_VOICE_CAPTURE;
			break;
		case XR_MMAP:
			scene_type_id = SCENE_MMAP_CAPTURE;
			break;
		case XR_KARAOKE:
			scene_type_id = SCENE_KARAOKE_CAPTURE;
			break;
		}
	}

	session_id = (scene_type_id << 8)|(stream_id);
	AUD_LOG_INFO(AUD_COMM, "session=0x%x", session_id);

	return session_id;
}

static void fk_audio_tool_tuning_cal_set(uint32_t stream_id, int port_id)
{
	uint16_t session_id = fk_audio_tool_get_session_id(stream_id, port_id);
	struct acore_stream *ac_strm = fk_acore_stream_get(session_id);

	if (ac_strm == NULL) {
		AUD_LOG_DBG(AUD_COMM, "none sence.");
		return;
	}

	AUD_LOG_INFO(AUD_COMM, "state: %d", ac_strm->state);
	if (ac_strm->state == ACORE_STATE_RUN)
		fk_audio_tool_stream_set_cal(ac_strm);
}

/************************************************************************
 * function: fk_audio_tool_cal_store
 * description: store cal param
 * parameter:
 *		cal_data	: cal data
 *		pipeline_t	: address&len
 *		cal_id		: cal id
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_audio_tool_cal_store(struct cal_param_info *cal_data,
	struct _pipeline_context_t pipeline_t, int cal_id)
{
	int len;
	void *cal_vaddr;
	void *user_data_addr;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct cal_memory_list_node *memory_list_node = NULL;
	unsigned long jiffies_1, jiffies_diff, m_diff;

	/* todo copy cal data from user*/
	len = pipeline_t.pipelineLen[cal_id];
	user_data_addr = pipeline_t.pPipelineData[cal_id];
	AUD_LOG_DBG(AUD_COMM, "user data addr: 0x%llx, len: %hu",
		(uint64_t)user_data_addr, len);
	if (len) {
		if (!user_data_addr) {
			AUD_LOG_INFO(AUD_COMM, "Invalid user addr: 0x%llx",
				(uint64_t)user_data_addr);
			return -1;
		}

		if (!access_ok(user_data_addr, len)) {
			AUD_LOG_ERR(AUD_COMM, "Invalid user addr.");
			return -1;
		}

		memory_list_node = kzalloc(sizeof(struct cal_memory_list_node), GFP_KERNEL);
		if (!memory_list_node) {
			AUD_LOG_ERR(AUD_COMM, "alloc memory node failure.");
			return -1;
		}

		cal_data->size = len;
		jiffies_1 = jiffies;
		cal_vaddr = fk_acore_smem_alloc(audio_tool_misc.parent,	&cal_data->phy_addr,
			cal_data->size);
		jiffies_diff = jiffies-jiffies_1;
		m_diff = jiffies_to_msecs(jiffies_diff);
		if (m_diff > 100)
			AUD_LOG_INFO(AUD_COMM, "alloc cal vaddr timeout, m_diff:%lums.", m_diff);
		if (!cal_vaddr) {
			AUD_LOG_ERR(AUD_COMM, "alloc cal vaddr failure.");
			kfree(memory_list_node);
			return -1;
		}

		memory_list_node->phy_addr = cal_data->phy_addr;
		memory_list_node->vir_addr = (uint64_t)cal_vaddr;
		memory_list_node->size = cal_data->size;

		cal_data->vir_addr = (uint64_t)cal_vaddr;
		if (copy_from_user(cal_vaddr, user_data_addr, len)) {
			fk_acore_smem_free(audio_tool_misc.parent, memory_list_node->size,
				(void *)memory_list_node->vir_addr, memory_list_node->phy_addr);
			kfree(memory_list_node);
			AUD_LOG_ERR(AUD_COMM, "Could not copy cal data from user.");
			return -1;
		}

		mutex_lock(&pdata->memory_list_lock);
		/* add memory node to tail of list */
		list_add_tail(&memory_list_node->data_list, &pdata->cal_memory_list);
		mutex_unlock(&pdata->memory_list_lock);
	}

	return 0;
}

/************************************************************************
 * function: fk_audio_tool_duplicate_cal_del
 * description: del duplicate cal
 * parameter:
 *		node	: cal list node
 * return:
 ************************************************************************/
static void fk_audio_tool_duplicate_cal_del(struct cal_data_list_node *node)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct cal_data_list_node *cal_node = NULL;
	struct list_head *ptr, *next;
	int cal_id;

	AUD_LOG_INFO(AUD_COMM, "enter");
	mutex_lock(&pdata->cal_list_lock);
	if (list_empty(&pdata->cal_data_list)) {
		mutex_unlock(&pdata->cal_list_lock);
		AUD_LOG_DBG(AUD_COMM, "none cal list");
		return;
	}

	/* get data node */
	list_for_each_safe(ptr, next, &pdata->cal_data_list) {
		cal_node = list_entry(ptr, struct cal_data_list_node, data_list);

		if ((cal_node->stream == node->stream) &&
			(cal_node->port == node->port)) {
			for (cal_id = 0; cal_id < CAL_ID_MAX; cal_id++) {
				if (cal_node->cal_data[cal_id].phy_addr &&
						cal_node->cal_data[cal_id].size)
					fk_audio_tool_memory_list_check(cal_node->cal_data[cal_id].phy_addr,
						cal_node->cal_data[cal_id].size);
			}
			AUD_LOG_INFO(AUD_COMM, "duplicate cal");
			list_del(&cal_node->data_list);
			/* free data node resource */
			kfree(cal_node);
		}
	}

	mutex_unlock(&pdata->cal_list_lock);
	AUD_LOG_INFO(AUD_COMM, "exit");
}

/************************************************************************
 * function: fk_audio_tool_set_cal
 * description: set cal param
 * parameter:
 *		tool_type	: log dump type
 *		data_size	: data_size
 *		data		: data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_set_cal(int32_t tool_type,
	size_t data_size, void *data)
{
	struct cal_data_list_node *cal_data_node = NULL;
	struct SneceContent cal_data_t;
	struct _pipeline_context_t pipeline_t = {0};
	int cal_id, ret = 0;
	struct audio_tool_pdata *pdata;

	if (!data) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	if (data_size != sizeof(struct SneceContent)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size:%zu.", data_size);
		return -EFAULT;
	}
#ifdef ATRACE_MACRO
	AUD_TRACE("enter.");
#endif
	pdata = fk_audio_tool_pdata_get();
	/* update cal info */
	if (copy_from_user(&cal_data_t, data, data_size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		return -EFAULT;
	}

	/* get stream id by pcm id of hal set info */
	cal_data_t.param.stream = fk_audio_tool_get_stream_id(cal_data_t.param.stream);
	if (cal_data_t.param.stream > XR_INCALL)
		return -EFAULT;

	AUD_LOG_INFO(AUD_COMM, "stream:0x%x,port:%d.", cal_data_t.param.stream,
		cal_data_t.param.port);

	if (!cal_data_t.piplelineAddr) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	if (copy_from_user(&pipeline_t, cal_data_t.piplelineAddr,
			sizeof(struct _pipeline_context_t))) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy addr info from user.");
		return -EFAULT;
	}

	cal_data_node = kzalloc(sizeof(struct cal_data_list_node), GFP_KERNEL);
	if (!cal_data_node) {
		AUD_LOG_DBG(AUD_COMM, "alloc cal data node failure.");
		return -ENOMEM;
	}

	cal_data_node->tool_type = tool_type;
	cal_data_node->stream = cal_data_t.param.stream;
	cal_data_node->port = cal_data_t.param.port;
	/* fill cal data node info */
	for (cal_id = 0; cal_id < CAL_ID_MAX; cal_id++) {
		memcpy(&cal_data_node->cal_data[cal_id].param,
			&cal_data_t.param, sizeof(struct sceneParam));
	}
	/* del duplicate cal */
	fk_audio_tool_duplicate_cal_del(cal_data_node);
	/* alloc cal memory */
	for (cal_id = 0; cal_id < CAL_ID_MAX; cal_id++) {
		ret = fk_audio_tool_cal_store(&cal_data_node->cal_data[cal_id],
			pipeline_t, cal_id);
		AUD_LOG_DBG(AUD_COMM, "phy addr: 0x%llx, vir addr: 0x%llx, size: %hu, cal_type:%d",
				cal_data_node->cal_data[cal_id].phy_addr, cal_data_node->cal_data[cal_id].vir_addr,
				cal_data_node->cal_data[cal_id].size, cal_data_node->tool_type);
	}

	mutex_lock(&pdata->cal_list_lock);
	/* add data node to tail of list */
	list_add_tail(&cal_data_node->data_list, &pdata->cal_data_list);
	mutex_unlock(&pdata->cal_list_lock);
	/* set tuning cal*/
	fk_audio_tool_tuning_cal_set(cal_data_t.param.stream, cal_data_t.param.port);

#ifdef ATRACE_MACRO
	AUD_TRACE("exit.");
#endif
	return ret;
}

/************************************************************************
 * function: fk_audio_vad_cal_info_send
 * description: vad cal info fill & send
 * parameter:
 *		ac_strm: ac data
 *		cal_info：cal param
 *		tool_type：cal type
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_audio_vad_cal_info_send(struct acore_stream *ac_strm,
	struct cal_param_info *cal_info, int tool_type)
{
	struct xring_aud_msg cal_info_msg = {0};
	struct cal_param_info *param_info = NULL;
	int ret = -1;

	/* fill message */
	cal_info_msg.header.scene_id = ac_strm->adsp_sc_t;
	cal_info_msg.header.ins_id = ac_strm->adsp_ins_id;
	cal_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct cal_param_info);
	cal_info_msg.header.result = 0;

	switch (tool_type) {
	case TOOL_TYPE_PIPE_PARAM:
	case TOOL_TYPE_DYN_PARAM:
		cal_info_msg.header.cmd_id = ADSP_PARAM_CTRL;
		cal_info_msg.header.func_id = ADSP_PARAM_CTRL_AUDIO_ALGO;
		break;
	case TOOL_TYPE_AUDIF_PARAM:
		cal_info_msg.header.cmd_id = ADSP_HW_MODULE_CTRL;
		cal_info_msg.header.func_id = ADSP_SET_AUDIOIF_MODULE;
		break;
	default:
		AUD_LOG_DBG(AUD_COMM, "f tool type:%d", tool_type);
		break;
	}

	param_info = (struct cal_param_info *)&cal_info_msg.data[0];

	memcpy(param_info, cal_info, sizeof(struct cal_param_info));
	param_info->param.port = fk_acore_port_id_get(ac_strm->port_id);

	AUD_LOG_INFO(AUD_COMM, "sence:%d,port%d,addr:0x%llx,vir addr: 0x%llx,size:%hu",
		param_info->param.stream, param_info->param.port,
		(uint64_t)param_info->phy_addr, (uint64_t)param_info->vir_addr,
		param_info->size);

	if (param_info->vir_addr) {
		/* send pause message */
		ret = fk_acore_message_send((struct xring_aud_msg *)&cal_info_msg);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_COMM, "send cal param info message f.");
			return ret;
		}
	}

	return ret;
}

/************************************************************************
 * function: fk_audio_stream_cal_info_send
 * description: cal info fill & send
 * parameter:
 *		ac_strm: ac data
 *		cal_info：cal param
 *		tool_type：cal type
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
static int fk_audio_stream_cal_info_send(struct acore_stream *ac_strm,
	struct cal_param_info *cal_info, int tool_type)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct xring_aud_msg cal_info_msg = {0};
	struct cal_param_info *param_info = NULL;
	int pcm_id, rc, ret = -1;

	pcm_id = (ac_strm->sess_id) & 0xFF;

	if ((cal_info->param.stream == pcm_id) &&
		(cal_info->param.stream == XR_LSM)) {
		ret = fk_audio_vad_cal_info_send(ac_strm, cal_info, tool_type);
		return ret;
	}

	if ((cal_info->param.stream == pcm_id) &&
		(cal_info->param.port == ac_strm->port_id)) {
		/* fill message */
		cal_info_msg.header.scene_id = ac_strm->adsp_sc_t;
		cal_info_msg.header.ins_id = ac_strm->adsp_ins_id;
		cal_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
			sizeof(struct cal_param_info);
		cal_info_msg.header.result = 0;

		switch (tool_type) {
		case TOOL_TYPE_PIPE_PARAM:
		case TOOL_TYPE_DYN_PARAM:
			cal_info_msg.header.cmd_id = ADSP_PARAM_CTRL;
			cal_info_msg.header.func_id = ADSP_PARAM_CTRL_AUDIO_ALGO;
			break;
		case TOOL_TYPE_AUDIF_PARAM:
			cal_info_msg.header.cmd_id = ADSP_HW_MODULE_CTRL;
			cal_info_msg.header.func_id = ADSP_SET_AUDIOIF_MODULE;
			break;
		default:
			AUD_LOG_DBG(AUD_COMM, "f tool type:%d", tool_type);
			break;
		}

		if (cal_info->param.stream == XR_VOICE ||
				cal_info->param.stream == XR_MEDIA6)
			cal_info_msg.header.func_id = ADSP_PARAM_CTRL_VOICE_ALGO;

		param_info = (struct cal_param_info *)&cal_info_msg.data[0];

		memcpy(param_info, cal_info, sizeof(struct cal_param_info));
		param_info->param.port = fk_acore_port_id_get(ac_strm->port_id);

		AUD_LOG_DBG(AUD_COMM, "sence:%d,port%d,addr:0x%llx,vir addr: 0x%llx,size:%hu",
			param_info->param.stream, param_info->param.port,
			(uint64_t)param_info->phy_addr, (uint64_t)param_info->vir_addr,
			param_info->size);

		if (param_info->vir_addr) {
			/* send pause message */
			ret = fk_acore_message_send((struct xring_aud_msg *)&cal_info_msg);
			if (ret < 0) {
				AUD_LOG_ERR(AUD_COMM, "send cal param info message f.");
				return ret;
			}

			atomic_set(&pdata->cal_wait, 1);
			rc = wait_event_timeout(pdata->cal_wait_que,
				!atomic_read(&pdata->cal_wait), CAL_WAIT_TIMEOUT);
			if (!rc && atomic_read(&pdata->cal_wait)) {
				AUD_LOG_INFO(AUD_COMM, "sess:0x%x,pord:%d,type:%d,dsp copy cal timeout.",
					ac_strm->sess_id, ac_strm->port_id, tool_type);
			}
		}
	}
	return ret;
}

/************************************************************************
 * function: fk_audio_tool_stream_set_cal
 * description: stream set cal param
 * parameter:
 *		ac_strm	: ac data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_stream_set_cal(struct acore_stream *ac_strm)
{
	struct cal_data_list_node *cal_node = NULL;
	struct list_head *ptr, *next;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	int cal_id, tool_type, ret = 0;

	if (!ac_strm) {
		AUD_LOG_ERR(AUD_COMM, "Invalid acore data.");
		return -EFAULT;
	}

	mutex_lock(&pdata->cal_list_lock);
	if (list_empty(&pdata->cal_data_list)) {
		mutex_unlock(&pdata->cal_list_lock);
		AUD_LOG_DBG(AUD_COMM, "none cal list");
		return -1;
	}

	/* get data node */
	list_for_each_safe(ptr, next, &pdata->cal_data_list) {
		cal_node = list_entry(ptr, struct cal_data_list_node, data_list);

		tool_type = cal_node->tool_type;
		if (tool_type == TOOL_TYPE_PIPE_PARAM) {
			for (cal_id = 0; cal_id < CAL_ID_MAX; cal_id++) {
				ret = fk_audio_stream_cal_info_send(ac_strm,
					&cal_node->cal_data[cal_id],
					TOOL_TYPE_PIPE_PARAM);
				if (!ret)
					cal_node->send_flag++;
			}
		} else if (tool_type == TOOL_TYPE_AUDIF_PARAM) {
			ret = fk_audio_stream_cal_info_send(ac_strm,
				&cal_node->cal_data[CAL_STREAM],
				TOOL_TYPE_AUDIF_PARAM);
			if (!ret)
				cal_node->send_flag++;
		} else if (tool_type == TOOL_TYPE_DYN_PARAM) {
			for (cal_id = 0; cal_id < CAL_ID_MAX; cal_id++) {
				ret = fk_audio_stream_cal_info_send(ac_strm,
					&cal_node->cal_data[cal_id],
					TOOL_TYPE_DYN_PARAM);
				if (!ret)
					cal_node->send_flag++;
			}
		}

		if (cal_node->send_flag) {
			list_del(&cal_node->data_list);
			/* free data node resource */
			kfree(cal_node);
			AUD_LOG_INFO(AUD_COMM, "sess id:0x%x,ac pord:%d send cal suc.",
				ac_strm->sess_id, ac_strm->port_id);
		}
	}

	mutex_unlock(&pdata->cal_list_lock);
	return ret;
}

/************************************************************************
 * function: fk_audio_tool_set_log_param
 * description: set dsp log dump param
 * parameter:
 *		tool_type	: log dump type
 *		data_size	: data_size
 *		data		: data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_set_log_param(int32_t tool_type,
	size_t data_size, void *data)
{
	struct xring_aud_msg log_dump_info_msg = {0};
	struct dsp_log_dump_param_info *dsp_log_info = NULL;
	struct audio_tool_pdata *pdata;
	int ret = 0;

	if (!data) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	pdata = fk_audio_tool_pdata_get();

	/* fill message */
	log_dump_info_msg.header.scene_id = ADSP_SCENE_NONE;
	log_dump_info_msg.header.ins_id = 0;
	log_dump_info_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	log_dump_info_msg.header.func_id = ADSP_DEBUG_CTRL_DSP_LOG;
	log_dump_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct dsp_log_dump_param_info);
	log_dump_info_msg.header.result = 0;

	/* update log info */
	dsp_log_info = (struct dsp_log_dump_param_info *)&log_dump_info_msg.data[0];

	if (data_size != sizeof(struct dsp_log_dump_param_user)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size:%zu.", data_size);
		return -EFAULT;
	}

	if (copy_from_user(&dsp_log_info->param, (struct dsp_log_dump_param_user *)data,
			data_size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		return -EFAULT;
	}

	if ((dsp_log_info->param.log_enable != 1) ||
		(dsp_log_info->param.log_level > DSP_LOG_LEVEL_MAX) ||
		(dsp_log_info->param.log_port > DSP_LOG_PORT_MAX)) {
		AUD_LOG_ERR(AUD_COMM, "invalid params.");
		return -EFAULT;
	}

	pdata->log_blk_info.size = DSP_LOG_DUMP_MEM_SIZE;
	pdata->log_blk_info.vir_addr = fk_get_audio_static_mem(LOG_MEM,
		(uint32_t *)&pdata->log_blk_info.phy_addr, pdata->log_blk_info.size);
	dsp_log_info->blk_size = DSP_LOG_DUMP_BLK_SIZE;
	dsp_log_info->blk_num = DSP_LOG_DUMP_BLK_NUM;
	dsp_log_info->phy_addr = pdata->log_blk_info.phy_addr;
	AUD_LOG_INFO(AUD_COMM, "level%d,port%d,enable:%d,phyaddr:0x%llx,viraddr：0x%llx",
		dsp_log_info->param.log_level, dsp_log_info->param.log_port,
		dsp_log_info->param.log_enable, (uint64_t)dsp_log_info->phy_addr,
		(uint64_t)pdata->log_blk_info.vir_addr);

	/* send pause message */
	ret = fk_audio_tool_message_send((struct xring_aud_msg *)&log_dump_info_msg);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
		return ret;
	}

	pdata->log_blk_info.count = 0;
	return ret;
}

/************************************************************************
 * function: fk_audio_tool_set_log_dump
 * description: dsp log dump
 * parameter:
 *		tool_msg	: tool msg
 ************************************************************************/
static void fk_audio_tool_dsp_log_dump(struct xring_aud_msg *tool_msg)
{
	struct dump_info *log_info;
	struct dump_blk dump_info = {0};
	int cnt = 0;
	struct packetheader *packetheader_t;
	struct audio_tool_pdata *pdata;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter.");
#endif

	pdata = fk_audio_tool_pdata_get();
	log_info = (struct dump_info *)&tool_msg->data[0];

#ifndef AUDIO_TOOL_DEBUG
	cnt = pdata->log_blk_info.count;

	dump_info.size = log_info->len;
	dump_info.vir_addr = vmalloc(dump_info.size);
	if (!dump_info.vir_addr) {
		AUD_LOG_ERR(AUD_COMM, "alloc vir addr failure.");
		return;
	}

	mutex_lock(&pdata->mem_lock);
	memcpy(dump_info.vir_addr, pdata->log_blk_info.vir_addr + cnt * DSP_LOG_DUMP_BLK_SIZE,
		dump_info.size);
	mutex_unlock(&pdata->mem_lock);

	packetheader_t = (struct packetheader *)dump_info.vir_addr;
	AUD_LOG_DBG(AUD_COMM, "packet_send_num:%d,addr:0x%x,packetlen:%d,blk count:%d,infolen:%d.",
		packetheader_t->packet_send_num, log_info->phy_addr,
		packetheader_t->packet_len, pdata->log_blk_info.count, dump_info.size);

	pdata->log_blk_info.count++;
	if (pdata->log_blk_info.count >= DSP_LOG_DUMP_BLK_NUM)
		pdata->log_blk_info.count = 0;

	fk_audio_tool_dump_copy_put(dump_info);
#ifdef ATRACE_MACRO
	AUD_TRACE("exit.");
#endif
#else
	cnt = pdata->log_blk_info.count;
	dump_blk_g.vir_addr = pdata->log_blk_info.vir_addr + cnt * DSP_LOG_DUMP_BLK_SIZE;
	pdata->log_blk_info.count++;
	if (pdata->log_blk_info.count >= DSP_LOG_DUMP_BLK_NUM)
		pdata->log_blk_info.count = 0;

	AUD_LOG_DBG(AUD_COMM, "blk count:%d", pdata->log_blk_info.count);
	dump_blk_g.size = log_info->len;
	AUD_LOG_DBG(AUD_COMM, "phy_addr:0x%llx, size:%d", (uint64_t)log_info->phy_addr,
		log_info->len);
#endif

}

/************************************************************************
 * function: fk_audio_tool_set_pcm_param
 * description: set dsp pcm dump param
 * parameter:
 *		tool_type	: pcm dump type
 *		data_size	: data_size
 *		data		: data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_set_pcm_param(int32_t tool_type,
	size_t data_size, void *data)
{
	struct xring_aud_msg pcm_dump_info_msg = {0};
	struct dsp_pcm_dump_param_info *dsp_pcm_info = NULL;
	struct audio_tool_pdata *pdata;
	int ret = 0;

	if (!data) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	pdata = fk_audio_tool_pdata_get();

	/* fill message */
	pcm_dump_info_msg.header.scene_id = ADSP_SCENE_NONE;
	pcm_dump_info_msg.header.ins_id = 0;
	pcm_dump_info_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	pcm_dump_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct dsp_pcm_dump_param_info);
	pcm_dump_info_msg.header.result = 0;

	/* fill pcm info */
	dsp_pcm_info = (struct dsp_pcm_dump_param_info *)&pcm_dump_info_msg.data[0];

	if (data_size != sizeof(struct dsp_pcm_dump_param_user)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size:%zu.", data_size);
		return -EFAULT;
	}

	if (copy_from_user(&dsp_pcm_info->param, (struct dsp_pcm_dump_param_user *)data, data_size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		ret = -EFAULT;
	}
	//memcpy(dsp_pcm_info, (struct dsp_pcm_dump_param_info *)data, data_size);
	dsp_pcm_info->blk_size = DSP_PCM_DUMP_BLK_SIZE;
	dsp_pcm_info->blk_num = DSP_PCM_DUMP_BLK_NUM;

	if ((dsp_pcm_info->param.point1 || dsp_pcm_info->param.point2 ||
		dsp_pcm_info->param.point3) && (!pdata->pcm_blk_malloc_flag)) {
		pcm_dump_info_msg.header.func_id = ADSP_DEBUG_CTRL_PCM_DUMP_OPEN;
		pdata->pcm_blk_info.vir_addr = fk_acore_smem_alloc(audio_tool_misc.parent,
			&pdata->pcm_blk_info.phy_addr,
			DSP_PCM_DUMP_MEM_SIZE);
		pdata->pcm_blk_malloc_flag = 1;
		pdata->pcm_blk_info.count = 0;
		dsp_pcm_info->phy_addr = pdata->pcm_blk_info.phy_addr;

		ret = fk_audio_tool_message_send((struct xring_aud_msg *)&pcm_dump_info_msg);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
			return ret;
		}
	} else if ((!(dsp_pcm_info->param.point1 || dsp_pcm_info->param.point2 ||
		dsp_pcm_info->param.point3)) && pdata->pcm_blk_malloc_flag) {
		pcm_dump_info_msg.header.func_id = ADSP_DEBUG_CTRL_PCM_DUMP_CLOSE;
		if (pdata->pcm_blk_info.vir_addr) {
			mutex_lock(&pdata->mem_lock);
			fk_acore_smem_free(audio_tool_misc.parent, DSP_PCM_DUMP_MEM_SIZE,
				pdata->pcm_blk_info.vir_addr, pdata->pcm_blk_info.phy_addr);
			pdata->pcm_blk_info.vir_addr = NULL;
			pdata->pcm_blk_info.phy_addr = 0;
			pdata->pcm_blk_malloc_flag = 0;
			pdata->pcm_blk_info.count = 0;
			mutex_unlock(&pdata->mem_lock);
		}
		dsp_pcm_info->phy_addr = pdata->pcm_blk_info.phy_addr;

		ret = fk_audio_tool_message_send((struct xring_aud_msg *)&pcm_dump_info_msg);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
			return ret;
		}
	}

	AUD_LOG_INFO(AUD_COMM, "point1:%x point2:%x point3:%x phy_addr:0x%llx size:%d",
		dsp_pcm_info->param.point1, dsp_pcm_info->param.point2,
		dsp_pcm_info->param.point3, (uint64_t)dsp_pcm_info->phy_addr,
		dsp_pcm_info->blk_size);

	return ret;
}

/************************************************************************
 * function: fk_audio_tool_dsp_pcm_dump
 * description: dsp pcm dump
 * parameter:
 *		tool_msg	: tool msg
 ************************************************************************/
static void fk_audio_tool_dsp_pcm_dump(struct xring_aud_msg *tool_msg)
{
	struct dump_info *pcm_info;
	struct dump_blk dump_info = {0};
	int cnt = 0;
	struct packetheader *packetheader_t;
	struct audio_tool_pdata *pdata;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter.");
#endif

	pdata = fk_audio_tool_pdata_get();
	pcm_info = (struct dump_info *)&tool_msg->data[0];
	cnt = pdata->pcm_blk_info.count;

#ifndef AUDIO_TOOL_DEBUG
	dump_info.size = pcm_info->len;
	dump_info.vir_addr = vmalloc(dump_info.size);
	if (!dump_info.vir_addr) {
		AUD_LOG_ERR(AUD_COMM, "alloc vir addr failure.");
		return;
	}

	mutex_lock(&pdata->mem_lock);
	if (!pdata->pcm_blk_info.vir_addr) {
		mutex_unlock(&pdata->mem_lock);
		return;
	}
	memcpy(dump_info.vir_addr, pdata->pcm_blk_info.vir_addr + cnt * DSP_PCM_DUMP_BLK_SIZE,
		dump_info.size);
	mutex_unlock(&pdata->mem_lock);

	packetheader_t = (struct packetheader *)dump_info.vir_addr;
	AUD_LOG_DBG(AUD_COMM, "packet_send_num:%d,addr:0x%x,packetlen:%d,blk count:%d,infolen:%d.",
		packetheader_t->packet_send_num, pcm_info->phy_addr,
		packetheader_t->packet_len, pdata->pcm_blk_info.count, dump_info.size);
	pdata->pcm_blk_info.count++;
	if (pdata->pcm_blk_info.count >= DSP_PCM_DUMP_BLK_NUM)
		pdata->pcm_blk_info.count = 0;

	fk_audio_tool_dump_copy_put(dump_info);
#ifdef ATRACE_MACRO
	AUD_TRACE("exit.");
#endif
#else
	cnt = pdata->pcm_blk_info.count;
	dump_blk_g.vir_addr = pdata->pcm_blk_info.vir_addr + cnt * DSP_PCM_DUMP_BLK_SIZE;
	pdata->pcm_blk_info.count++;
	if (pdata->pcm_blk_info.count >= DSP_PCM_DUMP_BLK_NUM)
		pdata->pcm_blk_info.count = 0;

	AUD_LOG_DBG(AUD_COMM, "blk count:%d", pdata->pcm_blk_info.count);
	dump_blk_g.size = pcm_info->len;
	AUD_LOG_DBG(AUD_COMM, "phy_addr:0x%llx, vir_addr: 0x%llx, size:%zu",
		(uint64_t)pcm_info->phy_addr, (uint64_t)dump_blk_g.vir_addr,
		pcm_info->len);
#endif
}

/************************************************************************
 * function: fk_audio_tool_reg_value_read
 * description: dsp reg value read
 * parameter:
 *		tool_type	: log dump type
 *		data_size	: data_size
 *		data		: data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_reg_value_read(int32_t tool_type,
	size_t data_size, void *data)
{
	struct xring_aud_msg dsp_reg_info_msg = {0};
	struct dsp_reg_value_info *dsp_reg_info = NULL;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct dsp_reg_read_info read_reg_info;
	int ret = 0;

	if (!data) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	if (data_size != sizeof(struct dsp_reg_read_info)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size:%zu.", data_size);
		return -EFAULT;
	}

	if (copy_from_user(&read_reg_info, (struct dsp_reg_read_info *)data, data_size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		ret = -EFAULT;
		return ret;
	}

	/* fill message */
	dsp_reg_info_msg.header.scene_id = ADSP_SCENE_NONE;
	dsp_reg_info_msg.header.ins_id = 0;
	dsp_reg_info_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	dsp_reg_info_msg.header.func_id = ADSP_DEBUG_CTRL_REG_VALUE;
	dsp_reg_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct dsp_reg_value_info);
	dsp_reg_info_msg.header.result = 0;

	/* update reg read info */
	dsp_reg_info = (struct dsp_reg_value_info *)&dsp_reg_info_msg.data[0];
	dsp_reg_info->type = DSP_REG_VALUE_READ;
	dsp_reg_info->reg_addr = read_reg_info.reg;
	dsp_reg_info->num = read_reg_info.len/sizeof(uint32_t);
	AUD_LOG_DBG(AUD_COMM, "reg: %d, num: %d", dsp_reg_info->reg_addr,
		dsp_reg_info->num);
	pdata->reg_blk_info.size = read_reg_info.len * 2 + DSP_REG_DUMP_PACKED_LEN;
	pdata->reg_blk_info.vir_addr = fk_acore_smem_alloc(audio_tool_misc.parent,
		&pdata->reg_blk_info.phy_addr, pdata->reg_blk_info.size);
	dsp_reg_info->phy_addr = pdata->reg_blk_info.phy_addr;
	AUD_LOG_DBG(AUD_COMM, "phy addr: 0x%llx, vir addr：0x%llx, size: %u",
		(uint64_t)pdata->reg_blk_info.phy_addr,
		(uint64_t)pdata->reg_blk_info.vir_addr,
		pdata->reg_blk_info.size);

	/* send tool message */
	ret = fk_audio_tool_message_send((struct xring_aud_msg *)&dsp_reg_info_msg);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_audio_tool_reg_value_write
 * description: dsp reg value write
 * parameter:
 *		tool_type	: log dump type
 *		data_size	: data_size
 *		data		: data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_reg_value_write(int32_t tool_type,
	size_t data_size, void *data)
{
	struct xring_aud_msg dsp_reg_info_msg = {0};
	struct dsp_reg_value_info *dsp_reg_info = NULL;
	struct dsp_reg_write_info write_reg_info;
	int ret = 0;

	if (!data) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	if (data_size != sizeof(struct dsp_reg_write_info)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size:%zu.", data_size);
		return -EFAULT;
	}

	if (copy_from_user(&write_reg_info, (struct dsp_reg_write_info *)data, data_size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		ret = -EFAULT;
		return ret;
	}

	/* fill message */
	dsp_reg_info_msg.header.scene_id = ADSP_SCENE_NONE;
	dsp_reg_info_msg.header.ins_id = 0;
	dsp_reg_info_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	dsp_reg_info_msg.header.func_id = ADSP_DEBUG_CTRL_REG_VALUE;
	dsp_reg_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct dsp_reg_value_info);
	dsp_reg_info_msg.header.result = 0;

	/* update reg read info */
	dsp_reg_info = (struct dsp_reg_value_info *)&dsp_reg_info_msg.data[0];
	dsp_reg_info->type = DSP_REG_VALUE_WRITE;
	dsp_reg_info->reg_addr = write_reg_info.reg;
	dsp_reg_info->value = write_reg_info.value;
	AUD_LOG_DBG(AUD_COMM, "reg: %d, value: %d", dsp_reg_info->reg_addr,
		dsp_reg_info->value);

	/* send tool message */
	ret = fk_audio_tool_message_send((struct xring_aud_msg *)&dsp_reg_info_msg);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_audio_tool_dsp_reg_dump
 * description: dsp reg dump
 * parameter:
 *		tool_msg	: tool msg
 ************************************************************************/
static void fk_audio_tool_dsp_reg_dump(struct xring_aud_msg *tool_msg)
{
	struct dump_blk dump_info = {0};
	struct dump_info *reg_info;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();

	reg_info = (struct dump_info *)&tool_msg->data[0];

	dump_info.size = reg_info->len;
	dump_info.vir_addr = vmalloc(dump_info.size);
	if (!dump_info.vir_addr) {
		AUD_LOG_ERR(AUD_COMM, "alloc vir addr failure.");
		return;
	}
	AUD_LOG_DBG(AUD_COMM, "vir_addr:0x%llx, size:%u", (uint64_t)dump_info.vir_addr, dump_info.size);
	if (!pdata->reg_blk_info.vir_addr)
		return;

	mutex_lock(&pdata->mem_lock);
	memcpy(dump_info.vir_addr, pdata->reg_blk_info.vir_addr, reg_info->len);
	mutex_unlock(&pdata->mem_lock);

	fk_audio_tool_dump_copy_put(dump_info);
}

int fk_audio_tool_close_msg_send(void)
{
	struct xring_aud_msg pcm_dump_close_msg = {0};
	struct audio_tool_pdata *pdata;
	int ret = 0;

	pdata = fk_audio_tool_pdata_get();

	/* fill close message */
	pcm_dump_close_msg.header.scene_id = ADSP_SCENE_NONE;
	pcm_dump_close_msg.header.ins_id = 0;
	pcm_dump_close_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	pcm_dump_close_msg.header.func_id = ADSP_DEBUG_CTRL_PCM_DUMP_CLOSE;
	pcm_dump_close_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(struct dsp_pcm_dump_param_info);
	pcm_dump_close_msg.header.result = 0;

	if (pdata->pcm_blk_malloc_flag) {
		ret = fk_audio_tool_message_send((struct xring_aud_msg *)&pcm_dump_close_msg);
		if (ret < 0) {
			AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
			return ret;
		}
		pdata->pcm_blk_malloc_flag = 0;
	}

	return ret;
}

/************************************************************************
 * function: fk_audio_tool_uncache_log_enable
 * description: enable cache log
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_uncache_log_enable(uint8_t en)
{
	struct xring_aud_msg log_dump_info_msg = {0};
	int ret = 0;

	/* fill message */
	log_dump_info_msg.header.scene_id = ADSP_SCENE_NONE;
	log_dump_info_msg.header.ins_id = 0;
	log_dump_info_msg.header.cmd_id = ADSP_DEBUG_CTRL;
	log_dump_info_msg.header.func_id = ADSP_DEBUG_CTRL_UNCACHE_LOG_ENABLE;
	log_dump_info_msg.header.len = sizeof(struct xring_aud_msg_header) +
		sizeof(uint8_t);
	log_dump_info_msg.header.result = 0;

	log_dump_info_msg.data[0] = en;

	/* send dsp uncache log message */
	ret = fk_audio_tool_message_send((struct xring_aud_msg *)&log_dump_info_msg);
	if (ret < 0) {
		AUD_LOG_ERR(AUD_COMM, "send log info message failure.");
		return ret;
	}

	return ret;
}

/************************************************************************
 * function: fk_audio_tool_dsp_uncache_log
 * description: dsp uncache log en
 * parameter:
 *		tool_type	: log dump type
 *		data_size	: data_size
 *		data		: data
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_audio_tool_dsp_uncache_log(int32_t tool_type,
	size_t data_size, void *data)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	uint32_t en_flag;
	int ret = 0;

	if (!data) {
		AUD_LOG_ERR(AUD_COMM, "Invalid user data.");
		return -EFAULT;
	}

	if (data_size != sizeof(uint32_t)) {
		AUD_LOG_ERR(AUD_COMM, "Invalid size:%zu.", data_size);
		return -EFAULT;
	}

	if (copy_from_user(&en_flag, (uint32_t *)data, data_size)) {
		AUD_LOG_ERR(AUD_COMM, "Could not copy size value from user.");
		ret = -EFAULT;
		return ret;
	}

	pdata->uncache_log_en_flag = en_flag;
	AUD_LOG_INFO(AUD_COMM, "flag: %d", pdata->uncache_log_en_flag);
	ret = fk_audio_tool_uncache_log_enable(pdata->uncache_log_en_flag);

	return ret;
}
/************************************************************************
 * function: fk_audio_tool_dsp_uncache_log_param_get
 * description: uncache dsp log param get
 * parameter:
 *		tool_msg	: tool msg
 ************************************************************************/
static void fk_audio_tool_dsp_uncache_log_param_get
	(struct xring_aud_msg *tool_msg)
{
	struct uncache_log_param *log_param;
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();

	log_param = (struct uncache_log_param *)&tool_msg->data[0];

	AUD_LOG_INFO(AUD_COMM, "type:%d,p_len:%d,p_addr:0x%x,c_len:%d,c_addr:0x%x",
		log_param->log_type, log_param->point_len, log_param->point_phy_addr,
		log_param->char_len, log_param->char_phy_addr);

	memcpy(&pdata->uncache_log_p, log_param, sizeof(struct uncache_log_param));
}

/************************************************************************
 * function: get_min_index_node
 * description: get min log node.
 * parameter:
 *      head : log node
 * return:
 *      min_node : min node
 ************************************************************************/
static struct log_node *get_min_index_node(struct log_node *head)
{
	struct log_node *min_node = head;
	struct log_node *current_node = head;

	while (current_node != NULL) {
		if (current_node->info.index < min_node->info.index)
			min_node = current_node;

		current_node = current_node->next;
	}
	return min_node;
}

/************************************************************************
 * function: fk_audio_adsp_uncache_log_dump
 * description: dump adsp uncache log.
 * parameter:
 * return:
 ************************************************************************/
void fk_audio_adsp_uncache_log_dump(void)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct uncache_log_param *log_param;
	int i;

	if (pdata->uncache_log_dumped_flag)
		return;

	log_param = &pdata->uncache_log_p;

	AUD_LOG_DBG(AUD_COMM, "log type:%d.", log_param->log_type);
	AUD_LOG_INFO(AUD_COMM, "adsp has asserted.");

	if (log_param->log_type == LOG_USB_P) {
		struct log_node *head = NULL;
		struct log_node *current_node = NULL;
		struct log_node *free_node = NULL;
		void __iomem *p_vddr;
		int num_p;

		if ((!log_param->point_phy_addr) || (!log_param->point_len))
			return;

		p_vddr = ioremap((phys_addr_t)log_param->point_phy_addr,
			(unsigned long)log_param->point_len);
		num_p = log_param->point_len/DSP_UNCACHE_LOG_PONINT_LEN;
		AUD_LOG_DBG(AUD_COMM, "len:%d, addr:0x%x, num:%d.",
			log_param->point_len, log_param->point_phy_addr, num_p);

		for (i = 0; i <= num_p; i++) {
			struct uncache_log_info *log_info;
			struct log_node *new_node = kzalloc(sizeof(struct log_node), GFP_KERNEL);

			log_info = (struct uncache_log_info *)(p_vddr
				+ i * DSP_UNCACHE_LOG_PONINT_LEN);
			new_node->info.id = log_info->id;
			new_node->info.index = log_info->index;
			new_node->info.value = log_info->value;
			new_node->next = NULL;

			if (head == NULL) {
				head = new_node;
				free_node = new_node;
				current_node = new_node;
			} else {
				current_node->next = new_node;
				current_node = new_node;
			}
		}

		while (head != NULL) {
			struct log_node *min_node = get_min_index_node(head);
			struct log_node *prev = NULL;
			struct log_node *current_node = head;

			AUD_LOG_INFO(AUD_COMM, "INDEX:%x ID:%x VALUE:%x.",
				min_node->info.index, min_node->info.id,
				min_node->info.value);
			while (current_node != NULL) {
				if (current_node->info.index == min_node->info.index) {
					if (prev == NULL)
						head = current_node->next;
					else
						prev->next = current_node->next;

					break;
				}
				prev = current_node;
				current_node = current_node->next;
			}
		}

		while (free_node != NULL) {
			struct log_node *next = free_node->next;

			kfree(free_node);
			free_node = next;
		}
	}

	if (log_param->log_type & (LOG_UART_MASK | LOG_USB_C_MASK)) {
		struct dump_blk dump_info = {0};
		void __iomem *c_vddr;

		if ((!log_param->char_phy_addr) || (!log_param->char_len))
			return;

		c_vddr = ioremap((phys_addr_t)log_param->char_phy_addr,
			(unsigned long)log_param->char_len);
		AUD_LOG_DBG(AUD_COMM, "len:%d, addr:0x%x.",
			log_param->char_len, log_param->char_phy_addr);

		dump_info.size = log_param->char_len;
		dump_info.vir_addr = vmalloc(dump_info.size);
		if (!dump_info.vir_addr) {
			AUD_LOG_ERR(AUD_COMM, "alloc vir addr failure.");
			return;
		}
		AUD_LOG_DBG(AUD_COMM, "vir_addr:0x%llx, size:%u",
			(uint64_t)dump_info.vir_addr, dump_info.size);
		memcpy(dump_info.vir_addr, c_vddr, dump_info.size);
		fk_audio_tool_dump_copy_put(dump_info);
		pdata->uncache_log_dumped_flag++;
	}
}

void fk_audio_init_tool_dump_data(void)
{
	struct audio_tool_pdata *pdata;

	pdata = fk_audio_tool_pdata_get();

	AUD_LOG_DBG(AUD_COMM, "dump pcm vir_addr: 0x%llx, phy_addr: 0x%llx",
		(uint64_t)pdata->pcm_blk_info.vir_addr, pdata->pcm_blk_info.phy_addr);
	AUD_LOG_DBG(AUD_COMM, "dump reg vir_addr: 0x%llx, phy_addr: 0x%llx",
		(uint64_t)pdata->reg_blk_info.vir_addr, pdata->reg_blk_info.phy_addr);

	mutex_lock(&pdata->mem_lock);
	if (pdata->pcm_blk_info.vir_addr != NULL && pdata->pcm_blk_info.phy_addr != 0) {
		fk_acore_smem_free(audio_tool_misc.parent, DSP_PCM_DUMP_MEM_SIZE,
			pdata->pcm_blk_info.vir_addr, pdata->pcm_blk_info.phy_addr);
		pdata->pcm_blk_info.vir_addr = NULL;
		pdata->pcm_blk_info.phy_addr = 0;
		pdata->pcm_blk_malloc_flag = 0;
		pdata->pcm_blk_info.count = 0;
	}

	if (pdata->reg_blk_info.vir_addr != NULL && pdata->reg_blk_info.phy_addr != 0) {
		fk_acore_smem_free(audio_tool_misc.parent, pdata->reg_blk_info.size,
			pdata->reg_blk_info.vir_addr, pdata->reg_blk_info.phy_addr);
		pdata->reg_blk_info.vir_addr = NULL;
		pdata->reg_blk_info.phy_addr = 0;
	}
	mutex_unlock(&pdata->mem_lock);

	pdata->log_blk_info.count = 0;
	pdata->pcm_blk_info.count = 0;
	INIT_LIST_HEAD(&pdata->dump_data_list);
}

/************************************************************************
 * function: fk_audio_tool_recovery_mem_free
 * description: audio tool recovery memory free.
 * parameter:
 * return:
 ************************************************************************/
static void fk_audio_tool_recovery_mem_free(void)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct cal_memory_list_node *memory_node = NULL;
	struct list_head *ptr, *next;

	/* free pcm dump mem */
	fk_audio_init_tool_dump_data();

	/* free calibration mem */
	mutex_lock(&pdata->memory_list_lock);
	if (list_empty(&pdata->cal_memory_list)) {
		mutex_unlock(&pdata->memory_list_lock);
		AUD_LOG_INFO(AUD_COMM, "none cal memory list");
		return;
	}

	/* get data node */
	list_for_each_safe(ptr, next, &pdata->cal_memory_list) {
		memory_node = list_entry(ptr, struct cal_memory_list_node, data_list);

		if ((!memory_node->vir_addr) || (!memory_node->phy_addr) ||
			!memory_node->size) {
			mutex_unlock(&pdata->memory_list_lock);
			return;
		}

		AUD_LOG_DBG(AUD_COMM, "v_addr:0x%llx p_addr=0x%llx size %hu",
			(uint64_t)memory_node->vir_addr, (uint64_t)memory_node->phy_addr,
			memory_node->size);

		fk_acore_smem_free(audio_tool_misc.parent, memory_node->size,
			(void *)memory_node->vir_addr, memory_node->phy_addr);
		list_del(&memory_node->data_list);
		/* free data node resource */
		kfree(memory_node);
	}
	mutex_unlock(&pdata->memory_list_lock);
}

/************************************************************************
 * function: fk_audio_tool_recovery
 * description: audio tool recovery.
 * parameter:
 * return:
 ************************************************************************/
static void fk_audio_tool_recovery(void *priv)
{
	AUD_LOG_INFO(AUD_COMM, "audio tool recovery.");
	fk_audio_tool_recovery_mem_free();
}

int fk_audio_init_tool_pdata(void)
{
	struct audio_tool_pdata *pdata = fk_audio_tool_pdata_get();
	struct recovery_ops *ops = NULL;

	INIT_LIST_HEAD(&pdata->dump_data_list);
	INIT_LIST_HEAD(&pdata->cal_data_list);
	INIT_LIST_HEAD(&pdata->cal_memory_list);
	mutex_init(&pdata->memory_list_lock);
	mutex_init(&pdata->cal_list_lock);
	mutex_init(&pdata->dump_list_lock);
	mutex_init(&pdata->mem_lock);
	pdata->uncache_log_dumped_flag = 0;

	init_waitqueue_head(&pdata->cal_wait_que);
	atomic_set(&pdata->cal_wait, 0);

	ops = kzalloc(sizeof(struct recovery_ops), GFP_KERNEL);
	ops->pre_recovery = fk_audio_tool_recovery;
	if (fk_adsp_ssr_register(ops, NULL) == NULL)
		AUD_LOG_ERR(AUD_COMM, "audio tool register adsp ssr action failed");

	return 0;
}

/************************************************************************
 * function: fk_audio_tool_msg_handle
 * description: response message process
 * parameter:
 *		msg : response message content
 *		len : length of message
 *		priv : private parameters of ipc callback
 * return:
 ************************************************************************/
void fk_audio_tool_msg_handle(void *msg, int len, void *priv)
{
	struct xring_aud_msg *tool_msg = (struct xring_aud_msg *)msg;
	enum adsp_ctrl_type tool_func_type = 0;

	if (len != tool_msg->header.len)
		AUD_LOG_DBG(AUD_COMM, "len unmatch.(%d) (%d)", len, tool_msg->header.len);

	tool_func_type = (enum adsp_ctrl_type)tool_msg->header.func_id;

	AUD_LOG_INFO(AUD_COMM, "tool_func_type:%d", tool_func_type);

	switch (tool_func_type) {
	case ADSP_DEBUG_CTRL_DSP_LOG:
	case ADSP_DEBUG_CTRL_PCM_DUMP_OPEN:
	case ADSP_DEBUG_CTRL_PCM_DUMP_CLOSE:
	case ADSP_DEBUG_CTRL_UNCACHE_LOG_ENABLE:
		if (fk_adsp_vote_unregister(AUDIO_TOOL_CONN_SESS) < 0)
			AUD_LOG_DBG(AUD_COMM, "audio tool dis power down.");
		break;
	case ADSP_DEBUG_CTRL_DSP_LOG_TELL_AP:
		fk_audio_tool_dsp_log_dump(tool_msg);
		break;
	case ADSP_DEBUG_CTRL_PCM_DUMP_TELL_AP:
		fk_audio_tool_dsp_pcm_dump(tool_msg);
		break;
	case ADSP_DEBUG_CTRL_REG_VALUE_TELL_AP:
		fk_audio_tool_dsp_reg_dump(tool_msg);
		break;
	case ADSP_DEBUG_CTRL_UNCACHE_MEM_TELL_AP:
		fk_audio_tool_dsp_uncache_log_param_get(tool_msg);
		break;
	default:
		break;
	}
}

/* Module information */
MODULE_DESCRIPTION("XRING AUDIO TOOL CORE DRIVER");
MODULE_LICENSE("Dual BSD/GPL");
