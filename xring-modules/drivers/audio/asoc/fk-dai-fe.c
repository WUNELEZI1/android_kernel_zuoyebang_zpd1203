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
#include <linux/version.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include "fk-dai-fe.h"
#include "fk-pcm.h"
#include "fk-compress.h"
#include "fk-voip.h"
#include "fk-mmap.h"
#include "../common/fk-audio-log.h"


static const struct snd_soc_dapm_route multimedia_route_playback[] = {
	{"XR_DL0", NULL, "XR_media0 Playback"},
	{"XR_DL1", NULL, "XR_media1 Playback"},
	{"XR_DL2", NULL, "XR_media2 Playback"},
	{"XR_DL3", NULL, "XR_media3 Playback"},
	{"XR_DL5", NULL, "XR_media5 Playback"},
	{"XR_DL6", NULL, "XR_media6 Playback"},
	{"XR_DL7", NULL, "XR_media7 Playback"},
	{"XR_DL8", NULL, "XR_spatial Playback"},
	{"XR_MDL", NULL, "XR_direct Playback"},
	{"XR_MMAP_DL", NULL, "XR_mmap Playback"},
	{"XR_Karaoke_DL", NULL, "XR_Karaoke Playback"},
	{"XR_INCALL_DL", NULL, "XR_incall Playback"},
	{"XR_HF_DL", NULL, "XR_hf Playback"},
};
static const struct snd_soc_dapm_route multimedia_route_capture[] = {
	{"XR_media0 Capture", NULL, "XR_UL0"},
	{"XR_media1 Capture", NULL, "XR_UL1"},
	{"XR_media2 Capture", NULL, "XR_UL2"},
	{"XR_media3 Capture", NULL, "XR_UL3"},
	{"XR_media4 Capture", NULL, "XR_UL4"},
	{"XR_media6 Capture", NULL, "XR_UL6"},
	{"XR_mmap Capture", NULL, "XR_MMAP_UL"},
	{"XR_Karaoke Capture", NULL, "XR_Karaoke_UL"},
	{"XR_incall Capture", NULL, "XR_INCALL_UL"},
	{"XR_hf Capture", NULL, "XR_HF_UL"},
};

#define MAX_STREAM_INS_NUM (8)
struct snd_substream_priv {
	int flag;
	int session_id;
	void *strm;
};

struct snd_substream_priv snd_prtd[MAX_STREAM_INS_NUM];

int get_pcm_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_PCM_PLAYBACK;
	/* Capture path */
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_PCM_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8)|(dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
			return session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "session unfound");

	return session_id;
}

int get_voip_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_VOIP_PLAYBACK;
	/* Capture path */
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_VOIP_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8)|(dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
		}
	}

	return session_id;
}

int get_compr_session_id(struct snd_compr_stream *cstream)
{
	struct snd_soc_pcm_runtime *rtd = cstream->private_data;
	struct snd_soc_dai *dai = asoc_rtd_to_cpu(rtd, 0);
	int scene_type_id = 0;
	int session_id = -1;

	if (dai->driver->cops && dai->driver->cops->startup) {
		if (cstream->direction == SND_COMPRESS_PLAYBACK)
			scene_type_id = SCENE_COMPR_PLAYBACK;
		/* Capture path */
		else if (cstream->direction == SNDRV_PCM_STREAM_CAPTURE)
			scene_type_id = SCENE_COMPR_CAPTURE;

		session_id = (scene_type_id << 8)|(dai->id);
	}

	return session_id;
}

int get_voice_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_VOICE_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_VOICE_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8) | (dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
		}
	}

	return session_id;
}

int get_haptic_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_HAPTIC;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8) | (dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
		}
	}

	return session_id;
}

int get_spat_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_SPATIAL;
	else
		return -1;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8)|(dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
			return session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "session unfound");

	return session_id;
}

int get_mmap_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_MMAP_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_MMAP_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8) | (dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
			return session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "session unfound");

	return session_id;
}

int get_karaoke_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_KARAOKE_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_KARAOKE_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8) | (dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
			return session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "session unfound");

	return session_id;
}

