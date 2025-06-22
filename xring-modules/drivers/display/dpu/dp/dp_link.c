// SPDX-License-Identifier: GPL-2.0
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

#include "dp_link.h"
#include "dp_display.h"

#define DP_TRAIN_SET_SIZE                                5

/* numerical link rate and link clock */
#define LINK_RATE_NUMERICAL_HBR3                         8100
#define LINK_CLK_NUMERICAL_HBR3                          202500
#define LINK_RATE_NUMERICAL_HBR2                         5400
#define LINK_CLK_NUMERICAL_HBR2                          135000
#define LINK_RATE_NUMERICAL_HBR                          2700
#define LINK_CLK_NUMERICAL_HBR                           67500
#define LINK_RATE_NUMERICAL_RBR                          1620
#define LINK_CLK_NUMERICAL_RBR                           40500

#define DP_LINK_TRAINING_CR_RETRY_CNT                    10
#define DP_LINK_TRAINING_CR_NOT_UPDATED_CNT              5
#define DP_LINK_TRAINING_EQ_RETRY_CNT                    5

#define DP_FEC_STATUS_CLEARED                            0x3

/**
 * pre main post adjusted table
 *
 * dimension 0：link rate, 0 for RBR, 1 for HBR, 2 for HBR2, 3 for HBR3
 * dimension 1: swing level
 * dimension 2: preemp level
 * dimension 3: adjusted signal type, 0 for pre, 1 for main, 2 for post
 *
 * eg: SIGNAL_TABLE[2][2][1][1] means value of main at swing leven 2, preemp
 * level 1 and link rate HBR2
 */
