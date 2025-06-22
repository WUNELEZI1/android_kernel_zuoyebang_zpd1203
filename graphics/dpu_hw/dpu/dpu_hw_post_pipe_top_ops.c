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
#include "dpu_hw_postpipe_full_top_reg.h"
#include "dpu_hw_postpipe_lite_top_reg.h"
#include "dpu_hw_post_pipe_top_ops.h"

static u32 dpu_post_pipe_pack_format_get(struct post_pipe_cfg *cfg)
{
	u32 pack_format;

	switch (cfg->pack_format) {
	case POSTPIPE_PACK_FMT_RGB666:
	case POSTPIPE_PACK_FMT_RGB888:
		pack_format = 1;
		break;
	case POSTPIPE_PACK_FMT_RGB101010:
		pack_format = 0;
		break;
	case POSTPIPE_PACK_FMT_DSC_BYTE:
		pack_format = 2;
		break;
	default:
		pack_format = 1;
	}

	return pack_format;
}

void dpu_post_pipe_port_config(struct dpu_hw_blk *hw, struct post_pipe_cfg *cfg)
{
	u32 dsc_port = 0;
	u32 pack_format;
	u32 val = 0;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	val = MERGE_BITS(val, cfg->debug_ctrl.crop_en,
			POSTPIPE_FULL_TOP_CROP_EN_SHIFT,
			POSTPIPE_FULL_TOP_CROP_EN_LENGTH);
	val = MERGE_BITS(val, cfg->debug_ctrl.inspect_en,
			POSTPIPE_FULL_TOP_INSPECT_EN_SHIFT,
			POSTPIPE_FULL_TOP_INSPECT_EN_LENGTH);
	val = MERGE_BITS(val, cfg->debug_ctrl.crc_en,
			POSTPIPE_FULL_TOP_CRC_EN_SHIFT,
			POSTPIPE_FULL_TOP_CRC_EN_LENGTH);
	val = MERGE_BITS(val, cfg->debug_ctrl.split_en,
			SPLIT_EN_SHIFT,
			SPLIT_EN_LENGTH);

	DPU_REG_WRITE(hw, POSTPIPE_FULL_TOP_CROP_EN_OFFSET, val, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, SPLIT_OVERLAP_OFFSET, 0x0, CMDLIST_WRITE);

	pack_format = dpu_post_pipe_pack_format_get(cfg);

	val = MERGE_BITS(val, cfg->port0_sel,
			POSTPIPE_FULL_TOP_TMG0_SEL_SHIFT,
			POSTPIPE_FULL_TOP_TMG0_SEL_LENGTH);
	val = MERGE_BITS(val, cfg->port1_sel,
			TMG1_SEL_SHIFT,
			TMG1_SEL_LENGTH);
	val = MERGE_BITS(val, pack_format,
			POSTPIPE_FULL_TOP_PACK_DATA_FORMT_SHIFT,
			POSTPIPE_FULL_TOP_PACK_DATA_FORMT_LENGTH);

	DPU_REG_WRITE(hw, POSTPIPE_FULL_TOP_TMG0_SEL_OFFSET, val, CMDLIST_WRITE);

	if (cfg->pack_format == POSTPIPE_PACK_FMT_DSC_BYTE) {
		dsc_port = MERGE_BITS(dsc_port, cfg->dsc_cfg.dsc_port0_en,
				DSC0_P0_EN_SHIFT,
				DSC0_P0_EN_LENGTH);
		dsc_port = MERGE_BITS(dsc_port, cfg->dsc_cfg.dsc_port1_en,
				DSC0_P1_EN_SHIFT,
				DSC0_P1_EN_LENGTH);
		dsc_port = MERGE_BITS(dsc_port, cfg->dsc_cfg.dsc_clr_per_frm_en,
				DSC0_CLR_PER_FRM_EN_SHIFT,
				DSC0_CLR_PER_FRM_EN_LENGTH);
		DPU_REG_WRITE(hw, DSC0_P0_EN_OFFSET, dsc_port, CMDLIST_WRITE);
	} else {
		DPU_REG_WRITE(hw, DSC0_P0_EN_OFFSET, 0, CMDLIST_WRITE);
	}
}

void dpu_post_pipe_rc_enable(struct dpu_hw_blk *hw, u16 width, u16 height, bool enable)
{
	DPU_REG_WRITE(hw, ACRP_BYPASS_OFFSET,
			(height << ACRP_FRAME_HEIGHT_SHIFT) | !enable, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, ACRP_FRAME_WIDTH_OFFSET, width, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, POSTPIPE_FULL_TOP_FORCE_UPDATE_EN_OFFSET, 2, CMDLIST_WRITE);
}

