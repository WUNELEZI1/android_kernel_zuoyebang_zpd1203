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
#include "platform_device.h"
#include "pipeline_hw_res.h"
#include "mipi/mipi_dsi_dev.h"
#include "dpu_common_info.h"
#include "dpu_conn_mgr.h"
#include "dpu_log.h"
#include "panel_mgr.h"
#include "dpu_dsc.h"

static struct conn_mgr g_conn_mgr;

struct connector *get_connector(uint32_t conn_id)
{
	return g_conn_mgr.conn_list[conn_id];
}

struct dpu_panel_info *conn_mgr_panel_info_get(uint32_t conn_id)
{
	struct list_head *pos = NULL;

	list_for_each(pos, &g_conn_mgr.panel_list) {
		struct panel_info_entry *pinfo_entry = container_of(pos, struct panel_info_entry, list);
		if ((pinfo_entry->pinfo->connector_id == conn_id) ||
			(pinfo_entry->pinfo->external_connector_id == conn_id))
			return pinfo_entry->pinfo;
	}
	return NULL;
}

void conn_mgr_prepare(uint32_t scene_id)
{
	struct connector *connector;
	uint32_t conn_id;

	dpu_pr_debug("scene_id:%u +\n", scene_id);

	conn_id = get_conn_id_from_scene_id(scene_id);
	dpu_pr_debug("conn id:%u +\n", conn_id);

	connector = get_connector(conn_id);
	dpu_check_and_no_retval(!connector, "connector is null\n");

	if (!connector->pinfo)
		connector->pinfo = conn_mgr_panel_info_get(conn_id);

	dpu_pr_debug("scene_id:%u -\n", scene_id);
}

void conn_mgr_unprepare(uint32_t scene_id)
{
	struct connector *connector;
	uint32_t conn_id;

	dpu_pr_debug("scene_id:%u +\n", scene_id);

	conn_id = get_conn_id_from_scene_id(scene_id);
	dpu_pr_debug("conn id:%u +\n", conn_id);

	connector = get_connector(conn_id);
	dpu_check_and_no_retval(!connector, "connector is null\n");

	if (connector->pinfo)
		connector->pinfo = NULL;

	dpu_pr_debug("scene_id:%u -\n", scene_id);
}

static int32_t conn_mgr_on(struct platform_device *pdev, void *value)
{
	struct connector *connector;
	uint32_t scene_id;
	uint32_t conn_id;
	int32_t ret = 0;

	scene_id = *(uint32_t *)value;
	dpu_pr_debug("scene id:%u +\n", scene_id);

	conn_id = get_conn_id_from_scene_id(scene_id);
	dpu_pr_debug("conn id:%u +\n", conn_id);

	connector = get_connector(conn_id);
	connector->pinfo = conn_mgr_panel_info_get(conn_id);
	dpu_check_and_return(!connector->pinfo, -1, "get pinfo failed\n");

	if (is_dsi_connector(scene_id))
		mipi_ops_set_up(connector);

	if (connector->on) {
		ret = connector->on(pdev, connector, scene_id);
		if (ret) {
			dpu_pr_err("conn id:%u on failed\n", conn_id);
			return ret;
		}
	}

	dpu_pr_debug("conn id:%u -\n", conn_id);
	return ret;
}

static int32_t conn_mgr_off(struct platform_device *pdev, void *value)
{
	struct connector *connector;
	uint32_t scene_id;
	uint32_t conn_id;
	int32_t ret = 0;

	scene_id = *(uint32_t *)value;
	conn_id = get_conn_id_from_scene_id(scene_id);
	connector = get_connector(conn_id);
	dpu_pr_debug("conn id:%u +\n", conn_id);

	if (connector->off) {
		ret = connector->off(pdev, connector, scene_id);
		if (ret) {
			dpu_pr_err("conn id:%u off failed\n", conn_id);
			return ret;
		}
	}

	dpu_pr_debug("conn id:%u -\n", conn_id);
	return ret;
}

static int32_t conn_mgr_minimal_init(struct platform_device *pdev, uint32_t id, void *value)
{
	struct dsi_ctrl_cfg ctrl_cfg = {0};
	struct connector *connector;

	connector = get_connector(id);
	if (((id == CONNECTOR_DSI0) || (id == CONNECTOR_DSI1)))
		mipi_ops_set_up(connector);

	/* panel power up */
	pipeline_next_on(pdev, &connector->id);

	/* minimal init dsi */
	display_to_dsi(connector, &ctrl_cfg);
	mipi_dsi_init(pdev, connector, &ctrl_cfg);

	return 0;
}

