// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2023 dpu Technologies Co., Ltd.
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
#include "dsi_hw_ctrl_ops.h"
#include "dsi_hw_phy_ops.h"
#include "dsi_ctrl_reg.h"
#include "dsi_hw_ctrl.h"
#include "dsi_hw_phy.h"

#define BIG_TO_LITTLE_32(A) \
	((((u32)(A) & 0xFF000000) >> 24) | (((u32)(A) & 0x00FF0000) >> 8) | \
	(((u32)(A) & 0x0000FF00) << 8) | (((u32)(A) & 0x000000FF) << 24))

int dsi_ulps_exit(struct dsi_ctrl_hw_blk *hw);

static void dsi_ctrl_irq_cfg(struct dpu_hw_blk *hw)
{
	DPU_REG_WRITE(hw, CTRL_0X460_INT_UNMASK_PHY, 0x3001F, DIRECT_WRITE);
	DPU_REG_WRITE(hw, CTRL_0X464_INT_UNMASK_TO, 0x7F, DIRECT_WRITE);
	DPU_REG_WRITE(hw, CTRL_0x468_INT_UNMASK_ACK, 0xFFFF, DIRECT_WRITE);
	DPU_REG_WRITE(hw, CTRL_0x46C_INT_UNMASK_IPI, 0xF010F, DIRECT_WRITE);
	DPU_REG_WRITE(hw, CTRL_0x470_INT_UNMASK_RPI, 0x3, DIRECT_WRITE);
	DPU_REG_WRITE(hw, CTRL_0x474_INT_UNMASK_CRI, 0x3F007F, DIRECT_WRITE);
}

static void dsi_ctrl_debug(struct dpu_hw_blk *hw)
{
	u32 pwr_up, soft_rst, mode_status, manual_mode, phy_mode, phy_status;
	u32 main, phy, to, ack, ipi, pri, cri;
	u32 general, vid_tx, pix_pkt;

	pwr_up = DPU_REG_READ(hw, CTRL_0X00C_PWR_UP);
	soft_rst = DPU_REG_READ(hw, CTRL_0X010_SOFT_RESET);
	pix_pkt = DPU_REG_READ(hw, CTRL_0X344_PIX_PKT_CFG);
	phy_mode = DPU_REG_READ(hw, CTRL_0X100_PHY_MODE_CFG);
	phy_status = DPU_REG_READ(hw, CTRL_0X108_PHY_STATUS);
	vid_tx = DPU_REG_READ(hw, CTRL_0X20C_DSI_VID_TX_CFG);
	mode_status = DPU_REG_READ(hw, CTRL_0X01C_MODE_STATUS);
	general = DPU_REG_READ(hw, CTRL_0X200_DSI_GENERAL_CFG);
	manual_mode = DPU_REG_READ(hw, CTRL_0X024_MANUAL_MODE_CFG);

	DSI_INFO("ctrl: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		pwr_up, soft_rst, mode_status, manual_mode,
		phy_mode, phy_status, general, vid_tx, pix_pkt);

	to = DPU_REG_READ(hw, CTRL_0X424_INT_ST_TO);
	phy = DPU_REG_READ(hw, CTRL_0X420_INT_ST_PHY);
	ack = DPU_REG_READ(hw, CTRL_0x428_INT_ST_ACK);
	ipi = DPU_REG_READ(hw, CTRL_0x42C_INT_ST_IPI);
	pri = DPU_REG_READ(hw, CTRL_0x430_INT_ST_RPI);
	cri = DPU_REG_READ(hw, CTRL_0x434_INT_ST_CRI);
	main = DPU_REG_READ(hw, CTRL_0X400_INT_ST_MAIN);

	/* MAIN PHY TO ACK IPI PRI CRI */
	DSI_DEBUG("phy: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		main, phy, to, ack, ipi, pri, cri);
}

static u32 dsi_ctrl_get_clk_mode(struct dpu_hw_blk *hw)
{
	u32 val;
	val = DPU_REG_READ(hw, CTRL_0X104_PHY_CLK_CFG);

	val = (val & CTRL_0X104_PHY_CLK_CFG_CLK_TYPE_MASK) >>
		CTRL_0X104_PHY_CLK_CFG_CLK_TYPE_SHIFT;

	return val;
}

static u32 dsi_ctrl_get_lane_number(struct dpu_hw_blk *hw)
{
	u32 val;
	val = DPU_REG_READ(hw, CTRL_0X100_PHY_MODE_CFG);

	val = (val & CTRL_0X100_PHY_MODE_CFG_PHY_LANES_MASK) >>
		CTRL_0X100_PHY_MODE_CFG_PHY_LANES_SHIFT;

	return val;
}

static u32 dpu_hw_get_du_value(u32 ipi_clk, u32 hstx_clk, u32 value)
{
	u32 time;

	if (ipi_clk == 0) {
		DSI_ERROR("ipi clk cannont be 0, ipi clk: %u\n", ipi_clk);
		time = 0;
		return time;
	}

	time = (value >> 2) * ((hstx_clk * (1 << 16)) / ipi_clk);

	return time;
}

static bool dsi_hw_ctrl_wait_pri_is_avail(struct dpu_hw_blk *hw)
{
	u32 value, ulps;
	int ret;

	ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X020_CORE_STATUS, value,
			!(value & PRI_BUSY), 10, 25000);
	if (ret < 0) {
		dsi_ctrl_debug(hw);

		ulps = DPU_REG_READ(hw, CTRL_0X108_PHY_STATUS);

		DSI_ERROR("wait pri avail timeout, core 0x:%x, ulps 0x:%x\n",
			value, ulps);
		return false;
	} else {
		DSI_DEBUG("wait pri avail in %dus\n", ret);
	}

	return true;
}

static bool dsi_hw_ctrl_cri_rd_data_avail(struct dsi_ctrl_hw_blk *hw, u32 timeout_ms)
{
	u32 value, ulps;
	int ret;

	ret = DPU_READ_POLL_TIMEOUT(hw->ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			(value & CRI_RD_DATA_AVAIL), 10, timeout_ms * 1000);

	if (ret < 0) {
		dsi_ctrl_debug(hw->ctrl_blk);

		dsi_hw_sctrl_debug(hw->sctrl_blk);

		ulps = DPU_REG_READ(hw->ctrl_blk, CTRL_0X108_PHY_STATUS);

		DSI_ERROR("wait cri rd avail timeout core 0x%x, ulps 0x%x, timeout %d ms\n",
			value, ulps, timeout_ms);

		return false;
	} else {
		DPU_DEBUG("wait cri rd avail in %d us\n", ret);
	}

	return true;
}

int dsi_hw_ctrl_cri_is_busy(struct dsi_ctrl_hw_blk *hw, u32 timeout_ms)
{
	u32 value, ulps;
	int ret;

	/**
	 * Wait controller cri interface is not busy,
	 * read cri status cyscical in 25000ns
	 * 0: cri is not busy
	 * 1: cri is busy
	 */
	ret = DPU_READ_POLL_TIMEOUT(hw->ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			!(value & CRI_BUSY), 10, 25 * 1000);
	if (ret < 0) {
		ulps = DPU_REG_READ(hw->ctrl_blk, CTRL_0X108_PHY_STATUS);
		if (ulps == 0x0) {
			DSI_INFO("need exit ulps just now\n");
			dsi_ulps_exit(hw);
		}
		DSI_INFO("need retry. core 0x%x, ulps 0x%x\n", value, ulps);
	} else {
		return (value & CRI_BUSY);
	}

	ret = DPU_READ_POLL_TIMEOUT(hw->ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			!(value & CRI_BUSY), 10, timeout_ms * 1000);
	if (ret < 0) {
		dsi_ctrl_debug(hw->ctrl_blk);
		dsi_hw_sctrl_debug(hw->sctrl_blk);
		ulps = DPU_REG_READ(hw->ctrl_blk, CTRL_0X108_PHY_STATUS);
		DSI_ERROR("check cri busy timeout, core 0x%x, ulps 0x%x, timeout:%u ms\n",
			value, ulps, timeout_ms);
	}
	return (value & CRI_BUSY);
}

