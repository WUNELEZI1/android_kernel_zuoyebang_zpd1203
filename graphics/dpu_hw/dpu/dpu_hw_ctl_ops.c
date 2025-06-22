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

#include "dpu_hw_ctl_ops.h"
#include "dpu_hw_ctl_top_reg.h"
#include "dpu_hw_scene_ctl_reg.h"
#include "dpu_hw_dsi.h"

#define DPU_SENSE_CTRL_IDLE_STATUS	(0x1fffffc1)

#define GET_BIT(x, bit) ((x & (1 << (bit))) >> (bit))

void dpu_hw_wb_input_position_setup(struct dpu_hw_blk *hw,
		enum dpu_wb_id wb_id, enum dpu_hw_wb_position position)
{
	u32 offset;

	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	offset = wb_id > WB0 ? WB1_SEL_ID_OFFSET : WB0_SEL_ID_OFFSET;
	DPU_REG_WRITE(hw, offset, position, DIRECT_WRITE);
}

/* dpu sence ctl */

void dpu_hw_rch_mount(struct dpu_hw_blk *hw, u32 rch_mask)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, rch_mask, NML_RCH_EN_SHIFT,
			NML_RCH_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_rch_vrt_config(struct dpu_hw_blk *hw, u32 rch_id, bool enable)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_VRT_REUSE_OFFSET, enable,
			rch_id, 1, DIRECT_WRITE);
}

void dpu_hw_wb_mount(struct dpu_hw_blk *hw, u32 wb_mask)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, wb_mask, NML_WB_EN_SHIFT,
			NML_WB_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_dsc_rdma_mount(struct dpu_hw_blk *hw, u32 mask)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, mask, NML_DSCR_EN_SHIFT,
			NML_DSCR_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_dsc_wdma_mount(struct dpu_hw_blk *hw, u32 mask)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, mask, NML_DSCW_EN_SHIFT,
			NML_DSCW_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_timing_engine_mode_set(struct dpu_hw_blk *hw,
		bool is_cmd_mode, bool auto_refresh_enable)
{
	u8 display_mode;

	if (is_cmd_mode)
		display_mode = MIPI_DSI_COMMAND_MODE;
	else
		display_mode = MIPI_DSI_VIDEO_MODE;

	DPU_BIT_WRITE(hw, VIDEO_MOD_OFFSET, display_mode,
			VIDEO_MOD_SHIFT, VIDEO_MOD_LENGTH, DIRECT_WRITE);

	DPU_BIT_WRITE(hw, NML_RCH_VRT_REUSE_OFFSET, auto_refresh_enable,
			CMD_AUTO_REFRESH_EN_SHIFT, CMD_AUTO_REFRESH_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_outctrl_mount(struct dpu_hw_blk *hw, bool enable)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, enable, NML_OUTCTL_EN_SHIFT,
			NML_OUTCTL_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_timing_engine_mount(struct dpu_hw_blk *hw, u32 tmg_mask)
{
	bool enable;

	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, tmg_mask, NML_FRM_TIMING_EN_SHIFT,
			NML_FRM_TIMING_EN_LENGTH, DIRECT_WRITE);

	enable = tmg_mask ? true : false;
	DPU_BIT_WRITE(hw, NML_RCH_EN_OFFSET, enable, NML_OUTCTL_EN_SHIFT,
			NML_OUTCTL_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_scene_cfg_ready_update(struct dpu_hw_blk *hw, bool update_en, bool tui_en)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_CMD_UPDT_EN_OFFSET, (update_en ? 1 : 0), NML_CMD_UPDT_EN_SHIFT,
			NML_CMD_UPDT_EN_LENGTH, DIRECT_WRITE);

	if (tui_en)
		DPU_BIT_WRITE(hw, BOTH_CFG_RDY_OFFSET, 1, NML_CFG_RDY_SHIFT,
				NML_CFG_RDY_LENGTH, DIRECT_WRITE);
	else
		DPU_BIT_WRITE(hw, BOTH_CFG_RDY_OFFSET, 1, BOTH_CFG_RDY_SHIFT,
				BOTH_CFG_RDY_LENGTH, DIRECT_WRITE);
}

void dpu_hw_scene_init_en(struct dpu_hw_blk *hw, bool init_en)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_BIT_WRITE(hw, NML_INIT_EN_OFFSET, (init_en ? 1 : 0), NML_INIT_EN_SHIFT,
			NML_INIT_EN_LENGTH, DIRECT_WRITE);
}

void dpu_hw_first_frame_start(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, SW_START_OFFSET, 1, DIRECT_WRITE);
}

void dpu_hw_scene_reset(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	// TODO: for continue
	// DPU_REG_WRITE(hw, REG_NML_VALUE_RST_OFFSET, REG_BANK_RESET_VAL, DIRECT_WRITE);
}

