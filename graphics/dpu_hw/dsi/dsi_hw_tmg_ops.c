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


#include "dpu_osal.h"
#include "dsi_hw_tmg_ops.h"
#include "dsi_tmg_reg.h"
#include "dsi_hw_tmg.h"

/* HW DVFS slow mode threshold */
#define DPU_DVFS_L_THRE_VALUE                                         0x9F7
#define DPU_DVFS_H_THRE_VALUE                                         0x14D8
#define TMG_1S_NS                                                 1000000000
#define T_120HZ_NS                                                   8333333

struct dsi_hw_tmg_hw_cfg_info {
	struct dsi_sync_timing timing;

	/**
	 * vtotal = vsa + vbp + actual_vact + vfp;
	 * if partital update enable
	 *    actual_vact = partial.height;
	 * else
	 *    actual_vact = timing.vact;
	 */
	u32 vtotal;
	u32 actual_vact;
	u32 skip_num;

	/* cmdlist cfg win */
	u32 cmdlist_ln_start;
	u32 cmdlist_ln_end;

	/* ddr act dvfs win */
	u32 ddr_act_dvfs_ln_start;
	u32 ddr_act_dvfs_ln_end;
	u32 ddr_dvfs_ok_ln_trig;

	/* power up/down win */
	u32 pwup_en;
	u32 pwdw_en;
	u32 pwdw_dly_fm;
	u32 pwdw_ln_trig;
	u32 dpu_ctrl_pwup_thre;

	/* ulps win */
	u32 ulps_en;
	u32 ulps_ln_enter;
	u32 ulps_dly_fm;
	u32 ulps_ln_exit;
	u32 ulps_pre_fm;

	/* doze win */
	u32 doze_dly_fm;
	u32 doze_pre_fm;
	u32 doze_ln_start;
	u32 doze_ln_end;

	/* sof cfg */
	u32 sof_cfg_ln_num;
};

static u32 __maybe_unused get_skip_frame(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	u32 skip;

	skip = cfg->fps_base / cfg->fps_update - 1;

	return skip;
}

static void __maybe_unused get_current_base_timing(struct dsi_sync_timing *tm,
		struct dsi_tmg_cfg *cfg)
{
	if (cfg->current_base > TMG_BASE_3) {
		DSI_ERROR("curren base out of range: %d\n", cfg->current_base);
		return;
	}

	switch (cfg->current_base) {
	case TMG_BASE_0:
		*tm = cfg->base.base0;
		break;
	case TMG_BASE_1:
		*tm = cfg->base.base1;
		break;
	case TMG_BASE_2:
		*tm = cfg->base.base2;
		break;
	case TMG_BASE_3:
		*tm = cfg->base.base3;
		break;
	default:
		*tm = cfg->base.base0;
		break;
	}
}

static u32 get_line_time(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	struct dsi_sync_timing tm = {0};
	u32 vtotal, t_line_ns, fps;

	get_current_base_timing(&tm, cfg);

	fps = cfg->fps_base;

	vtotal = tm.vsa + tm.vbp + tm.vfp + tm.vact;

	if ((fps == 0) || (vtotal == 0)) {
		DSI_ERROR("The divisor cannot be 0, fps: %d, vtotal: %d\n",
			cfg->fps_base, vtotal);
		return 0;
	}

	t_line_ns = TMG_1S_NS / (fps * vtotal);

	return t_line_ns;
}

static u32 get_line_count(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg, u32 t_ns)
{
	u32 t_line, line_count;

	t_line = get_line_time(hw, cfg);

	if (t_line == 0) {
		DSI_ERROR("t_line cannot be 0!: %d\n", t_line);

		t_line = TMG_T_LINE_NS_DEFAULTE;
	}

	line_count = DIV_ROUND_UP(t_ns, t_line);

	DSI_DEBUG("t_line %d, t_ns %d, line %d\n", t_line, t_ns, line_count);

	return line_count;
}

static u32 get_frame_time_ns(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	u32 t_frame_ns;

	if (cfg->fps_base == 0) {
		DSI_ERROR("fps_base cannot be 0!: %d\n", cfg->fps_base);

		return 0;
	}

	t_frame_ns = TMG_1S_NS / cfg->fps_base;

	DSI_DEBUG("t_frame_ns %d base %d\n", t_frame_ns, cfg->fps_base);

	return t_frame_ns;
}

static u32 __maybe_unused get_clk_time_ns(u32 clk)
{
	u32 t_clk;

	if (clk == 0) {
		DSI_ERROR("The divisor cannot be 0, clk: %d\n", clk);
		return -1;
	}

	t_clk = TMG_1S_NS / clk;

	DSI_DEBUG("t_clk %d clk %d\n", t_clk, clk);

	return t_clk;
}

void dsi_hw_tmg_timing_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	struct base_timing *base;
	enum dsi_pixel_format format;
	u32 hact_fsm;
	u32 value;
	u8 flag;

	base = &cfg->base;
	format = cfg->format;
	flag = cfg->cmdlist_flag;

	if (format == DSI_FMT_DSC)
		hact_fsm = DIV_ROUND_UP(base->base0.hact, 6);
	else
		hact_fsm = base->base0.hact;

	value = DPU_REG_READ(hw, TMG_DSI_HACT_OFFSET);

	value = MERGE_BITS(value, hact_fsm,
		TMG_DSI_HACT_FSM_SHIFT, TMG_DSI_HACT_FSM_LENGTH);
	value = MERGE_BITS(value, base->base0.hact,
		TMG_DSI_HACT_SHIFT, TMG_DSI_HACT_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_HACT_OFFSET, value, flag);

	DPU_BIT_WRITE(hw, TMG_DSI_VACT_OFFSET, base->base0.vact,
		TMG_DSI_VACT_SHIFT, TMG_DSI_VACT_LENGTH, flag);

	value = DPU_REG_READ(hw, HSA0_OFFSET);

	value = MERGE_BITS(value, base->base0.hsa, HSA0_SHIFT, HSA0_LENGTH);
	value = MERGE_BITS(value, base->base0.hbp, HBP0_SHIFT, HBP0_LENGTH);
	DPU_REG_WRITE(hw, HSA0_OFFSET, value, flag);

	value = DPU_REG_READ(hw, HFP0_OFFSET);

	value = MERGE_BITS(value, base->base0.hfp, HFP0_SHIFT, HFP0_LENGTH);
	value = MERGE_BITS(value, base->base0.vsa, VSA0_SHIFT, VSA0_LENGTH);
	DPU_REG_WRITE(hw, HFP0_OFFSET, value, flag);

	DPU_BIT_WRITE(hw, VBP0_OFFSET, base->base0.vbp,
		VBP0_SHIFT, VBP0_LENGTH, flag);
	DPU_BIT_WRITE(hw, VFP0_OFFSET, base->base0.vfp,
		VFP0_SHIFT, VFP0_LENGTH, flag);

	value = DPU_REG_READ(hw, HSA1_OFFSET);

	value = MERGE_BITS(value, base->base1.hsa, HSA1_SHIFT, HSA1_LENGTH);
	value = MERGE_BITS(value, base->base1.hbp, HBP1_SHIFT, HBP1_LENGTH);
	DPU_REG_WRITE(hw, HSA1_OFFSET, value, flag);

	value = DPU_REG_READ(hw, HFP1_OFFSET);

	value = MERGE_BITS(value, base->base1.hbp, HFP1_SHIFT, HFP1_LENGTH);
	value = MERGE_BITS(value, base->base1.vsa, VSA1_SHIFT, VSA1_LENGTH);
	DPU_REG_WRITE(hw, HFP1_OFFSET, value, flag);

	DPU_BIT_WRITE(hw, VBP1_OFFSET, base->base1.vbp,
		VBP1_SHIFT, VBP1_LENGTH, flag);
	DPU_BIT_WRITE(hw, VFP1_OFFSET, base->base1.vfp,
		VFP1_SHIFT, VFP1_LENGTH, flag);

	value = DPU_REG_READ(hw, HSA2_OFFSET);

	value = MERGE_BITS(value, base->base2.hsa, HSA2_SHIFT, HSA2_LENGTH);
	value = MERGE_BITS(value, base->base2.hbp, HBP2_SHIFT, HBP2_LENGTH);
	DPU_REG_WRITE(hw, HSA2_OFFSET, value, flag);

	value = DPU_REG_READ(hw, HFP2_OFFSET);

	value = MERGE_BITS(value, base->base2.hbp, HFP2_SHIFT, HFP2_LENGTH);
	value = MERGE_BITS(value, base->base2.vsa, VSA2_SHIFT, VSA2_LENGTH);
	DPU_REG_WRITE(hw, HFP2_OFFSET, value, flag);

	DPU_BIT_WRITE(hw, VBP2_OFFSET, base->base2.vbp,
		VBP2_SHIFT, VBP2_LENGTH, flag);
	DPU_BIT_WRITE(hw, VFP2_OFFSET, base->base2.vfp,
		VFP2_SHIFT, VFP2_LENGTH, flag);

	value = DPU_REG_READ(hw, HSA3_OFFSET);

	value = MERGE_BITS(value, base->base3.hsa, HSA3_SHIFT, HSA3_LENGTH);
	value = MERGE_BITS(value, base->base3.hbp, HBP3_SHIFT, HBP3_LENGTH);
	DPU_REG_WRITE(hw, HSA3_OFFSET, value, flag);

	value = DPU_REG_READ(hw, HFP3_OFFSET);

	value = MERGE_BITS(value, base->base3.hbp, HFP3_SHIFT, HFP3_LENGTH);
	value = MERGE_BITS(value, base->base3.vsa, VSA3_SHIFT, VSA3_LENGTH);
	DPU_REG_WRITE(hw, HFP3_OFFSET, value, flag);

	DPU_BIT_WRITE(hw, VBP3_OFFSET, base->base3.vbp,
		VBP3_SHIFT, VBP3_LENGTH, flag);
	DPU_BIT_WRITE(hw, VFP3_OFFSET, base->base3.vfp,
		VFP3_SHIFT, VFP3_LENGTH, flag);
}

