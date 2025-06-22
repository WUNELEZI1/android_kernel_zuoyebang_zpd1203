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

#include "dpu_hw_wb_ops.h"
#include "dpu_hw_wb_core_reg.h"
#include "dpu_hw_wdma_top_reg.h"

#define GET_CONTINUOUS_REG_ADDR(base, index)    ((base) + (index) * 0x04)

#define WB_ADDR_OFFSET_REG_NUM    2
#define ADDR_LOW_BIT_SIZE    32
#define NEXT_ONE_REG    1
#define WB_OUTSTANDING_NUM    31
#define WB_BURST_LEN 15
#define WB_BOUNDARY_EN 1

static void dpu_wb_module_enable_setup(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 value;

	value = MERGE_BITS(cfg->is_afbc, cfg->scl_en, WB_SCL_EN_SHIFT,
			WB_SCL_EN_LENGTH);

	value = MERGE_BITS(value, cfg->dither_en, WB_DITHER_EN_SHIFT,
			WB_DITHER_EN_LENGTH);

	value = MERGE_BITS(value, cfg->slice_header_wren, WB_SLICE_HEADER_WREN_SHIFT,
			WB_SLICE_HEADER_WREN_LENGTH);

	value = MERGE_BITS(value, cfg->is_offline_mode, WB_OFFLINE_EN_SHIFT,
			WB_OFFLINE_EN_LENGTH);

	value = MERGE_BITS(value, cfg->output_format, WB_OUT_FORMAT_SHIFT,
			WB_OUT_FORMAT_LENGTH);

	value = MERGE_BITS(value, cfg->frame_rot_mode, WB_ROT_MODE_SHIFT,
			WB_ROT_MODE_LENGTH);

	value = MERGE_BITS(value, cfg->frame_flip_mode, WB_FLIP_MODE_SHIFT,
			WB_FLIP_MODE_LENGTH);

	value = MERGE_BITS(value, cfg->crop0_en, WB_CROP0_EN_SHIFT,
			WB_CROP0_EN_LENGTH);

	value = MERGE_BITS(value, cfg->crop1_en, WB_CROP1_EN_SHIFT,
			WB_CROP1_EN_LENGTH);

	value = MERGE_BITS(value, cfg->crop2_en, WB_CROP2_EN_SHIFT,
			WB_CROP2_EN_LENGTH);

	DPU_REG_WRITE(hw, WB_WDMA_FBC_EN_OFFSET, value, CMDLIST_WRITE);

	value = 0x0;
	value = MERGE_BITS(value, cfg->qos, WB_WDMA_QOS_SHIFT,
				WB_WDMA_QOS_LENGTH);
	DPU_REG_WRITE(hw, WB_WDMA_QOS_OFFSET, value, CMDLIST_WRITE);
}

/* set the configurations of input frame */
static void dpu_hw_wb_input_setup(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 val;

	val = MERGE_BITS(cfg->input_width, cfg->input_height,
			WB_IN_HEIGHT_SHIFT, WB_IN_HEIGHT_LENGTH);
	DPU_REG_WRITE(hw, WB_IN_WIDTH_OFFSET, val, CMDLIST_WRITE);
}

