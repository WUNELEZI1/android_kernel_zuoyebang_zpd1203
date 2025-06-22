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

#include <linux/of.h>
#include "dpu_dt_util.h"

static int _dpu_parse_dt_bool(struct device_node *np,
		struct prop_node *prop_node,
		struct prop_data *prop_data)
{
	prop_data->value[0] = of_property_read_bool(np, prop_node->name);
	prop_data->counts = 1;

	return 0;
}

static int _dpu_parse_dt_u32(struct device_node *np,
		struct prop_node *prop_node,
		struct prop_data *prop_data)
{
	int ret;

	ret = of_property_read_u32(np, prop_node->name,
			&prop_data->value[0]);
	if (ret && prop_node->is_mandatory) {
		DPU_ERROR("failed to parse mandatory property %s, ret %d\n",
				prop_node->name, ret);
		return ret;
	}

	/* return directly if property is not mandatory on failure */
	if (ret)
		return 0;

	prop_data->counts = 1;

	return 0;
}

static int _dpu_parse_dt_u32_array(struct device_node *np,
		struct prop_node *prop_node,
		struct prop_data *prop_data)
{
	int temp_count;
	int ret = 0;

	temp_count = of_property_count_u32_elems(np, prop_node->name);
	if (temp_count <= 0 && prop_node->is_mandatory) {
		DPU_ERROR("failed to parse mandatory property %s, ret %d\n",
				prop_node->name, temp_count);
		return -EINVAL;
	}

	/* return directly if property is not mandatory on failure */
	if (temp_count <= 0)
		return 0;

	if (temp_count > MAX_PROP_ARRAY_SIZE) {
		DPU_ERROR("property %s count greater than max count: %d/%d\n",
				prop_node->name, temp_count, MAX_PROP_ARRAY_SIZE);
		return -EINVAL;
	}

	prop_data->counts = temp_count;

	ret = of_property_read_u32_array(np,
			prop_node->name,
			prop_data->value,
			prop_data->counts);
	if (ret) {
		DPU_ERROR("failed to get u32 array, property %s, ret %d\n",
				prop_node->name, ret);
		prop_data->counts = 0;
		return ret;
	}

	return ret;
}

static int _dpu_parse_dt_string(struct device_node *np,
		struct prop_node *prop_node,
		struct prop_data *prop_data)
{
	int ret;

	ret = of_property_read_string(np, prop_node->name,
			&prop_data->string[0]);
	if (ret && prop_node->is_mandatory) {
		DPU_ERROR("failed to parse mandatory property %s, ret %d\n",
				prop_node->name, ret);
		return ret;
	}

	/* return directly if property is not mandatory on failure */
	if (ret)
		return 0;

	prop_data->counts = 1;

	return 0;
}

static int _dpu_parse_dt_string_array(struct device_node *np,
		struct prop_node *prop_node,
		struct prop_data *prop_data)
{
	int temp_count;
	int ret;
	int i;

	temp_count = of_property_count_strings(np, prop_node->name);
	if (temp_count <= 0 && prop_node->is_mandatory) {
		DPU_ERROR("failed to parse mandatory property %s, ret %d\n",
				prop_node->name, temp_count);
		return -EINVAL;
	}

	/* return directly if property is not mandatory on failure */
	if (temp_count <= 0)
		return 0;

	if (temp_count > MAX_PROP_ARRAY_SIZE) {
		DPU_ERROR("property %s count greater than max count: %d/%d\n",
				prop_node->name, temp_count, MAX_PROP_ARRAY_SIZE);
		return -EINVAL;
	}

	prop_data->counts = temp_count;

	for (i = 0; i < prop_data->counts; i++) {
		ret = of_property_read_string_index(np,
				prop_node->name, i,
				&prop_data->string[i]);
		if (ret) {
			DPU_ERROR("failed to read string, %d/%d, %s\n",
					i, prop_data->counts,
					prop_node->name);
			prop_data->counts = 0;

			return -EINVAL;
		}
	}

	return 0;
}

int dpu_parse_dt_nodes(struct device_node *np,
		struct prop_node *prop_nodes,
		u32 prop_count,
		struct prop_data *prop_data)
{
	struct prop_node *cur_node;
	struct prop_data *cur_data;
	int i;
	int ret = 0;

	if (!np || !prop_nodes || !prop_data) {
		DPU_ERROR("invalid parameters, %pK, %pK, %pK\n",
				np, prop_nodes, prop_data);
		return -EINVAL;
	}

	if (prop_count > MAX_PROP_ARRAY_SIZE) {
		DPU_ERROR("reach the maximum prop array size %u/%u\n",
				prop_count, MAX_PROP_ARRAY_SIZE);
		return -EINVAL;
	}

	for (i = 0; i < prop_count; i++) {
		cur_node = &prop_nodes[i];
		cur_data = &prop_data[i];

		switch (cur_node->type) {
		case PROP_TYPE_BOOL:
			ret = _dpu_parse_dt_bool(np, cur_node, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_U32:
			ret = _dpu_parse_dt_u32(np, cur_node, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_U32_ARRAY:
			ret = _dpu_parse_dt_u32_array(np, cur_node, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_STRING:
			ret = _dpu_parse_dt_string(np, cur_node, cur_data);
			if (ret)
				return ret;
			break;
		case PROP_TYPE_STRING_ARRAY:
			ret = _dpu_parse_dt_string_array(np, cur_node, cur_data);
			if (ret)
				return ret;
			break;
		default:
			DPU_ERROR("unsupported prop type %d!\n",
					prop_nodes[i].type);
		}
	}

	return ret;
}

void __iomem *dpu_iomem_remap(struct platform_device *pdev, const char *name)
{
	struct resource *res;
	unsigned long size;
	void __iomem *iomem_ptr;

	if (!pdev || !name) {
		DPU_ERROR("invalid parameters: %pK, %s\n", pdev, name);
		return ERR_PTR(-EINVAL);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		DPU_ERROR("failed to get memory resource: %s\n", name);
		return ERR_PTR(-EINVAL);
	}

	size = resource_size(res);
	iomem_ptr = devm_ioremap(&pdev->dev, res->start, size);
	if (!iomem_ptr) {
		DPU_ERROR("failed to ioremap: %s\n", name);
		return ERR_PTR(-ENOMEM);
	}

	return iomem_ptr;
}

u32 dpu_iomem_size(struct platform_device *pdev, const char *name)
{
	struct resource *res;

	if (!pdev || !name) {
		DPU_ERROR("invalid parameters: %pK, %s\n", pdev, name);
		return 0;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);
	if (!res) {
		DPU_ERROR("failed to get memory resource: %s\n", name);
		return 0;
	}

	return resource_size(res);
}
