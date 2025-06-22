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

#ifndef _DSI_HOST_H_
#define _DSI_HOST_H_

#include "dsi_display.h"

int dsi_host_init(struct mipi_dsi_host *host, struct device *dev);

void dsi_host_deinit(struct mipi_dsi_host *host);

enum {
	NO_NEED_EXIT_ULPS = 0,
	NEED_EXIT_ULPS,
};

/*
 * function - Perform DSI command transfer operation
 *
 * This function is used to execute DSI command transfers, controlling the
 * transfer behavior based on the input parameters.
 * It allows specifying the transfer type, transfer flags, etc. The transfer
 * flags can be used to control the state of the DSI controller, such as exiting
 * or entering the ULPS mode, getting or clearing the MIPI state, and waiting
 * for the CRI to be available.
 *
 * @host:           A pointer to the mipi_dsi_host structure, representing the
 *                  DSI host device. This structure typically contains relevant
 *                  information about the DSI host and function pointers for
 *                  interacting with the hardware.
 * @cmd_desc:       A pointer to the dsi_cmd_desc structure, which describes the
 *                  DSI command to be transferred. This structure may contain
 *                  details such as the specific content and length of the command.
 * @transfer_type:  The transfer type, specifying the way the DSI command is
 *                  transferred, such as synchronous transfer or asynchronous
 *                  transfer. It can be a combination of the following flags:
 *                  USE_CPU
 *                  USE_CMDLIST
 * @transfer_flag:  Transfer flags used to control some special behaviors during
 *                  the DSI transfer, defined in dsi_common.h.
 *                  It can be a combination of the following flags:
 *                  DSI_CMD_FLAG_EXIT_ULPS
 *                  DSI_CMD_FLAG_ENTER_ULPS
 *                  DSI_CMD_FLAG_GET_MIPI_STATE
 *                  DSI_CMD_FLAG_CLEAR_MIPI_STATE
 *                  DSI_CMD_FLAG_WAIT_CRI_AVAILABLE
 *
 * Return:
 * Returns 0 on success, indicating that the DSI command transfer operation was
 * executed successfully. Returns a negative error code on failure.
 */
int dsi_host_cmd_transfer(struct mipi_dsi_host *host,
		struct dsi_cmd_desc *cmd_desc, u8 transfer_type, u32 transfer_flag);

#endif