static void dpu_hw_dsi_ipi_cfg(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	u32 hstx_clk, ipi_clk;
	u32 hline_time;
	u32 ipi_depth, ipi_format;
	u32 value;

	hstx_clk = cfg->phy_parms.phy_hs_speed / 2;
	ipi_clk = cfg->ipi_clk;

	switch (cfg->pixel_fomat) {
	case DSI_FMT_RGB565:
		ipi_depth = 0x2;
		ipi_format = 0x0;
		break;
	case DSI_FMT_RGB666:
		ipi_depth = 0x3;
		ipi_format = 0x0;
		break;
	case DSI_FMT_RGB888:
		ipi_depth = 0x5;
		ipi_format = 0x0;
		break;
	case DSI_FMT_RGB101010:
		ipi_depth = 0x6;
		ipi_format = 0x0;
		break;
	case DSI_FMT_RGB121212:
		ipi_depth = 0x7;
		ipi_format = 0x0;
		break;
	case DSI_FMT_YUV422:
		ipi_depth = 0x7;
		ipi_format = 0x1;
		break;
	case DSI_FMT_YUV420:
		ipi_depth = 0x3;
		ipi_format = 0x3;
		break;
	case DSI_FMT_YUV422_LOOSELY:
		ipi_depth = 0x4;
		ipi_format = 0x0;
		break;
	case DSI_FMT_RGB_LOOSELY:
		ipi_depth = 0x3;
		ipi_format = 0x5;
		break;
	case DSI_FMT_DSC:
		ipi_depth = 0x5;
		ipi_format = 0xB;
		break;
	default:
		ipi_depth = 0x5;
		ipi_format = 0x0;
		break;
	}

	DSI_DEBUG("ipi_format: %d, ipi_depth: %d\n", ipi_format, ipi_depth);

	/* set ipi format */
	value = DPU_REG_READ(hw, CTRL_0X300_IPI_COLOR_MAN_CFG);

	value = MERGE_MASK_BITS(value, ipi_format,
		CTRL_0X300_IPI_COLOR_MAN_CFG_IPI_FORMAT_SHIFT,
		CTRL_0X300_IPI_COLOR_MAN_CFG_IPI_FORMAT_MASK);
	value = MERGE_MASK_BITS(value, ipi_depth,
		CTRL_0X300_IPI_COLOR_MAN_CFG_IPI_DEPTH_SHIFT,
		CTRL_0X300_IPI_COLOR_MAN_CFG_IPI_DEPTH_MASK);
	DPU_REG_WRITE(hw, CTRL_0X300_IPI_COLOR_MAN_CFG, value, DIRECT_WRITE);

	if (cfg->manual_mode_en == DSI_IPI_MANUAL_MODE) {
		value = dpu_hw_get_du_value(ipi_clk, hstx_clk,
			cfg->timing.hsa);
		DPU_REG_WRITE(hw, CTRL_0X304_IPI_VID_HSA_MAN_CFG,
			0x691110, DIRECT_WRITE);

		value = dpu_hw_get_du_value(ipi_clk, hstx_clk,
			cfg->timing.hbp);
		DPU_REG_WRITE(hw, CTRL_0X30C_IPI_VID_HBP_MAN_CFG,
			0x691110, DIRECT_WRITE);

		value = dpu_hw_get_du_value(ipi_clk, hstx_clk,
			cfg->timing.hact);
		DPU_REG_WRITE(hw, CTRL_0X314_IPI_VID_HACT_MAN_CFG,
			0x00348888, DIRECT_WRITE);

		hline_time = (cfg->timing.hsa) + (cfg->timing.hbp) +
			(cfg->timing.hfp) + (cfg->timing.hact);

		value = dpu_hw_get_du_value(ipi_clk, hstx_clk,
			hline_time);
		DPU_REG_WRITE(hw, CTRL_0X31C_IPI_VID_HLINE_MAN_CFG,
			0x0A42AA90, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X324_IPI_VID_VSA_MAN_CFG,
			cfg->timing.vsa, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X32C_IPI_VID_VBP_MAN_CFG,
			cfg->timing.vbp, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X334_IPI_VID_VACT_MAN_CFG,
			cfg->timing.vact, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X33C_IPI_VID_VFP_MAN_CFG,
			cfg->timing.vfp, DIRECT_WRITE);
	}
}

static void dpu_hw_dsi_hibernate_cfg(struct dpu_hw_blk *hw, u8 type)
{
	u32 value;

	/**
	 * hib_ulps_wakeup_time: ULPS Twakeup time >=1ms
	 * phy_lptx_clk=10M, 1 cycle=100ns, value>=10000 cycle (0x2710)
	 * phy_lptx_clk=20M, 1 cycle=50ns, value>=20000 cycle (0x4E20)
	 */
	value = DPU_REG_READ(hw, CTRL_0X348_IPI_HIBERNATE_CFG);

	value = MERGE_MASK_BITS(value, DSI_HIB_WAKE_UP_TIME,
		CTRL_0X348_IPI_HIBERNATE_CFG_HIB_ULPS_WAKEUP_TIME_SHIFT,
		CTRL_0X348_IPI_HIBERNATE_CFG_HIB_ULPS_WAKEUP_TIME_MASK);
	value = MERGE_MASK_BITS(value, type,
		CTRL_0X348_IPI_HIBERNATE_CFG_HIB_TYPE_SHIFT,
		CTRL_0X348_IPI_HIBERNATE_CFG_HIB_TYPE_MASK);
	DPU_REG_WRITE(hw, CTRL_0X348_IPI_HIBERNATE_CFG, value, DIRECT_WRITE);
}

static void __maybe_unused dsi_ctrl_tear_cfg(struct dpu_hw_blk *hw,
		struct dsi_tear_parms *cfg)
{
	u32 value;

	/**
	 * hib_ulps_wakeup_time: ULPS Twakeup time >=1ms
	 * phy_lptx_clk=10M, 1 cycle=100ns, value>=10000 cycle (0x2710)
	 * phy_lptx_clk=20M, 1 cycle=50ns, value>=20000 cycle (0x4E20)
	 */

	value = DPU_REG_READ(hw, CTRL_0X214_DSI_TEAR_EFFECT_CFG);
	/* set_tear_scanline_args */
	value = MERGE_MASK_BITS(value, cfg->tear_scanline,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_SET_TEAR_SCANLINE_ARGS_HW_SHIFT,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_SET_TEAR_SCANLINE_ARGS_HW_MASK);
	/* set_tear_on_args */
	value = MERGE_MASK_BITS(value, cfg->tear_on,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_SET_TEAR_ON_ARGS_HW_SHIFT,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_SET_TEAR_ON_ARGS_HW_MASK);
	value = MERGE_MASK_BITS(value, cfg->te_type,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_TE_TYPE_HW_SHIFT,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_TE_TYPE_HW_MASK);
	/* ipi->te.auto_tear_bta_disable: 1- disable */
	value = MERGE_MASK_BITS(value, 0x1,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_AUTO_TEAR_BTA_DISABLE_SHIFT,
		CTRL_0X214_DSI_TEAR_EFFECT_CFG_AUTO_TEAR_BTA_DISABLE_MASK);
	DPU_REG_WRITE(hw, CTRL_0X214_DSI_TEAR_EFFECT_CFG, value, DIRECT_WRITE);
}

static void dpu_hw_dsi_manual_mode_en(struct dpu_hw_blk *hw, u8 enable)
{
	if (enable == DSI_IPI_MANUAL_MODE_EN) {
		DPU_BITMASK_WRITE(hw, CTRL_0X024_MANUAL_MODE_CFG, 0,
			CTRL_0X024_MANUAL_MODE_CFG_MANUAL_MODE_EN_SHIFT,
			CTRL_0X024_MANUAL_MODE_CFG_MANUAL_MODE_EN_MASK,
			DIRECT_WRITE);
	} else if (enable == DSI_IPI_AUTO_MODE_EN) {
		DPU_BITMASK_WRITE(hw, CTRL_0X024_MANUAL_MODE_CFG, 1,
			CTRL_0X024_MANUAL_MODE_CFG_MANUAL_MODE_EN_SHIFT,
			CTRL_0X024_MANUAL_MODE_CFG_MANUAL_MODE_EN_MASK,
			DIRECT_WRITE);
	} else {
		DPU_BITMASK_WRITE(hw, CTRL_0X024_MANUAL_MODE_CFG, 1,
			CTRL_0X024_MANUAL_MODE_CFG_MANUAL_MODE_EN_SHIFT,
			CTRL_0X024_MANUAL_MODE_CFG_MANUAL_MODE_EN_MASK,
			DIRECT_WRITE);
	}
}

static int dpu_hw_dsi_ctrl_mode_get(struct dpu_hw_blk *hw, u8 mode)
{
	u32 value;
	int ret = 0;
	int time;

	time = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X01C_MODE_STATUS, value,
			(value == mode), 10, 25000);
	if (time < 0) {
		dsi_ctrl_debug(hw);
		DSI_ERROR("get dsi ctrl mode timeout!, 0x%x, 0x%x\n",
				mode, value);
		ret = -1;
	} else {
		DSI_DEBUG("get dsi ctrl mode 0x%x, value 0x%x, wait %dms\n",
				mode, value, time / 1000);
	}

	return ret;
}

static void dpu_hw_dsi_mode_ctrl(struct dpu_hw_blk *hw, enum ipi_mode_ctrl mode)
{
	/* config dsi is video mode or command mode */
	u32 value;

	value = mode << CTRL_0X018_MODE_CTRL_MODE_CTRL_SHIFT;
	DPU_REG_WRITE(hw, CTRL_0X018_MODE_CTRL, value, DIRECT_WRITE);

	dpu_hw_dsi_ctrl_mode_get(hw, mode);

	/* TODO: double check ASCI AUTO_CAL timing */
	DPU_USLEEP(DSI_MODE_CHANGE_DELAY_US);
}

static u32 dpu_hw_dsi_mode_get(struct dpu_hw_blk *hw)
{
	u32 mode;

	mode = DPU_REG_READ(hw, CTRL_0X01C_MODE_STATUS);

	mode = ((mode >> CTRL_0X01C_MODE_STATUS_MODE_STATUS_SHIFT) &
		CTRL_0X01C_MODE_STATUS_MODE_STATUS_MASK);

	return mode;
}

static void __maybe_unused dpu_hw_dsi_phy_hs_transfer_en(
		struct dpu_hw_blk *hw, u8 enable)
{
	/**
	 * hs transfer enable: 0-Disable, 1-Enable
	 * The current version does not support!!!
	 */
	DPU_BITMASK_WRITE(hw, CTRL_0X100_PHY_MODE_CFG, enable,
		CTRL_0X100_PHY_MODE_CFG_HS_TRANSFEREN_EN_SHIFT,
		CTRL_0X100_PHY_MODE_CFG_HS_TRANSFEREN_EN_MASK, DIRECT_WRITE);
}