static void dsi_hw_tmg_dvfs_cfg(struct dpu_hw_blk *hw)
{
	u32 value;

	value = DPU_REG_READ(hw, TMG_DSI_CORE_URGENT_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, DPU_DVFS_L_THRE_VALUE,
			TMG_DSI_CORE_DPU_DVFS_L_THRE_BUF_SHIFT,
			TMG_DSI_CORE_DPU_DVFS_L_THRE_BUF_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CORE_URGENT_H_THRE_BUF_OFFSET, value, DIRECT_WRITE);

	value = DPU_REG_READ(hw, TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, DPU_DVFS_H_THRE_VALUE,
			TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_SHIFT,
			TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_OFFSET, value, DIRECT_WRITE);
}

static void dsi_hw_tmg_static_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	u32 tmp, ipi_format, ipi_depth, tmg_mode, tmg_format;
	u32 half_frame_sel, cfg_int_ln;
	struct dsi_sync_timing current_timing = {0};
	u32 value;
	u8 flag;

	get_current_base_timing(&current_timing, cfg);
	flag = cfg->cmdlist_flag;

	tmp = 0;
	ipi_format = TMG_FMT_DSC;
	ipi_depth  = TMG_IPI_DEPTH_888;
	tmg_format = TMG_FORMAT_RGB888;
	tmg_mode = TMG_COMMAND_MODE;

	switch (cfg->format) {
	case DSI_FMT_RGB666:
		tmp = DIV_ROUND_UP(cfg->base.base0.hact, 10);
		ipi_format = TMG_FMT_RGB;
		ipi_depth  = TMG_IPI_DEPTH_666;
		tmg_format = TMG_FORMAT_RGB666;
		break;
	case DSI_FMT_RGB888:
		tmp = DIV_ROUND_UP(cfg->base.base0.hact, 10);
		ipi_format = TMG_FMT_RGB;
		ipi_depth  = TMG_IPI_DEPTH_888;
		tmg_format = TMG_FORMAT_RGB888;
		break;
	case DSI_FMT_RGB101010:
		tmp = DIV_ROUND_UP(cfg->base.base0.hact, 8);
		ipi_format = TMG_FMT_RGB;
		ipi_depth  = TMG_IPI_DEPTH_101010;
		tmg_format = TMG_FORMAT_RGB101010;
		break;
	case DSI_FMT_DSC:
		tmp = DIV_ROUND_UP(cfg->base.base0.hact, 30);
		ipi_format = TMG_FMT_DSC;
		ipi_depth  = TMG_IPI_DEPTH_888;
		tmg_format = TMG_FORMAT_DSC;
		break;
	case DSI_FMT_RGB565:
	case DSI_FMT_RGB121212:
	case DSI_FMT_YUV422:
	case DSI_FMT_YUV420:
	case DSI_FMT_YUV422_LOOSELY:
	case DSI_FMT_RGB_LOOSELY:
		DSI_ERROR("Unsupported pixel format: %d\n", cfg->format);
		break;
	default:
		/* default set RGB888 */
		tmp = DIV_ROUND_UP(cfg->base.base0.hact, 10);
		ipi_format = TMG_FMT_RGB;
		ipi_depth = TMG_IPI_DEPTH_888;
		break;
	}

	if (cfg->tmg_mode == DSI_VIDEO_MODE)
		tmg_mode = TMG_VIDEO_MODE;
	else if (cfg->tmg_mode == DSI_CMD_MODE)
		tmg_mode = TMG_COMMAND_MODE;
	else
		DSI_ERROR("Unsupported tmg_mode: %d\n", cfg->tmg_mode);

	/* set TMG pixel format */
	value = DPU_REG_READ(hw, TMG_DSI_TMG_FORMAT_OFFSET);

	value = MERGE_BITS(value, tmg_format,
		TMG_DSI_TMG_FORMAT_SHIFT, TMG_DSI_TMG_FORMAT_LENGTH);
	value = MERGE_BITS(value, tmp,
		TMG_DSI_LN_OF_240_NUM_SHIFT, TMG_DSI_LN_OF_240_NUM_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_TMG_FORMAT_OFFSET, value, flag);

	/* set TMG to DSI IPI  pixel format */
	value = DPU_REG_READ(hw, TMG_DSI_IPI_COLOR_DEPTH_OFFSET);

	value = MERGE_BITS(value, ipi_depth,
		TMG_DSI_IPI_COLOR_DEPTH_SHIFT, TMG_DSI_IPI_COLOR_DEPTH_LENGTH);
	value = MERGE_BITS(value, ipi_format,
		TMG_DSI_IPI_FORMAT_SHIFT, TMG_DSI_IPI_FORMAT_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_IPI_COLOR_DEPTH_OFFSET, value, flag);

	value = DPU_REG_READ(hw, SCREEN_MODE_OFFSET);
	/* set tmg mode: video mode or command mode */
	value = MERGE_BITS(value, tmg_mode,
		SCREEN_MODE_SHIFT, SCREEN_MODE_LENGTH);
	/* set tmg sbs enable or disable */
	value = MERGE_BITS(value, cfg->sbs_en, SBS_EN_SHIFT, SBS_EN_LENGTH);
	DPU_REG_WRITE(hw, SCREEN_MODE_OFFSET, value, flag);

	/* set current base */
	DPU_BIT_WRITE(hw, BASE_SEL_OFFSET, cfg->current_base,
		BASE_SEL_SHIFT, BASE_SEL_LENGTH, flag);

	/* 0x54: */
	DPU_REG_WRITE(hw, TMG_DSI_CORE_INIT_LN_CFG_NUM_OFFSET,
		TMG_CORE_INIT_LN_CFG_NUM, flag);

	/*0x5C: vactive hline count interrupt: 1/2, 1/4, 3/4 */
	half_frame_sel = 1;
	/* vactive hline count interrupt: 0 same as vact interrupt */
	cfg_int_ln = 0;

	value = DPU_REG_READ(hw, TMG_DSI_HALF_FRAME_SEL_OFFSET);

	value = MERGE_BITS(value, cfg_int_ln,
		TMG_DSI_CFG_INT_LN_SHIFT, TMG_DSI_CFG_INT_LN_LENGTH);
	value = MERGE_BITS(value, half_frame_sel,
		TMG_DSI_HALF_FRAME_SEL_SHIFT, TMG_DSI_HALF_FRAME_SEL_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_HALF_FRAME_SEL_OFFSET, value, flag);

	/* 0x90 video mode EM pulse cfg; video_sync_opt is default val: 0 */
	DPU_BIT_WRITE(hw, VIDEO_SYNC_OPT_OFFSET, cfg->vrr_vid.video_em_unit_num,
		VIDEO_EM_UNIT_NUM_SHIFT, VIDEO_EM_UNIT_NUM_LENGTH, flag);

	DPU_REG_WRITE(hw, TMG_DSI_MCLK_CG2_AUTO_EN_OFFSET, 0x0, true);
	DPU_REG_WRITE(hw, TMG_DSI_FORCE_UPDATE_EN_OFFSET, 0x2, true);

	dsi_hw_tmg_dvfs_cfg(hw);
}

