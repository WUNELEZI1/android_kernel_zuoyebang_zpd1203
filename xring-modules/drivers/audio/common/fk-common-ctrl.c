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
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include <sound/pcm_params.h>
#include <sound/hwdep.h>
#include <sound/pcm_params.h>
#include <sound/info.h>

#include "audio-trace.h"

#include "fk-common-ctrl.h"
#include "fk-audio-log.h"
#include "fk-audio-pinctrl.h"
#include "../acore/fk-acore.h"
#include "../acore/fk-acore-define.h"

#define DIRECT_PLAY_SESSION 0xb

enum factory_mode {
	FACTORY_MODE_OFF,
	FACTORY_MODE_ON,
	FACTORY_MODE_MAX,
};

/********************** static parameter define begin ********************/
static int loopback_type_val = LP_TYPE_PCM;
static int audif_fmtbit_val = PCM_FMTBIT_16;
static int audio_tdm_fmtchs_val = PCM_FMTCHS_4;
static int audif_fmtchs_val = PCM_FMTCHS_2;
static int voice_type_val = VOICE_TYPE_NORMAL;
static int codec_type_val = PORT_DAI_CODEC_TYPE_NORMAL;
static int g_i2s5_func = PIN_INIT;
static int g_codec_clk_func = PIN_INIT;
static int audio_factory_mode = FACTORY_MODE_OFF;

static struct bt_codec_config g_bt_config;
/*for ec config*/
static int fk_ec_ref_ch = 2;
static int fk_ec_ref_bit_format = 1;
static int fk_ec_ref_sampling_rate = 2;
static int fk_ec_type;
static int adsp_reset_done;
static int direct_play_volume;
static int adsp_temp;
static int g_device_type;

/********************** static parameter define end **********************/

int fk_loopback_type_query(void)
{
	AUD_LOG_INFO(AUD_COMM, "loopback type = %d", loopback_type_val);
	return loopback_type_val;
}
EXPORT_SYMBOL(fk_loopback_type_query);

static int fk_loopback_type_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = loopback_type_val;

	return 0;
}

static int fk_loopback_type_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if (ucontrol->value.integer.value[0] < LP_TYPE_MAX)
		loopback_type_val = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "loopback type value invalid");

	AUD_LOG_INFO(AUD_COMM, "current loopback type = %d", loopback_type_val);

	return 1;
}

int fk_audif_fmtchs_query(void)
{
	AUD_LOG_DBG(AUD_COMM, "fmtchs value = %d", audif_fmtchs_val);
	return audif_fmtchs_val;
}
EXPORT_SYMBOL(fk_audif_fmtchs_query);

static int fk_audif_fmtchs_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = audif_fmtchs_val;

	return 0;
}

static int fk_audif_fmtchs_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if ((ucontrol->value.integer.value[0] == PCM_FMTCHS_2) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_4) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_6) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_8))
		audif_fmtchs_val = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "audif chs value invalid");

	AUD_LOG_INFO(AUD_COMM, "current audif chs value = %d", audif_fmtchs_val);

	return 1;
}


int fk_audif_fmtbit_query(void)
{
	AUD_LOG_DBG(AUD_COMM, "fmtbit value = %d", audif_fmtbit_val);
	return audif_fmtbit_val;
}
EXPORT_SYMBOL(fk_audif_fmtbit_query);

static int fk_audif_fmtbit_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = audif_fmtbit_val;

	return 0;
}

static int fk_audif_fmtbit_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if ((ucontrol->value.integer.value[0] == PCM_FMTBIT_16) ||
		(ucontrol->value.integer.value[0] == PCM_FMTBIT_24) ||
		(ucontrol->value.integer.value[0] == PCM_FMTBIT_32))
		audif_fmtbit_val = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "audif bit value invalid");

	AUD_LOG_INFO(AUD_COMM, "current audif bit value = %d", audif_fmtbit_val);

	return 1;
}

int fk_audio_tdm_fmtchs_query(void)
{
	AUD_LOG_DBG(AUD_COMM, "fmtchs value = %d", audio_tdm_fmtchs_val);
	return audio_tdm_fmtchs_val;
}
EXPORT_SYMBOL(fk_audio_tdm_fmtchs_query);

