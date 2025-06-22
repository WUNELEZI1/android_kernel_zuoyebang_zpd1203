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
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/asound.h>

#ifndef FK_AUDIO_USB_OFFLOAD
#define FK_AUDIO_USB_OFFLOAD 1
#endif

#include "fk-dai-be.h"
#include "fk-dai-fe.h"
#include "fk-pcm.h"

#ifdef FK_AUDIO_USB_OFFLOAD
#include "../common/fk-audio-usb.h"
#endif
#include "../common/fk-audio-log.h"

/* Default configuration of I2S channels */
static struct dai_format aif_dev_cfg[] = {
	[FK_XR_I2S0_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S0_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S1_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S1_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S2_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S2_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S3_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S3_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S4_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S4_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S5_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S5_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S6_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S6_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S7_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S7_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S8_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S8_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S9_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},
	[FK_XR_I2S9_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},

	[FK_XR_VAD_I2S_TX] = {PORT_RATE_16KHZ, PORT_BITS_16, PORT_CHANNELS_2},

	[FK_XR_MI2S0_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2},

	[FK_XR_AUDIO_USB_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_2,
		PCM_FORMAT_S16_LE},
	[FK_XR_AUDIO_USB_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_1,
		PCM_FORMAT_S16_LE},

	[FK_XR_TDM0_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM0_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM1_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM1_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM2_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM2_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM3_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM3_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM4_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM4_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM5_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM5_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM6_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM6_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM7_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_8},
	[FK_XR_TDM7_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_8},
	[FK_XR_TDM8_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_8},
	[FK_XR_TDM8_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_8},
	[FK_XR_TDM9_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM9_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_4},
	[FK_XR_TDM7_8_RX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_8},
	[FK_XR_TDM7_8_TX] = {PORT_RATE_48KHZ, PORT_BITS_16, PORT_CHANNELS_8},
};

static const struct snd_soc_dapm_route i2s_route_playback[] = {
	{"XR_I2S1 Playback", NULL, "XR_I2S1_RX"},
	{"XR_I2S2 Playback", NULL, "XR_I2S2_RX"},
	{"XR_I2S3 Playback", NULL, "XR_I2S3_RX"},
#ifdef AUDIO_ROUTES_RESERVE
	{"XR_I2S0 Playback", NULL, "XR_I2S0_RX"},
	{"XR_I2S4 Playback", NULL, "XR_I2S4_RX"},
	{"XR_I2S5 Playback", NULL, "XR_I2S5_RX"},
	{"XR_I2S6 Playback", NULL, "XR_I2S6_RX"},
	{"XR_I2S7 Playback", NULL, "XR_I2S7_RX"},
	{"XR_I2S8 Playback", NULL, "XR_I2S8_RX"},
	{"XR_I2S9 Playback", NULL, "XR_I2S9_RX"},
#endif
};

static const struct snd_soc_dapm_route i2s_route_capture[] = {
	{"XR_I2S1_TX", NULL, "XR_I2S1 Capture"},
	{"XR_I2S2_TX", NULL, "XR_I2S2 Capture"},
	{"XR_I2S3_TX", NULL, "XR_I2S3 Capture"},
	{"XR_I2S9_TX", NULL, "XR_I2S9 Capture"},
#ifdef AUDIO_ROUTES_RESERVE
	{"XR_I2S0_TX", NULL, "XR_I2S0 Capture"},
	{"XR_I2S4_TX", NULL, "XR_I2S4 Capture"},
	{"XR_I2S5_TX", NULL, "XR_I2S5 Capture"},
	{"XR_I2S6_TX", NULL, "XR_I2S6 Capture"},
	{"XR_I2S7_TX", NULL, "XR_I2S7 Capture"},
	{"XR_I2S8_TX", NULL, "XR_I2S8 Capture"},
#endif
};

struct dai_format *fk_aif_dev_cfg_get(int id)
{
	if (id >= FK_XR_TDM_MAX)
		return NULL;

	return (struct dai_format *)&aif_dev_cfg[id];
}

/*mi2s0 for DP*/
static const struct snd_soc_dapm_route mi2s0_route_playback[] = {
	{"XR_mi2s0 Playback", NULL, "XR_MI2S0_RX"},
};

int fk_dai_mi2s0_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int port_id = dai->id;
	int rc = 0;

	if ((!codec_dai) || (!codec_dai->driver) ||
		(!codec_dai->driver->ops) || (!codec_dai->driver->ops->set_fmt))
		return 0;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);
	if (port_id <= FK_XR_I2S_MAX) {
		rc = snd_soc_dai_set_fmt(codec_dai,
			SND_SOC_DAIFMT_CBC_CFC | SND_SOC_DAIFMT_I2S);
		if (rc) {
			AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
			return -1;
		}
	}

	return 0;
}

int fk_dai_mi2s0_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params, struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "number=%d", substream->number);
	return 0;
}

