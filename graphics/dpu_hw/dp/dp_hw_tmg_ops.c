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

#include "dpu_hw_common.h"
#include "dp_hw_tmg_reg.h"
#include "dp_hw_tmg_ops.h"

// hw recommended value: 100
#define CORE_INIT_LN_CFG_NUM_VALUE                   100

#define TMG_FORMAT_DSC                               3
#define IPI_COLOR_DEPTH_DSC                          0xE
#define IPI_FORMAT_DSC                               0xB
#define TMG_DP_SELFTEST_COLOR_RGB_DEFAULT            0x81C995
#define TMG_DP_SELFTEST_COLOR_RGB_BLACK              0x0
#define TMG_DP_SELFTEST_MODE_CUSTOM                  0x3
#define TMG_DP_SOF_CFG_SELECT_VSYNC                  1
#define TMG_DP_SOF_CFG_LN_NUM                        24

static u8 dp_hw_tmg_ipi_color_depth_code(enum dp_bpc bpc)
{
	switch (bpc) {
	case DP_BPC_10:
		return 0x6;
	case DP_BPC_8:
		return 0x5;
	case DP_BPC_6:
		return 0x3;
	default:
		return 0x5;
	}
}

static u32 dp_hw_tmg_line_of_240_num(u32 h_active, enum dp_bpc bpc)
{
	switch (bpc) {
	case DP_BPC_6:
	case DP_BPC_8:
		return DIV_ROUND_UP(h_active, 10);
	case DP_BPC_10:
		return DIV_ROUND_UP(h_active, 8);
	default:
		return DIV_ROUND_UP(h_active, 10);
	}
}

static u8 dp_hw_tmg_tmg_format_code(enum dp_bpc bpc)
{
	switch (bpc) {
	case DP_BPC_6:
		return 0;
	case DP_BPC_8:
		return 1;
	case DP_BPC_10:
		return 2;
	default:
		return 1;
	}
}

/**
 * tmg limitation: hbp, hfp, hsa should not be less than the minium value.
 * This function is to move partition of which is larger than the minium value
 * to those less than the minium value. We should satisfy the limitation by the
 * priority order of hsa, hbp, hfp.
 */
static void dp_hw_tmg_adjust_h_blank(u32 *hfp, u32 *hsa, u32 *hbp)
{
	int hfp_compensation = (int)(*hfp) - DP_TMG_MIN_H_FRONT_PORCH;
	int hsa_compensation = (int)(*hsa) - DP_TMG_MIN_H_SYNC_WIDTH;
	int hbp_compensation = (int)(*hbp) - DP_TMG_MIN_H_BACK_PORCH;
	/* the value all of three components should be compensated to minium value */
	int compensation = 0;

	/* hfp is less than limitation and should be compensated */
	if (hfp_compensation < 0) {
		compensation += abs(hfp_compensation);
		hfp_compensation = 0;
	}

	if (hsa_compensation < 0) {
		compensation += abs(hsa_compensation);
		hsa_compensation = 0;
	}

	if (hbp_compensation < 0) {
		compensation += abs(hbp_compensation);
		hbp_compensation = 0;
	}

	if (compensation == 0) {
		/* no partition need to be adjusted */
		return;
	}

	/* hfp is larger than limitation and can compensate to other component */
	if (hfp_compensation > 0) {
		/* the real compensated value from hfp */
		int tmp_comp = min_t(int, hfp_compensation, compensation);
		hfp_compensation -= tmp_comp;
		compensation -= tmp_comp;
	}

	if (hbp_compensation > 0 && compensation > 0) {
		int tmp_comp = min_t(int, hbp_compensation, compensation);
		hbp_compensation -= tmp_comp;
		compensation -= tmp_comp;
	}

	if (hsa_compensation > 0 && compensation > 0) {
		int tmp_comp = min_t(int, hsa_compensation, compensation);
		hsa_compensation -= tmp_comp;
		compensation -= tmp_comp;
	}

	DP_DEBUG("original, hfp:%d, hsa:%d, hbp:%d, h_blank:%d\n",
			*hfp, *hsa, *hbp, *hfp + *hsa + *hbp);
	*hfp = DP_TMG_MIN_H_FRONT_PORCH + hfp_compensation;
	*hsa = DP_TMG_MIN_H_SYNC_WIDTH + hsa_compensation;
	*hbp = DP_TMG_MIN_H_BACK_PORCH + hbp_compensation;
	DP_DEBUG("adjusted, hfp:%d, hsa:%d, hbp:%d, h_blank:%d\n",
			*hfp, *hsa, *hbp, *hfp + *hsa + *hbp);
}