static void dsi_hw_tmg_sbs_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	struct side_by_side sbs = {0};
	u32 value;
	u8 flag;

	flag = cfg->cmdlist_flag;
	sbs.sbs_dly_num = 0;
	/* TODO: ASIC double check 1 */
	sbs.sbs_dly_en = 0;
	sbs.sbs_hact_div2 = 0;
	sbs.overlap = 0;

	if (hw->blk_id == TMG_BLOCK_0)
		sbs.sbs_int_index = 1;
	else if (hw->blk_id == TMG_BLOCK_1)
		sbs.sbs_int_index = 0;

	value = DPU_REG_READ(hw, OVERLAP_OFFSET);

	value = MERGE_BITS(value, sbs.overlap, OVERLAP_SHIFT, OVERLAP_LENGTH);
	value = MERGE_BITS(value, sbs.sbs_hact_div2,
		SBS_HACT_DIV2_SHIFT, SBS_HACT_DIV2_LENGTH);
	value = MERGE_BITS(value, sbs.sbs_dly_en,
		SBS_DLY_EN_SHIFT, SBS_DLY_EN_LENGTH);
	value = MERGE_BITS(value, sbs.sbs_dly_num,
		SBS_DLY_NUM_SHIFT, SBS_DLY_NUM_LENGTH);
	DPU_REG_WRITE(hw, OVERLAP_OFFSET, value, flag);

	DPU_REG_WRITE(hw, SBS_INT_INDEX_OFFSET, sbs.sbs_int_index, flag);

	if (cfg->tmg_mode == DSI_CMD_MODE) {
		value = DPU_REG_READ(hw, DDIC_HSYNC_START_OFFSET);
		value = MERGE_BITS(value, 1,
			DDIC_HSYNC_ST_SEL_SHIFT, DDIC_HSYNC_ST_SEL_LENGTH);
		value = MERGE_BITS(value, 0,
			DDIC_HSYNC_POL_SHIFT, DDIC_HSYNC_POL_LENGTH);
		DPU_REG_WRITE(hw, DDIC_HSYNC_START_OFFSET, value, flag);
	}
}

static void dsi_hw_tmg_cmd_mode_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	u32 t_clk, cmd_wait_data_tout, value;
	u8 flag;

	flag = cfg->cmdlist_flag;

	t_clk = get_clk_time_ns(cfg->ipi_clk);

	cmd_wait_data_tout = DIV_ROUND_UP(TMG_CMD_WAIT_50MS, t_clk);

	DSI_DEBUG("cmd_wait_data_en:%d, cmd_wait_data_tout: %d\n",
		 cfg->cmd_wait_data_en, cmd_wait_data_tout);

	/* for blurred screen function: 0-Disable, 1-Enable */
	DPU_BIT_WRITE(hw, CMD_WAIT_DATA_EN_OFFSET, cfg->cmd_wait_data_en,
		CMD_WAIT_DATA_EN_SHIFT, CMD_WAIT_DATA_EN_LENGTH, flag);

	value = DPU_REG_READ(hw, CMD_WAIT_DATA_TOUT_OFFSET);
	/* select te from gpio */
	value = MERGE_BITS(value, TMG_SELECT_TE_FROM_GPIO,
		CMD_TE_SEL_SHIFT, CMD_TE_SEL_LENGTH);
	value = MERGE_BITS(value, cmd_wait_data_tout,
		CMD_WAIT_DATA_TOUT_SHIFT, CMD_WAIT_DATA_TOUT_LENGTH);
	/* ref: tmg register list */
	value = MERGE_BITS(value, TMG_CMD_TIME_AUTO_CAL_EN,
		CMD_TIME_AUTO_CAL_SHIFT, CMD_TIME_AUTO_CAL_LENGTH);
	DPU_REG_WRITE(hw, CMD_WAIT_DATA_TOUT_OFFSET, value, flag);

	value = DPU_REG_READ(hw, VRR_EN_OFFSET);
	/* command mode init cfg : vrr_en on */
	value = MERGE_BITS(value, TMG_VRR_ENABLE, VRR_EN_SHIFT, VRR_EN_LENGTH);
	/* ICG_OPT and CMD_MAN_MODE: default set 0 */
	value = MERGE_BITS(value, 0, CMD_ICG_OPT_SHIFT, CMD_ICG_OPT_LENGTH);
	value = MERGE_BITS(value, 0, CMD_MAN_MODE_SHIFT, CMD_MAN_MODE_LENGTH);
	/* command mode init cfg : set manual mode */
	value = MERGE_BITS(value, 0, CMD_AUTO_EN_SHIFT, CMD_AUTO_EN_LENGTH);
	DPU_REG_WRITE(hw, VRR_EN_OFFSET, value, flag);
}

static void dsi_hw_tmg_vg_test(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	struct vg_parms vg;
	u32 value;
	u8 flag;

	flag = cfg->cmdlist_flag;
	vg = cfg->vg;

	DSI_DEBUG("dsi_hw_tmg_vg_test\n");

	value = DPU_REG_READ(hw, TMG_DSI_AUTO_SELFTEST_EN_OFFSET);

	value = MERGE_BITS(value, 1, TMG_DSI_AUTO_SELFTEST_EN_SHIFT,
		TMG_DSI_AUTO_SELFTEST_EN_LENGTH);
	value = MERGE_BITS(value, vg.pat_sel, TMG_DSI_AUTO_SELFTEST_MODE_SHIFT,
		TMG_DSI_AUTO_SELFTEST_MODE_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_AUTO_SELFTEST_EN_OFFSET, value, flag);

	DPU_BIT_WRITE(hw, TMG_DSI_AUTO_SELFTEST_MAN_RGB_OFFSET,
		vg.user_pattern,
		TMG_DSI_AUTO_SELFTEST_MAN_RGB_SHIFT,
		TMG_DSI_AUTO_SELFTEST_MAN_RGB_LENGTH, flag);
}

