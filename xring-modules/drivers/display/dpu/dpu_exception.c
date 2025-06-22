// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024-2024 XRing Technologies Co., Ltd.
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

#include <drm/drm_writeback.h>
#include <drm/drm_vblank.h>
#include <soc/xring/ddr_devfreq.h>
#include <linux/dma-buf.h>
#include <linux/export.h>
#include <linux/mm.h>

#include "dpu_kms.h"
#include "dpu_log.h"
#include "dpu_crtc.h"
#include "dsi_encoder.h"
#include "dsi_connector.h"
#include "dpu_wb.h"
#include "dpu_cmdlist_frame_mgr.h"
#include "dpu_exception.h"
#include "dp_drm.h"
#include "dpu_power_helper.h"
#include "dpu_plane.h"
#include "dpu_trace.h"
#include "dpu_cmdlist.h"
#include "dpu_hw_ulps.h"
#include "dpu_gem.h"
#include "dpu_format.h"
#include "dsi_connector.h"
#include "dsi_encoder.h"
#include "dp_drm.h"

static u32 g_dma_buf_debug_check_size = 1024;
static u32 g_dma_buf_debug_check_delay;

static void dpu_hw_status_dump(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state);

static void dpu_kms_clear_screen_with_bg_color(struct dpu_virt_pipeline *virt_pipe,
		struct drm_crtc_state *crtc_state)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_hw_mixer *hw_mixer;
	struct dpu_crtc_state *dpu_cstate;
	struct mixer_color_cfg bg_color;
	u32 screen_width;
	u32 screen_height;
	u32 channel_mask;
	int ret;

	if (!virt_pipe || !crtc_state || !crtc_state->crtc) {
		DPU_ERROR("invalid parameter %pK %pK\n", virt_pipe, crtc_state);
		return;
	}

	hw_scene_ctl = virt_pipe->hw_scene_ctl;
	dpu_cstate = to_dpu_crtc_state(crtc_state);
	hw_mixer = to_dpu_crtc(crtc_state->crtc)->hw_mixer;

	screen_width = dpu_cstate->base.mode.hdisplay;
	screen_height = dpu_cstate->base.mode.vdisplay;

	if (hw_mixer) {
		/* disable cmdlist cmps channel first */
		channel_mask = BIT(hw_mixer->hw.blk_id + RCH_MAX);
		dpu_hw_cmdlist_enable(virt_pipe->dpu_kms, channel_mask, false);

		hw_mixer->ops.reset(&hw_mixer->hw, DIRECT_WRITE);
		hw_mixer->ops.output_size_config(&hw_mixer->hw,
				screen_width, screen_height, DIRECT_WRITE);

		bg_color.a = 0x0;
		bg_color.r = 0x0;
		bg_color.g = 0x0;
		bg_color.b = 0x0;
		hw_mixer->ops.bg_color_config(&hw_mixer->hw, &bg_color,
				DIRECT_WRITE);
	} else {
		DPU_ERROR("hw_mixer is null\n");
	}

	if (hw_scene_ctl) {
		atomic_set(&virt_pipe->cfg_done_flag, 0);
		hw_scene_ctl->ops.rch_mount(&hw_scene_ctl->hw, 0x0);
		hw_scene_ctl->ops.wb_mount(&hw_scene_ctl->hw, 0x0);
		hw_scene_ctl->ops.cfg_ready_update(&hw_scene_ctl->hw,
				NEED_UPDT_EN, dpu_get_tui_state());
		hw_scene_ctl->ops.first_frame_start(&hw_scene_ctl->hw);

		ret = wait_event_timeout(virt_pipe->cfg_done_wq,
				atomic_read(&virt_pipe->cfg_done_flag) != 0,
				msecs_to_jiffies(virt_pipe->frame_duration_ms));
		if (!ret) {
			DPU_ERROR("failed to restart display with background color\n");
		} else {
			virt_pipe->sw_clear_flag = false;
			/* need enable tmg doze after tmg startup */
			virt_pipe->sw_start_flag = true;
			DPU_INFO("success restart display with background color\n");
		}
	} else {
		DPU_ERROR("hw_scene_ctl is null\n");
	}
}

static void dpu_online_underflow_clear(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc *crtc, struct drm_connector *connector,
		struct drm_crtc_state *crtc_state)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	struct dsi_display *display;
	bool need_flush_bg = false;
	u32 wait_timeout;
	int ret;

	dsi_connector = to_dsi_connector(connector);
	display = dsi_connector->display;
	dsi_encoder = to_dsi_encoder(dsi_connector->encoder);
	dpu_crtc_state = to_dpu_crtc_state(crtc->state);

	if ((atomic_read(&pipeline->underflow_flag) != 1) &&
			(atomic_read(&pipeline->vsync_flag) != 1) &&
			(atomic_read(&pipeline->cfg_done_flag) != 1)) {
		DPU_ERROR("[CRTC:%d:%s] frame_no: %llu miss te!\n",
				crtc->base.id, crtc->name, dpu_crtc_state->frame_no);
		dsi_panel_lock(display->panel);
		display_frame_timeout_get(display, &wait_timeout);
		ret = dsi_display_te_check_timeout(display, wait_timeout);
		dsi_panel_unlock(display->panel);
		if (ret) {
			DSI_ERROR("%s display double check te failed\n",
					dsi_display_type_name_get(display->display_type));
			dsi_display_panel_dead_notify(display);
			return;
		}

	}

	dsi_encoder_tmg_disable(dsi_encoder);
	hw_scene_ctl = pipeline->hw_scene_ctl;
	if (hw_scene_ctl) {
		ret = hw_scene_ctl->ops.sw_clear(&hw_scene_ctl->hw);
		if (ret) {
			DPU_ERROR("sw clear fail, need hardware reset!\n");

			dpu_hw_status_dump(pipeline, crtc->state);
			need_flush_bg = false;
		} else {
			need_flush_bg = true;
		}
		pipeline->sw_clear_flag = true;
	}
	dsi_encoder_tmg_enable(dsi_encoder);

	if (need_flush_bg && pipeline->dpu_kms->flush_bg_at_recovery)
		dpu_kms_clear_screen_with_bg_color(pipeline, crtc_state);
}

