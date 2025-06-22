/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _DP_HW_CTRL_OPS_H_
#define _DP_HW_CTRL_OPS_H_

#include "dp_hw_common.h"
#include "dp_hw_ctrl_reg.h"
#include "xring_dpu_color.h"

#define DP_AUX_REPLY_STATUS_ERR                BIT(0)
#define DP_AUX_REPLY_STATUS_TIMEOUT            BIT(1)
#define DP_AUX_REPLY_STATUS_DISCONNECTED       BIT(2)
#define DP_AUX_REPLY_STATUS_CMD_INVALID        BIT(3)

enum pixel_format {
	PIXEL_FORMAT_RGB               = BIT(0),
	PIXEL_FORMAT_YCBCR420          = BIT(1),
	PIXEL_FORMAT_YCBCR422          = BIT(2),
	PIXEL_FORMAT_YCBCR444          = BIT(3),
	PIXEL_FORMAT_YONLY             = BIT(4),
	PIXEL_FORMAT_RAW               = BIT(5)
};

enum dynamic_range_type {
	DYNAMIC_RANGE_CEA     = 1,
	DYNAMIC_RANGE_VESA    = 2,
};

/**
 * dp_aux_reply_status - aus reply status
 * @bytes_read: bytes size read in the reply
 * @reply_type: reply type, ACK / NACK / DEFER
 * @aux_m: byte size actually wrote to sink
 * @reply_received: if received a reply (if the reply status is valid)
 * @err_status: error status
 * @err_code: error code
 */
struct dp_aux_reply_status {
	u8 bytes_read;
	u8 reply_type;
	u8 aux_m;

	bool reply_received;
	u32 err_status;
	u8 err_code;
};

/**
 * dp_hw_ctrl_tu_config - transfer unit configuration
 * @valid_bytes: valid byte size in a transfer unit, integer part
 * @valid_bytes_frac: valid byte size in a transfer unit, fraction part, tenths unit
 */
struct dp_hw_ctrl_tu_config {
	u8 valid_bytes;
	u8 valid_bytes_frac;
};

struct dp_hw_ctrl_msa_config {
	enum dynamic_range_type dynamic_range;
	enum dp_colorimetry_space colorimetry;
	enum pixel_format pixel_format;
	enum dp_bpc bpc;
};

struct dp_hw_ctrl_video_config {
	bool dsc_en;
	bool enhance_frame_en;
	struct dpu_dsc_config *dsc_config;
	u32 link_clk_khz;

	/**
	 * this is the number of clock cycles that the link should wait for the
	 * pixels to accumulate at the beginning of every video line before it
	 * starts transmitting. This threshold influences the link capability to
	 * maintain the average valid symbols per TU.
	 */
	u8 init_threshold;

	struct dp_hw_ctrl_msa_config msa;
	struct dp_hw_ctrl_tu_config tu;
	struct dp_display_timing timing;
};

enum dp_training_pattern {
	TPS_IDLE                      = 0x0,
	TPS_TPS1                      = 0x1,
	TPS_TPS2                      = 0x2,
	TPS_TPS3                      = 0x3,
	TPS_TPS4                      = 0x4,
	TPS_SYMBOL_ERR_RATE           = 0x5,
	TPS_PRBS7                     = 0x6,
	TPS_CUSTOMPAT                 = 0x7,
	TPS_CP2520_PAT_1              = 0x8,
	TPS_CP2520_PAT_2              = 0x9,
};

enum dp_intr_type {
	DP_INTR_AUX   = BIT(AUX_CMD_INVALID_SHIFT) | BIT(AUX_REPLY_EVENT_SHIFT),
	DP_INTR_HPD   = BIT(HPD_EVENT_SHIFT),
	DP_INTR_DSC   = BIT(DSC_EVENT_SHIFT),
	DP_INTR_POWER = BIT(PM_EVENT_SHIFT),
	DP_INTR_VIDEO_FIFO_OVERFLOW_0 = BIT(VIDEO_FIFO_OVERFLOW_EN_STREAM0_SHIFT),
	DP_INTR_VIDEO_FIFO_UNDERFLOW_0 = BIT(VIDEO_FIFO_UNDERFLOW_EN_STREAM0_SHIFT),
	DP_INTR_AUDIO_FIFO_OVERFLOW_0 = BIT(AUDIO_FIFO_OVERFLOW_EN_STREAM0_SHIFT),
	DP_INTR_ALL   = 0xFFFFFFFF,
};

