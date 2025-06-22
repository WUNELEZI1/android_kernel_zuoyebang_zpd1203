// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023, X-Ring technologies Inc., All rights reserved.
 */

#define pr_fmt(fmt)  "[XRISP_DRV][%14s] %s(%d): " fmt, "flash_soc", __func__, __LINE__

#include <linux/module.h>
#include <linux/version.h>
#include <linux/errno.h>
#include "xrisp_flash_soc.h"
#include "xrisp_log.h"

/*
 * config pinctrl
 */
int xrisp_pinctrl_state_set(struct pinctrl *pinctrl, char *pinctrl_name)
{
	struct pinctrl_state *state;
	int ret;

	if (!pinctrl || !pinctrl_name) {
		XRISP_PR_ERROR("pinctrl_name is NULL\n");
		return -EINVAL;
	}

	state = pinctrl_lookup_state(pinctrl, pinctrl_name);
	if (IS_ERR(state)) {
		devm_pinctrl_put(pinctrl);
		return -EINVAL;
	}

	ret = pinctrl_select_state(pinctrl, state);
	if (ret < 0) {
		devm_pinctrl_put(pinctrl);
		return -EINVAL;
	}

	return 0;
}

int xrisp_flash_parse_dts(struct xrisp_flash_data *flash_data)
{
	struct device_node *of_node;
	struct device *dev = flash_data->dev;
	uint32_t count = 0;
	uint32_t *val_array = NULL;
	int ret = 0;
	int i;

	if (!dev || !dev->of_node)
		return -ENODEV;

	of_node = dev->of_node;
	if (!of_get_property(of_node, "flash-table", &count)) {
		XRISP_PR_ERROR("parse dts fail");
		return 0;
	}
	count /= sizeof(uint32_t);
	if (count > FLASH_MAX_NUM || !count) {
		XRISP_PR_ERROR("flash-table err");
		return 0;
	}
	val_array = kcalloc(count, sizeof(uint32_t), GFP_KERNEL);
	if (!val_array)
		return -ENOMEM;
	ret = of_property_read_u32_array(of_node, "flash-table",
		val_array, count);
	if (ret) {
		XRISP_PR_ERROR("get flash-table failed, ret:%d", ret);
		goto read_array_err;
	}
	for (i = 0; i < count; i++)
		flash_data->flash_dev_id[i] = val_array[i];

	kfree(val_array);
	return 0;

read_array_err:
	kfree(val_array);
	return -EINVAL;
}
