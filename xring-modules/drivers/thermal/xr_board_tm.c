// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/iio/iio.h>
#include <linux/iio/consumer.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/thermal.h>

#include "thermal_core.h"
#include "xr_board_tm.h"

struct xr_adc_tm_data;

struct xr_adc_tm_chann {
	const char *name;
	int ch;
	unsigned int nbits;
	unsigned int vref;
	unsigned int resistance;
	struct iio_channel *iio;
	struct xr_adc_tm_data *data;
	struct thermal_zone_device *tzd;
	struct device_node *np;
	struct xr_t2c_map_meta *meta;
	bool initd;
};

struct xr_adc_tm_data {
	struct device *dev;
	struct xr_adc_tm_chann *channs;
	unsigned int nchanns;
	struct xr_t2c_map_meta *meta;
};

static inline int linear_interp(int x0, int y0, int x1, int y1, int x)
{
	if (y0 == y1 || x == x0)
		return y0;
	if (x1 == x0 || x == x1)
		return y1;
	return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

static int xr_pmic_adc_get_temp_comp(struct xr_t2c_map_meta *meta,
				     int adc_code, int *temp)
{
	int i = 0;
	int size = meta->size;
	struct xr_t2c_map *map = meta->map;

	while (map[i].code > adc_code && i < size)
		i++;

	if (i == 0)
		*temp = map[0].temp;
	else if (i >= size)
		*temp = map[size - 1].temp;
	else
		*temp = linear_interp(map[i - 1].code, map[i - 1].temp,
				      map[i].code, map[i].temp, adc_code);
	return 0;
}

static int xr_pmic_adc_get_temp_full(struct xr_t2c_map_meta *meta,
				     int adc_code, int *temp)
{
	int size = meta->size;
	struct xr_t2c_map *map = meta->map;

	if (adc_code >= size || adc_code < 0)
		return -EINVAL;
	*temp = map[adc_code].temp;
	return 0;
}

static int xr_of_parse_pmic_adc_hw(struct xr_adc_tm_data *data,
				   struct device_node *np)
{
	int i;
	int ret;
	/* default adc hw property */
	u32 prop;
	u32 vref = 1850;
	u32 nbits = 12;
	u32 resistance = 150000;
	u32 compressed = 0;

	/* mount get_temp function */
	for (i = 0; i < ARRAY_SIZE(xr_map_meta); i++) {
		if (xr_map_meta[i].compressed)
			xr_map_meta[i].get_temp = xr_pmic_adc_get_temp_comp;
		else
			xr_map_meta[i].get_temp = xr_pmic_adc_get_temp_full;
	}
	/* parse adc hw info */
	ret = of_property_read_u32(np, "xring,vref", &prop);
	if (ret == 0)
		vref = prop;
	ret = of_property_read_u32(np, "xring,nbits", &prop);
	if (ret == 0)
		nbits = prop;
	ret = of_property_read_u32(np, "xring,resistance", &prop);
	if (ret == 0)
		resistance = prop;
	ret = of_property_read_u32(np, "xring,compressed", &prop);
	if (ret == 0)
		compressed = prop;
	for (i = 0; i < ARRAY_SIZE(xr_map_meta); i++)
		if (xr_map_meta[i].vref == vref &&
			xr_map_meta[i].nbits == nbits &&
			xr_map_meta[i].compressed == compressed &&
			xr_map_meta[i].resistance == resistance)
			break;

	if (i == ARRAY_SIZE(xr_map_meta)) {
		dev_err(data->dev, "%s: select[%d] vref=%umV nbits=%u R=%u comp=%u\n",
			__func__, i, vref, nbits, resistance, compressed);
		return -EINVAL;
	}


	data->meta = &xr_map_meta[i];
	dev_info(data->dev, "%s: select[%d] vref=%umV nbits=%u R=%u comp=%u\n",
		__func__, i, vref, nbits, resistance, compressed);
	return 0;
}

static int xr_of_parse_pmic_adc_tm_child(struct xr_adc_tm_data *data,
					 struct xr_adc_tm_chann *chann,
					 struct device_node *np)
{
	int ret;
	const char *chann_name;
	struct of_phandle_args chann_spec;
	struct iio_channel *iio;
	const struct iio_chan_spec *iio_spec;
	struct device *dev = data->dev;

	ret = of_parse_phandle_with_args(np, "io-channels",
					 "#io-channel-cells", 0, &chann_spec);
	if (ret < 0 || chann_spec.args_count < 1) {
		dev_err(dev, "error parser %d: %d\n", chann->ch, ret);
		return ret;
	}
	ret = of_property_read_string_index(np, "io-channel-names", 0, &chann_name);

	if (ret < 0) {
		dev_err(dev, "unable parse io-channel[%d]\n", chann->ch);
		return -EINVAL;
	}
	chann->name = chann_name;
	chann->np = chann_spec.np;
	iio = devm_fwnode_iio_channel_get_by_name(dev, &np->fwnode, chann->name);

	if (IS_ERR(iio) || !iio) {
		dev_err(dev, "error getting channel: %s\n", chann_name);
		return -EINVAL;
	}
	iio_spec = iio->channel;
	chann->iio = iio;
	chann->nbits = iio_spec->scan_type.realbits;
	chann->meta = data->meta;
	chann->initd = true;
	dev_info(dev, "io-channel[%d] : %s parse succ\n", chann->ch, chann->name);
	return 0;
}

static int xr_of_parse_pmic_adc_tm(struct xr_adc_tm_data *data, struct device_node *np)
{
	int i;
	int ret;
	struct device_node *child;
	struct xr_adc_tm_chann *chann;
	struct device *dev = data->dev;

	ret = xr_of_parse_pmic_adc_hw(data, np);
	if (ret)
		return ret;

	data->nchanns = of_get_available_child_count(np);
	if (!data->nchanns)
		return -EINVAL;

	data->channs = devm_kzalloc(dev, sizeof(struct xr_adc_tm_chann) * data->nchanns, GFP_KERNEL);
	if (!data->channs)
		return -ENOMEM;

	i = 0;
	chann = data->channs;
	for_each_available_child_of_node(np, child) {
		chann->ch = i++;
		ret = xr_of_parse_pmic_adc_tm_child(data, chann, child);
		if (ret) {
			of_node_put(child);
			return ret;
		}
		chann++;
	}
	return 0;
}

static int xr_pmic_adc_tzd_get_temp(struct thermal_zone_device *tz, int *temp)
{
	int ret;
	int adc_code;
	struct xr_adc_tm_chann *chann = tz->devdata;
	struct xr_t2c_map_meta *meta;

	if (!chann || !chann->iio || !chann->meta)
		return -EINVAL;
	meta = chann->meta;
	ret = iio_read_channel_raw(chann->iio, &adc_code);
	return meta->get_temp(meta, adc_code, temp);
}

static const struct thermal_zone_device_ops xr_pmic_adc_tzd_ops = {
	.get_temp = xr_pmic_adc_tzd_get_temp,
};

static int xr_tzd_register(struct xr_adc_tm_data *data)
{
	int i;
	struct xr_adc_tm_chann *chann;
	struct device *dev;

	dev = data->dev;
	for (i = 0; i < data->nchanns; i++) {
		chann = &data->channs[i];
		if (!chann->initd)
			continue;
		chann->tzd = devm_thermal_of_zone_register(dev, chann->ch, chann, &xr_pmic_adc_tzd_ops);
		if (IS_ERR(chann->tzd)) {
			chann->tzd = NULL;
			dev_info(dev, "failed to register ntc sensor id%d\n", chann->ch);
			continue;
		}
		thermal_zone_device_disable(chann->tzd);
	}
	return 0;
}

static const struct of_device_id of_xr_adc_tm_match[] = {
	{
		.compatible = "xring,pmic-adc-tm",
		.data = NULL,
	},
	{ /* end */ }
};

MODULE_DEVICE_TABLE(of, of_xr_adc_tm_match);

static int xr_adc_tm_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *node = pdev->dev.of_node;
	struct xr_adc_tm_data *data = NULL;
	struct device *dev = &pdev->dev;

	/* init platform data */
	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	data->dev = &pdev->dev;
	platform_set_drvdata(pdev, data);

	/* parse pmic-adc-tm */
	ret = xr_of_parse_pmic_adc_tm(data, node);
	if (ret)
		return ret;

	/* build thermal-zone */
	xr_tzd_register(data);
	return 0;
}

static int xr_adc_tm_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	dev_info(dev, "xr adc tm remove\n");
	return 0;
}

static struct platform_driver xr_board_tm_driver = {
	.driver = {
		.name = "xr_ntc_tm",
		.of_match_table = of_xr_adc_tm_match,
	},
	.probe	= xr_adc_tm_probe,
	.remove	= xr_adc_tm_remove,
};

int xr_board_tm_init(void)
{
	int ret;

	ret = platform_driver_register(&xr_board_tm_driver);
	return ret;
}

void xr_board_tm_exit(void)
{
	platform_driver_unregister(&xr_board_tm_driver);
}