static int32_t conn_mgr_minimal_deinit(struct platform_device *pdev, uint32_t id, void *value)
{
	struct dpu_power_mgr pwr_mgr;
	struct connector *connector;
	uint32_t scene_id;
	int ret = 0;
	dpu_pr_debug("+\n");

	/* panel off cmd set */
	connector = get_connector(id);
	pipeline_next_ops_handle(pdev, "lcd_send_display_off_cmd", connector->id, NULL);

	/* dsi power off */
	pwr_mgr.pinfo = connector->pinfo;
	pwr_mgr.profile_id = 0;
	pwr_mgr.volt_level = 0;
	scene_id = *(uint32_t *)value;
	if (is_dsi_connector(scene_id)) {
		ret = dsi_power_off(&pwr_mgr);
		dpu_check_and_return(ret != 0, ret, "dsi power off fail\n");
	}

	/* panel power off */
	connector = get_connector(id);
	pipeline_next_off(pdev, &connector->id);

	connector = get_connector(id);
	if (((id == CONNECTOR_DSI0) || (id == CONNECTOR_DSI1)))
		mipi_ops_remove(connector);

	dpu_pr_debug("-\n");
	return 0;
}

/* All connector external interfaces are placed here */
static struct ops_handle_table g_priv_handle[] = {
	{"set_backlight", mipi_set_backlight},
	{"minimal_init", conn_mgr_minimal_init},
	{"minimal_deinit", conn_mgr_minimal_deinit},
};

static int32_t conn_mgr_ops_handle_parse(struct platform_device *pdev,
		char *ops_cmd, uint32_t id, void *value)
{
	struct ops_handle_table *ops_handle;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(g_priv_handle); i++) {
		ops_handle = &g_priv_handle[i];
		if ((!dpu_str_cmp(ops_cmd, ops_handle->ops_cmd)) && ops_handle->handle_func)
			return ops_handle->handle_func(pdev, id, value);
	}
	return pipeline_next_ops_handle(pdev, ops_cmd, id, value);
}

int32_t register_connector(struct platform_device *next_pdev,
		struct dpu_panel_info *pinfo)
{
	struct panel_info_entry *pinfo_entry;
	int32_t ret;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!pinfo, -1, "pinfo is null\n");

	pinfo_entry = dpu_mem_alloc(sizeof(struct panel_info_entry));
	dpu_check_and_return(!pinfo_entry, -1, "pinfo entry alloc failed\n");

	pinfo_entry->pinfo = pinfo;
	list_add(&pinfo_entry->list, &g_conn_mgr.panel_list);

	g_conn_mgr.base.next = next_pdev;
	g_conn_mgr.base.on_func = conn_mgr_on;
	g_conn_mgr.base.off_func = conn_mgr_off;
	g_conn_mgr.base.ops_handle_func = conn_mgr_ops_handle_parse;

	ret = platform_device_add_data(g_conn_mgr.pdev, &g_conn_mgr.base,
			sizeof(struct dpu_device_base));
	if (ret) {
		dpu_pr_err("conn mgr add data failed\n");
		return -1;
	}
	register_composer(g_conn_mgr.pdev, pinfo);

	dpu_pr_debug("-\n");
	return 0;
}

void unregister_connector(void)
{
	struct panel_info_entry *pinfo_entry;
	struct list_head *pos = NULL;
	struct list_head *list_head;

	dpu_pr_debug("+\n");
	unregister_composer();

	g_conn_mgr.pdev->platform_data = NULL;
	g_conn_mgr.base.next = NULL;

	list_head = &g_conn_mgr.panel_list;
	while(list_head->next != list_head) {
		pos = list_head->next;
		pinfo_entry = container_of(pos, struct panel_info_entry, list);
		dpu_check_and_no_retval(!pinfo_entry, "pinfo_entry or pinfo is null\n");
		list_del(pos);
		dpu_mem_free(pinfo_entry);

	}
	dpu_pr_debug("-\n");
}

int32_t dpu_conn_mgr_init()
{
	struct platform_device *this_dev;
	struct hw_list conn_hw_list;
	uint32_t i;

	dpu_pr_debug("+\n");
	conn_hw_list = get_support_conn_res();
	this_dev = platform_device_alloc("conn_mgr", 0);
	if (!this_dev) {
		dpu_pr_err("conn mgr dev alloc failed\n");
		return -1;
	}

	INIT_LIST_HEAD(&g_conn_mgr.panel_list);
	g_conn_mgr.pdev = this_dev;

	for (i = 0; i < conn_hw_list.res_num; i++) {
		struct connector *conn = dpu_mem_alloc(sizeof(*conn));
		if (!conn) {
			dpu_pr_err("connector alloc failed\n");
			return -1;
		}
		conn->id = conn_hw_list.res_list[i];
		g_conn_mgr.conn_list[i] = conn;
	}
	dpu_pr_debug("-\n");
	return 0;
}
