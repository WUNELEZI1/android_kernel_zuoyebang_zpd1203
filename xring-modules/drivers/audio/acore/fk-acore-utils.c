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
#include <linux/ktime.h>
#include <sound/compress_params.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/timer.h>

#include "fk-acore-utils.h"
#include "../asoc/fk-pcm-routing.h"
#include "../adsp/adsp_boot_driver.h"

#define NORMAL_PCM_PERIODS_SIZE		(20)

#define SAMPLES_PER_AAC_FRAME		(1024)
#define SAMPLES_PER_AMR_FRAME		(160)
#define SAMPLES_PER_AMRWB_FRAME		(320)
#define SAMPLES_PER_MP3_FRAME		(1152)

#define COMPRESS_RATE_AAC_DEFAULT	(15)
#define COMPRESS_RATE_AMR_DEFAULT	(10)
#define COMPRESS_RATE_MP3_DEFAULT	(10)

static bool adsp_vote_func = true;
/************************************************************************
 * function: fk_acore_adsp_codec_caps_check
 * description: check if the codec type support by the adsp
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_adsp_codec_caps_check(struct acore_stream *ac_strm)
{
	int ret = 0;

	if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_PLAY) {
		switch (ac_strm->fmt.codec_type) {
		case SND_AUDIOCODEC_MP3:
			ret = 0;
			break;
		case SND_AUDIOCODEC_AAC:
			ret = 0;
			break;
		case SND_AUDIOCODEC_PCM:
			ret = 0;
			break;
		default:
			ret = -EINVAL;
			break;
		}
	} else if (ac_strm->adsp_sc_t == ADSP_SCENE_OFFLOAD_RECORD) {
		switch (ac_strm->fmt.codec_type) {
		case SND_AUDIOCODEC_AAC:
		case SND_AUDIOCODEC_AMR:
		case SND_AUDIOCODEC_AMRWB:
			ret = 0;
			break;
		default:
			ret = -EINVAL;
			break;
		}
	} else {
		ret = 0;
		AUD_LOG_INFO(AUD_CORE, "raw data scene(%d).", ac_strm->adsp_sc_t);
	}

	return ret;
}


/************************************************************************
 * function: fk_acore_get_adsp_codec_type
 * description: get bytes one frame
 * parameter:
 *		int format: codec type which define in compress_params.h
 * return:
 *		codec type value
 ************************************************************************/
int fk_acore_get_adsp_codec_type(int format_type)
{
	int codec_type = CODEC_NONE;

	switch (format_type) {
	case SND_AUDIOCODEC_MP3:
		codec_type = CODEC_MP3;
		break;
	case SND_AUDIOCODEC_AAC:
		codec_type = CODEC_AAC;
		break;
	case SND_AUDIOCODEC_AMR:
		codec_type = CODEC_AMR;
		break;
	case SND_AUDIOCODEC_AMRWB:
		codec_type = CODEC_AMR;
		break;
	case SND_AUDIOCODEC_PCM:
		codec_type = CODEC_NONE;
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "0x%x is not compress format.", format_type);
		codec_type = CODEC_NONE;
	}

	return codec_type;
}

/************************************************************************
 * function: fk_acore_get_bytes_per_sample
 * description: get bytes one frame
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		bytes value
 ************************************************************************/
int fk_acore_get_bytes_per_sample(struct acore_stream *ac_strm)
{
	/* default as 2 channels and bit 16 */
	int bytes_per_frame = 4;

	if (!ac_strm)
		AUD_LOG_INFO(AUD_CORE, "acore stream is NULL.");
	else
		bytes_per_frame = (ac_strm->fmt.channels * ac_strm->fmt.bit_width / 8);

	return bytes_per_frame;
}

/************************************************************************
 * function: compr_per_frame_samples_get
 * description: get samples size one frame
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		>0: samples number one frame
 *		<=0: failure
 ************************************************************************/
static int compr_per_frame_samples_get(struct acore_stream *ac_strm)
{
	int ret = 0;
	uint32_t format_type;

	format_type = ac_strm->fmt.codec_type;
	switch (format_type) {
	case SND_AUDIOCODEC_MP3:
		ret = SAMPLES_PER_MP3_FRAME;
		break;
	case SND_AUDIOCODEC_AAC:
		ret = SAMPLES_PER_AAC_FRAME;
		break;
	case SND_AUDIOCODEC_AMR:
		ret = SAMPLES_PER_AMR_FRAME;
		break;
	case SND_AUDIOCODEC_AMRWB:
		ret = SAMPLES_PER_AMRWB_FRAME;
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "compress format(0x%x) unsupport.", format_type);
		ret = -EINVAL;
	}

	return ret;
}

