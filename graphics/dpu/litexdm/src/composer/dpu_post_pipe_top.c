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

#include "dpu_res_mgr.h"
#include "dpu_comp_mgr.h"
#include "dpu_log.h"

int dpu_post_pipe_top_cfg(struct composer *comp)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top = NULL;
	struct post_pipe_cfg cfg = {0};
	struct post_pipe_debug_ctrl *debug_ctrl = &cfg.debug_ctrl;
	uint32_t conn_id;

	dpu_check_and_return(!comp, -1, "comp is null\n");

	hw_post_pipe_top = comp->hw_post_pipe_top;
	dpu_check_and_return(!hw_post_pipe_top, -1, "hw_post_pipe_top is null\n");

	conn_id = get_conn_id_from_scene_id(comp->scene_id);

	if (is_dsi_connector(comp->scene_id)) {
		debug_ctrl->crc_en = DPU_POSTPIPE_CRC_EN;
		debug_ctrl->crop_en = false;
		debug_ctrl->inspect_en = false;
		debug_ctrl->split_en = comp->pinfo->dual_port;
		dpu_pr_debug("split_en:%d\n", debug_ctrl->split_en);

		cfg.port0_sel = comp->pinfo->connector_id;
		cfg.port1_sel = comp->pinfo->external_connector_id;
		dpu_pr_debug("port1_sel:%d\n",cfg.port1_sel);
		cfg.pack_format = POSTPIPE_PACK_FMT_RGB888;

		if (comp->hw_dsc && comp->dsc_cfg.valid) {
			cfg.pack_format = POSTPIPE_PACK_FMT_DSC_BYTE;
			cfg.dsc_cfg.dsc_port0_en = (cfg.port0_sel == NONE) ? false : true;
			cfg.dsc_cfg.dsc_port1_en = (cfg.port1_sel == NONE) ? false : true;
		}
		hw_post_pipe_top->ops.port_config(&hw_post_pipe_top->hw, &cfg);
	}

	return 0;
}

int32_t dpu_post_pipe_top_crc_read(struct composer *comp)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top = NULL;
	bool dsc_en;

	dpu_check_and_return(!comp, -1, "comp is null\n");

	hw_post_pipe_top = comp->hw_post_pipe_top;
	dpu_check_and_return(!hw_post_pipe_top, -1, "hw_post_pipe_top is null\n");

	dsc_en = comp->dsc_cfg.valid;
	return hw_post_pipe_top->ops.crc_dump(&hw_post_pipe_top->hw, dsc_en);
}
