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
#include "dp_hw_ctrl_ops.h"
#include "dpu_hw_dsc_cfg.h"
#include "dpu_hw_dsc_ops.h"

#define FPGA_PIXEL_CLOCK_KHZ                               25200

#define PHY_BUSY_CHECK_DELAY_US                            10
#define PHY_BUSY_CHECK_TIMEOUT_US                          10000

#define PPS_REPEAT_COUNT                                   0x3

extern enum dp_combo_mode g_combo_mode;

int _dp_hw_ctrl_wait_phy_busy(struct dpu_hw_blk *hw)
{
	u32 phy_busy_state = 0;
	u32 phy_busy_mask;
	int ret;

	phy_busy_mask = BITS_MASK(PHY_BUSY_SHIFT,
			g_combo_mode == DP_COMBO_MODE_DP4 ? 4 : 2);

	ret = DP_READ_POLL_TIMEOUT(hw, PHYIF_CTRL_OFFSET,
			phy_busy_state, (phy_busy_state & phy_busy_mask) == 0,
			PHY_BUSY_CHECK_DELAY_US, PHY_BUSY_CHECK_TIMEOUT_US);
	if (ret < 0)
		DP_ERROR("phy is busy 0x%x, ret %d\n", phy_busy_state, ret);

	return ret;
}

/* aux start */
void dp_hw_ctrl_read_aux_data(struct dpu_hw_blk *hw, u8 *buf, u8 len)
{
	u32 aux_data[AUX_DATA_COUNT] = {0};
	u8 aux_data_idx;
	u8 shift;
	int i;

	for (i = 0; i < AUX_DATA_COUNT; i++)
		aux_data[i] = DP_REG_READ(hw, AUX_DATA_OFFSET(i));

	for (i = 0; i < len; i++) {
		aux_data_idx = i / 4;
		shift = (i % 4) * 8;

		buf[i] = GET_BITS_VAL(aux_data[aux_data_idx], shift, 8);
	}
}

void dp_hw_ctrl_write_aux_data(struct dpu_hw_blk *hw, u8 *buf, u8 len)
{
	u32 aux_data[AUX_DATA_COUNT] = {0};
	u8 aux_data_idx;
	u8 shift;
	int i;

	for (i = 0; i < len; i++) {
		aux_data_idx = i / 4;
		shift = (i % 4) * 8;

		aux_data[aux_data_idx] = MERGE_BITS(aux_data[aux_data_idx],
				buf[i], shift, 8);
	}

	for (i = 0; i < AUX_DATA_COUNT; i++)
		DP_REG_WRITE(hw, AUX_DATA_OFFSET(i), aux_data[i]);
}

void dp_hw_ctrl_send_aux_request(struct dpu_hw_blk *hw, u8 request,
		u32 addr, u8 len)
{
	u32 aux_cmd = 0;

	aux_cmd = MERGE_BITS(aux_cmd, (u32)request,
			AUX_CMD_TYPE_SHIFT, AUX_CMD_TYPE_LEN);
	aux_cmd = MERGE_BITS(aux_cmd, (u32)addr,
			AUX_ADDR_SHIFT, AUX_ADDR_LEN);
	aux_cmd = MERGE_BITS(aux_cmd, (u32)(len == 0),
			I2C_ADDR_ONLY_SHIFT, I2C_ADDR_ONLY_LEN);
	/* len range [1, 16], need to write with [0, 15] */
	if (len)
		aux_cmd = MERGE_BITS(aux_cmd, (u32)(len - 1),
				AUX_LEN_REQ_SHIFT, AUX_LEN_REQ_LEN);

	DP_REG_WRITE(hw, AUX_CMD_OFFSET, aux_cmd);
}

void dp_hw_ctrl_get_aux_reply_status(struct dpu_hw_blk *hw,
		struct dp_aux_reply_status *rep_status)
{
	u32 value;

	memset(rep_status, 0, sizeof(struct dp_aux_reply_status));

	value = DP_REG_READ(hw, AUX_STATUS_OFFSET);

	rep_status->bytes_read = GET_BITS_VAL(value, AUX_BYTES_READ_SHIFT,
			AUX_BYTES_READ_LEN);
	rep_status->reply_type = GET_BITS_VAL(value, AUX_STATUS_SHIFT,
			AUX_STATUS_LEN);
	rep_status->aux_m = GET_BITS_VAL(value, AUX_M_SHIFT, AUX_M_LEN);
	rep_status->reply_received = !(value & BIT(AUX_REPLY_RECEIVED_SHIFT));
	rep_status->err_code = GET_BITS_VAL(value,
			AUX_REPLY_ERR_CODE_SHIFT, AUX_REPLY_ERR_CODE_LEN);

	if (value & BIT(AUX_REPLY_ERR_SHIFT))
		rep_status->err_status |= DP_AUX_REPLY_STATUS_ERR;
	if (value & BIT(AUX_TIMEOUT_SHIFT))
		rep_status->err_status |= DP_AUX_REPLY_STATUS_TIMEOUT;
	if (value & BIT(SINK_DISCONNECT_WHILE_ACTIVE_SHIFT))
		rep_status->err_status |= DP_AUX_REPLY_STATUS_DISCONNECTED;

	value = DP_REG_READ(hw, GENERAL_INTERRUPT_OFFSET);
	if (value & BIT(AUX_CMD_INVALID_SHIFT))
		rep_status->err_status |= DP_AUX_REPLY_STATUS_CMD_INVALID;
}

void dp_hw_ctrl_enable_intr(struct dpu_hw_blk *hw,
		enum dp_intr_type intr_type, bool enable)
{
	u32 val;

	val = DP_REG_READ(hw, GENERAL_INTERRUPT_ENABLE_OFFSET);
	DP_REG_WRITE(hw, GENERAL_INTERRUPT_ENABLE_OFFSET,
			enable ? (val | intr_type) : (val & ~intr_type));
}