int get_incall_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_INCALL_PLAYBACK;
	/* Capture path */
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_INCALL_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8)|(dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
			return session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "session unfound");

	return session_id;
}

int get_hf_session_id(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai *dai = NULL;
	int scene_type_id = 0;
	int session_id = -1;
	int i;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_HF_PLAYBACK;
	/* Capture path */
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_HF_CAPTURE;

	for_each_rtd_dais(rtd, i, dai) {
		if (dai->driver->ops && dai->driver->ops->startup) {
			session_id = (scene_type_id << 8)|(dai->id);
			AUD_LOG_DBG_LIM(AUD_SOC, "session=0x%x", session_id);
			return session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "session unfound");

	return session_id;
}

static void init_session_id(void)
{
	int i = 0;

	for (i = 0; i < MAX_STREAM_INS_NUM; i++) {
		snd_prtd[i].flag = 0;
		snd_prtd[i].session_id = 0;
		snd_prtd[i].strm = NULL;
	}
}

int get_session_id(void *strm)
{
	int i = 0;

	if (strm == NULL) {
		AUD_LOG_INFO(AUD_SOC, "strm is null");
		return -1;
	}

	for (i = 0; i < MAX_STREAM_INS_NUM; i++) {
		if ((strm == snd_prtd[i].strm) && snd_prtd[i].flag) {
			AUD_LOG_DBG_LIM(AUD_SOC, "strm(%d) is %p", i, strm);
			return snd_prtd[i].session_id;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "exit(%p)", strm);

	return -1;
}
EXPORT_SYMBOL(get_session_id);

static void set_session_id(enum scene_type type, void *strm, int sess_id)
{
	int i = 0;

	if (strm == NULL) {
		AUD_LOG_INFO(AUD_SOC, "strm is null");
		return;
	}

	for (i = 0; i < MAX_STREAM_INS_NUM; i++) {
		if (snd_prtd[i].flag == 0) {
			snd_prtd[i].session_id = sess_id;
			snd_prtd[i].strm = strm;
			snd_prtd[i].flag = 1;
			AUD_LOG_DBG_LIM(AUD_SOC, "strm(%d) is %p", i, strm);
			return;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "exit(%p)", strm);
}

static void clear_session_id(enum scene_type type, void *strm)
{
	int i = 0;

	if (strm == NULL) {
		AUD_LOG_INFO(AUD_SOC, "strm is null");
		return;
	}

	for (i = 0; i < MAX_STREAM_INS_NUM; i++) {
		if ((snd_prtd[i].flag == 1) && (strm == snd_prtd[i].strm)) {
			snd_prtd[i].strm = NULL;
			snd_prtd[i].session_id = 0;
			snd_prtd[i].flag = 0;
			AUD_LOG_DBG_LIM(AUD_SOC, "strm(%d) is %p", i, strm);
			return;
		}
	}

	AUD_LOG_INFO(AUD_SOC, "exit(%p)", strm);
}

static int multimedia_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	AUD_LOG_DBG(AUD_SOC, "substream->stream=%d fe-dai->id=%d name=%s",
		substream->stream, dai->id, (char *)&substream->name);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_PCM_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_PCM_CAPTURE;

	session_id = (scene_type_id << 8)|(dai->id);
	set_session_id(scene_type_id, (void *)substream, session_id);

	return 0;
}

static void multimedia_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	AUD_LOG_DBG(AUD_SOC, "substream->stream=%d fe-dai->id=%d",
		substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_PCM_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_PCM_CAPTURE;

	clear_session_id(scene_type_id, (void *)substream);
}

static int voip_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_VOIP_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_VOIP_CAPTURE;

	session_id = (scene_type_id << 8)|(dai->id);
	set_session_id(scene_type_id, (void *)substream, session_id);

	return 0;
}

static void voip_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_VOIP_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_VOIP_CAPTURE;

	clear_session_id(scene_type_id, (void *)substream);
}

