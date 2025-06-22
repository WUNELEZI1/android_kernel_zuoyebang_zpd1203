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

#include "dpu_reg_ops.h"
#include "dpu_color_common.h"
#include "dpu_hw_histogram_reg.h"
#include "dpu_hw_pre_hist_ops.h"

void dpu_hw_pre_hist_enable_update(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}
	DPU_REG_WRITE(hw, HIST_READ_DONE_OFFSET, 1, DIRECT_WRITE);
}

void dpu_hw_pre_hist_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, HIST_EN_OFFSET, enable, CMDLIST_WRITE);
}

static void dpu_hw_hist_roi_set(struct dpu_hw_blk *hw,
		struct dpu_hist_region cfg)
{
	u32 val = 0;

	val = MERGE_BITS(val, cfg.x_begin,
			HIST_ROI_PQ_POS_X_BGN_SHIFT,
			HIST_ROI_PQ_POS_X_BGN_LENGTH);
	val = MERGE_BITS(val, cfg.x_end,
			HIST_ROI_PQ_POS_X_END_SHIFT,
			HIST_ROI_PQ_POS_X_END_LENGTH);
	DPU_REG_WRITE(hw, HIST_ROI_PQ_POS_X_BGN_OFFSET, val, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg.y_begin,
			HIST_ROI_PQ_POS_Y_BGN_SHIFT,
			HIST_ROI_PQ_POS_Y_BGN_LENGTH);
	val = MERGE_BITS(val, cfg.y_end,
			HIST_ROI_PQ_POS_Y_END_SHIFT,
			HIST_ROI_PQ_POS_Y_END_LENGTH);
	DPU_REG_WRITE(hw, HIST_ROI_PQ_POS_Y_BGN_OFFSET, val, CMDLIST_WRITE);
}

static void dpu_hw_hist_blk_set(struct dpu_hw_blk *hw,
		struct dpu_hist_region cfg)
{
	u32 val = 0;

	val = MERGE_BITS(val, cfg.x_begin,
			HIST_BLK_POS_X_BGN_SHIFT,
			HIST_BLK_POS_X_BGN_LENGTH);
	val = MERGE_BITS(val, cfg.x_end,
			HIST_BLK_POS_X_END_SHIFT,
			HIST_BLK_POS_X_END_LENGTH);
	DPU_REG_WRITE(hw, HIST_BLK_POS_X_BGN_OFFSET, val, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg.y_begin,
			HIST_BLK_POS_Y_BGN_SHIFT,
			HIST_BLK_POS_Y_BGN_LENGTH);
	val = MERGE_BITS(val, cfg.y_end,
			HIST_BLK_POS_Y_END_SHIFT,
			HIST_BLK_POS_Y_END_LENGTH);
	DPU_REG_WRITE(hw, HIST_BLK_POS_Y_BGN_OFFSET, val, CMDLIST_WRITE);
}

static void __maybe_unused dpu_hw_hist_roi_sec_set(struct dpu_hw_blk *hw,
		struct dpu_hist_region cfg)
{
	u32 val = 0;

	val = MERGE_BITS(val, cfg.y_begin,
			HIST_ROI_SEC_POS_X_BGN_SHIFT,
			HIST_ROI_SEC_POS_X_BGN_LENGTH);
	val = MERGE_BITS(val, cfg.y_end,
			HIST_ROI_SEC_POS_X_END_SHIFT,
			HIST_ROI_SEC_POS_X_END_LENGTH);
	DPU_REG_WRITE(hw, HIST_ROI_SEC_POS_X_BGN_OFFSET, val, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg.x_begin,
			HIST_ROI_SEC_POS_Y_BGN_SHIFT,
			HIST_ROI_SEC_POS_Y_BGN_LENGTH);
	val = MERGE_BITS(val, cfg.x_end,
			HIST_ROI_SEC_POS_Y_END_SHIFT,
			HIST_ROI_SEC_POS_Y_END_LENGTH);
	DPU_REG_WRITE(hw, HIST_ROI_SEC_POS_Y_BGN_OFFSET, val, CMDLIST_WRITE);
}