bool dp_hw_ctrl_get_intr_state(struct dpu_hw_blk *hw, enum dp_intr_type intr_type)
{
	u32 val;

	val = DP_REG_READ(hw, GENERAL_INTERRUPT_OFFSET);
	return !!(val & intr_type);
}

void dp_hw_ctrl_clear_intr_state(struct dpu_hw_blk *hw,
		enum dp_intr_type intr_type)
{
	if (intr_type & DP_INTR_AUX)
		DP_REG_WRITE(hw, GENERAL_INTERRUPT_OFFSET, DP_INTR_AUX);

	if (intr_type & DP_INTR_HPD)
		DP_REG_WRITE(hw, HPD_STATUS_OFFSET, 0xFFFFFFFF);

	if (intr_type & DP_INTR_DSC)
		DP_REG_WRITE(hw, DSC_INTSTS_OFFSET, 0xFFFFFFFF);

	if (intr_type & DP_INTR_POWER)
		DP_REG_WRITE(hw, PM_INT_STS_OFFSET, 0xFFFFFFFF);

	if (intr_type & DP_INTR_VIDEO_FIFO_OVERFLOW_0)
		DP_REG_WRITE(hw, GENERAL_INTERRUPT_OFFSET, DP_INTR_VIDEO_FIFO_OVERFLOW_0);

	if (intr_type & DP_INTR_VIDEO_FIFO_UNDERFLOW_0)
		DP_REG_WRITE(hw, GENERAL_INTERRUPT_OFFSET, DP_INTR_VIDEO_FIFO_UNDERFLOW_0);

	if (intr_type & DP_INTR_AUDIO_FIFO_OVERFLOW_0)
		DP_REG_WRITE(hw, GENERAL_INTERRUPT_OFFSET, DP_INTR_AUDIO_FIFO_OVERFLOW_0);
}

void dp_hw_ctrl_default_config(struct dpu_hw_blk *hw)
{
	/* disable fast link training */
	DP_BIT_WRITE(hw, CCTL_OFFSET, 0,
			DEFAULT_FAST_LINK_TRAIN_EN_SHIFT, DEFAULT_FAST_LINK_TRAIN_EN_LEN);

	/* aux timeout fix to 3.2ms */
	DP_BIT_WRITE(hw, CCTL_OFFSET, 1,
			SEL_AUX_TIMEOUT_32MS_SHIFT, SEL_AUX_TIMEOUT_32MS_LEN);
}

/**
 * set reset_bitmap to DPTX_MODULE_CONTROLLER to reset controller,
 * DPTX_MODULE_CONTROLLER | DPTX_MODULE_PHY to rest controller and phy.
 */
void _dp_hw_ctrl_reset_module(struct dpu_hw_blk *hw, u32 modules)
{
	/* reset, wait to take effect and then clear reset state */
	DP_REG_WRITE(hw, SOFT_RESET_CTRL_OFFSET, modules);

	usleep_range(10, 20);

	DP_REG_WRITE(hw, SOFT_RESET_CTRL_OFFSET, 0);
}

static void dp_hw_ctrl_set_timing(struct dpu_hw_blk *hw,
		struct dp_display_timing *timing, u32 link_clk_khz)
{
	u32 reg;

	/* Configure video_config1 register */
	/* R_V_BLANK_IN_OSC keep default value */
	reg = 0;
	reg = MERGE_BITS(reg, timing->interlaced, I_P_SHIFT, I_P_LEN);
	reg = MERGE_BITS(reg, timing->h_blank, HBLANK_SHIFT, HBLANK_LEN);
	reg = MERGE_BITS(reg, timing->h_active, HACTIVE_SHIFT, HACTIVE_LEN);
	DP_REG_WRITE(hw, VIDEO_CONFIG_OFFSET1, reg);

	/* Configure video_config2 register */
	reg = 0;
	reg = MERGE_BITS(reg, timing->v_blank, VBLANK_SHIFT, VBLANK_LEN);
	reg = MERGE_BITS(reg, timing->v_active, VACTIVE_SHIFT, VACTIVE_LEN);
	DP_REG_WRITE(hw, VIDEO_CONFIG_OFFSET2, reg);

	/* Configure video_config3 register */
	reg = 0;
	reg = MERGE_BITS(reg, timing->h_front_porch,
			VIDEO_CONFIG_H_FRONT_PORCH_SHIFT, VIDEO_CONFIG_H_FRONT_PORCH_LEN);
	reg = MERGE_BITS(reg, timing->h_sync_width,
			VIDEO_CONFIG_H_SYNC_WIDTH_SHIFT, VIDEO_CONFIG_H_SYNC_WIDTH_LEN);
	DP_REG_WRITE(hw, VIDEO_CONFIG_OFFSET3, reg);

	/* Configure video_config4 register */
	reg = 0;
	reg = MERGE_BITS(reg, timing->v_front_porch,
			VIDEO_CONFIG_V_FRONT_PORCH_SHIFT, VIDEO_CONFIG_V_FRONT_PORCH_LEN);
	reg = MERGE_BITS(reg, timing->v_sync_width,
			VIDEO_CONFIG_V_SYNC_WIDTH_SHIFT, VIDEO_CONFIG_V_SYNC_WIDTH_LEN);
	DP_REG_WRITE(hw, VIDEO_CONFIG_OFFSET4, reg);

	/* polarity, keep with hw_tmg */
	reg = 0;
	reg = MERGE_BITS(reg, 1, HSYNC_IN_POLARITY_SHIFT, HSYNC_IN_POLARITY_LEN);
	reg = MERGE_BITS(reg, 1, VSYNC_IN_POLARITY_SHIFT, VSYNC_IN_POLARITY_LEN);
	DP_REG_WRITE(hw, VINPUT_POLARITY_CTRL_OFFSET, reg);

	/* h_blank interval */
	DP_BIT_WRITE(hw, VIDEO_HBLANK_INTERVAL_OFFSET,
			timing->h_blank * link_clk_khz / timing->pixel_clock_khz,
			HBLANK_INTERVAL_SHIFT, HBLANK_INTERVAL_LEN);

