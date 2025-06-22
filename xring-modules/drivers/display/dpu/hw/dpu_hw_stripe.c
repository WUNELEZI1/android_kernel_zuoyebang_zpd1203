// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024 XRing Technologies Co., Ltd.
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

#include "dpu_hw_stripe.h"
#include "dpu_hw_stripe_ops.h"

static void dpu_stripe_ops_init(struct dpu_hw_stripe_ops *ops)
{
	ops->layer_stripe_config = dpu_hw_stripe_rdma_layer_config;
	ops->layer_scaler_stripe_set = dpu_hw_stripe_pre_scaler_set;
	ops->cmps_wb_stripe_dst_config = dpu_hw_stripe_mixer_wb_input_config;
	ops->wb_frame_stripe_config = dpu_stripe_hw_wb_cfg_setup;
	ops->wb_scaler_stripe_set = dpu_hw_stripe_wb_scaler_set;
	ops->cmps_rch_layer_config = dpu_hw_stripe_mixer_rch_layer_config;
}

struct dpu_hw_stripe *dpu_stripe_res_init(void)
{
	struct dpu_hw_stripe *hw_stripe;

	hw_stripe = kzalloc(sizeof(*hw_stripe), GFP_KERNEL);
	if (!hw_stripe)
		return ERR_PTR(-ENOMEM);

	dpu_stripe_ops_init(&hw_stripe->ops);

	return hw_stripe;
}

void dpu_stripe_res_deinit(struct dpu_hw_stripe *hw_stripe)
{
	if (!hw_stripe) {
		PERF_ERROR("invalid parameter\n");
		return;
	}

	kfree(hw_stripe);
	hw_stripe = NULL;
}