int fk_dai_mi2s0_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct dai_format *aif_cfg = NULL;
	struct dp_audio_fmt_info fmt_info;
	int ret = 0;

	memset(&fmt_info, 0x0, sizeof(struct dp_audio_fmt_info));

	aif_cfg = fk_aif_dev_cfg_get(dai->id);

	if (aif_cfg == NULL) {
		AUD_LOG_INFO(AUD_SOC, "dai id (%d), use default value", dai->id);
		fmt_info.ch_num = PORT_CHANNELS_2;
		fmt_info.bit_width = PORT_BITS_16;
		fmt_info.samp_rate = PORT_RATE_48KHZ;
	} else {
		fmt_info.ch_num = aif_cfg->channels;
		fmt_info.bit_width = aif_cfg->bit_width;
		fmt_info.samp_rate = aif_cfg->samples;
	}
	ret = dp_enable_audio(&fmt_info, false);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "disable DP devices failed");

	return ret;
}
int fk_dai_mi2s0_prepare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct dai_format *aif_cfg = NULL;
	struct dp_audio_fmt_info fmt_info;
	int ret = 0;

	memset(&fmt_info, 0x0, sizeof(struct dp_audio_fmt_info));

	aif_cfg = fk_aif_dev_cfg_get(dai->id);

	if (aif_cfg == NULL) {
		AUD_LOG_INFO(AUD_SOC, "dai id (%d), use default value", dai->id);
		fmt_info.ch_num = PORT_CHANNELS_2;
		fmt_info.bit_width = PORT_BITS_16;
		fmt_info.samp_rate = PORT_RATE_48KHZ;
	} else {
		fmt_info.ch_num = aif_cfg->channels;
		fmt_info.bit_width = aif_cfg->bit_width;
		fmt_info.samp_rate = aif_cfg->samples;
	}
	AUD_LOG_DBG(AUD_SOC, "ch_num (%d), bit_width (%d),samp_rate (%d)", fmt_info.ch_num, fmt_info.bit_width, fmt_info.samp_rate);
	ret = dp_enable_audio(&fmt_info, true);
	if (ret < 0)
		AUD_LOG_ERR(AUD_SOC, "disable DP devices failed");

	return ret;
}
int fk_dai_mi2s0_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}


void fk_dai_mi2s0_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	int port_id = dai->id;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);
}

static int fk_dai_mi2s0_probe(struct snd_soc_dai *dai)
{
	struct snd_soc_dapm_context *dapm;
	int ret = 0;

	dapm = snd_soc_component_get_dapm(dai->component);
	snd_soc_dapm_add_routes(dapm, mi2s0_route_playback, ARRAY_SIZE(mi2s0_route_playback));
	return ret;
}

static int fk_dai_mi2s0_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static struct snd_soc_dai_ops fk_dai_mi2s0_ops = {
	.probe = fk_dai_mi2s0_probe,
	.remove = fk_dai_mi2s0_remove,
	.startup	= fk_dai_mi2s0_startup,
	.prepare	= fk_dai_mi2s0_prepare,
	.hw_params	= fk_dai_mi2s0_hw_params,
	.hw_free	= fk_dai_mi2s0_hw_free,
	.set_fmt	= fk_dai_mi2s0_set_fmt,
	.shutdown	= fk_dai_mi2s0_shutdown,
};
/*end mi2s0 for DP*/

static int fk_dai_i2s_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = of_platform_populate(pdev->dev.of_node, NULL, NULL, &pdev->dev);
	if (ret)
		AUD_LOG_ERR(AUD_SOC, "failed to add child nodes, ret=%d", ret);
	else
		AUD_LOG_INFO(AUD_SOC, "added child node");

	return ret;
}
static int fk_dai_i2s_remove(struct platform_device *pdev)
{
	return 0;
}

static int fk_dai_multi_i2s_probe(struct snd_soc_dai *dai)
{
	struct snd_soc_dapm_context *dapm;

	dapm = snd_soc_component_get_dapm(dai->component);
	snd_soc_dapm_add_routes(dapm, i2s_route_playback, ARRAY_SIZE(i2s_route_playback));
	snd_soc_dapm_add_routes(dapm, i2s_route_capture, ARRAY_SIZE(i2s_route_capture));
	return 0;
}

static int fk_dai_multi_i2s_remove(struct snd_soc_dai *dai)
{
	return 0;
}

int fk_dai_multi_i2s_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int port_id = dai->id;
	int rc = 0;

	if ((!codec_dai) || (!codec_dai->driver) ||
		(!codec_dai->driver->ops) || (!codec_dai->driver->ops->set_fmt))
		return 0;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);

	if (port_id <= FK_XR_I2S_MAX) {
		rc = snd_soc_dai_set_fmt(codec_dai,
			SND_SOC_DAIFMT_CBC_CFC | SND_SOC_DAIFMT_I2S);
		if (rc) {
			AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
			return -1;
		}
	}

	return 0;
}
void fk_dai_multi_i2s_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	int port_id = dai->id;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);
}
int fk_dai_multi_i2s_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params, struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "number=%d", substream->number);
	return 0;
}
int fk_dai_multi_i2s_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "number=%d", substream->number);
	return 0;
}
int fk_dai_multi_i2s_prepare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}
int fk_dai_multi_i2s_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	AUD_LOG_DBG(AUD_SOC, "fmt=%d", fmt);
	return 0;
}