static void dsi_hw_tmg_video_mode_vrr_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	u32 value;
	u8 flag;

	DSI_DEBUG("dsi_hw_tmg_video_mode_vrr_cfg\n");

	flag = cfg->cmdlist_flag;

	/* set current frame base */
	DPU_BIT_WRITE(hw, BASE_SEL_OFFSET, cfg->current_base,
		BASE_SEL_SHIFT, BASE_SEL_LENGTH, flag);

	/* vrr function disable: vrr_en only for skip frame mode */
	DPU_BIT_WRITE(hw, VRR_EN_OFFSET, TMG_VRR_DISABLE,
		VRR_EN_SHIFT, VRR_EN_LENGTH, flag);

	value = DPU_REG_READ(hw, VIDEO_FRAME_EM_NUM_OFFSET);
	value = MERGE_BITS(value, cfg->vrr_vid.video_frame_em_num,
		VIDEO_FRAME_EM_NUM_SHIFT, VIDEO_FRAME_EM_NUM_LENGTH);
	value = MERGE_BITS(value, 0,
		VIDEO_SKIP_FRAME_NUM_SHIFT, VIDEO_SKIP_FRAME_NUM_LENGTH);
	value = MERGE_BITS(value,  cfg->vrr_vid.video_em_next_num,
		VIDEO_EM_NEXT_NUM_SHIFT, VIDEO_EM_NEXT_NUM_LENGTH);
	value = MERGE_BITS(value, cfg->vrr_vid.video_det_te_en,
		VIDEO_DET_TE_EN_SHIFT, VIDEO_DET_TE_EN_LENGTH);
	DPU_REG_WRITE(hw, VIDEO_FRAME_EM_NUM_OFFSET, value, flag);

	if ((cfg->sbs_en == 1) && (cfg->at_once == 1))
		DSI_ERROR("MIPI dual port does not support at_once exit.\n");
	else if (cfg->at_once == 1)
		DPU_REG_WRITE(hw, VRR_ACK_AT_ONCE_OFFSET, TMG_ENABLE, DIRECT_WRITE);
	else
		DPU_REG_WRITE(hw, VRR_ACK_AT_ONCE_OFFSET, TMG_DISABLE, DIRECT_WRITE);
}

static void dsi_hw_tmg_command_mode_vrr_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	u32 command_mode_auto_vrr_en, skip_num;
	u32 value;
	u8 flag;

	flag = cfg->cmdlist_flag;

	if (cfg->vrr_mode == CMD_MODE_AUTO_VRR)
		command_mode_auto_vrr_en = 1;
	else
		command_mode_auto_vrr_en = 0;
	/* DIV_ROUND_UP(cfg->fps_base, cfg->fps_update); */
	skip_num = cfg->vrr_cmd.cmd_skip_te_next;

	/* set current frame base */
	DPU_BIT_WRITE(hw, BASE_SEL_OFFSET, cfg->current_base,
		BASE_SEL_SHIFT, BASE_SEL_LENGTH, flag);

	/* for blurred screen function: 0-Disable, 1-Enable */
	DPU_BIT_WRITE(hw, CMD_WAIT_DATA_EN_OFFSET, cfg->cmd_wait_data_en,
		CMD_WAIT_DATA_EN_SHIFT, CMD_WAIT_DATA_EN_LENGTH, flag);

	value = DPU_REG_READ(hw, VRR_EN_OFFSET);
	/* vrr function enable */
	value = MERGE_BITS(value, TMG_VRR_ENABLE, VRR_EN_SHIFT, VRR_EN_LENGTH);
	value = MERGE_BITS(value, skip_num,
		CMD_SKIP_TE_NEXT_SHIFT, CMD_SKIP_TE_NEXT_LENGTH);

	DPU_REG_WRITE(hw, VRR_EN_OFFSET, value, flag);

	if ((cfg->sbs_en == 1) && (cfg->at_once == 1))
		DSI_ERROR("MIPI dual port does not support at_once exit.\n");
	else if (cfg->at_once == 1)
		DPU_REG_WRITE(hw, VRR_ACK_AT_ONCE_OFFSET, TMG_ENABLE, DIRECT_WRITE);
	else
		DPU_REG_WRITE(hw, VRR_ACK_AT_ONCE_OFFSET, TMG_DISABLE, DIRECT_WRITE);
}

static void _get_tmg_base_timing_info(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;

	get_current_base_timing(timing, cfg);

	if (timing->vfp < TMG_MIN_VFP)
		DSI_ERROR("vfp must be >= %d, vfp: %d\n", TMG_MIN_VFP, timing->vfp);

	/* if partial update on : cmdlist window use partial hact */
	if (cfg->partial_en)
		hw_cfg_info->actual_vact = cfg->partial.height;
	else
		hw_cfg_info->actual_vact = timing->vact;

	hw_cfg_info->vtotal = timing->vsa + timing->vbp +
			hw_cfg_info->actual_vact + timing->vfp;

	hw_cfg_info->skip_num = get_skip_frame(hw, cfg);

	DSI_DEBUG("skip %d %d, fps %d %d\n",
			hw_cfg_info->skip_num,
			cfg->vrr_cmd.cmd_skip_te_next,
			cfg->fps_base,
			cfg->fps_update);
}

static void _get_cmdlist_cfg_info_in_active(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info,
		u32 *cmdlist_ln_end)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;
	u32 base_frame_time, cmdlist_pre_time;
	u32 line_count;

	base_frame_time = get_frame_time_ns(hw, cfg);
	if (base_frame_time == 0) {
		DSI_WARN("base_frame_time cannot be 0, set to 8.3 ms \n");

		base_frame_time = T_120HZ_NS;
	}

	cmdlist_pre_time = TMG_T_ULPS_EXIT_NS;

	if (cfg->frame_power_en)
		cmdlist_pre_time += TMG_T_PWUP_THRE_NS;

	if (cmdlist_pre_time > base_frame_time) {
		DSI_ERROR("cmdlist overflow, pre %d, base %d\n",
				cmdlist_pre_time, base_frame_time);

		cmdlist_pre_time = base_frame_time;
	}

	line_count = get_line_count(hw, cfg, base_frame_time - cmdlist_pre_time);
	/* cfg win must be in vtotal region, if partital update on, vtotal maybe very small */
	if (line_count >= hw_cfg_info->vtotal - timing->vfp - 1)
		line_count = hw_cfg_info->vtotal - timing->vfp - 2;

	*cmdlist_ln_end = line_count;

	DSI_DEBUG("base %d, pre %d, line_count %d, vtotal %d\n",
			base_frame_time,
			cmdlist_pre_time,
			line_count,
			hw_cfg_info->vtotal);
}

static bool _need_enable_frame_power_ctrl(struct dsi_tmg_cfg *cfg,
	struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	if ((hw_cfg_info->skip_num >= 1) && cfg->frame_power_en &&
			(cfg->fps_update <= cfg->frame_power_max_refresh_rate))
		return true;
	else
		return false;
}

bool need_enable_hw_ulps(struct dsi_tmg_cfg *cfg,
	struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	if ((hw_cfg_info->skip_num >= 1) && cfg->need_hw_ulps &&
			(cfg->fps_update <= cfg->hw_ulps_max_refresh_rate))
		return true;
	else
		return false;
}

static void _get_tmg_cmdlist_cfg_info(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;
	u32 ln_end_long_v1, ln_end_long_v2;
	u32 cmdlist_ln_end, cmdlist_ln_start;

	ln_end_long_v1 = hw_cfg_info->vtotal - 100;
	ln_end_long_v2 = hw_cfg_info->vtotal - timing->vfp + 6;

	/* make sure master DSI cfg win smaller then slave DSI */
	if ((cfg->sbs_en == 1) && (hw->blk_id == TMG_BLOCK_0)) {
			if (_need_enable_frame_power_ctrl(cfg, hw_cfg_info)) {
				_get_cmdlist_cfg_info_in_active(hw, cfg, hw_cfg_info,
						&cmdlist_ln_end);
				cmdlist_ln_end--;
			} else {
				cmdlist_ln_end = ((ln_end_long_v1 - 1) >= ln_end_long_v2) ?
						(ln_end_long_v1 - 1) : ln_end_long_v2;
			}

		cmdlist_ln_start = cmdlist_ln_end - 3;
	} else {
		if (_need_enable_frame_power_ctrl(cfg, hw_cfg_info) ||
				need_enable_hw_ulps(cfg, hw_cfg_info)) {
			_get_cmdlist_cfg_info_in_active(hw, cfg, hw_cfg_info,
						&cmdlist_ln_end);
		} else {
			cmdlist_ln_end = (ln_end_long_v1 >= (ln_end_long_v2 + 1)) ?
					ln_end_long_v1 : (ln_end_long_v2 + 1);
		}

		cmdlist_ln_start = cmdlist_ln_end - 5;
	}

	DSI_DEBUG("cmdlist v1 %d, v2 %d, start %d, end %d\n",
			ln_end_long_v1,
			ln_end_long_v2,
			cmdlist_ln_start,
			cmdlist_ln_end);

	/* update cmdlist cfg info */
	hw_cfg_info->cmdlist_ln_end = cmdlist_ln_end;
	hw_cfg_info->cmdlist_ln_start = cmdlist_ln_start;
}