int dpu_hw_scene_ctrl_sw_clear(struct dpu_hw_blk *hw)
{
	u32 clear_status;
	u32 mount_value;
	u32 clr_st_mask;
	int ret;

	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return 0;
	}

	DPU_BIT_WRITE(hw, NML_CMD_UPDT_EN_OFFSET, 0, NML_CMD_UPDT_EN_SHIFT,
			NML_CMD_UPDT_EN_LENGTH, DIRECT_WRITE);

	mount_value = DPU_REG_READ(hw, NML_RCH_EN_OFFSET);

	DPU_REG_WRITE(hw, NML_RCH_EN_OFFSET, 0x0, DIRECT_WRITE);

	DPU_REG_WRITE(hw, BOTH_CFG_RDY_OFFSET, 0x2, DIRECT_WRITE);

	clr_st_mask = BITS_MASK(SW_CLR_ST_SHIFT, SW_CLR_ST_LENGTH);

	ret = DPU_READ_POLL_TIMEOUT(hw, BOTH_CFG_RDY_OFFSET, clear_status,
			(clear_status & clr_st_mask) == 0, 1000, 16000);
	if (ret < 0)
		DPU_ERROR("wait ctl clear timeout! value 0x%x\n", clear_status);

	DPU_REG_WRITE(hw, NML_RCH_EN_OFFSET, mount_value, DIRECT_WRITE);
	return ret;
}

static void dpu_hw_ctrl_top_parse_slice(struct dpu_hw_blk *hw)
{
	u32 value;
	int i;
	char s[128];

	DPU_DUMP_REG("ctrl_top", hw->blk_id, WB0_SLICE_CNT_OFFSET);

	value = DPU_BIT_READ(hw, WB0_SLICE_CNT_OFFSET,
			WB0_SLICE_CNT_SHIFT, WB0_SLICE_CNT_LENGTH);
	DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "wb0 complete %d stripe\n", value);

	value = DPU_BIT_READ(hw, WB0_SLICE_CNT_OFFSET,
			WB1_SLICE_CNT_SHIFT, WB1_SLICE_CNT_LENGTH);
	DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "wb1 complete %d stripe\n", value);

	value = DPU_BIT_READ(hw, WB0_SLICE_CNT_OFFSET,
			WB_BUSY_SHIFT, WB_BUSY_LENGTH);
	for (i = 0; i < WB_BUSY_LENGTH; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "wb%d is busy\n", i);
	}

	value = DPU_BIT_READ(hw, WB0_SLICE_CNT_OFFSET,
			ACAD_BUSY_SHIFT, ACAD_BUSY_LENGTH);
	for (i = 0; i < ACAD_BUSY_LENGTH; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "acad%d is busy\n", i);
	}

	value = DPU_BIT_READ(hw, WB0_SLICE_CNT_OFFSET,
			DSCW_BUSY_SHIFT, DSCW_BUSY_LENGTH);
	for (i = 0; i < DSCW_BUSY_LENGTH; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "dscw%d is busy\n", i);
	}
}