static struct snd_soc_dai_ops fk_dai_multi_i2s_ops = {
	.probe = fk_dai_multi_i2s_probe,
	.remove = fk_dai_multi_i2s_remove,
	.startup	= fk_dai_multi_i2s_startup,
	.prepare	= fk_dai_multi_i2s_prepare,
	.hw_params	= fk_dai_multi_i2s_hw_params,
	.hw_free	= fk_dai_multi_i2s_hw_free,
	.set_fmt	= fk_dai_multi_i2s_set_fmt,
	.shutdown	= fk_dai_multi_i2s_shutdown,
};

/* Channel min and max are initialized base on platform data */
static struct snd_soc_dai_driver fk_dai_multi_i2s_dai[] = {
/*multi I2S*/
/*I2S0*/
	{
		.playback = {
			.stream_name = "XR_I2S0 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S0 playback",
		.id = FK_XR_I2S0_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S0 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S0 capture",
		.id = FK_XR_I2S0_TX,
	},
/*I2S1*/
	{
		.playback = {
			.stream_name = "XR_I2S1 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S1 playback",
		.id = FK_XR_I2S1_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S1 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S1 capture",
		.id = FK_XR_I2S1_TX,
	},
/*I2S2*/
	{
		.playback = {
			.stream_name = "XR_I2S2 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S2 playback",
		.id = FK_XR_I2S2_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S2 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S2 capture",
		.id = FK_XR_I2S2_TX,
	},
/*I2S3*/
	{
		.playback = {
			.stream_name = "XR_I2S3 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S3 playback",
		.id = FK_XR_I2S3_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S3 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S3 capture",
		.id = FK_XR_I2S3_TX,
	},
/*I2S4*/
	{
		.playback = {
			.stream_name = "XR_I2S4 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S4 playback",
		.id = FK_XR_I2S4_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S4 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S4 capture",
		.id = FK_XR_I2S4_TX,
	},
/*I2S5*/
	{
		.playback = {
			.stream_name = "XR_I2S5 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S5 playback",
		.id = FK_XR_I2S5_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S5 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S5 capture",
		.id = FK_XR_I2S5_TX,
	},
/*I2S6*/
	{
		.playback = {
			.stream_name = "XR_I2S6 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S6 playback",
		.id = FK_XR_I2S6_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S6 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S6 capture",
		.id = FK_XR_I2S6_TX,
	},
/*I2S7*/
	{
		.playback = {
			.stream_name = "XR_I2S7 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S7 playback",
		.id = FK_XR_I2S7_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S7 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S7 capture",
		.id = FK_XR_I2S7_TX,
	},
/*I2S8*/
	{
		.playback = {
			.stream_name = "XR_I2S8 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S8 playback",
		.id = FK_XR_I2S8_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S8 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S8 capture",
		.id = FK_XR_I2S8_TX,
	},
/*I2S9*/
	{
		.playback = {
			.stream_name = "XR_I2S9 Playback",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S9 playback",
		.id = FK_XR_I2S9_RX,
	},
	{
		.capture = {
			.stream_name = "XR_I2S9 Capture",
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
		.ops = &fk_dai_multi_i2s_ops,
		.name = "XR I2S9 capture",
		.id = FK_XR_I2S9_TX,
	},
};

static const struct snd_soc_component_driver fk_multi_i2s_dai_component = {
	.name		= "fk-dai-multi-i2s",
};
static int fk_dai_i2s_dev_probe(struct platform_device *pdev)
{
	const char *multi_i2s_dev_id = "xring,fk-dai-multi-i2s-dev-id";
	u32 i2s_intf = 0;
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, multi_i2s_dev_id,
				  &i2s_intf);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "missing 0x%x in dt node", i2s_intf);
		goto rtn;
	}

	//AUD_LOG_INFO(AUD_SOC, "dev name %s dev id 0x%x", dev_name(&pdev->dev), i2s_intf);

	pdev->id = i2s_intf;

	ret = snd_soc_register_component(&pdev->dev, &fk_multi_i2s_dai_component,
		&fk_dai_multi_i2s_dai[i2s_intf], 1);
	if (ret < 0)
		goto err_register;
	return 0;

err_register:
	dev_err(&pdev->dev, "fail to fk_dai_multi_i2s_dev_probe\n");
rtn:
	return ret;
}

static int fk_dai_i2s_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct snd_soc_dapm_route tdm_route_playback[] = {
#ifdef AUDIO_ROUTES_RESERVE
	{"XR_TDM0 Playback", NULL, "XR_TDM0_RX"},
	{"XR_TDM1 Playback", NULL, "XR_TDM1_RX"},
	{"XR_TDM2 Playback", NULL, "XR_TDM2_RX"},
	{"XR_TDM3 Playback", NULL, "XR_TDM3_RX"},
	{"XR_TDM4 Playback", NULL, "XR_TDM4_RX"},
	{"XR_TDM5 Playback", NULL, "XR_TDM5_RX"},
	{"XR_TDM6 Playback", NULL, "XR_TDM6_RX"},
#endif
	{"XR_TDM7 Playback", NULL, "XR_TDM7_RX"},
	{"XR_TDM8 Playback", NULL, "XR_TDM8_RX"},
	{"XR_TDM9 Playback", NULL, "XR_TDM9_RX"},
	{"XR_TDM7_8 Playback", NULL, "XR_TDM7_8_RX"},
};
static const struct snd_soc_dapm_route tdm_route_capture[] = {
#ifdef AUDIO_ROUTES_RESERVE
	{"XR_TDM0_TX", NULL, "XR_TDM0 Capture"},
	{"XR_TDM1_TX", NULL, "XR_TDM1 Capture"},
	{"XR_TDM2_TX", NULL, "XR_TDM2 Capture"},
	{"XR_TDM3_TX", NULL, "XR_TDM3 Capture"},
	{"XR_TDM4_TX", NULL, "XR_TDM4 Capture"},
	{"XR_TDM5_TX", NULL, "XR_TDM5 Capture"},
	{"XR_TDM6_TX", NULL, "XR_TDM6 Capture"},
#endif
	{"XR_TDM7_TX", NULL, "XR_TDM7 Capture"},
	{"XR_TDM8_TX", NULL, "XR_TDM8 Capture"},
	{"XR_TDM9_TX", NULL, "XR_TDM9 Capture"},
	{"XR_TDM7_8 Playback", NULL, "XR_TDM7_8_RX"},
};

static int fk_dai_tdm_probe(struct snd_soc_dai *dai)
{
	struct snd_soc_dapm_context *dapm;

	dapm = snd_soc_component_get_dapm(dai->component);
	snd_soc_dapm_add_routes(dapm, tdm_route_playback, ARRAY_SIZE(tdm_route_playback));
	snd_soc_dapm_add_routes(dapm, tdm_route_capture, ARRAY_SIZE(tdm_route_capture));
	return 0;
}

static int fk_dai_tdm_remove(struct snd_soc_dai *dai)
{
	return 0;
}

int fk_dai_tdm_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int port_id = dai->id;
	struct dai_format *aif_cfg = NULL;
	int channels, phy_width = PORT_PHYSICAL_WIDTH_BIT;
	unsigned int slot_mask;
	int rc = 0;

	if ((!codec_dai) || (!codec_dai->driver) ||
		(!codec_dai->driver->ops))
		return 0;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);

	aif_cfg = fk_aif_dev_cfg_get(dai->id);
	if (aif_cfg == NULL) {
		AUD_LOG_INFO(AUD_SOC, "dai id (%d), use default value", dai->id);
		channels = PORT_CHANNELS_4;
	} else {
		channels = aif_cfg->channels;
	}

	if (port_id <= FK_XR_TDM_MAX && port_id >= FK_XR_TDM_BEGIN) {
		if (codec_dai->driver->ops->set_fmt) {
			rc = snd_soc_dai_set_fmt(codec_dai,
				SND_SOC_DAIFMT_CBC_CFC | SND_SOC_DAIFMT_DSP_A);
			if (rc) {
				AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
				return rc;
			}
		}

		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			slot_mask = 0xFFFFFFFF >> (32 - channels);
			AUD_LOG_DBG(AUD_SOC, "slot_mask=0x%x channels=%d phy_width=%d",
				slot_mask, channels, phy_width);
			if (codec_dai->driver->ops->set_tdm_slot) {
				rc = snd_soc_dai_set_tdm_slot(codec_dai, 0, slot_mask,
					channels, phy_width);
				if (rc) {
					AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
					return rc;
				}
			}
		} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
			slot_mask = 0xFFFFFFFF >> (32 - channels);
			AUD_LOG_DBG(AUD_SOC, "slot_mask=0x%x channels=%d phy_width=%d",
				slot_mask, channels, phy_width);
			if (codec_dai->driver->ops->set_tdm_slot) {
				rc = snd_soc_dai_set_tdm_slot(codec_dai, slot_mask, 0,
					channels, phy_width);
				if (rc) {
					AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
					return rc;
				}
			}
		}
	}

	return 0;
}
void fk_dai_tdm_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int port_id = dai->id;
	int phy_width = PORT_PHYSICAL_WIDTH_BIT;
	int rc = 0;

	if ((!codec_dai) || (!codec_dai->driver) ||
		(!codec_dai->driver->ops))
		return;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (codec_dai->driver->ops->set_tdm_slot) {
			rc = snd_soc_dai_set_tdm_slot(codec_dai, 0, 0, 0, phy_width);
			if (rc) {
				AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
				return;
			}
		}
	} else if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if (codec_dai->driver->ops->set_tdm_slot) {
			rc = snd_soc_dai_set_tdm_slot(codec_dai, 0, 0, 0, phy_width);
			if (rc) {
				AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
				return;
			}
		}
	}
}
int fk_dai_tdm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params, struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "number=%d", substream->number);
	return 0;
}
int fk_dai_tdm_hw_free(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "number=%d", substream->number);
	return 0;
}
int fk_dai_tdm_prepare(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}
int fk_dai_tdm_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	AUD_LOG_DBG(AUD_SOC, "fmt=%d", fmt);
	return 0;
}

