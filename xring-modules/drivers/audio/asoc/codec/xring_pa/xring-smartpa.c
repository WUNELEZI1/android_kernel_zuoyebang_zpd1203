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
#include <sound/pcm.h>
#include <sound/soc.h>

#include "xring-smartpa.h"
#include "cs35l41b/cs35l41b_user.h"
#include "cs35l43/cs35l43-user.h"
#include "cs35l45/cs35l45-user.h"
#include "fs3001/fshaptic.h"
#include "cs40l26/include/cs40l26.h"

#define XRING_SMARTPA_DRV_NAME "xring-smartpa"

static struct xring_smartpa_priv xring_smartpa_pdata;

#define XRING_SMARTPA_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
		SNDRV_PCM_FMTBIT_S24_LE |\
		SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver xring_smartpa_dai[] = {
	{
		.name = "xring-spk",
		.id = 1,
		.playback = {
			.stream_name = "PA Playback",
			.channels_min = 1,
			.channels_max = 8,
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = XRING_SMARTPA_FORMATS,
		},
		.capture = {
			.stream_name = "PA Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = (SNDRV_PCM_RATE_8000_384000|
					SNDRV_PCM_RATE_KNOT),
			.formats = XRING_SMARTPA_FORMATS,
		},
	},
};

static const struct snd_soc_component_driver soc_smartpa_dev_xring = {
	.name = XRING_SMARTPA_DRV_NAME,
};

static const struct of_device_id xring_smartpa_of_match[] = {
	{ .compatible = "cirrus,cs35l41b",   .data = (void *)XR_CS35L41B },
	{ .compatible = "cirrus,cs35l43",    .data = (void *)XR_CS35L43  },
	{ .compatible = "cirrus,cs40l26a",   .data = (void *)XR_CS40L26  },
	{ .compatible = "cirrus,cs35l45",   .data = (void *)XR_CS35L45  },
	{ .compatible = "foursemi,fshaptic",   .data = (void *)XR_FS3001  },
	{}
};

static int xring_smartpa_i2c_probe(struct i2c_client *i2c)
{
	struct xring_smartpa_priv *xring_smartpa = &xring_smartpa_pdata;
	struct device *dev = &i2c->dev;
	const void *of_data;
	unsigned long type;
	static u32 init_count;
	int ret = -1;
	const struct i2c_device_id *id = i2c_client_get_device_id(i2c);

	xring_smartpa->dev = dev;
	if (xring_smartpa->dev == NULL)
		return -ENOMEM;

	dev_set_drvdata(dev, xring_smartpa);

	of_data = of_device_get_match_data(&i2c->dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_info(dev, "cs35l41b info filling\n");
		ret = cs35l41b_i2c_probe(i2c, id);
		break;
	case XR_CS35L43:
		dev_info(dev, "cs35l43 info filling\n");
		ret = cs35l43_i2c_probe(i2c, id);
		break;
	case XR_CS40L26:
		dev_info(dev, "cs40l26 info filling\n");
		ret = cs40l26_i2c_probe(i2c, id);
		break;
	case XR_CS35L45:
		dev_info(dev, "cs35l45 info filling\n");
		ret = cs35l45_i2c_probe(i2c, id);
		break;
	case XR_FS3001:
		dev_info(dev, "fs3001 info filling\n");
		ret = foursemi_i2c_probe(i2c, id);
		break;
	default:
		dev_info(dev, "none smartpa info filling\n");
		ret = -1;
		break;
	}

	if (ret && (init_count < PA_INIT_MAX_COUNT)) {
		init_count++;
		ret = -EPROBE_DEFER;
		return ret;
	}

	dev_info(dev, "ret = %d\n", ret);
	if (ret) {
		dev_err(dev, "add dummy smartpa\n");
		xring_smartpa->component =
			(const struct snd_soc_component_driver *)&soc_smartpa_dev_xring;
		xring_smartpa->dai_driver = (struct snd_soc_dai_driver *)&xring_smartpa_dai;
		xring_smartpa->num_dai = ARRAY_SIZE(xring_smartpa_dai);

		ret = snd_soc_register_component(xring_smartpa->dev,
						xring_smartpa->component,
						xring_smartpa->dai_driver,
						xring_smartpa->num_dai);

		if (ret < 0) {
			dev_err(dev, "Failed to register dummy smartpa: %d\n", ret);
			return ret;
		}
	}

	return ret;
}

void xring_smartpa_i2c_remove(struct i2c_client *i2c)
{
	struct device *dev = &i2c->dev;
	const void *of_data;
	unsigned long type;
	const struct i2c_device_id *id = i2c_client_get_device_id(i2c);

	of_data = of_device_get_match_data(&i2c->dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_info(dev, "cs35l41b remove\n");
		cs35l41b_i2c_remove(i2c);
		break;
	case XR_CS35L43:
		dev_info(dev, "cs35l43 remove\n");
		cs35l43_i2c_remove(i2c);
		break;
	case XR_CS40L26:
		dev_info(dev, "cs40l26 remove\n");
		cs40l26_i2c_remove(i2c);
		break;
	case XR_CS35L45:
		dev_info(dev, "cs35l43 remove\n");
		cs35l45_i2c_remove(i2c);
		break;
	case XR_FS3001:
		dev_info(dev, "fs3001 info filling\n");
		foursemi_i2c_remove(i2c);
		break;
	default:
		dev_info(dev, "none smartpa remove\n");
		}
}