static u32 DEFAULT_SIGNAL_TABLE[4][4][4][3] = {
	{/* RBR */
		{ /* sw level = 0 */
			{0, 32, 0}, {0, 40, 8}, {0, 48, 16}, {0, 64, 32},
		},
		{ /* sw level = 1 */
			{0, 48, 0}, {0, 60, 12}, {0, 72, 24}, {0, 0, 0},
		},
		{ /* sw level = 2 */
			{0, 64, 0}, {0, 80, 16}, {0, 0, 0}, {0, 0, 0},
		},
		{ /* sw level = 3 */
			{0, 96, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
		},
	},
	{/* HBR */
		{ /* sw level = 0 */
			{0, 32, 0}, {0, 40, 8}, {0, 48, 16}, {0, 64, 32},
		},
		{ /* sw level = 1 */
			{0, 48, 0}, {0, 60, 12}, {0, 72, 24}, {0, 0, 0},
		},
		{ /* sw level = 2 */
			{0, 64, 0}, {0, 80, 16}, {0, 0, 0}, {0, 0, 0},
		},
		{ /* sw level = 3 */
			{0, 96, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
		},
	},
	{/* HBR2 */
		{ /* sw level = 0 */
			{0, 32, 0},  {0, 38, 6},  {0, 44, 12},  {0, 54, 22},
		},
		{ /* sw level = 1 */
			{0, 48, 0}, {0, 58, 10}, {0, 66, 18}, {0, 0, 0},
		},
		{ /* sw level = 2 */
			{0, 64, 0}, {0, 76, 12}, {0, 0, 0}, {0, 0, 0},
		},
		{ /* sw level = 3 */
			{0, 96, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
		},
	},
	{/* HBR3 */
		{ /* sw level = 0 */
			{0, 32, 0},  {0, 38, 6},  {0, 44, 12},  {0, 54, 22},
		},
		{ /* sw level = 1 */
			{0, 48, 0}, {0, 58, 10}, {0, 66, 18}, {0, 0, 0},
		},
		{ /* sw level = 2 */
			{0, 64, 0}, {0, 76, 12}, {0, 0, 0}, {0, 0, 0},
		},
		{ /* sw level = 3 */
			{0, 96, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
		},
	},
};

static struct dp_link_phy_param DEFAULT_PARAM = {
#ifdef ASIC
	.init_link_rate = DP_LINK_RATE_HBR3,
#elif defined FPGA
	.init_link_rate = DP_LINK_RATE_RBR,
#endif
	.init_lane_count = DP_LANE_COUNT_MAX,

	.init_preemp_level = {0, 0, 0, 0},
	.init_swing_level = {0, 0, 0, 0},

	.fec_en = true,
	.ssc_en = true,
	.enhance_frame_en = true,

	.eq_pattern = TPS_TPS2,

	.signal_table = DEFAULT_SIGNAL_TABLE,
};

void dp_link_reset_to_default_param(struct dp_link *link)
{
	memcpy(&link->phy_param, &DEFAULT_PARAM, sizeof(link->phy_param));
}

void dp_link_set_default_param(const char *name, void *value)
{
	if (name == NULL || value == NULL) {
		DP_ERROR("Invalid pointer of param name %p or value %p\n", name, value);
		return;
	}

	if (strncmp("init_link_rate", name, strlen(name)) == 0)
		DEFAULT_PARAM.init_link_rate = *((int *)value);
	else if (strncmp("init_lane_count", name, strlen(name)) == 0)
		DEFAULT_PARAM.init_lane_count = *((int *)value);
	else if (strncmp("init_preemp_level", name, strlen(name)) == 0)
		memset(DEFAULT_PARAM.init_preemp_level, *((int *)value), DP_LANE_COUNT_MAX);
	else if (strncmp("init_swing_level", name, strlen(name)) == 0)
		memset(DEFAULT_PARAM.init_swing_level, *((int *)value), DP_LANE_COUNT_MAX);
	else if (strncmp("fec_en", name, strlen(name)) == 0)
		DEFAULT_PARAM.fec_en = *((bool *)value);
	else if (strncmp("ssc_en", name, strlen(name)) == 0)
		DEFAULT_PARAM.ssc_en = *((bool *)value);
	else if (strncmp("enhance_frame_en", name, strlen(name)) == 0)
		DEFAULT_PARAM.enhance_frame_en = *((bool *)value);
	else
		DP_ERROR("Invalid param name %s\n", name);
}

void dp_link_set_default_signal_table(u8 link_rate, u8 swing_level, u8 preemp_level,
		u32 pre, u32 main, u32 post)
{
	DEFAULT_SIGNAL_TABLE[link_rate][swing_level][preemp_level][0] = pre;
	DEFAULT_SIGNAL_TABLE[link_rate][swing_level][preemp_level][1] = main;
	DEFAULT_SIGNAL_TABLE[link_rate][swing_level][preemp_level][2] = post;

	DP_INFO("pre/main/post of link rate %d, swing level %d, preemp level %d set to %d/%d/%d\n",
			link_rate, swing_level, preemp_level, pre, main, post);
}

struct dp_link_phy_param *dp_link_get_default_param(void)
{
	return &DEFAULT_PARAM;
}

u8 dp_link_rate_to_dptx_code(u8 dpcd_bw)
{
	switch (dpcd_bw) {
	case DP_LINK_BW_1_62:
		return DP_LINK_RATE_RBR;
	case DP_LINK_BW_2_7:
		return DP_LINK_RATE_HBR;
	case DP_LINK_BW_5_4:
		return DP_LINK_RATE_HBR2;
	case DP_LINK_BW_8_1:
		return DP_LINK_RATE_HBR3;
	default:
		DP_ERROR("invalid dpcd bw 0x%x\n", dpcd_bw);
		return DP_LINK_RATE_RBR;
	}
}

static u8 dp_link_rate_to_dpcd_code(enum dp_link_rate rate)
{
	switch (rate) {
	case DP_LINK_RATE_RBR:
		return DP_LINK_BW_1_62;
	case DP_LINK_RATE_HBR:
		return DP_LINK_BW_2_7;
	case DP_LINK_RATE_HBR2:
		return DP_LINK_BW_5_4;
	case DP_LINK_RATE_HBR3:
		return DP_LINK_BW_8_1;
	default:
		DP_ERROR("Invalid rate 0x%x\n", rate);
		return DP_LINK_BW_1_62;
	}
}

/* the returned code contains of dpcd training pattern and scramble flag */
u8 dp_link_pattern_to_dpcd_code(enum dp_training_pattern pattern)
{
	switch (pattern) {
	case TPS_IDLE:
		return DP_TRAINING_PATTERN_DISABLE;
	case TPS_TPS1:
		return DP_TRAINING_PATTERN_1;
	case TPS_TPS2:
		return DP_TRAINING_PATTERN_2;
	case TPS_TPS3:
		return DP_TRAINING_PATTERN_3;
	case TPS_TPS4:
		return DP_TRAINING_PATTERN_4;
	case TPS_SYMBOL_ERR_RATE:
		return DP_LINK_QUAL_PATTERN_ERROR_RATE;
	case TPS_PRBS7:
		return DP_LINK_QUAL_PATTERN_PRBS7;
	case TPS_CUSTOMPAT:
		return DP_LINK_QUAL_PATTERN_80BIT_CUSTOM;
	case TPS_CP2520_PAT_1:
		return DP_LINK_QUAL_PATTERN_CP2520_PAT_1;
	case TPS_CP2520_PAT_2:
		return DP_LINK_QUAL_PATTERN_CP2520_PAT_2;
	default:
		return 0x0;
	}
}

u8 dp_link_pattern_to_dptx_code(u8 pattern)
{
	switch (pattern) {
	case DP_LINK_QUAL_PATTERN_DISABLE:
		return TPS_IDLE;
	case DP_LINK_QUAL_PATTERN_D10_2:
		return TPS_TPS1;
	case DP_LINK_QUAL_PATTERN_ERROR_RATE:
		return TPS_SYMBOL_ERR_RATE;
	case DP_LINK_QUAL_PATTERN_PRBS7:
		return TPS_PRBS7;
	case DP_LINK_QUAL_PATTERN_80BIT_CUSTOM:
		return TPS_CUSTOMPAT;
	case DP_LINK_QUAL_PATTERN_CP2520_PAT_1:
		return TPS_CP2520_PAT_1;
	case DP_LINK_QUAL_PATTERN_CP2520_PAT_2:
		return TPS_CP2520_PAT_2;
	case DP_LINK_QUAL_PATTERN_CP2520_PAT_3:
		return TPS_TPS4;
	default:
		return 0x0;
	}
}

/**
 * translate rate_code to real numerical link rate and link clock.
 * the unit of link rate is Mbps, the unit of link clock is Khz
 */
void dp_link_get_numerical_rate_and_clk(int rate_code,
		int *link_rate, int *link_clk)
{
	switch (rate_code) {
	case DP_LINK_RATE_RBR:
		*link_rate = LINK_RATE_NUMERICAL_RBR;
		*link_clk = LINK_CLK_NUMERICAL_RBR;
		break;
	case DP_LINK_RATE_HBR:
		*link_rate = LINK_RATE_NUMERICAL_HBR;
		*link_clk = LINK_CLK_NUMERICAL_HBR;
		break;
	case DP_LINK_RATE_HBR2:
		*link_rate = LINK_RATE_NUMERICAL_HBR2;
		*link_clk = LINK_CLK_NUMERICAL_HBR2;
		break;
	case DP_LINK_RATE_HBR3:
		*link_rate = LINK_RATE_NUMERICAL_HBR3;
		*link_clk = LINK_CLK_NUMERICAL_HBR3;
		break;
	default:
		*link_rate = LINK_RATE_NUMERICAL_RBR;
		*link_clk = LINK_CLK_NUMERICAL_RBR;
	}
}

static int dp_link_update_link_status(struct dp_link *link)
{
	if (drm_dp_dpcd_read_link_status(&link->aux->base, link->link_status) <
			DP_LINK_STATUS_SIZE) {
		DP_DEBUG("failed to read link status\n");
		return -EPROTO;
	}

	return 0;
}

static void __maybe_unused dp_link_dump_link_status(struct dp_link *link)
{
	u8 i;

	DP_DEBUG("DPCD link status:\n");
	for (i = 0; i < DP_LINK_STATUS_SIZE; i++)
		DP_DEBUG("dpcd addr:0x%04x, value:0x%02x\n",
				DP_LANE0_1_STATUS + i, link->link_status[i]);
}

bool dp_link_check_link_status(struct dp_link *link)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;

	if (drm_dp_dpcd_read_link_status(&link->aux->base, link->link_status) <
			DP_LINK_STATUS_SIZE) {
		DP_WARN("failed to read link status\n");
		return false;
	}

	if (!drm_dp_clock_recovery_ok(link->link_status, phy_status->lane_count)) {
		DP_WARN("clock_recovery is not done\n");
		return false;
	}

	if (!drm_dp_channel_eq_ok(link->link_status, phy_status->lane_count)) {
		DP_WARN("channel_eq is not done\n");
		return false;
	}

	return true;
}

static void dp_link_enable_fec(struct dp_link *link)
{
	struct dp_hw_ctrl *hw_ctrl = link->hw_ctrl;
	struct drm_dp_aux *aux = &link->aux->base;
	int i = 0, max_retries = 3;
	bool fec_valid = false;
	u8 fec_status = 0;

	drm_dp_dpcd_writeb(aux, DP_FEC_STATUS, DP_FEC_STATUS_CLEARED);
	/**
	 * Accord to DP spec, FEC enable sequence can first
	 * be transmitted anytime after 1000 LL codes have
	 * been transmitted on the link after link training
	 * completion. Using 1 lane RBR should have the maximum
	 * time for transmitting 1000 LL codes which is 6.173 us.
	 */
	DPU_UDELAY(10);

	if (drm_dp_dpcd_writeb(aux, DP_FEC_CONFIGURATION, 0x1) < 1) {
		DP_ERROR("failed to set dpcd fec_ready\n");
		goto error;
	}

	/* Need to try to enable multiple times due to BS symbols collisions */
	for (i = 0; i < max_retries; i++) {

		hw_ctrl->ops->enable_fec(&hw_ctrl->hw, false);
		DPU_UDELAY(10);
		hw_ctrl->ops->enable_fec(&hw_ctrl->hw, true);

		/* wait for controller to start fec sequence */
		DPU_UDELAY(900);

		/* read back FEC status and check if it is enabled */
		drm_dp_dpcd_readb(aux, DP_FEC_STATUS, &fec_status);
		if (fec_status & DP_FEC_DECODE_EN_DETECTED) {
			fec_valid = true;
			break;
		}
	}

	if (fec_valid) {
		link->phy_status.fec_en = true;
		DP_INFO("DP FEC is enabled\n");
		return;
	}

error:
	link->phy_status.fec_en = false;
	drm_dp_dpcd_writeb(aux, DP_FEC_CONFIGURATION, 0x0);
	hw_ctrl->ops->enable_fec(&hw_ctrl->hw, false);
	hw_ctrl->ops->enable_enhance_frame_with_fec(&hw_ctrl->hw, false);
	DP_INFO("retries %d to enable sink fec, sink fec take effected: %d\n",
			i, fec_valid);
	DP_INFO("DP FEC is disabled\n");
}

static int dp_link_training_init(struct dp_link *link)
{
	struct dp_link_phy_param *phy_param = &link->phy_param;
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct dp_hw_ctrl *hw_ctrl = link->hw_ctrl;
	struct drm_dp_aux *aux = &link->aux->base;

	if (drm_dp_read_dpcd_caps(aux, link->dpcd_caps)) {
		DP_ERROR("failed to read dpcd caps\n");
		goto link_conn_err;
	}

	if (phy_param->fec_en) {
		hw_ctrl->ops->enable_enhance_frame_with_fec(&hw_ctrl->hw, true);
		if (drm_dp_dpcd_writeb(aux, DP_FEC_CONFIGURATION, 0x1) < 1) {
			DP_ERROR("failed to set dpcd fec_ready\n");
			goto link_conn_err;
		}
	}

	hw_ctrl->ops->disable_ssc(&hw_ctrl->hw, !phy_param->ssc_en);
	/* set down spread (regular phy to be 1， rocket io phy to be 0) */
	if (drm_dp_dpcd_writeb(aux, DP_DOWNSPREAD_CTRL,
			phy_param->ssc_en ? DP_SPREAD_AMP_0_5 : 0) < 1) {
		DP_ERROR("failed to set dpcd down spread\n");
		goto link_conn_err;
	}

	/* set MAIN_LINK_CHANNEL_CODING to be 8b/10b，01h */
	if (drm_dp_dpcd_writeb(aux, DP_MAIN_LINK_CHANNEL_CODING_SET,
			DP_SET_ANSI_8B10B) < 1) {
		DP_ERROR("failed to set dpcd link channel coding\n");
		goto link_conn_err;
	}

	/* link training initial lane count and link rate */
	phy_status->lane_count = phy_param->init_lane_count;
	phy_status->link_rate = phy_param->init_link_rate;

	hw_ctrl->ops->enable_xmit(&hw_ctrl->hw, phy_status->lane_count, true);

	return 0;

link_conn_err:
	return -EPROTO;
}

static int dp_link_training_completed(struct dp_link *link, bool success)
{
	struct dp_link_phy_param *phy_param = &link->phy_param;
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct dp_hw_ctrl *hw_ctrl = link->hw_ctrl;
	struct dp_hw_sctrl *hw_sctrl = link->hw_sctrl;

	/* set idle pattern to end up link training */
	if (drm_dp_dpcd_writeb(&link->aux->base, DP_TRAINING_PATTERN_SET,
			dp_link_pattern_to_dpcd_code(TPS_IDLE)) < 1) {
		DP_ERROR("failed to set dpcd training pattern\n");
		goto exit;
	}
	/* idle pattern = tps none and video stream transfer false */
	hw_ctrl->ops->enable_video_transfer(&hw_ctrl->hw, false);
	hw_ctrl->ops->set_pattern(&hw_ctrl->hw, TPS_IDLE);

	if (!success)
		goto exit;

	if (phy_param->fec_en)
		dp_link_enable_fec(link);
	else
		phy_status->fec_en = false;

	dp_link_get_numerical_rate_and_clk(phy_status->link_rate,
			&phy_status->link_rate_mbps, &phy_status->link_clk_khz);

	DP_INFO("*******************************************************************\n");
	DP_INFO("*  link training is successful! lane count:%d, link rate:%8s  *\n",
			phy_status->lane_count, to_link_rate_str(phy_status->link_rate));
	DP_INFO("*******************************************************************\n");

	return 0;

exit:
	hw_ctrl->ops->enable_xmit(&hw_ctrl->hw, DP_LANE_COUNT_MAX, false);
	hw_ctrl->ops->set_per_lane_power_mode(&hw_ctrl->hw, &hw_sctrl->hw,
			PHY_POWERDOWN_STATE_POWER_DOWN);

	DP_ERROR("********************************\n");
	DP_ERROR("*  link training is failed!!!  *\n");
	DP_ERROR("********************************\n");

	return -EPROTO;
}

static int dp_link_set_lane_count_link_rate(struct dp_link *link)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct dp_link_phy_param *phy_param = &link->phy_param;
	struct drm_dp_aux *aux = &link->aux->base;
	struct dp_hw_ctrl *hw_ctrl = link->hw_ctrl;
	struct dp_hw_sctrl *hw_sctrl = link->hw_sctrl;
	struct dp_hw_msgbus *hw_msgbus = link->hw_msgbus;
	u8 lane_count_set = 0;
	int ret;

	/* sink setting */
	/* change training pattern to none before changing rate */
	if (drm_dp_dpcd_writeb(aux, DP_TRAINING_PATTERN_SET,
			dp_link_pattern_to_dpcd_code(TPS_IDLE)) < 1) {
		DP_ERROR("failed to set dpcd training pattern\n");
		goto link_conn_err;
	}

	/* set dpcd lane count */
	lane_count_set |= phy_status->lane_count;
	if (phy_param->enhance_frame_en)
		lane_count_set |= DP_LANE_COUNT_ENHANCED_FRAME_EN;

	if (drm_dp_dpcd_writeb(aux, DP_LANE_COUNT_SET, lane_count_set) < 1) {
		DP_ERROR("failed to set dpcd lane count\n");
		goto link_conn_err;
	}

	/* set dpcd link rate */
	if (drm_dp_dpcd_writeb(aux, DP_LINK_BW_SET,
			dp_link_rate_to_dpcd_code(phy_status->link_rate)) < 1) {
		DP_ERROR("failed to set dpcd link rate\n");
		goto link_conn_err;
	}

	/* dptx setting */
	ret = hw_ctrl->ops->set_lane_count_link_rate(&hw_ctrl->hw, &hw_sctrl->hw,
			&hw_msgbus->hw, phy_status->lane_count, phy_status->link_rate);
	if (ret)
		return ret;

	return 0;

link_conn_err:
	return -EPROTO;
}

static int dp_link_training_set_pattern(struct dp_link *link,
		const enum dp_training_pattern pattern)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct dp_hw_ctrl *hw_ctrl = link->hw_ctrl;
	u8 train_set[DP_TRAIN_SET_SIZE] = {0};
	u32 i = 0;

	/* dptx setting*/
	dp_link_set_vswing_preemp(link);

	hw_ctrl->ops->enable_video_transfer(&hw_ctrl->hw, false);
	hw_ctrl->ops->set_pattern(&hw_ctrl->hw, pattern);

	/* sink setting */
	train_set[0] = dp_link_pattern_to_dpcd_code(pattern);

	for (i = 0; i < phy_status->lane_count; i++) {
		u8 train_set_lanex = 0;

		train_set_lanex |= (phy_status->swing_level[i] << DP_TRAIN_VOLTAGE_SWING_SHIFT);
		if (phy_status->swing_level[i] == MAX_VOLTAGE_SWING_LEVEL)
			train_set_lanex |= DP_TRAIN_MAX_SWING_REACHED;

		train_set_lanex |= (phy_status->preemp_level[i] << DP_TRAIN_PRE_EMPHASIS_SHIFT);
		if (phy_status->preemp_level[i] == MAX_PRE_EMPHASIS_LEVEL)
			train_set_lanex |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

		train_set[i + 1] = train_set_lanex;
	}

	/**
	 * write 5 bytes contains of tps, scramble, vswing, preemp
	 * in a single aux transaction.
	 */
	if (drm_dp_dpcd_write(&link->aux->base, DP_TRAINING_PATTERN_SET,
			train_set, DP_TRAIN_SET_SIZE) < DP_TRAIN_SET_SIZE) {
		DP_ERROR("failed to write DP_TRAINING_LANE0_SET\n");
		return -EPROTO;
	}

	return 0;
}

static void dp_link_get_adjusted_vswing_preemp(struct dp_link *link,
		bool *is_updated)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;
	u8 swing_level[DP_LANE_COUNT_MAX] = {0};
	u8 preemp_level[DP_LANE_COUNT_MAX] = {0};
	int i;

	*is_updated = false;

	for (i = 0; i < DP_LANE_COUNT_MAX; i++) {
		swing_level[i] = drm_dp_get_adjust_request_voltage(link->link_status, i);
		preemp_level[i] = drm_dp_get_adjust_request_pre_emphasis(link->link_status, i) >>
				DP_TRAIN_PRE_EMPHASIS_SHIFT;

		if (swing_level[i] > MAX_VOLTAGE_SWING_LEVEL) {
			DP_WARN("adjust voltage is out of support, max %d, adjust %d\n",
					MAX_VOLTAGE_SWING_LEVEL, swing_level[i]);
			swing_level[i] = MAX_VOLTAGE_SWING_LEVEL;
		}

		if (preemp_level[i] > MAX_PRE_EMPHASIS_LEVEL) {
			DP_WARN("adjust pre emp is out of support, max %d, adjust %d\n",
					MAX_PRE_EMPHASIS_LEVEL, preemp_level[i]);
			preemp_level[i] = MAX_PRE_EMPHASIS_LEVEL;
		}

		if (preemp_level[i] != phy_status->swing_level[i] ||
				preemp_level[i] != phy_status->preemp_level[i])
			*is_updated = true;
	}

	memcpy(phy_status->swing_level, swing_level, DP_LANE_COUNT_MAX);
	memcpy(phy_status->preemp_level, preemp_level, DP_LANE_COUNT_MAX);
}

