// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022-2025, X-Ring technologies Inc., All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <soc/xring/securelib/securec.h>
#include <dt-bindings/xring/platform-specific/sensorhub_dts.h>
#include <dt-bindings/xring/platform-specific/sensorhub_ddr_layout.h>
#include <dt-bindings/xring/platform-specific/fcm_acpu_address_map.h>
#include <linux/io.h>
#include "shub_private_dts.h"

struct sh_dts_cb {
	int (*fun)(struct device_node *node, struct sh_bsp_dts *bsp_dts);
};

static uint8_t *g_sh_shm_base_addr;

static int shub_parse_i3c_dts(struct device_node *parent, struct sh_bsp_dts *dts_mem_block)
{
	struct sh_dts_i3c_info *i3c_info = &dts_mem_block->i3c_info;
	struct device_node *i3c_root = NULL;
	struct device_node *node = NULL;
	uint32_t val = 0;
	uint16_t idx = 0;
	int ret = 0;

	i3c_root = of_find_node_by_name(parent, "i3c");
	if (i3c_root == NULL) {
		ret = -ENODEV;
		pr_err("sh_i3c node not found!\n");
		goto _err_node;
	}

	if (!of_device_is_available(i3c_root)) {
		ret = -EINVAL;
		pr_err("sh_i3c node not enabled!\n");
		goto _err_node;
	}

	while (idx < SH_I3C_MASTER_NUM) {
		node = of_get_next_available_child(i3c_root, node);
		if (node == NULL) {
			pr_info("this is last node\n");
			break;
		}

		ret = of_property_read_u32(node, "i3c-master-id", &val);
		if (ret < 0) {
			pr_err("sh_i3c find id fail, ret[%d]!\n", ret);
			goto _err_node;
		}

		writew((uint16_t)val, &i3c_info->i3c_master[idx].i3c_master_id);
		pr_info("i3c_master_id[%u], val[%d]!\n", idx, val);

		ret = of_property_read_u32(node, "i3c-slave-num", &val);
		if (ret < 0) {
			pr_err("sh_i3c find i3c slave fail, ret[%d]!\n", ret);
			goto _err_node;
		}

		writew((uint16_t)val, &i3c_info->i3c_master[idx].i3c_slave_num);
		pr_info("i3c_slave_num[%u], val[%d]!\n", idx, val);

		ret = of_property_read_u32(node, "i2c-slave-num", &val);
		if (ret < 0) {
			pr_err("sh_i3c find i2c slave fail, ret[%d]!\n", ret);
			goto _err_node;
		}

		writew((uint16_t)val, &i3c_info->i3c_master[idx].i2c_slave_num);
		pr_info("i2c_slave_num[%u], val[%d]!\n", idx, val);
		idx++;
	}

	writew(idx, &i3c_info->i3c_master_num);

	pr_info("sh i3c-master-num[%u]\n", idx);

	return 0;

_err_node:
	i3c_info->i3c_master_num = 0;

	return ret;
}

static int shub_parse_conn_dts(struct device_node *parent, struct sh_bsp_dts *dts_mem_block)
{
	struct sh_dts_conn_info *conn_info = &dts_mem_block->conn_info;
	struct device_node *conn_root = NULL;
	struct device_node *child = NULL;
	int ret = 0;

	conn_root = of_find_node_by_name(parent, "conn");
	if (conn_root == NULL) {
		ret = -ENODEV;
		pr_err("shub_conn node not found!\n");
		return ret;
	}
	conn_info->is_conn_exist = of_device_is_available(conn_root) ? 1u : 0u;

	child = of_find_node_by_name(conn_root, "sh_wcn");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("sh_wcn node not found!\n");
		return ret;
	}
	conn_info->is_wcn_exist = of_device_is_available(child) ? 1u : 0u;

	child = of_find_node_by_name(conn_root, "sh_gnss");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("sh_gnss node not found!\n");
		return ret;
	}
	conn_info->is_gnss_exist = of_device_is_available(child) ? 1u : 0u;

	child = of_find_node_by_name(conn_root, "sh_modem");
	if (child == NULL) {
		ret = -ENODEV;
		pr_err("sh_modem node not found!\n");
		return ret;
	}
	conn_info->is_modem_exist = of_device_is_available(child) ? 1u : 0u;

	pr_info("shub_conn[%d], wcn[%d], gnss[%d], modem[%d]\n", conn_info->is_conn_exist,
	conn_info->is_wcn_exist, conn_info->is_gnss_exist, conn_info->is_modem_exist);
	return ret;
}

