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

#include <linux/slab.h>
#include "dp_hw_ctrl.h"
#include "dp_hw_sctrl.h"
#include "dp_hw_msgbus.h"

static enum dp_link_rate last_link_rate = DP_LINK_RATE_RBR;

void dp_hw_ctrl_reset_last_link_rate(void)
{
	last_link_rate = DP_LINK_RATE_RBR;
}

static int dp_hw_ctrl_set_per_lane_power_mode(struct dpu_hw_blk *ctrl_hw,
		struct dpu_hw_blk *sctrl_hw, u8 power_mode)
{
	int ret;

	/**
	 * 1, wait phy busy
	 * 2, set power mode
	 * 3, wait phy busy
	 */

#ifdef ASIC
	ret = _dp_hw_sctrl_wait_phy_status(sctrl_hw);
	if (ret)
		return ret;

	_dp_hw_ctrl_set_power_mode(ctrl_hw, power_mode);

	ret = _dp_hw_sctrl_wait_phy_status(sctrl_hw);
	if (ret)
		return ret;

#elif defined FPGA

	ret = _dp_hw_ctrl_wait_phy_busy(ctrl_hw);
	if (ret)
		return ret;

	_dp_hw_ctrl_set_power_mode(ctrl_hw, power_mode);

	ret = _dp_hw_ctrl_wait_phy_busy(ctrl_hw);
	if (ret)
		return ret;
#endif

	return 0;
}

static int dp_hw_ctrl_set_lane_count_link_rate(
		struct dpu_hw_blk *ctrl_hw, struct dpu_hw_blk *sctrl_hw,
		struct dpu_hw_blk *msgbus_hw, enum dp_lane_count lane_count,
		enum dp_link_rate link_rate)
{
	int ret;

	DP_INFO("set lane count %d, link rate %u\n", lane_count, link_rate);

	dp_hw_ctrl_enable_xmit(ctrl_hw, lane_count, false);

	/* close training pattern transmission, transmit idle pattern now */
	dp_hw_ctrl_set_pattern(ctrl_hw, TPS_IDLE);

	/**
	 * 1. set lane count
	 *
	 * if lane count = 1, sequence change to be
	 * a) set phyifctrl.lanecount = 2
	 * b) set phyifctrl.phyrate
	 * c) restart pll
	 * d) set phyifctrl.lanecount = 1
	 */
	_dp_hw_ctrl_set_lane_count(ctrl_hw,
			lane_count == DP_LANE_COUNT_1 ? DP_LANE_COUNT_2 : lane_count);

	/**
	 * We bypass link rate change if link rate is the same with last
	 * time, cause maxpclkchange_ok will not be asserted in such condition.
	 */
	if (last_link_rate != link_rate) {
		/* 2. change power mode to p2 */
		ret = dp_hw_ctrl_set_per_lane_power_mode(ctrl_hw, sctrl_hw,
				PHY_POWERDOWN_STATE_INTER_P2_POWER);
		if (ret) {
			DP_ERROR("failed to set lane power mode, ret %d\n", ret);
			return ret;
		}

		/* add delay here to avoid wait pclkchange_ok 1 timeout */
		DPU_MDELAY(5);

		/* 3. set phy rate */
		_dp_hw_ctrl_set_link_rate(ctrl_hw, link_rate);

#ifdef ASIC
		/* 4. restart combo phy PLL for rate change, if ASIC */
		ret = _dp_hw_sctrl_restart_combo_phy_pll(sctrl_hw);
		if (ret) {
			DP_ERROR("failed to restart combo phy pll for rate change\n");
			return ret;
		}
#endif
		last_link_rate = link_rate;

		/* 5. change power mode to power_on */
		ret = dp_hw_ctrl_set_per_lane_power_mode(ctrl_hw, sctrl_hw,
				PHY_POWERDOWN_STATE_POWER_ON);
		if (ret) {
			DP_ERROR("failed to set lane power mode, ret %d\n", ret);
			return ret;
		}
	}

	if (lane_count == DP_LANE_COUNT_1)
		_dp_hw_ctrl_set_lane_count(ctrl_hw, lane_count);

#ifdef ASIC
	/* close unused usb tx and reserve used usb tx */
	dp_hw_msgbus_reserve_usb_tx_lane(msgbus_hw, lane_count);
#endif

	dp_hw_ctrl_enable_xmit(ctrl_hw, lane_count, true);

	return 0;
}