static void dpu_writeback_timeout_clear(struct dpu_virt_pipeline *pipeline,
		struct drm_connector *connector)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_wb_connector *dpu_wb_conn;

	drm_wb_conn = drm_connector_to_writeback(connector);
	dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
	if (dpu_wb_conn->is_online)
		return;

	hw_scene_ctl = pipeline->hw_scene_ctl;
	if (hw_scene_ctl) {
		hw_scene_ctl->ops.sw_clear(&hw_scene_ctl->hw);
		pipeline->sw_clear_flag = true;
	}
}

void dpu_freeze_current_frame(struct dpu_kms *dpu_kms)
{
	if (!dpu_kms) {
		DPU_ERROR("invalid parameter\n");
		return;
	}

	dpu_kms->dpu_in_exceptional_state = true;
	while (!dpu_kms->auto_recovery_en)
		DPU_MSLEEP(200);

	dpu_kms->dpu_in_exceptional_state = false;
}

static void dpu_kms_underflow_clear(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct drm_connector *connector;
	u32 mask;

	for_each_connector_per_crtc(connector, crtc_state, mask) {
		switch (connector->connector_type) {
		case DRM_MODE_CONNECTOR_DSI:
			dpu_power_exception_recovery(pipeline);
			dpu_hw_ulps_exception_recovery(crtc_state, connector);
			/* TODO: add dual-mipi dual-panel scenario support in here */
			if (dsi_connector_is_cmd_mode(connector)) {
				dpu_online_underflow_clear(pipeline, crtc_state->crtc, connector,
						crtc_state);
				dpu_force_refresh_event_notify(crtc_state->crtc->dev);
			}
			break;
		case DRM_MODE_CONNECTOR_DisplayPort:
			/* TODO: adapt for DP */
			break;
		case DRM_MODE_CONNECTOR_WRITEBACK:
			dpu_writeback_timeout_clear(pipeline, connector);
			drm_wb_conn = drm_connector_to_writeback(connector);
			dpu_wb_connector_job_clear(crtc_state->crtc, drm_wb_conn);
			break;
		default:
			DPU_ERROR("unsupported connector type %u\n",
					connector->connector_type);
			break;
		}
	}
}

static void dpu_encoder_status_dump(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state)
{
	struct dpu_virt_pipe_state *working_state = &pipeline->working_state;
	struct dpu_virt_pipe_state *pending_state = &pipeline->pending_state;
	struct dsi_connector *dsi_connector;
	struct dsi_encoder *dsi_encoder;
	struct drm_connector *connector;
	u32 connectror_mask;

	DPU_INFO("cfg ts %12lld(us) curr ts %12lld(us)\n",
			ktime_to_us(pipeline->cfg_timestamp),
			ktime_to_us(ktime_get()));

	connectror_mask = pending_state->new_connector_mask | pending_state->old_connector_mask |
			working_state->new_connector_mask | working_state->old_connector_mask;
	for (connector = __find_connector(pipeline->dpu_kms->drm_dev, &connectror_mask);
			connector; connector = __find_connector(
			pipeline->dpu_kms->drm_dev, &connectror_mask)) {
		if (connector->connector_type == DRM_MODE_CONNECTOR_DSI) {
			dsi_connector = to_dsi_connector(connector);
			dsi_encoder = to_dsi_encoder(dsi_connector->encoder);
			dsi_encoder_tmg_status_dump(dsi_encoder);

			dpu_dsi_irq_status_dump(connector);
		} else if (connector->connector_type == DRM_MODE_CONNECTOR_DisplayPort)
			dpu_dp_irq_status_dump(connector);
	}
}

static void dpu_hw_status_dump(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state)
{
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct dpu_hw_rch_top *hw_rch_top;
	struct dpu_hw_wb_top *hw_wb_top;
	struct dpu_hw_mixer *hw_mixer;
	struct dpu_hw_intr *hw_intr;
	struct dpu_hw_rch *hw_rch;
	struct dpu_crtc *dpu_crtc;
	int i;

	dpu_crtc = to_dpu_crtc(crtc_state->crtc);
	hw_mixer = dpu_crtc->hw_mixer;
	hw_ctl_top = pipeline->dpu_kms->virt_ctrl.hw_ctl_top;
	hw_wb_top = pipeline->dpu_kms->virt_ctrl.hw_wb_top;
	hw_scene_ctl = pipeline->hw_scene_ctl;
	hw_post_pipe_top = dpu_crtc->hw_post_pipe_top;
	hw_mixer = dpu_crtc->hw_mixer;

	if (hw_ctl_top)
		hw_ctl_top->ops.status_dump(&hw_ctl_top->hw);

	if (hw_scene_ctl)
		hw_scene_ctl->ops.status_dump(&hw_scene_ctl->hw);

	if (dpu_crtc->postpq)
		dpu_crtc->postpq->funcs->status_dump(crtc_state->crtc);

	if (hw_post_pipe_top)
		hw_post_pipe_top->ops.status_dump(&hw_post_pipe_top->hw);

	if (hw_wb_top) {
		hw_wb_top->ops.debug_enable(&hw_wb_top->hw, false);
		hw_wb_top->ops.status_dump(&hw_wb_top->hw);
	}

	if (hw_mixer)
		hw_mixer->ops.status_dump(&hw_mixer->hw);

	hw_intr = pipeline->dpu_kms->virt_ctrl.intr_ctx->hw_intr;
	if (hw_intr)
		hw_intr->ops.status_dump(&hw_intr->hw);

	dpu_hw_tmg_mclk_auto_cg_dump();
	dpu_idle_status_dump();
	dpu_hw_ulps_status_dump(crtc_state);

	/* cancel the call to dpu_kms_hw_status_clear if needed */
	for (i = 0; i < RCH_MAX; i++) {
		hw_rch = pipeline->working_state.rch[i];
		hw_rch_top = pipeline->working_state.rch_top[i];

		if (!hw_rch)
			continue;

		hw_rch->ops.status_dump(&hw_rch->hw);
		// hw_rch_top->ops.status_dump(&hw_rch_top->hw);
	}

	dpu_encoder_status_dump(pipeline, crtc_state);
}

