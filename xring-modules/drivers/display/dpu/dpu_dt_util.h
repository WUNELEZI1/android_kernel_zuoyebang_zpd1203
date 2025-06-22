/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DPU_DT_UTIL_H_
#define _DPU_DT_UTIL_H_

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <drm/drm_print.h>
#include "dpu_log.h"

/* max property counts per node */
#define MAX_PROP_ARRAY_SIZE 10

/* the dpu supported device tree node type */
enum prop_type {
	PROP_TYPE_BOOL,
	PROP_TYPE_U32,
	PROP_TYPE_U32_ARRAY,
	PROP_TYPE_STRING,
	PROP_TYPE_STRING_ARRAY,
};

/**
 * prop_node - property of one device tree node
 * @id: the array index of this property node
 * @name: property name of device tree property
 * @is_mandatory: the property is mandatory or optional
 * @type: property type
 */
struct prop_node {
	u32 id;
	char *name;
	bool is_mandatory;
	enum prop_type type;
};

/**
 * prop_data - to store the parsed property data
 * @counts: the property data counts per node
 * @value: the property value for integer type
 * @string: the property value for string type
 */
struct prop_data {
	u32 counts;
	u32 value[MAX_PROP_ARRAY_SIZE];
	const char *string[MAX_PROP_ARRAY_SIZE];
};

/**
 * dpu_parse_dt_nodes - the helper function for parse one node group
 * @np: device node pointer
 * @prop_nodes: the property nodes pointer
 * @prop_count: the property nodes count
 * @prop_data: the parsed data of property nodes
 *
 * Return: error code on failure, 0 on success
 */
int dpu_parse_dt_nodes(struct device_node *np,
		struct prop_node *prop_nodes, u32 prop_count,
		struct prop_data *prop_data);

/**
 * dpu_iomem_remap - map an physical address to virtual address with name
 *
 * @pdev: platform device pointer
 * @name: the name pointer for the physical address
 *
 * Return: valid virtual address on success, error pointer on failure
 */
void __iomem *dpu_iomem_remap(struct platform_device *pdev, const char *name);

/**
 * dpu_iomem_size - get a address size with name
 *
 * @pdev: platform device pointer
 * @name: the name pointer for the physical address
 *
 * Return: valid size on success, 0 on failure
 */
u32 dpu_iomem_size(struct platform_device *pdev, const char *name);

#endif /* _DPU_HW_UTIL_H_ */
