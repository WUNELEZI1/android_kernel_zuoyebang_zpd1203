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

#include <linux/ktime.h>
#include "dp_hw_sctrl_ops.h"
#include "dpu_hw_common.h"

#define DPTX_IPI_INPUT_DPU                                   0
#define DPTX_IPI_INPUT_VG                                    1
#define PCLK_CHANGE_OK_CHECK_DELAY_US                        100
#define PCLK_CHANGE_OK_CHECK_TIMEOUT_US                      1000000
#define PHY_STATUS_CHECK_DELAY_US                            10
#define PHY_STATUS_CHECK_TIMEOUT_US                          10000

enum dp_combo_mode g_combo_mode = DP_COMBO_MODE_DP4;
enum dp_orientation g_orientation = DP_ORIENTATION_OBSERVE;

void dp_hw_sctrl_set_g_combo_mode(enum dp_combo_mode combo_mode)
{
	if (combo_mode != DP_COMBO_MODE_DP4 && combo_mode != DP_COMBO_MODE_DP2) {
		DP_ERROR("invalid dp combo mode (%d)\n", combo_mode);
		return;
	}

	g_combo_mode = combo_mode;
}

void dp_hw_sctrl_set_g_orientation(enum dp_orientation orientation)
{
	if (orientation != DP_ORIENTATION_OBSERVE && orientation != DP_ORIENTATION_RESERVE) {
		DP_ERROR("invalid dp orientation (%d)\n", orientation);
		return;
	}

	g_orientation = orientation;
}

void _dp_hw_sctrl_combo_mode_ack(struct dpu_hw_blk *hw, bool ack)
{
	DP_BIT_WRITE(hw, DPTX_PHY_CFG_OFFSET, ack,
			DPALT_DISABLE_ACK_SHIFT, DPALT_DISABLE_ACK_LEN);
}

void dp_hw_sctrl_powerdown_rate_sync_bypass(struct dpu_hw_blk *hw)
{
	DP_REG_WRITE(hw, CFG_POWERDOWN_RATE_SYNC_BYPASS, 0x1);
}

void dp_hw_sctrl_clear_reset(struct dpu_hw_blk *hw)
{
	usleep_range(10, 20);

	DP_REG_WRITE(hw, EXT_SDP_TX_VG_SOFT_RESET_OFFSET,
			BIT(IP_RST_DPTX_CONTROLLER_N_SHIFT) |
			BIT(IP_RST_DPTX_PHY_LANE0_N_SHIFT) |
			BIT(IP_RST_DPTX_PHY_LANE1_N_SHIFT) |
			BIT(IP_RST_DPTX_PHY_LANE2_N_SHIFT) |
			BIT(IP_RST_DPTX_PHY_LANE3_N_SHIFT) |
			BIT(IP_RST_DPTX_PHY_N_SHIFT));
}

void inline dp_hw_sctrl_reset_state(struct dpu_hw_blk *hw)
{
	DP_REG_WRITE(hw, EXT_SDP_TX_VG_SOFT_RESET_OFFSET + 0x4, 0xFFFFFFFF);
}

static u8 dp_hw_sctrl_bpc_to_code(enum dp_bpc bpc)
{
	switch (bpc) {
	case DP_BPC_6:
		return 0x0;
	case DP_BPC_8:
		return 0x1;
	case DP_BPC_10:
		return 0x2;
	default:
		DP_INFO("unsupported bpc %d in vg, set default 8\n", bpc);
		return 0x1;
	}
}

static void dp_hw_sctrl_config_vg(struct dpu_hw_blk *hw,
		struct dp_hw_sctrl_vg_config *config, u8 mode_idx)
{
	struct dp_display_timing *timing = config->timing;
	enum dp_bpc bpc = config->bpc;
	u32 reg;

	/* configure DPTX_VG_CFG 0 - 3 */
	reg = 0;
	reg = MERGE_BITS(reg, timing->h_sync_width,
			CFG_DPTX_HSA_SHIFT, CFG_DPTX_HSA_LEN);
	reg = MERGE_BITS(reg, timing->h_back_porch,
			CFG_DPTX_HBP_SHIFT, CFG_DPTX_HBP_LEN);
	DP_REG_WRITE(hw, DPTX_VG_CONFIG_OFFSET0, reg);

	reg = 0;
	reg = MERGE_BITS(reg, timing->h_active,
			CFG_DPTX_HACT_SHIFT, CFG_DPTX_HACT_LEN);
	reg = MERGE_BITS(reg, timing->h_front_porch,
			CFG_DPTX_HFP_SHIFT, CFG_DPTX_HFP_LEN);
	DP_REG_WRITE(hw, DPTX_VG_CONFIG_OFFSET1, reg);