	/* Configure video_msa1 register */
	reg = 0;
	reg = MERGE_BITS(reg, timing->h_blank - timing->h_front_porch,
			HSTART_SHIFT, HSTART_LEN);
	reg = MERGE_BITS(reg, timing->v_blank - timing->v_front_porch,
			VSTART_SHIFT, VSTART_LEN);
	DP_REG_WRITE(hw, VIDEO_MSA_OFFSET1, reg);
}

#define VIDEO_MAPPING_TABLE_SIZE 28
#define CONCAT(a, b) ((a << 16) | b)

static u32 video_mapping_table[VIDEO_MAPPING_TABLE_SIZE] = {
	CONCAT(PIXEL_FORMAT_RGB, DP_BPC_6),
	CONCAT(PIXEL_FORMAT_RGB, DP_BPC_8),
	CONCAT(PIXEL_FORMAT_RGB, DP_BPC_10),
	CONCAT(PIXEL_FORMAT_RGB, DP_BPC_12),
	CONCAT(PIXEL_FORMAT_RGB, DP_BPC_16),
	CONCAT(PIXEL_FORMAT_YCBCR444, DP_BPC_8),
	CONCAT(PIXEL_FORMAT_YCBCR444, DP_BPC_10),
	CONCAT(PIXEL_FORMAT_YCBCR444, DP_BPC_12),
	CONCAT(PIXEL_FORMAT_YCBCR444, DP_BPC_16),
	CONCAT(PIXEL_FORMAT_YCBCR422, DP_BPC_8),
	CONCAT(PIXEL_FORMAT_YCBCR422, DP_BPC_10),
	CONCAT(PIXEL_FORMAT_YCBCR422, DP_BPC_12),
	CONCAT(PIXEL_FORMAT_YCBCR422, DP_BPC_16),
	CONCAT(PIXEL_FORMAT_YCBCR420, DP_BPC_8),
	CONCAT(PIXEL_FORMAT_YCBCR420, DP_BPC_10),
	CONCAT(PIXEL_FORMAT_YCBCR420, DP_BPC_12),
	CONCAT(PIXEL_FORMAT_YCBCR420, DP_BPC_16),
	CONCAT(PIXEL_FORMAT_YONLY, DP_BPC_8),
	CONCAT(PIXEL_FORMAT_YONLY, DP_BPC_10),
	CONCAT(PIXEL_FORMAT_YONLY, DP_BPC_12),
	CONCAT(PIXEL_FORMAT_YONLY, DP_BPC_16),
	CONCAT(PIXEL_FORMAT_RAW, DP_BPC_8),
	CONCAT(PIXEL_FORMAT_RAW, DP_BPC_10),
	CONCAT(PIXEL_FORMAT_RAW, DP_BPC_12),
	CONCAT(PIXEL_FORMAT_RAW, DP_BPC_16)
};

static void dp_hw_ctrl_set_video_mapping(struct dpu_hw_blk *hw,
		struct dp_hw_ctrl_msa_config *msa)
{
	u32 concat_value = CONCAT(msa->pixel_format, msa->bpc);
	u8 video_mapping = 0;

	for (video_mapping = 0; video_mapping < VIDEO_MAPPING_TABLE_SIZE;
			video_mapping++) {
		if (video_mapping_table[video_mapping] == concat_value)
			break;
	}

	if (video_mapping == VIDEO_MAPPING_TABLE_SIZE)
		video_mapping = 0;

	DP_BIT_WRITE(hw, VSAMPLE_CTRL_OFFSET, video_mapping,
			VIDEO_MAPPING_SHIFT, VIDEO_MAPPING_LEN);
}

static u8 dp_hw_ctrl_misc0_bpc_mapping(
		struct dp_hw_ctrl_msa_config *msa)
{
	u8 pixel_format = msa->pixel_format;
	u8 bpc_mapping = 0;

	/* According to Table 2-96 of DisplayPort spec 1.4 */
	switch (msa->bpc) {
	case DP_BPC_6:
		bpc_mapping = (pixel_format == PIXEL_FORMAT_RAW) ? 1 : 0;
		break;
	case DP_BPC_8:
		bpc_mapping = (pixel_format == PIXEL_FORMAT_RAW) ? 3 : 1;
		break;
	case DP_BPC_10:
		bpc_mapping = (pixel_format == PIXEL_FORMAT_RAW) ? 4 : 2;
		break;
	case DP_BPC_12:
		bpc_mapping = (pixel_format == PIXEL_FORMAT_RAW) ? 5 : 3;
		break;
	case DP_BPC_16:
		bpc_mapping = (pixel_format == PIXEL_FORMAT_RAW) ? 7 : 4;
		break;
	}

	return bpc_mapping;
}

static u8 dp_hw_ctrl_misc0_colorimetry_mapping(
		struct dp_hw_ctrl_msa_config *msa)
{
	u8 dynamic_range = msa->dynamic_range;
	u8 colorimetry = msa->colorimetry;
	u8 colorimetry_mapping = 0;

	/* According to Table 2-96 of DisplayPort spec 1.4 */
	switch (msa->pixel_format) {
	case PIXEL_FORMAT_RGB:
		if (dynamic_range == DYNAMIC_RANGE_CEA)
			colorimetry_mapping = 4;
		else if (dynamic_range == DYNAMIC_RANGE_VESA)
			colorimetry_mapping = 0;
		break;
	case PIXEL_FORMAT_YCBCR422:
		if (colorimetry == DP_COLORIMETRY_BT601)
			colorimetry_mapping = 5;
		else if (colorimetry == DP_COLORIMETRY_BT709)
			colorimetry_mapping = 13;
		break;
	case PIXEL_FORMAT_YCBCR444:
		if (colorimetry == DP_COLORIMETRY_BT601)
			colorimetry_mapping = 6;
		else if (colorimetry == DP_COLORIMETRY_BT709)
			colorimetry_mapping = 14;
		break;
	case PIXEL_FORMAT_RAW:
		colorimetry_mapping = 1;
		break;
	case PIXEL_FORMAT_YCBCR420:
	case PIXEL_FORMAT_YONLY:
		colorimetry_mapping = 0;
		break;
	}