static int fk_audio_tdm_fmtchs_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = audio_tdm_fmtchs_val;

	return 0;
}

static int fk_audio_tdm_fmtchs_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if ((ucontrol->value.integer.value[0] == PCM_FMTCHS_2) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_4) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_6) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_8) ||
		(ucontrol->value.integer.value[0] == PCM_FMTCHS_16))
		audio_tdm_fmtchs_val = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "tdm chs value invalid");

	AUD_LOG_INFO(AUD_COMM, "current tdm chs value = %d", audio_tdm_fmtchs_val);

	return 1;
}

int fk_codec_type_query(void)
{
	AUD_LOG_INFO(AUD_COMM, "codec type = %d", codec_type_val);
	return codec_type_val;
}
EXPORT_SYMBOL(fk_codec_type_query);

static int fk_codec_type_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = codec_type_val;
	return 0;
}

static int fk_codec_type_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if (ucontrol->value.integer.value[0] < PORT_DAI_CODEC_TYPE_MAX)
		codec_type_val = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "codec type value invalid");

	AUD_LOG_INFO(AUD_COMM, "current codec type = %d", codec_type_val);

	return 1;
}

int fk_voice_type_query(void)
{
	AUD_LOG_INFO(AUD_COMM, "voice type = %d", voice_type_val);
	return voice_type_val;
}
EXPORT_SYMBOL(fk_voice_type_query);

static int fk_voice_type_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = voice_type_val;
	return 0;
}

static int fk_voice_type_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if (ucontrol->value.integer.value[0] < VOICE_TYPE_MAX)
		voice_type_val = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "voice type value invalid");

	AUD_LOG_INFO(AUD_COMM, "current voice type = %d", voice_type_val);

	return 1;
}

int fk_i2s5_func_query(void)
{
	AUD_LOG_INFO(AUD_COMM, "i2s5 func = %d", voice_type_val);
	return g_i2s5_func;
}
EXPORT_SYMBOL(fk_i2s5_func_query);

static int fk_i2s5_func_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = g_i2s5_func;

	return 0;
}

static int fk_i2s5_func_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int action;

	if (ucontrol->value.integer.value[0] < PIN_STATE_MAX)
		g_i2s5_func = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "i2s5 func invalid");

	AUD_LOG_INFO(AUD_COMM, "i2s5 func = %d", g_i2s5_func);
	if (g_i2s5_func == PIN_DEFAULT)
		action = PIN_DEFAULT;
	else
		action = PIN_SLEEP;

	ret = fk_audio_pinctrl_switch_state(I2S5_FUNC, action);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "pinctrl switch state failed");

	return 1;
}

static int fk_codec_clk_func_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = g_codec_clk_func;

	return 0;
}

static int fk_codec_clk_func_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int action;

#ifdef ATRACE_MACRO
	AUD_TRACE("enter. name(%s)", kcontrol->id.name);
#endif

	if (ucontrol->value.integer.value[0] < PIN_STATE_MAX)
		g_codec_clk_func = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "codec_clk_func invalid");

	if (g_codec_clk_func == PIN_DEFAULT)
		action = PIN_DEFAULT;
	else
		action = PIN_SLEEP;

	ret = fk_audio_pinctrl_switch_state(CDC_CLK_FUNC, action);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "pinctrl switch state failed");

#ifdef ATRACE_MACRO
	AUD_TRACE("exit");
#endif

	return 1;
}

static int fk_audio_factory_mode_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = audio_factory_mode;

	AUD_LOG_INFO(AUD_COMM, "audio_factory_mode:%d", audio_factory_mode);

	return 0;
}

static int fk_audio_factory_mode_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	if (ucontrol->value.integer.value[0] < FACTORY_MODE_MAX)
		audio_factory_mode = ucontrol->value.integer.value[0];
	else
		AUD_LOG_INFO(AUD_COMM, "audio_factory_mode invalid");

	AUD_LOG_INFO(AUD_COMM, "audio_factory_mode = %d", audio_factory_mode);

	return 1;
}