static void dpu_hw_hist_mode_set(struct dpu_hw_blk *hw,
		struct dpu_hist_mode cfg)
{
	u32 val = 0;

	val = MERGE_BITS(val, cfg.hist_chnl,
			HIST_CHNL_SHIFT,
			HIST_CHNL_LENGTH);
	val = MERGE_BITS(val, cfg.hist_cal_mode,
			HIST_MODE_SHIFT,
			HIST_MODE_LENGTH);
	val = MERGE_BITS(val, cfg.hist_cal_weight[0],
			HIST_MODE_WGT_COEF0_SHIFT,
			HIST_MODE_WGT_COEF0_LENGTH);
	DPU_REG_WRITE(hw, HIST_CHNL_OFFSET, val, CMDLIST_WRITE);

	val = MERGE_BITS(val, cfg.hist_cal_weight[1],
			HIST_MODE_WGT_COEF1_SHIFT,
			HIST_MODE_WGT_COEF1_LENGTH);
	val = MERGE_BITS(val, cfg.hist_cal_weight[2],
			HIST_MODE_WGT_COEF2_SHIFT,
			HIST_MODE_WGT_COEF2_LENGTH);
	DPU_REG_WRITE(hw, HIST_MODE_WGT_COEF1_OFFSET, val, CMDLIST_WRITE);
}

void dpu_hw_pre_hist_set(struct dpu_hw_blk *hw,
		struct prepq_hw_cfg *hw_cfg)
{
	struct dpu_hist_cfg *cfg;

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

	DPU_REG_WRITE(hw, HIST_POS_OFFSET, cfg->hist_pos, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, HIST_BIT_POS_OFFSET, cfg->hist_input_bit, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, HIST_READ_MODE_OFFSET, cfg->read_mode, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, HIST_ROI_PQ_EN_OFFSET, cfg->roi_enable, CMDLIST_WRITE);
	if (cfg->roi_enable)
		dpu_hw_hist_roi_set(hw, cfg->roi_region);

	DPU_REG_WRITE(hw, HIST_BLK_EN_OFFSET, cfg->blk_enable, CMDLIST_WRITE);
	if (cfg->blk_enable)
		dpu_hw_hist_blk_set(hw, cfg->blk_region);

	dpu_hw_hist_mode_set(hw, cfg->mode_cfg);

	DPU_REG_WRITE(hw, HIST_USE_LEFT_OFFSET, cfg->hist_left, CMDLIST_WRITE);
}

void dpu_hw_pre_hist_flush(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, HISTOGRAM_FORCE_UPDATE_EN_OFFSET, 0x02, CMDLIST_WRITE);
	DPU_COLOR_DEBUG("done\n");
}

void dpu_hw_pre_hist_get(struct dpu_hw_blk *hw, int base_idx, int end_idx, void *data)
{
	struct dpu_hist_data *hist;
	int i;

	if (!hw || !data) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	hist = (struct dpu_hist_data *)data;
	end_idx = end_idx > HIST_DATA_SIZE ? HIST_DATA_SIZE : end_idx;
	base_idx = base_idx < 0 ? 0 : base_idx;

	DPU_COLOR_DEBUG("hist read done = 0x%x\n",
			DPU_REG_READ(hw, HIST_READ_DONE_OFFSET));

	DPU_REG_WRITE(hw, HIST_READ_BIN_BASE_ADDR_OFFSET, base_idx, DIRECT_WRITE);
	DPU_REG_WRITE(hw, HIST_RE_READ_BIN_OFFSET, 1, DIRECT_WRITE);

	for (i = base_idx; i < end_idx; i++)
		hist->data[i] = DPU_REG_READ(hw, HIST_BINN_OFFSET);

	DPU_REG_WRITE(hw, HIST_READ_DONE_OFFSET, 1, DIRECT_WRITE);
}
