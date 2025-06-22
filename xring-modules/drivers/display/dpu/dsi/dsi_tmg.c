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

#include "hw/dsi_version_ctrl.h"
#include "dsi_tmg.h"
#include "dpu_log.h"

int dsi_tmg_frame_update(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", tmg, cfg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;
	if (tmg->tmg_hw.ops.frame_update) {
		tmg->tmg_hw.ops.frame_update(&tmg_hw->hw_blk, cfg);
	} else {
		DSI_ERROR("tmg hw frame_update function not implement\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_id_get(struct dsi_tmg *tmg)
{
	if (!tmg) {
		DSI_ERROR("invalid parameter\n");
		return -EINVAL;
	}

	return tmg->tmg_hw.hw_blk.blk_id;
}

int dsi_tmg_ulps_enter(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg)
{
	struct dsi_tmg_hw *tmg_hw;
	int ret = 0;

	if (!tmg || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", tmg, cfg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg->tmg_hw.ops.ulps_enter) {
		ret = tmg->tmg_hw.ops.ulps_enter(&tmg_hw->hw_blk, cfg);
		if (ret) {
			DSI_ERROR("failed to process ulps enter\n");
			return ret;
		}
	} else {
		DSI_ERROR("tmg hw ulps_enter function not implement\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_ulps_exit(struct dsi_tmg *tmg, u8 transfer_type)
{
	struct dsi_tmg_hw *tmg_hw;
	int ret = 0;

	if (!tmg) {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg->tmg_hw.ops.ulps_exit) {
		ret = tmg->tmg_hw.ops.ulps_exit(&tmg_hw->hw_blk, transfer_type);
		if (ret) {
			DSI_ERROR("failed to process ulps exit\n");
			return ret;
		}
	} else {
		DSI_ERROR("tmg hw ulps_exit function not implement\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_partial_update(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", tmg, cfg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.partial_update)
		tmg_hw->ops.partial_update(&tmg_hw->hw_blk, cfg);
	else {
		DSI_ERROR("tmg hw partial update function not implement\n");
		return -EINVAL;
	}

	return 0;
}

struct dsi_tmg *dsi_tmg_create(struct platform_device *pdev,
		struct device_node *np)
{
	struct dsi_tmg *tmg;
	int ret;

	tmg = kzalloc(sizeof(*tmg), GFP_KERNEL);
	if (!tmg)
		return ERR_PTR(-ENOMEM);

	ret = dsi_tmg_hw_parse(pdev, np, &tmg->tmg_hw);
	if (ret) {
		DSI_ERROR("parse tmg hw info failed\n");
		goto error;
	}

	ret = dsi_tmg_version_ctrl(&tmg->tmg_hw);
	if (ret)
		goto error;

	return tmg;

error:
	kfree(tmg);
	return NULL;
}

inline void dsi_tmg_destroy(struct dsi_tmg *tmg)
{
	kfree(tmg);
}

int dsi_tmg_enable(struct dsi_tmg *tmg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg) {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.enable) {
		tmg_hw->ops.enable(&tmg_hw->hw_blk);
	} else {
		DSI_ERROR("invalid parameters, %pK\n", tmg_hw);
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_disable(struct dsi_tmg *tmg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg) {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.disable) {
		tmg_hw->ops.disable(&tmg_hw->hw_blk);
	} else {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_hw_init(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", tmg, cfg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.init)
		tmg_hw->ops.init(&tmg_hw->hw_blk, cfg);
	else {
		DSI_ERROR("tmg hw init function not implement\n");
		return -EINVAL;
	}

	return 0;
}

u64 dsi_tmg_timestamp_get(struct dsi_tmg *tmg)
{
	struct dsi_tmg_hw *tmg_hw;
	u64 timestamp;

	if (!tmg) {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return 0;
	}

	tmg_hw = &tmg->tmg_hw;
	if (tmg_hw->ops.timestamp_get) {
		timestamp = tmg_hw->ops.timestamp_get(&tmg_hw->hw_blk);
	} else {
		DSI_ERROR("tmg hw timestamp_get function not implement\n");
		return 0;
	}

	return timestamp;
}

int dsi_tmg_dyn_freq_update(struct dsi_tmg *tmg, struct dsi_tmg_cfg *cfg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg || !cfg) {
		DSI_ERROR("invalid parameters, %pK, %pK\n", tmg, cfg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.dyn_freq_update)
		tmg_hw->ops.dyn_freq_update(&tmg_hw->hw_blk, cfg);
	else {
		DSI_ERROR("tmg hw dyn_freq_update function not implement\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_doze_enable(struct dsi_tmg *tmg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg) {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.doze_enable) {
		tmg_hw->ops.doze_enable(&tmg_hw->hw_blk);
	} else {
		DSI_ERROR("doze_enable uninitialized\n");
		return -EINVAL;
	}

	return 0;
}

int dsi_tmg_status_dump(struct dsi_tmg *tmg)
{
	struct dsi_tmg_hw *tmg_hw;

	if (!tmg) {
		DSI_ERROR("invalid parameters, %pK\n", tmg);
		return -EINVAL;
	}

	tmg_hw = &tmg->tmg_hw;

	if (tmg_hw->ops.status_dump) {
		tmg_hw->ops.status_dump(&tmg_hw->hw_blk);
	} else {
		DSI_ERROR("status_dump uninitialized\n");
		return -EINVAL;
	}

	return 0;
}