static void dp_hw_ctrl_prepare_combo_mode(struct dpu_hw_blk *ctrl_hw,
		struct dpu_hw_blk *sctrl_hw)
{
	/**
	 * turn pll off will not influence combophy mode changing.
	 * here turn pll off to reduce dptx controller stand-by power consumption in
	 * case of only cable is plugged-in, but monitor is not connectted.
	 */
	dp_hw_sctrl_pll_request(sctrl_hw, DP_MAXPCLKREQ_PLL_OFF_REF_CLK_OFF);
#if !IS_ENABLED(CONFIG_SWITCH_FSA4480)
	_dp_hw_sctrl_set_aux_orientation(sctrl_hw);
#endif
	dp_hw_sctrl_aux_enable(sctrl_hw, false);
	_dp_hw_ctrl_set_lane_count(ctrl_hw, DP_LANE_COUNT_4);
}

static void dp_hw_ctrl_close_combo_mode(struct dpu_hw_blk *ctrl_hw,
		struct dpu_hw_blk *sctrl_hw)
{
	dp_hw_ctrl_set_per_lane_power_mode(ctrl_hw, sctrl_hw,
			PHY_POWERDOWN_STATE_POWER_DOWN);

	/* request to close pll */
	dp_hw_sctrl_pll_request(sctrl_hw, DP_MAXPCLKREQ_PLL_OFF_REF_CLK_OFF);

	/* confirm that combo mode is disabled */
	_dp_hw_sctrl_combo_mode_ack(sctrl_hw, DP_COMBO_MODE_ACK_DISABLE);
}

static int dp_hw_ctrl_reset_module(struct dpu_hw_blk *ctrl_hw,
		struct dpu_hw_blk *sctrl_hw, u32 modules)
{
	int ret = 0;

	_dp_hw_ctrl_reset_module(ctrl_hw,
			DPTX_MODULE_PHY | DPTX_MODULE_AUX | DPTX_MODULE_CONTROLLER);

#ifdef ASIC
	ret = _dp_hw_sctrl_wait_phy_status(sctrl_hw);
#elif defined FPGA
	ret = _dp_hw_ctrl_wait_phy_busy(ctrl_hw);
#endif

	return ret;
}

static struct dp_hw_ctrl_ops dp_hw_ctrl_ops = {
	/* aux ops */
	.read_aux_data              = dp_hw_ctrl_read_aux_data,
	.write_aux_data             = dp_hw_ctrl_write_aux_data,
	.send_aux_request           = dp_hw_ctrl_send_aux_request,
	.get_aux_reply_status       = dp_hw_ctrl_get_aux_reply_status,

	/* ctrl ops */
	.reset_module               = dp_hw_ctrl_reset_module,
	.config_video               = dp_hw_ctrl_config_video,
	.enable_video_transfer      = dp_hw_ctrl_enable_video_transfer,
	.config_audio_info          = dp_hw_ctrl_config_audio_info,
	.enable_audio_transfer      = dp_hw_ctrl_enable_audio_transfer,
	.enable_dsc                 = dp_hw_ctrl_enable_dsc,
	.enable_intr                = dp_hw_ctrl_enable_intr,
	.get_intr_state             = dp_hw_ctrl_get_intr_state,
	.clear_intr_state           = dp_hw_ctrl_clear_intr_state,
	.default_config             = dp_hw_ctrl_default_config,
	.disable_sdp                = dp_hw_ctrl_disable_sdp,

	/* phy ops */
	.set_vswing_preemp             = dp_hw_ctrl_set_vswing_preemp,
	.enable_xmit                   = dp_hw_ctrl_enable_xmit,
	.set_lane_count_link_rate      = dp_hw_ctrl_set_lane_count_link_rate,
	.set_80b_custom_pattern        = dp_hw_ctrl_set_80b_custom_pattern,
	.set_pattern                   = dp_hw_ctrl_set_pattern,
	.disable_ssc                   = dp_hw_ctrl_disable_ssc,
	.set_per_lane_power_mode       = dp_hw_ctrl_set_per_lane_power_mode,
	.enable_fec                    = dp_hw_ctrl_enable_fec,
	.enable_enhance_frame_with_fec = dp_hw_ctrl_enable_enhance_frame_with_fec,
	.close_combo_mode              = dp_hw_ctrl_close_combo_mode,
	.prepare_combo_mode            = dp_hw_ctrl_prepare_combo_mode,
};

struct dp_hw_ctrl *dp_hw_ctrl_init(struct dp_blk_cap *cap)
{
	struct dp_hw_ctrl *hw_ctrl;

	hw_ctrl = kzalloc(sizeof(*hw_ctrl), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_ctrl))
		return ERR_PTR(-ENOMEM);

	hw_ctrl->hw.iomem_base = cap->vaddr;
	hw_ctrl->hw.base_addr = cap->paddr;
	hw_ctrl->hw.blk_len = cap->len;
	hw_ctrl->ops = &dp_hw_ctrl_ops;

	return hw_ctrl;
}

void dp_hw_ctrl_deinit(struct dp_hw_ctrl *hw_ctrl)
{
	kfree(hw_ctrl);
}
