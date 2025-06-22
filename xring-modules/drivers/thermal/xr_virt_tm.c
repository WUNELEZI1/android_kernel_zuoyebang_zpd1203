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
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include <linux/limits.h>
#include <linux/thermal.h>
#include <linux/string.h>
#include <dt-bindings/xring/platform-specific/common/tsens/vsens.h>
#include <dt-bindings/xring/platform-specific/common/tsens/tsens_id.h>
#include <soc/xring/xr_tsens.h>
#include "thermal_core.h"

struct xr_virt_data;
struct xr_virt_sensor;

struct xr_virt_sensor {
	int id;
	int algo;
	int ss_nr;
	int *ss_list;
	int *ss_weight;
	int emul_temp;
	bool emul_en;
	struct thermal_zone_device *tzd;
	bool valid;
};

struct xr_virt_data {
	int nr_sens;
	struct xr_virt_sensor *sens;
	struct platform_device *pdev;
};

static int algo_subsys_maxmin(struct xr_virt_sensor *sensor, int *temp, bool calc_max)
{
	int i;
	int ret;
	int tmp;
	int tmax = S32_MIN;
	int tmin = S32_MAX;

	for (i = 0; i < sensor->ss_nr; i++) {
		ret = xr_tsens_read_temp(sensor->ss_list[i], &tmp);
		if (ret)
			return ret;
		if (tmp > tmax)
			tmax = tmp;
		if (tmp < tmin)
			tmin = tmp;
	}
	if (calc_max)
		*temp = tmax;
	else
		*temp = tmin;
	return 0;
}

static int algo_linear_fit(struct xr_virt_sensor *sensor, int *temp)
{
	int ret;
	int tmp;
	int i;
	int sum = 0;
	int total = 0;

	for (i = 0; i < sensor->ss_nr; i++) {
		ret = xr_tsens_read_temp(sensor->ss_list[i], &tmp);
		if (ret)
			return ret;
		total += sensor->ss_weight[i];
		sum += (sensor->ss_weight[i]  * tmp);
	}
	if (total != 0)
		*temp = sum / total;
	return 0;
}

int __xr_virt_get_temp(struct xr_virt_sensor *sensor, int *temp)
{
	if (temp == NULL)
		return -EINVAL;

	if (sensor->emul_en) {
		*temp = sensor->emul_temp;
		return 0;
	}

	switch (sensor->algo) {
	case XR_VIRT_SENS_ALOG_MAXIMUM:
		return xr_tsens_read_temp(TSENS_ID_MAX, temp);
	case XR_VIRT_SENS_ALOG_MINMUM:
		return xr_tsens_read_temp(TSENS_ID_MIN, temp);
	case XR_VIRT_SENS_ALOG_SS_MIN:
		return algo_subsys_maxmin(sensor, temp, false);
	case XR_VIRT_SENS_ALOG_SS_MAX:
		return algo_subsys_maxmin(sensor, temp, true);
	case XR_VIRT_SENS_ALOG_LINEAR_FIT:
		return algo_linear_fit(sensor, temp);
	default:
		break;
	}
	return 0;
}

static int xr_of_virt_get_temp(struct thermal_zone_device *tz, int *temp)
{
	struct xr_virt_sensor *sensor = tz->devdata;

	return __xr_virt_get_temp(sensor, temp);
}
static const struct thermal_zone_device_ops xr_of_virt_thermal_ops = {
	.get_temp = xr_of_virt_get_temp,
};

static int
__xr_virt_tm_of_parse_tsens(struct platform_device *pdev, struct xr_virt_data *data)
{
	int num;
	int ret;
	u32 algo;
	s32 emul;
	int nr_sens;
	int count1, count2;
	struct xr_virt_sensor *sensor;
	struct device_node *child;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	/* get virtual sensor count */
	nr_sens = of_get_child_count(np);
	if (nr_sens <= 0)
		return -EINVAL;
	data->nr_sens = nr_sens;
	data->sens = devm_kzalloc(dev, sizeof(struct xr_virt_sensor) * nr_sens, GFP_KERNEL);
	if (!data->sens)
		return -ENOMEM;

	/* parse sensor */
	num = 0;
	for_each_child_of_node(np, child) {
		sensor = &data->sens[num];
		sensor->id = num++;
		/* calcation algorithm */
		ret = of_property_read_u32(child, "algo", &algo);
		if (ret)
			return ret;
		sensor->algo = algo;
		sensor->valid = true;
		/* emul temp */
		ret = of_property_read_s32(child, "emul", &emul);
		if (ret == 0) {
			sensor->emul_en = true;
			sensor->emul_temp = emul;
			continue;
		}

		/* sensor list and weight */
		count1 = of_property_count_u32_elems(child, "ss-list");
		count2 = of_property_count_u32_elems(child, "ss-weight");
		if ((count1 <= 0) || (count1 != count2))
			continue;
		sensor->ss_list = devm_kzalloc(dev, sizeof(int) * (count1 + count2), GFP_KERNEL);
		if (!sensor->ss_list) {
			sensor->valid = false;
			continue;
		}

		sensor->ss_nr = count1;
		sensor->ss_weight = &sensor->ss_list[count1];
		ret = of_property_read_u32_array(child, "ss-list", sensor->ss_list, count1);
		ret += of_property_read_u32_array(child, "ss-weight", sensor->ss_weight, count2);
		if (ret != 0)
			sensor->valid = false;
	}
	return 0;
}

static int
__xr_virt_tm_dev_register(struct platform_device *pdev, struct xr_virt_data *data)
{
	int i;
	struct xr_virt_sensor *sensor;
	struct device *dev = &pdev->dev;

	for (i = 0; i < data->nr_sens; i++) {
		sensor = &data->sens[i];
		if (!sensor->valid)
			continue;
		sensor->tzd = devm_thermal_of_zone_register(dev,
							    sensor->id, sensor,
							    &xr_of_virt_thermal_ops);
		if (IS_ERR(sensor->tzd)) {
			sensor->tzd = NULL;
			dev_info(dev, "failed to register sensor id=%d\n", sensor->id);
			continue;
		}
	}
	return 0;
}

static int xr_virt_tm_probe(struct platform_device *pdev)
{
	int ret;
	struct xr_virt_data *data;
	struct device *dev = &pdev->dev;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	data->pdev = pdev;
	platform_set_drvdata(pdev, data);

	/* of parse tsens */
	ret = __xr_virt_tm_of_parse_tsens(pdev, data);
	if (ret)
		return -EINVAL;

	/* register thermal zone device */
	__xr_virt_tm_dev_register(pdev, data);
	return 0;
}

static int xr_virt_tm_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id of_xr_virt_tm_match[] = {
	{
		.compatible = "xring,thermal-vsensor",
		.data = NULL,
	},
	{ /* end */ }
};

MODULE_DEVICE_TABLE(of, of_xr_virt_tm_match);

static struct platform_driver xr_virt_tm_driver = {
	.driver = {
		.name = "xr_virt_tm",
		.of_match_table = of_xr_virt_tm_match,
	},
	.probe	= xr_virt_tm_probe,
	.remove	= xr_virt_tm_remove,
};

int xr_virt_tm_init(void)
{
	int ret;

	ret = platform_driver_register(&xr_virt_tm_driver);
	if (ret)
		pr_err("%s fail\n", __func__);

	return ret;
}

void xr_virt_tm_exit(void)
{
	platform_driver_unregister(&xr_virt_tm_driver);
}
