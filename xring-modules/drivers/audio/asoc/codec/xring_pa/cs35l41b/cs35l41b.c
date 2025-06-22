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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <linux/gpio.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/of_irq.h>
#include <linux/completion.h>
#include <linux/spi/spi.h>
#include <linux/err.h>
#include <linux/firmware.h>
#include "cs35l41b_user.h"
#include "cs35l41b.h"

#define DRV_NAME "cs35l41b"

static int cs35l41b_enable_flag = CS35L41B_DISABLE;

static const char *const cs35l41b_supplies[] = {
	"VA",
	"VP",
};

static int cs35l41b_deconfiguration(struct cs35l41b_private *cs35l41b)
{
	int ret;

	ret = regmap_write(cs35l41b->regmap, MSM_GLOBAL_ENABLES_REG, 0x0);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register MSM_GLOBAL_ENABLES_REG failed\n");
		return ret;
	}

	msleep(20);

	return ret;
}

//48K 32bit 2ch
//32k 32bit 2ch
//16K 32bit 2ch
//8K  32bit 2ch
//8K  16bit 2ch
//16K 16bit 2ch
static int cs35l41b_configuration(struct cs35l41b_private *cs35l41b)
{
	int ret = TRUE;
	/* 3.enable the cs35l41b clocking reference */
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_REFCLK_INPUT_REG, 0x430);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_REFCLK_INPUT_REG, 0x3B0);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_REFCLK_INPUT_REG, 0x000002F0);
	ret = regmap_write(cs35l41b->regmap, CS35L41B_REFCLK_INPUT_REG, 0x00000250);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_REFCLK_INPUT_REG, 0x000001F0);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_REFCLK_INPUT_REG, 0x00000250);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_REFCLK_INPUT_REG failed\n");
		return ret;
	}

	/* 4.configure the PCM global sample rate */
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_GLOBAL_SAMPLE_RATE, 0x3);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_GLOBAL_SAMPLE_RATE, 0x13);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_GLOBAL_SAMPLE_RATE, 0x00000012);
	ret = regmap_write(cs35l41b->regmap, CS35L41B_GLOBAL_SAMPLE_RATE, 0x00000011);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_GLOBAL_SAMPLE_RATE, 0x00000011);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_GLOBAL_SAMPLE_RATE, 0x00000012);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_GLOBAL_SAMPLE_RATE failed\n");
		return ret;
	}

	/* 5.configure the ASP_BLCK frequency */
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL1_REG, 0x21);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL1_REG, 0x1D);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL1_REG, 0x00000017);
	ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL1_REG, 0x00000012);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL1_REG, 0x0000000F);
	//ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL1_REG, 0x00000012);

	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_ASP_CONTROL1_REG failed\n");
		return ret;
	}

	/* 6.configure the audio serial port format */
	ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_CONTROL2_REG, 0x20200200);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_ASP_CONTROL2_REG failed\n");
		return ret;
	}

	/* 7.configure the audio serial channel enables */
	ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_ENABLES1_REG, 0x00010003);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_ASP_ENABLES1_REG failed\n");
		return ret;
	}

	/* 8.write the value 0x0002C01C to the register at 0x00002D10 */
	ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_REG_VALUE_ADDR, 0x0002C01C);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_ASP_ENABLES1_REG failed\n");
		return ret;
	}

	/* 9.configure the sub-blocks to enable upon setting GLOBAL_EN */
	ret = regmap_write(cs35l41b->regmap, CS35L41B_ASP_BLOCK_ENABLES, 0x00003721);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register CS35L41B_ASP_BLOCK_ENABLES failed\n");
		return ret;
	}

	ret = regmap_write(cs35l41b->regmap, MSM_GLOBAL_ENABLES_REG, 0x00000001);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_write_register MSM_GLOBAL_ENABLES_REG failed\n");
		return ret;
	}

	msleep(20);

	return ret;
}

static int cs35l41b_pcm_startup(struct snd_pcm_substream *substream,
					struct snd_soc_dai *dai)
{
	//struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(dai->component);