static void dp_hw_tmg_config_video(struct dpu_hw_blk *hw,
		struct dp_display_timing *timing, enum dp_bpc bpc)
{
	u32 cmdlist_ln_start, cmdlist_ln_end;
	u32 sof_cfg_ln_num;
	u32 hfp, hsa, hbp;
	u32 val;

	hfp = timing->h_front_porch;
	hsa = timing->h_sync_width;
	hbp = timing->h_back_porch;
	dp_hw_tmg_adjust_h_blank(&hfp, &hsa, &hbp);

	/* config timing engine */
	/* clear underflow count */
	DP_REG_WRITE(hw, TMG_DP_UNDERFLOW_CNT_CLR_OFFSET, 0x1);

	/* HBP, HSA */
	val = 0;
	val = MERGE_BITS(val, hbp, TMG_DP_HBP_SHIFT, TMG_DP_HBP_LENGTH);
	val = MERGE_BITS(val, hsa, TMG_DP_HSA_SHIFT, TMG_DP_HSA_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_HSA_OFFSET, val);

	/* HFP, HACT */
	val = 0;
	val = MERGE_BITS(val, hfp, TMG_DP_HFP_SHIFT, TMG_DP_HFP_LENGTH);
	val = MERGE_BITS(val, timing->h_active,
			TMG_DP_HACT_SHIFT, TMG_DP_HACT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_HACT_OFFSET, val);

	/* VBP + VSA */
	val = 0;
	val = MERGE_BITS(val, timing->v_back_porch,
			TMG_DP_VBP_SHIFT, TMG_DP_VBP_LENGTH);
	val = MERGE_BITS(val, timing->v_sync_width,
			TMG_DP_VSA_SHIFT, TMG_DP_VSA_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_VSA_OFFSET, val);

	/* VACT */
	val = 0;
	val = MERGE_BITS(val, timing->v_active,
			TMG_DP_VACT_SHIFT, TMG_DP_VACT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_VACT_OFFSET, val);

	/* VFP */
	val = 0;
	val = MERGE_BITS(val, timing->v_front_porch, TMG_DP_VFP_SHIFT, TMG_DP_VFP_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_VFP_OFFSET, val);

	/* HACT_FSM */
	val = 0;
	val = MERGE_BITS(val, timing->h_active,
			TMG_DP_HACT_FSM_SHIFT, TMG_DP_HACT_FSM_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_HACT_FSM_OFFSET, val);

	/* core_int_ln_cfg_num */
	DP_REG_WRITE(hw, TMG_DP_CORE_INIT_LN_CFG_NUM_OFFSET,
			CORE_INIT_LN_CFG_NUM_VALUE);

	/* pre fetch 24 line for flow ctrl */
	if (timing->v_blank - timing->v_front_porch > TMG_DP_SOF_CFG_LN_NUM)
		sof_cfg_ln_num = TMG_DP_SOF_CFG_LN_NUM;
	else
		sof_cfg_ln_num = (TMG_DP_SOF_CFG_SELECT_VSYNC << 16) | TMG_DP_SOF_CFG_LN_NUM;
	DP_REG_WRITE(hw, TMG_DP_SOF_CFG_LN_NUM_OFFSET, sof_cfg_ln_num);

