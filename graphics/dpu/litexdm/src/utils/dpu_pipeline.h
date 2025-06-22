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

#ifndef _DPU_PIPELINE_H_
#define _DPU_PIPELINE_H_

#include "platform_device.h"
#include "dpu_common_info.h"
#include "dpu_log.h"

struct ops_handle_table {
	char *ops_cmd;
	/* The id could be either the scene_id or the conn_id */
	int32_t (*handle_func)(struct platform_device *pdev, uint32_t id, void *value);
};

struct pipe_params {
	uint32_t scene_id;
	uint32_t conn_id;
	void *priv;
};

struct dpu_device_base {
	struct platform_device *next;
	int32_t (*on_func)(struct platform_device *pdev, void *value);
	int32_t (*off_func)(struct platform_device *pdev, void *value);
	int32_t (*ops_handle_func)(struct platform_device *pdev, char *ops_cmd, uint32_t id, void *value);
};

static __attribute__((unused)) int32_t pipeline_next_on(struct platform_device *pdev, void *value)
{
	struct dpu_device_base *next_pdata;
	struct platform_device *next_pdev;
	struct dpu_device_base *pdata;
	int32_t ret = 0;

	if (!pdev) {
		dpu_pr_err("pdev is NULL!\n");
		return -1;
	}
	pdata = dev_get_platdata(pdev);
	if (!pdata) {
		dpu_pr_err("pdata is NULL!\n");
		return -1;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(next_pdev);
		if ((next_pdata) && (next_pdata->on_func)) {
			ret = next_pdata->on_func(next_pdev, value);
			if (ret) {
				dpu_pr_err("next dev on failed\n");
				return -1;
			}
		}
	}

	return ret;
}

static __attribute__((unused)) int32_t pipeline_next_off(struct platform_device *pdev, void *value)
{
	struct dpu_device_base *next_pdata;
	struct platform_device *next_pdev;
	struct dpu_device_base *pdata;
	int32_t ret = 0;

	if (!pdev) {
		dpu_pr_err("pdev is NULL!\n");
		return -1;
	}
	pdata = dev_get_platdata(pdev);
	if (!pdata) {
		dpu_pr_err("pdata is NULL!\n");
		return -1;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(next_pdev);
		if ((next_pdata) && (next_pdata->off_func)) {
			ret = next_pdata->off_func(next_pdev, value);
			if (ret) {
				dpu_pr_err("next dev off failed\n");
				return -1;
			}
		}
	}

	return ret;
}

static __attribute__((unused)) int32_t pipeline_next_ops_handle(struct platform_device *pdev, char *ops_cmd, uint32_t id, void *value)
{
	struct dpu_device_base *next_pdata;
	struct platform_device *next_pdev;
	struct dpu_device_base *pdata;
	int32_t ret = 0;

	if (!pdev) {
		dpu_pr_err("pdev is NULL!\n");
		return -1;
	}
	pdata = dev_get_platdata(pdev);
	if (!pdata) {
		dpu_pr_err("pdata is NULL!\n");
		return -1;
	}

	next_pdev = pdata->next;
	if (next_pdev != NULL) {
		next_pdata = dev_get_platdata(next_pdev);
		if ((next_pdata) && (next_pdata->ops_handle_func)) {
			ret = next_pdata->ops_handle_func(next_pdev, ops_cmd, id, value);
		}
	}
	return ret;
}
#endif