	return colorimetry_mapping;
}

static u8 dp_hw_ctrl_misc1_colorimetry_mapping(
		struct dp_hw_ctrl_msa_config *msa)
{
	u8 mapping = 0;

	switch (msa->pixel_format) {
	case PIXEL_FORMAT_RGB:
	case PIXEL_FORMAT_YCBCR420:
	case PIXEL_FORMAT_YCBCR422:
	case PIXEL_FORMAT_YCBCR444:
		break;
	case PIXEL_FORMAT_YONLY:
	case PIXEL_FORMAT_RAW:
		mapping = 1;
		break;
	}

	return mapping;
}

static void dp_hw_ctrl_set_msa(struct dpu_hw_blk *hw,
		struct dp_hw_ctrl_msa_config *msa)
{
	u32 reg = 0;

	/* Configure video_msa2 register */
	/* use ASYNCHRONOUS_MODE */
	reg = 0;
	reg = MERGE_BITS(reg, MISC0_ASYNCHRONOUS_MODE,
			MISC0_MODE_SHIFT, MISC0_MODE_LEN);
	reg = MERGE_BITS(reg, dp_hw_ctrl_misc0_bpc_mapping(msa),
			MISC0_BPC_SHIFT, MISC0_BPC_LEN);
	reg = MERGE_BITS(reg, dp_hw_ctrl_misc0_colorimetry_mapping(msa),
			MISC0_COLORIMETRY_SHIFT, MISC0_COLORIMETRY_LEN);
	/* don`t have to configure MVID in async mode */
	DP_BIT_WRITE(hw, VIDEO_MSA_OFFSET2, reg, MISC0_SHIFT, MISC0_LEN);

	/* Configure video_msa3 register */
	reg = 0;
	reg = MERGE_BITS(reg, dp_hw_ctrl_misc1_colorimetry_mapping(msa),
			MISC1_COLORIMETRY_SHIFT, MISC1_COLORIMETRY_LEN);
	DP_BIT_WRITE(hw, VIDEO_MSA_OFFSET3, reg, MISC1_SHIFT, MISC1_LEN);
}

static void dp_hw_ctrl_set_tu(struct dpu_hw_blk *hw,
		struct dp_hw_ctrl_tu_config *tu)
{
	u32 reg;

	/* Configure video_config5 register */
	reg = 0;
	reg = MERGE_BITS(reg, tu->valid_bytes,
			AVERAGE_BYTES_PER_TU_SHIFT, AVERAGE_BYTES_PER_TU_LEN);
	reg = MERGE_BITS(reg, tu->valid_bytes_frac,
			AVERAGE_BYTES_PER_TU_FRAC_SST_SHIFT,
			AVERAGE_BYTES_PER_TU_FRAC_SST_LEN);
	DP_REG_WRITE(hw, VIDEO_CONFIG_OFFSET5, reg);
}

static void dp_hw_ctrl_set_mvid(struct dpu_hw_blk *hw, u8 dsc_bpp,
		u32 pclk_khz, u32 h_total)
{
	u32 h_total_compressed;
	u32 quotient;
	u32 reminder;
	u32 reg;

	h_total_compressed = DIV_ROUND_UP(h_total * dsc_bpp, 48);

#ifdef ASIC
	quotient = (4 * h_total) / h_total_compressed;
	reminder = (4 * h_total) % h_total_compressed;
#elif defined FPGA
	/**
	 * dptx ipi pixel clock is fixed on FPGA, even dsc is on.
	 * so we have to configure mvid in current code via pixel clock.
	 */

	/* htotal / htotal_compressed is equal to pclk_khz / FPGA_PIXEL_CLOCK_KHZ */
	quotient = (4000 * pclk_khz) / FPGA_PIXEL_CLOCK_KHZ;
	reminder = (quotient % 1000) * h_total_compressed / 1000;
	quotient = quotient / 1000;
#endif

	DP_DEBUG("htotal:%d, h_total_compressed:%d, quotient:%d, reminder:%d\n",
			h_total, h_total_compressed, quotient, reminder);

	reg = 0;
	reg = MERGE_BITS(reg, h_total_compressed, MVID_CUST_DEN_SHIFT,
			MVID_CUST_DEN_LEN);
	reg = MERGE_BITS(reg, 1, MVID_CUST_EN_SHIFT, MVID_CUST_EN_LEN);
	DP_REG_WRITE(hw, MVID_CONFIG_OFFSET1, reg);

	reg = 0;
	reg = MERGE_BITS(reg, quotient, MVID_CUST_QUO_SHIFT, MVID_CUST_QUO_LEN);
	reg = MERGE_BITS(reg, reminder, MVID_CUST_MOD_SHIFT, MVID_CUST_MOD_LEN);
	DP_REG_WRITE(hw, MVID_CONFIG_OFFSET2, reg);
}

static void dp_hw_ctrl_clear_mvid(struct dpu_hw_blk *hw)
{
	DP_REG_WRITE(hw, MVID_CONFIG_OFFSET1, 0);
	DP_REG_WRITE(hw, MVID_CONFIG_OFFSET2, 0);
}

static void dp_hw_ctrl_set_pps(struct dpu_hw_blk *hw, u8 pps[DSC_PPS_TABLE_BYTES_SIZE])
{
	u32 reg;
	u32 i;

	for (i = 0; i < DSC_PPS_TABLE_BYTES_SIZE; i += 4) {
		reg = 0;
		reg = MERGE_BITS(reg, pps[i],     0,  8);
		reg = MERGE_BITS(reg, pps[i + 1], 8,  8);
		reg = MERGE_BITS(reg, pps[i + 2], 16, 8);
		reg = MERGE_BITS(reg, pps[i + 3], 24, 8);
		DP_REG_WRITE(hw, DSC_PPS_OFFSET(i / 4), reg);
	}
}

