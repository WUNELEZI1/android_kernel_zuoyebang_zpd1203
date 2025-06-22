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

#include "litexdm.h"
#include "dpu_comp_mgr.h"
#include "dpu_res_mgr.h"

static void dpu_rch_mount_cfg(struct dpu_frame *frame, struct dpu_hw_scene_ctl *hw_scene_ctl)
{
	uint32_t rch_mask;
	uint32_t i;

	rch_mask = 0;
	for (i = 0; i < frame->layer_nums; i++)
		rch_mask |= BIT(frame->layer_infos[i].chn_idx);

	hw_scene_ctl->ops.rch_mount(&hw_scene_ctl->hw, rch_mask);
}

static void dpu_connector_mode_cfg(struct dpu_hw_scene_ctl *hw_scene_ctl,
		uint32_t scene_id, struct dpu_panel_info *pinfo)
{
	uint32_t conn_id = get_conn_id_from_scene_id(scene_id);
	bool is_cmd_mode = (pinfo->display_type == DSI_CMD_MODE) ? true : false;

	if (is_dsi_connector(scene_id)) {
		hw_scene_ctl->ops.timing_engine_mode_set(&hw_scene_ctl->hw, is_cmd_mode,
				!is_cmd_mode);
		hw_scene_ctl->ops.timing_engine_mount(&hw_scene_ctl->hw, pinfo->port_mask);
	} else if (is_wb_connector(scene_id)) {
		hw_scene_ctl->ops.wb_mount(&hw_scene_ctl->hw, BIT(conn_id - CONNECTOR_WB0));
	} else {
		dpu_pr_err("unsupported connector type %u\n", conn_id);
	}
}

int32_t dpu_scene_ctl_cfg(struct composer *comp, struct dpu_frame *frame)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp || !frame, -1, "comp or frame is null\n");

	hw_scene_ctl = comp->hw_scene_ctl;
	dpu_check_and_return(!hw_scene_ctl, -1, "hw_scene_ctl is null\n");

	hw_scene_ctl->ops.reset(&hw_scene_ctl->hw);
	dpu_rch_mount_cfg(frame, hw_scene_ctl);

	dpu_connector_mode_cfg(hw_scene_ctl, frame->scene_id, comp->pinfo);

	hw_scene_ctl->ops.cfg_ready_update(&hw_scene_ctl->hw, true, false);

	if ((frame->frame_no == 0) && is_dsi_connector(frame->scene_id)) {
		dpu_pr_debug("first frame start\n");
		hw_scene_ctl->ops.first_frame_start(&hw_scene_ctl->hw);
	}

	dpu_pr_debug("-\n");
	return 0;
}

int32_t dpu_scene_ctl_clear(struct composer *comp)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!comp, -1, "comp is null\n");

	hw_scene_ctl = comp->hw_scene_ctl;
	if (hw_scene_ctl)
		hw_scene_ctl->ops.sw_clear(&hw_scene_ctl->hw);

	dpu_pr_debug("-\n");
	return 0;
}
