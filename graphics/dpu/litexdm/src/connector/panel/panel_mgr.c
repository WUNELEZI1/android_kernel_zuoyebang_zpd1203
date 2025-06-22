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

#include "osal.h"

#include "panel_mgr.h"
#include "panel.h"
#include "dpu_pipeline.h"
#include "dpu_log.h"

static struct panel_mgr g_panel_mgr;
int32_t g_conn_to_panel_table[CONNECTOR_MAX];


static void init_conn_to_panel_table(void)
{
	dpu_mem_set(g_conn_to_panel_table, CONNECTOR_NONE, sizeof(g_conn_to_panel_table));
}

int32_t get_panel_id(uint32_t connector_id)
{
	return connector_id >= CONNECTOR_MAX ? -1 : g_conn_to_panel_table[connector_id];
}

struct panel_dev *get_panel_dev(uint32_t panel_id)
{
	return panel_id < PANEL_MAX ? g_panel_mgr.panel_list[panel_id] : NULL;
}

static int32_t panel_mgr_ops_handle_parse(struct platform_device *pdev, char *ops_cmd, uint32_t id, void *value)
{
	struct ops_handle_table *ops_handle;
	struct panel_drv_private *priv;
	struct panel_dev *panel_dev;
	int32_t panel_id;
	uint32_t i;

	dpu_pr_debug("connector id:%u +\n", id);

	panel_id = get_panel_id(id);
	dpu_check_and_return(panel_id < 0, -1, "invalid panel id\n");
	dpu_pr_debug("panel id:%u +\n", panel_id);

	panel_dev = get_panel_dev(panel_id);
	dpu_check_and_return(!panel_dev, -1, "panel_dev is null\n");

	priv = to_panel_priv(platform_get_drvdata(panel_dev->pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	if (priv->is_fake_panel)
		return 0;

	ops_handle = panel_dev->handle_table;
	dpu_check_and_return(!ops_handle, -1, "ops_handle is null\n");
	for (i = 0; i < panel_dev->ops_size; i++) {
		if (!dpu_str_cmp(ops_cmd, ops_handle[i].ops_cmd) && ops_handle[i].handle_func)
			return ops_handle[i].handle_func(panel_dev->pdev, id, value);
	}

	return -1;
}

int32_t register_panel(enum PANEL_ID panel_id, const char *panel_name)
{
	struct platform_device *this_dev;
	struct panel_dev *panel_dev;
	int32_t ret;
	dpu_pr_debug("register %a id:%u +\n", panel_name, panel_id);
	panel_dev = dpu_mem_alloc(sizeof(*panel_dev));
	if (!panel_dev) {
		dpu_pr_err("panel %u dev alloc failed\n", panel_id);
		return -1;
	}
	g_panel_mgr.panel_list[panel_id] = panel_dev;

	this_dev = platform_device_alloc(panel_name, panel_id);
	if (!this_dev) {
		dpu_pr_err("%a id:%u alloc dev failed\n", panel_name, panel_id);
		return -1;
	}

	ret = panel_probe(this_dev);
	if (ret) {
		dpu_pr_err("device add failed\n");
		return -1;
	}
	panel_dev->pdev = this_dev;
	dpu_pr_debug("-\n");
	return ret;
}

static void panel_remove(struct platform_device *pdev)
{
	struct panel_drv_private *priv;

	if (!pdev) {
		dpu_pr_info("pdev is null!");
		return;
	}

	dpu_pr_debug("+\n");
	unregister_connector();

	priv = (struct panel_drv_private *)platform_get_drvdata(pdev);
	g_conn_to_panel_table[priv->base.connector_id] = CONNECTOR_NONE;
	if (priv->base.external_connector_id != CONNECTOR_NONE)
		g_conn_to_panel_table[priv->base.external_connector_id] = CONNECTOR_NONE;

	unprepare_panel_data(pdev);
	platform_set_drvdata(pdev, NULL);
	dpu_mem_free(priv);

	dpu_pr_debug("-\n");
}

void unregister_panel(enum PANEL_ID panel_id)
{
	struct panel_dev *panel_dev;
	dpu_pr_debug("+\n");

	panel_dev = g_panel_mgr.panel_list[panel_id];
	panel_remove(panel_dev->pdev);

	dpu_mem_free(panel_dev->pdev);
	g_panel_mgr.panel_list[panel_id] = NULL;
	dpu_mem_free(panel_dev);
	dpu_pr_debug("-\n");
}

int32_t panel_probe(struct platform_device *pdev)
{
	struct panel_drv_private *priv;
	int32_t ret;

	if (!pdev) {
		dpu_pr_info("pdev is null!");
		return -1;
	}

	dpu_pr_debug("pdev name:%a, id:%u +\n", pdev->name, pdev->id);
	priv = dpu_mem_alloc(sizeof(*priv));
	if (!priv) {
		dpu_pr_err("alloc failed\n");
		return -1;
	}
	priv->pdev = pdev;
	platform_set_drvdata(pdev, &(priv->base));

	ret = prepare_panel_data(pdev, get_panel_dev(pdev->id));
	if (ret != 0) {
		dpu_pr_err("prepare_panel_data failed\n");
		return -1;
	}

	g_conn_to_panel_table[priv->base.connector_id] = pdev->id;
	if (priv->base.external_connector_id != CONNECTOR_NONE)
		g_conn_to_panel_table[priv->base.external_connector_id] = pdev->id;

	ret = register_connector(g_panel_mgr.pdev, &priv->base);
	if (ret != 0) {
		dpu_pr_err("register_connector failed\n");
		return -1;
	}

	dpu_pr_debug("-\n");
	return ret;
}

static int32_t panel_mgr_on(struct platform_device *pdev, void *value)
{
	struct panel_drv_private *priv;
	struct panel_dev *panel_dev;
	uint32_t connector_id;
	int32_t panel_id;
	int32_t ret;

	connector_id = *(uint32_t *)value;
	dpu_pr_debug("connector id:%u +\n", connector_id);

	panel_id = get_panel_id(connector_id);
	dpu_check_and_return(panel_id < 0, -1, "invalid panel id\n");

	panel_dev = get_panel_dev(panel_id);
	dpu_check_and_return(!panel_dev, -1, "panel_dev is null\n");

	priv = to_panel_priv(platform_get_drvdata(panel_dev->pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	if (priv->is_fake_panel)
		return 0;

	if (panel_dev->on) {
		ret = panel_dev->on(panel_dev->pdev);
		if (ret) {
			dpu_pr_err("panel %u on failed\n", panel_id);
			return -1;
		}
	}
	dpu_pr_debug("-\n");
	return 0;
}

static int32_t panel_mgr_off(struct platform_device *pdev, void *value)
{
	struct panel_drv_private *priv;
	struct panel_dev *panel_dev;
	uint32_t connector_id;
	int32_t panel_id;
	int32_t ret;

	connector_id = *(uint32_t *)value;
	dpu_pr_debug("connector id:%u +\n", connector_id);

	panel_id = get_panel_id(connector_id);
	dpu_check_and_return(panel_id < 0, -1, "invalid panel id\n");

	panel_dev = get_panel_dev(panel_id);
	dpu_check_and_return(!panel_dev, -1, "panel_dev is null\n");

	priv = to_panel_priv(platform_get_drvdata(panel_dev->pdev));
	dpu_check_and_return(!priv, -1, "priv is null\n");

	if (priv->is_fake_panel)
		return 0;

	if (panel_dev->off) {
		ret = panel_dev->off(panel_dev->pdev);
		if (ret) {
			dpu_pr_err("panel %u off failed\n", panel_id);
			return -1;
		}
	}
	dpu_pr_debug("-\n");
	return 0;
}

int32_t panel_init(void)
{
	struct platform_device *this_dev;
	struct dpu_device_base *base;
	int32_t ret;

	dpu_pr_debug("+\n");
	this_dev = platform_device_alloc("panel_mgr", 0);
	dpu_check_and_return(!this_dev, -1, "conn mgr dev alloc failed\n");

	g_panel_mgr.pdev = this_dev;

	base = &g_panel_mgr.base;
	base->on_func = panel_mgr_on;
	base->off_func = panel_mgr_off;
	base->ops_handle_func = panel_mgr_ops_handle_parse;
	base->next = NULL;

	ret = platform_device_add_data(g_panel_mgr.pdev, &g_panel_mgr.base, sizeof(struct dpu_device_base));
	dpu_check_and_return(ret != 0, ret, "panel mgr add data failed\n");

	init_conn_to_panel_table();
	dpu_pr_debug("-\n");
	return ret;
}
