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

#include "dpu_hw_wb_scaler_ops.h"
#include "dpu_hw_wb_scaler_reg.h"

#define GET_SCALER_REG_ADDR(base, index)    ((base) + (index) * 0x4)

static void dpu_hw_wb_scaler_update(struct dpu_hw_blk *hw)
{
	DPU_REG_WRITE(hw, SCALER_8X6TAP_FORCE_UPDATE_EN_OFFSET, 0x2, CMDLIST_WRITE);
}

static void dpu_hw_wb_scaler_reset(struct dpu_hw_blk *hw)
{
	DPU_REG_WRITE(hw, SCALER_8X6TAP_TRIGGER2_OFFSET, 1, CMDLIST_WRITE);
}

void dpu_hw_wb_scaler_enable(struct dpu_hw_blk *hw, bool enable)
{
	u32 value;

	dpu_hw_wb_scaler_reset(hw);

	value = MERGE_BITS(enable, enable,
			M_NSCL_VER_ENABLE_SHIFT, M_NSCL_VER_ENABLE_LENGTH);

	DPU_REG_WRITE(hw, M_NSCL_HOR_ENABLE_OFFSET, value, CMDLIST_WRITE);
}

static void dpu_hw_wb_conf_scaler_hor_ver_coef(struct dpu_hw_blk *hw,
		int32_t *hor_coef, int32_t *ver_coef)
{
	u32 offset;
	u32 value;
	int i;

	for (i = 0; i < SR_8_TAP_COEF_SIZE / 2; ++i) {
		value = MERGE_BITS(hor_coef[2 * i], hor_coef[2 * i + 1],
				M_NSCL_HOR_COEF1_SHIFT, M_NSCL_HOR_COEF1_LENGTH);

		offset = GET_SCALER_REG_ADDR(M_NSCL_HOR_COEF0_OFFSET, i);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
	}

	for (i = 0; i < SR_6_TAP_COEF_SIZE / 2; ++i) {
		value = MERGE_BITS(ver_coef[2 * i], ver_coef[2 * i + 1],
				M_NSCL_VER_COEF1_SHIFT, M_NSCL_VER_COEF1_LENGTH);

		offset = GET_SCALER_REG_ADDR(M_NSCL_VER_COEF0_OFFSET, i);
		DPU_REG_WRITE(hw, offset, value, CMDLIST_WRITE);
	}
}

void dpu_hw_wb_scaler_set(struct dpu_hw_blk *hw,
		struct dpu_1d_scaler_cfg *scaler)
{
	u32 value;

	DPU_DEBUG("scaler->input_width:%d, scaler->input_height:%d\n",
			scaler->input_width, scaler->input_height);
	DPU_DEBUG("scaler->output_width:%d, scaler->output_height):%d\n",
			scaler->output_width, scaler->output_height);

	value = MERGE_BITS(scaler->input_width, scaler->input_height,
			M_NSCL_INPUT_HEIGHT_SHIFT, M_NSCL_INPUT_HEIGHT_LENGTH);

	DPU_REG_WRITE(hw, M_NSCL_INPUT_WIDTH_OFFSET, value, CMDLIST_WRITE);

	value = MERGE_BITS(scaler->output_width, scaler->output_height,
			M_NSCL_OUTPUT_HEIGHT_SHIFT, M_NSCL_OUTPUT_HEIGHT_LENGTH);

	DPU_REG_WRITE(hw, M_NSCL_OUTPUT_WIDTH_OFFSET, value, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, M_NSCL_HOR_INIT_PHASE_L32B_OFFSET,
			scaler->hor_init_phase_l32b, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, M_NSCL_HOR_INIT_PHASE_H1B_OFFSET, scaler->hor_init_phase_h1b,
			CMDLIST_WRITE);

	DPU_REG_WRITE(hw, M_NSCL_VER_INIT_PHASE_L32B_OFFSET,
			scaler->ver_init_phase_l32b, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, M_NSCL_VER_INIT_PHASE_H1B_OFFSET, scaler->ver_init_phase_h1b,
			CMDLIST_WRITE);

	DPU_REG_WRITE(hw, M_NSCL_HOR_DELTA_PHASE_OFFSET, scaler->hor_delta_phase,
			CMDLIST_WRITE);

	DPU_REG_WRITE(hw, M_NSCL_VER_DELTA_PHASE_OFFSET, scaler->ver_delta_phase,
			CMDLIST_WRITE);

	dpu_hw_wb_conf_scaler_hor_ver_coef(hw, scaler->hor_coef, scaler->ver_coef);
	dpu_hw_wb_scaler_update(hw);
}

