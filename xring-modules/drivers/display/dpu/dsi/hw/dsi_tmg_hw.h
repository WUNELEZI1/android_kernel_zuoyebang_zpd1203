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

#ifndef _DSI_TMG_HW_H_
#define _DSI_TMG_HW_H_

#include <linux/platform_device.h>

#include "dsi_hw_tmg_ops.h"
#include "dpu_reg_ops.h"
#include "dpu_hw_dsi.h"

struct dsi_tmg_hw;

struct dsi_tmg_hw_ops {
	/**
	 * init - the initialized callback of timing generator
	 * @tmg: the pointer of timing generator
	 * @cfg: the config data pointer of timing generator
	 */
	void (*init)(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

	/**
	 * enable - enable the timing generator
	 * @tmg: the pointer of timing generator
	 */
	void (*enable)(struct dpu_hw_blk *hw);

	/**
	 * disable - disable the timing generator
	 * @tmg: the pointer of timing generator
	 */
	void (*disable)(struct dpu_hw_blk *hw);

	/**
	 * ulps_enter - enter the ulps mode（only for video mode）
	 * @hw: the tmg hw blk
	 * @cfg: the config data pointer of timing generator
	 *
	 * Return: 0 on success or a negative number on failure.
	 */
	int (*ulps_enter)(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

	/**
	 * ulps_exit - exit the ulps mode（only for video mode）
	 * @hw: the tmg hw blk
	 *
	 * Return: 0 on success or a negative number on failure.
	 */
	int (*ulps_exit)(struct dpu_hw_blk *hw, u8 flag);

	/**
	 * timing_setup - set timing for timing generator
	 * @tmg: the pointer of timing generator
	 * @cfg: the config data pointer of timing generator
	 */
	void (*timing_setup)(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

	/**
	 * partial_update - update frame parameters
	 * @tmg: the pointer of timing generator
	 * @cfg: the config data pointer of timing generator
	 */
	void (*partial_update)(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

	/**
	 * frame_update - update tmg parameters
	 * @tmg: the pointer of timing generator
	 * @cfg: the config data pointer of timing generator
	 */
	void (*frame_update)(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

	/**
	 * timestamp_get - get tmg time stamp
	 * @hw: the pointer of tmg hw block
	 */
	u64 (*timestamp_get)(struct dpu_hw_blk *hw);

	/**
	 * dyn_freq_update - update tmg timing for dynamic frequency
	 * @hw: the pointer of tmg hw block
	 * @cfg: the config data pointer of timing generator
	 */
	void (*dyn_freq_update)(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg);

	/**
	 * doze_enable - enable dpu doze(lowpower feature on tmg)
	 * @hw: the pointer of tmg hw block
	 */
	void (*doze_enable)(struct dpu_hw_blk *hw);

	/**
	 * status_dump - dump tmg's hw status
	 * @hw: the pointer of tmg hw block
	 */
	void (*status_dump)(struct dpu_hw_blk *hw);
};

/**
 * dsi_tmg_hw - xring dsi timing generator
 * @hw: the instance of hardware dsi timing generator
 * @funcs: the support functions callback
 */
struct dsi_tmg_hw {
	struct dpu_hw_blk hw_blk;
	struct dsi_tmg_hw_ops ops;
};

int dsi_tmg_hw_parse(struct platform_device *pdev,
		struct device_node *np, struct dsi_tmg_hw *tmg_hw);

#endif
