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

#ifndef _PLATFORM_DEVICE_H_
#define _PLATFORM_DEVICE_H_

#include "osal.h"
#include "dpu_list.h"

#define MAX_DEV_NAME_LEN 128

struct of_device_id {
	char compatible[MAX_DEV_NAME_LEN];
	void *data;
};

struct platform_device {
	struct list_head list;
	char name[MAX_DEV_NAME_LEN];
	int32_t id;
	void *platform_data;
	const struct of_device_id *of_match_table;
	void *driver_data;
};

struct device_driver {
	char *name;
	const struct of_device_id *of_match_table;
	int32_t (*probe)(struct platform_device *pdev);
	struct list_head list;
};

struct platform_driver {
	int32_t (*probe)(struct platform_device *pdev);
	struct device_driver driver;
};

struct platform_device *platform_device_alloc(const char *name, int32_t id);

int32_t platform_device_add_data(struct platform_device *pdev, void *data, uint32_t size);

int32_t dpu_platform_device_add(struct platform_device *pdev);

int32_t dpu_platform_driver_register(struct platform_driver *pdrv);

static inline void *dev_get_platdata(const struct platform_device *pdev)
{
	return pdev->platform_data;
}

static inline void *platform_get_drvdata(const struct platform_device *pdev)
{
	return pdev->driver_data;
}

static inline void platform_set_drvdata(struct platform_device *pdev, void *data)
{
	pdev->driver_data = data;
}

static inline void *of_device_get_match_data(const struct platform_device *pdev)
{
	return pdev->of_match_table->data;
}
#endif