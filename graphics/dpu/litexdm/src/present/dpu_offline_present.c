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

#include "dpu_present.h"
#include "dpu_comp_mgr.h"
#include "dpu_res_mgr.h"
#include "dpu_log.h"
#include "dpu_intr.h"

static bool wait_frame_done(struct composer *comp, struct dpu_frame *frame)
{
	struct wb_intr_state *intr_state;
	struct dpu_hw_intr *hw_intr;
	bool is_triggered = false;
	u32 timeout_count = 0;

	dpu_pr_debug("+\n");

	hw_intr = comp->hw_intr;
	dpu_check_and_return(!hw_intr, false, "hw_intr is null\n");

	dpu_check_and_return(!comp->pinfo, false, "comp->pinfo is null\n");
	intr_state = comp->pinfo->wb_intr_ids;
	dpu_check_and_return(!intr_state, false, "intr_state is null\n");

	while (!is_triggered && (timeout_count < WAIT_INTR_TRIGGER_MAX_MS)) {
		dpu_mdelay(1);
		is_triggered = hw_intr->ops.status_get(&hw_intr->hw, intr_state->wb_intr_id);
		if (is_triggered) {
			hw_intr->ops.status_clear(&hw_intr->hw, intr_state->wb_intr_id);
			break;
		}

		timeout_count++;
	}

	dpu_pr_info("wait frame%d done used %dms\n", frame->frame_no, timeout_count);
	return true;
}

int32_t dpu_offline_present(struct composer *comp, struct dpu_frame *frame)
{
	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp || !frame, -1, "comp or frame is null\n");

	dpu_irq_init(comp);

	dpu_rch_cfg(comp, frame);
	dpu_mixer_cfg(comp, frame);

	dpu_wb_cfg(comp, frame);
	/* Attention : last step must be scene ctl cfg */
	dpu_scene_ctl_cfg(comp, frame);
	wait_frame_done(comp, frame);

	dpu_irq_deinit(comp);
	dpu_pr_debug("-\n");
	return 0;
}