static void dpu_hw_status_clear(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc *crtc)
{
	struct dpu_crtc *dpu_crtc = to_dpu_crtc(crtc);
	struct dpu_hw_post_pipe_top *hw_post_pipe_top;
	struct dpu_hw_ctl_top *hw_ctl_top;
	struct dpu_hw_wb_top *hw_wb_top;
	struct dpu_dbg_ctx *ctx;
	struct dpu_hw_rch *hw_rch;
	int i;

	hw_ctl_top = pipeline->dpu_kms->virt_ctrl.hw_ctl_top;
	if (hw_ctl_top)
		hw_ctl_top->ops.status_clear(&hw_ctl_top->hw);

	hw_wb_top = pipeline->dpu_kms->virt_ctrl.hw_wb_top;
	if (hw_wb_top)
		hw_wb_top->ops.status_clear(&hw_wb_top->hw);

	hw_post_pipe_top = dpu_crtc->hw_post_pipe_top;
	if (hw_post_pipe_top)
		hw_post_pipe_top->ops.status_clear(&hw_post_pipe_top->hw);

	ctx = pipeline->dpu_kms->dbg_ctx;
	for (i = 0; i < RCH_MAX; i++) {
		hw_rch = to_dpu_hw_rch(ctx->rch_hw_blk[i]);
		if (!hw_rch)
			continue;

		hw_rch->ops.status_clear(&hw_rch->hw);
	}
}

void dpu_core_perf_dump(struct dpu_kms *dpu_kms)
{
	unsigned long real_bandwidth = 0;
	struct dpu_core_perf *core_perf;
	struct dpu_power_mgr *power_mgr;

	if (!dpu_kms || !dpu_kms->power_mgr || !dpu_kms->core_perf)
		return;

	core_perf = dpu_kms->core_perf;
	power_mgr = dpu_kms->power_mgr;

	DFX_INFO("last vote: ddr %u MB/s, clk profile %u", core_perf->curr_bandwidth, core_perf->curr_profile);

	if (ddr_get_cur_bandwidth(&real_bandwidth) != 0)
		DFX_INFO("get real ddr fail");

	DFX_INFO("real time: ddr %lu MB/s, real core clk %lu, axi clk %lu",
		real_bandwidth,
		dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_CORE0),
		dpu_clk_get_rate(power_mgr->clk_mgr, CLK_DPU_AXI0));
}

static void dpu_frame_info_dump(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state)
{
	struct dpu_virt_pipe_state *working_state = &pipeline->working_state;
	struct dpu_virt_pipe_state *pending_state = &pipeline->pending_state;

	DPU_INFO("pending mode info vrefresh new %d(hz) old %d(hz)\n",
			pending_state->new_vrefresh_rate, pending_state->old_vrefresh_rate);

	DPU_INFO("working mode info vrefresh new %d(hz) old %d(hz)\n",
			working_state->new_vrefresh_rate, working_state->old_vrefresh_rate);

	DPU_INFO("new rch: 0x%x old rch: 0x%x\n",
			pending_state->rch_mask,
			working_state->rch_mask);

	if (working_state->partial_update)
		DPU_INFO("old partial update cfg height 0x%x to 0x%x\n",
				working_state->old_height,
				working_state->new_height);

	if (pending_state->partial_update)
		DPU_INFO("new partial update cfg height 0x%x to 0x%x\n",
				pending_state->old_height,
				pending_state->new_height);
}

void dpu_hw_cfg_dump(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state,
		enum dpu_hw_cfg_dump_type type)
{
	struct dpu_crtc_state *dpu_crtc_state;

	if (!pipeline || !crtc_state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", pipeline, crtc_state);
		return;
	}
	dpu_crtc_state = to_dpu_crtc_state(crtc_state);

	dpu_frame_info_dump(pipeline, crtc_state);
	dpu_core_perf_dump(pipeline->dpu_kms);
	dpu_hw_status_dump(pipeline, crtc_state);

	dpu_ddr_dvfs_ok_dump();

	dpu_freeze_current_frame(pipeline->dpu_kms);

	dpu_hw_status_clear(pipeline, crtc_state->crtc);
}

static bool dpu_check_hw_intr_state(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state)
{
	struct drm_writeback_connector *drm_wb_conn;
	struct dpu_wb_connector *dpu_wb_conn;
	struct dsi_connector *dsi_connector;
	struct dp_connector *dp_connector;
	struct drm_connector *connector;
	struct dpu_hw_intr *hw_intr;
	char *connector_name;
	bool cfg_rdy_status;
	int cfg_clr_id;
	u32 mask;

	hw_intr = pipeline->dpu_kms->virt_ctrl.intr_ctx->hw_intr;
	if (!hw_intr)
		return false;

	for_each_connector_per_crtc(connector, crtc_state, mask) {
		switch (connector->connector_type) {
		case DRM_MODE_CONNECTOR_DSI:
			dsi_connector = to_dsi_connector(connector);
			cfg_clr_id = dsi_connector->intr_ctx.base.intr_id[DPU_CFG_RDY_CLR_INT];
			connector_name = "dsi";
			break;
		case DRM_MODE_CONNECTOR_DisplayPort:
			dp_connector = to_dp_connector(connector);
			cfg_clr_id = dp_connector->intr_ctx.base.intr_id[DPU_CFG_RDY_CLR_INT];
			connector_name = "dp";
			break;
		case DRM_MODE_CONNECTOR_WRITEBACK:
			drm_wb_conn = drm_connector_to_writeback(connector);
			dpu_wb_conn = to_dpu_wb_connector(drm_wb_conn);
			if (dpu_wb_conn->is_online)
				break;
			cfg_clr_id = dpu_wb_conn->intr_state.wb_done_id;
			connector_name = "wb";
			break;
		default:
			break;
		}
	}

	cfg_rdy_status = hw_intr->ops.status_get(&hw_intr->hw, cfg_clr_id);

	DPU_INFO("%s - cfg_rdy done, intr id: %d, hw_status: %d",
			connector_name, cfg_clr_id, cfg_rdy_status);
	if (cfg_rdy_status) {
		DPU_WARN("[frame_no:%llu] dpu hardware working normally\n",
				to_dpu_crtc_state(crtc_state)->frame_no);
		return false;
	}

	return  true;
}

void dpu_exception_recovery(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc_state *crtc_state)
{
	bool need_recovery = false;

	if (!pipeline || !crtc_state) {
		DPU_ERROR("invalid parameter %pK, %pK\n", pipeline, crtc_state);
		return;
	}

	/* do dpu power and top auto cg protect */
	dpu_power_get_helper(DPU_ALL_PARTITIONS);
	need_recovery = dpu_check_hw_intr_state(pipeline, crtc_state);
	if (need_recovery) {
		dpu_hw_cfg_dump(pipeline, crtc_state, RECOVERY_DUMP);
		dpu_kms_underflow_clear(pipeline, crtc_state);
	}
	dpu_power_put_helper(DPU_ALL_PARTITIONS);
}

