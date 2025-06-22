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

#include <stddef.h>

#include "osal.h"
#include "dpu_list.h"
#include "platform_device.h"
#include "dpu_log.h"

static LIST_HEAD(drv_list);

struct platform_device *platform_device_alloc(const char *name, int32_t id)
{
	struct platform_device *pdev;

	pdev = dpu_mem_alloc(sizeof(struct platform_device));
	if (!pdev) {
		dpu_pr_err("platform dev alloc failed\n");
		return NULL;
	}
	dpu_str_cpy(pdev->name, name, MAX_DEV_NAME_LEN);
	pdev->id = id;

	return pdev;
}

int32_t dpu_platform_driver_register(struct platform_driver *pdrv)
{
	if (!pdrv) {
		dpu_pr_err("platform drv is null\n");
		return -1;
	}
	pdrv->driver.probe = pdrv->probe;
	list_add(&pdrv->driver.list, &drv_list);
	return 0;
}

int32_t dpu_platform_device_add(struct platform_device *pdev)
{
	struct list_head *pos = NULL;
	int32_t ret = -1;

	list_for_each(pos, &drv_list) {
		struct device_driver *drv = container_of(pos, struct device_driver, list);
		if (!drv) {
			dpu_pr_err("drv is null, pdrv->driver.list abnormal\n");
			return -1;
		}
		if (dpu_str_cmp(drv->name, pdev->name))
			continue;

		pdev->of_match_table = drv->of_match_table;
		if (!drv->probe) {
			dpu_pr_err("drv->probe is null\n");
			return -1;
		}
		ret = drv->probe(pdev);
	}
	return ret;
}

int32_t platform_device_add_data(struct platform_device *pdev, void *data, uint32_t size)
{
	void *data_priv;

	dpu_pr_debug("+\n");
	if (!pdev || !data) {
		return -1;
	}

	data_priv = dpu_mem_alloc(size);
	if (!data_priv) {
		dpu_pr_err("alloc failed\n");
		return -1;
	}
	dpu_mem_cpy(data_priv, data, size);
	dpu_mem_free(pdev->platform_data);
	pdev->platform_data = data_priv;

	dpu_pr_debug("-\n");
	return 0;
}