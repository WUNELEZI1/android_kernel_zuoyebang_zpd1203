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
#include "dp_hw_sctrl.h"

struct dp_hw_sctrl_ops hw_sctrl_ops = {
	.reset_state                = dp_hw_sctrl_reset_state,
	.clear_reset                = dp_hw_sctrl_clear_reset,
	.hpd_trigger                = dp_hw_sctrl_hpd_trigger,
	.enable_self_test           = dp_hw_sctrl_enable_self_test,
	.powerdown_rate_sync_bypass = dp_hw_sctrl_powerdown_rate_sync_bypass,
	.power_manage_ext_sdp       = dp_hw_sctrl_power_manage_ext_sdp,
	.send_ext_sdp               = dp_hw_sctrl_send_ext_sdp,
	.suspend_ext_sdp            = dp_hw_sctrl_suspend_ext_sdp,
	.aux_enable                 = dp_hw_sctrl_aux_enable,
	.pll_request                = dp_hw_sctrl_pll_request,
};

struct dp_hw_sctrl *dp_hw_sctrl_init(struct dp_blk_cap *cap)
{
	struct dp_hw_sctrl *hw_sctrl;

	hw_sctrl = kzalloc(sizeof(*hw_sctrl), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(hw_sctrl))
		return ERR_PTR(-ENOMEM);

	hw_sctrl->hw.iomem_base = cap->vaddr;
	hw_sctrl->hw.base_addr = cap->paddr;
	hw_sctrl->hw.blk_len = cap->len;
	hw_sctrl->ops = &hw_sctrl_ops;

	return hw_sctrl;
}

void dp_hw_sctrl_deinit(struct dp_hw_sctrl *hw_sctrl)
{
	kfree(hw_sctrl);
}