void dpu_ddr_dvfs_ok_dump(void)
{
	char buffer[DVFS_OK_BUF_LEN];
	int ret;

	ret = get_dvfs_ok_tout_info(buffer, DVFS_OK_BUF_LEN);
	if (ret > 0)
		DFX_INFO("ddr dvfs ok status: %s\n", buffer);
	else
		DFX_INFO("get ddr dvfs ok status fail\n");
}

static void dbg_dma_buf_need_dump(struct dpu_dbg_ctx *ctx, u32 rdma_dbg_status, u32 rch_id)
{
	/* left, right layer afbc dec err */
	if (rdma_dbg_status & (BIT(14) | BIT(15))) {
		ctx->need_dump_buf = true;
		ctx->need_dump_rch_id[rch_id] = true;
	}
}

static void dbg_irq_save(struct drm_crtc *crtc, enum dpu_intr_type intr_type)
{
	struct dpu_hw_scene_ctl *hw_scene_ctl;
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_hw_wb_top *hw_wb_top;
	struct dpu_hw_intr *hw_intr;
	struct dpu_hw_rch *hw_rch;
	struct dpu_dbg_ctx *ctx;
	struct dpu_kms *dpu_kms;
	u32 value;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	hw_wb_top = dpu_kms->virt_ctrl.hw_wb_top;
	pipeline = &dpu_kms->virt_pipeline[drm_crtc_index(crtc)];
	hw_scene_ctl = pipeline->hw_scene_ctl;
	hw_intr = dpu_kms->virt_ctrl.intr_ctx->hw_intr;
	ctx = dpu_kms->dbg_ctx;

	ctx->work_data[intr_type].crtc = crtc;
	ctx->work_data[intr_type].dpu_kms = dpu_kms;
	value = hw_wb_top->ops.dbg_irq_dump(&hw_wb_top->hw);
	ctx->work_data[intr_type].wdma_dbg_irq_status = value;
	hw_wb_top->ops.dbg_irq_clear(&hw_wb_top->hw);

	for (i = 0; i < RCH_MAX; i++) {
		hw_rch = to_dpu_hw_rch(ctx->rch_hw_blk[i]);

		if (!hw_rch)
			continue;

		value = hw_rch->ops.dbg_irq_dump(&hw_rch->hw);
		ctx->work_data[intr_type].rch_dbg_irq_status[i] = value;
		hw_rch->ops.dbg_irq_clear(&hw_rch->hw);

		dbg_dma_buf_need_dump(ctx, value, hw_rch->hw.blk_id);
	}

	if (hw_intr)
		ctx->work_data[intr_type].plsverr_status =
			hw_intr->ops.pslverr_dump(&hw_intr->hw, intr_type);

	queue_work(ctx->wq[intr_type], &ctx->work_data[intr_type].work);
}

static void dbg_irq_disable(struct drm_crtc *crtc)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_hw_wb_top *hw_wb_top;
	struct dpu_hw_rch *hw_rch;
	struct dpu_dbg_ctx *ctx;
	struct dpu_kms *dpu_kms;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	hw_wb_top = dpu_kms->virt_ctrl.hw_wb_top;
	ctx = dpu_kms->dbg_ctx;

	if (hw_wb_top)
		hw_wb_top->ops.dbg_irq_enable(&hw_wb_top->hw, false);

	for (i = 0; i < RCH_MAX; i++) {
		hw_rch = to_dpu_hw_rch(ctx->rch_hw_blk[i]);

		if (!hw_rch)
			continue;

		hw_rch->ops.dbg_irq_enable(&hw_rch->hw, false);
	}
}

static void dbg_irq_clear(struct drm_crtc *crtc)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_hw_wb_top *hw_wb_top;
	struct dpu_hw_rch *hw_rch;
	struct dpu_dbg_ctx *ctx;
	struct dpu_kms *dpu_kms;
	int i;

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	hw_wb_top = dpu_kms->virt_ctrl.hw_wb_top;
	ctx = dpu_kms->dbg_ctx;

	if (hw_wb_top)
		hw_wb_top->ops.dbg_irq_clear(&hw_wb_top->hw);

	for (i = 0; i < RCH_MAX; i++) {
		hw_rch = to_dpu_hw_rch(ctx->rch_hw_blk[i]);

		if (!hw_rch)
			continue;

		hw_rch->ops.dbg_irq_clear(&hw_rch->hw);
	}
}

static void dpu_dbg_irq_handler(int intr_id, void *data)
{
	struct dpu_drm_device *dpu_drm_dev;
	struct dpu_intr_context *intr_ctx;
	struct dpu_intr_pipe *intr_pipe;
	enum dpu_intr_type intr_type;
	struct dpu_dbg_ctx *ctx;
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;

	DPU_ISR_DEBUG("Enter, intr_id %d\n", intr_id);

	intr_ctx = data;
	intr_type = dpu_interrupt_id_to_type(intr_id);
	if (intr_type >= DPU_INTR_TYPE_MAX)
		return;

	intr_pipe = &intr_ctx->intr_pipe[intr_type];
	crtc = intr_pipe->crtc;
	if (!crtc) {
		DPU_ISR_DEBUG("crtc is null\n");
		return;
	}

	dpu_drm_dev = to_dpu_drm_dev(crtc->dev);
	dpu_kms = dpu_drm_dev->dpu_kms;
	ctx = dpu_kms->dbg_ctx;

	/* if irq status print not complete, don't handle new irq */
	if (ctx->work_data[intr_type].complete_print == false) {
		dbg_irq_clear(crtc);
		return;
	}

	if ((intr_id == INTR_ONLINE0_DMA_DBG_PART1_RDMA) ||
		(intr_id == INTR_ONLINE0_DMA_DBG_PART2_RDMA) ||
		(intr_id == INTR_ONLINE0_DMA_DBG_WDMA) ||
		(intr_id == INTR_ONLINE0_CMDLIST_DBG) ||
		(intr_id == INTR_COMBO_DMA_DBG_PART1_RDMA) ||
		(intr_id == INTR_COMBO_DMA_DBG_PART2_RDMA) ||
		(intr_id == INTR_COMBO_DMA_DBG_WDMA) ||
		(intr_id == INTR_COMBO_CMDLIST_DBG) ||
		(intr_id == INTR_OFFLINE0_DMA_DBG_PART1_RDMA) ||
		(intr_id == INTR_OFFLINE0_DMA_DBG_PART2_RDMA) ||
		(intr_id == INTR_OFFLINE0_DMA_DBG_WDMA) ||
		(intr_id == INTR_OFFLINE0_CMDLIST_DBG)) {
		dbg_irq_disable(crtc);
		ctx->work_data[intr_type].complete_print = false;
		dbg_irq_save(crtc, intr_type);
	}
}