static struct snd_soc_dai_ops fk_dai_tdm_ops = {
	.probe = fk_dai_tdm_probe,
	.remove = fk_dai_tdm_remove,
	.startup	= fk_dai_tdm_startup,
	.prepare	= fk_dai_tdm_prepare,
	.hw_params	= fk_dai_tdm_hw_params,
	.hw_free	= fk_dai_tdm_hw_free,
	.set_fmt	= fk_dai_tdm_set_fmt,
	.shutdown	= fk_dai_tdm_shutdown,
};

static struct snd_soc_dai_driver fk_dai_tdm_dai[] = {
/* tdm */
/* tdm0 */
	{
		.playback = {
			.stream_name = "XR_TDM0 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM0 playback",
		.id = FK_XR_TDM0_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM0 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM0 capture",
		.id = FK_XR_TDM0_TX,
	},
/* tdm1 */
	{
		.playback = {
			.stream_name = "XR_TDM1 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM1 playback",
		.id = FK_XR_TDM1_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM1 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM1 capture",
		.id = FK_XR_TDM1_TX,
	},
/* tdm2 */
	{
		.playback = {
			.stream_name = "XR_TDM2 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM2 playback",
		.id = FK_XR_TDM2_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM2 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM2 capture",
		.id = FK_XR_TDM2_TX,
	},
/* tdm3 */
	{
		.playback = {
			.stream_name = "XR_TDM3 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM3 playback",
		.id = FK_XR_TDM3_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM3 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM3 capture",
		.id = FK_XR_TDM3_TX,
	},
/* tdm4 */
	{
		.playback = {
			.stream_name = "XR_TDM4 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM4 playback",
		.id = FK_XR_TDM4_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM4 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM4 capture",
		.id = FK_XR_TDM4_TX,
	},
/* tdm5 */
	{
		.playback = {
			.stream_name = "XR_TDM5 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM5 playback",
		.id = FK_XR_TDM5_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM5 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM5 capture",
		.id = FK_XR_TDM5_TX,
	},
/* tdm6 */
	{
		.playback = {
			.stream_name = "XR_TDM6 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM6 playback",
		.id = FK_XR_TDM6_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM6 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM6 capture",
		.id = FK_XR_TDM6_TX,
	},
/* tdm7 */
	{
		.playback = {
			.stream_name = "XR_TDM7 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM7 playback",
		.id = FK_XR_TDM7_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM7 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM7 capture",
		.id = FK_XR_TDM7_TX,
	},
/* tdm8 */
	{
		.playback = {
			.stream_name = "XR_TDM8 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM8 playback",
		.id = FK_XR_TDM8_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM8 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM8 capture",
		.id = FK_XR_TDM8_TX,
	},
/* tdm9 */
	{
		.playback = {
			.stream_name = "XR_TDM9 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM9 playback",
		.id = FK_XR_TDM9_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM9 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM9 capture",
		.id = FK_XR_TDM9_TX,
	},
/* tdm7_8 */
	{
		.playback = {
			.stream_name = "XR_TDM7_8 Playback",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM7_8 playback",
		.id = FK_XR_TDM7_8_RX,
	},
	{
		.capture = {
			.stream_name = "XR_TDM7_8 Capture",
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
		.ops = &fk_dai_tdm_ops,
		.name = "XR TDM7_8 capture",
		.id = FK_XR_TDM7_8_TX,
	},
};

static const struct snd_soc_component_driver fk_tdm_dai_component = {
	.name		= "fk-dai-tdm",
};

static int fk_dai_tdm_dev_probe(struct platform_device *pdev)
{
	const char *tdm_dev_id = "xring,fk-dai-tdm-dev-id";
	u32 tdm_intf = 0;
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, tdm_dev_id,
				  &tdm_intf);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "missing 0x%x in dt node", tdm_intf);
		goto rtn;
	}

	//AUD_LOG_INFO(AUD_SOC, "dev name %s dev id 0x%x", dev_name(&pdev->dev), i2s_intf);

	pdev->id = tdm_intf;

	ret = snd_soc_register_component(&pdev->dev, &fk_tdm_dai_component,
		&fk_dai_tdm_dai[tdm_intf], 1);
	if (ret < 0)
		goto err_register;
	return 0;

err_register:
	dev_err(&pdev->dev, "fail to %s\n", __func__);
rtn:
	return ret;
}

static int fk_dai_tdm_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct snd_soc_dapm_route vad_route_capture[] = {
	{"XR_VAD_I2S_TX", NULL, "XR_VAD_I2S Capture"},
	// {"XR_VAD_PDM_TX", NULL, "XR_VAD_PDM capture"},
};

static int fk_dai_vad_i2s_probe(struct snd_soc_dai *dai)
{
	struct snd_soc_dapm_context *dapm;

	dapm = snd_soc_component_get_dapm(dai->component);
	snd_soc_dapm_add_routes(dapm, vad_route_capture, ARRAY_SIZE(vad_route_capture));
	return 0;
}


static int fk_dai_vad_i2s_remove(struct snd_soc_dai *dai)
{
	return 0;
}


static const struct snd_soc_component_driver fk_vad_i2s_dai_component = {
	.name		= "fk-dai-vad-i2s",
};

int fk_dai_vad_i2s_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	int port_id = dai->id;
	int rc = 0;

	if ((!codec_dai) || (!codec_dai->driver) ||
		(!codec_dai->driver->ops) || (!codec_dai->driver->ops->set_fmt))
		return 0;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);

	if (port_id <= FK_XR_I2S_MAX) {
		rc = snd_soc_dai_set_fmt(codec_dai,
			SND_SOC_DAIFMT_CBC_CFC | SND_SOC_DAIFMT_I2S);
		if (rc) {
			AUD_LOG_INFO(AUD_SOC, "codec dai set fmt fail");
			return -1;
		}
	}

	return 0;
}
void fk_dai_vad_i2s_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	int port_id = dai->id;

	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d port_id=%d",
		substream->name, substream->stream, substream->number, port_id);
}

static struct snd_soc_dai_ops fk_dai_vad_i2s_ops = {
	.probe = fk_dai_vad_i2s_probe,
	.remove = fk_dai_vad_i2s_remove,
	.startup	= fk_dai_vad_i2s_startup,
	.shutdown	= fk_dai_vad_i2s_shutdown,
};

static struct snd_soc_dai_driver fk_dai_vad_i2s_dai = {
				.capture = {
					.stream_name = "XR_VAD_I2S Capture",
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
				.ops = &fk_dai_vad_i2s_ops,
				.name = "XR VAD_I2S capture",
				.id = FK_XR_VAD_I2S_TX,
};

/*DP audio block_info form display driver*/
int get_audio_edid_blk_size(struct fk_disp_audio_edid_blk *blk)
{
	blk->audio_data_blk_size = sizeof(u8)*CTA_AUDIO_DATA_BLOCK_SIZE;
	blk->spk_alloc_data_blk_size = sizeof(u8)*CTA_SPEAKER_ALLOCATION_DATA_BLOCK_SIZE;
	return 0;
}
int get_audio_edid_blk(struct dp_audio_cta_blk *dp_blk)
{
	int ret = 0;

	ret  = dp_get_audio_cta_blk(dp_blk);
	if (ret < 0) {
		AUD_LOG_INFO(AUD_SOC, "get audio cta blk failed");
		return -EINVAL;
	}
	return ret;
}

static int fk_disp_edid_ctl_info(struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_info *uinfo)
{
	int ret = 0;
	struct fk_disp_audio_edid_blk edid_blk;

	memset(&edid_blk, 0x0, sizeof(struct fk_disp_audio_edid_blk));

	ret  = get_audio_edid_blk_size(&edid_blk);
	if (ret >= 0) {
		uinfo->type = SNDRV_CTL_ELEM_TYPE_BYTES;
		uinfo->count = edid_blk.audio_data_blk_size + edid_blk.spk_alloc_data_blk_size;
	}
	return ret;
}

static int fk_disp_edid_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct fk_disp_audio_edid_blk edid_blk;

	memset(&edid_blk, 0x0, sizeof(struct fk_disp_audio_edid_blk));
	/*get blk size*/

	get_audio_edid_blk_size(&edid_blk);
	ret  = get_audio_edid_blk(&edid_blk.dp_blk);
	if (ret >= 0) {
		memcpy(ucontrol->value.bytes.data,
		       &edid_blk.dp_blk.audio_data_blk,
		       edid_blk.audio_data_blk_size);
		memcpy((ucontrol->value.bytes.data +
		       edid_blk.audio_data_blk_size),
		       &edid_blk.dp_blk.spk_alloc_data_blk,
		       edid_blk.spk_alloc_data_blk_size);
	}
		AUD_LOG_DBG(AUD_SOC, "audio_data_blk:%d,%d, spk_alloc_data_blk:%d, %d",
			edid_blk.dp_blk.audio_data_blk[0],
			edid_blk.dp_blk.audio_data_blk[1],
			edid_blk.dp_blk.spk_alloc_data_blk[0],
			edid_blk.dp_blk.spk_alloc_data_blk[1]);
		AUD_LOG_DBG(AUD_SOC, "data_blk_size:%d, spk_alloc_data_blk_size:%d",
			edid_blk.audio_data_blk_size,
			edid_blk.spk_alloc_data_blk_size);
	return 0;
}

/*DP block info kctl*/
static const struct snd_kcontrol_new fk_disp_codec_rx_controls[] = {
	{
		.access = SNDRV_CTL_ELEM_ACCESS_READ |
			  SNDRV_CTL_ELEM_ACCESS_VOLATILE,
		.iface  = SNDRV_CTL_ELEM_IFACE_PCM,
		.name   = "Display Port EDID",
		.info   = fk_disp_edid_ctl_info,
		.get    = fk_disp_edid_get,
	},
};

/*mi2s0 component for DP*/
static const struct snd_soc_component_driver fk_mi2s0_dai_component = {
	.name		= "fk-dai-mi2s0",
	//.probe = fk_disp_audio_codec_rx_probe,
	//.remove =  fk_disp_audio_codec_rx_remove,
	.controls = fk_disp_codec_rx_controls,
	.num_controls = ARRAY_SIZE(fk_disp_codec_rx_controls),
};

/*mi2s0 dai for DP*/
static struct snd_soc_dai_driver fk_dai_mi2s0_dai = {
		.playback = {
			.stream_name = "XR_mi2s0 Playback",
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
		.ops = &fk_dai_mi2s0_ops,
		.name = "XR MI2S0 playback",
		.id = FK_XR_MI2S0_RX,
};

static int fk_dai_vad_i2s_dev_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = snd_soc_register_component(&pdev->dev, &fk_vad_i2s_dai_component,
	&fk_dai_vad_i2s_dai, 1);
	if (ret < 0)
		goto err_register;
	return ret;

err_register:
	AUD_LOG_ERR(AUD_SOC, "fail to %s\n", __func__);
	return ret;
}

static int fk_dai_vad_i2s_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static int fk_dai_mi2s0_dev_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = snd_soc_register_component(&pdev->dev, &fk_mi2s0_dai_component,
	&fk_dai_mi2s0_dai, 1);
	if (ret < 0)
		goto err_register;
	return ret;

err_register:
	AUD_LOG_ERR(AUD_SOC, "fail to %s\n", __func__);
	return ret;
}

static int fk_dai_mi2s0_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

/*register mi2s0 for DP*/
static const struct of_device_id fk_dai_mi2s0_dev_dt_match[] = {
	{ .compatible = "xring,fk-dai-mi2s0", },
	{ }
};

MODULE_DEVICE_TABLE(of, fk_dai_mi2s0_dev_dt_match);

static struct platform_driver fk_dai_mi2s0_driver = {
	.probe  = fk_dai_mi2s0_dev_probe,
	.remove  = fk_dai_mi2s0_dev_remove,
	.driver = {
		.name = "fk-dai-dev-mi2s0",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_mi2s0_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

static const struct of_device_id fk_dai_i2s_dt_match[] = {
	{ .compatible = "xring,fk-dai-i2s", },
	{ }
};

MODULE_DEVICE_TABLE(of, fk_dai_i2s_dt_match);

static struct platform_driver fk_dai_i2s = {
	.probe  = fk_dai_i2s_probe,
	.remove = fk_dai_i2s_remove,
	.driver = {
		.name = "fk-dai-i2s",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_i2s_dt_match,
		.suppress_bind_attrs = true,
	},
};

static const struct of_device_id fk_dai_i2s_dev_dt_match[] = {
	{ .compatible = "xring,fk-dai-multi-i2s", },
	{ }
};

MODULE_DEVICE_TABLE(of, fk_dai_i2s_dev_dt_match);

static struct platform_driver fk_dai_i2s_driver = {
	.probe  = fk_dai_i2s_dev_probe,
	.remove  = fk_dai_i2s_dev_remove,
	.driver = {
		.name = "fk-dai-dev-i2s",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_i2s_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

static const struct of_device_id fk_dai_vad_i2s_dev_dt_match[] = {
	{ .compatible = "xring,fk-dai-vad-i2s", },
	{ }
};

MODULE_DEVICE_TABLE(of, fk_dai_vad_i2s_dev_dt_match);

static struct platform_driver fk_dai_vad_i2s_driver = {
	.probe  = fk_dai_vad_i2s_dev_probe,
	.remove  = fk_dai_vad_i2s_dev_remove,
	.driver = {
		.name = "fk-dai-dev-vad-i2s",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_vad_i2s_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

static const struct of_device_id fk_dai_tdm_dev_dt_match[] = {
	{ .compatible = "xring,fk-dai-multi-tdm", },
	{ }
};

MODULE_DEVICE_TABLE(of, fk_dai_tdm_dev_dt_match);

static struct platform_driver fk_dai_tdm_driver = {
	.probe  = fk_dai_tdm_dev_probe,
	.remove  = fk_dai_tdm_dev_remove,
	.driver = {
		.name = "fk-dai-dev-tdm",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_tdm_dev_dt_match,
		.suppress_bind_attrs = true,
	},
};

#ifdef FK_AUDIO_USB_OFFLOAD

static const struct snd_soc_dapm_route fk_dai_usb_dapm_routes[] = {
	{"XR_AUDIO_USB Playback", NULL, "XR_AUDIO_USB_RX"},
	{"XR_AUDIO_USB_TX", NULL, "XR_AUDIO_USB Capture"},
};

static int fk_dai_audio_usb_probe(struct snd_soc_dai *dai)
{
	int ret = 0;
	struct snd_soc_dapm_context *dapm;

	AUD_LOG_INFO(AUD_SOC, "dai name(%s)", dai->name);

	dapm = snd_soc_component_get_dapm(dai->component);
	snd_soc_dapm_add_routes(dapm, fk_dai_usb_dapm_routes, ARRAY_SIZE(fk_dai_usb_dapm_routes));

	ret = fk_dai_audio_usb_dai_probe(dai);
	if (ret)
		AUD_LOG_INFO(AUD_SOC, "register usb jack. ret=%d", ret);

	return 0;
}

static int fk_dai_audio_usb_remove(struct snd_soc_dai *dai)
{
	return 0;
}

int fk_dai_audio_usb_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "name=%s stream=%d number=%d",
		substream->name, substream->stream, substream->number);
	return 0;
}
void fk_dai_audio_usb_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	AUD_LOG_DBG(AUD_SOC, "number=%d", substream->number);
}

static const struct snd_soc_dai_ops fk_dai_audio_usb_rx_ops = {
	.probe = fk_dai_audio_usb_probe,
	.remove = fk_dai_audio_usb_remove,
	.startup	= fk_dai_audio_usb_startup,
	.shutdown	= fk_dai_audio_usb_shutdown,
};

//static const struct snd_soc_dai_ops fk_dai_audio_usb_tx_ops = {
//	.startup	= fk_dai_audio_usb_startup,
//	.shutdown	= fk_dai_audio_usb_shutdown,
//};

/* Channel min and max are initialized base on platform data */
static struct snd_soc_dai_driver fk_dai_audio_usb[] = {
	{
		.playback = {
			.stream_name = "XR_AUDIO_USB Playback",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		.ops = &fk_dai_audio_usb_rx_ops,
		.name = "AUDIO USB Playback",
		.id = FK_XR_AUDIO_USB_RX,
	},
	{
		.capture = {
			.stream_name = "XR_AUDIO_USB Capture",
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
						SNDRV_PCM_FMTBIT_S24_LE |
						SNDRV_PCM_FMTBIT_S24_3LE |
						SNDRV_PCM_FMTBIT_S32_LE),
			.channels_min = 1,
			.channels_max = 2,
			.rate_min =     8000,
			.rate_max =	384000,
		},
		//.ops = &fk_dai_audio_usb_tx_ops,
		.name = "AUDIO USB Capture",
		.id = FK_XR_I2S0_TX,
	}
};

static const struct snd_soc_component_driver fk_dai_audio_usb_component = {
	.name = "fk-dai-audio-usb",
};

static int fk_dai_audio_usb_dev_probe(struct platform_device *pdev)
{
	const char *audio_usb_dai_id = "xring,fk-dai-audio-usb-id";
	u32 dai_id = 0;
	int ret;

	ret = of_property_read_u32(pdev->dev.of_node, audio_usb_dai_id,
				&dai_id);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "missing 0x%x in dt node", dai_id);
		goto rtn;
	}

	pdev->id = dai_id;

	AUD_LOG_INFO(AUD_SOC, "dev name %s. id %d", dev_name(&pdev->dev), dai_id);

	ret = snd_soc_register_component(&pdev->dev, &fk_dai_audio_usb_component,
		&fk_dai_audio_usb[dai_id], 1);
	if (ret < 0)
		goto err_register;
	return 0;

err_register:
	AUD_LOG_ERR(AUD_SOC, "fail to %s\n", __func__);
rtn:
	return ret;
}

static int fk_dai_audio_usb_dev_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static const struct of_device_id fk_dai_audio_usb_dt_match[] = {
	{ .compatible = "xring,fk-dai-audio-usb", },
	{ }
};

MODULE_DEVICE_TABLE(of, fk_dai_audio_usb_dt_match);

static struct platform_driver fk_dai_audio_usb_driver = {
	.probe  = fk_dai_audio_usb_dev_probe,
	.remove  = fk_dai_audio_usb_dev_remove,
	.driver = {
		.name = "fk-dai-audio-usb-dev",
		.owner = THIS_MODULE,
		.of_match_table = fk_dai_audio_usb_dt_match,
		.suppress_bind_attrs = true,
	},
};
#endif

int __init fk_be_dai_init(void)
{
	int ret;

	ret = platform_driver_register(&fk_dai_i2s);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "fail to register dai i2s");
		goto out;
	}
	ret = platform_driver_register(&fk_dai_i2s_driver);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "fail to register dai i2s dev drv");
		goto dai_i2s_fail;
	}
	ret = platform_driver_register(&fk_dai_vad_i2s_driver);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "fail to register dai vad i2s dev drv");
		goto dai_i2s_drv_fail;
	}
	ret = platform_driver_register(&fk_dai_tdm_driver);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "fail to register dai tdm dev drv");
		goto dai_vad_i2s_drv_fail;
	}
	ret = platform_driver_register(&fk_dai_mi2s0_driver);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "fail to register dai mutil i2s0 dev drv\n");
		goto dai_tdm_drv_fail;
	}