	return 0;
}

static int cs35l41b_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
	struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(codec_dai->component);

	dev_info(cs35l41b->dev, "%s, fmt = %d\n", __func__, fmt);
	return 0;
}


static int cs35l41b_pcm_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params,
					struct snd_soc_dai *dai)
{
	//struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(dai->component);

	return 0;
}

static int cs35l41b_dai_set_sysclk(struct snd_soc_dai *dai,
					int clk_id, unsigned int freq, int dir)
{
	//struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(dai->component);

	return 0;
}

static int cs35l41b_stream_mute(struct snd_soc_dai *dai, int mute,
					int stream)
{
	struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(dai->component);

	dev_info(cs35l41b->dev, "%s: mute = %d, stream = %d\n", __func__, mute, stream);
	return 0;
}

static const struct snd_soc_dai_ops cs35l41b_ops = {
	.startup = cs35l41b_pcm_startup,
	.set_fmt = cs35l41b_set_dai_fmt,
	.hw_params = cs35l41b_pcm_hw_params,
	.set_sysclk = cs35l41b_dai_set_sysclk,
	.mute_stream = cs35l41b_stream_mute,
};

static int cs35l41b_enable_flag_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component =
		snd_soc_kcontrol_component(kcontrol);
	struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = cs35l41b_enable_flag;

	dev_info(cs35l41b->dev, "enable_flag = %d\n", cs35l41b_enable_flag);

	return 0;
}

static int cs35l41b_enable_flag_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	int enable_flag, ret;
	struct snd_soc_component *component =
		snd_soc_kcontrol_component(kcontrol);
	struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(component);

	enable_flag = ucontrol->value.integer.value[0];

	cs35l41b_enable_flag = enable_flag;
	dev_info(cs35l41b->dev, "enable_flag = %d\n", enable_flag);
	switch (enable_flag) {
	case CS35L41B_DISABLE:
		ret = cs35l41b_deconfiguration(cs35l41b);
		if (ret < 0) {
			dev_err(cs35l41b->dev, "cs35l41b_deconfiguration failed\n");
			return ret;
		}
		break;
	case CS35L41B_ENABLE:
		ret = cs35l41b_configuration(cs35l41b);
		if (ret < 0) {
			dev_err(cs35l41b->dev, "cs35l41b_configuration failed\n");
			return ret;
		}
		break;
	default:
		ret = cs35l41b_deconfiguration(cs35l41b);
		if (ret < 0) {
			dev_err(cs35l41b->dev, "cs35l41b_deconfiguration failed\n");
			return ret;
		}
		break;
	}

	return 1;
}

static const struct snd_kcontrol_new cs35l41b_snd_controls[] = {
	SOC_SINGLE_EXT("CS35l41b_Enable", SND_SOC_NOPM, 0, 1, 0,
		cs35l41b_enable_flag_get, cs35l41b_enable_flag_put),
};

static struct snd_soc_dai_driver cs35l41b_dai[] = {
	{
		.name = "xring-spk",
		.id = 0,
		.playback = {
			.stream_name = "PA Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = SNDRV_PCM_RATE_KNOT,
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE),
		},
		.capture = {
			.stream_name = "PA Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_KNOT,
			.formats = (SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_S24_LE),
		},
		.ops = &cs35l41b_ops,
		.symmetric_rate = 1,
	},
};

static int cs35l41b_component_probe(struct snd_soc_component *component)
{
	struct cs35l41b_private *cs35l41b = snd_soc_component_get_drvdata(component);

	dev_info(cs35l41b->dev, "%s enter\n", __func__);
	snd_soc_component_init_regmap(component, cs35l41b->regmap);

	return 0;
}

static void cs35l41b_component_remove(struct snd_soc_component *component)
{
}

static const struct snd_soc_component_driver soc_component_dev_cs35l41b = {
	.name = DRV_NAME,
	.probe = &cs35l41b_component_probe,
	.remove = &cs35l41b_component_remove,
	.controls		= cs35l41b_snd_controls,
	.num_controls		= ARRAY_SIZE(cs35l41b_snd_controls),
};