/************************************************************************
 * function: compr_rate_get
 * description: get samples size one frame
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		>0: compress rate value
 *		<=0: failure
 ************************************************************************/
static int compr_rate_get(struct acore_stream *ac_strm)
{
	int ret = 0;
	uint32_t format_type;

	format_type = ac_strm->fmt.codec_type;
	switch (format_type) {
	case SND_AUDIOCODEC_MP3:
		ret = COMPRESS_RATE_MP3_DEFAULT;
		break;
	case SND_AUDIOCODEC_AAC:
		ret = COMPRESS_RATE_AAC_DEFAULT;
		break;
	case SND_AUDIOCODEC_AMR:
	case SND_AUDIOCODEC_AMRWB:
		ret = COMPRESS_RATE_AMR_DEFAULT;
		break;
	default:
		AUD_LOG_INFO(AUD_CORE, "format(0x%x) unsupport.", format_type);
		ret = -EINVAL;
	}

	return ret;
}

/************************************************************************
 * function: fk_compr_block_size_get
 * description: get bytes of compress stream one buffer block
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		bytes value
 ************************************************************************/
int fk_compr_block_size_get(struct acore_stream *ac_strm, uint16_t frames_perblk)
{
	int ret = 0;
	int frame_samples = 0;
	int bytes_per_sample = 0;
	int compress_rate = 1;
	int block_size = 0;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_DBG(AUD_CORE, "acore stream is NULL.");
		return ret;
	}

	frame_samples = compr_per_frame_samples_get(ac_strm);
	if (frame_samples <= 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "invalid size. codec_type(%d)", ac_strm->fmt.codec_type);
		return ret;
	}

	compress_rate = compr_rate_get(ac_strm);
	if (compress_rate <= 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "invalid size. codec_type(%d)", ac_strm->fmt.codec_type);
		return ret;
	}

	bytes_per_sample = fk_acore_get_bytes_per_sample(ac_strm);

	/* calculate the size of block. page align. */
	block_size = (((frame_samples * bytes_per_sample * frames_perblk) /
		compress_rate) + 0x1000) & 0x1000;

	AUD_LOG_INFO(AUD_CORE, "block size is %d.", block_size);

	return block_size;
}

/************************************************************************
 * function: fk_compr_frames_perblk_get
 * description: get frames of compress stream one buffer block
 * parameter:
 *		ac_strm : stream instance
 * return:
 *		frame number value
 ************************************************************************/
int fk_compr_frames_perblk_get(struct acore_stream *ac_strm, uint32_t size)
{
	int ret = 0;
	int frame_samples = 0;
	int bytes_per_sample = 0;
	int compress_rate = 1;
	int frames = 0;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_INFO(AUD_CORE, "ac_strm is NULL");
		return ret;
	}

	frame_samples = compr_per_frame_samples_get(ac_strm);
	if (frame_samples <= 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "invalid size. codec_type(%d)",
			ac_strm->fmt.codec_type);
		return ret;
	}

	compress_rate = compr_rate_get(ac_strm);
	if (compress_rate <= 0) {
		ret = -EINVAL;
		AUD_LOG_ERR(AUD_CORE, "invalid size. codec_type(%d)",
			ac_strm->fmt.codec_type);
		return ret;
	}

	bytes_per_sample = fk_acore_get_bytes_per_sample(ac_strm);

	/* calculate the size of frames can locate on one block */
	frames = (size * compress_rate) / (frame_samples * bytes_per_sample);
	AUD_LOG_DBG(AUD_CORE, "frames is %d.", frames);

	return frames;
}

/************************************************************************
 * function: fk_acore_periods_size_check
 * description: adsp just support 20ms one block. check periods size valid.
 * parameter:
 *		ac_strm : stream instance
 *		periods_size : samples size one block buffer
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_acore_periods_size_check(struct acore_stream *ac_strm, uint32_t periods_size)
{
	int ret = EOK;
	int frames_size = 0;

	if (!ac_strm) {
		ret = -EINVAL;
		AUD_LOG_INFO_LIM(AUD_CORE, "acore stream is NULL.");
		return ret;
	}

	if (ac_strm->adsp_sc_t == ADSP_SCENE_NORMAL_PLAY) {
		/* calculate frames size when scene is normal pcm */
		frames_size = (ac_strm->fmt.samples * NORMAL_PCM_PERIODS_SIZE) / 1000;
		if (frames_size != periods_size) {
			ret = -EINVAL;
			AUD_LOG_ERR_LIM(AUD_CORE, "sess(0x%x) periods_size(%d) frames_size(%d).",
				ac_strm->sess_id, periods_size, frames_size);
			return ret;
		}
	}

	return ret;
}