static void dump_dma_buf_to_buf(struct dma_buf *dmabuf,  void *buf)
{
	struct iosys_map map;
	size_t size;
	int ret;

	if (!dmabuf || !buf) {
		DPU_ERROR("invalid para %pK %pK\n", dmabuf, buf);
		return;
	}

	size = dmabuf->size > g_dma_buf_debug_check_size ?
			g_dma_buf_debug_check_size : dmabuf->size;

	ret = dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
	if (ret) {
		DPU_ERROR("failed to begin cpu access: %d\n", ret);
		goto out;
	}
	ret = dma_buf_vmap(dmabuf, &map);
	if (ret) {
		DPU_ERROR("failed to vmap dma_buf\n");
		ret = -ENOMEM;
		goto end_access;
	}

	memcpy(buf, map.vaddr, size);

	dma_buf_vunmap(dmabuf, &map);

end_access:
	dma_buf_end_cpu_access(dmabuf, DMA_FROM_DEVICE);
out:
	if (ret)
		DPU_ERROR("failed to dump dma buf: %d\n", ret);
}

static void dbg_input_buf_dump(struct dpu_dbg_ctx *dbg_ctx,
		struct drm_atomic_state *state, u32 buf_index)
{
	struct drm_plane_state *old_plane_state, *new_plane_state;
	struct drm_plane *plane;
	struct dpu_gem_object *gem_obj;
	int i, zorder;

	for_each_oldnew_plane_in_state(state, plane, old_plane_state, new_plane_state, i) {
		if (new_plane_state->fb && new_plane_state->fb->obj[0]) {
			zorder = new_plane_state->zpos;
			gem_obj = to_dpu_gem_obj(new_plane_state->fb->obj[0]);
			dump_dma_buf_to_buf(gem_obj->dmabuf, dbg_ctx->input_buf[buf_index][zorder]);
		}
	}
}

static int dbg_input_buf_cmp(struct dpu_dbg_ctx *dbg_ctx,
		struct drm_atomic_state *state)
{
	struct drm_plane_state *old_plane_state, *new_plane_state;
	struct drm_plane *plane;
	struct dpu_plane_state *dpu_pstate;
	struct dpu_gem_object *gem_obj;
	int i, zorder, size, ret = 0;

	for_each_oldnew_plane_in_state(state, plane, old_plane_state, new_plane_state, i) {
		if (new_plane_state->fb && new_plane_state->fb->obj[0]) {
			zorder = new_plane_state->zpos;
			gem_obj = to_dpu_gem_obj(new_plane_state->fb->obj[0]);
			size = gem_obj->dmabuf->size > g_dma_buf_debug_check_size ?
					g_dma_buf_debug_check_size : gem_obj->dmabuf->size;
			dpu_pstate = to_dpu_plane_state(new_plane_state);

			ret = memcmp(dbg_ctx->input_buf[0][zorder], dbg_ctx->input_buf[1][zorder], size);
			if (ret) {
				DPU_ERROR("dma buf check miss match, zorder %d, rch id %d\n",
						zorder, dpu_pstate->rch_id);
				break;
			}
		}
	}

	return ret;
}

void dpu_acquire_fences_debug(struct dpu_commit_ctx *commit_ctx,
		struct drm_atomic_state *state)
{
	struct dpu_virt_pipeline *pipeline;
	struct dpu_kms *dpu_kms;
	int i, j, ret;

	if (!commit_ctx || !state) {
		DPU_ERROR("invalid params, %pK %pK\n", commit_ctx, state);
		return;
	}

	pipeline = container_of(commit_ctx, struct dpu_virt_pipeline, commit_ctx);
	dpu_kms = pipeline->dpu_kms;

	if (!(dpu_kms->dbg_ctx->dma_buf_debug_type &
			DPU_DMA_BUF_BIT_CHECK_AFTER_ACQUIRE_FENCE))
		return;

	for (i = 0; i <= MAX_PLANE_ZPOS; i++) {
		for (j = 0; j < 2; j++) {
			if (!dpu_kms->dbg_ctx->input_buf[j][i]) {
				dpu_kms->dbg_ctx->input_buf[j][i] = kmalloc(g_dma_buf_debug_check_size, GFP_KERNEL);
				if (!dpu_kms->dbg_ctx->input_buf[j][i]) {
					DPU_ERROR("alloc buf failed\n");
					dpu_kms->dbg_ctx->dma_buf_debug_type &=
							~DPU_DMA_BUF_BIT_CHECK_AFTER_ACQUIRE_FENCE;
					return;
				}
			}
		}
	}

	if (g_dma_buf_debug_check_delay)
		mdelay(g_dma_buf_debug_check_delay);

	dbg_input_buf_dump(dpu_kms->dbg_ctx, state, 0);

	dbg_input_buf_dump(dpu_kms->dbg_ctx, state, 1);

	ret = dbg_input_buf_cmp(dpu_kms->dbg_ctx, state);
	if (ret)
		DPU_ERROR("frame no %lld: dma buff is still changing after the require fence is signaled\n",
				commit_ctx->ctx_frame_no);

}

void dpu_dbg_flush_workqueue(struct dpu_dbg_ctx *ctx,
		struct drm_crtc *crtc)
{
	u32 intr_type;
	int ret;

	if (!ctx || !crtc) {
		DPU_ERROR("invalid parameter %pK %pK\n", ctx, crtc);
		return;
	}

	ret = dpu_intr_type_get(crtc, &intr_type);
	if (ret)
		return;
	flush_workqueue(ctx->wq[intr_type]);
}