static void dp_hw_ctrl_config_dsc(struct dpu_hw_blk *hw,
		struct dpu_dsc_config *dsc_cfg, u32 pclk_khz, u32 htotal)
{
	u8 pps[DSC_PPS_TABLE_BYTES_SIZE] = {0};

	dpu_hw_dsc_convert_to_pps(dsc_cfg, pps);
	dp_hw_ctrl_set_pps(hw, pps);

	/* set pps repeat count */
	DP_BIT_WRITE(hw, DSC_CTL_OFFSET, PPS_REPEAT_COUNT,
			PPS_REPEAT_CNT_SHIFT, PPS_REPEAT_CNT_LEN);

	/* set vsample_ctrl.link_upd_pps */
	DP_BIT_WRITE(hw, VSAMPLE_CTRL_OFFSET, 0x1,
			LINK_UPD_PPS_SHIFT, LINK_UPD_PPS_LEN);

	/* set mvid */
	dp_hw_ctrl_set_mvid(hw, dsc_cfg->bits_per_pixel >> 4, pclk_khz, htotal);
}

void dp_hw_ctrl_enable_dsc(struct dpu_hw_blk *hw, bool enable)
{
	DP_BIT_WRITE(hw, VSAMPLE_CTRL_OFFSET, enable,
			ENABLE_DSC_SHIFT, ENABLE_DSC_LEN);
}

void dp_hw_ctrl_config_video(struct dpu_hw_blk *hw,
		struct dp_hw_ctrl_video_config *video_config)
{
	dp_hw_ctrl_set_timing(hw, &video_config->timing, video_config->link_clk_khz);
	dp_hw_ctrl_set_tu(hw, &video_config->tu);
	dp_hw_ctrl_set_msa(hw, &video_config->msa);

	/* set init_threshold */
	DP_BIT_WRITE(hw, VIDEO_CONFIG_OFFSET5,
			video_config->init_threshold,
			INIT_THRESHOLD_SHIFT, INIT_THRESHOLD_LEN);
	DP_BIT_WRITE(hw, VIDEO_CONFIG_OFFSET5,
			video_config->init_threshold >> INIT_THRESHOLD_LEN,
			INIT_THRESHOLD_HI_SHIFT, INIT_THRESHOLD_HI_LEN);

	/* set ipi interface */
	/* enable video_mapping_ipi */
	DP_BIT_WRITE(hw, VSAMPLE_CTRL_OFFSET, 1,
			VIDEO_MAPPING_IPI_EN_SHIFT, VIDEO_MAPPING_IPI_EN_LEN);
	/* set PIXEL MODE to quad pixel mode */
	DP_BIT_WRITE(hw, VSAMPLE_CTRL_OFFSET, 0x2,
			PIXEL_MODE_SELECT_SHIFT, PIXEL_MODE_SELECT_LEN);
	dp_hw_ctrl_set_video_mapping(hw, &video_config->msa);

	DP_BIT_WRITE(hw, CCTL_OFFSET, video_config->enhance_frame_en,
			ENHANCE_FRAMING_EN_SHIFT, ENHANCE_FRAMING_EN_LEN);

	dp_hw_ctrl_clear_mvid(hw);

	if (video_config->dsc_en)
		dp_hw_ctrl_config_dsc(hw, video_config->dsc_config,
				video_config->timing.pixel_clock_khz,
				video_config->timing.h_total);
}

void dp_hw_ctrl_enable_video_transfer(struct dpu_hw_blk *hw, bool enable)
{
	DP_BIT_WRITE(hw, VSAMPLE_CTRL_OFFSET, enable,
			VIDEO_STREAM_ENABLE_SHIFT, VIDEO_STREAM_ENABLE_LEN);
}

static void dp_hw_ctrl_config_audio_sdpinfo(struct dpu_hw_blk *hw,
		struct dp_audio_fmt_info *ainfo)
{
	/*
	 * HB0: 0x00 - secondary-data packet id
	 * HB1: 0x84 - audio infoframe type
	 * HB2: 0x1B - data byte count
	 * HB3[7:2]: 0x11 - infoframe sdp version number
	 */
	u32 audio_infoframe_head = 0x441B8400;
	/*
	 * byte1: 0x10
	 *     bit[2:0]: 000  - audio channel refer to stream header
	 *     bit[7:4]: 0001 - audio data type(PCM)
	 * byte2: 0x7
	 *     bit[1:0]: 00  - bit width refer to stream header
	 *     bit[4:2]: 000 - sample rate refer to stream header
	 */
	 u32 audio_infoframe_data[3] = {0x00000010, 0x0, 0x0};

	/* byte2 */
	switch (ainfo->bit_width) {
	case 16:
		/* bit[1:0]: 01 - bit width(16bit) */
		audio_infoframe_data[0] |= 0x00000100;
		break;
	case 24:
		/* bit[1:0]: 11 - bit width(24bit) */
		audio_infoframe_data[0] |= 0x00000300;
		break;
	default:
		DP_INFO("refer to stream head. bit width(%d).\n", ainfo->bit_width);
		/* audio_infoframe_data[0] |= 0x00000000; */
		break;
	}

	switch (ainfo->samp_rate) {
	case 32000:
		/* bit[4:2]: 001 - sample rate(32kHz) */
		audio_infoframe_data[0] |= 0x00000400;
		break;
	case 44100:
		/* bit[4:2]: 010 - sample rate(44.1kHz) */
		audio_infoframe_data[0] |= 0x00000800;
		break;
	case 48000:
		/* bit[4:2]: 011 - sample rate(48kHz) */
		audio_infoframe_data[0] |= 0x00000C00;
		break;
	case 96000:
		/* bit[4:2]: 101 - sample rate(96kHz) */
		audio_infoframe_data[0] |= 0x00001400;
		break;
	case 192000:
		/* bit[4:2]: 111 - sample rate(192kHz) */
		audio_infoframe_data[0] |= 0x00001C00;
		break;
	default:
		/* unsupported sample rate, use default value */
		DP_DEBUG("refer to stream head. sample rate(%d).\n", ainfo->samp_rate);
		/* audio_infoframe_data[0] |= 0x00000000; */
		break;
	}

