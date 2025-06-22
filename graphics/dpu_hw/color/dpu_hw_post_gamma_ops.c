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

#include "dpu_color_common.h"
#include "dpu_hw_post_gamma_reg.h"
#include "dpu_hw_post_gamma_ops.h"

void dpu_hw_post_gamma_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, USR_GMA_EN_OFFSET, enable,
			CMDLIST_WRITE);
}

void dpu_hw_post_gamma_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_gamma_cfg *gma;
	u32 val;
	int i;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*gma)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*gma));
	}

	gma = hw_cfg->data;

	for (i = 0; i < (GAMMA_CURVE_SIZE >> 1); i++) {
		val = 0;
		val = MERGE_BITS(val, gma->r[i << 1], M_NCURVE_A_000_SHIFT,
				M_NCURVE_A_000_LENGTH);
		val = MERGE_BITS(val, gma->r[(i << 1) + 1],M_NCURVE_A_001_SHIFT,
				M_NCURVE_A_001_LENGTH);
		DPU_REG_WRITE(hw, M_NCURVE_A_000_OFFSET + (i << 2), val, CMDLIST_WRITE);

		val = MERGE_BITS(val, gma->g[i << 1], M_NCURVE_A_000_SHIFT,
				M_NCURVE_A_000_LENGTH);
		val = MERGE_BITS(val, gma->g[(i << 1) + 1], M_NCURVE_A_001_SHIFT,
				M_NCURVE_A_001_LENGTH);
		DPU_REG_WRITE(hw, M_NCURVE_B_000_OFFSET + (i << 2), val, CMDLIST_WRITE);

		val = MERGE_BITS(val, gma->b[i << 1], M_NCURVE_A_000_SHIFT,
				M_NCURVE_A_000_LENGTH);
		val = MERGE_BITS(val, gma->b[(i << 1) + 1], M_NCURVE_A_001_SHIFT,
				M_NCURVE_A_001_LENGTH);
		DPU_REG_WRITE(hw, M_NCURVE_C_000_OFFSET + (i << 2), val, CMDLIST_WRITE);
	}

	DPU_REG_WRITE(hw, M_NCURVE_A_256_OFFSET, gma->r[GAMMA_CURVE_SIZE - 1], CMDLIST_WRITE);
	DPU_REG_WRITE(hw, M_NCURVE_B_256_OFFSET, gma->g[GAMMA_CURVE_SIZE - 1], CMDLIST_WRITE);
	DPU_REG_WRITE(hw, M_NCURVE_C_256_OFFSET, gma->b[GAMMA_CURVE_SIZE - 1], CMDLIST_WRITE);
}

void dpu_hw_post_gamma_flush(struct dpu_hw_blk *hw, bool use_cpu)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (use_cpu) {
		DPU_REG_WRITE(hw, USR_GMA_CFG_DONE_OFFSET, 1, DIRECT_WRITE);
	} else {
		DPU_REG_WRITE(hw, USR_GMA_CFG_DONE_OFFSET, 1, CMDLIST_WRITE);
	}

	DPU_COLOR_DEBUG("done\n");
}
