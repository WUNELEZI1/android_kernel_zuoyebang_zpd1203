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

#ifndef _DSI_TMG_H_
#define _DSI_TMG_H_

#include "hw/dsi_tmg_hw.h"

/**
 * dsi_tmg - dsi tmg structure
 * @tmg_hw: tmg hardware instance
 */
struct dsi_tmg {
	struct dsi_tmg_hw tmg_hw;
};

/**
 * dsi_tmg_frame_update - Update timing engine config.
 * @tmg: The dsi tmg phandle.
 * @cfg: The pointer of tmg configure.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_frame_update(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg);

/**
 * dsi_tmg_id_get - Get the timing engine hardware id.
 * @tmg: The phandle of dsi tmg.
 *
 * Return: hardware id or a negative number on failure.
 */
int dsi_tmg_id_get(struct dsi_tmg *tmg);

/**
 * dsi_tmg_create - create a tmg instance by given tmg of node
 * @pdev: the dsi platform device phandle
 * @np: the tmg of node phanlde
 *
 * Return: dsi tmg pointer on success, error pointer on failure.
 */
struct dsi_tmg *dsi_tmg_create(struct platform_device *pdev,
		struct device_node *np);

/**
 * dsi_tmg_destroy - destroy a tmg instance and release reserved resources.
 * @tmg: the dsi tmg phandle
 */
void dsi_tmg_destroy(struct dsi_tmg *tmg);

/**
 * dsi_tmg_enable - enable the tmg engine
 * @tmg: the dsi tmg phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_enable(struct dsi_tmg *tmg);

/**
 * dsi_tmg_destroy - disable the tmg engine
 * @tmg: the dsi tmg phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_disable(struct dsi_tmg *tmg);

/**
 * dsi_tmg_hw_init - init the timing engine hardware
 * @tmg: the dsi tmg phandle
 * @cfg: the pointer of tmg configure
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_hw_init(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg);

/**
 * dsi_tmg_partial_update - update the roi tmg cfg
 * @tmg: the dsi tmg phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_partial_update(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg);

/**
 * dsi_tmg_ulps_enter - enter the ulps mode（only for video mode）
 * @tmg: the pointer of timing generator
 * @cfg: the config data pointer of timing generator
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_ulps_enter(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg);

/**
 * ulps_exit - exit the ulps mode（only for video mode）
 * @tmg: the pointer of timing generator
 * @transfer_type: use cpu or cmdlist, only valid in cmd send.
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_ulps_exit(struct dsi_tmg *tmg, u8 transfer_type);

/**
 * dsi_tmg_timestamp_get - get tmg timie stamp, uint: ns
 * @tmg: the pointer of timing generator
 * Return: timestamp.
 */
u64 dsi_tmg_timestamp_get(struct dsi_tmg *tmg);

/**
 * dsi_tmg_dyn_freq_update - update tmg timing for dynamic frequency
 * @tmg: the pointer of timing generator
 * @cfg: dsi tmg config
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_dyn_freq_update(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg);

/**
 * dsi_tmg_doze_enable - enable the tmg doze feature
 * @tmg: the dsi tmg phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_doze_enable(struct dsi_tmg *tmg);

/**
 * dsi_tmg_status_dump - dump the tmg's hw status
 * @tmg: the dsi tmg phandle
 *
 * Return: 0 on success or a negative number on failure.
 */
int dsi_tmg_status_dump(struct dsi_tmg *tmg);

#endif