static const struct reg_sequence g_cs35l41_revb2_errata_patch[] = {
	{0x02BC1000,                           0x00000000},

	{0x00000040,                           0x00000055},
	{0x00000040,                           0x000000AA},
	{0x00004100,                           0x00000000},
	{0x00004310,                           0x00000000},
	{0x00000040,                           0x000000CC},
	{0x00000040,                           0x00000033},
	{0x02BC2020,                           0x00000000},
	{0x02BC20E0,                           0x00000000},
	{0x00004400,                           0x00000000},

	{0x02B80080,                           0x00000001},
	{0x02B80088,                           0x00000001},
	{0x02B80098,                           0x00000001},
	{0x02B800A0,                           0x00000001},
	{0x02B800A8,                           0x00000001},
	{0x02B800B0,                           0x00000001},
	{0x02B800B8,                           0x00000001},

	{0x02B80280,                           0x00000001},
	{0x02B80288,                           0x00000001},
	{0x02B80290,                           0x00000001},
	{0x02B80298,                           0x00000001},
	{0x02B802A0,                           0x00000001},
	{0x02B802A8,                           0x00000001},
	{0x02B802B0,                           0x00000001},
	{0x02B802B8,                           0x00000001},
};

static int cs35l41b_initialization(struct cs35l41b_private *cs35l41b)
{
	int ret;
	/* 1.device initialization sequence 4.1.5 */
	ret = regmap_multi_reg_write(cs35l41b->regmap,
			g_cs35l41_revb2_errata_patch,
			ARRAY_SIZE(g_cs35l41_revb2_errata_patch));
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41_write_errata failed, ret = %d\n", ret);
		return ret;
	}

	ret = regmap_write(cs35l41b->regmap, XM_UNPACKED24_DSP1_CCM_CORE_CONTROL_REG, 0);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "set failed, ret = %d\n", ret);
		return ret;
	}

	return ret;
}

int cs35l41b_probe(struct cs35l41b_private *cs35l41b, struct cs35l41b_platform_data *pdata)
{
	int ret, i;

	for (i = 0; i < ARRAY_SIZE(cs35l41b_supplies); i++)
		cs35l41b->supplies[i].supply = cs35l41b_supplies[i];

	cs35l41b->num_supplies = ARRAY_SIZE(cs35l41b_supplies);
	ret = devm_regulator_bulk_get(cs35l41b->dev, cs35l41b->num_supplies,
				cs35l41b->supplies);

	if (ret != 0) {
		dev_err(cs35l41b->dev, "failed to request core supply: %d\n", ret);
		return ret;
	}

	if (cs35l41b->dev == NULL) {
		dev_info(cs35l41b->dev, "%s cs35l41b->dev is invalid\n", __func__);
		return -ENOMEM;
	}

	/* reset cs35l41b pa pin*/
	cs35l41b->reset_gpio = of_get_named_gpio(cs35l41b->dev->of_node, "reset-gpios",
							0);
	dev_info(cs35l41b->dev, "reset_gpio: %d\n", cs35l41b->reset_gpio);
	if (!cs35l41b->reset_gpio) {
		dev_err(cs35l41b->dev, "reset_gpio: failed %d\n", cs35l41b->reset_gpio);
		return 0;
	}
	if (cs35l41b->reset_gpio) {
		usleep_range(2000, 2100);
		gpio_request(cs35l41b->reset_gpio, "reset_gpios");
		gpio_set_value(cs35l41b->reset_gpio, 1);
	}

	ret = cs35l41b_initialization(cs35l41b);
	if (ret < 0) {
		dev_err(cs35l41b->dev, "cs35l41b_initialization failed\n");
		return ret;
	}

	return snd_soc_register_component(cs35l41b->dev, &soc_component_dev_cs35l41b,
					cs35l41b_dai, ARRAY_SIZE(cs35l41b_dai));
}

MODULE_DESCRIPTION("ASoC CS35L41B driver");
MODULE_LICENSE("GPL");