static int compr_multimedia_startup(struct snd_compr_stream *stream,
			struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	AUD_LOG_DBG(AUD_SOC, "stream->direction=%d fe-dai->id=%d",
		stream->direction, dai->id);
	if (stream->direction == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_COMPR_PLAYBACK;
	else if (stream->direction == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_COMPR_CAPTURE;
	session_id = (scene_type_id << 8)|(dai->id);
	set_session_id(scene_type_id, (void *)stream, session_id);

	return 0;
}

static int compr_multimedia_shutdown(struct snd_compr_stream *stream,
			struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	AUD_LOG_INFO(AUD_SOC, "stream->direction=%d fe-dai->id=%d",
		stream->direction, dai->id);

	if (stream->direction == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_COMPR_PLAYBACK;
	else if (stream->direction == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_COMPR_CAPTURE;

	clear_session_id(scene_type_id, (void *)stream);

	return 0;
}

static int mmap_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_MMAP_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_MMAP_CAPTURE;

	session_id = (scene_type_id << 8)|(dai->id);
	set_session_id(scene_type_id, (void *)substream, session_id);

	return 0;
}

static void mmap_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_MMAP_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_MMAP_CAPTURE;

	clear_session_id(scene_type_id, (void *)substream);
}

static int incall_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_INCALL_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_INCALL_CAPTURE;

	session_id = (scene_type_id << 8)|(dai->id);
	set_session_id(scene_type_id, (void *)substream, session_id);

	return 0;
}

static void incall_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_INCALL_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_INCALL_CAPTURE;

	clear_session_id(scene_type_id, (void *)substream);
}

static int hf_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_HF_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_HF_CAPTURE;

	session_id = (scene_type_id << 8)|(dai->id);
	set_session_id(scene_type_id, (void *)substream, session_id);

	return 0;
}

static void hf_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_HF_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_HF_CAPTURE;

	clear_session_id(scene_type_id, (void *)substream);
}

static const struct snd_soc_dapm_route voice_call_route_playback[] = {
	{"XR_Voice_DL", NULL, "XR_voice DL"},
};
static const struct snd_soc_dapm_route voice_call_route_capture[] = {
	{"XR_voice UL", NULL, "XR_Voice_UL"},
};

static const struct snd_soc_dapm_route loopback_call_route_playback[] = {
	{"XR_Loopback_DL", NULL, "XR_Loopback DL"},
};
static const struct snd_soc_dapm_route loopback_call_route_capture[] = {
	{"XR_Loopback UL", NULL, "XR_Loopback_UL"},
};

static const struct snd_soc_dapm_route voice_trigger_route_capture[] = {
	{"XR_voicetrigger", NULL, "XR_voicetrigger_UL"},
	{"XR_asr Capture", NULL, "XR_asr_UL"},
};

static int voice_call_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;
	int session_id = 0;

	AUD_LOG_DBG(AUD_SOC, "substream->stream=%d fe-dai->id=%d name=%s",
		substream->stream, dai->id, (char *)&substream->name);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_VOICE_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_VOICE_CAPTURE;

	session_id = (scene_type_id << 8) | (dai->id);
	set_session_id(scene_type_id, (void *)substream, session_id);

	return 0;
}

void voice_call_shutdown(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int scene_type_id = 0;

	dev_info(dai->dev, "%s substream->stream=%d fe-dai->id=%d,\n",
				__func__, substream->stream, dai->id);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		scene_type_id = SCENE_VOICE_PLAYBACK;
	else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		scene_type_id = SCENE_VOICE_CAPTURE;

	clear_session_id(scene_type_id, (void *)substream);
}

static int voice_call_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	AUD_LOG_INFO(AUD_SOC, "substream->number=%d", substream->number);
	return 0;
}

static int voice_call_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	AUD_LOG_INFO(AUD_SOC, "substream->number=%d", substream->number);
	return 0;
}

static int voice_call_prepare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	AUD_LOG_INFO(AUD_SOC, "substream->number=%d", substream->number);
	return 0;
}

static int voice_call_trigger(struct snd_pcm_substream *substream,
	int cmd, struct snd_soc_dai *dai)
{
	AUD_LOG_INFO(AUD_SOC, "substream->number=%d", substream->number);
	return 0;
}

static int loopback_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	AUD_LOG_INFO(AUD_SOC, "name=%s stream=%d id=%d", (char *)substream->name,
		substream->stream, dai->id);
	return 0;
}

