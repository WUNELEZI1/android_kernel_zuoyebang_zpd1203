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

#include "dpu_hw_tm_ops.h"
#include "dpu_hw_tm_reg.h"
#include "dpu_color_common.h"

void dpu_hw_tm_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, TM_M_NLAYER_PRO_EN_OFFSET, enable, CMDLIST_WRITE);
}

static void dpu_hw_tm_eotf_oetf_set(struct dpu_hw_blk *hw, struct dpu_tm_transfer *cfg)
{
	u32 val = 0;
	int i;

	switch (cfg->eotf_mode) {
	case TM_PQ:
	case TM_HLG:
	case TM_SRGB:
	case TM_BT709:
		break;
	case TM_GAMMA22:
		if (cfg->eotf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl\n");
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, TM_LUT4_GROUP_0_OFFSET + (i << 2),
					cfg->eotf_user_tbl[i], CMDLIST_WRITE);
		break;
	case TM_GAMMA26:
		if (cfg->oetf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl\n");
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, TM_LUT5_GROUP_0_OFFSET + (i << 2),
					cfg->oetf_user_tbl[i], CMDLIST_WRITE);
		break;
	default:
		DPU_ERROR("invalid eotf transfer mode\n");
		cfg->eotf_mode = TM_SRGB;
		break;
	}

	switch (cfg->oetf_mode) {
	case TM_PQ:
	case TM_HLG:
	case TM_SRGB:
	case TM_BT709:
		break;
	case TM_GAMMA22:
		if (cfg->eotf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl\n");
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, TM_LUT4_GROUP_0_OFFSET + (i << 2),
					cfg->eotf_user_tbl[i], CMDLIST_WRITE);
		break;
	case TM_GAMMA26:
		if (cfg->oetf_tbl_size != TM_TRANSFER_LUT_SIZE) {
			DPU_ERROR("invalid lut tbl\n");
			break;
		}
		for (i = 0; i < TM_TRANSFER_LUT_SIZE; i++)
			DPU_REG_WRITE(hw, TM_LUT5_GROUP_0_OFFSET + (i << 2),
					cfg->oetf_user_tbl[i], CMDLIST_WRITE);
		break;
	default:
		DPU_ERROR("invalid oetf transfer mode\n");
		cfg->oetf_mode = TM_SRGB;
		break;
	}

	val = MERGE_BITS(val, cfg->eotf_mode,
			TM_M_NEOTF_MODE_SHIFT, TM_M_NEOTF_MODE_LENGTH);
	val = MERGE_BITS(val, cfg->oetf_mode,
			TM_M_NOETF_MODE_SHIFT, TM_M_NOETF_MODE_LENGTH);
	val = MERGE_BITS(val, cfg->oetf_max,
			TM_M_NOETF_MAX_SHIFT, TM_M_NOETF_MAX_LENGTH);
	DPU_REG_WRITE(hw, TM_M_NEOTF_MODE_OFFSET, val, CMDLIST_WRITE);
}

static void dpu_hw_tm_matrix_set(struct dpu_hw_blk *hw, struct dpu_tm_matrix *cfg)
{
	u32 val = 0;
	int i;

	for (i = 1; i < TM_MATRIX_SIZE; i = i + 2) {
		val = 0;
		val = MERGE_BITS(val, cfg->matrix[i],
				TM_M_PMATRIX_TABLE1_SHIFT, TM_M_PMATRIX_TABLE1_LENGTH);
		val = MERGE_BITS(val, cfg->matrix[i + 1],
				TM_M_PMATRIX_TABLE2_SHIFT, TM_M_PMATRIX_TABLE2_LENGTH);
		DPU_REG_WRITE(hw, TM_M_PMATRIX_TABLE1_OFFSET + ((i - 1) << 1), val, CMDLIST_WRITE);
	}

	for (i = 0; i < TM_MATRIX_OFFSET_SIZE; i++)
		DPU_REG_WRITE(hw, TM_M_PMATRIX_OFFSET0_OFFSET + (i << 2),
					cfg->offset_tbl[i], CMDLIST_WRITE);
}