	reg = 0;
	reg = MERGE_BITS(reg, timing->v_sync_width,
			CFG_DPTX_VSA_SHIFT, CFG_DPTX_VSA_LEN);
	reg = MERGE_BITS(reg, timing->v_back_porch,
			CFG_DPTX_VBP_SHIFT, CFG_DPTX_VBP_LEN);
	DP_REG_WRITE(hw, DPTX_VG_CONFIG_OFFSET2, reg);

	reg = 0;
	reg = MERGE_BITS(reg, timing->v_active,
			CFG_DPTX_VACT_SHIFT, CFG_DPTX_VACT_LEN);
	reg = MERGE_BITS(reg, timing->v_front_porch,
			CFG_DPTX_VFP_SHIFT, CFG_DPTX_VFP_LEN);
	DP_REG_WRITE(hw, DPTX_VG_CONFIG_OFFSET3, reg);

	/* configure VG color depth and vg output mode */
	DP_BIT_WRITE(hw, DPTX_VG_CONFIG_OFFSET6, dp_hw_sctrl_bpc_to_code(bpc),
			CFG_DPTX_COLOR_DEPTH_SHIFT, CFG_DPTX_COLOR_DEPTH_LEN);
	DP_BIT_WRITE(hw, DPTX_VG_CONFIG_OFFSET6, mode_idx,
			CFG_DPTX_DPMODE_SHIFT, CFG_DPTX_DPMODE_LEN);
}

void dp_hw_sctrl_enable_self_test(struct dpu_hw_blk *hw,
		struct dp_hw_sctrl_vg_config *config, u32 mode_idx, bool enable)
{
	if (enable) {
		if (mode_idx < 1 || mode_idx > 4) {
			DP_ERROR("vg self test idx is out of range (1-4), idx:%d\n", mode_idx);
			return;
		}
		DP_INFO("vg mode idx:%d, enable:%d\n", mode_idx, enable);

		/* clear reset state */
		DP_REG_WRITE(hw, EXT_SDP_TX_VG_SOFT_RESET_OFFSET,
				BIT(IP_RST_DPTX_VG_N_SHIFT));
		usleep_range(10, 20);
		/* turn on clock */
		DP_REG_WRITE(hw, CLK_GATE_EN_OFFSET, BIT(GT_CLK_DPTX_PIXEL_VG_SHIFT));
		/* input source: vg */
		DP_BIT_WRITE(hw, DP_MUX_SEL_CFG_OFFSET, DPTX_IPI_INPUT_VG,
				CFG_DPTX_IPI_MUX_SEL_SHIFT, CFG_DPTX_IPI_MUX_SEL_LEN);
		dp_hw_sctrl_config_vg(hw, config, mode_idx - 1);
		/* vg enable */
		DP_BIT_WRITE(hw, DPTX_VG_CONFIG_OFFSET6, 1,
				CFG_DPTX_IVG_EN_SHIFT, CFG_DPTX_IVG_EN_LEN);
	} else {
		/* turn off clock */
		DP_REG_WRITE(hw, CLK_GATE_EN_OFFSET + 0x4, BIT(GT_CLK_DPTX_PIXEL_VG_SHIFT));
		/* input source: ipi */
		DP_BIT_WRITE(hw, DP_MUX_SEL_CFG_OFFSET, DPTX_IPI_INPUT_DPU,
				CFG_DPTX_IPI_MUX_SEL_SHIFT, CFG_DPTX_IPI_MUX_SEL_LEN);
		/* enter reset state */
		DP_REG_WRITE(hw, EXT_SDP_TX_VG_SOFT_RESET_OFFSET + 0x4,
				BIT(IP_RST_DPTX_VG_N_SHIFT));
	}
}

void dp_hw_sctrl_hpd_trigger(struct dpu_hw_blk *hw, bool high)
{
	DP_REG_WRITE(hw, DPTX_HPD_CFG_OFFSET, high);
}

void dp_hw_sctrl_aux_enable(struct dpu_hw_blk *hw, bool enable)
{
	/* aux hys tune */
	DP_BIT_WRITE(hw, DPTX_AUX_CFG_OFFSET, 0x1,
			DP_AUX_HYS_TUNE_SHIFT, DP_AUX_HYS_TUNE_LEN);
	/* aux vod tune */
	DP_BIT_WRITE(hw, DPTX_AUX_CFG_OFFSET, 0x1,
			DP_AUX_VOD_TUNE_SHIFT, DP_AUX_VOD_TUNE_LEN);
	/* aux ctrl */
	DP_BIT_WRITE(hw, DPTX_AUX_CFG_OFFSET, 0x1,
			DP_AUX_CTRL_SHIFT, DP_AUX_CTRL_LEN);
	/* aux pwdnb */
	DP_BIT_WRITE(hw, DPTX_AUX_CFG_OFFSET, !!enable,
			DP_AUX_PWDNB_SHIFT, DP_AUX_PWDNB_LEN);
}