enum DPTX_MODULE {
	DPTX_MODULE_CONTROLLER            = BIT(CONTROLLER_RESET_SHIFT),
	DPTX_MODULE_PHY                   = BIT(PHY_SOFT_RESET_SHIFT),
	DPTX_MODULE_HDCP_MODULE           = BIT(HDCP_MODULE_RESET_SHIFT),
	DPTX_MODULE_AUX                   = BIT(AUX_RESET_SHIFT),
	DPTX_MODULE_VIDEO_STREAM0         = BIT(VIDEO_RESET_STREAM_SHIFT0),
	DPTX_MODULE_VIDEO_STREAM1         = BIT(VIDEO_RESET_STREAM_SHIFT1),
	DPTX_MODULE_VIDEO_STREAM2         = BIT(VIDEO_RESET_STREAM_SHIFT2),
	DPTX_MODULE_VIDEO_STREAM3         = BIT(VIDEO_RESET_STREAM_SHIFT3),
	DPTX_MODULE_AUDIO_SAMPLER_STREAM0 = BIT(AUDIO_SAMPLER_RESET_SHIFT),
	DPTX_MODULE_AUDIO_SAMPLER_STREAM1 = BIT(AUDIO_SAMPLER_RESET_STREAM_SHIFT1),
	DPTX_MODULE_AUDIO_SAMFLER_STREAM2 = BIT(AUDIO_SAMPLER_RESET_STREAM_SHIFT2),
	DPTX_MODULE_AUDIO_SAMPLER_STREAM3 = BIT(AUDIO_SAMPLER_RESET_STREAM_SHIFT3),
};

/* aux ops */
void dp_hw_ctrl_read_aux_data(struct dpu_hw_blk *hw, u8 *bytes, u8 len);
void dp_hw_ctrl_write_aux_data(struct dpu_hw_blk *hw, u8 *bytes, u8 len);
void dp_hw_ctrl_send_aux_request(struct dpu_hw_blk *hw, u8 request,
		u32 addr, u8 len);
void dp_hw_ctrl_get_aux_reply_status(struct dpu_hw_blk *hw,
		struct dp_aux_reply_status *status);

/* ctrl ops */
void _dp_hw_ctrl_reset_module(struct dpu_hw_blk *hw, u32 modules);
void dp_hw_ctrl_config_video(struct dpu_hw_blk *hw,
		struct dp_hw_ctrl_video_config *video_config);
void dp_hw_ctrl_enable_video_transfer(struct dpu_hw_blk *hw, bool enable);
void dp_hw_ctrl_config_audio_info(struct dpu_hw_blk *hw,
		struct dp_audio_fmt_info *ainfo);
void dp_hw_ctrl_enable_audio_transfer(struct dpu_hw_blk *hw, bool enable);
void dp_hw_ctrl_enable_dsc(struct dpu_hw_blk *hw, bool enable);
void dp_hw_ctrl_enable_intr(struct dpu_hw_blk *hw,
		enum dp_intr_type intr_type, bool enable);
bool dp_hw_ctrl_get_intr_state(struct dpu_hw_blk *hw, enum dp_intr_type intr_type);
void dp_hw_ctrl_clear_intr_state(struct dpu_hw_blk *hw, enum dp_intr_type intr_type);
void dp_hw_ctrl_default_config(struct dpu_hw_blk *hw);
void dp_hw_ctrl_disable_sdp(struct dpu_hw_blk *hw);
void dp_hw_ctrl_send_sdp(struct dpu_hw_blk *hw, struct dp_sdp_packet *sdp, u8 count);

/* phy ops */
void dp_hw_ctrl_set_vswing_preemp(struct dpu_hw_blk *hw,
		enum dp_lane_count lane_count,
		u8 v_level[DP_LANE_COUNT_MAX], u8 p_level[DP_LANE_COUNT_MAX]);
void dp_hw_ctrl_enable_xmit(struct dpu_hw_blk *hw,
		enum dp_lane_count lane_count, bool enable);
void dp_hw_ctrl_set_80b_custom_pattern(struct dpu_hw_blk *hw,
		enum dp_custom_pattern cus_pattern, u8 cus_pattern_80b[10]);
void dp_hw_ctrl_set_pattern(struct dpu_hw_blk *hw, enum dp_training_pattern pattern);
void dp_hw_ctrl_enable_per_lane_power_mode(struct dpu_hw_blk *hw);
void dp_hw_ctrl_disable_ssc(struct dpu_hw_blk *hw, bool disable);
void dp_hw_ctrl_enable_fec(struct dpu_hw_blk *hw, bool enable);
void dp_hw_ctrl_enable_enhance_frame_with_fec(struct dpu_hw_blk *hw, bool enable);
void _dp_hw_ctrl_set_power_mode(struct dpu_hw_blk *hw, u8 power_mode);
void _dp_hw_ctrl_set_lane_count(struct dpu_hw_blk *hw, enum dp_lane_count lane_count);
void _dp_hw_ctrl_set_link_rate(struct dpu_hw_blk *hw, enum dp_link_rate link_rate);
int _dp_hw_ctrl_wait_phy_busy(struct dpu_hw_blk *hw);

#endif /* _DP_HW_CTRL_OPS_H_ */
