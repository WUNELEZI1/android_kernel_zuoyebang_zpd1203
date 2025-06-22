/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024, X-Ring technologies Inc., All rights reserved.
 */

#ifndef __XRING_SOC_THERMAL_H__
#define __XRING_SOC_THERMAL_H__

#include <linux/thermal.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <soc/xring/vote_mng.h>

struct xr_thermal_data;
struct xr_thermal_tsensops;
struct xr_thermal_sensor;

struct xr_thermal_tsens_ops {
	int (*read_temp)(struct xr_thermal_data *data, int id, int *temp);
	int (*probe)(struct xr_thermal_data *data);
};

struct xr_thermal_sensor {
	uint32_t id;
	struct xr_thermal_data *data;
	struct thermal_zone_device *tzd;
};

struct xr_thermal_data {
	void __iomem *regs;
	unsigned int num_sensors;
	const struct xr_thermal_tsens_ops *ops;
	struct xr_thermal_sensor *sensor;
	struct platform_device *pdev;
	struct vote_mng *votemng;
	struct vote_mng *votemng_bcl;
	struct class *class;
	struct device lpc_dev;
	struct device bcl_dev;
	struct mutex bcl_lock;
};

/* sysfs */
#define to_xr_thermal_data(_dev) \
	container_of(_dev, struct xr_thermal_data, lpc_dev)

#define bcl_to_xr_thermal_data(_dev) \
	container_of(_dev, struct xr_thermal_data, bcl_dev)

int xr_thermal_sysfs_setup(struct xr_thermal_data *data);
int xr_thermal_sysfs_teardown(struct xr_thermal_data *data);

extern struct xr_thermal_tsens_ops tsens_data_v1;

#endif