	/* byte1: bit[2:0] channels number */
	/* byte4: channels map */
	switch (ainfo->ch_num) {
	case 1:
		/* CA: 0x01 - - - - - - FR FL */
		audio_infoframe_data[0] |= 0x00000000;
		break;
	case 2:
		/* CA: 0x01 - - - - - - FR FL */
		audio_infoframe_data[0] |= 0x00000001;
		break;
	case 3:
		/* CA: 0x01 - - - - - LFE FR FL */
		audio_infoframe_data[0] |= 0x02000002;
		break;
	case 4:
		/* CA: 0x03 - - - - FC LFE FR FL */
		audio_infoframe_data[0] |= 0x03000003;
		break;
	case 5:
		/* CA: 0x07 - - - RC FC LFE FR FL */
		audio_infoframe_data[0] |= 0x07000004;
		break;
	case 6:
		/* CA: 0x0B - - RR RL FC LFE FR FL */
		audio_infoframe_data[0] |= 0x0B000005;
		break;
	case 7:
		/* CA: 0x0F - RC RR RL FC LFE FR FL */
		audio_infoframe_data[0] |= 0x0F000006;
		break;
	case 8:
		/* CA: 0x13 RRC RC RR RL FC LFE FR FL */
		audio_infoframe_data[0] |= 0x13000007;
		break;
	default:
		/* unsupported channel map, use default value 0 */
		DP_DEBUG("refer to stream head. channel map(%d).\n", ainfo->ch_num);
		break;
	}

	/* config to infoframe sdp, audio use bank 0 */
	DP_BIT_WRITE(hw, SDP_REGISTER_BANK_OFFSET(0),
			audio_infoframe_head, 0, 32);
	DP_BIT_WRITE(hw, SDP_REGISTER_BANK_OFFSET(1),
			audio_infoframe_data[0], 0, 32);
	DP_BIT_WRITE(hw, SDP_REGISTER_BANK_OFFSET(2),
			audio_infoframe_data[1], 0, 32);
	DP_BIT_WRITE(hw, SDP_REGISTER_BANK_OFFSET(3),
			audio_infoframe_data[2], 0, 32);

	/* print data0 value */
	DP_DEBUG("audio register bank0: 0x%x.\n", audio_infoframe_data[0]);
}

void dp_hw_ctrl_config_audio_info(struct dpu_hw_blk *hw,
		struct dp_audio_fmt_info *ainfo)
{
	int reg_val = 0;

	/* set audio interface
	 *     0: I2S
	 *     1: SPDIF
	 */
	DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0,
			AUDIO_INF_SELECT_SHIFT, AUDIO_INF_SELECT_LEN);

	/* set data in */
	switch (ainfo->ch_num) {
	case 1:
		/* channel number to ONE_CHANNEL */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0,
				NUM_CHANNELS_SHIFT, NUM_CHANNELS_LEN);
		/* active channels12 */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x1,
				AUDIO_DATA_IN_EN_SHIFT, AUDIO_DATA_IN_EN_LEN);
		break;
	case 2:
		/* channel number to TWO_CHANNEL */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 1,
				NUM_CHANNELS_SHIFT, NUM_CHANNELS_LEN);
		/* active channels12 */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x1,
				AUDIO_DATA_IN_EN_SHIFT, AUDIO_DATA_IN_EN_LEN);
		break;
	case 3:
	case 4:
		/* channel number to EIGHT_CHANNEL */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 2,
				NUM_CHANNELS_SHIFT, NUM_CHANNELS_LEN);
		/* active channles1234
		 * NOTE: channel 4 unuse when channel number is 3.
		 */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x3,
				AUDIO_DATA_IN_EN_SHIFT, AUDIO_DATA_IN_EN_LEN);
		break;
	case 5:
	case 6:
		/* channel number to EIGHT_CHANNEL */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 2,
				NUM_CHANNELS_SHIFT, NUM_CHANNELS_LEN);
		/* active channels123456
		 * NOTE: channel 6 unuse when channel number is 5.
		 */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x7,
				AUDIO_DATA_IN_EN_SHIFT, AUDIO_DATA_IN_EN_LEN);
		break;
	case 7:
	case 8:
		/* channel number to EIGHT_CHANNEL */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 2,
				NUM_CHANNELS_SHIFT, NUM_CHANNELS_LEN);
		/* active channels12345678
		 * NOTE: channel 8 unuse when channel number is 7.
		 */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0xF,
				AUDIO_DATA_IN_EN_SHIFT, AUDIO_DATA_IN_EN_LEN);
		break;
	default:
		DP_INFO("unsupported channel number(%d).\n", ainfo->ch_num);
		/* channel number to TWO_CHANNEL */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 1,
				NUM_CHANNELS_SHIFT, NUM_CHANNELS_LEN);
		/* active channles12 */
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x1,
				AUDIO_DATA_IN_EN_SHIFT, AUDIO_DATA_IN_EN_LEN);
		break;
	};

	/* set data width */
	switch (ainfo->bit_width) {
	case 16:
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x10,
				AUDIO_DATA_WIDTH_SHIFT, AUDIO_DATA_WIDTH_LEN);
		break;
	case 24:
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x18,
				AUDIO_DATA_WIDTH_SHIFT, AUDIO_DATA_WIDTH_LEN);
		break;
	default:
		DP_INFO("unsupport bit width(%d).\n", ainfo->bit_width);
		DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x10,
				AUDIO_DATA_WIDTH_SHIFT, AUDIO_DATA_WIDTH_LEN);
		break;
	}

	/* 64FS */
	DP_BIT_WRITE(hw, AUD_CONFIG_OFFSET1, 0x3,
			AUDIO_CLK_MULT_FS_SHIFT, AUDIO_CLK_MULT_FS_LEN);

	reg_val = DP_REG_READ(hw, AUD_CONFIG_OFFSET1);
	DP_DEBUG("audio config value(0x%x).\n", reg_val);

	/* config sdp infoframe */
	dp_hw_ctrl_config_audio_sdpinfo(hw, ainfo);
}