void _dp_hw_sctrl_set_aux_orientation(struct dpu_hw_blk *hw)
{
	DP_BIT_WRITE(hw, DPTX_AUX_CFG_OFFSET,
			(g_orientation == DP_ORIENTATION_OBSERVE) ? 0 : 1,
			DP_AUX_DP_DN_SWAP_SHIFT, DP_AUX_DP_DN_SWAP_LEN);
}

int dp_hw_sctrl_pll_request(struct dpu_hw_blk *hw, u8 req)
{
	u32 max_pclk_req_shift, max_pclk_req_len;
	u32 max_pclk_ack_shift, max_pclk_ack_len;
	u32 value;
	int ret;

	if (g_combo_mode == DP_COMBO_MODE_DP4) {
		max_pclk_req_shift = PIPE_LANE0_MAXPCLKREQ_SHIFT;
		max_pclk_req_len = 4;
		max_pclk_ack_shift = PIPE_LANE0_MAXPCLKACK_SHIFT;
		max_pclk_ack_len = 4;
	} else {
		max_pclk_req_shift = PIPE_LANE1_MAXPCLKREQ_SHIFT;
		max_pclk_req_len = 2;
		max_pclk_ack_shift = PIPE_LANE1_MAXPCLKACK_SHIFT;
		max_pclk_ack_len = 2;
	}

	value = (g_combo_mode == DP_COMBO_MODE_DP4) ? ((req << 2) | req) : req;

	/* set max pclk req */
	DP_BIT_WRITE(hw, DPTX_PHY_CFG_OFFSET, value,
		max_pclk_req_shift, max_pclk_req_len);

	/* wait max pclk ack */
	ret = DP_READ_POLL_TIMEOUT(hw, COMBO_PHY_OBSERVE_OFFSET, value,
			GET_BITS_VAL(value, max_pclk_ack_shift, max_pclk_ack_len) ==
			(g_combo_mode == DP_COMBO_MODE_DP4 ? ((req << 2) | req) : req),
			PCLK_CHANGE_OK_CHECK_DELAY_US, PCLK_CHANGE_OK_CHECK_TIMEOUT_US);
	if (ret < 0) {
		DP_WARN("time out in waiting max_pclk_ack 0x%x\n", req);
		return -EBUSY;
	}

	return 0;
}

int _dp_hw_sctrl_restart_combo_phy_pll(struct dpu_hw_blk *hw)
{
	u32 pclk_change_ok_shift, pclk_change_ok_len;
	u32 pclk_change_ack_shift, pclk_change_ack_len;
	u32 value = 0;
	int ret;

	if (g_combo_mode == DP_COMBO_MODE_DP4) {
		pclk_change_ok_shift = PIPE_LANE0_PCLKCHANGEOK_SHIFT;
		pclk_change_ok_len = 2;
		pclk_change_ack_shift = PIPE_LANE0_PCLKCHANGEACK_SHIFT;
		pclk_change_ack_len = 2;
	} else {
		pclk_change_ok_shift = PIPE_LANE1_PCLKCHANGEOK_SHIFT;
		pclk_change_ok_len = 1;
		pclk_change_ack_shift = PIPE_LANE1_PCLKCHANGEACK_SHIFT;
		pclk_change_ack_len = 1;
	}

	/* wait pclkchangeok */
	ret = DP_READ_POLL_TIMEOUT(hw, COMBO_PHY_OBSERVE_OFFSET, value,
			GET_BITS_VAL(value, pclk_change_ok_shift, pclk_change_ok_len) ==
			(g_combo_mode == DP_COMBO_MODE_DP4 ? 0x3 : 0x1),
			PCLK_CHANGE_OK_CHECK_DELAY_US, PCLK_CHANGE_OK_CHECK_TIMEOUT_US);
	if (ret < 0) {
		DP_ERROR("time out in waiting pclk_change_ok 1 \n");
		return -EBUSY;
	}

	DPU_MDELAY(2);

	/* set maxpclkreq and wait maxpclkack, set 0b10 */
	ret = dp_hw_sctrl_pll_request(hw, DP_MAXPCLKREQ_PLL_OFF_REF_CLK_ON);
	if (ret < 0) {
		DP_ERROR("time out in closing pll\n");
		return -EBUSY;
	}

	DPU_MDELAY(2);

	/* set maxpclkreq and wait maxpclkack, set 0b11 */
	ret = dp_hw_sctrl_pll_request(hw, DP_MAXPCLKREQ_PLL_ON_REF_CLK_ON);
	if (ret < 0) {
		DP_ERROR("time out in opening pll\n");
		return -EBUSY;
	}

	DPU_MDELAY(2);

	/* set pclkchangeack, set 0b1 */
	DP_BIT_WRITE(hw, DPTX_PHY_CFG_OFFSET,
			(g_combo_mode == DP_COMBO_MODE_DP4) ? 0x3 : 0x1,
			pclk_change_ack_shift, pclk_change_ack_len);

	/* wait pclkchangeok, wait for 0b0 */
	ret = DP_READ_POLL_TIMEOUT(hw, COMBO_PHY_OBSERVE_OFFSET, value,
			GET_BITS_VAL(value, pclk_change_ok_shift, pclk_change_ok_len) == 0,
			PCLK_CHANGE_OK_CHECK_DELAY_US, PCLK_CHANGE_OK_CHECK_TIMEOUT_US);
	if (ret < 0) {
		DP_ERROR("time out in waiting pclk_change_ok 2 \n");
		return -EBUSY;
	}

	DPU_MDELAY(2);

	/* set pclkchangeack, set 0b0 */
	DP_BIT_WRITE(hw, DPTX_PHY_CFG_OFFSET, 0,
			pclk_change_ack_shift, pclk_change_ack_len);

	return 0;
}