static void _get_tmg_doze_cfg_info(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;
	u32 doze_ln_start, doze_ln_end, doze_dly_fm, doze_pre_fm;
	u32 line_count;

	line_count = get_line_count(hw, cfg, TMG_T_DOZE_END_NS);
	doze_ln_start = 2;

	if (hw_cfg_info->cmdlist_ln_start > line_count) {
		doze_ln_end = hw_cfg_info->cmdlist_ln_start - line_count;
	} else {
		DSI_ERROR("ln_start %d, line_count %d\n", hw_cfg_info->cmdlist_ln_start, line_count);
		doze_ln_end = doze_ln_start + 1;
	}
	doze_dly_fm = 1;
	doze_pre_fm = 0;

	if (timing->vfp >= timing->vact) {
		line_count = get_line_count(hw, cfg, TMG_T_DOZE_END_NS);
		doze_ln_start = timing->vsa + timing->vbp + timing->vact + 2;
		doze_ln_end = hw_cfg_info->cmdlist_ln_start - line_count;
		doze_dly_fm = 0;
		doze_pre_fm = 0;
	}

	DSI_DEBUG("doze: ln_start %d ln end %d dly_fm %d, pre_fm %d, line_count %d\n",
			doze_ln_start,
			doze_ln_end,
			doze_dly_fm,
			doze_pre_fm,
			line_count);

	/* update tmg doze cfg info */
	hw_cfg_info->doze_dly_fm = doze_dly_fm;
	hw_cfg_info->doze_pre_fm = doze_pre_fm;
	hw_cfg_info->doze_ln_start = doze_ln_start;
	hw_cfg_info->doze_ln_end = doze_ln_end;
}

static void _get_tmg_power_cfg_info(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	if (_need_enable_frame_power_ctrl(cfg, hw_cfg_info)) {
		hw_cfg_info->pwup_en = 1;
		hw_cfg_info->pwdw_en = 1;
		hw_cfg_info->pwdw_dly_fm = 1;
		hw_cfg_info->pwdw_ln_trig = 1;
		hw_cfg_info->dpu_ctrl_pwup_thre = get_line_count(hw, cfg, TMG_T_PWUP_THRE_NS);
	}

	DSI_DEBUG("pwup_en %d, pwdw_en %d pwdw_dly_fm %d pwdw_ln_trig %d dpu_ctrl_pwup_thre %d\n",
			hw_cfg_info->pwup_en,
			hw_cfg_info->pwdw_en,
			hw_cfg_info->pwdw_dly_fm,
			hw_cfg_info->pwdw_ln_trig,
			hw_cfg_info->dpu_ctrl_pwup_thre);
}

static void _get_tmg_ulps_cfg_info(struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;

	if ((hw_cfg_info->skip_num >= 1) || (timing->vfp >= timing->vact)) {
		hw_cfg_info->ulps_en = 1;
		hw_cfg_info->ulps_ln_enter = 1;
		hw_cfg_info->ulps_dly_fm = 1;
		hw_cfg_info->ulps_ln_exit = 1;
		hw_cfg_info->ulps_pre_fm = 1;
	}
}

static void _get_tmg_ddr_cfg_info(struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;

	hw_cfg_info->ddr_dvfs_ok_ln_trig = 0xFFFFF;
	/* TODO : how to cfg dvfs */
	hw_cfg_info->ddr_act_dvfs_ln_start = timing->vsa + timing->vbp + TMG_DVFS_LINE_CNT;
	hw_cfg_info->ddr_act_dvfs_ln_end = timing->vsa + timing->vbp +
			hw_cfg_info->actual_vact - TMG_DVFS_LINE_CNT;
}

static void _get_tmg_sof_cfg_info(struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;
	u32 vblk, sof_cfg_ref, sof_cfg_ln_num;

	vblk = timing->vsa + timing->vbp;

	sof_cfg_ref = (vblk >= 80) ? 80 : vblk;

	if (vblk >= 80) {
		sof_cfg_ln_num = 80;
	} else {
		sof_cfg_ref = 80 - vblk;
		sof_cfg_ln_num = ((hw_cfg_info->vtotal - hw_cfg_info->cmdlist_ln_end - 1) < sof_cfg_ref) ?
			(hw_cfg_info->vtotal - hw_cfg_info->cmdlist_ln_end - 1 + vblk) : 80;
	}

	hw_cfg_info->sof_cfg_ln_num = sof_cfg_ln_num;

	DSI_DEBUG("sof_cfg_ln_num %d\n", hw_cfg_info->sof_cfg_ln_num);
}

static void _do_tmg_hw_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg,
		struct dsi_hw_tmg_hw_cfg_info *hw_cfg_info)
{
	struct dsi_sync_timing *timing = &hw_cfg_info->timing;
	u32 value;

	DPU_REG_WRITE(hw, CMD_ULPS_EN_OFFSET, hw_cfg_info->ulps_en, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, CMD_ULPS_EN_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->ulps_ln_enter,
		ULPS_LN_ENTER_SHIFT, ULPS_LN_ENTER_LENGTH);
	value = MERGE_BITS(value, hw_cfg_info->ulps_dly_fm,
		ULPS_DLY_FM_SHIFT, ULPS_DLY_FM_LENGTH);
	DPU_REG_WRITE(hw, CMD_ULPS_EN_OFFSET, value, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, ULPS_LN_EXIT_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->ulps_ln_exit,
		ULPS_LN_EXIT_SHIFT, ULPS_LN_EXIT_LENGTH);
	value = MERGE_BITS(value, hw_cfg_info->ulps_pre_fm,
		ULPS_PRE_FM_SHIFT, ULPS_PRE_FM_LENGTH);
	DPU_REG_WRITE(hw, ULPS_LN_EXIT_OFFSET, value, cfg->cmdlist_flag);

	DPU_REG_WRITE(hw, PWUP_EN_OFFSET, hw_cfg_info->pwup_en, cfg->cmdlist_flag);
	DPU_REG_WRITE(hw, PWDW_EN_OFFSET, hw_cfg_info->pwdw_en, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, PWDW_LN_TRIG_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->pwdw_dly_fm,
		PWDW_DLY_FM_SHIFT, PWDW_DLY_FM_LENGTH);
	value = MERGE_BITS(value, hw_cfg_info->pwdw_ln_trig,
		PWDW_LN_TRIG_SHIFT, PWDW_LN_TRIG_LENGTH);
	DPU_REG_WRITE(hw, PWDW_LN_TRIG_OFFSET, value, cfg->cmdlist_flag);

	DPU_REG_WRITE(hw, DPU_CTRL_PWUP_THRE_OFFSET, hw_cfg_info->dpu_ctrl_pwup_thre,
			cfg->cmdlist_flag);

	/* PWUP_PRE_EN and PWUP_LN TRIGE keep default value */
	/* cmdlist setting */
	DPU_BIT_WRITE(hw, TMG_DSI_CMDLIST_LN_START_OFFSET, hw_cfg_info->cmdlist_ln_start,
		TMG_DSI_CMDLIST_LN_START_SHIFT,
		TMG_DSI_CMDLIST_LN_START_LENGTH, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, TMG_DSI_CMDLIST_LN_END_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->cmdlist_ln_end,
		TMG_DSI_CMDLIST_LN_END_SHIFT, TMG_DSI_CMDLIST_LN_END_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CMDLIST_LN_END_OFFSET, value, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, DOZE_LN_START_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->doze_dly_fm,
		DOZE_DLY_FM_SHIFT, DOZE_DLY_FM_LENGTH);
	value = MERGE_BITS(value, hw_cfg_info->doze_ln_start,
		DOZE_LN_START_SHIFT, DOZE_LN_START_LENGTH);
	DPU_REG_WRITE(hw, DOZE_LN_START_OFFSET, value, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, DOZE_LN_END_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->doze_pre_fm,
		DOZE_PRE_FM_SHIFT, DOZE_PRE_FM_LENGTH);
	value = MERGE_BITS(value, hw_cfg_info->doze_ln_end,
		DOZE_LN_END_SHIFT, DOZE_LN_END_LENGTH);
	DPU_REG_WRITE(hw, DOZE_LN_END_OFFSET, value, cfg->cmdlist_flag);