#define RC_BS_OFFSET (0x21100 - 0x18300)

void dpu_post_pipe_rc_stream_config(struct dpu_hw_blk *hw, void *rc_cfg)
{
	int i;
	struct dpu_rc_cfg *cfg = (struct dpu_rc_cfg *)rc_cfg;

	for (i = 0; i < cfg->acrp_bs_len; ++i)
		DPU_REG_WRITE(hw, RC_BS_OFFSET + i * 4, cfg->rc_bs[i], CMDLIST_WRITE);
}

void dpu_post_pipe_rc_config(struct dpu_hw_blk *hw, void *rc_cfg)
{
	u32 value = 0;
	struct dpu_rc_cfg *cfg = (struct dpu_rc_cfg *)rc_cfg;

	value = ((ACRP_PIXEL_FORMAT_REAL_RGB << ACRP_PIXEL_FORMAT_SHIFT) |
			(cfg->work_mode << ACRP_WORK_MODE_SHIFT) |
			(ACRP_SPR_POS << ACRP_SPR_POS_SHIFT) |
			(cfg->white_mode_en << ACRP_WHITE_MODE_EN_SHIFT) |
			(0 << ACRP_CLK_CORE_EN_SHIFT));

	DPU_REG_WRITE(hw, ACRP_CLK_CORE_EN_OFFSET, value, CMDLIST_WRITE);

	value = ((cfg->set_val_r << ACRP_SETVAL_R_SHIFT) |
			(cfg->set_val_g << ACRP_SETVAL_G_SHIFT));

	DPU_REG_WRITE(hw, ACRP_SETVAL_R_OFFSET, value, CMDLIST_WRITE);
	DPU_REG_WRITE(hw, ACRP_SETVAL_B_OFFSET, cfg->set_val_b, CMDLIST_WRITE);

	DPU_REG_WRITE(hw, ACRP_BS_LEN_OFFSET, cfg->acrp_bs_len, CMDLIST_WRITE);
}

static void dpu_post_pipe_full_top_parse_irq(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	DPU_DUMP_REG("post_pipe_full_top", hw->blk_id, POSTPIPE_FULL_TOP_IRQ_RAW_OFFSET);

	value = DPU_REG_READ(hw, POSTPIPE_FULL_TOP_IRQ_RAW_OFFSET);
	if (value & BIT(2))
		DPU_DFX_INFO("post_pipe_full_top", hw->blk_id, s, "acad_hist_done\n");

	if (value & BIT(3))
		DPU_DFX_INFO("post_pipe_full_top", hw->blk_id, s, "acad_curve_done\n");
}

static void dpu_post_pipe_lite_top_parse_irq(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	DPU_DUMP_REG("post_pipe_lite_top", hw->blk_id, POSTPIPE_LITE_TOP_IRQ_RAW_OFFSET);

	value = DPU_REG_READ(hw, POSTPIPE_LITE_TOP_IRQ_RAW_OFFSET);
	if (value & BIT(26))
		DPU_DFX_INFO("post_pipe_lite_top", hw->blk_id, s, "dsc_p0_reload_timeout\n");

	if (value & BIT(27))
		DPU_DFX_INFO("post_pipe_lite_top", hw->blk_id, s, "dsc_p0_clr_timeout\n");
}

void dpu_post_pipe_top_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (hw->blk_id == POST_PIPE_FULL_TOP)
		dpu_post_pipe_full_top_parse_irq(hw);
	else
		dpu_post_pipe_lite_top_parse_irq(hw);
}

void dpu_post_pipe_top_status_clear(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	if (hw->blk_id == POST_PIPE_FULL_TOP)
		DPU_REG_WRITE(hw, POSTPIPE_FULL_TOP_IRQ_STATUS_OFFSET, 0xC, DIRECT_WRITE);
	else
		DPU_REG_WRITE(hw, POSTPIPE_LITE_TOP_IRQ_STATUS_OFFSET, 0xC000000, DIRECT_WRITE);
}

int32_t dpu_post_pipe_crc_dump(struct dpu_hw_blk *hw, bool dsc_crc_selected)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return -1;
	}

	if (dsc_crc_selected)
		return DPU_REG_READ(hw, DSC0_P0_CRC_DAT_OFFSET);
	else
		return DPU_REG_READ(hw, POSTPIPE_FULL_TOP_CRC0_STA_DATA_OFFSET);
}
