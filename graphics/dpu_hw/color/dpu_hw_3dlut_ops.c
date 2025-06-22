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

#include "dpu_hw_3dlut_reg.h"
#include "dpu_hw_3dlut_ops.h"
#include "dpu_color_common.h"

void dpu_hw_3dlut_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, LUT_3D_EN_OFFSET, enable, CMDLIST_WRITE);
}

static void dpu_hw_3dlut_set(struct dpu_hw_blk *hw,
		struct dpu_lut_3d_cfg *tbl)
{
	int i;
	u32 val;

	for (i = 0; i < (LUT_3D_SIZE >> 1); i++) {
		val = 0;
		val = MERGE_BITS(val, tbl->lut_3d[i << 1],
				M_PLUT0_R_SHIFT, M_PLUT0_R_LENGTH);
		val = MERGE_BITS(val, tbl->lut_3d[(i << 1) + 1],
				M_PLUT0_G_SHIFT, M_PLUT0_G_LENGTH);
		DPU_REG_WRITE(hw, M_PLUT0_R_OFFSET + (i << 2), val, CMDLIST_WRITE);
	}
}

void dpu_hw_post_3dlut_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_lut_3d_cfg *tbl;

	if (!hw) {
		DPU_ERROR("invalid hw %pK\n", hw);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*tbl)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*tbl));
	}

	tbl = hw_cfg->data;
	dpu_hw_3dlut_set(hw, tbl);
}

void dpu_hw_pre_3dlut_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_lut_3d_cfg *tbl;

	if (!hw) {
		DPU_ERROR("invalid hw %pK\n", hw);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*tbl)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*tbl));
	}

	tbl = hw_cfg->data;
	dpu_hw_3dlut_set(hw, tbl);
}

void dpu_hw_3dlut_flush(struct dpu_hw_blk *hw, bool use_cpu)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (use_cpu) {
		DPU_REG_WRITE(hw, LUT_3D_CFG_DONE_OFFSET, 1, DIRECT_WRITE);
	} else {
		DPU_REG_WRITE(hw, LUT_3D_CFG_DONE_OFFSET, 1, CMDLIST_WRITE);
	}

	DPU_COLOR_DEBUG("done\n");
}

void dpu_hw_3dlut_get(struct dpu_hw_blk *hw, void *data)
{
	int i;
	u32 val;
	struct dpu_lut_3d_cfg *tbl_data;

	if (!hw || !data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	/* In 3dlut debug mode, after reading a number
	 * the addr will be accumulated by itself.
	 * Not truely support reading reg based addr index
	 */
	tbl_data = (struct dpu_lut_3d_cfg *)data;
	DPU_REG_WRITE(hw, DEBUG_MODE_OFFSET, 1, CMDLIST_WRITE);

	for (i = 0; i < (LUT_3D_SIZE >> 1); i++) {
		val = DPU_REG_READ(hw, M_PLUT0_R_OFFSET);
		tbl_data->lut_3d[i << 1] =  val & 0xFFF;
		tbl_data->lut_3d[(i << 1) + 1] =  (val >> 16) & 0xFFF;
	}
}