static void print_wdma_dbg_irq_status(u8 wch_id, u32 value)
{
	char s[128];
	int i;

	DPU_DFX_INFO("wb_top", wch_id, s, "dbg irq raw status is 0x%x", value);

	for (i = 0; i < 4; i++) {
		if (value & BIT(i))
			DPU_DFX_INFO("wb_top", wch_id, s, "master_wr%d_timeout\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 4))
			DPU_DFX_INFO("wb_top", wch_id, s, "mmu%d_timeout_err\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 8))
			DPU_DFX_INFO("wb_top", wch_id, s, "mmu%d_tbu_size_err\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 12))
			DPU_DFX_INFO("wb_top", wch_id, s, "mmu%d_tlb_miss\n", i);
	}

	for (i = 0; i < 4; i++) {
		if (value & BIT(i + 16))
			DPU_DFX_INFO("wb_top", wch_id, s, "va%d_missmatch\n", i);
	}

	if (value & BIT(20))
		DPU_DFX_INFO("wb_top", wch_id, s, "wdma_resp_err\n");

	for (i = 0; i < 2; i++) {
		if (value & BIT(i + 21))
			DPU_DFX_INFO("wb_top", wch_id, s,
					"dsc%d wdma input pixel num err\n", i);
	}
}

static void print_rdma_dbg_irq_status(u8 rch_id, u32 value)
{
	char s[128];

	DPU_DFX_INFO("rdma_path", rch_id, s, "dbg irq raw status is 0x%x", value);

	if (value & BIT(0))
		DPU_DFX_INFO("rdma_path", rch_id, s, "tlb miss\n");

	if (value & BIT(1))
		DPU_DFX_INFO("rdma_path", rch_id, s, "tlb size err\n");

	if (value & BIT(2))
		DPU_DFX_INFO("rdma_path", rch_id, s, "mmu rdma timeout\n");

	if (value & BIT(3))
		DPU_DFX_INFO("rdma_path", rch_id, s, "mmu rdma resp err\n");

	if (value & BIT(4))
		DPU_DFX_INFO("rdma_path", rch_id, s, "rdma timeout\n");

	if (value & BIT(5))
		DPU_DFX_INFO("rdma_path", rch_id, s, "rdma resp err\n");

	if (value & BIT(6))
		DPU_DFX_INFO("rdma_path", rch_id, s, "memory conflict err\n");

	if (value & BIT(7))
		DPU_DFX_INFO("rdma_path", rch_id, s, "rdma memory size err\n");

	if (value & BIT(8))
		DPU_DFX_INFO("rdma_path", rch_id, s, "left layer rdma eof\n");

	if (value & BIT(9))
		DPU_DFX_INFO("rdma_path", rch_id, s, "right layer rdma eof\n");

	if (value & BIT(10))
		DPU_DFX_INFO("rdma_path", rch_id, s, "left layer all eof\n");

	if (value & BIT(11))
		DPU_DFX_INFO("rdma_path", rch_id, s, "right layer all eof\n");

	if (value & BIT(12))
		DPU_DFX_INFO("rdma_path", rch_id, s, "afbc dec eof for left layer\n");

	if (value & BIT(13))
		DPU_DFX_INFO("rdma_path", rch_id, s, "afbc dec eof for right layer\n");

	if (value & BIT(14))
		DPU_DFX_INFO("rdma_path", rch_id, s, "left layer afbc decode err\n");

	if (value & BIT(15))
		DPU_DFX_INFO("rdma_path", rch_id, s, "right layer afbc decode err\n");

	if (value & BIT(16))
		DPU_DFX_INFO("rdma_path", rch_id, s, "left layer output eof\n");

	if (value & BIT(17))
		DPU_DFX_INFO("rdma_path", rch_id, s, "right layer output eof\n");

	if (value & BIT(18))
		DPU_DFX_INFO("rdma_path", rch_id, s, "rdma sof\n");

	if (value & BIT(19))
		DPU_DFX_INFO("rdma_path", rch_id, s, "slice_req_done of video slice mode\n");

	if (value & BIT(20))
		DPU_DFX_INFO("rdma_path", rch_id, s, "re-start of video slice mode\n");

	if (value & BIT(21))
		DPU_DFX_INFO("rdma_path", rch_id, s, "rdma urgent irq\n");

	if (value & BIT(22))
		DPU_DFX_INFO("rdma_path", rch_id, s, "mmu va mismatch irq\n");

	if (value & BIT(23))
		DPU_DFX_INFO("rdma_path", rch_id, s, "sw mem size err\n");
}

static void dbg_dump_input_buf_to_file(struct dpu_dbg_ctx *ctx,
		struct drm_atomic_state *old_state,
		u64 frame_no)
{
	struct drm_plane_state *old_plane_state, *new_plane_state;
	struct dpu_plane_state *dpu_pstate;
	const struct dpu_format_map *format_map;
	struct dpu_gem_object *gem_obj;
	struct drm_plane *plane;
	char f_name[256] = {0};
	int old_rch_cnt[RCH_MAX] = {0};
	int new_rch_cnt[RCH_MAX] = {0};
	int i;
	static int dump_file_num;

	for_each_oldnew_plane_in_state(old_state, plane, old_plane_state, new_plane_state, i) {
		if (old_plane_state->fb && old_plane_state->fb->obj[0]) {
			dpu_pstate = to_dpu_plane_state(old_plane_state);
			if (ctx->need_dump_rch_id[dpu_pstate->rch_id]) {
				gem_obj = to_dpu_gem_obj(old_plane_state->fb->obj[0]);
				format_map = dpu_format_get(FMT_MODULE_RCH, old_plane_state->fb->format->format,
						old_plane_state->fb->modifier);
				snprintf(f_name, sizeof(f_name),
						"%s_frame_no_%lld(%dth)_rch_id_%d(%d)_%d_%d_%s_yuv_tran(%d)_split(%d)_32x8(%d)_tile(%d).fbc",
						"/data/xdm/rch_dump/old",
						frame_no,
						dump_file_num,
						dpu_pstate->rch_id,
						old_rch_cnt[dpu_pstate->rch_id],
						old_plane_state->fb->width,
						old_plane_state->fb->height,
						format_map->dpu_format_name,
						DPU_FMT_HAS_AFBC_Y2R(old_plane_state->fb->modifier) > 0 ? 1 : 0,
						DPU_FMT_HAS_AFBC_SPLIT(old_plane_state->fb->modifier) > 0 ? 1 : 0,
						DPU_FMT_IS_AFBC_32x8(old_plane_state->fb->modifier) > 0 ? 1 : 0,
						DPU_FMT_HAS_AFBC_TILED(old_plane_state->fb->modifier) > 0 ? 1 : 0);
				DPU_WARN("dump buffer %s > %s\n", gem_obj->dmabuf->name, f_name);
				dump_file_num++;
				old_rch_cnt[dpu_pstate->rch_id]++;
			}
		}

		if (new_plane_state->fb && new_plane_state->fb->obj[0]) {
			dpu_pstate = to_dpu_plane_state(new_plane_state);
			if (ctx->need_dump_rch_id[dpu_pstate->rch_id]) {
				gem_obj = to_dpu_gem_obj(new_plane_state->fb->obj[0]);
				format_map = dpu_format_get(FMT_MODULE_RCH, new_plane_state->fb->format->format,
						new_plane_state->fb->modifier);
				snprintf(f_name, sizeof(f_name),
						"%s_frame_no_%lld(%dth)_rch_id_%d(%d)_%d_%d_%s_yuv_tran(%d)_split(%d)_32x8(%d)_tile(%d).fbc",
						"/data/xdm/rch_dump/new",
						frame_no,
						dump_file_num,
						dpu_pstate->rch_id,
						new_rch_cnt[dpu_pstate->rch_id],
						new_plane_state->fb->width,
						new_plane_state->fb->height,
						format_map->dpu_format_name,
						DPU_FMT_HAS_AFBC_Y2R(new_plane_state->fb->modifier) > 0 ? 1 : 0,
						DPU_FMT_HAS_AFBC_SPLIT(new_plane_state->fb->modifier) > 0 ? 1 : 0,
						DPU_FMT_IS_AFBC_32x8(new_plane_state->fb->modifier) > 0 ? 1 : 0,
						DPU_FMT_HAS_AFBC_TILED(new_plane_state->fb->modifier) > 0 ? 1 : 0);
				DPU_WARN("dump buffer %s > %s\n", gem_obj->dmabuf->name, f_name);
				dump_file_num++;
				new_rch_cnt[dpu_pstate->rch_id]++;
			}
		}
	}
}

static void dpu_exception_dbg_irq_dump(struct work_struct *arg)
{
	struct dpu_dbg_irq_work *work_data;
	struct dpu_crtc_state *dpu_crtc_state;
	struct dpu_virt_pipeline *pipeline;
	struct dpu_hw_wb_top *hw_wb_top;
	struct drm_atomic_state *old_state;
	struct dpu_kms *dpu_kms;
	struct drm_crtc *crtc;
	char tmp_name[256] = {0};
	u64 frame_no;
	u32 pipe_id;
	u32 value;
	int i;

	work_data = container_of(arg, struct dpu_dbg_irq_work, work);
	dpu_kms = work_data->dpu_kms;
	crtc = work_data->crtc;
	if (!crtc) {
		DPU_DEBUG("crtc is null\n");
		return;
	}

	pipe_id = drm_crtc_index(crtc);
	pipeline = &dpu_kms->virt_pipeline[pipe_id];
	hw_wb_top = dpu_kms->virt_ctrl.hw_wb_top;
	dpu_crtc_state = to_dpu_crtc_state(crtc->state);
	frame_no = dpu_crtc_state->frame_no;

	/* protect commit_ctx.state */
	mutex_lock(&pipeline->commit_ctx.commit_lock);
	old_state = pipeline->commit_ctx.state;

	(void)snprintf(tmp_name, sizeof(tmp_name), "dpu_exception_frame_no_%llu",
			dpu_crtc_state->frame_no);
	trace_dpu_exception_dma_dbg_irq_dump(tmp_name);

	DPU_WARN("[CRTC:%d:%s (%s)][frame_no:%llu] dbg irq trigger\n",
			crtc->base.id,
			crtc->name,
			pipeline->is_offline ? "offline" : "online",
			dpu_crtc_state->frame_no);

	DPU_INFO("old_rch_mask: 0x%x, new_rch_mask: 0x%x\n",
			work_data->old_rch_mask, work_data->new_rch_mask);

	if (hw_wb_top) {
		value = work_data->wdma_dbg_irq_status;
		if (value != 0) {
			print_wdma_dbg_irq_status(hw_wb_top->hw.blk_id, value);
			work_data->wdma_dbg_irq_status = 0;
		}
	}

	for (i = 0; i < RCH_MAX; i++) {
		value = work_data->rch_dbg_irq_status[i];
		if (value != 0) {
			print_rdma_dbg_irq_status(i, value);
			work_data->rch_dbg_irq_status[i] = 0;
		}
	}

	if (work_data->plsverr_status) {
		DPU_INFO("plsverr status: 0x%x\n", work_data->plsverr_status);
		work_data->plsverr_status = 0;
	}

	if (work_data->cmdlist_ch_clr_timeout_status) {
		value = work_data->cmdlist_ch_clr_timeout_status;
		work_data->cmdlist_ch_clr_timeout_status = 0;
	}

	if (dpu_kms->dbg_ctx->need_dump_buf) {
		if (old_state)
			dbg_dump_input_buf_to_file(dpu_kms->dbg_ctx, old_state, frame_no);
		dpu_kms->dbg_ctx->need_dump_buf = false;
		memset(dpu_kms->dbg_ctx->need_dump_rch_id, 0,
				sizeof(dpu_kms->dbg_ctx->need_dump_rch_id));
	}

	work_data->complete_print = true;
	mutex_unlock(&pipeline->commit_ctx.commit_lock);
}

static void dpu_exception_dbg_irq_create_work(struct dpu_kms *dpu_kms)
{
	struct dpu_dbg_ctx *ctx;
	char name[64];
	int i;

	ctx = dpu_kms->dbg_ctx;

	for (i = 0; i < DPU_INTR_TYPE_MAX; i++) {
		snprintf(name, sizeof(name), "dpu_dbg_wq%d", i);
		ctx->wq[i] = create_singlethread_workqueue(name);
		ctx->work_data[i].complete_print = true;
		INIT_WORK(&ctx->work_data[i].work, dpu_exception_dbg_irq_dump);
	}
}

static void dpu_exception_dbg_irq_destroy_work(struct dpu_kms *dpu_kms)
{
	struct dpu_dbg_ctx *ctx;
	int i;

	ctx = dpu_kms->dbg_ctx;

	for (i = 0; i < DPU_INTR_TYPE_MAX; i++)
		destroy_workqueue(ctx->wq[i]);
}

void dpu_exception_online_dbg_irq_register(struct dpu_intr_context *intr_ctx)
{
	int ret;

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_DMA_DBG_PART1_RDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_ONLINE0_DMA_DBG_PART1_RDMA, ret %d\n", ret);
		goto err_online0_dma_dbg_part1;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_DMA_DBG_PART2_RDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_ONLINE0_DMA_DBG_PART2_RDMA, ret %d\n", ret);
		goto err_online0_dma_dbg_part2;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_DMA_DBG_WDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_ONLINE0_DMA_DBG_WDMA, ret %d\n", ret);
		goto err_online0_dma_dbg_wdma;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_ONLINE0_CMDLIST_DBG,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_ONLINE0_DMA_DBG_WDMA, ret %d\n", ret);
		goto err_online0_cmdlist_dbg;
	}

	return;

err_online0_cmdlist_dbg:
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DMA_DBG_WDMA);
err_online0_dma_dbg_wdma:
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DMA_DBG_PART2_RDMA);
err_online0_dma_dbg_part2:
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DMA_DBG_PART1_RDMA);
err_online0_dma_dbg_part1:
	return;
}

