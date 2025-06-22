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

#ifndef _DSI_CRTL_H_
#define _DSI_CRTL_H_

#include "dsi_common.h"
#include "dsi_ctrl_hw.h"

#define dsi_ctrl_cmd_send(ctrl, cmd_desc, transfer_type, ret) \
	do { \
		if (transfer_type == USE_CMDLIST) \
			ret = dsi_ctrl_cmdlist_cmd_send(ctrl, cmd_desc); \
		else \
			ret = dsi_ctrl_cpu_cmd_send(ctrl, cmd_desc); \
	} while (0)

/**
 * dsi_ctrl - dsi control structure
 * @dev: parent device of dsi ctrl
 * @dsi_ctrl_hw: the instance of hardware controller
 * @list: ctrl entry in registry
 * @ctrl_lock: mutex lock for dsi ctrl
 */
struct dsi_ctrl {
	struct device *dev;
	struct dsi_ctrl_hw ctrl_hw;
	struct list_head list;
	struct mutex ctrl_lock;
};

inline void dsi_ctrl_lock(struct dsi_ctrl *ctrl);

inline void dsi_ctrl_unlock(struct dsi_ctrl *ctrl);

/**
 * dsi_ctrl_cpu_cmd_send - send dsi cmd desc by cpu
 * @ctrl: the dsi ctrl pointer
 * @cmd_set: cmd set tobe send
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_cpu_cmd_send(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd_desc);

/**
 * dsi_ctrl_cmdlist_cmd_send - send dsi cmd desc by cmdlist
 * @ctrl: the dsi ctrl pointer
 * @cmd_set: cmd set tobe send
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_cmdlist_cmd_send(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd_desc);

/**
 * dsi_ctrl_cmd_read_pre_tx - send pre tx cmd for read
 * @ctrl: the dsi ctrl pointer
 * @cmd: cmd set tobe send
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_cmd_read_pre_tx(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd);

/**
 * dsi_ctrl_hw_blk_id_get - get the controller hardware id
 * @ctrl: the dsi ctrl pointer
 *
 * Return: hardware id or a negative number on failure.
 */
inline int dsi_ctrl_hw_blk_id_get(struct dsi_ctrl *ctrl);

/**
 * dsi_ctrl_add - add the dsi ctrl to the ctrl list
 * @ctrl: the dsi ctrl phandle
 */
void dsi_ctrl_add(struct dsi_ctrl *ctrl);

/**
 * dsi_ctrl_remove - remove the dsi ctrl from the ctrl list
 * @ctrl: the dsi ctrl phandle
 */
void dsi_ctrl_remove(struct dsi_ctrl *ctrl);

/**
 * dsi_ctrl_init - init the ctrl hw
 * @ctrl: the dsi ctrl pointer
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_pre_init - pre init the ctrl hw
 * @ctrl: the dsi ctrl pointer
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_pre_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_phy_init - ctrl init the phy
 * @ctrl: the dsi ctrl pointer
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_phy_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_hw_init - init the dsi ctrl and phy
 * @ctrl: dsi ctrl phandle
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_hw_init(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_hw_wait_ready - init the dsi ctrl and phy
 * @ctrl: dsi ctrl phandle
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_hw_wait_ready(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_cmd_transfer - transfer a dsi cmd descriptor
 * @ctrl: dsi ctrl phandle
 * @cmd_desc: cmd descriptor tobe transfer
 * @transfer_type: USE_CPU or USE_CMDLIST.
 * @wait_timeout: wait cri busy timeout in ms
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_cmd_transfer(struct dsi_ctrl *ctrl,
		struct dsi_cmd_desc *cmd_desc, u8 transfer_type, u32 wait_timeout);

/**
 * dsi_ctrl_enable - enable the ctrl hw
 * @ctrl: dsi ctrl phandle
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_enable(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_ulps_enter - enter the ulps mode
 * @ctrl: dsi ctrl phandle
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_ulps_enter(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_ulps_exit - exit ulps
 * @ctrl: dsi ctrl phandle
 * @cfg: the pointer of dsi ctrl configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_ulps_exit(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_check_ulps_state - check ulps state
 * @ctrl: dsi ctrl phandle
 * @enter: enter or exit ulps
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_check_ulps_state(struct dsi_ctrl *ctrl, bool enter);

/**
 * dsi_ctrl_mipi_freq_update - modify mipi frequency
 * @ctrl: dsi ctrl phandle
 * @cfg: dsi ctrl cfg
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_mipi_freq_update(struct dsi_ctrl *ctrl, struct dsi_ctrl_cfg *cfg);

/**
 * dsi_ctrl_debug - dsi controller and cdphy debug interface
 * @ctrl: dsi ctrl phandle
 * @type: debug function select
 * @parm_cnt: parameters count
 * @parm: debug parmeters
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_debug(struct dsi_ctrl *ctrl,
		enum dsi_debug type, u8 parm_cnt, u8 *parm);

/**
 * dsi_ctrl_cri_is_busy - dsi ctrl cri is busy
 * @ctrl: dsi ctrl phandle
 * @wait_timeout: threshold for timeout
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_ctrl_cri_is_busy(struct dsi_ctrl *ctrl, u32 wait_timeout);

/**
 * dsi_ctrl_cri_tx_ctrl - config CRI TX ctrl
 * @ctrl: dsi ctrl phandle
 * @cri_mode: the dsi cri mode
 * @cri_hold: the ctrl flag of stack mode
 *
 * Return: 0 on success or a negative number on failure.
 */

int dsi_ctrl_cri_tx_ctrl(struct dsi_ctrl *ctrl, u8 cri_mode, u8 cri_hold);

#if defined(CONFIG_OF)
/**
 * of_find_dsi_ctrl - find the dsi ctrl by device node from the ctrl list
 * @np: the device node pointer
 *
 * Return: dsi phy pointer on success, error pointer on failure
 */
struct dsi_ctrl *of_find_dsi_ctrl(const struct device_node *np);
#else
struct dsi_ctrl *of_find_dsi_ctrl(const struct device_node *np)
{
	return ERR_PTR(-ENODEV);
}
#endif

#endif /* _DSI_CRTL_H_ */