static void dpu_hw_tm_end_ootf_set(struct dpu_hw_blk *hw, struct dpu_tm_end_ootf *cfg)
{
	int length;
	u32 val;
	int i;

	length = cfg->tbl_length;
	for (i = 0; i < length - 1; i = i + 2) {
		val = 0;
		val = MERGE_BITS(val, cfg->gain_tbl[i],
				TM_M_PEND_TMOOTF_GAIN_TABLE0_SHIFT,
				TM_M_PEND_TMOOTF_GAIN_TABLE0_LENGTH);
		val = MERGE_BITS(val, cfg->gain_tbl[i + 1],
				TM_M_PEND_TMOOTF_GAIN_TABLE1_SHIFT,
				TM_M_PEND_TMOOTF_GAIN_TABLE1_LENGTH);

		DPU_REG_WRITE(hw, TM_M_PEND_TMOOTF_GAIN_TABLE0_OFFSET + (i << 1),
				val, CMDLIST_WRITE);
	}

	DPU_REG_WRITE(hw, TM_M_PEND_TMOOTF_GAIN_TABLE0_OFFSET + (((length - 1) >> 1) << 2),
			cfg->gain_tbl[length - 1], CMDLIST_WRITE);

	if (cfg->rgb_mode == TM_OOTF_RGB_MODE_ONLY_Y) {
		val = 0;
		val = MERGE_BITS(val, cfg->y_weight[0],
				TM_M_TMOOTF_WEIGHTYR_SHIFT, TM_M_TMOOTF_WEIGHTYR_LENGTH);
		val = MERGE_BITS(val, cfg->y_weight[1],
				TM_M_TMOOTF_WEIGHTYG_SHIFT, TM_M_TMOOTF_WEIGHTYG_LENGTH);
		DPU_REG_WRITE(hw, TM_M_TMOOTF_WEIGHTYR_OFFSET, val, CMDLIST_WRITE);

		DPU_REG_WRITE(hw, TM_M_TMOOTF_WEIGHTYB_OFFSET,
				cfg->y_weight[2], CMDLIST_WRITE);
	}
}

void dpu_hw_tm_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_tm_cfg *cfg;
	u32 val = 0;
	u32 val_1 = 0;

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
	val = MERGE_BITS(val, cfg->transfer_en, TM_M_NEOTF_EN_SHIFT, TM_M_NEOTF_EN_LENGTH);
	val = MERGE_BITS(val, cfg->transfer_en, TM_M_NOETF_EN_SHIFT, TM_M_NOETF_EN_LENGTH);

	if (cfg->transfer_en)
		dpu_hw_tm_eotf_oetf_set(hw, &cfg->transfer);
	val = MERGE_BITS(val, cfg->gain_to_full_en, TM_M_NGAIN_TO_FULL_EN_SHIFT, TM_M_NGAIN_TO_FULL_EN_LENGTH);

	if (cfg->gain_to_full_en)
		DPU_REG_WRITE(hw, TM_M_NGAIN_TO_FULL_OFFSET, cfg->gain_to_full, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg->matrix_en, TM_M_NMATRIX_EN_SHIFT, TM_M_NMATRIX_EN_LENGTH);

	if (cfg->matrix_en) {
		dpu_hw_tm_matrix_set(hw, &cfg->matrix);
		val_1 = MERGE_BITS(val_1, cfg->matrix.matrix[0], TM_M_PMATRIX_TABLE0_SHIFT, TM_M_PMATRIX_TABLE0_LENGTH);
	}

	val = MERGE_BITS(val, cfg->end_ootf_en, TM_M_NEND_TMOOTF_EN_SHIFT, TM_M_NEND_TMOOTF_EN_LENGTH);

	if (cfg->end_ootf_en) {
		if ((cfg->end_ootf.tbl_length != END_OOTF_GAIN_TABLE_SIZE_V1) &&
			(cfg->end_ootf.tbl_length != END_OOTF_GAIN_TABLE_SIZE)) {
			DPU_ERROR("invalid data size %d at end ootf!\n",
					cfg->end_ootf.tbl_length);
			return;
		}
		dpu_hw_tm_end_ootf_set(hw, &cfg->end_ootf);
		val_1 = MERGE_BITS(val_1, cfg->end_ootf.shift_bits, TM_M_NEND_TMOOTF_SHIFT_BITS_SHIFT, TM_M_NEND_TMOOTF_SHIFT_BITS_LENGTH);
		val_1 = MERGE_BITS(val_1, cfg->end_ootf.rgb_mode, TM_M_NEND_TMOOTF_RGB_MODE_SHIFT, TM_M_NEND_TMOOTF_RGB_MODE_LENGTH);
	}
	val = MERGE_BITS(val, 0, TM_M_NFRONT_TMOOTF_EN_SHIFT, TM_M_NFRONT_TMOOTF_EN_LENGTH);
	val = MERGE_BITS(val, 1, TM_M_NLAYER_PRO_EN_SHIFT, TM_M_NLAYER_PRO_EN_LENGTH);
	DPU_REG_WRITE(hw, TM_M_NLAYER_PRO_EN_OFFSET, val, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, TM_M_NEND_TMOOTF_SHIFT_BITS_OFFSET, val_1, CMDLIST_WRITE);
}

void dpu_hw_tm_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, TM_FORCE_UPDATE_EN_OFFSET, 0x02, CMDLIST_WRITE);
	DPU_COLOR_DEBUG("done\n");
}
