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

static bool check_frame_info(struct dpu_frame *frame)
{
	uint32_t i;

	dpu_check_and_return(!frame, false, "frame is null\n");
	if (frame->layer_nums >= MAX_LAYER_SUPPORT) {
		dpu_pr_err("layer num:%u more than max layer\n", frame->layer_nums);
		return false;
	}

	for (i = 0; i < frame->layer_nums; i++) {
		if (frame->layer_infos[i].chn_idx >= RCH_MAX) {
			dpu_pr_err("layer%u: chn_idx %u is invalid\n", i, frame->layer_infos[i].chn_idx);
			return false;
		}
	}

	return true;
}

static bool wait_end_of_frame(struct composer *comp, struct dpu_frame *frame)
{
	struct dsi_intr_state *intr_state;
	struct dpu_hw_intr *hw_intr;
	bool is_triggered = false;
	u32 timeout_count = WAIT_INTR_TRIGGER_MAX_MS;
	bool ret = true;

	dpu_pr_debug("+\n");

	hw_intr = comp->hw_intr;
	dpu_check_and_return(!hw_intr, false, "hw_intr is null\n");

	dpu_check_and_return(!comp->pinfo, false, "comp->pinfo is null\n");
	intr_state = comp->pinfo->dsi_intr_ids;
	dpu_check_and_return(!intr_state, false, "intr_state is null\n");

	dpu_pr_debug("start loop cfg rdy intr, timeout_count = %d\n", timeout_count);
	while (!is_triggered && (timeout_count > 0)) {
		dpu_mdelay(1);
		is_triggered = hw_intr->ops.status_get(&hw_intr->hw, intr_state->cfg_rdy_clr_id);
		if (is_triggered) {
			hw_intr->ops.status_clear(&hw_intr->hw, intr_state->cfg_rdy_clr_id);
			break;
		}

		timeout_count--;
	}
	if (timeout_count == 0) {
		dpu_pr_err("wait cfg_rdy_clr intr timeout\n");
		ret = false;
	}

	dpu_pr_debug("start loop eof, timeout_count = %d\n", timeout_count);
	is_triggered = false;
	while (!is_triggered && (timeout_count > 0)) {
		dpu_mdelay(1);
		is_triggered = hw_intr->ops.status_get(&hw_intr->hw, intr_state->eof_id);
		if (is_triggered) {
			hw_intr->ops.status_clear(&hw_intr->hw, intr_state->eof_id);
			break;
		}

		timeout_count--;
	}

	if (timeout_count == 0) {
		dpu_pr_err("wait eof intr timeout\n");
		ret = false;
	}

	dpu_pr_debug("start check underflow, timeout_count = %d\n", timeout_count);
	is_triggered = false;
	is_triggered = hw_intr->ops.status_get(&hw_intr->hw, intr_state->underflow_id);
	if (is_triggered || (timeout_count == 0)) {
		dpu_pr_err("underflow clear\n");
		dpu_scene_ctl_clear(comp);

		hw_intr->ops.status_clear(&hw_intr->hw, intr_state->underflow_id);
		ret = false;
	}

	dpu_pr_info("present frame%d used %dms\n", frame->frame_no,
			WAIT_INTR_TRIGGER_MAX_MS - timeout_count - 1);
	return ret;
}

int32_t dpu_online_present(struct composer *comp, struct dpu_frame *frame)
{
	uint32_t ret;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp, -1, "comp is null\n");
	dpu_check_and_return(!check_frame_info(frame), -1, "check frame info fail\n");

	dpu_irq_init(comp);

	dpu_rch_cfg(comp, frame);
	dpu_mixer_cfg(comp, frame);
	ret = dpu_post_pipe_top_cfg(comp);
	dpu_check_and_return(ret, -1, "dpu_post_pipe_top_cfg failed\n");
	/* Attention : last step must be scene ctl cfg */
	dpu_scene_ctl_cfg(comp, frame);
	wait_end_of_frame(comp, frame);

	dpu_irq_deinit(comp);
	dpu_pr_debug("-\n");
	return 0;
}
