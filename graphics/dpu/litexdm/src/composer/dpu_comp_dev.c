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

#include "dpu_comp_mgr.h"
#include "dpu_hw_mixer.h"
#include "dpu_res_mgr.h"
#include "dpu_log.h"
#include "dpu_hw_power_ops.h"
#include "dpu_hw_postpq_top_ops.h"

int32_t composer_enable(struct composer *comp)
{
	struct dpu_hw_mixer *hw_mixer = comp->hw_mixer;
	struct dpu_hw_dsc *hw_dsc = comp->hw_dsc;
	struct dpu_panel_info *pinfo = comp->pinfo;
	struct postpq_hw_init_cfg cfg = {0};

	dpu_pr_debug("comp:%u +\n", comp->id);


	if (hw_mixer) {
		hw_mixer->ops.reset(&hw_mixer->hw, DIRECT_WRITE);
		hw_mixer->ops.enable(&hw_mixer->hw, true);
	}

	if (!is_offline_scene(comp->scene_id)) {
		cfg.hdisplay = comp->pinfo->xres;
		cfg.vdisplay = comp->pinfo->yres;
		dpu_hw_postpq_top_pipe_init(&comp->hw_pq_top->hw, &cfg);
		dpu_update_comp_dsc_cfg(comp);
	}

	if (hw_dsc && pinfo && pinfo->dsc_cfg.dsc_en) {
		dpu_pr_debug("dsc enable\n");
		hw_dsc->ops.enable(&hw_dsc->hw, &comp->dsc_cfg);
	}

	dpu_pr_debug("comp:%u -\n", comp->id);
	return 0;
}