int fk_bt_config_query(struct bt_codec_config *bt_config)
{
	memcpy(bt_config, &g_bt_config, sizeof(struct bt_codec_config));

	AUD_LOG_DBG(AUD_COMM, "bt_codec_format %d", bt_config->port_info.codec_type);

	return 0;
}
EXPORT_SYMBOL(fk_bt_config_query);

static int bt_codec_cfg_info(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_BYTES;
	uinfo->count = sizeof(struct bt_codec_config);

	return 0;
}

static int bt_codec_cfg_get(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int format_size = sizeof(struct bt_codec_port_info);

	memcpy(ucontrol->value.bytes.data, &g_bt_config.port_info, format_size);

	switch (g_bt_config.port_info.codec_type) {
	case CODEC_NONE:
		AUD_LOG_DBG(AUD_COMM, "pcm config: rate %d, chs %d, bits %d",
			g_bt_config.port_info.sample_rate,
			g_bt_config.port_info.channels,
			g_bt_config.port_info.bit_width);
		break;

	case CODEC_SBC:
		memcpy(ucontrol->value.bytes.data + format_size,
			&g_bt_config.data,
			sizeof(struct sbc_enc_info));
		AUD_LOG_DBG(AUD_COMM, "sbc cfg: chs %d, blks %d, subbands %d",
			g_bt_config.data.sbc_config.channel_mode,
			g_bt_config.data.sbc_config.blocks,
			g_bt_config.data.sbc_config.subbands);
		AUD_LOG_DBG(AUD_COMM, "sbc cfg: method %d, min_bitpool %d, max_bitpool %d",
			g_bt_config.data.sbc_config.alloc_method,
			g_bt_config.data.sbc_config.min_bitpool,
			g_bt_config.data.sbc_config.max_bitpool);
		break;

	case CODEC_AAC:
		memcpy(ucontrol->value.bytes.data + format_size,
			&g_bt_config.data,
			sizeof(struct aac_enc_info));
		AUD_LOG_DBG(AUD_COMM, "aac config: profile %d, vbr %d",
			g_bt_config.data.aac_config.profile,
			g_bt_config.data.aac_config.vbr);
		break;

	default:
		AUD_LOG_DBG(AUD_SOC, "Ignore enc config for unknown format = %d",
			g_bt_config.port_info.codec_type);
		break;
	}

	return ret;
}

