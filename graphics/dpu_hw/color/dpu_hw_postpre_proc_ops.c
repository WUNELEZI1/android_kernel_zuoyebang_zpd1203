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
#include "dpu_hw_postpre_proc_reg.h"
#include "dpu_hw_postpre_proc_ops.h"

void dpu_hw_post_preproc_enable(struct dpu_hw_blk *hw, struct postpq_feature_ctrl *ctrl)
{
	u32 enable = 0;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_LTM_EN))
		enable |= BIT(M_NPOST_PROC_EN_SHIFT) | BIT(LTM_M_NEOTF_EN_SHIFT) |
				BIT(LTM_M_NOETF_EN_SHIFT);

	if (ctrl->enabled_features & BIT(DPU_CRTC_POSTPQ_MUL_MATRIX_EN))
		enable |= BIT(M_NPOST_PROC_EN_SHIFT) | BIT(M_NENDMATRIX_EN_SHIFT);

	DPU_REG_WRITE(hw, M_NPOST_PROC_EN_OFFSET, enable, CMDLIST_WRITE);
}

void dpu_hw_post_ltm_eotf_oetf_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_ltm_cfg *cfg;
	u32 val = 0;
	int i;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*cfg));
		return;
	}

	cfg = hw_cfg->data;

	switch (cfg->eotf_mode) {
	case TM_PQ:
	case TM_HLG:
	case TM_SRGB:
	case TM_BT709:
		break;
	case TM_GAMMA22:
		if (cfg->eotf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl, size: %d\n", cfg->eotf_tbl_size);
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, LTM_LUT4_GROUP_0_OFFSET  + (i << 2),
					cfg->eotf_user_tbl[i], CMDLIST_WRITE);
		break;
	case TM_GAMMA26:
		if (cfg->oetf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl, size: %d\n", cfg->oetf_tbl_size);
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, LTM_LUT5_GROUP_0_OFFSET + (i << 2),
					cfg->oetf_user_tbl[i], CMDLIST_WRITE);
		break;
	default:
		DPU_ERROR("invalid transfer mode: %d\n", cfg->eotf_mode);
		cfg->eotf_mode = TM_SRGB;
		break;
	}

	switch (cfg->oetf_mode) {
	case TM_SRGB:
	case TM_BT709:
		break;
	case TM_GAMMA22:
		if (cfg->eotf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl: %d\n", cfg->eotf_tbl_size);
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, LTM_LUT4_GROUP_0_OFFSET + (i << 2),
					cfg->eotf_user_tbl[i], CMDLIST_WRITE);
		break;
	case TM_GAMMA26:
		if (cfg->oetf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl: %d\n", cfg->oetf_tbl_size);
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, LTM_LUT5_GROUP_0_OFFSET + (i << 2),
					cfg->oetf_user_tbl[i], CMDLIST_WRITE);
		break;
	default:
		DPU_ERROR("invalid transfer mode: %d\n", cfg->oetf_mode);
		cfg->oetf_mode = TM_SRGB;
		break;
	}

	DPU_REG_WRITE(hw, LTM_M_NEOTF_MODE_OFFSET, cfg->eotf_mode, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg->oetf_mode,
			LTM_M_NOETF_MODE_SHIFT, LTM_M_NOETF_MODE_LENGTH);
	val = MERGE_BITS(val, cfg->oetf_max,
			LTM_M_NOETF_MAX_SHIFT, LTM_M_NOETF_MAX_LENGTH);
	DPU_REG_WRITE(hw, LTM_M_NOETF_MODE_OFFSET, val, CMDLIST_WRITE);
}

void dpu_hw_post_mul_matrix_set(struct dpu_hw_blk *hw,
		struct postpq_hw_cfg *hw_cfg)
{
	struct dpu_post_matrix_3x4_cfg *cfg;
	u32 val;
	int i;

	if (!hw || !hw_cfg) {
		DPU_ERROR("invalid hw %pK, hw_cfg %pK\n", hw, hw_cfg);
		return;
	}

	if (!hw_cfg->data)
		return;

	if (hw_cfg->len != sizeof(*cfg)) {
		DPU_ERROR("invalid data size %d, expected size %lu\n",
				hw_cfg->len, sizeof(*cfg));
	}

	cfg = hw_cfg->data;

	for (i = 0; i < POST_MATRIX_3X4_SIZE - 1; i = i + 2) {
		val = 0;
		val = MERGE_BITS(val, cfg->matrix[i],
				M_PENDMATRIX_TABLE0_SHIFT, M_PENDMATRIX_TABLE0_LENGTH);
		val = MERGE_BITS(val, cfg->matrix[i + 1],
				M_PENDMATRIX_TABLE1_SHIFT, M_PENDMATRIX_TABLE1_LENGTH);
		DPU_REG_WRITE(hw, M_PENDMATRIX_TABLE0_OFFSET + (i << 1), val, CMDLIST_WRITE);
	}

	DPU_REG_WRITE(hw, M_PENDMATRIX_TABLE8_OFFSET,
			cfg->matrix[POST_MATRIX_3X4_SIZE - 1], CMDLIST_WRITE);

	for (i = 0; i < POST_MATRIX_3X4_OFFSET_SIZE; i++) {
		DPU_REG_WRITE(hw, M_PENDMATRIX_OFFSET0_OFFSET + (i << 2),
				cfg->offset_tbl[i], CMDLIST_WRITE);
	}
}

void dpu_hw_post_preproc_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, LTM_FORCE_UPDATE_EN_OFFSET, 0x02, CMDLIST_WRITE);
	DPU_COLOR_DEBUG("done\n");
}
