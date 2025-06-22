// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include "thermal_core.h"
#include "xr_soc_tm.h"

static struct xr_thermal_data *g_priv_data;

int xr_tsens_read_temp(int id, int *temp)
{
	if (!g_priv_data || !temp)
		return -EINVAL;

	if (!g_priv_data->ops)
		return -EINVAL;

	return g_priv_data->ops->read_temp(g_priv_data, id, temp);
}
EXPORT_SYMBOL(xr_tsens_read_temp);

static int __xr_thermal_get_temp(struct thermal_zone_device *tz, int *temp)
{
	struct xr_thermal_sensor *sensor = tz->devdata;
	struct xr_thermal_data *data = sensor->data;

	if (temp && data->ops && data->ops->read_temp)
		return data->ops->read_temp(data, sensor->id, temp);
	return -EINVAL;
}
static const struct thermal_zone_device_ops xr_thermal_of_thermal_ops = {
	.get_temp = __xr_thermal_get_temp,
};

static int
__xr_soc_tm_of_parse_tsens(struct platform_device *pdev, struct xr_thermal_data *data)
{
	int i;
	int ret;
	unsigned int sensor_num;
	const char *votemng_name;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct xr_thermal_sensor *sensor;

	/* get resource */
	data->regs = devm_platform_ioremap_resource(pdev, 0);
	if (data->regs == NULL) {
		dev_err(dev, "get resource failed\n");
		return -EINVAL;
	}

	/* parse sensor */
	ret = of_property_read_u32(np, "sensor-num", &sensor_num);
	if (ret < 0) {
		dev_err(dev, "%pOFn: missing sensor-num property\n", np);
		return -EINVAL;
	}

	/* parse votemng lpc*/
	ret = of_property_read_string_index(np, "vote-mng-names", 0, &votemng_name);
	data->votemng = vote_mng_get(dev, votemng_name);
	if (!data->votemng)
		dev_err(dev, "Failed get vote mng:%s\n", votemng_name);

	/* parse votemng bcl*/
	ret = of_property_read_string_index(np, "vote-mng-names", 1, &votemng_name);
	data->votemng_bcl = vote_mng_get(dev, votemng_name);
	if (!data->votemng_bcl)
		dev_err(dev, "Failed get vote mng:%s\n", votemng_name);

	data->num_sensors = sensor_num;
	data->sensor = devm_kzalloc(dev, (sensor_num + 1) * sizeof(*data->sensor), GFP_KERNEL);
	if (!data->sensor)
		return -ENOMEM;

	/* register sensor */
	for (i = 0; i < sensor_num; i++) {
		sensor = &data->sensor[i];
		sensor->id = i;
		sensor->data = data;
		sensor->tzd = devm_thermal_of_zone_register(dev,
							    sensor->id, sensor,
							    &xr_thermal_of_thermal_ops);
		if (IS_ERR(sensor->tzd)) {
			sensor->tzd = NULL;
			dev_info(dev, "failed to register sensor id %u: %d\n",
			sensor->id, ret);
			continue;
		}
	}
	return 0;
}

static int xr_soc_tm_probe(struct platform_device *pdev)
{
	int ret;
	struct xr_thermal_data *data;
	struct device *dev = &pdev->dev;

	/* init driver data */
	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	data->pdev = pdev;
	data->ops = of_device_get_match_data(dev);
	platform_set_drvdata(pdev, data);

	/* platform probe */
	if (data->ops && data->ops->probe)
		data->ops->probe(data);

	/* of parse tsens */
	ret = __xr_soc_tm_of_parse_tsens(pdev, data);
	if (ret) {
		dev_err(dev, "error parse tsensor\n");
		return -EINVAL;
	}

	/* setup sysfs */
	ret = xr_thermal_sysfs_setup(data);
	if (ret) {
		dev_err(dev, "error setup sysfs\n");
		return -EINVAL;
	}
	g_priv_data = data;
	return 0;
}

static int xr_soc_tm_remove(struct platform_device *pdev)
{
	struct xr_thermal_data *data;
	struct device *dev = &pdev->dev;

	g_priv_data = NULL;
	data = (struct xr_thermal_data *)platform_get_drvdata(pdev);
	if (!data)
		return -EINVAL;

	xr_thermal_sysfs_teardown(data);
	if (data->votemng) {
		vote_mng_put(dev, data->votemng);
		data->votemng = NULL;
	}
	if (data->votemng_bcl) {
		vote_mng_put(dev, data->votemng_bcl);
		data->votemng_bcl = NULL;
	}
	return 0;
}

static const struct of_device_id of_xr_soc_tm_match[] = {
	{
		.compatible = "xring,tsensor",
		.data = &tsens_data_v1,
	},
	{ /* end */ }
};

MODULE_DEVICE_TABLE(of, of_xr_soc_tm_match);

static struct platform_driver xr_soc_tm_driver = {
	.driver = {
		.name = "xr_soc_tm",
		.of_match_table = of_xr_soc_tm_match,
	},
	.probe	= xr_soc_tm_probe,
	.remove	= xr_soc_tm_remove,
};

int xr_soc_tm_init(void)
{
	int ret;

	g_priv_data = NULL;
	ret = platform_driver_register(&xr_soc_tm_driver);
	if (ret)
		pr_err("%s fail\n", __func__);

	return ret;
}

void xr_soc_tm_exit(void)
{
	platform_driver_unregister(&xr_soc_tm_driver);
}