static int fe_dai_probe(struct snd_soc_dai *dai)
{
	struct snd_soc_dapm_context *dapm;

	init_session_id();

	dapm = snd_soc_component_get_dapm(dai->component);
	snd_soc_dapm_add_routes(dapm, multimedia_route_playback, ARRAY_SIZE(multimedia_route_playback));
	snd_soc_dapm_add_routes(dapm, multimedia_route_capture, ARRAY_SIZE(multimedia_route_capture));

	snd_soc_dapm_add_routes(dapm, voice_call_route_playback, ARRAY_SIZE(voice_call_route_playback));
	snd_soc_dapm_add_routes(dapm, voice_call_route_capture, ARRAY_SIZE(voice_call_route_capture));

	snd_soc_dapm_add_routes(dapm, loopback_call_route_playback,
			ARRAY_SIZE(loopback_call_route_playback));
	snd_soc_dapm_add_routes(dapm, loopback_call_route_capture,
			ARRAY_SIZE(loopback_call_route_capture));
	snd_soc_dapm_add_routes(dapm, voice_trigger_route_capture,
			ARRAY_SIZE(voice_trigger_route_capture));
	return 0;
}

const struct snd_soc_dai_ops fk_fe_Multimedia_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= multimedia_startup,
	.shutdown	= multimedia_shutdown,
};

const struct snd_soc_dai_ops fk_fe_voip_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= voip_startup,
	.shutdown	= voip_shutdown,
};

const struct snd_soc_dai_ops fk_fe_Compr_dai_ops = {
	.probe		= fe_dai_probe,
	.compress_new	= fk_compr_new,
};

const struct snd_soc_cdai_ops fk_fe_Compr_dai_cops = {
	.startup	= compr_multimedia_startup,
	.shutdown	= compr_multimedia_shutdown,
};

static struct snd_soc_dai_ops fk_fe_voice_call_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= voice_call_startup,
	.shutdown	= voice_call_shutdown,
	.hw_params	= voice_call_hw_params,
	.hw_free	= voice_call_hw_free,
	.prepare	= voice_call_prepare,
	.trigger	= voice_call_trigger,
};

static struct snd_soc_dai_ops fk_fe_loopback_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= loopback_startup,
};

const struct snd_soc_dai_ops fk_fe_mmap_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= mmap_startup,
	.shutdown	= mmap_shutdown,
};

const struct snd_soc_dai_ops fk_fe_incall_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= incall_startup,
	.shutdown	= incall_shutdown,
};

const struct snd_soc_dai_ops fk_fe_hf_dai_ops = {
	.probe		= fe_dai_probe,
	.startup	= hf_startup,
	.shutdown	= hf_shutdown,
};

static const struct snd_soc_component_driver fk_fe_dai_component = {
	.name		= "fk-dai-fe",
};