static const struct i2c_device_id xring_smartpa_i2c_id[] = {
	{ "cs35l41b", XR_CS35L41B },
	{ "cs35l43",  XR_CS35L43 },
	{ "cs40l26",  XR_CS40L26 },
	{ "cs35l45",  XR_CS35L45 },
	{ "fshaptic",  XR_FS3001 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, xring_smartpa_i2c_id);

static int xring_smartpa_suspend(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_smartpa_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_dbg(dev, "cs35l41b suspend\n");
		break;
	case XR_CS35L43:
		dev_dbg(dev, "cs35l43 suspend\n");
		break;
	case XR_CS40L26:
		dev_dbg(dev, "cs40l26 suspend\n");
		cs40l26_suspend(dev);
		break;
	default:
		dev_dbg(dev, "none smartpa suspend\n");
		}
	return 0;
}

static int xring_smartpa_resume(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_smartpa_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_dbg(dev, "cs35l41b resume\n");
		break;
	case XR_CS35L43:
		dev_dbg(dev, "cs35l43 resume\n");
		break;
	case XR_CS40L26:
		dev_dbg(dev, "cs40l26 resume\n");
		cs40l26_resume(dev);
		break;
	default:
		dev_dbg(dev, "none smartpa resume\n");
		}

	return 0;
}

static int xring_smartpa_sys_suspend(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_smartpa_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_dbg(dev, "cs35l41b sys_suspend\n");
		break;
	case XR_CS35L43:
		dev_dbg(dev, "cs35l43 sys_suspend\n");
		break;
	case XR_CS40L26:
		dev_dbg(dev, "cs40l26 sys_suspend\n");
		cs40l26_sys_suspend(dev);
		break;
	default:
		dev_dbg(dev, "none smartpa sys_suspend\n");
		}

	return 0;
}

static int xring_smartpa_sys_resume(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_smartpa_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_dbg(dev, "cs35l41b sys_resume\n");
		break;
	case XR_CS35L43:
		dev_dbg(dev, "cs35l43 sys_resume\n");
		break;
	case XR_CS40L26:
		dev_dbg(dev, "cs40l26 sys_resume\n");
		cs40l26_sys_resume(dev);
		break;
	default:
		dev_dbg(dev, "none smartpa sys_resume\n");
		}

	return 0;
}

static int xring_smartpa_sys_suspend_noirq(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_smartpa_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_dbg(dev, "cs35l41b sys_suspend_noirq\n");
		break;
	case XR_CS35L43:
		dev_dbg(dev, "cs35l43 sys_suspend_noirq\n");
		break;
	case XR_CS40L26:
		dev_dbg(dev, "cs40l26 sys_suspend_noirq\n");
		//cs40l26_sys_suspend_noirq(dev);
		break;
	default:
		dev_dbg(dev, "none smartpa sys_suspend_noirq\n");
		}

	return 0;
}

static int xring_smartpa_sys_resume_noirq(struct device *dev)
{
	const void *of_data;
	unsigned long type;
	struct i2c_client *i2c = to_i2c_client(dev);
	const struct i2c_device_id *id = i2c_match_id(xring_smartpa_i2c_id, i2c);

	of_data = of_device_get_match_data(dev);
	if (of_data)
		type = (unsigned long)of_data;
	else
		type = id->driver_data;

	switch (type) {
	case XR_CS35L41B:
		dev_dbg(dev, "cs35l41b sys_resume_noirq\n");
		break;
	case XR_CS35L43:
		dev_dbg(dev, "cs35l43 sys_resume_noirq\n");
		break;
	case XR_CS40L26:
		dev_dbg(dev, "cs40l26 sys_resume_noirq\n");
		//cs40l26_sys_resume_noirq(dev);
		break;
	default:
		dev_dbg(dev, "none smartpa sys_resume_noirq\n");
		}

	return 0;
}

static const struct dev_pm_ops xring_smartpa_pm_ops = {
	SET_RUNTIME_PM_OPS(xring_smartpa_suspend, xring_smartpa_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(xring_smartpa_sys_suspend, xring_smartpa_sys_resume)
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(xring_smartpa_sys_suspend_noirq, xring_smartpa_sys_resume_noirq)
};

static struct i2c_driver xring_smartpa = {
	.driver = {
		.name	= "xring-smartpa",
		.of_match_table	= of_match_ptr(xring_smartpa_of_match),
		.pm = &xring_smartpa_pm_ops,
	},
	.probe		= xring_smartpa_i2c_probe,
	.remove		= xring_smartpa_i2c_remove,
	.id_table	= xring_smartpa_i2c_id,
};

static int xring_smartpa_i2c_init(void)
{
	return i2c_register_driver(THIS_MODULE, &xring_smartpa);
}

static void xring_smartpa_i2c_exit(void)
{
	i2c_del_driver(&xring_smartpa);
}

static int __init xring_smartpa_init(void)
{
	xring_smartpa_i2c_init();
	cs40l26_codec_driver_init();
	return 0;
}

static void xring_smartpa_exit(void)
{
	cs40l26_codec_driver_exit();
	xring_smartpa_i2c_exit();
}

module_init(xring_smartpa_init);
module_exit(xring_smartpa_exit);

MODULE_DESCRIPTION("Xring Smart PA driver");
MODULE_LICENSE("GPL v2");
MODULE_SOFTDEP("pre: xring_i2c");