static int shub_total_size_check(void)
{
	uint32_t tmp_app_dts_size = sizeof(struct sh_app_dts);
	struct sh_bsp_dts *p_bsp_dts = (struct sh_bsp_dts *)g_sh_shm_base_addr;
	uint32_t tmp_bsp_dts_size = sizeof(struct sh_bsp_dts);

	writel(tmp_bsp_dts_size, p_bsp_dts);
	if ((tmp_bsp_dts_size + tmp_app_dts_size) > SHUB_NS_DDR_SHM_DTS_SIZE) {
		pr_err("dts size is too big:bsp:%u,app:%u\n",
			tmp_bsp_dts_size, tmp_app_dts_size);
		return -ENOMEM;
	}

	pr_info("size:bsp=%u,app=%u\n", readl(p_bsp_dts), tmp_app_dts_size);

	return 0;
}

void shub_shm_addr_map(void)
{
	g_sh_shm_base_addr = (uint8_t *)ioremap_wc(SHUB_NS_DDR_SHM_DTS_ADDR,
		SHUB_NS_DDR_SHM_DTS_SIZE);
	if (IS_ERR_OR_NULL(g_sh_shm_base_addr))
		pr_err("%s err\n", __func__);
}
EXPORT_SYMBOL(shub_shm_addr_map);

void shub_shm_addr_unmap(void)
{
	iounmap(g_sh_shm_base_addr);
}
EXPORT_SYMBOL(shub_shm_addr_unmap);

struct sh_bsp_dts *shub_get_bsp_dts(void)
{
	return (struct sh_bsp_dts *)g_sh_shm_base_addr;
}
EXPORT_SYMBOL(shub_get_bsp_dts);

struct sh_app_dts *shub_get_app_dts(void)
{
	struct sh_bsp_dts *p_bsp_dts = NULL;
	uint32_t tmp_bsp_dts_size = sizeof(struct sh_bsp_dts);

	p_bsp_dts = (struct sh_bsp_dts *)g_sh_shm_base_addr;
	writel(tmp_bsp_dts_size, p_bsp_dts);
	if (tmp_bsp_dts_size > SHUB_NS_DDR_SHM_DTS_SIZE) {
		pr_err("sensorhub bsp dts size(%u) is to big!\n", readl(p_bsp_dts));
		return NULL;
	}
	return (struct sh_app_dts *)(g_sh_shm_base_addr + tmp_bsp_dts_size);
}
EXPORT_SYMBOL(shub_get_app_dts);

struct sh_dts_cb dts_cb_array[] = {
	{shub_parse_i3c_dts}, {shub_parse_conn_dts}, {shub_parse_aoc_dts},
};

int shub_run_bsp_dts_cb(void)
{
	uint32_t i = 0;
	int ret = 0;
	struct device_node *sh_node = NULL;

	sh_node = of_find_node_by_name(NULL, "sensorhub");
	if (!sh_node) {
		ret = -EOPNOTSUPP;
		pr_err("sensorhub root node is null!\n");
		goto _err_node;
	}

	pr_info("shub bsp cb size = %lu", ARRAY_SIZE(dts_cb_array));
	for (i = 0; i < ARRAY_SIZE(dts_cb_array); i++) {
		ret = dts_cb_array[i].fun(sh_node, (struct sh_bsp_dts *)g_sh_shm_base_addr);
		if (ret < 0) {
			pr_err("dts cb failed,i=%d,ret=0x%x", i, ret);
			break;
		}
	}

	shub_total_size_check();

_err_node:
	return ret;
}
EXPORT_SYMBOL(shub_run_bsp_dts_cb);