static void dpu_hw_ctrl_top_parse_int(struct dpu_hw_blk *hw)
{
	u32 value;
	int i;
	char s[128];

	DPU_DUMP_REG("ctrl_top", hw->blk_id, RCH_CONFLICT_INT_RAW_OFFSET);

	value = DPU_REG_READ(hw, RCH_CONFLICT_INT_RAW_OFFSET);
	for (i = 0; i < RCH_CONFLICT_INT_RAW_LENGTH; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "rch%d conflict\n", i);
	}

	for (i = 0; i < ACAD_TIMEOUT_INT_RAW_LENGTH; i++) {
		if (value & BIT(i + ACAD_TIMEOUT_INT_RAW_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "acad%d timeout\n", i);
	}

	for (i = 0; i < WB_TIMEOUT_INT_RAW_LENGTH; i++) {
		if (value & BIT(i + WB_TIMEOUT_INT_RAW_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "online wb%d timeout\n", i);
	}

	for (i = 0; i < CMDLIST_RDMA_CFG_TIMEOUT_INT_RAW_LENGTH; i++) {
		if (value & BIT(i + CMDLIST_RDMA_CFG_TIMEOUT_INT_RAW_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "cmdlist rdma%d cfg timeout\n", i);
	}

	for (i = 0; i < CMDLIST_PREPQ_CFG_TIMEOUT_INT_RAW_LENGTH; i++) {
		if (value & BIT(i + CMDLIST_PREPQ_CFG_TIMEOUT_INT_RAW_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "cmdlist prepq%d cfg timeout\n", i);
	}
}

static void dpu_hw_ctrl_top_parse_act(struct dpu_hw_blk *hw)
{
	u32 value;
	int i;
	char s[128];

	DPU_DUMP_REG("ctrl_top", hw->blk_id, REUSE_RDMA_ACT_OFFSET);

	value = DPU_REG_READ(hw, REUSE_RDMA_ACT_OFFSET);
	for (i = 0; i < REUSE_RDMA_ACT_LENGTH; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "rdma%d is busy\n", i);
	}

	for (i = 0; i < REUSE_PREPQ_ACT_LENGTH; i++) {
		if (value & BIT(i + REUSE_PREPQ_ACT_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "prepq%d is busy\n", i);
	}

	for (i = 0; i < REUSE_CMPS_ACT_LENGTH; i++) {
		if (value & BIT(i + REUSE_CMPS_ACT_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "cmps%d is busy\n", i);
	}

	for (i = 0; i < REUSE_WB_ACT_LENGTH; i++) {
		if (value & BIT(i + REUSE_WB_ACT_SHIFT))
			DPU_DFX_INFO("ctrl_top", hw->blk_id, s, "wb%d is busy\n", i);
	}
}

static void dpu_hw_ctrl_top_parse_cmdlist_cfg_rdy(struct dpu_hw_blk *hw)
{
	DPU_DUMP_REG("ctrl_top", hw->blk_id, CMDLIST_RDMA_CFG_RDY_OFFSET);
}

void dpu_hw_ctrl_top_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	dpu_hw_ctrl_top_parse_cmdlist_cfg_rdy(hw);
	dpu_hw_ctrl_top_parse_act(hw);
	dpu_hw_ctrl_top_parse_int(hw);
	dpu_hw_ctrl_top_parse_slice(hw);
}

void dpu_hw_ctrl_top_status_clear(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_REG_WRITE(hw, RCH_CONFLICT_INTS_OFFSET, 0x7FFFFFF, DIRECT_WRITE);
}

static void dpu_hw_scene_ctrl_parse_dbg0(struct dpu_hw_blk *hw)
{
	u32 value;
	int i;
	char s[128];

	DPU_DUMP_REG("scene_ctrl", hw->blk_id, SCENE_CTL_DBG0_OFFSET);

	value = DPU_REG_READ(hw, SCENE_CTL_DBG0_OFFSET);
	for (i = 0; i < 10; i++) {
		if (GET_BIT(value, i + 6) == 0)
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "reuse rdma%d busy\n", i);
	}

	for (i = 0; i < 10; i++) {
		if (GET_BIT(value, i + 16) == 0)
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "reuse prepq%d busy\n", i);
	}

	for (i = 0; i < 2; i++) {
		if (GET_BIT(value, i + 26) == 0)
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "reuse wb%d busy\n", i);
	}

	if (GET_BIT(value, 28) == 0)
		DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "reuse cmps busy\n");
}

static void dpu_hw_scene_ctrl_parse_dbg1(struct dpu_hw_blk *hw)
{
	u32 value;
	int i;
	char s[128];

	DPU_DUMP_REG("scene_ctrl", hw->blk_id, SCENE_CTL_DBG1_OFFSET);

	value = DPU_REG_READ(hw, SCENE_CTL_DBG1_OFFSET);
	for (i = 0; i < 10; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "rdma%d clr ack\n", i);
	}

	if (value & BIT(10))
		DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "outctl clr ack\n");

	for (i = 0; i < 2; i++) {
		if (value & BIT(i + 11))
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "dscw%d clr ack\n", i);
	}

	for (i = 0; i < 2; i++) {
		if (value & BIT(i + 13))
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "dscr%d clr ack\n", i);
	}

	for (i = 0; i < 2; i++) {
		if (value & BIT(i + 15))
			DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "wb%d clr ack\n", i);
	}

	if (value & BIT(27))
		DPU_DFX_INFO("scene_ctrl", hw->blk_id, s, "cmdlist ctl clr ack\n");
}

void dpu_hw_scene_ctrl_status_dump(struct dpu_hw_blk *hw)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return;
	}

	DPU_INFO("both_cfg_rdy 0x%x\n",
			DPU_REG_READ(hw, BOTH_CFG_RDY_OFFSET));
	dpu_hw_scene_ctrl_parse_dbg0(hw);
	dpu_hw_scene_ctrl_parse_dbg1(hw);
}

bool is_dpu_hw_sense_ctrl_idle(struct dpu_hw_blk *hw, u32 *hw_status)
{
	if (!hw) {
		DPU_ERROR("Invalid parameter\n");
		return false;
	}

	*hw_status = DPU_REG_READ(hw, SCENE_CTL_DBG0_OFFSET);

	return *hw_status == DPU_SENSE_CTRL_IDLE_STATUS;
}
