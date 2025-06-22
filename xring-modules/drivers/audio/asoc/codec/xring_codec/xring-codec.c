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
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/version.h>
#include <linux/pm.h>
#include <sound/pcm.h>
#include <sound/soc.h>

#include "xring-codec.h"

#define XRING_CODEC_DRV_NAME "xring-codec"

static struct xring_codec_priv xring_codec_pdata;

#define XRING_CODEC_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
		SNDRV_PCM_FMTBIT_S24_LE |\
		SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

static int xring_codec_startup(struct snd_pcm_substream *substream,
			  struct snd_soc_dai *dai)
{
	dev_info(dai->dev, "%s substream->name=%s substream->stream=%d substream->number=%d\n",
				__func__, substream->name, substream->stream, substream->number);

	return 0;
}

static int xring_codec_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	dev_info(dai->dev, "%s substream->name=%s substream->stream=%d substream->number=%d\n",
				__func__, substream->name, substream->stream, substream->number);

	return 0;
}

static const struct snd_soc_dai_ops xring_codec_dai_ops = {
	.startup = &xring_codec_startup,
	.hw_params = &xring_codec_hw_params,
};

static struct snd_soc_dai_driver xring_codec_dai[] = {
	{
		.name = "xring-aif1",
		.id = 1,
		.playback = {
			.stream_name = "AIF1 Playback",
			.channels_min = 1,
			.channels_max = 8,
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = XRING_CODEC_FORMATS,
		},
		.capture = {
			.stream_name = "AIF1 Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = XRING_CODEC_FORMATS,
		},
		.ops = &xring_codec_dai_ops,
	},
};

static const struct snd_kcontrol_new xring_codec_snd_controls[] = {
};

static const struct snd_soc_dapm_widget xring_codec_dapm_widgets[] = {
};

static const struct snd_soc_dapm_route xring_codec_dapm_routes[] = {
};

static int xring_soc_codec_probe(struct snd_soc_component *component)
{
	return 0;
};

static void xring_soc_codec_remove(struct snd_soc_component *component)
{
}

static int xring_codec_set_sysclk(struct snd_soc_component *component, int clk_id,
			int source, unsigned int freq, int dir)
{
	dev_info(component->dev, "clk_id=%d source= %d freq=%d dir=%d\n",
		clk_id, source, freq, dir);

	return 0;
}

static const struct snd_soc_component_driver soc_codec_dev_xring = {
	.name = XRING_CODEC_DRV_NAME,
	.probe = &xring_soc_codec_probe,
	.remove = &xring_soc_codec_remove,
	.set_sysclk		= &xring_codec_set_sysclk,

	.controls = xring_codec_snd_controls,
	.num_controls = ARRAY_SIZE(xring_codec_snd_controls),
	.dapm_widgets = xring_codec_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(xring_codec_dapm_widgets),
	.dapm_routes = xring_codec_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(xring_codec_dapm_routes),
};

static const struct of_device_id xing_codec_of_match[] = {
	{ .compatible = "cirrus,cs47l15", .data = (void *)CS47L15 },
	{ .compatible = "cirrus,cs47l35", .data = (void *)CS47L35 },
	{ .compatible = "cirrus,cs47l85", .data = (void *)CS47L85 },
	{ .compatible = "cirrus,cs47l90", .data = (void *)CS47L90 },
	{ .compatible = "cirrus,cs47l91", .data = (void *)CS47L91 },
	{ .compatible = "cirrus,cs42l92", .data = (void *)CS42L92 },
	{ .compatible = "cirrus,cs47l92", .data = (void *)CS47L92 },
	{ .compatible = "cirrus,cs47l93", .data = (void *)CS47L93 },
	{ .compatible = "cirrus,wm1840", .data = (void *)WM1840 },
	{}
};