void dpu_exception_cmb_dbg_irq_register(struct dpu_intr_context *intr_ctx)
{
	int ret;

	ret = dpu_interrupt_register(intr_ctx, INTR_COMBO_DMA_DBG_PART1_RDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_COMBO_DMA_DBG_PART1_RDMA, ret %d\n", ret);
		goto err_cmb_dma_dbg_part1;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_COMBO_DMA_DBG_PART2_RDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_COMBO_DMA_DBG_PART2_RDMA, ret %d\n", ret);
		goto err_cmb_dma_dbg_part2;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_COMBO_DMA_DBG_WDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_COMBO_DMA_DBG_WDMA, ret %d\n", ret);
		goto err_cmb_dma_dbg_wdma;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_COMBO_CMDLIST_DBG,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_COMBO_CMDLIST_DBG, ret %d\n", ret);
		goto err_cmb_cmdlist_dbg;
	}

	return;

err_cmb_cmdlist_dbg:
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_DMA_DBG_WDMA);
err_cmb_dma_dbg_wdma:
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_DMA_DBG_PART2_RDMA);
err_cmb_dma_dbg_part2:
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_DMA_DBG_PART1_RDMA);
err_cmb_dma_dbg_part1:
	return;
}

void dpu_exception_offline_dbg_irq_register(struct dpu_intr_context *intr_ctx)
{
	int ret;

	ret = dpu_interrupt_register(intr_ctx, INTR_OFFLINE0_DMA_DBG_PART1_RDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_OFFLINE0_DMA_DBG_PART1_RDMA, ret %d\n", ret);
		goto err_offline_dma_dbg_part1;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_OFFLINE0_DMA_DBG_PART2_RDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_OFFLINE0_DMA_DBG_PART2_RDMA, ret %d\n", ret);
		goto err_offline_dma_dbg_part2;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_OFFLINE0_DMA_DBG_WDMA,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_OFFLINE0_DMA_DBG_WDMA, ret %d\n", ret);
		goto err_offline_dma_dbg_wdma;
	}

	ret = dpu_interrupt_register(intr_ctx, INTR_OFFLINE0_CMDLIST_DBG,
			&dpu_dbg_irq_handler);
	if (ret) {
		DPU_ERROR("failed to register INTR_OFFLINE0_CMDLIST_DBG, ret %d\n", ret);
		goto err_offline_cmdlist_dbg;
	}

	return;

err_offline_cmdlist_dbg:
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_DMA_DBG_WDMA);
err_offline_dma_dbg_wdma:
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_DMA_DBG_PART2_RDMA);
err_offline_dma_dbg_part2:
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_DMA_DBG_PART1_RDMA);
err_offline_dma_dbg_part1:
	return;
}

