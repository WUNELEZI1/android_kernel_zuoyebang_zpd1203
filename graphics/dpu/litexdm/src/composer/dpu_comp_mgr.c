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

#include <stddef.h>
#include "osal.h"

#include "platform_device.h"
#include "pipeline_hw_res.h"
#include "dpu_hw_power_ops.h"
#include "dpu_comp_mgr.h"
#include "dpu_pipeline.h"
#include "dpu_comp_dev.h"
#include "dpu_log.h"
#include "panel_mgr.h"
#include "dpu_res_mgr.h"
#include "dpu_present.h"

static struct comp_mgr g_comp_mgr;

static struct composer *get_composer(uint32_t comp_id)
{
	return g_comp_mgr.comp_list[comp_id];
}

int32_t composer_dump(uint32_t scene_id, bool debug_en)
{
	struct composer *composer = NULL;
	uint32_t comp_id;

	comp_id = get_comp_id_from_scene_id(scene_id);
	composer = get_composer(comp_id);
	dpu_check_and_return(!composer, -1, "composer is null\n");

	if (debug_en) {
		if (composer->hw_mixer)
			dpu_hw_blk_reg_dump(&composer->hw_mixer->hw);

		if (composer->hw_scene_ctl)
			dpu_hw_blk_reg_dump(&composer->hw_scene_ctl->hw);

		if (composer->hw_post_pipe_top)
			dpu_hw_blk_reg_dump(&composer->hw_post_pipe_top->hw);

		if (composer->hw_dsc)
			dpu_hw_blk_reg_dump(&composer->hw_dsc->hw);

		if (composer->hw_wb_top)
			dpu_hw_blk_reg_dump(&composer->hw_wb_top->hw);

		if (composer->hw_wb)
			dpu_hw_blk_reg_dump(&composer->hw_wb->hw);

		if (composer->hw_wb_scaler)
			dpu_hw_blk_reg_dump(&composer->hw_wb_scaler->hw);
	}

	return dpu_post_pipe_top_crc_read(composer);
}

static struct dpu_panel_info *get_panel_info(uint32_t scene_id)
{
	struct list_head *pos = NULL;
	uint32_t conn_id;

	conn_id = get_conn_id_from_scene_id(scene_id);

	list_for_each(pos, &g_comp_mgr.panel_list) {
		struct panel_info_entry *pinfo_entry = container_of(pos, struct panel_info_entry, list);
		dpu_check_and_return((!pinfo_entry || !pinfo_entry->pinfo), NULL,
			"pinfo_entry or pinfo is null\n");
		if ((pinfo_entry->pinfo->connector_id == conn_id) ||
			(pinfo_entry->pinfo->external_connector_id == conn_id))
			return pinfo_entry->pinfo;
	}

	dpu_pr_err("scene_id = %d, conn_id = %d\n", scene_id, conn_id);
	return NULL;
}

static void postpq_handle(struct color_postpq *postpq,
		display_post_t *in_post)
{
	dpu_pr_debug("+\n");

	postpq->enable = true;
	postpq->dither_lite_en = in_post->dither_lite_en;

	postpq->ltm = &in_post->ltm_param;
	postpq->acad = &in_post->acad_param;
	postpq->mulmatrix = &in_post->mulmatrix_param;
	postpq->r2y = &in_post->r2y_param;
	postpq->scaler = &in_post->scaler_param;
	postpq->y2r = &in_post->y2r_param;
	postpq->lut3d = &in_post->lut3d_param;
	postpq->cscmatrix = &in_post->cscmatrix_param;
	postpq->gma = &in_post->gma_param;
	postpq->dither = &in_post->dither_param;
	postpq->hist = &in_post->hist_param;
	postpq->rc = in_post->rc_param;

	/* postpq callback funcs init */
	dpu_mixer_color_postpq_init(postpq);

	dpu_pr_debug("-\n");
}