void dp_hw_ctrl_enable_audio_transfer(struct dpu_hw_blk *hw, bool enable)
{
	int reg_val = 0;

	/* enable vertical timestamp sdp */
	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, enable,
			EN_AUDIO_TIMESTAMP_SDP_SHIFT,
			EN_AUDIO_TIMESTAMP_SDP_LEN);
	/* enable horizontal timestamp sdp */
	DP_BIT_WRITE(hw, SDP_HORIZONTAL_CTRL_OFFSET, enable,
			EN_AUDIO_TIMESTAMP_SDP_SHIFT,
			EN_AUDIO_TIMESTAMP_SDP_LEN);

	/* enable vertical sdp */
	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, enable,
			EN_AUDIO_STREAM_SDP_SHIFT,
			EN_AUDIO_STREAM_SDP_LEN);
	/* enable horizontal sdp */
	DP_BIT_WRITE(hw, SDP_HORIZONTAL_CTRL_OFFSET, enable,
			EN_AUDIO_STREAM_SDP_SHIFT,
			EN_AUDIO_STREAM_SDP_LEN);

	/* enable horizontal sdp NO */
	DP_BIT_WRITE(hw, SDP_HORIZONTAL_CTRL_OFFSET, 0x1,
			EN_HORIZONTAL_SDP_N_SHIFT, EN_HORIZONTAL_SDP_N_LEN);

	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, 0x1,
			EN_VERTICAL_SDP_N_SHIFT, EN_VERTICAL_SDP_N_LEN);

	if (enable)
		DP_DEBUG("enable audio transfer.\n");
	else
		DP_DEBUG("disable audio transfer.\n");

	reg_val = DP_REG_READ(hw, SDP_VERTICAL_CTRL_OFFSET);
	DP_DEBUG("audio sdp vertical ctrl value(0x%x).\n", reg_val);
	reg_val = DP_REG_READ(hw, SDP_HORIZONTAL_CTRL_OFFSET);
	DP_DEBUG("audio sdp horizontal ctrl value(0x%x).\n", reg_val);
}

void dp_hw_ctrl_enable_fec(struct dpu_hw_blk *hw, bool enable)
{
	DP_BIT_WRITE(hw, CCTL_OFFSET, enable, ENABLE_FEC_SHIFT, ENABLE_FEC_LEN);
}

void dp_hw_ctrl_enable_enhance_frame_with_fec(struct dpu_hw_blk *hw, bool enable)
{
	DP_BIT_WRITE(hw, CCTL_OFFSET, enable,
			ENHANCE_FRAMING_WITH_FEC_EN_SHIFT,
			ENHANCE_FRAMING_WITH_FEC_EN_LEN);
}

/**
 * if @lane_count=LANE_COUNT2, then enable lane 0 and 1, and disable
 * lane 2 and 3
 */
void dp_hw_ctrl_enable_xmit(struct dpu_hw_blk *hw, enum dp_lane_count lane_count,
		bool enable)
{
	u32 field = enable ? BITS_MASK(0, lane_count) : 0;

	DP_BIT_WRITE(hw, PHYIF_CTRL_OFFSET, field,
			XMIT_ENABLE_SHIFT, XMIT_ENABLE_LEN);
}

const static u8 cus_pattern_pltpat[] = {
	0xE0, 0x83, 0x0F, 0x3E, 0xF8, 0xE0, 0x83, 0x0F, 0X3E, 0XF8,
};

void dp_hw_ctrl_set_80b_custom_pattern(struct dpu_hw_blk *hw,
		enum dp_custom_pattern cus_pattern, u8 cus_pattern_80b[10])
{
	const u8 *pattern;
	u32 value;

	if (cus_pattern == DP_CUSTOM_PATTERN_CUS && cus_pattern_80b == NULL) {
		DP_ERROR("custom pattern is NULL\n");
		return;
	}

	switch (cus_pattern) {
	case DP_CUSTOM_PATTERN_CUS:
		pattern = cus_pattern_80b;
		break;
	case DP_CUSTOM_PATTERN_PLTPAT:
		pattern = cus_pattern_pltpat;
		break;
	default:
		return;
	}

	value = 0;
	value = MERGE_BITS(value, pattern[0], 0, 8);
	value = MERGE_BITS(value, pattern[1], 8, 8);
	value = MERGE_BITS(value, pattern[2], 16, 8);
	value = MERGE_BITS(value, pattern[3] & 0x3f, 24, 6);
	DP_BIT_WRITE(hw, CUSTOMPAT_OFFSET0, value, CUSTOM80B_0_SHIFT, CUSTOM80B_0_LEN);

	value = 0;
	value = MERGE_BITS(value, pattern[3] >> 6, 0, 2);
	value = MERGE_BITS(value, pattern[4], 2, 8);
	value = MERGE_BITS(value, pattern[5], 10, 8);
	value = MERGE_BITS(value, pattern[6], 18, 8);
	value = MERGE_BITS(value, pattern[7] & 0xf, 26, 4);
	DP_BIT_WRITE(hw, CUSTOMPAT_OFFSET1, value, CUSTOM80B_1_SHIFT, CUSTOM80B_1_LEN);

	value = 0;
	value = MERGE_BITS(value, pattern[7] >> 4, 0, 4);
	value = MERGE_BITS(value, pattern[8], 4, 8);
	value = MERGE_BITS(value, pattern[9], 12, 8);
	DP_BIT_WRITE(hw, CUSTOMPAT_OFFSET2, value, CUSTOM80B_2_SHIFT, CUSTOM80B_2_LEN);
}