static int dp_link_reduce_link_rate(struct dp_link *link)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;

	switch (phy_status->link_rate) {
	case DP_LINK_RATE_HBR3:
		phy_status->link_rate = DP_LINK_RATE_HBR2;
		break;
	case DP_LINK_RATE_HBR2:
		phy_status->link_rate = DP_LINK_RATE_HBR;
		break;
	case DP_LINK_RATE_HBR:
		phy_status->link_rate = DP_LINK_RATE_RBR;
		break;
	case DP_LINK_RATE_RBR:
		DP_DEBUG("already RBR\n");
		return -EINVAL;
	}

	return 0;
}

static int dp_link_reduce_lane_count(struct dp_link *link)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;

	switch (phy_status->lane_count) {
	case DP_LANE_COUNT_4:
		phy_status->lane_count = DP_LANE_COUNT_2;
		break;
	case DP_LANE_COUNT_2:
		phy_status->lane_count = DP_LANE_COUNT_1;
		break;
	case DP_LANE_COUNT_1:
		DP_DEBUG("already 1 lane\n");
		return -EINVAL;
	}

	return 0;
}

void dp_link_set_vswing_preemp(struct dp_link *link)
{
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct dp_link_phy_param *phy_param = &link->phy_param;
	struct dp_hw_msgbus *hw_msgbus;
	struct dp_hw_ctrl *hw_ctrl;

	hw_msgbus = link->hw_msgbus;
	hw_ctrl = link->hw_ctrl;

	DP_DEBUG("swing level: %d %d %d %d\n",
			phy_status->swing_level[0],
			phy_status->swing_level[1],
			phy_status->swing_level[2],
			phy_status->swing_level[3]);
	DP_DEBUG("preemp level: %d %d %d %d\n",
			phy_status->preemp_level[0],
			phy_status->preemp_level[1],
			phy_status->preemp_level[2],
			phy_status->preemp_level[3]);

#ifdef ASIC
	hw_msgbus->ops->set_vswing_preemp(
			&hw_msgbus->hw,
			phy_param->signal_table[phy_status->link_rate],
			phy_status->swing_level,
			phy_status->preemp_level);
#elif defined FPGA
	hw_ctrl->ops->set_vswing_preemp(
			&hw_ctrl->hw,
			phy_status->lane_count,
			phy_status->swing_level,
			phy_status->preemp_level);
#endif
}