	value = DPU_REG_READ(hw, DDR_ACT_DVFS_LN_START_OFFSET);
	value = MERGE_BITS(value, hw_cfg_info->ddr_act_dvfs_ln_end,
		DDR_ACT_DVFS_LN_END_SHIFT, DDR_ACT_DVFS_LN_END_LENGTH);
	value = MERGE_BITS(value, hw_cfg_info->ddr_act_dvfs_ln_start,
		DDR_ACT_DVFS_LN_START_SHIFT, DDR_ACT_DVFS_LN_START_LENGTH);
		DPU_REG_WRITE(hw, DDR_ACT_DVFS_LN_START_OFFSET, value, cfg->cmdlist_flag);

	DPU_REG_WRITE(hw, TMG_DSI_DDR_DVFS_OK_LN_TRIG_OFFSET,
		hw_cfg_info->ddr_dvfs_ok_ln_trig, cfg->cmdlist_flag);

	if (timing->vbp + timing->vsa > hw_cfg_info->sof_cfg_ln_num)
		value = (TMG_SOF_CFG_SELECT_VACT_START << 16) | hw_cfg_info->sof_cfg_ln_num;
	else
		value = (TMG_SOF_CFG_SELECT_VSYNC_RISING << 16) |
				(hw_cfg_info->sof_cfg_ln_num - timing->vbp - timing->vsa);

	DPU_REG_WRITE(hw, TMG_DSI_SOF_CFG_LN_NUM_OFFSET, value, cfg->cmdlist_flag);
}

static void dsi_hw_tmg_cmdlist_cfg(struct dpu_hw_blk *hw,
		struct dsi_tmg_cfg *cfg)
{
	struct dsi_hw_tmg_hw_cfg_info hw_cfg_info = {0};

	_get_tmg_base_timing_info(hw, cfg, &hw_cfg_info);

	_get_tmg_cmdlist_cfg_info(hw, cfg, &hw_cfg_info);

	_get_tmg_doze_cfg_info(hw, cfg, &hw_cfg_info);

	_get_tmg_power_cfg_info(hw, cfg, &hw_cfg_info);

	_get_tmg_ulps_cfg_info(cfg, &hw_cfg_info);

	_get_tmg_ddr_cfg_info(cfg, &hw_cfg_info);

	_get_tmg_sof_cfg_info(cfg, &hw_cfg_info);

	_do_tmg_hw_cfg(hw, cfg, &hw_cfg_info);
}

void dsi_hw_tmg_reset(struct dpu_hw_blk *hw)
{
	DSI_DEBUG("dsi_hw_tmg_reset\n");
}

void dsi_hw_tmg_enable(struct dpu_hw_blk *hw)
{
	DPU_BIT_WRITE(hw, TMG_DSI_TMG_EN_OFFSET, TMG_ENABLE,
		TMG_DSI_TMG_EN_SHIFT, TMG_DSI_TMG_EN_LENGTH, DIRECT_WRITE);
}

void dsi_hw_tmg_disable(struct dpu_hw_blk *hw)
{
	DPU_BIT_WRITE(hw, TMG_DSI_CMDLIST_LN_END_OFFSET, TMG_DOZE_DISABLE,
		DOZE_ENTER_EN_SHIFT, DOZE_ENTER_EN_LENGTH, DIRECT_WRITE);

	DPU_BIT_WRITE(hw, TMG_DSI_TMG_EN_OFFSET, TMG_DISABLE,
		TMG_DSI_TMG_EN_SHIFT, TMG_DSI_TMG_EN_LENGTH, DIRECT_WRITE);
}

void dsi_hw_tmg_doze_enable(struct dpu_hw_blk *hw)
{
	DPU_BIT_WRITE(hw, TMG_DSI_CMDLIST_LN_END_OFFSET, TMG_DOZE_ENABLE,
		DOZE_ENTER_EN_SHIFT, DOZE_ENTER_EN_LENGTH, DIRECT_WRITE);
}

int dsi_hw_tmg_ulps_entry(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	u32 ulps_ln_enter, ulps_dly_fm, ulps_pre_fm, ulps_ln_exit;
	struct dsi_sync_timing timing = {0};
	u32 hibernate_en;
	u32 value;
	u8 flag;

	flag = cfg->cmdlist_flag;

	get_current_base_timing(&timing, cfg);

	if (cfg->partial_en)
		timing.vact = cfg->partial.height;

	if (cfg->tmg_mode == DSI_VIDEO_MODE)
	{
		if (cfg->vrr_mode == TMG_VRR_SKIP_FRAME_MODE) {
			hibernate_en = 1;
			ulps_dly_fm = 0;
			ulps_pre_fm = 0;
			ulps_ln_enter = timing.vsa + timing.vbp + 1;
			ulps_ln_exit = timing.vsa + timing.vbp +
				timing.vact + timing.vfp - 2;
		} else {
			hibernate_en = 0;
			ulps_dly_fm = 0;
			ulps_pre_fm = 0;
			ulps_ln_enter = 0;
			ulps_ln_exit = 0;
			DSI_ERROR("video vrr is nont supported ulps\n");
		}
	} else {
		hibernate_en = 0;
		ulps_pre_fm = 0;
		ulps_ln_enter = 1;
		ulps_ln_exit = timing.vsa + timing.vbp + timing.vact +
			timing.vfp - 2;
		/* TODO: other vrr mode how to cfg?/ */
		if (cfg->vrr_mode == TMG_VRR_SKIP_FRAME_MODE)
			ulps_dly_fm = 1;
		else
			ulps_dly_fm = 0;
	}

	/* hibernate en: video mode enter ulps */
	if (cfg->tmg_mode == DSI_VIDEO_MODE)
		DPU_BIT_WRITE(hw, IPI_SHUTD_EN_OFFSET, 1,
			IPI_HIBERNATE_EN_SHIFT, IPI_HIBERNATE_EN_LENGTH, flag);

	/* ulps irq : 0- off, 1-on */
	DPU_BIT_WRITE(hw, CMD_ULPS_EN_OFFSET, 0,
		CMD_ULPS_EN_SHIFT, CMD_ULPS_EN_LENGTH, flag);

	value = DPU_REG_READ(hw, ULPS_LN_ENTER_OFFSET);

	value = MERGE_BITS(value, ulps_ln_enter,
		ULPS_LN_ENTER_SHIFT, ULPS_LN_ENTER_LENGTH);
	value = MERGE_BITS(value, ulps_dly_fm,
		ULPS_DLY_FM_SHIFT, ULPS_DLY_FM_LENGTH);
	DPU_REG_WRITE(hw, ULPS_LN_ENTER_OFFSET, value, flag);

	value = DPU_REG_READ(hw, ULPS_LN_EXIT_OFFSET);

	value = MERGE_BITS(value, ulps_ln_exit,
		ULPS_LN_EXIT_SHIFT, ULPS_LN_EXIT_LENGTH);
	value = MERGE_BITS(value, ulps_pre_fm,
		ULPS_PRE_FM_SHIFT, ULPS_PRE_FM_LENGTH);
	DPU_REG_WRITE(hw, ULPS_LN_EXIT_OFFSET, value, flag);

	return 0;
}

