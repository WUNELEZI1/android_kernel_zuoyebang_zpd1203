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

#ifndef DPU_CONN_MGR_H
#define DPU_CONN_MGR_H

#include "dpu_list.h"
#include "dpu_common_info.h"
#include "dpu_pipeline.h"
#include "platform_device.h"
#include "pipeline_hw_res.h"
#include "dpu_comp_mgr.h"
#include "dpu_res_mgr.h"
#include "dpu_hw_common.h"
#include "dpu_hw_dsi.h"

struct panel_timing {
	uint32_t framerate;

	uint32_t hdisplay;
	uint32_t hfp;
	uint32_t hpw;
	uint32_t hbp;
	uint32_t h_skew;

	uint32_t vdisplay;
	uint32_t vfp;
	uint32_t vpw;
	uint32_t vbp;
};

struct dsi_hw_res {
	struct dpu_hw_blk ctrl_blk;
	struct dpu_hw_blk sctrl_blk;
	struct dpu_hw_blk phy_blk;
};

/**
 * struct connector - connector object
 * @pinfo: Variables in pinfo cannot be changed
 * @mipi: mipi timing info
 * @id: logical connector id
 * @scene_id: scene_id
 * @hw_tmg: tmg hw res
 * @bind_connector: used for dual mipi
 * @on: dsi on ops
 * @off: dsi off ops
 */
struct connector {
	struct dpu_panel_info *pinfo;
	struct panel_timing timing;
	enum phy_select phy_sel;
	enum dsi_phy_ppi_width ppi_width;
	uint32_t id;
	uint32_t scene_id;
	struct dpu_hw_blk hw_tmg;
	struct dsi_hw_res dsi_hw_res;
	struct connector *bind_connector;
	int32_t (*on)(struct platform_device *pdev, struct connector *conn, uint32_t scene_id);
	int32_t (*off)(struct platform_device *pdev, struct connector *conn, uint32_t scene_id);
};

/**
 * struct conn_mgr - use to manager all connectors
 * @base: base dev
 * @conn_list: conn list parsed from pipeline_hw_res
 * @pdev: pdev of conn_mgr
 * @panel_list: panels mounted on comp_mgr
 */
struct conn_mgr {
	struct dpu_device_base base;
	struct connector *conn_list[CONNECTOR_MAX];
	struct platform_device *pdev;
	struct list_head panel_list;
};

/**
 * register_connector - reverse register connector dev
 *
 * @next_pdev: rear pdev(panel mgr dev)
 * @pinfo: panel info
 * @return 0 on success, -1 on failure
 */
int32_t register_connector(struct platform_device *next_pdev,
		struct dpu_panel_info *pinfo);

void unregister_connector(void);

/**
 * dpu_conn_mgr_init - init conn mgr
 *
 * @return 0 on success, -1 on failure
 */
int32_t dpu_conn_mgr_init(void);

/**
 * get_connector - Get the connector object
 *
 * @conn_id: logical connector id
 * @return 0 on success, -1 on failure
 */
struct connector *get_connector(uint32_t conn_id);

/**
 * conn_mgr_prepare - prepare info for scene_id connector
 *
 * @scene_id: logical scene id
 */
void conn_mgr_prepare(uint32_t scene_id);
void conn_mgr_unprepare(uint32_t scene_id);
struct dpu_panel_info *conn_mgr_panel_info_get(uint32_t conn_id);
#endif