int _dp_hw_sctrl_wait_phy_status(struct dpu_hw_blk *hw)
{
	u32 value = 0;
	u32 phy_status_mask;
	int ret;

	phy_status_mask = BITS_MASK(DPTX_PHY_LANE0_PHYSTATUS_SHIFT,
			g_combo_mode == DP_COMBO_MODE_DP4 ? 4 : 2);

	ret = DP_READ_POLL_TIMEOUT(hw, COMBO_PHY_OBSERVE_OFFSET,
			value, (value & phy_status_mask) == 0,
			PHY_STATUS_CHECK_DELAY_US, PHY_STATUS_CHECK_TIMEOUT_US);
	if (ret < 0) {
		DP_ERROR("phy status busy 0x%x, ret %d\n", value, ret);
		return -EBUSY;
	}

	return 0;
}

void dp_hw_sctrl_power_manage_ext_sdp(struct dpu_hw_blk *hw, bool enable)
{
	if (enable) {
		/* clear reset state */
		DP_REG_WRITE(hw, EXT_SDP_TX_VG_SOFT_RESET_OFFSET,
				BIT(IP_RST_DPTX_EXT_SDP_TX_N_SHIFT));
		/* turn on clock */
		DP_REG_WRITE(hw, CLK_GATE_EN_OFFSET, BIT(GT_CLK_DPTX_EXT_SDP_TX_SHIFT));
		/* turn on ext sdp interrupt */
		DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET0, 0x1);
	} else {
		/* turn off ext sdp interrupt */
		DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET0, 0x0);
		/* turn off clock */
		DP_REG_WRITE(hw, CLK_GATE_EN_OFFSET + 0x4, BIT(GT_CLK_DPTX_EXT_SDP_TX_SHIFT));
		/* enter reset state */
		DP_REG_WRITE(hw, EXT_SDP_TX_VG_SOFT_RESET_OFFSET + 0x4,
				BIT(IP_RST_DPTX_EXT_SDP_TX_N_SHIFT));
	}
}

static u32 EXT_SDP_DATA_OFFSET[] = {
	EXT_SDP_DATA1_OFFSET,
	EXT_SDP_DATA2_OFFSET,
	EXT_SDP_DATA3_OFFSET,
	EXT_SDP_DATA4_OFFSET
};

void dp_hw_sctrl_send_ext_sdp(struct dpu_hw_blk *hw, struct dp_sdp_packet *sdp,
		u8 count)
{
	u8 i, j;

	if (count == 0 || count > 4 || sdp == NULL)
		return;

	/**
	 * hardware restriction:
	 * line number starts from vsync, can not be vtotal - vfp.
	 * displayport spec requires that metadata should be sended in
	 * [vsync, vact_start - prepared_time], so line number would not be
	 * vtotal - vfp, which is vact_end.
	 *
	 * here we fix line number to be 1, 2, 3, 4
	 */

	/* tx line number */
	DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET2, 0x40003);
	DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET3, 0x20001);

	/* set all sdp 32Byte, set all sdp sended in vertical blanking */
	DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET4, 0x0);

	for (i = 0; i < count; i++) {
		DP_REG_WRITE(hw, EXT_SDP_DATA_OFFSET[i] + 8 * 0x04, sdp[i].header);

		for (j = 0; j < DP_SDP_PAYLOAD32_SIZE; j++)
			DP_REG_WRITE(hw, EXT_SDP_DATA_OFFSET[i] + (7 - j) * 0x04, sdp[i].payload[j]);
	}

	/* sdp enable */
	DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET1, BITS_MASK(0, count));
}

void dp_hw_sctrl_suspend_ext_sdp(struct dpu_hw_blk *hw)
{
	DP_REG_WRITE(hw, EXT_SDP_CFG_OFFSET1, 0x0);
}