int dsi_hw_tmg_ulps_exit(struct dpu_hw_blk *hw, u8 flag)
{
	if ((flag != 1) && (flag != 0)) {
		DSI_ERROR("cmdlist flag error, %d\n", flag);
		return -1;
	}

	/* hibernate en: video mode enter ulps */
	DPU_BIT_WRITE(hw, IPI_SHUTD_EN_OFFSET, 0,
		IPI_HIBERNATE_EN_SHIFT, IPI_HIBERNATE_EN_LENGTH, flag);

	/* ulps irq : 0- off, 1-on */
	DPU_BIT_WRITE(hw, CMD_ULPS_EN_OFFSET, 0,
		CMD_ULPS_EN_SHIFT, CMD_ULPS_EN_LENGTH, flag);

	return 0;
}

void dsi_hw_tmg_partial_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	u32 hact_fsm_par;
	u32 value;
	u8 flag;

	flag = cfg->cmdlist_flag;

	if (!cfg->partial_en) {
		DPU_REG_WRITE(hw, PARTIAL_MODE_EN_OFFSET, 0, flag);
		DPU_REG_WRITE(hw, HACT_FSM_PAR_OFFSET, 0, flag);
		return;
	}

	if (cfg->format == DSI_FMT_DSC)
		hact_fsm_par = DIV_ROUND_UP(cfg->partial.width, 6);
	else
		hact_fsm_par = cfg->partial.width;

	value = MERGE_BITS(1,  cfg->partial.width,
			HACT_PAR_SHIFT, HACT_PAR_LENGTH);
	DPU_REG_WRITE(hw, PARTIAL_MODE_EN_OFFSET, value, flag);

	value = MERGE_BITS(value, hact_fsm_par,
			HACT_FSM_PAR_SHIFT, HACT_FSM_PAR_LENGTH);
	value = MERGE_BITS(value,  cfg->partial.height,
			VACT_PAR_SHIFT, VACT_PAR_LENGTH);
	DPU_REG_WRITE(hw, HACT_FSM_PAR_OFFSET, value, flag);
}

static u64 gray_to_bcd(u64 gray)
{
	/* binary-code decimal */
	u64 bcd;
	u8 size = 63;
	u8 i = 0;

	bcd = BIT(63) & gray;

	for (i = size; i > 0; i --)
		bcd |= (((bcd >> i) & 0x01) ^ ((gray >> (i - 1)) & 0x01)) << (i - 1);

	return bcd;
}

u64 dsi_hw_tmg_timestamp_get(struct dpu_hw_blk *hw)
{
	u64 val1, val2, gray;

	val1 = DPU_REG_READ_NO_LOG(hw, TMG_DSI_SYS_TIMSTAMP_GRAY_VSYNC1_OFFSET);
	val2 = DPU_REG_READ_NO_LOG(hw, TMG_DSI_SYS_TIMSTAMP_GRAY_VSYNC0_OFFSET);

	gray = (val1 << 32) | val2;

	return gray_to_bcd(gray) * T_VSYNC_TIMESTAMP_CLK / 100;
}

void dsi_hw_tmg_obufen_config(struct dpu_hw_blk *hw, u32 obufen)
{
	u32 value;

	value = DPU_REG_READ(hw, CORE_FM_TIMING_EN_OFFSET);
	value = MERGE_BITS(value, (obufen & BIT(0)) >> 0,
			TMG_DSI_OBUF0_EN_SHIFT, TMG_DSI_OBUF0_EN_LENGTH);
	value = MERGE_BITS(value, (obufen & BIT(1)) >> 1,
			TMG_DSI_OBUF1_EN_SHIFT, TMG_DSI_OBUF1_EN_LENGTH);
	value = MERGE_BITS(value, (obufen & BIT(2)) >> 2,
			TMG_DSI_OBUF2_EN_SHIFT, TMG_DSI_OBUF2_EN_LENGTH);
	value = MERGE_BITS(value, (obufen & BIT(3)) >> 3,
			TMG_DSI_OBUF3_EN_SHIFT, TMG_DSI_OBUF3_EN_LENGTH);
	DPU_REG_WRITE(hw, CORE_FM_TIMING_EN_OFFSET, value, DIRECT_WRITE);
}

static void dsi_hw_tmg_obuffer_enable(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	u32 value;
	u8 flag;

	flag = cfg->cmdlist_flag;

	value = DPU_REG_READ(hw, TMG_DSI_SPLIT_EN_BUF_OFFSET);
	value = MERGE_BITS(value, cfg->obuf.split_en_buf,
			TMG_DSI_SPLIT_EN_BUF_SHIFT, TMG_DSI_SPLIT_EN_BUF_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_SPLIT_EN_BUF_OFFSET, value, flag);

	value = DPU_REG_READ(hw, CORE_FM_TIMING_EN_OFFSET);
	value = MERGE_BITS(value, cfg->obuf.core_fm_timing_en,
			TMG_DSI_CORE_FM_TIMING_EN_SHIFT, TMG_DSI_CORE_FM_TIMING_EN_LENGTH);
	DPU_REG_WRITE(hw, CORE_FM_TIMING_EN_OFFSET, value, flag);
}

void dsi_hw_tmg_obuffer_level_config(struct dpu_hw_blk *hw, struct obuffer_parms *obuffer)
{
	u32 value;

	/* REGISTER tmg_dsi_reg_25 */
	value = DPU_REG_READ(hw, TMG_DSI_SPLIT_EN_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_urgent_vld_en_buf,
		TMG_DSI_CORE_URGENT_VLD_EN_BUF_SHIFT,
		TMG_DSI_CORE_URGENT_VLD_EN_BUF_LENGTH);
	value = MERGE_BITS(value, obuffer->core_dfc_l_thre_buf,
		TMG_DSI_CORE_DFC_L_THRE_BUF_SHIFT,
		TMG_DSI_CORE_DFC_L_THRE_BUF_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_SPLIT_EN_BUF_OFFSET, value, DIRECT_WRITE);

	/* REGISTER tmg_dsi_reg_26 */
	value = DPU_REG_READ(hw, TMG_DSI_CORE_DFC_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_dfc_h_thre_buf,
		TMG_DSI_CORE_DFC_H_THRE_BUF_SHIFT,
		TMG_DSI_CORE_DFC_H_THRE_BUF_LENGTH);
	value = MERGE_BITS(value, obuffer->core_urgent_l_thre_buf,
		TMG_DSI_CORE_URGENT_L_THRE_BUF_SHIFT,
		TMG_DSI_CORE_URGENT_L_THRE_BUF_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CORE_DFC_H_THRE_BUF_OFFSET, value, DIRECT_WRITE);

	/* REGISTER tmg_dsi_reg_27 */
	value = DPU_REG_READ(hw, TMG_DSI_CORE_URGENT_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_urgent_h_thre_buf,
		TMG_DSI_CORE_URGENT_H_THRE_BUF_SHIFT,
		TMG_DSI_CORE_URGENT_H_THRE_BUF_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CORE_URGENT_H_THRE_BUF_OFFSET, value, DIRECT_WRITE);

	/* REGISTER tmg_dsi_reg_28 */
	value = DPU_REG_READ(hw, TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_OFFSET);
	value = MERGE_BITS(value, obuffer->core_obuf_lvl_thre_buf0,
		TMG_DSI_CORE_OBUF_LVL_THRE_BUF0_SHIFT,
		TMG_DSI_CORE_OBUF_LVL_THRE_BUF0_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CORE_DPU_DVFS_H_THRE_BUF_OFFSET, value, DIRECT_WRITE);

	/* REGISTER tmg_dsi_reg_29 */
	value = DPU_REG_READ(hw, TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_OFFSET);
	value = MERGE_BITS(value, obuffer->core_obuf_lvl_thre_buf1,
		TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_SHIFT,
		TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_LENGTH);
	value = MERGE_BITS(value, obuffer->core_obuf_lvl_thre_buf2,
		TMG_DSI_CORE_OBUF_LVL_THRE_BUF2_SHIFT,
		TMG_DSI_CORE_OBUF_LVL_THRE_BUF2_LENGTH);
	DPU_REG_WRITE(hw, TMG_DSI_CORE_OBUF_LVL_THRE_BUF1_OFFSET, value, DIRECT_WRITE);

	/* REGISTER tmg_dsi_reg_30 */
	DPU_BIT_WRITE(hw, TMG_DSI_CORE_OBUF_TH_BUF_OFFSET,
		obuffer->core_obuf_th_buf,
		TMG_DSI_CORE_OBUF_TH_BUF_SHIFT,
		TMG_DSI_CORE_OBUF_TH_BUF_LENGTH, DIRECT_WRITE);
}