static int dp_link_training_cr(struct dp_link *link)
{
	struct dp_link_phy_param *phy_param = &link->phy_param;
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct drm_dp_aux *aux = &link->aux->base;
	u8 retry = 0, not_updated = 0;
	bool is_updated;
	int ret;

	memcpy(phy_status->swing_level, phy_param->init_swing_level, DP_LANE_COUNT_MAX);
	memcpy(phy_status->preemp_level, phy_param->init_preemp_level, DP_LANE_COUNT_MAX);

	if (dp_link_set_lane_count_link_rate(link))
		goto error;

	do {
		if (dp_link_training_set_pattern(link, TPS_TPS1))
			goto error;

		drm_dp_link_train_clock_recovery_delay(aux, link->dpcd_caps);

		if (dp_link_update_link_status(link))
			goto error;

		if (drm_dp_clock_recovery_ok(link->link_status, phy_status->lane_count))
			goto success;

		dp_link_get_adjusted_vswing_preemp(link, &is_updated);

		retry++;
		not_updated = is_updated ? 0 : not_updated + 1;
	} while (retry < DP_LINK_TRAINING_CR_RETRY_CNT &&
			not_updated < DP_LINK_TRAINING_CR_NOT_UPDATED_CNT);

	ret = dp_link_reduce_link_rate(link);
	if (ret == 0)
		goto again;

	ret = dp_link_reduce_lane_count(link);
	if (ret == 0) {
		phy_status->link_rate = phy_param->init_link_rate;
		goto again;
	}

error:
	return -EPROTO;
again:
	return -EAGAIN;
success:
	return 0;
}