	/* line of 240 num & tmg format */
	val = 0;
	val = MERGE_BITS(val, dp_hw_tmg_line_of_240_num(timing->h_active, bpc),
			TMG_DP_LN_OF_240_NUM_SHIFT, TMG_DP_LN_OF_240_NUM_LENGTH);
	val = MERGE_BITS(val, dp_hw_tmg_tmg_format_code(bpc),
			TMG_DP_TMG_FORMAT_SHIFT, TMG_DP_TMG_FORMAT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_TMG_FORMAT_OFFSET, val);

	/* ipi format & color depth */
	val = 0;
	val = MERGE_BITS(val, dp_hw_tmg_ipi_color_depth_code(bpc),
			TMG_DP_IPI_COLOR_DEPTH_SHIFT, TMG_DP_IPI_COLOR_DEPTH_LENGTH);
	val = MERGE_BITS(val, 0x0, TMG_DP_IPI_FORMAT_SHIFT, TMG_DP_IPI_FORMAT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_IPI_COLOR_DEPTH_OFFSET, val);

	/* ipi polarity */
	DP_REG_WRITE(hw, TMG_DP_DDR_DVFS_OK_LN_TRIG_OFFSET, 0x0);

	/* underflow count enable */
	DP_REG_WRITE(hw, TMG_DP_UNDERFLOW_CNT_EN_OFFSET, 0x1);

	/* underflow drift enable */
	DP_REG_WRITE(hw, TMG_DP_MCLK_CG2_AUTO_EN_OFFSET, 0x0);

	/* cg gap */
	DP_REG_WRITE(hw, TMG_DP_DPU_TOP_CG_GAP_NUM_OFFSET, 0x10);

	/* vsync update */
	val = 0;
	val = MERGE_BITS(val, 1,
			TMG_DP_VSYNC_UPDATE_EN_SHIFT, TMG_DP_VSYNC_UPDATE_EN_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_FORCE_UPDATE_EN_OFFSET, val);

	/* core fm timing en */
	val = DP_REG_READ(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET);
	val = MERGE_BITS(val, 1,
		TMG_DP_CORE_FM_TIMING_EN_SHIFT, TMG_DP_CORE_FM_TIMING_EN_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET, val);

	/* cmdlist_ln_start and end */
	if (timing->v_front_porch == 1) {
		cmdlist_ln_start = timing->v_active + timing->v_blank
				- timing->v_front_porch;
		cmdlist_ln_end = 0;
	} else if (timing->v_front_porch == 2) {
		cmdlist_ln_start = timing->v_active + timing->v_blank
				- timing->v_front_porch + 1;
		cmdlist_ln_end = 0;
	} else {
		cmdlist_ln_start = timing->v_active + timing->v_blank
				- timing->v_front_porch + 1;
		cmdlist_ln_end = cmdlist_ln_start + 1;
	}

	val = 0;
	val = MERGE_BITS(val, cmdlist_ln_start,
			TMG_DP_CMDLIST_LN_START_SHIFT, TMG_DP_CMDLIST_LN_START_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CMDLIST_LN_START_OFFSET, val);

	val = 0;
	val = MERGE_BITS(val, cmdlist_ln_end,
			TMG_DP_CMDLIST_LN_END_SHIFT, TMG_DP_CMDLIST_LN_END_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CMDLIST_LN_END_OFFSET, val);
}