void dsi_hw_tmg_init(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	DSI_DEBUG("dsi_hw_tmg_init, vg:%d\n", cfg->vg_en);

	cfg->partial_en = 0;

	if ((cfg->cmdlist_flag != CMDLIST_WRITE) &&
		(cfg->cmdlist_flag != DIRECT_WRITE)) {
		DSI_ERROR("cmdlist flag error, %d\n", cfg->cmdlist_flag);
		return;
	}

	DSI_DEBUG("fps, base:%d, update:%d\n", cfg->fps_base, cfg->fps_update);
	DSI_DEBUG("h:%d, v:%d\n", cfg->base.base0.hact, cfg->base.base0.vact);

	dsi_hw_tmg_obuffer_enable(hw, cfg);

	if (cfg->vg_en)
		dsi_hw_tmg_vg_test(hw, cfg);

	if (cfg->sbs_en)
		dsi_hw_tmg_sbs_cfg(hw, cfg);

	dsi_hw_tmg_partial_update(hw, cfg);

	if (cfg->tmg_mode == DSI_CMD_MODE)
		dsi_hw_tmg_cmd_mode_cfg(hw, cfg);

	dsi_hw_tmg_timing_update(hw, cfg);

	dsi_hw_tmg_static_cfg(hw, cfg);

	if (cfg->tmg_mode == DSI_VIDEO_MODE)
		dsi_hw_tmg_video_mode_vrr_cfg(hw, cfg);
	else if (cfg->tmg_mode == DSI_CMD_MODE)
		dsi_hw_tmg_command_mode_vrr_cfg(hw, cfg);
	else
		DSI_ERROR("Unsupported mode: %d\n", cfg->tmg_mode);

	dsi_hw_tmg_cmdlist_cfg(hw, cfg);
}

void dsi_hw_tmg_frame_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	if ((cfg->cmdlist_flag != CMDLIST_WRITE) &&
		(cfg->cmdlist_flag != DIRECT_WRITE)) {
		DSI_ERROR("cmdlist flag error, %d\n", cfg->cmdlist_flag);
		return;
	}

	dsi_hw_tmg_timing_update(hw, cfg);

	dsi_hw_tmg_partial_update(hw, cfg);

	if (cfg->tmg_mode == DSI_VIDEO_MODE)
		dsi_hw_tmg_video_mode_vrr_cfg(hw, cfg);
	else
		dsi_hw_tmg_command_mode_vrr_cfg(hw, cfg);

	dsi_hw_tmg_cmdlist_cfg(hw, cfg);

	if ((cfg->ulps_enter == 1) && (cfg->ulps_exit == 1))
		DSI_DEBUG("ULPS enter / exit conflict, enter:%d, exit:%d\n",
			cfg->ulps_enter, cfg->ulps_exit);

	if (cfg->ulps_enter)
		dsi_hw_tmg_ulps_entry(hw, cfg);

	if (cfg->ulps_exit)
		dsi_hw_tmg_ulps_exit(hw, cfg->cmdlist_flag);
}

void dsi_hw_tmg_dyn_freq_update(struct dpu_hw_blk *hw, struct dsi_tmg_cfg *cfg)
{
	u32 value;
	value = DPU_REG_READ(hw, HFP0_OFFSET);
	value = MERGE_BITS(value, cfg->base.base0.hfp, HFP0_SHIFT, HFP0_LENGTH);
	DPU_REG_WRITE(hw, HFP0_OFFSET, value, DIRECT_WRITE);
}

static void hw_tmg_partial_update_dump(struct dpu_hw_blk *hw)
{
	DPU_INFO("0x%x = (0x%x), pu_enable is 0x%x, HACT_PAR 0x%x\n",
			PARTIAL_MODE_EN_OFFSET,
			DPU_REG_READ(hw, PARTIAL_MODE_EN_OFFSET),
			DPU_BIT_READ(hw, PARTIAL_MODE_EN_OFFSET,
					PARTIAL_MODE_EN_SHIFT, PARTIAL_MODE_EN_LENGTH),
			DPU_BIT_READ(hw, PARTIAL_MODE_EN_OFFSET,
					HACT_PAR_SHIFT, HACT_PAR_LENGTH));
	DPU_INFO("0x%x = (0x%x), HACT_FSM_PAR 0x%x, VACT_PAR 0x%x\n",
			HACT_FSM_PAR_OFFSET,
			DPU_REG_READ(hw, HACT_FSM_PAR_OFFSET),
			DPU_BIT_READ(hw, HACT_FSM_PAR_OFFSET,
					HACT_FSM_PAR_SHIFT, HACT_FSM_PAR_LENGTH),
			DPU_BIT_READ(hw, HACT_FSM_PAR_OFFSET,
					VACT_PAR_SHIFT, VACT_PAR_LENGTH));
}

static void hw_tmg_cmdlist_cfg_win_dump(struct dpu_hw_blk *hw)
{
	DPU_INFO("0x%x = (0x%x), cmdlist_start 0x%x\n",
			TMG_DSI_CMDLIST_LN_START_OFFSET,
			DPU_REG_READ(hw, TMG_DSI_CMDLIST_LN_START_OFFSET),
			DPU_BIT_READ(hw, TMG_DSI_CMDLIST_LN_START_OFFSET,
					TMG_DSI_CMDLIST_LN_START_SHIFT, TMG_DSI_CMDLIST_LN_START_LENGTH));
	DPU_INFO("0x%x = (0x%x), cmdlist_end 0x%x\n",
			TMG_DSI_CMDLIST_LN_END_OFFSET,
			DPU_REG_READ(hw, TMG_DSI_CMDLIST_LN_END_OFFSET),
			DPU_BIT_READ(hw, TMG_DSI_CMDLIST_LN_END_OFFSET,
					TMG_DSI_CMDLIST_LN_END_SHIFT, TMG_DSI_CMDLIST_LN_END_LENGTH));
}

static void hw_tmg_shadow_read_en(struct dpu_hw_blk *hw, bool enable)
{
	DPU_BIT_WRITE(hw, TMG_DSI_FORCE_UPDATE_EN_OFFSET, enable ? 1 : 0,
			TMG_DSI_SHADOW_READ_EN_SHIFT,
			TMG_DSI_SHADOW_READ_EN_LENGTH,
			DIRECT_WRITE);
}

void hw_tmg_fifo_status_dump(struct dpu_hw_blk *hw)
{
	u32 value;

	value = DPU_REG_READ(hw, TMG_DSI_DEBUG_RD_OFFSET);
	DPU_INFO("0x%x = (0x%x), obuf status: afifo_full %d, empty %d\n",
			TMG_DSI_DEBUG_RD_OFFSET,
			value,
			value & BIT(2) ? 1 : 0,
			value & BIT(3) ? 1 : 0);
}

void dsi_hw_tmg_status_dump(struct dpu_hw_blk *hw)
{
	DPU_INFO("tmg[%d] status dump:\n", hw->blk_id);
	hw_tmg_fifo_status_dump(hw);

	DPU_INFO("preload regs dump:\n");
	hw_tmg_partial_update_dump(hw);
	hw_tmg_cmdlist_cfg_win_dump(hw);

	DPU_INFO("shadow regs dump:\n");
	hw_tmg_shadow_read_en(hw, true);
	hw_tmg_partial_update_dump(hw);
	hw_tmg_cmdlist_cfg_win_dump(hw);

	hw_tmg_shadow_read_en(hw, false);
}