static int dp_link_training_eq(struct dp_link *link)
{
	struct dp_link_phy_param *phy_param = &link->phy_param;
	struct dp_link_phy_status *phy_status = &link->phy_status;
	struct drm_dp_aux *aux = &link->aux->base;
	bool is_updated;
	u8 retry = 0;
	int ret;

	do {
		if (dp_link_training_set_pattern(link, phy_param->eq_pattern))
			goto error;

		drm_dp_link_train_channel_eq_delay(aux, link->dpcd_caps);

		if (dp_link_update_link_status(link))
			goto error;

		if (!drm_dp_clock_recovery_ok(link->link_status, phy_status->lane_count))
			break;

		if (drm_dp_channel_eq_ok(link->link_status, phy_status->lane_count))
			goto success;

		dp_link_get_adjusted_vswing_preemp(link, &is_updated);

		retry++;
	} while (retry < DP_LINK_TRAINING_EQ_RETRY_CNT);

	ret = dp_link_reduce_link_rate(link);
	if (ret == 0)
		goto again;

	ret = dp_link_reduce_lane_count(link);
	if (ret == 0) {
		phy_status->link_rate = phy_param->init_link_rate;
		goto again;
	}

error:
	return -EPROTO;
again:
	return -EAGAIN;
success:
	return 0;
}

