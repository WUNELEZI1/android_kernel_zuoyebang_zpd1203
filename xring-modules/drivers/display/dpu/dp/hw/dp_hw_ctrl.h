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

#ifndef _DP_HW_CTRL_H_
#define _DP_HW_CTRL_H_

#include "dp_hw_ctrl_ops.h"
#include "dp_parser.h"

struct dp_hw_ctrl_ops {
	/* aux ops */
	/**
	 * read_aux_data - read data form aux data register
	 * @hw: hardware memory address
	 * @bytes: used to store read bytes
	 * @len: the length of read bytes, no more than 16
	 *
	 * this function will read received aux data from register, should be called
	 * after receiving an aux reply for a read request.
	 */
	void (*read_aux_data)(struct dpu_hw_blk *hw, u8 *bytes, u8 len);

	/**
	 * write_aux_data - write data to aux data register
	 * @hw: hardware memory address
	 * @bytes: bytes needed to be wrote
	 * @len: the length of wrote bytes, no more than 16
	 *
	 * this function will write data to register to transfer to sink device,
	 * should be called before sending an aux write request.
	 */
	void (*write_aux_data)(struct dpu_hw_blk *hw, u8 *bytes, u8 len);

	/**
	 * send_aux_request() - transmit an aux request immediately
	 * @hw: hardware memory address
	 * @request: 4bit, equal to comm31:28, bit3 indicate if I2C, bit2 indicates
	 * if MOT, bit1:0 indicate WRITE/READ/WRITE_STATUS_UPDATE
	 * @addr: sink address to read or write
	 * @len: the number of bytes need to be read or wrote
	 *
	 * if start a read request, the read data will be saved in AUX_DATA REG
	 * when received a reply; if start a write request, the sended data should
	 * be wrote in AUX_DATA REG before this function. The field of AUX_CMD is
	 * comm31:28 | addr27:8 | resv7:6 | aux_phy_wake5 | i2c_address_only4 | len3:0
	 * The function will sleep until time out or receiving a reply.
	 */
	void (*send_aux_request)(struct dpu_hw_blk *hw, u8 request, u32 addr, u8 len);

	/**
	 * get_aux_reply_status - get aux reply status
	 * @hw: hardware memory address
	 * @status: aux reply status
	 */
	void (*get_aux_reply_status)(struct dpu_hw_blk *hw,
			struct dp_aux_reply_status *status);

	/* ctrl ops */
	/**
	 * reset_module - reset dp module
	 * @ctrl_hw: hardware memory address of dptx controller
	 * @sctrl_hw: hardware memory address of dp sctrl
	 * @modules: dp modules, combination of DPTX_MODULE, eg
	 * DPTX_MODULE_CONTROLLER or
	 * DPTX_MODULE_CONTROLLER | DPTX_MODULE_PHY
	 */
	int (*reset_module)(struct dpu_hw_blk *ctrl_hw,
			struct dpu_hw_blk *sctrl_hw, u32 modules);
	/**
	 * config_video - configure video information
	 * @hw: hardware memory address
	 * @video_config: video configuration
	 */
	void (*config_video)(struct dpu_hw_blk *hw,
			struct dp_hw_ctrl_video_config *video_config);

	/**
	 * enable_video_transfer - enable video transmission
	 * @hw: hardware memory address
	 * @enable: true or false
	 */
	void (*enable_video_transfer)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * config_audio_info - configure audio information
	 * @hw: hardware memory address
	 * @ainfo: audio information
	 */
	void (*config_audio_info)(struct dpu_hw_blk *hw, struct dp_audio_fmt_info *ainfo);

