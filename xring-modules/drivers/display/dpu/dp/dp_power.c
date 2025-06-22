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

#include <linux/slab.h>
#include "dp_power.h"
#include "dp_display.h"
#include "dpu_hw_common.h"

static int dp_power_on(struct dp_power *power)
{
	struct dp_hw_sctrl *hw_sctrl = power->hw_sctrl;
	struct dp_hw_ctrl *hw_ctrl = power->hw_ctrl;
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_DPU_DP_POWER_ENABLE,
			SMC_ID_POWER_ON, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0) {
		DP_ERROR("failed to power on dptx controller\n");
		return -EBUSY;
	}

	dp_hw_ctrl_reset_last_link_rate();

#ifdef ASIC
	hw_ctrl->ops->prepare_combo_mode(&hw_ctrl->hw, &hw_sctrl->hw);
#elif defined FPGA
	/* powerdown_rate_sync_bypass */
	hw_sctrl->ops->powerdown_rate_sync_bypass(&hw_sctrl->hw);
#endif

	power->state = DP_POWER_STATE_ON;
	DP_INFO("dptx hardware module is powered-on\n");

	return 0;
}

static void dp_power_off(struct dp_power *power)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(FID_BL31_DPU_DP_POWER_ENABLE,
			SMC_ID_POWER_OFF, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0)
		DP_ERROR("failed to power off dptx controller\n");

	power->state = DP_POWER_STATE_OFF;
	DP_INFO("dptx hardware module is powered-off\n");
}

static void dp_power_lpm_enable(struct dp_power *power)
{
	struct dp_hw_ctrl *hw_ctrl = power->hw_ctrl;
	struct dp_hw_sctrl *hw_sctrl = power->hw_sctrl;

	power->is_lpm_enable = true;

	hw_ctrl->ops->set_per_lane_power_mode(&hw_ctrl->hw, &hw_sctrl->hw,
			PHY_POWERDOWN_STATE_POWER_DOWN);
	(void)hw_sctrl->ops->pll_request(&hw_sctrl->hw, DP_MAXPCLKREQ_PLL_OFF_REF_CLK_OFF);
}

static void dp_power_lpm_disable(struct dp_power *power)
{
	struct dp_hw_ctrl *hw_ctrl = power->hw_ctrl;
	struct dp_hw_sctrl *hw_sctrl = power->hw_sctrl;

	(void)hw_sctrl->ops->pll_request(&hw_sctrl->hw, DP_MAXPCLKREQ_PLL_ON_REF_CLK_ON);
	hw_ctrl->ops->set_per_lane_power_mode(&hw_ctrl->hw, &hw_sctrl->hw,
			PHY_POWERDOWN_STATE_POWER_ON);

	power->is_lpm_enable = false;
}

static struct dp_power_funcs dp_power_funcs = {
	.on             = dp_power_on,
	.off            = dp_power_off,
	.lpm_enable     = dp_power_lpm_enable,
	.lpm_disable    = dp_power_lpm_disable,
};

int dp_power_init(struct dp_display *display, struct dp_power **power)
{
	struct dp_power *power_priv;

	power_priv = kzalloc(sizeof(*power_priv), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(power_priv))
		return -ENOMEM;

	power_priv->display    = display;
	power_priv->hw_sctrl   = display->hw_modules.hw_sctrl;
	power_priv->hw_ctrl    = display->hw_modules.hw_ctrl;
	power_priv->funcs      = &dp_power_funcs;
	power_priv->hw_hss1    = dp_hw_hss1_init();
	if (IS_ERR_OR_NULL(power_priv->hw_hss1)) {
		DP_ERROR("failed to init dp hw hss1\n");
		kfree(power_priv);
		return -ENOMEM;
	}

	*power = power_priv;

	return 0;
}

void dp_power_deinit(struct dp_power *power)
{
	if (power->state == DP_POWER_STATE_ON)
		dp_power_off(power);
	dp_hw_hss1_deinit(power->hw_hss1);
	kfree(power);
}