/************************************************************************
 * function: fk_be_item_table_update
 * description: update backend info.
 * parameter:
 *		struct kctrl_item
 * return:
 ************************************************************************/
void fk_be_item_table_update(void *route_table, uint32_t items_num)
{
	struct acore_client *ac = NULL;

	ac = (struct acore_client *)fk_acore_get_acore_client();
	if (!ac) {
		AUD_LOG_INFO(AUD_CORE, "audio client is NULL");
		return;
	}

	ac->route_table = route_table;
	ac->items_num = items_num;
}
EXPORT_SYMBOL(fk_be_item_table_update);

/************************************************************************
 * function: fk_stream_port_info_update
 * description: update stream be port info
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_stream_port_info_update(struct acore_stream *ac_strm)
{
	int ret = 0;
	int i = 0;
	uint16_t item_sess_id = 0;

	struct acore_client *ac = NULL;
	struct kctrl_item *items;

	ac = (struct acore_client *)fk_acore_get_acore_client();
	if (!ac || !ac->route_table) {
		AUD_LOG_INFO(AUD_CORE, "audio client or route table is NULL");
		return ret;
	}

	/* reset port number */
	ac_strm->port_num = 0;

	items = (struct kctrl_item *)ac->route_table;
	for (i = 0; i < ac->items_num; i++) {
		item_sess_id = (items->scene_id << 8) |
			items->stream_id;

		if ((item_sess_id == ac_strm->sess_id) && items->val) {
			ac_strm->port_info[ac_strm->port_num] = items->port_id;
			ac_strm->port_num++;
			AUD_LOG_INFO(AUD_CORE, "num(%d) name(%s)", ac_strm->port_num, items->name);

			if (ac_strm->port_num >= BE_DEV_NUM_MAX) {
				AUD_LOG_ERR(AUD_CORE, "be devices max: name(%s) val=%d",
					items->name, items->val);
				ac_strm->port_num = BE_DEV_NUM_MAX;
				return ret;
			}
		}

		items++;
	}

	return ret;
}

