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

#ifndef _PANEL_MGR_H_
#define _PANEL_MGR_H_

#include "osal.h"
#include "dpu_pipeline.h"
#include "dpu_panel_interface.h"
#include "dpu_conn_mgr.h"
#include "dpu_common_info.h"
#include "mipi_dsi_dev.h"

enum PANEL_ID {
	PANEL_INVALID = -1,
	PANEL_PRIMARY,
	PANEL_SECONDARY,
	PANEL_OFFLINE,
	PANEL_MAX,
};

enum PANEL_BUILD_ID {
	PANEL_BUILD_P01 = 0x01,
	PANEL_BUILD_P10 = 0x10,
	PANEL_BUILD_P11 = 0x11,
	PANEL_BUILD_P12 = 0x12,
	PANEL_BUILD_P20 = 0x20,
	PANEL_BUILD_P21 = 0x21,
	PANEL_BUILD_MP = 0xAA,
	PANEL_BUILD_MAX = 0xFF
};

#define panel_device_match_data(name, setup, release) \
	struct panel_match_data name = {.of_device_setup = setup, .of_device_release = release }

#define to_panel_priv(pinfo) container_of(pinfo, struct panel_drv_private, base)

struct panel_match_data {
	int32_t (*of_device_setup)(struct platform_device *pdev);
	void (*of_device_release)(struct platform_device *pdev);
};

/**
 * struct gpio_state - gpio sequence
 *
 * @level: gpio level
 * @delay: delay ms after gpio operation
 */
struct seq_ops {
	uint32_t level;
	uint32_t delay;
};

enum io_type {
	INVALID_TYPE = 0,
	GPIO_TYPE,
	LDO_TYPE,
};

#define MAX_IO_OPS_SEQ_NUM 5

struct io_seq_info {
	enum io_type io_type;
	uint32_t id;
	uint32_t ops_num;
	struct seq_ops seq[MAX_IO_OPS_SEQ_NUM];
};

struct panel_seq_ctrl {
	uint32_t seq_num;
	struct io_seq_info *seq_info;
};

/**
 * dsi_cmd_set_type - the enum of panel cmd set type
 * @DSI_CMD_SET_ON: the panel on cmd set
 * @DSI_CMD_SET_OFF: the panel off cmd set
 * @DSI_CMD_SET_MAX:  invalid panel cmd set
 */
enum dsi_cmd_set_type {
	DSI_CMD_SET_ON,
	DSI_CMD_SET_OFF,
	DSI_CMD_SET_BACKLIGHT,
	DSI_CMD_SET_READ_BUILD_ID,
	DSI_CMD_SET_XEQ_SYNC,
	DSI_CMD_SET_MAX,
};

/**
 * dsi_cmd_set_state - command set state
 * @DSI_CMD_SET_STATE_HS:   dsi high speed mode
 * @DSI_CMD_SET_STATE_LP:   dsi low power mode
 * @DSI_CMD_SET_STATE_MAX
 */
enum dsi_cmd_set_state {
	DSI_CMD_SET_STATE_HS = 0,
	DSI_CMD_SET_STATE_LP,
	DSI_CMD_SET_STATE_MAX
};

/**
 * dsi_cmd_set - command set of the panel
 * @type: type of the command
 * @cmd_state: state of the command
 * @num_cmds: number of cmds
 * @cmds: arry of cmds
 */
struct dsi_cmd_set {
	enum dsi_cmd_set_type type;
	enum dsi_cmd_set_state cmd_state;
	uint32_t num_cmds;
	struct dsi_cmd_desc *cmds;
};

struct panel_build_id_config {
	struct dsi_cmd_set id_cmd;
	u32 id_cmds_rlen;
	u8 build_id;
};

/**
 * struct panel_drv_private - private panel info
 */
struct panel_drv_private {
	struct dpu_panel_info base;
	struct panel_seq_ctrl power_on_seq_ctrl;
	struct panel_seq_ctrl power_off_seq_ctrl;
	struct panel_seq_ctrl reset_on_seq_ctrl;
	struct panel_seq_ctrl reset_off_seq_ctrl;
	struct panel_build_id_config id_config;
	bool panel_xeq_enabled;

	bool reset_after_lp11;
	uint32_t lcd_te;
	struct platform_device *pdev;

	struct dsi_cmd_set cmd_sets[DSI_CMD_SET_MAX];
	uint32_t id;

	bool is_fake_panel;
};

/* used to match panel for composer and connector */
struct panel_info_entry {
	struct dpu_panel_info *pinfo;
	struct list_head list;
};

/**
 * struct panel_dev - panel dev data
 * @pdev: pdev of specific panel dev
 * @on: panel on func
 * @on: panel reset func
 * @off: panel off func
 * @handle_table: other panel ops
 * @ops_size: ops nums use with handle_table
 */
struct panel_dev {
	struct platform_device *pdev;

	int32_t (*on)(struct platform_device *pdev);
	int32_t (*off)(struct platform_device *pdev);
	struct ops_handle_table *handle_table;
	uint32_t ops_size;
};

/**
 * struct panel_mgr - panel mgr data
 * @base: base dev
 * @pdev: pdev of panel_mgr
 * @panel_list: total panel dev
 */
struct panel_mgr {
	struct dpu_device_base base;
	struct platform_device *pdev;
	struct panel_dev *panel_list[PANEL_MAX];
};

/**
 * register_panel - register panel dev
 *
 * @panel_type: panel type, online or offline. necessary
 * @panel_name: panel name. necessary
 * @return 0 on success, -1 on failure
 */
int32_t register_panel(enum PANEL_ID panel_type, const char *panel_name);

void unregister_panel(enum PANEL_ID panel_id);

/**
 * panel_init - init panel mgr
 *
 * @return 0 on success, -1 on failure
 */
int32_t panel_init(void);

/**
 * panel_probe - panel drv probe func
 *
 * @pdev: panel pdev
 * @return 0 on success, -1 on failure
 */
int32_t panel_probe(struct platform_device *pdev);

/*
 * List all the screen driver init functions,
 * and all the new header files that need to be added here
 */
extern struct panel_dev *get_panel_dev(uint32_t panel_id);
extern int32_t get_panel_id(uint32_t connector_id);

/**
 * dsi_panel_info_parse - parse all panel info from dts
 *
 * @priv: the panel preivate data
 * @panel_node_name: the panel node name
 * @return 0 on success, -1 on failure
 */
int dsi_panel_info_parse(struct panel_drv_private *priv,
		const char *panel_node_name);

void dsi_panel_info_release(struct panel_drv_private *priv);

/**
 * dsi_panel_offset_get - get dts offset of panel
 *
 * @fdt: fdt handle
 * @panel_node_name: the dsi panel node name

 * @return -1 on failure, otherwise on success
 */
int dsi_panel_offset_get(void *fdt, const char *panel_node_name);

#endif