void dpu_exception_online_dbg_irq_unregister(struct dpu_intr_context *intr_ctx)
{
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DMA_DBG_PART1_RDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DMA_DBG_PART2_RDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_DMA_DBG_WDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_ONLINE0_CMDLIST_DBG);
}

void dpu_exception_cmb_dbg_irq_unregister(struct dpu_intr_context *intr_ctx)
{
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_DMA_DBG_PART1_RDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_DMA_DBG_PART2_RDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_DMA_DBG_WDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_COMBO_CMDLIST_DBG);
}

void dpu_exception_offline_dbg_irq_unregister(struct dpu_intr_context *intr_ctx)
{
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_DMA_DBG_PART1_RDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_DMA_DBG_PART2_RDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_DMA_DBG_WDMA);
	dpu_interrupt_unregister(intr_ctx, INTR_OFFLINE0_CMDLIST_DBG);
}

void dpu_exception_dbg_irq_enable(struct dpu_kms *dpu_kms,
		struct drm_crtc *crtc)
{
	struct dpu_virt_pipeline *pipeline;
	struct dpu_hw_rch *hw_rch;
	struct dpu_dbg_ctx *ctx;
	u32 pipe_id;
	int i;

	if (!dpu_kms)
		return;

	ctx = dpu_kms->dbg_ctx;
	pipe_id = drm_crtc_index(crtc);
	pipeline = &dpu_kms->virt_pipeline[pipe_id];

	for (i = 0; i < RCH_MAX; i++) {
		hw_rch = pipeline->pending_state.rch[i];

		if (!hw_rch)
			continue;

		hw_rch->ops.dbg_irq_enable(&hw_rch->hw, true);
	}
}

void dpu_exception_dbg_irq_clear(struct dpu_virt_pipeline *pipeline,
		struct drm_crtc *crtc)
{
	dpu_hw_status_clear(pipeline, crtc);
}

int dpu_exception_dbg_ctx_init(struct dpu_kms *dpu_kms)
{
	struct dpu_dbg_ctx *ctx;
	int ret = 0;
	int i;

	if (!dpu_kms) {
		DPU_ERROR("invalid parameter %pK\n", dpu_kms);
		return -EINVAL;
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;


	dpu_kms->dbg_ctx = ctx;

	for (i = 0; i < RCH_MAX; i++)
		dpu_kms->dbg_ctx->rch_hw_blk[i] = dpu_kms->res_mgr_ctx->rch_hw_blk[i];

	dpu_exception_dbg_irq_create_work(dpu_kms);

	ctx->dma_buf_debug_type = DPU_DMA_BUF_DEBUG_NONE;

	return ret;
}

void dpu_exception_dbg_ctx_deinit(struct dpu_kms *dpu_kms)
{
	struct dpu_dbg_ctx *ctx;
	int i, j;

	if (!dpu_kms) {
		PERF_ERROR("invalid parameter %pK\n", dpu_kms);
		return;
	}

	dpu_exception_dbg_irq_destroy_work(dpu_kms);

	ctx = dpu_kms->dbg_ctx;
	for (i = 0; i < MAX_PLANE_ZPOS + 1; i++) {
		for (j = 0; j < 2; j++)
			kfree(ctx->input_buf[j][i]);
	}
	kfree(ctx);
}