static int xring_codec_i2c_probe(struct i2c_client *i2c)
{
	struct xring_codec_priv *xring_codec = &xring_codec_pdata;
	struct device *dev = &i2c->dev;
	const void *of_data;
	unsigned long type;
	static u32 init_count;
	int ret = -1;
	const struct i2c_device_id *id = i2c_client_get_device_id(i2c);

	xring_codec->dev = dev;
	if (xring_codec->dev == NULL)
		return -ENOMEM;

	dev_set_drvdata(dev, xring_codec);

	of_data = of_device_get_match_data(&i2c->dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case CS47L92:
		dev_info(dev, "madera codec info filling\n");
		ret = madera_i2c_init(i2c, id);
		break;
	default:
		dev_info(dev, "none codec info filling\n");
		ret = -1;
		break;
	}

	if (ret && (init_count < CODEC_INIT_MAX_COUNT)) {
		init_count++;
		ret = -EPROBE_DEFER;
		return ret;
	}

	dev_info(dev, "ret = %d\n", ret);
	if (ret) {
		dev_err(dev, "add dummy codec\n");
		xring_codec->component =
			(const struct snd_soc_component_driver *)&soc_codec_dev_xring;
		xring_codec->dai_driver = (struct snd_soc_dai_driver *)&xring_codec_dai;
		xring_codec->num_dai = ARRAY_SIZE(xring_codec_dai);
		ret = snd_soc_register_component(xring_codec->dev,
						xring_codec->component,
						xring_codec->dai_driver,
						xring_codec->num_dai);
		if (ret < 0) {
			dev_err(dev, "Failed to register dummy codec: %d\n", ret);
			return ret;
		}
	}
	return ret;
}

void xring_codec_i2c_remove(struct i2c_client *i2c)
{
	//cs42l92_remove(i2c);
	madera_i2c_deinit(i2c);
}

static const struct i2c_device_id xring_codec_i2c_id[] = {
	{ "cs47l15", CS47L15 },
	{ "cs47l35", CS47L35 },
	{ "cs47l85", CS47L85 },
	{ "cs47l90", CS47L90 },
	{ "cs47l91", CS47L91 },
	{ "cs42l92", CS42L92 },
	{ "cs47l92", CS47L92 },
	{ "cs47l93", CS47L93 },
	{ "wm1840", WM1840 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, xring_codec_i2c_id);

static int xring_codec_suspend(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_codec_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case CS47L92:
		dev_info(dev, "CS47L92 runtime suspend\n");
		madera_runtime_suspend(dev);
		break;
	default:
		dev_info(dev, "none codec runtime suspend\n");
		break;
	}
	return 0;
}

static int xring_codec_resume(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_codec_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case CS47L92:
		dev_info(dev, "CS47L92 runtime resume\n");
		madera_runtime_resume(dev);
		break;
	default:
		dev_info(dev, "none codec runtime resume\n");
		break;
	}

	return 0;
}

static int xring_codec_sys_suspend(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_codec_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case CS47L92:
		dev_info(dev, "CS47L92 sys suspend\n");
		//madera_runtime_suspend(dev);
		break;
	default:
		dev_info(dev, "none codec sys suspend\n");
		break;
	}
	return 0;
}

static int xring_codec_sys_resume(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_codec_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case CS47L92:
		dev_info(dev, "CS47L92 sys resume\n");
		//madera_runtime_resume(dev);
		break;
	default:
		dev_info(dev, "none codec sys resume\n");
		break;
	}

	return 0;
}

static const struct dev_pm_ops xring_codec_pm_ops = {
	SET_RUNTIME_PM_OPS(xring_codec_suspend, xring_codec_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(xring_codec_sys_suspend, xring_codec_sys_resume)
};

static struct i2c_driver xring_codec = {
	.driver = {
		.name	= "xring-codec",
		.of_match_table	= of_match_ptr(xing_codec_of_match),
		.pm = &xring_codec_pm_ops,
	},
	.probe		= xring_codec_i2c_probe,
	.remove		= xring_codec_i2c_remove,
	.id_table	= xring_codec_i2c_id,
};

module_i2c_driver(xring_codec);

MODULE_DESCRIPTION("Xring Third Codec driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xring_i2c");
MODULE_SOFTDEP("pre: madera_codec_cs47l92");