static void __conf_wb_output_original_area(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 value;

	value = MERGE_BITS(cfg->output_original_width,
			cfg->output_original_height,
			WB_OUT_ORI_HEIGHT_SHIFT,
			WB_OUT_ORI_HEIGHT_LENGTH);
	DPU_REG_WRITE(hw, WB_OUT_ORI_WIDTH_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_wb_output_subframe_area(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 value;

	value = MERGE_BITS(cfg->output_subframe_rect.x,
			cfg->output_subframe_rect.y,
			WB_OUT_SUBFRAME_LTOPY_SHIFT,
			WB_OUT_SUBFRAME_LTOPY_LENGTH);
	DPU_REG_WRITE(hw, WB_OUT_SUBFRAME_LTOPX_OFFSET, value, CMDLIST_WRITE);

	value = MERGE_BITS(cfg->output_subframe_rect.w,
			cfg->output_subframe_rect.h,
			WB_OUT_SUBFRAME_HEIGHT_SHIFT,
			WB_OUT_SUBFRAME_HEIGHT_LENGTH);
	DPU_REG_WRITE(hw, WB_OUT_SUBFRAME_WIDTH_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_wb_base_addr(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 reg_addr;
	int i;

	for (i = 0; i < cfg->plane_count; i++) {
		reg_addr = GET_CONTINUOUS_REG_ADDR(WB_WDMA_BASE_ADDR0_LOW_OFFSET,
			(i * WB_ADDR_OFFSET_REG_NUM));
		DPU_REG_WRITE(hw, reg_addr, cfg->wdma_base_addrs[i], CMDLIST_WRITE);

		reg_addr = GET_CONTINUOUS_REG_ADDR(reg_addr, NEXT_ONE_REG);
		DPU_REG_WRITE(hw, reg_addr,
			cfg->wdma_base_addrs[i] >> ADDR_LOW_BIT_SIZE, CMDLIST_WRITE);
	}
}

static void __conf_wb_format(struct dpu_hw_blk *hw, struct wb_frame_cfg *cfg)
{
	u32 value;
	int i;

	for (i = 0; i < (CSC_MATRIX_SIZE >> 1); i++) {
		value = 0;
		value = MERGE_BITS(value, cfg->csc_cfg.matrix[i << 1],
				FMT_CVT_RGB2YUV_MATRIX00_SHIFT,
				FMT_CVT_RGB2YUV_MATRIX00_LENGTH);
		value = MERGE_BITS(value, cfg->csc_cfg.matrix[(i << 1) + 1],
				FMT_CVT_RGB2YUV_MATRIX01_SHIFT,
				FMT_CVT_RGB2YUV_MATRIX01_LENGTH);
		DPU_REG_WRITE(hw, FMT_CVT_RGB2YUV_MATRIX00_OFFSET + (i << 2),
				value, CMDLIST_WRITE);
	}
}

static void __conf_wb_afbc(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 value;

	value = cfg->is_afbc_copy_mode;
	value = MERGE_BITS(value, cfg->is_afbc_default_color,
		FBC_DEFAULT_COLOR_EN_SHIFT, FBC_DEFAULT_COLOR_EN_LENGTH);
	value = MERGE_BITS(value, cfg->is_afbc_yuv_transform,
		FBC_YUV_TRANSFORM_EN_SHIFT, FBC_YUV_TRANSFORM_EN_LENGTH);
	value = MERGE_BITS(value, cfg->is_afbc_tile_header_mode,
		FBC_TILE_HD_MODE_EN_SHIFT, FBC_TILE_HD_MODE_EN_LENGTH);
	value = MERGE_BITS(value, cfg->is_afbc_32x8,
		FBC_TILE_WIDE_SHIFT, FBC_TILE_WIDE_LENGTH);
	value = MERGE_BITS(value, cfg->is_afbc_split,
		FBC_TILE_SPLIT_EN_SHIFT, FBC_TILE_SPLIT_EN_LENGTH);

	DPU_REG_WRITE(hw, FBC_COPY_MODE_EN_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_wb_stride(struct dpu_hw_blk *hw, struct wb_frame_cfg *cfg)
{
	if (!cfg->is_afbc)
		DPU_REG_WRITE(hw, WB_WDMA_STRIDE_OFFSET, cfg->raw_format_stride, CMDLIST_WRITE);
}

static void __conf_wb_crop_area(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 value;

	if (cfg->crop0_en) {
		value = cfg->crop0_area.x1;
		value = MERGE_BITS(value, cfg->crop0_area.y1, WB_CROP0_LTOPY_SHIFT,
			WB_CROP0_LTOPY_LENGTH);
		DPU_REG_WRITE(hw, WB_CROP0_LTOPX_OFFSET, value, CMDLIST_WRITE);

		value = cfg->crop0_area.x2;
		value = MERGE_BITS(value, cfg->crop0_area.y2, WB_CROP0_RBOTY_SHIFT,
			WB_CROP0_RBOTY_LENGTH);
		DPU_REG_WRITE(hw, WB_CROP0_RBOTX_OFFSET, value, CMDLIST_WRITE);
	}

	if (cfg->crop1_en) {
		value = cfg->crop1_area.x1;
		value = MERGE_BITS(value, cfg->crop1_area.y1, WB_CROP1_LTOPY_SHIFT,
			WB_CROP1_LTOPY_LENGTH);
		DPU_REG_WRITE(hw, WB_CROP1_LTOPX_OFFSET, value, CMDLIST_WRITE);

		value = cfg->crop1_area.x2;
		value = MERGE_BITS(value, cfg->crop1_area.y2, WB_CROP1_RBOTY_SHIFT,
			WB_CROP1_RBOTY_LENGTH);
		DPU_REG_WRITE(hw, WB_CROP1_RBOTX_OFFSET, value, CMDLIST_WRITE);
	}

	if (cfg->crop2_en) {
		value = cfg->crop2_area.x1;
		value = MERGE_BITS(value, cfg->crop2_area.y1, WB_CROP2_LTOPY_SHIFT,
			WB_CROP2_LTOPY_LENGTH);
		DPU_REG_WRITE(hw, WB_CROP2_LTOPX_OFFSET, value, CMDLIST_WRITE);

		value = cfg->crop2_area.x2;
		value = MERGE_BITS(value, cfg->crop2_area.y2, WB_CROP2_RBOTY_SHIFT,
			WB_CROP2_RBOTY_LENGTH);
		DPU_REG_WRITE(hw, WB_CROP2_RBOTX_OFFSET, value, CMDLIST_WRITE);
	}
}

static void __conf_wb_sliceinfor(struct dpu_hw_blk *hw, struct wb_frame_cfg *cfg)
{
	u32 value;

	value = cfg->slice_size;
	value = MERGE_BITS(value, cfg->slice_num, WB_SLICE_NUM_SHIFT,
			WB_SLICE_NUM_LENGTH);
	DPU_REG_WRITE(hw, WB_SLICE_SIZE_OFFSET, value, CMDLIST_WRITE);

	value = cfg->sliceinfor_addr_low;
	DPU_REG_WRITE(hw, WB_SLICEINFOR_ADDR_LOW_OFFSET, value, CMDLIST_WRITE);

	value = cfg->sliceinfor_addr_high & BITS_MASK(0, WB_SLICEINFOR_ADDR_HIGH_LENGTH);
	DPU_REG_WRITE(hw, WB_SLICEINFOR_ADDR_HIGH_OFFSET, value, CMDLIST_WRITE);
}

static void __conf_wb_dither(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u32 map_val = 0;
	u32 val = 0;
	int i;

	if (cfg->dither_en) {
		val = MERGE_BITS(val, cfg->dither_cfg.mode,
				WB_DITHER_MODE_SHIFT,
				WB_DITHER_MODE_LENGTH);

		val = MERGE_BITS(val, cfg->dither_cfg.dither_out_depth0,
				WB_DITHER_OUT_DPTH0_SHIFT,
				WB_DITHER_OUT_DPTH0_LENGTH);
		val = MERGE_BITS(val, cfg->dither_cfg.dither_out_depth1,
				WB_DITHER_OUT_DPTH1_SHIFT,
				WB_DITHER_OUT_DPTH1_LENGTH);
		val = MERGE_BITS(val, cfg->dither_cfg.dither_out_depth2,
				WB_DITHER_OUT_DPTH2_SHIFT,
				WB_DITHER_OUT_DPTH2_LENGTH);

		if (cfg->dither_cfg.mode == DITHER_MODE_PATTERN) {
			val = MERGE_BITS(val, cfg->dither_cfg.autotemp,
					WB_DITHER_AUTO_TEMP_EN_SHIFT,
					WB_DITHER_AUTO_TEMP_EN_LENGTH);
			val = MERGE_BITS(val, cfg->dither_cfg.rotatemode,
					WB_DITHER_ROT_MODE_SHIFT,
					WB_DITHER_ROT_MODE_LENGTH);
			val = MERGE_BITS(val, cfg->dither_cfg.temporalvalue,
					WB_DITHER_TMP_VALUE_SHIFT,
					WB_DITHER_TMP_VALUE_LENGTH);
			val = MERGE_BITS(val, cfg->dither_cfg.patternbits,
					WB_DITHER_PATTERN_BITS_SHIFT,
					WB_DITHER_PATTERN_BITS_LENGTH);

			for (i = 0; i < DITHER_BAYER_MAP_SIZE; i = i + 4) {
				map_val = MERGE_BITS(map_val, cfg->dither_cfg.bayermap[i],
						WB_DITHER_BAYER_MAP0_SHIFT,
						WB_DITHER_BAYER_MAP0_LENGTH);
				map_val = MERGE_BITS(map_val, cfg->dither_cfg.bayermap[i + 1],
						WB_DITHER_BAYER_MAP1_SHIFT,
						WB_DITHER_BAYER_MAP1_LENGTH);
				map_val = MERGE_BITS(map_val, cfg->dither_cfg.bayermap[i + 2],
						WB_DITHER_BAYER_MAP2_SHIFT,
						WB_DITHER_BAYER_MAP2_LENGTH);
				map_val = MERGE_BITS(map_val, cfg->dither_cfg.bayermap[i + 3],
						WB_DITHER_BAYER_MAP3_SHIFT,
						WB_DITHER_BAYER_MAP3_LENGTH);
				DPU_REG_WRITE(hw, WB_DITHER_BAYER_MAP0_OFFSET + i, map_val,
						CMDLIST_WRITE);
			}
		}

		DPU_REG_WRITE(hw, WB_DITHER_AUTO_TEMP_EN_OFFSET, val, CMDLIST_WRITE);
	}
}

static void __conf_wb_mid_set(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	u8 wb_id;
	u8 drm_status = 0;

	if (!hw || !cfg) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	wb_id = (u8)hw->blk_id;
	drm_status = cfg->drm_enable_status;

	if (drm_status == HOLD)
		return;

	DPU_SEC_WB(wb_id, drm_status);
}

/* set the configurations of output frame */
static void dpu_hw_wb_output_setup(struct dpu_hw_blk *hw,
		struct wb_frame_cfg *cfg)
{
	__conf_wb_output_original_area(hw, cfg);

	__conf_wb_output_subframe_area(hw, cfg);

	__conf_wb_base_addr(hw, cfg);

	__conf_wb_format(hw, cfg);

	__conf_wb_afbc(hw, cfg);

	__conf_wb_stride(hw, cfg);

	__conf_wb_crop_area(hw, cfg);

	__conf_wb_sliceinfor(hw, cfg);

	__conf_wb_dither(hw, cfg);

	__conf_wb_mid_set(hw, cfg);
}

void dpu_hw_wb_cfg_setup(struct dpu_hw_blk *hw, struct wb_frame_cfg *cfg)
{
	if (!hw) {
		DPU_ERROR("input dpu_hw_blk was invalid parameter\n");
		return;
	} else if (!cfg) {
		DPU_ERROR("input wb_frame_cfg was invalid parameter\n");
		return;
	}

	dpu_wb_module_enable_setup(hw, cfg);

	dpu_hw_wb_input_setup(hw, cfg);

	dpu_hw_wb_output_setup(hw, cfg);
}

void dpu_hw_wb_reset(struct dpu_hw_blk *hw)
{
	u32 val;

	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, WB_CORE_REG_VALUE_RST_OFFSET, REG_BANK_RESET_VAL, CMDLIST_WRITE);

	val = MERGE_BITS(WB_OUTSTANDING_NUM, WB_BURST_LEN, WB_WDMA_BURST_LEN_SHIFT,
			WB_WDMA_BURST_LEN_LENGTH);
	val = MERGE_BITS(val, WB_BOUNDARY_EN, WB_WDMA_BOUNDARY_EN_SHIFT,
			WB_WDMA_BOUNDARY_EN_LENGTH);

	/* reconfigure wdma outstanding num after reset */
	DPU_REG_WRITE(hw, WB_WDMA_OUTSTANDING_NUM_OFFSET, val,
			CMDLIST_WRITE);
}

static void dpu_hw_wb_core_parse_dbg_bus0(struct dpu_hw_blk *hw)
{
	u16 wb_in_col_cnt;
	u16 wb_in_row_cnt;
	char s[128];

	DPU_DUMP_REG("wb_core", hw->blk_id, WB_DBG_BUS0_OFFSET);

	wb_in_col_cnt = DPU_BIT_READ(hw, WB_DBG_BUS0_OFFSET, 0, 16);
	wb_in_row_cnt = DPU_BIT_READ(hw, WB_DBG_BUS0_OFFSET, 16, 16);
	DPU_DFX_INFO("wb_core", hw->blk_id, s, "wb in col cnt: %d, in row cnt: %d\n",
			wb_in_col_cnt, wb_in_row_cnt);
}

static void dpu_hw_wb_core_parse_dbg_bus1(struct dpu_hw_blk *hw)
{
	u16 dct_out_col_cnt;
	u16 dct_out_row_cnt;
	char s[128];

	DPU_DUMP_REG("wb_core", hw->blk_id, WB_DBG_BUS1_OFFSET);

	dct_out_col_cnt = DPU_BIT_READ(hw, WB_DBG_BUS1_OFFSET, 0, 16);
	dct_out_row_cnt = DPU_BIT_READ(hw, WB_DBG_BUS1_OFFSET, 16, 16);
	DPU_DFX_INFO("wb_core", hw->blk_id, s, "wb in col cnt: %d, in row cnt: %d\n",
			dct_out_col_cnt, dct_out_row_cnt);
}

static void dpu_hw_wb_core_parse_dbg_bus2(struct dpu_hw_blk *hw)
{
	u32 value;

	DPU_DUMP_REG("wb_core", hw->blk_id, WB_DBG_BUS2_OFFSET);

	value = DPU_REG_READ(hw, WB_DBG_BUS2_OFFSET);
}

static void dpu_hw_wb_core_parse_dbg_bus3(struct dpu_hw_blk *hw)
{
	u32 value;

	DPU_DUMP_REG("wb_core", hw->blk_id, WB_DBG_BUS3_OFFSET);

	value = DPU_REG_READ(hw, WB_DBG_BUS3_OFFSET);
}

static void dpu_hw_wb_core_parse_dbg_bus4(struct dpu_hw_blk *hw)
{
	u32 value;
	char s[128];

	value = DPU_REG_READ(hw, WB_DBG_BUS4_OFFSET);
	DPU_DFX_INFO("wb_core", hw->blk_id, s, "ostd id%d busy \n", value);
}

void dpu_hw_wb_core_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	dpu_hw_wb_core_parse_dbg_bus0(hw);
	dpu_hw_wb_core_parse_dbg_bus1(hw);
	dpu_hw_wb_core_parse_dbg_bus2(hw);
	dpu_hw_wb_core_parse_dbg_bus3(hw);
	dpu_hw_wb_core_parse_dbg_bus4(hw);
}

static void dpu_hw_wb_top_parse_arg_dbg_info(struct dpu_hw_blk *hw)
{
	u8 axi_read_ostd_cnt;
	u8 axi_write_ostd_cnt;
	u8 axi_wid_fifo_empty_status;
	u8 axi_wdata_fifo_empty_status;
	char s[128];

	DPU_DUMP_REG("wb_top", hw->blk_id, ARB_DEBUG_INFO_OFFSET);

	axi_read_ostd_cnt = DPU_BIT_READ(hw, ARB_DEBUG_INFO_OFFSET, 0, 8);
	DPU_DFX_INFO("wb_top", hw->blk_id, s,
			"axi read ostd cnt is %d\n", axi_read_ostd_cnt);

	axi_write_ostd_cnt = DPU_BIT_READ(hw, ARB_DEBUG_INFO_OFFSET, 8, 8);
	DPU_DFX_INFO("wb_top", hw->blk_id, s,
			"axi write ostd cnt is %d\n", axi_write_ostd_cnt);

	axi_wid_fifo_empty_status = DPU_BIT_READ(hw, ARB_DEBUG_INFO_OFFSET, 16, 1);
	if (axi_wid_fifo_empty_status == 1)
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "axi wid fifo is empty\n");

	axi_wdata_fifo_empty_status = DPU_BIT_READ(hw, ARB_DEBUG_INFO_OFFSET, 17, 1);
	if (axi_wdata_fifo_empty_status == 1)
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "axi wdata fifo is empty\n");
}

static void dpu_hw_wb_top_parse_dbg_irq_raw(struct dpu_hw_blk *hw)
{
	char s[128];
	u32 value;
	int i;

	DPU_DUMP_REG("wb_top", hw->blk_id, WDMA_DBG_IRQ_RAW_OFFSET);

	value = DPU_REG_READ(hw, WDMA_DBG_IRQ_RAW_OFFSET);
	for (i = 0; i < 4; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("wb_top", hw->blk_id, s, "master_wr%d_timeout\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 4))
			DPU_DFX_INFO("wb_top", hw->blk_id, s, "mmu%d_timeout_err\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 8))
			DPU_DFX_INFO("wb_top", hw->blk_id, s, "mmu%d_tbu_size_err\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 12))
			DPU_DFX_INFO("wb_top", hw->blk_id, s, "mmu%d_tlb_miss\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 16))
			DPU_DFX_INFO("wb_top", hw->blk_id, s, "va%d_missmatch\n", i);
	}

	if (value & BIT(20))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wdma_resp_err\n");

	for (i = 0; i < 2; i++) {
		if (value & BIT(i + 21))
			DPU_DFX_INFO("wb_top", hw->blk_id, s,
					"dsc%d wdma input pixel num err\n", i);
	}
}

static void dpu_hw_wb_top_parse_irq_raw(struct dpu_hw_blk *hw)
{
	char s[128];
	u32 value;

	DPU_DUMP_REG("wb_top", hw->blk_id, WB_IRQ_RAW_OFFSET);

	value = DPU_REG_READ(hw, WB_IRQ_RAW_OFFSET);
	if (value & BIT(0))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 rot & fbc & ~16x16\n");

	if (value & BIT(1))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 base addr is not 16 align\n");

	if (value & BIT(2))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 subframe ltopx/y align ---\
				fbc subframe ltopx/y not tile align --- rot 90/270 not 16 align\n");

	if (value & BIT(3))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 crop width 2alogn when YUV\n");

	if (value & BIT(4))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 over max frame width/height\n");

	if (value & BIT(5))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 cvt alpha err\n");

	if (value & BIT(6))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 s2t mem err\n");

	if (value & BIT(7))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 tmg &offline\n");

	if (value & BIT(8))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 tmg underrun\n");

	if (value & BIT(9))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 tmg start timeout\n");

	if (value & BIT(10))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 tmg done timeout\n");

	if (value & BIT(11))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 tmg cfg err\n");

	if (value & BIT(12))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 scl mem err\n");

	if (value & BIT(13))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 slice header err\n");

	if (value & BIT(14))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 in pnum err\n");

	if (value & BIT(15))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb0 dat cvt out pnum err\n");

	if (value & BIT(16))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 rot & fbc & ~16x16\n");

	if (value & BIT(17))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 base addr is not 16 align\n");

	if (value & BIT(18))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 subframe ltopx/y align\n");

	if (value & BIT(19))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 crop width 2alogn when YUV\n");

	if (value & BIT(20))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 over max frame width/height\n");

	if (value & BIT(21))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 cvt alpha err\n");

	if (value & BIT(22))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 s2t mem err\n");

	if (value & BIT(23))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 tmg &offline\n");

	if (value & BIT(24))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 tmg underrun\n");

	if (value & BIT(25))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 tmg start timeout\n");

	if (value & BIT(26))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 tmg done timeout\n");

	if (value & BIT(27))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 tmg cfg err\n");

	if (value & BIT(28))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 scl mem err\n");

	if (value & BIT(29))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 slice header err\n");

	if (value & BIT(30))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 in pnum err\n");

	if (value & BIT(31))
		DPU_DFX_INFO("wb_top", hw->blk_id, s, "wb1 dat cvt out pnum err\n");
}

void dpu_hw_wb_top_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	dpu_hw_wb_top_parse_arg_dbg_info(hw);
	dpu_hw_wb_top_parse_dbg_irq_raw(hw);
	dpu_hw_wb_top_parse_irq_raw(hw);
}

void dpu_hw_wb_top_status_clear(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, WDMA_DBG_IRQ_STATUS_OFFSET, 0x7FFFFF, DIRECT_WRITE);
}

void dpu_hw_wb_top_ro_trig_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, WDMA_TOP_RO_TRIG_ENABLE_OFFSET, enable, CMDLIST_WRITE);
}

void dpu_hw_wb_core_ro_trig_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, WB_CORE_RO_TRIG_ENABLE_OFFSET, enable, CMDLIST_WRITE);
}

