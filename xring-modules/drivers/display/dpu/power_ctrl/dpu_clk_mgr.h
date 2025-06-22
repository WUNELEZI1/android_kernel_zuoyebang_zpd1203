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

#ifndef _DPU_CLK_MGR_H_
#define _DPU_CLK_MGR_H_

#include <linux/types.h>
#include <linux/clk.h>
#include <drm/drm_crtc.h>

#include "dpu_hw_power_ops.h"

struct dpu_power_mgr;

#define DPU_CLK_NAME_LEN                                  50
#define CLK_MARGIN                                        100000

enum dpu_clk_type {
	CLK_DPU_CORE0,
	CLK_DPU_CORE1,
	CLK_DPU_CORE2,
	CLK_DPU_CORE3,
	CLK_DPU_AXI0,
	CLK_DPU_AXI1,
	CLK_DPU_AXI2,
	CLK_DPU_AXI3,
	CLK_DPU_BUS_DATA,
	CLK_DPU_PCLK,
	CLK_DPU_BUS_CFG,
	CLK_DSI_CFG,
	CLK_DPU_DSC0,
	CLK_DPU_DSC1,
	DPU_CLK_MAX_NUM,
};

struct dpu_clock {
	enum dpu_clk_type type;
	char clk_name[DPU_CLK_NAME_LEN];
	u32 part_id;
};

/**
 * dpu_clk_mgr - clk mgr object
 *
 * @dpu_clk: all dpu clk that need dynamic manager
 * @clk_count: clk vote count for each partition
 * @lock: mutex lock for clk_count
 */
struct dpu_clk_mgr {
	struct clk *dpu_clk[DPU_CLK_MAX_NUM];

	u32 clk_count[DPU_PARTITION_MAX_NUM];

	struct mutex lock;
};

/**
 * dpu_clk_mgr_init - init clk mgr
 *
 * @power_mgr: struct dpu_power_mgr object
 * Returns success (0) or negative errno.
 */
int dpu_clk_mgr_init(struct dpu_power_mgr *power_mgr);

/**
 * dpu_clk_mgr_deinit - deinit clk mgr
 *
 * @power_mgr: struct dpu_power_mgr object
 */
void dpu_clk_mgr_deinit(struct dpu_power_mgr *power_mgr);

/**
 * dpu_clk_reset_clk_rate - set reset clk rate
 *
 * @clk_mgr: struct dpu_clk_mgr object
 */
void dpu_clk_reset_clk_rate(struct dpu_clk_mgr *clk_mgr);

/**
 * dpu_clk_enable_for_partition - enable clks of one partition
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @part_id: partition id
 * Returns success (0) or negative errno.
 */
int dpu_clk_enable_for_partition(struct dpu_clk_mgr *clk_mgr, u32 part_id);

/**
 * dpu_clk_disable_for_partition - disable clks of one partition
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @part_id: partition id
 */
void dpu_clk_disable_for_partition(struct dpu_clk_mgr *clk_mgr, u32 part_id);

/**
 * dpu_clk_set_debug - set clk rate for debug
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @clk_type: clk type
 * @rate: clk rate
 * Returns success (0) or negative errno.
 */
int dpu_clk_set_debug(struct dpu_clk_mgr *clk_mgr, u32 clk_type, unsigned long rate);

/**
 * dpu_clk_enable - enable dpu clk
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @clk_type: clk type
 */
int dpu_clk_enable(struct dpu_clk_mgr *clk_mgr, u32 clk_type);
/**
 * dpu_clk_disable - disable dpu clk
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @clk_type: clk type
 */
void dpu_clk_disable(struct dpu_clk_mgr *clk_mgr, u32 clk_type);

/**
 * dpu_clk_ctrl - ctrl dpu clk
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @clk_type: clk type
 * @enable: enable or disable clk
 */
void dpu_clk_ctrl(struct dpu_clk_mgr *clk_mgr, u32 clk_type, bool enable);

/**
 * dpu_get_clk_name - get clk's string name
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @clk_type: clk type
 * Returns clk's name.
 */
const char *dpu_get_clk_name(struct dpu_clk_mgr *clk_mgr, u32 clk_type);

/**
 * dpu_clk_get_rate - get the clk type's clk rate
 *
 * @clk_mgr: struct dpu_clk_mgr object
 * @clk_type: clk type
 * Returns clk's rate
 */
unsigned long dpu_clk_get_rate(struct dpu_clk_mgr *clk_mgr, u32 clk_type);

/**
 * dpu_clk_set_rate - set clk type's clk rate
 * @clk_mgr struct dpu_clk_mgr object
 * @clk_type: clk_type
 * @rate: clk rate to set
 */
void dpu_clk_set_rate(struct dpu_clk_mgr *clk_mgr,
		u32 clk_type, unsigned long rate);

#endif