static struct snd_soc_dai_driver fk_fe_dais[] = {
	/*ultra fast*/
	{
		.playback = {
			.stream_name = "XR_media0 Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.capture = {
			.stream_name = "XR_media0 Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_MEDIA0,
		.name = "XR_media0",
	},
	/*fast*/
	{
		.playback = {
			.stream_name = "XR_media1 Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.capture = {
			.stream_name = "XR_media1 Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_MEDIA1,
		.name = "XR_media1",
	},
	/*normal*/
	{
		.playback = {
			.stream_name = "XR_media2 Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.capture = {
			.stream_name = "XR_media2 Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_MEDIA2,
		.name = "XR_media2",
	},

	{
		.playback = {
			.stream_name = "XR_media3 Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.capture = {
			.stream_name = "XR_media3 Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_MEDIA3,
		.name = "XR_media3",
	},
	/*compress capture*/
	{
		.capture = {
			.stream_name = "XR_media4 Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.ops = &fk_fe_Compr_dai_ops,
		.cops = &fk_fe_Compr_dai_cops,
		.id = XR_MEDIA4,
		.name = "XR_media4",
	},
	/*compress playback*/
	{
		.playback = {
			.stream_name = "XR_media5 Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.ops = &fk_fe_Compr_dai_ops,
		.cops = &fk_fe_Compr_dai_cops,
		.id = XR_MEDIA5,
		.name = "XR_media5",
	},

	/*voip*/
	{
		.playback = {
			.stream_name = "XR_media6 Playback",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.capture = {
			.stream_name = "XR_media6 Capture",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.ops = &fk_fe_voip_dai_ops,
		.id = XR_MEDIA6,
		.name = "XR_media6",
	},

	/* Voice Call */
	{
		.playback = {
			.stream_name = "XR_voice DL",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.capture = {
			.stream_name = "XR_voice UL",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &fk_fe_voice_call_dai_ops,
		.id = XR_VOICE,
		.name = "XR_voice",
	},
	/* Loopback */
	{
		.playback = {
			.stream_name = "XR_Loopback DL",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.capture = {
			.stream_name = "XR_Loopback UL",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE |
				    SNDRV_PCM_FMTBIT_S24_3LE |
				    SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	48000,
		},
		.ops = &fk_fe_loopback_dai_ops,
		.name = "XR_Loopback",
	},

	/*haptic playback*/
	{
		.playback = {
			.stream_name = "XR_media7 Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_MEDIA7,
		.name = "XR_media7",
	},

	/*voice trigger*/
	{
		.capture = {
			.stream_name = "XR_voicetrigger",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_LSM,
		.name = "XR_lsm",
	},

	/*mutil playback*/
	{
		.playback = {
			.stream_name = "XR_direct Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_DIRECT,
		.name = "XR_direct",
	},

	/*spatial playback*/
	{
		.playback = {
			.stream_name = "XR_spatial Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 96000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_SPATIAL,
		.name = "XR_spatial",
	},

	/*mmap*/
	{
		.playback = {
			.stream_name = "XR_mmap Playback",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.capture = {
			.stream_name = "XR_mmap Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.ops = &fk_fe_mmap_dai_ops,
		.id = XR_MMAP,
		.name = "XR_mmap",
	},

	/* karaoke */
	{
		.playback = {
			.stream_name = "XR_Karaoke Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 8,
			.rate_min = 8000,
			.rate_max = 96000,
		},
		.capture = {
			.stream_name = "XR_Karaoke Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 4,
			.rate_min = 8000,
			.rate_max = 96000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_KARAOKE,
		.name = "XR_Karaoke",
	},
	/*incall*/
	{
		.playback = {
			.stream_name = "XR_incall Playback",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.capture = {
			.stream_name = "XR_incall Capture",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.ops = &fk_fe_incall_dai_ops,
		.id = XR_INCALL,
		.name = "XR_incall",
	},
	/*hf*/
	{
		.playback = {
			.stream_name = "XR_hf Playback",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.capture = {
			.stream_name = "XR_hf Capture",
			.rates = (SNDRV_PCM_RATE_8000_48000),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
		},
		.ops = &fk_fe_hf_dai_ops,
		.id = XR_HF,
		.name = "XR_hf",
	},

	/*asr*/
	{
		.capture = {
			.stream_name = "XR_asr Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 32,
			.rate_min = 8000,
			.rate_max = 384000,
		},
		.ops = &fk_fe_Multimedia_dai_ops,
		.id = XR_ASR,
		.name = "XR_asr",
	},
};

static int fk_fe_dai_dev_probe(struct platform_device *pdev)
{

	AUD_LOG_INFO(AUD_SOC, "dev name %s", dev_name(&pdev->dev));
	return snd_soc_register_component(&pdev->dev, &fk_fe_dai_component,
		fk_fe_dais, ARRAY_SIZE(fk_fe_dais));
}

static int fk_fe_dai_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_dai_fe_dt_match[] = {
	{.compatible = "xring,fk-dai-fe"},
	{}
};

static struct platform_driver fk_fe_dai_driver = {
	.probe  = fk_fe_dai_dev_probe,
	.remove = fk_fe_dai_dev_remove,
	.driver = {
		.name = "fk-dai-fe",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_fe_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init fk_fe_dai_init(void)
{
	return platform_driver_register(&fk_fe_dai_driver);
}

void fk_fe_dai_exit(void)
{
	platform_driver_unregister(&fk_fe_dai_driver);
}

/* Module information */
MODULE_DESCRIPTION("XRING Frontend DAI driver");
MODULE_LICENSE("Dual BSD/GPL");