void dpu_hw_wb_line_buf_level_config(struct dpu_hw_blk *hw, struct wb_line_buf_level wb_lbuf_level)
{
	u32 value = 0;

	value = MERGE_BITS(value, wb_lbuf_level.buf_lvl_up0,
			BUF_LVL_QOS_TH_UP0_SHIFT, BUF_LVL_QOS_TH_UP0_LENGTH);
	value = MERGE_BITS(value, wb_lbuf_level.buf_lvl_down0,
			BUF_LVL_QOS_TH_DOWN0_SHIFT, BUF_LVL_QOS_TH_DOWN0_LENGTH);
	DPU_REG_WRITE(hw, BUF_LVL_QOS_TH_UP0_OFFSET, value, CMDLIST_WRITE);

	value = 0;
	value = MERGE_BITS(value, wb_lbuf_level.buf_lvl_up1,
			BUF_LVL_QOS_TH_UP1_SHIFT, BUF_LVL_QOS_TH_UP1_LENGTH);
	value = MERGE_BITS(value, wb_lbuf_level.buf_lvl_down1,
			BUF_LVL_QOS_TH_DOWN1_SHIFT, BUF_LVL_QOS_TH_DOWN1_LENGTH);
	DPU_REG_WRITE(hw, BUF_LVL_QOS_TH_UP1_OFFSET, value, CMDLIST_WRITE);

	value = 0;
	value = MERGE_BITS(value, wb_lbuf_level.buf_lvl_up2,
			BUF_LVL_QOS_TH_UP2_SHIFT, BUF_LVL_QOS_TH_UP2_LENGTH);
	value = MERGE_BITS(value, wb_lbuf_level.buf_lvl_down2,
			BUF_LVL_QOS_TH_DOWN2_SHIFT, BUF_LVL_QOS_TH_DOWN2_LENGTH);
	DPU_REG_WRITE(hw, BUF_LVL_QOS_TH_UP2_OFFSET, value, CMDLIST_WRITE);

	value = 0;
	value = MERGE_BITS(value, wb_lbuf_level.buf_urgent_up,
			BUF_LVL_URGENT_TH_UP_SHIFT, BUF_LVL_URGENT_TH_UP_LENGTH);
	value = MERGE_BITS(value, wb_lbuf_level.buf_urgent_down,
			BUF_LVL_URGENT_TH_DOWN_SHIFT, BUF_LVL_URGENT_TH_DOWN_LENGTH);
	DPU_REG_WRITE(hw, BUF_LVL_URGENT_TH_UP_OFFSET, value, CMDLIST_WRITE);
}

void dpu_hw_wb_top_dbg_irq_enable(struct dpu_hw_blk *hw, bool enable)
{
	u32 value;

	/* unmask below error irq:
	 * [3:0] master_wr_timeout
	 * [7:4] mmu_timeout_err
	 * [11:8] mmu_tbu_size_err
	 * [20] wdma_resp_err
	 */
	if (enable) {
		value = 0x1F0FFF;
		DPU_REG_WRITE(hw, WDMA_DBG_IRQ_MASK_OFFSET, value, CMDLIST_WRITE);
	} else {
		value = 0;
		DPU_REG_WRITE(hw, WDMA_DBG_IRQ_MASK_OFFSET, value, DIRECT_WRITE);
	}
}

void dpu_hw_wb_top_dbg_irq_clear(struct dpu_hw_blk *hw)
{
	dpu_hw_wb_top_status_clear(hw);
}

u32 dpu_hw_wb_top_dbg_irq_dump(struct dpu_hw_blk *hw)
{
	return DPU_REG_READ(hw, WDMA_DBG_IRQ_RAW_OFFSET);
}