static void dpu_hw_dsi_phy_cfg(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	struct dsi_phy_timing phy_timing;
	u32 value, transferen_en;
	u32 lp_speed, phy_wakeup_time;

	lp_speed = cfg->phy_parms.phy_lp_speed;

	phy_wakeup_time = DIV_ROUND_UP(lp_speed, 1000) + 500;

	if (cfg->phy_parms.phy_sel == XILINX_DPHY)
		transferen_en = 0;
	else /* SNPS_CDPHY */
		transferen_en = 1;

	phy_timing.wakeup_time = 0;
	phy_timing.ulps_clk_lanes = 0;
	phy_timing.ulps_data_lanes = 0;
	phy_timing.to_hstx_value = 0;
	phy_timing.to_hstx_rdy_value = 0;
	phy_timing.to_lprx_value = 0;
	phy_timing.to_lprx_rdy_value = 0;
	phy_timing.to_lprx_trig_value = 0;
	phy_timing.to_lptx_ulps_value = 0;
	phy_timing.to_bta_value = 0;

	/* set all of field value = 0xFF */
	DPU_BITMASK_WRITE(hw, CTRL_0X048_TO_HSTX_CFG,
		phy_timing.to_hstx_value,
		CTRL_0X048_TO_HSTX_CFG_TO_HSTX_VALUE_SHIFT,
		CTRL_0X048_TO_HSTX_CFG_TO_HSTX_VALUE_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, CTRL_0X04C_TO_HSTXRDY_CFG,
		/* to_hstxrdy_value */
		phy_timing.to_hstx_rdy_value,
		CTRL_0X04C_TO_HSTXRDY_CFG_TO_HSTXRDY_VALUE_SHIFT,
		CTRL_0X04C_TO_HSTXRDY_CFG_TO_HSTXRDY_VALUE_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, CTRL_0X050_TO_LPRX_CFG,
		phy_timing.to_lprx_value,
		CTRL_0X050_TO_LPRX_CFG_TO_LPRX_VAULE_SHIFT,
		CTRL_0X050_TO_LPRX_CFG_TO_LPRX_VAULE_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, CTRL_0X054_TO_LPTXRDY_CFG,
		/* to_lprxrdy_value */
		phy_timing.to_lprx_rdy_value,
		CTRL_0X054_TO_LPTXRDY_CFG_TO_LPTXRDY_VALUE_SHIFT,
		CTRL_0X054_TO_LPTXRDY_CFG_TO_LPTXRDY_VALUE_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, CTRL_0X058_TO_LPTXTRIG_CFG,
		/* to_lprxtrig_value */
		phy_timing.to_lprx_trig_value,
		CTRL_0X058_TO_LPTXTRIG_CFG_TO_LPTXTRIG_VALUE_SHIFT,
		CTRL_0X058_TO_LPTXTRIG_CFG_TO_LPTXTRIG_VALUE_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, CTRL_0X05C_TO_LPTXULPS_CFG,
		/* to_lptxulps_value */
		phy_timing.to_lptx_ulps_value,
		CTRL_0X05C_TO_LPTXULPS_CFG_TO_LPTXULPS_VALUE_SHIFT,
		CTRL_0X05C_TO_LPTXULPS_CFG_TO_LPTXULPS_VALUE_MASK, DIRECT_WRITE);

	DPU_BITMASK_WRITE(hw, CTRL_0X060_TO_BTA_CFG,
		phy_timing.to_bta_value,
		CTRL_0X060_TO_BTA_CFG_TO_BTA_VALUE_SHIFT,
		CTRL_0X060_TO_BTA_CFG_TO_BTA_VALUE_MASK, DIRECT_WRITE);

	value = DPU_REG_READ(hw, CTRL_0X100_PHY_MODE_CFG);

	value = MERGE_MASK_BITS(value, cfg->phy_parms.ppi_width,
		CTRL_0X100_PHY_MODE_CFG_PPI_WIDTH_SHIFT,
		CTRL_0X100_PHY_MODE_CFG_PPI_WIDTH_MASK);
	value = MERGE_MASK_BITS(value, cfg->phy_parms.lanes_num,
		CTRL_0X100_PHY_MODE_CFG_PHY_LANES_SHIFT,
		CTRL_0X100_PHY_MODE_CFG_PHY_LANES_MASK);
	value = MERGE_MASK_BITS(value, cfg->phy_parms.phy_type,
		CTRL_0X100_PHY_MODE_CFG_PHY_TYPE_SHIFT,
		CTRL_0X100_PHY_MODE_CFG_PHY_TYPE_MASK);
	value = MERGE_MASK_BITS(value, transferen_en,
		CTRL_0X100_PHY_MODE_CFG_HS_TRANSFEREN_EN_SHIFT,
		CTRL_0X100_PHY_MODE_CFG_HS_TRANSFEREN_EN_MASK);
	DPU_REG_WRITE(hw, CTRL_0X100_PHY_MODE_CFG, value, DIRECT_WRITE);

	value = MERGE_MASK_BITS(value, phy_wakeup_time,
		CTRL_0X1CC_PRI_ULPS_CTRL_PHY_WAKEUP_TIME_SHIFT,
		CTRL_0X1CC_PRI_ULPS_CTRL_RPHY_WAKEUP_TIME_MASK);
	value = MERGE_MASK_BITS(value, 1,
		CTRL_0X1CC_PRI_ULPS_CTRL_PHY_ULPS_CLK_LANE_SHIFT,
		CTRL_0X1CC_PRI_ULPS_CTRL_PHY_ULPS_CLK_LANE_MASK);
	value = MERGE_MASK_BITS(value, 1,
		CTRL_0X1CC_PRI_ULPS_CTRL_PHY_ULPS_DATA_LANES_SHIFT,
		CTRL_0X1CC_PRI_ULPS_CTRL_PHY_ULPS_DATA_LANES_MASK);
	DPU_REG_WRITE(hw, CTRL_0X1CC_PRI_ULPS_CTRL, value, DIRECT_WRITE);
}

static void dsi_phy_lptx_clk_cfg(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	struct dsi_phy_timing phy_timing;
	u32 value, sys_clk, lptx_clk;
	u8 div, div_reg;

	phy_timing.lp2hs_time = 0x0016FBF0;
	phy_timing.hs2lp_time = 0x0016FBF0;
	phy_timing.max_rd_time = 0xFF;
	phy_timing.esc_cmd_time = 0x0029DD2C;
	phy_timing.esc_byte_time = 0x0013B360;
	phy_timing.ipi_ratio = 0x000326B4;
	phy_timing.sys_ratio = 0x0000D221;
	phy_timing.cal_time = 0;

	if (cfg->phy_parms.phy_sel != SNPS_CDPHY_ASIC) {
		sys_clk = SYS_CLK_FPGA;
		lptx_clk = LPTX_CLK_FPGA;
	} else {
		sys_clk = SYS_CLK_ASIC;
		lptx_clk = LPTX_CLK_ASIC;
	}

	div = sys_clk / lptx_clk;
	div_reg = div/2;

	DSI_DEBUG("sys_clk:%d, lptx_clk:%d, div:%d, div_reg:%d\n",
		sys_clk, lptx_clk, div, div_reg);

	value = DPU_REG_READ(hw, CTRL_0X104_PHY_CLK_CFG);
	value = MERGE_MASK_BITS(value, div_reg,
		CTRL_0X104_PHY_CLK_CFG_PHY_LPTX_CLK_DIV_SHIFT,
		CTRL_0X104_PHY_CLK_CFG_PHY_LPTX_CLK_DIV_MASK);
	value = MERGE_MASK_BITS(value, cfg->phy_parms.clk_type,
		CTRL_0X104_PHY_CLK_CFG_CLK_TYPE_SHIFT,
		CTRL_0X104_PHY_CLK_CFG_CLK_TYPE_MASK);
	DPU_REG_WRITE(hw, CTRL_0X104_PHY_CLK_CFG, value, DIRECT_WRITE);

	if (cfg->manual_mode_en == DSI_IPI_MANUAL_MODE) {
		DPU_REG_WRITE(hw, CTRL_0X10C_PHY_LP2HS_MAN_CFG,
			phy_timing.lp2hs_time, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X11C_PHY_MAX_RD_T_MAN_CFG,
			phy_timing.max_rd_time, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X114_PHY_HS2LP_MAN_CFG,
			phy_timing.hs2lp_time, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X124_PHY_ESC_CMD_T_MAN_CFG,
			phy_timing.esc_cmd_time, DIRECT_WRITE);

		DPU_REG_WRITE(hw, CTRL_0X12C_PHY_ESC_BYTE_T_MAN_CFG,
			phy_timing.esc_byte_time, DIRECT_WRITE);

		/* TODO : bps : sys_clk 0x000326B4 phy->phy_timing.ipi_ratio */
		DPU_REG_WRITE(hw, CTRL_0X134_PHY_IPI_RATIO_MAN_CFG,
			phy_timing.ipi_ratio, DIRECT_WRITE);
		/* TODO phy_timing.sys_ratio */
		DPU_REG_WRITE(hw, CTRL_0X13C_PHY_SYS_RATIO_MAN_CFG,
			phy_timing.sys_ratio, DIRECT_WRITE);

	}

	/**
	 * 0X144 keep defalt vale
	 * DPU_REG_WRITE(hw, CTRL_0X1C8_PRI_CAL_CTRL,
	 * phy_timing.cal_time, DIRECT_WRITE);
	*/
}

