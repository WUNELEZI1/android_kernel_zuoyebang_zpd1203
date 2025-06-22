/* SPDX-License-Identifier: GPL-2.0-only */
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

#ifndef _DPU_HW_INIT_MODULE_OPS_H_
#define _DPU_HW_INIT_MODULE_OPS_H_

#include "dpu_hw_common.h"

enum {
	DISABLE_AUTO_CG = 0,
	ENABLE_AUTO_CG = 1,
};

enum {
	DISABLE_SRAM_LP = 0,
	ENABLE_SRAM_LP = 1,
};

/*
 * dpu_hw_init_cfg - config info for hw init
 * @sram_lp_cfg: dpu sram low power switch
 * @auto_cg_cfg: dpu mclk,aclk auto cg switch
 */
struct dpu_hw_init_cfg{
	u32 sram_lp_cfg;
	u32 auto_cg_cfg;
};

/*
 * dpu_hw_auto_cg_disable - config auto cg disable
 * @part_id: partion id
 */
void dpu_hw_auto_cg_disable(u32 part_id);
/*
 * dpu_hw_auto_cg_enable - config auto cg enable
 * @part_id: partion id
 */
void dpu_hw_auto_cg_enable(u32 part_id);
void dpu_hw_do_hw_init(u32 part_id, struct dpu_hw_init_cfg *cfg);
void dpu_hw_auto_cg_reset(u32 part_id);
void dpu_hw_top_clk_auto_cg_cfg(u32 op);
void dpu_hw_tmg_mclk_auto_cg_dump(void);
void dpu_hw_init_module_init(DPU_IOMEM dpu_base);
void dpu_hw_init_module_deinit(void);

#endif
