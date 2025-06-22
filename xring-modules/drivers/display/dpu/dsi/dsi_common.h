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

#ifndef _DSI_INTERNAL_H_
#define _DSI_INTERNAL_H_

#include "drm/drm_mipi_dsi.h"

#define MEM_DPU_BASE_ADDR 0xE7000000
#define MIPI_DCS_SET_TE_SCANLINE         0x44
#define MIPI_DCS_SET_COLUMN_ADDRESS      0x2A
#define MIPI_DCS_SET_PAGE_ADDRESS        0x2B

/*
 * DSI Command transfer flag
 * @DSI_CMD_FLAG_EXIT_ULPS:             The controller maybe in ulps, need exit.
 * @DSI_CMD_FLAG_ENTER_ULPS:            The current transfer need enter ulps.
 * @DSI_CMD_FLAG_GET_MIPI_STATE:        Command flag to indicate that this
 *                                      transfer should protected by mipi state.
 * @DSI_CMD_FLAG_CLEAR_MIPI_STATE:      Command flag to indicate that this
 *                                      transfer should clear mipi state.
 * @DSI_CMD_FLAG_EARLY_CLEAR_MIPI_STATE:Command flag to indicate that this
 *                                      transfer should early clear mipi state.
 * @DSI_CMD_FLAG_WAIT_CRI_AVAILABLE:    Transfer cmd packets after check CRI
 *                                      is available.
 * @DSI_CMD_FLAG_CRI_HOLD:              Transfer cmd packets with stack mode begin.
 * @DSI_CMD_FLAG_CRI_UNHOLD             Transfer cmd packets with stack mode end.
 */
#define  DSI_CMD_FLAG_EXIT_ULPS                  BIT(0)
#define  DSI_CMD_FLAG_ENTER_ULPS                 BIT(1)
#define  DSI_CMD_FLAG_GET_MIPI_STATE             BIT(2)
#define  DSI_CMD_FLAG_CLEAR_MIPI_STATE           BIT(3)
#define  DSI_CMD_FLAG_EARLY_CLEAR_MIPI_STATE     BIT(4)
#define  DSI_CMD_FLAG_WAIT_CRI_AVAILABLE         BIT(5)
#define  DSI_CMD_FLAG_CRI_HOLD                   BIT(6)
#define  DSI_CMD_FLAG_CRI_UNHOLD                 BIT(7)

enum dsi_cmd_set_priority {
	PRIORITY_LOW = 0,
	PRIORITY_NORMAL,
	PRIORITY_HIGH,
	PRIORITY_CRITICAL,
};

/**
 * The command transfer type, used only at the high level,
 * must be consistent with the definition at the low level.
 */
enum {
	USE_CMDLIST = 0,
	USE_CPU = 1,
};

/**
 * dsi_display_type - the dsi display pipe type
 * Usually use DSI_DISPLAY_PRIMARY for single mipi dsi, use both
 * DSI_DISPLAY_PRIMARY and DSI_DISPLAY_SECONDARY for dual mipi dsi
 */
enum dsi_display_type {
	DSI_DISPLAY_PRIMARY = 0,
	DSI_DISPLAY_SECONDARY,
	DSI_DISPLAY_MAX,
};

/**
 * dsi_port_type - the dsi port type
 * @DSI_PORT_0: dsi 0
 * @DSI_PORT_1: dsi 1
 * @DSI_PORT_MAX
 */
enum dsi_port_type {
	DSI_PORT_0 = 0,
	DSI_PORT_1,
	DSI_PORT_MAX,
};

/**
 * dsi_cmd_desc - description of a dsi command
 * @msg: dsi mipi msg packet
 * @post_wait_us: post wait duration
 */