static int bt_codec_cfg_put(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int format_size = sizeof(struct bt_codec_port_info);

	memset(&g_bt_config, 0x0, sizeof(struct bt_codec_config));
	memcpy(&g_bt_config.port_info, ucontrol->value.bytes.data, format_size);
	AUD_LOG_DBG(AUD_SOC, "Received encoder config for %d format",
		g_bt_config.port_info.codec_type);

	switch (g_bt_config.port_info.codec_type) {
	case CODEC_NONE:
		AUD_LOG_DBG(AUD_COMM, "pcm cfg: rate %d, chs %d, bits %d",
			g_bt_config.port_info.sample_rate,
			g_bt_config.port_info.channels,
			g_bt_config.port_info.bit_width);
		break;

	case CODEC_SBC:
		memcpy(&g_bt_config.data,
			ucontrol->value.bytes.data + format_size,
			sizeof(struct sbc_enc_info));
		AUD_LOG_DBG(AUD_COMM, "sbc cfg: ch_mode %d, blks %d, subbands %d",
			g_bt_config.data.sbc_config.channel_mode,
			g_bt_config.data.sbc_config.blocks,
			g_bt_config.data.sbc_config.subbands);
		AUD_LOG_DBG(AUD_COMM, "sbc cfg: method %d, min_bitpool %d, max_bitpool %d",
			g_bt_config.data.sbc_config.alloc_method,
			g_bt_config.data.sbc_config.min_bitpool,
			g_bt_config.data.sbc_config.max_bitpool);
		break;

	case CODEC_AAC:
		memcpy(&g_bt_config.data,
			ucontrol->value.bytes.data + format_size,
			sizeof(struct aac_enc_info));
		AUD_LOG_DBG(AUD_COMM, "aac config: profile %d, vbr %d",
			g_bt_config.data.aac_config.profile,
			g_bt_config.data.aac_config.vbr);
		break;

	default:
		AUD_LOG_DBG(AUD_SOC, "Ignore enc config for unknown format = %d",
			g_bt_config.port_info.codec_type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int fk_ec_ref_ch_query(void)
{
	AUD_LOG_DBG(AUD_COMM, "ec ref channel = %d", fk_ec_ref_ch);
	return fk_ec_ref_ch;
}
EXPORT_SYMBOL(fk_ec_ref_ch_query);

int fk_ec_ref_bit_format_query(void)
{
	int bit_fmt = 0;

	switch (fk_ec_ref_bit_format) {
	case 0:
		bit_fmt = 0;
		break;
	case 1:
		bit_fmt = 16;
		break;
	case 2:
		bit_fmt = 24;
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport bit format");
		break;
	}
	AUD_LOG_DBG(AUD_COMM, "ec ref bit format = %d", bit_fmt);
	return bit_fmt;
}
EXPORT_SYMBOL(fk_ec_ref_bit_format_query);

int fk_ec_ref_sample_rate_query(void)
{
	int samp_rate = 0;

	switch (fk_ec_ref_sampling_rate) {
	case 0:
		samp_rate = 0;
		break;
	case 1:
		samp_rate = 8000;
		break;
	case 2:
		samp_rate = 16000;
		break;
	case 3:
		samp_rate = 24000;
		break;
	case 4:
		samp_rate = 32000;
		break;
	case 5:
		samp_rate = 44100;
		break;
	case 6:
		samp_rate = 48000;
		break;
	case 7:
		samp_rate = 96000;
		break;
	case 8:
		samp_rate = 192000;
		break;
	case 9:
		samp_rate = 384000;
		break;
	default:
		AUD_LOG_INFO(AUD_COMM, "unsupport sample rate");
		break;
	}
	AUD_LOG_DBG(AUD_COMM, "ec ref samplate rate = %d", samp_rate);
	return samp_rate;
}
EXPORT_SYMBOL(fk_ec_ref_sample_rate_query);

int fk_ec_ref_type_query(void)
{
	AUD_LOG_DBG(AUD_COMM, "ec ref type = %d", fk_ec_type);

	return fk_ec_type;
}
EXPORT_SYMBOL(fk_ec_ref_type_query);

int fk_ec_type_from_val(int ec_val)
{
	int ec_type = 0;

	switch (ec_val) {
	case 0:
		ec_type = FROM_EC_NONE;
		break;
	case 1:
		ec_type = FROM_IIS_RX;
		break;
	case 2:
		ec_type = FROM_AUDIF_ADC;
		break;
	case 3:
		ec_type = FROM_DSP;
		break;
	case 4:
		ec_type = FROM_DSP_IIS_TX;
		break;
	case 5:
		ec_type = FROM_DSP_AUDIF_DAC;
		break;
	default:
		AUD_LOG_ERR(AUD_COMM, "unsupport ec type");
		break;
	}
	return ec_type;
}
EXPORT_SYMBOL(fk_ec_type_from_val);

static int fk_ec_ref_ch_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = fk_ec_ref_ch;
	AUD_LOG_DBG(AUD_COMM, "fk_ec_ref_ch = %ld\n",
		ucontrol->value.integer.value[0]);
	return 0;
}

static int fk_ec_ref_ch_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	fk_ec_ref_ch = ucontrol->value.integer.value[0];
	AUD_LOG_DBG(AUD_COMM, "fk_ec_ref_ch = %d\n", fk_ec_ref_ch);
	return 0;
}

static int fk_ec_ref_bit_format_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = fk_ec_ref_bit_format;
	AUD_LOG_DBG(AUD_COMM, "fk_ec_ref_bit_format = %ld\n",
		ucontrol->value.integer.value[0]);
	return 0;
}

static int fk_ec_ref_bit_format_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	fk_ec_ref_bit_format = ucontrol->value.integer.value[0];
	AUD_LOG_DBG(AUD_COMM, "fk_ec_ref_bit_format = %d\n", fk_ec_ref_bit_format);
	return 0;
}

static int fk_ec_ref_rate_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = fk_ec_ref_sampling_rate;
	AUD_LOG_DBG(AUD_COMM, "fk_ec_ref_sampling_rate = %ld\n",
		ucontrol->value.integer.value[0]);
	return 0;
}