static void dp_link_dump_irq_vector(struct dp_link *link)
{
	struct dp_link_irq_vector *vec = &link->irq_vector;
	int i;

	DP_DEBUG("irq vector: %hhu\n", vec->irq_vector);

	if (!vec->auto_test_flag)
		goto dump_hdcp;
	DP_INFO("auto test flag: %hhu\n", vec->auto_test_flag);

	DP_INFO("test training flag: %hhu\n", vec->auto_test.test_training_flag);
	if (vec->auto_test.test_training_flag) {
		DP_INFO("test lane count: %hhu\n", vec->auto_test.test_lane_count);
		DP_INFO("test link rate: %s\n", to_link_rate_str(vec->auto_test.test_link_rate));
	}

	DP_INFO("test phy pattern flag: %hhu\n", vec->auto_test.test_pattern_flag);
	if (vec->auto_test.test_pattern_flag) {
		DP_INFO("test phy pattern: %hhu\n", vec->auto_test.test_pattern);
		if (vec->auto_test.test_pattern == TPS_CUSTOMPAT) {
			for (i = 0; i < 10; i++)
				DP_INFO("bit%d_%d: 0x%02x ",
						i * 8 + 7, i * 8, vec->auto_test.test_cus_pattern_80b[i]);
		}
	}

dump_hdcp:
	if (!vec->cp_irq_flag)
		return;
	DP_INFO("cp irq flag: %hhu\n", vec->cp_irq_flag);
}