struct dsi_cmd_desc {
	struct mipi_dsi_msg msg;
	u32 port_index;
	u32 post_wait_us;
#ifdef MI_DISPLAY_MODIFY
	bool last_command;
#endif
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
 * dsi_cmd_set_type - the enum of panel cmd set type
 * @DSI_CMD_SET_ON: the panel on cmd set
 * @DSI_CMD_SET_OFF: the panel off cmd set
 * @DSI_CMD_SET_DAMAGE_AREA: the panel set roi region cmd set,
 * not parsed from DTSI, generated dynamically.
 * @DSI_CMD_SET_TIMING_SWITCH: switch the panel timing
 * @DSI_CMD_SET_POST_TIMING_SWITCH: post switch the panel timing
 * @DSI_CMD_SET_FLAT_MODE_ON: switch on the panel flat mode
 * @DSI_CMD_SET_FLAT_MODE_OFF: switch off the panel flat mode
 * @DSI_CMD_SET_BACKLIGHT: the cmd set of update panel backlight,
 * not parsed from DTSI, generated dynamically.
 * @DSI_CMD_SET_AOD_ON: set panel aod on
 * @DSI_CMD_SET_AOD_OFF: set panel aod off
 * @DSI_CMD_SET_PANEL_STATUS: set panel status
 * @DSI_CMD_SET_MAX:  invalid panel cmd set
 */
enum dsi_cmd_set_type {
	DSI_CMD_SET_ON,
	DSI_CMD_SET_OFF,
	DSI_CMD_SET_DAMAGE_AREA,
	DSI_CMD_SET_TIMING_SWITCH,
	DSI_CMD_SET_POST_TIMING_SWITCH,
	DSI_CMD_SET_FLAT_MODE_ON,
	DSI_CMD_SET_FLAT_MODE_OFF,
	DSI_CMD_SET_BACKLIGHT,
	DSI_CMD_SET_AOD_ON,
	DSI_CMD_SET_AOD_OFF,
	DSI_CMD_SET_PANEL_STATUS,
#ifdef MI_DISPLAY_MODIFY
	DSI_CMD_SET_MI_DIMMINGON,
	DSI_CMD_SET_MI_DIMMINGOFF,
	DSI_CMD_SET_MI_APLDIMMING,
	DSI_CMD_SET_MI_HBM_ON,
	DSI_CMD_SET_MI_HBM_OFF,
	DSI_CMD_SET_NOLP,
	DSI_CMD_SET_MI_DOZE_HBM,
	DSI_CMD_SET_MI_DOZE_LBM,
	DSI_CMD_SET_MI_DOZE_HBM_NOLP,
	DSI_CMD_SET_MI_DOZE_LBM_NOLP,
	DSI_CMD_SET_MI_FLAT_MODE_ON,
	DSI_CMD_SET_MI_FLAT_MODE_OFF,
	DSI_CMD_SET_MI_FLAT_MODE_SEC_ON,
	DSI_CMD_SET_MI_FLAT_MODE_SEC_OFF,
	DSI_CMD_SET_MI_PREPARE_READ_FLAT,
	DSI_CMD_SET_MI_PREPARE_READ_FLAT_OFF,
	DSI_CMD_SET_MI_DC_ON,
	DSI_CMD_SET_MI_DC_OFF,
	DSI_CMD_SET_MI_ROUND_CORNER_ON,
	DSI_CMD_SET_MI_ROUND_CORNER_PORT0_ON,
	DSI_CMD_SET_MI_ROUND_CORNER_PORT1_ON,
	DSI_CMD_SET_MI_ROUND_CORNER_OFF,
	DSI_CMD_SET_MI_FRAME_SWITCH_MODE_SEC,
	DSI_CMD_SET_MI_DOZE_TO_OFF,
	DSI_CMD_SET_MI_DOZE_PARAM_READ,
	DSI_CMD_SET_MI_DOZE_PARAM_READ_END,
	DSI_CMD_SET_MI_PANEL_STATUS_OFFSET,
	DSI_CMD_SET_MI_PANEL_STATUS_AFTER,
	DSI_CMD_SET_MI_PANEL_BUILD_ID,
	DSI_CMD_SET_MI_PANEL_BUILD_ID_SUB_WRITE_1,
	DSI_CMD_SET_MI_PANEL_BUILD_ID_SUB_WRITE_2,
	DSI_CMD_SET_MI_PANEL_BUILD_ID_SUB_READ,
	DSI_CMD_SET_MI_PANEL_CELL_ID_READ,
	DSI_CMD_SET_MI_PANEL_CELL_ID_READ_PRE_TX,
	DSI_CMD_SET_MI_PANEL_CELL_ID_READ_AFTER_TX,
	DSI_CMD_SET_MI_PANEL_WP_READ,
	DSI_CMD_SET_MI_PANEL_WP_READ_PRE_TX,
	DSI_CMD_SET_MI_FLATMODE_STATUS,
	DSI_CMD_SET_MI_FLATMODE_STATUS_OFFSET,
	DSI_CMD_SET_MI_FLATMODE_STATUS_OFFSET_END,
	DSI_CMD_SET_MI_TIMING_SWITCH_FROM_AUTO,
	DSI_CMD_SET_MI_TIMING_SWITCH_FROM_SKIP,
	DSI_CMD_SET_MI_TIMING_SWITCH_FROM_NORMAL,
	DSI_CMD_SET_MI_DEMURA_HBM,
	DSI_CMD_SET_MI_DEMURA_LBM,
	DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_OFF_MODE,
	DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_32_36_MODE,
	DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_36_40_MODE,
	DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_40_MODE,
	DSI_CMD_SET_MI_CSC_BY_TEMPER_COMP_45_MODE,
	DSI_CMD_SET_MI_XEQ_SYNC,
#endif
	DSI_CMD_SET_MAX,
};

/**
 * dsi_cmd_set_update_info - command set update info
 * @index: the index of updated cmd
 * @addr: the MIPI address of updated cmd
 * @len: number of bytes to be updated
 */
struct dsi_cmd_set_update_info {
	u32 index;
	u32 addr;
	u32 len;
};

/**
 * dsi_cmd_set - command set of the panel
 * @type: type of the command set
 * @cmd_state: cmd set transfer state
 * @port_index: transfer cmds by this DSI port
 * @num_cmds: number of cmds
 * @cmds: array of cmds
 * @num_infos: number of cmd set update_info;
 * @infos: array of cmd set update info
 * @payload_size: the total payload size of this cmd set
 * @priority_level: priority level for this cmd set
 */
struct dsi_cmd_set {
	enum dsi_cmd_set_type type;
	enum dsi_cmd_set_state cmd_state;
	int port_index;

	u32 num_cmds;
	struct dsi_cmd_desc *cmds;
	u32 num_infos;
	struct dsi_cmd_set_update_info *infos;
	u32 payload_size;
	u8 priority_level;
	bool delete_after_usage;
};

#endif