static int32_t composer_ops_set_up(struct composer *composer, struct dpu_frame *frame)
{
 	dpu_pr_debug("comp:%u +\n", composer->id);

	composer->enable = composer_enable;
	if (is_offline_scene(frame->scene_id))
		composer->present = dpu_offline_present;
	else
		composer->present = dpu_online_present;

	if (frame->postpq_enable)
		postpq_handle(&composer->postpq, frame->post_color);

	dpu_pr_debug("comp:%u -\n", composer->id);
	return 0;
}

static int32_t composer_resource_pq(struct composer *composer, uint32_t scene_id)
{
	struct dpu_hw_blk *hw_blk;

	dpu_pr_debug("+\n");

	if (!composer->postpq.enable)
		return 0;

	if (is_dsi_connector(scene_id)) {
		dpu_pr_debug("online pq resource\n");
		composer->postpq.hw_post_pipe_top = composer->hw_post_pipe_top;

		hw_blk = get_hw_blk(BLK_POST_PQ_TOP, composer->id);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_pq_top = to_dpu_hw_postpq_top(hw_blk);

		hw_blk = get_hw_blk(BLK_POST_PRE_PROC, POST_PRE_PROC_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_pre_proc = to_dpu_hw_postpre_proc(hw_blk);

		hw_blk = get_hw_blk(BLK_ACAD, ACAD_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_acad = to_dpu_hw_post_acad(hw_blk);

		hw_blk = get_hw_blk(BLK_POST_SCALER, POST_SCALER_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_scaler = to_dpu_hw_post_scaler(hw_blk);

		hw_blk = get_hw_blk(BLK_POST_3DLUT, POST_3DLUT_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_3dlut = to_dpu_hw_post_3dlut(hw_blk);

		hw_blk = get_hw_blk(BLK_POST_GAMMA, composer->id);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_gamma = to_dpu_hw_post_gamma(hw_blk);

		hw_blk = get_hw_blk(BLK_POST_HIST, POST_HIST_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->postpq.hw_hist = to_dpu_hw_post_hist(hw_blk);
	}

	dpu_pr_debug("-\n");
	return 0;
}

static int32_t composer_resource_get(struct composer *composer, uint32_t scene_id)
{
	struct dpu_hw_blk *hw_blk;
	int32_t ret;

	u32 conn_id = get_conn_id_from_scene_id(scene_id);

	dpu_pr_debug("composer->id:%u, conn_id:%u, scene_id:%u\n",
		composer->id, conn_id, scene_id);
	composer->scene_id = scene_id;

	composer->pinfo = get_panel_info(scene_id);
	dpu_check_and_return(!composer->pinfo, -1, "get panel info failed\n");

	hw_blk = get_hw_blk(BLK_DPU_INTR, DPU_INTR_0);
	dpu_check_and_return(!hw_blk, -1, "get intr blk failed\n");
	composer->hw_intr = to_dpu_hw_intr(hw_blk);

	hw_blk = get_hw_blk(BLK_MIXER, composer->id);
	dpu_check_and_return(!hw_blk, -1, "get mixer blk failed\n");
	composer->hw_mixer = to_dpu_hw_mixer(hw_blk);

	hw_blk = get_hw_blk(BLK_SCENE_CTL, composer->id);
	dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
	composer->hw_scene_ctl = to_dpu_hw_scene_ctl(hw_blk);

	if (is_dsi_connector(scene_id)) {
		hw_blk = get_hw_blk(BLK_POST_PIPE_TOP, composer->id);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_post_pipe_top = to_dpu_hw_post_pipe_top(hw_blk);

		hw_blk = get_hw_blk(BLK_POST_PQ_TOP, composer->id);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_pq_top = to_dpu_hw_postpq_top(hw_blk);

		hw_blk = get_hw_blk(BLK_DSC, composer->id);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_dsc = to_dpu_hw_dsc(hw_blk);
	}

	if (is_wb_connector(scene_id)) {
		hw_blk = get_hw_blk(BLK_CTL_TOP, CTL_TOP_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_ctl_top = to_dpu_hw_ctl_top(hw_blk);

		hw_blk = get_hw_blk(BLK_WB_TOP, DPU_TOP_0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_wb_top = to_dpu_hw_wb_top(hw_blk);

		hw_blk = get_hw_blk(BLK_WB, conn_id - CONNECTOR_WB0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_wb = to_dpu_hw_wb(hw_blk);
		hw_blk = get_hw_blk(BLK_WB_SCALER, conn_id - CONNECTOR_WB0);
		dpu_check_and_return(!hw_blk, -1, "get hw blk failed\n");
		composer->hw_wb_scaler = to_dpu_hw_wb_scaler(hw_blk);
	}

	ret = composer_resource_pq(composer, scene_id);
	dpu_check_and_return(ret, -1, "get post or wb pq resource failed\n");

	dpu_pr_debug("comp:%u -\n", composer->id);
	return 0;
}

static void comp_mgr_reset(struct composer *comp)
{
	comp->scene_id = 0;
	comp->pinfo = NULL;
	comp->hw_intr = NULL;
	comp->hw_mixer = NULL;
	comp->hw_ctl_top = NULL;
	comp->hw_scene_ctl = NULL;
	comp->hw_post_pipe_top = NULL;

	comp->hw_dsc = NULL;
	comp->hw_wb_top = NULL;
	comp->hw_wb = NULL;
	comp->hw_wb_scaler = NULL;

	dpu_mem_set(&comp->postpq, 0, sizeof(struct color_postpq));
	dpu_mem_set(&comp->dsc_cfg, 0, sizeof(struct dpu_dsc_config));
}

int32_t comp_mgr_enable(struct dpu_frame *frame)
{
	struct composer *comp;
	uint32_t scene_id = frame->scene_id;
	uint32_t comp_id;
	int32_t ret;

	dpu_pr_debug("+\n");

	comp_id = get_comp_id_from_scene_id(scene_id);
	comp = get_composer(comp_id);
	dpu_check_and_return(!comp, -1, "comp is null\n");

	comp_mgr_reset(comp);

	composer_ops_set_up(comp, frame);
	ret = composer_resource_get(comp, scene_id);
	dpu_check_and_return(ret != 0, -1, "comp resource get failed\n");

	conn_mgr_prepare(scene_id);

	if (comp->enable) {
		ret = comp->enable(comp);
		if (ret) {
			dpu_pr_err("comp:%u on fail\n", comp_id);
			return -1;
		}
	}

	dpu_pr_debug("comp:%u -\n", comp_id);
	return 0;
}

int32_t comp_mgr_power_on(struct dpu_frame *frame)
{
	struct dpu_panel_info *pinfo;
	struct composer *comp;
	int ret;

	dpu_check_and_return(!frame, -1, "frame is null\n");

	comp = get_composer(get_comp_id_from_scene_id(frame->scene_id));
	dpu_check_and_return(!comp, -1, "comp is null\n");

	pinfo = get_panel_info(frame->scene_id);
	dpu_check_and_return(!pinfo, -1, "get panel info failed\n");

	comp->pinfo = pinfo;
	comp->pwr_mgr.pinfo = pinfo;
	comp->pwr_mgr.profile_id = frame->profile_id;
	comp->pwr_mgr.volt_level = frame->volt_level;

	if (is_dsi_connector(frame->scene_id)) {
		ret = dsi_power_on(&comp->pwr_mgr);
		dpu_check_and_return(ret != 0, -1, "dsi power on fail\n");
	}

	ret = dpu_power_on(&comp->pwr_mgr);
	dpu_check_and_return(ret != 0, -1, "dpu power on fail\n");

	/* Open on and init the rear module */
	pipeline_next_on(g_comp_mgr.pdev, &frame->scene_id);

	dpu_pr_debug("-\n");
	return 0;
}

int32_t comp_mgr_power_off(struct dpu_frame *frame)
{
	struct composer *comp;
	uint32_t comp_id;
	int ret;

	comp_id = get_comp_id_from_scene_id(frame->scene_id);
	dpu_pr_debug("comp:%u +\n", comp_id);

	comp = get_composer(comp_id);
	dpu_check_and_return(!comp, -1, "comp is null\n");

	/* Power off the rear module */
	pipeline_next_off(g_comp_mgr.pdev, &frame->scene_id);

	ret = dpu_power_off(&comp->pwr_mgr);
	dpu_check_and_return(ret != 0, -1, "dpu power off fail\n");

	if (is_dsi_connector(frame->scene_id)) {
		ret = dsi_power_off(&comp->pwr_mgr);
		dpu_check_and_return(ret != 0, -1, "dsi power off fail\n");
	}

	return 0;
}

int32_t comp_mgr_present(struct dpu_frame *frame)
{
	struct composer *comp;
	uint32_t comp_id;

	comp_id = get_comp_id_from_scene_id(frame->scene_id);
	dpu_pr_debug("comp:%u +\n", comp_id);

	comp = get_composer(comp_id);
	dpu_check_and_return(!comp, -1, "comp is null\n");

	if (comp->present)
		comp->present(comp, frame);

	dpu_pr_debug("comp:%u -\n", comp_id);
	return 0;
}

int32_t comp_mgr_get_panel_info(struct panel_base_info *out_pinfo, uint32_t scene_id)
{
	struct composer *comp;
	uint32_t comp_id;

	dpu_check_and_return(!out_pinfo, -1, "out_pinfo is null\n");

	comp_id = get_comp_id_from_scene_id(scene_id);
	comp = get_composer(comp_id);
	dpu_check_and_return(!comp || !comp->pinfo, -1,
			"comp or comp->pinfo is null\n");

	out_pinfo->xres = comp->pinfo->xres;
	out_pinfo->yres = comp->pinfo->yres;
	return 0;
}

int32_t dpu_comp_mgr_init(void)
{
	struct platform_device *this_dev;
	struct hw_list comp_hw_list;
	uint32_t i;

	dpu_pr_debug("+\n");
	this_dev = platform_device_alloc("comp_mgr", 0);
	dpu_check_and_return(!this_dev, -1, "comp mgr dev alloc failed\n");

	INIT_LIST_HEAD(&g_comp_mgr.panel_list);
	g_comp_mgr.pdev = this_dev;
	comp_hw_list = get_support_comp_res();
	for (i = 0; i < comp_hw_list.res_num; i++) {
		struct composer *comp = dpu_mem_alloc(sizeof(*comp));
		dpu_check_and_return(!comp, -1, "comp is null\n");

		comp->id = comp_hw_list.res_list[i];
		comp->parent_dev = this_dev;
		g_comp_mgr.comp_list[i] = comp;
	}

	dpu_pr_debug("-\n");
	return 0;
}

int32_t register_composer(struct platform_device *next_pdev,
	struct dpu_panel_info *panel_info)
{
	struct panel_info_entry *pinfo_entry;
	int32_t ret;

	dpu_pr_debug("+\n");
	dpu_check_and_return(!panel_info, -1, "panel_info is null\n");

	pinfo_entry = dpu_mem_alloc(sizeof(struct panel_info_entry));
	dpu_check_and_return(!pinfo_entry, -1, "panel_info_entry alloc failed\n");

	pinfo_entry->pinfo = panel_info;
	list_add(&pinfo_entry->list, &g_comp_mgr.panel_list);
	g_comp_mgr.base.next = next_pdev;

	ret = platform_device_add_data(g_comp_mgr.pdev, &g_comp_mgr.base,
			sizeof(struct dpu_device_base));
	dpu_check_and_return(ret != 0, -1, "comp mgr add data failed\n");

	dpu_pr_debug("-\n");
	return ret;
}

void unregister_composer(void)
{
	struct panel_info_entry *pinfo_entry;
	struct list_head *pos = NULL;
	struct list_head *list_head;

	dpu_pr_debug("+\n");
	g_comp_mgr.pdev->platform_data = NULL;

	g_comp_mgr.base.next = NULL;

	list_head = &g_comp_mgr.panel_list;
	while(list_head->next != list_head) {
		pos = list_head->next;
		pinfo_entry = container_of(pos, struct panel_info_entry, list);
		dpu_check_and_no_retval(!pinfo_entry, "pinfo_entry or pinfo is null\n");
		list_del(pos);
		dpu_mem_free(pinfo_entry);

	}
	dpu_pr_debug("-\n");
}

struct comp_mgr *comp_mgr_get(void)
{
	return &g_comp_mgr;
}