/************************************************************************
 * function: fk_port_id_get_by_session
 * description: get backend port id by session id.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_port_id_get_by_session(uint16_t session_id)
{
	int ret = -1;
	int i = 0;
	uint16_t item_sess_id = 0;

	struct acore_client *ac = NULL;
	struct kctrl_item *items;

	ac = (struct acore_client *)fk_acore_get_acore_client();
	if (!ac || !ac->route_table) {
		AUD_LOG_INFO(AUD_CORE, "audio client or route table is NULL");
		return ret;
	}

	items = (struct kctrl_item *)ac->route_table;
	for (i = 0; i < ac->items_num; i++) {
		item_sess_id = (items->scene_id << 8) |
			items->stream_id;

		if ((item_sess_id == session_id) && items->val) {
			ret = items->port_id;
			AUD_LOG_INFO(AUD_CORE, "name(%s) val=%d", items->name, items->val);
			break;
		}
		items++;
	}

	return ret;
}

/************************************************************************
 * function: fk_port_id_get_by_sess_dir
 * description: get backend port id by session id and port direction.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_port_id_get_by_sess_dir(uint16_t session_id, uint16_t dir)
{
	int ret = -1;
	int i = 0;
	uint16_t item_sess_id = 0;

	struct acore_client *ac = NULL;
	struct kctrl_item *items;

	ac = (struct acore_client *)fk_acore_get_acore_client();
	if (!ac || !ac->route_table) {
		AUD_LOG_INFO(AUD_CORE, "audio client or route table is NULL");
		return ret;
	}

	items = (struct kctrl_item *)ac->route_table;
	for (i = 0; i < ac->items_num; i++) {
		item_sess_id = (items->scene_id << 8) |
			items->stream_id;

		if ((item_sess_id == session_id) && (dir == items->port_dir)
			&& items->val) {
			ret = items->port_id;
			AUD_LOG_INFO(AUD_CORE, "name(%s) val=%d", items->name, items->val);
			break;
		}
		items++;
	}

	return ret;
}

/************************************************************************
 * function: fk_adsp_vote_item_get
 * description: get adsp power vote info item.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
struct adsp_vote_info *fk_adsp_vote_item_get(uint16_t session_id)
{
	struct adsp_vote_info *voteInfo = NULL;
	struct acore_client *ac = fk_acore_get_acore_client();

	if (!ac) {
		AUD_LOG_ERR(AUD_CORE, "audio client is null");
		return NULL;
	}

	mutex_lock(&ac->vote_lock);

	list_for_each_entry(voteInfo, &ac->vote_list, vote_info_list) {
		if (voteInfo->sess_id == session_id) {
			mutex_unlock(&ac->vote_lock);
			return voteInfo;
		}
	}

	mutex_unlock(&ac->vote_lock);

	return NULL;
}

/************************************************************************
 * function: fk_adsp_vote_register
 * description: register adsp power vote.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_vote_register(uint16_t session_id)
{
	int ret = 0;
	struct adsp_vote_info *voteInfo = NULL;
	struct acore_client *ac = fk_acore_get_acore_client();

	if (!ac) {
		AUD_LOG_ERR(AUD_CORE, "audio client is null");
		ret = -1;
		return ret;
	}

	if (atomic_read(&ac->ssr_flag))
		return ret;

	voteInfo = fk_adsp_vote_item_get(session_id);
	if (voteInfo != NULL) {
		if (voteInfo->pu_vote_flag)
			AUD_LOG_INFO(AUD_CORE, "re-vote PU: session id(0x%x).", session_id);

		voteInfo->pu_vote_flag = 1;
		voteInfo->pu_vote_cnt++;
		voteInfo->pu_vote_tstamp = (uint64_t)ktime_get_ns();
	} else {
		AUD_LOG_INFO(AUD_CORE, "vote info alloc: session id(0x%x) tstamp(%lld).",
			session_id, (uint64_t)ktime_get_ns());
		voteInfo = kzalloc(sizeof(struct adsp_vote_info), GFP_KERNEL);
		if (voteInfo) {
			voteInfo->sess_id = session_id;
			voteInfo->pu_vote_flag = 1;
			voteInfo->pu_vote_cnt++;
			voteInfo->pu_vote_tstamp = (uint64_t)ktime_get_ns();

			mutex_lock(&ac->vote_lock);
			list_add(&voteInfo->vote_info_list, &ac->vote_list);
			mutex_unlock(&ac->vote_lock);
		} else
			AUD_LOG_ERR(AUD_CORE, "alloc vote info fail: session id(0x%x).", session_id);
	}

	mutex_lock(&ac->vote_lock);

	atomic_inc(&ac->vote_num);
	AUD_LOG_INFO(AUD_CORE, "session(0x%x) vote_num(%d)", session_id,
		atomic_read(&ac->vote_num));

	if (get_adsp_power_status() == false) {
		ret = adsp_poweron();
		if (ret < 0)
			AUD_LOG_ERR(AUD_CORE, "vote adsp power on fail");
	}

	mutex_unlock(&ac->vote_lock);

	return ret;
}

/************************************************************************
 * function: fk_adsp_vote_unregister
 * description: unregister adsp power vote.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_vote_unregister(uint16_t session_id)
{
	int ret = 0;
	struct adsp_vote_info *voteInfo = NULL;
	struct acore_client *ac = fk_acore_get_acore_client();

	if (!ac) {
		AUD_LOG_ERR(AUD_CORE, "audio client is null");
		ret = -1;
		return ret;
	}

	if (atomic_read(&ac->ssr_flag))
		return ret;

	voteInfo = fk_adsp_vote_item_get(session_id);
	if (voteInfo != NULL) {
		if (!voteInfo->pu_vote_flag)
			AUD_LOG_INFO(AUD_CORE, "re-vote PD: session id(0x%x).", session_id);

		voteInfo->pu_vote_flag = 0;
		voteInfo->pd_vote_cnt++;
		voteInfo->pd_vote_tstamp = (uint64_t)ktime_get_ns();
	} else {
		AUD_LOG_ERR(AUD_CORE, "vote PD unregister session(0x%x).", session_id);
		return ret;
	}

	if (voteInfo->pd_vote_cnt > voteInfo->pu_vote_cnt) {
		AUD_LOG_ERR(AUD_CORE, "session(0x%x) unvote more than vote, vote(0x%x), unvote(0x%x)",
			session_id, voteInfo->pu_vote_cnt, voteInfo->pd_vote_cnt);
		return 0;
	}

	mutex_lock(&ac->vote_lock);

	if (atomic_read(&ac->vote_num) > 0)
		atomic_dec(&ac->vote_num);
	AUD_LOG_INFO(AUD_CORE, "session(0x%x) vote_num(%d)", session_id,
		atomic_read(&ac->vote_num));

	if (!atomic_read(&ac->vote_num) && (adsp_vote_func == true)) {
		ret = adsp_poweroff();
		if (ret < 0)
			AUD_LOG_ERR(AUD_CORE, "vote adsp power off fail");
	}

	mutex_unlock(&ac->vote_lock);

	return ret;
}

/************************************************************************
 * function: fk_adsp_vote_info_query
 * description: query adsp power vote info.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_vote_info_query(void)
{
	int ret = 0;
	struct adsp_vote_info *voteInfo = NULL;
	struct acore_client *ac = fk_acore_get_acore_client();

	AUD_LOG_ERR(AUD_CORE, "vote info query");

	if (!ac) {
		AUD_LOG_ERR(AUD_CORE, "audio client is null");
		ret = -1;
		return ret;
	}

	mutex_lock(&ac->vote_lock);

	list_for_each_entry(voteInfo, &ac->vote_list, vote_info_list) {
		if (voteInfo) {
			AUD_LOG_INFO(AUD_CORE, "audio session(0x%x)", voteInfo->sess_id);
			AUD_LOG_INFO(AUD_CORE, "flag:%d", voteInfo->pu_vote_flag);
			AUD_LOG_INFO(AUD_CORE, "pu: cnt(%d) tstamp(%lld)",
				voteInfo->pu_vote_cnt, voteInfo->pu_vote_tstamp);
			AUD_LOG_INFO(AUD_CORE, "pd: cnt(%d) tstamp(%lld)",
				voteInfo->pd_vote_cnt, voteInfo->pd_vote_tstamp);
		}
	}

	mutex_unlock(&ac->vote_lock);

	return ret;
}


/************************************************************************
 * function: fk_adsp_vote_reset
 * description: reset adsp power vote info.
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_vote_reset(void)
{
	int ret = 0;
	struct list_head *ptr = NULL;
	struct list_head *next = NULL;
	struct adsp_vote_info *voteInfo = NULL;
	struct acore_client *ac = fk_acore_get_acore_client();

	if (!ac) {
		AUD_LOG_ERR(AUD_CORE, "audio client is null");
		ret = -1;
		return ret;
	}

	mutex_lock(&ac->vote_lock);

	list_for_each_safe(ptr, next, &ac->vote_list) {
		voteInfo = list_entry(ptr, struct adsp_vote_info, vote_info_list);
		if (voteInfo) {
			list_del(&voteInfo->vote_info_list);

			AUD_LOG_INFO(AUD_CORE, "free node(%llx) id(0x%x)",
				(uint64_t)voteInfo, voteInfo->sess_id);
			kfree(voteInfo);
		}
	}

	mutex_unlock(&ac->vote_lock);

	return ret;
}

/************************************************************************
 * function: fk_adsp_vote_config
 * description: adsp vote function configure
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_vote_config(int flag)
{
	AUD_LOG_INFO(AUD_CORE, "flag = %d", flag);

	if (flag)
		adsp_vote_func = true;
	else
		adsp_vote_func = false;

	return 0;
}

/************************************************************************
 * function: fk_adsp_vote_config_query
 * description: get adsp vote function configure info
 * parameter:
 * return:
 *		bool:
 *			true : enable vote
 *			false : disable vote
 ************************************************************************/
bool fk_adsp_vote_config_query(void)
{
	return adsp_vote_func;
}

/************************************************************************
 * function: fk_adsp_vote
 * description: adsp vote power
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_vote(bool enable)
{
	int ret = 0;

	if (enable == true) {
		AUD_LOG_INFO(AUD_CORE, "vote power on");
		fk_adsp_vote_register(0xFFFF);
	} else {
		AUD_LOG_INFO(AUD_CORE, "vote power off");
		fk_adsp_vote_unregister(0xFFFF);
	}

	return ret;
}

/************************************************************************
 * function: fk_adsp_power_status_get
 * description: adsp power status check
 * parameter:
 * return:
 *		0: success
 *		other: failure
 ************************************************************************/
int fk_adsp_power_status_get(void)
{
	return get_adsp_power_status();
}