void dp_hw_tmg_config_dsc(struct dpu_hw_blk *hw, struct dp_display_timing *timing)
{
	u32 h_act_compressed, h_blank_compressed, h_total_compressed;
	u32 hfp_compressed, hsa_compressed, hbp_compressed;
	u32 val;

#ifdef ASIC
	h_act_compressed = DIV_ROUND_UP(timing->h_active, 6);
	h_total_compressed = DIV_ROUND_UP(timing->h_total, 6);
	h_blank_compressed = h_total_compressed - h_act_compressed;

	hfp_compressed = DIV_ROUND_UP(timing->h_front_porch, 6);
	hsa_compressed = DIV_ROUND_UP(timing->h_sync_width, 6);
	hbp_compressed = h_blank_compressed - hfp_compressed - hsa_compressed;

	dp_hw_tmg_adjust_h_blank(&hfp_compressed, &hsa_compressed, &hbp_compressed);
#elif defined FPGA
	h_act_compressed = DIV_ROUND_UP(timing->h_active, 6);
	h_total_compressed = timing->h_total;
	h_blank_compressed = h_total_compressed - h_act_compressed;

	hfp_compressed = timing->h_front_porch;
	hsa_compressed = timing->h_sync_width;
	hbp_compressed = h_blank_compressed - hfp_compressed - hsa_compressed;
#endif

	/* h_back_porch and h_sync_active */
	val = 0;
	val = MERGE_BITS(val, hbp_compressed,
			TMG_DP_HBP_SHIFT, TMG_DP_HBP_LENGTH);
	val = MERGE_BITS(val, hsa_compressed,
			TMG_DP_HSA_SHIFT, TMG_DP_HSA_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_HSA_OFFSET, val);

	/* h_front_porch and h_active */
	val = 0;
	val = MERGE_BITS(val, hfp_compressed,
			TMG_DP_HFP_SHIFT, TMG_DP_HFP_LENGTH);
	val = MERGE_BITS(val, timing->h_active,
			TMG_DP_HACT_SHIFT, TMG_DP_HACT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_HACT_OFFSET, val);

	/* HACT_FSM */
	val = 0;
	val = MERGE_BITS(val, h_act_compressed,
			TMG_DP_HACT_FSM_SHIFT, TMG_DP_HACT_FSM_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_HACT_FSM_OFFSET, val);

	/**
	 * line of 240 num & tmg format
	 * line_of_240 = hact_bits / 240 = hact * bpp / 240 = hact * 8 / 240
	 */
	val = 0;
	val = MERGE_BITS(val, DIV_ROUND_UP(timing->h_active, 30),
			TMG_DP_LN_OF_240_NUM_SHIFT, TMG_DP_LN_OF_240_NUM_SHIFT);
	val = MERGE_BITS(val, TMG_FORMAT_DSC,
			TMG_DP_TMG_FORMAT_SHIFT, TMG_DP_TMG_FORMAT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_TMG_FORMAT_OFFSET, val);

	/* ipi format & color depth */
	val = 0;
	val = MERGE_BITS(val, IPI_COLOR_DEPTH_DSC,
			TMG_DP_IPI_COLOR_DEPTH_SHIFT, TMG_DP_IPI_COLOR_DEPTH_LENGTH);
	val = MERGE_BITS(val, IPI_FORMAT_DSC,
			TMG_DP_IPI_FORMAT_SHIFT, TMG_DP_IPI_FORMAT_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_IPI_COLOR_DEPTH_OFFSET, val);
}

void dp_hw_tmg_config(struct dpu_hw_blk *hw, struct dp_display_timing *timing,
		enum dp_bpc bpc, bool dsc_en)
{
	if (!hw) {
		DP_ERROR("hardware block pointer is NULL\n");
		return;
	}

	if (!timing) {
		DP_ERROR("timing is NULL\n");
		return;
	}

	if (bpc != DP_BPC_6 && bpc != DP_BPC_8 && bpc != DP_BPC_10) {
		DP_ERROR("bits per component (%d) is out of range, reset to 8\n", bpc);
		bpc = DP_BPC_8;
	}

	dp_hw_tmg_config_video(hw, timing, bpc);

	if (dsc_en) {
		DP_DEBUG("overwrite dsc related register\n");
		dp_hw_tmg_config_dsc(hw, timing);
	}
}