static int dp_link_parser_sink_irq_vector(struct dp_link *link)
{
	struct drm_dp_aux *aux = &link->aux->base;
	struct dp_link_irq_vector *vec = &link->irq_vector;
	u8 value;

	/* clear previous irq vector */
	memset(&link->irq_vector, 0, sizeof(link->irq_vector));

	/* raw irq vector */
	if (drm_dp_dpcd_readb(aux, DP_DEVICE_SERVICE_IRQ_VECTOR, &value) < 1) {
		DP_WARN("failed to read dpcd irq_vector\n");
		return -EIO;
	}
	vec->irq_vector = value;

	/* auto test flag */
	vec->auto_test_flag = !!(value & DP_AUTOMATED_TEST_REQUEST);
	if (!vec->auto_test_flag)
		goto parser_hdcp;

	if (drm_dp_dpcd_readb(aux, DP_TEST_REQUEST, &value) < 1) {
		DP_WARN("failed to read dpcd test_request\n");
		return -EIO;
	}
	vec->auto_test.test_training_flag = !!(value & DP_TEST_LINK_TRAINING);
	vec->auto_test.test_pattern_flag = !!(value & DP_TEST_LINK_PHY_TEST_PATTERN);

	/* auto test req -> test training */
	if (vec->auto_test.test_training_flag) {
		if (drm_dp_dpcd_readb(aux, DP_TEST_LINK_RATE, &value) < 1) {
			DP_WARN("failed to read dpcd test_link_rate\n");
			return -EIO;
		}
		vec->auto_test.test_link_rate = dp_link_rate_to_dptx_code(value);

		if (drm_dp_dpcd_readb(aux, DP_TEST_LANE_COUNT, &value) < 1) {
			DP_WARN("failed to read dpcd test_lane_count\n");
			return -EIO;
		}
		vec->auto_test.test_lane_count = value & BITS_MASK(0, 5);
	}

	/* auto test req -> test pattern */
	if (vec->auto_test.test_pattern_flag) {
		bool tmp;

		if (drm_dp_dpcd_readb(aux, DP_PHY_TEST_PATTERN, &value) < 1) {
			DP_WARN("failed to read dpcd test_pattern\n");
			return -EIO;
		}
		vec->auto_test.test_pattern = dp_link_pattern_to_dptx_code(
				value & BITS_MASK(0, 3));

		if (vec->auto_test.test_pattern == TPS_CUSTOMPAT) {
			if (drm_dp_dpcd_read(aux, DP_TEST_80BIT_CUSTOM_PATTERN_7_0,
					vec->auto_test.test_cus_pattern_80b, 10) < 10) {
				DP_WARN("failed to read dpcd test_80bit_custom_pattern\n");
				return -EIO;
			}
		}

		if (dp_link_update_link_status(link))
			return -EIO;

		dp_link_get_adjusted_vswing_preemp(link, &tmp);
	}

parser_hdcp:
	vec->cp_irq_flag = vec->irq_vector & DP_CP_IRQ;

	/* clear sink irq vector */
	drm_dp_dpcd_writeb(aux, DP_DEVICE_SERVICE_IRQ_VECTOR, 0xff);
	dp_link_dump_irq_vector(link);

	return 0;
}

struct dp_link_funcs link_funcs = {
	.training_init                   = dp_link_training_init,
	.training_cr                     = dp_link_training_cr,
	.training_eq                     = dp_link_training_eq,
	.training_completed              = dp_link_training_completed,
	.dump_link_status                = dp_link_dump_link_status,
	.check_link_status               = dp_link_check_link_status,
	.parser_sink_irq_vector          = dp_link_parser_sink_irq_vector,
};

int dp_link_init(struct dp_aux *aux, struct dp_hw_ctrl *hw_ctrl,
		struct dp_hw_sctrl *hw_sctrl, struct dp_hw_msgbus *hw_msgbus,
		struct dp_link **link)
{
	struct dp_link *link_priv;

	link_priv = kzalloc(sizeof(*link_priv), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(link_priv))
		return -ENOMEM;

	link_priv->aux       = aux;
	link_priv->hw_ctrl   = hw_ctrl;
	link_priv->hw_sctrl  = hw_sctrl;
	link_priv->hw_msgbus = hw_msgbus;
	link_priv->funcs     = &link_funcs;

	*link = link_priv;

	return 0;
}

void dp_link_deinit(struct dp_link *link)
{
	kfree(link);
}