void dp_hw_ctrl_set_pattern(struct dpu_hw_blk *hw, enum dp_training_pattern pattern)
{
	bool scramble_en;

	DP_BIT_WRITE(hw, PHYIF_CTRL_OFFSET, pattern,
			TPS_SEL_SHIFT, TPS_SEL_LEN);

	scramble_en = (pattern == TPS_IDLE ||
			pattern == TPS_TPS4 ||
			pattern == TPS_SYMBOL_ERR_RATE ||
			pattern == TPS_CP2520_PAT_1 ||
			pattern == TPS_CP2520_PAT_2);
	DP_BIT_WRITE(hw, CCTL_OFFSET, !scramble_en,
			SCRAMBLE_DIS_SHIFT, SCRAMBLE_DIS_LEN);
}

void dp_hw_ctrl_disable_ssc(struct dpu_hw_blk *hw, bool disable)
{
	DP_BIT_WRITE(hw, PHYIF_CTRL_OFFSET, !!disable, SSC_DIS_SHIFT, SSC_DIS_LEN);
}

void dp_hw_ctrl_set_vswing_preemp(struct dpu_hw_blk *hw,
		enum dp_lane_count lane_count,
		u8 v_level[DP_LANE_COUNT_MAX], u8 p_level[DP_LANE_COUNT_MAX])
{
	u32 phy_tx_eq = 0;
	int i;

	for (i = 0; i < lane_count; i++) {
		phy_tx_eq = MERGE_BITS(phy_tx_eq, p_level[i],
				LANE0_TX_PREEMP_SHIFT(i), LANE0_TX_PREEMP_LEN(i));
		phy_tx_eq = MERGE_BITS(phy_tx_eq, v_level[i],
				LANE0_TX_VSWING_SHIFT(i), LANE0_TX_VSWING_LEN(i));
	}

	DP_REG_WRITE(hw, PHY_TX_EQ_OFFSET, phy_tx_eq);
}

static u32 dp_hw_ctrl_lane_count_to_hw_code(enum dp_lane_count lane_count)
{
	switch (lane_count) {
	case DP_LANE_COUNT_1:
		return PHY_LANE_CNT_1;
	case DP_LANE_COUNT_2:
		return PHY_LANE_CNT_2;
	case DP_LANE_COUNT_4:
		return PHY_LANE_CNT_4;
	default:
		return PHY_LANE_CNT_1;
	}
}

static u32 dp_hw_ctrl_link_rate_to_hw_code(enum dp_link_rate link_rate)
{
	switch (link_rate) {
	case DP_LINK_RATE_RBR:
		return PHY_RATE_RBR;
	case DP_LINK_RATE_HBR:
		return PHY_RATE_HBR;
	case DP_LINK_RATE_HBR2:
		return PHY_RATE_HBR2;
	case DP_LINK_RATE_HBR3:
		return PHY_RATE_HBR3;
	default:
		return PHY_RATE_RBR;
	}
}

void _dp_hw_ctrl_set_lane_count(struct dpu_hw_blk *hw, enum dp_lane_count lane_count)
{
	if (g_combo_mode == DP_COMBO_MODE_DP2)
		lane_count = min_t(int, lane_count, DP_LANE_COUNT_2);

	DP_BIT_WRITE(hw, PHYIF_CTRL_OFFSET,
			dp_hw_ctrl_lane_count_to_hw_code(lane_count),
			PHY_LANES_SHIFT, PHY_LANES_LEN);
}

void _dp_hw_ctrl_set_link_rate(struct dpu_hw_blk *hw, enum dp_link_rate link_rate)
{
	DP_BIT_WRITE(hw, PHYIF_CTRL_OFFSET,
			dp_hw_ctrl_link_rate_to_hw_code(link_rate),
			PHY_RATE_SHIFT, PHY_RATE_LEN);
}

void _dp_hw_ctrl_set_power_mode(struct dpu_hw_blk *hw, u8 power_mode)
{
	DP_BIT_WRITE(hw, PHYIF_CTRL_OFFSET, power_mode,
			PHY_POWERDOWN_SHIFT, PHY_POWERDOWN_LEN);
}

void dp_hw_ctrl_send_sdp(struct dpu_hw_blk *hw, struct dp_sdp_packet *sdp,
		u8 count)
{
	u32 i, j;

	/* sdp data */
	for (i = 0; i < count; i++) {
		DP_REG_WRITE(hw, SDP_REGISTER_BANK_OFFSET(i * 9), sdp[i].header);

		for (j = 0; j < DP_SDP_PAYLOAD32_SIZE; j++)
			DP_REG_WRITE(hw, SDP_REGISTER_BANK_OFFSET(i * 9 + j + 1),
					sdp[i].payload[j]);
	}

	/* sdp config */
	/* enable vertical sdp NO. */
	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, BITS_MASK(0, count),
			EN_VERTICAL_SDP_N_SHIFT, EN_VERTICAL_SDP_N_LEN);
}

void dp_hw_ctrl_disable_sdp(struct dpu_hw_blk *hw)
{
	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, 0,
			EN_VERTICAL_SDP_N_SHIFT, EN_VERTICAL_SDP_N_LEN);
	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, 0,
			EN_AUDIO_STREAM_SDP_SHIFT, EN_AUDIO_STREAM_SDP_LEN);
	DP_BIT_WRITE(hw, SDP_VERTICAL_CTRL_OFFSET, 0,
			EN_AUDIO_TIMESTAMP_SDP_SHIFT, EN_AUDIO_TIMESTAMP_SDP_LEN);

	DP_BIT_WRITE(hw, SDP_HORIZONTAL_CTRL_OFFSET, 0,
			EN_HORIZONTAL_SDP_N_SHIFT, EN_HORIZONTAL_SDP_N_LEN);
	DP_BIT_WRITE(hw, SDP_HORIZONTAL_CTRL_OFFSET, 0,
			EN_AUDIO_STREAM_SDP_SHIFT, EN_AUDIO_STREAM_SDP_LEN);
	DP_BIT_WRITE(hw, SDP_HORIZONTAL_CTRL_OFFSET, 0,
			EN_AUDIO_TIMESTAMP_SDP_SHIFT, EN_AUDIO_TIMESTAMP_SDP_LEN);
}