static void _dp_hw_tmg_enable_self_test(struct dpu_hw_blk *hw, u32 mode_idx,
		u32 color_rgb, bool enable)
{
	u32 reg = 0;

	if (enable) {
		if (mode_idx > TMG_DP_SELFTEST_MODE_CUSTOM) {
			DP_ERROR("tmg self test idx (%d) is out of range (0 - 3).\n",
					mode_idx);
			return;
		}

		reg = MERGE_BITS(reg, mode_idx,
				TMG_DP_AUTO_SELFTEST_MODE_SHIFT,
				TMG_DP_AUTO_SELFTEST_MODE_LENGTH);
		reg = MERGE_BITS(reg, 0x1,
				TMG_DP_AUTO_SELFTEST_EN_SHIFT,
				TMG_DP_AUTO_SELFTEST_EN_LENGTH);
		DP_REG_WRITE(hw, TMG_DP_AUTO_SELFTEST_MAN_RGB_OFFSET, color_rgb);
		DP_REG_WRITE(hw, TMG_DP_AUTO_SELFTEST_EN_OFFSET, reg);
	} else {
		DP_REG_WRITE(hw, TMG_DP_AUTO_SELFTEST_EN_OFFSET, 0);
	}
}

void dp_hw_tmg_enable_self_test(struct dpu_hw_blk *hw, u32 mode_idx, bool enable)
{
	if (enable)
		DP_INFO("tmg mode idx:%d, enable:%d\n", mode_idx, enable);

	_dp_hw_tmg_enable_self_test(hw, mode_idx - 1,
			TMG_DP_SELFTEST_COLOR_RGB_DEFAULT, enable);
}

void dp_hw_tmg_send_black_frame(struct dpu_hw_blk *hw)
{
	_dp_hw_tmg_enable_self_test(hw, TMG_DP_SELFTEST_MODE_CUSTOM,
			TMG_DP_SELFTEST_COLOR_RGB_BLACK, true);
}

void dp_hw_tmg_enable(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DP_ERROR("hardware block pointer is NULL\n");
		return;
	}

	if (enable) {
		DP_REG_WRITE(hw, TMG_DP_TMG_EN_OFFSET, 0x1);
	} else {
		/* tmg disable */
		DP_REG_WRITE(hw, TMG_DP_TMG_EN_OFFSET, 0x0);
		/* obuf disable */
		DP_REG_WRITE(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET, 0x0);
		/* reload register */
		DP_REG_WRITE(hw, TMG_DP_TRIGGER2_OFFSET, 0x1);
		/* underflow cnt clear */
		DP_REG_WRITE(hw, TMG_DP_UNDERFLOW_CNT_CLR_OFFSET, 0x1);
	}
}

void dp_hw_tmg_get_state(struct dpu_hw_blk *hw, u32 *count, u32 *state)
{
	if (!hw) {
		DP_ERROR("hardware block pointer is NULL\n");
		return;
	}

	if (!count) {
		DP_ERROR("count pointer is NULL\n");
		return;
	}

	if (!state) {
		DP_ERROR("state pointer is NULL\n");
		return;
	}

	*count = DP_REG_READ(hw, TMG_DP_UNDERFLOW_EACH_FM_CNT_OFFSET);
	*state = DP_REG_READ(hw, TMG_DP_DEBUG_RD_OFFSET);
}

void dp_hw_tmg_obufen_config(struct dpu_hw_blk *hw, u32 obufen)
{
	u32 value = 0;

	/* enable obuffer */
	value = DP_REG_READ(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET);
	value = MERGE_BITS(value, GET_BIT_VAL(obufen, 0),
			TMG_DP_OBUF0_EN_SHIFT, TMG_DP_OBUF0_EN_LENGTH);
	value = MERGE_BITS(value, GET_BIT_VAL(obufen, 1),
			TMG_DP_OBUF1_EN_SHIFT, TMG_DP_OBUF1_EN_LENGTH);
	value = MERGE_BITS(value, GET_BIT_VAL(obufen, 2),
			TMG_DP_OBUF2_EN_SHIFT, TMG_DP_OBUF2_EN_LENGTH);
	value = MERGE_BITS(value, GET_BIT_VAL(obufen, 3),
			TMG_DP_OBUF3_EN_SHIFT, TMG_DP_OBUF3_EN_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET, value);
}