#ifdef FK_AUDIO_USB_OFFLOAD
	ret = platform_driver_register(&fk_dai_audio_usb_driver);
	if (ret) {
		AUD_LOG_ERR(AUD_SOC, "fail to register dai audio usb dev drv");
		goto dai_mi2s0_drv_fail;
	}
#endif

	return ret;

dai_mi2s0_drv_fail:
	platform_driver_unregister(&fk_dai_mi2s0_driver);
dai_tdm_drv_fail:
	platform_driver_unregister(&fk_dai_tdm_driver);
dai_vad_i2s_drv_fail:
	platform_driver_unregister(&fk_dai_vad_i2s_driver);
dai_i2s_drv_fail:
	platform_driver_unregister(&fk_dai_i2s_driver);
dai_i2s_fail:
	platform_driver_unregister(&fk_dai_i2s);

out:
	return ret;
}
void fk_be_dai_exit(void)
{
	platform_driver_unregister(&fk_dai_i2s);
	platform_driver_unregister(&fk_dai_i2s_driver);
	platform_driver_unregister(&fk_dai_vad_i2s_driver);
	platform_driver_unregister(&fk_dai_tdm_driver);
	platform_driver_unregister(&fk_dai_audio_usb_driver);
	platform_driver_unregister(&fk_dai_mi2s0_driver);
}

/* Module information */
MODULE_DESCRIPTION("FK DSP DAI driver");
MODULE_LICENSE("Dual BSD/GPL");