static int fk_ec_ref_rate_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	fk_ec_ref_sampling_rate = ucontrol->value.integer.value[0];
	AUD_LOG_DBG(AUD_COMM, "fk_ec_ref_sampling_rate = %d\n", fk_ec_ref_sampling_rate);
	return 0;
}

static int fk_ec_type_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = fk_ec_type;
	AUD_LOG_DBG(AUD_COMM, "fk_ec_type = %ld\n",
		ucontrol->value.integer.value[0]);
	return 0;
}

static int fk_ec_type_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	fk_ec_type = ucontrol->value.integer.value[0];
	AUD_LOG_DBG(AUD_COMM, "fk_ec_type = %d\n", fk_ec_type);
	return 0;
}

int fk_adsp_reset_done_query(void)
{
	return adsp_reset_done;
}

void fk_adsp_reset_done_set(int adsp_fixed)
{
	adsp_reset_done = adsp_fixed;
	AUD_LOG_INFO(AUD_COMM, "adsp_reset_done = %d", adsp_reset_done);
}

static int fk_adsp_reset_done_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = adsp_reset_done;

	return 0;
}

static int fk_adsp_reset_done_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	adsp_reset_done = ucontrol->value.integer.value[0];

	AUD_LOG_INFO(AUD_COMM, "adsp_reset_done = %d", adsp_reset_done);
	return 1;
}

static int fk_directpl_volume_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = direct_play_volume;

	return 0;
}

static int fk_directpl_volume_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	int volume_index = 0;

	volume_index = ucontrol->value.integer.value[0];
	ret = fk_acore_set_vol(DIRECT_PLAY_SESSION, volume_index);
	if (ret < 0)
		AUD_LOG_ERR(AUD_COMM, "set offload play vol failed");
	else
		direct_play_volume = volume_index;

	AUD_LOG_INFO(AUD_COMM, "current direct play volume = %d", direct_play_volume);
	return 1;
}

/************************************************************************
 * function: fk_adsp_temp_get
 * description: adsp low temperature ctrl get func
 * parameter:
 * return:
 ************************************************************************/

static int fk_adsp_temp_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = adsp_temp;

	return 0;
}

/************************************************************************
 * function: fk_adsp_temp_put
 * description: adsp low temperature ctrl put func
 * parameter:
 * return:
 ************************************************************************/

static int fk_adsp_temp_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int temp = 0;

	temp = ucontrol->value.integer.value[0];
	if (temp == 0 || temp == 1) {
		adsp_temp = temp;
		fk_acore_set_dsp_temp(temp);
	} else {
		AUD_LOG_ERR(AUD_COMM, "dsp temperature value invalid %d", adsp_temp);
	}
	AUD_LOG_INFO(AUD_COMM, "current dsp temperature = %d", adsp_temp);
	return 1;
}

void fk_device_type_set(int device_type)
{
	g_device_type = device_type;
	AUD_LOG_INFO(AUD_COMM, "g_device_type = %d", g_device_type);
}

static int fk_device_type_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = g_device_type;

	return 0;
}

static int fk_device_type_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	return 1;
}

static const char * const ec_ref_ch_text[] = {"Zero", "One", "Two", "Three",
	"Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven",
	"Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen"};

static const char * const ec_ref_bit_format_text[] = {"0", "S16_LE", "S24_LE"};

static const char * const ec_ref_rate_text[] = {"0", "8000", "16000",
	"24000", "32000", "44100", "48000", "96000", "192000", "384000"};

static const char * const ec_ref_type_text[] = {"FROM_EC_NONE", "FROM_IIS_RX",
	"FROM_AUDIF_ADC", "FROM_DSP", "FROM_DSP_IIS_TX", "FROM_DSP_AUDIF_DAC"};

static const struct soc_enum fk_route_ec_ref_params_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(ec_ref_ch_text), ec_ref_ch_text),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(ec_ref_bit_format_text),
				ec_ref_bit_format_text),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(ec_ref_rate_text), ec_ref_rate_text),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(ec_ref_type_text), ec_ref_type_text),
};