void dp_hw_tmg_obuffer_level_config(struct dpu_hw_blk *hw, struct obuffer_parms *obuffer)
{
	u32 value;

	if (!hw) {
		DP_ERROR("hardware block pointer is NULL\n");
		return;
	}

	/* REGISTER tmg_dp_reg_11 */
	value = DP_REG_READ(hw, TMG_DP_SPLIT_EN_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_urgent_vld_en_buf,
		TMG_DP_CORE_URGENT_VLD_EN_BUF_SHIFT,
		TMG_DP_CORE_URGENT_VLD_EN_BUF_LENGTH);
	value = MERGE_BITS(value, obuffer->core_dfc_l_thre_buf,
		TMG_DP_CORE_DFC_L_THRE_BUF_SHIFT,
		TMG_DP_CORE_DFC_L_THRE_BUF_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_SPLIT_EN_BUF_OFFSET, value);

	/* REGISTER tmg_dp_reg_12 */
	value = DP_REG_READ(hw, TMG_DP_CORE_DFC_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_dfc_h_thre_buf,
		TMG_DP_CORE_DFC_H_THRE_BUF_SHIFT,
		TMG_DP_CORE_DFC_H_THRE_BUF_LENGTH);
	value = MERGE_BITS(value, obuffer->core_urgent_l_thre_buf,
		TMG_DP_CORE_URGENT_L_THRE_BUF_SHIFT,
		TMG_DP_CORE_URGENT_L_THRE_BUF_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_DFC_H_THRE_BUF_OFFSET, value);

	/* REGISTER tmg_dp_reg_13 */
	value = DP_REG_READ(hw, TMG_DP_CORE_URGENT_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_urgent_h_thre_buf,
		TMG_DP_CORE_URGENT_H_THRE_BUF_SHIFT,
		TMG_DP_CORE_URGENT_H_THRE_BUF_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_URGENT_H_THRE_BUF_OFFSET, value);

	/* REGISTER tmg_dp_reg_14 */
	value = DP_REG_READ(hw, TMG_DP_CORE_DPU_DVFS_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_obuf_lvl_thre_buf0,
		TMG_DP_CORE_OBUF_LVL_THRE_BUF0_SHIFT,
		TMG_DP_CORE_OBUF_LVL_THRE_BUF0_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_DPU_DVFS_H_THRE_BUF_OFFSET, value);

	/* REGISTER tmg_dp_reg_15 */
	value = DP_REG_READ(hw, TMG_DP_CORE_OBUF_LVL_THRE_BUF1_OFFSET);
	value = MERGE_BITS(value, obuffer->core_obuf_lvl_thre_buf1,
		TMG_DP_CORE_OBUF_LVL_THRE_BUF1_SHIFT,
		TMG_DP_CORE_OBUF_LVL_THRE_BUF1_LENGTH);
	value = MERGE_BITS(value, obuffer->core_obuf_lvl_thre_buf2,
		TMG_DP_CORE_OBUF_LVL_THRE_BUF2_SHIFT,
		TMG_DP_CORE_OBUF_LVL_THRE_BUF2_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_OBUF_LVL_THRE_BUF1_OFFSET, value);

	/* REGISTER tmg_dp_reg_16 */
	value = DP_REG_READ(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_obuf_th_buf,
		TMG_DP_CORE_OBUF_TH_BUF_SHIFT,
		TMG_DP_CORE_OBUF_TH_BUF_LENGTH);
	DP_REG_WRITE(hw, TMG_DP_CORE_OBUF_TH_BUF_OFFSET, value);
}