static void dsi_general_cfg(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	u32 sss_behavior, max_pixel_pkt, value;

	if(cfg->phy_parms.phy_type == DSI_PHY_TYPE_CPHY)
		sss_behavior = 1;
	else
		sss_behavior = 0;

	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		max_pixel_pkt = 0;
	else
		max_pixel_pkt = cfg->timing.hact;

	if ((cfg->bta_en) && (cfg->phy_parms.phy_sel == XILINX_DPHY)) {
		DSI_ERROR("unsupported bta, phy:%d, bta:%d, disable bta!\n",
			cfg->phy_parms.phy_sel, cfg->bta_en);
		cfg->bta_en = 0;
	}

	value = DPU_REG_READ(hw, CTRL_0X200_DSI_GENERAL_CFG);
	value = MERGE_MASK_BITS(value, sss_behavior,
		CTRL_0X200_DSI_GENERAL_CFG_SSS_BEHAVIOR_SHIFT,
		CTRL_0X200_DSI_GENERAL_CFG_SSS_BEHAVIOR_MASK);
	value = MERGE_MASK_BITS(value, cfg->bta_en,
		CTRL_0X200_DSI_GENERAL_CFG_BTA_EN_SHIFT,
		CTRL_0X200_DSI_GENERAL_CFG_BTA_EN_MASK);
	value = MERGE_MASK_BITS(value, cfg->eotp_en,
		CTRL_0X200_DSI_GENERAL_CFG_EOTP_TX_EN_SHIFT,
		CTRL_0X200_DSI_GENERAL_CFG_EOTP_TX_EN_MASK);
	DPU_REG_WRITE(hw, CTRL_0X200_DSI_GENERAL_CFG, value, DIRECT_WRITE);

	/* set vertual chanel is 0 */
	DPU_REG_WRITE(hw, CTRL_0X204_DSI_VCID_CFG, 0, DIRECT_WRITE);

	/* disable scrambling */
	DPU_REG_WRITE(hw, CTRL_0X208_DSI_SCRAMBLING_CFG, 0, DIRECT_WRITE);

	if (cfg->ctrl_mode == DSI_VIDEO_MODE) {
		value = DPU_REG_READ(hw, CTRL_0X20C_DSI_VID_TX_CFG);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_LPDT_DISPLAY_CMD_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_LPDT_DISPLAY_CMD_EN_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_VFP_DISABLE_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_VFP_DISABLE_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_HFP_DISABLE_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_HFP_DISABLE_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_VBP_DISABLE_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_VBP_DISABLE_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_VSA_DISABLE_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_CMD_VSA_DISABLE_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_VFP_HS_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_VFP_HS_EN_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_VBP_HS_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_VBP_HS_EN_MASK);
		value = MERGE_MASK_BITS(value, 0,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_VSA_HS_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_VSA_HS_EN_MASK);
		value = MERGE_MASK_BITS(value, 1,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_HFP_HS_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_HFP_HS_EN_MASK);
		value = MERGE_MASK_BITS(value, 1,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_HBP_HS_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_HBP_HS_EN_MASK);
		value = MERGE_MASK_BITS(value, 1,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_HSA_HS_EN_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_BLK_HSA_HS_EN_MASK);
		value = MERGE_MASK_BITS(value, cfg->video_mode_type,
			CTRL_0X20C_DSI_VID_TX_CFG_VID_MODE_TYPE_SHIFT,
			CTRL_0X20C_DSI_VID_TX_CFG_VID_MODE_TYPE_MASK);
		DPU_REG_WRITE(hw, CTRL_0X20C_DSI_VID_TX_CFG,
			value, DIRECT_WRITE);
	}

	DPU_REG_WRITE(hw, CTRL_0X210_DSI_MAX_RPS_CFG, 0x100, DIRECT_WRITE);

	DPU_REG_WRITE(hw, CTRL_0X214_DSI_TEAR_EFFECT_CFG, 0x1, DIRECT_WRITE);

	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		dpu_hw_dsi_hibernate_cfg(hw, DSI_HIBERNATE_LP11);

	DPU_REG_WRITE(hw, CTRL_0X344_PIX_PKT_CFG, max_pixel_pkt, DIRECT_WRITE);
}

int dsi_hw_check_ulps_state(struct dsi_ctrl_hw_blk *ctrl_hw_blk, u8 enter)
{
	u32 value, clk_type, lane_num;
	struct dpu_hw_blk *hw;
	u32 state = 0;
	int ret;
	bool ulps_state_check_fail = false;

	if ((enter != DSI_ULPS_EXIT) && (enter != DSI_ULPS_ENTER))
		DSI_ERROR("please enter a valid ulps status, %d\n", enter);

	hw = ctrl_hw_blk->ctrl_blk;
	clk_type = dsi_ctrl_get_clk_mode(hw);
	lane_num = dsi_ctrl_get_lane_number(hw);

	if (enter == DSI_ULPS_ENTER) {
		state =  CTRL_0X108_PHY_STATUS_PHY_L0_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			!(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane0 not enter ulps status: 0x%08X\n", value);
		}

		state =  CTRL_0X108_PHY_STATUS_PHY_L1_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			!(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane1 not enter ulps status: 0x%08X\n", value);
		}

		state =  CTRL_0X108_PHY_STATUS_PHY_L2_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			!(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane2 not enter ulps status: 0x%08X\n", value);
		}

		state =  CTRL_0X108_PHY_STATUS_PHY_L3_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			!(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane3 not enter ulps status: 0x%08X\n", value);
		}

		if (clk_type == DSI_PHY_NON_CONTINUOUS_CLK) {
			state = CTRL_0X108_PHY_STATUS_PHY_CLK_ULPSACTIVENOT_MASK;
			ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS,
				value, !(value & state), 10, 10000);
			if (ret < 0) {
				ulps_state_check_fail = true;
				DSI_INFO("clk not enter ulps status: 0x%08X\n", value);
			}
		}
	}

	if (enter == DSI_ULPS_EXIT) {
		state =  CTRL_0X108_PHY_STATUS_PHY_L0_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane0 not exit ulps status: 0x%08X\n", value);
		}

		state =  CTRL_0X108_PHY_STATUS_PHY_L1_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane1 not exit ulps status: 0x%08X\n", value);
		}

		state =  CTRL_0X108_PHY_STATUS_PHY_L2_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane2 not exit ulps status: 0x%08X\n", value);
		}

		state =  CTRL_0X108_PHY_STATUS_PHY_L3_ULPSACTIVENOT_MASK;
		ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS, value,
			(value & state), 10, 10000);
		if (ret < 0) {
			ulps_state_check_fail = true;
			DSI_INFO("lane3 not exit ulps status: 0x%08X\n", value);
		}

		if (clk_type == DSI_PHY_NON_CONTINUOUS_CLK) {
			state = CTRL_0X108_PHY_STATUS_PHY_CLK_ULPSACTIVENOT_MASK;
			ret = DPU_READ_POLL_TIMEOUT(hw, CTRL_0X108_PHY_STATUS,
				value, (value & state), 10, 10000);
			if (ret < 0) {
				ulps_state_check_fail = true;
				DSI_INFO("clk not exit ulps status: 0x%08X\n", value);
			}
		}
	}

	if (ulps_state_check_fail) {
		dsi_ctrl_debug(hw);
		return -EBUSY;
	}

	return 0;
}