static const struct snd_kcontrol_new ec_ref_param_controls[] = {
	SOC_ENUM_EXT("EC Reference Channels", fk_route_ec_ref_params_enum[0],
		fk_ec_ref_ch_get, fk_ec_ref_ch_put),
	SOC_ENUM_EXT("EC Reference Bit Format", fk_route_ec_ref_params_enum[1],
		fk_ec_ref_bit_format_get, fk_ec_ref_bit_format_put),
	SOC_ENUM_EXT("EC Reference SampleRate", fk_route_ec_ref_params_enum[2],
		fk_ec_ref_rate_get, fk_ec_ref_rate_put),
	SOC_ENUM_EXT("EC Type", fk_route_ec_ref_params_enum[3],
		fk_ec_type_get, fk_ec_type_put),
};

static const struct snd_kcontrol_new fk_audio_common_controls[] = {
	{
		.access = (SNDRV_CTL_ELEM_ACCESS_READWRITE |
			   SNDRV_CTL_ELEM_ACCESS_INACTIVE),
		.iface = SNDRV_CTL_ELEM_IFACE_PCM,
		.name = "A2DP Encoder Config",
		.info = bt_codec_cfg_info,
		.get = bt_codec_cfg_get,
		.put = bt_codec_cfg_put,
	},
	SOC_SINGLE_EXT("Loopback_Type", SND_SOC_NOPM, 0, LP_TYPE_MAX, 0,
		fk_loopback_type_get, fk_loopback_type_put),
	SOC_SINGLE_EXT("audio_fmtbit", SND_SOC_NOPM, 0, PCM_FMTBIT_32, 0,
		fk_audif_fmtbit_get, fk_audif_fmtbit_put),
	SOC_SINGLE_EXT("voice_type", SND_SOC_NOPM, 0, VOICE_TYPE_MAX, 0,
		fk_voice_type_get, fk_voice_type_put),
	SOC_SINGLE_EXT("Codec Type", SND_SOC_NOPM, 0, PORT_DAI_CODEC_TYPE_MAX, 0,
		fk_codec_type_get, fk_codec_type_put),
	SOC_SINGLE_EXT("i2s5_func", SND_SOC_NOPM, 0, PIN_STATE_MAX, 0,
		fk_i2s5_func_get, fk_i2s5_func_put),
	SOC_SINGLE_EXT("audio_tdm_fmtchs", SND_SOC_NOPM, 0, PCM_FMTCHS_2, 0,
		fk_audio_tdm_fmtchs_get, fk_audio_tdm_fmtchs_put),
	SOC_SINGLE_EXT("audio_fmtchs", SND_SOC_NOPM, 0, PCM_FMTCHS_2, 0,
		fk_audif_fmtchs_get, fk_audif_fmtchs_put),
	SOC_SINGLE_EXT("Codec Clock", SND_SOC_NOPM, 0, PIN_STATE_MAX, 0,
		fk_codec_clk_func_get, fk_codec_clk_func_put),
	SOC_SINGLE_EXT("Factory Mode", SND_SOC_NOPM, 0, FACTORY_MODE_MAX, 0,
		fk_audio_factory_mode_get, fk_audio_factory_mode_put),
	SOC_SINGLE_EXT("adsp_reset_done", SND_SOC_NOPM, 0, 1, 0,
		fk_adsp_reset_done_get, fk_adsp_reset_done_put),
	SOC_SINGLE_EXT("direct playback volume", SND_SOC_NOPM, 0, VOLUME_VALUE_MAX, 0,
		fk_directpl_volume_get, fk_directpl_volume_put),
	SOC_SINGLE_EXT("adsp_temp", SND_SOC_NOPM, 0, 1, 0,
		fk_adsp_temp_get, fk_adsp_temp_put),
	SOC_SINGLE_EXT("device_type", SND_SOC_NOPM, 0, 1, 0,
		fk_device_type_get, fk_device_type_put),
};

void fk_common_controls_add(struct snd_soc_component *component)
{
	/* init parameter */
	fk_ec_type = 0;

	snd_soc_add_component_controls(component, fk_audio_common_controls,
		ARRAY_SIZE(fk_audio_common_controls));
	snd_soc_add_component_controls(component, ec_ref_param_controls,
		ARRAY_SIZE(ec_ref_param_controls));
}
EXPORT_SYMBOL(fk_common_controls_add);