	/**
	 * enable_audio_transfer - enable audio transmission
	 * @hw: hardware memory address
	 * @enable: true or false
	 */
	void (*enable_audio_transfer)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * enable_dsc - enable dsc function of dptx
	 * @hw: hardware memory address
	 * @enable: true or false
	 *
	 * dptx doesn`t compress video, but need to change the timing to transfer
	 * compressed data stream. dptx also transfer pps to sink for decoding.
	 */
	void (*enable_dsc)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * enable_intr - enable dptx interrupt
	 * @hw: hardware memory address
	 * @intr_type: interrupt type, such as aux, hpd, power and so on
	 * @enable: true or false
	 */
	void (*enable_intr)(struct dpu_hw_blk *hw, enum dp_intr_type intr_type,
			bool enable);

	/**
	 * get_intr_state - get interrupt state
	 * @hw: hardware memory address
	 * @intr_type: interrupt type, such as aux, hpd, power and so on
	 *
	 * return true if the interrupt of specified intr_type occurred, false if not
	 */
	bool (*get_intr_state)(struct dpu_hw_blk *hw, enum dp_intr_type intr_type);

	/**
	 * clear_intr_state - clear interrupt state
	 * @hw: hardware memory address
	 * @intr_type: interrupt type, such as aux, hpd, power and so on
	 */
	void (*clear_intr_state)(struct dpu_hw_blk *hw, enum dp_intr_type intr_type);

	/* phy ops */
	/**
	 * set_vswing_preemp - set phy voltage swing level and pre-emphasis level
	 * @hw: hardware memory address
	 * @lane_count: lane count
	 * @v_level: voltage swing level, range in [0, MAX_VOLTAGE_SWING_LEVEL]
	 * @p_level: pre-emphasis level, range in [0, MAX_PRE_EMPHASIS_LEVEL]
	 */
	void (*set_vswing_preemp)(struct dpu_hw_blk *hw,
			enum dp_lane_count lane_count,
			u8 v_level[DP_LANE_COUNT_MAX], u8 p_level[DP_LANE_COUNT_MAX]);

	/**
	 * enable_xmit - enable xmit hardware module
	 * @hw: hardware memory address
	 * @lane_count: lane count
	 * @enable: true or false
	 *
	 * xmit controls sending and receiving of data, if xmit is on, software can
	 * turn on/off video stream to send video stream or training pattern, if
	 * xmit is off, there is no data will be sended or received
	 */
	void (*enable_xmit)(struct dpu_hw_blk *hw, enum dp_lane_count lane_count,
			bool enable);

	/**
	 * set_lane_count_link_rate - set lane count and link rate of dptx
	 * @ctrl_hw: hardware memory address of dptx controller
	 * @sctrl_hw: hardware memory address of dp sctrl
	 * @msgbus_hw: hardware memory address of msgbus
	 * @lane_count: lane count
	 * @link_rate: link rate
	 */
	int (*set_lane_count_link_rate)(
			struct dpu_hw_blk *ctrl_hw, struct dpu_hw_blk *sctrl_hw,
			struct dpu_hw_blk *msgbus_hw, enum dp_lane_count lane_count,
			enum dp_link_rate link_rate);

	/**
	 * set_80b_custom_pattern - set 80 bit custom pattern
	 * @hw: hardware memory address
	 * @cus_pattern: type of custom pattern
	 * @cus_pattern_80b: custom pattern
	 *
	 * 80 bit custom pattern is really sended by set_pattern(hw, TPS_CUSTOMPAT)
	 */
	void (*set_80b_custom_pattern)(struct dpu_hw_blk *hw,
			enum dp_custom_pattern cus_pattern, u8 cus_pattern_80b[10]);

	/**
	 * set_pattern - set dptx training pattern
	 * @hw: hardware memory address
	 * @pattern: training pattern
	 */
	void (*set_pattern)(struct dpu_hw_blk *hw, enum dp_training_pattern pattern);

	/**
	 * set_per_lane_power_mode - set power mode of every single lane
	 * @ctrl_hw: hardware memory address of dptx controller
	 * @sctrl_hw: hardware memory address of dp sctrl
	 * @power_mode: power mode of every lane
	 */
	int (*set_per_lane_power_mode)(struct dpu_hw_blk *ctrl_hw,
			struct dpu_hw_blk *sctrl_hw, u8 power_mode);

	/**
	 * disable_ssc - disable spread spectrum clocking of phy
	 * @hw: hardware memory address
	 * @disable: true for disabling ssc
	 *
	 * if ssc is disabled in TX, DPCD 0x107h, bit 4 SPREAD_AMP also should be
	 * disabled.
	 */
	void (*disable_ssc)(struct dpu_hw_blk *hw, bool disable);

	/**
	 * enable_fec - enable fec function of dptx
	 * @hw: hardware memory address
	 * @enable: true or false
	 */
	void (*enable_fec)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * enable_enhance_frame_with_fec - enable enhance_frame_with_fec of dptx
	 * @hw: hardware memory address
	 * @enable: true or false
	 */
	void (*enable_enhance_frame_with_fec)(struct dpu_hw_blk *hw, bool enable);

	/**
	 * disable_fast_link_training - disable fast link training
	 * @hw: hardware memory address
	 */
	void (*default_config)(struct dpu_hw_blk *hw);

	/**
	 * prepare_combo_mode - hardware prepare operation for combo mode
	 * @ctrl_hw: hardware memory address of dptx controller
	 * @sctrl_hw: hardware memory address of dptx sctrl
	 */
	void (*prepare_combo_mode)(struct dpu_hw_blk *ctrl_hw,
			struct dpu_hw_blk *sctrl_hw);

	/**
	 * close_combo_mode - close combo mode
	 * @ctrl_hw: hardware memory address of dptx controller
	 * @sctrl_hw: hardware memory address of dptx sctrl
	 */
	void (*close_combo_mode)(struct dpu_hw_blk *ctrl_hw,
			struct dpu_hw_blk *sctrl_hw);

	/**
	 * disable_sdp - disable sdp packet transfer
	 * @hw: hardware memory address
	 */
	void (*disable_sdp)(struct dpu_hw_blk *hw);
};

/**
 * dp_hw_ctrl - handle of dptx controller hardware module
 * @hw: hardware memory address
 * @ops: dptx controller supported operations
 */
struct dp_hw_ctrl {
	struct dpu_hw_blk hw;

	struct dp_hw_ctrl_ops *ops;
};

void dp_hw_ctrl_reset_last_link_rate(void);

struct dp_hw_ctrl *dp_hw_ctrl_init(struct dp_blk_cap *cap);
void dp_hw_ctrl_deinit(struct dp_hw_ctrl *ctrl);

#endif