static void __maybe_unused dpu_hw_dsi_get_phy_state(struct dpu_hw_blk *hw,
		struct dsi_ctrl_phy *phy)
{
	u32 value;
	struct dsi_phy_state phy_state;
	value = DPU_REG_READ(hw, CTRL_0X108_PHY_STATUS);
	/* phy_l3_ulpsactivenot */
	phy_state.phy_l3_ulps_active_not =
		(value & CTRL_0X108_PHY_STATUS_PHY_L3_ULPSACTIVENOT_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L3_ULPSACTIVENOT_SHIFT;
	/* phy_l2_ulpsactivenot */
	phy_state.phy_l2_ulps_active_not =
		(value & CTRL_0X108_PHY_STATUS_PHY_L2_ULPSACTIVENOT_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L2_ULPSACTIVENOT_SHIFT;
	/* phy_l1_ulpsactivenot */
	phy_state.phy_l1_ulps_active_not =
		(value & CTRL_0X108_PHY_STATUS_PHY_L1_ULPSACTIVENOT_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L1_ULPSACTIVENOT_SHIFT;
	/* phy_l0_ulpsactivenot */
	phy_state.phy_l0_ulps_active_not =
		(value & CTRL_0X108_PHY_STATUS_PHY_L0_ULPSACTIVENOT_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L0_ULPSACTIVENOT_SHIFT;
	/* phy_clk_ulpsactivenot */
	phy_state.phy_clk_ulps_active_not =
		(value & CTRL_0X108_PHY_STATUS_PHY_CLK_ULPSACTIVENOT_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_CLK_ULPSACTIVENOT_SHIFT;
	/* phy_l3_stopstate */
	phy_state.phy_l3_stop_state =
		(value & CTRL_0X108_PHY_STATUS_PHY_L3_STOPSTATE_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L3_STOPSTATE_SHIFT;
	/* phy_l2_stopstate */
	phy_state.phy_l2_stop_state =
		(value & CTRL_0X108_PHY_STATUS_PHY_L2_STOPSTATE_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L2_STOPSTATE_SHIFT;
	/* phy_l1_stopstate */
	phy_state.phy_l1_stop_state =
		(value & CTRL_0X108_PHY_STATUS_PHY_L1_STOPSTATE_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L1_STOPSTATE_SHIFT;
	/* phy_l0_stopstate */
	phy_state.phy_l0_stop_state =
		(value & CTRL_0X108_PHY_STATUS_PHY_L0_STOPSTATE_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_L0_STOPSTATE_SHIFT;
	/* phy_clk_stopstate */
	phy_state.phy_clk_stop_state =
		(value & CTRL_0X108_PHY_STATUS_PHY_CLK_STOPSTATE_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_CLK_STOPSTATE_SHIFT;
	phy_state.phy_direction =
		(value & CTRL_0X108_PHY_STATUS_PHY_DIRECTION_MASK) >>
		CTRL_0X108_PHY_STATUS_PHY_DIRECTION_SHIFT;

	DSI_DEBUG("lane3 ulps state:0x%08X\n", phy_state.phy_l3_ulps_active_not);
	DSI_DEBUG("lane2 ulps state:0x%08X\n", phy_state.phy_l2_ulps_active_not);
	DSI_DEBUG("lane1 ulps state:0x%08X\n", phy_state.phy_l1_ulps_active_not);
	DSI_DEBUG("lane0 ulps state:0x%08X\n", phy_state.phy_l0_ulps_active_not);
	DSI_DEBUG("clk ulps state: 0x%08X\n", phy_state.phy_clk_ulps_active_not);
	DSI_DEBUG("phy_l3_stop_state: 0x%08X\n", phy_state.phy_l3_stop_state);
	DSI_DEBUG("phy_l2_stop_state: 0x%08X\n", phy_state.phy_l2_stop_state);
	DSI_DEBUG("phy_l1_stop_state: 0x%08X\n", phy_state.phy_l1_stop_state);
	DSI_DEBUG("phy_l0_stop_state: 0x%08X\n", phy_state.phy_l0_stop_state);
	DSI_DEBUG("phy_clk_stop_state: 0x%08X\n", phy_state.phy_clk_stop_state);
	DSI_DEBUG("phy_direction: 0x%08X\n", phy_state.phy_direction);
}

static u32 dpu_hw_dsi_ctrl_wr_fifo_full(struct dpu_hw_blk *hw)
{
	u32 value;

	value = DPU_REG_READ(hw, CTRL_0X020_CORE_STATUS);

	value = (value & CTRL_0X020_CORE_STATUS_CRI_WR_FIFOS_NOT_EMPTY_MASK) >>
		CTRL_0X020_CORE_STATUS_CRI_WR_FIFOS_NOT_EMPTY_SHIFT;

	return value;
}

static void dpu_hw_dsi_ctrl_info_get(struct dpu_hw_blk *hw)
{
	u32 version, core_id;

	version = DPU_REG_READ(hw, CTRL_0X004_VERSION);
	core_id = DPU_REG_READ(hw, CTRL_0X000_CORE_ID);

	DSI_DEBUG("MIPI DSI version is: 0x%08X, core id is: 0x%08X\n",
		version, core_id);
}

static void dsi_ctrl_power_up(struct dpu_hw_blk *hw, u8 pwr_up)
{
	DPU_BITMASK_WRITE(hw, CTRL_0X00C_PWR_UP, pwr_up,
		CTRL_0X00C_PWR_UP_PWR_UP_SHIFT,
		CTRL_0X00C_PWR_UP_PWR_UP_MASK, DIRECT_WRITE);
}

static void dsi_hw_ctrl_sw_reset(struct dpu_hw_blk *hw, int reset)
{
	u32 value;

	value = DPU_REG_READ(hw, CTRL_0X010_SOFT_RESET);

	value = MERGE_MASK_BITS(value, reset,
		CTRL_0X010_SOFT_RESET_SYS_RSTN_SHIFT,
		CTRL_0X010_SOFT_RESET_SYS_RSTN_MASK);
	value = MERGE_MASK_BITS(value, reset,
		CTRL_0X010_SOFT_RESET_PHY_RSTN_SHIFT,
		CTRL_0X010_SOFT_RESET_PHY_RSTN_MASK);
	value = MERGE_MASK_BITS(value, reset,
		CTRL_0X010_SOFT_RESET_IPI_RSTN_SHIFT,
		CTRL_0X010_SOFT_RESET_IPI_RSTN_MASK);
	DPU_REG_WRITE(hw, CTRL_0X010_SOFT_RESET, value, DIRECT_WRITE);
}

static void __maybe_unused dpu_hw_dsi_phy_alternatecal(struct dpu_hw_blk *hw)
{
	u32 value;

	value = 1 << CTRL_0X1C0_PRI_TX_CMD_PHY_ALTERNATECAL_SHIFT;

	DPU_REG_WRITE(hw, CTRL_0X1C0_PRI_TX_CMD, value, DIRECT_WRITE);
}

static void dpu_hw_dsi_phy_deskewcal(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	u32 value, hs_ui, sys_clk_ui, sys_clk_cycles;
	u32 t_skewcal_min, t_skewcal_max, t_skewcal;
	int ret;

	/* dsi phy deskew function enable: phy hs speed >= 1.5Gbps */
	if (cfg->phy_parms.phy_hs_speed < DESKEW_RATE_1G5) {
		DSI_DEBUG("lane rate(%d) < 1.5G\n", cfg->phy_parms.phy_hs_speed);
		return;
	} else {
		DSI_INFO("deskew enable\n");
	}

	/* only dphy support phy deskew */
	if (cfg->phy_parms.phy_type == DSI_PHY_TYPE_CPHY) {
		DSI_INFO("cphy no deskew required, phy(:%d)\n",
			cfg->phy_parms.phy_type);
		return;
	}

	hs_ui = DIV_ROUND_UP(1000000000, (cfg->phy_parms.phy_hs_speed / 1000));
	sys_clk_ui = DIV_ROUND_UP(1000000000, (cfg->phy_parms.sys_clk / 1000));

	/* T_skewcal Min: 2^15 hs_speed's ui, T_skewcal Max: 100us */
	t_skewcal_min = DIV_ROUND_UP(hs_ui * POWER_2_15, 1000);
	t_skewcal_max = DSI_DESKEW_MAX_100US;
	t_skewcal = t_skewcal_min + (t_skewcal_max - t_skewcal_min) / 2;

	sys_clk_cycles = DIV_ROUND_UP(t_skewcal * 1000, sys_clk_ui);

	DSI_DEBUG("hs_ui %d, skewcal_min %d, skewcal %d, cycles %d\n",
		hs_ui, t_skewcal_min, t_skewcal, sys_clk_cycles);

	ret = dsi_hw_ctrl_wait_pri_is_avail(hw);

	if (!ret) {
		DSI_ERROR("dsi phy deskew wait pri vaild timeout\n");
		return;
	}

	DPU_REG_WRITE(hw, CTRL_0X1C8_PRI_CAL_CTRL, sys_clk_cycles, DIRECT_WRITE);

	value = 1 << CTRL_0X1C0_PRI_TX_CMD_PHY_DESCKEWCAL_SHIFT;

	DPU_REG_WRITE(hw, CTRL_0X1C0_PRI_TX_CMD, value, DIRECT_WRITE);

	ret = dsi_hw_ctrl_wait_pri_is_avail(hw);
	if (!ret) {
		DSI_WARN("dsi phy deskew failed, ret %d\n", ret);
	} else {
		DSI_INFO("dsi phy deskew success\n");
	}
}

static void dsi_ctrl_clk_rate_get(struct dpu_hw_blk *hw)
{
	u32 lp2hx, hs2lp, max_rd_t, esc_cmd_t, esc_byte_t, ipi_ratio, sys_ratio;

	lp2hx = DPU_REG_READ(hw, CTRL_0X110_PHY_LP2HX_AUTO);
	hs2lp = DPU_REG_READ(hw, CTRL_0X118_PHY_HS2LP_AUTO);
	max_rd_t = DPU_REG_READ(hw, CTRL_0X120_PHY_MAX_RD_T_AUTO);
	esc_cmd_t = DPU_REG_READ(hw, CTRL_0X128_PHY_ESC_CMD_T_AUTO);
	esc_byte_t = DPU_REG_READ(hw, CTRL_0X130_PHY_ESC_BYTE_T_AUTO);
	ipi_ratio = DPU_REG_READ(hw, CTRL_0X138_PHY_IPI_RATIO_AUTO);
	sys_ratio = DPU_REG_READ(hw, CTRL_0X140_PHY_SYS_RATIO_AUTO);

	DSI_DEBUG("LP2HX:%d, HS2LP:%d, MAX_RD_T:%d, ESC_CMD_T:%d, "
		"ESC_BYTE_T:%d, IPI_RATIO:%d, SYS_RATIO:%d\n",
		lp2hx, hs2lp, max_rd_t, esc_cmd_t,
		esc_byte_t, ipi_ratio, sys_ratio);
}

int dsi_hw_wait_ready(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	enum ipi_mode_ctrl mode;
	int ret = 0;

	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		mode = IPI_VIDEO_MODE;
	else
		mode = IPI_DATA_STREAM_MODE;

	//dsi_ctrl_power_up(hw, 1);

	/* dsi phy de-skew calculate */
	dpu_hw_dsi_phy_deskewcal(hw->ctrl_blk, cfg);

	if (cfg->auto_calc_en) {
		DSI_DEBUG("auto cal mode Enable!\n");
		dpu_hw_dsi_mode_ctrl(hw->ctrl_blk, IPI_AUTO_CAL_MODE);

		/* check dsi is in auto-calculate mode */
		ret = dpu_hw_dsi_ctrl_mode_get(hw->ctrl_blk, IPI_IDLE_MODE);
		if (ret)
			DSI_ERROR("wait auto cal timeout!\n");
	}

	dsi_ctrl_clk_rate_get(hw->ctrl_blk);

	/* set vide mode or command mode */
	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		dpu_hw_dsi_mode_ctrl(hw->ctrl_blk, IPI_COMMAND_MODE);
	else
		dpu_hw_dsi_mode_ctrl(hw->ctrl_blk, IPI_DATA_STREAM_MODE);

	return ret;
}

void dpu_dsi_ctrl_hw_gen_tx_hdr(struct dpu_hw_blk *hw,
		struct cri_tx_hdr *hdr)
{
	u32 value = 0;

	dpu_hw_dsi_ctrl_wr_fifo_full(hw);

	/*
	 * Note:
	 * Use only the data types described in MIPI DSI-2 Specification.
	 * Do not use the CMD_HDR_LONG and CMD_HDR_RD filed of the CRI_TX_HDR
	 * register. Base on above reasons:
	 * cmd_hdr_rd must set to 0
	 * cmd_hdr_long must set to 0
	 */
	hdr->cmd_hdr_rd = 0;
	hdr->cmd_hdr_long = 0;

	value = MERGE_MASK_BITS(value, hdr->cmd_hdr_long,
		CTRL_0X2C0_CRI_TX_HDR_CMD_HDR_LONG_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_CMD_HDR_LONG_MASK);
	value = MERGE_MASK_BITS(value, hdr->cmd_hdr_rd,
		CTRL_0X2C0_CRI_TX_HDR_CMD_HDR_RD_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_CMD_HDR_RD_MASK);
	value = MERGE_MASK_BITS(value, hdr->cmd_tx_mode,
		CTRL_0X2C0_CRI_TX_HDR_CMD_TX_MODE_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_CMD_TX_MODE_MASK);
	value = MERGE_MASK_BITS(value, hdr->wc_msb,
		CTRL_0X2C0_CRI_TX_HDR_WC_MSB_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_WC_MSB_MASK);
	value = MERGE_MASK_BITS(value, hdr->wc_lsb,
		CTRL_0X2C0_CRI_TX_HDR_WC_LSB_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_WC_LSB_MASK);
	value = MERGE_MASK_BITS(value, hdr->virtual_channel,
		CTRL_0X2C0_CRI_TX_HDR_VIRTUAL_CHANNEL_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_VIRTUAL_CHANNEL_MASK);
	value = MERGE_MASK_BITS(value, hdr->data_type,
		CTRL_0X2C0_CRI_TX_HDR_DATA_TYPE_SHIFT,
		CTRL_0X2C0_CRI_TX_HDR_DATA_TYPE_MASK);

	DPU_REG_WRITE(hw, CTRL_0X2C0_CRI_TX_HDR, value, DIRECT_WRITE);
}

void dpu_dsi_ctrl_hw_gen_tx_pld(struct dpu_hw_blk *hw, u32 payload)
{
	DPU_REG_WRITE(hw, CTRL_0X2C4_CRI_TX_PLD, payload, DIRECT_WRITE);
}

u32 dpu_dsi_ctrl_hw_get_rx_hdr(struct dpu_hw_blk *hw,
		struct cri_rx_hdr *hdr)
{
	u32 value;

	value = DPU_REG_READ(hw, CTRL_0X2C8_CRI_RX_HDR);

	hdr->wc_msb = (value & CTRL_0X2C8_CRI_RX_HDR_WC_MSB_MASK) >>
		CTRL_0X2C8_CRI_RX_HDR_WC_MSB_SHIFT;

	hdr->wc_lsb = (value & CTRL_0X2C8_CRI_RX_HDR_WC_LSB_MASK) >>
		CTRL_0X2C8_CRI_RX_HDR_WC_LSB_SHIFT;

	hdr->virtual_channel =
		(value >> 6) & CTRL_0X2C8_CRI_RX_HDR_VIRTUAL_CHANNEL_MASK;
	hdr->data_type = (value) & CTRL_0X2C8_CRI_RX_HDR_DATA_TYPE_MASK;

	return value;
}

u32 dpu_dsi_ctrl_hw_get_rx_pld(struct dpu_hw_blk *hw)
{
	u32 payload;

	payload = DPU_REG_READ(hw, CTRL_0X2CC_CRI_RX_PLD);

	return payload;
}

static bool __maybe_unused dsi_packet_is_short(u8 type)
{
	switch (type) {
	case MIPI_DSI_V_SYNC_START:
	case MIPI_DSI_V_SYNC_END:
	case MIPI_DSI_H_SYNC_START:
	case MIPI_DSI_H_SYNC_END:
	case MIPI_DSI_COMPRESSION_MODE:
	case MIPI_DSI_END_OF_TRANSMISSION:
	case MIPI_DSI_COLOR_MODE_OFF:
	case MIPI_DSI_COLOR_MODE_ON:
	case MIPI_DSI_SHUTDOWN_PERIPHERAL:
	case MIPI_DSI_TURN_ON_PERIPHERAL:
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM:
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_DCS_READ:
	case MIPI_DSI_EXECUTE_QUEUE:
	case MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE:
		return true;
	}

	return false;
}

static bool __maybe_unused dsi_packet_is_long(u8 type)
{
	switch (type) {
	case MIPI_DSI_NULL_PACKET:
	case MIPI_DSI_BLANKING_PACKET:
	case MIPI_DSI_GENERIC_LONG_WRITE:
	case MIPI_DSI_DCS_LONG_WRITE:
	case MIPI_DSI_PICTURE_PARAMETER_SET:
	case MIPI_DSI_COMPRESSED_PIXEL_STREAM:
	case MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20:
	case MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24:
	case MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16:
	case MIPI_DSI_PACKED_PIXEL_STREAM_30:
	case MIPI_DSI_PACKED_PIXEL_STREAM_36:
	case MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12:
	case MIPI_DSI_PACKED_PIXEL_STREAM_16:
	case MIPI_DSI_PACKED_PIXEL_STREAM_18:
	case MIPI_DSI_PIXEL_STREAM_3BYTE_18:
	case MIPI_DSI_PACKED_PIXEL_STREAM_24:
	case MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE:
		return true;
	}

	return false;
}

int dsi_hw_ctrl_tx_ctrl(struct dsi_ctrl_hw_blk *hw, u8 cri_mode, u8 cri_hold)
{
	struct dpu_hw_blk *ctrl_blk;
	u32 value = 0;

	ctrl_blk = hw->ctrl_blk;

	value = MERGE_MASK_BITS(value, cri_hold, CTRL_0X2D0_RI_TX_CTRL_CRI_HOLD_SHIFT, CTRL_0X2D0_RI_TX_CTRL_CRI_HOLD_MASK);
	value = MERGE_MASK_BITS(value, cri_mode, CTRL_0X2D0_RI_TX_CTRL_CRI_MODE_SHIFT, CTRL_0X2D0_RI_TX_CTRL_CRI_MODE_MASK);
	DPU_REG_WRITE(ctrl_blk, CTRL_0X2D0_RI_TX_CTRL, value, DIRECT_WRITE);
	DSI_DEBUG("HZF 0x%x:0x%x\n", CTRL_0X2D0_RI_TX_CTRL, value);

	return 0;
}

int dsi_hw_ctrl_send_pkt(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg)
{
	struct dpu_hw_blk *ctrl_blk;
	struct cri_tx_hdr hdr = {0};
	int pld_data_bytes;
	int len, ret = 0;
	const u8 *tx;
	u32 word;

	ctrl_blk = hw->ctrl_blk;
	pld_data_bytes = sizeof(u32);

	/* do some minimum sanity checking */
	if (!dsi_packet_is_long(msg->type) && !dsi_packet_is_short(msg->type)) {
		DSI_ERROR("Please input correct type!, dt:%d\n", msg->type);
		return -1;
	}
	/* do virtual channel checking */
	if (msg->channel > 3) {
		DSI_ERROR("Please input correct vc!, vc:%d\n", msg->channel);
		return -1;
	}

	tx = msg->tx_buf;
	len = msg->tx_len;

	if (len > CRI_PLD_FIFO_MAX) {
		DSI_ERROR("Params length too large!, len:%zu\n", msg->tx_len);
		ret = -1;
	}

	/* Write CRI TX Payload */
	if (dsi_packet_is_long(msg->type)) {
		while (len) {
			if (len < pld_data_bytes) {
				word = 0;
				memcpy(&word, tx, len);
				dpu_dsi_ctrl_hw_gen_tx_pld(ctrl_blk, word);
				len = 0;
			} else {
				memcpy(&word, tx, pld_data_bytes);
				dpu_dsi_ctrl_hw_gen_tx_pld(ctrl_blk, word);
				tx += pld_data_bytes;
				len -= pld_data_bytes;
			}
		}
	}

	hdr.virtual_channel = msg->channel;
	hdr.data_type = msg->type;

	if (dsi_packet_is_long(msg->type)) {
		hdr.wc_msb = (msg->tx_len >> 8) & 0xff;
		hdr.wc_lsb = (msg->tx_len >> 0) & 0xff;
	} else {
		hdr.wc_msb = (msg->tx_len > 1) ? tx[1] : 0;
		hdr.wc_lsb = (msg->tx_len > 0) ? tx[0] : 0;
	}

	if (msg->flags & MIPI_DSI_MSG_USE_LPM)
		hdr.cmd_tx_mode = CRI_SEND_BY_LP;
	else
		hdr.cmd_tx_mode = CRI_SEND_BY_HS;

	dpu_dsi_ctrl_hw_gen_tx_hdr(ctrl_blk, &hdr);

	return ret;
}

int dsi_hw_ctrl_send_pkt_by_cmdlist(struct dsi_ctrl_hw_blk *hw,
		struct dsi_msg *msg)
{
	struct dpu_hw_blk *ctrl_blk;
	struct cri_tx_hdr hdr;
	int pld_data_bytes;
	u32 word, header;
	const u8 *tx;
	int len;

	word = 0;
	header = 0;

	if (hw == NULL) {
		DSI_ERROR("hw is null! \n");
		return -1;
	}

	if (hw->dctrl_blk == NULL) {
		DSI_ERROR("hw->dctrl_blk is null! \n");
		return -1;
	}

	ctrl_blk = hw->dctrl_blk;

	/* do some minimum sanity checking */
	if (!dsi_packet_is_long(msg->type) && !dsi_packet_is_short(msg->type)) {
		DSI_ERROR("Please input correct type!, dt:%d\n", msg->type);
		return -1;
	}

	/* do virtual channel checking */
	if (msg->channel > 3) {
		DSI_ERROR("Please input correct vc!, vc:%d\n", msg->channel);
		return -1;
	}

	pld_data_bytes = sizeof(u32);
	tx = msg->tx_buf;
	len = msg->tx_len;

	if (len > CRI_PLD_FIFO_MAX) {
		DSI_ERROR("Params length too large!, len:%zu\n", msg->tx_len);
		return -1;
	}

	DPU_REPEAT_WRITE(ctrl_blk, CTRL_0X2D0_RI_TX_CTRL, 0x1, CMDLIST_WRITE);

	if (dsi_packet_is_long(msg->type)) {
		while (len) {
			if (len < pld_data_bytes) {
				word = 0;
				memcpy(&word, tx, len);
				DPU_REPEAT_WRITE(ctrl_blk, CTRL_0X2C4_CRI_TX_PLD,
					word, CMDLIST_WRITE);
				len = 0;
			} else {
				memcpy(&word, tx, pld_data_bytes);
				DPU_REPEAT_WRITE(ctrl_blk, CTRL_0X2C4_CRI_TX_PLD,
					word, CMDLIST_WRITE);
				tx += pld_data_bytes;
				len -= pld_data_bytes;
			}
		}
	}

	hdr.virtual_channel = msg->channel;
	hdr.data_type = msg->type;
	hdr.cmd_hdr_rd = 0;
	hdr.cmd_hdr_long = 0;

	if (dsi_packet_is_long(msg->type)) {
		hdr.wc_msb = (msg->tx_len >> 8) & 0xff;
		hdr.wc_lsb = (msg->tx_len >> 0) & 0xff;
	} else {
		hdr.wc_msb = (msg->tx_len > 1) ? tx[1] : 0;
		hdr.wc_lsb = (msg->tx_len > 0) ? tx[0] : 0;
	}

	if (msg->flags & MIPI_DSI_MSG_USE_LPM)
		hdr.cmd_tx_mode = CRI_SEND_BY_LP;
	else
		hdr.cmd_tx_mode = CRI_SEND_BY_HS;

	header = (hdr.cmd_hdr_long <<
		CTRL_0X2C0_CRI_TX_HDR_CMD_HDR_LONG_SHIFT) |
		(hdr.cmd_hdr_rd << CTRL_0X2C0_CRI_TX_HDR_CMD_HDR_RD_SHIFT) |
		(hdr.cmd_tx_mode << CTRL_0X2C0_CRI_TX_HDR_CMD_TX_MODE_SHIFT) |
		(hdr.wc_msb << CTRL_0X2C0_CRI_TX_HDR_WC_MSB_SHIFT) |
		(hdr.wc_lsb << CTRL_0X2C0_CRI_TX_HDR_WC_LSB_SHIFT) |
		(hdr.virtual_channel <<
		CTRL_0X2C0_CRI_TX_HDR_VIRTUAL_CHANNEL_SHIFT) |
		(hdr.data_type << CTRL_0X2C0_CRI_TX_HDR_DATA_TYPE_SHIFT);

	DPU_REPEAT_WRITE(ctrl_blk, CTRL_0X2C0_CRI_TX_HDR, header, CMDLIST_WRITE);

	if (msg->flags & MIPI_DSI_MSG_BATCH_COMMAND)
		DPU_REPEAT_WRITE(ctrl_blk, CTRL_0X2D0_RI_TX_CTRL,
			0x1, CMDLIST_WRITE);
	else
		DPU_REPEAT_WRITE(ctrl_blk, CTRL_0X2D0_RI_TX_CTRL,
			0x0, CMDLIST_WRITE);

	return 0;
}

int dsi_hw_ctrl_cmdlist_send(struct dsi_ctrl_hw_blk *hw, struct dsi_cmds *cmds)
{
	struct dsi_msg *msg;
	int hdr_count = 0;
	int pld_count = 0;
	int i = 0;
	int size;
	int ret = 0;

	size = cmds->count;

	DPU_REPEAT_WRITE(hw->dctrl_blk, CTRL_0X2D0_RI_TX_CTRL,
		0x1, CMDLIST_WRITE);

	for (i = 0; i < cmds->count; i++) {
		msg = cmds[i].msg;
		ret = dsi_hw_ctrl_send_pkt_by_cmdlist(hw, msg);
		hdr_count = 0;
		pld_count = 0;
	}

	DPU_REPEAT_WRITE(hw->dctrl_blk, CTRL_0X2D0_RI_TX_CTRL,
		0x0, CMDLIST_WRITE);

	return ret;
}

int dsi_hw_ctrl_read_pkt_pre_tx(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg)
{
	struct cri_tx_hdr tx_hdr = {0};
	int tx_len;
	const u8 *tx;

	/* do some minimum sanity checking */
	if (!dsi_packet_is_long(msg->type) && !dsi_packet_is_short(msg->type)) {
		DSI_ERROR("Please input correct type!, dt:%d\n", msg->type);
		return -1;
	}
	/* do virtual channel checking */
	if (msg->channel > 3) {
		DSI_ERROR("Please input correct vc!, vc:%d\n", msg->channel);
		return -1;
	}

	tx = msg->tx_buf;
	tx_len = msg->tx_len;
	tx_hdr.virtual_channel = msg->channel;
	if (msg->flags & MIPI_DSI_MSG_USE_LPM)
		tx_hdr.cmd_tx_mode = CRI_SEND_BY_LP;
	else
		tx_hdr.cmd_tx_mode = CRI_SEND_BY_HS;

	tx_hdr.data_type = MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE;
	tx_hdr.wc_msb = (msg->rx_len >> 8) & 0xff;
	tx_hdr.wc_lsb = (msg->rx_len >> 0) & 0xff;
	dpu_dsi_ctrl_hw_gen_tx_hdr(hw->ctrl_blk, &tx_hdr);

	tx_hdr.data_type = msg->type;
	tx_hdr.wc_msb = 0;
	tx_hdr.wc_lsb = ((u8 *)msg->tx_buf)[0];
	dpu_dsi_ctrl_hw_gen_tx_hdr(hw->ctrl_blk, &tx_hdr);

	return 0;
}

int dsi_hw_ctrl_read_pkt(struct dsi_ctrl_hw_blk *hw, struct dsi_msg *msg, u32 wait_timeout)
{
	struct cri_rx_hdr rx_hdr;
	bool debug_flag = 0;
	int rx_len;
	u32 word, header;
	u8 data_type = 0;
	int i = 0;
	u8 *rx;

	/* do some minimum sanity checking */
	if (!dsi_packet_is_long(msg->type) && !dsi_packet_is_short(msg->type)) {
		DSI_ERROR("Please input correct type!, dt:%d\n", msg->type);
		return -1;
	}
	/* do virtual channel checking */
	if (msg->channel > 3) {
		DSI_ERROR("Please input correct vc!, vc:%d\n", msg->channel);
		return -1;
	}

	if(dsi_hw_ctrl_cri_rd_data_avail(hw, wait_timeout)) {
		header = dpu_dsi_ctrl_hw_get_rx_hdr(hw->ctrl_blk, &rx_hdr);
		data_type = header & CRI_RX_DATA_TYPE;
		rx_len = msg->rx_len;
		if (debug_flag == 1)
			DSI_INFO("dt 0x%x, len 0x%x, lsb 0x%x, msb 0x%x\n",
				data_type, rx_len,
				rx_hdr.wc_lsb, rx_hdr.wc_msb);

		/* short pkt format WC_LSB and WC_MSB: Data 0 and Data 1 */
		((u8 *)msg->rx_buf)[0] = rx_hdr.wc_lsb;
		((u8 *)msg->rx_buf)[1] = rx_hdr.wc_msb;

		if (dsi_packet_is_short(data_type))
			return 0;

		rx = &((u8 *)msg->rx_buf)[0];

		for(i = 0; i< DIV_ROUND_UP(rx_len, 4); i++){
			if (dsi_hw_ctrl_cri_rd_data_avail(hw, wait_timeout)) {
				word = dpu_dsi_ctrl_hw_get_rx_pld(hw->ctrl_blk);
				memcpy(rx, &word, 4);
				rx += 4;
			} else {
				word = dpu_dsi_ctrl_hw_get_rx_pld(hw->ctrl_blk);
			}
		}

	} else {
		header = dpu_dsi_ctrl_hw_get_rx_hdr(hw->ctrl_blk, &rx_hdr);
		rx_len = msg->rx_len;
		DSI_ERROR("hdr 0x%x, dt 0x%x, len 0x%x, lsb 0x%x, msb 0x%x\n",
			header, data_type, rx_len, rx_hdr.wc_lsb, rx_hdr.wc_msb);
		return -1;
	}

	return 0;
}

static int dsi_hw_ctrl_init(struct dpu_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	u32 state;

	dpu_hw_dsi_ctrl_info_get(hw);


	/* step 2: initializing dsi host control */
	state = dpu_hw_dsi_mode_get(hw);
	if (state == IPI_IDLE_MODE)
		DSI_INFO("DSI ctroller is power up, state: %d\n", state);
	else
		DSI_ERROR("DSI ctroller state error, state: %d\n", state);

	/* manual mode cfg: 0- disable(default), 1- enable */
	dpu_hw_dsi_manual_mode_en(hw, cfg->manual_mode_en);

	dpu_hw_dsi_phy_cfg(hw, cfg);

	dsi_phy_lptx_clk_cfg(hw, cfg);

	dsi_general_cfg(hw, cfg);

	dpu_hw_dsi_ipi_cfg(hw, cfg);

	dsi_ctrl_irq_cfg(hw);

	return 0;
}

int dsi_hw_init(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	struct dpu_hw_blk *phy_blk;
	struct dpu_hw_blk *ctrl_blk;
	struct dpu_hw_blk *sctrl_blk;

	phy_blk = hw->phy_blk;
	ctrl_blk = hw->ctrl_blk;
	sctrl_blk = hw->sctrl_blk;

	cfg->manual_mode_en = 0;
	cfg->auto_calc_en = 1;

	DSI_INFO("select phy: %d\n", cfg->phy_parms.phy_sel);

	dsi_hw_sctrl_init_clk_set(sctrl_blk, cfg);

	dsi_hw_ctrl_init(ctrl_blk, cfg);

	dsi_hw_sctrl_init_phy_pre(sctrl_blk, cfg);

	dsi_hw_phy_init(phy_blk, cfg);

	dsi_hw_sctrl_init_phy(sctrl_blk, cfg);

	dsi_hw_phy_wait_ready(phy_blk, &cfg->phy_parms);

	dsi_ctrl_power_up(ctrl_blk, 1);
	/* auto caculate, deskew */
	//dsi_ctrl_ready(ctrl_blk, cfg);

	if (cfg->phy_parms.phy_sel == SNPS_CDPHY_ASIC)
		dsi_hw_clk_debug(sctrl_blk);

	return 0;
}

int dsi_hw_ctrl_init_phy(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	struct dpu_hw_blk *sctrl_blk;

	sctrl_blk = hw->sctrl_blk;

	dsi_hw_sctrl_init_phy(sctrl_blk, cfg);
	return 0;
}

void dsi_hw_ctrl_deinit(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	struct dpu_hw_blk *ctrl_blk;

	ctrl_blk = hw->ctrl_blk;

	dsi_ctrl_power_up(ctrl_blk, 0);
}

int dsi_hw_ctrl_clk_cfg(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	struct dpu_hw_blk *ctrl_blk;

	ctrl_blk = hw->ctrl_blk;

	/* 1 - change hs clock mode, 2 - update LP TX speed */
	dsi_phy_lptx_clk_cfg(ctrl_blk, cfg);

	return 0;
}

static int dsi_ulps_enter(struct dsi_ctrl_hw_blk *hw)
{
	struct dpu_hw_blk *ctrl_blk;
	bool flag = false;
	u32 value;
	int ret;

	ctrl_blk = hw->ctrl_blk;

	ret = dsi_hw_check_ulps_state(hw, DSI_ULPS_EXIT);
	if (ret) {
		DSI_INFO("mipi dsi is aleady in ulps\n");
		return 0;
	}

	ret = DPU_READ_POLL_TIMEOUT(ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			!(value & IPI_BUSY), 10, 25000);
	if (ret < 0) {
		DSI_ERROR("dsi wait ipi vaild timeout, value %d\n", value);
		flag = true;
	}

	ret = DPU_READ_POLL_TIMEOUT(ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			!(value & CRI_BUSY), 10, 25000);
	if (ret < 0) {
		DSI_ERROR("dsi wait cri vaild timeout, value %d\n", value);
		flag = true;
	}

	ret = DPU_READ_POLL_TIMEOUT(ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			!(value & PRI_BUSY), 10, 25000);
	if (ret < 0) {
		DSI_ERROR("dsi wait pri vaild timeout, value %d\n", value);
		flag = true;
	}

	if (flag)
		return -1;

	value = 1 << CTRL_0X1C0_PRI_TX_CMD_PHY_ULPS_ENTRY_SHIFT;
	DPU_REG_WRITE(ctrl_blk, CTRL_0X1C0_PRI_TX_CMD, value, DIRECT_WRITE);
	DSI_DEBUG("enter ulps\n");
	ret = dsi_hw_check_ulps_state(hw, DSI_ULPS_ENTER);
	if (ret)
		DSI_WARN("mipi dsi ulps enter failed, value:%d\n", value);

	return ret;
}

void dsi_ulps_enter_with_cmdlist(struct dsi_ctrl_hw_blk *hw, s64 node_id)
{
	struct dpu_hw_blk *ctrl_blk;
	u32 value;

	ctrl_blk = hw->dctrl_blk;

	value = 1 << CTRL_0X1C0_PRI_TX_CMD_PHY_ULPS_ENTRY_SHIFT;
	DPU_REG_WRITE_WITH_NODE_ID(ctrl_blk, node_id, CTRL_0X1C0_PRI_TX_CMD, value);
}

int dsi_ulps_exit(struct dsi_ctrl_hw_blk *hw)
{
	struct dpu_hw_blk *ctrl_blk, *phy_blk;
	u32 value;
	int ret;

	ctrl_blk = hw->ctrl_blk;
	phy_blk = hw->phy_blk;

	ret = dsi_hw_check_ulps_state(hw, DSI_ULPS_ENTER);
	if (ret) {
		DSI_INFO("mipi dsi is aleady in lp11\n");
		return 0;
	}

	dsi_hw_phy_hibernate_exit(phy_blk);

	value = 1 << CTRL_0X1C0_PRI_TX_CMD_PHY_ULPS_EXIT_SHIFT;
	DPU_REG_WRITE(ctrl_blk, CTRL_0X1C0_PRI_TX_CMD, value, DIRECT_WRITE);
	DSI_DEBUG("exit ulps\n");
	ret = dsi_hw_check_ulps_state(hw, DSI_ULPS_EXIT);
	if (ret)
		DSI_ERROR("mipi dsi ulps exit failed, ret:%d\n", ret);

	return ret;
}

void dsi_ulps_exit_with_cmdlist(struct dsi_ctrl_hw_blk *hw, s64 node_id)
{
	struct dpu_hw_blk *ctrl_blk;
	u32 value;

	ctrl_blk = hw->dctrl_blk;

	DPU_REG_WRITE_WITH_NODE_ID(hw->dsctrl_blk, node_id, 0xE4, 1);
	DPU_REG_WRITE_WITH_NODE_ID(hw->dsctrl_blk, node_id, 0xE4, 0);

	value = 1 << CTRL_0X1C0_PRI_TX_CMD_PHY_ULPS_EXIT_SHIFT;
	DPU_REG_WRITE_WITH_NODE_ID(ctrl_blk, node_id, CTRL_0X1C0_PRI_TX_CMD, value);
}

int dsi_hw_ctrl_ulps_enter(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	int ret = 0;

	if ((cfg->cmdlist_flag != CMDLIST_WRITE) &&
		(cfg->cmdlist_flag != DIRECT_WRITE)) {
		DSI_ERROR("cmdlist flag error, %d\n", cfg->cmdlist_flag);
		return -1;
	}

	if (cfg->cmdlist_flag == DIRECT_WRITE)
		ret = dsi_ulps_enter(hw);
	else
		dsi_ulps_enter_with_cmdlist(hw, cfg->cmdlist_node_id);
	return ret;
}

int dsi_hw_ctrl_ulps_exit(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	int ret = 0;

	if ((cfg->cmdlist_flag != CMDLIST_WRITE) &&
		(cfg->cmdlist_flag != DIRECT_WRITE)) {
		DSI_ERROR("cmdlist flag error, %d\n", cfg->cmdlist_flag);
		return -1;
	}

	if (cfg->cmdlist_flag == DIRECT_WRITE)
		ret = dsi_ulps_exit(hw);
	else
		dsi_ulps_exit_with_cmdlist(hw, cfg->cmdlist_node_id);
	return ret;
}

int dsi_hw_debug(struct dsi_ctrl_hw_blk *hw,
		enum dsi_debug type, u8 parm_cnt, u8 *parm)
{
	int ret = 0;

	switch (type) {
	case DSI_SET_PHY_AMPLITUDE:
		break;
	case DSI_SET_PHY_EQ:
		dsi_hw_phy_set_eq_debug(hw->phy_blk, 5, parm[0], parm[1]);
		break;
	case DSI_GET_PHY_EQ_PARMS:
		ret = dsi_hw_phy_get_eq_parms(hw->phy_blk);
		break;
	default:
		DSI_ERROR("unsupported debug function, type %d\n", type);
		ret = -1;
		break;
	}

	return ret;
}

int dsi_hw_ctrl_enable(struct dsi_ctrl_hw_blk *hw, struct dsi_ctrl_cfg *cfg)
{
	if(cfg->vg_en)
		dsi_hw_sctrl_vg_en(hw->sctrl_blk, cfg);

	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		dpu_hw_dsi_mode_ctrl(hw->ctrl_blk, IPI_VIDEO_MODE);
	else
		dpu_hw_dsi_mode_ctrl(hw->ctrl_blk, IPI_DATA_STREAM_MODE);

	DSI_DEBUG("set dsi mode %d(0 video, 1 command)\n", cfg->ctrl_mode);

	return 0;
}

int dsi_hw_ctrl_disable(struct dsi_ctrl_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	struct dpu_hw_blk *ctrl_blk;

	ctrl_blk = hw->ctrl_blk;

	/* DSI ctrl power off */
	dsi_ctrl_power_up(ctrl_blk, 0);

	return 0;
}

void dsi_hw_ctrl_reset(struct dsi_ctrl_hw_blk *hw)
{
	struct dpu_hw_blk *ctrl_blk;

	ctrl_blk = hw->ctrl_blk;

	dsi_hw_ctrl_sw_reset(ctrl_blk, 0);

	DPU_USLEEP(1000);

	dsi_hw_ctrl_sw_reset(ctrl_blk, 1);
	DPU_USLEEP(2000);
}

static void dsi_hw_ctrl_dyn_freq_wait_ready(struct dpu_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	enum ipi_mode_ctrl mode;
	int ret;

	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		mode = IPI_VIDEO_MODE;
	else
		mode = IPI_DATA_STREAM_MODE;

	/* dsi phy de-skew calculate */
	dpu_hw_dsi_phy_deskewcal(hw, cfg);

	if (cfg->auto_calc_en) {
		DSI_DEBUG("auto cal mode Enable!\n");
		dpu_hw_dsi_mode_ctrl(hw, IPI_AUTO_CAL_MODE);

		/* check dsi is in auto-calculate mode */
		ret = dpu_hw_dsi_ctrl_mode_get(hw, IPI_IDLE_MODE);
		if (ret)
			DSI_ERROR("wait auto cal timeout!\n");
	}

	dsi_ctrl_clk_rate_get(hw);

	/* set vide mode or command mode */
	if (cfg->ctrl_mode == DSI_VIDEO_MODE)
		dpu_hw_dsi_mode_ctrl(hw, IPI_COMMAND_MODE);
	else
		dpu_hw_dsi_mode_ctrl(hw, IPI_DATA_STREAM_MODE);
}

int dsi_hw_ctrl_mipi_freq_update(struct dsi_ctrl_hw_blk *hw,
		struct dsi_ctrl_cfg *cfg)
{
	int ret;
	u32 value;
	struct dpu_hw_blk *phy_blk;
	struct dpu_hw_blk *ctrl_blk;
	struct dpu_hw_blk *sctrl_blk;
	u32 phy_status;

	phy_blk = hw->phy_blk;
	ctrl_blk = hw->ctrl_blk;
	sctrl_blk = hw->sctrl_blk;

	ret = DPU_READ_POLL_TIMEOUT(ctrl_blk, CTRL_0X020_CORE_STATUS, value,
			(value == 0), 50, 25000);
	if (ret < 0) {
		DSI_ERROR("wait timeout, ctrl is busy, status 0x%x\n", value);
		return -1;
	}

	phy_status = CTRL_0X108_PHY_STATUS_PHY_CLK_STOPSTATE_MASK |
			CTRL_0X108_PHY_STATUS_PHY_L0_STOPSTATE_MASK |
			CTRL_0X108_PHY_STATUS_PHY_L1_STOPSTATE_MASK |
			CTRL_0X108_PHY_STATUS_PHY_L2_STOPSTATE_MASK |
			CTRL_0X108_PHY_STATUS_PHY_L3_STOPSTATE_MASK;
	ret = DPU_READ_POLL_TIMEOUT(ctrl_blk, CTRL_0X108_PHY_STATUS, value,
			((value & phy_status) == phy_status), 50, 25000);
	if (ret < 0) {
		DSI_ERROR("wait timeout, ctrl phy is not stop, status 0x%x\n", value);
		return -1;
	}

	dsi_hw_sctrl_pll_clk_sel(sctrl_blk, false);
	dsi_hw_phy_dyn_freq_pll_disable(phy_blk);

	dsi_hw_sctrl_phy_pll_config(sctrl_blk, cfg);

	dsi_hw_phy_mipi_freq_update(phy_blk, cfg);

	dsi_hw_phy_dyn_freq_pll_enable(phy_blk);

	dsi_hw_sctrl_dyn_freq_wait_pll_lock(sctrl_blk);

	dsi_hw_sctrl_pll_clk_sel(sctrl_blk, true);

	dsi_hw_sctrl_dyn_freq_wait_phy_ready(sctrl_blk);
	dsi_hw_ctrl_dyn_freq_wait_ready(ctrl_blk, cfg);

	if (cfg->phy_parms.phy_sel == SNPS_CDPHY_ASIC)
		dsi_hw_clk_debug(sctrl_blk);

	return 0;
}
